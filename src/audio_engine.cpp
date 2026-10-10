/*
 * ============================================================================
 *   ████████╗██╗███╗   ██╗ █████╗ ███╗   ███╗██╗   ██╗███████╗██╗ ██████╗
 *   ╚══██╔══╝██║████╗  ██║██╔══██╗████╗ ████║██║   ██║██╔════╝██║██╔════╝
 *      ██║   ██║██╔██╗ ██║███████║██╔████╔██║██║   ██║███████╗██║██║     
 *      ██║   ██║██║╚██╗██║██╔══██║██║╚██╔╝██║██║   ██║╚════██║██║██║     
 *      ██║   ██║██║ ╚████║██║  ██║██║ ╚═╝ ██║╚██████╔╝███████║██║╚██████╗
 *      ╚═╝   ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝
 * ============================================================================
 *
 * Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 * Architecture: Native C++20 Core + Go MTProto WebRTC Voice Bridge
 * Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 * Support: https://t.me/Z0iiw
 *
 * ----------------------------------------------------------------------------
 * LEGAL NOTICE & LICENSE (GNU Affero General Public License v3.0):
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * STRICT COPYLEFT CLAUSE:
 * In accordance with Section 13 of the GNU AGPLv3, if you modify this program
 * or run it as a network service (e.g., hosted on servers, Telegram, or cloud),
 * you MUST make the complete source code of your modified version available
 * to all users under the exact same AGPLv3 license terms.
 * Closed-source derivatives or rebranded copies are strictly prohibited.
 * ============================================================================
 */

#include "audio_engine.hpp"
#include "logger.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realsize);
    return realsize;
}

static PlayResult CallAssistantBridge(const std::string& endpoint, const nlohmann::json& payload) {
    PlayResult result;
    CURL* curl = curl_easy_init();
    if (!curl) {
        result.ok = false;
        result.error = "CURL_INIT_FAILED";
        return result;
    }
    std::string url = "http://127.0.0.1:8055/" + endpoint;
    std::string json_data = payload.dump();
    std::string response_buf;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buf);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        result.ok = false;
        result.error = "ASSISTANT_UNREACHABLE";
        result.detail = curl_easy_strerror(res);
        return result;
    }

    try {
        auto j = nlohmann::json::parse(response_buf);
        result.ok = j.value("ok", false);
        result.error = j.value("error", "");
        result.detail = j.value("detail", "");
        if (result.detail.empty()) {
            result.detail = response_buf;
        }
    } catch (...) {
        result.ok = (http_code == 200);
        result.error = "PARSE_ERROR";
        result.detail = response_buf;
    }
    return result;
}

AudioStreamSession::AudioStreamSession(int64_t chat_id, const std::string& stream_url, int64_t start_offset, double speed, int volume, const std::string& invite_link, const std::string& username)
    : chat_id_(chat_id), stream_url_(stream_url), start_offset_(start_offset), speed_(speed), volume_(volume),
      invite_link_(invite_link), username_(username),
      start_time_(std::chrono::steady_clock::now()) {}

AudioStreamSession::~AudioStreamSession() {
    Stop();
}

PlayResult AudioStreamSession::Start() {
    if (is_running_) {
        Stop();
    }

    nlohmann::json payload = {
        {"chat_id", chat_id_},
        {"url", stream_url_}
    };
    if (!invite_link_.empty()) payload["invite_link"] = invite_link_;
    if (!username_.empty()) payload["username"] = username_;

    PlayResult res = CallAssistantBridge("play", payload);

    if (res.ok) {
        start_time_ = std::chrono::steady_clock::now();
        is_running_ = true;
        is_paused_ = false;
        Logger::Info("AUDIO", "Fast assistant playback started for chat: " + std::to_string(chat_id_));
    } else {
        is_running_ = false;
        Logger::Error("AUDIO", "Playback failed for chat " + std::to_string(chat_id_) + ": [" + res.error + "] " + res.detail);
    }
    return res;
}

void AudioStreamSession::Pause() {
    if (is_running_ && !is_paused_) {
        CallAssistantBridge("pause", {{"chat_id", chat_id_}});
        is_paused_ = true;
        Logger::Info("AUDIO", "Paused playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Resume() {
    if (is_running_ && is_paused_) {
        CallAssistantBridge("resume", {{"chat_id", chat_id_}});
        is_paused_ = false;
        Logger::Info("AUDIO", "Resumed playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Stop() {
    if (is_running_) {
        CallAssistantBridge("leave", {{"chat_id", chat_id_}});
        is_running_ = false;
        is_paused_ = false;
        Logger::Info("AUDIO", "Stopped playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Mute() {
    is_muted_ = true;
    Logger::Info("AUDIO", "Muted playback for chat: " + std::to_string(chat_id_));
}

void AudioStreamSession::Unmute() {
    is_muted_ = false;
    Logger::Info("AUDIO", "Unmuted playback for chat: " + std::to_string(chat_id_));
}

bool AudioStreamSession::IsMuted() const {
    return is_muted_;
}

bool AudioStreamSession::IsRunning() const {
    return is_running_;
}

int64_t AudioStreamSession::GetCurrentPosition() const {
    if (!is_running_) return 0;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count();
    return start_offset_ + static_cast<int64_t>(elapsed * speed_);
}

AudioEngine& AudioEngine::Instance() {
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::VerifyFFmpeg() {
    int ret = system("ffmpeg -version > /dev/null 2>&1");
    return (ret == 0);
}

PlayResult AudioEngine::StartPlayback(int64_t chat_id, const std::string& stream_url, int64_t start_offset, double speed, int volume, const std::string& invite_link, const std::string& username) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Stop();
    }

    auto session = std::make_shared<AudioStreamSession>(chat_id, stream_url, start_offset, speed, volume, invite_link, username);
    PlayResult res = session->Start();
    if (res.ok) {
        sessions_[chat_id] = session;
        current_urls_[chat_id] = stream_url;
    } else {
        sessions_.erase(chat_id);
        current_urls_.erase(chat_id);
    }
    return res;
}

bool AudioEngine::JoinChat(int64_t chat_id, const std::string& invite_link, const std::string& username) {
    nlohmann::json payload = {{"chat_id", chat_id}};
    if (!invite_link.empty()) payload["invite_link"] = invite_link;
    if (!username.empty()) payload["username"] = username;
    PlayResult res = CallAssistantBridge("join", payload);
    return res.ok;
}

std::string AudioEngine::GetAssistantUsername() {
    if (!assistant_username_.empty()) {
        return assistant_username_;
    }
    PlayResult res = CallAssistantBridge("status", nlohmann::json::object());
    if (res.ok && !res.detail.empty()) {
        try {
            auto j = nlohmann::json::parse(res.detail);
            if (j.contains("assistant_username")) {
                assistant_username_ = j["assistant_username"].get<std::string>();
                return assistant_username_;
            }
        } catch (...) {}
    }
    return "Yzcxxy";
}

void AudioEngine::PausePlayback(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Pause();
    }
}

void AudioEngine::ResumePlayback(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Resume();
    }
}

void AudioEngine::StopPlayback(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Stop();
        sessions_.erase(it);
    }
    current_urls_.erase(chat_id);
}

void AudioEngine::MutePlayback(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Mute();
    }
}

void AudioEngine::UnmutePlayback(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Unmute();
    }
}

bool AudioEngine::IsMuted(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        return it->second->IsMuted();
    }
    return false;
}

bool AudioEngine::SeekPlayback(int64_t chat_id, int64_t seconds) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    auto url_it = current_urls_.find(chat_id);
    if (it != sessions_.end() && url_it != current_urls_.end()) {
        std::string url = url_it->second;
        it->second->Stop();
        auto session = std::make_shared<AudioStreamSession>(chat_id, url, seconds);
        PlayResult res = session->Start();
        if (res.ok) {
            sessions_[chat_id] = session;
            return true;
        }
    }
    return false;
}

bool AudioEngine::SetPlaybackSpeed(int64_t chat_id, double speed) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    auto url_it = current_urls_.find(chat_id);
    if (it != sessions_.end() && url_it != current_urls_.end()) {
        int64_t pos = it->second->GetCurrentPosition();
        std::string url = url_it->second;
        it->second->Stop();
        auto session = std::make_shared<AudioStreamSession>(chat_id, url, pos, speed);
        PlayResult res = session->Start();
        if (res.ok) {
            sessions_[chat_id] = session;
            return true;
        }
    }
    return false;
}

bool AudioEngine::IsActive(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    return it != sessions_.end() && it->second->IsRunning();
}

bool AudioEngine::IsPlaying(int64_t chat_id) {
    return IsActive(chat_id);
}

int64_t AudioEngine::GetPosition(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        return it->second->GetCurrentPosition();
    }
    return 0;
}

std::vector<int64_t> AudioEngine::GetActiveChats() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    std::vector<int64_t> result;
    for (const auto& [cid, sess] : sessions_) {
        if (sess && sess->IsRunning()) {
            result.push_back(cid);
        }
    }
    return result;
}
