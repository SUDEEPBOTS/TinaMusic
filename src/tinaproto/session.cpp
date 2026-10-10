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

#include "tinaproto/session.hpp"
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <arpa/inet.h>

namespace tinaproto {

std::vector<uint8_t> SessionDecoder::Base64UrlDecode(std::string input) {
    std::replace(input.begin(), input.end(), '-', '+');
    std::replace(input.begin(), input.end(), '_', '/');
    while (input.length() % 4 != 0) {
        input.push_back('=');
    }

    static const std::string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> out;
    std::vector<int> t(256, -1);
    for (size_t i = 0; i < b64.size(); ++i) t[static_cast<uint8_t>(b64[i])] = i;

    int val = 0, valb = -8;
    for (uint8_t c : input) {
        if (c == '=') break;
        if (t[c] == -1) continue;
        val = (val << 6) + t[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string SessionDecoder::GetDcIp(uint8_t dc_id, bool test_mode) {
    if (test_mode) {
        static const std::unordered_map<uint8_t, std::string> kTestDcs = {
            {1, "149.154.175.10"},
            {2, "149.154.167.40"},
            {3, "149.154.175.117"}
        };
        auto it = kTestDcs.find(dc_id);
        return (it != kTestDcs.end()) ? it->second : "149.154.167.40";
    }

    static const std::unordered_map<uint8_t, std::string> kProdDcs = {
        {1, "149.154.175.58"},
        {2, "149.154.167.50"},
        {3, "149.154.175.100"},
        {4, "149.154.167.91"},
        {5, "91.108.56.151"}
    };
    auto it = kProdDcs.find(dc_id);
    return (it != kProdDcs.end()) ? it->second : "149.154.167.50";
}

std::optional<SessionData> SessionDecoder::DecodePyrogram(const std::string& session_str) {
    if (session_str.empty()) return std::nullopt;

    std::vector<uint8_t> data = Base64UrlDecode(session_str);
    if (data.size() != 271 && data.size() != 267) {
        return std::nullopt;
    }

    SessionData s;
    s.dc_id = data[0];
    s.api_id = (static_cast<uint32_t>(data[1]) << 24) |
               (static_cast<uint32_t>(data[2]) << 16) |
               (static_cast<uint32_t>(data[3]) << 8)  |
               static_cast<uint32_t>(data[4]);
    s.test_mode = (data[5] != 0);

    s.auth_key.assign(data.begin() + 6, data.begin() + 6 + 256);

    if (data.size() == 271) {
        uint64_t uid = 0;
        for (int i = 0; i < 8; ++i) {
            uid = (uid << 8) | static_cast<uint64_t>(data[262 + i]);
        }
        s.user_id = static_cast<int64_t>(uid);
        s.is_bot = (data[270] != 0);
    } else {
        uint32_t uid = 0;
        for (int i = 0; i < 4; ++i) {
            uid = (uid << 8) | static_cast<uint32_t>(data[262 + i]);
        }
        s.user_id = static_cast<int64_t>(uid);
        s.is_bot = (data[266] != 0);
    }

    s.dc_ip = GetDcIp(s.dc_id, s.test_mode);
    s.dc_port = 443;
    return s;
}

std::optional<SessionData> SessionDecoder::DecodeTelethon(const std::string& session_str) {
    if (session_str.size() < 2) return std::nullopt;
    std::string payload = session_str.substr(1);
    std::vector<uint8_t> data = Base64UrlDecode(payload);

    size_t ip_len = (data.size() == 352) ? 16 : 4;
    size_t expected_len = 1 + ip_len + 2 + 256;
    if (data.size() != expected_len) {
        return std::nullopt;
    }

    SessionData s;
    s.dc_id = data[0];

    char ip_str[INET6_ADDRSTRLEN] = {0};
    if (ip_len == 4) {
        inet_ntop(AF_INET, data.data() + 1, ip_str, INET_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, data.data() + 1, ip_str, INET6_ADDRSTRLEN);
    }
    s.dc_ip = ip_str;

    size_t port_offset = 1 + ip_len;
    s.dc_port = (static_cast<uint16_t>(data[port_offset]) << 8) | static_cast<uint16_t>(data[port_offset + 1]);

    size_t key_offset = port_offset + 2;
    s.auth_key.assign(data.begin() + key_offset, data.begin() + key_offset + 256);

    return s;
}

}
