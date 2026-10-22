# D08K2 - Crouch-height movement collision and safe stand-up

Job: [D08K2](../MODERNIZATION_JOBS.md) (split from D08K, 2026-10-22). Status: Done (user-accepted 2026-10-22; animation quality continues in D08K6). The
crouch walk (D08K4) needs Duke to move while crouched with collision for a
crouched body, not the standing one the original move probe assumes.

## How the original moves Duke

Found with the write trace (lab run with `PSX_FORENSICS=1`; the launcher's
player setting mutes the forensic write hook) and the move routines:

- Root motion (`0x800598f0`) turns the legs channel's root travel into a
  per-tick velocity at `player+0xfc/+0xfe/+0x100`, rotated by Duke's yaw
  (`0x80066d94`).
- A movement handler asks `0x8007926c(player, mode)` whether that step is
  allowed (20 handlers call it; walk `0x80053404`, rolls `0x80054218` /
  `0x80054308`) and, if it is, adds the velocity to the position itself (walk:
  `0x800538dc..0x8005390c`). `0x8007926c` writes no position; it returns 0
  (clear), 1 (clearance), 3 (wall, ray or object), 2/4/5/6 (floor relation).
- Inside `0x8007926c`:
  - `0x800780b4(player, delta)`: object probe (a box 250 units above the feet,
    0x226 high), contact in `player+0x174`;
  - `0x8007765c(player, point)`: floor and ceiling at the point
    (`0x8005d358`), returns floor - ceiling, writes the floor step relative to
    the feet `player+0x1c4` (`player+0x1c8` = feet height, positive y is down)
    and the floor normal `player+0x1a4..0x1ac`; 0xffff / step -0x10000 when
    there is no floor (outside the room = wall);
  - refused when clearance < **0x370** (`0x800794d0`);
  - a head ray `0x800774cc(player, from, to)` (`0x80060980`) at feet - **0x370**
    from here to there (`0x800795dc`; `0x80079530` for big step-ups);
  - `0x80077438(normal)`: steep-slope test for steps up.
- Stand-up after crouching already uses the same clearance query at Duke's
  position with 0x370 (crouch.inc, D08A).

## Duke's crouched height

From the decoded poses (D08K1): standing Duke's highest vertex is 819 units
over the floor (the game's 0x370 = 880 keeps 61 spare), crouched (178, all
frames) 579-591, the crouch transition's low frames 588. The hull was first
0x290 (656, the standing margin kept over the kneel); the D08K3 crouch walk
stands up to 654 (pelvis 330), so the hull is now **0x2c0 (704)**, about 50
units over the walk. No level geometry lies between 0x290 and 0x370 anyway.

## Implementation (`recomp/src/ttk/crouch_hull.inc`)

`crouch_probe(cpu, dx, dz, step)` asks the original queries the same questions
with the crouch hull (0x2c0), from the player update (original guest calls, guest
scratch below the stack pointer, player query fields restored on refusal):

1. object probe `0x800780b4` -> refused on contact;
2. `0x8007765c` at the destination -> refused with no floor, clearance under
   the hull, a step up over 0x100 or a drop over 0x100 (a crouched Duke stops
   at an edge instead of falling);
3. head ray `0x800774cc` at feet - hull, here to there -> refused on a hit;
4. steps up: `0x80077438` slope test.

On success the step is applied as the walk handler applies its velocity (x/z,
plus the floor change to `+8` and `+0x1c8`). Stand-up is unchanged (0x370 at
the position, never object-relative).

Debug tools (Modernized, backtick console; nothing changes without them):

- `crouchgo <dx> <dz> [hull]`: the same in world units per update.
- `crouchwalk <fwd> [side] [hull]` / `crouchwalk off`: while Duke holds the
  stationary crouch (178) on ordinary ground, move him by (fwd, side) units per
  update relative to his facing, sliding along a clear axis. The reply lists the
  probe counters (moves, refusals by reason). The feet slide (no animation yet;
  that is D08K3/D08K4). The optional hull tests the probe at another height.
- `crouchscan [radius] [grid]`: around Duke, count floor points by clearance
  band and list spots that fit a crouched Duke but not a standing one, and
  overhang steps (standing head ray hits, crouch head ray and knee ray clear),
  and the nearest cliffs (a neighbouring floor more than 0x100 lower).

## Findings

- **No crouch-only geometry around any level start.** `crouchscan 8192 256`
  at the start of the twelve selectable levels 0-3 and 5-12 (level 4, the
  Reaper, is not in the debug level list; a 16,384-unit square each) and finer
  scans (4096/64 at levels 0 and 3) found no floor point whose ceiling fits a
  crouched Duke but not a standing one: same-floor clearance is never under
  0x370 (the lowest band is 0x370-0x500, on LEVEL00 and level 9). No overhang
  either: wherever the standing head ray hits, the crouch head ray and a knee
  ray hit too. The standing ray does hit geometry (up to 150 of 2788 tests a
  level), so the probe is live.
- The GameRevolution walkthrough agrees: crouch and roll appear only in combat
  (level 4: "Duck and roll behind the mine cart", rolling around a turret),
  never to pass under something. The level 1 air vents are reached by
  jumping and grabbing.
- So the crouch hull is a safety rule, not a door opener: in the original
  levels a crouch walk can go where walking goes, and the user's reason for
  the feature is precise combat movement (2026-10-22: "Rolling during combat an
  arbitrary amount rather than a more precise crouch walk is not favorable in
  combat at all"). Custom content or object overhangs would use it.

## Evidence

Private lab `recomp/analysis/d08k2-hull/` (ignored): offscreen, ports
9402/9403, private copies of lab cards and savestate slot 5, Duke kept alive
by a test-only health write, crouch held through the diagnostics input file
(LCtrl). No player card, save or preference was used.

| Check | Result |
| --- | --- |
| LEVEL00 street, crouched, `crouchwalk` / `crouchgo` 24-32 units per update in four directions | moves 2,700-11,500 units; crosses the 128-unit curb up and down (feet -9216 / -9088); stops at building walls and slides along them (233-2,362 wall refusals); props refuse (15-918 object refusals) |
| Level 3 (Miner 69er), crouched | walks down the mine stairs from feet -768 to -202 in steps of at most 0x100, no fall, no clipping; walls refuse |
| Clearance gate, level 3, same spots | 0x290 hull (the first value) moves; a test hull of 0x7f0 is refused as "low" 340 / 515 times (every ceiling there is 0x500-0x800) |
| Edge, level 5 (Obey or Die), walking at a 384-unit drop | steps down 128, then stops 49 units short of the drop: 186 edge refusals, no fall |
| Edge, level 6 (Family Jewels), walking at a 1,105-unit drop | stops 15 units short (refused as no floor at Duke's height), no fall |
| Release | crouch released after every run: Duke stands (63) where the clearance allows |
| Suites | `ttk-controls-test` (all groups), `ttk-input-test`, `ttk-near-test`, `ttk-aim-test` PASS; Python 175 OK (2 skipped); Vanilla route `d08k2-20261022-01` checkpoints reviewed. Executable `3e58625a743856ff8e1832cfc5baac9c86e5d797b80cf2fa21e1f94e6d61b8e0` |
| Native `ttk-controls-test`, new group | crouch-only clearance (0x2e0) passes with the head ray at feet - 0x2c0; too low (0x2b0), no floor, drop 0x180, step 0x180, object, ray hit and steep step refuse and restore the player's query fields; a 0x40 step down follows the floor; only at 178; off. The existing stance fixture keeps Duke crouched at clearance 500 and stands him at 2048 |

## Limits

- No natural low ceiling exists to show "under a ceiling that blocks
  standing" in play; that case is proven by the native fixture and the
  clearance gate run, not on real terrain.
- Steep slopes were not met live (slope refusals 0); the slope rule is the
  original `0x80077438` test, covered natively.
- The test mover slides the stationary crouch; feet animation, speed, input
  and camera are D08K3/D08K4.
- Objects use the original object probe at its fixed height, so a crouched
  Duke is not allowed under props a standing Duke cannot pass either.
