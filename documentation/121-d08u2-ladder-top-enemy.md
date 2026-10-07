# D08U2 - Enemy at a ladder top blocks the climb-off

Status: **Accepted** (2026-10-07, user: "fully accepted. this is exactly what i wanted. well done!"). Modernized only; Vanilla unchanged.

User request: "when an enemy is stood at the top of a ladder, duke cannot climb
onto the surface. Duke should push into the enemy forcing it back." Then:
"whether duke actually pushes the enemy back or not is not a hard factor in
this job, but Duke should certainly be able to make the climb". Also, as
background: in Duke3D, running hard into an enemy pushes it a little.

## Reproduction (UI slot 5 = savestate file 04)

Private copy in `recomp/analysis/d08u2-20261007/cards` (SHA-256
`1f62950d1f1435821afe0161b019a38fc91b1ce8771d55fc3eb7609af2fef7c7`, level 0,
the first alley ladder, type 46 at (9225, -10239, 13769)). W + E at the ladder:
185 mount, 188/189 climb, then Duke rests on the top rung (187, Y -11006)
indefinitely with Up held. A LARD (type 57, hostile class 6, object
`0x801d0ac4`) stands on the platform at (8698, -11609, 13619), about 530 units
past the ladder. Lifting only that actor by a debug RAM write made the original
exit 190 play on the next update (`top.py`), so the enemy is the sole blocker.

## Original contract

- Ladder handler, rest case `0x80043fd0..`: with Up (input table
  `0x800d1ce8[player+0x233]` bit 0) and no `+0x224` flags `0x200000`/`0x241`,
  it calls `0x8007d65c(player, 0, 1)`. Return 0 is another rung (188/189), 1
  is the top. At the top it calls `0x8007d240(player)` and, when that returns
  nonzero, writes `+0x60 = 190`, `+0x68 = 0`, `+0x6a = 0`. Otherwise nothing:
  Duke stays on the rung.
- `0x8007d240` first calls `0x8007cde8(player, ladder top)`: a short segment
  forward from Duke at the ladder top (64 above it), tested with
  `0x80077558` against every object in Duke's cell list (`player+0x220`)
  whose type flags lack bit 8. Any hit returns 1 and the exit is refused.
  Characters are such objects. Then it finds the landing surface, checks
  clearance (`0x8007cf28`, 881) and the step height (64), and on success
  attaches the landing (`+0x17c`, `+0x180`, `+0x1c8`).
- So the refusal is in the original code and Vanilla has it as well (read
  from the code; not exercised live in Vanilla). Our ladder handling does
  not cause it.

## Change (`recomp/src/ttk/ladder_top.inc`, `ladder_actor_exit`)

At the player update (`0x8005a210`, before the handler), in Modernized, on a
plain ladder resting at 186/187 with Up sent:

1. Collect the living characters in Duke's cell list: object `+0x15` class 6
   (hostile AI) or 8 (NPC), matching the type table's class byte, not dead,
   health above 0. None: nothing to do.
2. `0x8007d65c(player, 0, 1)` must return 1 (top of the ladder), and
   `0x8007cde8` must report a block. The touch fields these calls rewrite are
   restored when the host does not act; the handler calls them again anyway.
3. The characters are moved far above the level for the length of the calls
   only. If `0x8007cde8` is then clear and the original `0x8007d240` passes,
   the host starts the exit exactly as the handler does. The positions are
   restored before anything else runs.

Walls, crates and other objects still refuse the exit: if the segment is still
blocked without the characters, nothing changes. The enemy is not moved; the
original 190 root motion and ground collision stop Duke short of it. The
session log records each `[TTK ladder] top exit past N character(s)` line;
`ttk_input` reports `ladder_top.actor_exits` / `actor_exit_checks`.

`original_call` (shortcuts.inc) gained an optional a2 for the
`0x8007d65c` call; existing callers are unchanged.

## Evidence (binary `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`)

Private Xvfb runs, real keys and mouse, dummy audio, isolated profile and
cards (`recomp/analysis/d08u2-20261007/`):

| Check | Result |
| --- | --- |
| Slot 5, third person (`climb.py`, `regress.py`) | 187 at the top -> host exit -> 190 -> 63 on the platform at about (9170, -11770, 13706); pistol redrawn, lease ready |
| Slot 5, first person (`regress.py`) | same states and landing; one host exit |
| Second ladder, same level (`lad2b.py`) | the other alley ladder (type 46 at (4105, -12299, 13769)), Duke attached by a logged research fixture (holstered with E held). No enemy: original 190 exit, no host exit. With a LARD respawned on the landing through `dnmonsters` off, its placement record moved, `dnmonsters` on: host exit fired, Duke landed at (4205, -13820) about 300 units from the enemy, no overlap |
| Enemy off the climb line | respawned 250 units to the side: the original exit passed by itself; host not involved |
| D08U top mount (`regress.py`, both views) | after the climb-off, E at the ladder top mounts (156 -> 186), S descends with the 185 step-off, pistol redrawn |
| D08U1 ladder, slot 12 copy (`sewer.py`) | E mount, descent, last-rung let-go (108), landing, pistol redrawn, lease ready; no host exit |
| Vanilla | the hook returns before any ladder code outside Modernized; `ttk-controls-test` Vanilla group passes |
| Suites | `ttk-controls-test` (LEVEL00, LEVEL01, all levels), `ttk-input-test`, `ttk-aim-test`, `ttk-near-test` PASS; Python 121 OK (2 skipped: owned-disc tests) |

The player's slot-5 and slot-12 files were copied (hashes verified), never
loaded in place or written.

## Limits

- Duke does not push the enemy back. The user marked pushing as optional;
  a Duke3D-style shove when running into enemies would be a separate,
  game-wide job.
- Only plain ladders (type flag 0x200) resting at the top with Up. Climbing
  walls (0x400) and the 0x100 family were not examined.
- Covered: two ladders in level 0 (the user's slot-5 one with its own enemy,
  and the second alley ladder with a fixture-placed enemy). The second ladder
  was reached by a research fixture, not by its leap.
- In Vanilla the original refusal remains (unchanged by design).
