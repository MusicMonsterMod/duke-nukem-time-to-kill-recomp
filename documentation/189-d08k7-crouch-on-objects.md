# D08K7 - Stuck crouch and legacy roll on the slot 10 dumpster

Job: [D08K7](../MODERNIZATION_JOBS.md). Status: **Done** (user-accepted 2026-10-22: "confirmed working! lock it in and close it out").
Modernized only; Vanilla never reaches this code.

## Report

Player UI slot 10 (savestate file `slot09`, level 5, saved in the pause menu,
SHA-256 `eabed6a88f24c5afb12a9cc35cc1c206ce0ca30321792956513a0eadd83be3ad`):
Duke on top of the green dumpster. Hold LCtrl, try to crouch walk forward,
release everything: Duke stays on one knee; W then plays the original roll.

## Reproduction (lab, before the fix)

Private copy in `recomp/analysis/d08k7-lab/` (port 9407, `a.py`, uncaptured
Escape to leave the saved pause menu, `play` for gameplay frames). Third and
first person, 60 and 120 fps:

- On the dumpster Duke's object contact `player+0x174` holds the dumpster
  (`0x801d9264`) after any standing step on it (the standing walk's object
  probe reports the object he stands on).
- Crouch walk: 126 of 126 crouch probes refused with `hull_object`; Duke never
  moved.
- Crouch released: animation stays 178 every update (stand-up refused because
  `+0x174` was set; no limit, no other way out).
- W: animation 181, the original roll, then 108: Duke rolls off the dumpster.
  With Ctrl released the crouch walk let go of the directions and the original
  crouch handler got them.
- Standing on the dumpster without a prior step (+0x174 still 0) stood up
  normally, which is why the bug needed a step on the object first.

## What the original does (disassembly of the owned executable)

`0x8007926c` (every standing step) never refuses a contact outright:

1. Object probe `0x800780b4(player, delta)`: a box Duke's radius wide,
   centred 250 above the feet, +-0x226 high (so it reaches 300 below the
   feet: the object Duke stands on is a contact). It skips `player+0x17c`.
2. With a contact, `0x80077748(player, point)` (not `0x8007765c`) writes the
   step onto the object's top to `+0x1c4` and its normal to `+0x1a4..`.
3. Step up over 0x100 (or no top): blocked (3), like a wall.
4. Otherwise, objects whose type (`*(0x800d2660) + 28 * type`) has flag
   `0x1000` (the dumpsters, type 165 flags `0x081090e2`) are floor: their top
   is walked on. Any other object is handed to `0x800790ec`, which clears the
   contact and judges the room floor (the object is not in the way).
5. The probe point is not the step itself but a look-ahead: `0x80073b7c`
   builds a vector of `(+0xb0 * 275) >> 12` along the move direction. So
   standing Duke stops that far from a ledge (x 3070 on the dumpster).

## Fix

`recomp/src/ttk/crouch_hull.inc`:

- `crouch_floor()`: the floor at a point as `0x8007926c` judges it: object
  probe; with a contact, the `0x80077748` step onto its top; taller than a
  step refuses (unless Duke already overlaps it and steps away from its
  centre, so he is never pinned); a walkable top (`0x1000`) is the floor,
  with the headroom measured from it to the room's ceiling
  (`clearance - room step + top step`); any other object is ignored and the
  room decides.
- `crouch_probe()` judges the look-ahead point first (`(+0xb0 * 275) >> 12`
  along the step), then the step itself for the height, head ray and slope.
  The crouch now stops where the standing walk stops (x 3062 / 3070 on the
  dumpster edge; 1472 / 1457 against the alley dumpster's side).
- `crouch_stand_room()`: stand-up needs the 0x370 standing clearance over
  the floor Duke is on (a walkable top included); something taller than a
  step overlapping him refuses. Player query fields are restored.
- Debug: backtick console `crouchstand block|free` refuses every stand-up for
  testing (no natural crouch-only ceiling exists at the level starts, D08K2);
  `crouchstand` alone prints the checks and refusals. `crouchwalk off` now
  also prints them.

`recomp/src/ttk/crouch.inc`: the stand-up uses `crouch_stand_room()`;
`crouch_stand_pending` is set while Duke is still kneeling (178) with crouch
released (refused, or not yet allowed). `crouch_walk.inc` owns the
directions in that state too (`crouch_walk_input_ready()`), so they crouch
walk and never reach the original roll; `stance_request` keeps retrying and
stands Duke as soon as there is room. CROUCH ORIGINAL is unaffected (the
crouch walk is Modern only), so its roll stays, and it gains the corrected
stand-up on objects.

## Evidence (executable `a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963`)

Lab `recomp/analysis/d08k7-lab/` (`a.py`, logs and `shots/` per tag):

| Run | Result |
| --- | --- |
| Slot 10, walk onto the dumpster, Ctrl, Ctrl+W, release, W (third person 60 Hz `f`; first person 120 Hz `m`) | crouch walks over the top (68 moves), stops at the edge (x 3062-3065, the standing walk stops at 3070), slides along it; release stands (63); W at the ledge stays standing like the original standing walk (no roll); S walks back |
| Slot 10, `crouchstand block`, Ctrl, release, W, `crouchstand free` (`g`) | stays 178 after release; W crouch walks (no 181) to the edge; freed, stands by itself; W walks |
| Slot 10, CROUCH ORIGINAL (`h`) | release now stands on the dumpster; Ctrl+W rolls (181) as before |
| Alley dumpster (D08T slot 5 copy, `cards-alley`): crouch walk from the street over the curb into the alley (`k`) | crosses the 128 curb, stops against the dumpster at x 1472 and slides along it; release beside it stands |
| Alley dumpster: W+E mantle onto it, crouch walk on top, release (`j`) | crouch walks over the top, stops at the far edge (3074), stands |

Tests: Python 179 OK (2 skipped); `ttk-input-test` PASS; `ttk-aim-test`,
`ttk-near-test` PASS; `ttk-controls-test` (LEVEL00 fixture
`analysis/pc-input/d07-final-camera/`, LEVEL01) PASS with new D08K7 cases in
the D08K2 group: tall object refused and restored, step away from an
overlapped object, walkable top as floor (headroom from the top, step up onto
it, no edge over a drop while on it, edge off it), other objects ignored,
stand-up on a walkable top (refused / allowed by headroom, refused while
overlapping a tall object, `crouchstand block|free`).

## Limits

- No real crouch-only ceiling was found (D08K2), so "stays crouched under a
  low ceiling" was shown with the debug block, not real geometry.
- Objects without flag `0x1000` are now ignored by the crouch, as the
  standing walk ignores them; D08K2 had refused every contact.
- D08K5 (game-wide coverage) still has to sweep other objects and levels.
