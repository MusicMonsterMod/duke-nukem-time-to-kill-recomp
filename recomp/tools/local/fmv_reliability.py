#!/usr/bin/env python3
"""Record D18 movie reliability evidence with private settings and blank cards.

Run under Xvfb for offscreen diagnostics, or on the desktop for hardware-backed
visual review. Uses real SDL audio, not dummy/headless audio. See note 281.
Non-intro movies use an explicit, guarded RAM-only startup table fixture; this
is decoder/teardown coverage, not natural campaign or native replay coverage.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import socket
import signal
import struct
import subprocess
import sys
import time

from check_cd_seeks import check
from debug_client import request
from disc_lab import Disc, EXE_SHA256
from vanilla_regression import running_game

ROOT = Path(__file__).resolve().parents[2]
MOVIES = {'logo': 0, 'gtlogo': 1, 'intro': 2, 'ending': 3,
          'rtopen': 4, 'credits': 5, 'kilt': 6, '3drlogo': 7}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--name', required=True)
    p.add_argument('--movie', choices=MOVIES, default='intro')
    p.add_argument('--mode', choices=('modernized', 'vanilla'), default='modernized')
    p.add_argument('--rate', choices=('60', '120'), default='120')
    p.add_argument('--scale', choices=('1', '4'), default='1')
    p.add_argument('--action', choices=('full', 'skip', 'late-skip', 'quit', 'display'), default='full')
    p.add_argument('--gameplay', action='store_true', help='after movies, start a fresh game and sample voice/music handoff')
    p.add_argument('--port', type=int, default=9290)
    a = p.parse_args()
    if not a.name or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_' for c in a.name):
        p.error('name must contain only letters, digits, hyphens and underscores')
    if a.mode == 'vanilla' and a.rate != '60':
        p.error('Vanilla runs at 60; specify --rate 60')
    if a.gameplay and a.action == 'quit':
        p.error('--gameplay is incompatible with --action quit')
    if not 1024 <= a.port <= 65535:
        p.error('port must be 1024..65535')
    if running_game():
        p.error('a player process is already running; refusing to attach')
    with socket.socket() as probe:
        probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        probe.bind(('127.0.0.1', a.port))
    exe_bytes = (ROOT / 'disc/SLUS_005.83').read_bytes()
    if hashlib.sha256(exe_bytes).hexdigest() != EXE_SHA256:
        p.error('owned executable identity mismatch')
    base = struct.unpack_from('<I', exe_bytes, 24)[0]
    table = exe_bytes[0x800c6764-base+2048:0x800c6764-base+2048+64]
    d = ROOT / 'analysis/d18-reliability' / a.name
    d.mkdir(parents=True, exist_ok=False)
    binary = ROOT / 'build-local/Duke_Nukem__Time_to_Kill_Recompiled'
    report = {'settings': vars(a), 'binary_sha256': hashlib.file_digest(binary.open('rb'), 'sha256').hexdigest(),
              'samples': [], 'events': [], 'coverage': 'diagnostic observations; listening and natural campaign acceptance separate'}
    disc = Disc(ROOT / 'disc/time-to-kill.bin')
    try:
        report['movie_extents'] = [f for f in disc.files() if f['path'].startswith('/MOV/')]
    finally:
        disc.close()
    process = None
    start = time.monotonic()

    def clean(value):
        return json.loads(json.dumps(value).replace(str(ROOT.parent), '$WORKSPACE').replace(str(Path.home()), '~'))

    def call(cmd, **kw):
        r = request({'cmd': cmd, **kw}, a.port)
        if not r.get('ok'):
            raise RuntimeError(f'{cmd}: {r}')
        return r

    def ram(addr, n):
        return bytes.fromhex(call('read_ram', addr=hex(addr), len=n)['hex'])

    def half(addr):
        return int.from_bytes(ram(addr, 2), 'little')

    def sample(tag):
        row = {'tag': tag, 't': time.monotonic()-start}
        for cmd in ('frame', 'fmv_state', 'audio_stats', 'cdrom_state'):
            row[cmd] = call(cmd)
        row['mode'] = half(0x800bcbb0)
        row['menu'] = half(0x800d24e4)
        row['phase'] = half(0x800ce1b0)
        report['samples'].append(clean(row))
        return row

    def capture(tag):
        call('screenshot_file', path=str(d/(tag+'.png')))
        staged = call('present_shot', path=str(d/(tag+'-present.png')))
        wait_until(lambda: call('present_shot_seq')['seq'] > staged['seq'], 5, 'present capture')
        report['events'].append(clean(call('present_shot_seq')))
        taps = call('audio_stats')['taps']
        for tap in (0, 1, 2):
            # start=-1 means oldest retained samples, not the latest tail.
            first = max(0, taps[tap]['frames']-44100*4)
            report['events'].append(clean(request({'cmd': 'audio_wav', 'tap': tap,
                'path': str(d/f'{tag}-tap{tap}.wav'), 'start': str(first), 'count': 44100*4}, a.port)))

    def press(mask):
        frame = call('frame')['frame']
        report['events'].append({'press_mask': mask, 'frame': frame, 'duration_frames': 12})
        call('press', buttons=65535-mask, frames=12)
        wait_until(lambda: call('frame')['frame'] >= frame+14, 20, 'input release')

    def confirm_menu():
        if a.mode == 'vanilla':
            press(16384)
            return
        game_pid = running_game()
        if not game_pid or str(d/'cards').encode() not in Path(f'/proc/{game_pid}/cmdline').read_bytes():
            raise RuntimeError('cannot identify this private player window')
        windows = subprocess.check_output(['xdotool', 'search', '--all', '--pid', str(game_pid),
                                          '--name', 'Time.to.Kill'], text=True).split()
        if len(windows) != 1:
            raise RuntimeError('expected one private diagnostic window')
        subprocess.run(['xdotool', 'key', '--window', windows[0], 'Return'], check=True)
        report['events'].append({'host_key': 'Return', 'frame': call('frame')['frame']})

    def wait_until(predicate, seconds, label):
        deadline = time.monotonic()+seconds
        while not predicate():
            if process.poll() is not None:
                raise RuntimeError('player exited during '+label)
            if time.monotonic() > deadline:
                raise TimeoutError(label)
            time.sleep(.05)

    cmd = [sys.executable, str(ROOT/'tools/local/run.py'), '--settings-file', str(d/'profiles.json'),
           '--memcard-dir', str(d/'cards'), '--debug-port', str(a.port), '--no-session-log',
           '--display', 'windowed', '--window-width', '960', '--frame-rate', a.rate, '--internal-scale', a.scale]
    if a.mode == 'vanilla':
        cmd.append('--vanilla')
    # Prevent unrelated developer experiments leaking into this private route.
    env = {k: v for k, v in os.environ.items() if not k.startswith(('DNTTK_', 'PSX_'))}
    env['SDL_AUDIODRIVER'] = 'pulseaudio'
    report['command'] = clean(cmd)
    try:
        with (d/'runtime.log').open('w') as log:
            process = subprocess.Popen(cmd, cwd=ROOT, env=env, stdout=log, stderr=log, start_new_session=True)
            deadline = time.monotonic()+30
            while True:
                try:
                    call('frame')
                    break
                except (OSError, RuntimeError):
                    if process.poll() is not None or time.monotonic()>deadline:
                        raise RuntimeError('debug service unavailable')
                    time.sleep(.1)
            wait_until(lambda: ram(0x800c6764, 64) == table, 25, 'resident movie table identity')
            if call('fmv_state')['mdec_decode_count']:
                raise RuntimeError('movie fixture arrived after decoding started')
            movie = MOVIES[a.movie]
            if movie != 2:
                replacement = table[movie*8:movie*8+8]
                report['events'].append({'fixture': 'startup movie table record 2', 'original': table[16:24].hex(),
                                         'replacement': replacement.hex(), 'movie_id': movie})
                for i, byte in enumerate(replacement):
                    call('write_ram', addr=hex(0x800c6774+i), val=hex(byte))
                if ram(0x800c6774, 8) != replacement:
                    raise RuntimeError('fixture readback failed')
            wait_until(lambda: call('fmv_state')['mdec_decode_count'] > 3, 30, 'first movie decoding')
            begun = time.monotonic()
            sample('movie-start')
            capture('movie-start')
            captured = skipped = False
            display_steps = 0
            next_log = 0
            while time.monotonic()-begun < 360:
                row = sample('movie-or-transition')
                elapsed = time.monotonic()-begun
                if elapsed >= next_log:
                    print(a.name, round(elapsed, 1), 'frame', row['frame']['frame'], 'mdec', row['fmv_state']['mdec_decode_count'],
                          'lba', row['cdrom_state']['last_sector']['lba'], 'out', row['audio_stats']['out'], flush=True)
                    next_log += 15
                if elapsed > 5 and not captured:
                    capture('movie-five')
                    captured = True
                    if a.action == 'quit':
                        report['quit_during_movie'] = row['fmv_state']['xa_stream_active'] == 1
                        break
                if a.action == 'display' and elapsed > (6 if display_steps == 0 else 12) and display_steps < 2:
                    game_pid = running_game()
                    if not game_pid or str(d/'cards').encode() not in Path(f'/proc/{game_pid}/cmdline').read_bytes():
                        raise RuntimeError('cannot identify this private player window')
                    windows = subprocess.check_output(['xdotool', 'search', '--all', '--pid', str(game_pid), '--name', 'Time.to.Kill'], text=True).split()
                    if not windows:
                        raise RuntimeError('no diagnostic window found')
                    for win in windows:
                        subprocess.run(['xdotool', 'windowsize', win, '800' if display_steps == 0 else '960',
                                        '600' if display_steps == 0 else '540'], check=True)
                        subprocess.run(['xdotool', 'key', '--window', win, 'F11'], check=True)
                    report['events'].append({'display_toggle_at': elapsed, 'step': display_steps})
                    display_steps += 1
                    time.sleep(.3)
                    capture('display-'+str(display_steps))
                    for win in windows:
                        report['events'].append({'display_geometry': subprocess.check_output(['xdotool', 'getwindowgeometry', '--shell', win], text=True),
                            'wm_state': subprocess.run(['xprop', '-id', win, '_NET_WM_STATE'], capture_output=True, text=True).stdout})
                if a.action in ('skip', 'late-skip', 'display') and not skipped and elapsed > (6 if a.action == 'skip' else 25):
                    press(8)
                    skipped = True
                if row['mode'] == 0 and row['menu'] == 5 and row['phase'] == 2:
                    time.sleep(2)
                    sample('title')
                    capture('title')
                    break
                time.sleep(.5)
            else:
                raise TimeoutError('movie/title transition')
            if a.gameplay:
                call('clear_input')
                # State 5/phase 2 can still draw a loading spinner and the
                # original title has an input delay. Do not equate it to ready.
                deadline = time.monotonic()+120
                def main_ready():
                    if a.mode == 'vanilla':
                        return half(0x800d24e4) == 7
                    if half(0x800d24e4) != 12:
                        return False
                    menu = call('render_replay').get('plugin', {}).get('frontend', {}).get('game_menu', {})
                    return menu.get('owned') and menu.get('page') == 5 and not menu.get('loading')
                while not main_ready():
                    if time.monotonic() > deadline:
                        raise TimeoutError('interactive main menu')
                    if half(0x800d24e4) == 5:
                        press(8)
                    sample('title-to-main')
                    time.sleep(1)
                capture('interactive-main')
                confirm_menu()
                def difficulty_ready():
                    if a.mode == 'vanilla':
                        return half(0x800d24e4) == 8
                    menu = call('render_replay').get('plugin', {}).get('frontend', {}).get('game_menu', {})
                    return menu.get('owned') and menu.get('page') == 6
                wait_until(difficulty_ready, 15, 'difficulty menu')
                capture('difficulty')
                confirm_menu()
                wait_until(lambda: half(0x800bcbb0) == 1 and half(0x800be568) == 0 and half(0x800ce1b0) == 2, 90, 'fresh gameplay')
                # Allow the level intro voice to finish and music to resume.
                for i in range(30):
                    sample('gameplay')
                    if i in (5, 25):
                        capture('gameplay-'+str(i))
                    time.sleep(1)
            for command in ('cdrom_command_history', 'overlay_loader_status', 'render_replay'):
                data = call(command, **({'count': 4096} if command == 'cdrom_command_history' else {}))
                (d/(command+'.json')).write_text(json.dumps(clean(data), indent=2)+'\n')
                if command == 'cdrom_command_history':
                    report['seek_checks'] = check(data)
            report['driver_completed'] = True
    except BaseException as exc:
        report['error'] = clean(f'{type(exc).__name__}: {exc}')
        raise
    finally:
        if process and process.poll() is None:
            try:
                call('quit')
            except (OSError, RuntimeError):
                pass
            try:
                process.wait(timeout=8)
            except subprocess.TimeoutExpired:
                report['forced_cleanup'] = True
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
        report['exit_code'] = process.returncode if process else None
        report['elapsed_s'] = time.monotonic()-start
        (d/'report.json').write_text(json.dumps(clean(report), indent=2)+'\n')
        logpath = d/'runtime.log'
        if logpath.exists():
            text = logpath.read_text(errors='replace').replace(str(ROOT.parent), '$WORKSPACE').replace(str(Path.home()), '~')
            logpath.write_text(text)
    print(a.name, 'completed; review report and captures', flush=True)


if __name__ == '__main__':
    main()
