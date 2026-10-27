# D08A25 - Night vision on/off sound

Status: Done, user-accepted 2026-10-25. Explicit lock-in and closeout authorized.

## Acceptance and closeout

User: "its perfect! lock it in and close us out". The final on/off cue and
in-game mix are accepted. Implementation, licensed audio/provenance, tests,
manual and documentation are authorized for commit/push as MusicMonsterMod.
Accepted executable: `a92f2378eb3ed1f9502aee095c22632d463b2eb6be37caca5212a94cc2020313`.
No post-acceptance gameplay changes, builds or launches. The runtime patch was
re-exported unchanged and verified against the clean pinned framework: all
51 changed files match. No dependency-pin or player-data changes. Evidence
limits below remain; user acceptance does not add campaign-wide test coverage.

## Delivered behavior

Successful Modern night vision activation through N or selected-item Enter
plays the selected 0.24-second A cue once on either activation or manual
removal, including while firing. Empty/refused activation, depletion,
pause/resume and restoring active goggles
from a save do not trigger it. Held-key suppression remains in the existing
input dispatcher. Original night vision and Vanilla retain their original audio.
Native visuals, battery drain, animation ownership and weapon timing are unchanged.

The redistributable WAV is embedded at build time, so launch directory and
missing external asset files cannot affect playback. It is the selected A
converted from 48 kHz to the runtime's 44.1 kHz mix rate. Source/license and
edit parameters are in `recomp/assets/audio/README.md` and
`night-vision-on.json`. The supplied camera recording is the sole audio source;
the Duke 3D video is reference only. Research inputs and audition exports stay local.

## Implementation and native evidence

`shortcuts.inc` requests the cue only after all guarded activation writes succeed.
`night_vision_audio.cpp` mixes mono PCM equally into both host channels with
saturating addition. The emulation-thread cursor is not serialized into guest
state. Rapid activations restart a single bounded cue rather than queueing sounds.
The mixer cancels on load generation change, leaving gameplay, switching to
Original/Vanilla, runtime audio resync or hard mute. Discard/turbo and netplay
paths reset it. No allocations, disk access or new audio device on activation.

The native SOUND slider at `8008acc8..8008acfc` stores
`floor(percent * 0x3fff / 100)` at `800c39e0`; native effect creation also
checks that value (`8006b788..8006b794`). A SHA-256 guard covers the slider
contract. The cue uses that live effects gain. The framework's host PCM seam
runs after SPU render but before existing fade, master volume and audio bridge.
It never changes emulated SPU RAM/voices or saved game state. Existing host
buffering still applies; no latency retuning is included.

## Initial activation-only verification (superseded by removal revision below)

- Player build, movie/math shard checks, controls/input/aim and new audio native
  tests pass. Dispatcher coverage counts on/off, both activation routes, held
  fire, empty/absent/dead/invalid/native-pending refusals and Original/Vanilla.
- Audio tests reproduce every embedded PCM sample at full effects volume,
  check proportional gain, mute with no deferred playback, end-of-clip silence,
  load/pause/reset/profile cancellation, chunk boundaries, restarts and saturation.
- Private Xvfb 60 FPS third-person route: one cue on N/Enter, off silent,
  held-key suppression, repeated toggles during fire, pause/load no replay.
- Captured the real host output tap with dummy audio at 44.1 kHz. The selected
  cue correlates 0.609 with the effects-enabled mix (native effects 9829/16383)
  versus 0.003 with effects muted; bridge rate correction accounts for a 0.995
  duration factor. Peak captured channel-average amplitude was 11186/32768.
  This verifies the output pipeline, not listening through physical speakers.
- Private 120 FPS first-person route: exactly eight cues for eight successful
  activations. Moving/firing, held keys, active save/load, pause/resume, shortened
  private battery depletion and empty-item rejection pass.
- Original route retains native firing rejection, equip animation and on/off
  completion, with zero new cue events. Vanilla exclusion is covered by native
  tests; no new live Vanilla activation coverage is claimed.
- All 40 player source save/card files retain hashes and modification times.
  Diagnostics used copied cards/checkpoints and private settings. Sessions stopped.
- Exported framework patch applies to clean pinned
  `08ec704a974b1f3a16335b4afeb340b9eff19926`; all 51 modified/added framework
  files match. Dependency pins unchanged; pre-existing edits preserved.

Local scripts, logs, PCM captures, source manifest and correlation report:
`recomp/analysis/d08a25/`. Candidate executable SHA-256:
`0db3051ee6370c402916f059cbc4dee4e7c8388894a964a5c4cb3fece596b48b`.

## Player check and acceptance

Listen to N activation and removal in either camera, in quiet play and while firing.
The user accepted A's loudness and timing, including the removal revision.
Closeout is authorized; the diagnostic coverage limits above remain.

```sh
python3 recomp/tools/local/run.py
```

Modern input captures automatically. NIGHT VISION should be MODERN (default).

## 2026-10-25 - D08A25 removal cue revision (Needs playtest)

User liked A in game and requested the same sound when taking goggles off,
as in Duke 3D. Successful Modern N/Enter toggles now play A in both directions.
Failed/empty activation, held-key repeats, depletion and load/pause remain silent.
Original/Vanilla and the approved asset, gain and audio mixer are unchanged.

Build and controls/audio native tests pass. A private 120 FPS first-person
route verifies on/off through N and Enter, repeated toggles while firing,
held-key suppression and no extra cue on pause or active save/load. Host PCM
captures match A on activation (correlation 0.583) and removal (0.631); both
are absent with effects muted (below 0.002). This is pipeline verification;
the revised in-game listening check remains with the user.

All 40 player source files retain hashes/mtimes; private session stopped.
No new framework edits, asset edits, commit/push or acceptance closeout.
Candidate: `a92f2378eb3ed1f9502aee095c22632d463b2eb6be37caca5212a94cc2020313`.
Launch: `python3 recomp/tools/local/run.py`; press N on and off.
