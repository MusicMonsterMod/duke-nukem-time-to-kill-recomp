# D14G1 - Swim ascend/descend: arms judder and legs in first person

Job: [D14G1](../jobs/first-person.md#d14g1---swim-ascenddescend-arms-judder-and-legs-show-in-first-person).
Status: Done (user-accepted 2026-11-19: "lock it in and close it out. great work"). Evidence labels as note 257: S source/tests, L lab runtime, U user.

## 1. The report (U)

"when swimming in first person, when using space or ctrl to ascend or descend,
duke's arms judder during the initial button press. this is the only time when
duke's legs become visible when swimming. if this is shown near a boundary like
a wall etc, the legs can appear at the torso cuttoff point on screen completely
disembodied, and in worst case scenario, even backwards."

## 2. Cause

S/L, arms: underwater (native mode 5) the host steering (`swim_steer_underwater`,
D08O) wrote the body pitch `+0x1e/+0x26` straight to the input's target:
+900 for Space, -900 for Ctrl, the look pitch otherwise. The pitch also stays at
the last thrust value through idle 127, so a press often flipped it by 1800
units (about 160 degrees) in one game update. The whole skeleton turned about
the root in one tick while the D11E swim eye stays root-anchored. Holstered, the
D14G contact rig follows the native hands, so the arms jumped (180 Hz present
trace, level 6 moat, private copies: contact hand moved about 300 view units in
one tick at the press; hips stepped at tick boundaries with a boundary/interior
ratio of 21-24). Armed swimming pins the weapon hand to the view (still).

L, legs: D36J5 revision 3 admits the shared legs while swimming (48-unit setback,
torso hidden). With the body stood up (Space) and the view looking down, the
thighs rose into the bottom of the picture without a torso; diving (Ctrl) put
them around the root-anchored eye, which is how they could show backwards near
walls. Horizontal strokes keep them out of view, which matches the report.
A ladder-style hip-clearance setback (D36J5 r5) was tried first and did not
help: the hip joint origins were already below the picture; the thigh mesh
above them was what showed. It was removed.

## 3. Change (Modernized only)

- `swim.inc`: `swim_target_pitch()` is the former pitch choice. The drawn body
  pitch now steps toward it from the guest's own value, half the gap and at most
  96 per steering pass (two passes per update): about 0.15 s from level to
  vertical, 0.3 s for a full flip. Stateless (no host state; savestates and
  replay unaffected). `swim_pace` (0x80066e34, ra 0x80046ec8) writes the target
  pitch into the thrust's stack copy for thrust 128-130, so the travel direction
  is immediate: no added ascend/descend delay. Third-person swimming turns the
  same way. Developer A/B: `DNTTK_SWIM_PITCH_EASE=0`. Counter `swim_pitch_eases`.
- `first_person.inc`: in the Swimming action with native mode 5 the ordinary
  legs are claimed `HiddenBody` with the camera matrix (no depth bridge), as the
  hidden torso. Surface treading (mode 4), every other action, the Q/door kick
  chains, third person, reflections and Vanilla are unchanged. Developer A/B:
  `DNTTK_FP_SWIM_LEGS=native`. Counter `fp.swim_legs_hidden`.
- Developer trace: `leg_hip_l` for `DNTTK_PRESENT_TRACE` (present_trace.py).

## 4. Evidence

L (ripspace, offscreen SDL on the real GPU, private profile/cards copies, level 6
moat, holster bound to Z in the copy; `recomp/analysis/d14g1/`, local):

| 180 Hz, holstered | base max hand step | fix | base hip ratio | fix |
| --- | --- | --- | --- | --- |
| Space underwater | 2125 | 106 | 24.1 | 1.4 |
| Ctrl underwater | 2508 | 180 | 21.0 | 1.5 |
| Space looking down | 1789 | 316 | 4.6 | 2.1 (legs hidden) |

(present_trace.py --view rates; largest raw per-redraw hand move after the fix
about 45 units, comparable to the stroke itself.) Vertical travel at 0.2-1.0 s
matches base within sampling jitter. Matched stills (`runs/ab2.png`): base shows
thigh pieces at the bottom centre when ascending/looking down and beside the
wall; the fix shows none, the contact arms stay connected; surface legs return.
60 Hz: same easing. Zero first-person fallbacks. Player files hash-checked.

S: `python3 -m unittest discover -s recomp/tests/local` 266 OK; `ttk-input-test`
passes. `ttk-controls-test` and `ttk-near-test` do not link on this tree
(`g_psx_rw_lean_suspend` undefined from `near_clip.cpp`), unrelated to this
change and not built by CI; no new native test was added for that reason.

Limits: one moat (level 6), default costume, offscreen only (no real-display
look), the wall case near one moat wall. Human review of the motion needed.

## 5. Revision 2 - surface swimming beside a wall (player UI slot 9)

U (after accepting revision 1, video 2026-11-19 16:12): the issue is worse
when surface swimming; "he ends up behind his own disembodied legs".
Revision 1 is committed as a local checkpoint.

L (private copy of the player's cards, UI slot 9 = runtime 8: level 6, surface
idle 122 at mode 4, armed, beside the channel wall): looking down to about 60
degrees, the eye jumped in one update from 290 above the root to 108 above it
and about 55 back (84 below the 8192 surface) and stayed there at rest; Duke's
shorts then sat in the centre of the view. The native camera solver casts rays
from Duke's pivot to the requested eye plus its forward distance; beside the
wall they hit and the nearest hit pulled the eye toward the pivot (the D36H9
hang problem). Even with the eye in place, looking down showed the open tops of
the shorts at the bottom of the picture, most on the backstroke 126.

S, change:
- `swim_eye_owner()`: a fully blended surface swim eye (mode 4) takes its
  requested position after the native solve, through the D36H9 path
  (`unsolved_eye_owner()`). The eye stays over Duke's root, inside his hull.
  Underwater keeps the native solve. A/B: `DNTTK_FP_SWIM_EYE=native`.
- Surface legs use the D36J5 r5 ladder clearance (`leg_hip_clearance()`,
  factored out, ladder behaviour unchanged) with a 240-unit margin: the shorts'
  mesh rises far above the hip joints. A stroke the 192-unit setback cannot
  clear hides the legs for that draw (HiddenBody, as underwater). Tuning A/B:
  `DNTTK_FP_SWIM_LEG_MARGIN=<units>` (160 left a shorts edge on the backstroke;
  320 hid about 20 times as many draws for no visible gain).

L, evidence (slot 9 copy, 180 and 60 Hz, offscreen): eye displaced in 0 of 274
/ 267 samples (base: the whole look-down rest); matched stills (`runs/s9c-ab.png`,
`runs/s9m-ab.png`) show no shorts in view at rest, forward, back and side
strokes or at rest after them; about 110 hidden leg draws per run (backstroke).
The underwater moat route still hides legs and eases pitch, no fallbacks. Player
cards were only copied, never written (hash check).

Limits: one surface spot (slot 9) and the level 6 moat; no real-display check.
