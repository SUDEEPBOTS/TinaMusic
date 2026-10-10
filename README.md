<p align="center">
  <img src="assets/banner.png" alt="TinaMusic Banner" width="100%">
</p>

<h1 align="center">🌸 TinaMusic ⚡ Telegram's First Native C++20 Music Engine</h1>

<p align="center">
  <b>The pioneer, high-performance Telegram music bot crafted entirely in modern compiled C++20 with a native Go MTProto WebRTC voice bridge.</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/First_Ever-Telegram_C%2B%2B20_Music_Bot-FF69B4?style=for-the-badge&logo=cplusplus" alt="Telegram 1st C++ Music Bot">
  <img src="https://img.shields.io/badge/Language-C%2B%2B20-00599C?style=for-the-badge&logo=cplusplus" alt="C++20">
  <img src="https://img.shields.io/badge/Voice_Bridge-Go_1.22+-00ADD8?style=for-the-badge&logo=go" alt="Go 1.22+">
  <img src="https://img.shields.io/badge/License-GNU_AGPLv3-blue?style=for-the-badge" alt="AGPLv3">
  <img src="https://img.shields.io/badge/Cloud_Ready-Heroku%20%7C%20Render%20%7C%20Railway-brightgreen?style=for-the-badge" alt="Cloud Ready">
  <img src="https://img.shields.io/badge/Zero_Python-100%25_Native-success?style=for-the-badge" alt="Zero Python">
</p>

---

<p align="center">
  ⭐ <b>Enjoying TinaMusic? Please Star this repository!</b> ⭐<br>
  <i>Your stars motivate continued high-performance open-source development and help other developers discover Telegram's first native C++ audio engine.</i>
</p>

---

## ⚡ Why TinaMusic?

Traditional Telegram music bots written in Python (Pyrogram & PyTgCalls) suffer from heavy runtime bloat:
- **Massive RAM Consumption**: Python bots consume 300MB – 500MB of RAM, causing constant crashes on budget VPS instances and cloud free tiers.
- **Python Global Interpreter Lock (GIL)**: High CPU contention and stuttering audio during group voice chat playback.
- **Slow Startup**: Heavy dependency graphs take 20–45 seconds to spin up.

### 🚀 The TinaMusic Difference
**TinaMusic** is built from the ground up to redefine Telegram audio streaming:
- **Telegram's First Ever C++20 Music Bot**: 100% compiled native binary execution.
- **Ultra-Low Memory Footprint**: Runs rock-solid in **~30 MB RAM** — less than 1/10th the memory of Python bots!
- **Zero Python Runtime**: No Python, no Pyrogram, no PyTgCalls, no pip dependencies.
- **Instant Voice Chat Connection**: Joins and streams within **1–2 seconds**.
- **Mature & Clean Console Logs**: No emojis or cluttered outputs in console logs. Strictly structured `[TIMESTAMP] [LEVEL] [SCOPE] Message`.
- **Bulletproof Cloud Deployment**: Runs buttery-smooth on **Heroku**, **Render**, and **Railway** without crashing or hitting memory quotas!

---

## ☁️ Cloud Deployments: Heroku, Render & Railway

> [!NOTE]
> Because TinaMusic uses only **~30 MB of RAM**, it runs smoothly 24/7 on free/starter tiers of Heroku, Render, and Railway without encountering `R14 - Memory Quota Exceeded` errors!

### 💜 1. Deploy on Heroku

[![Deploy](https://www.herokucdn.com/deploy/button.svg)](https://heroku.com/deploy)

1. Fork or clone this repository to your GitHub.
2. Click the **Deploy to Heroku** button above or link your GitHub repo in the Heroku Dashboard.
3. Heroku automatically detects `heroku.yml` and builds the optimized multi-stage Docker container.
4. Fill in your environment variables (`API_ID`, `API_HASH`, `BOT_TOKEN`, `STRING_SESSIONS`, `OWNER_ID`).
5. Scale your worker dyno in the **Resources** tab:
   ```bash
   heroku ps:scale worker=1
   ```

### 💙 2. Deploy on Render

1. Create a free account on [Render](https://render.com/).
2. Create a **New Blueprint Instance** or a **New Background Worker**.
3. Select **Docker** runtime. Render will automatically read `render.yaml` or `Dockerfile`.
4. Add your `.env` variables under the Environment tab.
5. Click **Deploy**. Your bot will build and start streaming within minutes on Render's free tier!

### 🚂 3. Deploy on Railway

1. Open [Railway](https://railway.app/) and create a new project.
2. Select **Deploy from GitHub repo** and choose `TinaMusic`.
3. Railway automatically recognizes `railway.json` and the multi-stage `Dockerfile`.
4. Go to **Variables** and paste your environment keys from `sample.env`.
5. Railway deploys the container instantly with automated health restarts!

---

## 📐 Architecture & Dataflow

```mermaid
flowchart LR
    User["Telegram Group / User"] -->|Commands: /play, /pause, /skip| TGAPI["Telegram Bot API"]
    TGAPI -->|libcurl Long-Polling| Core["TinaMusic Core (C++20)"]
    
    subgraph CoreEngine ["TinaMusic C++20 Core"]
        Core --> Queue["Thread-Safe Queue Manager"]
        Core --> Resolver["Yuki CDN & InnerTube Resolver"]
        Core --> Locales["Dynamic YAML Localization"]
    end
    
    Resolver -->|Direct Audio Stream URL| Core
    Core -->|Loopback IPC: /play (Port 8055)| Bridge["Native Voice Bridge (Go 1.22+)"]
    
    subgraph VoiceBridge ["Assistant Voice Bridge"]
        Bridge --> Gogram["Gogram MTProto Client"]
        Bridge --> NTgCalls["libntgcalls WebRTC Cgo"]
        NTgCalls --> FFmpeg["FFmpeg PCM Transcoder"]
    end
    
    NTgCalls -->|WebRTC RTP Audio Packets| VC["Telegram Group Voice Chat"]
    NTgCalls -.->|Stream End Hook (Port 8056)| Core
```

---

## 🛠️ Local VPS Installation & Setup

### Automated Installer (Debian, Ubuntu, Fedora, Arch)

```bash
git clone https://github.com/your-username/TinaMusic.git
cd TinaMusic
chmod +x install.sh
./install.sh
```

The installer installs system compilers, downloads prebuilt `libntgcalls` WebRTC static libraries, sets up `.env`, and builds both native binaries.

### Manual Compilation

```bash
# 1. Clone repository
git clone https://github.com/your-username/TinaMusic.git
cd TinaMusic

# 2. Setup environment variables
cp sample.env .env
nano .env

# 3. Compile sequentially (preserves memory on small VPS nodes)
make -j1

# 4. Launch bot & voice bridge
./start.sh
```

---

## 🐳 Docker & Docker Compose

```bash
# Copy and configure environment variables
cp sample.env .env
nano .env

# Build and start in background
docker compose up -d --build

# Inspect live logs
docker compose logs -f
```

---

## ⚙️ Configuration Reference (`.env`)

| Variable | Required | Description | Example |
| :--- | :---: | :--- | :--- |
| `API_ID` | **Yes** | Telegram App API ID from [my.telegram.org](https://my.telegram.org) | `1234567` |
| `API_HASH` | **Yes** | Telegram App API Hash from [my.telegram.org](https://my.telegram.org) | `abcdef0123456789...` |
| `BOT_TOKEN` | **Yes** | Telegram Bot Token from [@BotFather](https://t.me/BotFather) | `123456:ABC-DEF...` |
| `STRING_SESSIONS` | **Yes** | Pyrogram or Telethon session string for Assistant userbot | `1BVtsOG...` |
| `SESSION_TYPE` | No | Session string format (`pyrogram` or `telethon`, default: `pyrogram`) | `pyrogram` |
| `OWNER_ID` | **Yes** | Telegram User ID of the primary owner | `620476130` |
| `MONGO_DB_URI` | No | MongoDB URI for persistent settings & auth lists | `mongodb://...` |
| `LOGGER_ID` | No | Telegram Chat ID for administrative log channel | `-1001234567890` |
| `MUSIC_API_URL` | No | Yuki CDN stream URL template | `https://music.yukiapi.site/stream/{VIDEO_ID}?key=...` |
| `DURATION_LIMIT` | No | Maximum song duration in seconds (default: `4200` = 70 min) | `4200` |
| `QUEUE_LIMIT` | No | Maximum tracks permitted in a queue | `25` |
| `DEFAULT_LANG` | No | Bot interface language (`en`, `hi`, `ar`, `tr`) | `en` |

---

## 🎮 Command Reference

### User Commands
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/play` | `<track title / link>` | Stream high-quality audio in active group voice chat |
| `/vplay` | `<track title / link>` | Stream video/audio in active voice chat |
| `/queue` | - | Display current chat queue and upcoming songs |
| `/ping` | - | Measure bot latency and system host performance |
| `/help` | - | View interactive help and usage guide |
| `/settings` | - | Configure chat language and audio preferences |

### Admin Commands
| Command | Arguments | Description |
| :--- | :--- | :--- |
| `/pause` | - | Temporarily pause current audio stream |
| `/resume` | - | Resume paused audio stream |
| `/skip` | - | Skip current song and immediately play next in queue |
| `/end` / `/stop` | - | Clear playback queue and disconnect assistant from VC |
| `/shuffle` | - | Randomize the order of tracks in the queue |
| `/loop` | `[1-10 / disable]` | Enable repeat mode for single track or queue |
| `/volume` | `[1-200]` | Set voice chat output volume |
| `/auth` | `[reply / username]` | Grant bot administrative privileges to a user |
| `/unauth` | `[reply / username]` | Revoke administrative privileges from a user |

---

## ⚖️ Strict Copyleft License Notice

TinaMusic is licensed under the **GNU Affero General Public License v3.0 (AGPL-3.0)**.

```
TinaMusic - Telegram's First Native C++20 Music Engine
Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
Support: https://t.me/Z0iiw

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.
```

> [!WARNING]
> **Copyleft Protection**: Under AGPL-3.0, if you modify, host, or distribute TinaMusic (including network-hosted services), you **MUST** provide the complete corresponding source code to all users under the AGPLv3. Rebranding or deploying proprietary, closed-source forks is strictly forbidden.
