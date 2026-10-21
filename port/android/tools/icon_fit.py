#!/usr/bin/env python3
"""Fits the game's launcher icon to the wrapper's: crops the transparent margin old (2011) icons carry
and scales the rest to a square of the given size (bilinear). Plain zlib, no imaging library.
A frame drawn into the picture itself (MC3's white rounded border) is cut off with inset: that many percent
of the visible side from every edge; the launcher's own mask then gives the shape.
Usage: icon_fit.py <in.png> <out.png> <size> [inset %]     (8-bit RGB or RGBA, non-interlaced)"""
import struct, sys, zlib


def read_png(path):
    d = open(path, 'rb').read()
    assert d[:8] == b'\x89PNG\r\n\x1a\n', 'not a PNG'
    pos, idat, w = 8, b'', 0
    while pos < len(d):
        n, kind = struct.unpack('>I4s', d[pos:pos + 8])
        body = d[pos + 8:pos + 8 + n]
        if kind == b'IHDR':
            w, h, depth, ctype, _, _, inter = struct.unpack('>IIBBBBB', body)
            assert depth == 8 and ctype in (2, 6) and inter == 0, 'unsupported PNG layout'
        elif kind == b'IDAT':
            idat += body
        pos += 12 + n
    bpp = 4 if ctype == 6 else 3
    raw, stride = zlib.decompress(idat), w * bpp
    rows, prev, i = [], bytearray(stride), 0
    for _ in range(h):
        f, line = raw[i], bytearray(raw[i + 1:i + 1 + stride])
        i += 1 + stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0
            b = prev[x]
            c = prev[x - bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + b) & 255
            elif f == 3: line[x] = (line[x] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        rows.append(line)
        prev = line
    px = [[tuple(r[x * bpp:x * bpp + bpp]) + ((255,) if bpp == 3 else ()) for x in range(w)] for r in rows]
    return w, h, px


def write_png(path, size, px):
    raw = b''.join(b'\x00' + bytes(v for p in row for v in p) for row in px)
    def chunk(k, b):
        return struct.pack('>I', len(b)) + k + b + struct.pack('>I', zlib.crc32(k + b) & 0xffffffff)
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', size, size, 8, 6, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def main():
    src, dst, size = sys.argv[1], sys.argv[2], int(sys.argv[3])
    inset = float(sys.argv[4]) if len(sys.argv) > 4 else 0.0
    w, h, px = read_png(src)
    # The visible part: alpha above a faint threshold (drop shadows and anti-aliasing fringes stay out).
    ys = [y for y in range(h) if any(p[3] > 24 for p in px[y])]
    xs = [x for x in range(w) if any(px[y][x][3] > 24 for y in range(h))]
    x0, x1, y0, y1 = min(xs), max(xs) + 1, min(ys), max(ys) + 1
    side = max(x1 - x0, y1 - y0) * (1 - 2 * inset / 100)
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    out = []
    for oy in range(size):
        row = []
        for ox in range(size):
            sx = cx - side / 2 + (ox + 0.5) * side / size - 0.5
            sy = cy - side / 2 + (oy + 0.5) * side / size - 0.5
            ix, iy = int(sx // 1), int(sy // 1)
            fx, fy = sx - ix, sy - iy
            acc = [0.0] * 4
            for dy, wy in ((0, 1 - fy), (1, fy)):
                for dx, wx in ((0, 1 - fx), (1, fx)):
                    xx, yy = min(max(ix + dx, 0), w - 1), min(max(iy + dy, 0), h - 1)
                    p = px[yy][xx]
                    k = wx * wy
                    # premultiplied, so transparent neighbours do not darken the edge
                    acc[0] += p[0] * p[3] * k; acc[1] += p[1] * p[3] * k; acc[2] += p[2] * p[3] * k; acc[3] += p[3] * k
            a = acc[3]
            row.append((int(acc[0] / a + .5), int(acc[1] / a + .5), int(acc[2] / a + .5), int(a + .5)) if a > 0 else (0, 0, 0, 0))
        out.append(row)
    write_png(dst, size, out)
    print('icon: %dx%d, visible %d..%d x %d..%d, inset %g%% -> %dx%d' % (w, h, x0, x1, y0, y1, inset, size, size))


if __name__ == '__main__':
    main()
