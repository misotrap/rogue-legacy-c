# Rogue Legacy, native C port for the Miyoo Mini Plus

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

Why C: the previous port ran the original C# game through Mono on the
console. It reached about 14 fps and took minutes to start because the game
loads ~240 MB into 128 MB of RAM. Do not go back to Mono, .NET or FNA.

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
- `RogueCastle/src/*EV.cs` (GameEV, LevelEV, PlayerEV, EnemyEV): constants.

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

Not converted yet: 3 sheets use DXT5 compression (surface format 6):
`titleScreen3Spritesheet`, `parallaxBGSpritesheet`, `getItemScreenSpritesheet`.
Add a DXT5 decoder to the converter when they are needed (FNA's
`DxtUtil.cs` shows the algorithm). Fonts (`Content/Fonts`) and sounds are
not converted yet either.

## Architecture (keep it)

- `src/platform.h`: 640x480 XRGB frame, buttons, clock.
  - `platform_sdl.c`: computer build (SDL2 window), for development.
  - `platform_miyoo.c`: console build, `/dev/fb0` (rotated 180 degrees, two
    pages) and `/dev/input/event0`. **No SDL on the console**, binary linked
    statically.
- `src/gfx.c`: software blitter, premultiplied alpha, nearest scaling, flip.
- `src/sprites.c`: sprite sheets, animations, hitboxes, characters.
- `src/main.c`: game loop. **Fixed 60 Hz logic steps**, drawing once per
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
   walking and jumping on a flat floor (`make && ./rlc`).
   First task: check the knight looks right (parts aligned, anchors,
   flipping). If parts are offset, fix the anchor or container offset
   convention in `spr_draw` / `DrawCharacter` against `SpriteSystem` and
   `ObjContainer.cs`. Rotated parts need rotation support in `gfx_blit`.
1. **One room**: load `rooms.txt`, show a room's CollHullObj as walls,
   doors, and collide the player with them (port PlayerObj physics values:
   speed, jump height, gravity, terminal velocity, drop-through platforms).
   Camera follows the player inside the room bounds.
2. **Player moves**: all states from PlayerObj (idle, walk, jump, fall,
   attack, air attack/down attack, dash, block, hurt, death) with the right
   characters and animation timing (AnimationDelay and per-frame duration).
3. **Combat**: sword hitbox from the attack frames (`R` records), breakable
   objects, damage, knockback, invincibility frames.
4. **Enemies**: start with the common ones (skeleton, knight, bat/flying
   eye, slime), then the rest, porting their AI from EnemyObjs.
5. **Castle**: LevelBuilder2 generation with the 4 areas, room transitions
   through doors, minimap.
6. **Lineage**: heirs, classes, traits, gold, manor/skill tree, blacksmith,
   enchantress, saves (own simple format in the app folder).
7. **Spells**, bosses, HUD and menus, fonts, sound (later, optional).

## Console package

Folder on the SD card: `/mnt/SDCARD/App/RogueLegacyC/` with `rlc` (from
`make miyoo`), `assets/`, `launch.sh`:

    #!/bin/sh
    cd "$(dirname "$0")"
    echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    ./rlc assets > log.txt 2>&1

and `config.json` (OnionOS app entry):
`{"label":"Rogue Legacy C","icon":"","launch":"launch.sh","description":"Rogue Legacy (C)"}`

Miyoo buttons are mapped in `platform_miyoo.c` (evdev key codes). MENU quits.

## Working rules

- Small steps: build (`make`), run, check, then the next step. Also run
  `make miyoo` regularly so the console build never breaks.
- Plain C99, no external libraries on the console (libc and libm only).
- Keep files focused (one subsystem per file); comment the why.
- Port numbers and rules from the C# source instead of guessing; note the
  source file in a comment.
- Never add game assets or the `ref/` source to the repository.
