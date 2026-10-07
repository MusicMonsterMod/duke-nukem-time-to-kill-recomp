#!/usr/bin/env python3
"""Reduce a 16x16 RGBA icon to 15 colours + transparent for a 4bpp HUD cell.

D08A4 (steroids, like the D08A7 medkit): k-means in PSX 15-bit colour, no
dither. Pixels with alpha < 128 become transparent, the rest opaque. Output
colours are exact 5-bit values expanded to 8 bits ((v << 3) | (v >> 2)), so
tests/local/test_ui_art.py can re-derive the cell tables. Deterministic: the
centres start from the most common colours. Needs Pillow.
"""
import argparse
from collections import Counter
from pathlib import Path

from PIL import Image


def expand(v):
    return (v << 3) | (v >> 2)


def reduce(image, colours=15, rounds=50):
    im = image.convert('RGBA')
    pixels = list(im.getdata())
    opaque = [(r >> 3, g >> 3, b >> 3) for r, g, b, a in pixels if a >= 128]
    counts = Counter(opaque)
    centres = [c for c, _ in sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))[:colours]]
    centres = [tuple(float(v) for v in c) for c in centres]

    def nearest(c):
        return min(range(len(centres)), key=lambda i: sum((c[k] - centres[i][k]) ** 2 for k in range(3)))

    for _ in range(rounds):
        sums = [[0.0, 0.0, 0.0, 0] for _ in centres]
        for colour, n in counts.items():
            s = sums[nearest(colour)]
            for k in range(3):
                s[k] += colour[k] * n
            s[3] += n
        moved = [tuple(s[k] / s[3] for k in range(3)) if s[3] else centres[i] for i, s in enumerate(sums)]
        if moved == centres:
            break
        centres = moved
    snapped = [tuple(min(31, max(0, round(v))) for v in c) for c in centres]
    out = []
    for r, g, b, a in pixels:
        if a < 128:
            out.append((0, 0, 0, 0))
            continue
        c = snapped[nearest((r >> 3, g >> 3, b >> 3))]
        out.append((expand(c[0]), expand(c[1]), expand(c[2]), 255))
    result = Image.new('RGBA', im.size)
    result.putdata(out)
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('source', type=Path)
    ap.add_argument('output', type=Path)
    a = ap.parse_args()
    image = Image.open(a.source)
    if image.size != (16, 16):
        raise SystemExit('expected a 16x16 icon')
    reduce(image).save(a.output)
    print(f'wrote {a.output}')


if __name__ == '__main__':
    main()
