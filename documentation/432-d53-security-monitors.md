# D53 - Security monitors, Duke 3D style

Status: Done (user-accepted 2026-11-09: "lock it in and close it out").
Follow-ups: D53A (the screen keeps a low-res live picture as you walk away),
D41K (props in dynamic light), D23I (memory growth, seen meanwhile).

## Request

User, slot 9: "In duke3d, there was a recognizable UI over the security
cameras, you knew you were looking at the security cam view. Duke TTK doesnt
really have that." The monitors should look like Duke 3D's ("Many times have I
just walked past those screens in the subway"), be bigger ("the native TTK ones
just look insignificant"), give off light, and have a settings switch between
the TTK and Duke 3D looks. The design proposal (artifact) was approved with
option B (a Duke-style TV face), the 4:3 overlay placement and a screen light.
During the buildout the user asked to go "a lot closer to how duke3d's
security screen worked", match its colours, its glass and its bezel, fix the
bezel's bottom, and keep the screen text generic (no level names) so the same
asset suits every security screen, custom levels included.

## Duke 3D reference (EDuke32 source, research/eduke32)

- `screens.cpp G_DrawCameraText`: CAMCORNER 2482 (corner with REC) at
  (24,33), CAMCORNER+1 at (294,34), (22,163) and (300,163) rotated, on the
  320x200 screen, tiles centred; CAMLIGHT 2484 at (46,32) while
  `totalclock & 16` (133 ms on, 133 ms off at 120 Hz). The status bar stays.
  A shot-out camera shows STATIC instead (TTK cameras cannot be shot out).
- VIEWSCREEN 502: the idle screen, a near-black glass (16,12,16) with a
  rounded bevelled rim and corner glints, SECURITY in heavy olive capitals
  (112,108,68 with a lighter top and darker foot), two short lines and a
  status table below, the lower half empty.
- Tile 638 (the user's bezel reference): warm grey plastic (36..88 grey,
  noisy), 8-cell borders top and sides on a 64 grid, a double bevelled frame
  lit from the left, and the controls in the deeper bottom border: a dark lip,
  a raised bevel line with a small centre emblem, then a tray with dotted
  grilles at both ends and keys and a readout in the middle.
- Lingering picture (D53A): `actors.cpp`/`sector.cpp` VIEWSCREEN cases keep
  rendering the camera into a small tile every `ud.camera_time` ticks while
  the player is within `VIEWSCREEN_ACTIVE_DISTANCE` 8192.

## TTK's monitor

LEVEL00 (UI slot 9): three type 858 static props, subtypes 39/40/41 (the D35K
callback), mesh `0x80142660`: 16 vertices, a flat 400x500 panel of nine
textured quads (0x2c) in a 3x3 slice at z 0 (border, a 32x32 glass texture,
the 64x16 SECURITY plate), outward model -z, y down, u along +x. Header: +6
vertex count, +8 bounds (min xyz, max xyz, radius 320 at +14), +0x10
vertices, +0x14 polygon list; the prop keeps its own bounds copy at +0x4c.
Probe: `recomp/analysis/d53/lab.py probe`.

## Implementation

`recomp/src/ttk/security_monitor.cpp` (+ `.h`), Modernized, Polymorph, option
DUKE 3D only:

- **TV.** The near-clip object entry (`0x80010000`, near_clip.cpp) asks
  `security_tv_swap(mesh)`. A TTK monitor mesh (found by shape: 16 vertices,
  nine 0x2c quads, first corner (-200,250,0), radius 320) is replaced in a0 by
  a guest copy of the TV mesh in mod memory: a 620x620 box standing 140 from
  the wall, the bezel at z -140, the square screen recessed to -125 with
  recess walls, ten quads, 16 vertices. Its faces are drawn by the near-clip
  host path from the second texture bank (the D17AO1 borrowed-sky route),
  where the TV atlas sits at x 0..255 (15-bit texels, tpage 0x100); the bank
  is built once (`ttk_foreign_vram`, now shared with the sky pack). If the
  host path cannot take the mesh (budget, downgrade), the TTK monitor is put
  back for the original routine (`security_tv_fallbacks` in the near-clip
  JSON), so retail VRAM is never sampled with the TV's coordinates; in a game
  view, TV faces nearer than H are dropped as the original drops them. The
  prop, its use, collision and camera are the game's own.
- **Screen.** Gouraud faces with a fixed colour (140 of 128, 230 since revision 6, a faint flicker
  every 33 ms), so room light does not tint the glass; the mesh is a glow mesh
  (no dynamic light on it). The cursor blinks every 530 ms (two screen
  frames in the atlas). Clock-driven, so redraw workers match.
- **Light.** Each TV within 9000 of Duke adds a light 420 in front of its
  screen in the screen's olive (0.92, 0.95, 0.66), strength 0.9, radius 1800
  (D41A light list, after the level lamps). Lab tuning showed 0.55/2200 below
  visibility and 2.0/4000 flooding the wall. `DNTTK_SECURITY_LIGHT` overrides
  for tuning with the user. TVs are found by a room-list prop scan per level.
- **Camera frame.** `ttk_security_overlay_piece` gives the present pass
  (gpu_gl_renderer.c, framework patch) the frame corners and the record
  light, authored at 1600x1200 for the 4:3 area of the picture, scaled to it
  with linear filtering and alpha, while Duke +0x224 has bit 0x20000000 in
  play. The record light blinks 133 ms on/off in real time.
- **Option.** OPTIONS > GAMEPLAY > SECURITY MONITORS (DUKE 3D / TTK, live),
  profile schema 72 `security_monitors`, `run.py --security-monitors`,
  `DNTTK_SECURITY_MONITORS` (Vanilla: ttk).

Art (original, Photon Studio): `recomp/assets/ui/security-monitor`.
`camera-overlay.photon` (vector corners, blocky REC, light with glow; exported
as frame and light layers); `make_tv_layers.py` paints the TV layers in Duke
3D's style and palette values (no tile pixels), `assemble_atlas.py` assembles
`tv-atlas.photon`/`.png` in Photon. `tools/local/build_ttk_security_monitor.py`
packs everything into `ttk-security-monitor.pack` in every build.

## Evidence

Lab `recomp/analysis/d53/lab.py tour` (private copy of UI slot 9, offscreen):

- DUKE 3D: the three TVs drawn at 60 and 180 Hz, first and third person;
  glass (18,9,18) after the glow-mesh change (was (32,14,26) with dynamic
  light on it); camera view shows the frame; at 180 Hz the record light
  alternates on/off across shots 90 ms apart; returning from the camera and
  walking back are normal. Lights: 3 published per tick (trace).
- TTK (`--security-monitors ttk`): original monitors and bare camera view.
- Vanilla (`--vanilla`): original monitors.
- Matched before/after: `research/before-after/D53/` (local).
- `ttk-near-test` and `ttk-aim-test disc/SLUS_005.83` pass;
  `test_player_profiles.py` and `test_ui_art.py` pass (146 tests).
  `ttk-controls-test` still does not link (`gpu_vram_peek` from
  level_lights.inc, D41D1, as recorded in D52).
- The framework patch was exported and applies to the pinned framework.

## Revision 2 (user, first playtest feedback)

User: "excellent work for the first iteration - now when the 3d object enters
your peripheral vision, it disappears, maybe a culling thing like we
experienced with the train. can you also make the texture of the tv borders
not look like stretched pixels ... the white glow on the record red dot
[doesn't work]. also can we make the console by the door emit light too,
right at the controls section!"

- **Edge culling (cause confirmed).** The game culls a prop by the bounds it
  keeps (+0x4c: min x,y,z, a fourth half, max x,y,z, radius +0x5a), copied
  from TTK's 400x500 panel, so the larger, protruding TV was dropped while
  its edge was still on screen (the D17AZ case: the object's own extent, not
  the room). A scene-composition hook (`0x80026164`, live frames) finds the
  monitor props once a level and every 300 frames and gives them the TV's
  bounds (radius 430) while the Duke 3D look is on, the mesh's own in the TTK
  look (rechecked every 60 frames; Vanilla only reads). Fine 180 Hz sweep at
  the left edge, `DNTTK_SECURITY_BOUNDS=0` (developer A/B) against the fix:
  the next TV's edge is missing before, drawn after
  (`research/before-after/D53/edge-pair.png`). Using a monitor still enters
  the camera view.
- **TV sides.** One 16x16 texture had been stretched over 620x140 faces. Now
  a 128x32 strip for the top and bottom and a 32x128 strip for the sides,
  at the bezel's density (about 4.8 units a texel), grained like the bezel,
  the lit edge meeting the bezel and shade toward the wall; the recess walls
  are a flat dark fill. Atlas rows now 256.
- **Record light.** The outer glow is removed in `camera-overlay.photon`; the
  light is the red dot alone, as in Duke 3D.
- **Console light.** (Replaced in revision 3.) A green light over the type 17
  console's top (strength 1.8, radius 900).

## Revision 3

User: "the only part of the console i want to glow is that section on the
right hand side where the controls are ... only the part the user would
interact with", and "when you get far away enough from the control room
there, the light emission from the security monitors goes off".

- **Keypad.** The controls are a separate prop: type 477 (interactive,
  subtype 21), one 256x192 quad lying on the console's right section, red
  and green buttons and a number pad. The revision 2 console light is gone; a
  point light cannot make a panel glow (strength 25 at radius 600 only
  greyed the desk top: the per-pixel light saturates on these props). Now,
  in Modernized (Polymorph), a keypad found lying within 900 of a console
  (both by shape) is drawn self-lit: the object entry gets a guest copy of
  its mesh whose face is a gouraud quad with a fixed colour of 255 (twice the
  neutral 128, the most a vertex colour gives) and no dynamic light, using
  the keypad's own retail texture. Only the panel brightens (lab: its pixels
  about 1.7x to 2x, the desk unchanged). The copy is plain retail VRAM, so if
  the host path leaves it the original routine draws it correctly.
  `DNTTK_KEYPAD_GLOW` (64..255) tunes it. Keypad slots are atomics: redraw
  workers read them.
- **TV light at a distance.** A 9000-unit range cut dropped the TV lights;
  it is gone, the nearest six of these lights are always published (lab: 3
  TV lights at every point of a walk to the far door).

## Revision 4: generic screen text

User: the CAM 1-3 rows assume three cameras; the screen must suit any
security monitor. Four row sets were mocked up (status, feed, sweep, log);
the user chose B, "it's gotta be B feed": FEED LIVE / LINK SECURE / TAPE
REC, describing the one feed behind the screen, in the same layout and
density. `make_tv_layers.py` then `assemble_atlas.py` rebuilt the atlas.

## Revision 5: the power switch (UI slot 10)

User: "in slot 10, i want this power button to emit light ... nothing draws
the players attention to it. so the button console and the texture above it.
make it look really cool". At Duke (-7471, 50691) in LEVEL00 a wall power
switch: a type 477 keypad (subtype 38, the same mesh as the console's) under
a type 481 sign, a 256x260 quad (first corner (-128, 130, 0)) with a yellow
lightning bolt on blue.

- Keypads now glow wherever the game makes them usable (subtype +0x34 not
  0), on consoles and walls, at 255 (self-lit copy, as revision 3); the
  console pairing rule is gone.
- A power sign within 600 of such a keypad is drawn self-lit with an
  electric pulse: colour 170..255, a 1.8 s breath with a quick crackle (about
  one 50 ms step in 14 jumps to full or drops low), from the clock so redraw
  workers match. A light 260 in front of the panel, between sign and keypad,
  electric blue (0.38, 0.62, 1.0), strength 1.8, radius 1200, pulses with it
  (0.45..1.0) and throws a halo on the wall. `DNTTK_POWER_LIGHT` tunes it.
- Glow copies live in a 0x100 block of two 0x80 slots (keypad, sign);
  matched meshes in fixed atomic slots.
Lab: matched shots at the user's save, before/after and a pulse strip in
`research/before-after/D53/power-*.png`; the console keypad still glows.

## Revision 6: brighter screens

User, slot 9: "increase the light emission of the screens themselves ...
just the screens". The screen faces' fixed colour (flicker around it) rose
from 140 to 230 (`screen_base`, `DNTTK_SCREEN_LEVEL` 64..251); the bezel,
box and the wall light are unchanged (the light follows only the flicker).
Lab at 140 / 200 / 240: 240 read most lit but lifted the glass toward plum;
230 chosen. Matched shots: screen text about 1.6x brighter (126 to 205),
bezel and wall pixels equal (`research/before-after/D53/screens-pair.png`).

## Revision 7: a glow ring around the screen

User: the border "looks a little too dark when the lights go off ... a touch
of light haze around it". The TV is a glow mesh (no dynamic light), so its
bezel follows only the room's own light. Added (nothing existing changed):

- **Atlas:** a 32x16 haze gradient at (144,192), new Photon layer
  `screen-haze`, row 0 at the screen's edge fading to clear at row 15, olive
  white, alpha 160; the pack builder sets the PS1 STP bit for alpha 128..254
  (semi-transparent texels).
- **Model:** four mitred strips 3 units in front of the bezel, from 10 units
  over the screen's edge (222) out to 300, gouraud, semi-transparent with
  blend mode 1 (additive), palette entry 1: the texture's own strength times
  the screen's flicker. Self-lit, so it stays when the room light drops.
- **Draw order:** translucent faces are painter-only; the one big bezel quad
  sorted over the far strips (only top and left showed). In the TV mesh the
  ring now takes the ordering slot just nearer than the nearest opaque face
  (near_clip.cpp, TV meshes only).
Lab: full ring in matched shots (`haze-pair.png`), steady at 180 Hz, third
person fine; `ttk-near-test` passes. The room light did not dip during the
lab runs, so the dark case is for the user's playtest.

## Limits

- Only the LEVEL00 monitor mesh shape is recognised; monitors in other levels
  with another mesh keep the TTK look until they are added (check LEVEL05 and
  LEVEL09, whose cameras use the same entry, D35K).
- The screen light strength is a first value, to tune with the user in game.
- Classic and Vanilla keep the original monitors by design.
