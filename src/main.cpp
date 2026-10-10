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
 * Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 * Support: https://t.me/Z0iiw
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

#include "config.hpp"
#include "logger.hpp"
#include "audio_engine.hpp"
#include "bot_engine.hpp"
#include "tinaproto/client.hpp"
#include <curl/curl.h>
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
    (void)argc;
    (void)argv;

    std::cout << R"(
========================================================================
  TINAMUSIC C++20 NATIVE TELEGRAM MUSIC ENGINE
  High-Performance, Low-Latency Audio Streaming Platform
========================================================================
)" << std::endl;

    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    AppConfig config = ConfigLoader::Load();
    if (config.bot_token.empty()) {
        Logger::Error("STARTUP", "BOT_TOKEN not found in environment or .env file!");
        curl_global_cleanup();
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
        curl_global_cleanup();
        return 1;
    }
    Logger::Info("TELEGRAM", "Telegram connection established successfully.");

    int32_t api_id_int = 0;
    try {
        api_id_int = std::stoi(config.api_id);
    } catch (...) {}

    Logger::Info("STARTUP", "Initializing pure C++20 TinaProto native voice engine...");
    if (!tinaproto::TinaProtoClient::GetInstance().Initialize(config.string_session, api_id_int, config.api_hash)) {
        Logger::Warn("STARTUP", "TinaProtoClient initialization failed or deferred");
    } else {
        Logger::Info("STARTUP", "Pure C++20 TinaProto voice engine ready and operational");
    }

    g_bot = std::make_unique<BotEngine>(config);
    g_bot->Run();

    curl_global_cleanup();
    Logger::Info("SYSTEM", "TinaMusic C++ Engine stopped.");
    return 0;
}
