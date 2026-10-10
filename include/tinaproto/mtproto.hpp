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
#include "tinaproto/tl_codec.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>

namespace tinaproto {

class MtprotoClient {
public:
    MtprotoClient();
    ~MtprotoClient();

    bool Connect(const SessionData& session);
    void Disconnect();
    bool IsConnected() const { return connected_; }

    std::vector<uint8_t> SendRpc(const std::vector<uint8_t>& rpc_query);

    bool InitConnection(uint32_t api_id);

    bool LoadPeerCache(const std::string& cache_path);

    bool ResolveChannel(int64_t chat_id, int64_t& out_channel_id, int64_t& out_access_hash);

    std::optional<InputGroupCall> GetGroupCall(int64_t chat_id);

    std::string JoinGroupCall(int64_t chat_id, const std::string& local_sdp_json);

    bool LeaveGroupCall(int64_t chat_id);

private:
    int sock_{-1};
    bool connected_{false};
    SessionData session_;
    std::vector<uint8_t> auth_key_id_;
    int64_t session_id_{0};
    int64_t salt_{0};
    int32_t seq_no_{1};
    std::mutex mtx_;

    std::unordered_map<int64_t, int64_t> channel_access_hashes_;
    std::unordered_map<int64_t, int64_t> user_access_hashes_;
    std::unordered_map<int64_t, InputGroupCall> group_calls_;

    bool SendPacket(const std::vector<uint8_t>& payload, int32_t seq_no, int64_t msg_id);
    std::vector<uint8_t> ReceivePacket();
    std::vector<uint8_t> ExtractRpcResult(const std::vector<uint8_t>& payload);
};

}
