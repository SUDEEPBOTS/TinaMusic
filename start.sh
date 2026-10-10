#!/bin/bash
cd /root/YukkiMusic-Cpp

pkill -9 -f "assistant-bridge" 2>/dev/null
pkill -9 -f "assistant_bridge.py" 2>/dev/null
pkill -9 -f "yukki-music-bot" 2>/dev/null
pkill -9 -f "yukkimusic" 2>/dev/null
sleep 1

./bin/assistant-bridge > /root/YukkiMusic-Cpp/bridge.log 2>&1 &
BRIDGE_PID=$!
echo "⚡ Native Gogram Voice Bridge started [PID: $BRIDGE_PID]"

echo "⏳ Waiting for Voice Bridge to initialize..."
for i in {1..15}; do
    RES=$(curl -s http://127.0.0.1:8055/status 2>/dev/null)
    if [[ "$RES" == *"\"ok\":true"* || "$RES" == *"\"ok\": true"* ]]; then
        echo "✅ Native Voice Bridge is ready!"
        break
    fi
    sleep 1
done

trap "kill -9 $BRIDGE_PID 2>/dev/null" EXIT

./bin/yukki-music-bot
