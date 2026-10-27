# Rogue Legacy, native C++ port for the Miyoo Mini Plus

Read this whole file before changing anything. It is the brief.

## Goal

A native C version of Rogue Legacy that runs **smoothly on a Miyoo Mini Plus**
(OnionOS, ARM Cortex-A7 1.2 GHz dual core, 128 MB RAM, no GPU, 640x480
screen) and **starts in a few seconds**. Priorities, in order:

1. Fluid gameplay: at least 30 fps, target 60. Moving, jumping, attacking,
   spells, enemies and hitboxes must feel like the original.
2. Fast start (under 10 s) and low memory (under 60 MB).
3. Faithful game rules (classes, traits, skills, castle generation, gold,
   lineage). Graphics may be simplified: plain-colour backgrounds and walls
   are fine; no sky, rain, lightning, parallax or colour effects needed.

Why C++: the port transposes the C# game code almost line by line: same
classes, same method names, same file structure as the original source.
(Why not C# on the console: the previous port ran the original game through
Mono at about 14 fps, loading ~240 MB into 128 MB of RAM and taking minutes
to start. Do not go back to Mono, .NET or FNA.)

## Reference source (the rules to port)

The official source code: https://github.com/flibitijibibo/RogueLegacy1
(personal use licence: never redistribute game assets; this repo must stay
private). Clone it once into `ref/` (git-ignored):

    git clone https://github.com/flibitijibibo/RogueLegacy1 ref/RogueLegacy1

Port behaviour from it, file by file. Key files:
- `RogueCastle/src/Game Objects/Player/PlayerObj.cs`: movement, jump,
  attacks, dash, states, which body parts are visible.
- `RogueCastle/src/Game Objects/EnemyObjs/*.cs`: one file per enemy type.
- `RogueCastle/src/LevelBuilder2.cs`, `LevelParser.cs`, `RoomObj.cs`:
  castle generation and room templates.
- `RogueCastle/src/Managers/PhysicsManager*`, `DS2DEngine/src/...`:
  collisions (CollHullObj), physics, camera, sprite animation.
- `SpriteSystem/`: sprite sheet, animation and hitbox semantics.
- `RogueCastle/src/EVs/*.cs` (GameEV, LevelEV, PlayerEV, EnemyEV,
  SpellEV, GlobalEV): constants, converted automatically (see below).

## Porting rules (C++17)

The port is **C++17, compiled with `-fno-exceptions -fno-rtti`, static
link on the console**. Transpose the C# code file by file:

- Mirror the file structure: `src/DS2DEngine/GameObjs/ObjContainer.cpp`
  for `DS2DEngine/src/GameObjs/ObjContainer.cs`,
  `src/RogueCastle/GameObjects/Player/PlayerObj.cpp` for
  `RogueCastle/src/Game Objects/Player/PlayerObj.cs`, and so on (spaces
  in the C# directories are dropped). One C# class = one C++ class with
  the same name, in the same namespace.
- C# properties become a getter/setter pair with the property name:
  `obj.X`, `obj.X = 5` become `obj.X()`, `obj.X(5)`.
- The `as`/`is` casts are emulated without RTTI: `GameObj` has virtual
  `AsSpriteObj()`-style helpers; no `dynamic_cast`.
- No exceptions: C# `throw` becomes a returned null / default value, and
  callers must handle it. `new` failure aborts (fine for this project).
- C# `null` is `nullptr`; ownership: containers delete their children.
- Keep a `// C# filename.cs line N` comment on ported methods.

### Constants: the *EV.cs converter

`tools/convert_ev.py` converts the constant classes (*EV.cs and the
simple Types/*.cs) into C++ headers:

    make ev        # = python3 tools/convert_ev.py ref/RogueLegacy1 src

The generated headers are COMMITTED (src/RogueCastle/EVs/*.h,
src/RogueCastle/Types/*.h, src/DS2DEngine/Consts.h, EngineEV.h) because
ref/ is git-ignored. C# `GameEV.X` becomes C++ `GameEV::X`. Unsupported
statements (methods, enums, string[,]) appear as `// TODO C++` comments
and are reported by the converter: port those by hand.

## Game data

Never commit game files. Each developer converts their own Steam copy:

    python3 tools/convert_assets.py "<Steam>/Rogue Legacy/Rogue Legacy.app/Contents/Resources/Content" assets

Formats (documented at the top of `tools/convert_assets.py`):
- `assets/<sheet>.tex`: full-size BGRA premultiplied pixels.
- `assets/<sheet>.spr`: `I` frame images (name, index, x, y, w, h, anchor),
  `A` animation frames (name, frame from 1, image index from 0, duration),
  `R` hitboxes of the previous `A` frame, `C` character parts (container
  name, child sprite, offset, rotation).
- `assets/rooms.txt`: every room template (`ROOM`) and its objects (`G Type
  key=value ...`), all XML attributes kept. Object types: CollHullObj
  (solid terrain), BorderObj (terrain edges), EnemyObj (EnemyType,
  Difficulty), DoorObj, ChestObj, HazardObj, PhysicsObj, SpriteObj,
  ObjContainer, PhysicsObjContainer, EnemyOrbObj.

DXT5 sheets (surface format 6) are decoded by the converter:
`titleScreen3Spritesheet`, `parallaxBGSpritesheet`, `getItemScreenSpritesheet`.
(decoded like FNA's DxtUtil.cs).
Fonts are converted to `assets/fonts/<name>.tex/.fnt` and drawn by
`src/Text` (Text::Font: Measure/Draw/DrawOutlined, UTF-8, same layout
as SpriteFont; Junicode has the French accents). Check: `make texttest`.
Translations: `make loc` writes `assets/lang/<code>.txt` (from the
reference source's LocStrings*.resx) and `src/Text/LocIDs.h`;
`Text::Loc::Load("assets", "fr")` then `Text::Loc::Get(id)` /
`Format(id, ...)`. The player is French: default to "fr". Check:
`make loctest`.
Sounds are
not converted yet either.

## Architecture (keep it)

- `src/platform.h`: 640x480 XRGB frame, buttons, clock.
  - `platform_sdl.cpp`: computer build (SDL2 window), for development.
  - `platform_miyoo.cpp`: console build, `/dev/fb0` (rotated 180 degrees, two
    pages) and `/dev/input/event0`. **No SDL on the console**, binary linked
    statically.
- `src/gfx.cpp`: software blitter, premultiplied alpha, nearest scaling,
  flip. Rotation is still missing (the player parts never rotate).
- `src/DS2DEngine/`, `src/SpriteSystem/`, `src/RogueCastle/` and
  `src/InputSystem/`: the C# projects transposed class by class
  (GameObj, SpriteObj, ObjContainer, PhysicsObj, PhysicsObjContainer,
  CollisionBox, Camera2D, PhysicsManager, SpriteLibrary, PlayerObj...).
- `src/main.cpp`: game loop. **Fixed 60 Hz logic steps**, drawing once per
  loop; never tie game speed to the frame rate.

Rendering rules:
- The world is 1320x720 units per room screen, drawn at **scale 0.5**
  (660x360, letterboxed at y = 60 in 640x480). Most objects have scale 2,
  so their texels land exactly 1:1 on the screen: keep that, it is what
  makes the art crisp and blits cheap.
- Walls = plain coloured rectangles from CollHullObj. Background = one
  colour per area (castle, garden, tower, dungeon).
- Load only the sheets the current area needs; free the others.
- Performance budget per frame on the console (16 ms at 60 fps): a few
  thousand blits at most. Measure with `plat_time()` and print fps.

## Milestones (do them in order, test each on the computer first)

0. **Done**: converter, platforms, blitter, sprite library, the knight
   walking and jumping on a flat floor (`make && ./rlc`), pixel-verified
   against the original drawing semantics. The knight assembly (anchors,
   flip, hair visible, shield hidden like PlayerObj.cs
   UpdateEquipmentObjs) and the animation timing (1/10 s ticks, per-frame
   durations) are ported. The C++ classes (GameObj, SpriteObj,
   ObjContainer, PhysicsObj, PhysicsObjContainer, PhysicsManager,
   PlayerObj) carry the real physics: Speed 500, JumpHeight 1180,
   DoubleJumpHeight 845, gravity 1830 (GlobalEV.GRAVITY), terminal
   velocity 2000.
1. **One room**: load `rooms.txt`, show a room's CollHullObj as walls,
   doors, and collide the player with them (terrain collision response,
   drop-through platforms). Camera follows the player inside the room
   bounds.
2. **Player moves**: the remaining states from PlayerObj (attack, air
   attack/down attack, dash, block, hurt, death) with the right characters
   and animation timing.
3. **Combat**: sword hitbox from the attack frames (`R` records), breakable
   objects, damage, knockback, invincibility frames.
4. **Enemies**: start with the common ones (skeleton, knight, bat/flying
   eye, slime), then the rest, porting their AI from EnemyObjs
   (EnemyEV.h has their constants).
5. **Castle**: LevelBuilder2 generation with the 4 areas, room transitions
   through doors, minimap.
   **Generation done** in `src/RogueCastle/Levels/` (LevelParser,
   LevelBuilder2: one template per room, area and mirroring applied on
   placement, ~115 rooms in under 10 ms). `make castletest &&
   ./castletest 1 10 map.ppm` checks it. Still to do in game code: room
   transitions, special room behaviour (bonus/boss/challenge rooms keep
   their template name and tag), teleporters of linker and boss entrance
   rooms, garden floating rocks (HoverObj), the minimap.
6. **Lineage**: heirs, classes, traits, gold, manor/skill tree, blacksmith,
   **Logic done** (no screens yet): `src/RogueCastle/Lineage` (heirs),
   `src/RogueCastle/Skills` (manor, data from `make skills`),
   `src/RogueCastle/Equipment` (blacksmith and runes, data from
   `make equipment`; the Equipment*Type constants live there, do not
   generate them again with convert_ev.py). Checks: `make lineagetest
   skilltest equiptest`.
   **Save done**: `src/RogueCastle/Save` (Save::GameState = PlayerStats,
   family tree, offered heirs, bestiary, castle seed and visited rooms,
   manor, equipment; text file written through a .tmp and a rename, the
   previous save kept as .bak, a cut-short file falls back to it).
   Check: `make savetest`.
   **Heir choice screen done**: `src/RogueCastle/Screens/LineageScreen`
   (Open(state, generator), Update(buttons, pressed, dt) returns true when
   an heir is chosen = state.hero; Draw(frame)). Check: `make
   lineagescreentest && ./lineagescreentest out` (PPM images). To hook up:
   on the player's death add the hero to state.familyTree, save, open it.
   **Manor screen done**: `src/RogueCastle/Screens/ManorScreen` (Open(state),
   Update(buttons, pressed, dt) returns true on START, Draw(frame); icon
   positions and cursor links generated by `make skills`). Check: `make
   manortest && ./manortest out`. Original flow after a death: manor ->
   heir choice -> new run. Save after each room change and at death, in
   the app folder (e.g. `save.txt` next to `rlc`).
   enchantress, saves (own simple format in the app folder).
7. **Spells**, bosses, HUD and menus, fonts, sound (later, optional).

## Console package

Folder on the SD card: `/mnt/SDCARD/App/RogueLegacyC/` with `rlc` (from
`make miyoo`), `assets/` (only the files the game loads), `lib/` and
`launch.sh`. Tested on the console (OnionOS):

- **Never link glibc statically**: the toolchain's glibc refuses the
  console's kernel ("FATAL: kernel too old"). `make miyoo` builds the C++
  runtime in (`-static-libstdc++ -static-libgcc`) and uses glibc
  dynamically.
- `lib/` holds Debian bookworm armhf `ld-linux-armhf.so.3`, `libc.so.6`
  and `libm.so.6` (the same files as the Mono port's `lib/`), and the game
  starts through that loader:

```sh
#!/bin/sh
cd "$(dirname "$0")"
unset LD_PRELOAD   # OnionOS preloads libmi_*.so, which the bundled glibc cannot load
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
echo "Rogue Legacy C, $(date)" > log.txt
export LIBC_FATAL_STDERR_=1   # glibc fatal errors go to log.txt, not the tty
./lib/ld-linux-armhf.so.3 --library-path ./lib ./rlc assets >> log.txt 2>&1
echo "exited with status $?" >> log.txt
echo ondemand > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null
```

- `config.json` (OnionOS app entry):
  `{"label":"Rogue Legacy C","icon":"","launch":"launch.sh","description":"Rogue Legacy (C)"}`
- Remove macOS `._*` files when copying from a Mac.

Miyoo buttons are mapped in `platform_miyoo.cpp` (evdev key codes). MENU quits.

## Working rules

- Small steps: build (`make`), run, check, then the next step. Also run
  `make miyoo` regularly so the console build never breaks.
- C++17, `-fno-exceptions -fno-rtti`; on the console only libc, libm and
  the static C++ runtime (no external libraries).
- Transpose the C# code class by class (see "Porting rules"); keep the
  same class, method and file names as the original source.
- Keep files focused (one C# file = one .h/.cpp pair); comment the why.
- Port numbers and rules from the C# source instead of guessing; note the
  source file (and line) in a comment. Use the generated *EV headers
  (make ev) instead of retyping constants.
- Never add game assets or the `ref/` source to the repository.
