# D17AY - Full-refresh presentation: draw-time readers of tick state

2026-11-02. Done: accepted by the user ("great work, fully accept - the
animations looks super smooth"); closeout, commit and push authorized. Labels: U user report, S source/native tests,
L isolated runtime observations.

## Problem

U (while accepting D35D1): "should we stand up a backlog job to spec
everything up to the full refresh rate rather than having various animations
etc interpolate at 30hz?"

Game logic, physics and animation stay at the original 30 Hz tick. Render
replay ([note 80](80-d17-high-refresh-audit.md) and later D17 notes) redraws each
game frame at the present rate in worker processes: the camera, Duke's joint
matrices and other objects' transforms are interpolated, but every other guest
field is the last tick's, and host state reaches a worker only through
`RenderState`. Host code that, while drawing, combines tick-only state with
the interpolated view steps at 30 Hz against it. D35D1 revision 2 (push grip)
and D14H (body camera) were two instances.

## Inventory

Every draw-time host path in `recomp/src/ttk/` was read: hooks inside the
composition `0x80026164` (actor draw `0x800348d8`, list step `0x8001ca4c`,
joint transform `0x800292a0`, weapon/flash/held-item `0x80033e40`,
`0x800341e4`, `0x80033f5c`, mesh begin, projection load `0x800b4d9c`, HUD
`0x8008ba30`, near clip, host visibility, draw distance, sky, water, poles)
and `frame_replay.cpp` itself. Classification:

### Fixed

| Reader | Tick state it read | Fix |
| --- | --- | --- |
| Push grip `push_grip` (`first_person.inc`) | Duke +4..+12 and +0x1c, object box from +4..+12 | Duke and the object from the shared in-between pose (vertical anchor, face, width, the 121 hold blend) |
| Third-person dissolve `third_person_body_coverage` (`modern_controls.cpp`) | Duke +4..+12 against the interpolated eye | Duke's in-between position (developer tight camera `DNTTK_TIGHT_CAMERA=1` only; not in the player build) |
| First-person recoil `kick` (`prepare_display_matrix`) | decays per game frame; blended only for slot 6 | every weapon's recoil eases back between ticks (a new shot still starts at its muzzle flash) |
| Quick-kick leg `kick_leg_joint` (`kick.inc`) | integer kick frame per game frame | in-between leg frame (`kick_draw_frame`); attack clock, hit and sound keep `kick_frame()` |
| Door-kick leg `door_kick_frame` | channel 0 keys and timer +0x62..+0x66 | frame captured with each game frame and blended |
| Crouch/swim animated hand `animated_weapon_channel` | channel 1 keys and timer | hand captured with each game frame and blended (normalized quaternion, position) |
| Eye projection `first_person_projection` | `fp_blend`, stepping during the 0.3 s view change | in-between blend |
| Jetpack flame re-arm (`0x800348d8`) | `jet_flight_live` from the worker's fork | carried in `RenderState` (correctness, not smoothness) |

### Already interpolated

Weapon display pose (fire poses, live torso/hand from interpolated joints,
rifle reload and running arms already blended), display arms, contact arms
other than the push grip (native hands and the eye rig), first-person legs,
torso and hidden joints, unarmed lowering, the Bio Mask bob (`mask_bob`
blended), host visibility, draw distance and open sky, near clip and water
clip (per-draw GTE state), model depth refinement (falls back for moving
objects), sky geometry (rebuilt from the worker's camera), widescreen and HUD
layout, reflections (native, with Duke's interpolated joints).

### Intentionally tick-rate

Discrete decisions: animation ids and gates (+0x60, +0x22c, +0x3b8, +0x35c,
+0x854), perspective policy, head/backpack hide flags, limb ownership, pole
visibility, the eye room walk (`duke+4` and its room are a consistent pair;
the result is a room index), HUD values and the inventory overlay (fixed
screen positions), the red dot (native, only hidden by the host), the
crosshair (screen centre). Time animations that are not spatial: the sky
cloud phase `0x800c0c64`. Native actor culling (guest code) is outside host
scope.

## Shared source

`frame_replay.cpp` records each published frame's object poses (position
+4..+12, heading +0x1c) for Duke, the objects he holds or climbs (+0x290,
+0x17c) and the drawn actors (24 at most), and every redraw job carries the
in-between poses (a jump over 1536 units keeps the base pose; heading by the
shortest turn). Draw hooks read them through
`draw_object_pose(object, pos, yaw)`, which in a live draw (or for an
unrecorded object) reads guest memory, so live frames, 60 Hz and Vanilla are
unchanged. Tick-rate host values that are not object poses use the existing
`render_state_blend_mask` path. `DNTTK_DRAW_POSE=tick` (developer, one
launch) restores the pre-D17AY readers for comparison.

## Diagnostic

`DNTTK_PRESENT_TRACE=1` (developer) makes redraws log traced presentation
quantities (`[present]` lines: base frame, fraction, the point in the view of
the draw that showed it): the push hands and object, the weapon hand, the
kick foot and frame, the eye projection and the third-person dissolve.
`recomp/tools/local/present_trace.py LOG` orders redraws by base frame and
fraction and reports, per quantity, the mean movement into the first redraw
of a new game frame (boundary) and between redraws of one game frame
(interior). A tick-rate reader moves only at boundaries (interior about 0,
ratio very large); a smooth one moves at both. `--relative NAME` measures
against another traced point (hands against the held object), `--view` in
view units (points near the eye, scalars), `--segment` selects probe steps.

## Verification

S: player build (executable
`972de2e2f0ef2e106d04db074da26fd187a7c311d40b1f902e8ff590adb6be23`); native controls (121
groups; new D17AY checks: the push grip follows the in-between Duke and
object in a redraw, including a slope step, where the last tick would leave
it 30 units off; the third-person dissolve follows Duke's in-between
position), near, aim and input suites pass.

L: private card copies (`recomp/analysis/d17ay/cards`, from
`saves/local-play`), Xvfb/OpenGL, dummy audio, the player's Corrected
precision, FOV 90 and camera distance 1728, debug-port driver
`recomp/analysis/d17ay/probe.py`; player save/card/profile hashes checked
before and after every run (unchanged). UI slot 12 (LEVEL00 alley): grab,
push, pull and turn at the dumpster; pistol fire; three quick kicks; view
out of and back into first person. Each case ran with D17AY and with
`DNTTK_DRAW_POSE=tick` (the old readers), at 120 and 180 Hz.
`present_trace.py` rates, movement per game frame (boundary / interior):

| Quantity | 120 Hz before | 120 Hz after | 180 Hz before | 180 Hz after |
| --- | --- | --- | --- | --- |
| Weapon hand while firing (px at 2560) | 4.49 / 0.00 | 2.81 / 2.32 | 5.46 / 0.00 | 3.49 / 2.21 |
| Quick-kick foot (view units) | 137.1 / 0.38 | 119.7 / 107.8 | 141.9 / 0.37 | 135.3 / 119.6 |
| Quick-kick frame | 2.00 / 0.00 | 1.80 / 2.16 | 1.92 / 0.00 | 1.92 / 1.65 |
| Eye projection H, view change | 6.21 / 0.00 | 2.71 / 2.46 | 5.32 / 0.00 | 2.93 / 6.63 |
| Push hand against the object (view units) | 2.42 / 2.10 | 1.96 / 2.01 | 1.59 / 7.90 | 1.73 / 8.04 |

Before, each quantity moved only when a new game frame began (interior 0):
the 30 Hz step. After, it moves between redraws at a similar rate. The push
grip on flat ground was already smooth (D35D1 anchors it to the eye) and
reads the same in both modes; its slope case is covered by the native test.
At 60 Hz no redraws run and nothing is traced (smoke run, no failures).
Replay reported 0 misses and 0 failed jobs in every run.

## Limits

- Offscreen software runs fall behind (often one or two redraws per game
  frame instead of four or six), so rates are averaged over uneven redraw
  fractions; the boundary/interior comparison, not absolute values, is the
  evidence. Not a frame-perfect visual proof; dummy audio.
- A new shot's recoil still starts at its game tick, with the muzzle flash.
- The door kick (D35K) and the crouch/swim animated hand were not exercised
  at runtime; they use the same captured-and-blended path as the quick kick.
- The third-person dissolve is a developer path; the route did not bring the
  camera close enough to engage it (coverage stayed 1), so its fix rests on
  the native test.
- Native actor culling, the sky cloud phase and HUD values stay tick-rate.
- Vanilla is unchanged by construction (it never runs redraws), and live
  frames read the same guest values as before; Vanilla was not launched.
- U: the user's playtest found the animations "super smooth"; the last two
  player session logs show no failures.

Files: `recomp/src/ttk/frame_replay.cpp` (pose table, `draw_object_pose`,
`replay_redraw_active`, present trace), `modern_controls.cpp` (`RenderState`
fields and blends, dissolve), `first_person.inc` (push grip, animated hand,
projection, weapon trace), `kick.inc` (leg and door frames),
`modern_controls.h`, `recomp/tools/local/present_trace.py`, native test
`recomp/tests/local/modern_controls_native.cpp`. No framework change.

```sh
python3 recomp/tools/local/run.py
```
