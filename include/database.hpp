#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <nlohmann/json.hpp>

struct ChatConfig {
    int volume{100};
    std::string play_mode{"Everyone"};
    bool clean_mode{true};
    std::string language{"en"};
    std::unordered_set<int64_t> auth_users;
};

class Database {
public:
    static Database& Instance();

    void Init(const std::string& db_path = "data/bot_database.json");

    ChatConfig GetChatConfig(int64_t chat_id);
    void SetChatConfig(int64_t chat_id, const ChatConfig& cfg);

    void AddSudo(int64_t user_id);
    void RemoveSudo(int64_t user_id);
    bool IsSudo(int64_t user_id, int64_t owner_id);
    std::unordered_set<int64_t> GetSudoUsers();

    void Blacklist(int64_t id, const std::string& reason);
    void Unblacklist(int64_t id);
    bool IsBlacklisted(int64_t id);

    void IncrementPlays();
    void RegisterChat(int64_t chat_id);
    int64_t GetTotalPlays();
    size_t GetTotalChats();

private:
    Database() = default;
    ~Database() = default;

    void SaveToDisk();
    void LoadFromDisk();

    std::string db_file_path_{"data/bot_database.json"};
    std::unordered_map<int64_t, ChatConfig> chat_configs_;
    std::unordered_set<int64_t> sudo_users_;
    std::unordered_map<int64_t, std::string> blacklisted_;
    std::unordered_set<int64_t> served_chats_;
    int64_t total_plays_{0};
    std::mutex db_mutex_;
};
