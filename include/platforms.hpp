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
