#!/usr/bin/env python3
"""Measure Time to Kill's intro using a fresh process and isolated memory cards."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import time

from debug_client import request

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--name', default='fmv-current')
    parser.add_argument('--samples', type=int, default=20)
    parser.add_argument('--start-frame', type=int, default=900)
    parser.add_argument('--port', type=int, default=9123)
    parser.add_argument('--disable-poll', action='store_true')
    args = parser.parse_args()
    if not args.name or any(c not in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_' for c in args.name):
        parser.error('name must contain only letters, digits, hyphens and underscores')
    if args.samples < 2:
        parser.error('at least two samples are required')
    docs = ROOT.parent / 'documentation'
    env = dict(os.environ, DNTTK_FMV_POLL='0' if args.disable_poll else '1')
    log_path = docs / 'logs' / (args.name + '.log')
    report_path = docs / 'reports' / (args.name + '.json')
    for path in (log_path, report_path):
        path.parent.mkdir(parents=True, exist_ok=True)
    command = [str(ROOT / 'build-local/Duke_Nukem__Time_to_Kill_Recompiled'),
               '--game', str(ROOT / 'game.local.toml'),
               '--disc', str(ROOT / 'disc/time-to-kill.cue'),
               '--memcard-dir', str(ROOT / 'saves' / args.name),
               '--debug-port', str(args.port), '--renderer', 'opengl', '--no-launcher']
    rows = []
    with log_path.open('w') as log:
        process = subprocess.Popen(command, cwd=ROOT, env=env, stdout=log, stderr=log)
        try:
            deadline = time.monotonic() + 90
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError(f'game exited early; see {log_path}')
                time.sleep(0.5)
                try:
                    if request({'cmd': 'frame'}, args.port).get('frame', 0) >= args.start_frame:
                        break
                except (OSError, RuntimeError):
                    continue
            else:
                raise TimeoutError('game did not reach the measurement frame')
            for _ in range(args.samples):
                row = {'wall': time.monotonic()}
                for cmd in ('frame', 'audio_stats', 'fmv_state'):
                    row[cmd] = request({'cmd': cmd}, args.port)
                    if not row[cmd].get('ok'):
                        raise RuntimeError(f'{cmd}: {row[cmd]}')
                rows.append(row)
                time.sleep(1)
            result = {'poll_enabled': not args.disable_poll, 'samples': rows,
                      'profile': request({'cmd': 'phase_profile'}, args.port),
                      'overlay': request({'cmd': 'overlay_loader_status'}, args.port)}
            result['guest_fps'] = ((rows[-1]['frame']['frame'] - rows[0]['frame']['frame']) /
                                   (rows[-1]['wall'] - rows[0]['wall']))
            result['underrun_samples_added'] = (rows[-1]['audio_stats']['out']['underruns'] -
                                                rows[0]['audio_stats']['out']['underruns'])
            report_path.write_text(json.dumps(result, indent=2) + '\n')
            print(json.dumps({k: result[k] for k in ('guest_fps', 'underrun_samples_added', 'profile')}, indent=2))
        finally:
            if process.poll() is None:
                try:
                    request({'cmd': 'quit'}, args.port)
                except (OSError, RuntimeError):
                    pass
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.terminate()
                    try:
                        process.wait(timeout=5)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()


if __name__ == '__main__':
    main()
