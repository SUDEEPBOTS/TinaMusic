#pragma once

#include "models.hpp"
#include <optional>
#include <string>

class PlatformYouTube {
public:
    static std::optional<Track> SearchTrack(const std::string& query, const std::string& requester, int64_t requester_id);
    static std::string ExtractDirectAudioURL(const std::string& video_url);

private:
    static std::string ExecCommand(const std::string& cmd);
};
