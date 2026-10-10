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

#include <vector>
#include <cstdint>
#include <cstddef>

namespace tinaproto {

class Crypto {
public:
    static std::vector<uint8_t> ComputeAuthKeyId(const std::vector<uint8_t>& auth_key);

    static std::vector<uint8_t> ComputeMsgKey(const uint8_t* auth_key_part, const uint8_t* plaintext, size_t plaintext_len);

    static void DeriveAesKeyIv(const uint8_t* auth_key, const uint8_t* msg_key, bool decode, uint8_t* aes_key_out, uint8_t* aes_iv_out);

    static void AesIgeEncrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv);

    static void AesIgeDecrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv);

    static int64_t GenerateMsgId();

    static int64_t GenerateSessionId();
};

}
