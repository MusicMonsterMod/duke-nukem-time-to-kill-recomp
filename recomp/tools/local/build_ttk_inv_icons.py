#!/usr/bin/env python3
"""D08A3: rebuild the inventory switcher icon pack from Time to Kill's own HUD art.

Source (owned USA SLUS-00583 disc, read only):
  /DATA/FONTS.RAW (32768 bytes) is a raw 64x256 16-bit VRAM image the game
  uploads to VRAM (960, 0). It holds the HUD sheet: 4bpp item cells and their
  16-colour palettes (CLUTs) in column x = 1008.
  The executable's HUD sprite table (0x800c44b4.., 16-byte records: CLUT id,
  width, height, VRAM x, VRAM y) says which cell and palette each HUD indicator
  draws. The records used here are read and checked from SLUS_005.83.

Output: a TTKICO2 pack (item icons, kind 0) keeping the existing selection frame
(kind 1) unchanged, plus review PNGs in a local-only directory. The PNGs and the
pack are retail-derived and must stay out of the public repository.
"""
import argparse, hashlib, json, struct, sys, zlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
FONTS_PATH = '/DATA/FONTS.RAW;1'
FONTS_SHA256 = '54e16c1af6b63edcc5a7d92eefe45220b9921ebd0a41fa8047c92ebb5f18e1be'
FONTS_ORIGIN = (960, 0)
EXE_LOAD = 0x80010000
# item id (switcher) -> HUD record address; steroids (4) is extracted only.
RECORDS = {
    1: ('jetpack', 0x800c44b4),   # drawn while player+0x358 (jetpack) is on
    2: ('biomask', 0x800c44c4),   # player+0x360 branch, biomask
    3: ('goggles', 0x800c4504),   # player+0x360 branch, night-vision goggles
    4: ('steroids', 0x800c44f4),  # drawn while player+0x364 bit 2 (steroids) is set
    5: ('medkit', 0x800c44e4),    # the HUD health cross (player+0x32); no separate medkit icon
}
SWITCHER_ITEMS = (1, 2, 3, 5)


def record(exe, address):
    offset = address - EXE_LOAD + 2048
    clut, wh, x, zero1, y, zero2 = struct.unpack_from('<HHHHHH', exe, offset)
    if zero1 or zero2 or (clut & 0x3f) != 0x3f:
        raise SystemExit(f'unexpected HUD record at {address:#x}')
    return {'clut_x': (clut & 0x3f) * 16, 'clut_y': clut >> 6, 'w': wh & 255, 'h': wh >> 8, 'x': x, 'y': y}


def word(raw, vx, vy):
    ox, oy = FONTS_ORIGIN
    return struct.unpack_from('<H', raw, ((vy - oy) * 64 + (vx - ox)) * 2)[0]


def rgba(value):
    # PSX 15-bit BGR; 0x0000 is the hardware's transparent colour.
    if value == 0:
        return (0, 0, 0, 0)
    r, g, b = value & 31, (value >> 5) & 31, (value >> 10) & 31
    return ((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2), 255)


def cell(raw, rec):
    palette = [rgba(word(raw, rec['clut_x'] + i, rec['clut_y'])) for i in range(16)]
    pixels = []
    for yy in range(rec['h']):
        row = []
        for xx in range(rec['w']):
            texel = (word(raw, rec['x'] + xx // 4, rec['y'] + yy) >> (4 * (xx % 4))) & 15
            row.append(palette[texel])
        pixels.append(row)
    return pixels


def key_background(pixels):
    """The HUD cells sit on an opaque near-black square (the original draws them
    inside a metal frame). Clear the colour of the top-left corner wherever it is
    connected to the cell border, so the icon sits on the strip like other art."""
    h, w = len(pixels), len(pixels[0])
    key = pixels[0][0]
    seen, stack = set(), [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for y in range(h) for x in (0, w - 1)]
    while stack:
        x, y = stack.pop()
        if (x, y) in seen or not (0 <= x < w and 0 <= y < h):
            continue
        seen.add((x, y))
        if pixels[y][x][:3] != key[:3] and pixels[y][x][3]:
            continue
        pixels[y][x] = (0, 0, 0, 0)
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return pixels


def trim(pixels):
    rows = [y for y, r in enumerate(pixels) if any(p[3] for p in r)]
    cols = [x for x in range(len(pixels[0])) if any(r[x][3] for r in pixels)]
    if not rows:
        return pixels
    return [r[cols[0]:cols[-1] + 1] for r in pixels[rows[0]:rows[-1] + 1]]


def png(path, pixels):
    h, w = len(pixels), len(pixels[0])
    data = b''.join(b'\0' + bytes(c for p in r for c in p) for r in pixels)
    def chunk(tag, body):
        return struct.pack('>I', len(body)) + tag + body + struct.pack('>I', zlib.crc32(tag + body) & 0xffffffff)
    path.write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) +
                     chunk(b'IDAT', zlib.compress(data, 9)) + chunk(b'IEND', b''))


def old_cursor(pack_path):
    data = Path(pack_path).read_bytes()
    if data[:8] != b'TTKICO2\0':
        raise SystemExit(f'{pack_path}: not a TTKICO2 pack (need its selection frame)')
    count, offset = struct.unpack_from('<I', data, 8)[0], 12
    for _ in range(count):
        kind, item, tile, w, h = struct.unpack_from('<HHHHH', data, offset)
        size = 10 + w * h * 4
        if kind == 1:
            return data[offset:offset + size]
        offset += size
    raise SystemExit(f'{pack_path}: no selection frame entry')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--exe', default=str(ROOT / 'disc/SLUS_005.83'))
    ap.add_argument('--frame-from', default=str(ROOT / 'assets/ttk-inv-icons.pack'),
                    help='existing pack whose selection frame is kept')
    ap.add_argument('--output', default=str(ROOT / 'assets/ttk-inv-icons.pack'))
    ap.add_argument('--png-dir', default=str(ROOT / 'analysis/d08a3-ttk-icons/png'))
    args = ap.parse_args()

    disc = Disc(args.image)
    try:
        entry = next((f for f in disc.files() if f['path'] == FONTS_PATH), None)
        if not entry or entry['size'] != 32768:
            raise SystemExit(f'{FONTS_PATH} not found with the expected size')
        raw = disc.read(entry['lba'], entry['size'])
    finally:
        disc.close()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != FONTS_SHA256:
        raise SystemExit(f'{FONTS_PATH} sha256 {digest} does not match the owned USA disc')
    exe = Path(args.exe).read_bytes()
    frame = old_cursor(args.frame_from)

    png_dir = Path(args.png_dir); png_dir.mkdir(parents=True, exist_ok=True)
    entries, provenance = [], {}
    for item, (name, address) in RECORDS.items():
        rec = record(exe, address)
        native = cell(raw, rec)
        png(png_dir / f'{name}-cell.png', native)
        icon = trim(key_background([row[:] for row in native]))
        png(png_dir / f'{name}.png', icon)
        provenance[str(item)] = {'name': name, 'hud_record': f'{address:#x}', 'cell_vram': [rec['x'], rec['y']],
                                 'size': [rec['w'], rec['h']], 'clut_vram': [rec['clut_x'], rec['clut_y']],
                                 'fonts_raw_offset': ((rec['y'] - FONTS_ORIGIN[1]) * 64 + rec['x'] - FONTS_ORIGIN[0]) * 2,
                                 'icon_size': [len(icon[0]), len(icon)], 'in_pack': item in SWITCHER_ITEMS}
        if item in SWITCHER_ITEMS:
            body = struct.pack('<HHHHH', 0, item, 0, len(icon[0]), len(icon))
            body += b''.join(bytes(p) for r in icon for p in r)
            entries.append(body)
    entries.append(frame)
    Path(args.output).write_bytes(b'TTKICO2\0' + struct.pack('<I', len(entries)) + b''.join(entries))
    meta = {'source': f'{FONTS_PATH} (lba {entry["lba"]}, {entry["size"]} bytes, sha256 {digest}); VRAM origin (960, 0)',
            'format': '4bpp cells, 16-colour CLUTs in the same file; HUD sprite records from SLUS_005.83',
            'transparency': 'PSX colour 0x0000 transparent; the cell background colour connected to the cell border cleared, then trimmed',
            'frame': 'selection frame (kind 1) copied unchanged from the previous pack',
            'map': provenance,
            'note': 'Retail-derived local asset built by recomp/tools/local/build_ttk_inv_icons.py. Original disc untouched.'}
    Path(args.output).with_suffix('.provenance.json').write_text(json.dumps(meta, indent=2) + '\n')
    print(json.dumps({'fonts_sha256': digest, 'output': args.output, 'pngs': str(png_dir)}, indent=1))


if __name__ == '__main__':
    main()
