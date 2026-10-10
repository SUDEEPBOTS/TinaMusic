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
 * Copyright (C) 2026 TheTeamVivek / SUDEEPBOTS / TinaMusic Contributors
 * All Rights Reserved.
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

#include "voice_bridge.hpp"
#include "glibc_compatibility.h"
#include "logger.hpp"
#include <ntgcalls.h>

VoiceBridge& VoiceBridge::Instance() {
    static VoiceBridge instance;
    return instance;
}

VoiceBridge::VoiceBridge() {
    Init();
}

VoiceBridge::~VoiceBridge() {
    if (ntg_handle_) {
        ntg_destroy(reinterpret_cast<uintptr_t>(ntg_handle_));
        ntg_handle_ = nullptr;
    }
}

bool VoiceBridge::Init() {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    if (!ntg_handle_) {
        ntg_handle_ = reinterpret_cast<void*>(ntg_init());
        if (ntg_handle_) {
            Logger::Info("VOICE_BRIDGE", "WebRTC ntgcalls C++ engine initialized successfully");
            return true;
        } else {
            Logger::Error("VOICE_BRIDGE", "Failed to initialize ntgcalls WebRTC engine");
            return false;
        }
    }
    return true;
}

bool VoiceBridge::JoinVoiceChat(int64_t chat_id, const std::string& group_call_params) {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    if (!ntg_handle_) {
        if (!Init()) return false;
    }

    ntg_async_struct async_future = {nullptr, nullptr, nullptr, nullptr};
    std::string params = group_call_params;
    ntg_connect(reinterpret_cast<uintptr_t>(ntg_handle_), chat_id, const_cast<char*>(params.c_str()), false, async_future);

    active_calls_[chat_id] = true;
    Logger::Info("VOICE_BRIDGE", "Connecting WebRTC voice channel to chat " + std::to_string(chat_id) + " (Params size: " + std::to_string(group_call_params.size()) + ")");
    return true;
}

bool VoiceBridge::LeaveVoiceChat(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    if (!ntg_handle_) return false;

    auto it = active_calls_.find(chat_id);
    if (it != active_calls_.end()) {
        ntg_async_struct async_future = {nullptr, nullptr, nullptr, nullptr};
        ntg_stop(reinterpret_cast<uintptr_t>(ntg_handle_), chat_id, async_future);
        active_calls_.erase(it);
        Logger::Info("VOICE_BRIDGE", "Disconnected WebRTC voice call from chat " + std::to_string(chat_id));
        return true;
    }
    return false;
}

bool VoiceBridge::Pause(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    if (ntg_handle_ && active_calls_.count(chat_id)) {
        ntg_async_struct async_future = {nullptr, nullptr, nullptr, nullptr};
        ntg_pause(reinterpret_cast<uintptr_t>(ntg_handle_), chat_id, async_future);
        Logger::Info("VOICE_BRIDGE", "Paused WebRTC stream in chat " + std::to_string(chat_id));
        return true;
    }
    return false;
}

bool VoiceBridge::Resume(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    if (ntg_handle_ && active_calls_.count(chat_id)) {
        ntg_async_struct async_future = {nullptr, nullptr, nullptr, nullptr};
        ntg_resume(reinterpret_cast<uintptr_t>(ntg_handle_), chat_id, async_future);
        Logger::Info("VOICE_BRIDGE", "Resumed WebRTC stream in chat " + std::to_string(chat_id));
        return true;
    }
    return false;
}

bool VoiceBridge::IsConnected(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(bridge_mutex_);
    return active_calls_.find(chat_id) != active_calls_.end();
}
