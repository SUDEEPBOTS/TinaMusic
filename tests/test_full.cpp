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
    Logger::Info("TEST", "Testing Locales with YAML files...");
    auto& lm = LocalizationManager::Instance();

    std::string addBtn = lm.Get("en", "ADD_ME_BTN");
    assert(!addBtn.empty());
    assert(addBtn.find("ᴧᴅᴅ") != std::string::npos);

    std::string pingStart = lm.Get("en", "ping_start");
    assert(!pingStart.empty());
    assert(pingStart.find("ᴘɪηɢɪηɢ") != std::string::npos);

    std::string startPriv = lm.Get("en", "start_private", {{"user", "Alice"}, {"bot", "YukkiBot"}});
    assert(!startPriv.empty());
    assert(startPriv.find("Alice") != std::string::npos);
    assert(startPriv.find("YukkiBot") != std::string::npos);
    assert(startPriv.find("https://yukiapi.site/file/iCggpP12") != std::string::npos);

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
