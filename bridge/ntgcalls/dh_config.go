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

import "unsafe"

type DhConfig struct {
	G      int32
	P      []byte
	Random []byte
}

func (ctx *DhConfig) ParseToC() C.ntg_dh_config_struct {
	var x C.ntg_dh_config_struct
	x.g = C.int32_t(ctx.G)
	pC, pSize := parseBytes(ctx.P)
	rC, rSize := parseBytes(ctx.Random)
	x.p = pC
	x.sizeP = pSize
	x.random = rC
	x.sizeRandom = rSize
	return x
}

func freeDhConfig(config *C.ntg_dh_config_struct) {
	if config.p != nil {
		C.free(unsafe.Pointer(config.p))
	}
	if config.random != nil {
		C.free(unsafe.Pointer(config.random))
	}
}
