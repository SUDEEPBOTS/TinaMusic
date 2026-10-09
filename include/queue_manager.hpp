#pragma once

#include "models.hpp"
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <optional>
#include <vector>
#include <memory>

struct ChatQueueData {
    std::deque<Track> pending_tracks;
    std::optional<Track> active_track;
    PlaybackStatus status{PlaybackStatus::IDLE};
    bool loop{false};
    std::vector<Track> history;
    std::vector<Track> last_cleared;
    std::unordered_set<std::string> played_ids;
    std::mutex queue_mutex;
};

class QueueManager {
public:
    static QueueManager& Instance();

    void Enqueue(int64_t chat_id, const Track& track);
    void EnqueueForce(int64_t chat_id, const Track& track);
    std::optional<Track> Next(int64_t chat_id);
    std::optional<Track> GetCurrent(int64_t chat_id);
    std::vector<Track> GetPending(int64_t chat_id);
    void Clear(int64_t chat_id);
    std::vector<Track> ClearPending(int64_t chat_id);
    bool RestoreQueue(int64_t chat_id);
    bool RemoveAt(int64_t chat_id, size_t index);
    bool MoveTrack(int64_t chat_id, size_t from_index, size_t to_index);
    void Shuffle(int64_t chat_id);
    void SetLoop(int64_t chat_id, bool enable);
    bool IsLooping(int64_t chat_id);
    void SetStatus(int64_t chat_id, PlaybackStatus status);
    PlaybackStatus GetStatus(int64_t chat_id);
    size_t QueueSize(int64_t chat_id);
    size_t PendingSize(int64_t chat_id);
    bool HasNext(int64_t chat_id);

    void MarkPlayed(int64_t chat_id, const std::string& track_id);
    bool HasPlayed(int64_t chat_id, const std::string& track_id);
    std::unordered_set<std::string> GetPlayedHistory(int64_t chat_id);
    void ClearPlayedHistory(int64_t chat_id);

private:
    QueueManager() = default;
    ~QueueManager() = default;
    QueueManager(const QueueManager&) = delete;
    QueueManager& operator=(const QueueManager&) = delete;

    ChatQueueData& GetOrCreateQueue(int64_t chat_id);

    std::unordered_map<int64_t, std::shared_ptr<ChatQueueData>> queues_;
    std::mutex registry_mutex_;
};
