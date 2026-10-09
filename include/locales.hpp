#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>

using LocaleArgs = std::unordered_map<std::string, std::string>;

class LocalizationManager {
public:
    static LocalizationManager& Instance();

    bool Load(const std::string& locales_dir = "locales");
    std::string Get(const std::string& lang, const std::string& key, const LocaleArgs& args = {}) const;
    std::string F(int64_t chat_id, const std::string& key, const LocaleArgs& args = {}) const;
    void SetChatLang(int64_t chat_id, const std::string& lang);
    std::string GetChatLang(int64_t chat_id) const;
    std::vector<std::string> GetAvailableLanguages() const;

private:
    LocalizationManager();
    void InitLocales();

    std::string default_lang_{"en"};
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> strings_;
    mutable std::mutex lang_mutex_;
    std::unordered_map<int64_t, std::string> chat_langs_;
};
