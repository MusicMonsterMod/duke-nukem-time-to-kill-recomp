# D08T9 - Level-5 dumpster repeat grabs

Status: Done, user-accepted 2026-10-25. Explicit lock-in and closeout authorized.

## Acceptance and closeout

User: "perfect! lock it in and close it out". Implementation, tests, manual
and documentation are authorized for commit and push as MusicMonsterMod.
The tested candidate below is the accepted executable. No post-acceptance
gameplay edits, build or launch occurred. No next job is started.
The runtime patch was re-exported unchanged and verified against clean pinned
framework `08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files match.
Acceptance retains the evidence limits below.

## Reproduction and cause

Private current UI slot 8 contains LEVEL05 (authenticated tag 8), type 165
object at (-61959,-9728,-15864), yaw 1023. It is different from D08T4's
fixed type-165 scenery at (3460,-9728,-42). No object address or save slot
is used as implementation identity.

On the accepted baseline, short push/regrab cycles succeed. Pushing farther
moves Duke from a floor record with byte-4 bit 0 set onto one with it clear.
At object Z -11454 the original floor rule then refuses a new grab: Duke
stays armed in idle 63. This is an acquisition failure, not a movement limit;
the held object can still travel farther before releasing it. Earlier D08T7
permission covered the alley and medieval X lanes only.

## Implementation

The placement table now describes either an X or Z travel lane. This verified
LEVEL05 dumpster gains Z-lane acquisition using its overlay, type, height,
fixed X and rotation. The original line-up must face along that lane and the
existing stationary support, clearance, wall and object checks must pass.
The existing manual mover, speed and collision rules are unchanged. Floor
records are never edited. Other LEVEL05 dumpsters retain the original rule.
Vanilla never enters the placement extension.

Source: `recomp/src/ttk/push.inc`; owned-data regression coverage in
`recomp/tests/local/modern_controls_native.cpp`.

## Evidence

Candidate executable:
`5ea6c549eb2fc1d3cb4ed8c8f14af031927b4b67556dd16ccfc3d4a6212ba5cc`.
Private scripts, snapshots and logs: `recomp/analysis/d08t9/` (ignored).
Runs use isolated copied savestates, private settings/cards, Xvfb, OpenGL,
real keyboard/mouse events and dummy audio.

- Incremental build and controls/input/aim native suites pass. Controls cover
  fresh LEVEL05 Z-lane acquisition at a boundary and midpoint, both facings,
  wrong-axis, wall, unsupported floor, line-up and Vanilla refusal, unchanged
  floor data, and placement eligibility across all 21 authenticated overlays.
- `baseline.json` records successful short regrabs; `long.json` reproduces
  the lost grab after longer movement on the unmodified accepted executable.
- `candidate60b.json` and `candidate120.json`: reach the north limit at
  Z -9488, remain stopped while pushing, release/reapproach/regrab, pull away,
  regrab and return, save privately at the boundary, load and regrab, then
  repeat both movement directions. The latter uses first-person preference
  and 120 FPS; the former uses third person and 60 FPS.
- `limits.json`: pull to the south support/collision stop near Z -16898,
  hold without further travel, release/regrab and push away, pause/resume,
  regrab and return to the stop. Both lane ends permit retreat.
- All 40 player save/card files retain their intake SHA-256 and modification
  time. Only private copies were loaded or saved.
- `east-fixed.json`: the existing alley X lane still regrabs at X 7292 and
  pushes away from the opposite face. The historical fixed LEVEL05 dumpster
  stays armed/idle with unchanged position after Grab and W.

An initial route incorrectly assumed five seconds reached the north limit;
a further push still moved, so that assertion failed. The corrected route
uses twelve seconds plus a separate held-boundary check. The first south
route assumed an identical absolute stop after pause/regrab; original line-up
changed Duke's support position and the stop differed by 14 units. The final
route checks a bounded alignment difference plus no travel under continued
blocked input. Failed attempts remain in the local evidence directory.

## Limits and player check

Coverage is this verified placement and the listed regressions, not a general
campaign mobility classifier. Dummy audio does not establish sound quality.
No new falling-object physics or recovery of unsupported old saves is added.
No framework/generated code, pins or player data changed. Authored title code
and documentation are included in the authorized closeout commit/push.

Load slot 8. Hold RMB (or Alt), push with W, release, walk back into contact,
then grab again and try both W and S. Repeat near the ends of the travel lane.

```sh
python3 recomp/tools/local/run.py
```
