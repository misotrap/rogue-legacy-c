#!/usr/bin/env python3
"""Converts the Rogue Legacy content of your own Steam copy into the simple
files the C port loads. Runs once, on the computer (stdlib only).

    python3 tools/convert_assets.py "<.../Rogue Legacy.app/Contents/Resources/Content>" assets

Output (in assets/):
  <sheet>.tex   'RLT1', uint32 width, uint32 height, then width*height
                pixels, 4 bytes each (B, G, R, A), alpha premultiplied.
                Full size: the port draws the 1320x720 world at 1/2 scale and
                most objects (player, enemies) at scale 2, so their texels
                land 1:1 on the screen. Only load the sheets a scene needs.
  <sheet>.spr   text, one record per line, tab separated, coordinates in
                ORIGINAL (full size) pixels:
                  I name index x y w h anchorX anchorY      frame image
                  A name frame index duration label         animation frame
                                                            (label is the original XML
                                                            label, used by
                                                            PlayAnimation("Start",
                                                            "End") in the C#)
                  R name frame x y w h rotation type        hitbox of the
                                                            previous A line
                  C name child childX childY childRotation  character part
  rooms.txt     text, tab separated:
                  M mapfile                                  start of a map file
                  ROOM key=value ...                         a room template
                  G Type key=value ...                       object in it
                Every XML attribute is kept, so nothing is lost.
"""
import os
import shutil
import struct
import sys
import xml.etree.ElementTree as ET


def read_7bit(data, pos):
    result = shift = 0
    while True:
        b = data[pos]
        pos += 1
        result |= (b & 0x7F) << shift
        if not b & 0x80:
            return result, pos
        shift += 7


def read_string(data, pos):
    n, pos = read_7bit(data, pos)
    return data[pos:pos + n].decode('utf-8'), pos + n


def decode_dxt5(data, w, h):
    """DXT5 (BC3) blocks -> RGBA bytes (same algorithm as FNA's DxtUtil.cs)."""
    out = bytearray(w * h * 4)
    bw, bh = (w + 3) // 4, (h + 3) // 4
    p = 0
    for by in range(bh):
        for bx in range(bw):
            a0, a1 = data[p], data[p + 1]
            abits = int.from_bytes(data[p + 2:p + 8], 'little')
            c0, c1, cbits = struct.unpack_from('<HHI', data, p + 8)
            p += 16
            alphas = [a0, a1]
            if a0 > a1:
                alphas += [((6 - i) * a0 + (1 + i) * a1) // 7 for i in range(6)]
            else:
                alphas += [((4 - i) * a0 + (1 + i) * a1) // 5 for i in range(4)] + [0, 255]

            def rgb(c):
                r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
                return (r << 3 | r >> 2, g << 2 | g >> 4, b << 3 | b >> 2)
            k0, k1 = rgb(c0), rgb(c1)
            cols = [k0, k1,
                    tuple((2 * x + y) // 3 for x, y in zip(k0, k1)),
                    tuple((x + 2 * y) // 3 for x, y in zip(k0, k1))]
            for j in range(4):
                y = by * 4 + j
                if y >= h:
                    break
                for i in range(4):
                    x = bx * 4 + i
                    if x >= w:
                        continue
                    n = j * 4 + i
                    r, g, b = cols[(cbits >> (2 * n)) & 3]
                    o = (y * w + x) * 4
                    out[o:o + 4] = bytes((r, g, b, alphas[(abits >> (3 * n)) & 7]))
    return bytes(out)


def decode_dxt3(data, w, h):
    """DXT3 (BC2) blocks -> RGBA bytes: explicit 4-bit alpha, DXT1 colours."""
    out = bytearray(w * h * 4)
    p = 0
    for by in range((h + 3) // 4):
        for bx in range((w + 3) // 4):
            abits = int.from_bytes(data[p:p + 8], 'little')
            c0, c1, cbits = struct.unpack_from('<HHI', data, p + 8)
            p += 16

            def rgb(c):
                r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
                return (r << 3 | r >> 2, g << 2 | g >> 4, b << 3 | b >> 2)
            k0, k1 = rgb(c0), rgb(c1)
            cols = [k0, k1,
                    tuple((2 * x + y) // 3 for x, y in zip(k0, k1)),
                    tuple((x + 2 * y) // 3 for x, y in zip(k0, k1))]
            for j in range(4):
                y = by * 4 + j
                if y >= h:
                    break
                for i in range(4):
                    x = bx * 4 + i
                    if x >= w:
                        continue
                    n = j * 4 + i
                    r, g, b = cols[(cbits >> (2 * n)) & 3]
                    a = ((abits >> (4 * n)) & 15) * 17
                    o = (y * w + x) * 4
                    out[o:o + 4] = bytes((r, g, b, a))
    return bytes(out)


def read_xnb_header(data):
    """Returns the position of the first object in an uncompressed XNB."""
    if data[:3] != b'XNB':
        raise ValueError('not an XNB file')
    if data[5] & 0xC0:
        raise ValueError('compressed XNB files are not supported')
    pos = 10
    readers, pos = read_7bit(data, pos)
    for _ in range(readers):
        _, pos = read_string(data, pos)
        pos += 4
    _, pos = read_7bit(data, pos)  # shared resources
    return pos


def read_texture_body(data, pos):
    """Reads a Texture2D (after its type id): returns w, h, RGBA bytes, new pos."""
    fmt, w, h, levels = struct.unpack_from('<iIII', data, pos)
    pos += 16
    if fmt not in (0, 5, 6):
        raise ValueError('surface format %d (only Color, Dxt3 and Dxt5 are handled)' % fmt)
    first = None
    for _ in range(levels):
        size, = struct.unpack_from('<I', data, pos)
        pos += 4
        if first is None:
            first = data[pos:pos + size]
        pos += size
    if fmt == 5:
        first = decode_dxt3(first, w, h)
    elif fmt == 6:
        first = decode_dxt5(first, w, h)
    return w, h, first, pos


def write_tex(dst, w, h, px):
    out = bytearray(px[:w * h * 4])
    # RGBA -> BGRA (little-endian 0xAARRGGBB); XNA content is premultiplied
    out[0::4], out[2::4] = px[2:w * h * 4:4], px[0:w * h * 4:4]
    with open(dst, 'wb') as f:
        f.write(b'RLT1' + struct.pack('<II', w, h))
        f.write(out)


def convert_texture(src, dst):
    data = open(src, 'rb').read()
    pos = read_xnb_header(data)
    _, pos = read_7bit(data, pos)  # type id of the asset
    w, h, px, _ = read_texture_body(data, pos)
    write_tex(dst, w, h, px)
    return w, h


def convert_font(src, dst_base):
    """XNA SpriteFont -> <name>.tex (glyph sheet) + <name>.fnt (text):
         F lineSpacing spacing defaultChar
         G codepoint glyphX glyphY glyphW glyphH cropX cropY cropW cropH kernA kernB kernC
       (SpriteFontReader: texture, glyph rects, croppings, characters,
       line spacing, spacing, kerning, default character)."""
    data = open(src, 'rb').read()
    pos = read_xnb_header(data)
    _, pos = read_7bit(data, pos)  # SpriteFont
    _, pos = read_7bit(data, pos)  # Texture2D
    w, h, px, pos = read_texture_body(data, pos)

    def rect_list(pos):
        _, pos = read_7bit(data, pos)
        n, = struct.unpack_from('<i', data, pos)
        pos += 4
        rects = [struct.unpack_from('<iiii', data, pos + 16 * i) for i in range(n)]
        return rects, pos + 16 * n

    glyphs, pos = rect_list(pos)
    crops, pos = rect_list(pos)
    _, pos = read_7bit(data, pos)  # List<char>
    n, = struct.unpack_from('<i', data, pos)
    pos += 4
    chars = []
    for _ in range(n):
        # CharReader: one UTF-8 encoded character
        b = data[pos]
        size = 1 if b < 0x80 else 2 if b < 0xE0 else 3 if b < 0xF0 else 4
        chars.append(ord(data[pos:pos + size].decode('utf-8')))
        pos += size
    line_spacing, spacing = struct.unpack_from('<if', data, pos)
    pos += 8
    _, pos = read_7bit(data, pos)  # List<Vector3>
    n, = struct.unpack_from('<i', data, pos)
    pos += 4
    kerning = [struct.unpack_from('<fff', data, pos + 12 * i) for i in range(n)]
    pos += 12 * n
    default_char = 0
    if data[pos]:
        default_char = ord(data[pos + 1:pos + 5].decode('utf-8', 'ignore')[:1] or '\0')
    write_tex(dst_base + '.tex', w, h, px)
    with open(dst_base + '.fnt', 'w') as f:
        f.write('F %d %g %d\n' % (line_spacing, spacing, default_char))
        for c, g, cr, k in zip(chars, glyphs, crops, kerning):
            f.write('G %d %d %d %d %d %d %d %d %d %g %g %g\n' % ((c,) + tuple(g) + tuple(cr) + tuple(k)))
    return len(chars)


def convert_sheet_xml(src, dst):
    root = ET.parse(src).getroot()
    lines = []
    for el in root:
        a = el.attrib
        if el.tag == 'instance':
            lines.append('I\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s' % (
                a['name'], a['index'], a['posX'], a['posY'], a['width'], a['height'],
                a['anchorX'], a['anchorY']))
        elif el.tag == 'animation':
            lines.append('A\t%s\t%s\t%s\t%s\t%s' % (
                a['name'], a['frame'], a['index'], a['duration'], a.get('label', '')))
            for r in el:
                if r.tag == 'rectangle':
                    ra = r.attrib
                    lines.append('R\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s' % (
                        a['name'], a['frame'], ra['x'], ra['y'], ra['width'], ra['height'],
                        ra.get('rotation', '0'), ra.get('type', '0')))
        elif el.tag == 'container':
            lines.append('C\t%s\t%s\t%s\t%s\t%s' % (
                a['name'], a['child'], a['childX'], a['childY'], a.get('childRotation', '0')))
    with open(dst, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    return len(lines)


def attrs(el):
    # Tabs and newlines never appear in values; '=' may, so split on the first one
    return '\t'.join('%s=%s' % (k, v.replace('\t', ' ')) for k, v in el.attrib.items())


def convert_rooms(levels_dir, dst):
    count = 0
    with open(dst, 'w') as f:
        for name in sorted(os.listdir(levels_dir)):
            if not name.endswith('.xml'):
                continue
            root = ET.parse(os.path.join(levels_dir, name)).getroot()
            f.write('M\t%s\n' % name)
            for room in root.iter('RoomObject'):
                f.write('ROOM\t%s\n' % attrs(room))
                count += 1
                for obj in room.iter('GameObject'):
                    f.write('G\t%s\t%s\n' % (obj.attrib.get('Type', '?'), attrs(obj)))
    return count


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    content, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    sheets = os.path.join(content, 'GameSpritesheets')
    for name in sorted(os.listdir(sheets)):
        base, ext = os.path.splitext(name)
        src = os.path.join(sheets, name)
        if ext == '.xnb':
            try:
                w, h = convert_texture(src, os.path.join(out, base + '.tex'))
                print('texture %-32s %dx%d' % (base, w, h))
            except ValueError as e:
                print('skipped %s: %s' % (name, e))
        elif ext == '.xml':
            n = convert_sheet_xml(src, os.path.join(out, base + '.spr'))
            print('sprites %-32s %d records' % (base, n))
    n = convert_rooms(os.path.join(content, 'Levels'), os.path.join(out, 'rooms.txt'))
    print('rooms: %d templates' % n)
    # Fonts (src/Text): assets/fonts/<name>.tex + .fnt
    fonts = os.path.join(content, 'Fonts')
    os.makedirs(os.path.join(out, 'fonts'), exist_ok=True)
    for name in sorted(os.listdir(fonts)):
        base, ext = os.path.splitext(name)
        if ext != '.xnb':
            continue
        try:
            n = convert_font(os.path.join(fonts, name), os.path.join(out, 'fonts', base))
            print('font    %-32s %d characters' % (base, n))
        except (ValueError, struct.error, UnicodeDecodeError) as e:
            print('skipped font %s: %s' % (name, e))
    # Heir names (Lineage::Generator::LoadNames)
    for names in ('HeroNames.txt', 'HeroineNames.txt'):
        shutil.copyfile(os.path.join(content, names), os.path.join(out, names))
    print('names: HeroNames.txt, HeroineNames.txt')


if __name__ == '__main__':
    main()
