/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#pragma once

#include <string>
#include <functional>
#include <cstdint>
#include <mutex>
#include <unordered_set>

namespace tinaproto {

class VoiceBinding {
public:
    VoiceBinding();
    ~VoiceBinding();

    bool Init();

    std::string CreateCall(int64_t chat_id);

    bool SetStreamSources(int64_t chat_id, const std::string& shell_cmd);

    bool Connect(int64_t chat_id, const std::string& remote_params);

    bool Pause(int64_t chat_id);

    bool Resume(int64_t chat_id);

    bool Stop(int64_t chat_id);

    int64_t Time(int64_t chat_id);

    void SetOnStreamEnd(std::function<void(int64_t)> cb);

    bool HasActiveCall(int64_t chat_id);

private:
    uintptr_t handle_{0};
    std::mutex calls_mtx_;
    std::unordered_set<int64_t> active_calls_;
    std::function<void(int64_t)> on_stream_end_cb_;

    static void StreamEndThunk(uintptr_t ptr, int64_t chat_id, int stream_type, int stream_device, void* user_data);
};

}
