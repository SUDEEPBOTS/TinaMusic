# Contributing to TinaMusic

Thank you for your interest in contributing to **TinaMusic**! We welcome bug fixes, performance optimizations, platform support, and architectural improvements.

---

## 🏗️ Architecture Overview

The system is decoupled into two high-performance native components:
1. **Core Bot Engine (`src/`, `include/`)**:
   - Written in modern **C++20**.
   - Handles Telegram Bot API long-polling, queue management, InnerTube API integration, SQLite/memory storage, and administrative commands.
   - Embeds an internal IPC listener on `127.0.0.1:8056` for audio playback callbacks.
2. **Native Voice Bridge (`bridge/`)**:
   - Written in **Go 1.22+** leveraging `gogram` MTProto client and Cgo bindings to `libntgcalls.a`.
   - Manages Assistant account WebRTC streaming, voice chat joining, and audio pipeline transcoding.
   - Provides an HTTP IPC interface on `127.0.0.1:8055`.

---

## 📋 Development Standards

### 1. Zero Comments in C++ Source Files
- To keep the codebase clean, lean, and self-documenting, **no comments** (`//` or `/* */`) are permitted in `.cpp` or `.hpp` files.
- Code should be written clearly with descriptive variable and function names.

### 2. Mature, Structured Console Logs
- Console logs must follow standard enterprise logging patterns:
  ```
  [YYYY-MM-DD HH:MM:SS] [LEVEL] [SCOPE] Message
  ```
- **Zero emojis in console logs**. Emojis and visual typography belong exclusively in Telegram user-facing messages, never in server or terminal outputs.

### 3. Sequential Compilation (`make -j1`)
- When compiling on memory-constrained VPS instances (1 GB RAM or lower), compile using `make -j1` to prevent out-of-memory kernel termination during heavy C++ template instantiation.

---

## 🛠️ Local Build & Test

1. Install dependencies:
   ```bash
   ./install.sh
   ```
2. Build binaries:
   ```bash
   make -j1
   ```
3. Run tests:
   ```bash
   make test
   ```

---

## 📜 Licensing Notice

All contributions are subject to the **GNU Affero General Public License v3 (AGPL-3.0)**. By submitting a pull request, you agree that your code will be released under this license.
