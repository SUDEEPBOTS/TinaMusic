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

#include "models.hpp"
#include <string>
#include <optional>
#include <vector>

enum class PlatformType {
    YOUTUBE,
    SPOTIFY,
    SOUNDCLOUD,
    DIRECT_URL,
    TELEGRAM
};

class MediaPlatformResolver {
public:
    static PlatformType DetectPlatform(const std::string& query);

    static std::optional<Track> Resolve(const std::string& query, const std::string& requester, int64_t requester_id);

    static std::optional<Track> ResolveSpotify(const std::string& url, const std::string& requester, int64_t requester_id);
    static std::optional<Track> ResolveSoundCloud(const std::string& url, const std::string& requester, int64_t requester_id);
    static std::optional<Track> ResolveDirectStream(const std::string& url, const std::string& requester, int64_t requester_id);
};
