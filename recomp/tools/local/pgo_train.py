#!/usr/bin/env python3
"""D23AH: the PGO training route for an instrumented build (optimized_build.py).

Runs the instrumented executable offscreen through run.py (DNTTK_EXE) with a
private settings file and a temporary copy of a card directory; the player's
own settings, saves and cards are only read. From a savestate it enters every
level with the console's `level N` and plays the same route in each (stand, a
turn, a walk with turns, look up and down, a weapon burst), in first and third
person, then a short Vanilla pass. The redraw governor's fallback is off
(DNTTK_REPLAY_FLOOR=0) so the redraw workers run, and are trained, although
the instrumented game is several times slower. Every launch ends with the
debug `quit`, so the game and its retiring redraw workers write their counts.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, str(Path(__file__).resolve().parent))
from debug_client import request  # noqa: E402
import player_profiles as profiles  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
LEVELS = '0 1 2 3 5 6 7 8 9 10 11 12 21 22 23 24 25 26 27 28 29'
ROUTE = [('', 3), ('dx=4', 6), ('w', 3), ('w dx=4', 3), ('w dx=-4', 3), ('w shift', 2),
         ('dy=-4', 1.5), ('dy=4', 2), ('dy=-4', 1), ('mouse1', 3), ('', 1)]


def staged(exe):
    """The runtime takes its overlay shard cache (native overlays, the geometry
    math shard) from the executable's own directory. Another build's executable
    runs from build-local under its own name, so every build shares the player
    build's cache and runs the same native code paths."""
    exe = Path(exe).resolve()
    home = ROOT / 'build-local'
    if exe.parent == home:
        return exe
    dst = home / f'{exe.name}.{exe.parent.name}'
    if not dst.is_file() or dst.stat().st_size != exe.stat().st_size or dst.stat().st_mtime < exe.stat().st_mtime:
        shutil.copy2(exe, dst)
    return dst


class Game:
    def __init__(self, exe, port, settings, cards, vanilla, log, extra_env=None):
        self.port = port
        self.input = Path(settings).with_name('input')
        self.input.write_text('')
        env = {k: v for k, v in os.environ.items() if not k.startswith(('DNTTK_', 'PSX_'))}
        env.update(DNTTK_EXE=str(staged(exe)), SDL_VIDEODRIVER='offscreen', SDL_AUDIODRIVER='dummy',
                   DNTTK_TEST_INPUT=str(self.input), DNTTK_TEST_UNCAPTURED='1', **(extra_env or {}))
        env.pop('DISPLAY', None)
        env.pop('WAYLAND_DISPLAY', None)
        args = [sys.executable, str(ROOT / 'tools/local/run.py'), '--settings-file', str(settings),
                '--memcard-dir', str(cards), '--debug-port', str(self.port), '--no-session-log']
        if vanilla:
            args.append('--vanilla')
        self.p = subprocess.Popen(args, cwd=ROOT, env=env, stdout=log, stderr=subprocess.STDOUT)
        for _ in range(1200):
            try:
                if self.call('frame').get('ok'):
                    return
            except Exception:
                pass
            if self.p.poll() is not None:
                raise SystemExit('the game exited during start-up')
            time.sleep(0.1)
        raise SystemExit('no debug port')

    def call(self, cmd, **kw):
        r = request(dict(cmd=cmd, **kw), self.port)
        if not r.get('ok'):
            raise RuntimeError(str(r)[:300])
        return r

    def u(self, addr, n=4):
        b = bytes.fromhex(self.call('read_ram', addr=hex(addr), len=n)['hex'])
        return int.from_bytes(b, 'little')

    def state(self):
        return dict(mode=self.u(0x800bcbb0, 2), phase=self.u(0x800ce1b0, 2), level=self.u(0x800be570),
                    end=self.u(0x800be55e, 1))

    def keys(self, k):
        self.input.write_text('play cheat=none act=none ' + k)

    def wait_frames(self, n, limit=120):
        t = self.call('frame')['frame'] + n
        end = time.monotonic() + limit
        while self.call('frame')['frame'] < t and time.monotonic() < end:
            time.sleep(0.05)

    def enter(self, lv, limit=300):
        """`level N`, then wait until that level is in play (slow in this build)."""
        self.call('console', line=f'level {lv}')
        t = time.monotonic()
        left = False
        while time.monotonic() - t < limit:
            time.sleep(0.5)
            try:
                s = self.state()
            except Exception:
                continue
            if s['mode'] != 1 or s['phase'] != 2:
                left = True
            if left and s['mode'] == 1 and s['phase'] == 2 and s['level'] == lv and s['end'] == 1:
                return round(time.monotonic() - t, 1)
        return None

    def route(self, scale):
        for k, secs in ROUTE:
            self.keys(k)
            time.sleep(secs * scale)
        self.keys('')

    def quit(self):
        self.keys('')
        try:
            self.call('quit')
        except Exception:
            pass
        try:
            self.p.wait(180)  # the instrumented game writes its counts on the way out
        except subprocess.TimeoutExpired:
            print('[train] the game did not quit; its counts are lost', flush=True)
            self.p.terminate()
            self.p.wait(30)


def digest(paths):
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--exe', type=Path, required=True, help='the instrumented executable')
    p.add_argument('--memcard-dir', type=Path, default=ROOT / 'saves/local-play',
                   help='cards and savestates to copy (read only); default the local play saves')
    p.add_argument('--start-slot', type=int, default=None, help='savestate slot to start from (default the first)')
    p.add_argument('--levels', default=LEVELS)
    p.add_argument('--views', default='first,third')
    p.add_argument('--vanilla-levels', default='0 3 22', help="'' for no Vanilla pass")
    p.add_argument('--frame-rate', default='60')
    p.add_argument('--scale', type=float, default=2.0, help='route seconds multiplier (the build is slow)')
    p.add_argument('--port', type=int, default=9671)
    p.add_argument('--log', type=Path, default=None)
    a = p.parse_args()
    a.exe = a.exe.resolve()
    if not a.exe.is_file():
        p.error(f'no executable {a.exe}')
    if subprocess.run(['pgrep', '-x', 'Duke_Nukem__Tim'], capture_output=True).stdout:
        p.error('a game is running; close it first (debug quit on port 9123)')
    player = [x for x in a.memcard_dir.rglob('*') if x.is_file()] + [ROOT / 'config/player-profiles.json']
    before = digest(player)
    work = Path(tempfile.mkdtemp(prefix='ttk-pgo-'))
    log = (a.log or work / 'train.log').open('a')
    try:
        cards = work / 'cards'
        shutil.copytree(a.memcard_dir, cards)
        slots = sorted(cards.glob('openbios/*_slot[0-9][0-9].pst'))
        if not slots:
            p.error('no savestate in the card directory')
        slot = a.start_slot if a.start_slot is not None else int(slots[0].stem[-2:])
        src = ROOT / 'config/player-profiles.json'
        base = json.loads(src.read_text()) if src.is_file() else profiles.defaults()
        passes = [(False, v) for v in a.views.split(',') if v] + ([(True, None)] if a.vanilla_levels.strip() else [])
        for vanilla, view in passes:
            s = json.loads(json.dumps(base))
            if not vanilla:
                c = s['profiles']['modernized']['controls']
                c['view'] = view
                c['frame_rate'] = a.frame_rate
            settings = work / f'profiles-{view or "vanilla"}.json'
            settings.write_text(json.dumps(s))
            levels = [int(x) for x in (a.vanilla_levels if vanilla else a.levels).split()]
            t0 = time.monotonic()
            g = Game(a.exe, a.port, settings, cards, vanilla, log, dict(DNTTK_REPLAY_FLOOR='0'))
            try:
                g.call('savestate', op='load', slot=slot)
                time.sleep(2)
                g.wait_frames(30)
                for lv in levels:
                    took = g.enter(lv)
                    print(f'[train] {"vanilla" if vanilla else view} level {lv}:',
                          'arrived in %.0f s' % took if took is not None else 'did not arrive', flush=True)
                    if took is not None:
                        g.route(a.scale)
            finally:
                g.quit()
            print(f'[train] pass {"vanilla" if vanilla else view} done in {time.monotonic() - t0:.0f} s', flush=True)
    finally:
        log.close()
        shutil.rmtree(work, ignore_errors=True)
    if digest(player) != before:
        print('[train] PLAYER FILES CHANGED', flush=True)
        return 3
    print('[train] player files unchanged', flush=True)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
