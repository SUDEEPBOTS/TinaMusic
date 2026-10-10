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

// #include "ntgcalls.h"
// extern void unlockMutex(void*);
import "C"

import (
	"sync"
	"unsafe"
)

type Future struct {
	mutex      *sync.Mutex
	errCode    *C.int
	errMessage **C.char
}

func CreateFuture() *Future {
	res := &Future{
		mutex:      &sync.Mutex{},
		errCode:    new(C.int),
		errMessage: new(*C.char),
	}
	res.mutex.Lock()
	return res
}

func (ctx *Future) ParseToC() C.ntg_async_struct {
	var x C.ntg_async_struct
	x.userData = unsafe.Pointer(ctx.mutex)
	x.promise = (C.ntg_async_callback)(unsafe.Pointer(C.unlockMutex))
	x.errorCode = (*C.int)(unsafe.Pointer(ctx.errCode))
	x.errorMessage = ctx.errMessage
	return x
}

func (ctx *Future) wait() {
	ctx.mutex.Lock()
}

//export unlockMutex
func unlockMutex(p unsafe.Pointer) {
	m := (*sync.Mutex)(p)
	m.Unlock()
}
