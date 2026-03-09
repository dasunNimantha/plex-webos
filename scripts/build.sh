#!/bin/bash
set -e
cd "$(dirname "$0")/.."

BUILD_TYPE="${1:-desktop}"

SDK_IMAGE="${WEBOS_SDK_IMAGE:-dasunnimantha/webos-sdk:latest}"

if [ "$BUILD_TYPE" = "webos" ]; then
    echo "=== Building for webOS ARM (armhf) via Docker ==="
    echo "    Using image: ${SDK_IMAGE}"

    if ! docker run --rm --platform linux/arm/v7 "${SDK_IMAGE}" true 2>/dev/null; then
        echo "Setting up QEMU for ARM emulation..."
        docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
    fi

    docker run --rm --platform linux/arm/v7 \
        -v "$(pwd):/src" -w /src \
        "${SDK_IMAGE}" bash -c '
            set -e
            rm -rf build-webos && mkdir build-webos && cd build-webos
            cmake .. -DCMAKE_BUILD_TYPE=Release -DWEBOS_BUILD=ON \
                -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11
            make -j$(nproc)

            patchelf --set-interpreter /lib/ld-linux.so.3 plex-webos
            patchelf --replace-needed ld-linux-armhf.so.3 ld-linux.so.3 plex-webos || true
        '
    echo "=== Build complete: build-webos/plex-webos ==="

elif [ "$BUILD_TYPE" = "webos64" ]; then
    echo "=== Building for webOS ARM (aarch64) via Docker ==="
    echo "    Using image: ${SDK_IMAGE}"

    if ! docker run --rm --platform linux/arm64 "${SDK_IMAGE}" true 2>/dev/null; then
        echo "Setting up QEMU for ARM64 emulation..."
        docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
    fi

    docker run --rm --platform linux/arm64 \
        -v "$(pwd):/src" -w /src \
        "${SDK_IMAGE}" bash -c '
            set -e
            rm -rf build-webos && mkdir build-webos && cd build-webos
            cmake .. -DCMAKE_BUILD_TYPE=Release -DWEBOS_BUILD=ON \
                -DCMAKE_C_COMPILER=gcc-11 -DCMAKE_CXX_COMPILER=g++-11
            make -j$(nproc)

            patchelf --set-interpreter /lib/ld-linux-aarch64.so.1 plex-webos || true
        '
    echo "=== Build complete: build-webos/plex-webos ==="

else
    echo "=== Building for desktop ==="
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    echo "=== Build complete: build/plex-webos ==="
fi
