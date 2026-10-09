import asyncio
import os
import sys
from aiohttp import web
from pyrogram import Client
from pytgcalls import PyTgCalls
from pytgcalls.types import MediaStream, AudioQuality, StreamEnded

def load_env(path="/root/YukkiMusic-Cpp/.env"):
    env = {}
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "=" in line:
                    k, v = line.split("=", 1)
                    v = v.strip().strip('"').strip("'")
                    env[k.strip()] = v
    return env

env = load_env()
api_id = int(env.get("API_ID", os.getenv("API_ID", "0")))
api_hash = env.get("API_HASH", os.getenv("API_HASH", ""))
session_str = env.get("STRING_SESSIONS", env.get("STRING_SESSION", os.getenv("STRING_SESSIONS", "")))
logger_id = int(env.get("LOGGER_ID", os.getenv("LOGGER_ID", "0")))

if not session_str or not api_id or not api_hash:
    print("[ASSISTANT ERROR] Missing API_ID, API_HASH, or STRING_SESSIONS in .env")
    sys.exit(1)

app = Client("yukki_assistant", api_id=api_id, api_hash=api_hash, session_string=session_str, in_memory=True)
pytgcalls = PyTgCalls(app)
active_calls = set()

@pytgcalls.on_update()
async def on_stream_end(client, update):
    if isinstance(update, StreamEnded):
        chat_id = update.chat_id
        if chat_id in active_calls:
            active_calls.discard(chat_id)
        try:
            import aiohttp
            async with aiohttp.ClientSession() as session:
                await session.post("http://127.0.0.1:8056/stream_end", json={"chat_id": chat_id}, timeout=3)
        except Exception:
            pass

async def handle_play(request):
    try:
        data = await request.json()
        chat_id = int(data.get("chat_id"))
        url = data.get("url")
        if not chat_id or not url:
            return web.json_response({"ok": False, "error": "Missing chat_id or url"}, status=400)

        try:
            me = await app.get_me()
            try:
                await app.get_chat_member(chat_id, me.id)
            except Exception:
                try:
                    chat = await app.get_chat(chat_id)
                    if chat.username:
                        await app.join_chat(chat.username)
                    elif getattr(chat, "invite_link", None):
                        await app.join_chat(chat.invite_link)
                except Exception:
                    pass
        except Exception:
            pass

        stream = MediaStream(url, audio_parameters=AudioQuality.HIGH)
        await pytgcalls.play(chat_id, stream)
        active_calls.add(chat_id)
        print(f"[ASSISTANT] Playing in VC for chat {chat_id}: {url}")
        return web.json_response({"ok": True})
    except Exception as e:
        print(f"[ASSISTANT ERROR] Play failed for {data.get('chat_id')}: {e}")
        return web.json_response({"ok": False, "error": str(e)}, status=500)

async def handle_pause(request):
    try:
        data = await request.json()
        chat_id = int(data.get("chat_id"))
        await pytgcalls.pause(chat_id)
        return web.json_response({"ok": True})
    except Exception as e:
        return web.json_response({"ok": False, "error": str(e)}, status=500)

async def handle_resume(request):
    try:
        data = await request.json()
        chat_id = int(data.get("chat_id"))
        await pytgcalls.resume(chat_id)
        return web.json_response({"ok": True})
    except Exception as e:
        return web.json_response({"ok": False, "error": str(e)}, status=500)

async def handle_leave(request):
    try:
        data = await request.json()
        chat_id = int(data.get("chat_id"))
        await pytgcalls.leave_call(chat_id)
        active_calls.discard(chat_id)
        return web.json_response({"ok": True})
    except Exception as e:
        return web.json_response({"ok": False, "error": str(e)}, status=500)

async def handle_volume(request):
    try:
        data = await request.json()
        chat_id = int(data.get("chat_id"))
        vol = int(data.get("volume", 100))
        await pytgcalls.change_volume_call(chat_id, vol)
        return web.json_response({"ok": True})
    except Exception as e:
        return web.json_response({"ok": False, "error": str(e)}, status=500)

async def handle_status(request):
    me = await app.get_me()
    return web.json_response({
        "ok": True,
        "assistant_id": me.id,
        "assistant_name": me.first_name,
        "assistant_username": me.username,
        "active_calls": list(active_calls)
    })

async def main():
    print("[ASSISTANT] Starting Pyrogram and PyTgCalls MTProto engine...")
    await pytgcalls.start()
    me = await app.get_me()
    print(f"[ASSISTANT] Logged in as: {me.first_name} (@{me.username}) [ID: {me.id}]")

    if logger_id != 0:
        try:
            await app.send_message(logger_id, f"⚡ <b>Yukki Assistant Online</b>: @{me.username} (ID: {me.id})\n<i>Voice chat engine ready!</i>")
        except Exception:
            pass

    server = web.Application()
    server.router.add_post("/play", handle_play)
    server.router.add_post("/pause", handle_pause)
    server.router.add_post("/resume", handle_resume)
    server.router.add_post("/leave", handle_leave)
    server.router.add_post("/volume", handle_volume)
    server.router.add_get("/status", handle_status)

    runner = web.AppRunner(server)
    await runner.setup()
    site = web.TCPSite(runner, "127.0.0.1", 8055)
    await site.start()
    print("[ASSISTANT] IPC API listening on http://127.0.0.1:8055")

    while True:
        await asyncio.sleep(3600)

if __name__ == "__main__":
    asyncio.run(main())
