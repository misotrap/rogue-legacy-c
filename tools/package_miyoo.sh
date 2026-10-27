#!/bin/sh
# Builds build/RogueLegacyC/: the folder to copy to /App/ on the Miyoo SD
# card (OnionOS). Contains the ARM binary, the Debian C library it runs on
# (tools/miyoo-runtime, see AGENTS.md "Console package"), only the assets
# the game loads, launch.sh and config.json.
#
#   make package          (then copy build/RogueLegacyC to SD:/App/)
set -eu
cd "$(dirname "$0")/.."
OUT=build/RogueLegacyC
rm -rf "$OUT"
mkdir -p "$OUT/lib" "$OUT/assets/fonts" "$OUT/assets/lang"
cp build/miyoo/rlc "$OUT/"
cp tools/miyoo-runtime/ld-linux-armhf.so.3 tools/miyoo-runtime/libc.so.6 tools/miyoo-runtime/libm.so.6 "$OUT/lib/"

# Sprite sheets used so far (add new ones here as the game loads them)
for sheet in playerSpritesheetBig startingRoomSpritesheet traitsCastleSpritesheet blacksmithUISpritesheet \
	castleTerrainSpritesheet enemyFinal2Spritesheet enemyFinalSpritesheetBig miscSpritesheet; do
	for ext in tex spr; do
		[ -f "assets/$sheet.$ext" ] && cp "assets/$sheet.$ext" "$OUT/assets/"
	done
done
cp assets/rooms.txt assets/HeroNames.txt assets/HeroineNames.txt "$OUT/assets/"
for font in Junicode JunicodeLarge GoldFont; do
	cp "assets/fonts/$font.tex" "assets/fonts/$font.fnt" "$OUT/assets/fonts/"
done
cp assets/lang/fr.txt assets/lang/en.txt "$OUT/assets/lang/"

cat > "$OUT/launch.sh" <<'LAUNCH'
#!/bin/sh
cd "$(dirname "$0")"
unset LD_PRELOAD   # OnionOS preloads libmi_*.so, which the bundled glibc cannot load
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
echo "Rogue Legacy C, $(date)" > log.txt
export LIBC_FATAL_STDERR_=1   # glibc fatal errors go to log.txt, not the tty
./lib/ld-linux-armhf.so.3 --library-path ./lib ./rlc assets >> log.txt 2>&1
echo "exited with status $?" >> log.txt
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
LAUNCH
printf '{\n  "label": "Rogue Legacy C",\n  "icon": "",\n  "launch": "launch.sh",\n  "description": "Rogue Legacy (port C++)"\n}\n' > "$OUT/config.json"
chmod +x "$OUT/launch.sh" "$OUT/rlc"
du -sh "$OUT"
echo "Copy $OUT to the SD card's App/ folder (remove ._* files if copying from a Mac)."
