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

#include "config.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

static std::string TrimString(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void ConfigLoader::ParseEnvFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = TrimString(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        auto pos = trimmed.find('=');
        if (pos != std::string::npos) {
            std::string key = TrimString(trimmed.substr(0, pos));
            std::string val = TrimString(trimmed.substr(pos + 1));
            if (!val.empty() && (val.front() == '"' || val.front() == '\'') && val.front() == val.back()) {
                val = val.substr(1, val.length() - 2);
            }
            setenv(key.c_str(), val.c_str(), 1);
        }
    }
}

std::string ConfigLoader::GetEnvVar(const std::string& key, const std::string& default_val) {
    const char* val = std::getenv(key.c_str());
    if (val && *val) {
        return std::string(val);
    }
    return default_val;
}

AppConfig ConfigLoader::Load(const std::string& env_path) {
    ParseEnvFile(env_path);
    ParseEnvFile(".env");
    ParseEnvFile("../.env");

    AppConfig cfg;
    cfg.bot_token = GetEnvVar("TOKEN", GetEnvVar("BOT_TOKEN", ""));
    cfg.api_id = GetEnvVar("API_ID", "");
    cfg.api_hash = GetEnvVar("API_HASH", "");

    std::string owner_str = GetEnvVar("OWNER_ID", "0");
    if (!owner_str.empty()) {
        try { cfg.owner_id = std::stoll(owner_str); } catch (...) {}
    }

    cfg.mongo_uri = GetEnvVar("MONGO_DB_URI", GetEnvVar("MONGO_URI", ""));

    std::string log_str = GetEnvVar("LOGGER_ID", GetEnvVar("LOG_GROUP_ID", "0"));
    if (!log_str.empty()) {
        try { cfg.log_group_id = std::stoll(log_str); } catch (...) {}
    }

    cfg.string_session = GetEnvVar("STRING_SESSIONS", GetEnvVar("STRING_SESSION", ""));
    cfg.session_type = GetEnvVar("SESSION_TYPE", "pyrogram");
    cfg.support_chat = GetEnvVar("SUPPORT_CHAT", "https://t.me/Z0iiw");
    cfg.support_channel = GetEnvVar("SUPPORT_CHANNEL", "https://t.me/Z0iiw");
    cfg.default_lang = GetEnvVar("DEFAULT_LANG", "en");

    std::string dur_str = GetEnvVar("DURATION_LIMIT", "4200");
    try { cfg.duration_limit = std::stoi(dur_str); } catch (...) {}

    std::string queue_str = GetEnvVar("QUEUE_LIMIT", "24");
    try { cfg.max_queue_size = std::stoi(queue_str); } catch (...) {}

    cfg.default_platform = GetEnvVar("DEFAULT_PLATFORM", "YouTube");
    cfg.spotify_client_id = GetEnvVar("SPOTIFY_CLIENT_ID", "");
    cfg.spotify_client_secret = GetEnvVar("SPOTIFY_CLIENT_SECRET", "");
    cfg.yuki_api_url = GetEnvVar("YUKI_API_URL", GetEnvVar("API_URL", "https://music.yukiapi.site"));
    cfg.yuki_api_key = GetEnvVar("YUKI_API_KEY", GetEnvVar("API_KEY", "yuki_96fcd2d09a8d4f1e36896505bd6ebda8"));
    cfg.music_api_url = GetEnvVar("MUSIC_API_URL", GetEnvVar("STREAM_API_URL", "https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8"));
    cfg.ping_img_url = GetEnvVar("PING_IMG_URL", "https://telegra.ph/file/91533956c91d0fd7c9f20.jpg");

    std::string start_img = GetEnvVar("START_IMAGES", GetEnvVar("START_IMG_URL", ""));
    if (!start_img.empty()) {
        cfg.start_images.push_back(start_img);
    }

    return cfg;
}
