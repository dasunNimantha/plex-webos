#!/bin/bash
set -e
cd "$(dirname "$0")/.."

echo "=== Packaging for webOS ==="

DIST=dist
rm -rf "$DIST"
mkdir -p "$DIST"

cp build-webos/plex-webos "$DIST/"
cp webos/appinfo.json "$DIST/"

if [ -f webos/icon.png ]; then
    cp webos/icon.png "$DIST/"
else
    echo "Warning: No icon.png found, creating placeholder"
    # Create a minimal 1x1 PNG as placeholder
    printf '\x89PNG\r\n\x1a\n' > "$DIST/icon.png"
fi

ares-package "$DIST"

echo "=== Package created ==="
ls -la *.ipk
