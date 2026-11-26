#!/usr/bin/env python3
"""D61B: build the hidden levels' sound pack from the owned disc.

Each level folder's SND.DAT (owned USA SLUS-00583 disc, read only) is the
game's sound table followed by a VAB (documentation/561-d61b-hidden-levels.md):

  0x00  u32 VAB offset (= table size, copied whole by 0x8006a2c0 into a
        0x2000-byte buffer, *0x800c39c4), u32 VH size, u32 VB size
  0x0c  29 groups of (u32 count, u32 offset); the loader rewrites the
        offsets as RAM addresses; entries follow from 0xf4
  entry 16 bytes: byte 0 * 16 + byte 1 the sample index k (plays VAB sample
        k + 1), byte 2 the pitch base, byte 4 bit 0x80 disables the entry
        (0x80068b20 then returns no sound), bytes 6-7 the rate
  VH    its last 0x200 bytes hold 256 u16 sample sizes / 8; 0x8006a3ac
        turns the first 140 into SPU addresses from 0x1010
  VB    uploaded whole to SPU 0x1010 (0x8006a3fc)

The hidden levels lack sounds the shipped levels have: no gadget group 0x1c
at all in 13, 14, 30 and 31 (the silent jetpack), every gadget entry disabled
in 4, other ids disabled, and a prototype menu group. For each hidden level
this pack carries, from a shipped level of the same era (TIME TO KILL for the
modern ones, DUKE HILL for the Old West True Grit):

  - the whole gadget group 0x1c;
  - the menu group 0 (ids 0x0-0x4);
  - every other id of groups 0-7 that is disabled in the hidden level and
    plays a sound in the donor.

Ids the hidden level plays with its own, different sample (its variants and
re-encodes) are kept. Each donor sample is added once per level, at the next
free sample index after the level's own; the entry's index is rewritten to it.

Output: a TTKSND1 pack (little endian): magic, version 1, level count; per
level: index, donor, sha256 of the level's table bytes [0, 0xc) and [0xf4,
table size), its table size, VB size and first free index, entry count, then
(id, 16 entry bytes) each, sample count, then (index, size, bytes) each.
Retail-derived: build output only, never committed. Loaded by
src/ttk/hidden_level_sound.inc.
"""
import argparse, hashlib, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
GROUPS = 29
ENTRIES = 12 + 8 * GROUPS          # 0xf4
TABLE_BUFFER = 0x2000
SAMPLE_LIMIT = 140                 # 0x8006a3ac builds 0x8c addresses
GADGET = 0x1c
HIDDEN = {4: 1, 13: 0, 14: 0, 30: 0, 31: 0}   # hidden level: donor


def folder(level):
    return f'/D{level // 10}/DB{level:02d}/SND.DAT;1'


class Bank:
    def __init__(self, data):
        self.data = data
        self.table_size, self.vh_size, self.vb_size = struct.unpack_from('<III', data, 0)
        if self.table_size > TABLE_BUFFER or self.table_size < ENTRIES:
            raise SystemExit('unexpected sound table size')
        self.groups = [struct.unpack_from('<II', data, 12 + 8 * g) for g in range(GROUPS)]
        base = self.groups[0][1] - ENTRIES
        self.entries = {}
        for g, (count, ptr) in enumerate(self.groups):
            o = ptr - base
            for i in range(count):
                self.entries[g << 12 | i] = data[o + 16 * i:o + 16 * i + 16]
        vh = data[self.table_size:self.table_size + self.vh_size]
        if vh[:4] != b'pBAV':
            raise SystemExit('no VAB header after the sound table')
        # The VH's sample count; the size table can hold stale entries past it
        # (the empty room and the sound test), which no address reaches.
        self.used = struct.unpack_from('<H', vh, 0x16)[0]
        self.sizes = [s * 8 for s in struct.unpack_from('<256H', vh, self.vh_size - 0x200)]
        vb = data[self.table_size + self.vh_size:self.table_size + self.vh_size + self.vb_size]
        if self.sizes[0] or sum(self.sizes[1:self.used + 1]) != self.vb_size or len(vb) != self.vb_size:
            raise SystemExit('VAB sample sizes do not add up to the VB')
        self.samples, at = {}, 0
        for v in range(1, self.used + 1):
            self.samples[v - 1] = vb[at:at + self.sizes[v]]
            at += self.sizes[v]

    def digest(self):
        return hashlib.sha256(self.data[:12] + self.data[ENTRIES:self.table_size]).digest()


def index(entry):
    return entry[0] * 16 + entry[1]


def disabled(entry):
    return bool(entry[4] & 0x80)


def patch(hidden, donor):
    """The entries to write and the samples to add for one hidden level."""
    ids = []
    for ident, entry in sorted(donor.entries.items()):
        group = ident >> 12
        if group == GADGET or group == 0:
            ids.append(ident)
        elif group < 8 and not disabled(entry):
            own = hidden.entries.get(ident)
            if own is not None and disabled(own):
                ids.append(ident)
    added, entries, samples = {}, [], []
    for ident in ids:
        entry = bytearray(donor.entries[ident])
        if not disabled(entry):
            k = index(entry)
            if k not in donor.samples:
                raise SystemExit(f'donor id {ident:#x} plays a missing sample')
            if k not in added:
                added[k] = hidden.used + len(added)
                samples.append((added[k], donor.samples[k]))
            entry[0], entry[1] = added[k] >> 4, added[k] & 15
        entries.append((ident, bytes(entry)))
    if hidden.used + len(added) > SAMPLE_LIMIT:
        raise SystemExit('more samples than the address table holds')
    # A missing gadget group is appended after the level's own entries.
    if not hidden.groups[GADGET][0]:
        grow = 16 * sum(1 for i, _ in entries if i >> 12 == GADGET)
        if hidden.table_size + grow > TABLE_BUFFER:
            raise SystemExit('the gadget group does not fit the table buffer')
    return entries, samples


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-sound.pack'))
    args = ap.parse_args()

    disc = Disc(args.image)
    try:
        files = {f['path']: f for f in disc.files()}
        def bank(level):
            entry = files.get(folder(level))
            if not entry:
                raise SystemExit(f'{folder(level)} not found')
            return Bank(disc.read(entry['lba'], entry['size']))
        banks = {level: bank(level) for level in set(HIDDEN) | set(HIDDEN.values())}
    finally:
        disc.close()

    out = bytearray(b'TTKSND1\0') + struct.pack('<II', 1, len(HIDDEN))
    for level, donor in sorted(HIDDEN.items()):
        hidden = banks[level]
        entries, samples = patch(hidden, banks[donor])
        out += struct.pack('<II', level, donor) + hidden.digest()
        out += struct.pack('<IIII', hidden.table_size, hidden.vb_size, hidden.used, len(entries))
        for ident, entry in entries:
            out += struct.pack('<I', ident) + entry
        out += struct.pack('<I', len(samples))
        for k, blob in samples:
            out += struct.pack('<II', k, len(blob)) + blob
        print(f'level {level}: donor {donor}, {len(entries)} entries, {len(samples)} samples '
              f'({sum(len(b) for _, b in samples)} bytes) from index {hidden.used}')
    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    Path(args.output).write_bytes(out)
    print(f'{args.output}: {len(out)} bytes, sha256 {hashlib.sha256(out).hexdigest()}')


if __name__ == '__main__':
    main()
