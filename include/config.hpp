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
    std::string support_chat{"https://t.me/TheTeamVk"};
    std::string support_channel{"https://t.me/TheTeamVivek"};
    std::string default_lang{"en"};
    int duration_limit{4200};
    int max_queue_size{24};
    std::string default_platform{"YouTube"};
    std::string spotify_client_id;
    std::string spotify_client_secret;
    std::string fallen_api_url{"https://beta.fallenapi.fun"};
    std::string fallen_api_key;
    std::string music_api_url;
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
