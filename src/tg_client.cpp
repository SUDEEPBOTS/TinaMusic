#include "tg_client.hpp"
#include "logger.hpp"
#include <curl/curl.h>

TelegramClient::TelegramClient(const std::string& bot_token)
    : bot_token_(bot_token), api_base_url_("https://api.telegram.org/bot" + bot_token + "/") {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

TelegramClient::~TelegramClient() {
}

size_t TelegramClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string TelegramClient::PerformRequest(const std::string& method, const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    std::string response_string;
    if (!curl) return "";

    std::string url = api_base_url_ + method;
    std::string json_data = payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 35L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        Logger::Error("TELEGRAM", std::string("Curl request failed: ") + curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response_string;
}

bool TelegramClient::TestConnection() {
    std::string res = PerformRequest("getMe", nlohmann::json::object());
    if (res.empty()) return false;
    try {
        auto j = nlohmann::json::parse(res);
        if (j.value("ok", false) && j.contains("result")) {
            bot_username_ = j["result"].value("username", "YukkiMusicBot");
            bot_first_name_ = j["result"].value("first_name", "YukkiMusic");
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

std::string TelegramClient::GetBotUsername() const {
    return bot_username_;
}

std::string TelegramClient::GetBotFirstName() const {
    return bot_first_name_;
}

std::vector<TgUpdate> TelegramClient::GetUpdates(int64_t offset, int timeout) {
    nlohmann::json payload = {
        {"offset", offset},
        {"timeout", timeout},
        {"allowed_updates", {"message", "callback_query"}}
    };

    std::string res = PerformRequest("getUpdates", payload);
    std::vector<TgUpdate> updates;
    if (res.empty()) return updates;

    try {
        auto root = nlohmann::json::parse(res);
        if (!root.value("ok", false)) return updates;

        for (const auto& item : root["result"]) {
            TgUpdate u;
            u.update_id = item.value("update_id", 0LL);

            if (item.contains("message")) {
                u.has_message = true;
                const auto& m = item["message"];
                u.message.message_id = m.value("message_id", 0);
                u.message.date = m.value("date", 0LL);
                u.message.text = m.value("text", "");

                if (m.contains("from")) {
                    u.message.from.id = m["from"].value("id", 0LL);
                    u.message.from.is_bot = m["from"].value("is_bot", false);
                    u.message.from.first_name = m["from"].value("first_name", "");
                    u.message.from.username = m["from"].value("username", "");
                }

                if (m.contains("chat")) {
                    u.message.chat.id = m["chat"].value("id", 0LL);
                    u.message.chat.type = m["chat"].value("type", "");
                    u.message.chat.title = m["chat"].value("title", "");
                }
            }

            if (item.contains("callback_query")) {
                u.has_callback = true;
                const auto& c = item["callback_query"];
                u.callback_query.id = c.value("id", "");
                u.callback_query.data = c.value("data", "");

                if (c.contains("from")) {
                    u.callback_query.from.id = c["from"].value("id", 0LL);
                    u.callback_query.from.first_name = c["from"].value("first_name", "");
                    u.callback_query.from.username = c["from"].value("username", "");
                }

                if (c.contains("message")) {
                    u.callback_query.message.message_id = c["message"].value("message_id", 0);
                    if (c["message"].contains("chat")) {
                        u.callback_query.message.chat.id = c["message"]["chat"].value("id", 0LL);
                    }
                }
            }

            updates.push_back(u);
        }
    } catch (const std::exception& e) {
        Logger::Error("TELEGRAM", std::string("Failed to parse updates: ") + e.what());
    }

    return updates;
}

int TelegramClient::SendMessage(int64_t chat_id, const std::string& text, const nlohmann::json& markup) {
    nlohmann::json payload = {
        {"chat_id", chat_id},
        {"text", text},
        {"parse_mode", "HTML"}
    };
    if (!markup.is_null() && !markup.empty()) {
        payload["reply_markup"] = markup;
    }

    std::string res = PerformRequest("sendMessage", payload);
    if (res.empty()) return 0;
    try {
        auto j = nlohmann::json::parse(res);
        if (j.value("ok", false) && j.contains("result")) {
            return j["result"].value("message_id", 0);
        }
    } catch (...) {}
    return 0;
}

bool TelegramClient::EditMessageText(int64_t chat_id, int message_id, const std::string& text, const nlohmann::json& markup) {
    nlohmann::json payload = {
        {"chat_id", chat_id},
        {"message_id", message_id},
        {"text", text},
        {"parse_mode", "HTML"}
    };
    if (!markup.is_null() && !markup.empty()) {
        payload["reply_markup"] = markup;
    }

    std::string res = PerformRequest("editMessageText", payload);
    if (res.empty()) return false;
    try {
        auto j = nlohmann::json::parse(res);
        return j.value("ok", false);
    } catch (...) {
        return false;
    }
}

bool TelegramClient::DeleteMessage(int64_t chat_id, int message_id) {
    nlohmann::json payload = {
        {"chat_id", chat_id},
        {"message_id", message_id}
    };
    std::string res = PerformRequest("deleteMessage", payload);
    if (res.empty()) return false;
    try {
        auto j = nlohmann::json::parse(res);
        return j.value("ok", false);
    } catch (...) {
        return false;
    }
}

bool TelegramClient::AnswerCallbackQuery(const std::string& callback_query_id, const std::string& text, bool show_alert) {
    nlohmann::json payload = {
        {"callback_query_id", callback_query_id}
    };
    if (!text.empty()) {
        payload["text"] = text;
    }
    if (show_alert) {
        payload["show_alert"] = true;
    }
    std::string res = PerformRequest("answerCallbackQuery", payload);
    return !res.empty();
}
