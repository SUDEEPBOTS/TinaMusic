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

package ntgcalls

//#include "ntgcalls.h"
//#include <stdlib.h>
import "C"

type AudioDescription struct {
	MediaSource  MediaSource
	Input        string
	SampleRate   uint32
	ChannelCount uint8
	KeepOpen     bool
}

func (ctx *AudioDescription) ParseToC() C.ntg_audio_description_struct {
	var x C.ntg_audio_description_struct
	x.mediaSource = ctx.MediaSource.ParseToC()
	x.input = C.CString(ctx.Input)
	x.sampleRate = C.uint32_t(ctx.SampleRate)
	x.channelCount = C.uint8_t(ctx.ChannelCount)
	x.keepOpen = C.bool(ctx.KeepOpen)
	return x
}
