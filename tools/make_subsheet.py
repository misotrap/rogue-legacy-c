#!/usr/bin/env python3
"""Copies a few sprites of a big converted sheet into a small sheet, so a
screen that needs four images does not load a 16 MB texture. Run after
convert_assets.py (stdlib only):

    python3 tools/make_subsheet.py assets

Writes assets/uiExtraSpritesheet.tex/.spr from miscSpritesheet: the
dialogue box, the choice highlight, the stat plate, the talk bubbles, the
death screen spotlight and ghost. Same sprite and character names as the
original sheet, so the code just loads the small sheet instead.
"""
import os
import struct
import sys

SOURCE = 'miscSpritesheet'
OUT = 'uiExtraSpritesheet'
SPRITES = ['DialogBox_Sprite', 'DialogueChoiceHighlight_Sprite', 'GameOverStatPlate_Sprite',
           'ContinueTextIcon_Sprite', 'UpArrowBubble_Sprite', 'ExclamationSquare_Sprite',
           'GameOverSpotlight_Sprite', 'PlayerGhost_Sprite']
CHARACTERS = ['DialogBox_Character', 'GameOverStatPlate_Character']
WIDTH = 1024


def read_tex(path):
    with open(path, 'rb') as f:
        magic, w, h = struct.unpack('<4sII', f.read(12))
        assert magic == b'RLT1', path
        return w, h, f.read(w * h * 4)


def main():
    assets = sys.argv[1] if len(sys.argv) > 1 else 'assets'
    sw, sh, px = read_tex(os.path.join(assets, SOURCE + '.tex'))
    lines = open(os.path.join(assets, SOURCE + '.spr'), encoding='utf-8').read().splitlines()
    frames = []   # (fields of the I line)
    keep = []     # A/R lines of the kept sprites, C lines of the kept characters
    last = None
    for line in lines:
        f = line.split('\t')
        if f[0] == 'I' and f[1] in SPRITES:
            frames.append(f)
        elif f[0] == 'A':
            last = f[1]
            if last in SPRITES:
                keep.append(line)
        elif f[0] == 'R' and last in SPRITES:
            keep.append(line)
        elif f[0] == 'C' and f[1] in CHARACTERS:
            keep.append(line)
    # shelf packing, tallest first, 1 px apart
    order = sorted(range(len(frames)), key=lambda i: -int(frames[i][6]))
    x = y = shelf = 0
    place = {}
    for i in order:
        w, h = int(frames[i][5]), int(frames[i][6])
        if x + w > WIDTH:
            x, y, shelf = 0, y + shelf + 1, 0
        place[i] = (x, y)
        x += w + 1
        shelf = max(shelf, h)
    height = y + shelf
    out = bytearray(WIDTH * height * 4)
    spr = []
    for i, f in enumerate(frames):
        fx, fy, w, h = int(f[3]), int(f[4]), int(f[5]), int(f[6])
        ox, oy = place[i]
        for row in range(h):
            s = ((fy + row) * sw + fx) * 4
            d = ((oy + row) * WIDTH + ox) * 4
            out[d:d + w * 4] = px[s:s + w * 4]
        spr.append('\t'.join([f[0], f[1], f[2], str(ox), str(oy)] + f[5:]))
    with open(os.path.join(assets, OUT + '.tex'), 'wb') as f:
        f.write(struct.pack('<4sII', b'RLT1', WIDTH, height))
        f.write(out)
    with open(os.path.join(assets, OUT + '.spr'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(spr + keep) + '\n')
    print('%s: %d frames, %dx%d' % (OUT, len(frames), WIDTH, height))


if __name__ == '__main__':
    main()
