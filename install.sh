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

echo "========================================================================"
echo "  TINAMUSIC C++20 NATIVE TELEGRAM MUSIC ENGINE INSTALLER"
echo "  High-Performance, Low-Latency Telegram Voice Chat Platform"
echo "========================================================================"

install_deps_debian() {
    echo "[INSTALLER] Updating package manager..."
    sudo apt-get update -y
    echo "[INSTALLER] Installing compiler and system libraries..."
    sudo apt-get install -y \
        build-essential \
        g++ \
        make \
        cmake \
        git \
        curl \
        ffmpeg \
        libcurl4-openssl-dev \
        libyaml-cpp-dev \
        nlohmann-json3-dev \
        zlib1g-dev \
        golang-go \
        unzip
}

install_deps_fedora() {
    echo "[INSTALLER] Installing dependencies via dnf..."
    sudo dnf install -y \
        gcc-c++ \
        make \
        cmake \
        git \
        curl \
        ffmpeg \
        libcurl-devel \
        yaml-cpp-devel \
        json-devel \
        zlib-devel \
        golang \
        unzip
}

install_deps_arch() {
    echo "[INSTALLER] Installing dependencies via pacman..."
    sudo pacman -Sy --noconfirm \
        base-devel \
        cmake \
        git \
        curl \
        ffmpeg \
        yaml-cpp \
        nlohmann-json \
        go \
        unzip
}

if command -v apt-get >/dev/null 2>&1; then
    install_deps_debian
elif command -v dnf >/dev/null 2>&1; then
    install_deps_fedora
elif command -v pacman >/dev/null 2>&1; then
    install_deps_arch
else
    echo "[INSTALLER] Unsupported package manager. Please ensure g++, make, libcurl, yaml-cpp, ffmpeg, go, and unzip are installed."
fi

if [ ! -f ".env" ] && [ -f "sample.env" ]; then
    echo "[INSTALLER] Initializing .env from sample.env..."
    cp sample.env .env
    echo "[INSTALLER] Please edit .env with your Telegram API credentials and BOT_TOKEN."
fi

NTGCALLS_LIB="vendor/ntgcalls/lib/libntgcalls.a"
if [ ! -f "$NTGCALLS_LIB" ]; then
    echo "[INSTALLER] NTgCalls static library not found. Downloading prebuilt library..."
    mkdir -p vendor/ntgcalls/lib vendor/ntgcalls/include
    ARCH=$(uname -m)
    NTG_URL=""
    if [ "$ARCH" = "x86_64" ]; then
        NTG_URL="https://github.com/pytgcalls/ntgcalls/releases/download/v2.1.0/ntgcalls.linux-x86_64-static_libs.zip"
    elif [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then
        NTG_URL="https://github.com/pytgcalls/ntgcalls/releases/download/v2.1.0/ntgcalls.linux-arm64-static_libs.zip"
    else
        echo "[INSTALLER] Unsupported architecture for prebuilt NTgCalls: $ARCH"
        exit 1
    fi
    curl -sSL "$NTG_URL" -o /tmp/ntgcalls.zip
    unzip -q -o /tmp/ntgcalls.zip -d /tmp/ntgcalls_extracted
    LIB_FOUND=$(find /tmp/ntgcalls_extracted -name "libntgcalls.a" | head -n1)
    if [ -n "$LIB_FOUND" ]; then
        mv "$LIB_FOUND" "$NTGCALLS_LIB"
    fi
    if [ -f "/tmp/ntgcalls_extracted/include/ntgcalls.h" ]; then
        cp "/tmp/ntgcalls_extracted/include/ntgcalls.h" vendor/ntgcalls/include/
    fi
    rm -rf /tmp/ntgcalls.zip /tmp/ntgcalls_extracted
    echo "[INSTALLER] NTgCalls static library installed successfully."
fi

mkdir -p bridge/ntgcalls
(cd bridge && ln -sf ../vendor/ntgcalls/lib/libntgcalls.a libntgcalls.a)
(cd bridge/ntgcalls && ln -sf ../../vendor/ntgcalls/lib/libntgcalls.a libntgcalls.a)

mkdir -p bin

echo "[INSTALLER] Compiling TinaMusic C++ Engine and Native Voice Bridge..."
make clean || true
make -j1

chmod +x start.sh bin/tina-music-bot bin/yukki-music-bot bin/assistant-bridge 2>/dev/null || chmod +x start.sh bin/*

echo "========================================================================"
echo "  INSTALLATION COMPLETED SUCCESSFULLY"
echo "  Start your bot using: ./start.sh"
echo "========================================================================"
