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
