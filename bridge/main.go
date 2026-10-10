/*
 * ============================================================================
 * TinaMusic - Telegram's First Native C++20 Music Engine
 * Copyright (C) 2026 TheTeamVivek / TinaMusic Contributors
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
 * ============================================================================
 */

package main

/*
#cgo CFLAGS: -I.
#cgo linux LDFLAGS: -L. -lntgcalls -lm -lz
#cgo darwin LDFLAGS: -L. -lntgcalls -lc++ -lz -lbz2 -liconv
#include "ntgcalls/ntgcalls.h"
#include "glibc_compatibility.h"
*/
import "C"

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/Laky-64/gologging"
	"github.com/amarnathcjd/gogram/telegram"
	"github.com/joho/godotenv"
	"main/ntgcalls"
	"main/ubot"
)

type Assistant struct {
	Client *telegram.Client
	Self   *telegram.UserObj
	Ntg    *ubot.Context
}

var (
	ass         *Assistant
	activeMu    sync.RWMutex
	activeCalls = make(map[int64]bool)
)

func getMediaDescription(url string) ntgcalls.MediaDescription {
	baseCmd := "ffmpeg "
	if strings.HasPrefix(url, "http://") || strings.HasPrefix(url, "https://") {
		baseCmd += "-re -reconnect 1 -reconnect_streamed 1 -reconnect_delay_max 5 "
	}
	baseCmd += "-v warning -i \"" + url + "\" -f s16le -ac 2 -ar 96000 pipe:1"
	return ntgcalls.MediaDescription{
		Microphone: &ntgcalls.AudioDescription{
			MediaSource:  ntgcalls.MediaSourceShell,
			Input:        baseCmd,
			SampleRate:   96000,
			ChannelCount: 2,
		},
	}
}

func handlePlay(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	var req struct {
		ChatID int64  `json:"chat_id"`
		URL    string `json:"url"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		json.NewEncoder(w).Encode(map[string]any{"ok": false, "error": "BAD_REQUEST", "detail": err.Error()})
		return
	}
	if req.ChatID == 0 || req.URL == "" {
		json.NewEncoder(w).Encode(map[string]any{"ok": false, "error": "MISSING_PARAMS", "detail": "chat_id and url required"})
		return
	}

	desc := getMediaDescription(req.URL)
	err := ass.Ntg.Play(req.ChatID, desc)
	if err != nil {
		gologging.ErrorF("[AUDIO] Play failed for %d: %v", req.ChatID, err)
		errStr := err.Error()
		errCode := "PLAY_FAILED"
		if strings.Contains(strings.ToLower(errStr), "group call") || strings.Contains(errStr, "GROUPCALL_INVALID") {
			errCode = "NO_ACTIVE_GROUP_CALL"
		}
		json.NewEncoder(w).Encode(map[string]any{"ok": false, "error": errCode, "detail": errStr})
		return
	}

	activeMu.Lock()
	activeCalls[req.ChatID] = true
	activeMu.Unlock()

	gologging.InfoF("[AUDIO] Native playback started for chat %d", req.ChatID)
	json.NewEncoder(w).Encode(map[string]any{"ok": true})
}

func handlePause(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	var req struct {
		ChatID int64 `json:"chat_id"`
	}
	_ = json.NewDecoder(r.Body).Decode(&req)
	if req.ChatID != 0 {
		_, _ = ass.Ntg.Pause(req.ChatID)
	}
	json.NewEncoder(w).Encode(map[string]any{"ok": true})
}

func handleResume(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	var req struct {
		ChatID int64 `json:"chat_id"`
	}
	_ = json.NewDecoder(r.Body).Decode(&req)
	if req.ChatID != 0 {
		_, _ = ass.Ntg.Resume(req.ChatID)
	}
	json.NewEncoder(w).Encode(map[string]any{"ok": true})
}

func handleLeave(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	var req struct {
		ChatID int64 `json:"chat_id"`
	}
	_ = json.NewDecoder(r.Body).Decode(&req)
	if req.ChatID != 0 {
		activeMu.Lock()
		delete(activeCalls, req.ChatID)
		activeMu.Unlock()
		_ = ass.Ntg.Stop(req.ChatID)
	}
	json.NewEncoder(w).Encode(map[string]any{"ok": true})
}

func handleVolume(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(map[string]any{"ok": true})
}

func handleStatus(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	activeMu.RLock()
	chats := make([]int64, 0, len(activeCalls))
	for cid := range activeCalls {
		chats = append(chats, cid)
	}
	activeMu.RUnlock()

	json.NewEncoder(w).Encode(map[string]any{
		"ok":                 true,
		"assistant_id":       ass.Self.ID,
		"assistant_name":     ass.Self.FirstName,
		"assistant_username": ass.Self.Username,
		"active_calls":       chats,
	})
}

func initAssistant(appID int32, appHash, rawSession, sessionType string) (*Assistant, error) {
	stringSession, err := ResolveSession(rawSession, sessionType)
	if err != nil {
		return nil, fmt.Errorf("resolving session: %w", err)
	}

	client, err := telegram.NewClient(telegram.ClientConfig{
		AppID:         appID,
		AppHash:       appHash,
		LogLevel:      telegram.LogError,
		ParseMode:     "HTML",
		StringSession: stringSession,
		Session:       "ass_0.session",
	})
	if err != nil {
		return nil, fmt.Errorf("creating client: %w", err)
	}

	user, err := client.GetMe()
	if err != nil {
		return nil, fmt.Errorf("fetching identity: %w", err)
	}

	client.SetCommandPrefixes(".")

	return &Assistant{
		Client: client,
		Self:   user,
		Ntg:    ubot.NewContext(client),
	}, nil
}

func main() {
	_ = godotenv.Load(".env")
	_ = godotenv.Load("../.env")

	apiID, _ := strconv.Atoi(os.Getenv("API_ID"))
	apiHash := os.Getenv("API_HASH")
	sessionStr := os.Getenv("STRING_SESSIONS")
	if sessionStr == "" {
		sessionStr = os.Getenv("STRING_SESSION")
	}
	sessionType := os.Getenv("SESSION_TYPE")
	if sessionType == "" {
		sessionType = "pyrogram"
	}

	if sessionStr == "" || apiID == 0 || apiHash == "" {
		gologging.Fatal("[SYSTEM] [ERROR] Missing API_ID, API_HASH, or STRING_SESSIONS in .env")
	}

	gologging.Info("[SYSTEM] [STARTUP] Starting native voice bridge engine...")
	var err error
	ass, err = initAssistant(int32(apiID), apiHash, sessionStr, sessionType)
	if err != nil {
		gologging.FatalF("[SYSTEM] [ERROR] Failed to initialize assistant: %v", err)
	}

	gologging.InfoF("[SYSTEM] [STARTUP] Assistant identity verified: @%s (ID: %d)", ass.Self.Username, ass.Self.ID)

	ass.Ntg.OnStreamEnd(func(chatID int64, streamType ntgcalls.StreamType, _ ntgcalls.StreamDevice) {
		if streamType == ntgcalls.VideoStream {
			return
		}
		activeMu.Lock()
		delete(activeCalls, chatID)
		activeMu.Unlock()
		gologging.InfoF("[AUDIO] Stream ended for chat %d, notifying C++ bot...", chatID)

		client := &http.Client{Timeout: 2 * time.Second}
		payload, _ := json.Marshal(map[string]any{"chat_id": chatID})
		_, _ = client.Post("http://127.0.0.1:8056/stream_end", "application/json", bytes.NewReader(payload))
	})

	http.HandleFunc("/play", handlePlay)
	http.HandleFunc("/pause", handlePause)
	http.HandleFunc("/resume", handleResume)
	http.HandleFunc("/leave", handleLeave)
	http.HandleFunc("/volume", handleVolume)
	http.HandleFunc("/status", handleStatus)

	gologging.Info("[SYSTEM] [STARTUP] Voice bridge IPC listening on http://127.0.0.1:8055")
	if err := http.ListenAndServe("127.0.0.1:8055", nil); err != nil {
		gologging.FatalF("[SYSTEM] [ERROR] Server error: %v", err)
	}
}
