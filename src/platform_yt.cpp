#include "platform_yt.hpp"
#include "logger.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <memory>
#include <sstream>

struct PipeCloser {
    void operator()(FILE* fp) const {
        if (fp) {
            pclose(fp);
        }
    }
};

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

std::optional<Track> PlatformYouTube::SearchTrack(const std::string& query, const std::string& requester, int64_t requester_id) {
    std::string cleanQuery = SanitizeInput(query);
    if (cleanQuery.empty()) return std::nullopt;

    std::string cmd;
    if (cleanQuery.rfind("http://", 0) == 0 || cleanQuery.rfind("https://", 0) == 0) {
        cmd = "yt-dlp -j --no-playlist \"" + cleanQuery + "\" 2>/dev/null";
    } else {
        cmd = "yt-dlp -j --default-search \"ytsearch1:\" --no-playlist \"" + cleanQuery + "\" 2>/dev/null";
    }

    std::string rawJson = ExecCommand(cmd);
    if (rawJson.empty()) {
        Logger::Warn("YOUTUBE", "No metadata returned for query: " + cleanQuery);
        return std::nullopt;
    }

    try {
        auto j = nlohmann::json::parse(rawJson);
        Track t;
        t.id = j.value("id", "");
        t.title = j.value("title", "Unknown Track");
        t.url = j.value("webpage_url", "https://youtube.com/watch?v=" + t.id);
        
        int64_t dur = j.value("duration", 0);
        t.duration_seconds = dur;
        int mins = static_cast<int>(dur / 60);
        int secs = static_cast<int>(dur % 60);
        std::stringstream ss;
        ss << mins << ":" << (secs < 10 ? "0" : "") << secs;
        t.duration = ss.str();

        t.thumbnail_url = j.value("thumbnail", "");
        if (t.thumbnail_url.empty() && !t.id.empty()) {
            t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
        }

        t.requester_name = requester;
        t.requester_id = requester_id;
        t.platform = "YouTube";

        const char* env_music_api = std::getenv("MUSIC_API_URL");
        std::string music_api = env_music_api ? env_music_api : "https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8";
        auto pos = music_api.find("{VIDEO_ID}");
        if (!t.id.empty() && pos != std::string::npos) {
            std::string direct_url = music_api;
            direct_url.replace(pos, 10, t.id);
            t.stream_url = direct_url;
        } else {
            t.stream_url = ExtractDirectAudioURL(t.url);
        }
        return t;
    } catch (const std::exception& e) {
        Logger::Error("YOUTUBE", std::string("JSON parsing error: ") + e.what());
        return std::nullopt;
    }
}

std::optional<Track> PlatformYouTube::GetRelatedTrack(const std::string& current_title, const std::string& current_id, const std::unordered_set<std::string>& played_ids) {
    std::string clean = SanitizeInput(current_title);
    if (clean.empty()) return std::nullopt;

    std::string cmd = "yt-dlp -j --default-search \"ytsearch5:\" --no-playlist \"ytsearch5:" + clean + " related\" 2>/dev/null";
    std::string output = ExecCommand(cmd);
    if (output.empty()) return std::nullopt;

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
            int mins = static_cast<int>(dur / 60);
            int secs = static_cast<int>(dur % 60);
            std::stringstream dss;
            dss << mins << ":" << (secs < 10 ? "0" : "") << secs;
            t.duration = dss.str();

            t.thumbnail_url = j.value("thumbnail", "");
            if (t.thumbnail_url.empty()) {
                t.thumbnail_url = "https://i.ytimg.com/vi/" + t.id + "/hqdefault.jpg";
            }

            t.requester_name = "🎵 ᴀᴜᴛᴏᴘʟᴀʏ";
            t.requester_id = 0;
            t.platform = "YouTube";

            const char* env_music_api = std::getenv("MUSIC_API_URL");
            std::string music_api = env_music_api ? env_music_api : "https://music.yukiapi.site/stream/{VIDEO_ID}?key=yuki_96fcd2d09a8d4f1e36896505bd6ebda8";
            auto pos = music_api.find("{VIDEO_ID}");
            if (!t.id.empty() && pos != std::string::npos) {
                std::string direct_url = music_api;
                direct_url.replace(pos, 10, t.id);
                t.stream_url = direct_url;
            } else {
                t.stream_url = ExtractDirectAudioURL(t.url);
            }
            return t;
        } catch (...) {
            continue;
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
        std::string vid;
        auto vpos = cleanUrl.find("v=");
        if (vpos != std::string::npos) {
            vid = cleanUrl.substr(vpos + 2);
            auto amp = vid.find('&');
            if (amp != std::string::npos) vid = vid.substr(0, amp);
        } else {
            auto slash = cleanUrl.rfind('/');
            if (slash != std::string::npos && slash + 1 < cleanUrl.length()) {
                vid = cleanUrl.substr(slash + 1);
                auto qmark = vid.find('?');
                if (qmark != std::string::npos) vid = vid.substr(0, qmark);
            }
        }
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
