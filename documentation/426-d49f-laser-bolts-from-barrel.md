# D49F - Laser Gatling bolts fly from the drawn barrel

Status: Done (user-accepted 2026-11-09: "lock it in and close it out"). User decision: Option 3 (real launch from
the barrel), with Option 4 (gun toe-in) if the gun still looked off. It did,
so both are in.

## Report

User, after the D49E closeout: "in first person, the lasers appear to fire
from the direction of the gun and then bend to the direction of the
crosshair".

## Cause (measured)

- The Laser Gatling (record 28; launches projectile type 27, the stretched
  laser streak model 0x76a) launched from the third-person skeleton's muzzle
  `player+0xbc`. In first person the gun is drawn in its own presentation pose
  (D12/D36): the drawn barrel tip sits about 1050 units ahead of the eye, and
  the skeleton muzzle sat 400-850 units below and beside it.
- D49E draws the bolt's first tick from the drawn barrel to the bolt's first
  position. From the second tick the streak follows the real line from the
  skeleton muzzle to the crosshair point. So the visible path was barrel to
  first head, then the real line: a bend about one tick (2000 units) out.
- Lab, 180 Hz, first person, `boltlaunch off` (the pre-D49F build): bend at
  the first tick 14.5 deg median for taps (drawn barrel 340 units off the
  bolt's line), 4.3 deg held, 3.1 deg held while turning.
- After Option 3 the bolts were straight, but the gun still looked aimed
  elsewhere. Slot 7's framing (tilt pitch -12, yaw -26, roll 16, chosen in the
  2026-09-29 playtest so the Gatling's side shows) put the barrel's line on
  screen 30.4 deg off the bolts' line. The Gatling's rounds were invisible, so
  this never showed before the Laser Gatling's bolts.

## Change

`recomp/src/ttk/weapon_aim.cpp` (shot adapter at `0x8003c500`, gun path,
record 28, first person, Modernized):

- The origin is the drawn barrel tip, the point the muzzle flash uses (D49
  `first_person_drawn_muzzle`, view frame, recorded by Duke's live draw). It is
  placed in the world under the view the shot aims from: the D10G3 shown view
  at 120/180 Hz (`shot_view_rows`, new; `frame_replay.cpp` now keeps the shown
  view's whole rotation), or the game camera at 60 Hz. The direction stays the
  crosshair point's (screen ray), so the bolt flies one straight line from the
  barrel to the crosshair point.
- Guards. The existing body-to-origin cover check keeps a barrel pushed
  through a wall on Duke's side. A new eye-to-barrel check keeps it out of a
  floor or ceiling. If the bolt's line would leave more than 20 deg off the
  view, or meet the world short of the crosshair point, the origin steps back
  toward the eye (1/4, 1/2, 3/4, then the eye itself, which always reaches the
  point). A miss into open sky (room -1) counts as reaching. Such shots (close
  targets, the floor ahead when looking down) arrive within one tick, where
  D49E still draws the bolt from the drawn barrel.
- Why the 20 deg rule: looking down 36 deg, the barrel point sat 6 units above
  the floor. Its line to the crosshair point grazed the floor, and the game's
  sweep struck 30-90 units past the point, although the segment query said
  the point was reached. The step-back puts those bolts back within 1 unit.

`recomp/src/ttk/first_person.inc` (Option 4, display only): slot 7's tilt,
shared by the Gatling Gun and the Laser Gatling (record 28), is now (-12,
-8.8, 16), was (-12, -26, 16). This is the old pose turned in by yaw alone, so
the accepted raise and side view stay. At first only the Laser Gatling got
it; the user then asked for the regular Gatling in the same position
(2026-11-09: "duke should hold the non-upgraded gatling gun in the same new
position too"). The mesh's barrel (a hexagonal prism along the hand's +y,
y 120 to 396, centred on x 35, z 14, matching the muzzle vector (34, 400, 14))
now lies on screen along the line to the crosshair (0.0 deg, was 30.4). Its
axis meets the view ray about 2700 units out. A toe-in to 5000 units
(-8, -4.6, 16) also lines up but shows more of the gun's back. Override:
`DNTTK_FP_WEAPON_TILT7`.

Switch: console `boltlaunch on|off|default` or `DNTTK_BOLT_LAUNCH=0` turns off
both the launch and the toe-in (the old pose for both Gatlings), which is the
exact pre-D49F state. Redraw
workers take it from the render state (`RenderState::laser_toe_in`). Aim
status JSON: `bolt_launch`, `barrel_launches`, `barrel_refused`,
`barrel_guarded`, `barrel_steps`, `last_barrel_offset`. Diagnostics:
`DNTTK_AIM_TRACE=1` logs `ttk-barrel-launch`. `DNTTK_BOLT_BARREL_TRACE=1` now
also logs `kink` (deg) and `off` (drawn barrel to the bolt's line) per bolt
draw. `DNTTK_FP_AXIS_TRACE=1` logs the first-person hand axes, tip and weapon
mesh pointer.

## Evidence

Lab `recomp/analysis/d49f/lab.py` (local; offscreen, private card copies,
`dnupgrade`, apartment street):

| first person | `boltlaunch off` | on |
|---|---|---|
| 180 Hz taps: bend, median (max) | 14.5 deg (32.7) | 0.03 deg (0.31) |
| 180 Hz taps: drawn barrel off the bolt's line | 340 units | 1 |
| 180 Hz held: bend | 4.3 deg | 0.27 deg |
| 180 Hz held, turning: bend | 3.1 deg | 0.91 deg (max 1.35) |
| 60 Hz taps / held (game camera path) | not measured | 0.04 / 0.30 deg |

- The remaining 0.3-0.9 deg is the view and gun moving after the shot while
  the bolt keeps its world line, the EDuke32 behaviour D49D chose.
- Hits: impacts land 1-6 units from the crosshair point at a far wall, in
  both modes. Looking down 42 deg: 1 unit (step 1/2). Looking down 58 deg:
  median 0; one shot at a curb edge 34 units off, from the eye, an eye-ray
  edge case, not D49F.
- A pig cop pinned by the D10H2 test mover: 1580 units, 7/7 hits (step 1/4);
  2227 units, 6/6 hits from the barrel. Off: 6/6 at 1580.
- The regular Gatling logs the same pose matrix as the Laser Gatling (old
  pose with `boltlaunch off`); its rounds are unchanged (no barrel launches).
  Third person: no barrel launches.
- Suites: `ttk-aim-test` (new D49F case: the barrel origin, the regular
  Gatling, third person, no drawn barrel, `boltlaunch off`, the 20 deg
  step-back) and `ttk-input-test` pass.
- Matched stills (one session, `boltlaunch off`/`on`):
  `research/before-after/D49F/` (`fp180-laser-gatling*`, and
  `fp180-gatling-*` for the regular Gatling's pose).

## Limits

- Hits now start from the barrel point, so cover between the barrel and the
  target decides what the bolt meets, as it is drawn. Very close targets
  (nearer than about 1100 units, inside the barrel's depth) keep the original
  retraction to Duke's body.
- The barrel point is the live frame's (one game tick old view-relative pose,
  then placed under the shown view); kick or bob since then show as at most
  about 10 units.
- Enemy laser bolts (type 35), rockets and fireballs are unchanged (the
  D49B/D10H8 feel was accepted).
- Vanilla was not run; the code sits behind `input_modernized()` (asserted
  in `ttk-aim-test`).
