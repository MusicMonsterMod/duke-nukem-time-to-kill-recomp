# D08A44 - Duke 3D-style night vision look

Status: Done, user-accepted 2026-11-16: "the duke3d style night vis becomes the
default setting in duke ttk. lock it in and close it out". DUKE 3D is the
default NIGHT VISION LOOK; ORIGINAL stays selectable. The user played the
look with the overlay tint below and accepted it; blood stays as Duke 3D draws
it (checked in EDuke32: only enemies and players glow, a blood sprite takes the
palette's orange-yellow; the blood pool and gibs were not clearly in view).

## Request

User, 2026-11-14 playthrough: night vision should look like Duke Nukem 3D's,
"especially enemies in bright fluorescent green". Job: research how Duke 3D and
EDuke32 do it, then a palette or lookup-table step in Polymorph rather than an
expensive post-process. Acceptance: matched shots against EDuke32, cost
negligible at 180 Hz, the current look still selectable.

## How Duke 3D does it

EDuke32 source (r10669, local `research/eduke32`):

- `player.cpp` `P_UpdateScreenPal`: with `heat_on` the view palette is
  `SLIMEPAL`, one of LOOKUP.DAT's five alternate full palettes.
- `game.cpp` (tsprite loop, "Night vision goggles tsprite tinting"): with the
  goggles on and charged, every enemy (`A_CheckEnemySprite`), any sprite flagged
  `SFLAG_NVG`, player sprites and dummy players get `t->pal = 6` and
  `t->shade = 0`: palette lookup 6, drawn at full brightness.
- `screens.cpp`: for hightile (true-colour replacement art) Polymost multiplies
  the screen by (208, 255, 192) instead; 8-bit art uses the palette itself.

The data (the user's own DUKE3D.GRP, read locally, nothing shipped):

- SLIMEPAL is the base palette with green raised per palette ramp: greys and
  blues about +20 to +50, red-browns raised toward their red, the yellow, orange
  and fire ramps unchanged.
- Lookup 6 maps each colour's position in its 16- or 32-entry ramp onto one
  green ramp (indices 96-127): the darkest position becomes the brightest
  pale lime (178, 255, 121), the brightest becomes black. Enemies therefore look
  like a pale-lime negative, which shade 0 keeps bright in any light.

The user's EDuke32 recording `research/screencaps/nv-sound.mp4` (static
rooftop, the goggles toggled): brown wood (72, 34, 26) becomes (78, 74, 22),
gun metal (68, 64, 65) becomes (70, 85, 67), window yellow barely changes, the
red status bar digits turn yellow. No enemies are in that clip, so we captured
our own (`recomp/analysis/d08a44/eduke32/`, local): a private copy of the
user's EDuke32 r10669 and config, 640x480 Polymost, 60 fps cap (without it the
game never advanced on the offscreen X display), a test CON `nv.con` that
places a lizard trooper, pig cop, lizman and commander ahead of Duke as static
sprites and toggles `heat_on` every 4 s, captured losslessly.

| EDuke32 enemies under night vision (enemy pixels, 7586) | Value |
| --- | --- |
| Mean colour | (155, 188, 96) |
| Luminance 5 / 25 / 50 / 75 / 95 % | 39 / 160 / 186 / 199 / 209 |
| R/G, B/G | 0.830, 0.496 |
| Correlation with the unlit brightness | -0.33 (the negative) |

## TTK's own goggles

Traced in the executable (generated code, read only):

- While player+0x224 bit 0x80000 is set, the player update calls 0x80040dd8
  every frame. It stores one flat colour in 0x800c0bb8 (0x8000, green 128, or
  0x7000 on frames where the hblank counter is odd: a flicker), sets
  0x800c270c and stores 0x00305030 in 0x800c0bcc (never read).
- Every 3D renderer uses 0x800c0bb8 in place of its vertex light: rooms
  (0x80062b48, 0x80037778: context +0x50 bit 2, +0x54), props and actors
  (0x8003177c, 0x80031d10), Duke, his held item and weapon meshes, the sky and
  the water grid. The small-model face walk 0x8002d848 pins its colour to it
  through 0x800c270c. Textures therefore show only their green and the level's
  darkness is gone. HUD and 2D are untouched; enemies get no special treatment.
- 0x8002e850, after the frame's draw, clears the colour.

## Design

Three cheap parts, all Modernized and Polymorph only:

1. **No flat green.** At the composition entry 0x80026164 (frame_replay,
   before the frame is published to the redraw workers) the goggles' colour
   (only the values 0x8000 / 0x7000) and 0x800c270c are cleared, so the world,
   Duke and his weapon draw with their own light, as in Duke 3D. The native
   charge, toggle, sound and saved state are untouched.
2. **The world colour** is a colour lookup applied by the picture's own present
   draw (no extra pass). Shipping a table derived from Duke 3D's palette was
   avoided; the table is generated from our rule: only green changes,
   G' = G + max(0, w . (1, L, L^2, (R-G)+, (B-G)+, (G-R)+, R(1-B), B)), display
   values 0..1, L the luma, w = (0.0222, 0.5779, -0.7633, 0.8319, -0.5926,
   -0.1063, -0.2773, 0.0093), fitted to the user's clip and SLIMEPAL.
3. **Enemies** are tagged per triangle and drawn by a remap in the textured
   shader: unshaded and unlit, the texel's luma inverted onto a pale-lime ramp,
   p = clamp(1 - 1.4 L, 0, 1), piecewise linear through (0, 0, 0),
   (69, 65, 40), (142, 138, 85), (174, 202, 105), (178, 255, 121). The ramp's
   colours are measured from lookup 6's target ramp; the gain 1.4 is matched to
   the EDuke32 capture (below).

| World colour model, user's clip, held-out frame pair | Green error, all / bright pixels |
| --- | --- |
| No change | 12.4 / 20.5 |
| Polymost's hightile tint (208, 255, 192) | 12.4 / 20.5 |
| Straight affine fit | 5.1 / 8.9 (pure red becomes 210, 223, 0) |
| Nearest-palette transfer (reference only, not shipped) | 6.4 / 5.6 |
| Our rule (shipped) | 5.6 / 6.8 |

The rule keeps red (255, 0, 0) at (255, 174, 0), yellow-orange as the clip's
status bar digits; window yellow (101, 101, 68) at G 115 (clip 106).

**Overlay tint (user, 2026-11-16).** "Almost quite easy to forget the night
vision is on ... just slightly more of a tint ... not like brighter ... a tiny
tiny touch." After the rule, red is scaled by 0.93 and blue by 0.90, and green
raised by exactly the luma they lose, so no pixel gets brighter or darker and
dark stays dark (developer `DNTTK_NV_TINT=r,b`, `1,1` none). UI slot 4 frame
mean with the goggles on: (86.5, 84.2, 39.0) before, (80.6, 88.0, 35.1) after:
the town goes from olive-gold to olive-green; the dark sewer barely changes.
Shots: `research/before-after/D08A44/tint-before-after.png` (local).

## Implementation

Framework (`recomp/psxrecomp`, exported into the tracked patch):

- `psx_mod_gpu_host_vertex_mode()` (`mod_plugins.h`): a host vertex with the
  mode word explicit (depth bits 0-1, `PSX_MOD_HOST_BANK` 4,
  `PSX_MOD_HOST_REMAP` 8). The mode word is already recorded into the redraw
  streams, so redraws carry it with no new record kind. `gpu.c` turns a
  triangle whose three vertices carry bit 8 into `gl_set_host_remap()`.
- `gpu_gl_renderer.c`: such a textured triangle is queued with `drs[1] = 2`
  (raw 0 modulated, 1 raw), unlit. `TEX_FS` gains the remap branch under
  `NV_REMAP`. Four textured programs now: full and lean (D23J), each without
  and with the remap; a batch binds a remap variant only when it holds a remap
  triangle (`tex_batch_remap`, a scan done only after one was ever queued in
  that renderer), so ordinary drawing runs exactly the old shader. The ramp is
  chained `mix` calls, not an indexed constant array (D41F1: NVIDIA puts those in
  local memory).
- `psx_mod_set_present_lut()`: a 32 x 32 x 32 RGB table, stored as a 1024 x 32
  strip and sampled with two bilinear taps, applied by program variants of the
  plain present (`present_target_quad`: the live picture, widescreen and the
  120/180 Hz redraw images) and of the interpolation blend, only for pictures
  whose callback says so. A hold-last copy of the drawable is never graded
  twice. The HUD overlays drawn by the runtime above the picture and the D39
  tint are not graded; the game's status bar is part of the picture and is.
- `render_copy/rc*_names.h` regenerated for the new globals.

Title (`recomp/src/ttk`):

- `night_vision_look.inc` (in `modern_controls.cpp`): the setting, the live
  state (player+0x224 bit 0x80000 in a gameplay frame), the composition clear,
  the table, and the object loop's current object: entries of the five draw
  routines of table 0x800c0bf8 (a1 the object) and the loop step 0x8001ca4c,
  with the D36H9 stack test for a nested step. An object glows when its type's
  class (`*0x800d2660 + 28 * type + 6`) is 6, the hostile AI class, and it is
  not Duke. D17BF's sleeping stand-ins go through 0x80032e78 and glow too;
  `sleeping_enemies.inc` ends the scope after each.
- `near_clip.cpp`: an object frame of a glowing creature emits its host
  vertices through the mode call with `PSX_MOD_HOST_REMAP` added to the mode
  it had.
- `frame_replay.cpp`: each composed frame records whether the look is on; a
  redraw image is graded with its own frame's state, other pictures with the
  live state. Developer `DNTTK_NV_TRACE=1` prints presents graded, frames
  cleared and creature draws about every two seconds.
- Setting NIGHT VISION LOOK (OPTIONS > ADVANCED > ITEMS), `night_vision_look`
  / `DNTTK_NIGHT_VISION_LOOK` `duke3d` (default) / `original`, live on CONFIRM,
  `--night-vision-look`, `--settings` N, launcher schema 77 (migration keeps
  every choice). Vanilla always receives `original`; with the Classic renderer
  the row explains it needs Polymorph.

## Evidence

Lab `recomp/analysis/d08a44/` (local): `probe.py` on private profile and card
copies, SDL offscreen on the GTX 1080 Ti, debug port, test input
(`act=goggles`, cheats, `spawn enemy 26`); every run asserts the player's 40
save/card files and profile unchanged. Candidate executable
`1f79e2de402bffee56065cac974d9a2fe4a2d75a912124efe5c744ebdf42b542`.

| Check | Result |
| --- | --- |
| UI slot 4, first person, a lizard trooper spawned ahead (`v4`) | the trooper a pale-lime negative, the town olive, status bar digits yellow-orange; first-person weapon and Duke's arms keep their shading |
| Enemy pixels, TTK against EDuke32 | mean (165, 201, 103) against (155, 188, 96); luminance 25/50/75 % 166/187/201 against 160/186/199; R/G 0.83 against 0.83; correlation -0.30 against -0.33 |
| Ramp gain | 1.0: median 196, too flat; 1.4 matches (chosen); 1.8 median 176 |
| Dark sewer (UI slot 6), third person | green world, darkness kept |
| ORIGINAL look; Classic renderer | TTK's flat green; trace: 0 graded presents, 0 frames cleared, 0 creature draws |
| Vanilla | boots and draws as before (no plugin) |
| Savestate saved with the goggles on, loaded after switching them off | the Duke 3D look comes back at once |
| `PSX_GL_PROGRAM_DUMP` (NVIDIA assembly) | no `lmem` in any of the four programs; the remap variants R0-R24 / R0-R23 as the plain ones, 19 instructions more |

GPU at 180 Hz (whole GPU, clock-weighted from `nvidia-smi` every 200 ms over
15 s, UI slot 4, first person, turning with `dx=3`, 2928-2932 presents in each
run, graphics CUSTOM so AUTO does not fall to 60 offscreen):

| Build and state | Runs | Mean |
| --- | --- | --- |
| First build (remap in the everyday program), goggles off | 24.0, 24.0, 23.1, 23.9 | 23.7 % |
| Same, remap compiled out (`PSX_NV_RAMP=off`) | 23.0, 23.4, 21.3, 21.3 | 22.3 % |
| Final (remap only in its own programs), goggles off | 22.6, 21.6 | 22.1 % |
| Final, remap compiled out | 23.8, 23.1 | 23.4 % |
| Final, goggles on, DUKE 3D | 21.2, 23.8 | 22.5 % |
| First build, goggles on, ORIGINAL | 20.5 | 20.5 % |

The first build cost about 1.5 points with the goggles off although the remap
never ran (the D41F1 effect); with the remap in its own programs the goggles-off
cost is gone and the look costs nothing measurable (run-to-run spread about
1.3 points). ORIGINAL is cheaper because the flat colour skips the vertex light.

Suites: Python 260 tests OK (2 skipped), `ttk-near-test` with the owned
executable (new D08A44 group: a creature's host vertices carry the remap with
their depth mode, other objects never), `ttk-input-test`, `ttk-flash-test`,
the three CI runtime tests, the patch on a clean pinned `08ec704a` (1762 files
match). `ttk-controls-test` stops at its `hud_intact()` identity assertion with
and without this job's sources (a state table at 0x800c29f8 differs from its
fixture), so that failure predates D08A44; `ttk-aim-test` does not link
(`psx_mod_replay_active` missing from its stubs) for the same reason.

## Limits

- Matched shots are numerical (colour statistics of enemy pixels, the world
  model against the user's clip), not the same scene: TTK and Duke 3D share no
  map or art. The look needs the user's judgement in play.
- TTK's red-brown towns read olive-gold rather than strongly green, because the
  measured Duke 3D transform turns reds toward yellow (its own red ramps do the
  same). The rule's weights are easy to change if a greener world is wanted.
- Enemies are the hostile class 6 types; NPCs (class 8) and pickups do not
  glow; dead enemies of class 6 still glow (not compared with Duke 3D's
  corpses). Faces of a mesh that keeps the original renderer path
  (a mesh a frame cannot afford, D17AM3) are not tagged.
- The runtime's own overlays (inventory switcher, level stats, crosshair image)
  are drawn above the picture and are not graded.
- Measured on the desktop (GTX 1080 Ti); the field laptop and TV PC were not
  run.

## Player check

Pick up or spawn goggles (`spawn goggles` in the console) and press **N** in a
dark area with enemies near; compare with **OPTIONS > ADVANCED > ITEMS >
NIGHT VISION LOOK > ORIGINAL**. From the workspace root:

```sh
python3 recomp/tools/local/run.py
```
