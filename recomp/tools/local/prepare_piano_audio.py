#!/usr/bin/env python3
"""Finish the supplied WAV as a mono 44.1-kHz piano interaction cue.

Usage: prepare_piano_audio.py INPUT.wav OUTPUT.wav
Keeps the supplied performance and timbre; no time/pitch changes or compression.
"""
import array
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys
import tempfile
import wave


def main():
    source, output = map(Path, sys.argv[1:])
    with tempfile.TemporaryDirectory(prefix='ttk-piano-') as tmp:
        filtered = Path(tmp)/'filtered.raw'
        subprocess.run(['ffmpeg', '-v', 'error', '-y', '-i', str(source), '-af',
                        'highpass=f=40', '-ac', '1', '-ar', '44100',
                        '-f', 'f32le', str(filtered)], check=True)
        pcm = array.array('f', filtered.read_bytes())
        if sys.byteorder != 'little': pcm.byteswap()
        # Retain the whole performance, then 250 ms beyond the last -60 dBFS
        # sample. Fade only the quiet end, never truncate an internal rest.
        active = [i for i, value in enumerate(pcm) if abs(value) > 0.001]
        if not active: raise ValueError('Source is silent')
        pcm = pcm[max(0, active[0]-221):min(len(pcm), active[-1]+11026)]
        for i in range(min(88, len(pcm))): pcm[i] = pcm[i]*i/88
        for i in range(min(4410, len(pcm))): pcm[-1-i] = pcm[-1-i]*i/4410
        peak = max(abs(v) for v in pcm)
        rms = math.sqrt(sum(v*v for v in pcm)/len(pcm))
        # Same conservative -6 dBFS peak ceiling as Bio Mask, with a -21 dBFS
        # RMS target. Preserve the piano's natural crest factor and dynamics.
        gain = min(10**(-6/20)/peak, 10**(-21/20)/rms)
        pcm = array.array('h', (round(v*gain*32767) for v in pcm))
        stats = {'frames': len(pcm), 'rate': 44100, 'channels': 1,
                 'duration_seconds': len(pcm)/44100, 'gain_db': 20*math.log10(gain),
                 'peak_dbfs': 20*math.log10(max(abs(v) for v in pcm)/32768),
                 'rms_dbfs': 20*math.log10(math.sqrt(sum(v*v for v in pcm)/len(pcm))/32768),
                 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
                 'source_name': source.name, 'source_bytes': source.stat().st_size,
                 'source': 'User-performed pianobag.wav; supplied soundfont rendering preserved',
                 'processing': 'mono 44100 Hz, 40 Hz high-pass, -60 dBFS tail trim plus 250 ms, 2 ms/100 ms fades, -21 dBFS RMS target with -6 dBFS peak ceiling'}
        if sys.byteorder != 'little': pcm.byteswap()
        output.parent.mkdir(parents=True, exist_ok=True)
        with wave.open(str(output), 'wb') as wav:
            wav.setparams((1, 2, 44100, 0, 'NONE', 'not compressed'))
            wav.writeframes(pcm.tobytes())
        stats['wav_sha256'] = hashlib.sha256(output.read_bytes()).hexdigest()
        output.with_suffix('.json').write_text(json.dumps(stats, indent=2)+'\n')
        print(json.dumps(stats, indent=2))


if __name__ == '__main__': main()
