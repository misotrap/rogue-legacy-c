#!/bin/sh
# Copies the game files the port loads, from assets/ (made by
# tools/make_assets.sh from your own copy of the game) into a Rogue Legacy C
# folder, e.g. the one on your Miyoo's SD card.
#
#   sh tools/copy_assets.sh /Volumes/<SD card>/App/RogueLegacyC
set -eu
cd "$(dirname "$0")/.."
DEST=${1:?"usage: sh tools/copy_assets.sh <RogueLegacyC folder>"}
[ -f assets/rooms.txt ] || { echo "assets/ is empty: run tools/make_assets.sh first"; exit 1; }
mkdir -p "$DEST/assets/fonts" "$DEST/assets/lang"

# Every sprite sheet the code names
for sheet in $(grep -rhoE '"[a-zA-Z0-9]+Spritesheet[a-zA-Z0-9]*"' src | tr -d '"' | sort -u); do
	for ext in tex spr; do
		[ -f "assets/$sheet.$ext" ] && cp "assets/$sheet.$ext" "$DEST/assets/"
	done
done
cp assets/rooms.txt assets/HeroNames.txt assets/HeroineNames.txt "$DEST/assets/"
# The fonts (NotoSansSC: the Chinese texts)
for font in Junicode JunicodeLarge GoldFont NotoSansSC; do
	cp "assets/fonts/$font.tex" "assets/fonts/$font.fnt" "$DEST/assets/fonts/"
done
# The eight languages
cp assets/lang/*.txt "$DEST/assets/lang/"
# Sound (tools/convert_audio.cpp), when converted
if [ -d assets/audio ]; then
	mkdir -p "$DEST/assets/audio"
	cp -R assets/audio/. "$DEST/assets/audio/"
fi
# macOS leaves ._* files on FAT cards: the console does not need them
find "$DEST" -name '._*' -delete 2>/dev/null || true
echo "Game files copied to $DEST"
