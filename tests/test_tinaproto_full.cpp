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
#include <iostream>
#include <cstdlib>

int main() {
    std::string session_str = "BQFwyZ4AtFCm6nW7btd1fogfVyowc02LIvJQ7KDP03r59AuzhhRwO1C5ULNGoH0F4Z05dCS6-stmK8P1Vf4CHkDjuTD9YdUZzWppwK6M4RZzonFfyqqp8PjPEjeXtBtZzBMirT4rFPdYF-HIzU8eLCq_DusODnvj-0gKho3h4BwU3lZawoNoEX-6rFGwmeQqzowj4vMkc163TKgaHBjDZDeuiG5HkMfFkDy6bBHgl36MAuAAROf19mY-kaOHQmcWQOXs4uoAvp91x33T_EE3wzE7zu1z7BkN0rc42DnEwuFKunkoj0SCIulhcBKk0JKtsC0jjnnPDD44KJWa5xYwRUjxF-tqsgAAAAH7TuXNAA";
    int32_t api_id = 38674666;
    std::string api_hash = "b4f0fbf8fb560c4bc9e7b9f3698e474c";

    std::cout << "[TEST] Starting TinaProtoClient E2E initialization test..." << std::endl;
    bool ok = tinaproto::TinaProtoClient::GetInstance().Initialize(session_str, api_id, api_hash);
    if (!ok) {
        std::cerr << "[TEST] FAILED: TinaProtoClient initialization failed!" << std::endl;
        return 1;
    }

    std::cout << "[TEST] SUCCESS: TinaProtoClient initialized with Telegram DC & NTgCalls!" << std::endl;

    std::cout << "[TEST] Testing GetGroupCall for -1002324064857..." << std::endl;
    auto call_opt = tinaproto::TinaProtoClient::GetInstance().GetGroupCall(-1002324064857);
    if (!call_opt) {
        std::cerr << "[TEST] FAILED: Could not resolve GroupCall for -1002324064857" << std::endl;
        return 1;
    }

    std::cout << "[TEST] SUCCESS: Found GroupCall! ID: " << call_opt->id << " Hash: " << call_opt->access_hash << std::endl;
    return 0;
}
