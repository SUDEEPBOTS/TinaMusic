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

#include "tinaproto/session.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::string test_session = "BQFwyZ4AtFCm6nW7btd1fogfVyowc02LIvJQ7KDP03r59AuzhhRwO1C5ULNGoH0F4Z05dCS6-stmK8P1Vf4CHkDjuTD9YdUZzWppwK6M4RZzonFfyqqp8PjPEjeXtBtZzBMirT4rFPdYF-HIzU8eLCq_DusODnvj-0gKho3h4BwU3lZawoNoEX-6rFGwmeQqzowj4vMkc163TKgaHBjDZDeuiG5HkMfFkDy6bBHgl36MAuAAROf19mY-kaOHQmcWQOXs4uoAvp91x33T_EE3wzE7zu1z7BkN0rc42DnEwuFKunkoj0SCIulhcBKk0JKtsC0jjnnPDD44KJWa5xYwRUjxF-tqsgAAAAH7TuXNAA";

    auto sess = tinaproto::SessionDecoder::DecodePyrogram(test_session);
    if (!sess.has_value()) {
        std::cerr << "FAIL: Session decode failed!" << std::endl;
        return 1;
    }

    std::cout << "[TINAPROTO] DC ID: " << static_cast<int>(sess->dc_id) << std::endl;
    std::cout << "[TINAPROTO] DC IP: " << sess->dc_ip << ":" << sess->dc_port << std::endl;
    std::cout << "[TINAPROTO] User ID: " << sess->user_id << std::endl;
    std::cout << "[TINAPROTO] AuthKey Size: " << sess->auth_key.size() << " bytes" << std::endl;

    assert(sess->auth_key.size() == 256);
    assert(sess->user_id == 8511219149);
    assert(!sess->dc_ip.empty());

    std::cout << "[TINAPROTO] All session decoder assertions passed successfully!" << std::endl;
    return 0;
}
