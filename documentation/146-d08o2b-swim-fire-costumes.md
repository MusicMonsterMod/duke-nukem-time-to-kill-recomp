# D08O2B - swim-fire aim on every Duke model

2026-10-18. **Accepted** (user: "it certainly is working, duke shoots in the right direction"; the brief-dive forward drift seen in the same playtest is D08O4). User report (2026-10-09): in player UI slot 3
the weapon points down again while swimming and firing; UI slot 2 still aims
forward. Modernized only; Vanilla unchanged.

## Reproduction

- UI slot 3 = savestate file 02 (saved 2026-10-09), SHA-256
  `ac80edb3b5e9095d76be371e80ee9db6bcd6a65f5ba311aa2f8df30778fe67c7`, copied
  read-only into the private lab `recomp/analysis/d08o2b-20261018/` (ignored).
  Duke floats at the surface of a level 2 sewer pool (mode 4, anim 122); Ctrl
  dives (mode 5), and swimming W + fire showed the bug.
- The D08O2A lab was level 6 (`slot11` copy), where the fix was tuned.

## Cause

D08O2A `swim_aim_arm` picked the shoulders by record number: records 2 and 5,
with record 1 checked as the chest at depth 2. That matches the level 6 model
only. Duke's models list their joints in different orders. The model header's
part -> record table at `+0x24` (already used by D11D/D12B first person and
kick) shows:

| Part | Meaning | Level 6 model | Level 2 model |
| --- | --- | --- | --- |
| 0x09 | chest (`[model+0x2d]`) | 1 | 10 |
| 0x0a | left shoulder | 2 | 16 |
| 0x0d | right shoulder (`[model+0x31]`) | 5 | 11 |
| 0x10 | head (`[model+0x34]`) | 9 | 15 |

On the level 2 model record 1 is a leg joint at depth 2 and record 2 its depth-3
child, so the shape checks passed for record 2 and the hook turned a thigh,
while record 5 (depth 2) was refused. The arms stayed in the stroke:
`swim_arm_aims == swim_aims` instead of twice it.

## Change

- `recomp/src/ttk/duke_model.inc` (new): `duke_joints`, the part constants,
  `duke_model()`, `duke_part_joint()` and `duke_joint_part()`, moved unchanged
  from `first_person.inc` and included before `swim.inc`. Adds
  `part_left_shoulder` (0x0a) and `part_right_shoulder` (0x0d).
- `swim_aim_arm` finds the shoulders and the chest by part; the flag clears
  after the later shoulder record. The depth, scratchpad and matrix checks are
  unchanged. No new hooks or guards; codegen hash unchanged.

## Verification

Build `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25`.
The lab measurement now also finds the head and hands by part (the D08O2A
script used level 6 record numbers). Gun hand ahead of the head along the view,
average of 8 samples:

| Level 2 (slot 3 copy) | Baseline `18597e61...` | Fixed |
| --- | --- | --- |
| Surface fire, idle / W / A (original aim) | 241 / 242 / 242 | 241 / 241 / 242 |
| Dive, W + fire (Desert Eagle) | 85 (arms 1 per build) | 221 (arms 2 per build) |
| Dive, idle fire | 234 | 220 |
| Dive, W without fire | 98 | 88 |

Fixed build, level 2, after Ctrl dive, W / A / D / Ctrl + fire:
Desert Eagle 223 / 223 / 215 / 243; shotgun 183 / 187 / 183 / 211; Gatling
96 / 101 / 95 / 153 (its floating-fire pose is about 100); pipe bomb
150 / 136 / 152 / 175. Chest axis matches swimming without fire (torso stays
in the stroke). Single taps (Desert Eagle, shotgun, Gatling) keep the arms
forward through raise / fire / lower, then return to the stroke.

Level 6 regression (D08O2A `slot11` copy), W / A / D / Ctrl:
Desert Eagle 227 / 229 / 224 / 257 (D08O2A 228 / 230 / 229 / 254); shotgun
193 / 194 / 191 / 225 (199 / 192 / 193 / 225); Gatling 110 / 108 / 97 / 152
(110 / 86 / 91 / 153); pipe bomb, fresh load each, 154 / 151 / 144 / 178
(152 / 145 / 152 / 174); crossbow 167 / 168 / 168 / 197 (166 / 167 / 168 / 195).
`swim_arm_aims == 2 * swim_aims` in every run.

Vanilla (slot 3 copy, Down + Cross dive, Up + Square): no arming.
Suites: ttk-controls-test (LEVEL00 fixture, LEVEL01, all levels),
ttk-input-test, ttk-aim-test, ttk-near-test, Python unittest (146) pass.
No framework edits.

## Limits

- The chase camera sees Duke from behind in a dark pool; the pose is the
  user's playtest.
- Only level 2 and level 6 models were driven. The part lookup covers any
  19-joint Duke model whose table agrees with its records; other costumes
  underwater are unverified in play.
