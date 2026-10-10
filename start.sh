#!/bin/bash
# ============================================================================
#   ████████╗██╗███╗   ██╗ █████╗ ███╗   ███╗██╗   ██╗███████╗██╗ ██████╗
#   ╚══██╔══╝██║████╗  ██║██╔══██╗████╗ ████║██║   ██║██╔════╝██║██╔════╝
#      ██║   ██║██╔██╗ ██║███████║██╔████╔██║██║   ██║███████╗██║██║     
#      ██║   ██║██║╚██╗██║██╔══██║██║╚██╔╝██║██║   ██║╚════██║██║██║     
#      ██║   ██║██║ ╚████║██║  ██║██║ ╚═╝ ██║╚██████╔╝███████║██║╚██████╗
#      ╚═╝   ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝
# ============================================================================
#
# Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
# Architecture: Native C++20 Core + Go MTProto WebRTC Voice Bridge
# Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
# Support: https://t.me/Z0iiw
#
# ----------------------------------------------------------------------------
# LEGAL NOTICE & LICENSE (GNU Affero General Public License v3.0):
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# STRICT COPYLEFT CLAUSE:
# In accordance with Section 13 of the GNU AGPLv3, if you modify this program
# or run it as a network service (e.g., hosted on servers, Telegram, or cloud),
# you MUST make the complete source code of your modified version available
# to all users under the exact same AGPLv3 license terms.
# Closed-source derivatives or rebranded copies are strictly prohibited.
# ============================================================================

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

pkill -9 -f "assistant-bridge" 2>/dev/null || true
pkill -9 -f "tina-music-bot" 2>/dev/null || true
pkill -9 -f "yukki-music-bot" 2>/dev/null || true
pkill -9 -f "tinamusic" 2>/dev/null || true
sleep 1

if [ ! -f "bin/assistant-bridge" ] || [ ! -f "bin/tina-music-bot" ]; then
    echo "[SYSTEM] [STARTUP] Binaries not found, compiling..."
    make -j1
fi

./bin/assistant-bridge > bridge.log 2>&1 &
BRIDGE_PID=$!
echo "[SYSTEM] [STARTUP] Native voice bridge started [PID: $BRIDGE_PID]"

echo "[SYSTEM] [STARTUP] Waiting for voice bridge initialization..."
for i in {1..20}; do
    RES=$(curl -s http://127.0.0.1:8055/status 2>/dev/null || true)
    if [[ "$RES" == *"\"ok\":true"* || "$RES" == *"\"ok\": true"* ]]; then
        echo "[SYSTEM] [STARTUP] Native voice bridge verified and operational."
        break
    fi
    sleep 1
done

trap "kill -9 $BRIDGE_PID 2>/dev/null || true" EXIT

exec ./bin/tina-music-bot
