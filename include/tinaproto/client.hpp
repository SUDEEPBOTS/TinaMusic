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

#include "tinaproto/session.hpp"
#include "tinaproto/mtproto.hpp"
#include "tinaproto/voice_binding.hpp"
#include <string>
#include <functional>
#include <memory>
#include <mutex>

namespace tinaproto {

struct StreamResult {
    bool ok{false};
    std::string error;
    std::string detail;
};

class TinaProtoClient {
public:
    static TinaProtoClient& GetInstance();

    bool Initialize(const std::string& session_str, int32_t api_id, const std::string& api_hash);

    StreamResult Play(int64_t chat_id, const std::string& stream_url);

    void Pause(int64_t chat_id);

    void Resume(int64_t chat_id);

    void Stop(int64_t chat_id);

    int64_t Time(int64_t chat_id);

    void SetOnStreamEnd(std::function<void(int64_t)> cb);

    std::optional<InputGroupCall> GetGroupCall(int64_t chat_id);

    bool HasActiveCall(int64_t chat_id);

    bool IsInitialized() const { return initialized_; }

private:
    TinaProtoClient();
    ~TinaProtoClient();

    bool initialized_{false};
    std::mutex mtx_;
    MtprotoClient mtproto_;
    VoiceBinding voice_binding_;
    std::function<void(int64_t)> on_stream_end_cb_;

    std::string BuildFfmpegCommand(const std::string& url);
};

}
