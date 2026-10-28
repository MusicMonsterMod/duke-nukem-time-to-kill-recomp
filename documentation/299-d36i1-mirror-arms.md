# D36I1 - Mirror grooming arm continuity

2026-10-28. Done, user accepted and lock-in/closeout authorized.
User selected registration and immediate work on
first-person hands popping during the current slot-5 mirror interaction.
The accepted [D36 audit](257-d36-first-person-presentation-audit.md) and
[D36I contract](260-d36i-interaction-arm-continuity.md) define the ownership.

## Cause and bounded correction

**S:** `interaction_depth_bridge` admitted Push/Use/traversal, but excluded
Mirror despite already assigning its native arm parts to the polygon clipper.
The original joint loop rejects an entire part when its origin is nearer than
GTE H. Mirror now uses the same guarded bridge as other native interactions.
Only saved loop admission depth changes. Real transforms, world projection,
polygon depth, lighting policy, animation and gameplay remain unchanged.

The primary authenticated Duke draw owns the correction. Reflections retain
their full native actor; display weapons/items, hidden arms and unrelated actors
retain their existing ownership. No assets, allocation, serialization, hooks,
generated code or framework edits. Existing framework changes were preserved.

**L:** a private copy of current UI slot 5 (file index 4, LEVEL00 sidecar)
reaches native pose 278 with equipment 0 and effective Mirror first person.
Baseline captures show the reflected grooming action while primary arms are
absent. Sample 6 has approximate arm-origin depths 56/188/113 and 81/214/136,
all below H=256. These derive from sequential camera/matrix reads, not atomic
GTE snapshots. The source-confirmed exclusion explains the whole-part cutoff;
the user reported popping, and sparse baseline stills do not capture every pop.

## Verification

- Player build and full controls/input/aim/near suites pass. Controls include
  LEVEL00, LEVEL01 and all available overlay fixtures.
- Added mirror tests reuse all-six-part admission checks across negative depth
  and both sides of H, unchanged native matrices/projection, world depth,
  wrong actor/caller/stack camera and draw-boundary cleanup. Explicit reflected
  draws, global third person and Vanilla remain excluded. Existing pause/load,
  replay, intentional effects and gameplay/input tests pass. The new admission
  assertion fails on old code and passes with the correction.
- Private offscreen OpenGL at 60 and 120 settings shows connected forearms and
  hands entering view during grooming, with the reflected action intact. FOV
  60/default/90 and approximately +/-0.63-radian pitch samples retain native
  framing. Hands may naturally move offscreen as the animation or view changes.
- Ordinary use routes reach pose 63/equipment 2 afterward. Repeat use and a
  mid-pose-278 private save/load succeed at both settings. The final repeat
  capture reaches idle before weapon redraw, so it does not independently
  certify that final redraw interval.
- At 120, three replay workers complete 3307 jobs with zero worker, replay or
  plugin failures. This is a bounded correctness check, not a performance claim.
  Sampled packet peak is 19824 of 139744 bytes; host geometry peak 26760 bytes;
  no object budget hits, packet skips or copy overflows.
- Player saves/cards/settings hashes and mtimes match intake after both runs.
  All private sessions are stopped. Local evidence is `recomp/analysis/d36i1/`.

**Acceptance and limits:** the user accepted the candidate and requested
"lock it in and close us out". These are sampled
stills and state checks, not frame-by-frame motion certification. Other mirrors,
costumes, damage interruption, software rendering and a live third-person or
Vanilla route were not newly surveyed. Those exclusion paths have native tests.
Implementation, tests and documentation commit/push are authorized as
MusicMonsterMod. No new build or game run during closeout; prior evidence stands.
No next job started.

Candidate SHA-256:
`7ae3abe53ae84a275174e7129c351e5fa283f2e29e88625533042fa8b1bf3950`.

From the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Load slot 5 in first person and press E at the mirror. Repeat after Duke finishes.
