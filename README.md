# YukkiMusic C++20 🎵

A high-performance, native C++20 Telegram Music Bot engine ported from `YukkiMusic-Go`. Designed for ultra-low latency audio processing, minimal RAM footprint, and multi-chat queue management.

---

## ⚡ Features
- **Ultra-low RAM & CPU footprint:** Runs in ~25MB RAM under heavy playback workloads.
- **Native FFmpeg pipeline:** High-throughput 48000Hz 16-bit PCM stereo audio streaming.
- **Thread-safe Queue System:** Concurrent multi-chat queue with repeat loop, shuffle, and skip mechanics.
- **Modern C++20 Standard:** Lock-free / mutex-guarded state models using RAII and smart pointers.
- **REST & Telegram Bot API Integration:** Real-time polling and webhook-ready libcurl client.
- **Dual Build System:** Full support for both `make` and `CMake`.

---

## 🛠️ Build & Installation

### Requirements
- `g++` (version 13+ with C++20 support)
- `cmake` (version 3.20+) or `make`
- `libcurl4-openssl-dev`
- `nlohmann-json3-dev`
- `ffmpeg` & `yt-dlp`

### Quick Build (Make)
```bash
make
make test
./bin/yukki-music-bot
```

### CMake Build
```bash
mkdir -p build && cd build
cmake ..
make
./yukki-music-bot
```

---

## 🚀 Configuration
Create a `.env` file in the root directory:
```env
BOT_TOKEN=your_telegram_bot_token
OWNER_ID=your_telegram_user_id
```

---

## 📜 Architecture Overview
```
/root/YukkiMusic-Cpp/
├── include/
│   ├── audio_engine.hpp   # FFmpeg pipeline & playback stream session manager
│   ├── bot_engine.hpp     # Command router (/play, /pause, /skip, etc.)
│   ├── config.hpp         # Environment configuration loader
│   ├── logger.hpp         # Thread-safe console audit logger
│   ├── models.hpp         # Track, ChatPlaybackState models
│   ├── platform_yt.hpp    # YouTube metadata & audio URL extraction
│   ├── queue_manager.hpp  # Thread-safe multi-chat song queue
│   └── tg_client.hpp      # libcurl Telegram Bot API client
├── src/
│   ├── audio_engine.cpp
│   ├── bot_engine.cpp
│   ├── config.cpp
│   ├── logger.cpp
│   ├── main.cpp
│   ├── platform_yt.cpp
│   ├── queue_manager.cpp
│   └── tg_client.cpp
└── tests/
    └── test_queue.cpp     # Unit test suite
```
