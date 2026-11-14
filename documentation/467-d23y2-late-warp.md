# 467 - D23Y2: late warp research spike

**Cancelled (user, 2026-11-13: "lets just absolutely can that warp thing. its
awful"). The code, the `--late-warp` flag and the manual entry are removed;
this note stays as the record of what was measured and why it failed
(sections 8 and 9: a rotation over the game's ~100 ms pipeline smears a third
to half of the screen on every turn and snaps back on fast ones).**

Part of the D23Y plan (note 463): for each in-between present, re-draw the
newest finished scene image with the newest camera look, and draw the HUD,
first-person weapon and screen overlays unwarped on top; measured on the
field laptop. Acceptance: 120 pictures a second at 2x with CPU as the
original 60 (all threads under 75%), GPU busy under 80%, input to picture
under 50 ms (note 462's probe), the artefacts listed. Built as an
experimental option, off by default; D23Y3 decides the player build.
Autonomous session, 2026-11-13.

Evidence labels: MEASURED (the field laptop, this session), CODE, INFERRED.

## 1. Design

- **Rotation only, no depth.** Turning in place moves every pixel by a
  homography that does not depend on depth (as VR's timewarp), so the warp
  needs only the image, the camera it was drawn with and the newest look.
  The host depth buffer is a renderbuffer, not a texture (`s_hr_rb`), so a
  depth-aware (positional) warp would need another copy per frame; moving
  the eye between game frames is left to D23Y3 / D23N.
- **The world layer.** The game draws its ordering table from the far slots
  to slot 0; near_clip links the first-person weapon into slot 1, drawn after
  everything but slot 0 (the HUD). When the weapon is linked, slot 1's head is
  the first packet drawn after the world (`near_clip_take_world_end`). At the
  next GPU display list (the game sends about ten a frame) the title hands
  that packet and the frame's camera to the runtime (`psx_mod_set_late_warp`,
  `dma.c`); when the list walker reaches it, the renderer copies the band the
  flip will show (the drawing area's) into a world layer, one per buffer.
- **The present.** Replay presentation runs at the warp rate with no redraw
  workers (`DNTTK_LATE_WARP=HZ`, a provider that always picks the real
  image). At each present the title gives the rotation from the image's
  camera to the newest look (`late_warp_delta`: the yaw and pitch from
  `late_view`, the rest of the game camera such as roll kept), and a shader
  (`lw_render`) draws, per pixel: the final image where it differs from the
  world layer (the weapon, the HUD, anything drawn after the world),
  otherwise the world layer re-projected. The game's camera rows carry its
  pixel aspect (it draws 512 pixels across): horizontal focal 256, vertical
  160 at the default field of view; the centre is the GTE offset plus the GPU
  draw offset. The tint, LEVEL STATS and the runtime's overlays are drawn at
  present as before, unwarped. A turn of more than 35 degrees (a cut) is not
  warped.

## 2. Built (CODE)

- Framework: `psx_mod_set_late_warp`, `psx_mod_late_warp_stats`
  (`mod_plugins.h`, `main.cpp`); the boundary in the display-list walker
  (`dma.c`, `g_dma_world_end_fn`); the world layers, the warp shader and
  pass, the present (`gpu_gl_renderer.c`: `gl_late_warp_world_done`,
  `lw_promote`, `lw_render`, used by `rp_present_slot` for the real image).
  Developer `PSX_WORLD_END_TRACE=1` traces the boundary, the layers and why a
  present was not warped.
- Title: the frame camera from the host visibility walk
  (`late_warp_note_camera`, `modern_controls.cpp`), the boundary
  (`near_clip.cpp`), the provider, the rotation and counters
  (`frame_replay.cpp`; `[TTK warp]` every 10 s); developer
  `DNTTK_LATE_WARP_TEST_YAW=deg` adds a fixed turn to every warp.
- Launcher: `run.py --late-warp HZ` (30-360, this launch only, Modernized),
  or `DNTTK_LATE_WARP=HZ`. Off by default; Vanilla never.

## 3. Correctness (MEASURED)

UI slot 2, still view, the user's 3x Relit settings, 120 presents a second:
1,186 of 1,199 presents warped (the 13 others during the load). A warp of
+3 degrees (`DNTTK_LATE_WARP_TEST_YAW=3`) of the unturned view against a real
3-degree turn without the warp (phase correlation over the upper world
region): no horizontal shift, 1 window pixel vertically, mean difference 5.5
grey levels (the water's animation and the clock); against the unturned view
the shift is 27 window pixels. The warp is geometrically right.

A 10-degree warp in UI slot 9 against a real 10-degree turn: 2 window pixels
horizontally, 1 vertically; the difference is the uncovered edge (section
5). Screenshots (local, not tracked: game pictures):
`recomp/analysis/d23z/d23y2-warp10-vs-turn10-s8.png` (warp above, real turn
below), `lwA-s1-w0.png` and `lwB-s1-w0.png` (3 degrees).

With the warp off (the default), still views in UI slots 2 and 9 against the
build before the spike: identical (the only differences are the ones two runs
of the same build show: the LEVEL STATS clock and the water's animation).

## 4. Measured (MEASURED)

30 s walks (walk, strafe, turn, fire), the user's settings at 2x and 3x,
LIGHTING RELIT, the real display, VSYNC off, three interleaved runs per
slot; pictures are the replay presents (the lab counted the guest's vblank
presents too before this job; fixed).

| Configuration | UI slot | Pictures/s (lowest tenth) | Game frames/s median (lowest tenth) | Emulation thread | All threads | GPU busy |
| --- | --- | --- | --- | --- | --- | --- |
| Warp 120, 2x | 2 | 120 (120) | 60.0 (59.6) | 62% | 68% | 76% |
| Warp 120, 2x | 8 | 120 (119) | 60.0 (59.4) | 76% | 78% | 77% |
| Warp 120, 2x | 9 | 120 (119) | 60.0 (59.4) | 61% | 67% | 73% |
| Original 60, 2x | 2 / 8 / 9 | 60 (59-60) | 60.0 (59.2-59.8) | 57-64% | 62-67% | 42-44% |
| Warp 120, 3x | 2 / 8 / 9 | 120 (119) | 51.0-59.7 (46.2-53.6) | 64-88% | 67-87% | 87-89% |
| Original 60, 3x (note 465) | 2 / 8 / 9 | 60 (59) | 60.0 (59.2-59.7) | 56-66% | 61-68% | 55-58% |

Input to picture (note 462's probe: XTest mouse turns, the window polled
through XShm; UI slot 2, 30 trials a run, medians):

| Configuration | Median | p90 |
| --- | --- | --- |
| Original 60, 2x (two runs) | 119.0 / 110.1 ms | 125 / 124 ms |
| Warp 120, 2x (two runs) | 20.1 / 19.7 ms | 30 / 26 ms |
| Original 60, 3x | 115.7 ms | 124 ms |
| Warp 120, 3x | 30.3 ms | 41 ms |
| Warp 60, 3x | 33.1 ms | 52 ms |

For reference on the same laptop (note 462): EDuke32 22 ms, UZDoom 40 ms.

Warp 60 at 3x (one run per slot): GPU busy 62-67%, all threads 66-79%, game
frames 60 (lowest tenth 59.1-59.5), but the replay timeline presented only
22-30 pictures a second: at the guest's own rate its deadlines fall in
phase with the guest's fields and every other one is missed (to fix in
D23Y3); the latency is already a third of the original's.

## 5. Artefacts

1. **Uncovered edges (the main problem).** A turn uncovers a band on the
   side the view turns toward; the spike clamps to the last column, so it
   shows as horizontal streaks. The warp does not cover one in-between
   frame but the game's whole pipeline: the image's camera lags the newest
   look by about 100 ms (note 462), so a steady turn keeps the warp 5-20
   degrees ahead (real mouse turns in the lab, section 8): a band of about
   15% of the width at 10 degrees and 26% at 20, all through the turn. A
   gap over 35 degrees is refused and that present snaps back to the old
   camera.
2. **Transparent layers.** Translucent world faces are drawn before the
   boundary and warp with the world. Anything translucent drawn after it
   (the HUD, a weapon effect) is shown as drawn, with the world behind it as
   it was in the game image: it does not turn with the view under those
   pixels.
3. **Moving objects and movement.** Rotation only: enemies, doors,
   projectiles and Duke's own walking still update at the game's 30 images a
   second, moved only by the turn; strafing shows no in-between motion. A
   positional warp needs the depth (a copy of the depth renderbuffer per
   frame) and object motion (D23N).
4. **A constant pitch offset.** Warped and real views differ by 1 window
   pixel vertically, about 0.07 degrees: the game camera's pitch against the
   newest look's when the mouse is still. D23Y3: calibrate it from still
   frames, or take the image's look from the game's input frame.
5. **No weapon, no warp.** The boundary is the first-person weapon's slot:
   third person, a hidden weapon, game-owned cameras (security monitors,
   scripted views, where `late_view` refuses) and menus present unwarped.
   D23Y3: slot 0's head (the HUD) as the boundary when slot 1 is empty.
6. **Presents on the emulation thread.** At 120 the presents (warp pass,
   picture, swap) run on the emulation thread: 3-12 points more than the
   original 60, so all threads reach 78% in UI slot 8 at 2x.
7. **The GPU at 3x.** At the user's 3x the HD 5500 is at 87-89% and the game
   slows (46-54 frames in the lowest tenth); 120 here needs 2x.

## 6. Against the targets

- 120 pictures a second at 2x: met (119-120 in the lowest tenth).
- CPU as at the original 60, all threads under 75%: met in UI slots 2 and 9
  (68%, 67%), not in UI slot 8 (78%); the game kept 60 frames in all three.
- GPU busy under 80%: met at 2x (73-77%), not at 3x (87-89%).
- Input to picture under 50 ms: met, 20 ms at 2x and 30 ms at 3x (from
  110-119 ms), the first time this title is as direct as EDuke32 here.
- Artefacts: listed above.

For D23Y3 (the player build decision): as built, the warp is not playable
(section 8). Its latency figure is real, but it is bought by showing the
picture up to 100 ms ahead of the image, which uncovers wide edges. A usable
form warps only the look change since the image appeared (at most one game
image, about 33 ms: a third of the edge), with less latency gain, or needs a
wider-than-screen image to warp from. Needed there:
edge fill, the pitch calibration, the HUD boundary for frames without a
weapon, presents off the emulation thread or cheaper, an OPTIONS row, and
the user's judgement of the 30-image world motion under a 120 camera.

## 8. The user's playtest (2026-11-13)

User: "late warp is completely unplayable. it stutters and glitches like
absolutely crazy." Their session (`--late-warp 120` at 3x): the game itself
fell to 44-46 frames a second (the GPU at 3x, section 4), and 1,623 of about
5,600 presents in the first minute were refused (drawn unwarped, a snap back
to the old camera). The host was quiet (no backup, update or other load in
the journal; on mains). Real mouse turns in the lab afterwards (2x,
xdotool steady turns): the gap between the image's camera and the newest
look was 0-5 degrees at a moderate turn and 5-20 degrees at a fast one
(1,281 presents at 5-10, 276 at 10-20), so the edge band in item 5.1 is wide
whenever Duke turns; refusals there were 9-15 (over 35 degrees), so the
user's turns were faster or the newest look was unavailable (the counters
now split the two). In the same session the user found FRAME RATE 120 (the
redraws) "playable", "how the real 60fps should actually feel": its governor
held it at every 2nd or 3rd refresh, about 44-46 redrawn pictures a second,
each with the newest camera and no uncovered edges.

## 9. The stutter's cause, found and fixed (2026-11-13, after the playtest)

The user's session after its first 10 s had every present warped, yet it
stuttered. Offscreen, a steady turn (test input) with 40 back-to-back
present shots reproduced it: the picture moved +5 window pixels a shot and
every few shots jumped back 30-35 and forward 40, a steady turn moving
backwards. A developer mode (`PSX_LATE_WARP_DEBUG=1`, magenta where the
final image is kept) showed those presents with 93% of the picture failing
the final-against-world test (2% normally: HUD and weapon), so they showed
the unwarped picture, a jump back by the warp's lead. Cause: the game starts
drawing a buffer's next frame while that buffer's image is still presented
(142 of 144 world copies landed in the layer on screen), and the final image
was re-copied at every vblank, so world layer and final image came from
different frames mid-display. Fixed: a ring of three world layers (a copy
never replaces the one shown) and the final image frozen at its flip while a
layer is shown. After: 2% kept in all 40 shots, no backward step in the
turn. The remaining limits are those of section 5 (the edge band on fast
turns, the GPU at 3x, the world at 30 images a second); the user has not yet
judged the fixed build.

## 7. Reproduce

`python3 recomp/tools/local/run.py --late-warp 120` (this launch only; with
the internal resolution at 2x for the measured case). Lab: `recomp/analysis/d23z/abn.sh 3 "1 7 8" w2 "LAB_DNTTK_LATE_WARP=120 PRES={\"internal_scale\":2}"`,
`CASES=warpcheck` with `LAB_DNTTK_LATE_WARP_TEST_YAW=3` or `TURNDEG=3`,
`shift.py`; latency: `recomp/analysis/d23x/lab.py` with
`ENV='{"DNTTK_LATE_WARP":"120"}'`.
