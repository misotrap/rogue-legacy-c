#!/bin/sh
# Builds the Miyoo version and installs it on the SD card (ONION).
# Usage, from the project folder:  sh tools/install_sd.sh
set -e
cd "$(dirname "$0")/.."
DEST=/Volumes/ONION/App/RogueLegacyC

if [ ! -d "$DEST" ]; then
	echo "Carte SD introuvable : branche-la (elle doit s'appeler ONION)."
	exit 1
fi

echo "Compilation de la version Miyoo..."
make miyoo

echo "Copie du jeu sur la carte..."
cp "$DEST/rlc" "$DEST/rlc.bak" 2>/dev/null || true
cp build/miyoo/rlc "$DEST/rlc"
echo "Mise a jour des images et des sons..."
rsync -rt --modify-window=2 assets/ "$DEST/assets/"
# The C library the game runs on (libdl included: the sound loads the
# console's SDL with it)
mkdir -p "$DEST/lib"
cp tools/miyoo-runtime/*.so* "$DEST/lib/"
# launch.sh: the console's libraries (/customer/lib) for the sound; the
# rest of the file (the CPU setting...) stays as it is
if ! grep -q "/customer/lib" "$DEST/launch.sh"; then
	sed -i '' 's|--library-path ./lib |--library-path ./lib:/customer/lib |' "$DEST/launch.sh"
	echo "launch.sh : chemin du son ajoute."
fi
sync

if cmp -s build/miyoo/rlc "$DEST/rlc"; then
	echo "Installe. Ejection de la carte..."
	diskutil eject /Volumes/ONION
	echo "Tu peux retirer la carte."
else
	echo "Erreur : la copie ne correspond pas, ne retire pas la carte."
	exit 1
fi
