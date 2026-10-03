#!/usr/bin/env python3
"""Check recorded CD command history for reads continuing at a pre-seek location.

Input is the JSON response to cdrom_command_history. Only the first ReadN/ReadS
after an explicit SeekL/SeekP is checked; another Setloc supersedes that target.
The 16-sector allowance covers command-service observation latency, not a
hardware timing assertion. A seek with no subsequent read is not a passing case.
"""
import argparse
import json
from pathlib import Path


def sector(msf):
    return (msf[0] * 60 + msf[1]) * 75 + msf[2]


def check(history):
    target = None
    results = []
    for entry in sorted(history['entries'], key=lambda e: e['seq']):
        if entry['kind'] != 'exec':
            continue
        command = int(entry['cmd'], 16)
        if command in (0x15, 0x16):
            target = entry
        elif command in (0x02, 0x08, 0x0a):
            target = None
        elif command in (0x06, 0x1b) and target is not None:
            distance = sector(entry['read_msf']) - sector(target['seek_msf'])
            results.append({'seek_frame': target['frame'], 'read_frame': entry['frame'],
                            'target_msf': target['seek_msf'], 'read_msf': entry['read_msf'],
                            'sector_delta': distance, 'ok': 0 <= distance <= 16})
            target = None
    return results


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('history', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = check(json.loads(args.history.read_text()))
    summary = {'checked': len(result), 'failed': sum(not r['ok'] for r in result),
               'seeks': result}
    text = json.dumps(summary, indent=2) + '\n'
    if args.output:
        args.output.write_text(text)
    print(text, end='')
    raise SystemExit(0 if result and all(r['ok'] for r in result) else 1)
