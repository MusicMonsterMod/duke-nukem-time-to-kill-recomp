# Night vision activation cue

`night-vision-on.wav` is the user-selected D08A25 option A, 0.24 seconds,
mono 44.1 kHz 16-bit PCM. It is a modified camera-flash recording, not audio
extracted from Duke 3D. The Duke 3D video was used only as a timing/pitch reference.

Source: [Old camera flash](https://pixabay.com/sound-effects/technology-old-camera-flash-6800/),
MichelleGrobler (Freesound), distributed through freesound_community on Pixabay.
The source listing states free for use under the
[Pixabay Content License](https://pixabay.com/service/license-summary/),
checked 2026-10-25. This modified effect is included as part of the game.

Edits: reverse the source's 0.10-1.40 second tonal body, compress it to
0.222 seconds with Rubber Band at pitch ratio 0.5, filter 650-6500 Hz,
apply a falling envelope and fades, and mix a short source flash transient.
Audition A is normalized to -21 dBFS RMS. The selected audition is converted
from 48 kHz to 44.1 kHz by FFmpeg for the runtime's existing mix rate.
No further pitch, timing or gain changes are made in the game at full volume.
Source/selection hashes and parameters are in `night-vision-on.json`.
The local reproducible audition script and original input remain under
`research/d08a25-nightvision-audio/`; neither retail nor source recordings
are needed to build. The standard-library embed tool uses this licensed WAV.
