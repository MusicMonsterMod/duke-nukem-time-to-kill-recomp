# 462 - D23X: input latency on the field laptop, and VSYNC

The user, playing their settings on the field laptop (note 461): "it plays
well, but there feels to be a slight latency with it. i cant quite explain.
am i imagining this?" They are not. Measured below, with the cause, what was
changed and what remains (D23Y).

Evidence labels: MEASURED (this laptop, this session), CODE (read in the
source), INFERRED.

## 1. Method

A camera-free probe (`recomp/analysis/d23x/latprobe.c`, local lab): it sends
a relative mouse motion through XTest (300 counts, a 0.4 radian turn at the
user's sensitivity) and polls a 256 x 96 strip at the top third of the game
window with XShm until a twentieth of its pixels change, timing from just
before the event to that grab. It measures input to the composited picture
readable from the X server; the panel's own scanout is not included and is
the same for every game. 30-40 trials a run, turning back and forth, half a
second apart. Grabs cost 0.2-0.4 ms (2-13 ms for programs that change the
display mode, which makes their figures coarser).

Our game: the lab (`recomp/analysis/d23x/lab.py`) launches `run.py` with the
player's profile copied read-only, private card copies, real input (no test
script) and the real display, loads UI slot 2 and moves the window to a known
place. Developer traces (`PSX_LAT_TRACE=1` in the runtime, `DNTTK_LAT_TRACE=1`
in the title) stamp, on SDL's performance clock: the vblank's input sample,
each swap, each display flip the game makes (GP1(05h), with its guest cycle
since vblank), the mouse event's arrival, the look's use by the camera, the
view matrix, the camera the view composition reads, and Duke's heading.

Hardware as note 461 (i7-5600U, HD Graphics 5500, 1920x1080 60 Hz, XFCE with
its compositor).

## 2. The numbers (MEASURED)

Median input to picture, UI slot 2:

| Program | Setting | Median | Lowest tenth |
| --- | --- | --- | --- |
| EDuke32 (Polymost, the user's 1600x900 fullscreen) | - | 22 ms | 16 |
| Quake Brutalist Jam III (its own build, fullscreen) | - | 35 ms (coarse) | 27 |
| UZDoom 4.14 + Ancient Aliens (the user's settings, 60 cap, vsync off) | - | 40 ms | 35 |
| Quake Alkaline (QSS-M, fullscreen) | - | 52 ms (coarse) | 33 |
| Ours, original 60, windowed | vsync on (before) | 129-135 ms | 114-121 |
| Ours, original 60, windowed | vsync off | 129-131 ms | 120-122 |
| Ours, original 60, borderless / exclusive | vsync on | 130-135 ms | 119-121 |
| Ours, 60 with late-camera redraws (DISPLAY, forced past the core cap) | - | 66 ms | 49 |
| Ours, 120 (late-camera redraws) | windowed / borderless | 74 / 64 ms | 60 / 52 |
| Ours, original 60, present at the flip (this job) | vsync on | 126-129 ms | 113-121 |
| Ours, original 60, present at the flip (this job) | vsync off | 113-119 ms | 98-102 |

Hexen II was not measured: it starts through Steam here, and the other four
already bracket the result.

So the latency the user feels is real: three to six times the reference ports.
Vsync, the compositor and fullscreen change it by a few milliseconds; the late
camera halves it.

## 3. Where the 130 ms goes (MEASURED, CODE)

One trial of the trace, times after the mouse event:

| ms | Event |
| --- | --- |
| 4-20 | the event reaches the game at the next vblank's event pump (once a field) |
| 27-50 | the look is applied (`orbit_begin`) at the next game logic frame (the game runs 30 a second), the view matrix and the view composition read it within 2 ms |
| +11 | the frame's flip: it shows the previous frame's image and starts drawing this one |
| +33 | the next flip shows the image drawn before it |
| +33 | the image with the new view is flipped (the third flip after the look) |
| +4-15 | our present, then the compositor |

The game itself is the latency: a 30 Hz logic frame and two more frames of
buffering between the frame that reads the look and the flip that shows it.
The same with the Classic renderer (126 ms), so it is the original game's
pipeline (the replay code records it too: "a frame published with flip count c
is on screen from flip c+lag (its image is drawn after its own flip and
displayed at the next one)", `frame_replay.cpp`). A console on a CRT has the
same three frames (INFERRED from the code path; not measured on hardware).

Two things on our side added time:

1. The present ran at the start of vertical blank, before the game's flip.
   Time to Kill flips about 19,900 guest cycles after vblank (median of 355
   flips; at most 23,423), inside the console's vertical blank (23 of 263
   NTSC lines, about 49,000 cycles), so the console scans the new buffer out
   in the next field. We showed it one field (16.7 ms) later.
2. Duke's heading (the body and movement) is written one game frame after the
   camera takes the look (`face_view` in the player update, which runs before
   the camera update). This does not delay the picture, only the body's
   facing.

## 4. Built

- **Present at the flip** (framework `main.cpp`, `gpu.c`; title
  `frame_replay.cpp`, Modernized only): the vblank present samples input, pumps
  events and paces as before, then leaves its display half due; the next
  device-service edge after the game changes the display area, or after the
  end of vertical blank (23/263 of the field), shows it (`sdl_present_display`,
  split out of `sdl_vblank_present_body`, and the service tick that already
  serves replay presents). Off for netplay, self-check, movies (24-bit
  display) and replay presentation. `psx_mod_set_flip_present(1)` at
  activation; `DNTTK_FLIP_PRESENT=0` or `PSX_FLIP_PRESENT=0` turns it off
  (developer). Vanilla never sets it.
  Measured: with vsync on, 126-129 ms against 129-135 (the swap still waits
  for the same real vblank, since the emulation's vblank follows the panel's);
  with vsync off, 113-119 ms against 129-131.
- **VSYNC** in OPTIONS > DISPLAY (ON, OFF, ADAPTIVE), schema 74 `vsync`
  (default on), `run.py --vsync`, launched as `PSX_VSYNC` (1, 0, -1) and
  changed live through the new `psx_mod_request_vsync` (applied on the next
  present, restored when replay presentation stops). It is the original
  presentation's swap interval; above 60 the redraws pace themselves without
  driver vsync, as before. Off is about 10-15 ms quicker with present at the
  flip. In a window the desktop compositor keeps the picture tear-free; in
  borderless or exclusive fullscreen it can tear. Adaptive is the driver's
  late-swap tear (GLX_EXT_swap_control_tear here), otherwise on.
- Developer traces `PSX_LAT_TRACE=1` (runtime: input, swap, flip with its
  cycle) and `DNTTK_LAT_TRACE=1` (title: mouse event, look, view matrix,
  composition camera, heading). Silent unless set.

## 5. What remains

The original 60 presentation stays at about 115-130 ms because the game's
logic and buffering are three 30 Hz frames; presentation tricks cannot remove
that without changing the game. The late camera can: every redraw is drawn with
the newest look, so a turn shows at the next present (66 ms at 60, 64-74 ms at
120, with the rest the redraw pipeline's own lead). On this laptop the redraws
slow the game itself (D23Y), so making them affordable is the latency fix too;
D23Y's plan (note 463) carries the target: the late camera at every present on
2 cores, input to picture under 60 ms.

Not done: moving Duke's heading into the same frame as the camera (body only,
no visible latency); a pad-path measurement (the probe turns with the mouse).

## 6. Reproduce

```
gcc -O2 -o recomp/analysis/d23x/latprobe recomp/analysis/d23x/latprobe.c -lXtst -lXext -lX11
cd recomp/analysis/d23x && TAG=t SLOT=1 RATE=60 PROBE="-n 30 -frac 0.05 -delta 24" python3 lab.py
```

`ENV='{"PSX_LAT_TRACE":"1","DNTTK_LAT_TRACE":"1"}'` with `-trace` adds the
timeline (`an.py` aligns it with the trials). For another program, start it
and run `latprobe` with the same thresholds; restore the display mode
afterwards if it changed it (`xrandr --output eDP-1 --mode 1920x1080`).
