#!/usr/bin/env python3
"""D23AH: link-time (LTO) and profile-guided (PGO) optimized builds.

The normal player build (build.py, build-local) stays as it is. These builds
use the same sources, generated code and settings in their own trees, so the
normal objects are never rebuilt:

  lto         build-lto: the player build with link-time optimization.
  instrument  build-pgo: LTO plus profile counters (-fprofile-generate).
  train       run the instrumented game through pgo_train.py's route; the
              counts land beside build-pgo's objects (.gcda files).
  use         rebuild build-pgo from the counts (-fprofile-use). Functions
              changed since the training keep the normal optimization
              (-fprofile-partial-training), so a profile stays usable across
              small source edits; retrain after large ones.
  pgo         instrument, train and use in one go.
  install     copy build-lto's or build-pgo's executable into build-local and
              record it in build-local/optimized-installed.json (run.py prints
              which build it launches); `install --restore` relinks the
              normal one from build-local's objects (seconds, always current).

D62 build policy: dev sessions on the desktop play the normal build; the
optimized build is made when the user asks (end of day, copies for the field
laptop and citadel, releases, performance measurements).

Run build.py first (disc import, generated code). No -march, no fast-math:
the same instructions and floating point rules as the normal build, only
inlining and code layout change. See documentation/554-d23ah-lto-pgo-build.md.
"""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
EXE_NAME = 'Duke_Nukem__Time_to_Kill_Recompiled' + ('.exe' if os.name == 'nt' else '')
TARGETS = ['psx-runtime', 'ttk-movie-shard', 'ttk-math-shard']
# The local-dev preset's cache variables (CMakePresets.json).
PRESET = ['-DCMAKE_BUILD_TYPE=RelWithDebInfo', '-DPSX_DEBUG_TOOLS=ON', '-DPSX_RECOMP_UI=OFF',
          '-DPSX_ENABLE_VULKAN=OFF', '-DPSX_NETPLAY=OFF']
PGO_GEN = '-fprofile-generate -fprofile-update=atomic'
PGO_USE = ('-fprofile-use -fprofile-partial-training -Wno-missing-profile '
           '-Wno-coverage-mismatch -Wno-error=coverage-mismatch')


ENV = os.environ.copy()
_venv = ROOT.parent / '.venv' / ('Scripts' if os.name == 'nt' else 'bin')
if _venv.is_dir():  # cmake/ninja from the workspace venv, as build.py does
    ENV['PATH'] = str(_venv) + os.pathsep + ENV.get('PATH', '')


def run(args, **kw):
    print('+', ' '.join(map(str, args)), flush=True)
    subprocess.run(list(map(str, args)), cwd=ROOT, check=True, env=kw.pop('env', ENV), **kw)


def fetched_sources():
    """Reuse build-local's fetched SDL3 and libchdr: no download, same version."""
    out = []
    for name, var in [('sdl3-src', 'SDL3'), ('psx_libchdr-src', 'PSX_LIBCHDR')]:
        src = ROOT / 'build-local/_deps' / name
        if src.is_dir():
            out.append(f'-DFETCHCONTENT_SOURCE_DIR_{var}={src}')
    return out


def python():
    """The asset steps need numpy: the interpreter build-local was configured
    with, else the one running this tool (not the venv's, which has none)."""
    cache = ROOT / 'build-local/CMakeCache.txt'
    if cache.is_file():
        for line in cache.read_text(errors='replace').splitlines():
            if line.startswith('_Python3_EXECUTABLE:INTERNAL='):
                return line.split('=', 1)[1]
    return sys.executable


NO_LTO = ['']


def configure(tree, flags, jobs):
    run(['cmake', '-S', '.', '-B', tree, '-G', 'Ninja', *PRESET, *fetched_sources(),
         f'-DPSX_NO_LTO_GLOB={NO_LTO[0]}',
         f'-DPython3_EXECUTABLE={python()}',
         '-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON',
         f'-DCMAKE_C_FLAGS={flags}', f'-DCMAKE_CXX_FLAGS={flags}',
         f'-DCMAKE_EXE_LINKER_FLAGS={flags}'])


def build(tree, jobs):
    t = time.time()
    run(['cmake', '--build', tree, '--parallel', jobs, '--target', *TARGETS])
    return time.time() - t


def stamp(tree, kind, seconds, extra=None):
    head = subprocess.run(['git', '-C', str(ROOT.parent), 'log', '-1', '--format=%h'],
                          capture_output=True, text=True).stdout.strip()
    info = dict(kind=kind, built=time.strftime('%Y-%m-%d %H:%M:%S'), head=head,
                build_seconds=round(seconds), **(extra or {}))
    (ROOT / tree / 'optimized-build.json').write_text(json.dumps(info, indent=1) + '\n')
    print(json.dumps(info), flush=True)


INSTALLED = ROOT / 'build-local' / 'optimized-installed.json'


def describe_build(exe):
    """One line for run.py: which build this launch runs. The install stamp
    counts only while the executable is the one installed (a normal relink
    replaces it and changes its size or time)."""
    exe = Path(exe)
    if exe.parent.name != 'build-local':
        return f'build: {exe.parent.name} (DNTTK_EXE)'
    try:
        info = json.loads(INSTALLED.read_text())
        st = exe.stat()
        if info['size'] == st.st_size and info['mtime_ns'] == st.st_mtime_ns:
            kind = {'pgo': 'optimized LTO+PGO', 'lto': 'optimized LTO'}.get(info['kind'], info['kind'])
            return f"build: {kind} @{info['head']}, built {info['built']}"
    except (OSError, ValueError, KeyError):
        pass
    return 'build: normal (dev build, -O3)'


def restore_normal(dst):
    """Put the normal executable back: ninja sees the installed copy as newer
    than its inputs, so remove it and relink build-local (seconds)."""
    INSTALLED.unlink(missing_ok=True)
    dst.unlink(missing_ok=True)
    run(['cmake', '--build', 'build-local', '--target', 'psx-runtime'])
    print('normal executable relinked', flush=True)


def gcda_count(tree):
    return sum(1 for _ in (ROOT / tree).rglob('*.gcda'))


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('step', choices=['lto', 'instrument', 'train', 'use', 'pgo', 'install'])
    p.add_argument('--jobs', type=int, default=max(1, (os.cpu_count() or 4) // 2),
                   help='parallel compiles (LTO links use more memory: about 2-4 GB each)')
    p.add_argument('--from', dest='source', choices=['lto', 'pgo'], default='pgo', help='install: which build')
    p.add_argument('--restore', action='store_true', help='install: put the normal executable back')
    p.add_argument('--train-args', default='', help='train: extra arguments for pgo_train.py')
    p.add_argument('--cflags', default='', help='extra compile flags for every step (C and C++)')
    p.add_argument('--no-lto-glob', default='', help='developer: ;-separated source patterns built without LTO')
    a = p.parse_args()
    NO_LTO[0] = a.no_lto_glob
    if not (ROOT / 'generated').is_dir():
        p.error('no generated code; run tools/local/build.py first')
    if a.step == 'lto':
        configure('build-lto', a.cflags, a.jobs)
        stamp('build-lto', 'lto', build('build-lto', a.jobs))
    if a.step in ('instrument', 'pgo'):
        for old in (ROOT / 'build-pgo').rglob('*.gcda') if (ROOT / 'build-pgo').is_dir() else []:
            old.unlink()
        configure('build-pgo', f'{PGO_GEN} {a.cflags}'.strip(), a.jobs)
        stamp('build-pgo', 'pgo-instrumented', build('build-pgo', a.jobs))
    if a.step in ('train', 'pgo'):
        run([sys.executable, 'tools/local/pgo_train.py', '--exe', ROOT / 'build-pgo' / EXE_NAME,
             *a.train_args.split()])
        n = gcda_count('build-pgo')
        print(f'profile files: {n}', flush=True)
        if n == 0:
            raise SystemExit('training wrote no profile counts')
    if a.step in ('use', 'pgo'):
        n = gcda_count('build-pgo')
        if n == 0:
            p.error('no profile counts in build-pgo; run the train step first')
        configure('build-pgo', f'{PGO_USE} {a.cflags}'.strip(), a.jobs)
        stamp('build-pgo', 'pgo', build('build-pgo', a.jobs), dict(profile_files=n))
    if a.step == 'install':
        dst = ROOT / 'build-local' / EXE_NAME
        if a.restore:
            restore_normal(dst)
            return 0
        src = ROOT / f'build-{a.source}' / EXE_NAME
        info = ROOT / f'build-{a.source}' / 'optimized-build.json'
        if not src.is_file() or not info.is_file() or json.loads(info.read_text())['kind'] == 'pgo-instrumented':
            p.error(f'no finished {a.source} build')
        shutil.copy2(src, dst)
        os.utime(dst)
        st = dst.stat()
        stamp_info = dict(json.loads(info.read_text()), size=st.st_size, mtime_ns=st.st_mtime_ns)
        INSTALLED.write_text(json.dumps(stamp_info, indent=1) + '\n')
        print(f'installed {src.relative_to(ROOT)} -> build-local ({info.read_text().strip()})', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
