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

#pragma once

#include "models.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

struct PlayResult {
    bool ok{false};
    std::string error;
    std::string detail;
};

class AudioStreamSession {
public:
    AudioStreamSession(int64_t chat_id, const std::string& stream_url, int64_t start_offset = 0, double speed = 1.0, int volume = 100);
    ~AudioStreamSession();

    PlayResult Start();
    void Pause();
    void Resume();
    void Stop();
    void Mute();
    void Unmute();
    bool IsMuted() const;
    bool IsRunning() const;

    int64_t GetCurrentPosition() const;
    void SetVolume(int vol);
    void SetSpeed(double spd);

private:
    int64_t chat_id_{0};
    std::string stream_url_;
    int64_t start_offset_{0};
    double speed_{1.0};
    int volume_{100};
    std::atomic<bool> is_paused_{false};
    std::atomic<bool> is_muted_{false};
    std::atomic<bool> is_running_{false};
    std::chrono::steady_clock::time_point start_time_;
};

class AudioEngine {
public:
    static AudioEngine& Instance();

    bool VerifyFFmpeg();
    PlayResult StartPlayback(int64_t chat_id, const std::string& audio_url, int64_t start_offset = 0, double speed = 1.0, int volume = 100);
    void PausePlayback(int64_t chat_id);
    void ResumePlayback(int64_t chat_id);
    void StopPlayback(int64_t chat_id);
    void MutePlayback(int64_t chat_id);
    void UnmutePlayback(int64_t chat_id);
    bool IsMuted(int64_t chat_id);
    bool SeekPlayback(int64_t chat_id, int64_t seconds);
    bool SetPlaybackSpeed(int64_t chat_id, double speed);
    bool IsActive(int64_t chat_id);
    bool IsPlaying(int64_t chat_id);
    int64_t GetPosition(int64_t chat_id);
    std::vector<int64_t> GetActiveChats();

private:
    AudioEngine() = default;
    ~AudioEngine() = default;

    std::unordered_map<int64_t, std::shared_ptr<AudioStreamSession>> sessions_;
    std::unordered_map<int64_t, std::string> current_urls_;
    std::mutex sessions_mutex_;
};
