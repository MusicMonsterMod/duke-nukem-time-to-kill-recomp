# D53A - Security monitors keep a low-res live picture

Status: Done (user-accepted 2026-11-21: "lock it in and close it out! this is insanely good work"). Dependencies: D53 (Done).

## Request

User: "in duke3d, when you arent watching the screen anymore, it shows a low
res image on the screen still as you walk away until it resets. i'd love
this, so much". At selection: "this has really got to be done as cheaply as
possible" (Duke 3D's were deliberately very low resolution).

Duke 3D reference (EDuke32, note 432): after the use key the VIEWSCREEN keeps
rendering its camera into a small tile every `ud.camera_time` ticks while the
player is within `VIEWSCREEN_ACTIVE_DISTANCE` 8192; past it the screen resets.

## Design: one more redraw, shrunk into the second bank

Polymorph already draws in-between images by re-running the game's own view
composition in forked worker processes (frame_replay.cpp, D17), and the
composition draws through the alternate camera `0x800d7010` (+0xb0 per
player) while Duke +0x224 has bit 0x20000000 (D52). So the feed costs no new
renderer:

1. **Watching** (live frames, `security_monitor.cpp`): the TV in use is
   latched from Duke +0x290 as the view begins (the word is cleared during
   the view), and the alternate camera's 0xb0 bytes are copied every frame.
   After the view the struct stays frozen (the game refreshes it from its
   camera object +0xa4 only while someone watches).
2. **Request**: at each gameplay capture `frame_replay.cpp` asks
   `security_feed_request`; a feed job (Params flag 128 with the camera
   bytes) is submitted on the newest published slot at most every
   `DNTTK_SECURITY_FEED_EVERY` game frames (default 2: 15 pictures a second),
   and only while the feed TV was drawn by a live frame in the last 2 frames
   (the first picture right after the view regardless).
3. **Worker**: writes the saved camera into its own image and sets Duke's
   view bit, then runs the composition as for any redraw (no camera
   interpolation). The live guest never sees either write.
4. **Draw**: the finished job is drawn into renderer cache entry 15 (the
   provider now presents from 15 entries), on a render thread when they run.
5. **Shrink** (framework, `gl_replay_cache_to_bank`): the middle square of
   the image's 4:3 picture (x 0.125..0.875) is box filtered (4x4 taps) into a
   96x96 rectangle of the second texture bank as raw 1555 texels, with a
   gamma lift (0.75) so dark camera rooms read on a small screen. Two
   rectangles at (256,64) and (352,64) alternate; the newest is named in a
   guest block (mod memory: TV, rectangle | 0x100), so each redraw uses the
   picture of its own frame.
6. **TV**: when the object being drawn (entry hooks on the object-list kinds
   0x800632b0, 0x80031d10, 0x80032e78, 0x800348d8, 0x80031c14) is the feed
   TV and a picture exists, its screen face samples the feed rectangle (page
   0x104) at self-lit level 220 instead of the SECURITY art.

The render threads' renderer copies now sample the emulation thread's second
bank texture (one GL name in the share group, `gl_foreign_bank_shared`), so
a picture written once reaches every redraw.

**Ends**: Duke farther than `DNTTK_SECURITY_FEED_RANGE` (8192) from the TV,
the TV gone or broken, a level change, a savestate load, the TTK look; the
screen returns to SECURITY until the TV is used again. Using another TV moves
the feed to it.

Switches (developer): `DNTTK_SECURITY_FEED=0` off, `_RANGE`, `_EVERY`,
`_LEVEL` (64..255), `_GAMMA` (0.3..1). Debug: `render_replay` ->
`plugin.security_feed` (submits, images, lost, and the monitor's feed state
with distance).

## Evidence (offscreen, real GPU, private copy of the 2026-11-09 UI slot 9 save)

Lab `recomp/analysis/d53/lab.py feed|perf|sweep|again` (local).

- After the middle camera's view the TV shows its picture, the other two keep
  SECURITY (`research/before-after/D53A/`, local: `after`, `back`, `far`
  pairs, feed off/on, same route). The picture is the camera's own view at
  96x96 texels, oriented as the view, with its live content (the yellow
  figure in the cell block after a second watch).
- 180 Hz first and third person, 60 Hz, render threads off
  (`PSX_RENDER_THREAD=0`): pictures stored, none lost (59 in 4 s).
  FRAME RATE ORIGINAL (no workers): no pictures, the screen stays SECURITY.
  TTK look: original monitors, no feed. Vanilla: no redraw plugin, no feed.
- Gating: in a step turn sweep, 15 pictures a second whenever the feed TV is
  on screen, 0 when it is not (other TVs in view or not).
- Range: with the range set to 2500 the feed ended at 2507 and the TV stayed
  on SECURITY after walking back; using the TV again restarts it; a
  savestate load ends it.
- Cost, 12 s facing the feed TV at 180 Hz, two runs each: GPU (clock
  weighted) 9.4/9.2 % off, 10.0/10.1 % on (raw 10.1 to 13.2 %); 180
  presents a second either way; about 15 more worker jobs a second (about
  2.5 ms each on a worker core), none failed. Nothing runs when the TV is
  out of view or out of range.
- Gamma: mean screen brightness 22.6 at 1.0, 42.2 at 0.75 (the idle
  SECURITY screen is 42), 64.5 at 0.6 (washed out).
- `ttk-near-test`, `ttk-aim-test`, `ttk-controls-test` pass; local unit
  tests 268 OK; the CI runtime tests pass; `ttk-input-test` stops at its
  relative-mouse check under the offscreen driver (as before). The exported
  framework patch applies to the pinned framework.

## Limits

- A panning camera keeps the direction it had when the player looked away
  (continuing the pan would mean running the game's camera update in the
  worker).
- Needs the redraw workers: FRAME RATE ORIGINAL (also chosen automatically
  on 2-core machines) shows SECURITY after the view.
- When the TV comes back into view after being off screen, the picture from
  when it left the view shows for up to about 70 ms.
- Only LEVEL00's monitor mesh is a Duke 3D TV (note 432), so only those
  monitors have the feed.
- Not measured on the field laptop.
