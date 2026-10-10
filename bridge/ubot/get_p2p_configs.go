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
)

func (ctx *Context) getP2PConfigs(GAorB []byte) (*P2PConfig, error) {
	dhConfigRaw, err := ctx.app.MessagesGetDhConfig(0, 256)
	if err != nil {
		return nil, err
	}
	dhConfig := dhConfigRaw.(*tg.MessagesDhConfigObj)
	return &P2PConfig{
		DhConfig:   dhConfig,
		IsOutgoing: GAorB == nil,
		GAorB:      GAorB,
		WaitData:   make(chan error),
	}, nil
}
