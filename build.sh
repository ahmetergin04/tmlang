#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"
LSPF="$ROOT/lspf"
DEBUG_EXE="$BUILD/Debug/tmlang"
RELEASE_EXE="$BUILD/Release/tmlang"

# Prefer Ninja Multi-Config so the layout matches Windows (build/<Config>/tmlang).
# Without ninja, fall back to one single-config Makefile tree per config at build/<Config>.
if command -v ninja >/dev/null 2>&1; then
    MULTI_CONFIG=1
else
    MULTI_CONFIG=0
fi

usage() {
    echo "Usage:"
    echo "  ./build.sh init"
    echo "  ./build.sh clean"
    echo "  ./build.sh compile"
    echo "  ./build.sh run"
    echo "  ./build.sh compile-release"
    echo "  ./build.sh run-release"
    echo "  ./build.sh restart"
    echo "  ./build.sh lsp"
    exit 1
}

init() {
    mkdir -p "$BUILD"
    if [ "$MULTI_CONFIG" -eq 1 ]; then
        cmake -S "$ROOT" -B "$BUILD" -G "Ninja Multi-Config"
    else
        cmake -S "$ROOT" -B "$BUILD/Debug" -DCMAKE_BUILD_TYPE=Debug
        cmake -S "$ROOT" -B "$BUILD/Release" -DCMAKE_BUILD_TYPE=Release
    fi
}

is_configured() {
    [ -f "$BUILD/CMakeCache.txt" ] || [ -f "$BUILD/Debug/CMakeCache.txt" ]
}

clean() {
    rm -rf "$BUILD"
    mkdir -p "$BUILD"
}

build_config() {
    local config="$1"
    is_configured || init
    if [ "$MULTI_CONFIG" -eq 1 ]; then
        cmake --build "$BUILD" --config "$config" --parallel
    else
        cmake --build "$BUILD/$config" --parallel
    fi
}

compile()         { build_config Debug; }
run()             { compile; "$DEBUG_EXE"; }
compile_release() { build_config Release; }
run_release()     { compile_release; "$RELEASE_EXE"; }

lsp() {
    rm -rf "$LSPF"
    mkdir -p "$LSPF"
    if [ "$MULTI_CONFIG" -eq 1 ]; then
        cmake -S "$ROOT" -B "$LSPF" -G "Ninja Multi-Config" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    else
        cmake -S "$ROOT" -B "$LSPF" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    fi
    mv -f "$LSPF/compile_commands.json" "$ROOT/"
    rm -rf "$LSPF"
}

restart() {
    lsp
    clean
    init
    run
}

case "${1:-}" in
    init)            init ;;
    clean)           clean ;;
    compile)         compile ;;
    run)             run ;;
    compile-release) compile_release ;;
    run-release)     run_release ;;
    restart)         restart ;;
    lsp)             lsp ;;
    *)               usage ;;
esac
