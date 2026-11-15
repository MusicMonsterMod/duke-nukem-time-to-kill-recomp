# D17BH - Translucent horizontal bands: redraw presents never used vsync

2026-11-15. Job: [D17BH](../jobs/graphics.md#d17bh---translucent-horizontal-bands-over-the-bottom-of-the-screen).
Labs (local, untracked): `recomp/analysis/d17bh/` (`lab.py`, `run.sh`,
`gdb/`), `recomp/analysis/vanilla-regression/d17bh-vanilla*`.

## 1. Report

User, 2026-11-14 and 2026-11-15: two or more see-through horizontal stripes
over about the bottom quarter of the picture, now and then; they stay put
rather than following the camera; once gone on their own, back after a
restart and a load. Settings: Modernized, Polymorph, FRAME RATE AUTO (180 Hz
display), 4x, exclusive fullscreen, VSYNC ON. UI slot 1 (level 7 tunnel,
22:13 save) given as a good place.

## 2. D50G (the sleeping-enemy packet rewind) - ruled out as the explanation

- Every band report came from sessions before D50G (11-14); the only later
  session (2026-11-15 00:02) is a one-minute check of level 7.
- A/B on a private copy of the D50G spot (UI slot 6) with a temporary build
  of the pre-D50G code (180 Hz, offscreen, shots every 0.3 s): the
  overwritten packets show as wrong colours and textures on world polygons
  (lava turning flat green or flat orange), moving with the camera; not fixed
  translucent stripes. In UI slot 1's tunnel no stand-in is drawn at all
  (stand-in draw counter 0 over a 40 s walk with the pre-D50G build).
- So D50G cannot explain stripes that stay put; it is not excluded that a
  corrupt frame drew stripes once, but it does not fit the report.

## 3. What is in the picture - ruled out

Private copies of UI slot 1, offscreen on the GTX 1080 Ti at 180 Hz, the
player's profile (graphics rows as ULTRA, FRAME RATE 180 because AUTO sees no
display offscreen), composed presented frames (`present_shot`, after the
output pass, HUD and overlays):

- turning in place 40 s, 76 shots; walking, strafing, turning and three
  savestate loads, 108 shots: no stripes in any picture;
- redraw-only frames and copy-on-read snapshot tiles: `PSX_RT_VALIDATE=1`
  compared 1,357 render-thread redraws with the same jobs drawn on the
  emulation thread: 1,356 exact; the exception is the first compared redraw
  after the third load (sub-pixel edges and the weapon pose of a different
  moment, over the whole picture, no band);
- temporal frame blending (a blend could look see-through) is off in this
  build for every display (`g_frame_interpolation` is a config option).

Whatever made the stripes is not in the image the game presents.

## 4. Cause: the redraw presents swapped without vsync (CONFIRMED in code, MEASURED on the display)

- `present_replay_vsync()` (runtime `main.cpp`, D17 2026-10-02) returned swap
  interval 0 unless the developer variable `PSX_REPLAY_VSYNC=1` was set:
  "a blocking swap would stall the guest". So at 120/180 Hz the player's
  VSYNC setting did nothing; OPTIONS said so ("AT 60 FPS").
- The player's desktop is Cinnamon on a 1920x1080 180 Hz HDMI panel.
  Exclusive fullscreen is not composited, so an immediate swap lands part
  way down the scan: a tear line. The redraw pacer presents on a wall clock
  at exactly the panel's rate, so the tear lines stand almost still (the two
  clocks differ by parts per million) and drift away slowly; present jitter
  (p95 6.6 ms against 5.56) spreads them over a band of the screen; next to
  near-identical consecutive redraws they read as faint, see-through
  stripes. A restart gives a new phase. This matches every part of the
  report; D17 (section 12 of note 80) had listed "tearing with swap interval
  0 in exclusive fullscreen" as never checked on the real monitor.
- Tearing happens at scanout, so no capture of the presented image can show
  it; that is why section 3 found nothing. The real-display measurement below
  confirms the swap mode; the stripes themselves need the player's eyes.

## 5. Fix (framework, `recomp/patches/time-to-kill-accepted-source.patch`)

1. `present_replay_vsync()`: the player's choice (ON 1, OFF 0, ADAPTIVE -1,
   tracked in `s_replay_vsync_choice`; the old saved value used -1 for both
   "nothing saved" and ADAPTIVE) applies whenever the redraw rate resolves to
   no more than the panel's (+1 %): AUTO and DISPLAY always, a fixed rate up
   to the panel. Above it (240 on 180, UNLIMITED) the swap stays immediate,
   since vsync would cap the rate. `PSX_REPLAY_VSYNC=0|1|-1` still forces it.
   A live FRAME RATE change re-applies the swap interval.
2. Plain vsync was not enough (measured, 5): the wall-clock pacer and the
   panel's blanks are two clocks; with jitter two presents sometimes fell in
   one refresh, the second swap waited for the blank on the emulation thread,
   and the late-camera pacing dropped to every 2nd refresh. Now
   `gl_renderer_rp_tick` takes the blanks' phase from swaps that waited over
   1 ms (a quarter of the way to each one: those returns scatter by 1-2 ms)
   and puts each next present in the middle of the refresh after the one the
   last present fell in. At most one present per refresh, so swaps rarely
   wait; a late present costs its own refresh only.
3. Logs: `psxrecomp: swap interval N (...)` at each change and, while redraws
   present, a `[present swap]` line a minute in the session log (swaps, waits
   over 1 and 3 ms, longest, mean, re-anchors).
4. Text: OPTIONS VSYNC "ON: NO TEARING. OFF: LESS INPUT LAG"; run.py help,
   launcher line and GAME_MANUAL say it applies to every rate up to the
   display's.

A `GLX_SGI_video_sync` retrace counter was tried for the phase and dropped:
on this driver and context it advanced 4 times a minute.

## 6. Measurements on the real display (user permission 2026-11-15)

Exclusive fullscreen 1920x1080 at 180 Hz, the player's profile (ULTRA, AUTO,
VSYNC ON unless stated), private copy of UI slot 1, scripted turning and
walking 70 s, one game at a time, closed through the debug port; no
`nvidia-modeset` timeouts in the kernel log after any run.

| Build | Swap | Presents/s | Game fields/s | Swaps a minute waiting > 1 / > 3 ms | Mean swap | Present p99 |
| --- | --- | --- | --- | --- | --- | --- |
| before (`PSX_REPLAY_VSYNC=0`) | 0 | 179.8 | 59.9 | 33 / 12 | 0.161 ms | 7.2 ms |
| plain vsync | 1 | 96.2 (every 2nd refresh) | 56.7-58 in places | 514 / 481 | 0.583 ms | - |
| re-anchor after each wait | 1 | 177.2 | 59.9 | 149 / 13 | 0.197 ms | 8.8 ms; 5-7 late presents every second |
| final (mid-refresh placement) | 1 | 178.0 | 59.9-60.0 | 87-130 / 11-21 | 0.185-0.194 ms | 8.8 ms; 3-5 late presents in 12-13 seconds of 70 |
| final, VSYNC OFF profile | 0 | - | - | - | - | - |
| final, borderless 4x | 1 | 177.8 | 60 | 265 / 25 | 0.229 ms | 8.6 ms; 9 seconds of 70 with late presents |
| final, window (auto scale, composited) | 1 | 179.5 | 60 | 18 / 14 | 0.074 ms | 6.6 ms; no late seconds |

The cost of tear-free presents at 180: about 1-2 % of presents show one
refresh later (a repeated refresh, 8-12 ms interval) and a few short swap
waits; the game rate is unchanged. VSYNC OFF keeps the old behaviour.

## 7. Vanilla and other checks

- Vanilla never selects redraw presentation; its swap interval path is
  unchanged. Vanilla route (`vanilla_regression.py`, offscreen): new against
  the unmodified build differs only as much as two runs of the new build
  differ from each other (the route is wall-clock driven; title, menus,
  intro and difficulty identical).
- Local tests: 259 run, 1 failure, `test_primitive_trace` (pre-existing since
  2026-11-14).

## 8. Open

- The player's eyes on the real display: stripes gone with VSYNC ON in
  normal play and after loads. A/B for one launch:
  `PSX_REPLAY_VSYNC=0 python3 recomp/tools/local/run.py` (the old swap).
- Whether the 1-2 % repeated refreshes are visible in turns.
- Windowed and borderless with VSYNC ON now also wait for the refresh;
  measured above (a window: no cost; borderless: as exclusive).
