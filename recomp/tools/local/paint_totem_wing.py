#!/usr/bin/env python3
"""D61B: paint True Grit's totem wing texture (original art, the project's own).

True Grit's two totem poles (room 25, model 87) load with the faces of their
outstretched wings untextured (CLUT 0xffff; the level ships no wing texture),
so they drew the frame buffer. This paints a 96x32 carved-feather band in the
level's own 16-step gold ramp order (index 0 brightest, 15 black; the game
draws it through the totem's CLUT 0x1438), as layered feathers with dark
outlines and a carved rib, so any part of it reads as carved wood and gold
like the pole itself. The palette stored in the PNG is our own preview ramp.
Output: recomp/assets/hidden-levels/true-grit-totem-wing.png (indexed).
"""
from pathlib import Path
from PIL import Image

W, H = 96, 32
OUT = Path(__file__).resolve().parents[2] / 'assets/hidden-levels/true-grit-totem-wing.png'
PREVIEW = [(250, 226, 100), (246, 206, 84), (238, 184, 82), (228, 168, 66), (198, 150, 90), (182, 128, 52),
           (150, 98, 42), (142, 82, 28), (112, 72, 34), (102, 58, 18), (74, 56, 34), (72, 48, 18),
           (70, 40, 10), (40, 24, 10), (16, 8, 2), (0, 0, 0)]


def main():
    px = [[15] * W for _ in range(H)]
    # Three rows of overlapping feathers, longer toward the wing tip (right).
    rows = [(0, 11, 9), (10, 22, 11), (21, 32, 13)]
    for r, (top, bottom, step) in enumerate(rows):
        for start in range(-step, W, step):
            for x in range(start, start + step + 3):
                if not 0 <= x < W:
                    continue
                t = (x - start) / (step + 2)            # 0..1 across the feather
                for y in range(top, bottom):
                    s = (y - top) / max(1, bottom - top - 1)   # 0 root .. 1 tip
                    edge = abs(t - 0.5) * 2 > 1 - 0.35 * s     # the feather narrows to its tip
                    if edge:
                        continue
                    shade = 1 + int(5 * abs(t - 0.45)) + int(4 * s) + r
                    px[y][x] = min(12, shade)
            # outlines: the feather's leading edge and its shaft
            for y in range(top, bottom):
                for x in (start, start + step // 2):
                    if 0 <= x < W and px[y][x] != 15:
                        px[y][x] = 13 if x == start else 9
        # the carved rib under each row
        for x in range(W):
            if bottom - 1 < H:
                px[bottom - 1][x] = 14
            if bottom < H and x % 4 == 0:
                px[bottom][x] = 3
    img = Image.new('P', (W, H))
    flat = []
    for c in PREVIEW:
        flat += c
    img.putpalette(flat + [0] * (768 - len(flat)))
    img.putdata([v for row in px for v in row])
    OUT.parent.mkdir(parents=True, exist_ok=True)
    img.save(OUT)
    print(OUT)


if __name__ == '__main__':
    main()
