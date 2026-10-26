#!/usr/bin/env python3
"""Converts the constant classes of the reference C# source (*EV.cs and the
simple Types/*.cs files) into C++17 headers, so game rules can be ported
with the exact same names (GameEV.KNIGHT_BLOCK_DRAIN etc.).

    python3 tools/convert_ev.py ref/RogueLegacy1 src

Generated headers are COMMITTED (ref/ is git-ignored), so building does not
require the reference source. Re-run this after updating ref/ only.

Handles: public const/static fields of int, float, bool, byte, string,
Vector2, Vector3, Color, and arrays (int[], byte[], string[]). Anything else
(methods, string[,], ProjectileData, GameTypes enums, ...) is emitted as a
'TODO C++' comment and reported on stderr so it can be ported by hand.

C# classes become C++ namespaces (GameEV.X -> GameEV::X in ported code).
"""
import os
import re
import sys

# C# identifiers whose '.' must become '::' (class-as-namespace conversions).
KNOWN = ['EnemyType', 'BossRoomTypes', 'Consts', 'GlobalEV', 'GameEV',
         'LevelEV', 'PlayerEV', 'EnemyEV', 'SpellEV', 'PlayerPart',
         'EquipmentCategoryType', 'ClassType', 'TraitType', 'SkillType']

# Files to convert, relative to the repo root: (source .cs, output .h)
FILES = [
    ('RogueCastle/src/EVs/GlobalEV.cs', 'src/RogueCastle/EVs/GlobalEV.h'),
    ('RogueCastle/src/EVs/GameEV.cs', 'src/RogueCastle/EVs/GameEV.h'),
    ('RogueCastle/src/EVs/LevelEV.cs', 'src/RogueCastle/EVs/LevelEV.h'),
    ('RogueCastle/src/EVs/PlayerEV.cs', 'src/RogueCastle/EVs/PlayerEV.h'),
    ('RogueCastle/src/EVs/EnemyEV.cs', 'src/RogueCastle/EVs/EnemyEV.h'),
    ('RogueCastle/src/EVs/SpellEV.cs', 'src/RogueCastle/EVs/SpellEV.h'),
    ('DS2DEngine/src/EngineEV.cs', 'src/DS2DEngine/EngineEV.h'),
    ('DS2DEngine/src/Consts.cs', 'src/DS2DEngine/Consts.h'),
    ('RogueCastle/src/Types/EnemyType.cs', 'src/RogueCastle/Types/EnemyType.h'),
    ('RogueCastle/src/Types/BossRoomTypes.cs', 'src/RogueCastle/Types/BossRoomTypes.h'),
    ('RogueCastle/src/Types/PlayerPart.cs', 'src/RogueCastle/Types/PlayerPart.h'),
]

CPP_TYPE = {'int': 'int', 'float': 'float', 'double': 'double', 'bool': 'bool',
            'byte': 'uint8_t', 'sbyte': 'int8_t', 'short': 'short',
            'long': 'long long', 'uint': 'unsigned int',
            'string': 'const char* const',
            'Vector2': 'Vector2', 'Vector3': 'Vector3', 'Color': 'Color'}

SKIP_STMT = re.compile(r'^(using\s|namespace\s+\w+\s*$|public\s+(partial\s+)?class\s+\w+\s*$)')
STMT = re.compile(r'public (const|static) ([A-Za-z_][\w\.\[\],]*)\s+(\w+)\s*=\s*(.*)$', re.S)


def scan(text):
    """One pass over the source: returns a list of items, in file order.
    ('STMT', statement, trailing_comment) for top-level ';'-statements,
    ('COMMENT', text) for standalone line comments."""
    items = []
    buf = ''
    pending = None   # trailing comment of the statement currently in buf
    depth = 0
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == '\\':
                    j += 2
                    continue
                if text[j] == '"':
                    break
                j += 1
            buf += text[i:j + 1]
            i = j + 1
            continue
        if c == '/' and text[i + 1:i + 2] == '/':
            j = text.find('\n', i)
            if j == -1:
                j = n
            comment = text[i + 2:j].rstrip()
            if buf.strip():
                pending = comment if pending is None else pending
            elif comment:
                items.append(('COMMENT', comment, None))
            i = j
            continue
        if c == '/' and text[i + 1:i + 2] == '*':
            j = text.find('*/', i + 2)
            i = n if j == -1 else j + 2
            continue
        if c == '{':
            # namespace/class wrapper braces are transparent.
            if re.search(r'(namespace\s+\w+|class\s+\w+)$', buf.strip()):
                buf = ''
            else:
                depth += 1
                buf += c
        elif c == '}':
            if depth == 0:
                buf = ''   # class/namespace closer
            else:
                depth -= 1
                buf += c
        else:
            buf += c
        if c == ';' and depth == 0:
            stmt = buf.strip().rstrip(';').strip()
            buf = ''
            comment, pending = pending, None
            i += 1
            if not stmt or SKIP_STMT.match(stmt):
                continue
            items.append(('STMT', stmt, comment))
            continue
        i += 1
    if buf.strip():
        items.append(('STMT', buf.strip(), None))
    return items


def cs_value(value):
    for name in KNOWN:
        value = re.sub(r'\b%s\.' % name, '%s::' % name, value)
    value = value.replace('Vector2.One', 'Vector2(1, 1)')
    value = value.replace('Vector2.Zero', 'Vector2(0, 0)')
    value = re.sub(r'new (Vector2|Vector3|Color)', r'\1', value)
    # C# integer-float suffix (30f) is invalid C++; add the dot.
    value = re.sub(r'(?<![\d.])\d+f\b', lambda m: m.group(0)[:-1] + '.0f', value)
    return value.strip()


def convert(text, name, warnings):
    body = []
    for kind, stmt, comment in scan(text):
        suffix = '  //%s' % comment if comment else ''
        if kind == 'COMMENT':
            body.append('    //%s' % stmt)
            continue
        m = STMT.match(stmt)
        if not m:
            warnings.append('%s: skipped statement: %s' % (name, stmt[:70].replace('\n', ' ')))
            body.append('    // TODO C++: %s%s' % (stmt.replace('\n', ' '), suffix))
            continue
        kind_cs, ctype, cname, value = m.groups()
        value = cs_value(value)
        if ctype.endswith('[]'):
            elem = ctype[:-2]
            value = re.sub(r'^new\s+[A-Za-z_][\w\.\[\],]*\s*', '', value)
            if elem == 'string':
                cpp = 'inline const char* const'
            elif kind_cs == 'const':
                cpp = 'inline constexpr ' + CPP_TYPE[elem]
            else:
                cpp = 'inline ' + CPP_TYPE[elem]
            body.append('    %s %s[] = %s;%s' % (cpp, cname, value, suffix))
        elif ctype in CPP_TYPE:
            t = CPP_TYPE[ctype]
            if kind_cs == 'const':
                body.append('    constexpr %s %s = %s;%s' % (t, cname, value, suffix))
            elif ctype == 'string':
                body.append('    inline const char* %s = %s;%s' % (cname, value, suffix))
            else:
                body.append('    inline %s %s = %s;%s' % (t, cname, value, suffix))
        else:
            warnings.append('%s: skipped %s %s' % (name, ctype, cname))
            body.append('    // TODO C++: %s;%s' % (stmt.replace('\n', ' '), suffix))
    return body


def main():
    ref, src = sys.argv[1], sys.argv[2]
    warnings = []
    for cs, dst in FILES:
        text = open(os.path.join(ref, cs), encoding="utf-8-sig").read().replace("\r\n", "\n")
        name = os.path.basename(cs)[:-3]
        body = convert(text, name, warnings)
        header = os.path.basename(dst)[:-2].upper()
        rel = os.path.relpath('src/DS2DEngine/Types.h', os.path.dirname(dst))
        out = ['// Generated by tools/convert_ev.py from %s -- DO NOT EDIT.' % cs,
               '#pragma once',
               '#include "%s"' % rel,
               '',
               '// C# class %s -> C++ namespace %s (C# %s.X == C++ %s::X).' % (name, name, name, name),
               'namespace %s' % name,
               '{']
        out.extend(body)
        out.append('}')
        os.makedirs(os.path.dirname(dst) or '.', exist_ok=True)
        with open(dst, 'w') as f:
            f.write('\n'.join(out) + '\n')
        print('wrote %s' % dst)
    if warnings:
        print()
        for w in warnings:
            print('WARNING:', w)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
