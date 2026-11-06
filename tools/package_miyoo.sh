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
mkdir -p "$OUT/lib" "$OUT/assets"
cp build/miyoo/rlc "$OUT/"
# glibc 2.36 (+ the libpthread/libdl/librt stubs the console's SDL 1.2 asks for)
cp tools/miyoo-runtime/*.so* "$OUT/lib/"

# The game files, from assets/ (tools/make_assets.sh); NO_ASSETS=1 leaves
# them out: the package to share, each player adds their own
if [ "${NO_ASSETS:-0}" != 1 ]; then
	sh tools/copy_assets.sh "$OUT"
fi

cat > "$OUT/launch.sh" <<'LAUNCH'
#!/bin/sh
cd "$(dirname "$0")"
unset LD_PRELOAD   # OnionOS preloads libmi_*.so, which the bundled glibc cannot load
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null   # 60 fps on demand, the battery lasts
echo "Rogue Legacy C, $(date)" > log.txt
export LIBC_FATAL_STDERR_=1   # glibc fatal errors go to log.txt, not the tty
# /customer/lib after ./lib: the console's SDL 1.2 for the sound (dlopen)
./lib/ld-linux-armhf.so.3 --library-path ./lib:/customer/lib ./rlc assets >> log.txt 2>&1
echo "exited with status $?" >> log.txt
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
LAUNCH
printf '{\n  "label": "Rogue Legacy C",\n  "icon": "",\n  "launch": "launch.sh",\n  "description": "Rogue Legacy (port C++)"\n}\n' > "$OUT/config.json"
chmod +x "$OUT/launch.sh" "$OUT/rlc"
du -sh "$OUT"
if [ "${NO_ASSETS:-0}" = 1 ]; then
	# The download: the folder without any game file, zipped
	(cd build && rm -f RogueLegacyC-miyoo.zip && zip -qr RogueLegacyC-miyoo.zip RogueLegacyC)
	echo "build/RogueLegacyC-miyoo.zip is ready to share (no game file inside)."
else
	echo "Copy $OUT to the SD card's App/ folder (remove ._* files if copying from a Mac)."
fi
