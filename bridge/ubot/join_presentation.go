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
	"slices"

	tg "github.com/amarnathcjd/gogram/telegram"

	"main/ntgcalls"
)

func (ctx *Context) joinPresentation(chatId int64, join bool) error {
	defer func() {
		ctx.waitConnectMutex.Lock()
		delete(ctx.waitConnect, chatId)
		ctx.waitConnectMutex.Unlock()
	}()

	connectionMode, err := ctx.binding.GetConnectionMode(chatId)
	if err != nil {
		return err
	}

	if connectionMode == ntgcalls.StreamConnection {
		ctx.pendingConnectionsMutex.Lock()
		if ctx.pendingConnections[chatId] != nil {
			ctx.pendingConnections[chatId].Presentation = join
		}
		ctx.pendingConnectionsMutex.Unlock()
		return nil
	}

	if connectionMode != ntgcalls.RtcConnection {
		return nil
	}

	if join {
		ctx.presentationsMutex.Lock()
		alreadyPresenting := slices.Contains(ctx.presentations, chatId)
		ctx.presentationsMutex.Unlock()

		if alreadyPresenting {
			return nil
		}

		ctx.waitConnectMutex.Lock()
		waitChan := make(chan error)
		ctx.waitConnect[chatId] = waitChan
		ctx.waitConnectMutex.Unlock()

		jsonParams, err := ctx.binding.InitPresentation(chatId)
		if err != nil {
			return err
		}

		ctx.inputGroupCallsMutex.RLock()
		inputGroupCall := ctx.inputGroupCalls[chatId]
		ctx.inputGroupCallsMutex.RUnlock()

		resultParams := "{\"transport\": null}"
		callResRaw, err := ctx.app.PhoneJoinGroupCallPresentation(
			inputGroupCall,
			&tg.DataJson{
				Data: jsonParams,
			},
		)
		if err != nil {
			return err
		}

		callRes := callResRaw.(*tg.UpdatesObj)
		for _, u := range callRes.Updates {
			switch update := u.(type) {
			case *tg.UpdateGroupCallConnection:
				resultParams = update.Params.Data
			}
		}

		err = ctx.binding.Connect(chatId, resultParams, true)
		if err != nil {
			return err
		}

		<-waitChan

		ctx.presentationsMutex.Lock()
		ctx.presentations = append(ctx.presentations, chatId)
		ctx.presentationsMutex.Unlock()

	} else {
		// Leave presentation
		ctx.presentationsMutex.Lock()
		isPresenting := slices.Contains(ctx.presentations, chatId)
		if isPresenting {
			ctx.presentations = stdRemove(ctx.presentations, chatId)
		}
		ctx.presentationsMutex.Unlock()

		if !isPresenting {
			return nil
		}

		err = ctx.binding.StopPresentation(chatId)
		if err != nil {
			return err
		}

		ctx.inputGroupCallsMutex.RLock()
		inputGroupCall := ctx.inputGroupCalls[chatId]
		ctx.inputGroupCallsMutex.RUnlock()

		_, err = ctx.app.PhoneLeaveGroupCallPresentation(inputGroupCall)
		if err != nil {
			return err
		}
	}

	return nil
}
