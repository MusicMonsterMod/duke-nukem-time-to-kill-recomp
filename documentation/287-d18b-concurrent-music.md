# D18B - Concurrent gameplay music and speech

Status: Done, user-accepted 2026-10-27. User: "perfect!!!! lock it in and close
it out. absolutely beautiful". Implementation, tests, manual and documentation
commit/push are explicitly authorized as MusicMonsterMod. Modernized enables
this by default; Vanilla retains the native single-stream behavior.

## Cause and implementation

The original game uses one XA/CD stream for both music and speech. The native
speech owner at `0x8003c000` mutes that stream, selects a speech channel and
seeks to its clip. The update at `0x8003b9d0` eventually calls `0x8003c240` to
seek back to the saved music position. Removing a mute alone cannot make the
two sources coexist. This is original resource ownership, distinct from the
accepted D18D seek-callback fix and the remaining D18C crackle investigation.

`recomp/src/ttk/concurrent_music.cpp` adds an independent, bounded music stream:

- Authenticate the resident native audio state-machine code and validate the
  active track, channel, start/end positions and disc base before taking over.
- Read the mounted, owned disc without changing the emulated CD position, IRQs,
  filter, queue or decoder predictors. No extracted music pack is required.
- Reuse the runtime's existing XA decoding and resampling with private predictor
  history. The five original channels in MUSIC1.IDF keep their original loop
  ranges. No tracks are assigned to otherwise silent challenge stages.
- Silence only the native decoded PCM for the owned music channel/range. The
  native drive still runs normally; speech selection, eligibility, seeks,
  completion callbacks, effects and movies remain on their existing paths.
- Advance music on the existing guest-cycle audio budget while speech is
  playing or seeking. Pause holds the cursor; track changes, death/stop,
  leaving gameplay, load generation changes and host audio resets clear it.
- Mix music after the SPU using the native MUSIC and main gains. SPEECH remains
  independent, including live edits: the original MUSIC preview setter cannot
  overwrite an active voice's CD gain. MASTER still uses the existing output
  control. Mono averages the music channels; stereo preserves them.
- Protect the combined stereo peak with an immediate linked gain reduction and
  a 50 ms linear release. This adds no lookahead, output queue or latency. An
  earlier candidate exposed a few over-range sums when effects and music
  coincided; peak protection addresses those sums, not production starvation.

The runtime additions are `cdrom_host_xa_read`, a native-XA payload filter and
calls from the existing main audio pump. The isolated reader saves/restores the
native predictor samples synchronously on the emulation thread. No decoder
algorithm or primary drive timing was changed. Discarded output/netplay cannot
retain music suppression. Read errors fail back to native playback.

Native state evidence: `0x800c26a8` is the drive state, `0x800c26b4` the track,
`0x800d7178` the disc base (23), `0x800d7194` the music start,
`0x800d7180` the end, `0x800d7184` the native saved cursor and
`0x800d718c` the channel. The five table records begin at `0x800c26b8`.
`0x800bb51c` writes its two gain arguments directly to SPU CD L/R registers.
D21's MUSIC/SPEECH/MODE globals remain their existing owners.

## Verification

Native tests:

```sh
cmake --build recomp/build-local --target ttk-music-test ttk-xa-test ttk-audio-test --parallel 4
recomp/build-local/ttk-music-test recomp/disc/SLUS_005.83
recomp/build-local/ttk-xa-test
recomp/build-local/ttk-audio-test
```

All pass. Music contracts cover continued cursors through voice/seek states,
pause, volume zero without deferred playback, stereo/mono, track changes,
loop boundaries, load restoration, corrupted identity, bad source reads,
Vanilla exclusion, independent live speech gain, reset suppression and peak
protection. XA tests exercise the actual runtime reader/decoder for stereo and
mono at both supported rates, compare PCM and final predictors against the
native decoder, and prove native predictors/stream identity survive unchanged.
The existing gadget PCM suite also passes.

Private evidence is under `recomp/analysis/d18b/`; no player cards or settings
were modified. `probe.py` copies earlier private test cards and preferences
into a new directory per run. The deterministic speech fixture clears the
already-played bit for IDs 1-3 and queues them through the game's normal
`0x800c2704` request owner. This is native playback coverage, not proof that all
natural speech triggers have been visited. The level-1 opening voice is a
natural level-entry route via the existing debug level selector.

| Route | Evidence |
| --- | --- |
| `final60` | Real SDL/PulseAudio, Xvfb OpenGL 1x: repeated voice/music overlap, real Escape pause/resume, reload and level-1 opening speech/track change. Zero output underrun samples throughout the recorded route; captured host WAV slices have zero full-scale samples. |
| `final120` | Same lifecycle/capture route at 120, software OpenGL: overlap and restoration work, but output underruns occur. This run is not clean performance acceptance. |
| `original120lean` | Concurrent music disabled, same software renderer, less polling: underruns rise from 91790 to 156240 during the measured interval. The original path also starves here. |
| `gpu-final120lean` | NVIDIA offscreen OpenGL, real PulseAudio, repeated voice route: zero output underruns across all recorded samples; approximately 169-183 ms queue fill against the unchanged 180 ms target. |
| `vanilla60lean` | Native voice-to-music returns, zero output underruns, no independent-music activation. |
| `d18b-final-intro` | Existing FMV runner: Modernized 60 intro decoding/audio and quit during playback complete. No music takeover and zero measured output underruns. Startup overflow/drop counters are nonzero, as in the D18 baseline. |

The last two non-gameplay checks and `final60` preceded the final live speech
slider ownership correction; that correction is restricted to authenticated
Modernized gameplay speech states. The final candidate is additionally checked
in `gpu-release120lean`, with the new unit cases for live gain ownership.

The initial `candidate` route inherited a muted SPEECH/MASTER test profile and
is excluded from audible overlap claims. Later routes explicitly set both
sources to full native gain and MASTER to 75 percent. An early GPU fixture
used a large health write; the final GPU routes instead set the existing native
god flag and normal health to keep the repeated-voice route alive.

During the software-renderer check, host memory pressure was zero, about 22 GiB
RAM was available, and CPU/I/O pressure was low at the sampled instant. The
NVIDIA run uses the same output backend and has no measured underruns. These
observations distinguish the software/offscreen workload from normal hardware
rendering; they do not prove the cause of every historical crackle or close D18C.

Accepted executable SHA-256: `3e2a640dd7df6f3dde1fffd3b15b4e182687c829f913a9ea6dff533b28e6a288`.
Clean pinned framework patch application and byte comparison pass.

## Acceptance and retained evidence limits

User acceptance closes the listening/playback review. It does not establish
that every natural campaign trigger, loop/track transition or death/Continue
route was tested. Automated PCM/counter evidence is not a listening test. Loop
boundaries for all five channels are contract-tested; live coverage here is
tracks 0 and 1. The mixer reuses existing decoder fidelity, not a new hardware
accuracy claim. Music is mixed outside the emulated CD reverb bus.

Loads restore the original track at the native saved sector, with fresh
predictors. The new music cursor is not serialized sample-exactly; because
music now advances during speech, that native cursor can trail the music heard
when saving. There is no claim of sample-exact audio restoration. No save format
or player save/card content is changed by this implementation.

For future regression checks, use familiar voice triggers, repeated lines,
pause/resume, reload, death/Continue and independent MUSIC/SPEECH controls.
Music should continue under speech without an intentional duck, restart or
gap at the speech handoff. Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

`DNTTK_CONCURRENT_MUSIC=0` is a developer comparison only, not a saved setting.
`DNTTK_MUSIC_TRACE=1` logs track/cursor, overlap and limiter counters. The complete
framework delta is exported in the accepted-source patch and verified against
the pinned clean dependency. Closeout includes the authorized implementation,
tests and documentation commit/push. No next job started.
