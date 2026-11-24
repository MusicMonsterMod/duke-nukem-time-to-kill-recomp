#!/usr/bin/env python3
"""D23AH: are two builds the same game? VRAM and pictures, byte for byte.

For each executable (the normal build first), one offscreen launch per mode
loads each listed savestate in turn and holds no input; the runtime's own
exactness hook (PSX_VRAM_DUMP_DIR) writes the guest's main RAM, all of VRAM
and the picture
presented at +60, +200 and +400 vblanks after each load. Every executable
runs `--reps` times, so a scene that is not repeatable within one build
(asynchronous host work, note 508) is reported as such instead of as a
difference. Exit status 0 only when every repeatable scene matches the
first executable in every dump.

Cards are a temporary copy (read only: default the local play saves), the
settings a private copy of the player's; nothing of the player's is written.
"""
import argparse
import collections
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
from pgo_train import Game, ROOT  # noqa: E402


def md5(p):
    return hashlib.md5(p.read_bytes()).hexdigest()[:12] if p.is_file() else None


def run_one(exe, mode, slots, frames, cards, base, work, port, log, ctrl=None):
    """One launch: {slot: {name: digest}} for VRAM and pictures at each frame."""
    dump = work / 'dump'
    shutil.rmtree(dump, ignore_errors=True)
    dump.mkdir()
    s = json.loads(json.dumps(base))
    for prof in s['profiles'].values():  # timing-dependent overlays off: pictures comparable
        if 'fps' in prof.get('controls', {}):
            prof['controls'].update(fps=False, level_stats=False)
    if ctrl:
        s['profiles']['modernized']['controls'].update(ctrl)
    settings = work / f'profiles-{mode}.json'
    settings.write_text(json.dumps(s))
    # Every launch starts from the same pristine card copy: a launch may write
    # the cards (the game's own saves, the savestate log), and the next must
    # not see that.
    fresh = work / 'cards-run'
    shutil.rmtree(fresh, ignore_errors=True)
    shutil.copytree(cards, fresh)
    g = Game(exe, port, settings, fresh, mode == 'vanilla', log,
             dict(PSX_VRAM_DUMP_DIR=str(dump), PSX_VRAM_DUMP_FRAMES=','.join(map(str, frames))))
    out = {}
    try:
        g.wait_frames(60)
        for slot in slots:
            for f in dump.iterdir():
                f.unlink()
            g.keys('')
            g.call('savestate', op='load', slot=slot)
            t0 = g.call('frame')['frame']
            g.wait_frames(max(frames) + 30, limit=180)
            time.sleep(1.5)  # the last picture is written at its present
            names = ([f'ram-f{n}.bin' for n in frames] + [f'vram-f{n}.bin' for n in frames]
                     + [f'shot-f{n}.png' for n in frames])
            out[slot] = {n: md5(dump / n) for n in names}
            out[slot]['level'] = g.u(0x800be570)
            print(f'[compare] {exe.parent.name} {mode} slot {slot:02d} (level {out[slot]["level"]}, '
                  f'from frame {t0}):', ' '.join(f'{k[:-4]}={v}' for k, v in out[slot].items() if k != 'level'),
                  flush=True)
    finally:
        g.quit()
    return out


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--exe', type=Path, action='append', required=True,
                   help='executables to compare; the first is the reference (normally build-local)')
    p.add_argument('--memcard-dir', type=Path, default=ROOT / 'saves/local-play')
    p.add_argument('--slots', default='0 1 2', help='Modernized savestate slots')
    p.add_argument('--vanilla-slots', default='0', help="Vanilla savestate slots ('' for none)")
    p.add_argument('--frames', default='60,200,400')
    p.add_argument('--reps', type=int, default=2)
    p.add_argument('--port', type=int, default=9672)
    p.add_argument('--json', type=Path, default=None, help='write the digests here')
    p.add_argument('--ctrl', default='{}',
                   help='JSON merged into the Modernized controls (e.g. a deterministic timing set)')
    p.add_argument('--allow-running', action='store_true',
                   help='run beside another lab game (own port and card copy); never beside the player')
    a = p.parse_args()
    exes = [e.resolve() for e in a.exe]
    for e in exes:
        if not e.is_file():
            p.error(f'no executable {e}')
    if not a.allow_running and subprocess.run(['pgrep', '-x', 'Duke_Nukem__Tim'], capture_output=True).stdout:
        p.error('a game is running; close it first')
    frames = [int(x) for x in a.frames.split(',')]
    player = [x for x in a.memcard_dir.rglob('*') if x.is_file()] + [ROOT / 'config/player-profiles.json']
    before = {str(x): md5(x) for x in player}
    work = Path(tempfile.mkdtemp(prefix='ttk-compare-'))
    res = collections.defaultdict(lambda: collections.defaultdict(list))  # (mode, slot) -> exe -> [runs]
    try:
        cards = work / 'cards'
        shutil.copytree(a.memcard_dir, cards)
        src = ROOT / 'config/player-profiles.json'
        import player_profiles as profiles
        base = json.loads(src.read_text()) if src.is_file() else profiles.defaults()
        modes = [('modernized', [int(x) for x in a.slots.split()])]
        if a.vanilla_slots.strip():
            modes.append(('vanilla', [int(x) for x in a.vanilla_slots.split()]))
        with (work / 'game.log').open('a') as log:
            for rep in range(a.reps):
                for e in exes:  # interleaved: A B A B
                    for mode, slots in modes:
                        for slot, d in run_one(e, mode, slots, frames, cards, base, work, a.port, log, json.loads(a.ctrl)).items():
                            res[(mode, slot)][str(e)].append(d)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    ok = True
    report = []
    ref = str(exes[0])
    for (mode, slot), by in sorted(res.items()):
        refruns = by[ref]
        keys = [k for k in refruns[0] if k != 'level']
        stable = {k for k in keys if len({r[k] for r in refruns}) == 1 and refruns[0][k]}
        line = dict(mode=mode, slot=slot, level=refruns[0]['level'], repeatable=sorted(stable),
                    not_repeatable=sorted(set(keys) - stable), differs={})
        for e in exes[1:]:
            diff = [k for k in sorted(stable) if any(r[k] != refruns[0][k] for r in by[str(e)])]
            # outside the reference's repeatable dumps: does e show only values the reference showed?
            alien = [k for k in sorted(set(keys) - stable) if any(r[k] not in {x[k] for x in refruns} for r in by[str(e)])]
            line['differs'][Path(e).parent.name] = dict(repeatable=diff, other_values=alien)
            if diff:
                ok = False
        report.append(line)
        print(f'[result] {mode} slot {slot:02d} level {line["level"]}: repeatable {len(stable)}/{len(keys)}',
              ' '.join(f'{k}: {"SAME" if not v["repeatable"] else "DIFFERS " + ",".join(v["repeatable"])}'
                       f'{" (unrepeatable dumps with new values: " + ",".join(v["other_values"]) + ")" if v["other_values"] else ""}'
                       for k, v in line['differs'].items()), flush=True)
    if a.json:
        a.json.write_text(json.dumps(dict(exes=[str(e) for e in exes], frames=frames, reps=a.reps, scenes=report,
                                          runs={f'{m}/{s}': dict(v) for (m, s), v in res.items()}), indent=1))
    after = {str(x): md5(x) for x in player}
    print('[compare] player files', 'unchanged' if after == before else 'CHANGED', flush=True)
    print('[compare]', 'ALL REPEATABLE DUMPS IDENTICAL' if ok else 'BUILDS DIFFER', flush=True)
    return 0 if ok and after == before else 1


if __name__ == '__main__':
    raise SystemExit(main())
