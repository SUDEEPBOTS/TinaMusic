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

    Track t;
    t.id = "yt_1";
    t.title = "Test Track 1";
    t.duration = "3:30";
    t.requester_name = "Admin";
    qm.Enqueue(chat_id, t);

    assert(qm.QueueSize(chat_id) == 1);
    assert(qm.GetCurrent(chat_id).has_value());
    assert(qm.GetCurrent(chat_id)->title == "Test Track 1");

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
    db.SetChatConfig(-100888, cfg);
    ChatConfig loaded = db.GetChatConfig(-100888);
    assert(loaded.volume == 80);
    assert(loaded.language == "hi");

    Logger::Info("TEST", "Database tests passed!");
}

void TestLocales() {
    Logger::Info("TEST", "Testing Locales...");
    auto& lm = LocalizationManager::Instance();

    std::string enHelp = lm.Get("en", "help");
    assert(!enHelp.empty());
    assert(enHelp.find("YukkiMusic") != std::string::npos);

    std::string hiHelp = lm.Get("hi", "help");
    assert(!hiHelp.empty());
    assert(hiHelp.find("कमांड्स") != std::string::npos);

    std::string arHelp = lm.Get("ar", "help");
    assert(!arHelp.empty());

    std::string trHelp = lm.Get("tr", "help");
    assert(!trHelp.empty());

    Logger::Info("TEST", "Locales tests passed!");
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
