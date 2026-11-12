# D23N2 - What a later redraw job changes, and the plan to rebuild it

Status: inventory measured (2026-11-12); building in progress. Job board:
D23N2. Builds on the D23N1 record ([note 454](454-d23n1-capture-and-rebuild.md)).
Evidence labels: CONFIRMED (our measurement or code), INFERRED.

## 1. The question

D23N1 showed that a redraw job is a pure function of its record: the CPU
state at each top-level hook entry and the game's top-level stores. A later
job of the same game frame differs only in its parameters: the camera (the
late camera, sampled for each present) and the interpolation fraction (the
blended actor joint matrices, object transforms and poses). D23N2 has to
produce, for a later job, what its record would have been without running
the game's code. So the first thing to know is exactly what in the record
changes between two jobs of one frame.

## 2. The tool

`DNTTK_N2_DIFF=1` (developer, `recomp/src/ttk/redraw_check.inc`): every
gameplay job is recorded (no copy, no rebuild) and compared with the
previous job of the same game frame recorded by the same worker. Reported:
whether the two jobs entered the same top-level hooks in the same order; for
each hook address, how often its CPU state differed and in which registers
(general, GTE data, GTE control, hi/lo, as bit masks); for the game's stores
between each pair of hooks, how many differ in value, in address, or in
count; value differences by 4 KB page.

## 3. Results (CONFIRMED)

Offscreen, 180 Hz, the player's profile; pairs are two jobs of one frame at
different fractions.

| View | Pairs | Same hook sequence | Hook entries with other CPU state | Stores differing in value |
| --- | --- | --- | --- | --- |
| UI slot 2, turning | 576 | 70% | 62% | 8.7% |
| UI slot 8, turning | 512 | 72% | 83% | 17.7% |
| UI slot 9, turning | 576 | 78% | 73% | 11.3% |
| UI slot 2, still | 512 | 100% | 8% | 0.9% |

- **The camera drives almost all of it.** In a still view the two jobs
  enter the same hooks, and only the actors' blended poses differ (8% of
  hook entries, under 1% of stores). Turning, most hook entries see other
  GTE state.
- **Turning changes what is drawn.** In 22-30% of turning pairs the game
  entered a different sequence of hooks: other rooms and objects passed its
  visibility tests for the other camera.
- **Where the differing state comes from**, by hook (all three views agree):
  the water/fill hook `0x800b4d2c`, the room lift `0x800a2690`, the
  transform `0x800292a0` (camera x object matrix for props and actor
  joints), the object renderer `0x80010000`, the list step `0x8001ca4c`,
  the per-object draw `0x80031d10`, the small-model hooks `0x8002d848` /
  `0x8002bd48`, water `0x8002bc18`, and the HUD hooks `0x800b4b28`-`0x800b4b50`
  and `0x800b42ec`. The registers are the GTE rotation and translation
  (control 0-7) and the GTE data registers the game's projections leave
  behind, plus general registers holding derived values.
- **Where the differing stores go**: the scratchpad (the game's GTE
  workspace, about half), the stack pages (`0x801fb000`, `0x801ff000`), then
  the packet arena and matrix pages.

## 4. What this means

The game's view pipeline depends on the camera all the way through: it
composes a camera-space matrix for every prop and actor joint, culls rooms
and objects against the camera, positions water, small models and the sky,
and leaves camera-dependent values in the GTE and on the stack that our
hooks read. A later job cannot reuse the first job's record for any of
that. Rebuilding it means doing those steps in host code from the camera
and the blended matrices, exactly as the game does them, and proving each
step exact against the game's own result before using it.

## 5. Plan (D23N2 build order)

Each piece gets a verifier that runs in every job (developer switch) and
compares the host result with the game's own, word for word, before
anything uses it. A piece can be either a host port or the game's own
small, pure routine run on a scratch CPU state: the latter is exact by
construction and cheap for maths leaves. `0x800292a0` is one: it takes
the eye-relative translation (halved when a component would overflow),
then calls PsyQ `ApplyMatrixLV` and `MulMatrix0` into the scratchpad matrix
`0x1f800148`, then loads that matrix into the GTE rotation and translation.
What must not run in a later job is the expensive part of the game's code:
the per-object traversal, animation and list building that a frame's
first job has already done (CONFIRMED by the disassembly of `0x800292a0`;
INFERRED for the rest from the profile in note 449 section 6).

1. **Object transforms** (`0x800292a0`): for each recorded transform, the
   job's camera and blended matrix through the game's own routine on a
   scratch CPU (or a host port on the runtime's GTE emulator), exact to the
   bit against the GTE state the object hook then receives.
2. **Rooms**: the host visibility pass already walks rooms and computes
   their transforms in host code; prove its output is a function of the
   camera and static data only.
3. **Visibility**: the game's room and object culling for the late camera,
   or a capture made with a margin wide enough for a frame's turn, with a
   fall back to the full job when a later job's camera leaves it.
4. **The remaining camera-dependent hooks**: water and fills, small models
   and decals, the room lift, the sky, the HUD hooks.
5. **Rebuild a later job** from the first job's record with these pieces,
   checked word for word against that later job's real stream (the D23N1
   comparison, with the record of the other job).

D23N3 (the game's own camera-dependent packets) and D23N4 (switching it on)
follow. The still-view result also shows a cheaper intermediate: when the
camera did not move between two jobs of a frame, only actor poses differ,
so a rebuild needs only steps 1 and 5 for actors.
