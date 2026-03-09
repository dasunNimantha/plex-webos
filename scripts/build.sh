#!/bin/bash
set -e
cd "$(dirname "$0")/.."

BUILD_TYPE="${1:-desktop}"

if [ "$BUILD_TYPE" = "webos" ]; then
    echo "=== Building for webOS ARM ==="
    mkdir -p build-webos
    cd build-webos
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-webos.cmake \
             -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo "=== Build complete: build-webos/plex-webos ==="
else
    echo "=== Building for desktop ==="
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo "=== Build complete: build/plex-webos ==="
fi
