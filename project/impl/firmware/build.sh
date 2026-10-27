#!/usr/bin/env bash
set -euo pipefail

SAMPLES_URL=https://github.com/lancaster-university/microbit-v2-samples
SAMPLES_TAG=v0.3.0
TOOLCHAIN_VERSION=13.3.rel1

here="$(cd "$(dirname "$0")" && pwd)"
work="$here/.build"
codal="$work/codal"
mkdir -p "$work"

if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    if [ ! -x "$work/toolchain/bin/arm-none-eabi-gcc" ]; then
        case "$(uname -s)-$(uname -m)" in
            Darwin-arm64) host=darwin-arm64 ;;
            Darwin-x86_64) host=darwin-x86_64 ;;
            Linux-x86_64) host=x86_64 ;;
            Linux-aarch64) host=aarch64 ;;
            *) echo "No Arm GNU toolchain download for $(uname -s)-$(uname -m)." >&2; exit 1 ;;
        esac
        name="arm-gnu-toolchain-$TOOLCHAIN_VERSION-$host-arm-none-eabi"
        curl -fSL --progress-bar -o "$work/toolchain.tar.xz" \
            "https://developer.arm.com/-/media/Files/downloads/gnu/$TOOLCHAIN_VERSION/binrel/$name.tar.xz"
        mkdir -p "$work/toolchain"
        tar -xf "$work/toolchain.tar.xz" -C "$work/toolchain" --strip-components=1
        rm "$work/toolchain.tar.xz"
    fi
    export PATH="$work/toolchain/bin:$PATH"
fi

if [ ! -d "$codal" ]; then
    git clone --quiet --depth 1 --branch "$SAMPLES_TAG" "$SAMPLES_URL" "$codal"
fi
cp "$here/codal.json" "$codal/codal.json"
rm -rf "$codal/source"
mkdir -p "$codal/source"
cp "$here/main.cpp" "$codal/source/"
cp "$(dirname "$here")/src/board_behavior.hpp" "$codal/source/"
(cd "$codal" && python3 build.py)
cp "$codal/MICROBIT.hex" "$here/MICROBIT.hex"
echo "Built $here/MICROBIT.hex"
