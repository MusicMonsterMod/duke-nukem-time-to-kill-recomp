#!/usr/bin/env python3
"""Embed supplied mono PCM cues. No retail inputs."""
import struct
import sys
import wave
from pathlib import Path

name = sys.argv[3] if len(sys.argv) > 3 else 'night_vision'
frames = {'night_vision': 10584, 'bio_mask': 24255, 'piano': 287318}[name]
with wave.open(sys.argv[1], 'rb') as source:
    if (source.getnchannels(), source.getsampwidth(), source.getframerate(), source.getnframes()) != (1, 2, 44100, frames):
        raise SystemExit(f'{name} cue must be {frames} frames of mono 44100 Hz signed 16-bit PCM')
    data = source.readframes(source.getnframes())
values = struct.unpack('<' + 'h' * (len(data) // 2), data)
text = f'// Generated from {Path(sys.argv[1]).name}; do not edit.\n'
text += '#pragma once\n#include <cstdint>\nstatic constexpr int16_t ' + name + '_pcm[] = {\n'
text += '\n'.join('    ' + ','.join(map(str, values[i:i+20])) + ',' for i in range(0, len(values), 20))
Path(sys.argv[2]).write_text(text + '\n};\n')
