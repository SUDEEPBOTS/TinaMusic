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
