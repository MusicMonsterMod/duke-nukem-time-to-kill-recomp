# Audio credits and asset provenance

## Night vision activation cue

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

## Bio Mask equip/removal cue

`bio-mask-toggle.wav` is a 0.55-second edited cue from
[Gas Decompression](https://pixabay.com/sound-effects/film-special-effects-gas-decompression-329819/)
by CreatorsHome, under the [Pixabay Content License](https://pixabay.com/service/license-summary/)
(source and license checked 2026-10-25). The user supplied the MP3 and requested
this sound on mask activation/removal, authorizing edits as appropriate.

The edit retains the initial gas burst and decay, removes the nearly silent
remainder, filters 100-7000 Hz, and applies 3 ms / 35 ms edge fades. Gain targets
-21 dBFS RMS subject to a -6 dBFS peak ceiling; the transient makes the ceiling
binding (actual RMS -26.89 dBFS). No pitch or speed change. Mono 44.1 kHz 16-bit
PCM matches the runtime mixer. Endpoints are zero. This modified effect is
embedded as part of the game, not distributed as standalone stock audio.

`bio-mask-toggle.json` records hashes and parameters. Reproduce with:

```sh
python3 recomp/tools/local/prepare_bio_mask_audio.py /path/to/creatorshome-gas-decompression-329819.mp3 recomp/assets/audio/bio-mask-toggle.wav
```

The normal build uses the tracked edited WAV and the standard-library embed
script; it needs neither FFmpeg nor the original recording. No retail sound is
included. Successful activation and removal (automatic or manual) play
one cue; depletion, failed use, pause, restoring an active mask in gas, and held repeats do not. Restoring an
active mask in clean air removes it after the exit grace, with a removal cue.

## Playable piano

Piano performance and supplied recording: **MusicMonsterMod**. The short
performance interpolates "Grabbag" by **Lee Jackson**; this credit identifies
MusicMonsterMod as the performer, not the composer of the underlying theme. The current source
(D38A) is the audio track of the user's MIDI-keyboard re-recording
`research/music/pianobag-final.mp4`, replacing the D38 `pianobag.wav` take.

`piano.wav` keeps that performance's notes, timing and pitch. The finishing
pass folds to mono, removes subsonic rumble (50 Hz, 24 dB/oct), trims 2 dB of
250-Hz mud, lifts presence (+5 dB at 2.8 kHz, +3 dB shelf at 5 kHz), applies
gentle 2:1 compression above -24 dB, resamples to 44.1 kHz with soxr, and adds
a deterministic damped small-room response 15 dB under the dry signal. The
leading silence is trimmed to 5 ms; the recording's cut-off tail rings out
350 ms and ends in a 700 ms raised-cosine release. Loudness matches the
accepted D38 asset (-23.3 LUFS) under a -6 dBFS peak ceiling, with TPDF dither.
Final duration is 6.515 seconds (287318 frames), peak -9.59 dBFS and RMS
-23.92 dBFS. `piano.json` records source/output hashes and measurements.
Reproduce from the local source with:

```sh
python3 recomp/tools/local/prepare_piano_audio.py research/music/pianobag-final.mp4 recomp/assets/audio/piano.wav
```

The ordinary build embeds the finished WAV; it needs neither the original
recording, NumPy, FFmpeg, a MIDI renderer nor a soundfont. Playback starts on successful
Modernized piano use, follows effects/master volume, and fades with distance
from the piano. See [D38/D38A](../../../documentation/315-d38-playable-piano.md).
