/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/crypto.hpp"
#include <cstring>
#include <chrono>
#include <mutex>
#include <sys/random.h>

namespace tinaproto {
namespace internal {

static inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

static inline uint32_t rotl(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

static void LocalSha1(const uint8_t* data, size_t len, uint8_t out[20]) {
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    std::vector<uint8_t> msg(data, data + len);
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    uint64_t bit_len = static_cast<uint64_t>(len) * 8;
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(msg[chunk + i * 4]) << 24) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(msg[chunk + i * 4 + 3]));
        }
        for (int i = 16; i < 80; ++i) {
            w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f = 0, k = 0;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            uint32_t temp = rotl(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rotl(b, 30);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    uint32_t h[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; ++i) {
        out[i * 4] = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
        out[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
        out[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
        out[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xFF);
    }
}

static const uint32_t K256[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

struct LocalSha256 {
    uint32_t state[8]{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint64_t count{0};
    uint8_t buffer[64]{0};

    static void Transform(uint32_t s[8], const uint8_t chunk[64]) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(chunk[i * 4]) << 24) |
                   (static_cast<uint32_t>(chunk[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(chunk[i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(chunk[i * 4 + 3]));
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + K256[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        s[0] += a;
        s[1] += b;
        s[2] += c;
        s[3] += d;
        s[4] += e;
        s[5] += f;
        s[6] += g;
        s[7] += h;
    }

    void Update(const uint8_t* data, size_t len) {
        size_t buf_index = static_cast<size_t>((count >> 3) & 63);
        count += static_cast<uint64_t>(len) << 3;

        size_t part_len = 64 - buf_index;
        size_t i = 0;
        if (len >= part_len) {
            std::memcpy(buffer + buf_index, data, part_len);
            Transform(state, buffer);
            for (i = part_len; i + 63 < len; i += 64) {
                Transform(state, data + i);
            }
            buf_index = 0;
        }
        if (i < len) {
            std::memcpy(buffer + buf_index, data + i, len - i);
        }
    }

    void Final(uint8_t out[32]) {
        uint8_t final_count[8];
        for (int i = 0; i < 8; ++i) {
            final_count[i] = static_cast<uint8_t>((count >> ((7 - i) * 8)) & 0xFF);
        }

        size_t buf_index = static_cast<size_t>((count >> 3) & 63);
        size_t pad_len = (buf_index < 56) ? (56 - buf_index) : (120 - buf_index);
        static const uint8_t PADDING[64] = {0x80};
        Update(PADDING, pad_len);
        Update(final_count, 8);

        for (int i = 0; i < 8; ++i) {
            out[i * 4] = static_cast<uint8_t>((state[i] >> 24) & 0xFF);
            out[i * 4 + 1] = static_cast<uint8_t>((state[i] >> 16) & 0xFF);
            out[i * 4 + 2] = static_cast<uint8_t>((state[i] >> 8) & 0xFF);
            out[i * 4 + 3] = static_cast<uint8_t>(state[i] & 0xFF);
        }
    }
};

static const uint8_t AES_SBOX[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t AES_RSBOX[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const uint8_t AES_RCON[15] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D
};

struct LocalAesKey {
    uint8_t round_keys[240];

    void Expand(const uint8_t key[32]) {
        std::memcpy(round_keys, key, 32);
        size_t bytes_generated = 32;
        size_t rcon_iteration = 1;
        uint8_t temp[4];

        while (bytes_generated < 240) {
            for (int i = 0; i < 4; ++i) temp[i] = round_keys[bytes_generated - 4 + i];

            if (bytes_generated % 32 == 0) {
                uint8_t k = temp[0];
                temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = k;
                for (int i = 0; i < 4; ++i) temp[i] = AES_SBOX[temp[i]];
                temp[0] ^= AES_RCON[rcon_iteration++];
            } else if (bytes_generated % 32 == 16) {
                for (int i = 0; i < 4; ++i) temp[i] = AES_SBOX[temp[i]];
            }

            for (int i = 0; i < 4; ++i) {
                round_keys[bytes_generated] = round_keys[bytes_generated - 32] ^ temp[i];
                bytes_generated++;
            }
        }
    }

    static inline uint8_t xtime(uint8_t x) {
        return (x << 1) ^ (((x >> 7) & 1) * 0x1b);
    }

    static inline uint8_t multiply(uint8_t x, uint8_t y) {
        uint8_t r = 0;
        for (int i = 0; i < 8; ++i) {
            if ((y >> i) & 1) r ^= x;
            x = xtime(x);
        }
        return r;
    }

    void EncryptBlock(const uint8_t in[16], uint8_t out[16]) const {
        uint8_t state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i] ^ round_keys[i];

        for (int round = 1; round <= 13; ++round) {
            for (int i = 0; i < 16; ++i) state[i] = AES_SBOX[state[i]];

            uint8_t temp[16];
            temp[0] = state[0]; temp[4] = state[4]; temp[8] = state[8]; temp[12] = state[12];
            temp[1] = state[5]; temp[5] = state[9]; temp[9] = state[13]; temp[13] = state[1];
            temp[2] = state[10]; temp[6] = state[14]; temp[10] = state[2]; temp[14] = state[6];
            temp[3] = state[15]; temp[7] = state[3]; temp[11] = state[7]; temp[15] = state[11];

            for (int i = 0; i < 4; ++i) {
                int c = i * 4;
                uint8_t a0 = temp[c], a1 = temp[c + 1], a2 = temp[c + 2], a3 = temp[c + 3];
                state[c]     = xtime(a0 ^ a1) ^ a1 ^ a2 ^ a3;
                state[c + 1] = xtime(a1 ^ a2) ^ a2 ^ a3 ^ a0;
                state[c + 2] = xtime(a2 ^ a3) ^ a3 ^ a0 ^ a1;
                state[c + 3] = xtime(a3 ^ a0) ^ a0 ^ a1 ^ a2;
            }

            const uint8_t* rk = round_keys + round * 16;
            for (int i = 0; i < 16; ++i) state[i] ^= rk[i];
        }

        for (int i = 0; i < 16; ++i) state[i] = AES_SBOX[state[i]];

        uint8_t temp[16];
        temp[0] = state[0]; temp[4] = state[4]; temp[8] = state[8]; temp[12] = state[12];
        temp[1] = state[5]; temp[5] = state[9]; temp[9] = state[13]; temp[13] = state[1];
        temp[2] = state[10]; temp[6] = state[14]; temp[10] = state[2]; temp[14] = state[6];
        temp[3] = state[15]; temp[7] = state[3]; temp[11] = state[7]; temp[15] = state[11];

        const uint8_t* rk = round_keys + 14 * 16;
        for (int i = 0; i < 16; ++i) out[i] = temp[i] ^ rk[i];
    }

    void DecryptBlock(const uint8_t in[16], uint8_t out[16]) const {
        uint8_t state[16];
        const uint8_t* rk = round_keys + 14 * 16;
        for (int i = 0; i < 16; ++i) state[i] = in[i] ^ rk[i];

        for (int round = 13; round >= 1; --round) {
            uint8_t temp[16];
            temp[0] = state[0]; temp[4] = state[4]; temp[8] = state[8]; temp[12] = state[12];
            temp[1] = state[13]; temp[5] = state[1]; temp[9] = state[5]; temp[13] = state[9];
            temp[2] = state[10]; temp[6] = state[14]; temp[10] = state[2]; temp[14] = state[6];
            temp[3] = state[7]; temp[7] = state[11]; temp[11] = state[15]; temp[15] = state[3];

            for (int i = 0; i < 16; ++i) temp[i] = AES_RSBOX[temp[i]];

            rk = round_keys + round * 16;
            for (int i = 0; i < 16; ++i) temp[i] ^= rk[i];

            for (int i = 0; i < 4; ++i) {
                int c = i * 4;
                uint8_t a0 = temp[c], a1 = temp[c + 1], a2 = temp[c + 2], a3 = temp[c + 3];
                state[c]     = multiply(a0, 0x0e) ^ multiply(a1, 0x0b) ^ multiply(a2, 0x0d) ^ multiply(a3, 0x09);
                state[c + 1] = multiply(a0, 0x09) ^ multiply(a1, 0x0e) ^ multiply(a2, 0x0b) ^ multiply(a3, 0x0d);
                state[c + 2] = multiply(a0, 0x0d) ^ multiply(a1, 0x09) ^ multiply(a2, 0x0e) ^ multiply(a3, 0x0b);
                state[c + 3] = multiply(a0, 0x0b) ^ multiply(a1, 0x0d) ^ multiply(a2, 0x09) ^ multiply(a3, 0x0e);
            }
        }

        uint8_t temp[16];
        temp[0] = state[0]; temp[4] = state[4]; temp[8] = state[8]; temp[12] = state[12];
        temp[1] = state[13]; temp[5] = state[1]; temp[9] = state[5]; temp[13] = state[9];
        temp[2] = state[10]; temp[6] = state[14]; temp[10] = state[2]; temp[14] = state[6];
        temp[3] = state[7]; temp[7] = state[11]; temp[11] = state[15]; temp[15] = state[3];

        for (int i = 0; i < 16; ++i) temp[i] = AES_RSBOX[temp[i]];

        for (int i = 0; i < 16; ++i) out[i] = temp[i] ^ round_keys[i];
    }
};

static void LocalIgeEncrypt(const uint8_t* in, uint8_t* out, size_t length, const uint8_t key[32], const uint8_t iv[32]) {
    LocalAesKey aes;
    aes.Expand(key);

    uint8_t iv1[16], iv2[16];
    std::memcpy(iv1, iv, 16);
    std::memcpy(iv2, iv + 16, 16);

    for (size_t offset = 0; offset < length; offset += 16) {
        uint8_t block[16];
        for (int i = 0; i < 16; ++i) block[i] = in[offset + i] ^ iv1[i];
        uint8_t enc[16];
        aes.EncryptBlock(block, enc);
        for (int i = 0; i < 16; ++i) {
            out[offset + i] = enc[i] ^ iv2[i];
            iv1[i] = out[offset + i];
            iv2[i] = in[offset + i];
        }
    }
}

static void LocalIgeDecrypt(const uint8_t* in, uint8_t* out, size_t length, const uint8_t key[32], const uint8_t iv[32]) {
    LocalAesKey aes;
    aes.Expand(key);

    uint8_t iv1[16], iv2[16];
    std::memcpy(iv1, iv, 16);
    std::memcpy(iv2, iv + 16, 16);

    for (size_t offset = 0; offset < length; offset += 16) {
        uint8_t block[16];
        for (int i = 0; i < 16; ++i) block[i] = in[offset + i] ^ iv2[i];
        uint8_t dec[16];
        aes.DecryptBlock(block, dec);
        for (int i = 0; i < 16; ++i) {
            out[offset + i] = dec[i] ^ iv1[i];
            iv1[i] = in[offset + i];
            iv2[i] = out[offset + i];
        }
    }
}

static void LocalRandomBytes(uint8_t* out, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = getrandom(out + total, len - total, 0);
        if (n > 0) {
            total += static_cast<size_t>(n);
        }
    }
}

}

std::vector<uint8_t> Crypto::ComputeAuthKeyId(const std::vector<uint8_t>& auth_key) {
    uint8_t digest[20];
    internal::LocalSha1(auth_key.data(), auth_key.size(), digest);
    return std::vector<uint8_t>(digest + 12, digest + 20);
}

std::vector<uint8_t> Crypto::ComputeMsgKey(const uint8_t* auth_key_part, const uint8_t* plaintext, size_t plaintext_len) {
    internal::LocalSha256 ctx;
    ctx.Update(auth_key_part, 32);
    ctx.Update(plaintext, plaintext_len);
    uint8_t digest[32];
    ctx.Final(digest);
    return std::vector<uint8_t>(digest + 8, digest + 24);
}

void Crypto::DeriveAesKeyIv(const uint8_t* auth_key, const uint8_t* msg_key, bool decode, uint8_t* aes_key_out, uint8_t* aes_iv_out) {
    size_t x = decode ? 8 : 0;

    internal::LocalSha256 ctx_a;
    ctx_a.Update(msg_key, 16);
    ctx_a.Update(auth_key + x, 36);
    uint8_t sha256_a[32];
    ctx_a.Final(sha256_a);

    internal::LocalSha256 ctx_b;
    ctx_b.Update(auth_key + 40 + x, 36);
    ctx_b.Update(msg_key, 16);
    uint8_t sha256_b[32];
    ctx_b.Final(sha256_b);

    std::memcpy(aes_key_out, sha256_a, 8);
    std::memcpy(aes_key_out + 8, sha256_b + 8, 16);
    std::memcpy(aes_key_out + 24, sha256_a + 24, 8);

    std::memcpy(aes_iv_out, sha256_b, 8);
    std::memcpy(aes_iv_out + 8, sha256_a + 8, 16);
    std::memcpy(aes_iv_out + 24, sha256_b + 24, 8);
}

void Crypto::AesIgeEncrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv) {
    internal::LocalIgeEncrypt(in, out, len, key, iv);
}

void Crypto::AesIgeDecrypt(const uint8_t* in, uint8_t* out, size_t len, const uint8_t* key, const uint8_t* iv) {
    internal::LocalIgeDecrypt(in, out, len, key, iv);
}

int64_t Crypto::GenerateMsgId() {
    static std::mutex mu;
    static int64_t last_msg_id = 0;
    std::lock_guard<std::mutex> lock(mu);

    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    int64_t now_sec = now_ns / 1000000000LL;
    int64_t now_nano = (now_ns % 1000000000LL) & ~3LL;

    int64_t msg_id = (now_sec << 32) | now_nano;
    if (msg_id <= last_msg_id) {
        msg_id = last_msg_id + 4;
    }
    last_msg_id = msg_id;
    return msg_id;
}

int64_t Crypto::GenerateSessionId() {
    int64_t session_id = 0;
    internal::LocalRandomBytes(reinterpret_cast<uint8_t*>(&session_id), sizeof(int64_t));
    return session_id;
}

}
