/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/tl_codec.hpp"
#include <cstring>

namespace tinaproto {

void TlWriter::PutInt32(int32_t val) {
    buf_.push_back(static_cast<uint8_t>(val & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
}

void TlWriter::PutUint32(uint32_t val) {
    buf_.push_back(static_cast<uint8_t>(val & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    buf_.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
}

void TlWriter::PutInt64(int64_t val) {
    for (int i = 0; i < 8; ++i) {
        buf_.push_back(static_cast<uint8_t>((val >> (i * 8)) & 0xFF));
    }
}

void TlWriter::PutDouble(double val) {
    uint64_t raw = 0;
    std::memcpy(&raw, &val, sizeof(double));
    PutInt64(static_cast<int64_t>(raw));
}

void TlWriter::PutBool(bool val) {
    if (val) {
        PutUint32(0x997275b5);
    } else {
        PutUint32(0xbc799737);
    }
}

void TlWriter::PutString(const std::string& str) {
    size_t size = str.size();
    if (size == 0) {
        PutUint32(0);
        return;
    }
    int pad = 0;
    if (size < 254) {
        buf_.push_back(static_cast<uint8_t>(size));
        pad = static_cast<int>((4 - ((size + 1) % 4)) % 4);
    } else {
        buf_.push_back(0xFE);
        buf_.push_back(static_cast<uint8_t>(size & 0xFF));
        buf_.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((size >> 16) & 0xFF));
        pad = static_cast<int>((4 - (size % 4)) % 4);
    }
    buf_.insert(buf_.end(), str.begin(), str.end());
    buf_.insert(buf_.end(), pad, 0);
}

void TlWriter::PutBytes(const std::vector<uint8_t>& bytes) {
    size_t size = bytes.size();
    if (size == 0) {
        PutUint32(0);
        return;
    }
    int pad = 0;
    if (size < 254) {
        buf_.push_back(static_cast<uint8_t>(size));
        pad = static_cast<int>((4 - ((size + 1) % 4)) % 4);
    } else {
        buf_.push_back(0xFE);
        buf_.push_back(static_cast<uint8_t>(size & 0xFF));
        buf_.push_back(static_cast<uint8_t>((size >> 8) & 0xFF));
        buf_.push_back(static_cast<uint8_t>((size >> 16) & 0xFF));
        pad = static_cast<int>((4 - (size % 4)) % 4);
    }
    buf_.insert(buf_.end(), bytes.begin(), bytes.end());
    buf_.insert(buf_.end(), pad, 0);
}

void TlWriter::PutRawBytes(const uint8_t* data, size_t len) {
    buf_.insert(buf_.end(), data, data + len);
}

TlReader::TlReader(const uint8_t* data, size_t size)
    : data_(data), size_(size), offset_(0), error_(false) {}

TlReader::TlReader(const std::vector<uint8_t>& data)
    : data_(data.data()), size_(data.size()), offset_(0), error_(false) {}

size_t TlReader::Remaining() const {
    if (offset_ >= size_) return 0;
    return size_ - offset_;
}

int32_t TlReader::ReadInt32() {
    if (Remaining() < 4) {
        error_ = true;
        return 0;
    }
    int32_t val = static_cast<int32_t>(data_[offset_]) |
                  (static_cast<int32_t>(data_[offset_ + 1]) << 8) |
                  (static_cast<int32_t>(data_[offset_ + 2]) << 16) |
                  (static_cast<int32_t>(data_[offset_ + 3]) << 24);
    offset_ += 4;
    return val;
}

uint32_t TlReader::ReadUint32() {
    return static_cast<uint32_t>(ReadInt32());
}

int64_t TlReader::ReadInt64() {
    if (Remaining() < 8) {
        error_ = true;
        return 0;
    }
    uint64_t val = 0;
    for (int i = 0; i < 8; ++i) {
        val |= (static_cast<uint64_t>(data_[offset_ + i]) << (i * 8));
    }
    offset_ += 8;
    return static_cast<int64_t>(val);
}

double TlReader::ReadDouble() {
    int64_t raw = ReadInt64();
    double val = 0.0;
    std::memcpy(&val, &raw, sizeof(double));
    return val;
}

bool TlReader::ReadBool() {
    uint32_t crc = ReadUint32();
    return (crc == 0x997275b5);
}

std::string TlReader::ReadString() {
    if (Remaining() < 1) {
        error_ = true;
        return "";
    }
    uint8_t first = data_[offset_];
    size_t len = 0;
    int pad = 0;
    if (first < 254) {
        len = first;
        offset_ += 1;
        pad = static_cast<int>((4 - ((len + 1) % 4)) % 4);
    } else {
        if (Remaining() < 4) {
            error_ = true;
            return "";
        }
        len = static_cast<size_t>(data_[offset_ + 1]) |
              (static_cast<size_t>(data_[offset_ + 2]) << 8) |
              (static_cast<size_t>(data_[offset_ + 3]) << 16);
        offset_ += 4;
        pad = static_cast<int>((4 - (len % 4)) % 4);
    }
    if (Remaining() < len + pad) {
        error_ = true;
        return "";
    }
    std::string result(reinterpret_cast<const char*>(data_ + offset_), len);
    offset_ += len + pad;
    return result;
}

std::vector<uint8_t> TlReader::ReadBytes() {
    if (Remaining() < 1) {
        error_ = true;
        return {};
    }
    uint8_t first = data_[offset_];
    size_t len = 0;
    int pad = 0;
    if (first < 254) {
        len = first;
        offset_ += 1;
        pad = static_cast<int>((4 - ((len + 1) % 4)) % 4);
    } else {
        if (Remaining() < 4) {
            error_ = true;
            return {};
        }
        len = static_cast<size_t>(data_[offset_ + 1]) |
              (static_cast<size_t>(data_[offset_ + 2]) << 8) |
              (static_cast<size_t>(data_[offset_ + 3]) << 16);
        offset_ += 4;
        pad = static_cast<int>((4 - (len % 4)) % 4);
    }
    if (Remaining() < len + pad) {
        error_ = true;
        return {};
    }
    std::vector<uint8_t> result(data_ + offset_, data_ + offset_ + len);
    offset_ += len + pad;
    return result;
}

std::vector<uint8_t> TlReader::ReadRawBytes(size_t len) {
    if (Remaining() < len) {
        error_ = true;
        return {};
    }
    std::vector<uint8_t> result(data_ + offset_, data_ + offset_ + len);
    offset_ += len;
    return result;
}

}
