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
#include <string>
#include <cstdint>
#include <optional>

namespace tinaproto {

struct InputGroupCall {
    int64_t id{0};
    int64_t access_hash{0};
    bool valid{false};
};

class TlWriter {
public:
    TlWriter() = default;

    void PutInt32(int32_t val);
    void PutUint32(uint32_t val);
    void PutInt64(int64_t val);
    void PutDouble(double val);
    void PutBool(bool val);
    void PutString(const std::string& str);
    void PutBytes(const std::vector<uint8_t>& bytes);
    void PutRawBytes(const uint8_t* data, size_t len);

    const std::vector<uint8_t>& GetData() const { return buf_; }
    std::vector<uint8_t> TakeData() { return std::move(buf_); }

private:
    std::vector<uint8_t> buf_;
};

class TlReader {
public:
    TlReader(const uint8_t* data, size_t size);
    explicit TlReader(const std::vector<uint8_t>& data);

    int32_t ReadInt32();
    uint32_t ReadUint32();
    int64_t ReadInt64();
    double ReadDouble();
    bool ReadBool();
    std::string ReadString();
    std::vector<uint8_t> ReadBytes();
    std::vector<uint8_t> ReadRawBytes(size_t len);

    size_t Remaining() const;
    size_t Offset() const { return offset_; }
    void Seek(size_t offset) { offset_ = offset; }
    bool HasError() const { return error_; }

private:
    const uint8_t* data_;
    size_t size_;
    size_t offset_{0};
    bool error_{false};
};

}
