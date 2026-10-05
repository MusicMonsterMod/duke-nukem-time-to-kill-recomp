#!/usr/bin/env python3
"""Derive the Modernized control lease's level overlay guards (D22B) from the
owned disc, read-only.

Every LEVELxx.OVR loads at 0x800ca968 and has one layout: its level tag word,
strings and jump tables, the code (ending at its last `jr $ra`), then one data
block that the code addresses with lui pairs. The level writes only a few
words at the very end of that data block: variables it stores directly and
hit-position vectors it passes as the fifth argument of 0x8007177c. Those
trailing bytes are level state; everything before them is authenticated.

The tool checks that layout for each selectable level and fails if a level
does not fit it (a write outside the trailing block, data referenced inside
the code, a write followed by unwritten bytes). It prints the C++ table for
src/ttk/control_guards.inc, or with --check compares that file.

  python3 recomp/tools/local/level_overlay_guards.py [--disc BIN] [--check]
"""
import argparse
import hashlib
import re
import struct
import sys
from pathlib import Path

from disc_lab import Disc

ROOT = Path(__file__).resolve().parents[2]
BASE = 0x800ca968
HIT_VECTOR_CALL = 0x8007177c
HIT_VECTOR_BYTES = 16
# The original title level-select cheat's levels (documentation/101): 0-31
# without 4 and 13 (XXX), 14 (animation test), 15-20 (two-player) and 30-31.
SELECTABLE = (0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 21, 22, 23, 24, 25, 26, 27, 28, 29)
STORES = {0x28: 1, 0x29: 2, 0x2b: 4}  # sb, sh, sw


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def analyse(data):
    """Return (code_end, data_refs, writes) as offsets into the overlay."""
    words = len(data) // 4
    ins = struct.unpack_from(f'<{words}I', data)
    code_end = max(4 * i for i, w in enumerate(ins) if w == 0x03e00008) + 8
    hi, refs, writes = {}, [], []
    for i, w in enumerate(ins[:code_end // 4]):
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xffff
        if op == 0x0f:
            hi[rt] = imm << 16
            continue
        if rs not in hi:
            continue
        if op in (0x09, 0x0d):          # addiu / ori: an address formed in rt
            a = (hi[rs] + (imm if op == 0x0d else s16(imm))) & 0xffffffff
            if BASE < a < BASE + len(data):
                refs.append(a - BASE)
                # A vector formed right before the hit-vector call is its
                # output argument (LEVEL00/01/03/09 pass it at 0x10($sp)).
                window = ins[i + 1:i + 6]
                if any(x == (0x0c000000 | (HIT_VECTOR_CALL & 0x3ffffff) >> 2) for x in window):
                    writes.append((a - BASE, HIT_VECTOR_BYTES))
        elif 0x20 <= op <= 0x2b:        # load/store with a lui base
            a = (hi[rs] + s16(imm)) & 0xffffffff
            if BASE < a < BASE + len(data):
                refs.append(a - BASE)
                if op in STORES:
                    writes.append((a - BASE, STORES[op]))
    return code_end, refs, writes


def level_guard(number, data):
    code_end, refs, writes = analyse(data)
    trailing = [r for r in refs if r >= code_end]
    if not trailing or min(trailing) != code_end:
        raise SystemExit(f'LEVEL{number:02d}: data block does not start at code end {code_end:#x}')
    if any(o < code_end for o, _ in writes):
        raise SystemExit(f'LEVEL{number:02d}: writes inside its code/header: {writes}')
    scratch = min((o for o, _ in writes), default=len(data))
    if writes and max(o + n for o, n in writes) > len(data):
        raise SystemExit(f'LEVEL{number:02d}: write past the file end')
    # Nothing unwritten may follow the first scratch byte except the rest of
    # the written block (the trailing block is contiguous to the file end).
    covered = set()
    for o, n in writes:
        covered.update(range(o, o + n))
    if writes and not set(range(scratch, len(data))) <= covered:
        raise SystemExit(f'LEVEL{number:02d}: unwritten bytes inside the trailing block {scratch:#x}..{len(data):#x}')
    body = scratch & ~3
    return dict(level=number, tag=struct.unpack_from('<I', data)[0], size=len(data), code_end=code_end,
                body=body, scratch=len(data) - body, digest=hashlib.sha256(data[:body]).hexdigest())


def guards(disc):
    found = {}
    for e in disc.files():
        m = re.search(r'/LEVEL(\d\d)\.OVR;1$', e['path'])
        if m and int(m.group(1)) in SELECTABLE:
            found[int(m.group(1))] = (e['path'], disc.read(e['lba'], e['size']))
    missing = set(SELECTABLE) - set(found)
    if missing:
        raise SystemExit(f'missing level overlays: {sorted(missing)}')
    rows = []
    for n in SELECTABLE:
        path, data = found[n]
        row = level_guard(n, data)
        row['path'] = path.split(';')[0]
        rows.append(row)
    # The lease picks the body by tag, so each tag must name one level.
    if len({r['tag'] for r in rows}) != len(rows):
        raise SystemExit('duplicate level overlay tags')
    return rows


def table(rows):
    out = []
    for r in rows:
        tail = f'{r["scratch"]}-byte scratch tail' if r['scratch'] else 'no scratch tail'
        out.append(f'    // LEVEL{r["level"]:02d}.OVR, {r["size"]} bytes, code to {BASE + r["code_end"]:#x}; {tail}.')
        out.append(f'    {{{r["tag"]:#x}, {{{{0x800ca968, {r["body"]}, "{r["digest"]}"}}}}}},')
    return '\n'.join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--disc', default=str(ROOT / 'disc/time-to-kill.bin'))
    ap.add_argument('--check', action='store_true', help='compare src/ttk/control_guards.inc')
    args = ap.parse_args()
    rows = guards(Disc(args.disc))
    if not args.check:
        for r in rows:
            print(f'# LEVEL{r["level"]:02d} tag {r["tag"]:#x} size {r["size"]} code_end {r["code_end"]:#x} '
                  f'body {r["body"]} scratch {r["scratch"]}', file=sys.stderr)
        print(table(rows))
        return
    text = (ROOT / 'src/ttk/control_guards.inc').read_text()
    have = set(re.findall(r'\{(0x[0-9a-f]+|\d+), \{\{0x800ca968, (\d+), "([0-9a-f]{64})"\}\}\}', text))
    want = {(f'{r["tag"]:#x}', str(r['body']), r['digest']) for r in rows}
    have = {(f'{int(t, 0):#x}', s, d) for t, s, d in have}
    if have != want:
        print('control_guards.inc level_overlays differ from the disc:', file=sys.stderr)
        for x in sorted(want - have):
            print('  missing', x, file=sys.stderr)
        for x in sorted(have - want):
            print('  unexpected', x, file=sys.stderr)
        raise SystemExit(1)
    print(f'level_overlays match the owned disc ({len(rows)} levels)')


if __name__ == '__main__':
    main()
