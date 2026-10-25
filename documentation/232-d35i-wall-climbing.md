# D35I - First-person walls and fluid wall climbing

2026-10-25. Done, user-accepted: "fully accepted! lock it in and close it out".
Explicit implementation, documentation, commit and push closeout authorized.
User selected D35I, supplied current UI slot 10 and added fluid Modernized
WASD/diagonal movement to the scope.

## Behavior

WALL CLIMB VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers FIRST
PERSON (default) / THIRD PERSON. CONFIRM applies and saves it. Global Third
Person wins. Schema 48 adds `wall_view`; `run.py --wall-view first|legacy-third`
saves the choice. The eye stays over Duke's root at the established 294-unit
height, with independent mouse look and native climbing hands.

Modern controls move continuously along the wall: W/S up/down, A/D sideways,
including normalized diagonals. Release stops movement without waiting for the
old stroke to finish; changing direction restarts the native directional
animation. The pace is one unit per native simulation tick, independent of presentation
rate. Body yaw stays attached to
the wall. Mouse look does not change the climbing axes. Space releases through
the original drop. This movement also works in third person.

Slot 10 faces a medieval wall assembled from object types 756/757, with flags
0x108102 (wall family 0x100). W+E approaches and ground-mounts it. A jump followed
by held E catches it. The straight upward route ends under solid geometry;
move left along the upper wall to reach the native top exit. Original movement
also stops on that straight route, and Up suppresses sideways input there.

## Implementation

- Wall presentation owns wall-qualified 147/148 catches, transfer 155 and
  197-206. Native mode-8 mount/exit 197/206 can already own a landing. An
  attachment-cleared 108/9/3 drop retains the preceding Wall presentation.
- The shared root eye, native-arm clipping and actor-scoped near-cull bridge
  include Wall. Normal camera obstruction and reflection separation remain.
  Presentation grants no locomotion, weapon aiming or fire permission.
- New `wall_climb.inc` runs at native dispatcher 0x80043eb8, only for Duke
  from 0x8004b60c, Modern controls, authenticated gameplay, wall family 0x100
  and settled clips 198-205. Replay workers do not run the movement hook.
- Native query 0x8007d65c(player, direction, 0) checks each requested axis.
  Its ceiling/floor, ray/object tests and panel transfer result decide whether
  a step is allowed. Refusal restores query/attachment scratch and floor data.
  A blocked axis does not prevent the other axis from moving.
- For diagonals, the second query uses temporarily translated world-space
  hand matrices after the first successful axis. All matrix translations are
  restored before returning. Root motion is normalized, then the native
  animation delta is zeroed to prevent an additional stroke-driven step.
- The original dispatcher still selects/advances strokes, handles their sound
  events, top/bottom exits and dropping. Direction changes select native idle
  198 so its directional selection responds immediately. Mounts, catches and
  committed exits retain their original root motion.
- Added owned-EXE guards cover the native climb query, clearance and exit
  routines. The standard generator added one opt-in hook in one shard;
  generated C was not hand-edited. No framework or submodule pin changes.
- Developer comparisons: `DNTTK_WALL_MOTION=original` retains native wall
  movement with the new perspective support; `DNTTK_WALL_TRACE=1` logs motion.
  These are not saved player options.

## Verification

Accepted executable SHA-256:
`6b2b0ec48fa56c3fbd3a0a5aefac23dcb111ea4c95d9e66010f989e5408a2d42`.

- Player build passes. Native movie shard was rebuilt for the new hook/config;
  initialized geometry math shard is current.
- Controls suite passes against the owned EXE, clean LEVEL00 fixture, LEVEL01
  and all 21 selectable overlays. New checks cover Wall ownership, independent
  view choices, pause/load/global view, normalized axes, prompt stopping,
  blocked-step rollback, temporary hand-matrix restoration and exclusion of
  Vanilla, inactive input, ladders and poles. Policy matrix covers 16,384 cases.
- Native input, aim and near-clipping suites pass. All 98 profile tests pass,
  including schema-47 migration/backup, preference retention, validation,
  menu-sidecar merge, CLI persistence and Vanilla's legacy-third environment.
- Private Xvfb/OpenGL 120-setting probes cover ground mounting, eight movement
  directions, stable wall-plane position, stop/reversal, adjacent panel
  transfers, side boundaries, the upper obstruction, upper-left exit, bottom
  step-off and Space drop. Cardinal movement measured about 540-550 units in
  1.5 seconds; diagonal components about 370-380 each, without a diagonal
  speed boost. These are bounded wall-clock gameplay probes, not benchmarks.
- Native-movement comparison at the 60 setting also stops below the upper
  obstruction. Modern diagonal movement traverses left and reaches exit 206,
  then returns to ordinary ground play. Hands inspected looking sideways;
  straight-ahead hands can sit outside the narrow wall-facing field of view.
- Live menu at the 60 setting: row layout inspected, cancel writes nothing,
  CONFIRM applies both choices while attached, global P works both ways.
- Final-build lifecycle runs at the 120 setting (First Person) and 60 setting
  (Third Person choice) cover airborne catch, saved-attachment reload, pause/
  resume, all four diagonals with reversals, upper-left exit, bottom step-off
  and Space drop. All 752/734 sampled Wall-owned states use the selected
  effective perspective. First-person catch and exit stay at blend 1.0;
  the Third Person choice returns to global First Person on ordinary play.
- Vanilla private attached-save probe uses original pad Up/Left/Up+Right,
  retains the original climbing strokes and camera, and has zero FP blend.

Private evidence, scripts, copied saves and settings are in
`recomp/analysis/d35i/`. Tests use private writable cards, never the player's
cards. UI slot 10 was copied read-only to private file slot 9, with a source
hash/mtime manifest. Additional attached saves are private only. Source hashes and mtimes are
unchanged after testing. All diagnostic sessions are stopped.

## Acceptance and coverage limits

The user fully accepted the first-person framing and movement. Their acceptance
does not enumerate additional gameplay routes. This
single medieval wall route does not establish all wall shapes, costume models,
wall-to-other-climb-family transfers or campaign coverage. A save loaded
straight into an attachment-cleared drop has no preceding host Wall ownership;
ordinary fall reacquires the selected view. Dummy audio is not an audible
sound check; frame-rate settings are not presentation-cadence measurements.

The user explicitly authorized implementation, documentation, commit and push
closeout as MusicMonsterMod.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Use P for global First Person if needed. WALL CLIMB VIEW defaults to First
Person; either menu choice retains fluid Modern wall movement. Gameplay
captures automatically. Try diagonals, release/reversal, looking around and
Space drop on slot 10, including the upper-left exit.


## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export is
unchanged; applying it to the clean pinned framework index reproduces all 51
changed/new files. No pins changed. No post-acceptance gameplay edits, rebuilds
or launches, and no player data touched. No next job started.
