#!/usr/bin/env python3
"""Compile the actual SIO encoder/decoder at one and two seats."""
import json, shlex, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
cache=(ROOT/'build-local/CMakeCache.txt').read_text().splitlines()
ninja=next(x.split('=',1)[1] for x in cache if x.startswith('CMAKE_MAKE_PROGRAM:'))
commands=json.loads(subprocess.check_output([ninja,'-t','compdb'],cwd=ROOT/'build-local'))
e=next(c for c in commands if c['file'].endswith('/runtime/src/sio.c'))
a=shlex.split(e['command']);flags=[];i=1
while i<len(a):
    if a[i] in ('-o','-MT','-MF'):i+=2;continue
    if a[i] in ('-MD','-MMD','-c') or a[i]==e['file'] or a[i].startswith('-DPSX_MAX_PLAYERS='):i+=1;continue
    flags.append(a[i]);i+=1
with tempfile.TemporaryDirectory(prefix='ttk-d34b-') as tmp:
    tmp=Path(tmp)
    def run(args):subprocess.run([str(x) for x in args],cwd=e['directory'],check=True)
    for seats in (1,2):
        exe=tmp/str(seats)
        run([a[0],*flags,f'-DPSX_MAX_PLAYERS={seats}','-UNDEBUG',ROOT/'tests/local/sio_compat_native.c','-o',exe])
        run([exe,'write' if seats==1 else 'read',tmp/'one-seat.bin'])
    print('PASS: one-seat and pre-rumble compatibility, two-seat state/rumble round trips')
