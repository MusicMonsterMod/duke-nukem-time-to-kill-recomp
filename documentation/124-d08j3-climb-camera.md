# D08J3 - Free camera on ladders, poles and chains

Status: **Accepted** (2026-10-07, user: "we have an absolute winner once again!! it's working well, very fun, looks great"). Modernized only; Vanilla unchanged.

User request (2026-10-07): "free camera while on ladders/climbing, and of
course this will require deep investigation and user friendliness testing."

## What owned the camera

Private runs (`recomp/analysis/d08j3-20261007/cam.py`, `van.py`) on copies of
UI slot 5 (alley ladder, file 04) and UI slot 3 (medieval chain, file 02):

- The climb never leaves the normal camera. `camera+0xa4` stays Duke,
  the look target stays his pivot (`camera+0x4c`/`+0x50` = `player+0x7bc`) and
  the normal camera update `0x8003ade4` (from `0x80025ee8`) keeps running.
- On the ladder (mode 3, 188/189) the original only swaps the boom: local
  offset `camera+0x94..0x9c` (0, -384/-512, -3000) becomes (0, 2000, -2000)
  and the target height `camera+0x88` -256 becomes -512, a high camera looking
  down (pitch about -50 degrees) that eases back to the follow boom at rest.
  Vanilla shows exactly this, with `orbits` 0.
- The host orbit already covered the ladder mount (185), the top exit (190),
  mantles, hangs and ceilings (D08V `traversal_camera_early`). It stopped only
  in mode 3: the orbit counter froze there and the camera lease dropped, so
  the original boom took over. Mouse motion during the climb was kept and
  applied after the climb ended.

## Change

- `modern_controls.cpp` `climb_state_early()`: mode 3 with animation 147..156
  or 185..211 (the attached set `traversal_state_ready` accepts), previous mode
  0/3/6/7/8/9 so the entry frames are covered. It joins
  `traversal_camera_early()`, so the camera-only lease holds through the climb.
  The mouse orbit, distance (Alt+wheel), shoulder and V recentre then work as
  on the ground. No facing, gait or root-motion writes
  (`locomotion_input_ready()` excludes these states); the original climb
  handlers still move and turn Duke. In the first-person profile the view
  steps out to the orbit during a climb (the D08V rule for hangs; the eye
  would be against the ladder wall).
- Inputs are unchanged on ladders: W/S still send Up/Down. The D08V
  fall-button block only acts on entry frames, as the tank fallback did
  before.
- Poles and chains (`ladder_top.inc` `pole_view_from_front()`, used by
  `pc_input.cpp`): with the camera free, Duke can be seen from his front,
  where the D08J2 swap reads backwards. The side is the view yaw against his
  facing (he faces the pole); beyond 90 degrees the swap is undone. It is
  re-read only while A/D are up, or on the first frame after a gap, so a held
  sidestep never reverses when the camera swings past his side.

## Evidence (binary `2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`)

Private Xvfb runs, real keys and mouse, isolated profile and card copies
(`recomp/analysis/d08j3-20261007/`):

| Check | Result |
| --- | --- |
| Ladder, third person (`lad.py`) | at rest on the ladder the view follows the mouse through +60, +120, 180 degrees and back, pitch -35; orbit counter advancing; W then climbs to the top exit (188/189/190) with the camera in front of Duke |
| Ladder, first-person profile | same orbit (`fp` reason `unsupported`, blend 0) during the climb; climb completes |
| Chain (`chain.py`), 40 frames per key | A / D, screen-relative X: behind -210 / +232, front -195 / +170, side -181 / +179, behind again -235 / +193; Duke stays on the chain (193, mode 3); W climbs (Y 2117 -> 1789), S descends (1758 -> 1884) |
| Front view (`front.py`) | the boom swings round Duke and settles about 1,200 units in front of him on the chain (room wall), 1,900 on the ground at the same place; Duke and the chain in view |
| Ladder regression (`regress.py`, third and first) | W+E climb past the LARD at the top (`actor_exits` 1), E top mount (156 -> 186), S descent to the floor: same states as the D08U2 baseline |
| Vanilla (`van.py`, raw pad) | ladder climb 188/189 with the original (0, 2000, -2000) boom; `orbits` 0 |
| Suites | `ttk-input-test` (new D08J3 case: front view unswaps pole A/D, held key keeps its side), `ttk-controls-test` (`d08-camera-final` LEVEL00 fixture, LEVEL01, all levels; 37 PASS groups; the D08U attached-ladder assertion now expects the camera lease), `ttk-aim-test`, `ttk-near-test` PASS; Python 121 OK (7 skipped) |

## Follow-up: centred view on climbs (binary `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`)

User playtest of `2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`: "it works perfectly! the only thing i think we
should add is that regardless of whether the player is using over the
shoulder view for the third person, it should centralize the view on climbs".
`orbit_begin()` now eases the D10 shoulder offset to 0 while
`climb_state_early()` holds (mode 3) and back to the H setting afterwards; the
saved preference is not changed. Private chain run (`shoulder.py`): shoulder
offset 640 (or -640) on the ground, 0 on the chain, back to the setting after
letting go and landing. Suites pass (37 controls groups).

## Behaviour changes to judge in the playtest

- The camera no longer snaps to the high look-down view while climbing. Where
  that view helped (seeing the top of a tall ladder), look up with the mouse
  or press V to swing behind Duke.
- After stepping off a ladder, held S walks away from the camera's current
  view, not from where the original camera put it.
- Narrow spaces: side views on the alley ladder sit against the alley walls,
  as when walking there.

Candidates if the free orbit is not friendly enough: automatic recentre
behind Duke when a climb starts, or a limited look range around the original
boom.

## Limits

- One ladder (slot 5 alley) and one chain (slot 3) played. Other ladders,
  poles, chains and climbing walls (198..205) share the mode-3 state rule;
  climbing walls were not played.
- Ceilings (mode 7) were already on the orbit (D08J4), not changed.
- Both slot-5 runs that idled on the ladder ended with the LARD at the top
  shooting Duke; that is the level, not the change.
