#!/usr/bin/env python3
"""D53: build the security monitor pack (Duke 3D-style TVs and camera frame).

Sources, all original project art tracked in assets/ui/security-monitor (made
in Photon Studio: tv-atlas.photon, camera-overlay.photon; no disc input, so
every build makes it):
  tv-atlas.png               - 256x256 RGBA texture atlas for the TV mesh, in
                               the style and colours of Duke 3D's security
                               screen and bezel (original pixels): bezel
                               (0,0 128x128, screen opening 16..111 clear),
                               screen with the cursor off (128,0 96x96) and
                               on (128,96 96x96), the box's top and bottom
                               (0,192 128x32) and sides (224,0 32x128), and
                               the recess wall (128,192 16x16);
  camera-overlay-frame.png   - 1600x1200 (the 4:3 picture): corner brackets
                               and REC, Duke 3D placement;
  camera-overlay-light.png   - 1600x1200: the record light alone.

Pack TTKSEC1: 'TTKSEC1\\0', u16 atlas width, u16 atlas height, then the atlas
as little-endian PS1 15-bit texels (0 = clear, opaque black = 0x8000); u32
piece count, then per piece u16 kind (0 frame, 1 record light), u16 x, u16 y,
u16 w, u16 h in the 1600x1200 picture and RGBA bytes. The frame is cut into
its four corners (bounding boxes of the quadrants). Standard library only;
deterministic.
"""
import argparse, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from build_ttk_inv_icons import read_rgba_png  # noqa: E402

ROOT = HERE.parents[1]
ART = ROOT / 'assets/ui/security-monitor'
ATLAS_ROWS = 256


def texel(r, g, b, a):
    # Alpha 255 opaque, 128..254 semi-transparent (the STP bit: the glow
    # ring's haze, drawn additively), below 128 clear.
    if a < 128:
        return 0
    v = (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10
    if a < 255:
        return v | 0x8000 if v else 0
    return v or 0x8000


def pieces(w, h, rgba, kind, split):
    out = []
    regions = [(0, 0, w // 2, h // 2), (w // 2, 0, w, h // 2), (0, h // 2, w // 2, h), (w // 2, h // 2, w, h)] if split else [(0, 0, w, h)]
    for x0, y0, x1, y1 in regions:
        xs, ys = [], []
        for y in range(y0, y1):
            row = rgba[y * w * 4:(y + 1) * w * 4]
            for x in range(x0, x1):
                if row[x * 4 + 3]:
                    xs.append(x); ys.append(y)
        if not xs:
            continue
        bx0, bx1, by0, by1 = min(xs), max(xs) + 1, min(ys), max(ys) + 1
        data = b''.join(rgba[(y * w + bx0) * 4:(y * w + bx1) * 4] for y in range(by0, by1))
        out.append(struct.pack('<5H', kind, bx0, by0, bx1 - bx0, by1 - by0) + data)
    return out


def build():
    w, h, rgba = read_rgba_png(ART / 'tv-atlas.png')
    if (w, h) != (256, 256):
        raise SystemExit('tv-atlas.png: need 256x256')
    atlas = bytearray(struct.pack('<2H', 256, ATLAS_ROWS))
    for y in range(ATLAS_ROWS):
        for x in range(256):
            atlas += struct.pack('<H', texel(*rgba[(y * w + x) * 4:(y * w + x) * 4 + 4]))
    parts = []
    for name, kind, split in (('camera-overlay-frame.png', 0, True), ('camera-overlay-light.png', 1, False)):
        fw, fh, frgba = read_rgba_png(ART / name)
        if (fw, fh) != (1600, 1200):
            raise SystemExit(f'{name}: need 1600x1200')
        parts += pieces(fw, fh, frgba, kind, split)
    return b'TTKSEC1\0' + bytes(atlas) + struct.pack('<I', len(parts)) + b''.join(parts)


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-security-monitor.pack'))
    a = ap.parse_args()
    data = build()
    Path(a.output).write_bytes(data)
    print(f'{a.output}: {len(data)} bytes')
