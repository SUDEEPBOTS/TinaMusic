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
