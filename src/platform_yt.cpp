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
 * Copyright (C) 2026 TheTeamVivek / SUDEEPBOTS / TinaMusic Contributors
 * All Rights Reserved.
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

#include "platform_yt.hpp"
#include "logger.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <unordered_map>

struct PipeCloser {
    void operator()(FILE* fp) const {
        if (fp) {
            pclose(fp);
        }
    }
};

struct CacheEntry {
    Track track;
    std::chrono::steady_clock::time_point timestamp;
};

static std::unordered_map<std::string, CacheEntry> s_trackCache;
static std::mutex s_cacheMutex;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

static std::string SanitizeInput(const std::string& input) {
    std::string clean;
    for (char c : input) {
        if (c == '"' || c == '\'' || c == '`' || c == '$' || c == ';' || c == '&' || c == '|') {
            continue;
        }
        clean += c;
    }
    return clean;
}

static std::string ToLowerString(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::tolower(c); });
    return res;
}

static int64_t ParseDurationString(const std::string& s) {
    int64_t total = 0;
    int64_t cur = 0;
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            cur = cur * 10 + (c - '0');
        } else if (c == ':') {
            total = total * 60 + cur;
            cur = 0;
        }
    }
    return total * 60 + cur;
}

static std::string FormatDuration(int64_t dur) {
    int mins = static_cast<int>(dur / 60);
    int secs = static_cast<int>(dur % 60);
    std::stringstream ss;
    ss << mins << ":" << (secs < 10 ? "0" : "") << secs;
    return ss.str();
}

static bool ParseVideoRenderer(const nlohmann::json& node, Track& out) {
    if (node.is_object()) {
        if (node.contains("videoRenderer")) {
            const auto& vr = node["videoRenderer"];
            if (vr.contains("videoId") && vr["videoId"].is_string()) {
                std::string vid = vr["videoId"].get<std::string>();
                if (vid.length() == 11) {
                    out.id = vid;
                    out.title = "YouTube Track";
                    if (vr.contains("title")) {
                        const auto& t = vr["title"];
                        if (t.contains("runs") && t["runs"].is_array() && !t["runs"].empty()) {
                            if (t["runs"][0].contains("text") && t["runs"][0]["text"].is_string()) {
                                out.title = t["runs"][0]["text"].get<std::string>();
                            }
                        } else if (t.contains("simpleText") && t["simpleText"].is_string()) {
                            out.title = t["simpleText"].get<std::string>();
                        }
                    }
                    if (vr.contains("lengthText") && vr["lengthText"].contains("simpleText") && vr["lengthText"]["simpleText"].is_string()) {
                        out.duration = vr["lengthText"]["simpleText"].get<std::string>();
                        out.duration_seconds = ParseDurationString(out.duration);
                    }
                    if (vr.contains("thumbnail") && vr["thumbnail"].contains("thumbnails") && vr["thumbnail"]["thumbnails"].is_array()) {
                        const auto& thumbs = vr["thumbnail"]["thumbnails"];
                        if (!thumbs.empty() && thumbs.back().contains("url") && thumbs.back()["url"].is_string()) {
                            out.thumbnail_url = thumbs.back()["url"].get<std::string>();
                        }
                    }
                    if (out.thumbnail_url.empty()) {
                        out.thumbnail_url = "https://i.ytimg.com/vi/" + out.id + "/hqdefault.jpg";
                    }
                    out.url = "https://www.youtube.com/watch?v=" + out.id;
                    return true;
                }
            }
        }
        for (auto it = node.begin(); it != node.end(); ++it) {
            if (ParseVideoRenderer(it.value(), out)) {
                return true;
            }
        }
    } else if (node.is_array()) {
        for (const auto& item : node) {
            if (ParseVideoRenderer(item, out)) {
                return true;
            }
        }
    }
    return false;
}

static void CollectVideoRenderers(const nlohmann::json& node, std::vector<Track>& list, size_t limit) {
    if (list.size() >= limit) return;
    if (node.is_object()) {
        if (node.contains("videoRenderer")) {
            const auto& vr = node["videoRenderer"];
            if (vr.contains("videoId") && vr["videoId"].is_string()) {
                std::string vid = vr["videoId"].get<std::string>();
                if (vid.length() == 11) {
                    Track out;
                    out.id = vid;
                    out.title = "YouTube Track";
                    if (vr.contains("title")) {
                        const auto& t = vr["title"];
                        if (t.contains("runs") && t["runs"].is_array() && !t["runs"].empty()) {
                            if (t["runs"][0].contains("text") && t["runs"][0]["text"].is_string()) {
                                out.title = t["runs"][0]["text"].get<std::string>();
                            }
                        } else if (t.contains("simpleText") && t["simpleText"].is_string()) {
                            out.title = t["simpleText"].get<std::string>();
                        }
                    }
                    if (vr.contains("lengthText") && vr["lengthText"].contains("simpleText") && vr["lengthText"]["simpleText"].is_string()) {
                        out.duration = vr["lengthText"]["simpleText"].get<std::string>();
                        out.duration_seconds = ParseDurationString(out.duration);
                    }
                    if (vr.contains("thumbnail") && vr["thumbnail"].contains("thumbnails") && vr["thumbnail"]["thumbnails"].is_array()) {
                        const auto& thumbs = vr["thumbnail"]["thumbnails"];
                        if (!thumbs.empty() && thumbs.back().contains("url") && thumbs.back()["url"].is_string()) {
                            out.thumbnail_url = thumbs.back()["url"].get<std::string>();
                        }
                    }
                    if (out.thumbnail_url.empty()) {
                        out.thumbnail_url = "https://i.ytimg.com/vi/" + out.id + "/hqdefault.jpg";
                    }
                    out.url = "https://www.youtube.com/watch?v=" + out.id;
                    list.push_back(out);
                    if (list.size() >= limit) return;
                }
            }
        }
        for (auto it = node.begin(); it != node.end(); ++it) {
            CollectVideoRenderers(it.value(), list, limit);
            if (list.size() >= limit) return;
        }
    } else if (node.is_array()) {
        for (const auto& item : node) {
            CollectVideoRenderers(item, list, limit);
            if (list.size() >= limit) return;
        }
    }
}

static bool ParseMusicItem(const nlohmann::json& node, Track& out) {
    if (node.is_object()) {
        if (node.contains("musicResponsiveListItemRenderer")) {
            const auto& mr = node["musicResponsiveListItemRenderer"];
            std::string vid;
            if (mr.contains("playlistItemData") && mr["playlistItemData"].contains("videoId") && mr["playlistItemData"]["videoId"].is_string()) {
                vid = mr["playlistItemData"]["videoId"].get<std::string>();
            }
            if (vid.empty() && mr.contains("navigationEndpoint") && mr["navigationEndpoint"].contains("watchEndpoint") && mr["navigationEndpoint"]["watchEndpoint"].contains("videoId") && mr["navigationEndpoint"]["watchEndpoint"]["videoId"].is_string()) {
                vid = mr["navigationEndpoint"]["watchEndpoint"]["videoId"].get<std::string>();
            }
            if (vid.length() == 11) {
                out.id = vid;
                out.title = "YouTube Music Track";
                if (mr.contains("flexColumns") && mr["flexColumns"].is_array() && !mr["flexColumns"].empty()) {
                    const auto& col0 = mr["flexColumns"][0];
                    if (col0.contains("musicResponsiveListItemFlexColumnRenderer") && col0["musicResponsiveListItemFlexColumnRenderer"].contains("text") && col0["musicResponsiveListItemFlexColumnRenderer"]["text"].contains("runs")) {
                        const auto& runs = col0["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"];
                        if (runs.is_array() && !runs.empty() && runs[0].contains("text") && runs[0]["text"].is_string()) {
                            out.title = runs[0]["text"].get<std::string>();
                        }
                    }
                    if (mr["flexColumns"].size() > 1) {
                        const auto& col1 = mr["flexColumns"][1];
                        if (col1.contains("musicResponsiveListItemFlexColumnRenderer") && col1["musicResponsiveListItemFlexColumnRenderer"].contains("text") && col1["musicResponsiveListItemFlexColumnRenderer"]["text"].contains("runs")) {
                            const auto& runs = col1["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"];
                            if (runs.is_array()) {
                                for (auto it = runs.rbegin(); it != runs.rend(); ++it) {
                                    if (it->contains("text") && (*it)["text"].is_string()) {
                                        std::string txt = (*it)["text"].get<std::string>();
                                        if (txt.find(':') != std::string::npos) {
                                            out.duration = txt;
                                            out.duration_seconds = ParseDurationString(txt);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                out.thumbnail_url = "https://i.ytimg.com/vi/" + out.id + "/hqdefault.jpg";
                out.url = "https://www.youtube.com/watch?v=" + out.id;
                return true;
            }
        }
        for (auto it = node.begin(); it != node.end(); ++it) {
            if (ParseMusicItem(it.value(), out)) return true;
        }
    } else if (node.is_array()) {
        for (const auto& item : node) {
            if (ParseMusicItem(item, out)) return true;
        }
    }
    return false;
}

std::string PlatformYouTube::ExecCommand(const std::string& cmd) {
    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, PipeCloser> pipe(popen(cmd.c_str(), "r"));
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string PlatformYouTube::HttpPost(const std::string& url, const std::string& json_body, const std::string& referer) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string ref_header = "Referer: " + referer;
    headers = curl_slist_append(headers, ref_header.c_str());
    std::string origin_header = "Origin: " + referer;
    headers = curl_slist_append(headers, origin_header.c_str());
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code >= 400) {
        return "";
    }
    return response;
}

std::string PlatformYouTube::ExtractVideoId(const std::string& input) {
    if (input.length() == 11) {
        bool valid = true;
        for (char c : input) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
                valid = false;
                break;
            }
        }
        if (valid) return input;
    }
    std::regex re(R"((?:v=|youtu\.be\/|shorts\/|embed\/|live\/)([a-zA-Z0-9_-]{11}))");
    std::smatch match;
    if (std::regex_search(input, match, re) && match.size() > 1) {
        return match.str(1);
    }
    return "";
}

std::optional<Track> PlatformYouTube::FetchVideoDetails(const std::string& video_id) {
    nlohmann::json payload;
    payload["context"]["client"]["clientName"] = "WEB";
    payload["context"]["client"]["clientVersion"] = "2.20250101.01.00";
    payload["context"]["client"]["hl"] = "en-IN";
    payload["context"]["client"]["gl"] = "IN";
    payload["videoId"] = video_id;

    std::string res = HttpPost("https://www.youtube.com/youtubei/v1/player?key=AIzaSyBOti4mM-6x9WDnZIjIeyEU21OpBXqWBgw", payload.dump(), "https://www.youtube.com/");
    if (res.empty()) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(res);
        if (j.contains("videoDetails")) {
            const auto& vd = j["videoDetails"];
            Track t;
            t.id = vd.value("videoId", video_id);
            t.title = vd.value("title", "YouTube Track");
            std::string secStr = vd.value("lengthSeconds", "0");
            try {
                t.duration_seconds = std::stoll(secStr);
            } catch (...) {
                t.duration_seconds = 0;
            }
            t.duration = FormatDuration(t.duration_seconds);
            t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
            t.url = "https://www.youtube.com/watch?v=" + t.id;
            return t;
        }
    } catch (...) {}
    return std::nullopt;
}

std::optional<Track> PlatformYouTube::SearchInnerTube(const std::string& query) {
    nlohmann::json payload;
    payload["context"]["client"]["clientName"] = "WEB";
    payload["context"]["client"]["clientVersion"] = "2.20250101.01.00";
    payload["context"]["client"]["hl"] = "en-IN";
    payload["context"]["client"]["gl"] = "IN";
    payload["query"] = query;
    payload["params"] = "CAASAhAB";

    std::string res = HttpPost("https://www.youtube.com/youtubei/v1/search?key=AIzaSyBOti4mM-6x9WDnZIjIeyEU21OpBXqWBgw", payload.dump(), "https://www.youtube.com/");
    if (res.empty()) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(res);
        Track t;
        if (ParseVideoRenderer(j, t)) {
            return t;
        }
    } catch (...) {}
    return std::nullopt;
}

std::optional<Track> PlatformYouTube::SearchInnerTubeMusic(const std::string& query) {
    nlohmann::json payload;
    payload["context"]["client"]["clientName"] = "WEB_REMIX";
    payload["context"]["client"]["clientVersion"] = "1.20260213.01.00";
    payload["context"]["client"]["hl"] = "en-IN";
    payload["context"]["client"]["gl"] = "IN";
    payload["query"] = query;

    std::string res = HttpPost("https://music.youtube.com/youtubei/v1/search?key=AIzaSyBOti4mM-6x9WDnZIjIeyEU21OpBXqWBgw", payload.dump(), "https://music.youtube.com/");
    if (res.empty()) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(res);
        Track t;
        if (ParseMusicItem(j, t)) {
            return t;
        }
    } catch (...) {}
    return std::nullopt;
}

std::optional<Track> PlatformYouTube::SearchYtDlpFast(const std::string& query) {
    std::string cmd = "yt-dlp --no-playlist --geo-bypass --get-id --get-title --get-duration \"ytsearch1:" + query + "\" 2>/dev/null";
    std::string out = ExecCommand(cmd);
    if (out.empty()) return std::nullopt;

    std::stringstream ss(out);
    std::string title, vid, dur;
    std::getline(ss, title);
    std::getline(ss, vid);
    std::getline(ss, dur);

    while (!title.empty() && (title.back() == '\r' || title.back() == '\n')) title.pop_back();
    while (!vid.empty() && (vid.back() == '\r' || vid.back() == '\n')) vid.pop_back();
    while (!dur.empty() && (dur.back() == '\r' || dur.back() == '\n')) dur.pop_back();

    if (vid.length() == 11) {
        Track t;
        t.id = vid;
        t.title = title.empty() ? "YouTube Track" : title;
        t.duration = dur.empty() ? "0:00" : dur;
        t.duration_seconds = ParseDurationString(t.duration);
        t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
        t.url = "https://www.youtube.com/watch?v=" + t.id;
        return t;
    }
    return std::nullopt;
}

std::optional<Track> PlatformYouTube::SearchYtDlpJson(const std::string& query) {
    std::string cmd;
    if (query.rfind("http://", 0) == 0 || query.rfind("https://", 0) == 0) {
        cmd = "yt-dlp -j --no-playlist \"" + query + "\" 2>/dev/null";
    } else {
        cmd = "yt-dlp -j --default-search \"ytsearch1:\" --no-playlist \"" + query + "\" 2>/dev/null";
    }
    std::string rawJson = ExecCommand(cmd);
    if (rawJson.empty()) return std::nullopt;

    try {
        auto j = nlohmann::json::parse(rawJson);
        Track t;
        t.id = j.value("id", "");
        t.title = j.value("title", "Unknown Track");
        t.url = j.value("webpage_url", "https://youtube.com/watch?v=" + t.id);
        int64_t dur = j.value("duration", 0);
        t.duration_seconds = dur;
        t.duration = FormatDuration(dur);
        t.thumbnail_url = j.value("thumbnail", "");
        if (t.thumbnail_url.empty() && !t.id.empty()) {
            t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
        }
        return t;
    } catch (...) {}
    return std::nullopt;
}

static void PopulateStreamUrl(Track& t) {
    const char* env_music_api = std::getenv("MUSIC_API_URL");
    std::string music_api = env_music_api ? env_music_api : "https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8";
    auto pos = music_api.find("{VIDEO_ID}");
    if (!t.id.empty() && pos != std::string::npos) {
        std::string direct_url = music_api;
        direct_url.replace(pos, 10, t.id);
        t.stream_url = direct_url;
    } else {
        t.stream_url = PlatformYouTube::ExtractDirectAudioURL(t.url);
    }
}

std::optional<Track> PlatformYouTube::SearchTrack(const std::string& query, const std::string& requester, int64_t requester_id) {
    std::string cleanQuery = SanitizeInput(query);
    if (cleanQuery.empty()) return std::nullopt;

    std::string cacheKey = ToLowerString(cleanQuery);
    {
        std::lock_guard<std::mutex> lock(s_cacheMutex);
        auto it = s_trackCache.find(cacheKey);
        if (it != s_trackCache.end()) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::hours>(now - it->second.timestamp).count() < 1) {
                Track t = it->second.track;
                t.requester_name = requester;
                t.requester_id = requester_id;
                t.platform = "YouTube";
                PopulateStreamUrl(t);
                return t;
            }
        }
    }

    std::optional<Track> candidate;
    std::string extractedId = ExtractVideoId(cleanQuery);

    if (!extractedId.empty()) {
        candidate = FetchVideoDetails(extractedId);
        if (!candidate) {
            candidate = SearchYtDlpFast(cleanQuery);
        }
        if (!candidate) {
            candidate = SearchYtDlpJson(cleanQuery);
        }
    } else {
        candidate = SearchInnerTube(cleanQuery);
        if (!candidate) {
            candidate = SearchInnerTubeMusic(cleanQuery);
        }
        if (!candidate) {
            candidate = SearchYtDlpFast(cleanQuery);
        }
        if (!candidate) {
            candidate = SearchYtDlpJson(cleanQuery);
        }
    }

    if (!candidate) {
        Logger::Warn("YOUTUBE", "No metadata returned for query: " + cleanQuery);
        return std::nullopt;
    }

    Track t = *candidate;
    t.requester_name = requester;
    t.requester_id = requester_id;
    t.platform = "YouTube";
    PopulateStreamUrl(t);

    {
        std::lock_guard<std::mutex> lock(s_cacheMutex);
        CacheEntry entry{t, std::chrono::steady_clock::now()};
        s_trackCache[cacheKey] = entry;
        if (!t.id.empty()) {
            s_trackCache[t.id] = entry;
        }
    }

    return t;
}

std::optional<Track> PlatformYouTube::GetRelatedTrack(const std::string& current_title, const std::string& current_id, const std::unordered_set<std::string>& played_ids) {
    std::string clean = SanitizeInput(current_title);
    if (clean.empty()) return std::nullopt;

    std::string query = clean + " song";
    nlohmann::json payload;
    payload["context"]["client"]["clientName"] = "WEB";
    payload["context"]["client"]["clientVersion"] = "2.20250101.01.00";
    payload["context"]["client"]["hl"] = "en-IN";
    payload["context"]["client"]["gl"] = "IN";
    payload["query"] = query;
    payload["params"] = "CAASAhAB";

    std::string res = HttpPost("https://www.youtube.com/youtubei/v1/search?key=AIzaSyBOti4mM-6x9WDnZIjIeyEU21OpBXqWBgw", payload.dump(), "https://www.youtube.com/");
    if (!res.empty()) {
        try {
            auto j = nlohmann::json::parse(res);
            std::vector<Track> candidates;
            CollectVideoRenderers(j, candidates, 10);
            for (auto& t : candidates) {
                if (t.id.empty() || t.id == current_id || played_ids.find(t.id) != played_ids.end()) {
                    continue;
                }
                t.requester_name = "🎵 ᴀᴜᴛᴏᴘʟᴀʏ";
                t.requester_id = 0;
                t.platform = "YouTube";
                PopulateStreamUrl(t);
                return t;
            }
        } catch (...) {}
    }

    std::string cmd = "yt-dlp -j --default-search \"ytsearch5:\" --no-playlist \"ytsearch5:" + clean + " related\" 2>/dev/null";
    std::string output = ExecCommand(cmd);
    if (!output.empty()) {
        std::stringstream ss(output);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.empty() || line.front() != '{') continue;
            try {
                auto j = nlohmann::json::parse(line);
                std::string vid = j.value("id", "");
                if (vid.empty() || vid == current_id || played_ids.find(vid) != played_ids.end()) {
                    continue;
                }

                Track t;
                t.id = vid;
                t.title = j.value("title", "Related Song");
                t.url = j.value("webpage_url", "https://youtube.com/watch?v=" + t.id);
                int64_t dur = j.value("duration", 0);
                t.duration_seconds = dur;
                t.duration = FormatDuration(dur);
                t.thumbnail_url = j.value("thumbnail", "");
                if (t.thumbnail_url.empty()) {
                    t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
                }
                t.requester_name = "🎵 ᴀᴜᴛᴏᴘʟᴀʏ";
                t.requester_id = 0;
                t.platform = "YouTube";
                PopulateStreamUrl(t);
                return t;
            } catch (...) {
                continue;
            }
        }
    }

    return std::nullopt;
}

std::string PlatformYouTube::ExtractDirectAudioURL(const std::string& video_url) {
    std::string cleanUrl = SanitizeInput(video_url);
    const char* env_music_api = std::getenv("MUSIC_API_URL");
    std::string music_api = env_music_api ? env_music_api : "https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8";
    auto pos = music_api.find("{VIDEO_ID}");
    if (pos != std::string::npos) {
        std::string vid = ExtractVideoId(cleanUrl);
        if (!vid.empty()) {
            std::string direct_url = music_api;
            direct_url.replace(pos, 10, vid);
            return direct_url;
        }
    }
    std::string cmd = "yt-dlp -g -f bestaudio/best \"" + cleanUrl + "\" 2>/dev/null";
    std::string out = ExecCommand(cmd);
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
        out.pop_back();
    }
    return out;
}
