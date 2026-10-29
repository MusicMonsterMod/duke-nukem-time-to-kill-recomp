# D38 - Playable piano with distance falloff

Done, user-accepted 2026-10-29; implementation/documentation commit and push
closeout explicitly authorized. Final revision uses MusicMonsterMod's supplied
WAV, replacing the initial MIDI render. Credits and processing:
[audio provenance](../recomp/assets/audio/README.md).

## Behavior and implementation

Successful Modernized use of the verified LEVEL01 piano (overlay tag 4,
type 102, actor kind 16) initializes pose 277 via 800493a4 from 8005a3a0.
The hook requires track zero, stowed equipment, committed-use bit 0x40,
normal modes and authenticated player/overlay identity. The native selector
has an owned-executable digest guard. No guest writes or animation changes.
A simulation-only held-E guard prevents native repeat use replaying the tune.

`piano_audio.cpp` embeds the final mono 44.1-kHz PCM on a separate voice
from gadget cues. Repeated use cannot restart, stack or relocate playback.
The source copies the piano's XYZ position. Listener position is Duke's root,
not the camera: full gain through 768 units, smoothstep falloff to silence at
4096, with a 20 ms slew. Playback advances while inaudible. This is distance
attenuation, without wall/door occlusion or stereo panning. Effects/master
volume apply; pause, load and world reset cancel. Vanilla remains unchanged.

## Verification and limits

- Final player build and controls/audio suites passed. Audio regressions cover
  exact PCM, gain/mute, gadget coexistence, cancellation, XYZ full/half/zero
  falloff, smoothing, silent progression and source retention.
- Hook regressions cover actor/caller/pose/phase/object/equipment/mode/pause/
  Vanilla refusal, held use and zero guest writes. Input/aim suites passed
  during initial implementation; they were not claimed newly run for revision 2.
- Private third-person 60-setting walking attenuated gain from 1 to 0.1316.
  First-person 120-setting walk-away/return passed, reaching 0.7931 and
  returning to 1. A prior first-person away route reached 0.1799 but failed
  an overly strict 0.15 travel assertion; that attempt is not a passing run.
- Exact out-of-range silence is native-test coverage. Natural movement routes
  did not establish leaving the full radius before this six-second tune ended.
  Initial candidate routes covered repeat, pause/load, active-save reload and
  native Vanilla use. Earlier Modern-capture assumptions in the Vanilla harness
  were invalid and excluded; the subsequent native-pad route passed.
- Finished asset: 6.035 seconds, -6.00 dBFS peak, -23.61 dBFS RMS, 532306 bytes.
  Revision-2 captured host output peaked at -6.81 dBFS; zero clipped samples.
  User acceptance supplies the listening/gameplay verdict, not broader campaign
  or occlusion coverage. Only the verified piano family is authenticated.
- All 40 player save/card files retained hashes and mtimes. Tests used private
  settings/cards. Evidence and failed attempts: `recomp/analysis/d08p/`, with
  revision-2 measurements and routes under `r2/` and parent `r2-*.log` files.

No post-acceptance gameplay change, rebuild or launch. No framework changes.
No next job selected. Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```
