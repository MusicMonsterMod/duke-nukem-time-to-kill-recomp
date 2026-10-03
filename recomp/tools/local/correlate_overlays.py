#!/usr/bin/env python3
"""Correlate owned-disc OVR bytes with a local 2 MiB RAM snapshot (read-only).

Chunk matches are residency candidates, not execution/loader ownership proof.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path

from disc_lab import Disc, EXE_SHA256

ROOT = Path(__file__).resolve().parents[2]


def correlate(ram, payload):
    votes = Counter()
    chunks = 0
    for offset in range(0, len(payload) - 255, 256):
        chunk = payload[offset:offset + 256]
        if len(set(chunk)) < 8:
            continue
        chunks += 1
        where = ram.find(chunk)
        while where >= 0:
            base = where - offset
            if 0 <= base <= len(ram) - len(payload):
                votes[base] += 1
            where = ram.find(chunk, where + 1)
    candidates = []
    for base, count in votes.most_common():
        if count < 3:
            continue
        live = ram[base:base + len(payload)]
        differences = [i for i, (a, b) in enumerate(zip(live, payload)) if a != b]
        candidates.append({'load_address_candidate': hex(0x80000000 + base),
                           'matching_chunks': count, 'tested_chunks': chunks,
                           'equal_bytes': len(payload) - len(differences),
                           'size': len(payload), 'exact_match': live == payload,
                           'first_differing_offsets': [hex(i) for i in differences[:32]]})
    return candidates


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ram', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    ram = args.ram.read_bytes()
    if len(ram) != 0x200000:
        parser.error('expected a complete 2 MiB RAM snapshot')
    if hashlib.sha256((ROOT / 'disc/SLUS_005.83').read_bytes()).hexdigest() != EXE_SHA256:
        parser.error('unsupported executable')
    disc = Disc(ROOT / 'disc/time-to-kill.bin')
    groups = {}
    try:
        for entry in disc.files():
            if not entry['path'].endswith('.OVR;1'):
                continue
            data = disc.read(entry['lba'], entry['size'])
            sha = hashlib.sha256(data).hexdigest()
            if sha not in groups:
                groups[sha] = {'sha256': sha, 'files': [], 'candidates': correlate(ram, data)}
            groups[sha]['files'].append(entry['path'])
    finally:
        disc.close()
    result = {'ram_sha256': hashlib.sha256(ram).hexdigest(), 'unique_overlays_examined': len(groups),
              'matches': [g for g in groups.values() if g['candidates']],
              'limitation': 'At least three 256-byte nontrivial chunks must agree on a base. An exact file match proves bytes are resident in this snapshot, not that code executed or that future frames retain this overlay.'}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
