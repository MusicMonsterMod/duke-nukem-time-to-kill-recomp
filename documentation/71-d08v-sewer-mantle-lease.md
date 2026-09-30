# D08V - Sewer mantle/hang modern-control coverage

Status: **Done** (user-accepted 2026-09-30). Modernized only; Vanilla unchanged.

## Report and reproduction

The user saw controls switch in and out of modern mode while mantling in the
sewers near their slot-12 save (54 tank-fallback/resume pairs in a session log
that has since rotated away). Reproduced on a private copy of slot 12
(`recomp/analysis/d08v-sewer/cards`, byte-identical to the player's file, which
was not touched): turning 180 degrees from the save position and holding W + E
walks into a ledge that Duke mantles (139 -> 140, mode 8, about 2 seconds).

Lease refusals seen (`[TTK lease] inactive (state)`), matching the report:

| State | Why the lease dropped |
| --- | --- |
| 139/140, mode 8/0 and 8/8 (mantle, pull-up) | The camera lease (`state(camera_only)`) never covered attached traversal; the pad lease (`traversal_state_ready`) needed mode == previous, so the 8/0 entry fell to the tank fallback |
| 72/76/78, mode 0/8 and 0/9 (gait right after a pull-up or landing) | The ground lease required 0/0; the previous-mode byte lags a few frames |
| 107/108, mode 9/9 (falls) | Only owned jumps and short falls had a flight lease; a fall after a jump or off an unowned edge had none |

## Change (`modern_controls.cpp`, `first_person.inc`, `pc_input.cpp`)

- `mantle_state_early()`: mode 8 anims 134..142 and hang mode 6 anims
  147..153 with previous mode 0/6/7/8/9, shimmy mode 7 anims 149..153 from 6/7.
- `unowned_fall_early()`: 107/108 in mode 9 (previous 9 or 0) unless it is the
  owned short fall (108 with `short_fall` and `flight_valid`). The camera hook
  then clears stale jump ownership as for any animation outside the flight set.
- `state(camera_only)` accepts both, plus land gait while mode is 0 and
  previous is 8 or 9. Mouse camera only: `locomotion_input_ready()` excludes
  the mantle/fall states, so there are no facing, gait or root-motion writes;
  the original mantle, hang, pull-up and fall routines are untouched.
- `traversal_state_ready()` accepts the mantle/hang entry frames (ladders
  still need a settled mode 3), so directional buttons stay modern.
- New `traversal_camera_ready()`: `input_pad` counts it as a modern lease (no
  tank fallback, no `ORIGINAL MOVEMENT` banner). During an unowned fall it keeps
  the buttons the fallback used to send (Up/Down and strafe pads, never turns).
- First person blends to the orbit during mantles, hangs and unowned falls
  (the eye would be inside the ledge), then returns to the eye.

## Evidence (binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`)

`recomp/analysis/d08v-sewer` (`harness.py`, `sweep.py`, `timeline.py`,
`shots.py`), Xvfb, real keys and mouse.

- Mantle timeline at the reproduced ledge, before vs. after: identical
  animation sequence and positions (139 at the same frame and place, 140, back
  to gait at the same spot, final position within a few units). Before: orbit
  counter frozen during the mantle and two tank fallbacks (8/0 entry, 0/8 exit).
  After: orbit counter advancing throughout (45 -> 113), no refusal, no fallback.
- 16-case sweep (8 headings, W + E, with and without jumps): tank fallbacks
  12 -> 0; the only remaining lease line is the expected `released` at load.
  Mantles occurred in the same cases (3/16 before, 4/16 after with run-to-run
  heading variance); no mantle was lost.
- Mid-mantle captures: third person shows the orbit behind Duke; first person
  blends to the same orbit and is back to `active` after the pull-up.
- Native `ttk-input-test` (new: the traversal camera lease feeds Up and strafe
  pads with no fallback banner), `ttk-controls-test`, `ttk-aim-test` PASS;
  Python 87 OK. D08T1 dumpster route rerun on this binary: unchanged results.

## Limits

- Only the ledge reachable from the slot-12 position was exercised; other sewer
  hangs (mode 6/7) follow the same rule but were not reached in these runs.
- "Mantling felt harder" was not reproduced: the mantle itself is the
  original routine and plays identically before and after.
- Ladders (mode 3) keep the original camera, as accepted in D08.
- After a blocked D08T push the original stagger (82/84, player flag 0x2)
  still uses the fallback for a moment; outside this job.
