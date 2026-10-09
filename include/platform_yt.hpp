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
};
