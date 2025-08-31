#!/usr/bin/env bash
# build_and_run.sh - Convenience script to configure, build, test (optional) and run the 'compi' executable
# Location: tools/dds/compi/
#
# Features:
#  - Idempotent incremental builds (reconfigure only when needed)
#  - Debug / Release toggle (default Release)
#  - Optional clean
#  - Optional test execution
#  - Pass-through of remaining args to the executable
#  - Minimal dependency on global environment; works in fresh clone
#
# Usage:
#   ./build_and_run.sh [options] [-- <program arguments>]
# Options:
#   -d, --debug          Build with DEBUG=ON and CMAKE_BUILD_TYPE=Debug
#   -r, --release        Force Release build (default)
#   -c, --clean          Remove build directory before configuring
#   -t, --tests          Run tests after build (if enabled in CMake)
#       --no-tests       Skip tests (default)
#   -j, --jobs N         Parallel build jobs (default: number of cores)
#       --preset NAME    Use a CMake preset if available
#       --generator G    Explicit CMake generator (overrides preset)
#       --build-dir DIR  Custom build directory (default: ./build)
#   -v, --verbose        Verbose build (V=1 / VERBOSE=1)
#   -h, --help           Show help
#
# Examples:
#   ./build_and_run.sh -d -- -i sample.dds
#   ./build_and_run.sh --tests
#   ./build_and_run.sh --build-dir build-debug -d
#
set -euo pipefail
IFS=$'\n\t'

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
DEFAULT_BUILD_DIR="$PROJECT_ROOT/build"
BUILD_DIR="$DEFAULT_BUILD_DIR"
BUILD_TYPE="Release"
DEBUG_FLAG="OFF"
RUN_TESTS=0
CMAKE_PRESET=""
GENERATOR=""
VERBOSE=0
JOBS=""
CLEAN=0
PROGRAM_ARGS=()

color() { local code="$1"; shift; if [[ -t 1 ]]; then printf "\e[%sm%s\e[0m" "$code" "$*"; else printf "%s" "$*"; fi; }
info()  { echo "$(color 36 [INFO]) $*"; }
warn()  { echo "$(color 33 [WARN]) $*"; }
err()   { echo "$(color 31 [ERR ]) $*" >&2; }

show_help() { sed -n '1,/^set -euo/p' "$0" | sed '/^set -euo/q'; }

# Parse args
while [[ $# -gt 0 ]]; do
  case "$1" in
    -d|--debug)   BUILD_TYPE="Debug"; DEBUG_FLAG="ON";;
    -r|--release) BUILD_TYPE="Release"; DEBUG_FLAG="OFF";;
    -c|--clean)   CLEAN=1;;
    -t|--tests)   RUN_TESTS=1;;
       --no-tests) RUN_TESTS=0;;
    -j|--jobs)    shift; JOBS="${1:-}";;
       --jobs=*)  JOBS="${1#*=}";;
       --preset)  shift; CMAKE_PRESET="${1:-}";;
       --preset=*) CMAKE_PRESET="${1#*=}";;
       --generator) shift; GENERATOR="${1:-}";;
       --generator=*) GENERATOR="${1#*=}";;
       --build-dir) shift; BUILD_DIR="$(realpath -m "${1:-}")";;
       --build-dir=*) BUILD_DIR="$(realpath -m "${1#*=}")";;
    -v|--verbose) VERBOSE=1;;
    -h|--help)    show_help; exit 0;;
    --) shift; PROGRAM_ARGS=("$@"); break;;
    *) PROGRAM_ARGS+=("$1");; # passthrough to executable
  esac
  shift || true
done

if [[ -n "$JOBS" && ! "$JOBS" =~ ^[0-9]+$ ]]; then
  err "Invalid jobs count: $JOBS"; exit 1
fi

if [[ $CLEAN -eq 1 && -d "$BUILD_DIR" ]]; then
  info "Cleaning build directory $BUILD_DIR"
  rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
CONFIGURE_NEEDED=0

# Trigger reconfigure if cache missing or build type mismatch
if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
  CONFIGURE_NEEDED=1
else
  # Inspect existing cache for build type / debug flag
  if ! grep -q "CMAKE_BUILD_TYPE:STRING=$BUILD_TYPE" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then CONFIGURE_NEEDED=1; fi
  if ! grep -q "DEBUG:BOOL=$DEBUG_FLAG" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then CONFIGURE_NEEDED=1; fi
fi

if [[ $CONFIGURE_NEEDED -eq 1 ]]; then
  info "Configuring (type=$BUILD_TYPE DEBUG=$DEBUG_FLAG)"
  pushd "$BUILD_DIR" >/dev/null
  if [[ -n "$CMAKE_PRESET" ]]; then
    cmake --preset "$CMAKE_PRESET" -DDEBUG=$DEBUG_FLAG || { err "CMake preset configure failed"; exit 1; }
  else
    CMAKE_GENERATOR_ARGS=()
    if [[ -n "$GENERATOR" ]]; then
      CMAKE_GENERATOR_ARGS+=("-G" "$GENERATOR")
    fi
    cmake "${CMAKE_GENERATOR_ARGS[@]}" \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DDEBUG=$DEBUG_FLAG \
      "$PROJECT_ROOT" || { err "CMake configure failed"; exit 1; }
  fi
  popd >/dev/null
else
  info "Reusing existing configuration in $BUILD_DIR"
fi

# Build
BUILD_CMD=(cmake --build "$BUILD_DIR" --target compi)
if [[ -n "$JOBS" ]]; then BUILD_CMD+=(-- -j"$JOBS"); fi
if [[ $VERBOSE -eq 1 ]]; then BUILD_CMD+=(--verbose); fi
info "Building target 'compi'"
"${BUILD_CMD[@]}" || { err "Build failed"; exit 1; }

# Optional tests
if [[ $RUN_TESTS -eq 1 ]]; then
  if [[ -x "$BUILD_DIR/compi_tests" ]]; then
    info "Running tests (compi_tests)"
    (cd "$BUILD_DIR" && ctest --output-on-failure) || { err "Tests failed"; exit 1; }
  else
    warn "Tests requested but compi_tests not built (maybe ENABLE_TESTING=OFF or no tests present)."
  fi
fi

# Run executable
EXEC_PATH="$BUILD_DIR/compi"
if [[ ! -x "$EXEC_PATH" ]]; then
  err "Executable not found: $EXEC_PATH"; exit 1
fi
info "Running: $EXEC_PATH ${PROGRAM_ARGS[*]:-}";
"$EXEC_PATH" "${PROGRAM_ARGS[@]}"
EXIT_CODE=$?
info "Program exited with code $EXIT_CODE"
exit $EXIT_CODE
