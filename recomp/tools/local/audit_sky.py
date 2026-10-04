#!/usr/bin/env python3
"""Read-only D17O sky provenance audit of an owned EXE/disc and private RAM dump.

Writes derived metadata only. Never reads or changes player cards. Example:
  python3 recomp/tools/local/audit_sky.py --ram PRIVATE_RAM.bin --output REPORT.json
See documentation/92-d17o-sky-intake.md for capture conditions and limitations.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path
from disc_lab import Disc, EXE_SHA256

ROOT = Path(__file__).resolve().parents[2]
SKY_ADDRESS, SKY_SIZE = 0x800388E4, 0x578
SKY_SHA = 'c78073edff91c270605ab870d7804078115604c2c49073eb79d359fd5322df13'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def audit(exe_path, disc_path, ram_path):
    exe, ram = exe_path.read_bytes(), ram_path.read_bytes()
    if digest(exe) != EXE_SHA256 or len(ram) != 0x200000:
        raise ValueError('Expected owned US executable and a 2 MiB private RAM dump')
    base = struct.unpack_from('<I', exe, 0x18)[0]
    routine = exe[SKY_ADDRESS-base+0x800:SKY_ADDRESS-base+0x800+SKY_SIZE]
    if digest(routine) != SKY_SHA or ram[SKY_ADDRESS & 0x1fffff:(SKY_ADDRESS & 0x1fffff)+SKY_SIZE] != routine:
        raise ValueError('Active resident sky code does not match the authenticated executable')

    def read(address, size):
        if not 0x80010000 <= address <= 0x80200000-size:
            raise ValueError(f'Out-of-range guest pointer: {address:08x}')
        return ram[address & 0x1fffff:(address & 0x1fffff)+size]

    def word(address):
        return struct.unpack('<I', read(address, 4))[0]

    disc = Disc(str(disc_path))
    try:
        entry = next(f for f in disc.files() if f['path'] == '/D0/DB00/DB00MOD.BIN;1')
        assets = disc.read(entry['lba'], entry['size'])
    finally:
        disc.close()
    rows = []
    for role, address in [('fixed', 0x800da344), ('slow', 0x800da45c),
                          ('backdrop', 0x800da464), ('fast', 0x800da46c)]:
        ident = word(address)
        row = dict(role=role, selector_address=hex(address), model_id=hex(ident))
        if ident:
            if ident >= 2048:
                raise ValueError('Unexpected mesh selector')
            mesh = word(word(0x800ddb18)+4*ident)
            count = read(mesh+6, 1)[0]
            vertices = read(word(mesh+0x10), count*8)
            xyz = [struct.unpack_from('<3h', vertices, i*8) for i in range(count)]
            offsets, offset = [], -1
            while True:
                offset = assets.find(vertices, offset+1)
                if offset < 0:
                    break
                offsets.append(hex(offset))
            row.update(mesh=hex(mesh), vertices=count, vertex_sha256=digest(vertices),
                       bounds=[[min(v[k] for v in xyz), max(v[k] for v in xyz)] for k in range(3)],
                       matching_vertex_offsets=offsets)
        rows.append(row)
    return dict(executable_sha256=digest(exe), ram_sha256=digest(ram),
                resident_code=dict(address=hex(SKY_ADDRESS), bytes=SKY_SIZE, sha256=SKY_SHA),
                asset=dict(path=entry['path'], bytes=len(assets), sha256=digest(assets)),
                layers=rows,
                limitation='Vertex matches establish geometry provenance, not texture names or full campaign coverage.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=ROOT/'disc/SLUS_005.83')
    parser.add_argument('--disc', type=Path, default=ROOT/'disc/time-to-kill.bin')
    parser.add_argument('--ram', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    report = audit(args.exe, args.disc, args.ram)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(args.output)


if __name__ == '__main__':
    main()
