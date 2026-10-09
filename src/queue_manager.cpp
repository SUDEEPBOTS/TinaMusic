#include "queue_manager.hpp"
#include <random>
#include <algorithm>

QueueManager& QueueManager::Instance() {
    static QueueManager instance;
    return instance;
}

ChatQueueData& QueueManager::GetOrCreateQueue(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(registry_mutex_);
    auto it = queues_.find(chat_id);
    if (it == queues_.end()) {
        auto data = std::make_shared<ChatQueueData>();
        queues_[chat_id] = data;
        return *data;
    }
    return *(it->second);
}

void QueueManager::Enqueue(int64_t chat_id, const Track& track) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    if (!q.active_track.has_value() && q.pending_tracks.empty()) {
        q.active_track = track;
        q.status = PlaybackStatus::PLAYING;
    } else {
        q.pending_tracks.push_back(track);
    }
}

std::optional<Track> QueueManager::Next(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);

    if (q.loop && q.active_track.has_value()) {
        return q.active_track;
    }

    if (q.active_track.has_value()) {
        q.history.push_back(q.active_track.value());
        q.active_track.reset();
    }

    if (!q.pending_tracks.empty()) {
        q.active_track = q.pending_tracks.front();
        q.pending_tracks.pop_front();
        q.status = PlaybackStatus::PLAYING;
        return q.active_track;
    }

    q.status = PlaybackStatus::IDLE;
    return std::nullopt;
}

std::optional<Track> QueueManager::GetCurrent(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.active_track;
}

std::vector<Track> QueueManager::GetPending(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return std::vector<Track>(q.pending_tracks.begin(), q.pending_tracks.end());
}

void QueueManager::Clear(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    q.pending_tracks.clear();
    q.active_track.reset();
    q.status = PlaybackStatus::IDLE;
}

void QueueManager::Shuffle(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(q.pending_tracks.begin(), q.pending_tracks.end(), g);
}

void QueueManager::SetLoop(int64_t chat_id, bool enable) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    q.loop = enable;
}

bool QueueManager::IsLooping(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.loop;
}

void QueueManager::SetStatus(int64_t chat_id, PlaybackStatus status) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    q.status = status;
}

PlaybackStatus QueueManager::GetStatus(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.status;
}

size_t QueueManager::QueueSize(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.pending_tracks.size() + (q.active_track.has_value() ? 1 : 0);
}

bool QueueManager::HasNext(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return !q.pending_tracks.empty() || q.loop;
}
