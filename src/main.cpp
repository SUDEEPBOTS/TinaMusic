#include "config.hpp"
#include "logger.hpp"
#include "audio_engine.hpp"
#include "bot_engine.hpp"
#include <iostream>
#include <csignal>
#include <memory>

static std::unique_ptr<BotEngine> g_bot;

void SignalHandler(int signum) {
    Logger::Info("SYSTEM", "Received signal " + std::to_string(signum) + ", shutting down cleanly...");
    if (g_bot) {
        g_bot->Stop();
    }
}

int main(int argc, char* argv[]) {
    std::cout << R"(
========================================================================
  YUKKIMUSIC C++20 NATIVE TELEGRAM MUSIC ENGINE
  High-Performance, Low-Latency Audio Streaming Platform
========================================================================
)" << std::endl;

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    AppConfig config = ConfigLoader::Load();
    if (config.bot_token.empty()) {
        Logger::Error("STARTUP", "BOT_TOKEN not found in environment or .env file!");
        return 1;
    }

    Logger::Info("STARTUP", "Config loaded successfully.");

    if (!AudioEngine::Instance().VerifyFFmpeg()) {
        Logger::Warn("STARTUP", "FFmpeg not detected in system PATH. Audio playback will be impaired.");
    } else {
        Logger::Info("STARTUP", "FFmpeg audio engine verified and operational.");
    }

    TelegramClient testClient(config.bot_token);
    if (!testClient.TestConnection()) {
        Logger::Error("TELEGRAM", "Failed to connect to Telegram Bot API. Please check BOT_TOKEN.");
        return 1;
    }
    Logger::Info("TELEGRAM", "Telegram connection established successfully.");

    g_bot = std::make_unique<BotEngine>(config);
    g_bot->Run();

    Logger::Info("SYSTEM", "YukkiMusic C++ Engine stopped.");
    return 0;
}
