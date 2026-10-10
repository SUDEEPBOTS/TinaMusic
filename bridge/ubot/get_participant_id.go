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

import tg "github.com/amarnathcjd/gogram/telegram"

func getParticipantId(peer tg.Peer) int64 {
	var participantId int64
	switch chatObj := peer.(type) {
	case *tg.PeerUser:
		participantId = chatObj.UserID
	case *tg.PeerChannel:
		participantId = chatObj.ChannelID
	case *tg.PeerChat:
		participantId = chatObj.ChatID
	}
	return participantId
}
