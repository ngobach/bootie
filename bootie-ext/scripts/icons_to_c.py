#!/usr/bin/env python3
"""Embed all 16x16 PNGs in a directory as raw byte arrays in a C header.

Usage: python3 icons_to_c.py <icons_dir> <output_header>

Each icon's raw PNG data is stored as `unsigned char ICON_NAME_PNG[]`.
At runtime, decode with gfx_png_decode() and draw with gfx_draw_image().
"""

import sys
import os
import glob


def var_name(filename):
    name, _ = os.path.splitext(filename)
    parts = name.replace('-', '_').upper().split('_')
    return 'ICON_' + '_'.join(parts) + '_PNG'


def c_array(data, varname):
    lines = [f'static const unsigned char {varname}[{len(data)}] = {{']
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        hexes = ', '.join(f'0x{b:02x}' for b in chunk)
        lines.append(f'    {hexes},')
    lines.append('};')
    return lines


def main():
    if len(sys.argv) < 3:
        print("Usage: python3 icons_to_c.py <icons_dir> <output_header>", file=sys.stderr)
        sys.exit(1)

    icons_dir = sys.argv[1]
    out_path = sys.argv[2]

    pngs = sorted(glob.glob(os.path.join(icons_dir, '*.png')))
    if not pngs:
        print(f"No PNGs found in {icons_dir}", file=sys.stderr)
        sys.exit(1)

    lines = []
    lines.append('#ifndef BOOTIE_ICONS_H')
    lines.append('#define BOOTIE_ICONS_H')
    lines.append('')

    for png in pngs:
        basename = os.path.basename(png)
        vname = var_name(basename)
        with open(png, 'rb') as f:
            data = f.read()
        lines.append(f'/* {basename} */')
        lines.extend(c_array(data, vname))
        lines.append('')

    lines.append('#endif /* BOOTIE_ICONS_H */')
    lines.append('')

    with open(out_path, 'w') as f:
        f.write('\n'.join(lines))

    print(f"Wrote {len(pngs)} icon(s) to {out_path}", file=sys.stderr)


if __name__ == '__main__':
    main()
