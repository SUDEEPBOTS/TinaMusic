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

#include "models.hpp"
#include "queue_manager.hpp"
#include "database.hpp"
#include "locales.hpp"
#include "logger.hpp"
#include <cassert>
#include <iostream>

void TestQueueManager() {
    Logger::Info("TEST", "Testing QueueManager...");
    auto& qm = QueueManager::Instance();
    int64_t chat_id = -100999;
    qm.Clear(chat_id);
    qm.ClearPlayedHistory(chat_id);

    Track t;
    t.id = "yt_1";
    t.title = "Test Track 1";
    t.duration = "3:30";
    t.requester_name = "Admin";
    qm.Enqueue(chat_id, t);

    assert(qm.QueueSize(chat_id) == 1);
    assert(qm.GetCurrent(chat_id).has_value());
    assert(qm.GetCurrent(chat_id)->title == "Test Track 1");
    assert(qm.HasPlayed(chat_id, "yt_1"));

    qm.SetLoop(chat_id, true);
    assert(qm.IsLooping(chat_id) == true);
    qm.SetLoop(chat_id, false);
    assert(qm.IsLooping(chat_id) == false);

    qm.Clear(chat_id);
    assert(qm.QueueSize(chat_id) == 0);
    Logger::Info("TEST", "QueueManager tests passed!");
}

void TestDatabase() {
    Logger::Info("TEST", "Testing Database persistence...");
    auto& db = Database::Instance();
    db.Init("data/test_db.json");

    int64_t testUser = 12345678;
    assert(!db.IsBlacklisted(testUser));
    db.Blacklist(testUser, "Spamming");
    assert(db.IsBlacklisted(testUser));
    db.Unblacklist(testUser);
    assert(!db.IsBlacklisted(testUser));

    db.AddSudo(testUser);
    assert(db.IsSudo(testUser, 9999));
    db.RemoveSudo(testUser);
    assert(!db.IsSudo(testUser, 9999));

    ChatConfig cfg;
    cfg.volume = 80;
    cfg.language = "hi";
    cfg.autoplay = true;
    cfg.show_thumbnails = false;
    db.SetChatConfig(-100888, cfg);

    ChatConfig loaded = db.GetChatConfig(-100888);
    assert(loaded.volume == 80);
    assert(loaded.language == "hi");
    assert(loaded.autoplay == true);
    assert(loaded.show_thumbnails == false);

    db.SetAutoplay(-100888, false);
    assert(!db.IsAutoplayEnabled(-100888));
    db.SetThumbnails(-100888, true);
    assert(db.IsThumbnailsEnabled(-100888));

    db.SetMaintenance(true, "Server upgrade");
    assert(db.IsMaintenanceEnabled() == true);
    assert(db.GetMaintenanceReason() == "Server upgrade");
    db.SetMaintenance(false, "");
    assert(db.IsMaintenanceEnabled() == false);

    db.SetLoggerEnabled(true);
    assert(db.IsLoggerEnabled() == true);
    db.SetLoggerEnabled(false);
    assert(db.IsLoggerEnabled() == false);

    db.SetAutoLeaveEnabled(true);
    assert(db.IsAutoLeaveEnabled() == true);
    db.SetAutoLeaveEnabled(false);
    assert(db.IsAutoLeaveEnabled() == false);

    Logger::Info("TEST", "Database tests passed!");
}

void TestLocales() {
    Logger::Info("TEST", "Testing Locales with YAML files...");
    auto& lm = LocalizationManager::Instance();

    std::string addBtn = lm.Get("en", "ADD_ME_BTN");
    assert(!addBtn.empty());
    assert(addBtn.find("ᴧᴅᴅ") != std::string::npos);

    std::string pingStart = lm.Get("en", "ping_start");
    assert(!pingStart.empty());
    assert(pingStart.find("ᴘɪηɢɪηɢ") != std::string::npos);

    std::string startPriv = lm.Get("en", "start_private", {{"user", "Alice"}, {"bot", "TinaBot"}});
    assert(!startPriv.empty());
    assert(startPriv.find("Alice") != std::string::npos);
    assert(startPriv.find("TinaBot") != std::string::npos);
    assert(startPriv.find("https://yukiapi.site/file/iCggpP12") != std::string::npos);

    std::string nowPlaying = lm.Get("en", "stream_now_playing", {
        {"thumb", "<a href=\"https://i.ytimg.com/vi/abc/hqdefault.jpg\">🎵</a>"},
        {"url", "https://youtube.com/watch?v=abc"},
        {"title", "Faded"},
        {"duration", "3:32"},
        {"by", "Shadow"}
    });
    assert(!nowPlaying.empty());
    assert(nowPlaying.find("Faded") != std::string::npos);
    assert(nowPlaying.find("hqdefault.jpg") != std::string::npos);

    std::string queueFinished = lm.Get("en", "stream_queue_finished");
    assert(!queueFinished.empty());
    assert(queueFinished.find("ǫᴜєᴜє ғɪηɪsʜєᴅ") != std::string::npos);

    std::string autoFetch = lm.Get("en", "autoplay_fetching_next");
    assert(!autoFetch.empty());
    assert(autoFetch.find("ᴧᴜᴛσᴘʟᴧʏ") != std::string::npos);

    std::string hiPing = lm.Get("hi", "ping_start");
    assert(!hiPing.empty());

    std::string arPing = lm.Get("ar", "ping_start");
    assert(!arPing.empty());

    std::string trPing = lm.Get("tr", "ping_start");
    assert(!trPing.empty());

    Logger::Info("TEST", "Locales YAML tests passed!");
}

int main() {
    Logger::Info("TEST", "Running full test suite...");
    TestQueueManager();
    TestDatabase();
    TestLocales();
    Logger::Info("TEST", "ALL COMPONENT TESTS PASSED 100%!");
    std::cout << "TEST_RESULT: 100% PASS" << std::endl;
    return 0;
}
