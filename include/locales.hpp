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
