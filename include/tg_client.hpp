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

#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TgUser {
    int64_t id{0};
    bool is_bot{false};
    std::string first_name;
    std::string username;
};

struct TgChat {
    int64_t id{0};
    std::string type;
    std::string title;
};

struct TgMessage {
    int message_id{0};
    TgUser from;
    TgChat chat;
    std::string text;
    int64_t date{0};
};

struct TgCallbackQuery {
    std::string id;
    TgUser from;
    TgMessage message;
    std::string data;
};

struct TgUpdate {
    int64_t update_id{0};
    bool has_message{false};
    TgMessage message;
    bool has_callback{false};
    TgCallbackQuery callback_query;
};

class TelegramClient {
public:
    TelegramClient(const std::string& bot_token);
    ~TelegramClient();

    bool TestConnection();
    std::string GetBotUsername() const;
    std::string GetBotFirstName() const;
    std::vector<TgUpdate> GetUpdates(int64_t offset = 0, int timeout = 25);
    int SendMessage(int64_t chat_id, const std::string& text, const nlohmann::json& markup = nullptr);
    bool EditMessageText(int64_t chat_id, int message_id, const std::string& text, const nlohmann::json& markup = nullptr);
    bool DeleteMessage(int64_t chat_id, int message_id);
    bool AnswerCallbackQuery(const std::string& callback_query_id, const std::string& text = "", bool show_alert = false);

private:
    std::string bot_token_;
    std::string api_base_url_;
    std::string bot_username_{"TinaMusicBot"};
    std::string bot_first_name_{"TinaMusic"};

    std::string PerformRequest(const std::string& method, const nlohmann::json& payload);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};
