#!/usr/bin/env python3
"""D08A5: build the mission item pack for the Modernized item switcher.

Sources, all original project art tracked in assets/ui (no disc input, so every
build makes it):
  items/items.json and its 16x16 icons  - one icon per mission item name;
  item-frame.png                        - the project frame, in three colours.

Pack TTKMIS1: 'TTKMIS1\\0', u32 count, then per entry u16 kind, u16 name length,
the ASCII name, u16 w, u16 h and RGBA bytes. Kinds:
  0 icon (name = item name), 1 missing-item silhouette (name = item name),
  2 frame grey (the project frame as drawn), 3 frame orange (gadget selection),
  4 frame steel (browsed mission slot / item card),
  5 used tick (D08A19: items/mark-used.png, the user's green tick, at most 16x16,
    drawn over the icon's bottom-right corner once the item is used).
The orange and steel frames are palette swaps (approved design E): each frame
pixel's brightness, normalised over the frame, picks a step of the ramp, so the
bevel and shading carry over. Only colour values are reused (the orange ramp is
the old Duke 3D tile0020 colours; steel is the project's Console steel).
The silhouette follows the approved mockup: greyscale, brightness 0.45, contrast
0.8, alpha 0.85. Standard library only; deterministic.
"""
import argparse, hashlib, json, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from build_ttk_inv_icons import read_rgba_png  # noqa: E402

ROOT = HERE.parents[1]
UI = ROOT / 'assets/ui'
RAMP_ORANGE = ['341c00', '442800', '583000', '6c3800', '804000', '904800', 'a45000', 'b45404', 'cc6818', 'd47430', 'd88444']
RAMP_STEEL = ['0c0c18', '14182c', '202840', '405080', '5c70a0', '7088b4', '84a4cc', '94b0d8', 'a4c4e4', 'b4d4f4', 'bcdcfc']


def luma(r, g, b):
    return 0.299 * r + 0.587 * g + 0.114 * b


def swap(rgba, ramp):
    """The mockup's palette swap: brightness over opaque (alpha > 128) pixels sets
    the range; every visible pixel takes the nearest ramp step."""
    px = [rgba[i:i + 4] for i in range(0, len(rgba), 4)]
    lit = [luma(*p[:3]) for p in px if p[3] > 128]
    lo, hi = min(lit), max(lit)
    out = bytearray()
    for r, g, b, a in px:
        if a:
            t = (luma(r, g, b) - lo) / (hi - lo) if hi > lo else 0
            step = ramp[min(len(ramp) - 1, int(t * (len(ramp) - 1) + 0.5))]
            r, g, b = (int(step[i:i + 2], 16) for i in (0, 2, 4))
        out += bytes((r, g, b, a))
    return bytes(out)


def silhouette(rgba):
    """CSS grayscale(1) brightness(0.45) contrast(0.8), drawn at alpha 0.85."""
    out = bytearray()
    for i in range(0, len(rgba), 4):
        r, g, b, a = rgba[i:i + 4]
        v = (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255 * 0.45
        v = min(1.0, max(0.0, (v - 0.5) * 0.8 + 0.5))
        c = int(v * 255 + 0.5)
        out += bytes((c, c, c, int(a * 0.85 + 0.5)))
    return bytes(out)


def entry(kind, name, w, h, rgba):
    label = name.encode('ascii')
    return struct.pack('<HH', kind, len(label)) + label + struct.pack('<HH', w, h) + rgba


def build(ui=UI):
    manifest = json.loads((ui / 'items/items.json').read_text())
    entries, provenance = [], {}
    for item in manifest['items']:
        if item.get('hud') or item.get('gadget'):
            continue  # steroids and the medkit gadget: not mission items
        w, h, rgba = read_rgba_png(ui / 'items' / item['file'])
        if (w, h) != (16, 16):
            raise SystemExit(f"{item['file']}: expected 16x16, got {w}x{h}")
        entries += [entry(0, item['name'], w, h, rgba), entry(1, item['name'], w, h, silhouette(rgba))]
        provenance[item['name']] = item['file']
    w, h, frame = read_rgba_png(ui / 'item-frame.png')
    entries += [entry(2, 'frame', w, h, frame), entry(3, 'frame', w, h, swap(frame, RAMP_ORANGE)),
                entry(4, 'frame', w, h, swap(frame, RAMP_STEEL))]
    w, h, tick = read_rgba_png(ui / 'items/mark-used.png')
    if w > 16 or h > 16:
        raise SystemExit(f'mark-used.png: at most 16x16, got {w}x{h}')
    entries.append(entry(5, 'used', w, h, tick))
    provenance['used tick'] = 'mark-used.png'
    return b'TTKMIS1\0' + struct.pack('<I', len(entries)) + b''.join(entries), provenance


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-mission-items.pack'))
    args = ap.parse_args()
    pack, provenance = build()
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(pack)
    out.with_suffix('.provenance.json').write_text(json.dumps({
        'source': 'assets/ui/items (items.json, 16x16 icons) and assets/ui/item-frame.png: original project art',
        'icons': provenance,
        'frame_sha256': hashlib.sha256((UI / 'item-frame.png').read_bytes()).hexdigest(),
        'format': 'TTKMIS1; see tools/local/build_ttk_mission_items.py',
        'note': 'Built by recomp/tools/local/build_ttk_mission_items.py (D08A5).'}, indent=2) + '\n')
    print(out)


if __name__ == '__main__':
    main()
