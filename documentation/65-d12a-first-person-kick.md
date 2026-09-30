# D12A - First-person quick kick

**Done - user accepted 2026-09-30:** "its done! accepted. ... this has been yet another amazing feat of engineering." In the Modernized eye view (D11), **Q** is now a
short Duke 3D style quick kick. Duke's own right leg (the original leg meshes)
snaps out in front of the eye, the view never leaves first person, and the hit
goes where the crosshair points. Any weapon stays drawn. Third person and
Vanilla keep the original kick.

User request (2026-09-30): no "duke-fu spin around and crazy moves" in first
person; something like the Duke Nukem 3D kick, using his leg, and more accurate
to aim.

## The original kick (verified)

Static reading of the owned executable plus isolated runs (private cards,
private port, headless):

| Piece | Address / value | Meaning |
| --- | --- | --- |
| Boot attack | `0x800517a4` | Requires `player+0x3b8 & 0x00ff00ff == 0` (holstered, Boot selected). Target search `0x80077fec`, then animation `112 + rand() % 4` (`0x800bb64c`), lower track zeroed |
| Callers | `0x800518d0` (idle Action, no object target) and the Modernized Q shortcut | In Modernized: Q, E, and a held left-click all reach it with Boot selected |
| Hit | Dispatcher `0x80048410` case `0x80049044` (table `0x800140a8`, entries 112..115) | Runs on **every player update** of 112..115 (callback `0x8004b594`, table `0x800c2754` +4, called unconditionally from `0x8005a4a8`) |
| Sphere | `0x800a979c(point, 96, 10, damage, player, 0)` | `point` = model point 7 = joint 16 (right ankle), from `0x8003964c`. `damage` = `[0x800d21fc] * 20`, or `* 80` with steroids (`player+0x364` bit 1). Tests actors (`+0x48` list) then objects (`+0x4c`), skips the attacker, and damages the first box the sphere touches through its class handler (`0x800c5ecc[obj+0x16]`) |
| Sound | `0x8006b270(0x1000, player+4, 0x800)` | On the kick animations' `0x800` event (`player+0xb8`); bank 1 entry 0 |

Measured on the first street (pig cop, 3750 health, 375 per contact at the
normal update delta of 10):

| Variant | What it looks like | Length |
| --- | --- | --- |
| 112 | High kick, Duke leans back and swings | about 75 frames |
| 113 | High kick, foot to chest height | 66 frames |
| 114 | (not captured in detail) | - |
| 115 | Straight front snap kick: knee to hip height (frame 40), foot straight out about 455 ahead at hip height (frames 45-51) | 66 frames |

The foot damages on every update it overlaps the enemy box: a point-blank 113
landed 5 contacts (1875); a point-blank 115 killed the pig cop (10 contacts,
some updates 500 or 1000 when the frame delta was larger). Duke's root steps
back about 64 and forward about 84 during 115. 112..115 have no eye-view
lease, which is why first person cut to the third-person camera.

## What D12A changes

Code: `recomp/src/ttk/kick.inc` (new, included by `modern_controls.cpp`),
small hooks in `first_person.inc`, `shortcuts.inc` and `modern_controls.cpp`.
No new hook addresses and no generated-C change: it uses the existing entry
hooks `0x800493a4`, `0x80058120` and `0x800292a0`. Everything is gated on the
live eye view (Modernized, independent camera, blend at least 0.5, orbit lease,
supported state, code identity).

- **Held attack with the Boot selected** starts the quick kick too, also
  while moving (first playtest: the original only kicks from a standing
  idle, so left-click with key 1 did nothing while running).
- **Chaining.** A Q pressed during a kick queues the next one, which starts
  as soon as the current kick ends; holding Q repeats, at the same pace as a
  held attack with the Boot (about 0.45 s per kick).
- **Q in the eye view** starts the quick kick with any weapon (or holstered),
  standing, walking, running, crouched or in a jump. It never touches the
  original equipment, animation or input. One kick at a time; about 0.45 s.
- **Original requests are converted.** With Boot selected, Q, E and a held
  left-click reach `0x800517a4`, which sets 112..115 with the lower track at 0.
  At the next lower-body initializer (`0x800493a4`, callers `0x8005a3a0`,
  `0x8005a490`, `0x8005a4d0`) the eye view restarts idle 63 exactly as the
  original transition does (animation, track and time words). Only when the
  attack is held does it start the quick kick; a request made by E (Cross is
  both attack and Action) kicks nothing (user playtest: only Q or the attack
  may kick). For that one update `state(camera_only)` accepts 112..115
  with the track still at 0, so the eye view does not drop. An already
  initialized kick, another caller or another actor is never touched.
- **Hit.** On every player update in kick frames 8..20 (about 4 updates) the
  original sphere `0x800a979c` is called with its own radius (96), type (10),
  damage formula and attacker. Its point follows the crosshair: the original
  query-only segment trace `0x8006d980` (the D07 aim query, exposed as
  `view_segment_query` under the aim code identity) runs from the eye 1200
  along the view; if it meets a surface or actor within 480 horizontally of
  the eye (the original foot reaches about 455 ahead of the root), the sphere
  goes 24 short of that point, otherwise 340 along the view. Revised after
  the first playtest: at a fixed 340 a low object that filled the crosshair
  (the alley garbage bags) was only hit when crouching, because the point
  stayed above it. Four contacts at the normal delta deal 1500 to a
  pig cop; three quick kicks kill one. The struck object's own class handler
  runs its hit reaction.
- **Sound.** The original kick sound is called once as the leg starts its
  snap (frame 7).
- **Leg.** During Duke's draw, joints 14..17 (right hip, knee, ankle, toe)
  get private matrices (the same mechanism as the D12 hand), so the loop draws
  Duke's own leg meshes in front of the eye; the near-clip viewmodel path
  draws them on top of walls. Poses are three keys recorded from 115 in this
  build (rest f0, chamber f40, extension f49; each joint's rotation as a
  quaternion in Duke's level frame and its position from the root), eased
  rest -> chamber (frames 0-7) -> extension (7-11) -> hold (11-17) -> chamber
  (17-21) -> rest (21-27). Shown pitched with the view, offset (-170, 80, 160)
  from the eye and turned 12 degrees in about the hip: the shin rises from the
  lower left and the boot lands just under the crosshair, clear of a drawn
  weapon. Duke's joint matrices are never written.
- **Thigh.** The thigh's own origin (the hip) is about 140 from the eye, inside
  the loop's near cull (GTE H 256), so the loop skipped the whole thigh (the
  shin floated). It is now drawn end for end: origin at the knee (which passes
  the cull), turned 180 degrees about an axis across the thigh, so the same
  mesh runs from the knee back toward the hip and off the bottom edge.
  `DNTTK_FP_KICK_THIGH=0` restores the floating shin for comparison.
  User tuning (2026-09-30): the thigh alone is nudged down the screen by 13%
  of the screen height (31 of 240 lines at its knee-end depth; 10% tried
  first, 0.13 chosen by the user); the calf and boot are unchanged.
  `DNTTK_FP_KICK_THIGH_DROP=0..0.5` overrides it for one launch.
- **Guards.** SHA-256 code guards added for `0x80049044` (the case's sphere
  arguments and damage formula), the table entries for 112..115, `0x800a979c`
  and `0x8006b270`.

Developer overrides (one launch, not saved): `DNTTK_FP_KICK=x,y,z` (display
offset), `DNTTK_FP_KICK_TILT=pitch,yaw,roll`, `DNTTK_FP_KICK_REACH=100..600`,
`DNTTK_FP_KICK_FREEZE=0..26` (hold every kick at one frame, for framing).
Debug JSON `ttk_input` -> `controls` -> `fp` -> `kick`: `active`, `frame`,
`kicks`, `converts`, `fire_starts`, `queues`, `repeats`, `suppressed`, `hits` (sphere calls, not confirmed
contacts), `surface_points` (hits placed at the crosshair trace), `sounds`,
`leg_draws`, `thigh_draws`, `cancels`, `reach`.

## Evidence

Binary `93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`
(copy `recomp/analysis/d12a-kick/d12a-candidate.bin`; previous binary
`before.bin`, sources before and after in `before-src/` and `after-src/`).
Isolated runs: private Xvfb display `:81`, port 9181, fresh private cards
(`recomp/analysis/d12a-kick/fresh-cards`, savestate slot 3: first street, god
mode, facing a pig cop about 520 away). Scripts in `recomp/analysis/d12a-kick/probes`.

- Q with the pistol drawn, facing the pig cop: 3750 -> 2250 (4 contacts),
  kick sound once, leg drawn, animation stays idle 63, eye view fallbacks 0.
  A second kick 2250 -> 625; the third kills it (reactions 70, 76, death
  215/217).
- Boot selected, E: animation never leaves 63 (the original request is
  converted), 3750 -> 2250, no fallback. Boot selected, held left-click for 60
  frames: repeated quick kicks (the held attack keeps requesting; each request
  during a kick is converted and ignored), 2250 -> 0, no fallback.
- Q while running forward: the run continues (72/74, 558 units in 30 frames).
- Q in a jump (animation 105): the kick plays, the eye view stays, fallbacks 0.
- Third person: Q armed does nothing (unchanged); Boot selected + Q plays the
  original 114/115 (a point-blank 115 killed the pig cop and it slid about 150).
- Framing captured with `DNTTK_FP_KICK_FREEZE` at frames 4, 7, 9, 13, 19, 23,
  and at frame 13 with pistol and gatling, level and looking up.
- Frame budget, same state, 15 s walking with mouse sweeps: 59.89 fps without
  kicks, 59.93 fps with 15 kicks, 0 audio underruns each.
- Vanilla regression route `analysis/vanilla-regression/d12a-vanilla` exit 0;
  captures show the original camera.
- Native: `ttk-controls-test` (new D12A case: conversion only for an
  uninitialized 112..115 from the initializer's callers on Duke; lease kept
  for that update; sound at frame 7; sphere arguments 96/10/200/Duke/0 at the
  eye + 340 along the view; 4-13 sphere calls; right-leg joints get private
  matrices, the left leg and head do not; nothing after the kick; Duke's joint
  matrices byte-identical; third person untouched), `ttk-aim-test`,
  `ttk-input-test`, `ttk-near-test`, `ttk-scene-test` PASS. Python tests OK
  (2 skipped).

### First playtest follow-up (2026-09-30)

User: "The kick looks amazing"; knockback and lower damage per kick accepted
("a sign of thoughtful tuning"); a trash can / garbage bags and the pallet by
the subway entrance should be kickable where the crosshair is; left-click with
key 1 should kick while moving; the missing thigh needs tuning and options.
Binary `e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`
(copy `d12a-candidate2.bin`).

- Alley garbage bags (type 219, private copy of the user's slot-5 state):
  approached to about 520 from the bag's centre, crosshair on the bag (pitch
  about 52 degrees down), Q: the bag is struck (health 1 -> 0, word 0 bit
  `0x4`) and disappears; the hit came from the crosshair trace. The user
  reported the fixed-point build only hit it when crouching.
- Boot selected, running forward, left-click held for 60 frames: two quick
  kicks, the run continues (761 units), 0 fallbacks.
- Pig cop: still 1500 per kick, all four contacts at the crosshair trace.
- Thigh captured at frames 7 and 13, on and off, level and looking down
  (`runs/thigh-sheet.png`).
- 59.95 fps with repeated kicks, 0 underruns; Vanilla route
  `d12a-vanilla-2` exit 0; native suites (new cases: crosshair point within
  and beyond reach, held attack with Boot and not with a weapon, reversed
  thigh at the knee) and Python tests PASS.
- The pallet by the subway entrance was not located in the private saves
  (the object near the stairs was not identified); it relies on the same
  crosshair trace.

### Second playtest follow-up (2026-09-30)

User: thigh drop 0.13 is the sweet spot; E pushed into a wall (W+D, Shift)
sometimes kicked, only Q should; Q should chain freely while running with a
weapon, like a held left-click with the Boot. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`
(copy `d12a-candidate4.bin`).

- Boot selected, E at idle: no kick, animation stays 63, 3 requests
  suppressed. Pistol, W+D+Shift into a wall, E four times: no kick (the
  user's exact case was not reproduced; the rule removes the E kick in every
  state).
- Pistol, running, Q ten times in 50 frames: kicks chain (2 queued); Q held
  for 180 frames while running: 7 kicks, 0 fallbacks.
- Native (conversion only with the attack held, E suppressed), aim, input,
  near and scene suites PASS; Python OK; Vanilla route `d12a-vanilla-3`
  exit 0.

## Limits

- **No shove** (accepted by the user). The original kick sometimes throws an enemy back about 100
  (its reaction 84 with its own root motion). With the same sphere, damage and
  attacker, the quick kick only produced reactions 70/76 and death; changing
  the hit height (eye, chest, hip) or reach (200, 340) did not bring 84 back.
  What selects 84 is inside each enemy type's AI callback and was not traced.
- **Damage per kick** (accepted by the user). 1500 per quick kick at the normal update rate, versus
  1875-3750+ for a point-blank original kick that takes 2.5 times as long.
  Contacts per kick depend on the update rate, as in the original.
- With nothing under the crosshair within reach the sphere is 340 along the
  view; like the original foot it is not blocked by a thin wall.
- Only the right leg is shown. The thigh is the original mesh drawn end for
  end (its knee end sits at the hip); it is large near the lower-left edge
  because the whole leg sits left of centre to clear the weapon. Framing of
  the thigh is still open. No left-leg or body motion; the weapon does not
  lower during the kick.
- Checked on the first street against pig cops only. Other enemies, breakable
  objects, other eras and the campaign are not covered.
