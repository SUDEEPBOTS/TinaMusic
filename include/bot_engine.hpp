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
    void HandlePlayer(const TgMessage& msg);
    void HandleFPlay(const TgMessage& msg, const std::string& query);
    void HandleUserbotJoin(const TgMessage& msg);
    void HandlePause(const TgMessage& msg);
    void HandleResume(const TgMessage& msg);
    void HandleSkip(const TgMessage& msg);
    void HandleStop(const TgMessage& msg);
    void HandleMute(const TgMessage& msg);
    void HandleUnmute(const TgMessage& msg);
    void HandleSeekBack(const TgMessage& msg, const std::string& args);
    void HandleJump(const TgMessage& msg, const std::string& args);
    void HandlePosition(const TgMessage& msg);
    void HandleClear(const TgMessage& msg);
    void HandleRemove(const TgMessage& msg, const std::string& args);
    void HandleMove(const TgMessage& msg, const std::string& args);
    void HandleRestore(const TgMessage& msg);
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
    void HandleAuthList(const TgMessage& msg);
    void HandleBlacklist(const TgMessage& msg, const std::string& args);
    void HandleSudoList(const TgMessage& msg);
    void HandleAddSudo(const TgMessage& msg, const std::string& args);
    void HandleDelSudo(const TgMessage& msg, const std::string& args);
    void HandleBlockUser(const TgMessage& msg, const std::string& args);
    void HandleUnblockUser(const TgMessage& msg, const std::string& args);
    void HandleBlockChat(const TgMessage& msg, const std::string& args);
    void HandleUnblockChat(const TgMessage& msg, const std::string& args);
    void HandleBlacklisted(const TgMessage& msg);
    void HandleSpeedtest(const TgMessage& msg);
    void HandleActive(const TgMessage& msg);
    void HandleLogs(const TgMessage& msg);
    void HandleBug(const TgMessage& msg, const std::string& args);
    void HandlePrivacy(const TgMessage& msg);
    void HandleShell(const TgMessage& msg, const std::string& args);
    void HandleRestart(const TgMessage& msg);
    void HandleAutoplay(const TgMessage& msg, const std::string& args);
    void HandleNoThumb(const TgMessage& msg, const std::string& args);
    void HandlePlayMode(const TgMessage& msg, const std::string& args);
    void HandleAdminMode(const TgMessage& msg, const std::string& args);
    void HandleCmdDelete(const TgMessage& msg, const std::string& args);
    void HandleCleanMode(const TgMessage& msg, const std::string& args);
    void HandleMaintenance(const TgMessage& msg, const std::string& args);
    void HandleAutoLeave(const TgMessage& msg, const std::string& args);
    void HandleLogger(const TgMessage& msg, const std::string& args);
    void HandleReload(const TgMessage& msg);
    void HandleJson(const TgMessage& msg, const std::string& args);
    void HandleEval(const TgMessage& msg, const std::string& args);
    void HandleSetCPlay(const TgMessage& msg, const std::string& args);
    void HandleStream(const TgMessage& msg, const std::string& args);
    void HandleStreamStop(const TgMessage& msg);
    void HandleStreamStatus(const TgMessage& msg);
    void HandleSetRTMP(const TgMessage& msg, const std::string& args);
    void StartIpcListener();
    void OnTrackEnded(int64_t chat_id);

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
