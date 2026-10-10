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

#include "locales.hpp"
#include "logger.hpp"
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <iostream>

LocalizationManager& LocalizationManager::Instance() {
    static LocalizationManager instance;
    return instance;
}

LocalizationManager::LocalizationManager() {
    InitLocales();
}

void LocalizationManager::InitLocales() {
    std::vector<std::string> search_dirs = {
        "locales",
        "../locales",
        "./locales"
    };

    bool loaded = false;
    for (const auto& dir : search_dirs) {
        if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
            if (Load(dir)) {
                loaded = true;
                break;
            }
        }
    }

    if (!loaded) {
        strings_["en"]["welcome"] = "<blockquote>🎵 <b>Welcome to TinaMusic (C++ Engine)</b></blockquote>";
        strings_["en"]["help"] = "<blockquote>📖 <b>TinaMusic C++ Commands:</b></blockquote>";
        strings_["en"]["ping_start"] = "<blockquote>🏓 <b><u>ᴘɪηɢɪηɢ...</u></b>\n\n<i>ᴄʜєᴄᴋɪηɢ ʟᴧᴛєηᴄʏ &amp; sʏsᴛєᴍ sᴛᴧᴛs</i></blockquote>";
        strings_["en"]["start_group"] = "<b>💫 I’m here!</b> Try a command… if you think you can handle me 😉";
    }
}

bool LocalizationManager::Load(const std::string& locales_dir) {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    try {
        if (!std::filesystem::exists(locales_dir)) {
            return false;
        }

        int loaded_files = 0;
        for (const auto& entry : std::filesystem::directory_iterator(locales_dir)) {
            if (!entry.is_regular_file()) continue;

            auto path = entry.path();
            if (path.extension() == ".yml" || path.extension() == ".yaml") {
                std::string lang = path.stem().string();
                try {
                    YAML::Node root = YAML::LoadFile(path.string());
                    if (root.IsMap()) {
                        for (auto it = root.begin(); it != root.end(); ++it) {
                            std::string key = it->first.as<std::string>();
                            if (it->second.IsScalar()) {
                                strings_[lang][key] = it->second.as<std::string>();
                            }
                        }
                        loaded_files++;
                    }
                } catch (const std::exception& e) {
                    Logger::Error("LOCALES", "Failed to parse " + path.string() + ": " + e.what());
                }
            }
        }
        return loaded_files > 0;
    } catch (const std::exception& e) {
        Logger::Error("LOCALES", "Error iterating locales directory: " + std::string(e.what()));
        return false;
    }
}

std::string LocalizationManager::Get(const std::string& lang, const std::string& key, const LocaleArgs& args) const {
    std::lock_guard<std::mutex> lock(lang_mutex_);

    std::vector<std::string> try_langs = {lang, default_lang_, "en"};
    std::string result;
    bool found = false;

    for (const auto& l : try_langs) {
        auto lang_it = strings_.find(l);
        if (lang_it != strings_.end()) {
            auto key_it = lang_it->second.find(key);
            if (key_it != lang_it->second.end()) {
                result = key_it->second;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        return "[" + key + "]";
    }

    for (const auto& [placeholder, replacement] : args) {
        std::string token = "{" + placeholder + "}";
        size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos) {
            result.replace(pos, token.length(), replacement);
            pos += replacement.length();
        }
    }

    return result;
}

std::string LocalizationManager::F(int64_t chat_id, const std::string& key, const LocaleArgs& args) const {
    std::string lang = GetChatLang(chat_id);
    return Get(lang, key, args);
}

void LocalizationManager::SetChatLang(int64_t chat_id, const std::string& lang) {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    chat_langs_[chat_id] = lang;
}

std::string LocalizationManager::GetChatLang(int64_t chat_id) const {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    auto it = chat_langs_.find(chat_id);
    if (it != chat_langs_.end()) {
        return it->second;
    }
    return default_lang_;
}

std::vector<std::string> LocalizationManager::GetAvailableLanguages() const {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    std::vector<std::string> langs;
    for (const auto& [lang, _] : strings_) {
        langs.push_back(lang);
    }
    return langs;
}
