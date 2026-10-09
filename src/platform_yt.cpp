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
        t.requester_name = requester;
        t.requester_id = requester_id;
        t.platform = "YouTube";

        t.stream_url = ExtractDirectAudioURL(t.url);
        return t;
    } catch (const std::exception& e) {
        Logger::Error("YOUTUBE", std::string("JSON parsing error: ") + e.what());
        return std::nullopt;
    }
}

std::string PlatformYouTube::ExtractDirectAudioURL(const std::string& video_url) {
    std::string cleanUrl = SanitizeInput(video_url);
    std::string cmd = "yt-dlp -g -f bestaudio/best \"" + cleanUrl + "\" 2>/dev/null";
    std::string out = ExecCommand(cmd);
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
        out.pop_back();
    }
    return out;
}
