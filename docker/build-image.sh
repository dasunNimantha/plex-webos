#!/bin/bash
set -e

IMAGE_NAME="${DOCKER_IMAGE:-dasunnimantha/webos-sdk}"
VERSION="1.0.0"

echo "=== Building and pushing multi-arch image ==="
echo "  ${IMAGE_NAME}:${VERSION}"
echo "  ${IMAGE_NAME}:latest"

# Create builder if it doesn't exist
docker buildx inspect webos-builder >/dev/null 2>&1 || \
    docker buildx create --name webos-builder --use

docker buildx use webos-builder

docker buildx build \
    --platform linux/arm/v7,linux/arm64 \
    --tag "${IMAGE_NAME}:${VERSION}" \
    --tag "${IMAGE_NAME}:latest" \
    --push \
    -f "$(dirname "$0")/Dockerfile" \
    "$(dirname "$0")"

echo "=== Pushed ${IMAGE_NAME}:${VERSION} + latest (armhf + arm64) ==="
