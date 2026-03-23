# Copyright (c) 2026 MidnightHammer-code
# This source code is licensed under the GPL 3.0 license
# LICENSE file in the root directory of this source tree.


#!/bin/bash

set -e

echo "========================================"
echo "    dhewm3 Auto-Compiler Script         "
echo "========================================"

echo ">>> Step 1: Installing build dependencies..."
if command -v apt >/dev/null 2>&1; then
    echo "Detected Debian/Ubuntu-based system (apt)."
    sudo apt update
    sudo apt install -y git cmake build-essential libsdl2-dev libopenal-dev libcurl4-openssl-dev
elif command -v pacman >/dev/null 2>&1; then
    echo "Detected Arch-based system (pacman)."
    sudo pacman -Sy --noconfirm git cmake base-devel sdl2 openal curl
elif command -v dnf >/dev/null 2>&1; then
    echo "Detected Fedora-based system (dnf)."
    sudo dnf install -y git cmake gcc-c++ make SDL2-devel openal-soft-devel libcurl-devel
elif command -v zypper >/dev/null 2>&1; then
    echo "Detected openSUSE-based system (zypper)."
    sudo zypper install -y git cmake gcc-c++ make libSDL2-devel openal-soft-devel libcurl-devel
else
    echo "WARNING: Could not detect your package manager."
    echo "Please manually install: git, cmake, a C/C++ compiler suite (like build-essential), and the development headers for SDL2, OpenAL, and cURL."
    echo "Press ENTER to continue anyway, or Ctrl+C to abort."
    read -r
fi

mkdir -p build
cd build

echo -e "\n>>> Step 4: Configuring CMake..."
cmake ../neo/ -DCMAKE_BUILD_TYPE=Release

echo -e "\n>>> Step 5: Compiling dhewm3..."
CORES=$(nproc 2>/dev/null || echo 4)
echo "Using $CORES CPU threads for compilation..."
make -j"$CORES"


echo -e "\n========================================"
echo " SUCCESS! dhewm3 has been compiled."
echo "========================================"

echo "Moving executable, base.so, libidlib.a and d3xp.so to main dhewm3 folder"

mv dhewm3 ..
mv d3xp.so ..
mv libidlib.a ..
mv base.so ..

echo ""
echo "To run the game, run:"
echo "./run.sh"
echo "Or double clic the run.sh file."
echo "(Make sure that the dhewm3 folder contains the base/ folder with pak000.pk4 to pak008.pk4)"
