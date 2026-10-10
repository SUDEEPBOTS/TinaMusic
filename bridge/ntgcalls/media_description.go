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

package ntgcalls

//#include "ntgcalls.h"
//#include <stdlib.h>
import "C"
import "unsafe"

type MediaDescription struct {
	Microphone *AudioDescription
	Speaker    *AudioDescription
	Camera     *VideoDescription
	Screen     *VideoDescription
}

func (ctx *MediaDescription) ParseToC() C.ntg_media_description_struct {
	var x C.ntg_media_description_struct

	if ctx.Microphone != nil {
		cMic := C.malloc(
			C.size_t(unsafe.Sizeof(C.ntg_audio_description_struct{})),
		)
		micStruct := ctx.Microphone.ParseToC()
		*(*C.ntg_audio_description_struct)(cMic) = micStruct
		x.microphone = (*C.ntg_audio_description_struct)(cMic)
	}

	if ctx.Speaker != nil {
		cSpeaker := C.malloc(
			C.size_t(unsafe.Sizeof(C.ntg_audio_description_struct{})),
		)
		speakerStruct := ctx.Speaker.ParseToC()
		*(*C.ntg_audio_description_struct)(cSpeaker) = speakerStruct
		x.speaker = (*C.ntg_audio_description_struct)(cSpeaker)
	}

	if ctx.Camera != nil {
		cCamera := C.malloc(
			C.size_t(unsafe.Sizeof(C.ntg_video_description_struct{})),
		)
		cameraStruct := ctx.Camera.ParseToC()
		*(*C.ntg_video_description_struct)(cCamera) = cameraStruct
		x.camera = (*C.ntg_video_description_struct)(cCamera)
	}

	if ctx.Screen != nil {
		cScreen := C.malloc(
			C.size_t(unsafe.Sizeof(C.ntg_video_description_struct{})),
		)
		screenStruct := ctx.Screen.ParseToC()
		*(*C.ntg_video_description_struct)(cScreen) = screenStruct
		x.screen = (*C.ntg_video_description_struct)(cScreen)
	}

	return x
}

func freeMediaDescriptionC(desc C.ntg_media_description_struct) {
	if desc.microphone != nil {
		C.free(unsafe.Pointer(desc.microphone.input))
		C.free(unsafe.Pointer(desc.microphone))
	}
	if desc.speaker != nil {
		C.free(unsafe.Pointer(desc.speaker.input))
		C.free(unsafe.Pointer(desc.speaker))
	}
	if desc.camera != nil {
		C.free(unsafe.Pointer(desc.camera.input))
		C.free(unsafe.Pointer(desc.camera))
	}
	if desc.screen != nil {
		C.free(unsafe.Pointer(desc.screen.input))
		C.free(unsafe.Pointer(desc.screen))
	}
}
