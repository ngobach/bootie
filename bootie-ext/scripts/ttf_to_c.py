#!/usr/bin/env python3
"""
Convert a TrueType/OpenType font file into a C byte-array header for
embedding in the bootie-ext bare-metal modules.

Usage:
    python3 ttf_to_c.py <input.ttf> [output.h]

If output is omitted, writes to stdout.
The generated header defines:
    static const unsigned char FONT_TTF_DATA[] = { ... };
    static const int FONT_TTF_SIZE = <size>;
"""

import sys
import os

def ttf_to_c(ttf_path, out_path=None):
    with open(ttf_path, 'rb') as f:
        data = f.read()

    lines = []
    lines.append('#ifndef FONT_TTF_HEADER_H')
    lines.append('#define FONT_TTF_HEADER_H')
    lines.append('')
    lines.append('static const unsigned char FONT_TTF_DATA[] = {')

    # Write bytes in rows of 12
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        hexes = ', '.join(f'0x{b:02x}' for b in chunk)
        lines.append(f'  {hexes},')

    lines.append('};')
    lines.append(f'static const int FONT_TTF_SIZE = {len(data)};')
    lines.append('')
    lines.append('#endif')

    output = '\n'.join(lines) + '\n'

    if out_path:
        with open(out_path, 'w') as f:
            f.write(output)
        print(f'Wrote {len(data)} bytes to {out_path}', file=sys.stderr)
    else:
        sys.stdout.write(output)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__, file=sys.stderr)
        sys.exit(1)
    ttf_to_c(*sys.argv[1:3])
