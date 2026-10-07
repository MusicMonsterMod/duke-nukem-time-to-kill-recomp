#!/usr/bin/env python3
"""Audit the Modernized control guards (src/ttk/control_guards.inc) for bytes
the original game writes at run time (D08V1), read-only.

A guard that covers game state turns the Modernized lease off the moment the
game writes it, for the rest of the session (2026-10-07: the pole/hang side
probe 0x800439e4 cleared bit 0x40 of flag-table entry 196 on a chain top exit).

Static mode scans SLUS_005.83 and every .OVR on the owned disc for stores
whose lui-formed base lands in a guard range, following lui values through
branch delay slots into the branch target. An indexed store (base + unknown
index) is reported when its base is inside a guard range. Ranges listed in
state_guards are reported separately with the mask they apply; the audit
fails if a store hits a code guard, or a masked state range from a writer
that is not in the reviewed list below.

Live mode (--port) reads every guard range from a running game's debug port
and reports words that differ from the disc (after the state masks).

  python3 recomp/tools/local/guard_writer_audit.py [--disc BIN] [--port 9123]
"""
import argparse
import re
import struct
import sys
from pathlib import Path

from disc_lab import Disc

ROOT = Path(__file__).resolve().parents[2]
OVERLAY_BASE = 0x800ca968
STORES = {0x28: 'sb', 0x29: 'sh', 0x2b: 'sw'}
LOADS = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26}
CALLER_SAVED = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 31}
# Original writers of state_guards ranges, reviewed (control_guards.inc):
# ledge hang sets / object hang clears 0x40 on 149/152/153, and the pole/hang
# side probe sets or clears 0x40 on the current animation's entry.
REVIEWED_STATE_WRITERS = {
    ('EXE', 0x8004c778), ('EXE', 0x8004c784), ('EXE', 0x8004c78c),
    ('EXE', 0x8004c804), ('EXE', 0x8004c810), ('EXE', 0x8004c818),
    ('EXE', 0x8004c8c4), ('EXE', 0x8004c8d0), ('EXE', 0x8004c8d8),
    ('EXE', 0x80055444), ('EXE', 0x80055450), ('EXE', 0x8005545c),
    ('EXE', 0x80043bec),
    *((f'/D2/DB2{n}/BONUS.OVR', pc) for n in range(1, 7) for pc in (0x800cf9a8, 0x800cfc70)),
}
# Stores into code-guard ranges, reviewed: the 0x800bcbb4 handler table's
# setter 0x8001bcf8 (sw at 0x8001bd14) has no jal or pointer to it anywhere on
# the disc, so the table never changes.
REVIEWED_CODE_STORES = {('EXE', 0x8001bd14)}


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def guard_ranges(text):
    code = [(int(a, 16), int(n)) for a, n in re.findall(r'\{(0x[0-9a-f]+), (\d+), "[0-9a-f]{64}"\}', text)
            if int(a, 16) != OVERLAY_BASE]
    state = [(int(a, 16), int(n), int(m, 16))
             for a, n, m in re.findall(r'\{(0x[0-9a-f]+), (\d+), (0x[0-9a-f]+), "[0-9a-f]{64}"\}', text)]
    return code, state


def stores(words, base):
    """Yield (pc, mnemonic, address, indexed) for stores with a lui-formed base."""
    regs, pending, clear_after = {}, {}, None
    for i, w in enumerate(words):
        pc = base + 4 * i
        if clear_after == pc:
            regs, clear_after = {}, None
        for r, v in pending.pop(pc, {}).items():
            regs.setdefault(r, v)
        op, rs, rt, rd, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, w & 0xffff
        fn = w & 63
        branch = op in (1, 4, 5, 6, 7) or op in (2, 3) or (op == 0 and fn in (8, 9))
        if branch:
            # Process the delay slot first, then hand the state to the target.
            target = None
            if op in (1, 4, 5, 6, 7):
                target = pc + 4 + 4 * s16(imm)
            elif op == 2:
                target = (pc & 0xf0000000) | ((w & 0x3ffffff) << 2)
            nxt = words[i + 1] if i + 1 < len(words) else 0
            if (nxt >> 26) == 0x0f and target is not None:
                st = dict(regs)
                st[(nxt >> 16) & 31] = ('c', (nxt & 0xffff) << 16)
                pending.setdefault(target, {}).update(st)
            elif target is not None:
                pending.setdefault(target, {}).update(regs)
            if op == 3 or (op == 0 and fn == 9):
                for r in CALLER_SAVED:
                    regs.pop(r, None)
            if op == 2 or (op == 0 and fn == 8):
                clear_after = pc + 8
            continue
        if op == 0x0f:
            regs[rt] = ('c', imm << 16)
        elif op in (0x09, 0x0d) and rs in regs:
            k, v = regs[rs]
            regs[rt] = (k, (v + (imm if op == 0x0d else s16(imm))) & 0xffffffff)
        elif op == 0 and fn == 0x21:
            a, b = regs.get(rs), regs.get(rt)
            if (a is None) != (b is None):
                regs[rd] = ('b', (a or b)[1])
            else:
                regs.pop(rd, None)
        elif op in STORES:
            if rs in regs:
                k, v = regs[rs]
                yield pc, STORES[op], (v + s16(imm)) & 0xffffffff, k == 'b'
        elif op in LOADS:
            regs.pop(rt, None)
        elif op == 0:
            regs.pop(rd, None)
        elif op not in (0x2a, 0x2e, 0x32, 0x3a):  # lwl/swl/lwc2/swc2 excluded
            regs.pop(rt, None)


def images(disc_path):
    exe = (ROOT / 'disc/SLUS_005.83').read_bytes()
    t0 = struct.unpack_from('<I', exe, 0x18)[0]
    yield 'EXE', exe[0x800:], t0
    disc = Disc(disc_path)
    seen = set()
    for e in disc.files():
        m = re.search(r'/([^/]+)\.OVR;1$', e['path'])
        if m and e['path'] not in seen:
            seen.add(e['path'])
            yield e['path'].split(';')[0], disc.read(e['lba'], e['size']), OVERLAY_BASE


def audit(disc_path, text):
    code, state = guard_ranges(text)
    code_hits, state_hits = [], []
    for name, data, base in images(disc_path):
        words = struct.unpack_from(f'<{len(data) // 4}I', data)
        for pc, mn, addr, indexed in stores(words, base):
            for a, n in code:
                if a <= addr < a + n:
                    code_hits.append((name, pc, mn, addr, indexed, a))
            for a, n, mask in state:
                if a <= addr < a + n:
                    state_hits.append((name, pc, mn, addr, indexed, a, mask))
    return code, state, code_hits, state_hits


def live(port, text):
    from debug_client import request
    code, state = guard_ranges(text)
    exe = (ROOT / 'disc/SLUS_005.83').read_bytes()
    t0 = struct.unpack_from('<I', exe, 0x18)[0]
    diffs = []
    for a, n, mask in [(a, n, 0xffffffff) for a, n in code] + state:
        got = bytes.fromhex(request({'cmd': 'read_ram', 'addr': hex(a), 'len': n}, port)['hex'])
        want = exe[0x800 + a - t0:0x800 + a - t0 + n]
        for o in range(0, n, 4):
            g, w = struct.unpack_from('<I', got, o)[0], struct.unpack_from('<I', want, o)[0]
            if (g ^ w) & mask:
                diffs.append((a + o, g, w, mask))
    return diffs


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--disc', default=str(ROOT / 'disc/time-to-kill.bin'))
    ap.add_argument('--port', type=int, help='compare a running game instead of scanning')
    args = ap.parse_args()
    text = (ROOT / 'src/ttk/control_guards.inc').read_text()
    if args.port:
        diffs = live(args.port, text)
        for a, g, w, mask in diffs:
            print(f'{a:#010x}: live {g:#010x} disc {w:#010x} (mask {mask:#010x})')
        print(f'{len(diffs)} guarded word(s) differ from the disc')
        raise SystemExit(1 if diffs else 0)
    code, state, code_hits, state_hits = audit(args.disc, text)
    bad = False
    for name, pc, mn, addr, indexed, a in code_hits:
        known = (name, pc) in REVIEWED_CODE_STORES
        print(f'code guard {a:#010x}: {name} {pc:#010x} {mn} {"indexed from " if indexed else ""}{addr:#010x}'
              f'{"  (reviewed: unreachable)" if known else "  UNREVIEWED"}')
        bad |= not known
    for name, pc, mn, addr, indexed, a, mask in sorted(set(state_hits)):
        known = (name, pc) in REVIEWED_STATE_WRITERS
        print(f'state {a:#010x} mask {mask:#010x}: {name} {pc:#010x} {mn} '
              f'{"indexed from " if indexed else ""}{addr:#010x}{"" if known else "  UNREVIEWED"}')
        bad |= not known
    print(f'{len(code)} code guards, {len(state)} state tables: '
          f'{len(code_hits)} code-guard store(s), {len(set(state_hits))} state store(s)')
    raise SystemExit(1 if bad else 0)


if __name__ == '__main__':
    main()
