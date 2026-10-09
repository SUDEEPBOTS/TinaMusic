#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

class LocalizationManager {
public:
    static LocalizationManager& Instance();

    std::string Get(const std::string& lang, const std::string& key) const;
    void SetChatLang(int64_t chat_id, const std::string& lang);
    std::string GetChatLang(int64_t chat_id) const;

private:
    LocalizationManager();
    void InitLocales();

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> strings_;
    mutable std::mutex lang_mutex_;
    std::unordered_map<int64_t, std::string> chat_langs_;
};
