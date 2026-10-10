/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/session.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <random>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

static int64_t GenerateMsgId() {
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    int64_t now_sec = now_ns / 1000000000LL;
    int64_t now_nano = (now_ns % 1000000000LL) & ~3LL;
    return (now_sec << 32) | now_nano;
}

static void PutInt32(std::vector<uint8_t>& buf, int32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

static void PutInt64(std::vector<uint8_t>& buf, int64_t val) {
    for (int i = 0; i < 8; ++i) {
        buf.push_back((val >> (i * 8)) & 0xFF);
    }
}

static void PutString(std::vector<uint8_t>& buf, const std::string& str) {
    size_t size = str.size();
    if (size == 0) {
        PutInt32(buf, 0);
        return;
    }
    int pad = 0;
    if (size < 254) {
        buf.push_back(static_cast<uint8_t>(size));
        pad = (4 - ((size + 1) % 4)) % 4;
    } else {
        buf.push_back(0xFE);
        buf.push_back(size & 0xFF);
        buf.push_back((size >> 8) & 0xFF);
        buf.push_back((size >> 16) & 0xFF);
        pad = (4 - (size % 4)) % 4;
    }
    buf.insert(buf.end(), str.begin(), str.end());
    buf.insert(buf.end(), pad, 0);
}

int main() {
    std::string session_str = "BQFwyZ4AtFCm6nW7btd1fogfVyowc02LIvJQ7KDP03r59AuzhhRwO1C5ULNGoH0F4Z05dCS6-stmK8P1Vf4CHkDjuTD9YdUZzWppwK6M4RZzonFfyqqp8PjPEjeXtBtZzBMirT4rFPdYF-HIzU8eLCq_DusODnvj-0gKho3h4BwU3lZawoNoEX-6rFGwmeQqzowj4vMkc163TKgaHBjDZDeuiG5HkMfFkDy6bBHgl36MAuAAROf19mY-kaOHQmcWQOXs4uoAvp91x33T_EE3wzE7zu1z7BkN0rc42DnEwuFKunkoj0SCIulhcBKk0JKtsC0jjnnPDD44KJWa5xYwRUjxF-tqsgAAAAH7TuXNAA";
    auto sess_opt = tinaproto::SessionDecoder::DecodePyrogram(session_str);
    if (!sess_opt) {
        std::cerr << "Failed to decode session" << std::endl;
        return 1;
    }
    const auto& session = *sess_opt;
    std::cout << "[TEST] Decoded DC: " << session.dc_ip << ":" << session.dc_port << " User: " << session.user_id << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(session.dc_port);
    inet_pton(AF_INET, session.dc_ip.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to DC" << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "[TEST] Connected to DC " << session.dc_ip << std::endl;

    uint8_t handshake[4] = {0xee, 0xee, 0xee, 0xee};
    if (send(sock, handshake, 4, 0) != 4) {
        std::cerr << "Failed to send handshake" << std::endl;
        close(sock);
        return 1;
    }

    uint8_t sha1_hash[SHA_DIGEST_LENGTH];
    SHA1(session.auth_key.data(), session.auth_key.size(), sha1_hash);
    std::vector<uint8_t> auth_key_id(sha1_hash + 12, sha1_hash + 20);

    uint64_t session_id = 0;
    RAND_bytes(reinterpret_cast<uint8_t*>(&session_id), 8);
    int64_t msg_id = GenerateMsgId();
    int32_t seq_no = 1;

    std::vector<uint8_t> rpc_query;
    PutInt32(rpc_query, static_cast<int32_t>(0xda9b0d0d));
    PutInt32(rpc_query, 173);
    PutInt32(rpc_query, static_cast<int32_t>(0xc1cd5ea9));
    PutInt32(rpc_query, 0);
    PutInt32(rpc_query, 38674666);
    PutString(rpc_query, "TinaMusic C++20");
    PutString(rpc_query, "Linux x86_64");
    PutString(rpc_query, "2.0.0");
    PutString(rpc_query, "en");
    PutString(rpc_query, "");
    PutString(rpc_query, "en");
    PutInt32(rpc_query, static_cast<int32_t>(0xc4f9186b));

    std::vector<uint8_t> plaintext;
    PutInt64(plaintext, 0);
    PutInt64(plaintext, session_id);
    PutInt64(plaintext, msg_id);
    PutInt32(plaintext, seq_no);
    PutInt32(plaintext, static_cast<int32_t>(rpc_query.size()));
    plaintext.insert(plaintext.end(), rpc_query.begin(), rpc_query.end());

    size_t total_len = plaintext.size();
    size_t pad_len = 16 - (total_len % 16);
    if (pad_len < 12) {
        pad_len += 16;
    }
    std::vector<uint8_t> pad_bytes(pad_len);
    RAND_bytes(pad_bytes.data(), pad_len);
    plaintext.insert(plaintext.end(), pad_bytes.begin(), pad_bytes.end());

    uint8_t msg_key_large[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_large;
    SHA256_Init(&sha256_large);
    SHA256_Update(&sha256_large, session.auth_key.data() + 88, 32);
    SHA256_Update(&sha256_large, plaintext.data(), plaintext.size());
    SHA256_Final(msg_key_large, &sha256_large);

    std::vector<uint8_t> msg_key(msg_key_large + 8, msg_key_large + 24);

    uint8_t sha256_a[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx_a;
    SHA256_Init(&ctx_a);
    SHA256_Update(&ctx_a, msg_key.data(), 16);
    SHA256_Update(&ctx_a, session.auth_key.data(), 36);
    SHA256_Final(sha256_a, &ctx_a);

    uint8_t sha256_b[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx_b;
    SHA256_Init(&ctx_b);
    SHA256_Update(&ctx_b, session.auth_key.data() + 40, 36);
    SHA256_Update(&ctx_b, msg_key.data(), 16);
    SHA256_Final(sha256_b, &ctx_b);

    uint8_t aes_key[32];
    std::memcpy(aes_key, sha256_a, 8);
    std::memcpy(aes_key + 8, sha256_b + 8, 16);
    std::memcpy(aes_key + 24, sha256_a + 24, 8);

    uint8_t aes_iv[32];
    std::memcpy(aes_iv, sha256_b, 8);
    std::memcpy(aes_iv + 8, sha256_a + 8, 16);
    std::memcpy(aes_iv + 24, sha256_b + 24, 8);

    AES_KEY enc_key;
    AES_set_encrypt_key(aes_key, 256, &enc_key);
    std::vector<uint8_t> encrypted(plaintext.size());
    AES_ige_encrypt(plaintext.data(), encrypted.data(), plaintext.size(), &enc_key, aes_iv, AES_ENCRYPT);

    std::vector<uint8_t> packet;
    packet.insert(packet.end(), auth_key_id.begin(), auth_key_id.end());
    packet.insert(packet.end(), msg_key.begin(), msg_key.end());
    packet.insert(packet.end(), encrypted.begin(), encrypted.end());

    std::vector<uint8_t> wire;
    PutInt32(wire, static_cast<int32_t>(packet.size()));
    wire.insert(wire.end(), packet.begin(), packet.end());

    std::cout << "[TEST] Sending wire_len=" << wire.size() << " prefix=[";
    for (size_t i = 0; i < std::min<size_t>(wire.size(), 32); ++i) {
        printf("%02x ", wire[i]);
    }
    std::cout << "]" << std::endl;

    if (send(sock, wire.data(), wire.size(), 0) != static_cast<ssize_t>(wire.size())) {
        std::cerr << "Failed to send packet" << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "[TEST] Sent encrypted initConnection packet (" << wire.size() << " bytes)" << std::endl;

    uint32_t resp_len = 0;
    if (recv(sock, &resp_len, 4, MSG_WAITALL) != 4) {
        std::cerr << "Failed to receive response length" << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "[TEST] Received response length: " << resp_len << " bytes" << std::endl;

    std::vector<uint8_t> resp_buf(resp_len);
    size_t received = 0;
    while (received < resp_len) {
        ssize_t n = recv(sock, resp_buf.data() + received, resp_len - received, 0);
        if (n <= 0) break;
        received += n;
    }
    if (received != resp_len) {
        std::cerr << "Failed to receive full response payload" << std::endl;
        close(sock);
        return 1;
    }

    if (resp_len < 24) {
        std::cerr << "Response too small" << std::endl;
        close(sock);
        return 1;
    }

    std::vector<uint8_t> resp_auth_key_id(resp_buf.begin(), resp_buf.begin() + 8);
    std::vector<uint8_t> resp_msg_key(resp_buf.begin() + 8, resp_buf.begin() + 24);
    std::vector<uint8_t> resp_encrypted(resp_buf.begin() + 24, resp_buf.end());

    uint8_t resp_sha256_a[SHA256_DIGEST_LENGTH];
    SHA256_CTX resp_ctx_a;
    SHA256_Init(&resp_ctx_a);
    SHA256_Update(&resp_ctx_a, resp_msg_key.data(), 16);
    SHA256_Update(&resp_ctx_a, session.auth_key.data() + 8, 36);
    SHA256_Final(resp_sha256_a, &resp_ctx_a);

    uint8_t resp_sha256_b[SHA256_DIGEST_LENGTH];
    SHA256_CTX resp_ctx_b;
    SHA256_Init(&resp_ctx_b);
    SHA256_Update(&resp_ctx_b, session.auth_key.data() + 48, 36);
    SHA256_Update(&resp_ctx_b, resp_msg_key.data(), 16);
    SHA256_Final(resp_sha256_b, &resp_ctx_b);

    uint8_t resp_aes_key[32];
    std::memcpy(resp_aes_key, resp_sha256_a, 8);
    std::memcpy(resp_aes_key + 8, resp_sha256_b + 8, 16);
    std::memcpy(resp_aes_key + 24, resp_sha256_a + 24, 8);

    uint8_t resp_aes_iv[32];
    std::memcpy(resp_aes_iv, resp_sha256_b, 8);
    std::memcpy(resp_aes_iv + 8, resp_sha256_a + 8, 16);
    std::memcpy(resp_aes_iv + 24, resp_sha256_b + 24, 8);

    AES_KEY dec_key;
    AES_set_decrypt_key(resp_aes_key, 256, &dec_key);
    std::vector<uint8_t> resp_plaintext(resp_encrypted.size());
    AES_ige_encrypt(resp_encrypted.data(), resp_plaintext.data(), resp_encrypted.size(), &dec_key, resp_aes_iv, AES_DECRYPT);

    uint8_t verify_msg_key_large[SHA256_DIGEST_LENGTH];
    SHA256_CTX verify_sha256;
    SHA256_Init(&verify_sha256);
    SHA256_Update(&verify_sha256, session.auth_key.data() + 96, 32);
    SHA256_Update(&verify_sha256, resp_plaintext.data(), resp_plaintext.size());
    SHA256_Final(verify_msg_key_large, &verify_sha256);

    if (std::memcmp(verify_msg_key_large + 8, resp_msg_key.data(), 16) != 0) {
        std::cerr << "Response MsgKey verification failed!" << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "[TEST] Response MsgKey verified successfully!" << std::endl;

    int64_t resp_salt = *reinterpret_cast<int64_t*>(resp_plaintext.data());
    int64_t resp_session_id = *reinterpret_cast<int64_t*>(resp_plaintext.data() + 8);
    int64_t resp_msg_id = *reinterpret_cast<int64_t*>(resp_plaintext.data() + 16);
    int32_t resp_seq_no = *reinterpret_cast<int32_t*>(resp_plaintext.data() + 24);
    int32_t resp_data_len = *reinterpret_cast<int32_t*>(resp_plaintext.data() + 28);
    uint32_t rpc_constructor = *reinterpret_cast<uint32_t*>(resp_plaintext.data() + 32);

    std::cout << "[TEST] Server Salt: " << resp_salt << std::endl;
    std::cout << "[TEST] Response Session ID: " << resp_session_id << std::endl;
    std::cout << "[TEST] Response Msg ID: " << resp_msg_id << " (seq_no: " << resp_seq_no << ")" << std::endl;
    std::cout << "[TEST] Response Data Length: " << resp_data_len << std::endl;
    std::cout << "[TEST] Root TL Constructor: 0x" << std::hex << rpc_constructor << std::dec << std::endl;

    std::cout << "[TEST] Resending with server salt: " << resp_salt << std::endl;
    int64_t msg_id2 = GenerateMsgId();
    int32_t seq_no2 = 3;

    std::vector<uint8_t> plaintext2;
    PutInt64(plaintext2, resp_salt);
    PutInt64(plaintext2, session_id);
    PutInt64(plaintext2, msg_id2);
    PutInt32(plaintext2, seq_no2);
    PutInt32(plaintext2, static_cast<int32_t>(rpc_query.size()));
    plaintext2.insert(plaintext2.end(), rpc_query.begin(), rpc_query.end());

    size_t total_len2 = plaintext2.size();
    size_t pad_len2 = 16 - (total_len2 % 16);
    if (pad_len2 < 12) pad_len2 += 16;
    std::vector<uint8_t> pad_bytes2(pad_len2);
    RAND_bytes(pad_bytes2.data(), pad_len2);
    plaintext2.insert(plaintext2.end(), pad_bytes2.begin(), pad_bytes2.end());

    uint8_t msg_key_large2[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256_large2;
    SHA256_Init(&sha256_large2);
    SHA256_Update(&sha256_large2, session.auth_key.data() + 88, 32);
    SHA256_Update(&sha256_large2, plaintext2.data(), plaintext2.size());
    SHA256_Final(msg_key_large2, &sha256_large2);
    std::vector<uint8_t> msg_key2(msg_key_large2 + 8, msg_key_large2 + 24);

    uint8_t sha256_a2[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx_a2;
    SHA256_Init(&ctx_a2);
    SHA256_Update(&ctx_a2, msg_key2.data(), 16);
    SHA256_Update(&ctx_a2, session.auth_key.data(), 36);
    SHA256_Final(sha256_a2, &ctx_a2);

    uint8_t sha256_b2[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx_b2;
    SHA256_Init(&ctx_b2);
    SHA256_Update(&ctx_b2, session.auth_key.data() + 40, 36);
    SHA256_Update(&ctx_b2, msg_key2.data(), 16);
    SHA256_Final(sha256_b2, &ctx_b2);

    uint8_t aes_key2[32];
    std::memcpy(aes_key2, sha256_a2, 8);
    std::memcpy(aes_key2 + 8, sha256_b2 + 8, 16);
    std::memcpy(aes_key2 + 24, sha256_a2 + 24, 8);

    uint8_t aes_iv2[32];
    std::memcpy(aes_iv2, sha256_b2, 8);
    std::memcpy(aes_iv2 + 8, sha256_a2 + 8, 16);
    std::memcpy(aes_iv2 + 24, sha256_b2 + 24, 8);

    AES_KEY enc_key2;
    AES_set_encrypt_key(aes_key2, 256, &enc_key2);
    std::vector<uint8_t> encrypted2(plaintext2.size());
    AES_ige_encrypt(plaintext2.data(), encrypted2.data(), plaintext2.size(), &enc_key2, aes_iv2, AES_ENCRYPT);

    std::vector<uint8_t> packet2;
    packet2.insert(packet2.end(), auth_key_id.begin(), auth_key_id.end());
    packet2.insert(packet2.end(), msg_key2.begin(), msg_key2.end());
    packet2.insert(packet2.end(), encrypted2.begin(), encrypted2.end());

    std::vector<uint8_t> wire2;
    PutInt32(wire2, static_cast<int32_t>(packet2.size()));
    wire2.insert(wire2.end(), packet2.begin(), packet2.end());

    send(sock, wire2.data(), wire2.size(), 0);

    uint32_t resp_len2 = 0;
    recv(sock, &resp_len2, 4, MSG_WAITALL);
    std::cout << "[TEST] Received second response length: " << resp_len2 << " bytes" << std::endl;

    std::vector<uint8_t> resp_buf2(resp_len2);
    size_t received2 = 0;
    while (received2 < resp_len2) {
        ssize_t n = recv(sock, resp_buf2.data() + received2, resp_len2 - received2, 0);
        if (n <= 0) break;
        received2 += n;
    }

    std::vector<uint8_t> resp_msg_key2(resp_buf2.begin() + 8, resp_buf2.begin() + 24);
    std::vector<uint8_t> resp_encrypted2(resp_buf2.begin() + 24, resp_buf2.end());

    uint8_t r_sha_a2[SHA256_DIGEST_LENGTH];
    SHA256_CTX r_ctx_a2;
    SHA256_Init(&r_ctx_a2);
    SHA256_Update(&r_ctx_a2, resp_msg_key2.data(), 16);
    SHA256_Update(&r_ctx_a2, session.auth_key.data() + 8, 36);
    SHA256_Final(r_sha_a2, &r_ctx_a2);

    uint8_t r_sha_b2[SHA256_DIGEST_LENGTH];
    SHA256_CTX r_ctx_b2;
    SHA256_Init(&r_ctx_b2);
    SHA256_Update(&r_ctx_b2, session.auth_key.data() + 48, 36);
    SHA256_Update(&r_ctx_b2, resp_msg_key2.data(), 16);
    SHA256_Final(r_sha_b2, &r_ctx_b2);

    uint8_t r_aes_k2[32];
    std::memcpy(r_aes_k2, r_sha_a2, 8);
    std::memcpy(r_aes_k2 + 8, r_sha_b2 + 8, 16);
    std::memcpy(r_aes_k2 + 24, r_sha_a2 + 24, 8);

    uint8_t r_aes_iv2[32];
    std::memcpy(r_aes_iv2, r_sha_b2, 8);
    std::memcpy(r_aes_iv2 + 8, r_sha_a2 + 8, 16);
    std::memcpy(r_aes_iv2 + 24, r_sha_b2 + 24, 8);

    AES_KEY dec_key2;
    AES_set_decrypt_key(r_aes_k2, 256, &dec_key2);
    std::vector<uint8_t> resp_plaintext2(resp_encrypted2.size());
    AES_ige_encrypt(resp_encrypted2.data(), resp_plaintext2.data(), resp_encrypted2.size(), &dec_key2, r_aes_iv2, AES_DECRYPT);

    int32_t resp_data_len2 = *reinterpret_cast<int32_t*>(resp_plaintext2.data() + 28);
    uint32_t rpc_constructor2 = *reinterpret_cast<uint32_t*>(resp_plaintext2.data() + 32);

    std::cout << "[TEST] Second Response Data Length: " << resp_data_len2 << std::endl;
    std::cout << "[TEST] Second TL Constructor: 0x" << std::hex << rpc_constructor2 << std::dec << std::endl;

    std::cout << "[TEST] Payload bytes (hex): ";
    for (int i = 0; i < std::min<int>(resp_data_len2, 88); ++i) {
        printf("%02x ", resp_plaintext2[32 + i]);
    }
    printf("\n");

    close(sock);
    std::cout << "[TEST] SUCCESS: Native C++20 MTProto 2.0 handshake verified with Telegram DC!" << std::endl;
    return 0;
}
