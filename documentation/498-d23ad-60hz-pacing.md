# 498 - D23AD: 60 Hz screens keep 60 pictures (no 30-picture step, loads not judged)

Session of 2026-11-16, afternoon, with the user. User: "I feel like on the
field laptop, the performance is actually worse somehow" ... "both play
worse than this morning" ... "we have GOT to get this working on lower end
hardware comfortable. it has to work". Real-display use of the field laptop
and citadel granted ("you can use as much real display on the laptop and on
citadel as you want").

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Why the field laptop felt worse than in the morning (MEASURED)

- Host: on mains, performance profile, 47 C, no thermal throttle recorded,
  3.1 GHz, no update, backup or indexer activity; disk 83% full, healthy
  (SMART passed), I/O pressure low.
- Build: this morning's build (`081386c`) and today's (`2e10fa8`, D23N5 and
  D23Y10 off) were rebuilt side by side and run interleaved on the laptop's
  real 60 Hz screen at 2x and 120, five scenes, three runs each: pictures a
  second 30-60 for both, with no consistent difference (UI 2 55 / 55, the
  town 45 / 43, UI 8 46 / 53, UI 9 30 / 51, the FAMILY JEWELS start 55 /
  43). Not the build.
- Settings: the lab's read-only copy of the player's profile taken before
  the morning session (05:22) had 2x, LIGHTING ON, LIVE SHADOWS SIMPLE; the
  profile now had LIGHTING RELIT and LIVE SHADOWS SMOOTH (and 4x after a
  test). On the real screen, three runs each: the 05:22 settings 60
  pictures a second in the morning's level-3 save (60 60 60) and UI 2 (60
  60 55); the current settings at 2x 41-55, at 4x 30 (and the game itself
  slowed, lowest tenth 35 in UI 2). The morning's light atlas for level 3
  was also the smaller one (4096x1729 against 4096x2746 with RELIT).

## 2. What the pacer did (MEASURED, CODE)

At FRAME RATE 120 on a 60 Hz screen the redraws present at the display's
rate (D23Y6). When two judged seconds fall behind (under 57 game frames),
the governor steps to a redraw at every 2nd refresh: 30 pictures a second
(`last_div()` gives 2 on a 60 Hz timeline). It then needs 5 s x backoff
clean seconds to try every refresh again, and a probe that fails doubles
the backoff ("faster rate failed, backoff 8" in the player's logs). Only
after 3 more slow seconds at the last step does the original presentation
(60 game frames a second) take over.

Two defects in that on 60 Hz screens:

1. A stall was judged as a slow scene. Every session and lab run opened
   with a second of 2-22 game frames (startup, the title screen, a load
   before the load counter moves; the 3 s settle after a savestate load
   only starts when the counter moves), which shed the redraws to 30
   pictures for the first half minute or more. In the player's sessions:
   "game 13.7/s ... presents every 2 refreshes (emulation behind)".
2. The 30-picture step itself: on a 60 Hz screen it shows half the
   pictures of the original presentation, and the backoff kept it there.
   On citadel at the player's ULTRA 120 the game held 60 in every run
   (lowest tenth 59.1-59.6) while the pictures sat at 30 for most walks.

## 3. The change (CODE; on by default)

- `overclock_lease` (modern_controls.cpp): a second under 20 game frames is
  a stall, not judged (the heaviest views run 40-55).
- `last_div()` (frame_replay.cpp): on a timeline under 75 Hz the redraws'
  last step is every refresh, so a scene that cannot hold it goes to the
  original presentation (`floor_fallback`, retried after 2, 4, 8 minutes)
  instead of 30 pictures. Faster displays keep the 2nd/3rd-refresh steps.
  Developer `DNTTK_REPLAY_SKIP30=0` restores the 30-picture step.

Pictures are unchanged frame for frame (presentation pacing only); Vanilla
presents no redraws and is unaffected.

## 4. Measured on the real screens (MEASURED)

Interleaved, two runs a cell, the lab's private card copy, the player's
profile copied read-only, FRAME RATE 120, host checked before every run:
old = today's build before this change, fix = the stall fix alone, fix30 =
both.

Citadel (Ryzen 3 3200GE, Vega 8, 60 Hz TV, native Wayland, ULTRA 4x, 3
workers), pictures a second:

| Scene | old | fix | fix30 |
| --- | --- | --- | --- |
| UI 2 | 59.5 | 56.5 | 59.0 |
| DUKE HILL town | 36.5 | 35.0 | 59.6 |
| UI 8 | 59.3 | 45.2 | 59.4 |
| UI 9 | 54.2 | 30.5 | 58.6 |
| FAMILY JEWELS start | 43.5 | 50.3 | 58.5 |

Field laptop (60 Hz panel, one worker), pictures a second:

| Settings | Scene | old | fix | fix30 |
| --- | --- | --- | --- | --- |
| 2x, RELIT, SMOOTH | level 3 (the morning's save) | 60.0 | 60.0 | 60.0 |
| | UI 2 | 55.2 | 55.1 | 59.9 |
| | DUKE HILL town | 44.2 | 46.6 | 59.8 |
| 2x, ON, SIMPLE | level 3 | 60.0 | 60.0 | 60.0 |
| | UI 2 | 55.2 | 57.7 | 59.9 |
| | DUKE HILL town | 48.2 | 45.1 | 59.9 |

Game rate unchanged in every cell (median 59.7-60.0). In the town the
redraws gave way to the original presentation for part of the walk (redraw
presents 0 in those seconds), which is the intended fallback: 60 pictures,
full speed.

Field laptop at 4x with RELIT and SMOOTH (ULTRA's rows), old against fix30:
pictures a second level 3 30.6 / 56.3, UI 2 46.9 / 58.1, DUKE HILL town
40.9 / 50.8; but the game itself slows there either way (fix30: median
59.4 / 57.1 / 43.8, lowest tenth 33 / 35 / 24; old 59.7 / 55.1 / 41.6):
the redraws mostly stay off and the GPU's share of the 15 W package keeps
the CPU from 60 game frames (note 494). On this laptop 4x is beyond the
machine; 2x now holds 60. (The host check waited three times in this set,
for under 2 minutes in all.)

## 4b. Citadel at pure AUTO (MEASURED)

GRAPHICS AUTO (BALANCED from the probe taken on the TV, 2x) and FRAME RATE
AUTO, the real screen, two runs a scene, the D23AD build: 59.9-60 pictures a
second in all five scenes, game 60.0 / 59.3-59.6; ULTRA at 120 alternating
with it: 58.3-59.7.

## 4c. The user's session after the change (MEASURED, field laptop log)

2026-11-16 19:40-19:47, GRAPHICS AUTO (BALANCED: 2x, LIGHTING ON, LIVE
SHADOWS SIMPLE) and FRAME RATE AUTO: about 400 s of play, three savestate
loads, level 3 and level 1; no 30-picture step; hitch seconds 36 of 407
(9%), the game at 59.8 a second in their median; one audio underrun; one
hand-over to the original presentation and the redraws back after it;
presents a minute 58.8, 60.0, 15.6 (a pause or menu), 60.0, 50.3. User:
"ok now gameplay is looking significantly better!" Accepted; D23AD and
D23AE Done.

## 5. Also found

- Citadel's GRAPHICS AUTO on its TV (the GPU probe now taken on the real
  display, native Wayland): 5.46 ms on the Vega, AUTO resolution 2x,
  GRAPHICS BALANCED (offscreen the probe fails, note 496). Its TV runs
  1920x1080 at 60 Hz (it offers no 120 Hz mode) with KDE's scale 1.45;
  FRAME RATE AUTO there means the original presentation, while 120 gives
  60 redraws a second with the newest camera, which the user found
  smoother. The redraw worker count (1, 2 or 3) made no difference on the
  real screen at ULTRA (D23AC).
- D19Z4 (the light square behind the cursor on citadel): the game creates
  no cursor of its own (SDL's default system cursor; relative mouse mode
  while playing). The setup (KWin on Wayland, fractional scale 1.45, an AMD
  GPU, a fullscreen window) matches known KWin cursor-plane bugs (KDE bugs
  466749, 511061, 500602); to be confirmed by eye on the TV.
- D17BK (new): after a load the first ~15 game frames show 4:3, then 16:9
  (desktop shots at frames 8, 14, 20 after a load).
- D41W (new): lights arrive late after a load (RELIT's larger atlas).

## 7. D23AE: AUTO on 2-core CPUs with SMT (MEASURED, CODE)

D23W capped AUTO at the original 60 presentation on CPUs with 2 or fewer
physical cores. On the field laptop's real screen at GRAPHICS BALANCED,
interleaved, two runs a scene: FRAME RATE AUTO (the original
presentation) 60 pictures, game 60.0 / 59.3-59.6; 120 (the redraws at the
display's 60) 60 pictures, game 59.9-60.0 / 55.7-59.1. The pictures differ
in content: in busy views the game draws a new image every second field, so
the original presentation shows each image twice, the redraws 60 fresh ones
with the newest camera (what the user found "silky" on citadel). Change
(`replay_auto_cap_for`, replay_rate.h): 2 physical cores with 4 or more
logical CPUs take the 120 cap; 2 logical CPUs or fewer keep 60. Pure AUTO
after it (GRAPHICS AUTO = BALANCED, FRAME RATE AUTO), the real screen: "at
most 120 images a second (4 CPUs, 2 cores)", one worker, 60 pictures a
second in the morning's level-3 save, UI 2, 8, 9, the town and the FAMILY
JEWELS start, game median 59.9-60.0, lowest tenth 58.1-59.1; the town handed
over to the original presentation for part of each walk.

## 6. Reproduce

`recomp/analysis/d23ab/` on each machine (local): `realab.sh` (builds A/B),
`realset.sh` (settings), `fixab.sh` (env SETS, CFGSET, SLOTS, REPS), with
`xmr.sh` (the real-display variant of `xm.sh`; on citadel the KDE
session's environment from `~/ttk-ab/session.env` and `VIDEODRV=wayland`).
