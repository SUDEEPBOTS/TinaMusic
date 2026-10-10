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
	tg "github.com/amarnathcjd/gogram/telegram"

	"main/ntgcalls"
)

func parseRTCServers(connections []tg.PhoneConnection) []ntgcalls.RTCServer {
	rtcServers := make([]ntgcalls.RTCServer, len(connections))
	for i, c := range connections {
		switch connection := c.(type) {
		case *tg.PhoneConnectionWebrtc:
			rtcServers[i] = ntgcalls.RTCServer{
				ID:       connection.ID,
				Ipv4:     connection.Ip,
				Ipv6:     connection.Ipv6,
				Username: connection.Username,
				Password: connection.Password,
				Port:     connection.Port,
				Turn:     connection.Turn,
				Stun:     connection.Stun,
			}
		case *tg.PhoneConnectionObj:
			rtcServers[i] = ntgcalls.RTCServer{
				ID:      connection.ID,
				Ipv4:    connection.Ip,
				Ipv6:    connection.Ipv6,
				Port:    connection.Port,
				Turn:    true,
				Tcp:     connection.Tcp,
				PeerTag: connection.PeerTag,
			}
		}
	}
	return rtcServers
}
