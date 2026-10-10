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
 * Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 * Support: https://t.me/Z0iiw
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

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <nlohmann/json.hpp>

struct ChatConfig {
    int volume{100};
    std::string play_mode{"Everyone"};
    bool clean_mode{true};
    std::string language{"en"};
    bool autoplay{false};
    bool show_thumbnails{true};
    bool command_delete{false};
    std::string admin_mode{"adminauth"};
    int clean_mode_duration{15};
    int64_t channel_play_id{0};
    std::string rtmp_url{""};
    std::string rtmp_key{""};
    std::unordered_set<int64_t> auth_users;
};

class Database {
public:
    static Database& Instance();

    void Init(const std::string& db_path = "data/bot_database.json");

    ChatConfig GetChatConfig(int64_t chat_id);
    void SetChatConfig(int64_t chat_id, const ChatConfig& cfg);

    bool IsAutoplayEnabled(int64_t chat_id);
    void SetAutoplay(int64_t chat_id, bool enabled);

    bool IsThumbnailsEnabled(int64_t chat_id);
    void SetThumbnails(int64_t chat_id, bool enabled);

    bool IsMaintenanceEnabled();
    std::string GetMaintenanceReason();
    void SetMaintenance(bool enabled, const std::string& reason);

    bool IsLoggerEnabled();
    void SetLoggerEnabled(bool enabled);

    bool IsAutoLeaveEnabled();
    void SetAutoLeaveEnabled(bool enabled);

    void AddSudo(int64_t user_id);
    void RemoveSudo(int64_t user_id);
    bool IsSudo(int64_t user_id, int64_t owner_id);
    std::unordered_set<int64_t> GetSudoUsers();

    void Blacklist(int64_t id, const std::string& reason);
    void Unblacklist(int64_t id);
    bool IsBlacklisted(int64_t id);

    void IncrementPlays();
    void RegisterChat(int64_t chat_id);
    int64_t GetTotalPlays();
    size_t GetTotalChats();
    std::unordered_set<int64_t> GetServedChats();

private:
    Database() = default;
    ~Database() = default;

    void SaveToDisk();
    void LoadFromDisk();

    std::string db_file_path_{"data/bot_database.json"};
    std::unordered_map<int64_t, ChatConfig> chat_configs_;
    std::unordered_set<int64_t> sudo_users_;
    std::unordered_map<int64_t, std::string> blacklisted_;
    std::unordered_set<int64_t> served_chats_;
    int64_t total_plays_{0};
    bool maintenance_mode_{false};
    std::string maintenance_reason_{""};
    bool logger_enabled_{false};
    bool autoleave_enabled_{false};
    std::mutex db_mutex_;
};
