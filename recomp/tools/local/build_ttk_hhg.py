#!/usr/bin/env python3
"""D08A51: the Holy Hand Grenade pickup for levels that never load it.

The ACCURATE grenade look (D08A47, recomp/src/ttk/hhg_look.inc) draws the held
and thrown grenade with the pickup's model (type 608, model 1129). TIME TO
KILL (level index 0) loads the held (945) and thrown (944) models but not the
pickup: its model file DB00MOD.BIN has no model 1129 and the level's model
table entry stays 0. documentation/520-d08a47-holy-hand-grenade-look.md.

This pack gives such a level the pickup as level 6 ships it (DB06MOD.BIN):
its vertices and its polygon list, with each textured face's CLUT, page and
UVs moved onto the grenade's two 16x24 texture boxes, copied (with their two
CLUTs) from level 6's raw images into free VRAM of the level. Faces whose
CLUT is the first box's (0xc7a) take the first, the others the second. Every
place was chosen all zero in the level's raw images (with a 2-texel margin) and
zero in VRAM ten seconds into the level (D08A51 lab); the builder checks the
zeros, the texture page and overlaps again.

Output: a TTKHHG01 pack (little endian): magic, version 1, level count; per
level: index, vertex count (u16), polygon list size in words (u16), upload
count (u32); the vertices (4 x s16 each), the polygon list (u32 words, ending
with the 0xff group), then per upload x, y, w, h (u16) and w*h words.
Retail-derived: build output only, never committed. Loaded by hhg_look.inc.
"""
import argparse, hashlib, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402
import build_ttk_look as look  # noqa: E402

ROOT = HERE.parents[1]
MODEL, DONOR = 1129, 6
# Level 6's two boxes: (u0, v0) on page 0x2b, the box's VRAM words, its CLUT.
BOXES = (dict(clut=0x0c7a, u0=176, v0=48, words=(748, 48, 4, 24), clut_at=(928, 49)),
         dict(clut=0x0cba, u0=216, v0=48, words=(758, 48, 4, 24), clut_at=(928, 50)))
PAGE = 0x2b & 0x1f   # page 0x2b (blend bits above the 5-bit page)
# level -> (block x, y: 8x24 words for both boxes side by side), (CLUT x, y: two rows).
PLACES = {0: ((852, 136), (784, 122))}


def mesh_bytes(mod, ident):
    """The model's vertex words and polygon list words, as in its model file."""
    offset = struct.unpack_from('<I', mod, 4 * ident)[0]
    if offset == 0xffffffff:
        raise SystemExit(f'model {ident} missing')
    m = look.MODEL_TABLE + offset
    count = mod[m + 6]
    vp, fp = (struct.unpack_from('<I', mod, m + k)[0] for k in (0x10, 0x14))
    verts = [struct.unpack_from('<4h', mod, m + vp + 8 * i) for i in range(count)]
    words, cur = [], m + fp
    while True:
        header = struct.unpack_from('<I', mod, cur)[0]
        words.append(header)
        cur += 4
        kind, n = header & 0xffff, header >> 16
        if kind == 0xff:
            return verts, words
        if kind not in look.RECORD:
            raise SystemExit(f'model {ident}: unknown polygon group {header:#x}')
        size = look.RECORD[kind]
        recs = list(struct.unpack_from(f'<{n * size // 4}I', mod, cur))
        if kind in (0x24, 0x2c):
            for k in range(n):
                recs[k * size // 4:(k + 1) * size // 4] = relocate.face(kind, recs[k * size // 4:(k + 1) * size // 4])
        words += recs
        cur += n * size


class Relocate:
    """Moves a textured face from level 6's boxes onto the level's placed copies."""
    def __init__(self):
        self.places = None

    def face(self, kind, w):
        w = list(w)
        clut, page = w[1] >> 16, (w[2] >> 16) & 0x1f
        box = next((i for i, b in enumerate(BOXES) if b['clut'] == clut), None)
        if box is None or page != PAGE:
            raise SystemExit(f'model {MODEL}: face with CLUT {clut:#x} page {page:#x}')
        b = BOXES[box]
        (bx, by), (cx, cy) = self.places
        px = bx + 4 * box
        du, dv = (px % 64) * 4 - b['u0'], by % 256 - b['v0']
        def uv(x):
            u, v = (x & 0xff) + du, ((x >> 8) & 0xff) + dv
            if not (0 <= u < 256 and 0 <= v < 256):
                raise SystemExit('UV outside the page')
            return v << 8 | u
        new_clut = (cy + box) << 6 | cx // 16
        new_page = ((w[2] >> 16) & ~0x1f) | look.page5(px, by)
        w[1] = uv(w[1] & 0xffff) | new_clut << 16
        w[2] = uv(w[2] & 0xffff) | new_page << 16
        w[3] = uv(w[3] & 0xffff) | ((uv(w[3] >> 16) if kind == 0x2c else w[3] >> 16) << 16)
        return w


relocate = Relocate()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-hhg.pack'))
    args = ap.parse_args()

    disc = Disc(args.image)
    try:
        files = {f['path']: f for f in disc.files()}
        def read(path):
            e = files.get(path)
            if not e:
                raise SystemExit(f'{path} not found')
            return disc.read(e['lba'], e['size'])
        levels = sorted(PLACES)
        vram = {lv: look.Vram(read(look.raw_path(lv, 'A')), read(look.raw_path(lv, 'B'))) for lv in levels + [DONOR]}
        donor_mod = read(look.raw_path(DONOR, 'MOD').replace('MOD.RAW', 'MOD.BIN'))
        mods = {lv: read(look.raw_path(lv, 'MOD').replace('MOD.RAW', 'MOD.BIN')) for lv in levels}
    finally:
        disc.close()

    out = bytearray(b'TTKHHG01') + struct.pack('<II', 1, len(PLACES))
    for level in levels:
        if struct.unpack_from('<I', mods[level], 4 * MODEL)[0] != 0xffffffff:
            raise SystemExit(f'level {level} has model {MODEL} of its own')
        (bx, by), (cx, cy) = PLACES[level]
        relocate.places = PLACES[level]
        verts, words = mesh_bytes(donor_mod, MODEL)
        own, uploads = vram[level], []
        def put(x, y, w, h, data):
            if any(own.rect(x, y, w, h)):
                raise SystemExit(f'level {level}: ({x},{y}) {w}x{h} is not free')
            if x % 64 + w > 64:
                raise SystemExit(f'level {level}: ({x},{y}) crosses a texture page')
            for ux, uy, uw, uh, _ in uploads:
                if x < ux + uw and ux < x + w and y < uy + uh and uy < y + h:
                    raise SystemExit(f'level {level}: ({x},{y}) {w}x{h} overlaps ({ux},{uy}) {uw}x{uh}')
            uploads.append((x, y, w, h, data))
        for i, b in enumerate(BOXES):
            block, clut = vram[DONOR].rect(*b['words']), vram[DONOR].rect(*b['clut_at'], 16, 1)
            if not any(block) or not any(clut):
                raise SystemExit('empty donor texture')
            put(bx + 4 * i, by, 4, 24, block)
            put(cx, cy + i, 16, 1, clut)
        out += struct.pack('<IHHI', level, len(verts), len(words), len(uploads))
        for v in verts:
            out += struct.pack('<4h', *v)
        out += struct.pack(f'<{len(words)}I', *words)
        for x, y, w, h, data in uploads:
            out += struct.pack('<4H', x, y, w, h) + struct.pack(f'<{w * h}H', *data)
        print(f'level {level}: model {MODEL} from level {DONOR}, {len(verts)} vertices, {len(words)} words, {len(uploads)} uploads')
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_bytes(out)
    print(f'{args.output}: {len(out)} bytes, sha256 {hashlib.sha256(out).hexdigest()}')


if __name__ == '__main__':
    main()
