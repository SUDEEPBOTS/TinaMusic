#pragma once

#include "config.hpp"
#include "tg_client.hpp"
#include "queue_manager.hpp"
#include "audio_engine.hpp"
#include <atomic>
#include <memory>

class BotEngine {
public:
    BotEngine(const AppConfig& config);
    ~BotEngine() = default;

    void Run();
    void Stop();

private:
    void ProcessMessage(const TgMessage& msg);
    void ProcessCallback(const TgCallbackQuery& cb);

    void HandleStart(const TgMessage& msg);
    void HandleHelp(const TgMessage& msg);
    void HandlePing(const TgMessage& msg);
    void HandlePlay(const TgMessage& msg, const std::string& query);
    void HandlePause(const TgMessage& msg);
    void HandleResume(const TgMessage& msg);
    void HandleSkip(const TgMessage& msg);
    void HandleStop(const TgMessage& msg);
    void HandleQueue(const TgMessage& msg);
    void HandleLoop(const TgMessage& msg);
    void HandleShuffle(const TgMessage& msg);

    nlohmann::json MakePlayerControlsMarkup(int64_t chat_id);

    AppConfig config_;
    std::unique_ptr<TelegramClient> tg_;
    std::atomic<bool> is_running_{false};
    int64_t last_update_id_{0};
};
