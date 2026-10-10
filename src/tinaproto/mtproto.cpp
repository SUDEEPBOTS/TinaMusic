/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/mtproto.hpp"
#include "tinaproto/crypto.hpp"
#include "logger.hpp"
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sys/random.h>

namespace tinaproto {

MtprotoClient::MtprotoClient() = default;

MtprotoClient::~MtprotoClient() {
    Disconnect();
}

void MtprotoClient::Disconnect() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (sock_ >= 0) {
        close(sock_);
        sock_ = -1;
    }
    connected_ = false;
}

bool MtprotoClient::Connect(const SessionData& session) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (connected_) {
        return true;
    }
    session_ = session;
    auth_key_id_ = Crypto::ComputeAuthKeyId(session_.auth_key);
    session_id_ = Crypto::GenerateSessionId();
    salt_ = 0;
    seq_no_ = 1;

    std::string key_id_hex;
    for (auto b : auth_key_id_) {
        char h[4];
        snprintf(h, sizeof(h), "%02x", b);
        key_id_hex += h;
    }
    Logger::Info("TINAPROTO", "Auth key size: " + std::to_string(session_.auth_key.size()) + " auth_key_id: " + key_id_hex);

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        Logger::Error("TINAPROTO", "Failed to create TCP socket for DC " + session_.dc_ip);
        return false;
    }

    struct timeval tv{};
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
    setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(session_.dc_port);
    inet_pton(AF_INET, session_.dc_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        Logger::Error("TINAPROTO", "Failed to connect to DC " + session_.dc_ip + ":" + std::to_string(session_.dc_port));
        close(sock_);
        sock_ = -1;
        return false;
    }

    uint8_t handshake[4] = {0xee, 0xee, 0xee, 0xee};
    if (send(sock_, handshake, 4, 0) != 4) {
        Logger::Error("TINAPROTO", "Failed to send intermediate transport handshake");
        close(sock_);
        sock_ = -1;
        return false;
    }

    connected_ = true;
    Logger::Info("TINAPROTO", "Connected to Telegram DC " + session_.dc_ip + ":" + std::to_string(session_.dc_port));
    return true;
}

bool MtprotoClient::SendPacket(const std::vector<uint8_t>& payload, int32_t seq_no, int64_t msg_id) {
    TlWriter pw;
    pw.PutInt64(salt_);
    pw.PutInt64(session_id_);
    pw.PutInt64(msg_id);
    pw.PutInt32(seq_no);
    pw.PutInt32(static_cast<int32_t>(payload.size()));
    pw.PutRawBytes(payload.data(), payload.size());

    std::vector<uint8_t> plaintext = pw.TakeData();
    size_t total_len = plaintext.size();
    size_t pad_len = 16 - (total_len % 16);
    if (pad_len < 12) pad_len += 16;
    std::vector<uint8_t> pad_bytes(pad_len);
    getrandom(pad_bytes.data(), pad_len, 0);
    plaintext.insert(plaintext.end(), pad_bytes.begin(), pad_bytes.end());

    std::vector<uint8_t> msg_key = Crypto::ComputeMsgKey(session_.auth_key.data() + 88, plaintext.data(), plaintext.size());

    uint8_t aes_key[32];
    uint8_t aes_iv[32];
    Crypto::DeriveAesKeyIv(session_.auth_key.data(), msg_key.data(), false, aes_key, aes_iv);

    std::vector<uint8_t> encrypted(plaintext.size());
    Crypto::AesIgeEncrypt(plaintext.data(), encrypted.data(), plaintext.size(), aes_key, aes_iv);

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), auth_key_id_.begin(), auth_key_id_.end());
    packet.insert(packet.end(), msg_key.begin(), msg_key.end());
    packet.insert(packet.end(), encrypted.begin(), encrypted.end());

    TlWriter wire_w;
    wire_w.PutInt32(static_cast<int32_t>(packet.size()));
    wire_w.PutRawBytes(packet.data(), packet.size());

    const auto& wire = wire_w.GetData();
    ssize_t sent = send(sock_, wire.data(), wire.size(), 0);
    if (sent != static_cast<ssize_t>(wire.size())) {
        Logger::Error("TINAPROTO", "send failed: sent=" + std::to_string(sent) + " wire=" + std::to_string(wire.size()) + " errno=" + std::to_string(errno) + " (" + strerror(errno) + ")");
    }
    return (sent == static_cast<ssize_t>(wire.size()));
}

std::vector<uint8_t> MtprotoClient::ReceivePacket() {
    uint32_t resp_len = 0;
    ssize_t r = recv(sock_, &resp_len, 4, MSG_WAITALL);
    if (r != 4) {
        Logger::Error("TINAPROTO", "recv header failed: r=" + std::to_string(r) + " errno=" + std::to_string(errno) + " (" + strerror(errno) + ")");
        return {};
    }
    if (resp_len < 24) {
        std::vector<uint8_t> err_buf(resp_len);
        if (resp_len > 0) {
            recv(sock_, err_buf.data(), resp_len, MSG_WAITALL);
        }
        std::string err_hex;
        for (auto b : err_buf) {
            char h[4];
            snprintf(h, sizeof(h), "%02x ", b);
            err_hex += h;
        }
        Logger::Error("TINAPROTO", "Received short packet resp_len=" + std::to_string(resp_len) + " payload_hex=[" + err_hex + "]");
        return {};
    }

    std::vector<uint8_t> resp_buf(resp_len);
    size_t received = 0;
    while (received < resp_len) {
        ssize_t n = recv(sock_, resp_buf.data() + received, resp_len - received, 0);
        if (n <= 0) break;
        received += n;
    }
    if (received != resp_len) {
        return {};
    }

    std::vector<uint8_t> resp_msg_key(resp_buf.begin() + 8, resp_buf.begin() + 24);
    std::vector<uint8_t> resp_encrypted(resp_buf.begin() + 24, resp_buf.end());

    uint8_t resp_aes_key[32];
    uint8_t resp_aes_iv[32];
    Crypto::DeriveAesKeyIv(session_.auth_key.data(), resp_msg_key.data(), true, resp_aes_key, resp_aes_iv);

    std::vector<uint8_t> resp_plaintext(resp_encrypted.size());
    Crypto::AesIgeDecrypt(resp_encrypted.data(), resp_plaintext.data(), resp_encrypted.size(), resp_aes_key, resp_aes_iv);

    std::vector<uint8_t> verify_msg_key = Crypto::ComputeMsgKey(session_.auth_key.data() + 96, resp_plaintext.data(), resp_plaintext.size());
    if (std::memcmp(verify_msg_key.data(), resp_msg_key.data(), 16) != 0) {
        Logger::Error("TINAPROTO", "Incoming packet MsgKey verification failed");
        return {};
    }

    int64_t new_salt = *reinterpret_cast<int64_t*>(resp_plaintext.data());
    if (new_salt != 0) {
        salt_ = new_salt;
    }

    int32_t data_len = *reinterpret_cast<int32_t*>(resp_plaintext.data() + 28);
    if (data_len <= 0 || static_cast<size_t>(32 + data_len) > resp_plaintext.size()) {
        return {};
    }

    std::vector<uint8_t> payload(resp_plaintext.begin() + 32, resp_plaintext.begin() + 32 + data_len);
    return payload;
}

std::vector<uint8_t> MtprotoClient::ExtractRpcResult(const std::vector<uint8_t>& payload) {
    if (payload.size() < 4) return {};
    uint32_t constructor = *reinterpret_cast<const uint32_t*>(payload.data());

    if (constructor == 0x73f1f8dc) {
        TlReader r(payload);
        r.ReadUint32();
        int32_t count = r.ReadInt32();
        for (int32_t i = 0; i < count; ++i) {
            r.ReadInt64();
            r.ReadInt32();
            int32_t inner_bytes = r.ReadInt32();
            std::vector<uint8_t> inner = r.ReadRawBytes(static_cast<size_t>(inner_bytes));
            if (!inner.empty()) {
                uint32_t inner_c = *reinterpret_cast<const uint32_t*>(inner.data());
                if (inner_c == 0x9ec20908 && inner.size() >= 28) {
                    salt_ = *reinterpret_cast<const int64_t*>(inner.data() + 20);
                }
                std::vector<uint8_t> res = ExtractRpcResult(inner);
                if (!res.empty()) return res;
            }
        }
    } else if (constructor == 0xf35c6d01) {
        TlReader r(payload);
        r.ReadUint32();
        r.ReadInt64();
        return r.ReadRawBytes(r.Remaining());
    } else if (constructor == 0x9ec20908 && payload.size() >= 28) {
        salt_ = *reinterpret_cast<const int64_t*>(payload.data() + 20);
        return {};
    } else if (constructor == 0x62d6b459) {
        return {};
    } else {
        return payload;
    }
    return {};
}

std::vector<uint8_t> MtprotoClient::SendRpc(const std::vector<uint8_t>& rpc_query) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!connected_) {
        return {};
    }

    int64_t msg_id = Crypto::GenerateMsgId();
    int32_t cur_seq = seq_no_;
    seq_no_ += 2;

    if (!SendPacket(rpc_query, cur_seq, msg_id)) {
        return {};
    }

    for (int attempts = 0; attempts < 5; ++attempts) {
        std::vector<uint8_t> resp = ReceivePacket();
        if (resp.empty()) {
            Logger::Error("TINAPROTO", "ReceivePacket returned empty on attempt " + std::to_string(attempts));
            return {};
        }

        uint32_t constructor = *reinterpret_cast<const uint32_t*>(resp.data());
        Logger::Info("TINAPROTO", "Attempt " + std::to_string(attempts) + " received constructor 0x" + std::to_string(constructor) + " size " + std::to_string(resp.size()));
        if (constructor == 0xedab447b) {
            TlReader r(resp);
            r.ReadUint32();
            r.ReadInt64();
            r.ReadInt32();
            r.ReadInt32();
            salt_ = r.ReadInt64();
            Logger::Info("TINAPROTO", "Updated server salt, resending RPC");

            msg_id = Crypto::GenerateMsgId();
            cur_seq = seq_no_;
            seq_no_ += 2;
            if (!SendPacket(rpc_query, cur_seq, msg_id)) {
                return {};
            }
            continue;
        }

        std::vector<uint8_t> rpc_res = ExtractRpcResult(resp);
        if (!rpc_res.empty()) {
            return rpc_res;
        }
    }

    return {};
}

bool MtprotoClient::InitConnection(uint32_t api_id) {
    TlWriter w;
    w.PutUint32(0xda9b0d0d);
    w.PutInt32(173);
    w.PutUint32(0xc1cd5ea9);
    w.PutUint32(0);
    w.PutInt32(static_cast<int32_t>(api_id));
    w.PutString("TinaMusic C++20");
    w.PutString("Linux x86_64");
    w.PutString("2.0.0");
    w.PutString("en");
    w.PutString("");
    w.PutString("en");
    w.PutUint32(0xc4f9186b);

    std::vector<uint8_t> resp = SendRpc(w.TakeData());
    if (resp.empty()) {
        Logger::Error("TINAPROTO", "initConnection RPC failed");
        return false;
    }
    Logger::Info("TINAPROTO", "initConnection succeeded! Response size: " + std::to_string(resp.size()) + " bytes");
    return true;
}

bool MtprotoClient::LoadPeerCache(const std::string& cache_path) {
    std::ifstream f(cache_path);
    if (!f.is_open()) {
        Logger::Warn("TINAPROTO", "Peer cache file not found: " + cache_path);
        return false;
    }
    try {
        nlohmann::json j;
        f >> j;
        if (j.contains("InputChannels")) {
            for (auto& [k, v] : j["InputChannels"].items()) {
                channel_access_hashes_[std::stoll(k)] = v.get<int64_t>();
            }
        }
        if (j.contains("InputUsers")) {
            for (auto& [k, v] : j["InputUsers"].items()) {
                user_access_hashes_[std::stoll(k)] = v.get<int64_t>();
            }
        }
        Logger::Info("TINAPROTO", "Loaded " + std::to_string(channel_access_hashes_.size()) + " channels and " + std::to_string(user_access_hashes_.size()) + " users from cache");
        return true;
    } catch (const std::exception& e) {
        Logger::Error("TINAPROTO", "Error parsing peer cache: " + std::string(e.what()));
        return false;
    }
}

bool MtprotoClient::ResolveChannel(int64_t chat_id, int64_t& out_channel_id, int64_t& out_access_hash) {
    int64_t channel_id = chat_id;
    if (channel_id < 0) {
        channel_id = -channel_id;
    }
    if (channel_id > 1000000000000LL) {
        channel_id -= 1000000000000LL;
    }
    out_channel_id = channel_id;

    auto it = channel_access_hashes_.find(channel_id);
    if (it != channel_access_hashes_.end()) {
        out_access_hash = it->second;
        return true;
    }
    return false;
}

std::optional<InputGroupCall> MtprotoClient::GetGroupCall(int64_t chat_id) {
    auto git = group_calls_.find(chat_id);
    if (git != group_calls_.end() && git->second.valid) {
        return git->second;
    }

    int64_t channel_id = 0;
    int64_t access_hash = 0;
    if (!ResolveChannel(chat_id, channel_id, access_hash)) {
        Logger::Error("TINAPROTO", "Could not resolve channel access hash for chat " + std::to_string(chat_id));
        return std::nullopt;
    }

    TlWriter w;
    w.PutUint32(0x08736a09);
    w.PutUint32(0xf35aec28);
    w.PutInt64(channel_id);
    w.PutInt64(access_hash);

    std::vector<uint8_t> resp = SendRpc(w.TakeData());
    if (resp.empty()) {
        Logger::Error("TINAPROTO", "channels.getFullChannel failed for chat " + std::to_string(chat_id));
        return std::nullopt;
    }

    for (size_t i = 0; i + 20 <= resp.size(); ++i) {
        uint32_t magic = *reinterpret_cast<const uint32_t*>(resp.data() + i);
        if (magic == 0xd8aa840f) {
            int64_t call_id = *reinterpret_cast<const int64_t*>(resp.data() + i + 4);
            int64_t call_hash = *reinterpret_cast<const int64_t*>(resp.data() + i + 12);
            InputGroupCall call{call_id, call_hash, true};
            group_calls_[chat_id] = call;
            Logger::Info("TINAPROTO", "Resolved InputGroupCall for chat " + std::to_string(chat_id) + " (id: " + std::to_string(call_id) + ")");
            return call;
        }
    }

    Logger::Warn("TINAPROTO", "No active group call found in chat full response for " + std::to_string(chat_id));
    return std::nullopt;
}

std::string MtprotoClient::JoinGroupCall(int64_t chat_id, const std::string& local_sdp_json) {
    auto call_opt = GetGroupCall(chat_id);
    if (!call_opt) {
        return "";
    }
    const auto& call = *call_opt;

    int64_t self_id = session_.user_id;
    int64_t self_access_hash = user_access_hashes_[self_id];

    TlWriter w;
    w.PutUint32(0x8fb53057);
    w.PutUint32(4);
    w.PutUint32(0xd8aa840f);
    w.PutInt64(call.id);
    w.PutInt64(call.access_hash);
    w.PutUint32(0xdde8a54c);
    w.PutInt64(self_id);
    w.PutInt64(self_access_hash);
    w.PutUint32(0x7d748d04);
    w.PutString(local_sdp_json);

    std::vector<uint8_t> resp = SendRpc(w.TakeData());
    if (resp.empty()) {
        Logger::Error("TINAPROTO", "phone.joinGroupCall RPC failed for chat " + std::to_string(chat_id));
        return "";
    }

    for (size_t i = 0; i + 8 <= resp.size(); ++i) {
        uint32_t magic = *reinterpret_cast<const uint32_t*>(resp.data() + i);
        if (magic == 0x0b783982) {
            TlReader r(resp.data() + i + 4, resp.size() - (i + 4));
            r.ReadUint32();
            uint32_t dj = r.ReadUint32();
            if (dj == 0x7d748d04) {
                std::string sdp = r.ReadString();
                if (!sdp.empty()) {
                    Logger::Info("TINAPROTO", "Received remote WebRTC transport SDP (" + std::to_string(sdp.size()) + " bytes)");
                    return sdp;
                }
            }
        }
    }

    for (size_t i = 0; i + 4 <= resp.size(); ++i) {
        uint32_t dj = *reinterpret_cast<const uint32_t*>(resp.data() + i);
        if (dj == 0x7d748d04) {
            TlReader r(resp.data() + i + 4, resp.size() - (i + 4));
            std::string candidate = r.ReadString();
            if (candidate.find("transport") != std::string::npos || candidate.find("fingerprint") != std::string::npos) {
                Logger::Info("TINAPROTO", "Extracted remote WebRTC transport SDP (" + std::to_string(candidate.size()) + " bytes)");
                return candidate;
            }
        }
    }

    return "{\"transport\": null}";
}

bool MtprotoClient::LeaveGroupCall(int64_t chat_id) {
    auto it = group_calls_.find(chat_id);
    if (it == group_calls_.end() || !it->second.valid) {
        return true;
    }
    const auto& call = it->second;
    group_calls_.erase(it);

    TlWriter w;
    w.PutUint32(0x500377f9);
    w.PutUint32(0xd8aa840f);
    w.PutInt64(call.id);
    w.PutInt64(call.access_hash);
    w.PutInt32(0);

    std::vector<uint8_t> resp = SendRpc(w.TakeData());
    return !resp.empty();
}

}
