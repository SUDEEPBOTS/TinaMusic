#!/bin/bash
cd /root/YukkiMusic-Cpp

pkill -f "assistant_bridge.py" 2>/dev/null
pkill -f "yukki-music-bot" 2>/dev/null
sleep 1

python3 assistant_bridge.py > /root/YukkiMusic-Cpp/assistant.log 2>&1 &
ASSISTANT_PID=$!
echo "⚡ Assistant Voice Bridge started [PID: $ASSISTANT_PID]"

sleep 2

trap "kill $ASSISTANT_PID 2>/dev/null" EXIT

./bin/yukki-music-bot
