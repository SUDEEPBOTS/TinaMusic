/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/client.hpp"
#include "logger.hpp"

namespace tinaproto {

TinaProtoClient& TinaProtoClient::GetInstance() {
    static TinaProtoClient instance;
    return instance;
}

TinaProtoClient::TinaProtoClient() = default;

TinaProtoClient::~TinaProtoClient() = default;

std::string TinaProtoClient::BuildFfmpegCommand(const std::string& url) {
    std::string base = "ffmpeg ";
    if (url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0) {
        base += "-reconnect 1 -reconnect_at_eof 1 -reconnect_streamed 1 -reconnect_delay_max 2 ";
    }
    base += "-v quiet -i \"" + url + "\" -f s16le -ac 2 -ar 48000 pipe:1";
    return base;
}

bool TinaProtoClient::Initialize(const std::string& session_str, int32_t api_id, const std::string& api_hash) {
    (void)api_hash;
    std::lock_guard<std::mutex> lock(mtx_);
    if (initialized_) {
        return true;
    }

    auto sess_opt = SessionDecoder::DecodePyrogram(session_str);
    if (!sess_opt) {
        Logger::Error("TINAPROTO", "Failed to decode Pyrogram session string");
        return false;
    }

    if (!mtproto_.Connect(*sess_opt)) {
        Logger::Error("TINAPROTO", "Failed to connect MTProto transport");
        return false;
    }

    if (!mtproto_.InitConnection(static_cast<uint32_t>(api_id))) {
        Logger::Error("TINAPROTO", "Failed to initialize MTProto connection layer");
        return false;
    }

    mtproto_.LoadPeerCache("data/peer_cache.json");

    if (!voice_binding_.Init()) {
        Logger::Error("TINAPROTO", "Failed to initialize native NTgCalls engine");
        return false;
    }

    voice_binding_.SetOnStreamEnd([this](int64_t chat_id) {
        Logger::Info("TINAPROTO", "Voice stream ended for chat " + std::to_string(chat_id));
        if (on_stream_end_cb_) {
            on_stream_end_cb_(chat_id);
        }
    });

    initialized_ = true;
    Logger::Info("TINAPROTO", "Pure C++20 TinaProto voice engine initialized successfully");
    return true;
}

StreamResult TinaProtoClient::Play(int64_t chat_id, const std::string& stream_url) {
    std::lock_guard<std::mutex> lock(mtx_);
    StreamResult result;
    if (!initialized_) {
        result.ok = false;
        result.error = "NOT_INITIALIZED";
        result.detail = "TinaProto client has not been initialized";
        return result;
    }

    std::string cmd = BuildFfmpegCommand(stream_url);

    if (voice_binding_.HasActiveCall(chat_id)) {
        if (voice_binding_.SetStreamSources(chat_id, cmd)) {
            result.ok = true;
            Logger::Info("TINAPROTO", "Updated active stream sources for chat " + std::to_string(chat_id));
            return result;
        } else {
            result.ok = false;
            result.error = "SET_SOURCES_FAILED";
            result.detail = "Failed to update stream sources for active call";
            return result;
        }
    }

    std::string local_sdp = voice_binding_.CreateCall(chat_id);
    if (local_sdp.empty()) {
        result.ok = false;
        result.error = "CALL_CREATE_FAILED";
        result.detail = "libntgcalls failed to generate local WebRTC SDP offer";
        return result;
    }

    if (!voice_binding_.SetStreamSources(chat_id, cmd)) {
        voice_binding_.Stop(chat_id);
        result.ok = false;
        result.error = "SET_SOURCES_FAILED";
        result.detail = "libntgcalls failed to attach audio stream command";
        return result;
    }

    std::string remote_sdp = mtproto_.JoinGroupCall(chat_id, local_sdp);
    if (remote_sdp.empty() || remote_sdp == "{\"transport\": null}") {
        voice_binding_.Stop(chat_id);
        result.ok = false;
        result.error = "JOIN_GROUP_CALL_FAILED";
        result.detail = "Telegram MTProto rejected joinGroupCall or returned empty transport";
        return result;
    }

    if (!voice_binding_.Connect(chat_id, remote_sdp)) {
        voice_binding_.Stop(chat_id);
        mtproto_.LeaveGroupCall(chat_id);
        result.ok = false;
        result.error = "WEBRTC_CONNECT_FAILED";
        result.detail = "libntgcalls failed to establish WebRTC connection with Telegram server";
        return result;
    }

    result.ok = true;
    Logger::Info("TINAPROTO", "WebRTC voice call connected and streaming for chat " + std::to_string(chat_id));
    return result;
}

void TinaProtoClient::Pause(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    voice_binding_.Pause(chat_id);
}

void TinaProtoClient::Resume(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    voice_binding_.Resume(chat_id);
}

void TinaProtoClient::Stop(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    voice_binding_.Stop(chat_id);
    mtproto_.LeaveGroupCall(chat_id);
}

int64_t TinaProtoClient::Time(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    return voice_binding_.Time(chat_id);
}

void TinaProtoClient::SetOnStreamEnd(std::function<void(int64_t)> cb) {
    std::lock_guard<std::mutex> lock(mtx_);
    on_stream_end_cb_ = std::move(cb);
}

std::optional<InputGroupCall> TinaProtoClient::GetGroupCall(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    return mtproto_.GetGroupCall(chat_id);
}

}
