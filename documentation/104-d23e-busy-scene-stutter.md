# D23E - Busy-scene stutter in the western town (UI slots 9 and 10)

Status: **Needs playtest** (2026-10-05). Candidate binary
`18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`.

## Report

UI slot 9: look down toward the enemies on the ground and walk left and right;
the game stutters. UI slot 10 (with `dnkroz`): walking up and down the street
stutters too. Both savestates are in the western town (cowboy costume): slot 9
faces a log building with chickens on the ground and a rider to the left, slot
10 looks down the main street.

## Method

`recomp/analysis/d23e-20261005/h.py` (local, not committed): real GPU through
SDL offscreen + NVIDIA EGL, a profile copied from the player's (Modernized,
first person, 120 Hz, 4x, 16:9, corrected precision, `cpu_overclock 100`),
private copies of the savestates, private port 9263, silent audio device.
God mode is the `dnkroz` flag (`0x800c3cc6`) written in the private process.
Routes: slot 9 looks down 60 counts and strafes A/D in 1.5 s halves; slot 10
walks W/S in 3 s halves. Measures:

- logic frame length in display fields, from `DNTTK_FRAME_TRACE=1
  DNTTK_FRAME_TRACE_MS=0` (wall time between player updates; 3 fields = 20
  game frames per second, 4 = 15);
- host fields per second and the `[TTK cpu]` safety-net lines;
- `phase_profile`, `render_replay` (redraw counts, `feed_ms`, `pace_div`) and
  `audio_stats`;
- `PSX_PROF` + `PSX_PROF_CALLERS=1` for the emulation thread, symbolized with
  `d17-high-refresh/symprof.py`.

## Findings

Game frames per second come from the emulated PlayStation CPU. When its work
for one frame does not fit, the frame takes another field.

Slot 9, logic frame length in fields (20 s strafes):

| Setting | Fields per game frame | Notes |
| --- | --- | --- |
| Vanilla (4:3, third person) | 3 (163), 4 (15) | steady 20 |
| Modernized 100%, 16:9, 120 Hz (player) | 4 (267), 3 (15), 5-6 (19) | 15 with steps |
| same, widescreen off | 3 (253), 4 (112) | |
| same, draw distance original | as 16:9 | no effect |
| same, 60 Hz | as 16:9 | no effect at 100% |
| Modernized 150%, 120 Hz | 4 (209), 3 (76), 5-8 (28) | 3 overclock pauses |
| Modernized 150%, 60 Hz | 3 (399), 2 (3), 5-6 (2) | no pauses |

Slot 10 (24 s walks): Vanilla 4-5 (the original game is slow here too);
Modernized 100% 4 with 5-7 steps; 150% at 60 Hz steady 3; 150% at 120 Hz
3 and 4 with an overclock pause.

Two causes:

1. **The emulated CPU is over budget.** The 16:9 view draws real extra rooms,
   props and characters (D14 widens the portal root rectangle). At the
   original CPU speed the frame no longer fits in 3 fields and lands on 4
   with occasional 5-6. That is the player's setting (`cpu_overclock 100`; the
   default is 150). Draw distance does not matter.
2. **At 120 Hz the 150% overclock could not hold.** Late-camera redraws run on
   the emulation thread at every present: 120 per second at about 1.4 ms
   (`feed_ms`), about 190 ms of every second (19%). The game at 150% needs
   about 83% of the thread in this scene (`static_share` at 60 Hz). Together
   the thread fell behind real time (55-57 fields per second), and the D08Y
   safety net then paused the overclock for 5 s at a time: the game swung
   between 20 and 15 frames per second. The 60/120 Hz profile difference is
   almost all GL driver and batch-flush work called from the redraw feed.

The recompiled game code itself is about 3% of the emulation thread; most of
the rest is the cycle-timing model (`psx_cyc_*`, about 25%), interrupt checks
and dispatch. Speeding those up is framework work outside this job.

Not related to D23C/D23D/D17H by evidence: those are other scenes. D23C/D23D
should be measured the same way; the mechanism (busy view plus 120 Hz redraws
pushing the overclock out) may well apply there too.

## Change

`frame_replay.cpp` `replay_shed_load()`: when emulation falls behind real time
the redraws give way first. Presents go to every 2nd, then 3rd refresh, evenly
(the existing late-camera pacing divisor), and step back up with the existing
rule: 4 clean windows of 90 presents, doubled whenever the faster rate falls
behind again within 3 windows, at most 32. A one-off hitch (render workers
starting after a load) costs a few seconds; a busy scene settles on one rate.
`modern_controls.cpp` `overclock_lease()` calls it before pausing; the lease
pauses only when there is nothing left to shed (60 Hz, 30 Hz, interpolation
off, or already at every 3rd refresh). Debug JSON: `plugin.emu_sheds`. Log:
`[TTK cpu] emulation behind real time (...): high-refresh presents reduced`.
Vanilla is unchanged (the lease and redraws are Modernized only).

## Results (150%, 120 Hz, candidate)

| Run | Fields per game frame | Overclock pauses | Present rate |
| --- | --- | --- | --- |
| Slot 9, 40 s | 3 (764), 2 (40), 4-6 (13) | 0 | every 3rd refresh |
| Slot 9, 60 s | 3 (915), 2 (346), 4-6 (49) | 1 | 2 for 9 s, then 3 |
| Slot 9, 60 s repeat | 3 (993), 2 (146), 4-6 (87) | 1 | 3 |
| Slot 10, 90 s | 3 (1767), 4-6 (38) | 0 | 3 |
| Slot 10, 60 s | 3 (1187), 4-6 (23) | 0 | 3 |
| Slot 1 and 5 (light), 30 s | 2 (about 900) | 0 | every refresh |

Before: slot 9 at the same settings was about 30% 2-3 field frames and 70%
4 or more; after, about 93% are 2-3. One light-scene run shed once right
after the savestate load (render workers starting); a repeat did not.

The player's 100% setting stays mostly at 4 fields in slot 9 (the emulated
CPU itself is over budget); one shed happened there too.

Native tests: `ttk-input-test`, `ttk-aim-test disc/SLUS_005.83`,
`ttk-near-test disc/SLUS_005.83`, `ttk-controls-test disc/SLUS_005.83
analysis/pc-input/d06-acceptance/level00-guard-fixture.bin
analysis/d22b-20261004/levels/LEVEL01.OVR analysis/d22b-20261004/levels`
(34 groups) PASS. The controls test gained a `replay_shed_load` stub.

## Limits

- Audio was measured on the silent dummy device (no underruns in the long
  runs; one 60 s run reported underruns on that device that a repeat did
  not). Real audio needs the playtest.
- In the heaviest moments even every 3rd refresh plus 150% can fall behind;
  then the old 5 s pause still happens (about once a minute in slot 9).
- At 40 presents per second turning is less smooth than at 120 in these
  scenes; the user judges the trade on the real display.
- Offscreen runs, not the player's window and compositor.

## Follow-up: is 150% a good default? (2026-10-05)

The user confirmed 150% relieves the stutter but keeps 100% for now: 150% felt
"different, not smoother", seams looked more visible, and audio seemed
slightly degraded right at the start. Checks (`seams.py`, `motion.py`):

- Standing still, 300 fields per slot: whole/subdivided wall counts equal at
  100% and 150% (slot 9 118.8 / 119.0 whole per frame, 0-0.2 kept; slot 10
  207; slot 1 65). Screenshots are pixel-identical in slots 10, 1 and 5; slot
  9 differs only where things move (chickens, rider, an enemy).
- Walking and turning 20 s at 120 Hz:

| Slot | OC | Presents/s | Game frames/s | Pacing |
| --- | --- | --- | --- | --- |
| 1 (light) | 100 | 121 | 26.3 | every refresh |
| 1 (light) | 150 | 75 | 29.8 | every 1st-2nd |
| 9 (town) | 100 | 120 | 18.5 | every refresh |
| 9 (town) | 150 | 54 | 26.2 | every 2nd-3rd |

So the feel at 150% comes from the picture rate: one emulation thread cannot
run the faster game and 120 redraws per second on this PC, even in light
scenes. Motion at 40-60 pictures per second also shows texture shimmer more.
The emulation-thread profile while moving at 150% (slot 1): cycle-timing model
about 34%, interrupt checks 4%, game code 4%, GL/redraw paths 12%, always-on
observers 4-5%. Making room is job D23F (faster timing model). The start-up
audio report (buffer overshoot and rate correction at the limit for about 2 s
after gameplay starts, seen on the dummy device) is unconfirmed on real audio.
