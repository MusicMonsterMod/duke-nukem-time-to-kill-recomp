"""AUTO internal resolution from a measured GPU probe (Time to Kill D23W).

The runtime's `--gpu-probe` draws a synthetic 4x frame (2048x960, three
layers of a lit textured shader, scaled to 1920x1080) and prints its GPU time.
Calibrated on the field laptop (Intel HD Graphics 5500, real display, UI slot 8
at 60 Hz): the probe read about 6.5 ms; 2x held 60 game frames a second
throughout, 3x dipped to 42 with audio underruns and 4x averaged 55 with dips
to 41. The cost scales with the pixel count (scale squared), so AUTO takes the
largest scale whose estimate fits SCALE_BUDGET_MS: 2x there, 4x on a GPU about
2.6 times faster or more.

The result is cached in config/gpu-probe.json with a fingerprint of the GPUs
(PCI ids on Linux) and the runtime binary, and measured again when either
changes. A failed probe falls back to FALLBACK_SCALE.
"""
import json
import os
import re
import subprocess
import sys
from pathlib import Path

SCALE_BUDGET_MS = 2.5
FALLBACK_SCALE = 2
DEFAULT_CACHE = Path(__file__).resolve().parents[2] / 'config' / 'gpu-probe.json'
PROBE_TIMEOUT_S = 20


def scale_for(ms):
    """The largest internal scale (4..1) whose estimated cost fits the budget."""
    for scale in (4, 3, 2):
        if ms * scale * scale / 16.0 <= SCALE_BUDGET_MS:
            return scale
    return 1


def fingerprint(exe):
    """GPUs and runtime binary; a change measures again."""
    gpus = []
    for card in sorted(Path('/sys/class/drm').glob('card[0-9]*')) if sys.platform.startswith('linux') else []:
        if '-' in card.name:
            continue
        ids = []
        for leaf in ('vendor', 'device', 'revision'):
            try:
                ids.append((card / 'device' / leaf).read_text().strip())
            except OSError:
                pass
        if ids:
            gpus.append(':'.join(ids))
    try:
        st = Path(exe).stat()
        binary = f'{st.st_size}:{int(st.st_mtime)}'
    except OSError:
        binary = '?'
    return {'gpus': gpus, 'binary': binary, 'platform': sys.platform}


def load_cache(cache_path):
    try:
        data = json.loads(Path(cache_path).read_text())
        return data if isinstance(data, dict) else None
    except (OSError, ValueError):
        return None


def run_probe(exe):
    """{'ms', 'renderer', 'version'} or {'error'}."""
    try:
        out = subprocess.run([str(exe), '--gpu-probe'], capture_output=True, text=True,
                             timeout=PROBE_TIMEOUT_S).stdout
    except (OSError, subprocess.TimeoutExpired) as exc:
        return {'error': type(exc).__name__}
    m = re.search(r'gpu probe: ms=([0-9.]+) renderer="([^"]*)" version="([^"]*)"', out)
    if not m:
        f = re.search(r'gpu probe: failed \(([^)]*)\)', out)
        return {'error': f.group(1) if f else 'no result'}
    return {'ms': float(m.group(1)), 'renderer': m.group(2), 'version': m.group(3)}


def resolve(exe, cache_path, log=None):
    """The AUTO internal scale for this machine, probing when the cache does not
    match. `log` (a print-like callable) receives one line when it measures."""
    fp = fingerprint(exe)
    cached = load_cache(cache_path)
    if cached and cached.get('fingerprint') == fp and isinstance(cached.get('scale'), int):
        return cached['scale']
    result = run_probe(exe)
    if 'ms' in result:
        scale = scale_for(result['ms'])
        line = (f"AUTO resolution: {scale}x (GPU probe {result['ms']:.2f} ms for a 4x frame on "
                f"{result['renderer']}; measured once per GPU and driver)")
    else:
        scale = FALLBACK_SCALE
        line = f"AUTO resolution: {scale}x (GPU probe unavailable: {result['error']})"
    record = {'fingerprint': fp, 'scale': scale, 'budget_ms': SCALE_BUDGET_MS, **result}
    try:
        if 'error' in result:
            raise OSError('a failed probe is not cached')
        Path(cache_path).parent.mkdir(parents=True, exist_ok=True)
        tmp = Path(str(cache_path) + '.tmp')
        tmp.write_text(json.dumps(record, indent=2) + '\n')
        os.replace(tmp, cache_path)
    except OSError:
        pass
    if log:
        log(line)
    return scale


def cached_scale(cache_path=DEFAULT_CACHE):
    """The last measured AUTO scale, without probing (None when unmeasured)."""
    cached = load_cache(cache_path)
    return cached.get('scale') if cached and isinstance(cached.get('scale'), int) else None
