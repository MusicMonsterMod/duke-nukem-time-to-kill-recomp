#!/usr/bin/env python3
"""Compile this exact SLUS-00583 MOVIE.OVR from the user's prepared raw disc.

No decoded video replacement: the original MIPS MDEC/VLC routines become native
functions with the runtime's code-byte guards and guest cycle accounting.
Evidence and limitations: documentation/13-fmv-fidelity-pass.md.

The shard cache namespace includes the overlay config hash of game.local.toml
(including its host hook list), so any hook change strands the previous shard
and the intro silently falls back to the interpreter. The CMake build and
run.py therefore call this with --if-ready on every build/launch (about 0.2 s
when the shard is current). See documentation/59-fmv-shard-namespace.md.
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

from disc_lab import Disc, EXE_SHA256

ROOT = Path(__file__).resolve().parents[2]
MOVIE_SHA256 = 'eacbb6eadd142bc6d28df17b13923722a146355ec12f47df9278ca2c19028e52'
LOAD = 0x800CA968
CODE_START, CODE_END = 0x800CA9BC, 0x800CB4B0
# Verified direct calls, stack prologues, and the leaf routine after CA9BC.
# CB07C is the hot VLC decoder; CB3CC initializes its lookup data.
ENTRIES = (0x800CA9BC, 0x800CA9F0, 0x800CAA7C, 0x800CAB14, 0x800CAB20,
           0x800CAB9C, 0x800CABBC, 0x800CABF8, 0x800CAC34, 0x800CAC58,
           0x800CAC7C, 0x800CAD6C, 0x800CADFC, 0x800CAE88, 0x800CAF1C,
           0x800CAFB0, 0x800CAFC8, 0x800CB07C, 0x800CB3CC)


def recipe(data):
    if len(data) != 6884 or hashlib.sha256(data).hexdigest() != MOVIE_SHA256:
        raise ValueError('MOVIE.OVR does not match the inspected Time to Kill revision')
    entries = [f'0x{address:08X}' for address in ENTRIES]
    return {'schema': 'psxrecomp overlay capture v2', 'load_addr': hex(LOAD),
            'size': len(data), 'bytes_b64': base64.b64encode(data).decode(),
            'executed_pcs': [], 'seeds': entries, 'function_entry_pcs': entries,
            'dispatch_entry_pcs': entries, 'static_dispatch_entry_pcs': entries,
            'producer_ranges': [{'start': hex(CODE_START), 'end': hex(CODE_END)}],
            'strict_producer_ranges': True}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--gcc', default='gcc')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--force', action='store_true')
    parser.add_argument('--if-ready', action='store_true',
                        help='skip (exit 0) when the disc, recompiler or compiler is not present yet')
    parser.add_argument('--quiet', action='store_true',
                        help='print only a one-line result; full compiler output on failure')
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('jobs must be positive')
    executable = 'psxrecomp-game' + ('.exe' if os.name == 'nt' else '')
    recompiler = ROOT / 'build-recompiler' / executable
    missing = [str(path.relative_to(ROOT)) for path in
               (ROOT / 'disc/SLUS_005.83', ROOT / 'disc/time-to-kill.bin', recompiler)
               if not path.is_file()]
    if not shutil.which(args.gcc):
        missing.append(args.gcc)
    if missing:
        if args.if_ready:
            print(f'movie shard: skipped, not ready ({", ".join(missing)} missing)')
            return 0
        parser.error(f'movie overlay needs: {", ".join(missing)}')
    if hashlib.sha256((ROOT / 'disc/SLUS_005.83').read_bytes()).hexdigest() != EXE_SHA256:
        parser.error('boot executable differs from the inspected revision')
    disc = Disc(ROOT / 'disc/time-to-kill.bin')
    try:
        # Guarding the complete file hash makes this fixed title-specific extent
        # fail closed if the prepared image belongs to another disc revision.
        record = recipe(disc.read(23959, 6884))
    finally:
        disc.close()
    captures = ROOT / 'analysis/movie-overlay/capture.json'
    captures.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps([record], indent=2) + '\n'
    if not captures.is_file() or captures.read_text() != text:
        captures.write_text(text)
    command = [sys.executable, str(ROOT / 'psxrecomp/tools/compile_overlays.py'),
               '--captures', str(captures), '--game-toml', str(ROOT / 'game.local.toml'),
               '--recompiler', str(recompiler),
               '--runtime-include', str(ROOT / 'psxrecomp/runtime/include'),
               '--out-dir', str(ROOT / 'build-local/cache'), '--compiler', 'gcc',
               '--gcc', args.gcc, '--flavor', '0', '--cps', '--jobs', str(args.jobs)]
    if args.force:
        command.append('--force')
    # An inherited live-capture override must not replace this disc recipe.
    env = os.environ.copy()
    env.pop('PSX_OVERLAY_CAPTURES', None)
    if not args.quiet:
        subprocess.run(command, cwd=ROOT, env=env, check=True)
        return 0
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True)
    output = result.stdout + result.stderr
    summary = [line for line in output.splitlines() if line.startswith('PSX_SHARD_RESULT')]
    fields = dict(item.split('=', 1) for item in summary[-1].split()[1:]) if summary else {}
    if result.returncode != 0 or not summary or fields.get('failed', '0') != '0':
        sys.stderr.write(output)
        print('movie shard: FAILED (intro FMV will run interpreted and may stutter)', file=sys.stderr)
        return result.returncode or 1
    built = fields.get('ok', '0')
    print('movie shard: rebuilt for the current config' if built != '0' else 'movie shard: current')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
