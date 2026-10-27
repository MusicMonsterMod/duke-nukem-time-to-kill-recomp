# D08Q9 - Jetpack walk/run and steroid pace

2026-10-25. Done, user-accepted. Lock-in and implementation/documentation
commit/push closeout authorized. Candidate evidence below is retained as history.

## Request and scope

User reports flight feels about as fast as steroid running. Modern and EDuke32
horizontal flight should follow the ground walk/run choice and active steroids.
Shift uses the existing run action, including its Caps Lock reversal. Classic
retains its intentionally original burst physics; Vanilla bypasses these paths.

## Implementation

`jetpack.inc` replaces the fixed 16000 horizontal target with the shared
`modern_pace(input_running())`: walk 2030, run 9520. Active steroid bit 1 with
positive remaining time applies 5696/4096. Held or expired doses do not boost.
This matches the native addition of 0x640 to the 0x1000 animation rate at
800414c8, read from the owned executable; the existing 800414a0 guard covers it.
The damage-mask-aware steroid flags preserve the effect through damage hooks.

The existing direction normalization and exponential ease remain. At the
already authenticated 8004ac08 root-apply hook (caller 8004b570), horizontal
travel while moving/braking is resolved from the eased velocity and global dt
before the native clearance sweeps. Native animation dt and direction-specific
thrust must not add extra speed. The original 8003ea58 integrator uses
`bv += acceleration * animation_dt`, then adds half acceleration times dt
squared when forming the root. The horizontal prediction cancellation now
includes that full half-acceleration term too. Vertical velocity, learned
vertical correction, takeoff, fuel, aiming and collision policy are retained.
No new hooks, generated-code edits, framework edits or dependency pin changes.

## Verification

Private evidence: `recomp/analysis/d08q9/`, with private settings, copied cards
and states, offscreen OpenGL, dummy audio and debug port 9439. No player data
was used as a writable test destination.

Before: `baseline2` measured land walk 10.6, run 49.0, steroid walk 13.2 and
steroid run 71.8 units per debug frame. Flight was 76.5 without Shift and 82.7
with Shift. These are short gameplay position samples, not exact simulation
constants. The initial `baseline` run failed to grant steroids and was stopped;
its steroid-labelled samples are not steroid evidence.

An intermediate candidate exposed residual directional native thrust at walk
speed. Final root resolution removes that source of error; earlier candidate
logs are retained as development history, not final verification.

Final native controls suite passes with the owned executable, LEVEL00 fixture,
LEVEL01 and supplied level fixtures. New cases cover walk/run changes, held,
active and expired steroids, backward travel, normalized diagonal targets and
vertical independence. An independent collision fixture supplies an arbitrary
large native root and animation dt and checks that the resulting displacement
matches the host walking pace. Existing clearance, slope following, blocked
walls, departure and exclusion cases still pass. EDuke32 toggle/fuel tests pass.
Input and aiming suites passed during this task; those sources did not change.

Final `final-eduke32` (120 target, first person) measures forward walk 10.6,
strafe walk 9.9, forward run 51.3, backward run 47.9, and live Shift release
10.6; hover is zero. Active steroids increase walk to 14.0 and run to 59.7-66.4
in the short forward/diagonal windows. Unboosted diagonal measured 43.0.
Short windows vary with update boundaries and collision, so these are bounded
route observations rather than exact cross-direction speed ratios. Native
contracts verify the identical normalized targets. Private flight reloads,
Ctrl descent and J-off assertions pass. User feel was subsequently accepted; see closeout below.

Final `final3-modern` (60 target, third person) passes the same route: walk
10.6, run 51.3 forward / 49.4 backward / 47.9 diagonal, steroid walk 12.6-14.1,
steroid run 59.7 in the short windows, live Shift release 10.6, hover zero.
Flight reload, descent and J-off assertions pass. Both diagnostic games stopped.
The walking and running targets are shared; matching steroid feel on varied
terrain was a player acceptance item, subsequently accepted below. `git diff --check` passes.

Candidate SHA-256:
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.

## Player check

Launch normally. With a Modern or EDuke32 pack, compare WASD with and without
Shift, then activate steroids with R and repeat. Test sideways/diagonal flight,
release-to-hover, Space/Ctrl and the accepted floor/ceiling following route.
Caps Lock reverses Shift as it does on the ground. No new setting is required.

```sh
python3 recomp/tools/local/run.py
```

## Acceptance and closeout - 2026-10-25

User: "lock it in! close us out". D08Q9 is Done. This accepts the tested
candidate and authorizes implementation/documentation commit and push.
No post-acceptance gameplay changes, builds or launches were made.

Re-exporting the runtime patch produced no change. It applies cleanly to pin
`08ec704a974b1f3a16335b4afeb340b9eff19926` and reconstructs all 52 authored
framework files byte-for-byte. The UI pin is unchanged. Player data, generated
code, media, builds and private evidence remain local. Tests and measurements
above remain bounded evidence, not full-campaign certification. No next job
started.
