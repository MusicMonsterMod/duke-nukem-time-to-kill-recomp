# D35K3 - First-person statue interaction

2026-10-25. Done, user-accepted.

## Acceptance

User: "lock it in and close it out". D35K3 is accepted and closed. The
implementation, tests, manual and documentation are authorized for commit/push
as MusicMonsterMod. No post-acceptance gameplay changes, builds or launches.
The validation limits below remain recorded; no additional coverage is implied.

## Reproduction and native identity

A fresh private copy of current UI slot 5 is LEVEL09, facing a winged wall
statue. Walk forward to it, stop, then press E. E at the saved distance only
stows the weapon and does not establish action coverage.

The naturally selected object is `0x801df324`, type 926, kind 7, subtype 23,
position `(-5949,5385,35328)`. Type metadata selects class 13. The resident
selector at `0x800516d4` selects pose 279; lower and upper poses both use it.
During the baseline action, mode/history are 0/0, flags `0x80000040`, normal
camera owner `0x800d7198`, both pivots `0x800d7954`, back-pointer `0x800d6eb0`.
The alternate-camera bit is absent. First-person blend drops to zero with
`unsupported-camera`, then recovers after native completion. The baseline
trace contains 37 pose-279 samples; sequential reads can straddle transitions.

The active LEVEL09 kind-7 callback is `0x800cc338`. Subtype 23 dispatches to
`0x800cc720`: event 3 sets object+0x35 from zero to one, calls `0x80091c88`
with arguments 4/0x20, and follows the original sound/quote calls. This source
trace establishes the callback path; dummy-audio diagnostics do not establish
audibility. No guest-memory patch, teleport or forced pose was used.

## Change

`use_interaction_eye()` admits pose 279 under the existing USE INTERACTION
VIEW setting, with the existing committed-action, mode/history, identity,
death, script and normal-camera ownership guards. No input permission,
alignment, native callback, animation, timing, effects or weapon restoration
is changed. Global Third Person and Vanilla keep their presentation policy.
No new hooks, settings schema, framework changes or generated-code edits.

## Validation

Player build and controls/input/aim/near native suites pass. The controls suite
uses the owned executable, original LEVEL00 fixture and all 21 selectable
overlay fixtures. Pose 279 participates in the existing transient tests for
action isolation, view choices, input suspension, load/reset, replay, death,
script and owner rejection. Wrong mode/history are rejected; unexplained
poses 269/270/276/280 remain excluded. Accepted interaction poses remain in
the same regression matrix.

Natural private statue routes at 60 and 120 retain first-person blend 1
throughout 38 and 40 sampled pose-279 intervals respectively. Both complete
to idle, clear the committed bit, redraw the weapon and change the native
one-time effect byte from 0 to 1. Captures were inspected. Action Third Person
and global Third Person complete at blend 0 (37 and 36 sampled intervals),
with the same one-time effect. Action Third Person recovers selected first
person afterward.

Pause/resume and loading a private mid-action save retain first person and
complete the native action with its effect flag set. Vanilla loading that
private mid-action save also completes to idle in third person and sets the
one-time effect. Fresh Vanilla activation was not reproduced with either
keyboard or original-pad inputs from the supplied checkpoint; those failed
attempts do not establish a cause or fresh-activation coverage.

Adjacent private 120-FPS routes also pass: current slot-7 mirror (41 pose-278
samples) and a separate copy of the accepted toilet checkpoint (72 pose-275
samples), retaining first person and completing. The toilet effect was already
consumed and stays consumed; this is not fresh healing coverage.

Diagnostics are stopped. All 40 source player save/card hashes and modification
times match the intake manifest. Existing framework edits and dependency pins
are untouched. No player settings were written.

Private evidence, settings, cards, scripts and captures are under
`recomp/analysis/d35k3/`. Current slot identity supersedes the earlier slot-5
door/toilet checkpoint. Coverage of type 838 and other statue variants is
not established. User framing/comfort acceptance is recorded above.

Accepted executable SHA-256:
`0e18a38e16e0a0a3a87afe62b44e559a87e7cad569c6a82267c4c1819439339f`.

## Player check

```bash
python3 recomp/tools/local/run.py
```

Load slot 5, use global First Person and USE INTERACTION VIEW First Person,
walk up to the statue and press E. F10 captures the mouse if needed.
User acceptance authorizes closeout, commit and push. No next job is started.
