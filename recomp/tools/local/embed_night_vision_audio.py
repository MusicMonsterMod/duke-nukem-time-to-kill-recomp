#!/usr/bin/env python3
"""Embed the approved, redistributable D08A25 mono PCM cue. No retail inputs."""
import struct
import sys
import wave
from pathlib import Path

with wave.open(sys.argv[1], 'rb') as source:
    if (source.getnchannels(), source.getsampwidth(), source.getframerate(), source.getnframes()) != (1, 2, 44100, 10584):
        raise SystemExit('night vision cue must be 0.24 s mono 44100 Hz signed 16-bit PCM')
    data = source.readframes(source.getnframes())
values = struct.unpack('<' + 'h' * (len(data) // 2), data)
text = '// Generated from the licensed night-vision-on.wav; do not edit.\n'
text += '#pragma once\n#include <cstdint>\nstatic constexpr int16_t night_vision_pcm[] = {\n'
text += '\n'.join('    ' + ','.join(map(str, values[i:i+20])) + ',' for i in range(0, len(values), 20))
Path(sys.argv[2]).write_text(text + '\n};\n')
