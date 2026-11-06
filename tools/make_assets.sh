#!/bin/sh
# Makes assets/ from your own copy of Rogue Legacy, once: the pictures, the
# sounds and music, and the texts of the eight languages. Needs Python 3, a
# C++17 compiler and git.
#
#   sh tools/make_assets.sh "<your game>/Content"
#
# With the Steam version on macOS, the Content folder is in
# "Rogue Legacy.app/Contents/Resources/Content"; on Windows and Linux it is
# next to the game's executable.
set -eu
cd "$(dirname "$0")/.."
CONTENT=${1:?"usage: sh tools/make_assets.sh <the game's Content folder>"}
[ -d "$CONTENT" ] || { echo "Not found: $CONTENT"; exit 1; }

# The official source code: its data files hold the texts and the rooms
if [ ! -d ref/RogueLegacy1 ]; then
	mkdir -p ref
	git clone --depth 1 https://github.com/flibitijibibo/RogueLegacy1 ref/RogueLegacy1
fi

echo "Pictures..."
python3 tools/convert_assets.py "$CONTENT" assets
echo "Sounds and music..."
mkdir -p build
c++ -O2 -std=c++17 -o build/convert_audio tools/convert_audio.cpp
./build/convert_audio "$CONTENT/Audio" assets/audio
echo "Texts..."
python3 tools/convert_loc.py ref/RogueLegacy1 assets src
echo "Done: assets/ is ready (keep it to yourself, never publish it)."
