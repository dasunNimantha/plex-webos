#!/bin/bash
set -e
cd "$(dirname "$0")/.."

DEVICE="${1:-tv}"
IPK=$(ls -1 com.plex.webos_*.ipk 2>/dev/null | head -1)

if [ -z "$IPK" ]; then
    echo "No IPK found. Run package.sh first."
    exit 1
fi

echo "=== Deploying $IPK to $DEVICE ==="

ares-install --device "$DEVICE" "$IPK"
echo "=== Installed. Launching... ==="

ares-launch --device "$DEVICE" com.plex.webos
echo "=== Launched! ==="
echo ""
echo "To view logs:"
echo "  ares-inspect --device $DEVICE --app com.plex.webos --open"
