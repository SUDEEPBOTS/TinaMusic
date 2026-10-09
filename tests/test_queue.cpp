#include "models.hpp"
#include "queue_manager.hpp"
#include "logger.hpp"
#include <cassert>
#include <iostream>

int main() {
    Logger::Info("TEST", "Starting QueueManager unit tests...");

    auto& qm = QueueManager::Instance();
    int64_t chat_id = -100123456789LL;

    qm.Clear(chat_id);
    assert(qm.QueueSize(chat_id) == 0);
    assert(!qm.GetCurrent(chat_id).has_value());

    Track t1;
    t1.id = "song1";
    t1.title = "Alan Walker - Faded";
    t1.duration = "3:32";
    t1.requester_name = "UserA";

    qm.Enqueue(chat_id, t1);
    assert(qm.QueueSize(chat_id) == 1);
    assert(qm.GetCurrent(chat_id).has_value());
    assert(qm.GetCurrent(chat_id)->title == "Alan Walker - Faded");

    Track t2;
    t2.id = "song2";
    t2.title = "The Spectre";
    t2.duration = "3:13";
    t2.requester_name = "UserB";

    qm.Enqueue(chat_id, t2);
    assert(qm.QueueSize(chat_id) == 2);
    assert(qm.HasNext(chat_id));

    auto nextOpt = qm.Next(chat_id);
    assert(nextOpt.has_value());
    assert(nextOpt->title == "The Spectre");
    assert(qm.QueueSize(chat_id) == 1);

    auto endOpt = qm.Next(chat_id);
    assert(!endOpt.has_value());
    assert(qm.QueueSize(chat_id) == 0);

    Logger::Info("TEST", "All QueueManager unit tests passed successfully!");
    std::cout << "TEST_RESULT: PASS" << std::endl;
    return 0;
}
