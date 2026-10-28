#!/usr/bin/env bash
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
work="$here/.build"
codal="$work/codal"
mkdir -p "$work"
if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    toolchain="$work/toolchain"
    if [ ! -x "$toolchain/bin/arm-none-eabi-gcc" ]; then
        host="x86_64"
        archive="arm-gnu-toolchain-13.3.rel1-$host-arm-none-eabi.tar.xz"
        curl -fSL --progress-bar -o "$work/toolchain.tar.xz" \
            "https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/$archive"
        mkdir -p "$toolchain"
        tar -xf "$work/toolchain.tar.xz" -C "$toolchain" --strip-components=1
        rm "$work/toolchain.tar.xz"
    fi
    export PATH="$toolchain/bin:$PATH"
fi
if [ ! -d "$codal" ]; then
    git clone --quiet --depth 1 --branch v0.3.0 https://github.com/lancaster-university/microbit-v2-samples "$codal"
fi
cp "$here/codal.json" "$codal/codal.json"
rm -rf "$codal/source"
mkdir -p "$codal/source"
cp "$here/main.cpp" "$codal/source/main.cpp"
(cd "$codal" && python3 build.py)
cp "$codal/MICROBIT.hex" "$here/MICROBIT.hex"
echo "Built $here/MICROBIT.hex"
