#include "platforms.hpp"
#include "platform_yt.hpp"
#include "logger.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    auto* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realsize);
    return realsize;
}

static std::string HttpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) return "";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return "";
    return response;
}

PlatformType MediaPlatformResolver::DetectPlatform(const std::string& query) {
    if (query.find("open.spotify.com") != std::string::npos || query.find("spotify:") != std::string::npos) {
        return PlatformType::SPOTIFY;
    }
    if (query.find("soundcloud.com") != std::string::npos) {
        return PlatformType::SOUNDCLOUD;
    }
    if (query.rfind("http://", 0) == 0 || query.rfind("https://", 0) == 0) {
        if (query.find("youtube.com") == std::string::npos && query.find("youtu.be") == std::string::npos) {
            std::string lower = query;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find(".mp3") != std::string::npos || lower.find(".m3u8") != std::string::npos ||
                lower.find(".aac") != std::string::npos || lower.find(".flac") != std::string::npos ||
                lower.find(".ogg") != std::string::npos || lower.find(".wav") != std::string::npos) {
                return PlatformType::DIRECT_URL;
            }
        }
    }
    return PlatformType::YOUTUBE;
}

std::optional<Track> MediaPlatformResolver::ResolveSpotify(const std::string& url, const std::string& requester, int64_t requester_id) {
    Logger::Info("SPOTIFY", "Resolving Spotify track metadata: " + url);
    std::string oembedUrl = "https://open.spotify.com/oembed?url=" + url;
    std::string rawJson = HttpGet(oembedUrl);

    if (rawJson.empty()) {
        Logger::Warn("SPOTIFY", "Failed to query Spotify oEmbed API, falling back to YouTube query");
        return PlatformYouTube::SearchTrack(url, requester, requester_id);
    }

    try {
        auto j = nlohmann::json::parse(rawJson);
        std::string title = j.value("title", "");
        if (!title.empty()) {
            Logger::Info("SPOTIFY", "Matched Spotify Track: " + title);
            auto ytTrack = PlatformYouTube::SearchTrack(title, requester, requester_id);
            if (ytTrack.has_value()) {
                ytTrack->platform = "Spotify";
                return ytTrack;
            }
        }
    } catch (const std::exception& e) {
        Logger::Error("SPOTIFY", std::string("JSON parsing failed: ") + e.what());
    }

    return PlatformYouTube::SearchTrack(url, requester, requester_id);
}

std::optional<Track> MediaPlatformResolver::ResolveSoundCloud(const std::string& url, const std::string& requester, int64_t requester_id) {
    Logger::Info("SOUNDCLOUD", "Resolving SoundCloud track: " + url);
    auto t = PlatformYouTube::SearchTrack(url, requester, requester_id);
    if (t.has_value()) {
        t->platform = "SoundCloud";
    }
    return t;
}

std::optional<Track> MediaPlatformResolver::ResolveDirectStream(const std::string& url, const std::string& requester, int64_t requester_id) {
    Logger::Info("DIRECT_STREAM", "Creating direct stream track: " + url);
    Track t;
    t.id = "direct_" + std::to_string(std::hash<std::string>{}(url));
    t.title = "Direct Live Audio Stream";
    t.url = url;
    t.stream_url = url;
    t.duration = "LIVE";
    t.duration_seconds = 0;
    t.requester_name = requester;
    t.requester_id = requester_id;
    t.platform = "Direct Stream";
    return t;
}

std::optional<Track> MediaPlatformResolver::Resolve(const std::string& query, const std::string& requester, int64_t requester_id) {
    PlatformType p = DetectPlatform(query);
    switch (p) {
        case PlatformType::SPOTIFY:
            return ResolveSpotify(query, requester, requester_id);
        case PlatformType::SOUNDCLOUD:
            return ResolveSoundCloud(query, requester, requester_id);
        case PlatformType::DIRECT_URL:
            return ResolveDirectStream(query, requester, requester_id);
        default:
            return PlatformYouTube::SearchTrack(query, requester, requester_id);
    }
}
