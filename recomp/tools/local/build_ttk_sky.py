#!/usr/bin/env python3
"""D17AO1: build the borrowed-sky pack (Duke Hill's sky) from the owned disc.

Source (owned USA SLUS-00583 disc, read only), DUKE HILL (level 1):
  /D0/DB01/DB01MOD.BIN  the level's model file. The game loads it unchanged at
      the model table address (word 0x800ddb18): one 32-bit offset per model
      id (0xffffffff none), then the meshes. A mesh's +8, +0x10 and +0x14
      fields are offsets from the mesh (relocated at load; +0xc is cleared and
      +0 takes the id). Sky layers: backdrop 0x611 (8 G4), fixed ring 0x612
      (36 FT4), slow and fast cloud bands 0x613/0x614 (72 FT4 each).
  /D0/DB01/DB01A.RAW, DB01B.RAW  raw 16-bit VRAM images the game uploads to
      (512,0) 448x256 and (512,256) 512x256 (note 184). The sky's 4-bit pages
      (704,0)/(704,256) and their CLUTs lie inside them.

Cloud deck (the project's own geometry, D17AO1): the cloud bands cover only
10..31 degrees above the horizon, so a skylight overhead would show the flat
fill colour. Each band's top ring (36 vertices, y -250, radius ~410) is closed
by a flat deck in its plane: an 8x8 grid cut to the ring and fanned into FT3
faces, each cell one panorama tile of the band (page 0x3b, UV 0..63), mirrored
in alternate cells so the tiling is seamless. The deck turns with its band, so
the slow and fast decks drift against each other.

Output: a TTKSKY1 pack (little endian): magic, source level, mesh count; each
mesh as id, byte size and the file's bytes (relative offsets kept); then the
two VRAM images. Retail-derived: build output only, never committed. Loaded by
src/ttk/sky_borrow.cpp. See documentation/398-d17ao1-borrowed-sky.md.
"""
import argparse, hashlib, math, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
LEVEL = 1
FILES = {
    'mod': ('/D0/DB01/DB01MOD.BIN;1', 217448),
    'a': ('/D0/DB01/DB01A.RAW;1', 448 * 256 * 2),
    'b': ('/D0/DB01/DB01B.RAW;1', 512 * 256 * 2),
}
TABLE = 2048 * 4
SKY_IDS = (0x611, 0x612, 0x613, 0x614)
RECORD_SIZE = {0x20: 8, 0x28: 8, 0x30: 12, 0x38: 12, 0x24: 16, 0x2c: 16, 0x34: 20, 0x3c: 20}


def mesh_bytes(mod, ident):
    """The mesh's span in the model file: header to the end of its polygon list."""
    offset = struct.unpack_from('<I', mod, 4 * ident)[0]
    if offset == 0xffffffff or offset + TABLE + 0x18 > len(mod):
        raise SystemExit(f'model {ident:#x} missing from the model file')
    offset += TABLE
    count = mod[offset + 6]
    vertices, polygons = (struct.unpack_from('<I', mod, offset + k)[0] for k in (0x10, 0x14))
    if not count or not 0x18 <= vertices < polygons:
        raise SystemExit(f'model {ident:#x}: unexpected layout')
    cursor = offset + polygons
    while True:
        header = struct.unpack_from('<I', mod, cursor)[0]
        cursor += 4
        kind, n = header & 0xffff, header >> 16
        if kind == 0xff:
            break
        if kind not in RECORD_SIZE or not n:
            raise SystemExit(f'model {ident:#x}: unknown polygon group {header:#x}')
        cursor += n * RECORD_SIZE[kind]
    return mod[offset:cursor]


DECK = (0x613, 0x614)   # the slow and fast cloud bands
DECK_CELLS = 8          # grid cells across the deck's diameter
DECK_TILE = (0x3b, 0, 0, 63, 63)   # tpage, u0, v0, u1, v1: the band's first panorama tile


def clip(polygon, a, b):
    """Sutherland-Hodgman: the part of `polygon` left of the edge a->b."""
    out = []
    side = lambda p: (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0])
    for i, p in enumerate(polygon):
        q = polygon[(i + 1) % len(polygon)]
        sp, sq = side(p), side(q)
        if sp >= 0:
            out.append(p)
        if (sp >= 0) != (sq >= 0):
            t = sp / (sp - sq)
            out.append((p[0] + t * (q[0] - p[0]), p[1] + t * (q[1] - p[1])))
    return out


def cloud_deck(blob):
    """Close the band's top ring with a flat cloud deck (FT3 group)."""
    count = blob[6]
    vertices = [list(struct.unpack_from('<3hH', blob, 0x28 + 8 * i)) for i in range(count)]
    polygons = struct.unpack_from('<I', blob, 0x14)[0]
    if polygons != 0x28 + 8 * count:
        raise SystemExit('cloud band: unexpected layout')
    header = struct.unpack_from('<I', blob, polygons)[0]
    end = blob[polygons + 4 + 16 * (header >> 16):]
    if header & 0xffff != 0x2c or len(end) != 4 or end[0] != 0xff:
        raise SystemExit('cloud band: expected one FT4 group')
    records = [blob[polygons + 4 + 16 * k:polygons + 20 + 16 * k] for k in range(header >> 16)]
    top = min(v[1] for v in vertices)
    rim = sorted((i for i, v in enumerate(vertices) if v[1] == top),
                 key=lambda i: math.atan2(vertices[i][2], vertices[i][0]))
    if len(rim) != 36:
        raise SystemExit('cloud band: expected a 36-vertex top ring')
    ring = [(vertices[i][0], vertices[i][2]) for i in rim]
    # The template tile's CLUT and page/flag word (semi-transparent, two-sided).
    tpage, u0, v0, u1, v1 = DECK_TILE
    template = None
    for r in records:
        t1, t2, t3 = struct.unpack_from('<3I', r, 4)
        uvs = [t1 & 0xffff, t2 & 0xffff, t3 & 0xffff, t3 >> 16]
        if (t2 >> 16) & 0x1ff == tpage and {(uv & 0xff, uv >> 8) for uv in uvs} == {(u0, v0), (u1, v0), (u0, v1), (u1, v1)}:
            template = (t1 >> 16, t2 >> 16)
    if not template:
        raise SystemExit('cloud band: deck tile not found')
    clut, flags = template
    index = {(vertices[i][0], vertices[i][2]): i for i in rim}

    def vertex(x, z):
        key = (round(x), round(z))
        if key not in index:
            index[key] = len(vertices)
            vertices.append([key[0], top, key[1], vertices[rim[0]][3]])
        return index[key]

    reach = max(math.hypot(x, z) for x, z in ring)
    size = 2 * reach / DECK_CELLS
    triangles = []
    for i in range(DECK_CELLS):
        for j in range(DECK_CELLS):
            x0, z0 = -reach + i * size, -reach + j * size
            cell = [(x0, z0), (x0 + size, z0), (x0 + size, z0 + size), (x0, z0 + size)]
            for k in range(36):
                cell = clip(cell, ring[k], ring[(k + 1) % 36])
                if not cell:
                    break
            if len(cell) < 3:
                continue

            def uv(p):
                s, t = (p[0] - x0) / size, (p[1] - z0) / size
                s, t = min(max(s, 0), 1), min(max(t, 0), 1)
                if i & 1:
                    s = 1 - s   # mirrored neighbours tile seamlessly
                if j & 1:
                    t = 1 - t
                return round(u0 + s * (u1 - u0)) | round(v0 + t * (v1 - v0)) << 8
            corners = [(vertex(*p), uv(p)) for p in cell]
            for k in range(1, len(corners) - 1):
                tri = (corners[0], corners[k], corners[k + 1])
                if len({c[0] for c in tri}) == 3:
                    triangles.append(tri)
    if len(vertices) > 255:
        raise SystemExit(f'cloud band: deck needs {len(vertices)} vertices')
    body = bytearray(blob[:0x28])
    body[6] = len(vertices)
    for v in vertices:
        body += struct.pack('<3hH', *v)
    struct.pack_into('<I', body, 0x14, len(body))
    body += struct.pack('<I', header)
    for r in records:
        body += r
    body += struct.pack('<I', 0x24 | len(triangles) << 16)
    for (a, ua), (b, ub), (c, uc) in triangles:
        body += struct.pack('<4B3I', a, b, c, 0, ua | clut << 16, ub | flags << 16, uc)
    body += end
    return bytes(body)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-sky.pack'))
    args = ap.parse_args()

    disc = Disc(args.image)
    try:
        entries = {f['path']: f for f in disc.files()}
        data = {}
        for key, (path, size) in FILES.items():
            entry = entries.get(path)
            if not entry or entry['size'] != size:
                raise SystemExit(f'{path} not found with the expected size')
            data[key] = disc.read(entry['lba'], entry['size'])
    finally:
        disc.close()

    out = bytearray(b'TTKSKY1\0')
    out += struct.pack('<II', LEVEL, len(SKY_IDS))
    for ident in SKY_IDS:
        blob = mesh_bytes(data['mod'], ident)
        if ident in DECK:
            blob = cloud_deck(blob)
        out += struct.pack('<II', ident, len(blob)) + blob
    out += data['a'] + data['b']
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_bytes(out)
    print(f'{args.output}: {len(out)} bytes, sha256 {hashlib.sha256(out).hexdigest()}')


if __name__ == '__main__':
    main()
