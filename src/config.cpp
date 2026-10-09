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
            setenv(key.c_str(), val.c_str(), 0);
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
    ParseEnvFile("../.env");
    ParseEnvFile("/root/YukkiMusic-Go/.env");

    AppConfig cfg;
    cfg.bot_token = GetEnvVar("BOT_TOKEN", "");
    cfg.api_id = GetEnvVar("API_ID", "");
    cfg.api_hash = GetEnvVar("API_HASH", "");
    
    std::string owner_str = GetEnvVar("OWNER_ID", "0");
    if (!owner_str.empty()) {
        try { cfg.owner_id = std::stoll(owner_str); } catch (...) {}
    }

    cfg.mongo_uri = GetEnvVar("MONGO_URI", "");

    std::string log_str = GetEnvVar("LOG_GROUP_ID", "0");
    if (!log_str.empty()) {
        try { cfg.log_group_id = std::stoll(log_str); } catch (...) {}
    }

    cfg.default_platform = GetEnvVar("DEFAULT_PLATFORM", "YouTube");
    return cfg;
}
