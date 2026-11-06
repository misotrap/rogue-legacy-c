# Rogue Legacy C

A native C++17 port of **Rogue Legacy** (Cellar Door Games, 2013), written
from the official [Rogue Legacy 1 source code](https://github.com/flibitijibibo/RogueLegacy1)
to run on small Linux handhelds, first of all the **Miyoo Mini Plus**
(OnionOS), at 60 frames per second. It also runs on macOS and Linux with SDL2.

> Rogue Legacy is the copyright of Cellar Door Games. This project is based on
> the Rogue Legacy 1 source code and follows its non-commercial license (see
> [LICENSE.md](LICENSE.md)). **No game asset is included**: you need your own
> copy of the game.

## Playing on a Miyoo Mini Plus (OnionOS)

1. Download `RogueLegacyC-miyoo.zip` from the [Releases](../../releases)
   page and unzip it into the SD card's `App/` folder, so that you get
   `App/RogueLegacyC/`. It holds the port only, without any game file.
2. On a computer, make the game files from your own copy of the game (once),
   with Python 3, a C++ compiler and git installed:

   ```sh
   git clone <this repository> rogue-legacy-c
   cd rogue-legacy-c
   sh tools/make_assets.sh "<your game>/Content"
   ```

   With the Steam version on macOS, the Content folder is in
   `Rogue Legacy.app/Contents/Resources/Content`; on Windows and Linux it is
   next to the game's executable.
3. Copy them next to the port on the SD card:

   ```sh
   sh tools/copy_assets.sh "<SD card>/App/RogueLegacyC"
   ```

4. Start "Rogue Legacy C" from the Apps menu. If it does not start, the
   reason is in `App/RogueLegacyC/log.txt`.

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

From your own copy of the game, once: `sh tools/make_assets.sh "<your game>/Content"`
(it clones the official source code into `ref/RogueLegacy1`, then converts the
pictures, the sounds and the texts). The `assets/` folder is yours: never
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
make package                 # build/RogueLegacyC, with your game files
NO_ASSETS=1 make package     # build/RogueLegacyC-miyoo.zip, without them (to share)
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
