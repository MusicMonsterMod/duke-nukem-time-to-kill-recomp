#!/usr/bin/env python3
"""D61B: build the hidden levels' look pack (textures for untextured models).

Some models in True Grit (4) and Modern 4 (13) load with faces whose texture
the level's set does not have: the loader leaves CLUT 0xffff on page 0x20, and
the face draws the frame buffer (True Grit's striped totem wings, Modern 4's
magenta bar fronts and crates). Their UVs are local coordinates on the missing
texture (a 32x32 box from 0,0, say). documentation/561-d61b-hidden-levels.md.

Each fix names a model, the faces to texture (in polygon-list order, textured
faces only), and a source:
  donor  the same model in a shipped level (the same vertices, measured from
         RAM): its texture block (VRAM words, read here from the donor's raw
         images) and CLUT are copied into free VRAM of the hidden level;
  own    a texture the hidden level already has (no upload);
  art    the project's own painting (recomp/assets/hidden-levels), uploaded
         and drawn through a CLUT of the level's own;
  drak   (True Grit's totem head, user 2026-11-26: "texture the head to be
         similar to that of one of the draks ... colored up nice to match the
         aesthetic of the totem pole") the lizard guardian's own head profile
         and scales from True Grit's raw image, recoloured by brightness into
         the totem's gold ramp (CLUT 0x1438), with UVs projected from the
         head's vertices (DB04MOD.BIN): side faces take the profile (snout
         forward), the top, underside and back the scales.
A level's texture VRAM is its two raw images (DBnnA.RAW at (512,0) 448x256,
DBnnB.RAW at (512,256) 512x256). Every place a block or CLUT goes was chosen
all zero in the hidden level's raw image and clear of every texture its world
and models name (D61B lab, from RAM); the builder checks the zeros again.

Output: a TTKLOOK2 pack (little endian): magic, version 2, level count; per
level: index, model-fix count, upload count; per fix: model id, vertex count,
face count, CLUT, page (5 bits), du, dv, mode (0 listed faces, 1 every
untextured face, 2 explicit UVs), then the face indices (and, mode 2, four
UVs per face); per upload:
x, y, w, h (u16) and w*h words. Retail-derived (the donor blocks): build output
only, never committed. Loaded by src/ttk/hidden_level_look.cpp.
"""
import argparse, hashlib, struct, sys
from pathlib import Path
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
ART = ROOT / 'assets/hidden-levels'
A_W, B_W, H = 448, 512, 256

# level -> fixes. donor: (level, x, y, w, h, clut x, y) source; place: (x, y) of
# the block, (cx, cy) of its CLUT copy; du, dv shift the local UVs onto it.
HEAD_FACES = [1, 2, 48, 49, 53, 54] + list(range(56, 70))   # model 87's head (y above -1380), untextured on the disc
FIXES = {
    13: [
        # The club's bar front (type 79, room 12): TIME TO KILL page 0x239 u e0-ff v 20-3f, CLUT 0xe38.
        dict(model=85, verts=8, faces=[0, 1, 2, 3], total=5, donor=(0, 632, 288, 8, 32, 896, 56),
             place=(796, 152), clut=(928, 131), du=112, dv=152),
        # Type 232 (room 13): page 0x2a u 60-7f v 68-7b (CLUT 0x2bf9) and page 0x2e u b0-c7 v 11-34 (0x2bb9).
        dict(model=528, verts=6, faces=[0], total=2, donor=(0, 664, 104, 8, 20, 912, 175),
             place=(796, 188), clut=(928, 134), du=112, dv=188),
        dict(model=528, verts=6, faces=[1], total=2, donor=(0, 940, 17, 6, 36, 912, 174),
             place=(840, 392), clut=(912, 146), du=32, dv=136),
        # Types 472/594 (rooms 6, 8): page 0x2b u 68-85 v 0-17, CLUT 0x322c.
        dict(model=872, verts=8, faces=[0, 1, 2, 3, 4, 5], total=6, donor=(0, 730, 0, 8, 24, 704, 200),
             place=(900, 228), clut=(928, 149), du=16, dv=228),
        # Room 15's crates (types 596, 602; no shipped twin): the level's own crate (type 20).
        dict(model=1103, verts=8, faces=[0, 1, 2, 3, 4], total=5, own=(0x16fb, 0x38), du=0x80, dv=0),
        dict(model=1109, verts=14, faces=[0, 1, 2, 3, 4, 5, 11], total=13, own=(0x16fb, 0x38), du=0x80, dv=0),
    ],
    4: [
        # The totem poles' head (type 81, room 25): a Drak's face in the totem's gold.
        dict(model=87, verts=81, faces=HEAD_FACES, total=106, drak=0x1438, place=(912, 72)),
        # The totem poles' wings (no shipped twin): our painting, the totem's CLUT 0x1438.
        dict(model=87, verts=81, faces='unresolved', total=106, art=('true-grit-totem-wing.png', 24, 32, 0x1438),
             place=(664, 64), du=96, dv=64),
    ],
}


# The guardian's head textures in True Grit (page 0x2c, x 768): the profile
# (CLUT 0x2171, u 64-95 v 160-175) and the scales (CLUT 0x2271, u 72-95 v 216-231).
DRAK_PROFILE = (768, 64, 160, 32, 16, 0x2171)
DRAK_SCALES = (768, 72, 216, 24, 16, 0x2271)
MODEL_TABLE = 2048 * 4
RECORD = {0x20: 8, 0x28: 8, 0x30: 12, 0x38: 12, 0x24: 16, 0x2c: 16, 0x34: 20, 0x3c: 20}


def model_mesh(mod, ident):
    """Vertices and textured faces (type, vertex indices) of a model in a level's model file."""
    offset = struct.unpack_from('<I', mod, 4 * ident)[0]
    if offset == 0xffffffff:
        raise SystemExit(f'model {ident} missing')
    m = MODEL_TABLE + offset
    count = mod[m + 6]
    vp, fp = (struct.unpack_from('<I', mod, m + k)[0] for k in (0x10, 0x14))
    verts = [struct.unpack_from('<3h', mod, m + vp + 8 * i) for i in range(count)]
    faces, cur = [], m + fp
    while True:
        header = struct.unpack_from('<I', mod, cur)[0]
        cur += 4
        kind, n = header & 0xffff, header >> 16
        if kind == 0xff:
            return verts, faces
        if kind not in RECORD:
            raise SystemExit(f'model {ident}: unknown polygon group {header:#x}')
        for k in range(n):
            if kind in (0x24, 0x2c):
                w = struct.unpack_from('<I', mod, cur + RECORD[kind] * k)[0]
                faces.append((kind, [(w >> (8 * j)) & 0xff for j in range(4 if kind == 0x2c else 3)]))
        cur += n * RECORD[kind]


def texels(vram, x0, u0, v0, w, h, clut):
    """Colours (15-bit) of a 4-bit texture box through its CLUT."""
    cx, cy = (clut & 0x3f) * 16, clut >> 6
    out = []
    for v in range(v0, v0 + h):
        for u in range(u0, u0 + w):
            i = (vram.px(x0 + u // 4, v) >> (4 * (u & 3))) & 0xf
            out.append(vram.px(cx + i, cy))
    return out


def luma(c):
    return 0.299 * (c & 31) + 0.587 * ((c >> 5) & 31) + 0.114 * ((c >> 10) & 31)


def drak_block(vram, ramp_clut):
    """The head block, 32x32 4-bit (8x32 words): profile on top, scales below, in the ramp's order."""
    cx, cy = (ramp_clut & 0x3f) * 16, ramp_clut >> 6
    ramp = [luma(vram.px(cx + i, cy)) for i in range(16)]
    usable = [i for i in range(16) if vram.px(cx + i, cy)]   # colour 0 is transparent
    profile = texels(vram, *DRAK_PROFILE)
    x0, u0, v0, w, h, clut = DRAK_SCALES
    scales = texels(vram, x0, u0, v0, w, h, clut)
    scales = [scales[(i // 32) * w + (i % 32) % w] for i in range(32 * h)]   # 24 wide, repeated to 32
    out = []
    for part in (profile, scales):
        ls = [luma(c) for c in part]
        lo, hi = min(ls), max(ls)
        rlo, rhi = min(ramp[i] for i in usable), max(ramp[i] for i in usable)
        for l in ls:
            t = rlo + (l - lo) / max(hi - lo, 1e-6) * (rhi - rlo)
            out.append(min(usable, key=lambda i: abs(ramp[i] - t)))
    return [sum(out[y * 32 + 4 * i + k] << (4 * k) for k in range(4)) for y in range(32) for i in range(8)]


def head_uvs(verts, faces, indices, u0, v0):
    """Projected UVs per face: sides the profile (rows 0-15), the rest the scales (rows 16-31)."""
    head = {j for i in indices for j in faces[i][1]}
    xs, ys, zs = ([verts[j][k] for j in head] for k in range(3))
    span = lambda a, lo, hi, n: round((a - lo) / max(hi - lo, 1) * n)
    out = {}
    for i in indices:
        kind, idx = faces[i]
        a, b, c = (verts[j] for j in idx[:3])
        nx = (b[1] - a[1]) * (c[2] - a[2]) - (b[2] - a[2]) * (c[1] - a[1])
        ny = (b[2] - a[2]) * (c[0] - a[0]) - (b[0] - a[0]) * (c[2] - a[2])
        nz = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
        uvs = []
        for j in idx:
            x, y, z = verts[j]
            if abs(nz) >= max(abs(nx), abs(ny)):        # a side: the profile, snout at the head's front (-x)
                u, v = span(x, min(xs), max(xs), 31), span(y, min(ys), max(ys), 15)
            elif abs(ny) >= abs(nx):                    # top or underside: scales across x and z
                u, v = span(x, min(xs), max(xs), 31), 16 + span(z, min(zs), max(zs), 15)
            else:                                       # the back: scales across z and y
                u, v = span(z, min(zs), max(zs), 31), 16 + span(y, min(ys), max(ys), 15)
            uvs.append((v0 + v) << 8 | (u0 + u))
        while len(uvs) < 4:
            uvs.append(0)
        out[i] = uvs
    return out


def raw_path(level, part):
    return f'/D{level // 10}/DB{level:02d}/DB{level:02d}{part}.RAW;1'


class Vram:
    def __init__(self, a, b):
        if len(a) != A_W * H * 2 or len(b) != B_W * H * 2:
            raise SystemExit('unexpected raw image size')
        self.a, self.b = a, b

    def px(self, x, y):
        if 512 <= x < 512 + A_W and 0 <= y < H:
            return struct.unpack_from('<H', self.a, (y * A_W + x - 512) * 2)[0]
        if 512 <= x < 1024 and H <= y < 2 * H:
            return struct.unpack_from('<H', self.b, ((y - H) * B_W + x - 512) * 2)[0]
        raise SystemExit(f'({x},{y}) is outside the level images')

    def rect(self, x, y, w, h):
        return [self.px(x + i, y + j) for j in range(h) for i in range(w)]


def art_words(name, w, h):
    """Our indexed painting as 4-bit VRAM words (four texels a word, low nibble first)."""
    img = Image.open(ART / name)
    if img.mode != 'P' or img.size != (4 * w, h):
        raise SystemExit(f'{name}: expected a {4 * w}x{h} indexed image')
    px = list(img.getdata())
    if max(px) > 15:
        raise SystemExit(f'{name}: more than 16 colours')
    return [sum(px[y * 4 * w + 4 * i + k] << (4 * k) for k in range(4)) for y in range(h) for i in range(w)]


def page5(x, y):
    return (x // 64) | (y // 256) << 4


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-look.pack'))
    args = ap.parse_args()

    levels = set(FIXES) | {f['donor'][0] for fixes in FIXES.values() for f in fixes if 'donor' in f}
    disc = Disc(args.image)
    try:
        files = {f['path']: f for f in disc.files()}
        def read(path):
            e = files.get(path)
            if not e:
                raise SystemExit(f'{path} not found')
            return disc.read(e['lba'], e['size'])
        vram = {lv: Vram(read(raw_path(lv, 'A')), read(raw_path(lv, 'B'))) for lv in levels}
        mods = {4: read('/D0/DB04/DB04MOD.BIN;1')}
    finally:
        disc.close()

    out = bytearray(b'TTKLOOK2') + struct.pack('<II', 2, len(FIXES))
    for level, fixes in sorted(FIXES.items()):
        own = vram[level]
        entries, uploads = [], []
        def put(x, y, w, h, words):
            if any(own.rect(x, y, w, h)):
                raise SystemExit(f'level {level}: ({x},{y}) {w}x{h} is not free')
            if x % 64 + w > 64:
                raise SystemExit(f'level {level}: ({x},{y}) crosses a texture page')
            uploads.append((x, y, w, h, words))
        for f in fixes:
            if 'donor' in f:
                dl, dx, dy, w, h, dcx, dcy = f['donor']
                block, clut = vram[dl].rect(dx, dy, w, h), vram[dl].rect(dcx, dcy, 16, 1)
                if not any(block) or not any(clut):
                    raise SystemExit(f'model {f["model"]}: empty donor texture')
                (px, py), (cx, cy) = f['place'], f['clut']
                put(px, py, w, h, block)
                put(cx, cy, 16, 1, clut)
                clut_id, page = cy << 6 | cx // 16, page5(px, py)
            elif 'drak' in f:
                px, py = f['place']
                put(px, py, 8, 32, drak_block(own, f['drak']))
                clut_id, page = f['drak'], page5(px, py)
                verts, faces = model_mesh(mods[level], f['model'])
                if len(verts) != f['verts'] or len(faces) != f['total']:
                    raise SystemExit(f'model {f["model"]}: unexpected mesh')
                uvs = head_uvs(verts, faces, f['faces'], (px % 64) * 4, py % 256)
                entries.append((f['model'], f['verts'], f['total'], clut_id, page, 0, 0, 2, f['faces'], uvs))
                continue
            elif 'own' in f:
                clut_id, page = f['own'][0], f['own'][1] & 0x1f
            else:
                name, w, h, clut_id = f['art']
                px, py = f['place']
                put(px, py, w, h, art_words(name, w, h))
                page = page5(px, py)
            faces = f['faces']
            flag = 0
            if faces == 'unresolved':
                faces, flag = [], 1   # every face that names CLUT 0xffff
            entries.append((f['model'], f['verts'], f['total'], clut_id, page, f['du'], f['dv'], flag, faces, None))
        out += struct.pack('<III', level, len(entries), len(uploads))
        for model, verts, total, clut_id, page, du, dv, flag, faces, uvs in entries:
            out += struct.pack('<HHHHBBBBI', model, verts, total, clut_id, page, du, dv, flag, len(faces))
            out += struct.pack(f'<{len(faces)}H', *faces)
            if flag == 2:   # explicit UVs, four per face
                for i in faces:
                    out += struct.pack('<4H', *uvs[i])
        for x, y, w, h, words in uploads:
            out += struct.pack('<4H', x, y, w, h) + struct.pack(f'<{w * h}H', *words)
        print(f'level {level}: {len(entries)} model fixes, {len(uploads)} uploads')
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_bytes(out)
    print(f'{args.output}: {len(out)} bytes, sha256 {hashlib.sha256(out).hexdigest()}')


if __name__ == '__main__':
    main()
