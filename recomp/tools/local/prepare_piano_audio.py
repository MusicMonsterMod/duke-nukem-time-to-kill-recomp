#!/usr/bin/env python3
"""Finish the user's recorded piano take as a mono 44.1-kHz interaction cue.

Usage: prepare_piano_audio.py INPUT OUTPUT.wav
INPUT is any FFmpeg-readable recording (D38A: research/music/pianobag-final.mp4).
Keeps the performance's notes, timing and pitch. The pass folds to mono,
removes subsonic rumble, clears low-mid mud, lifts presence so the dark
keyboard tone reads in the game mix, evens dynamics with gentle 2:1
compression, adds a short damped room (a small wooden saloon), trims the
leading silence, rings out and fades the cut-off tail, then matches the
accepted D38 asset's loudness under a -6 dBFS peak ceiling with TPDF dither.
"""
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import wave

import numpy as np

RATE = 44100
# Integrated loudness of the user-accepted D38 revision-2 asset, so the
# replacement keeps the in-game level the user signed off.
TARGET_LUFS = -23.3
PEAK_CEILING_DB = -6.0
CHAIN = ('pan=mono|c0=0.5*c0+0.5*c1,'
         'highpass=f=50:p=2,highpass=f=50:p=2,'
         'equalizer=f=250:t=q:w=1.0:g=-2,'
         'equalizer=f=2800:t=q:w=1.0:g=5,'
         'equalizer=f=5000:t=h:w=0.7:g=3,'
         'acompressor=threshold=-24dB:ratio=2:attack=8:release=160:knee=4,'
         'aresample=44100:resampler=soxr:precision=28')
ROOM_WET_DB = -15.0


def decode(path, filters):
    data = subprocess.run(['ffmpeg', '-v', 'error', '-i', str(path), '-vn', '-af', filters,
                           '-f', 'f32le', '-'], check=True, capture_output=True).stdout
    return np.frombuffer(data, dtype='<f4').astype(np.float64)


def room_ir(rate):
    """Deterministic small-room response: early wood reflections, then a
    diffuse tail whose highs die faster than its lows (RT60 0.65/0.30 s)."""
    rng = np.random.default_rng(38)
    n = int(rate*0.9)
    t = np.arange(n)/rate
    noise = rng.standard_normal(n)
    spectrum = np.fft.rfft(noise)
    freq = np.fft.rfftfreq(n, 1/rate)
    split = 1/(1+(freq/2500)**2)
    low = np.fft.irfft(spectrum*split, n)
    high = np.fft.irfft(spectrum*(1-split), n)
    tail = low*np.exp(-6.91*t/0.65) + high*np.exp(-6.91*t/0.30)
    tail *= np.clip((t-0.012)/0.02, 0, 1)  # 12 ms pre-delay, 20 ms build
    for delay_ms, gain in ((7, 0.55), (11, -0.4), (17, 0.35), (23, -0.25), (31, 0.2)):
        tail[int(rate*delay_ms/1000)] += gain*np.abs(tail).max()*2
    return tail/math.sqrt(np.sum(tail**2))


def loudness(samples, tmp):
    path = Path(tmp)/'measure.wav'
    write_wav(path, np.clip(np.round(samples*32767), -32768, 32767).astype('<i2'))
    log = subprocess.run(['ffmpeg', '-hide_banner', '-nostats', '-i', str(path), '-af', 'ebur128',
                          '-f', 'null', '-'], capture_output=True, text=True).stderr
    return float(re.findall(r'I:\s+(-?[\d.]+) LUFS', log)[-1])


def write_wav(path, pcm):
    with wave.open(str(path), 'wb') as wav:
        wav.setparams((1, 2, RATE, 0, 'NONE', 'not compressed'))
        wav.writeframes(pcm.astype('<i2').tobytes())


def main():
    source, output = map(Path, sys.argv[1:])
    dry = decode(source, CHAIN)
    active = np.nonzero(np.abs(dry) > 0.001)[0]
    if not len(active): raise ValueError('Source is silent')
    # 5 ms before the first note; keep everything to the recording's end
    # (it stops mid-decay), then let the room ring 350 ms past it.
    dry = dry[max(0, active[0]-221):]
    dry = np.concatenate([dry, np.zeros(int(RATE*0.35))])
    wet = np.fft.irfft(np.fft.rfft(dry, len(dry)*2)*np.fft.rfft(room_ir(RATE), len(dry)*2))[:len(dry)]
    mix = dry + wet*10**(ROOM_WET_DB/20)
    # The recording's cut becomes a 700 ms raised-cosine release ending in
    # silence; the 2 ms head fade only guards the first sample.
    fade = int(RATE*0.7)
    mix[-fade:] *= 0.5+0.5*np.cos(np.linspace(0, math.pi, fade))
    mix[:88] *= np.arange(88)/88
    with tempfile.TemporaryDirectory(prefix='ttk-piano-') as tmp:
        gain = 10**((TARGET_LUFS-loudness(mix, tmp))/20)
        gain = min(gain, 10**(PEAK_CEILING_DB/20)/np.abs(mix).max())
        rng = np.random.default_rng(3838)
        dither = (rng.random(len(mix))-rng.random(len(mix)))/32767
        pcm = np.clip(np.round((mix*gain+dither)*32767), -32768, 32767).astype('<i2')
        pcm[0] = pcm[-1] = 0
        output.parent.mkdir(parents=True, exist_ok=True)
        write_wav(output, pcm)
        level = pcm.astype(np.float64)/32768
        stats = {'frames': len(pcm), 'rate': RATE, 'channels': 1,
                 'duration_seconds': len(pcm)/RATE, 'gain_db': 20*math.log10(gain),
                 'peak_dbfs': 20*math.log10(np.abs(level).max()),
                 'rms_dbfs': 20*math.log10(math.sqrt(np.mean(level**2))),
                 'integrated_lufs': loudness(level*32768/32767, tmp),
                 'source_sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
                 'source_name': source.name, 'source_bytes': source.stat().st_size,
                 'source': 'User-performed MIDI-keyboard re-recording (pianobag-final.mp4 audio track)',
                 'processing': 'mono fold-down; 50 Hz 24 dB/oct high-pass; -2 dB 250 Hz, +5 dB 2.8 kHz, +3 dB 5 kHz shelf; '
                               '2:1 compression above -24 dB; soxr to 44100 Hz; -15 dB damped small-room reverb '
                               '(RT60 0.65/0.30 s); lead trim to 5 ms; 350 ms ring-out and 700 ms tail fade; '
                               '-23.3 LUFS (accepted D38 level) with -6 dBFS peak ceiling; TPDF dither',
                 'wav_sha256': hashlib.sha256(output.read_bytes()).hexdigest(),
                 'performer': 'MusicMonsterMod'}
    output.with_suffix('.json').write_text(json.dumps(stats, indent=2)+'\n')
    print(json.dumps(stats, indent=2))


if __name__ == '__main__': main()
