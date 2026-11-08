# D50 - Frame rate halves after loading (audio running low at load)

Status: Needs playtest (2026-11-08; the energy-weapon stalls found and fixed, section "Root cause").

## Report

During the D41B playtest the user felt a slowdown (and "maybe a touch of
audio crackle") while firing the energy weapon in their UI slot 8. The
session logs of 2026-11-08 showed the cause was not the weapon: in four of
five sessions (09:44, 10:42, 11:14, 11:26) the governor logged
`audio running low (38-59 of 180 ms)` a few seconds after the savestate
load and shed high-refresh presents to every 2nd refresh (90 presents at
180 Hz). In the 11:26 session it stayed there about 2.5 minutes: a pause
menu (Escape) held it, then it stepped back up within a minute of play.
Every per-minute line reported 0 windows behind real time.

Host: Chrome was busy and a Flatpak system helper used 10.5 s of CPU and
stopped at 11:30; possible contributors, not established. The portal's
"Realtime error" appears at every game start and is not related.

## Cause

`overclock_lease` (`recomp/src/ttk/modern_controls.cpp`, D17AQ) shed a
present step as soon as the audio ring fell under a third of its 180 ms
target, without asking whether it was falling. A ring that starts low after
a load stays low while the game keeps real time: the output's rate
correction (`recomp_audio_drc.h`, at most 0.5%) refills it by about 5 ms a
second, and fewer presents cannot refill it at all. So the shed was a false
alarm. The step back up then needed 30 clean gameplay seconds
(`replay_load_window`, `frame_replay.cpp`), and every menu or pause second
reset the count.

Offscreen runs (dummy audio), real audio into a silent PulseAudio null sink,
cold-start loads (`PSX_LOAD_SLOT=7`) and two real-window runs on a private
copy of slot 8 never started low, so the low start itself was not
reproduced; it depends on the real output device's startup.

## Change

- Audio shed only on a drain: the ring under a third of its target and at
  least 30 ms lower than 0.4-1.5 s before (a game short of real time drains
  100+ ms a second), or an underrun while low. A low, steady ring logs
  `[TTK cpu] audio ring low but steady (...): no present shed` (at most 5
  times) and sheds nothing.
- A shed for the audio ring steps back up after 5 clean seconds (as a
  lighter view does), not 30; if the scene still cannot keep up, the ring
  drains again, it sheds again and the failed probe doubles the backoff.
- Seconds that are not judged (menus, pauses, loads) no longer reset the
  clean run.

No framework change; Modernized only (the governor runs only there).

## Evidence

Lab `recomp/analysis/d41b/lab.py` (`audioboot`, `audiostress`; local),
private slot 8 copy, 180 Hz, internal scale 3, real audio into the null sink
`ttklab`:

- Cold start with the new build: ring at 189-246 ms, no shed, no notes.
- CPU squeeze (systemd scope `CPUQuota=40%` for 3 s): the ring drains to
  0-23 ms and the governor sheds to every 2nd, then 3rd refresh (correct).
  Released: the ring sits at 28-45 ms and rises slowly; it logs "low but
  steady" and sheds no further. Back to every refresh about 9 s after the
  release (before: about 27 s, and minutes with a pause menu open).
- Tests: `ttk-near-test` PASS, `ttk-flash-test`, `ttk-live-env-test` PASS,
  profiles OK; `ttk-controls-test` stops at the known D14 HUD check (line
  1755).

## Limits

- Why the ring started low on the player's machine is not established
  (real device startup, or a stall right after the load). A low ring still
  plays with less margin; if crackle remains, that start is the next thing
  to look at.
- Needs the user's playtest in slot 8: load, play and fire the energy
  weapon; the session log should show no `audio running low` shed at load.

## Follow-up: still felt in slot 8 (2026-11-08)

User, after the change: "its still happening! it's very specifically
triggered when you fire the energy weapon in that corner", "for all i know
this root may run far deeper", and: fire "all around, left to right up and
down ... at human speed". The 12:00 session log showed no shed at all (the
change held), so what remains is a cost the governor does not see.

Not reproduced in the lab (private slot 8 copy, first person as saved,
energy weapon, 180 Hz, scale 3, offscreen; `corner`, `SWEEP=1`):
- stationary fire at the far building corner, 12 s: the game 60-61 frames/s,
  182 presents/s, per-update hook cost the same as idle (frame trace);
- fire while sweeping left/right and up/down, 20 s: the game 60-62,
  presents 182-186.

Hitch log added (`[TTK hitch]`, `hitch_second` in `modern_controls.cpp`,
Modernized only): for each gameplay second that goes wrong (the game under
58 frames/s, 3 or more presents at 1.4x the second's mean interval or more,
a missed redraw, an audio underrun, or the ring falling 40 ms or more) one
session-log line with the game rate, presents, late presents by bucket,
audio ring start/end/minimum, underruns, weapon, light count, view and
Duke's position. Healthy seconds log nothing (one line in a 42 s healthy
sweep: 6 presents at 8-12 ms); at most 300 lines a session. Checked with the
CPU squeeze: three lines for the three bad seconds. The next reproduction in
the player's game will show which of these went wrong.

## Root cause: the energy beam starves the redraws (2026-11-08)

User: "do whatever it takes. WHATEVER it takes".

Player profile (12:17 session, `PSX_PROF_WALL=1 PSX_PROF_STACKS`, analysers
`recomp/analysis/d41b/stalls.py`, `gaps.py`, `lift.py`; local): the stalls
are 60-100 ms stretches without a present in which the emulation thread
draws the original display lists live (`gpu_write_gp0`, `dma_advance`,
`glb_draw_shaded_textured_triangle` about 3x their normal share). That is
redraw-only switched off (D17AR3): after 3 presents without a worker redraw
the live frames draw again, on the emulation thread.

Why the redraws stop while the beam fires (lab, private slot 8 copy, 180 Hz,
scale 3, offscreen; `corner` with `SWEEP=1`, `PLUG=1`, `PSX_GL_FLUSH_DIAG=1`):

1. Misses classified per present (`[TTK redraw-only] misses ...`): almost all
   are redraws finished by a worker but still queued on the render thread
   (D17AW); 16 worker job slots also ran out (fixed: `RW_JOBS` 32).
2. Only the energy weapon does it: render thread busy 23 s against 12-13 s
   for the flamethrower or Gatling over the same sweep.
3. Render-thread profile (`PSX_PROF_RT=1`, new): 66-74 % of its time is in
   the NVIDIA driver. The beam interleaves untextured and textured
   primitives; each switch flushes the other batch (painter order), so the
   textured and flat batch flushes rise from about 1000 to up to 7800 a
   second.
4. In native widescreen every batch that reaches the margins is drawn twice,
   into the 4:3 frame and the wide surface, with two framebuffer switches.
   Ablation (`gl_ws_ablate`): mirror draws without the switch cost about the
   same as no mirror at all; the switches are the cost (render thread
   16.6 s against 12.2 s, repeats 377 against 35, timer queries off as in
   the player's build). With widescreen off: 0 switch-offs.

## Fix

- Framework (`gpu_gl_renderer.c`, patch exported and verified on a pristine
  `08ec704a` checkout, 78 files equal): the mirror pass of a painter-order
  flush is logged (vertices and state) and replayed in order with one
  framebuffer switch before anything else uses the wide surface or the
  textures the mirror draws read: every other batch flush (uploads, packs,
  state keys, lights, presents, replay begin/end, render-thread snapshot)
  and every wide-surface entry point (`glb_wide_*`, `wide_blit_center`, fill,
  copy, the full-width overlay rect). Default on; `PSX_GL_MIRROR_DEFER=0`
  draws each mirror immediately. `PSX_REPLAY_VALIDATE_MIRROR=1` draws each
  redraw again with the immediate mirror and compares the images.
- Framework: `RW_JOBS` / `RT_JOBS` 16 to 32 (worker job slots).
- Framework (developer): `PSX_PROF_RT=1` profiles render thread 0;
  `PSX_GL_FLUSH_DIAG=1` prints batch and upload flushes and mirror log
  counts per renderer every 2 s. Render-copy name headers regenerated.
- TTK (`frame_replay.cpp`): redraw-only turns live drawing back on only
  after 3 presents and 250 ms without a redraw (the workers' stuck-job
  deadline), not after 3 presents; each switch-off is logged with its
  reason and miss counts. The hitch line counts switch-offs.

## Evidence

- Exact images: `PSX_REPLAY_VALIDATE_MIRROR=1` on the single-thread path:
  slot 8 copy first person 2560 redraws, third person 2560, level 0 street
  third and first person with Gatling, RPG, flamethrower and energy weapon
  6336: 0 differences.
- Energy weapon sweep (slot 8 copy, 180 Hz, scale 3, timer queries off):
  before 4 switch-offs and 370 repeated presents in 20 s of firing; after 0
  and 5. Render thread busy 16.4 s against 13.6 s.
- Tests: `ttk-near-test` PASS, `ttk-flash-test`, `ttk-live-env-test` PASS,
  profiles OK; `ttk-controls-test` (stubs added for the hitch log) stops at
  the known D14 HUD check (line 1758).

## Limits

- Offscreen lab only: the player's real window, audio device and 180 Hz
  display remain to be confirmed in play.
- The beam still costs the render thread more than other weapons (the
  painter-order batch splits remain; only their framebuffer switches are
  gone). Merging untextured primitives into the textured batch would remove
  the splits (renderer work, not done).

## Closeout

User-confirmed in slot 8 ("it is now fixed!!!!") and closed 2026-11-08.
The limits above stay as known follow-ups.
