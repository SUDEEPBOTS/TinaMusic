#pragma once

#include "models.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>

class AudioStreamSession {
public:
    AudioStreamSession(int64_t chat_id, const std::string& stream_url);
    ~AudioStreamSession();

    bool Start();
    void Pause();
    void Resume();
    void Stop();
    bool IsRunning() const;

private:
    int64_t chat_id_{0};
    std::string stream_url_;
    pid_t ffmpeg_pid_{-1};
    std::atomic<bool> is_paused_{false};
    std::atomic<bool> is_running_{false};
};

class AudioEngine {
public:
    static AudioEngine& Instance();

    bool VerifyFFmpeg();
    bool StartPlayback(int64_t chat_id, const std::string& audio_url);
    void PausePlayback(int64_t chat_id);
    void ResumePlayback(int64_t chat_id);
    void StopPlayback(int64_t chat_id);
    bool IsActive(int64_t chat_id);

private:
    AudioEngine() = default;
    ~AudioEngine() = default;

    std::unordered_map<int64_t, std::shared_ptr<AudioStreamSession>> sessions_;
    std::mutex sessions_mutex_;
};
