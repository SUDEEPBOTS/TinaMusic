#pragma once

#include <string>

struct AppConfig {
    std::string bot_token;
    std::string api_id;
    std::string api_hash;
    int64_t owner_id{0};
    std::string mongo_uri;
    int64_t log_group_id{0};
    std::string default_platform{"YouTube"};
    int max_queue_size{50};
};

class ConfigLoader {
public:
    static AppConfig Load(const std::string& env_path = ".env");

private:
    static std::string GetEnvVar(const std::string& key, const std::string& default_val = "");
    static void ParseEnvFile(const std::string& path);
};
