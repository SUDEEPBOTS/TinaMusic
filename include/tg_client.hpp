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
    bool AnswerCallbackQuery(const std::string& callback_query_id, const std::string& text = "");

private:
    std::string bot_token_;
    std::string api_base_url_;
    std::string bot_username_{"YukkiMusicBot"};
    std::string bot_first_name_{"YukkiMusic"};

    std::string PerformRequest(const std::string& method, const nlohmann::json& payload);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};
