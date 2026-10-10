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
#include <vector>
#include <cstdint>

struct AppConfig {
    std::string bot_token;
    std::string api_id;
    std::string api_hash;
    int64_t owner_id{0};
    std::string mongo_uri;
    int64_t log_group_id{0};
    std::string string_session;
    std::string session_type{"pyrogram"};
    std::string support_chat{"https://t.me/Z0iiw"};
    std::string support_channel{"https://t.me/Z0iiw"};
    std::string default_lang{"en"};
    int duration_limit{4200};
    int max_queue_size{24};
    std::string default_platform{"YouTube"};
    std::string spotify_client_id;
    std::string spotify_client_secret;
    std::string yuki_api_url{"https://music.yukiapi.site"};
    std::string yuki_api_key{"yuki_96fcd2d09a8d4f1e36896505bd6ebda8"};
    std::string music_api_url{"https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8"};
    std::string ping_img_url{"https://telegra.ph/file/91533956c91d0fd7c9f20.jpg"};
    std::vector<std::string> start_images;
};

class ConfigLoader {
public:
    static AppConfig Load(const std::string& env_path = ".env");

private:
    static std::string GetEnvVar(const std::string& key, const std::string& default_val = "");
    static void ParseEnvFile(const std::string& path);
};
