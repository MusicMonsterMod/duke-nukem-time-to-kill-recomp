#!/usr/bin/env python3
"""Decode Time to Kill's object-handler state from an abnormal-exit snapshot."""
import argparse
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]


def inspect(trace, ram):
    if len(ram) != 0x200000:
        raise ValueError('expected a complete 2 MiB RAM snapshot')
    if len(trace.get('gpr', [])) != 32:
        raise ValueError('this older exit trace has no complete register snapshot; reproduce with --diagnostics')
    registers = [int(value, 16) for value in trace['gpr']]

    def physical(address, size):
        address &= 0x1fffffff
        if address + size > len(ram):
            raise ValueError('object/table pointer is outside RAM')
        return address

    def word(address):
        return struct.unpack_from('<I', ram, physical(address, 4))[0]

    result = {'final_pc': trace['final_pc'], 'final_ra': trace['final_ra'],
              'matches_object_dispatch_return': registers[31] == 0x80025c04,
              'v0': hex(registers[2]), 'object': hex(registers[4]),
              'note': 'A null final PC does not alone prove that the original indirect-call target was null.'}
    if result['matches_object_dispatch_return']:
        obj = registers[4]
        state = ram[physical(obj + 0x15, 1)]
        table = word(0x800dd850)
        result.update({'object_state': state, 'table': hex(table),
                       'table_slot': hex(table + state * 4),
                       'handler_now': hex(word(table + state * 4)),
                       'object_header_hex': ram[physical(obj, 32):physical(obj, 32)+32].hex()})
        # The actor animation buffers must never be dispatched as world objects.
        # Their allocation is a per-level pointer array of 0x544-byte slots.
        level = word(0x800be570)
        overlaps = []
        if level < 30:
            count = struct.unpack_from('<h', ram, physical(0x800c56b4 + level * 2, 2))[0]
            slots = word(0x800ddb08)
            if 0 < count <= 64 and 0x80010000 <= slots <= 0x801fff00:
                for index in range(count):
                    base = word(slots + index * 4)
                    if 0x80010000 <= base <= 0x801ffabc and base <= obj < base + 0x544:
                        overlaps.append({'slot': index, 'base': hex(base), 'end_exclusive': hex(base + 0x544)})
        result['current_animation_buffer_overlap'] = overlaps
        memberships = []
        for head in (0x800c5694, 0x800c5698):
            node = word(head)
            seen = set()
            while node and len(seen) < 1024:
                if not 0x800ce418 <= node < 0x800ce418 + 1024 * 12 or (node - 0x800ce418) % 12 or node in seen:
                    break
                seen.add(node)
                if word(node + 8) == obj:
                    memberships.append({'head': hex(head), 'node': hex(node)})
                node = word(node + 4)
        result['scene_list_memberships'] = memberships

    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--trace', type=Path, default=ROOT / 'psx_cps_exit_trace.json')
    parser.add_argument('--ram', type=Path, default=ROOT / 'psx_cps_exit_ram.bin')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = inspect(json.loads(args.trace.read_text()), args.ram.read_bytes())
    text = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.write_text(text)
    print(text, end='')
