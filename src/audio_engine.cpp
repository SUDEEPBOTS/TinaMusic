/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "audio_engine.hpp"
#include "tinaproto/client.hpp"
#include "logger.hpp"
#include <iostream>
#include <cstdlib>

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

    auto res = tinaproto::TinaProtoClient::GetInstance().Play(chat_id_, stream_url_);
    PlayResult out;
    out.ok = res.ok;
    out.error = res.error;
    out.detail = res.detail;

    if (out.ok) {
        start_time_ = std::chrono::steady_clock::now();
        is_running_ = true;
        is_paused_ = false;
        Logger::Info("AUDIO", "Pure C++20 TinaProto WebRTC playback started for chat: " + std::to_string(chat_id_));
    } else {
        is_running_ = false;
        Logger::Error("AUDIO", "Playback failed for chat " + std::to_string(chat_id_) + ": [" + out.error + "] " + out.detail);
    }
    return out;
}

void AudioStreamSession::Pause() {
    if (is_running_ && !is_paused_) {
        tinaproto::TinaProtoClient::GetInstance().Pause(chat_id_);
        is_paused_ = true;
        Logger::Info("AUDIO", "Paused playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Resume() {
    if (is_running_ && is_paused_) {
        tinaproto::TinaProtoClient::GetInstance().Resume(chat_id_);
        is_paused_ = false;
        Logger::Info("AUDIO", "Resumed playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Stop() {
    if (is_running_) {
        is_running_ = false;
        is_paused_ = false;
        Logger::Info("AUDIO", "Stopped playback session for chat: " + std::to_string(chat_id_));
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
    (void)chat_id;
    (void)invite_link;
    (void)username;
    return true;
}

std::string AudioEngine::GetAssistantUsername() {
    if (!assistant_username_.empty()) {
        return assistant_username_;
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
    tinaproto::TinaProtoClient::GetInstance().Stop(chat_id);
    current_urls_.erase(chat_id);
    Logger::Info("AUDIO", "Stopped playback and disconnected call for chat: " + std::to_string(chat_id));
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
