FROM ubuntu:24.04 AS builder

WORKDIR /build

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        make \
        cmake \
        libcurl4-openssl-dev \
        libyaml-cpp-dev \
        zlib1g-dev \
        curl \
        unzip \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

COPY . .

RUN ARCH=$(uname -m) && \
    mkdir -p vendor/ntgcalls/lib vendor/ntgcalls/include && \
    if [ ! -f "vendor/ntgcalls/lib/libntgcalls.a" ]; then \
        if [ "$ARCH" = "x86_64" ]; then \
            curl -sSL "https://github.com/pytgcalls/ntgcalls/releases/download/v2.1.0/ntgcalls.linux-x86_64-static_libs.zip" -o /tmp/ntgcalls.zip; \
        elif [ "$ARCH" = "aarch64" ] || [ "$ARCH" = "arm64" ]; then \
            curl -sSL "https://github.com/pytgcalls/ntgcalls/releases/download/v2.1.0/ntgcalls.linux-arm64-static_libs.zip" -o /tmp/ntgcalls.zip; \
        fi && \
        unzip -q -o /tmp/ntgcalls.zip -d /tmp/ntgcalls_extracted && \
        mv $(find /tmp/ntgcalls_extracted -name "libntgcalls.a" | head -n1) vendor/ntgcalls/lib/libntgcalls.a && \
        cp /tmp/ntgcalls_extracted/include/ntgcalls.h vendor/ntgcalls/include/ && \
        rm -rf /tmp/ntgcalls.zip /tmp/ntgcalls_extracted; \
    fi && \
    make -j1

FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ffmpeg \
        curl \
        ca-certificates \
        libcurl4 \
        libyaml-cpp0.8 \
        zlib1g && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/bin /app/bin
COPY --from=builder /build/locales /app/locales
COPY --from=builder /build/data /app/data
COPY --from=builder /build/start.sh /app/start.sh

RUN chmod +x /app/start.sh /app/bin/*

ENTRYPOINT ["/bin/bash", "/app/start.sh"]
