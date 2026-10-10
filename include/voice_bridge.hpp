/*
 * ============================================================================
 * TinaMusic - Telegram's First Native C++20 Music Engine
 * Copyright (C) 2026 TheTeamVivek / TinaMusic Contributors
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
 * ============================================================================
 */

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
