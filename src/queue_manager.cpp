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
        auto q = std::make_shared<ChatQueueData>();
        queues_[chat_id] = q;
        return *q;
    }
    return *(it->second);
}

void QueueManager::Enqueue(int64_t chat_id, const Track& track) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);

    if (!q.active_track.has_value() && q.pending_tracks.empty()) {
        q.active_track = track;
        q.played_ids.insert(track.id);
        q.status = PlaybackStatus::PLAYING;
    } else {
        q.pending_tracks.push_back(track);
    }
}

void QueueManager::EnqueueForce(int64_t chat_id, const Track& track) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);

    if (q.active_track.has_value()) {
        q.pending_tracks.push_front(*q.active_track);
    }
    q.active_track = track;
    q.played_ids.insert(track.id);
    q.status = PlaybackStatus::PLAYING;
}

std::optional<Track> QueueManager::Next(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);

    if (q.active_track.has_value()) {
        q.history.push_back(*q.active_track);
        if (q.loop) {
            q.pending_tracks.push_back(*q.active_track);
        }
    }

    if (q.pending_tracks.empty()) {
        q.active_track = std::nullopt;
        q.status = PlaybackStatus::IDLE;
        return std::nullopt;
    }

    Track next_track = q.pending_tracks.front();
    q.pending_tracks.pop_front();
    q.active_track = next_track;
    q.played_ids.insert(next_track.id);
    q.status = PlaybackStatus::PLAYING;
    return next_track;
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
    q.active_track = std::nullopt;
    q.status = PlaybackStatus::IDLE;
}

std::vector<Track> QueueManager::ClearPending(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    std::vector<Track> cleared(q.pending_tracks.begin(), q.pending_tracks.end());
    q.last_cleared = cleared;
    q.pending_tracks.clear();
    return cleared;
}

bool QueueManager::RestoreQueue(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    if (q.last_cleared.empty()) return false;
    for (const auto& t : q.last_cleared) {
        q.pending_tracks.push_back(t);
    }
    q.last_cleared.clear();
    return true;
}

bool QueueManager::RemoveAt(int64_t chat_id, size_t index) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    if (index >= q.pending_tracks.size()) return false;
    q.pending_tracks.erase(q.pending_tracks.begin() + index);
    return true;
}

bool QueueManager::MoveTrack(int64_t chat_id, size_t from_index, size_t to_index) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    if (from_index >= q.pending_tracks.size() || to_index >= q.pending_tracks.size()) return false;
    Track t = q.pending_tracks[from_index];
    q.pending_tracks.erase(q.pending_tracks.begin() + from_index);
    q.pending_tracks.insert(q.pending_tracks.begin() + to_index, t);
    return true;
}

void QueueManager::Shuffle(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    if (q.pending_tracks.size() > 1) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(q.pending_tracks.begin(), q.pending_tracks.end(), g);
    }
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

size_t QueueManager::PendingSize(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.pending_tracks.size();
}

bool QueueManager::HasNext(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return !q.pending_tracks.empty();
}

void QueueManager::MarkPlayed(int64_t chat_id, const std::string& track_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    q.played_ids.insert(track_id);
}

bool QueueManager::HasPlayed(int64_t chat_id, const std::string& track_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.played_ids.find(track_id) != q.played_ids.end();
}

std::unordered_set<std::string> QueueManager::GetPlayedHistory(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    return q.played_ids;
}

void QueueManager::ClearPlayedHistory(int64_t chat_id) {
    auto& q = GetOrCreateQueue(chat_id);
    std::lock_guard<std::mutex> lock(q.queue_mutex);
    q.played_ids.clear();
}
