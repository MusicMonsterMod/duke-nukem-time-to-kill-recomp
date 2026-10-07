# D08O2 - weapon points forward while swimming and firing

2026-10-07. **Accepted as v1** (user: "it works and i accept this as a v1"); the more natural pose is D08O2A. User request (2026-10-06, after accepting
D08O1): "make the weapon point forward when in use when swimming and in
motion, as it currently points downwards."

## Cause

The player update (`0x80042634..0x80042764`) chooses Duke's model build:

| Test (`0x800426f4`) | Build | Arm aim |
| --- | --- | --- |
| `+0x224 & 0x200100`, or `+0x2b8` / `+0x2b4` / `+0x358` bits | `0x80097c04` (call `0x80042748`) | yes, under the same bits |
| otherwise | `0x800987cc` | never |

Inside `0x80097c04` (swim modes 4/5 take the default path) the aim helper
`0x80097a44` (call `0x80097f20`, ra `0x80097f28`) is applied per joint:

- `0x100`: joint `[model+0x2d]` (joint 1, which carries both arms) and then
  `[model+0x34]` (joint 9, neck and head).
- `0x200000` only: joint 9 alone.

Firing while floating (idle 127) sets `0x100`, so the original aims the arms
there and the existing D07A presentation hook gives them the view direction.
The thrust handlers (128-130), which D08O1 lets fire, never set it: `+0x224`
reads `0x2`, the build is `0x800987cc`, and the arms play the swim stroke.

Private lab (level 6 FAMILY JEWELS, private slots 9 and 11), joint
translations from `[player+0x3c]` projected on the camera forward, relative
to joint 9:

| Pose (Desert Eagle) | Gun hand (joint 7) |
| --- | --- |
| Floating, firing (original aim) | +243 |
| Swimming A + fire, before | -3 |
| Swimming W + fire, before | -23 |
| Swimming, no fire | +32 to -110 |

A first try with `0x200000` (chest only) still left the hand near 0: joint 9
does not carry the arms.

## Change (Modernized only)

- `swim.inc` (`swim_aim_begin`, `swim_aim_joint`, `swim_aim_restore`): new
  entry hook `0x800411b8` (`game.local.toml`, regenerated), the last call
  before the build test (a0 player, ra `0x80042634`). When Duke is underwater
  (state 5, anims 127-130/133) under the swim lease, a weapon out
  (`+0x3b8 == 2`), presentation ready (view aim, not original aim), neither
  `0x100` nor `0x200000` set, and fire held (or, after release, the shot's
  upper animation still playing), `+0x224 |= 0x100`.
- It is cleared at the `0x80097a44` call for joint `[model+0x34]` (the
  build's last aim; a0 is the joint's scratchpad matrix), so the bit lives
  only inside the player update between that call and the build. Fallback
  clears: the swim handler `0x800455bc`, `0x80055e80`, `0x800493a4`, the
  camera update `0x8003ade4` and the next `0x800411b8`.
- `0x800411b8` notifies the objects Duke touches (`+0x174`, `+0x17c`) through
  their callbacks. If such a callback would run, the bit is not set for that
  update (counter `swim_aim_contacts`; 0 in every lab run).
- The view direction comes from the existing D07A hook on `0x80097a44`; no
  new direction, matrix or animation code.
- Guards: `0x800411b8` (236 bytes), `0x80042634` (308); `0x80097c04` (1536)
  and `0x80097a44` were already guarded. Codegen hash unchanged; existing
  savestates load.
- Vanilla: no change (the set is behind `input_modernized()`; the clear only
  undoes our own write).
- Debug counters (`ttk_input` controls): `swim_aims`, `swim_aim_restores`,
  `swim_aim_contacts`.

## Verification

Private lab `recomp/analysis/d08o2-20261007/` (ignored): Xvfb display 96,
debug port 9352, copies of the D08O1 private profile, cards and savestates;
audio on a null sink. No player card, save or preference was used.

Gun hand along the view (max of joints 4/7), 40 frames each, after:

| Weapon | Floating fire | W | A | D | Ctrl | W, no fire |
| --- | --- | --- | --- | --- | --- | --- |
| Desert Eagle | 243 | 243 | 243 | 226 | 244 | 94 |
| Combat Shotgun | 203 | 199 | 210 | 205 | 212 | -92 |
| Gatling Gun | 116 | 111 | 119 | 123 | 109 | -111 |
| Pipe Bomb (fresh load) | 145 | 151 | 167 | 149 | 124 | -63 |
| Buffalo Rifle | 181 | 183 | 183 | 180 | 181 | -107 |
| Crossbow | 167 | 160 | 167 | 155 | 169 | -101 |

- Swimming continues (1265-1790 units per run, thrust 128) and shots stay
  adapted; `swim_aims == swim_aim_restores` after each run.
- Floating fire: no sets (the original's own `0x100` path, hand 243 as
  before). Surface W + fire: no sets, hand 239 (original surface firing).
- Same hand values at 120 fps (fast CPU timing).
- Screenshots (`shots/cmp2.png`, brightened): before, Duke's back faces the
  camera pitched head-down with arms in the stroke; after, the upper body
  faces the view, arms forward, legs still stroking.
- Regression: Vanilla underwater Up + Cross still stops Duke (0 units, no
  sets); Modernized ground W + fire moves with adapted shots, no sets.
- Suites: ttk-controls-test (LEVEL00 fixture, LEVEL01, all levels),
  ttk-aim-test, ttk-input-test, ttk-near-test, Python unittest (121, 7
  skipped) pass. ttk-inventory-test and ttk-font-test were not run (HUD
  only, need asset arguments; untouched).

Build SHA-256:
`653243025d6958cec86c8c0c3029f14f4e0eda34d07365d2ca897cd85693fc94`.

## Limits

- Body yaw still follows the swim direction (D08O1), and joint 1 turns the
  upper body to the view, so strafing or backing while firing twists at the
  waist; the legs keep the stroke.
- The jetpack route in the regression script did not take off this time
  (J left Duke in a jump), so flight was not re-driven; the change has no
  path into flight states.
- The holster key did not register under Xvfb; a holstered weapon fails the
  `+0x3b8 == 2` test by construction.
- Only level 6 was driven. Functional Xvfb runs are not feel or animation
  quality judgments; that is the user's playtest.
