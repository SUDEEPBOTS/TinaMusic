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

#include "database.hpp"
#include "logger.hpp"
#include <fstream>
#include <sys/stat.h>

Database& Database::Instance() {
    static Database instance;
    return instance;
}

void Database::Init(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    db_file_path_ = db_path;
    mkdir("data", 0755);
    LoadFromDisk();
    Logger::Info("DATABASE", "Database engine initialized. Loaded " + std::to_string(served_chats_.size()) + " active chats.");
}

void Database::SaveToDisk() {
    nlohmann::json root;
    root["total_plays"] = total_plays_;
    root["sudo_users"] = sudo_users_;
    root["blacklisted"] = blacklisted_;
    root["maintenance_mode"] = maintenance_mode_;
    root["maintenance_reason"] = maintenance_reason_;
    root["logger_enabled"] = logger_enabled_;
    root["autoleave_enabled"] = autoleave_enabled_;

    nlohmann::json chatsJson = nlohmann::json::object();
    for (const auto& [cid, cfg] : chat_configs_) {
        nlohmann::json c;
        c["volume"] = cfg.volume;
        c["play_mode"] = cfg.play_mode;
        c["clean_mode"] = cfg.clean_mode;
        c["language"] = cfg.language;
        c["autoplay"] = cfg.autoplay;
        c["show_thumbnails"] = cfg.show_thumbnails;
        c["command_delete"] = cfg.command_delete;
        c["admin_mode"] = cfg.admin_mode;
        c["clean_mode_duration"] = cfg.clean_mode_duration;
        c["channel_play_id"] = cfg.channel_play_id;
        c["rtmp_url"] = cfg.rtmp_url;
        c["rtmp_key"] = cfg.rtmp_key;
        c["auth_users"] = cfg.auth_users;
        chatsJson[std::to_string(cid)] = c;
    }
    root["chat_configs"] = chatsJson;
    root["served_chats"] = served_chats_;

    std::string tmpPath = db_file_path_ + ".tmp";
    std::ofstream ofs(tmpPath);
    if (ofs.is_open()) {
        ofs << root.dump(2);
        ofs.close();
        rename(tmpPath.c_str(), db_file_path_.c_str());
    }
}

void Database::LoadFromDisk() {
    std::ifstream ifs(db_file_path_);
    if (!ifs.is_open()) return;

    try {
        nlohmann::json root = nlohmann::json::parse(ifs);
        total_plays_ = root.value("total_plays", 0LL);
        maintenance_mode_ = root.value("maintenance_mode", false);
        maintenance_reason_ = root.value("maintenance_reason", "");
        logger_enabled_ = root.value("logger_enabled", false);
        autoleave_enabled_ = root.value("autoleave_enabled", false);

        if (root.contains("sudo_users")) {
            sudo_users_ = root["sudo_users"].get<std::unordered_set<int64_t>>();
        }
        if (root.contains("blacklisted")) {
            blacklisted_ = root["blacklisted"].get<std::unordered_map<int64_t, std::string>>();
        }
        if (root.contains("served_chats")) {
            served_chats_ = root["served_chats"].get<std::unordered_set<int64_t>>();
        }
        if (root.contains("chat_configs")) {
            for (auto& [key, val] : root["chat_configs"].items()) {
                int64_t cid = std::stoll(key);
                ChatConfig cfg;
                cfg.volume = val.value("volume", 100);
                cfg.play_mode = val.value("play_mode", "Everyone");
                cfg.clean_mode = val.value("clean_mode", true);
                cfg.language = val.value("language", "en");
                cfg.autoplay = val.value("autoplay", false);
                cfg.show_thumbnails = val.value("show_thumbnails", true);
                cfg.command_delete = val.value("command_delete", false);
                cfg.admin_mode = val.value("admin_mode", "adminauth");
                cfg.clean_mode_duration = val.value("clean_mode_duration", 15);
                cfg.channel_play_id = val.value("channel_play_id", 0LL);
                cfg.rtmp_url = val.value("rtmp_url", "");
                cfg.rtmp_key = val.value("rtmp_key", "");
                if (val.contains("auth_users")) {
                    cfg.auth_users = val["auth_users"].get<std::unordered_set<int64_t>>();
                }
                chat_configs_[cid] = cfg;
            }
        }
    } catch (const std::exception& e) {
        Logger::Error("DATABASE", std::string("Failed to parse disk database: ") + e.what());
    }
}

ChatConfig Database::GetChatConfig(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    auto it = chat_configs_.find(chat_id);
    if (it != chat_configs_.end()) {
        return it->second;
    }
    ChatConfig def;
    chat_configs_[chat_id] = def;
    return def;
}

void Database::SetChatConfig(int64_t chat_id, const ChatConfig& cfg) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    chat_configs_[chat_id] = cfg;
    SaveToDisk();
}

bool Database::IsAutoplayEnabled(int64_t chat_id) {
    return GetChatConfig(chat_id).autoplay;
}

void Database::SetAutoplay(int64_t chat_id, bool enabled) {
    ChatConfig cfg = GetChatConfig(chat_id);
    cfg.autoplay = enabled;
    SetChatConfig(chat_id, cfg);
}

bool Database::IsThumbnailsEnabled(int64_t chat_id) {
    return GetChatConfig(chat_id).show_thumbnails;
}

void Database::SetThumbnails(int64_t chat_id, bool enabled) {
    ChatConfig cfg = GetChatConfig(chat_id);
    cfg.show_thumbnails = enabled;
    SetChatConfig(chat_id, cfg);
}

bool Database::IsMaintenanceEnabled() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return maintenance_mode_;
}

std::string Database::GetMaintenanceReason() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return maintenance_reason_;
}

void Database::SetMaintenance(bool enabled, const std::string& reason) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    maintenance_mode_ = enabled;
    maintenance_reason_ = reason;
    SaveToDisk();
}

bool Database::IsLoggerEnabled() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return logger_enabled_;
}

void Database::SetLoggerEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    logger_enabled_ = enabled;
    SaveToDisk();
}

bool Database::IsAutoLeaveEnabled() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return autoleave_enabled_;
}

void Database::SetAutoLeaveEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    autoleave_enabled_ = enabled;
    SaveToDisk();
}

void Database::AddSudo(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    sudo_users_.insert(user_id);
    SaveToDisk();
}

void Database::RemoveSudo(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    sudo_users_.erase(user_id);
    SaveToDisk();
}

bool Database::IsSudo(int64_t user_id, int64_t owner_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (user_id == owner_id) return true;
    return sudo_users_.find(user_id) != sudo_users_.end();
}

std::unordered_set<int64_t> Database::GetSudoUsers() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return sudo_users_;
}

void Database::Blacklist(int64_t id, const std::string& reason) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    blacklisted_[id] = reason;
    SaveToDisk();
}

void Database::Unblacklist(int64_t id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    blacklisted_.erase(id);
    SaveToDisk();
}

bool Database::IsBlacklisted(int64_t id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return blacklisted_.find(id) != blacklisted_.end();
}

void Database::IncrementPlays() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    total_plays_++;
    SaveToDisk();
}

void Database::RegisterChat(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    served_chats_.insert(chat_id);
    SaveToDisk();
}

int64_t Database::GetTotalPlays() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return total_plays_;
}

size_t Database::GetTotalChats() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return served_chats_.size();
}

std::unordered_set<int64_t> Database::GetServedChats() {
    std::lock_guard<std::mutex> lock(db_mutex_);
    return served_chats_;
}
