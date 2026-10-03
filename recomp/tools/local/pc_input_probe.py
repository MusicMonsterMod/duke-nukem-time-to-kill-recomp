#!/usr/bin/env python3
"""Linux D04 window/input checks on a private Xvfb display and fresh cards."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import signal
import subprocess
import time

from debug_client import request
from vanilla_regression import ROOT, EXE, running_game


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name', required=True)
    parser.add_argument('--isolated-concurrent', action='store_true', help='permit a private Xvfb run alongside a player; requires a non-player debug port')
    parser.add_argument('--mode', choices=('vanilla', 'modernized'), default='modernized')
    parser.add_argument('--port', type=int, default=9164)
    parser.add_argument('--automatic-entry', action='store_true')
    parser.add_argument('--menu-back', action='store_true', help='test returning from difficulty to main menu instead of entering the level')
    parser.add_argument('--controls', choices=('movement', 'camera', 'weapons', 'facing', 'facing-research', 'presentation', 'wall', 'continuation', 'aim-options', 'traversal', 'polish', 'shortcuts', 'standing', 'red-dot'), help='extend first-map navigation with D05/D06 gameplay checks')
    parser.add_argument('--weapon-fixtures', action='store_true', help='D07 ONLY: grant/select weapon slots in isolated RAM; never save')
    parser.add_argument('--weapon-slots', default='5,6,7,8,9,10,11,12,13,14', help='comma-separated slots for isolated weapon fixtures')
    parser.add_argument('--synthetic-weapons', action='store_true', help='D07 only: select slots directly in isolated RAM; excludes animation/acquisition acceptance')
    parser.add_argument('--renderer', choices=('software','opengl'), default='software')
    parser.add_argument('--weapon-aim', choices=('view','original'), default='view')
    parser.add_argument('--aim-assist',choices=('off','original-lock'),default='off')
    parser.add_argument('--red-dot',choices=('on','off'),default='on')
    parser.add_argument('--crosshair',choices=('on','off'),default='on')
    parser.add_argument('--near-cover', action='store_true')
    args = parser.parse_args()
    try:
        slots=[int(x) for x in args.weapon_slots.split(',')]
        if not slots or any(x<1 or x>14 for x in slots): raise ValueError()
    except ValueError: parser.error('--weapon-slots requires IDs 1..14')
    if (args.weapon_fixtures or args.synthetic_weapons or args.near_cover) and args.controls != 'weapons': parser.error('--weapon-fixtures requires --controls weapons')
    if not args.name or any(c not in 'abcdefghijklmnopqrstuvwxyz0123456789-_' for c in args.name):
        parser.error('use a unique lowercase name')
    if args.isolated_concurrent and args.port == 9123:
        parser.error('concurrent probes must not use the player debug port')
    if running_game() and not args.isolated_concurrent:
        parser.error('a game is already running; refusing to interfere')
    with socket.socket() as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('127.0.0.1', args.port))
    directory = ROOT / 'analysis/pc-input' / args.name
    directory.mkdir(parents=True, exist_ok=False)
    report = {'aim_assist':args.aim_assist,'red_dot':args.red_dot,'crosshair':args.crosshair,'mode': args.mode, 'weapon_aim': args.weapon_aim, 'renderer': args.renderer, 'checks': [], 'binary_sha256': hashlib.sha256(EXE.read_bytes()).hexdigest(),
              'driver_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}
    def flush():
        (directory / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    def interrupted(signum, frame):
        raise InterruptedError(f'probe received signal {signum}')
    signal.signal(signal.SIGTERM, interrupted)
    flush()
    process = display = sink = None
    logs = []
    try:
        rfd, wfd = os.pipe()
        display = subprocess.Popen(['Xvfb', '-displayfd', str(wfd), '-screen', '0', '1024x768x24', '-nolisten', 'tcp'],
                                   pass_fds=(wfd,), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        os.close(wfd)
        with os.fdopen(rfd) as reader:
            number = reader.readline().strip()
        env = os.environ.copy()
        env.update(DISPLAY=':' + number, SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy',
                   PSX_OVERLAY_CAPTURES=str(directory / 'overlay-captures.json'))
        if args.controls in ('weapons','aim-options'): env['DNTTK_AIM_TRACE']='1'
        profiles = directory / 'profiles.json'
        command = ['python3', str(ROOT / 'tools/local/run.py'), '--settings-file', str(profiles),
                   '--mode', args.mode, '--memcard-dir', str(directory / 'cards'), '--debug-port', str(args.port),
                   '--renderer', args.renderer, '--weapon-aim', args.weapon_aim, '--aim-assist', args.aim_assist, '--red-dot', args.red_dot, '--crosshair',args.crosshair]
        if args.controls == 'movement':
            command += ['--camera', 'original']
        if args.mode == 'modernized':
            subprocess.run(command + ['--bind', 'jump=J', '--bind', 'jetpack=Space'], check=True, env=env, stdout=subprocess.DEVNULL)
        log = (directory / 'runtime.log').open('w'); logs.append(log)
        process = subprocess.Popen(command, cwd=directory, env=env, stdout=log, stderr=log, start_new_session=True)
        def call(cmd, **kw):
            value = request({'cmd': cmd, **kw}, args.port)
            if not value.get('ok'):
                raise RuntimeError(str(value))
            return value
        def eventually(fn, timeout=20):
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError('game exited early')
                try:
                    value = fn()
                    if value: return value
                except (OSError, RuntimeError, subprocess.CalledProcessError):
                    pass
                time.sleep(.03)
            raise TimeoutError('condition not met')
        eventually(lambda: call('frame'))
        def xdo(*items):
            return subprocess.check_output(['xdotool', *map(str, items)], env=env, text=True).strip()
        window = eventually(lambda: xdo('search', '--name', 'Duke')).splitlines()[0]
        xdo('windowfocus', '--sync', window)
        eventually(lambda: call('ttk_input')['input']['focused'])
        def check(name, predicate):
            result = eventually(predicate)
            report['checks'].append({'name': name, 'input': call('ttk_input')['input'],
                                     'pad': call('pad_status')['slot0'], 'frame': call('frame')['frame']})
            flush()
            print('PASS:', name, flush=True)
            return result
        def pad(): return int(call('pad_status')['slot0']['buttons'], 16)
        def state(): return call('ttk_input')['input']
        xdo('keydown', 'Up')
        check('fixed menu Up reaches SIO', lambda: pad() & 16 == 0)
        # Real X focus transfer while the key remains down.
        sink = subprocess.Popen(['xmessage', '-title', 'D04 focus sink', 'Isolated focus test'], env=env,
                                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        other = eventually(lambda: xdo('search', '--name', '^D04 focus sink$')).splitlines()[0]
        xdo('windowfocus', '--sync', other)
        check('focus loss releases SIO', lambda: pad() == 65535 and not state()['focused'])
        xdo('keyup', 'Up')
        xdo('windowfocus', '--sync', window)
        check('focus regain is neutral', lambda: pad() == 65535 and state()['focused'])
        if args.mode == 'modernized' and not args.automatic_entry:
            xdo('key', 'F10')
            check('explicit mouse capture', lambda: state()['captured'])
            xdo('keydown', 'w', 'd')
            check('independent diagonal without PS1 turn bits', lambda: .70 < state()['move_x'] < .71 and .70 < state()['move_y'] < .71 and pad() == 65535)
            xdo('keyup', 'w', 'd')
            xdo('keydown', 'j')
            check('persisted rebound jump reaches SIO', lambda: pad() & 32768 == 0)
            xdo('keyup', 'j')
            xdo('mousedown', '1')
            check('mouse fire reaches SIO', lambda: pad() & 16384 == 0)
            xdo('windowfocus', '--sync', other)
            check('focus loss releases capture and held mouse', lambda: not state()['captured'] and pad() == 65535)
            xdo('mouseup', '1')
            xdo('windowfocus', '--sync', window)
            check('no automatic recapture', lambda: state()['focused'] and not state()['captured'] and pad() == 65535)
            xdo('key', 'F10')
            check('recapture', lambda: state()['captured'])
            xdo('keydown', 'Escape')
            check('Escape invokes Start and releases mouse for menus', lambda: not state()['captured'] and pad() & 8 == 0)
            xdo('keyup', 'Escape')
            xdo('key', 'F10')
            check('capture before pause', lambda: state()['captured'])
            xdo('keydown', 'Return')
            check('pause releases capture and reaches SIO', lambda: not state()['captured'] and pad() & 8 == 0)
            xdo('keyup', 'Return')
        elif args.mode == 'vanilla':
            xdo('keydown', 'a')
            check('Vanilla original Triangle translation', lambda: pad() & 4096 == 0)
            xdo('keyup', 'a')
            xdo('key', 'F10')
            check('Vanilla never captures mouse', lambda: not state()['captured'] and not state()['modernized'])
        # Navigate title -> main menu -> difficulty -> level using real SDL keys.
        call('turbo', enabled=1)
        eventually(lambda: call('frame')['frame'] >= 900, 60)
        def pulse(key, wait_frames):
            xdo('keydown', key)
            hold_until = call('frame')['frame'] + 12
            eventually(lambda: call('frame')['frame'] >= hold_until, 30)
            xdo('keyup', key)
            target = call('frame')['frame'] + wait_frames
            eventually(lambda: call('frame')['frame'] >= target, 90)
        pulse('Escape' if args.mode=='modernized' else 'Return', 6000)
        for name, key, frames in [('main-menu', 'Return', 360), ('difficulty', 'x', 360), ('spawn', 'x', 900 if args.controls else 1200)]:
            pulse(key, frames)
            call('screenshot_file', path=str(directory / (name + '.png')))
            report['checks'].append({'name': name + ' SDL navigation capture', 'frame': call('frame')['frame'], 'menu_globals':call('read_ram',addr='0x800be550',len=48)['hex']})
            flush()
            if name == 'difficulty' and args.menu_back:
                pulse('z' if args.mode == 'modernized' else 'a', 360)
                call('screenshot_file', path=str(directory / 'menu-back.png'))
                report['checks'].append({'name': 'menu-back SDL navigation capture', 'frame': call('frame')['frame']})
                flush()
                break
        if args.controls == 'shortcuts':
            import re
            report['live_guards']=[]
            for address,size,digest in re.findall(r'\{(0x[0-9a-f]+), (\d+), "([0-9a-f]+)"', (ROOT/'src/ttk/control_guards.inc').read_text()):
                raw=bytes.fromhex(call('read_ram',addr=address,len=int(size))['hex'])
                actual=hashlib.sha256(raw).hexdigest()
                report['live_guards'].append({'address':address,'size':int(size),'ok':actual==digest,'actual':actual})
                if actual!=digest: print('GUARD MISMATCH',address,size,flush=True)
            flush()
        if args.automatic_entry:
            check('automatic gameplay capture without F10', lambda: state()['captured'] and state()['controls']['ready'])
        if args.controls:
            if args.controls == 'standing':
                from standing_jump_probe import exercise
                exercise(call,xdo,eventually,directory,report,flush)
            elif args.controls == 'shortcuts':
                from shortcuts_probe import exercise
                exercise(call,xdo,eventually,directory,report,flush)
            elif args.controls == 'polish':
                from polish_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'traversal':
                from traversal_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'red-dot':
                from red_dot_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'aim-options':
                from aim_options_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'continuation':
                from continuation_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'wall':
                from wall_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'presentation':
                from presentation_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'facing-research':
                from facing_research_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'facing':
                from facing_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush)
            elif args.controls == 'weapons':
                from weapon_aim_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush, fixtures=('synthetic' if args.synthetic_weapons else args.weapon_fixtures), near_cover=args.near_cover, slots=slots)
            else:
                from modern_controls_probe import exercise
                exercise(call, xdo, eventually, directory, report, flush, camera=args.controls == 'camera')
        call('turbo', enabled=0)
        try: call('quit')
        except (OSError, RuntimeError): pass
        report['exit_code'] = process.wait(timeout=10)
        if report['exit_code'] != 0: raise RuntimeError('nonzero game exit')
        report['result'] = 'input assertions pass; navigation captures require review'
    except BaseException as exc:
        try: report['failure_input']=call('ttk_input')['input']
        except Exception: pass
        report['error'] = repr(exc)
        raise
    finally:
        for proc in (process, sink, display):
            if proc and proc.poll() is None:
                if proc is process:
                    os.killpg(proc.pid, signal.SIGTERM)
                else:
                    proc.terminate()
                try: proc.wait(timeout=5)
                except subprocess.TimeoutExpired: proc.kill(); proc.wait()
        for log in logs: log.close()
        flush()
    print(directory / 'report.json')


if __name__ == '__main__':
    main()
