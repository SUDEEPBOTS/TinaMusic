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
#include <optional>
#include <string>
#include <unordered_set>

class PlatformYouTube {
public:
    static std::optional<Track> SearchTrack(const std::string& query, const std::string& requester, int64_t requester_id);
    static std::optional<Track> GetRelatedTrack(const std::string& current_title, const std::string& current_id, const std::unordered_set<std::string>& played_ids);
    static std::string ExtractDirectAudioURL(const std::string& video_url);

private:
    static std::string ExecCommand(const std::string& cmd);
    static std::string HttpPost(const std::string& url, const std::string& json_body, const std::string& referer);
    static std::string ExtractVideoId(const std::string& input);
    static std::optional<Track> FetchVideoDetails(const std::string& video_id);
    static std::optional<Track> SearchInnerTube(const std::string& query);
    static std::optional<Track> SearchInnerTubeMusic(const std::string& query);
    static std::optional<Track> SearchYtDlpFast(const std::string& query);
    static std::optional<Track> SearchYtDlpJson(const std::string& query);
};
