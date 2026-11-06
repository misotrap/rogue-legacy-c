# Rogue Legacy C

A native C++17 port of **Rogue Legacy** (Cellar Door Games, 2013), written
from the official [Rogue Legacy 1 source code](https://github.com/flibitijibibo/RogueLegacy1)
to run on small Linux handhelds, first of all the **Miyoo Mini Plus**
(OnionOS), at 60 frames per second. It also runs on macOS and Linux with SDL2.

> Rogue Legacy is the copyright of Cellar Door Games. This project is based on
> the Rogue Legacy 1 source code and follows its non-commercial license (see
> [LICENSE.md](LICENSE.md)). **No game asset is included**: you need your own
> copy of the game.

## What you need

- A copy of **Rogue Legacy** (for example on Steam). The game's pictures,
  sounds, music and texts come from it.
- The [Rogue Legacy 1 source code](https://github.com/flibitijibibo/RogueLegacy1),
  cloned as `ref/RogueLegacy1` (some converters read its data).
- Python 3 and a C++17 compiler.
- For the computer version: SDL2 (`brew install sdl2`, or your distribution's
  `libsdl2-dev`).
- For the Miyoo: an `armv7-unknown-linux-gnueabihf` cross toolchain (the
  `Makefile` looks for it in `~/toolchains`).

## Making the game files (`assets/`)

From your own copy of the game, once:

```sh
git clone https://github.com/flibitijibibo/RogueLegacy1 ref/RogueLegacy1
python3 tools/convert_assets.py "<your game>/Content" assets
c++ -O2 -std=c++17 -o convert_audio tools/convert_audio.cpp
./convert_audio "<your game>/Content/Audio" assets/audio
python3 tools/convert_loc.py ref/RogueLegacy1 assets src
```

On macOS with the Steam version, `<your game>` is
`Rogue Legacy.app/Contents/Resources`. The `assets/` folder is yours: never
publish it.

## Building and playing

On the computer:

```sh
make
./rlc
```

Keys: arrows to move, `Z` jump, `X` attack, `C` spell, `V` special,
`A`/`S` shoulder buttons, `Enter` start.

On the Miyoo Mini Plus (OnionOS):

```sh
make package     # builds build/RogueLegacyC
```

Copy `build/RogueLegacyC` into the SD card's `App/` folder. With the card in
a Mac, `sh tools/install_sd.sh` builds and installs in one go.

The language (English by default, or French, German, Spanish, Polish,
Portuguese, Russian, Chinese) is chosen in the options and kept in
`config.txt`.

## State of the port

Most of the game is in: the generated castle and its four areas, the enemies
and their variants, the bosses and their rematches, the manor and its skills,
the blacksmith and enchantress, the heirs and their traits, the special rooms,
the map and teleporters, the credits and New Game+. It is still being tested:
see `AUDIT.md` for the comparison with the original and `BUGS.md` for the
known bugs.

Not done yet:

- The Dragon's flight, and the flight runes and equipment.
- The last boss's rematch, and the "Neo" titles of the rematch bosses.
- The second shooting gallery, the diary's list of entries.
- The ending room with the boss portraits before the credits.

Developer notes (debug switches, test harness, layout of the code) are in
`AGENTS.md`.
