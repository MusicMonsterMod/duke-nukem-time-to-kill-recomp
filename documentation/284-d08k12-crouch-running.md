# D08K12 - Crouch running

2026-10-27. Done, user-accepted. Closeout, commit and push authorized.
The requested Ctrl takeover correction is tracked separately as D21A.
The user selected D08K12 ahead of the earlier audio-work schedule. Its actual
implementation prerequisites D08K, D08K11, D08L1 and D21 are accepted.

## Behavior and implementation

Modern crouch movement now selects the shared Modernized walk/run pace using
the configured speed action and RUN MODE. Both momentary crouch and toggled
crouch already converge on the same movement owner. Releasing the run modifier
returns to walking pace with RUN MODE off; RUN MODE on retains the standing
policy where the modifier temporarily walks.

`crouch_walk_step` uses `modern_pace_step(input_running(false))`: approximately
20.815 or 97.617 units per player update before direction rounding. The false
argument excludes Original swimming's forced shallow-water run from crouch;
all existing callers retain that policy through the default true argument.
The real configured binding and autorun state remain authoritative.

Only pace selection changes. The existing crouch hull, directional collision
probes, wall sliding, stand-up clearance and ledge fall handoff retain ownership.
The existing animation advances by accepted distance, including diagonals; it
therefore cycles faster without new meshes, poses or animation assets. This is
not a separately authored sprint gait. Stationary kneeling and stance transitions
are unchanged. Original crouch and Vanilla do not acquire the Modern crouch mover.
No new hooks, generated-code edits, framework changes or save-format changes.

## Verification

- Player build passes. Native input, controls and weapon-aim suites pass.
- Controls suite uses the owned EXE, clean LEVEL00 fixture, LEVEL01 and full
  selectable-level fixture directory. New cases exercise the actual crouch mover
  through walk, run, diagonal run and release in clearance that fits the crouch
  hull but refuses standing. Released crouch remains low when standing is blocked.
- Existing ledge regression now runs at both paces: original falling handoff,
  retained departure height, no jump impulse, insufficient headroom, blocked head
  ray, excessive step and missing floor refusals. These are collision-contract
  fixtures, not a played campaign vent or ledge route.
- Native SDL input checks verify run/release, RUN MODE inversion and exclusion
  of the Original shallow-water forced-run policy for crouch. Existing toggle,
  cancellation, input-focus, action-binding and Vanilla tests also pass.
- Private real-SDL/Xvfb OpenGL gameplay at 60 presentation verifies Ctrl-held
  and C-toggle walk/run, run release, crouched save/load and Ctrl/Space toggle
  cancellation. Sampled animations stay at crouch 178 during movement; typical
  travel is about 11 versus 48-50 units per guest frame on this route.

- Private real-SDL/Xvfb OpenGL gameplay at 120 presentation repeats these
  checks in first person with crouch rebound to Z, toggle to G and speed to T.
  All movement samples retain crouch 178; run is over four times walking pace
  in paired trials. Guest-frame travel varies with update cadence in this short
  diagnostic; this does not certify frame pacing or performance. Both captures
  were inspected for the selected view, not for animation smoothness.

Local scripts, measurements and captures live under `recomp/analysis/d08k12/`.
The tests use copied prior lab cards and private settings. Player saves, memory
cards and preferences are untouched. An initial diagnostic used an invalid save
slot 14; the corrected test saves only to private file slot 11. An initial custom
binding attempt used reserved RShift and correctly triggered profile recovery;
that attempt does not count as custom-binding coverage. The corrected harness
waits for successful load completion and animation 178 before measuring; an
earlier cold-start sample included entry animation 176 and was rejected.

## Verification limits and player review

Natural vent traversal and live crouched ledge departure are not established by
these new runs. Earlier crouch investigations also did not identify a natural
crouch-only passage near sampled starts. The native low-clearance cases establish
the collision contract only. Full campaign geometry, high-speed passage corners
and human judgment of the faster existing gait remain player-review items.
Vanilla exclusion is covered by the native regression suite, not a new full
Vanilla campaign playthrough. Audio uses a dummy sink in the private runs and
is not claimed as listening evidence.

Review both Ctrl and C, Shift press/release, low passages, wall/corner approaches,
ledge walk-offs and first/third-person presentation. The current player build is:

```sh
python3 recomp/tools/local/run.py
```

Use CROUCH: MODERN; press F10 if gameplay input is not captured.


## Acceptance and closeout

User confirmed "it works for sure!", then accepted the D21A Ctrl takeover
correction and explicitly requested "perfect! lock it in, close it out".
D08K12 is Done. Implementation, tests, manual and documentation commit/push
are authorized as MusicMonsterMod. The verification limits above are retained;
user acceptance does not turn fixture coverage into a natural vent-route test.
No next job started.
