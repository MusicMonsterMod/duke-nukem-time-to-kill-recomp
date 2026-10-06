#!/usr/bin/env python3
"""D24A: build the inventory switcher digit pack from the open 3x5 Microfont.

Source: assets/ui/fonts/microfont/3x5-Microfont_1D.png (CC0, nimaid/microfont;
see SOURCE.md there). It replaced the Duke Nukem 3D THREEBYFIVE digits (tiles
3010-3019 and the % from tile 3076); the TTKDIG3 format and the tile ids the
runtime checks are unchanged. Glyph pixels take the switcher green the old
digits used. Standard library only; deterministic.
"""
import argparse, hashlib, json, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from build_ttk_inv_icons import read_rgba_png  # noqa: E402

ROOT = HERE.parents[1]
SHEET = ROOT / 'assets/ui/fonts/microfont/3x5-Microfont_1D.png'
GREEN = (152, 156, 88, 255)  # the switcher green of the digits this replaces
GLYPHS = [(3010 + d, str(d)) for d in range(10)] + [(3076, '%')]


def glyph(rgba, sheet_w, ch):
    x0, out = 3 * ord(ch), b''
    for y in range(5):
        for x in range(3):
            alpha = rgba[(y * sheet_w + x0 + x) * 4 + 3]
            out += bytes(GREEN if alpha >= 128 else (0, 0, 0, 0))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--sheet', default=str(SHEET))
    ap.add_argument('--output', default=str(ROOT / 'assets/ttk-inv-digits.pack'))
    args = ap.parse_args()
    w, h, rgba = read_rgba_png(args.sheet)
    if (w, h) != (384, 5):
        raise SystemExit(f'{args.sheet}: expected the 384x5 Microfont 1D sheet, got {w}x{h}')
    body = b''.join(struct.pack('<HHH', tile, 3, 5) + glyph(rgba, w, ch) for tile, ch in GLYPHS)
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(b'TTKDIG3\0' + struct.pack('<I', len(GLYPHS)) + body)
    sheet = Path(args.sheet).read_bytes()
    out.with_suffix('.provenance.json').write_text(json.dumps({
        'source': f'3x5 Microfont (nimaid/microfont, CC0 1.0), {Path(args.sheet).name} sha256 {hashlib.sha256(sheet).hexdigest()}',
        'format': 'TTKDIG3 RGBA bytes (host converts to ARGB); tiles 3010-3019 digits, 3076 %',
        'colour': 'rgb(%d,%d,%d)' % GREEN[:3],
        'note': 'Built by recomp/tools/local/build_ttk_inv_digits.py; replaced the Duke Nukem 3D THREEBYFIVE digits (D24A).'},
        indent=2) + '\n')
    print(out)


if __name__ == '__main__':
    main()
