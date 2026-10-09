#include "audio_engine.hpp"
#include "logger.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

AudioStreamSession::AudioStreamSession(int64_t chat_id, const std::string& stream_url)
    : chat_id_(chat_id), stream_url_(stream_url) {}

AudioStreamSession::~AudioStreamSession() {
    Stop();
}

bool AudioStreamSession::Start() {
    if (is_running_) {
        Stop();
    }

    pid_t pid = fork();
    if (pid < 0) {
        Logger::Error("AUDIO", "Failed to fork process for FFmpeg");
        return false;
    }

    if (pid == 0) {
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull >= 0) {
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        std::vector<const char*> args = {
            "ffmpeg",
            "-reconnect", "1",
            "-reconnect_streamed", "1",
            "-reconnect_delay_max", "5",
            "-i", stream_url_.c_str(),
            "-vn",
            "-f", "s16le",
            "-ac", "2",
            "-ar", "48000",
            "-acodec", "pcm_s16le",
            "/dev/null",
            nullptr
        };

        execvp("ffmpeg", const_cast<char* const*>(args.data()));
        _exit(1);
    }

    ffmpeg_pid_ = pid;
    is_running_ = true;
    is_paused_ = false;
    Logger::Info("AUDIO", "Started audio pipeline PID " + std::to_string(pid) + " for chat " + std::to_string(chat_id_));
    return true;
}

void AudioStreamSession::Pause() {
    if (is_running_ && !is_paused_ && ffmpeg_pid_ > 0) {
        kill(ffmpeg_pid_, SIGSTOP);
        is_paused_ = true;
        Logger::Info("AUDIO", "Paused audio stream for chat " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Resume() {
    if (is_running_ && is_paused_ && ffmpeg_pid_ > 0) {
        kill(ffmpeg_pid_, SIGCONT);
        is_paused_ = false;
        Logger::Info("AUDIO", "Resumed audio stream for chat " + std::to_string(chat_id_));
    }
}

void AudioStreamSession::Stop() {
    if (is_running_ && ffmpeg_pid_ > 0) {
        kill(ffmpeg_pid_, SIGTERM);
        int status;
        waitpid(ffmpeg_pid_, &status, WNOHANG);
        Logger::Info("AUDIO", "Terminated audio pipeline PID " + std::to_string(ffmpeg_pid_));
        ffmpeg_pid_ = -1;
    }
    is_running_ = false;
    is_paused_ = false;
}

bool AudioStreamSession::IsRunning() const {
    return is_running_;
}

AudioEngine& AudioEngine::Instance() {
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::VerifyFFmpeg() {
    int res = system("ffmpeg -version > /dev/null 2>&1");
    return (res == 0);
}

bool AudioEngine::StartPlayback(int64_t chat_id, const std::string& audio_url) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        it->second->Stop();
    }

    auto session = std::make_shared<AudioStreamSession>(chat_id, audio_url);
    if (session->Start()) {
        sessions_[chat_id] = session;
        return true;
    }
    return false;
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
}

bool AudioEngine::IsActive(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(chat_id);
    if (it != sessions_.end()) {
        return it->second->IsRunning();
    }
    return false;
}
