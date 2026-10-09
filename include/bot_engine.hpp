#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>
#include "config.hpp"
#include "tg_client.hpp"
#include "queue_manager.hpp"
#include "database.hpp"
#include "locales.hpp"
#include "audio_engine.hpp"

class BotEngine {
public:
    BotEngine(const AppConfig& config);
    void Run();
    void Stop();

private:
    void ProcessMessage(const TgMessage& msg);
    void ProcessCallback(const TgCallbackQuery& cb);

    void HandleStart(const TgMessage& msg);
    void HandleHelp(const TgMessage& msg);
    void HandlePing(const TgMessage& msg);
    void HandleStats(const TgMessage& msg);
    void HandlePlay(const TgMessage& msg, const std::string& query);
    void HandlePause(const TgMessage& msg);
    void HandleResume(const TgMessage& msg);
    void HandleSkip(const TgMessage& msg);
    void HandleStop(const TgMessage& msg);
    void HandleQueue(const TgMessage& msg);
    void HandleLoop(const TgMessage& msg);
    void HandleShuffle(const TgMessage& msg);
    void HandleSeek(const TgMessage& msg, const std::string& args);
    void HandleSpeed(const TgMessage& msg, const std::string& args);
    void HandleReplay(const TgMessage& msg);
    void HandleSettings(const TgMessage& msg);
    void HandleLanguage(const TgMessage& msg, const std::string& args);
    void HandleBroadcast(const TgMessage& msg, const std::string& args);
    void HandleAuth(const TgMessage& msg);
    void HandleUnauth(const TgMessage& msg);
    void HandleBlacklist(const TgMessage& msg, const std::string& args);
    void HandleAutoplay(const TgMessage& msg, const std::string& args);
    void HandleNoThumb(const TgMessage& msg, const std::string& args);

    nlohmann::json MakeStartMarkup(int64_t chat_id);
    nlohmann::json MakeHelpMarkup(int64_t chat_id);
    nlohmann::json MakeBackMarkup(int64_t chat_id);
    nlohmann::json MakeSupportMarkup(int64_t chat_id);
    nlohmann::json MakePlayerMarkup(int64_t chat_id, int current_pos, int duration);
    nlohmann::json MakeSettingsMarkup(int64_t chat_id);
    nlohmann::json MakeLanguageMarkup();
    nlohmann::json BuildAutoplayMarkup(int64_t chat_id, bool enabled);
    std::string GetAutoplayMessage(bool enabled);

    std::string GetUptimeString();
    std::string GetSystemMemoryUsage();
    std::string GetCpuUsageString();
    std::string GetDiskUsageString();

    AppConfig config_;
    std::unique_ptr<TelegramClient> tg_;
    bool is_running_{false};
    int64_t last_update_id_{0};
    std::chrono::steady_clock::time_point boot_time_;
};
