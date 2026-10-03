#!/usr/bin/env python3
"""Record a bounded Vanilla input route with fresh cards; captures require review.

Never attaches to a running game. Linux baseline only. See documentation/16.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import time

from debug_client import request
from ttk_state_probe import sample as sample_state

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / 'build-local/Duke_Nukem__Time_to_Kill_Recompiled'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def running_game():
    for entry in Path('/proc').iterdir():
        if entry.name.isdigit():
            try:
                if Path(os.readlink(entry / 'exe')).name.removesuffix(' (deleted)') == EXE.name:
                    return int(entry.name)
            except (FileNotFoundError, PermissionError, ProcessLookupError):
                pass
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name', required=True)
    parser.add_argument('--port', type=int, default=9152)
    parser.add_argument('--state-probe', action='store_true', help='record guarded read-only D03 candidate state at gameplay checkpoints')
    parser.add_argument('--mode', choices=('vanilla', 'modernized'), help='test the profile launcher using isolated settings')
    parser.add_argument('--renderer', choices=('opengl', 'software'), default='opengl')
    args = parser.parse_args()
    if not args.name or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_' for c in args.name):
        parser.error('name must contain only letters, digits, hyphens and underscores')
    if not 1024 <= args.port <= 65535:
        parser.error('port must be in 1024..65535')
    if not Path('/proc').is_dir():
        parser.error('this baseline runner currently requires Linux')
    pid = running_game()
    if pid:
        parser.error(f'game already running (PID {pid}); refusing to interfere')
    with socket.socket() as probe:
        probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        probe.bind(('127.0.0.1', args.port))
    directory = ROOT / 'analysis/vanilla-regression' / args.name
    directory.mkdir(parents=True, exist_ok=False)
    cards = directory / 'cards'
    command = [str(EXE), '--game', str(ROOT / 'game.local.toml'),
               '--disc', str(ROOT / 'disc/time-to-kill.cue'),
               '--memcard-dir', str(cards), '--debug-port', str(args.port),
               '--headless', '--renderer', args.renderer]
    if args.mode:
        command = [sys.executable, str(ROOT / 'tools/local/run.py'),
                   '--mode', args.mode, '--renderer', args.renderer,
                   '--settings-file', str(directory / 'profiles.json'),
                   '--memcard-dir', str(cards), '--debug-port', str(args.port), '--headless']
    identity_files = [EXE, Path(__file__), ROOT / 'tools/local/run.py', ROOT / 'tools/local/player_profiles.py', ROOT / 'tools/local/ttk_state_probe.py', ROOT / 'game.local.toml', ROOT / 'disc/import-receipt.json']
    for pattern in ('*.toml', '*.ini', 'bios/*.bin', 'cache/**/*.so'):
        identity_files.extend((ROOT / 'build-local').glob(pattern))
    identity_files.extend((ROOT / 'patches').glob('*.patch'))
    report = {'schema': 1, 'command': command, 'cwd': str(directory),
              'headless': True, 'semantic_result': 'requires capture review',
              'files_sha256': {str(p.relative_to(ROOT)): digest(p) for p in identity_files if p.is_file()},
              'environment': {k: v for k, v in os.environ.items() if k.startswith(('PSX_', 'DNTTK_'))},
              'git': {}, 'events': []}
    for repo in (ROOT, ROOT / 'psxrecomp'):
        report['git'][str(repo.relative_to(ROOT))] = {
            'head': subprocess.check_output(['git', '-C', str(repo), 'rev-parse', 'HEAD'], text=True).strip(),
            'status': subprocess.check_output(['git', '-C', str(repo), 'status', '--short'], text=True)}
    start = time.monotonic()
    process = None

    def flush():
        (directory / 'report.json').write_text(json.dumps(report, indent=2) + '\n')

    def call(cmd, **kwargs):
        response = request({'cmd': cmd, **kwargs}, args.port)
        if not response.get('ok'):
            raise RuntimeError(f'{cmd}: {response}')
        return response

    def wait_frame(target):
        deadline = time.monotonic() + 90
        while time.monotonic() < deadline:
            if process.poll() is not None:
                raise RuntimeError('game exited before checkpoint')
            if call('frame')['frame'] >= target:
                return
            time.sleep(.02)
        raise TimeoutError(f'frame {target} not reached in 90 seconds')

    def checkpoint(name, buttons=65535, hold=1, after=120):
        tracing = args.state_probe and name not in ('intro', 'title', 'main-menu', 'difficulty', 'spawn')
        if tracing:
            sample_state(call)  # Refuse unsupported resident code/overlay before arming diagnostics.
            call('wtrace_range', lo='0x800d719c', hi='0x800d71c2')
            call('wtrace_add', lo='0x800d7294', hi='0x800d729a')
            call('wtrace_add', lo='0x800d73f0', hi='0x800d73f2')
            for target in (0x800598f0, 0x80053500, 0x8007926c, 0x80056770, 0x80057230, 0x8003ade4, 0x8003aa48):
                call('fntrace_arm', target=hex(target))
            call('fntrace_clear')
            call('wtrace_add', lo='0x800d6eb0', hi='0x800d6f60')
            call('wtrace_clear')
        before = call('frame')['frame']
        call('press', buttons=buttons, frames=hold)
        wait_frame(before + after)
        row = {'name': name, 'buttons': buttons, 'hold_frames': hold,
               'wait_frames': after, 'before_frame': before,
               'after_frame': call('frame')['frame'],
               'elapsed_seconds': time.monotonic() - start}
        report['events'].append(row)
        row['capture'] = call('screenshot_file', path=str(directory / (name + '.png')))
        if args.state_probe and name not in ('intro', 'title', 'main-menu', 'difficulty'):
            row['state_probe'] = sample_state(call)
        if tracing:
            row['player_write_trace'] = call('wtrace_dump', addr_lo='0x800d719c', addr_hi='0x800d73f2', count=2048, newest=1)
            row['camera_write_trace'] = call('wtrace_dump', addr_lo='0x800d6eb0', addr_hi='0x800d6f60', count=512, newest=1)
            row['function_trace'] = call('fntrace_dump', count=2048)
            call('fntrace_arm_clear')
            call('wtrace_range', lo='0', hi='0')
        flush()
        print(f'{name}: frame {row["after_frame"]}', flush=True)

    try:
        with (directory / 'runtime.log').open('w') as log:
            process = subprocess.Popen(command, cwd=directory, stdout=log, stderr=log)
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError('game exited during startup')
                try:
                    call('frame')
                    break
                except (OSError, RuntimeError):
                    time.sleep(.1)
            else:
                raise TimeoutError('debug service unavailable')
            wait_frame(900)
            checkpoint('intro', after=1)
            checkpoint('title', 65527, 12, 6000)
            checkpoint('main-menu', 65527, 12, 120)
            checkpoint('difficulty', 49151, 12, 120)
            checkpoint('spawn', 49151, 12, 900)
            checkpoint('fire', 49151, 60, 65)
            checkpoint('holster', 57343, 12, 60)
            checkpoint('draw', 57343, 12, 60)
            checkpoint('jump', 32767, 12, 35)
            checkpoint('land', after=120)
            checkpoint('inventory', 65534, 12, 60)
            checkpoint('inventory-close', 65534, 12, 60)
            checkpoint('forward', 65519, 45, 60)
            checkpoint('turn', 65503, 30, 45)
            for cmd in ('frame', 'audio_stats', 'cdrom_command_history', 'dispatch_stats', 'overlay_loader_status'):
                report[cmd] = call(cmd)
            call('clear_input')
            # The server can close/timeout its reply while the process exits.
            try:
                report['quit_response'] = request({'cmd': 'quit'}, args.port)
            except (OSError, RuntimeError) as exc:
                report['quit_response'] = {'transport_error': str(exc)}
            report['exit_code'] = process.wait(timeout=10)
            if report['exit_code'] != 0:
                raise RuntimeError(f'unexpected exit code {report["exit_code"]}')
            report['driver_result'] = 'completed; gameplay assertions require review'
    except BaseException as exc:
        report['error'] = f'{type(exc).__name__}: {exc}'
        raise
    finally:
        if process and process.poll() is None:
            try:
                call('clear_input')
                call('quit')
                process.wait(timeout=5)
            except (OSError, RuntimeError, subprocess.TimeoutExpired):
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            report['cleanup_exit_code'] = process.returncode
        if (directory / 'profiles.json').exists():
            report['profile_settings'] = json.loads((directory / 'profiles.json').read_text())
        report['cards_sha256'] = {str(p.relative_to(directory)): digest(p) for p in cards.rglob('*') if p.is_file()}
        report['elapsed_seconds'] = time.monotonic() - start
        flush()
    print(directory / 'report.json')


if __name__ == '__main__':
    main()
