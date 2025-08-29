#!/usr/bin/env bash
# Convenience helper to configure (if needed) and build Sphinx HTML docs.
set -euo pipefail
BUILD_DIR="build"
CONFIG_ARGS=${CONFIG_ARGS:-"-DENABLE_TESTING=ON"}

if [ ! -d "$BUILD_DIR" ]; then
  echo "[build_docs] Configuring CMake project..."
  cmake -S . -B "$BUILD_DIR" $CONFIG_ARGS
fi

echo "[build_docs] Building docs target..."
cmake --build "$BUILD_DIR" --target docs -j "${JOBS:-4}"
