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

static void CallAssistantBridge(const std::string& endpoint, const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) return;
    std::string url = "http://127.0.0.1:8055/" + endpoint;
    std::string json_data = payload.dump();
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

AudioStreamSession::AudioStreamSession(int64_t chat_id, const std::string& stream_url, int64_t start_offset, double speed, int volume)
    : chat_id_(chat_id), stream_url_(stream_url), start_offset_(start_offset), speed_(speed), volume_(volume),
      start_time_(std::chrono::steady_clock::now()) {}

AudioStreamSession::~AudioStreamSession() {
    Stop();
}

bool AudioStreamSession::Start() {
    if (is_running_) {
        Stop();
    }

    start_time_ = std::chrono::steady_clock::now();
    is_running_ = true;
    is_paused_ = false;

    CallAssistantBridge("play", {
        {"chat_id", chat_id_},
        {"url", stream_url_}
    });

    pid_t pid = fork();
    if (pid == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0) {
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        std::stringstream ssOffset;
        ssOffset << start_offset_;
        std::string offsetStr = ssOffset.str();

        std::stringstream ssFilter;
        ssFilter << "atempo=" << speed_ << ",volume=" << (volume_ / 100.0);
        std::string filterStr = ssFilter.str();

        execlp("ffmpeg", "ffmpeg", "-y", "-ss", offsetStr.c_str(), "-i", stream_url_.c_str(),
               "-filter:a", filterStr.c_str(), "-f", "s16le", "-ac", "2", "-ar", "48000",
               "-loglevel", "quiet", "/dev/null", nullptr);
        _exit(1);
    } else if (pid > 0) {
        ffmpeg_pid_ = pid;
        Logger::Info("AUDIO", "Started audio pipeline for chat: " + std::to_string(chat_id_) + " [PID: " + std::to_string(pid) + "]");
        return true;
    }

    is_running_ = false;
    return false;
}

void AudioStreamSession::Pause() {
    if (is_running_ && !is_paused_) {
        CallAssistantBridge("pause", {{"chat_id", chat_id_}});
        if (ffmpeg_pid_ > 0) {
            kill(ffmpeg_pid_, SIGSTOP);
        }
        is_paused_ = true;
        Logger::Info("AUDIO", "Paused playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Resume() {
    if (is_running_ && is_paused_) {
        CallAssistantBridge("resume", {{"chat_id", chat_id_}});
        if (ffmpeg_pid_ > 0) {
            kill(ffmpeg_pid_, SIGCONT);
        }
        is_paused_ = false;
        Logger::Info("AUDIO", "Resumed playback for chat: " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Stop() {
    if (is_running_) {
        CallAssistantBridge("leave", {{"chat_id", chat_id_}});
        if (ffmpeg_pid_ > 0) {
            kill(ffmpeg_pid_, SIGTERM);
            int status;
            waitpid(ffmpeg_pid_, &status, WNOHANG);
            ffmpeg_pid_ = -1;
        }
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

bool AudioEngine::StartPlayback(int64_t chat_id, const std::string& stream_url, int64_t start_offset, double speed, int volume) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Stop();
    }

    auto session = std::make_shared<AudioStreamSession>(chat_id, stream_url, start_offset, speed, volume);
    sessions_[chat_id] = session;
    current_urls_[chat_id] = stream_url;
    return session->Start();
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
        sessions_[chat_id] = session;
        return session->Start();
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
        sessions_[chat_id] = session;
        return session->Start();
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
