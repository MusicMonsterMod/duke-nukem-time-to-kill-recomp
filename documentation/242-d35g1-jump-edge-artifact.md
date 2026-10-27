# D35G1 - Slot-5 jump edge artifact

2026-10-25. Done, user-accepted.

## Acceptance and closeout

User: "lock it in and close it out!!". D35G1 is accepted and closed. The authored
implementation, regression test, backlog research and documentation are included
in the authorized MusicMonsterMod commit/push. No post-acceptance gameplay edits,
rebuilds or launches. No player data changed. D08A23 and D08A24 remain Todo;
no next job started. Existing framework changes and pins remain unchanged.
The diagnostic coverage and limitations below remain; acceptance does not imply
additional routes or whole-campaign verification.

## Report and reproduction

The user supplied `research/screencaps/jump-artifact.mp4`: a broad skin-colored
polygon crosses the first-person view on a jump into a ledge edge/corner,
without holding E. Fresh private UI slot 5 is LEVEL00's sewer walkway,
initial position approximately (-38998,-6642,90587), pistol drawn.

The private natural route turns toward the adjoining raised ledge, looks down,
runs briefly and jumps. A roughly 0.8-0.9 second run-up reaches the edge and
selects native pose 134, mode/history 8/8, with equipment still 2. This is an
armed short step-up, not an E-stowed hang or a clean landing. The baseline
`north.py` trial 2 reproduces stretched skin-colored triangles during pose 134.
No pose, position or guest code was forced. Earlier straight/gap approaches
missed this transition and do not establish reproduction.

The initial harness inherited older diagnostic settings. Those runs were
exploratory only; the successful baseline and candidate use a fresh copy of
current player settings (Manual jump, corrected geometry/textures, strong bob),
with isolated 60/120 settings and 1x rendering for inspection.

## Cause and bounded fix

`first_person_weapon_joint()` marks native traversal arm parts as
`interaction_arm`. With the pistol still held, the same hook hides parts
0x0a-0x0e by substituting `hidden_matrix` at the camera origin, relying on the
native joint-depth rejection. `first_person_ladder_near()` then mistakes those
hidden limbs for native mantle hands and raises their depth to GTE H, forcing
them through the draw loop. The interaction clip path also sees the stale flag.
The artificial transform produces the stretched polygons.

Clear `interaction_arm` when substituting the hidden arm matrix. Native
unarmed mantle arms retain their real matrices and near-clipping bridge; the
pistol hand/viewmodel retains its own path. No animation, collision, movement,
input, settings, camera ownership or guest code changes. This uses existing
identity-guarded resident hooks at 0x800292a0 / 0x800b4d2c and the existing
actor/camera/caller gates. No framework edits, generated C or dependency pins.

## Verification

- Player build passes. Executable SHA-256:
  `58821f8d6191768dde25a04940386ace8d98b5caf0ca52a7ae860d6bad37094f`.
- The new controls regression fails on the old implementation at the forced
  hidden-arm depth assertion and passes with the fix. It exercises all five
  hidden arm parts on armed pose 134, paired with unarmed native-arm depth
  promotion and unchanged matrices. Existing policy, replay, camera ownership,
  other actors, input suspension and Vanilla refusal coverage also passes.
- Controls suite uses the owned EXE, clean LEVEL00 fixture, LEVEL01 and all
  21 selectable overlays. Input, aiming and near-clipping suites pass.
- Two natural armed step-up routes each at 60 and 120: 29 and 28 sampled
  pose-134 states respectively, equipment 2, first-person blend 1, return to
  land pose 72. The adjacent short-run-up route lands without pose 134.
  Canonical captures and displayed-window video contact sheets show the
  reproduced stretched skin polygons removed, with the pistol still visible.
- Holding E through the 120 route stows the pistol and performs the unarmed
  step-up: 15 pose-134 samples, equipment 0, blend 1. Native arms remain visible;
  return to idle succeeds. This is an E-assisted short step-up check, not fresh
  coverage of every full ledge hang, shimmy or pull-up variant.

Action Third Person follows its existing blend-out during pose 134 and returns
to selected First Person afterward. The global Third Person comparison stays
at blend zero but lands in pose 104 without reaching 134; it is a jump/landing
check, not live third-person step-up coverage. Diagnostic sessions are stopped.

The first candidate-60 harness completed its routes and recordings but failed
while writing its JSON due to a local Python path expression. The harness was
fixed and the 60 run repeated successfully. The game fix was unchanged.

Private copies, scripts, traces, screenshots and window videos are under
`recomp/analysis/d35g1/`. All 40 source player save/card hashes and modification
times match the intake manifest. Player settings are not used as writable test
settings. The supplied video, original media and player saves are untouched.

## Limits and player check

The requested skin-colored artifact is the reproduced/fixed defect. Other
existing near-view black geometry visible in these captures was not diagnosed
or changed. Frame-rate settings are not cadence benchmarks; dummy audio does
not verify sound. Native regression coverage is not a fresh Vanilla gameplay
route or whole-campaign/costume verification. User acceptance is recorded above.

Run the current player build, load slot 5 and repeat the edge/corner jumps from
the video, plus clean landings and deliberate E mantles. F10 captures if needed.

```sh
python3 recomp/tools/local/run.py
```

D08A23 instant night vision and D08A24 automatic Bio Mask remain Todo. Their
initial source research, independent settings requirements and open native
lifecycle questions are recorded on the job board. Neither is implemented.
