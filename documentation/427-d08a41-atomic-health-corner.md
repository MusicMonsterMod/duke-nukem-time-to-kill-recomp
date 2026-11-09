# D08A41 - Level 0 atomic health moved out of its corner

Status: **Done** (user-accepted 2026-11-09: "much smoother. great work! lock it
in and close it out"). Modernized only; Vanilla unchanged.

## Report

User, UI slot 8 (level 0, the stone room): "the hitbox for this atomic health
means it's difficult to pick up. i think the original devs put it slightly too
far into the corner ... you can test by simply walking into the atomic health
and duke wont always pick it up". Then: "i really dont mind if you just bring
the atomic health forward!"

## Cause (measured)

- The pickup loop `0x8007fd04` touches an item only when Duke's centre is
  within `max(0x100, +0x5a)` of it on X and on Z (a square; D26H,
  [note 111](111-d26e-debug-spawn.md)). The atomic health (type 581) has
  radius 229, so the reach is 256 per axis.
- It is placed at (-2907, 19306). Steering Duke deep into the corner from
  five headings, the walls stop his centre about 100 units off it on X and
  170-220 on Z. The only spots that touch it form a pocket about 150 x 85
  units. A walk that slides along a wall into the corner often misses it:
  the +40 degree corner slide stopped at (+369, -169) without taking it.
- Walking straight at it always took it in the private runs, but only at
  the very edge of the reach (254-278 units off on the larger axis) after
  grinding into the wall: 171-203 frames from the save position.

## Change

`recomp/src/ttk/pickup_nudge.inc` (from `modern_controls.cpp`, beside
`spawn_update`): every 30 player updates, in a level with an entry, the room
lists are scanned for that type at its exact original X/Z. A match is moved
by the entry's offset and relinked to its cell with the original
`0x80095cb0` (as D26H does). A moved item no longer matches, so it moves
once, on a fresh level, a restart or any savestate load (including the
player's existing slot 8). One entry: level 0, type 581, (-2907, 19306),
moved +128 X, -160 Z, to (-2779, 19146). The walls are now about 20 units
off it on X and 15 on Z, so Duke can stand on it. `DNTTK_PICKUP_NUDGE=0`
disables the move (A/B testing only). Log line: `[TTK pickup] level 0 atomic
health ... moved +128,-160 out of its corner`.

## Evidence

Private copy of UI slot 8 (file slot 7), Xvfb, debug port, first person,
60 Hz; `recomp/analysis/d08a41-pickup-reach/probe.py` (local). Player saves,
cards and profiles unchanged after every run.

| Check | Original spot | Moved |
| --- | --- | --- |
| Steered walk-in, headings -40/-20/0/+20/+40 | 5 of 5, 171-203 frames, edge of reach | 5 of 5, 152-170 frames |
| Steered into the corner, headings 0/+20/+40 | 2 of 3 (the +40 wall slide missed) | 3 of 3 |
| Steered into the corner, -40/-20 | wedged on the wall 450-780 units short; never near the item in either build | same |
| Vanilla (`--vanilla`), survey | item at (-2907, 19306), no move logged | - |

Build: `build.py` OK. `ttk-input-test` passes. `ttk-controls-test` gained the
missing D49F stub `aim_bolt_launch_enabled()` so it links; with the
available LEVEL00 fixtures it still stops at its known earlier fixture
checks, as noted since D47 ([note 422](422-d47-first-shot-smoke.md)).

## Limits

- Only this one item is moved; other corner pickups keep their original
  spots. Add an entry to `pickup_nudges` for any other awkward placement.
- Not checked at 180 Hz or in third person; the move does not depend on
  either.
