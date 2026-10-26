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
                  A name frame index duration               animation frame
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


def convert_texture(src, dst):
    data = open(src, 'rb').read()
    if data[:3] != b'XNB':
        raise ValueError('not an XNB file')
    flags = data[5]
    if flags & 0xC0:
        raise ValueError('compressed XNB files are not supported')
    pos = 10
    readers, pos = read_7bit(data, pos)
    for _ in range(readers):
        _, pos = read_string(data, pos)
        pos += 4
    _, pos = read_7bit(data, pos)  # shared resources
    _, pos = read_7bit(data, pos)  # type id of the asset
    fmt, w, h, levels = struct.unpack_from('<iIII', data, pos)
    pos += 16
    if fmt != 0:
        raise ValueError('surface format %d (only Color is handled)' % fmt)
    size, = struct.unpack_from('<I', data, pos)
    pos += 4
    px = data[pos:pos + size]
    out = bytearray(px[:w * h * 4])
    # RGBA -> BGRA (little-endian 0xAARRGGBB); XNA content is premultiplied
    out[0::4], out[2::4] = px[2:w * h * 4:4], px[0:w * h * 4:4]
    with open(dst, 'wb') as f:
        f.write(b'RLT1' + struct.pack('<II', w, h))
        f.write(out)
    return w, h


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
            lines.append('A\t%s\t%s\t%s\t%s' % (a['name'], a['frame'], a['index'], a['duration']))
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


if __name__ == '__main__':
    main()
