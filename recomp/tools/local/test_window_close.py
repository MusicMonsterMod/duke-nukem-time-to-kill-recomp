#!/usr/bin/env python3
"""Linux/X11 integration: close healthy, halted, and idle-debug-client games.

Requires the built game, owned disc, gdb and wmctrl. Isolated test cards/logs
are written under analysis/security-key/. Sends the window manager's close
request and verifies process termination, not merely hidden presentation.
"""
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import time

from debug_client import request

ROOT = Path(__file__).resolve().parents[2]


def belongs_to_process(pid, root_pid):
    # GDB puts its inferior in a different process group; follow ancestry.
    for _ in range(16):
        if pid == root_pid:
            return True
        if pid <= 1:
            return False
        try:
            status = Path(f'/proc/{pid}/status').read_text()
        except FileNotFoundError:
            return False
        pid = int(next(line.split()[1] for line in status.splitlines()
                       if line.startswith('PPid:')))
    return False


def run_case(name, port):
    directory = ROOT / 'analysis/security-key' / ('close-' + name)
    directory.mkdir(parents=True, exist_ok=True)
    log_path = directory / 'runtime.log'
    command = [str(ROOT / 'build-local/Duke_Nukem__Time_to_Kill_Recompiled'),
               '--game', str(ROOT / 'game.local.toml'),
               '--disc', str(ROOT / 'disc/time-to-kill.cue'),
               '--memcard-dir', str(directory / 'cards'), '--debug-port', str(port),
               '--renderer', 'opengl', '--no-launcher']
    if name == 'fatal':
        command = ['gdb', '-q', '-batch', '-ex', 'break psx_scheduler_run',
                   '-ex', 'run', '-ex',
                   'call psx_fatal_halt("controlled window-close test")',
                   '--args'] + command
    client = None
    with log_path.open('w') as log:
        process = subprocess.Popen(command, cwd=directory, stdout=log, stderr=log,
                                   start_new_session=True)
        try:
            deadline = time.monotonic() + 60
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError(f'{name}: game exited before test; see {log_path}')
                if name == 'fatal':
                    if 'emulation halted; TCP' in log_path.read_text(errors='replace'):
                        break
                else:
                    try:
                        if request({'cmd': 'frame'}, port).get('frame', 0) > 30:
                            break
                    except (OSError, RuntimeError):
                        pass
                time.sleep(0.25)
            else:
                raise TimeoutError(f'{name}: ready condition not reached')
            if name == 'idle-client':
                client = socket.create_connection(('127.0.0.1', port))
                client.sendall(b'{"cmd":')  # deliberately incomplete request
                time.sleep(0.3)
            windows = subprocess.check_output(['wmctrl', '-lp'], text=True)
            window = None
            for line in windows.splitlines():
                fields = line.split()
                try:
                    if belongs_to_process(int(fields[2]), process.pid):
                        window = fields[0]
                        break
                except (ProcessLookupError, ValueError):
                    pass
            if window is None:
                raise RuntimeError(f'{name}: no window belonging to test process')
            start = time.monotonic()
            subprocess.run(['wmctrl', '-ic', window], check=True)
            code = process.wait(timeout=10)
            seconds = time.monotonic() - start
            if name != 'fatal' and code != 0:
                raise RuntimeError(f'{name}: unexpected exit code {code}')
            if name == 'fatal' and 'exited with code 01' not in log_path.read_text(errors='replace'):
                raise RuntimeError('fatal: debugger did not observe intentional exit(1)')
            return {'case': name, 'close_seconds': seconds, 'driver_exit_code': code,
                    'method': 'window-manager close request', 'passed': True}
        finally:
            if client:
                client.close()
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()


if __name__ == '__main__':
    results = []
    for index, case in enumerate(('healthy', 'fatal', 'idle-client')):
        result = run_case(case, 9140 + index)
        results.append(result)
        print(json.dumps(result), flush=True)
    output = ROOT.parent / 'documentation/reports/window-close-regression.json'
    output.write_text(json.dumps(results, indent=2) + '\n')
