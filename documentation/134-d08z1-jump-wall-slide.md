# D08Z1 - Keep jump momentum when bumping a wall

Status: **Done** (2026-10-08, user-accepted: "mechanically this feels significantly better, where we can call the actual job as accepted."). Known follow-ups: D08Z2 (mantle pose regression; Done, [note 135](135-d08z2-mantle-upper-body.md)), D08Z3 (stuck after some landings). Modernized option `jump_walls`,
`slide` by default; `original` and Vanilla keep the original bounce.

## User request (2026-10-06)

"i dont want duke to lose his motion when you bump into a wall when jumping, i
want that behavior to be more like eduke also, duke should be able to jump
into a wall without consequence."

## What the original does (static analysis of the owned executable)

The running/directional airborne handler `0x80055904` calls the reach
`0x800557f8`, the ballistic integration `0x8003ebf4` (delta
`+0xfc/+0xfe/+0x100` = velocity `+0x1f4/+0x1f8/+0x1fc` * dt / 1024, gravity
added to `+0x1f8`, positive is down) and the flight sweep `0x8007a98c`, then
acts on the sweep's result:

| Result | Meaning (finalizer `0x80079f4c`) | Original response |
| --- | --- | --- |
| 0 | clear | position += delta |
| 1 | wall: normal `+0x194`, reflected direction `+0x1b4`, `+0x1c0` = angle between the reversed horizontal motion and the normal (0x400 = 90 degrees) | angle <= 45 degrees or >= 90: `0x8003ef08` (zero a rising vertical speed, velocity = reflection at half speed, rerun the integration), animation **107**, rumble `0x8001e840`, sound `0x2008` (and `0x200b` at random). 45..90 degrees: `0x8003ef78` (velocity = reflection * (angle/1024 - 0.25), i.e. 0.25..0.75 of the speed, away from the wall), same rumble and sound. |
| 2 | floor | landing animation `0x80054c04` |
| 3 | ceiling | steep (`0x80077408`): bounce as above with 107; otherwise `0x8003efd0`: while rising, zero the rise and rescale the horizontal speed by the stale `+0x1c0` angle |

In every wall case the jump's travel is gone: head-on Duke is thrown back and
staggers; glancing he is deflected away at a fraction of his speed. The rerun
integration also adds gravity a second time in that update.

## Change

`recomp/src/ttk/jump_walls.inc`, a second entry plugin (`ttk.jump.walls`) at
the existing `0x8003EBF4` hook, registered after `ttk.modern.controls`, so it
sees the velocity after this update's takeoff redirect, D08Z air steering and
delayed vertical. Only Duke's call from the airborne handler (ra
`0x80055934`), mode 9, Modernized with the guards (`player_identity_ready`).

1. Save Duke's state (`0x8a4` bytes), run the original `0x8003ebf4` and
   `0x8007a98c` in isolation (`original_call`, as the D08X reach retries do),
   read the result and normal, restore everything.
2. Wall (result 1): remove the velocity component into the wall and leave a
   small outward drift (128 velocity units, about 1.4 world units per update)
   so the slide does not keep touching it. If Duke is already moving along or
   away from it and still touches it (a 45-degree wall in level 12 did this),
   push off harder each pass (256, 512, ...).
3. Ceiling while rising (result 3): stop the rise, keep the horizontal speed
   (minus any part into a sloped ceiling).
4. A second clip handles the crease of two walls. If a corner still blocks
   after that (a convex corner, below), turn around it instead.
5. The original update then runs with the new velocity; its own sweep checks
   the slide. No bounce, 107, rumble or contact sound, and no second gravity
   tick.

A hit the pre-pass cannot resolve (a near-vertical normal) is left to the
original. Falls into a ceiling and landings are untouched.

Ledge helpers that started on the first updates of a bounce (D08X crate jump
mantle, D08X/D08Y lifted/lowered acquisition retries, D08Y low-lip step-up)
now also start on the first updates after a slide (`jump_wall_recent()`); the
crate mantle uses the direction Duke was heading before the slide.

Option: profile schema 30 `controls.jump_walls` `slide` (default) or
`original`; `run.py --jump-walls`, `--settings` choice K, `DNTTK_JUMP_WALLS`
(always `original` for Vanilla). Guards added for `0x8007a98c` (884 bytes)
and `0x80079f4c` (248 bytes). No new hook, no generated code change, codegen
hash unchanged (the player's savestates load).

Diagnostics: `ttk_controls.jump_walls` (`slides`, `corners`, `stops`,
`ceilings`, `unsolved`, `nudges`, `passes`, `age`); `DNTTK_TRAVERSAL_TRACE`
prints `ttk-jump-wall` lines.

## Evidence

Executable `c91a3ef128d9f98d923efe858fef75f432b8d729b2ffe5f16654f0034a56b627`. Private copies of the player's savestates and blank
private cards only (`recomp/analysis/d08z1-wall-momentum`, Xvfb, dummy audio).

**Wall jumps, eight headings 45 degrees apart** (`walls.py`; running = Shift+W
then Space, standing = W+Space, W held through the flight):

| Level (UI slot) | Jumps | `original`: 107 bounces | `slide`: 107 bounces | Example airborne travel, original -> slide |
| --- | --- | --- | --- | --- |
| Level tag 3 (slot 5) | 8 running + 8 standing | 2 + 6 | 0 + 0 | glancing running jumps 739 -> 1900, 756 -> 2171, 1027 -> 2429 units |
| Level tag 12 (slot 12) | 8 + 8 | - | 0 + 0 (two re-touches on a 45-degree wall bounced until the push-off was added) | standing 225/270: 892/837 -> 1019/1037 |
| Level tag 4 (slot 9) | 8 + 8 | - | 0 + 0 | - |

With `slide` the flights keep their arc (9..15 airborne samples where the
bounce cut them to 5..7). Head-on jumps stop at the wall and drop down its
face. Manual jump style with W steering into the wall every update
(slots 5 and 12, 32 jumps): no bounce; travel along the wall kept (2334 units at
45 degrees), up to 27 slides in one jump, all silent.

**Regression routes, `slide` against `original` on the same binary**
(`regress.sh`):

- Slot-12 1024 wall: W+E+Space grab 148 and pull-up 140 in both. W+Space:
  `original` 98, 107, landing; `slide` 98, lands at the foot of the wall.
- Slot-12 cases: holstered E jumps and the pit jump all catch (slide 6/6;
  original 5/6, the known pit flake); the tall wall never grabs.
- Crates: crate-to-crate 8/8 on top in both; crate A E mantles 139/140 in both;
  crate B varies run to run in both (as in D08Y).
- D08X angle grabs: -15 and -10 catch, 0..15 do not, in both.
- D08U ladder top: identical states.
- D08W subway wade jumps: `original` bounced 107 off the train in four of the
  eight ledge sweeps; `slide` none; ledge rises unchanged. Sideways wade jumps
  keep more of their sideways travel.
- Furniture run-offs: identical.
- D08Z slot-5 gap (both jump styles): every E case lands across or mantles in
  both modes. Without E, the low-lip scramble 134 still fires (manual 86600,
  86750; assisted 86750), now from the wall contact instead of a 107. Misses
  are the known ones: manual pressed 1000 before the lip falls short, and
  assisted has no grace after running off the edge.

Native: `ttk-controls-test` 45 groups. The new D08Z1 group covers head-on,
45-degree and diagonal walls, the push-off, a corner stop and a rising ceiling.
It also checks that falls, clear paths, other callers, a short guest stack,
other modes and Vanilla are untouched. The test stub now chains several
plugins per address, as the runtime does. `ttk-input-test` and `ttk-aim-test`
pass. Python 136 OK (10 skipped), including schema 30 migration, CLI,
environment and menu. `check_repo.py` OK.

Not verified: the user's feel in play; Vanilla in-game (covered by the
launcher forcing `original` and the native Vanilla case); ceilings were only
exercised natively (no ceiling hit came up in the routes).

## Revision 1: convex corners (UI slot 6 monument)

User (2026-10-08, after the first build): "I tried jumping into the corner of
the monument here, and i still get that bounce back. Otherwise, the feel of
this new mechanic feels much more modern and great, and alleviates one of the
biggest clunkiness sources of the entire game."

Reproduced on a private copy of UI slot 6 (level tag 3, the statue on its
base), with the user's settings: manual jump, first person, running at the
base's corner. Two of eight jumps from -9..0 degrees still played 107. The
others stopped dead (the first build's fallback zeroed the horizontal speed).
Cause: at a convex corner the sweep reports a face whose normal is almost
perpendicular to the slide, so removing the into-wall part never clears it.
With the speed zeroed the real sweep still touched the corner, and the
original bounce ran.

Change:

- **Corner turn:** after two clips, if a wall still blocks, the takeoff
  heading is turned in 15-degree steps (up to 90). It turns first toward the
  side the first clip slid to, and keeps the speed's share along the new
  heading (at least 1200). The first heading the original sweep clears is
  taken. Failing that, Duke is pushed straight off the faces.
- **Safety net:** if a corner still reaches the original bounce
  `0x8003ef08` (from the wall case, `0x800559f8`) or deflection `0x8003ef78`
  (`0x800559e8`), their rerun of the integration (`0x8003ebf4`, ra
  `0x8003ef68`/`0x8003efc0`, caller at `sp+0x24`) gets the pre-pass velocity
  minus the part into the wall, plus a push off it. It also keeps the
  update's own vertical speed and timer. The 107 the handler then writes is
  undone at the next animation runner call (`0x80059db0`, the upper body's
  in the same update; the airborne handler runs after the lower body's).
  That update's original rumble and contact sound have already played.
- **D08Z2 correction:** the 107 undo now happens at the entry of the contact
  sound `0x8006bbd8` right after the write (new hook); the runner-call undo
  left 107's leg joints in track 3's mask and froze the legs. See
  [note 135](135-d08z2-mantle-upper-body.md#revision-1-legs-frozen-after-a-corner-bump).
- **Guards:** `0x8003ef08` (200 bytes) added.

Evidence (executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`):

- Slot 6 corner, running, manual, first person, nine headings -20..+6:
  no 107. 43 corner turns were found and the safety net caught 7. Assisted
  (six headings) and standing manual (six): no 107.
- Slots 5 and 12, all eight headings, running and standing, manual: no 107.
- `regress.sh slide` again: the same catches, mantles and gap results as the
  first build. The D08X angle route varies run to run in both modes (-10
  caught 1 of 2 with `slide` and 1 of 2 with `original`).
- Native: `ttk-controls-test` 45 groups. The D08Z1 group now covers the
  corner turn, the push-off when nothing clears, and the safety net (once
  only, wall case only, 107 undone). Python 136 OK.

## Limits

- The original contact sound and rumble only came from the bounce code, so a
  slide is silent. If a thud on hard head-on impacts is wanted, it can be added.
- A hit with a near-vertical normal is still left to the original; none was
  seen in testing. When the safety net catches a corner, that update's
  original rumble and contact sound still play.
- Holding W into a convex corner in manual style re-steers into it every
  update, so Duke edges around it rather than gliding past (about 450..820
  units of air travel at the slot-6 base).
- The isolated sweep runs every airborne update (two original calls).
