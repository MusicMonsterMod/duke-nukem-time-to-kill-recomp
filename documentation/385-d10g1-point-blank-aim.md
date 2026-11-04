# D10G1 - Point-blank aim

2026-11-04. Status: Done (user closeout 2026-11-04: "great, lock it in and close it out"; defaults lift 0.35, cap 35). Follows the D10G report
([note 384](384-d10g-aim-mouse-feel.md)).
Executable: `aece128a2062afb9000d28bbf2f1072d9c5cef6ecd383c81f69c8e252b8a5062` (revision 2, lift; revision 1 was `ae1b0b9f...`).

## Problem

The muzzle sits about 300 units below the screen ray (and to the side), so
with Duke against a surface and the boom compressed (lens 450-520) the
crosshair point was a few hundred units ahead and well above the gun. Rounds
climbed steeply toward it: up to 71 degrees off the view in the D10G wall
sweep (median 44, 18 of 24 over 20).

## Change (`recomp/src/ttk/weapon_aim.cpp`)

Modernized, view aiming, third person, after the existing cover guard and
64-unit retraction:

- `d` = direction from the (possibly retracted) muzzle to the crosshair point,
  `c` = how far that point is ahead of the muzzle along the view.
- Blend `d` toward the view forward by `smoothstep((c - 128) / (512 - 128))`:
  fully along the view inside 128 units, the exact point beyond 512.
- Then cap the result at 25 degrees off the view (rotated toward the view in
  the same plane).

Both steps are continuous in `c` and in the crosshair position, so nothing
visibly switches (fixture sweep: 100:0.0, 200:4.7, 300:19.4, 400-600:25.0,
700:24.3, 1000:17.6 degrees for a muzzle 300 below and 100 beside the ray).

Not bent (exact point kept):
- the crosshair on a **targetable actor** (in its room's targetable list, the
  D22D rule), so a round at a close enemy still meets it;
- an assisted shot (`original-lock`);
- thrown weapons (original lob);
- first person (`first_person_view_live`), `original` aiming, Vanilla.

Walls and untargetable actor props bend: in the apartment fixture the
crosshair often lands on such a prop (`0x801dd2a4`, room 8) at the surface
in front of Duke. The body-to-muzzle cover guard, the 64-unit retraction,
beam locks (D22D) and all original projectile, spread and damage code are
unchanged. The drawn gun arm already aims along the view (upper-body look-at,
`0x80097a44`), so point-blank rounds and the arm now agree; at range the round
still converges a few degrees onto the crosshair point.

`DNTTK_POINT_BLANK=0` keeps the earlier point aim (developer A/B). Diagnostics:
`ttk-aim-shot ... bend=` and `point_blank_shots`/`last_bend` in the aim debug
JSON.

## Evidence (offscreen, private apartment card copies, pistol)

Local: `recomp/analysis/d10g/` (`agree.py`, `divergence.py`, `door.py`,
`look1.py`, `d10g1-*.json/png`). Player files hash-checked unchanged.

| Sweep | Before (D10G) | After |
| --- | --- | --- |
| Against a wall (`PRE=w:2.5`, 24 steps) | max 70.9, median 43.7, 18 over 20 degrees | max 25.0, median 1.0, 5 over 20 (all at or under 25) |
| Open room (24 steps) | max 45.7, median 7.3, 3 over 20 | max 13.6, median 4.5, 0 over 20 |

- World wall hits: 6 of the 7 rounds fired at the side wall (x 7167) still
  record an impact on it, now lower, near muzzle height. The seventh (step 1)
  aimed 2 units from the wall's end where it meets the prop: the bent round
  crosses that plane about 15 units past the end and records no impact
  (before, it hit). A parallel shot can slip past a corner the crosshair is
  right on; judge in play whether this matters.
- Rounds at the apartment prop record no world impact before or after: the
  pistol round takes one 3500-unit step through it (`door.py`). That prop
  never stops rounds; native, not introduced here.
- Long-range open-room shots: identical directions (bend 0).
- `ttk-aim-test` (real adapter, collision fixtures): new D10G1 group (close
  wall 4.7 degrees instead of 57.7, A/B off 57.7, first person unchanged,
  1000 units unchanged at 17.6, sweep never over 25 and continuous) plus all
  earlier groups pass.
- D08J6 climb fire (`d08j6/cand.py`, ladder `d35e` W, chain `d35f` S, wall
  `d35i1` S): draw, 6-7 view shots, climb, let go armed and land as before.
- Look screenshots (`d10g1-look-*.png`): arm pose unchanged.

## Not verified

- Real enemies at point-blank (no enemy spawn yet); the targetable exemption
  is covered by code and the fixture rule only.
- Other weapons in game (the rule applies to every gun and continuous weapon;
  fixture checks cover the gun callers).
- Player feel at 180 Hz on the real display, crouched cover, doorways.

## Revision 2: lift (2026-11-04)

User after playing revision 1: "i think the point blank positioning might be
a touch low? that's just my instinctive feel". Revision 1 sent close rounds
along the view from the muzzle, so they struck the wall at muzzle height.

Now the round aims at a point at the crosshair point's depth, `lift` of the
way from the muzzle's view-parallel level up to the crosshair point inside
128 units, rising to the crosshair point itself at 512 (smoothstep), then the
cap. Revision 1 is lift 0 with a 25-degree cap. Defaults: lift 0.35, cap 35
(revision 3, the user's choice; 0.25 in revision 2)
degrees. Session tuning in the backtick console:
`pointblank [lift 0..1 [max 5..45]|default]`; launch overrides
`DNTTK_POINT_BLANK_LIFT`, `DNTTK_POINT_BLANK_MAX`.

Wall sweep (`agree.py` with `EXTRA`), how far below the crosshair the bent
rounds strike the side wall, units:

| lift / cap | Below crosshair | Round off the view, max / median |
| --- | --- | --- |
| 0 / 25 (revision 1) | 112-154 | 25.0 / 1.0 |
| 0.25 / 25 | 95-154 (the cap limits it) | 25.0 / 24.7 |
| 0.25 / 35 (revision 2) | 46-112 | 35.0 / 24.7 |
| **0.35 / 35 (default)** | **40-132**, all 7 side-wall rounds hit | **35.0 / 32.6** |
| 0.5 / 35 | 31-101 | 35.0 / 34.7 |
| 0.5 / 45 | 31-101 | 45.0 / 34.3 |

Up close the cap, not the lift, sets the height, so a higher strike costs a
steeper round. The board's "about 25 degrees" target is relaxed to 35 for
the user's feel. Open room with defaults: max 14.4, median 7.3 (point aim:
45.7, 7.3). `ttk-aim-test` passes (lift 0 gives 8.3 degrees at a wall 200
ahead, default 26.8; console parse and refusals). Revision 1's numbers above
(sweep table, the corner round) are lift 0 / 25.

Console fix (same day): the backtick console typed `.` only from the numpad
with Num Lock; the main-keyboard period now types it too (framework
`runtime/src/main.cpp`, exported into the runtime patch, which applies to the
pinned clean dependency). User: "i cant use the . in the console". Executable
`eb1a3175897a0d503ad44b34c94abfccc86d3884f0700d2520dc02ce058cbe45`.

## Revision 3: the user's lift (2026-11-04)

After trying the console values: "it's gotta be pointblank 0.35 35". Default
lift 0.35, cap 35. Wall sweep: bent side-wall rounds strike 40-132 units below
the crosshair (91, 132, 86, 40, 79, 97, 95); all 7 record an impact, including
the corner round revision 1 lost. Round off the view max 35.0, median 32.6.
`ttk-aim-test` passes (wall 200 ahead 33.0 degrees). Executable
`54c62f65e96603be2f58e1e0332f2a385ce45cc02a6583c6d489460c23b0d748`.
