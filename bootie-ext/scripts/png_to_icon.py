#!/usr/bin/env python3
"""Convert a 16x16 PNG to a C uint16_t bitmap array for bootie-icons.h.

Usage: python3 png_to_icon.py <input.png> [varname]

Reads a 16x16 RGBA PNG and writes a C array to stdout.
Each row is packed as a uint16_t, bit 0 = leftmost pixel, 1 = opaque.
"""

import struct
import zlib
import sys


def parse_png(path):
    with open(path, 'rb') as f:
        data = f.read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n'
    pos = 8
    chunks = []
    while pos < len(data):
        length = struct.unpack('>I', data[pos:pos+4])[0]
        pos += 4
        typ = data[pos:pos+4]
        pos += 4
        cd = data[pos:pos+length]
        pos += length
        pos += 4
        chunks.append((typ, cd))

    ihdr = chunks[0][1]
    width = struct.unpack('>I', ihdr[0:4])[0]
    height = struct.unpack('>I', ihdr[4:8])[0]
    color_type = ihdr[9]
    return width, height, color_type, chunks


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 png_to_icon.py <input.png> [varname]", file=sys.stderr)
        sys.exit(1)

    path = sys.argv[1]
    var = sys.argv[2] if len(sys.argv) > 2 else "ICON"

    width, height, color_type, chunks = parse_png(path)
    if width != 16 or height != 16:
        print(f"Error: expected 16x16, got {width}x{height}", file=sys.stderr)
        sys.exit(1)
    if color_type != 6:
        print(f"Error: expected RGBA (color type 6), got {color_type}", file=sys.stderr)
        sys.exit(1)

    idat_data = b''
    for t, d in chunks:
        if t == b'IDAT':
            idat_data += d
    raw = zlib.decompress(idat_data)

    bpp, stride = 4, width * 4

    def recon(ft, rd, prev):
        out = bytearray(rd)
        if ft == 1:
            for i in range(bpp, len(out)):
                out[i] = (out[i] + out[i - bpp]) & 0xFF
        elif ft == 2:
            for i in range(len(out)):
                out[i] = (out[i] + prev[i]) & 0xFF
        elif ft == 3:
            for i in range(len(out)):
                l = out[i - bpp] if i >= bpp else 0
                u = prev[i]
                out[i] = (out[i] + (l + u) // 2) & 0xFF
        elif ft == 4:
            for i in range(len(out)):
                l = out[i - bpp] if i >= bpp else 0
                u = prev[i]
                ul = prev[i - bpp] if i >= bpp else 0
                p = l + u - ul
                pa, pb, pc = abs(p - l), abs(p - u), abs(p - ul)
                pr = l if pa <= pb and pa <= pc else (u if pb <= pc else ul)
                out[i] = (out[i] + pr) & 0xFF
        return out

    prev = bytearray(stride)
    pos = 0
    print(f"static const uint16_t {var}[16] = {{")
    for y in range(height):
        ft = raw[pos]
        pos += 1
        rd = raw[pos:pos + stride]
        pos += stride
        r = recon(ft, rd, prev)
        prev = r
        bits = 0
        for x in range(width):
            if r[x * 4 + 3] > 128:
                bits |= 1 << x
        print(f"    0x{bits:04x},")
    print("};")


if __name__ == "__main__":
    main()
