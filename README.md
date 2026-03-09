# Plex webOS Client

Native C++ Plex client for LG webOS TVs. Uses SDL2 + OpenGL ES for rendering, libcurl for HTTP, and the webOS Luna media pipeline for video playback.

## Dependencies

### Desktop (Linux)
```bash
sudo apt-get install -y \
    build-essential cmake pkg-config \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
    libcurl4-openssl-dev libgl-dev
```

### webOS Cross-Compilation
```bash
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
sudo npm install -g @webos-tools/cli
```

## Build

### Desktop (for development)
```bash
./scripts/build.sh desktop
./build/plex-webos
```

### webOS ARM
```bash
./scripts/build.sh webos
```

## Deploy to TV

1. Enable Developer Mode on your LG TV (install "Developer Mode" app from LG Content Store)
2. Register the TV:
   ```bash
   ares-setup-device
   ares-novacom --device tv --getkey
   ```
3. Package and deploy:
   ```bash
   ./scripts/package.sh
   ./scripts/deploy.sh tv
   ```

## Controls

| Key | Action |
|-----|--------|
| Arrow keys / D-pad | Navigate |
| Enter / OK | Select |
| Escape / Back | Go back |
| Space | Play/Pause |
| Q | Quit |

## Architecture

- `src/plex/` - Plex API client (auth, libraries, hubs, streaming)
- `src/ui/` - OpenGL ES renderer, text, image cache, screens
- `src/input/` - TV remote / keyboard input mapping
- `src/media/` - webOS Luna media pipeline integration
