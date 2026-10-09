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
