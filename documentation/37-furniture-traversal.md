# Furniture traversal — 2026-09-27

> Superseded behavior note (2026-09-27): D07C/D08H implementation and current
> coverage are in [feedback implementation](40-feedback-implementation.md).
> Earlier held-aim exclusions, weapon-family limits, and apartment non-reproduction
> below are historical evidence, not the current implementation.


D08E is Done on the user's explicit acceptance: held fire draws/shoots, release
while drawing does not queue a shot, and 1.5x draw/holster speed feels right.
D08B remains In progress; this is a bounded furniture iteration, pending human feel
and broader couch/terrain coverage. The original campaign scope is not closed.

## Implementation

Modernized only, in `src/ttk/terrain.inc` and the existing authenticated controls hooks:

- An owned directional takeoff that initially selects vertical animation97 can
  recheck original forward clearance while ascending and holding the same direction.
  The query's cached foot-height reference is temporarily rebased to current Y;
  support scratch, heading and offset are restored afterwards. Original result0
  and a surface at/below the feet are required. It enters original directional98,
  restoring the existing vertical velocity once after initialization: no second
  upward impulse, position write or bypass of swept collision.
- Normal walking's original result6 (drop) may use the run/fall dispatcher only
  for a measured drop over256 and at most768 game units. Walls and other query
  results, larger drops, water, crouch and interaction retain their old choice.
  Ground displacement is limited to walking speed before original integration.
- An owned short fall keeps the live movement/camera lease through animation108.
  The original landing selector receives continued movement without its walk-stop
  modifier. Ground gait returns to the requested speed. Fall direction is aligned
  once to the takeoff intent; walking horizontal magnitude is capped at2048.
  Later collisions keep their velocity changes. Release/focus still removes input;
  the lease expires on landing, context changes and interruptions.

Original gravity, swept collision, damage and animation events remain authoritative.
Vanilla returns before these hooks. Added SHA guards cover the original fall
initializer, handlers and drop probe; no generated C or disc edits.

## Verification

Final replay and hashes are recorded in [structured evidence](reports/d08f-terrain.json).
Current build: `022a18969f6c682aaf71cf8f53fd5195bd27267ce62c88afb484084e16afda0d`.
57 Python tests including owned-disc integration, four native suites and a final
14-checkpoint Vanilla route pass; firing/holster/draw/jump/inventory captures reviewed.
Final gameplay/state assertions confirm three contact bed jumps, bed and couch
walk-off, continuous held-Shift bed landing, release-to-idle and ladder-exit redraw.
Couch front-center entry succeeds; outer-end and a strafe attempt remain unsuccessful.
The incorrectly named `couch-contact` capture is the tall wardrobe; it remains blocked.

Native checks cover temporary clearance height/restoration, blocked/above-foot
surfaces, preserved vertical impulse, bounded drop selection, one-time fall velocity,
expired lease after landing, input release and focus. Existing weapon/input/aim and
scene-lifetime checks remain required. Gameplay uses private settings/cards/X display,
logged player-health longevity fixtures and original shots to defeat apartment guards.
No enemy-health writes, player teleport, velocity fixtures or inventory grants are
used in these gameplay routes. Native RAM fixtures are separate from live evidence.

The first candidate fixed walking but retained a stale height reference for jumps;
it is excluded from close-jump success evidence. Candidate2 confirms contact97→98
and direct108→78 run landing; its long run continued out of the apartment window,
which is excluded from the furniture landing assertion. A final-route street waypoint
missed the ladder; normal inputs recovered position and climbed it. Route labels and
process exit alone are not pass criteria.

## Low-priority pickup report

The user reports taking pipe bombs by walking over the pillows before opening the bed.
The unchanged original pickup is already present below the bed: the earlier room
snapshot includes type58 at (8388,-11187,7463), near bed object type5 at
(8702,-11457,7741). This is a candidate for the proximity leak, not yet a proven
pickup call contract. A final pillow sweep left slot12 unowned/0, with secret flag/count0, so it did not
reproduce the reported pickup. Keep the user report valid and open.
The light switch sets LEVEL00 flag0x400 and moves the bed;
it does not create this pickup. No pickup-radius or inventory rule was changed in
this iteration. A follow-up should authenticate the particular pickup callback,
reject it while the bed is closed, then verify natural collection after opening,
including an already-owned item and loading existing saves.

## Player check

In Modernized, push against the bed, hold W and press Space; repeat from both sides.
Walk off without Shift or another jump. Repeat with Shift held and check for a smooth
landing. Release movement during a drop, and repeat the entry/exit checks on the couch.
Check an ordinary wall still blocks movement. The pipe-bomb shortcut remains open.

Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Gameplay captures automatically; F10 toggles capture.

## Subsequent human feedback — 2026-09-27

User still finds bed-contact jumps inconsistent and benefits from a tiny step back.
The run-off pause is much improved (roughly 80% acceptable), but the short falling
phase still feels separate from running. Light-switch E targeting can need repeated
attempts. Keep these D08B/D10 follow-ups open; this report does not accept them Done.
User could not collect the pipe bombs through the bed this time, then opened the
bed and collected them naturally. Record that successful run without attributing it
to a pickup-code fix: no such fix was made. The next selected task is D08G cheats,
so these movement details remain outstanding.

## Latest user correction — 2026-09-27, before context reset

The user again collected pipe bombs through the end of the unopened bed. The
previous non-reproduction is superseded; no pickup fix was ever implemented.
D08H now owns the focused apartment follow-up under D08B, including contact jumps,
residual drop/landing slowdown and switch targeting. See
[next-iteration brief](39-next-iteration-brief.md). This is a report, not a new
local reproduction or code change.
