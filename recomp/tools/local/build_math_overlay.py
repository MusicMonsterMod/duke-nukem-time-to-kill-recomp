#!/usr/bin/env python3
"""Compile TTK's observed post-initialization geometry-math code variant.

The game changes ADD to ADDU at two sites in this routine. Compile those exact
bytes through the normal overlay compiler, retaining its live code-byte guards.
Never modify the player's executable, disc, saves, or generated C by hand.
"""
import argparse
import base64
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from disc_lab import EXE_SHA256

ROOT = Path(__file__).resolve().parents[2]
LOAD, SIZE = 0x800B3D9C, 0xC0
PATCHES = ((0x800B3DBC, 0x016C5820, 0x016C5821),
           (0x800B3DC0, 0x016D1020, 0x016D1021))


def recipe(exe):
    if hashlib.sha256(exe).hexdigest() != EXE_SHA256:
        raise ValueError('boot executable differs from the inspected revision')
    base = int.from_bytes(exe[0x18:0x1c], 'little')
    data = bytearray(exe[2048 + LOAD - base:2048 + LOAD - base + SIZE])
    if len(data) != SIZE:
        raise ValueError('geometry routine is missing')
    for address, original, initialized in PATCHES:
        offset = address - LOAD
        if int.from_bytes(data[offset:offset + 4], 'little') != original:
            raise ValueError(f'geometry instruction guard failed at {address:08X}')
        data[offset:offset + 4] = initialized.to_bytes(4, 'little')
    entries = [hex(LOAD)]
    return {'schema': 'psxrecomp overlay capture v2', 'load_addr': hex(LOAD),
            'size': SIZE, 'bytes_b64': base64.b64encode(data).decode(),
            'executed_pcs': [], 'seeds': entries, 'function_entry_pcs': entries,
            'dispatch_entry_pcs': entries, 'static_dispatch_entry_pcs': entries,
            'producer_ranges': [{'start': hex(LOAD), 'end': hex(LOAD + SIZE)}],
            'strict_producer_ranges': True}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--if-ready', action='store_true')
    parser.add_argument('--quiet', action='store_true')
    parser.add_argument('--force', action='store_true')
    args = parser.parse_args()
    recompiler = ROOT / 'build-recompiler' / ('psxrecomp-game.exe' if os.name == 'nt' else 'psxrecomp-game')
    exe = ROOT / 'disc/SLUS_005.83'
    if not exe.is_file() or not recompiler.is_file() or not shutil.which('gcc'):
        if args.if_ready:
            print('geometry math shard: skipped, prerequisites unavailable')
            return 0
        parser.error('prepared executable, recompiler and gcc are required')
    try:
        record = recipe(exe.read_bytes())
    except ValueError as exc:
        parser.error(str(exc))
    captures = ROOT / 'analysis/math-overlay/capture.json'
    captures.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps([record], indent=2) + '\n'
    if not captures.exists() or captures.read_text() != text:
        captures.write_text(text)
    command = [sys.executable, str(ROOT / 'psxrecomp/tools/compile_overlays.py'),
               '--captures', str(captures), '--game-toml', str(ROOT / 'game.local.toml'),
               '--recompiler', str(recompiler), '--runtime-include', str(ROOT / 'psxrecomp/runtime/include'),
               '--out-dir', str(ROOT / 'build-local/cache'), '--compiler', 'gcc',
               '--gcc', 'gcc', '--flavor', '0', '--cps', '--jobs', str(args.jobs)]
    if args.force:
        command.append('--force')
    env = os.environ.copy()
    env.pop('PSX_OVERLAY_CAPTURES', None)
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True)
    output = result.stdout + result.stderr
    summaries = [line for line in output.splitlines() if line.startswith('PSX_SHARD_RESULT')]
    fields = dict(item.split('=', 1) for item in summaries[-1].split()[1:]) if summaries else {}
    if result.returncode or not summaries or fields.get('failed', '0') != '0':
        sys.stderr.write(output)
        return result.returncode or 1
    if not args.quiet:
        print(output, end='')
    print('geometry math shard: rebuilt' if fields.get('ok', '0') != '0' else 'geometry math shard: current')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
