#!/usr/bin/env python3
"""Reproduce D08A24's edited cue from the user-supplied licensed MP3 (FFmpeg)."""
import array
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import sys
import wave

source, output = map(Path, sys.argv[1:3])
rate, seconds = 44100, 0.55
filters = 'atrim=start=0:end=0.55,highpass=f=100,lowpass=f=7000'
data = subprocess.check_output(['ffmpeg', '-v', 'error', '-i', str(source),
    '-af', filters, '-ac', '1', '-ar', str(rate), '-f', 'f32le', '-'])
samples = array.array('f'); samples.frombytes(data)
if sys.byteorder != 'little': samples.byteswap()
n = round(rate * seconds)
samples = list(samples[:n]) + [0.0] * max(0, n - len(samples))
for i in range(n):
    samples[i] *= min(1.0, i / (rate * 0.003), (n - 1 - i) / (rate * 0.035))
rms = math.sqrt(sum(x*x for x in samples) / n)
gain = min(10**(-21/20) / rms, 10**(-6/20) / max(map(abs, samples)))
values = [round(x * gain * 32767) for x in samples]
with wave.open(str(output), 'wb') as wav:
    wav.setparams((1, 2, rate, 0, 'NONE', 'not compressed'))
    wav.writeframes(struct.pack('<' + 'h'*n, *values))
manifest = {
    'source': source.name, 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
    'source_url': 'https://pixabay.com/sound-effects/film-special-effects-gas-decompression-329819/',
    'creator': 'CreatorsHome', 'title': 'Gas Decompression',
    'license': 'Pixabay Content License', 'license_url': 'https://pixabay.com/service/license-summary/',
    'license_checked': '2026-10-25', 'filter': filters,
    'fade_in_seconds': .003, 'fade_out_seconds': .035, 'gain': gain,
    'seconds': seconds, 'sample_rate': rate,
    'rms_dbfs': 20*math.log10(math.sqrt(sum(x*x for x in values)/n)/32768),
    'peak_dbfs': 20*math.log10(max(map(abs,values))/32768),
    'asset_sha256': hashlib.sha256(output.read_bytes()).hexdigest(),
    'use': 'Modified Bio Mask equip/removal effect embedded as part of the game; not standalone stock audio.'}
output.with_suffix('.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps(manifest, indent=2))
