# D35K4 - First-person wheel turn

2026-10-25. Done, user-accepted.

## Acceptance

User: "lock it in and close it out". D35K4 is accepted and closed. Implementation,
tests, manual and documentation are authorized for commit/push as MusicMonsterMod.
No post-acceptance gameplay changes, builds or launches. The validation limits
below remain recorded; acceptance does not imply additional diagnostic coverage.

## Scope and native identity

User requested a new job and immediate implementation for the current slot-5
wheel turn reverting to third person. D35 and D35K remain closed.

Fresh private slot 5 is LEVEL00, facing a wheel. Walk forward to contact and
press E. Native object `0x801d6424` is type 206, kind 1, subtype 22, position
`(-9857,-6651,75290)`. Type metadata selects class 7. The resident selector
at `0x800515e8` selects pose 270 (`0x10e`), installed by `0x80051730`.
Baseline naturally selected lower/upper pose 270 loses first person with
`unsupported-camera`. Mode/history remain 0/0, normal owner `0x800d7198`,
both pivots `0x800d7954`, back-pointer `0x800d6eb0`; no alternate-camera bit.

The active LEVEL00 kind-1 callback is `0x800cc14c`. Subtype 22 dispatches to
`0x800cc744`. Event 3 distinguishes object+0x35: zero activates native world
changes and sets it to 2; one calls `0x8006cad8(7)` and writes player+0x230.
This checkpoint starts at one, so it tests the native restricted wheel attempt,
not successful world activation. No guest-memory patch, teleport or forced
pose was used. Dummy audio does not establish audibility.

## Change

`use_interaction_eye()` admits pose 270 under USE INTERACTION VIEW with the
existing committed-action, mode/history, identity, death, script and normal
camera ownership gates. Input, alignment, native callbacks, effects, timing
and weapon restoration are unchanged. Global Third Person and Vanilla keep
their existing policy. No new hook, setting, framework or generated-code edit.

The controls test adds pose 270 to the transient-action regression matrix,
including mode/history rejection, input suspension, view choices, reset/load,
replay, committed-bit, death, script and ownership rejection. Unknown poses
269/276/280 remain excluded. The manual includes wheel turns among supported
USE INTERACTION VIEW actions.

## Evidence

Player build and controls/input/aim/near native suites pass. Controls uses the
owned executable, original LEVEL00 fixture and all 21 selectable overlays.
Natural private routes at 60 and 120 retain first-person blend 1 through all
28 sampled pose-270 intervals each, complete to idle, clear the committed bit
and redraw the weapon. Captures were inspected. The object restriction byte
remains one, as expected for this checkpoint. Baseline held-E reproduction
recorded 53 pose-270 samples, all at first-person blend zero.

Action Third Person retains blend zero through 21 coherent pose-270 samples,
then recovers selected first person. Sequential RAM/camera reads can straddle
an animation transition: an initial assertion caught the final old pose with
the already recovered camera. The repeat checks the pose again after reading
the camera; no gameplay change was made in response to that sampling artifact.

Private scripts, save copies, settings, traces and captures are under
`recomp/analysis/d35k4/`. Global Third Person completes at blend zero (24 coherent pose-270 samples).
Pause/resume and a private mid-action save/load retain first person and complete.
Vanilla loading the private mid-action save completes in third person. Fresh
Vanilla activation and successful wheel world activation remain unverified;
other class-7 wheel variants are not claimed as gameplay-tested.

All 40 source player save/card hashes and modification times match the intake
manifest. Diagnostics are stopped. Player settings, framework changes and
submodule pins are untouched. User framing/comfort acceptance is recorded above.

Accepted executable SHA-256:
`fe660aaf2bd31c478c58d8bbf48b1989513adf9662c3644419d982a5c0375614`.

## Player check

Run `python3 recomp/tools/local/run.py`, load slot 5, use global First Person
and USE INTERACTION VIEW First Person, approach the wheel and press E.
F10 captures the mouse if needed. User framing/comfort acceptance is recorded above.
User authorized closeout, commit and push. No next job is started.
