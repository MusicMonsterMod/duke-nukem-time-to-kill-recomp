# D08A10 - Steroids heartbeat (experimental)

Status: **Done** (user-accepted 2026-10-08: "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!"). Earlier: Needs playtest (2026-10-08; sound changed to the Shift click after the user's first test: "the beat is right but the sound is wrong"). Experimental and easy to revert: one
file, `recomp/src/ttk/steroids_beat.inc`, and three hook-ups. Modernized with
`steroids` = `portable` only; `original` and Vanilla unchanged.

User request: "In duke3d, when taking the steroids, a sound plays of duke
swallowing the pills over and over for the duration of the steroid use. the
rhythm is 226bpm. ... we could use the sound that duke makes in ttk when you
hit shift ... unless of course there is an actual heartbeat sound available".

## Duke 3D reference

`research/screencaps/Video_2026-10-07_23-59-03.mp4` (local): the repeating
sound runs from 2.7 s to 16.2 s (about 13.5 s, Duke 3D's 400-tic steroids).
Peak picking over that span: 50 beats, median interval 268 ms, mean 268.2 ms
(224 bpm). This matches Duke 3D's `DUKE_HARTBEAT` every 8 game tics at 30 Hz
(266.7 ms, 225 bpm), so the target is 225 bpm, close to the user's 226.

## TTK sounds

The sound call `0x8006b73c(id, position, volume, pitch)`: `id` is
`bank << 12 | index` (`0x8006b7c8`: bank `(id >> 12) & 0xff`, index
`id & 0x7ff`); `a3` is a small pitch offset (the footstep call passes
`(rand & 7) - 4`).

- **Shift sound (used):** pressing or releasing Shift in Modernized starts SPU
  sample `0x012F0` at pitch `0x228`-`0x22F`, standing or walking. It is sound
  `0x0001` (bank 0), the walk/run toggle click: the player update flips
  `player+0x224` bit 1 and calls the non-positional sound call
  `0x8006bbd8(1)` (`0x800418cc`, `0x80041934`, `0x80041994`, `0x800419c8`).
  It does not go through `0x8006b73c`, which is why a log of that call alone
  missed it.
- **Footstep (first choice, rejected by the user):** walking plays `0x2000` /
  `0x2001` (random) from the gait plant at `0x80048378` (ra `0x80048380`),
  volume `0x80`, random pitch.
- **Survey:** a new debug `sfx <id>` plays any sound at Duke. Each sound's SPU
  KEYON (sample address, pitch) was caught and its ADPCM sample decoded from
  SPU RAM (clean, no ambience). Bank 1 is `0x1000`-`0x101d` (30 sounds);
  banks 2-6 were surveyed to index 23 (many indices only replay a level
  ambience). There is no named heartbeat; ranked by low-frequency content (bank 0 below):

| ID | Length | Below 250 Hz | Envelope peaks | Notes |
| --- | --- | --- | --- | --- |
| `0x1012` | 313 ms | 95% | 20, 110, 275 ms | Low double thump; played by the original at `0x8006f3c8`. First default; the user wanted the Shift sound |
| `0x1010` | 989 ms | 76% | many | Too long |
| `0x1000` | 230 ms | 29% | 45, 95 ms | Short double hit, brighter |
| `0x101c` | 260 ms | 18% | 30 ms | Single hit |
| `0x2000` / `0x2001` | 305 / 332 ms | 23% / 12% | two | The Shift-run footstep |

Bank 0 holds `0x0000`-`0x0004` (samples `0x1010`, `0x12F0`, `0x1620`,
`0x1D60`, `0x2190`); `0x0001` is the Shift click.

Bank 1 is Duke's own: `0x1012` is resident at SPU `0xE900` in level 0 and
`0xEDD0` in level 6, byte-identical.

Previews at 225 bpm (local): `recomp/analysis/d08a10-beat/preview/`
(`beat-shift.wav`, the chosen Shift click at its game pitch; `beat-01012.wav`, `beat-02000+02001.wav`, `beat-01000.wav`, `beat-0101c.wav`,
and `duke3d-reference.wav`), for `paplay`.

## Implementation

- **Clock:** the drain `0x800414a0` takes the frame step `0x800d21fc` from
  `player+0x366`; measured 15 per game update at 20 updates/s in level 0 =
  300 a second. A beat falls each time the amount, counted from where the
  dose started, crosses a multiple of 80 (80/300 s = 266.7 ms). The first beat
  is at once (Duke 3D). A refresh (amount rises) restarts the count.
- **Where:** the authenticated player update (`0x8005a210` from `0x80041b34`,
  existing hook, the same place as `spawn`), which runs in ground, swim,
  jetpack and ladder states and never in render replays. No new hooks, no
  codegen change; savestates load.
- **When:** `portable` steroids, `+0x364` bit 1 on with an amount (a used dose,
  a pickup while running, `dnhyper`), Duke alive (`+0x32` > 0), identity
  ready. A savestate load resyncs (no burst). The original ends it at the
  timer, a damage cut (`0x800a4154`, -1500: one beat, not a burst) and the
  level reset.
- **Sound:** `0x8006bbd8(0x0001)`, the walk/run toggle's own call, on a
  private stack, so it sounds exactly like pressing Shift (same sample, same
  pitch spread). `DNTTK_STEROID_BEAT=off` silences it,
  `DNTTK_STEROID_BEAT=<id>` plays another sound through the same call.
- **Debug:** `ttk_input` -> `controls.steroid_beat` (`sound`, `beats`,
  `skips`, `refused`, `bucket`, and `log`: the last 32 sound calls as
  `[id, ra, host frame]`, recorded at the existing `0x8006b73c` entry hook).
  Console `sfx <id>`.

## Evidence (executable `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`)

Private Xvfb runs, private profile and a copy of the test cards
(`recomp/analysis/d08a10-beat/`, local).

| Check | Result |
| --- | --- |
| Shift click, slot 4, 60 fps | 26 beats in 6 s; each KEYON is sample `0x012F0` at pitch `0x228`-`0x22F`, the same as a real Shift press; intervals 15, 15, 18 |
| Shift click, level 6 | 14 beats in 3 s, same sample |
| Timing (first build, `0x1012`), slot 4, 60 fps | 34 beats in 8 s; KEYON intervals 15, 15, 18 fields repeating (250, 250, 300 ms; mean 266.7 ms = 225 bpm) |
| Timing, 120 fps | identical intervals and count |
| Pickup (spawned), R (first build) | 12 beats in 3 s, each a KEYON of the `0x1012` sample; pickup sound and footsteps still play |
| Savestate save, then load mid-run | resumes on the loaded amount; intervals 18, 15, 17, 14, 18, 15, 15, 18 (no burst) |
| Damage cuts (slot 3, under attack) | beats continue, `skips` counted, stop with the effect |
| `level 6` travel | stops (original reset clears steroids) |
| Level 6, `dnhyper` | 18 beats in 4 s from the same sample at its level-6 address |
| `original`, `DNTTK_STEROID_BEAT=off` | no beats |
| SPU voices | the beat takes one voice at a time (0, 4, 9 seen) |
| Suites | `ttk-controls-test` (PS-X EXE, LEVEL00 fixture, LEVEL01, all levels), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK (2 skipped), `level_overlay_guards.py --check`, `check_repo.py` |

## Limits

- **Even rhythm:** the game updates at 20 Hz in busy scenes, so beats can only
  fall on 50 ms steps: 250, 250, 300 ms (225 bpm on average, slightly
  uneven). In scenes running at 30 Hz they are exactly 266.7 ms. A smoother
  beat would need a sound start outside the game update (not attempted).
- Death while steroids run was not reached in scripted runs (the beat is gated
  on health > 0 by code). Vanilla is gated by code path (`portable_steroids()`
  requires Modernized; the player-update hook returns for Vanilla).
- The `sfx` survey of banks 2-6 stopped at index 23; level-specific banks were
  not mapped by level.
- Whether the Shift click works as a heartbeat is the user's call.
