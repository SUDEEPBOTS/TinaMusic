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

package ubot

import (
	"time"

	tg "github.com/amarnathcjd/gogram/telegram"

	"main/ntgcalls"
)

type (
	CallParticipantsCache struct {
		CallParticipants  map[int64]*tg.GroupCallParticipant
		LastMtprotoUpdate time.Time
	}

	CallSources struct {
		CameraSources, ScreenSources map[int64]string
	}

	P2PConfig struct {
		DhConfig       *tg.MessagesDhConfigObj
		PhoneCall      *tg.PhoneCallObj
		IsOutgoing     bool
		KeyFingerprint int64
		GAorB          []byte
		WaitData       chan error
	}

	PendingConnection struct {
		MediaDescription ntgcalls.MediaDescription
		Payload          string
		Presentation     bool
	}
)
