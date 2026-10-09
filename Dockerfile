FROM ubuntu:24.04 AS builder

WORKDIR /build

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        cmake \
        libcurl4-openssl-dev \
        libyaml-cpp-dev \
        nlohmann-json3-dev \
        zlib1g-dev \
        curl \
        unzip \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY . .

RUN make clean && make -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ffmpeg \
        curl \
        unzip \
        ca-certificates \
        libcurl4 \
        libyaml-cpp0.8 \
        zlib1g && \
    rm -rf /var/lib/apt/lists/*

RUN curl -fL https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp_linux -o /usr/local/bin/yt-dlp && \
    chmod 0755 /usr/local/bin/yt-dlp

WORKDIR /app

COPY --from=builder /build/bin/yukki-music-bot /app/yukki-music-bot
COPY locales /app/locales

ENTRYPOINT ["/app/yukki-music-bot"]
