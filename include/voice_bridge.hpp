#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

class VoiceBridge {
public:
    static VoiceBridge& Instance();

    bool Init();
    bool JoinVoiceChat(int64_t chat_id, const std::string& group_call_params);
    bool LeaveVoiceChat(int64_t chat_id);
    bool Pause(int64_t chat_id);
    bool Resume(int64_t chat_id);
    bool IsConnected(int64_t chat_id);

private:
    VoiceBridge();
    ~VoiceBridge();

    void* ntg_handle_{nullptr};
    std::unordered_map<int64_t, bool> active_calls_;
    std::mutex bridge_mutex_;
};
