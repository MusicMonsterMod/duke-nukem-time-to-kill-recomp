# D52 - Security camera screens: low frame rate and glitches over them

Status: Done (user-accepted 2026-11-09: "I can confirm the cams are now
stable, and cams 2 and 3 show no enemies"). Follow-up: D17BF sleeping
enemies.

## Report

User: "slot 9, the security cameras display at a seemingly low framerate,
and display graphical glitches over them. check the 3 security cams and then
investigate why this is happening, and then fix the security camera". UI slot
9 is file index 8 (LEVEL00, first person, Duke facing three SECURITY
monitors). All work used a private copy of that slot (lab
`recomp/analysis/d52/lab.py`, offscreen, port 9521); the player's files were
hash-checked unchanged after every run.

## The three cameras

Walk up to a monitor and press the action (left mouse in Modernized). The
level overlay's callback (D35K: LEVEL00 `0x800cc14c`, subtypes 39/40/41)
calls `0x8003b36c`, which sets Duke's script bit `0x20000000` (+0x224) and
points the alternate camera `0x800d7010` at a type 722 camera object.

- Left monitor: a machine room with a blue teleporter pad and a claw.
- Middle monitor: a cell block of grates and chains; this camera pans
  slowly (heading about 15 to 313 degrees and back).
- Right monitor: a flooded room with a grate platform and a pillar. Its
  see-through block under the grate is the original's translucent water
  (same with the host near clip off), and the band at the top is a ceiling
  beam.

## Fault 1: the picture moves at 30 fps

TTK runs its logic at 30 fps; above 60 Hz the redraw workers draw in-between
images (frame_replay.cpp). They interpolated only the player's camera
`0x800d6eb0`. The view composition `0x80026164` chooses its camera at
`0x8002620c..0x8002626c`: `0x800da488` while `0x800c3c4c` is set, the
alternate camera `0x800d7010` (+0xb0 per player) while Duke's +0x224 has bit
`0x20000000`, else `0x800d6eb0`. So in a security view every in-between kept
the camera of the frame it was drawn from. In first person with the late
camera, some redraws are drawn from the newer frame's list (D10H8) and some
from the older one, so the picture also stepped back and forth between the
two camera positions. Objects drawn through the alternate camera were not
recorded or interpolated either (`actor_draw` and `transform_load` accepted
only `0x800d6eb0`).

Measured (old build, 180 Hz, middle camera, 40 consecutive present shots):
the alternate camera's fields change every 33 ms (30 Hz), the pan moves in
whole game-frame steps of 0, 3 or 6 pixels, and 7 of 39 shot pairs were the
same image.

## Fault 2: a dark slab across the picture

A long dark polygon crossed the middle camera's picture (Polymorph and
Classic, 60 and 180 Hz, first and third person). It went away with
`DNTTK_NEAR_CLIP=0` and with `DNTTK_NEAR_CLIP=world` (object near clip off),
and with no other host switch. The original object renderer `0x80010000`
drops a polygon whose corners are all nearer than H. Since D11B the near clip
draws those polygons clipped in view space, so props up close in the eye view
do not turn see-through. A security camera sits inside its own housing
model, so those faces were drawn across its picture.

## Fault 3: enemies blink on monitors 2 and 3 (revision 2)

User, after the first fix: "the enemies on the security feed seem to glitch
in and out. this seems to affect only monitors 2 and 3". On the middle
monitor at 180 Hz an enemy showed in 2 of every 8 shots while the camera
barely moved. The same at 60 Hz (no redraws), with `DNTTK_NEAR_CLIP`,
`DNTTK_PRECISE_CULLING`, `DNTTK_UNIFIED_GEOMETRY` or `DNTTK_OPEN_PORTAL_TOP`
off, in Classic, and in the build from before D52: not the renderer.

A per-tick list of the actors the game drew through the monitor camera
showed actor `0x801cff94` (draw kind `0x80032e78`) in 3 ticks of 18. Its
position and room changed almost every tick (rooms 43, 76, 31, 48, 34, 67,
54, ...), each one the position of a spawner record (`*0x800de720`, 48
bytes; type 26 enemies and two type 123 records). With
`DNTTK_ENEMY_REACH=0` (the original spawn range) it kept one place.

The original spawner tick `0x80099530` visits four spawners a tick. The
spawn check (`0x8009926c`, `0x800991bc`) asks the range test `0x80099164`;
with the live cap full (`0x800c56a4` of `0x800c56a8`, 12 in level 0) it
calls the replacement `0x80099048`, which keeps the farthest live enemy
(actor +0 without bit 2) by `0x800675d4` and returns it unless the new
spawner is farther; that enemy is removed (`0x80098d70`) and the spawner
created (`0x80098f3c`). `0x800675d4` returns `0x7fffffff` once an axis
differs by 32768. The original's radius (fade + 0x13d0, about 25552) never
gets there, but D17AQ dormant enemies spawn to the extended draw distance.
So a far spawner tied with the far live enemy and replaced it, the next far
spawner replaced that one, and so on: one actor round-robined between them,
and an enemy in a watched room existed one tick in six.

Fix (`recomp/src/ttk/enemy_reach.inc`, in the existing range-test hook,
Modernized Extended draw distance only): with the cap full, a spawner counts
as in reach only if the enemy the replacement would remove, worked out on
the host with the original's saturated distances, is farther from Duke than
the spawner by exact distance. Every replacement now brings a nearer enemy
in, so none can repeat. No new game hook and no regenerated code; the
replacement and the distance routine joined the reach code guards
(`churn` counts refusals in the reach debug JSON). Those far rooms now show
no enemy, as in the original; game-wide, wherever dormant enemies reach past
32768 units.

## Fix

- `composition_camera()` and `game_camera_view()` (near_clip.cpp,
  near_clip.h) read the composition's choice from guest memory, live or in a
  redraw worker's image.
- Redraws (frame_replay.cpp) capture, interpolate and write the camera the
  frame is drawn from (`CameraState::base`). A switch between the player's
  and a game-owned camera is a cut. The late mouse view, Duke's body camera,
  the beam follow and the room walk from Duke apply only to the player's
  camera; the mouse never turns a security camera. Objects drawn through the
  composition's camera are recorded and interpolated (a mirror's stacked
  camera is still excluded); the sky is still recognised as sky.
- Near clip: in a game-owned view an object polygon with every corner nearer
  than H stays with the original routine, which drops it, as on the
  PlayStation (`game_view_drops` in the near clip debug JSON).

Game-wide: every view drawn through the alternate camera or `0x800da488`
(security monitors in any level, scripted views), not these three cameras
only. Ordinary play is unchanged: with the player's camera, every changed
path gives the same result as before (`game_camera_blends` stays 0, the late
camera keeps applying). Vanilla never runs redraws or the near clip.

## Evidence

- 180 Hz, middle camera, 40 present shots after: no repeated pairs, and
  in-between steps of 1 and 2 pixels; `game_camera_blends` about 500 per
  run. The same in third person and in Classic. All three cameras at 180 Hz
  show no new artifacts.
- Matched shots at camera heading 350 (60 Hz, same savestate): before, the
  slab; after, gone, with the rest of the picture unchanged. Kept locally in
  `research/before-after/D52/`.
- Entering and leaving the view: game-camera blends only while the script
  bit is set, cuts at the switch frames, late view applied again afterwards,
  Duke's view and HUD back normal.
- `ttk-near-test` and `ttk-aim-test` pass. `ttk-input-test` (which builds
  no D52 file) passed before the change; after rebuilding it, it hung on
  the desktop display socket, and with the offscreen video driver it stops
  at its relative-mouse check.

- Revision 2: actor `0x801cff94` keeps one position over 1.5 s (before: 28
  changes in 2 s); the middle monitor's enemy pixels stay 0 in 40 shots at
  180 Hz (before: 2 of 8); the right monitor draws the same 16 objects every
  tick. Walking and turning in slot 9: cap 12, live 12, 10 dormant, 474 tie
  replacements refused, no hops. Strips kept locally in
  `research/before-after/D52/enemy-flicker-*.png`. `ttk-near-test` and
  `ttk-aim-test` pass; `ttk-controls-test` does not link (`gpu_vram_peek`
  from `level_lights.inc`, D41D1, before this change).

## Limits

- Vanilla was not compared on screen: this savestate loads frozen in Vanilla
  (almost no RAM changes, pad input ignored). By code, Vanilla reaches none
  of the changes.
- Scripted views through `0x800da488` and other levels' cameras (LEVEL05
  and LEVEL09 subtypes in note 238) were not visited. Whether Duke's own
  model shows when a security camera looks at him in first person was not
  checked (Duke is not in these three views).
