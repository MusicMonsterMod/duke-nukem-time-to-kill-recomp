#!/usr/bin/env python3
"""Summarize owned D03 reports without copying raw RAM or retail code."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
from ttk_hook_contract import qualifies

def summarize(path):
    report = json.loads(path.read_text())
    result = {'run': path.parent.name, 'report_sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
              'exit_code': report.get('exit_code'), 'error': report.get('error'), 'checkpoints': []}
    for row in report['events']:
        if 'state_probe' not in row:
            continue
        state = row['state_probe']
        entries = row.get('function_trace', {}).get('entries', [])
        accepted = sorted((x for x in entries if qualifies(x)), key=lambda x: x['seq'])
        counts = Counter((x['target'], x['ra']) for x in accepted)
        result['checkpoints'].append({
            'name': row['name'], 'frame_bracket': [state['frame_before'],state['frame_after']],
            'guard_count': len(state['code_guards']), 'overlay': state['overlay_guard'],
            'position': state['player_position_candidate'], 'rotation': state['player_rotation_candidate'],
            'qualified_context_counts': [{'target': k[0], 'ra': k[1], 'count': v} for k,v in sorted(counts.items())],
            'sequence_excerpt': accepted[:16] if row['name'] == 'forward' else [],
            'other_contexts_not_qualified': len(entries)-len(accepted)})
    return result

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('reports', type=Path, nargs='+')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.write_text(json.dumps({'schema': 1, 'runs': [summarize(p) for p in args.reports]}, indent=2)+'\n')
