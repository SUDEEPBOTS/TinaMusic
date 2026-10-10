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
 * Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 * Support: https://t.me/Z0iiw
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

package ubot

import (
	"maps"
	"slices"
	"time"

	tg "github.com/amarnathcjd/gogram/telegram"
)

func (ctx *Context) GetParticipants(
	chatId int64,
) ([]*tg.GroupCallParticipant, error) {
	ctx.participantsMutex.Lock()
	defer ctx.participantsMutex.Unlock()
	if ctx.callParticipants[chatId] == nil {
		ctx.callParticipants[chatId] = &CallParticipantsCache{
			CallParticipants: make(map[int64]*tg.GroupCallParticipant),
		}
	}
	if time.Since(
		ctx.callParticipants[chatId].LastMtprotoUpdate,
	) > time.Minute {
		groupCall, err := ctx.GetInputGroupCall(chatId)
		if err != nil {
			return nil, err
		}
		ctx.callParticipants[chatId].CallParticipants = make(
			map[int64]*tg.GroupCallParticipant,
		)
		var nextOffset string
		for {
			res, err := ctx.app.PhoneGetGroupParticipants(
				groupCall,
				[]tg.InputPeer{},
				[]int32{},
				nextOffset,
				0,
			)
			if err != nil {
				return nil, err
			}
			for _, participant := range res.Participants {
				ctx.callParticipants[chatId].CallParticipants[getParticipantId(participant.Peer)] = participant
			}
			if res.NextOffset == "" {
				break
			}
			nextOffset = res.NextOffset
		}
		ctx.callParticipants[chatId].LastMtprotoUpdate = time.Now()
	}
	return slices.Collect(
		maps.Values(ctx.callParticipants[chatId].CallParticipants),
	), nil
}
