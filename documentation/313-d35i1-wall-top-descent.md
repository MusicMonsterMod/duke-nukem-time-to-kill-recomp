# D35I1 - Climb down from a wall top

2026-10-28. Done, user-accepted. User selected the missing climb-down prompt
at current UI slot 7 and explicitly requested a job and work log. After the
playtest candidate, user authorized acceptance and closeout: "lock it in,
close it out". Implementation, documentation, commit and push are authorized.

## Finding and accepted implementation

Private slot 7 loads Duke on the platform above medieval wall panels, type
756, flags 0x108102. The nearby panel top is about 506 units below his feet;
he stands 481 units behind its plane. The existing top-entry finder only
accepted plain ladders and poles. No wall-top prompt or entry was available.
The report that this is the only such location is not established.

`ladder_top.inc` now accepts thin wall-family panels using the same settled
Modernized ground gate and deliberate E request. Wall reach is 560 units;
ladder reach remains 420. Wall targets retain lateral position, require a
40-unit panel-side inset and use the native 155 wall catch rather than the
156 ladder catch. The owned executable's 0x80047600-0x80047624 branch confirms
that distinction. Existing identity guards and active gameplay checks apply.
The shared blend finishes on native wall poses 198-205, including an early
native handoff. Existing wall movement/query code owns subsequent travel,
panel transfers and exits. No framework, generated C or presentation changes.

## Work log and verification

- Reproduced the absent prompt on an isolated copy of current slot 7.
- Built the player executable. The private 60-setting route emits
  `E TO CLIMB DOWN`; E stows the pistol and enters 155 then 198. S descends
  from Y=2372 to Y=3642 and transfers to the next panel below.
- At the 120 setting: attach, save/reload attached, further descent, Space
  release/landing, top-save reload, backing away beyond reach, E away from
  the edge and simultaneous E/Space followed by a clean reload all pass.
  A long held W continues into ordinary movement after leaving the wall;
  releasing W when native exit 206 starts returns to stable ground and
  redraws the pistol. Waiting until the ground handoff to release W was too
  late in the probe: the view faces the drop, so ordinary forward motion
  takes Duke back over the edge. No exit or camera policy was changed.
- Controls suite passes with owned EXE, clean LEVEL00 fixture, LEVEL01 and
  all 21 level overlays. Added actual-hook tests cover wall geometry/family
  and Vanilla rejection, native 155 attachment, retained lateral position
  and early 198 handoff. Existing ladder/pole and wall movement tests pass.
- Input, aim and near-clipping suites pass. `git diff --check` passes.
- Test fixture correction: the synthetic wall test initially retained an
  old water height, making the positive-Y location submerged. It now sets
  and restores the no-water sentinel. No gameplay change was needed.
- Harness correction: an attempted private save at file slot 15 was refused;
  the successful restoration test uses private file slot 11.

Private evidence is in `recomp/analysis/d35i1/`, including source hashes and
mtimes, copied cards/settings, baseline and candidate captures, route traces
and lifecycle scripts. Source player files remain unchanged. Diagnostic runs
are stopped after testing. Existing unrelated camera/settings edits remain.

Accepted player-build executable SHA-256:
`a7cb2754a14cd21bca32c2ff80709e287d29fed15ad85f6a5d5529e14dce199d`.

## Review

Load slot 7, press E at the edge, then S to descend. Space releases as before.
User accepted the result on 2026-10-28. This bounded panel entry is
not campaign-wide wall-top certification. Frame-rate settings are coverage
points, not performance measurements; dummy audio is not an audible check.

```sh
python3 recomp/tools/local/run.py
```

## Closeout

Implementation and native regressions are included with the manual, job board,
status, handoff and this work log. Existing D19X/D19Y edits are excluded from
this commit and preserved in the workspace. No framework changes, rebuild or
additional game launch were needed for acceptance closeout. No next job started.
