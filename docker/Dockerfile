FROM ubuntu:jammy

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++-11 gcc-11 cmake pkg-config patchelf make \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
    libcurl4-openssl-dev libgles2-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-11 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-11 100

# Stub library for starfish media pipeline (real lib is on the TV)
RUN echo "void __stub(void){}" > /tmp/stub.c \
    && gcc -shared -o /usr/lib/libplayerAPIs.so /tmp/stub.c \
    && rm /tmp/stub.c

WORKDIR /src
