#!/usr/bin/env bash
# Simple helper to build the compi compiler and run it on a C source file.
# Usage:
#   ./build_and_run.sh <input.c> [output.vhdl]
# Example:
#   ./build_and_run.sh examples/example.c output.vhdl
# If output not provided, defaults to output.vhdl next to script.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INPUT_FILE="${1:-}"
OUTPUT_FILE="${2:-}"

if [[ -z "${INPUT_FILE}" ]]; then
  echo "Usage: $0 <input.c> [output.vhdl]" >&2
  exit 1
fi

if [[ ! -f "${INPUT_FILE}" ]]; then
  # Allow relative path from script dir
  if [[ -f "$SCRIPT_DIR/${INPUT_FILE}" ]]; then
    INPUT_FILE="$SCRIPT_DIR/${INPUT_FILE}"
  else
    echo "Error: input file '${INPUT_FILE}' not found" >&2
    exit 2
  fi
fi

if [[ -z "${OUTPUT_FILE}" ]]; then
  OUTPUT_FILE="$SCRIPT_DIR/output.vhdl"
fi

# Configure (only if needed)
if [[ ! -d "$BUILD_DIR" || ! -f "$BUILD_DIR/Makefile" && ! -f "$BUILD_DIR/build.ninja" ]]; then
  echo "[configure] Running CMake configure in $BUILD_DIR" >&2
  mkdir -p "$BUILD_DIR"
  cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" >/dev/null
fi

# Build compi target
if command -v nproc >/dev/null 2>&1; then
  JOBS="-j$(nproc)"
else
  JOBS=""
fi

echo "[build] Building compi target" >&2
cmake --build "$BUILD_DIR" --target compi $JOBS >/dev/null

BIN="$BUILD_DIR/compi"
if [[ ! -x "$BIN" ]]; then
  echo "Error: built binary not found at $BIN" >&2
  exit 3
fi

echo "[run] $BIN $INPUT_FILE $OUTPUT_FILE" >&2
"$BIN" "$INPUT_FILE" "$OUTPUT_FILE"

if [[ -f "$OUTPUT_FILE" ]]; then
  echo "[done] Generated VHDL: $OUTPUT_FILE" >&2
fi
