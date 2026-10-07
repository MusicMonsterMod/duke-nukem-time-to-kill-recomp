#!/usr/bin/env python3
"""D24A: build the Modernized text glyph pack from Time to Kill's own fonts.

Replaces the Duke Nukem 3D message/Atomic glyph pack (build_duke_fonts.py). All
glyph pixels come from the player's own disc at build time; nothing is tracked.

Sources (owned USA SLUS-00583, read only, hashes pinned):
  /DATA/FONTS.RAW  64x256 16-bit VRAM image at (960, 0): TTK Big Italic (17 px,
                   rows 0..70), TTK Medium Italic (11 px, rows 72..106) and the
                   16-colour CLUTs in column x = 1008.
  SLUS_005.83      the PSY-Q system font (FntPrint) at 0x800c7d70, 128x32,
                   ASCII 0x20..0x5f; the fallback for characters the italic
                   fonts lack (! % ( ) & + " and others).
Palettes: disc CLUT 225 (gold, the pause-menu text) and "Console steel", the
project's own palette matching the old console font.

Choices (user, 2026-10-06, D24A font picker; sets 3-7 from the approved D24B
savestate panel mockup):
  set 0 messages  (styles 0 and 2): TTK Big Italic, Console steel, 1x, spacing 1, drop shadow, all caps
  set 1 headings  (style 1):        TTK Big Italic, Gold (CLUT 225), 2x, spacing 1, all caps
  set 2 console   (style 3):        TTK Medium Italic, Console steel, 1x, spacing 0, drop shadow, all caps
Missing glyphs fall back to the system 8x8 font in the set's palette.

It also writes ttk-ui.pack (TTKUI1): the disc's button sprites and radiation
emblem for the D24B savestate panel.

Pack TTKFONT2 (little endian): 'TTKFONT2', u32 payload size, u32 FNV-1a of the
payload. Payload: u32 set count (3); per set 8 bytes (u8 line height, u8 scale,
u8 shadow, u8 0, u32 shadow ARGB) and 95 glyph records for ASCII 32..126
(u8 w, u8 h, u8 advance, u8 y, u32 pixel offset in the payload); then ARGB
pixels (u32, 0 = transparent).
"""
import argparse, hashlib, json, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
FONTS_PATH = '/DATA/FONTS.RAW;1'
FONTS_SHA256 = '54e16c1af6b63edcc5a7d92eefe45220b9921ebd0a41fa8047c92ebb5f18e1be'
SYSTEM_FONT = 0x800c7d70
EXE_LOAD = 0x80010000
STEEL = ['000000', '0c0c18', '14182c', '202840', '405080', '5c70a0', '7088b4', '84a4cc', '94b0d8',
         'a4c4e4', 'b4d4f4', 'bcdcfc', 'd0e6fc', 'e4f0ff', 'f4faff', '000000']
SHADOW = 0xff0c0c18  # Console steel's darkest navy

BIG_ROWS = [(0, ',-./0123456789A'), (18, 'BCDEFGHIJKLM'), (36, 'NOPQRSTUVWXY'), (54, "Z:;<>=?@'")]
MEDIUM_ROWS = [(72, ',-./0123456789ABCDEFG'), (84, 'HIJKLMNOPQRSTUVWXY'), (96, "Z:;<>=?@©™'")]
SETS = [  # name, font, palette or '#rrggbb' tint (system font), scale, spacing, shadow
    ('messages', 'big', 'steel', 1, 1, True),
    ('headings', 'big', 225, 2, 1, False),
    ('console', 'medium', 'steel', 1, 0, True),
    # D24B savestate panel (approved mockup): title, slot titles, selected slot, details.
    ('panel_title', 'big', 225, 1, 1, True),
    ('panel_slot', 'medium', 225, 1, 1, True),
    ('panel_slot_selected', 'medium', 226, 1, 1, True),
    ('panel_text', 'system', '#dedede', 1, 0, False),
    ('panel_dim', 'system', '#848484', 1, 0, False),
    # D08A5 mission row and item card (approved design E): FOUND, then the 2x
    # Microfont "MISSION" label and found/total count (all found: green).
    ('mission_found', 'system', '#5fd35f', 1, 0, False),
    ('mission_label', 'micro', '#848484', 2, 1, False),
    ('mission_count', 'micro', '#989c58', 2, 1, False),
    ('mission_complete', 'micro', '#5fd35f', 2, 1, False),
]
FONT_INFO = {'big': (BIG_ROWS, 17, 8), 'medium': (MEDIUM_ROWS, 11, 5), 'system': (None, 8, 8), 'micro': (None, 5, 4)}  # rows, height, space
MICROFONT = ROOT / 'assets/ui/fonts/microfont/3x5-Microfont_1D.png'  # CC0, tracked


class Sheet:
    def __init__(self, raw):
        self.raw = raw

    def word(self, vx, vy):
        return struct.unpack_from('<H', self.raw, (vy * 64 + (vx - 960)) * 2)[0]

    def texel(self, px, py):
        return (self.word(960 + px // 4, py) >> (4 * (px % 4))) & 15

    def segments(self, y0, y1):
        cols = [any(self.texel(x, y) for y in range(y0, y1 + 1)) for x in range(256)]
        out, start = [], None
        for i, lit in enumerate(cols + [False]):
            if lit and start is None:
                start = i
            if not lit and start is not None:
                out.append((start, i - 1)); start = None
        return out

    def font(self, rows, height):
        glyphs = {}
        for y0, chars in rows:
            segs = self.segments(y0, y0 + height - 1)
            if len(segs) != len(chars):
                raise SystemExit(f'FONTS.RAW row {y0}: {len(segs)} glyphs for {len(chars)} characters')
            for ch, (a, b) in zip(chars, segs):
                glyphs[ch] = (b - a + 1, height, [self.texel(x, y) for y in range(y0, y0 + height) for x in range(a, b + 1)])
        return glyphs

    def palette(self, clut_y):
        out = []
        for k in range(16):
            v = self.word(1008 + k, clut_y)
            r, g, b = v & 31, (v >> 5) & 31, (v >> 10) & 31
            out.append((r * 255 // 31, g * 255 // 31, b * 255 // 31))
        return out


def micro_font(path=MICROFONT):
    """The open 3x5 Microfont (1D sheet, 3 px per ASCII code), 1-bit glyphs."""
    from build_ttk_inv_icons import read_rgba_png
    w, h, rgba = read_rgba_png(path)
    if (w, h) != (384, 5):
        raise SystemExit(f'{path}: expected the 384x5 Microfont 1D sheet, got {w}x{h}')
    return {chr(code): (3, 5, [int(rgba[(y * w + 3 * code + x) * 4 + 3] >= 128) for y in range(5) for x in range(3)])
            for code in range(0x21, 0x60)}


def system_font(exe):
    data = exe[SYSTEM_FONT - EXE_LOAD + 2048:SYSTEM_FONT - EXE_LOAD + 2048 + 2048]
    glyphs = {}
    for code in range(0x20, 0x60):
        i = code - 0x20
        gx, gy = (i % 16) * 8, (i // 16) * 8
        bits = []
        for y in range(8):
            for x in range(8):
                px = gx + x
                bits.append((data[(gy + y) * 64 + px // 2] >> (4 * (px % 2))) & 15)
        glyphs[chr(code)] = (8, 8, bits)
    return glyphs


# D24B UI sprites from the HUD sprite table (16-byte records: CLUT id, w, h,
# VRAM x, VRAM y), cells and CLUTs in FONTS.RAW. Pack TTKUI1: 'TTKUI1\0\0', u32
# count, then per sprite u16 id, u16 w, u16 h and ARGB u32 pixels.
UI_SPRITES = [(1, 'Up/Down', 0x800c4474), (2, 'Cross', 0x800c4414), (3, 'Square', 0x800c4424),
              (4, 'Circle', 0x800c4404), (5, 'Triangle', 0x800c43f4), (6, 'Radiation disc', 0x800c4454)]


def ui_pack(raw, exe):
    sheet, out = Sheet(raw), b''
    for ident, _, address in UI_SPRITES:
        clut, wh, x, z1, y, z2 = struct.unpack_from('<HHHHHH', exe, address - EXE_LOAD + 2048)
        if z1 or z2 or (clut & 0x3f) != 0x3f:
            raise SystemExit(f'unexpected sprite record at {address:#x}')
        w, h, cy = wh & 255, wh >> 8, clut >> 6
        pixels = []
        for yy in range(h):
            for xx in range(w):
                t = (sheet.word(x + xx // 4, y + yy) >> (4 * (xx % 4))) & 15
                v = sheet.word(1008 + t, cy)
                pixels.append(0 if v == 0 else argb(((v & 31) * 255 // 31, ((v >> 5) & 31) * 255 // 31, ((v >> 10) & 31) * 255 // 31)))
        out += struct.pack('<HHH', ident, w, h) + struct.pack(f'<{len(pixels)}I', *pixels)
    return b'TTKUI1\0\0' + struct.pack('<I', len(UI_SPRITES)) + out


def argb(rgb):
    return 0xff000000 | (rgb[0] << 16) | (rgb[1] << 8) | rgb[2]


def build(raw, exe):
    sheet = Sheet(raw)
    fonts = {name: sheet.font(rows, height) for name, (rows, height, _) in FONT_INFO.items() if rows}
    system = system_font(exe)
    fonts['system'] = system
    fonts['micro'] = micro_font()
    palettes = {225: sheet.palette(225), 226: sheet.palette(226),
                'steel': [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in STEEL]}
    headers, pixels, records, fallbacks = b'', b'', [], {}
    base = 4 + len(SETS) * (8 + 95 * 8)
    for name, font, pal, scale, spacing, shadow in SETS:
        _, height, space = FONT_INFO[font]
        if isinstance(pal, str) and pal.startswith('#'):
            tint = tuple(int(pal[i:i + 2], 16) for i in (1, 3, 5))
            colours = [tint] * 16  # 1-bit system glyphs: every lit texel takes the tint
        else:
            colours = palettes[pal]
        headers += struct.pack('<BBBBI', height + 2, scale, int(shadow), 0, SHADOW if shadow else 0)
        used = []
        for code in range(32, 127):
            ch = chr(code).upper()  # all caps
            if ch == ' ':
                records.append(struct.pack('<BBBBI', 1, 1, space, 0, base + len(pixels)))
                pixels += struct.pack('<I', 0)
                continue
            if ch in fonts[font]:
                w, h, texels = fonts[font][ch]
                y, advance = 0, w + spacing
                data = [0 if t == 0 else argb(colours[t]) for t in texels]
            elif ch in system:
                w, h, bits = system[ch]
                y, advance = max(0, height - h - (1 if height >= 11 else 0)), w
                data = [argb(colours[11]) if b else 0 for b in bits]
                used.append(ch)
            else:
                ch = '?'
                w, h, texels = fonts[font]['?']
                y, advance = 0, w + spacing
                data = [0 if t == 0 else argb(colours[t]) for t in texels]
            records.append(struct.pack('<BBBBI', w, h, advance, y, base + len(pixels)))
            pixels += struct.pack(f'<{len(data)}I', *data)
        fallbacks[name] = ''.join(sorted(set(used)))
    glyph_block = b''
    for s in range(len(SETS)):
        glyph_block += headers[8 * s:8 * s + 8] + b''.join(records[95 * s:95 * s + 95])
    payload = struct.pack('<I', len(SETS)) + glyph_block + pixels
    assert len(struct.pack('<I', len(SETS)) + glyph_block) == base
    h = 2166136261
    for byte in payload:
        h = ((h ^ byte) * 16777619) & 0xffffffff
    return b'TTKFONT2' + struct.pack('<II', len(payload), h) + payload, fallbacks


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--exe', default=str(ROOT / 'disc/SLUS_005.83'))
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-fonts.pack'))
    ap.add_argument('--ui-output', help='UI sprite pack (default: ttk-ui.pack beside --output)')
    args = ap.parse_args()
    disc = Disc(args.image)
    try:
        entry = next((f for f in disc.files() if f['path'] == FONTS_PATH), None)
        if not entry or entry['size'] != 32768:
            raise SystemExit(f'{FONTS_PATH} not found with the expected size: is this the owned USA SLUS-00583 disc?')
        raw = disc.read(entry['lba'], entry['size'])
    finally:
        disc.close()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != FONTS_SHA256:
        raise SystemExit(f'{FONTS_PATH} sha256 {digest} does not match the owned USA disc')
    exe = Path(args.exe).read_bytes()
    pack, fallbacks = build(raw, exe)
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(pack)
    ui = Path(args.ui_output) if args.ui_output else out.with_name('ttk-ui.pack')
    ui.write_bytes(ui_pack(raw, exe))
    out.with_suffix('.json').write_text(json.dumps({
        'source': f'{FONTS_PATH} (sha256 {digest}) and the SLUS_005.83 system font at {SYSTEM_FONT:#x}; extracted from the player\'s disc at build time',
        'sets': [{'set': i, 'use': name, 'font': font,
                  'palette': 'Console steel (project)' if pal == 'steel' else pal if isinstance(pal, str) else f'CLUT {pal}',
                  'scale': scale, 'spacing': spacing, 'shadow': shadow, 'system_font_fallback': fallbacks[name]}
                 for i, (name, font, pal, scale, spacing, shadow) in enumerate(SETS)],
        'format': 'TTKFONT2; see tools/local/build_ttk_fonts.py',
        'note': 'Retail-derived build output: never commit or distribute.'}, indent=2) + '\n')
    print(out)


if __name__ == '__main__':
    main()
