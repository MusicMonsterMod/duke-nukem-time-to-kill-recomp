#!/usr/bin/env python3
"""D37A native regressions using the configured runtime compiler/includes."""
import json,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
cache=(ROOT/'build-local/CMakeCache.txt').read_text().splitlines()
ninja=next(x.split('=',1)[1] for x in cache if x.startswith('CMAKE_MAKE_PROGRAM:'))
commands=json.loads(subprocess.check_output([ninja,'-t','compdb'],cwd=ROOT/'build-local'))
entry=next(c for c in commands if c['file'].endswith('/runtime/src/memory.c'))
args=shlex.split(entry['command']);compiler=args[0]
# Reuse real runtime feature definitions and include paths, but isolate outputs.
flags=[];i=1
while i<len(args):
    if args[i] in ('-o','-MT','-MF'):i+=2;continue
    if args[i] in ('-MD','-MMD'):i+=1;continue
    if args[i]=='-c':i+=1;continue
    if args[i]==entry['file']:i+=1;continue
    flags.append(args[i]);i+=1
with tempfile.TemporaryDirectory(prefix='ttk-d37a-') as tmp:
    tmp=Path(tmp)
    def run(argv):subprocess.run([str(x) for x in argv],cwd=entry['directory'],check=True)
    run([compiler,*flags,'-ffunction-sections','-fdata-sections','-c',entry['file'],'-o',tmp/'memory.o'])
    run([compiler,'-UNDEBUG','-I'+str(ROOT/'psxrecomp/runtime/include'),ROOT/'tests/local/modmem_snapshot_native.c',tmp/'memory.o','-Wl,--gc-sections','-o',tmp/'memory-test'])
    run([tmp/'memory-test'])
    run([tmp/'memory-test','biomask'])
    run(['c++','-std=c++17','-UNDEBUG','-I'+str(ROOT/'src/ttk'),ROOT/'tests/local/anim_save_header_native.cpp','-o',tmp/'header-test'])
    run([tmp/'header-test'])
