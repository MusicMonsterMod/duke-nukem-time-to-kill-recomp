#!/usr/bin/env python3
"""D61 level atlas: see every part of every level, and go there.

Runs the player build offscreen with a private copy of the player's settings
and empty memory cards (no savestate, no card of the player's is used; their
files are hashed before and after every run). The game boots, Enter starts a
new game, and the console's `level N` visits each level. What a level has is
read from the game's own memory once the level is in play.

  capture [LEVELS]     boot, visit each level, save a RAM dump and a start
                       screenshot per level (research/atlas/raw/)
  build [LEVELS]       read the dumps: rooms, portals, floors, objects, enemy
                       spawners, lamps and standing points per level, plus the
                       cut-content flags (research/atlas/levelNN.json, a
                       top-down plan levelNN.svg, index.json, findings.md)
  survey [LEVELS]      capture, then build
  shot TARGET [VIEWS]  boot, `goto TARGET`, save VIEWS screenshots turning
                       around (research/atlas/shots/)
  tour LEVEL           visit every room's standing point of LEVEL, four views
                       each, and lay them out as a contact sheet beside the
                       plan (research/atlas/tour/levelNN/)
  models LEVEL [TYPES]  spawn each type (default: the level's unused-model
                       flags) in front of Duke and photograph it
                       (research/atlas/models/levelNN/)
  secrets [LEVELS]     photograph every secret room (room flag 0x4) of LEVELS
                       in one boot (research/atlas/secrets/)
  levels               the level list

LEVELS: numbers separated by commas or spaces, default every selectable level.
TARGET: what the in-game console's goto takes, for example "3 room 41",
"3 object 17" or "lava-upper".

Everything this writes is derived from the retail game and stays local
(research/ is not tracked). See documentation/559-d61-level-atlas.md.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, str(Path(__file__).resolve().parent))
from debug_client import request  # noqa: E402
import level_atlas as la  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]       # recomp/
WORK = ROOT.parent                              # the workspace
OUT = WORK / 'research' / 'atlas'
LEVELS = la.SELECTABLE
EXE_NAME = 'Duke_Nukem__Time_to_Kill_Recompiled' + ('.exe' if os.name == 'nt' else '')


def player_files():
    saves = ROOT / 'saves' / 'local-play'
    files = [p for p in saves.rglob('*') if p.is_file() and p.name != 'writable-state.log'] if saves.is_dir() else []
    return files + [ROOT / 'config' / 'player-profiles.json']


def digest(paths):
    return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}


def parse_levels(words):
    if not words:
        return list(LEVELS)
    out = []
    for w in ' '.join(words).replace(',', ' ').split():
        if '-' in w:
            a, b = w.split('-', 1)
            out += list(range(int(a), int(b) + 1))
        else:
            out.append(int(w))
    return out


class Game:
    """One offscreen launch. Private settings copy, empty cards, own port."""

    def __init__(self, port=9721, exe=None, extra_env=None, keep=None, vanilla=False, cards=None):
        if subprocess.run(['pgrep', '-x', 'Duke_Nukem__Tim'], capture_output=True).stdout:
            raise SystemExit('a game is running; close it first (debug quit on port 9123)')
        self.port = port
        self.keep = bool(keep)
        self.dir = Path(keep) if keep else Path(tempfile.mkdtemp(prefix='ttk-atlas-'))
        self.dir.mkdir(parents=True, exist_ok=True)
        if cards:   # a private copy (read only); the player's files are hashed by Guard
            shutil.rmtree(self.dir / 'cards', ignore_errors=True)
            shutil.copytree(cards, self.dir / 'cards')
        (self.dir / 'cards').mkdir(exist_ok=True)
        settings = json.loads((ROOT / 'config' / 'player-profiles.json').read_text())
        (self.dir / 'profiles.json').write_text(json.dumps(settings))
        self.input = self.dir / 'input'
        self.input.write_text('')
        env = {k: v for k, v in os.environ.items() if not k.startswith(('DNTTK_', 'PSX_'))}
        env.update(SDL_VIDEODRIVER='offscreen', SDL_AUDIODRIVER='dummy',
                   DNTTK_TEST_INPUT=str(self.input), DNTTK_TEST_UNCAPTURED='1', **(extra_env or {}))
        if exe:
            env['DNTTK_EXE'] = str(Path(exe).resolve())
        env.pop('DISPLAY', None)
        env.pop('WAYLAND_DISPLAY', None)
        self.log_path = self.dir / 'run.log'
        self.log = self.log_path.open('w')
        args = [sys.executable, str(ROOT / 'tools/local/run.py'), '--settings-file', str(self.dir / 'profiles.json'),
                '--memcard-dir', str(self.dir / 'cards'), '--debug-port', str(port), '--no-session-log']
        if vanilla:
            args.append('--vanilla')
        self.p = subprocess.Popen(args, cwd=ROOT, env=env, stdout=self.log, stderr=subprocess.STDOUT)
        for _ in range(1200):
            try:
                if self.call('frame').get('ok'):
                    return
            except Exception:
                pass
            if self.p.poll() is not None:
                raise SystemExit(f'the game exited during start-up (log {self.log_path})')
            time.sleep(0.1)
        raise SystemExit('no debug port')

    def call(self, cmd, **kw):
        r = request(dict(cmd=cmd, **kw), self.port)
        if not r.get('ok'):
            raise RuntimeError(str(r)[:300])
        return r

    def ram(self, addr, n):
        return bytes.fromhex(self.call('read_ram', addr=hex(addr), len=n)['hex'])

    def u(self, addr, n=4):
        return int.from_bytes(self.ram(addr, n), 'little')

    def state(self):
        return dict(mode=self.u(0x800bcbb0, 2), phase=self.u(0x800ce1b0, 2), level=self.u(0x800be570),
                    end=self.u(0x800be55e, 1))

    def keys(self, words=''):
        self.input.write_text(words)

    def console(self, line):
        return self.call('console', line=line).get('reply', '')

    def cheat(self, word):
        """A debug cheat through the test-input word (acts once per change)."""
        self.keys('play cheat=none')
        time.sleep(0.3)
        self.keys(f'play cheat={word}')
        time.sleep(0.6)
        self.keys('play')

    def god(self, on=True):
        if bool(self.u(0x800c3cc6, 2)) != on:
            self.cheat('dnkroz')
        return bool(self.u(0x800c3cc6, 2)) == on

    def alive(self, limit=40):
        """Wait out a death: until a level is in play with Duke alive."""
        t = time.monotonic()
        while time.monotonic() - t < limit:
            try:
                if self.in_play() and not self.u(0x800be55c, 2) and self.u(la.PLAYER + 0x32, 2) > 0:
                    return True
            except Exception:
                pass
            time.sleep(0.5)
        return False

    def in_play(self, s=None):
        s = s or self.state()
        return s['mode'] == 1 and s['phase'] == 2 and s['end'] == 1

    def boot(self, limit=120):
        """Enter skips the intro and confirms ONE PLAYER (Modernized menu keys)."""
        t = time.monotonic()
        i = 0
        while time.monotonic() - t < limit:
            try:
                s = self.state()
                if s['mode'] == 1:          # a game is starting: no more Enter (it pauses)
                    self.keys('')
                    if self.in_play(s):
                        time.sleep(1.5)
                        self.unpause()
                        self.keys('play')
                        return round(time.monotonic() - t, 1)
                    time.sleep(0.3)
                    continue
            except Exception:
                pass
            self.keys(('enter', '', 'x', '')[i % 4])   # Enter / X: the Modernized menu keys
            if i % 4 == 2:                              # and the pad's own Start / Cross (Vanilla)
                try:
                    self.call('press', buttons=0xFFFF ^ (0x0008, 0x4000)[(i // 4) % 2], frames=6)   # active low
                except Exception:
                    pass
            i += 1
            time.sleep(0.6)
        raise SystemExit('could not start a game from the title')

    def unpause(self, tries=6):
        """Close the pause menu (0x800be56a) if a key press opened it."""
        for _ in range(tries):
            if not self.u(0x800be56a, 2):
                return True
            self.keys('esc')
            time.sleep(0.4)
            self.keys('')
            time.sleep(0.6)
        return not self.u(0x800be56a, 2)

    def go_level(self, n, limit=120, force=False):
        reply = self.console(f'level force {n}' if force else f'level {n}')
        t = time.monotonic()
        left = False
        while time.monotonic() - t < limit:
            time.sleep(0.4)
            try:
                s = self.state()
            except Exception:
                continue
            if not self.in_play(s):
                left = True
            if left and self.in_play(s) and s['level'] == n:
                return round(time.monotonic() - t, 1)
        raise RuntimeError(f'level {n} did not load ({reply.strip()})')

    def settle(self, frames=120, limit=30):
        f = self.call('frame')['frame'] + frames
        end = time.monotonic() + limit
        while self.call('frame')['frame'] < f and time.monotonic() < end:
            time.sleep(0.05)

    def shot(self, path):
        seq = self.call('present_shot_seq')['seq']
        self.call('present_shot', path=str(path))
        for _ in range(200):
            if self.call('present_shot_seq')['seq'] != seq:
                return True
            time.sleep(0.05)
        return False

    def close(self):
        self.keys('')
        try:
            self.call('quit')
        except Exception:
            pass
        try:
            self.p.wait(30)
        except subprocess.TimeoutExpired:
            self.p.terminate()
            self.p.wait(10)
        self.log.close()
        if not self.keep:
            if os.environ.get('ATLAS_KEEP_LOG'):
                shutil.copy2(self.log_path, OUT / 'last-run.log')
            shutil.rmtree(self.dir, ignore_errors=True)


class Guard:
    """The player's saves, cards and settings are only read: checked after."""

    def __enter__(self):
        self.before = digest(player_files())
        return self

    def __exit__(self, *exc):
        after = digest(player_files())
        if after != self.before:
            changed = sorted(k for k in set(after) | set(self.before) if after.get(k) != self.before.get(k))
            raise SystemExit(f'player files changed during the run: {changed}')


def capture(levels, port, force=False):
    raw = OUT / 'raw'
    raw.mkdir(parents=True, exist_ok=True)
    with Guard():
        g = Game(port)
        try:
            print(f'[atlas] in play after {g.boot()} s', flush=True)
            for n in levels:
                try:
                    secs = g.go_level(n, limit=60 if force else 120, force=force)
                except RuntimeError as e:
                    print(f'[atlas] level {n}: {e}', flush=True)
                    g.shot(raw / f'level{n:02d}.failed.png')
                    if g.p.poll() is not None:
                        print(f'[atlas] the game exited at level {n}; stopping', flush=True)
                        break
                    if not g.alive(20):
                        g.unpause()
                        try:
                            g.go_level(0)
                        except RuntimeError:
                            print('[atlas] could not get back to a level; stopping', flush=True)
                            break
                    continue
                g.settle()
                ram = g.ram(0x80000000, 0x200000)
                (raw / f'level{n:02d}.ram').write_bytes(ram)
                g.shot(raw / f'level{n:02d}.start.png')
                meta = dict(level=n, load_seconds=secs, frame=g.call('frame')['frame'],
                            names=g.console('levels').splitlines())
                (raw / f'level{n:02d}.meta.json').write_text(json.dumps(meta, indent=1))
                print(f'[atlas] level {n:2d}: loaded in {secs} s, dumped', flush=True)
        finally:
            g.close()


def build(levels):
    OUT.mkdir(parents=True, exist_ok=True)
    index = []
    atlases = {}
    for n in levels:
        f = OUT / 'raw' / f'level{n:02d}.ram'
        if not f.is_file():
            print(f'[atlas] level {n}: no dump (run capture first)')
            continue
        try:
            a = la.read_level(f.read_bytes(), lamps=la.read_lamps(ROOT / 'assets' / 'lights' / f'level{n:02d}.lights'))
        except ValueError as e:
            print(f'[atlas] level {n}: {e}')
            continue
        atlases[n] = a
        (OUT / f'level{n:02d}.json').write_text(json.dumps(a, indent=1))
        (OUT / f'level{n:02d}.svg').write_text(la.plan_svg(a))
        index.append(la.summary(a))
        print(f"[atlas] level {n:2d} {a['name']}: {len(a['rooms'])} rooms, {len(a['objects'])} objects, "
              f"{len(a['spawners'])} spawners, {sum(len(r['stand']) for r in a['rooms'])} standing points, "
              f"{len(a['flags'])} flags")
    la.cross_flags(atlases, dumps={n: (OUT / 'raw' / f'level{n:02d}.ram').read_bytes() for n in atlases})
    for n, a in atlases.items():
        (OUT / f'level{n:02d}.json').write_text(json.dumps(a, indent=1))
    index = [la.summary(a) for a in atlases.values()]
    (OUT / 'index.json').write_text(json.dumps(index, indent=1))
    (OUT / 'findings.md').write_text(la.findings_md(atlases))
    print(f'[atlas] wrote {OUT.relative_to(WORK)}/')


def shot(target, views, port):
    d = OUT / 'shots'
    d.mkdir(parents=True, exist_ok=True)
    tag = '-'.join(target.split())
    with Guard():
        g = Game(port)
        try:
            g.boot()
            print(g.console(f'goto {target}').strip(), flush=True)
            wait_goto(g)
            for k in range(views):
                g.shot(d / f'{tag}-{k}.png')
                turn(g, 360 / views)
            print(f'[atlas] {views} views in {d.relative_to(WORK)}/{tag}-*.png')
        finally:
            g.close()


def goto_results(g):
    """The goto result lines so far ('arrived ...' / 'refused ...')."""
    out = []
    for line in g.log_path.read_text(errors='replace').splitlines():
        if '[TTK goto]' in line:
            s = line.split('[TTK goto]', 1)[1].strip()
            if s.startswith(('arrived', 'refused')):
                out.append(s)
    return out


def wait_goto(g, limit=150):
    """Until the next goto result line after the ones already seen."""
    seen = getattr(g, '_goto_seen', 0)
    t = time.monotonic()
    while time.monotonic() - t < limit:
        res = goto_results(g)
        if len(res) > seen:
            g._goto_seen = len(res)
            g.settle(30)
            return res[-1]
        time.sleep(0.3)
    raise RuntimeError('goto did not arrive')


def turn(g, degrees):
    g.console(f'goto face {degrees:+.0f}')
    g.settle(20)


def duke(g):
    p = g.ram(la.PLAYER, 0x900)
    i = lambda o: int.from_bytes(p[o:o + 4], 'little', signed=True)
    h = lambda o: int.from_bytes(p[o:o + 2], 'little', signed=True)
    return dict(pos=[i(4), i(8), i(12)], feet=i(0x1c8), room=p[0x2e], health=h(0x32), ammo=h(0x2d6))


def tour(level, views, port, limit_rooms=None, points=False, mortal=False):
    """Every room of LEVEL (or every atlas standing point with points): goto
    with quiet, VIEWS shots turning around, then Duke's health, height and
    room after a second and a half. God mode (dnkroz) keeps the tour alive;
    mortal leaves it off so health drops are recorded (enemies included:
    quiet's invisibility does not stop one already at Duke)."""
    a_path = OUT / f'level{level:02d}.json'
    if not a_path.is_file():
        raise SystemExit(f'no atlas for level {level}: run survey {level} first')
    a = json.loads(a_path.read_text())
    d = OUT / 'tour' / f'level{level:02d}'
    d.mkdir(parents=True, exist_ok=True)
    if points:
        stops = [(r['index'], k, f"at {p['pos'][0]} {p['pos'][1]} {p['pos'][2]}")
                 for r in a['rooms'] for k, p in enumerate(r['stand'])]
    else:
        stops = [(r['index'], 0, f"room {r['index']}") for r in a['rooms']]
    if limit_rooms:
        stops = stops[:limit_rooms]
    results = []
    t0 = time.monotonic()
    with Guard():
        g = Game(port)
        try:
            g.boot()
            g.go_level(level, force=level not in la.SELECTABLE)
            g.settle()
            god = False if mortal else g.god(True)
            for room, k, target in stops:
                if not g.alive():
                    print('[atlas] the level did not come back after a death', flush=True)
                    break
                if g.state()['level'] != level:   # left the level: back in
                    g.unpause()
                    g.go_level(level, force=level not in la.SELECTABLE)
                    g.settle()
                    god = False if mortal else g.god(True)
                g.console(f'goto {target} quiet')
                try:
                    status = wait_goto(g, 30)
                except RuntimeError:
                    status = 'timeout'
                pics, before, after = [], None, None
                if status.startswith('arrived'):
                    before = duke(g)
                    for v in range(views):
                        p = d / f'room{room:03d}-{k}-{v}.png'
                        g.shot(p)
                        pics.append(p.name)
                        turn(g, 360 / views)
                    g.settle(90)
                    try:
                        after = duke(g)
                    except Exception:
                        after = None
                hazard = []
                if before and after:
                    if after['health'] < before['health']:
                        hazard.append(f"health {before['health'] // 100} -> {after['health'] // 100}")
                    if after['pos'][1] - before['pos'][1] > 256:
                        hazard.append(f"fell {after['pos'][1] - before['pos'][1]}")
                    if after['room'] != before['room']:
                        hazard.append(f"left the room for {after['room']}")
                results.append(dict(room=room, point=k, target=target, status=status, shots=pics,
                                    before=before, after=after, hazard=hazard, god=god))
                print(f"[atlas] room {room:3d}.{k}: {status}{'  ! ' + '; '.join(hazard) if hazard else ''}", flush=True)
        finally:
            g.close()
    (d / 'tour.json').write_text(json.dumps(results, indent=1))
    la.contact_sheet(a, results, d, views)
    ok = sum(r['status'].startswith('arrived') for r in results)
    print(f'[atlas] {ok}/{len(results)} arrivals, {sum(bool(r["hazard"]) for r in results)} with a hazard, '
          f'{time.monotonic() - t0:.0f} s; contact sheet {(d / "sheet.html").relative_to(WORK)}')


def models(level, types, port):
    """Spawn each type in front of Duke (the console's spawn, the game's own
    creator) and photograph it: what a loaded-but-unused model looks like.
    Default types: the level's unused-model flags. Six per level entry, Duke
    turning 60 degrees between them (spawned objects stay)."""
    a_path = OUT / f'level{level:02d}.json'
    a = json.loads(a_path.read_text()) if a_path.is_file() else None
    if not types:
        if not a:
            raise SystemExit(f'no atlas for level {level}: run survey {level} first')
        types = sorted({t for f in a['flags'] if f['kind'].startswith('unused-model') for t in f['types']})
    d = OUT / 'models' / f'level{level:02d}'
    d.mkdir(parents=True, exist_ok=True)
    results = []
    with Guard():
        g = Game(port)
        try:
            g.boot()
            for k, t in enumerate(types):
                if k % 6 == 0:
                    g.unpause()
                    g.go_level(level, force=level not in la.SELECTABLE)
                    g.settle(60)
                    start = a['duke_room'] if a else 0
                    g.console(f'goto room {start} quiet')
                    try:
                        wait_goto(g, 30)
                    except RuntimeError:
                        pass
                reply = g.console(f'spawn {t}').strip()
                g.settle(45)
                p = d / f'type{t:04d}.png'
                g.shot(p)
                results.append(dict(type=t, reply=reply, shot=p.name))
                print(f'[atlas] type {t}: {reply}', flush=True)
                turn(g, 60)
        finally:
            g.close()
    (d / 'models.json').write_text(json.dumps(results, indent=1))
    cells = ''.join(f'<figure><img src="{r["shot"]}"><figcaption>type {r["type"]}: {r["reply"]}</figcaption></figure>'
                    for r in results)
    (d / 'sheet.html').write_text('<!doctype html><meta charset="utf-8"><title>models</title><style>body{background:#111;'
                                  'color:#ddd;font:13px monospace}div{display:grid;grid-template-columns:repeat(3,1fr);gap:6px}'
                                  f'img{{width:100%}}</style><h1>Level {level}: {len(results)} types</h1><div>{cells}</div>')
    print(f'[atlas] {len(results)} types; sheet {(d / "sheet.html").relative_to(WORK)}')


def secrets(levels, views, port):
    """Every secret room (room flag 0x4) of LEVELS, in one boot: goto it with
    quiet and god mode, VIEWS shots turning around (research/atlas/secrets/)."""
    d = OUT / 'secrets'
    d.mkdir(parents=True, exist_ok=True)
    results = []
    with Guard():
        g = Game(port)
        try:
            g.boot()
            for lv in levels:
                a_path = OUT / f'level{lv:02d}.json'
                if not a_path.is_file():
                    continue
                a = json.loads(a_path.read_text())
                rooms = [r['index'] for r in a['rooms'] if r.get('secret') or r['flags'] & la.SECRET_ROOM]
                if not rooms:
                    continue
                g.unpause()
                g.go_level(lv, force=lv not in la.SELECTABLE)
                g.settle()
                g.god(True)
                for room in rooms:
                    if not g.alive():
                        break
                    g.console(f'goto room {room} quiet')
                    try:
                        status = wait_goto(g, 30)
                    except RuntimeError:
                        status = 'timeout'
                    pics = []
                    if status.startswith('arrived'):
                        for v in range(views):
                            p = d / f'level{lv:02d}-room{room:03d}-{v}.png'
                            g.shot(p)
                            pics.append(p.name)
                            turn(g, 360 / views)
                    results.append(dict(level=lv, name=a['name'], room=room, status=status, shots=pics))
                    print(f"[atlas] level {lv} room {room}: {status}", flush=True)
        finally:
            g.close()
    (d / 'secrets.json').write_text(json.dumps(results, indent=1))
    print(f"[atlas] {sum(r['status'].startswith('arrived') for r in results)}/{len(results)} secret rooms in {d.relative_to(WORK)}/")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('command', choices=['capture', 'build', 'survey', 'shot', 'tour', 'models', 'secrets', 'levels'])
    p.add_argument('args', nargs='*')
    p.add_argument('--port', type=int, default=9721)
    p.add_argument('--views', type=int, default=4)
    p.add_argument('--rooms', type=int, default=None, help='tour: only the first N stops')
    p.add_argument('--points', action='store_true', help='tour: every atlas standing point, not one per room')
    p.add_argument('--mortal', action='store_true', help='tour: no god mode (record damage)')
    p.add_argument('--force', action='store_true',
                   help="capture/survey: travel with 'level force N' (the indices the level select skips: 4, 13-20, 30, 31)")
    a = p.parse_intermixed_args()
    if a.command == 'levels':
        for n in LEVELS:
            print(n)
    elif a.command == 'capture':
        capture(parse_levels(a.args), a.port, a.force)
    elif a.command == 'build':
        build(parse_levels(a.args))
    elif a.command == 'survey':
        lv = parse_levels(a.args)
        capture(lv, a.port, a.force)
        build(lv)
    elif a.command == 'shot':
        shot(' '.join(a.args), a.views, a.port)
    elif a.command == 'tour':
        tour(int(a.args[0]), a.views, a.port, a.rooms, a.points, a.mortal)
    elif a.command == 'secrets':
        secrets(parse_levels(a.args), a.views, a.port)
    elif a.command == 'models':
        models(int(a.args[0]), [int(x) for x in a.args[1:]], a.port)


if __name__ == '__main__':
    main()
