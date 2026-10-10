/*
 * ============================================================================
 * TinaMusic - Telegram's First Native C++20 Music Engine
 * Copyright (C) 2026 TheTeamVivek / TinaMusic Contributors
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
 * ============================================================================
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class PlaybackStatus {
    IDLE,
    PLAYING,
    PAUSED,
    STOPPED
};

struct Track {
    std::string id;
    std::string title;
    std::string duration;
    int64_t duration_seconds{0};
    std::string url;
    std::string stream_url;
    std::string requester_name;
    int64_t requester_id{0};
    std::string platform{"YouTube"};
    std::string thumbnail_url;
};

struct ChatPlaybackState {
    int64_t chat_id{0};
    PlaybackStatus status{PlaybackStatus::IDLE};
    Track current_track;
    bool loop{false};
    int volume{100};
    int speed{100};
    int64_t start_timestamp{0};
};
