# D14 - Widescreen (Modernized)

Status: **Done** (user-accepted 2026-10-01: "im very happy with it! i accept!"). Modernized now opens in true 16:9 by
default (`run.py --widescreen off|16:9|16:10|21:9|auto`). Vanilla and
`--widescreen off` keep the original 4:3 picture. The file name is historical;
it held the first-pass notes, kept at the end.

## What the player gets

- A wider view rendered natively: the world is not stretched. The projection
  (GTE H, centre) is the original one, so the vertical view and everything in
  the old 4:3 area are pixel-for-pixel where they were; the extra columns at
  both sides are real rendering (Hor+). At 16:9 the third-person horizontal
  field of view grows from about 67 to 83 degrees.
- No geometry holes in the revealed columns (rooms, props, enemies, effects,
  near walls).
- HUD boxes in the screen corners at their original size and edge spacing
  (health/armour bottom left, ammo and the active item bottom right).
- Movies, the title, main menus and the memory-card screens stay 4:3
  (pillarboxed, centred). Pause and Select screens show the wide world behind
  their centred text; the pause dim covers the full width. Night vision tints
  the whole width.
- Aiming is unchanged: the crosshair is at the true screen centre and view
  aiming uses the same projection.
- 16:10 and 21:9 work the same way; `auto` follows the window between 4:3 and
  21:9 (borderless fullscreen then matches the monitor).

## How it works

### Framework path: native-wide

`game.local.toml [widescreen] gte_game_mode = true` plus the `ttk.widescreen`
activation plugin (preloaded package `dnttk.presentation.widescreen`). The
plugin calls `psx_mod_set_fixed_display_aspect(W,H)` (or the adaptive variant
for `auto`) only when `DNTTK_INPUT_MODE=modernized` and `DNTTK_WIDESCREEN` is
not `off`. The runtime's default native-wide mode then:

- keeps the GTE unsquashed and canonical VRAM untouched (TTK displays 512x240
  with two buffers stacked vertically at x=0);
- mirrors framebuffer primitives into a separate wide surface per buffer,
  shifted by the per-side reveal (85 native pixels at 16:9, 192 at 21:9), and
  presents that surface 1:1;
- classifies frames with a 3D world (GTE vertex volume) as gameplay; MDEC movies
  and 2D menus present 4:3.

### Why the first preview was "tremendously broken"

1. **Off centre and tearing:** the preview config had `nw_hud_corners = true`.
   For a title without a sprite anchor that option shifts *every* untagged
   polygon by thirds (left third -85, right third +85), i.e. world geometry
   too. The canonical frame itself came out shifted left (Duke at x 170 of
   512) and torn. Never enable it for TTK; the config comment says so.
2. **Holes at the widened edges:** TTK's own visibility (below).

The squash path (`ws_nw on=0`, DuckStation-style) was A/B tested on the same
frames: it is centred, but it stretches Duke, every 2D element and the HUD by
4/3. Native-wide was kept.

### TTK visibility: the portal root rectangle

TTK is a portal engine. The camera render `0x8002e48c` walks rooms from the
camera room (`0x8006276c`, or `0x80037104` per room when the camera room is
invalid) through portals clipped against the root screen rectangle
**`0x800d2210`** (x, y, w, h). Room meshes (`0x800365e8` bounds test), props,
actors and the after-render effect passes (`0x8006d08c`, `0x8009ce44`, ...,
all given `0x800d2210`) cull against that rectangle or ones derived from it.
`0x8001f9f4`, called every frame from the screen-mode setter `0x8001fc44`,
rewrites it as `(-w/2, -h/2, w, h) = (-256, -120, 512, 240)` from the screen-mode
record `0x800d1de8 + mode*0xd0` (+4 width, +6 height; the record also holds
prebuilt DR_ENV packets, so it is not edited).

`widescreen_view_rect()` (hook `0x800b4d9c` from `0x8002e4c8`, ra `0x8002e4d0`:
the render's per-frame projection load, just before the portal walk) widens it
to `(-256-m, -120, 512+2m, 240)` with `m = psx_mod_widescreen_x_margin()`. It
only acts on the exact 4:3 rectangle (never twice, never split-screen
viewports). The next frame's rewrite restores it.

### Near walls in third person

With a wider view, third person shows polygons beside the camera that have a
corner at or behind the eye. The world renderer drops those and the object
renderer tears them (documented in D11B). D11B's conservative near clip draws
exactly those polygons; `widescreen_near_clip_live()` now runs it in third
person whenever the widescreen margin is live. The first-person-only occluder
fade change stays first-person only. Measured cost: none (fps identical with
`DNTTK_WS_NEAR=0`).

### HUD corners

The status bar layout table **`0x800dd778`** holds (x, y) pairs relative to the
screen centre, two per element (player slots). It is built once at level start
by `0x8008c7b0` (from `0x800257a8`), using the view rectangle: health
`(-246,89)`, armour `(-165,89)`, the third left element `(-246,69)`, ammo
`(169,89)`, item `(169,69)`, `(82,89)`, `(169,-105)`. New hooks (regenerated,
`game.local.toml` `mod_function_entry_funcs`):

- `0x8008BA30` (status bar draw, from `0x80026588`, ra `0x80026590`): when one
  player and the margin is live, x < -32 moves left by m and x > 32 right by m;
  the original values are kept host-side.
- `0x8001FC44` (the screen-mode call right after the status bar in the frame
  loop `0x80026164`): restores the game's values. The render hook also
  restores as a fallback.

The table therefore only differs from the original while the status bar draws,
inside one frame: savestates, menus, 4:3 and Vanilla never see the shift.

Host overlays (Duke font messages, the D08A2 inventory strip, the D07
crosshair) are drawn on the presented frame and were already placed correctly.

## Launcher

Profile schema 18 adds Modernized `controls.widescreen` (`off`, `16:9`,
`16:10`, `21:9`, `auto`; default `16:9`; earlier files migrate with a backup).
`run.py --widescreen VALUE` saves it, `--settings` choice W edits it, and the
launcher passes `DNTTK_WIDESCREEN` (always `off` for Vanilla). Developer
switches (environment, not saved): `DNTTK_WS_RECT=0` keeps the 4:3 portal
rectangle, `DNTTK_WS_NEAR=0` keeps third person on the original near handling.

## Evidence

Binary `3f726b9173ef236ca7c5f38b5c12b6713a877f85b6bbd5c0b2a8aca5e392af2c`.
Private copies of the player's UI savestates (no memory cards), Xvfb window
captures at 1920x1080 / 2520x1080, OpenGL 4x. Captures and harness in
`recomp/analysis/d14-widescreen/r2/` (`ws2.py`, `sweep.py`, `shots/`).

- Centring: canonical frame with/without `nw_hud_corners` (`shots/ab.png`).
- Portal rectangle A/B at fixed headings: right-margin near-black fraction
  0.39-0.41 off vs 0.10-0.14 on; full sweeps (6-12 headings, slots 3 and 12)
  better in every heading (`sw2`). Remaining dark areas inspected: real dark
  geometry.
- Near clip A/B in third person: slot 12 heading 3 right margin 0.945 -> 0.000,
  slot 3 heading 1 left margin 0.212 -> 0.000 (`nc1`).
- HUD corners at 16:9 and 21:9 (`hud-s4.png`, `w21-s4.png`), pause/Select
  screens, night vision, first person, inventory strip, boot FMVs/logos/title
  pillarboxed (`boot.png`, `menu*.png`), `auto` live resize 16:9 -> 21:9 -> 4:3
  (`auto.png`).
- Real launch path: `run.py` (private settings file) selects 16:9, native-wide.
- DuckStation reference (portable copy of the installed flatpak, private
  config, no memory cards): its widescreen hack shows the same wider view but
  stretches Duke, text and HUD by 4/3 (`shots/vs-ds.png`, attract demo).
- Software GL (llvmpipe, CPU) frame rate, 4x: 4:3 53/48/49 fps, 16:9 43/38/34;
  1x 16:9: 61/60/59. The drop is fill rate on the CPU renderer; real-GPU frame
  rate not measured here.
- Tests: `ttk-controls-test` (activation for every value, rectangle and HUD
  hooks incl. callers, Vanilla, split screen, 21:9), `ttk-near-test`,
  `ttk-input-test`, Python 92 OK (new `WidescreenTest`), Vanilla route
  `d14-vanilla-1` exit 0 (normal 512x240 captures, no widescreen activation).

## Case study: DuckStation widescreen hack versus native-wide

The user's reference was Time to Kill in DuckStation fullscreen. We ran the
same disc in DuckStation (a portable copy of the installed build, private
settings: widescreen hack on, 16:9, 4x resolution, no memory cards) and in the
recomp at 16:9 and 4x, on the same attract-mode demo.

How the emulator does it: the widescreen hack squashes every 3D vertex
horizontally by 3/4 inside the GTE, then stretches the whole 4:3 frame to 16:9.
3D geometry comes out at the right shape with a wider view, but anything the
game draws in screen space without the GTE is stretched by 4/3: the title and
"PRESS START" text, the menus, the HUD boxes and digits, sprites and effects.
(Movie framing under the hack was not captured.) The game's own 4:3 screen-space culling still runs against the
squashed coordinates, so the emulator cannot tell what the game skipped.

How the recomp does it: the projection is left exactly as the original, and
the runtime renders the extra columns as real pixels into a wider surface,
presented 1:1. Nothing is squashed or stretched, so text, HUD, menus, movies,
sprites and Duke keep their original proportions, and the 4:3 middle of the
picture is the original image. Because the game is recompiled rather than
emulated, the game's own decisions can be changed where they assume 4:3: the
portal root rectangle is widened so the revealed columns are actually drawn,
near walls are clipped properly in third person, and the status bar layout is
moved to the real corners for the frame it is drawn in. Movies and 2D menus are
recognised and kept at 4:3, and 16:10, 21:9 and window-following `auto` use the
same path.

Result on identical frames: the same wider field of view, with DuckStation's
text and HUD visibly 33% wider and the recomp's at their authored shape
(captures `recomp/analysis/d14-widescreen/r2/shots/vs-ds.png`, local only).

## Limits and follow-ups

- Tested on a handful of first-episode scenes, the attract demo and the boot
  path before acceptance; report any level with edge holes.
- A short transient of black margin can still appear for a frame while the
  camera swings hard next to a wall (seen once in a capture series, before the
  near clip was on in third person; not reproduced after).
- Split-screen two-player keeps the original 4:3 layout.
- Pre-rendered movies stay 4:3 by design.
- The runtime-patch applicator currently fails on `time-to-kill-stopped-window.patch`
  (an existing stacked-patch check), so `build.py` stops before generation;
  this session ran its remaining steps directly (generate, configure, build,
  movie shard). Not caused by D14.

## First pass (2026-09-30), kept for history

The first pass added the inert activation plugin and package, found that a
private 16:9 experiment rendered wider but with black bands and an unanchored
ammo box, and the user preview (with `nw_hud_corners`) was off centre and
torn. The causes are explained above.
