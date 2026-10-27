# D08T5 - Alley dumpster ladder approach limit

Status: Done, user-accepted 2026-10-24. Explicit closeout authorized.
Selected, investigated and implemented 2026-10-23.

## Reference and implementation

Current UI slot 7 (file index 6), privately copied before testing, records the
LEVEL00 alley dumpster at (7292, -9728, 13829), yaw 2048, type 165. Its live
address is 0x801d9264; that address is evidence, not a runtime identity rule.
Overlay tag 3 authenticates LEVEL00 through existing full code guards. The
ladder is type 46 at (9225, -10239, 13769). The old stop was X 8418.
Slot 7 and the candidate stop were visually inspected in framebuffer captures.

`push.inc` clamps positive X movement to 7292 for that authenticated level,
object type, Y/Z lane and rotation. A partial last step reaches exactly 7292.
The existing support, wall, object and player-clearance checks still validate
that step. Negative X is unrestricted by this additional boundary; saves already
past it are not relocated and may retreat while held. Other objects/lanes and
Vanilla retain their existing paths. No hook, generated-code or framework edits.

## Verification

Accepted executable:
`bf634052fc9cf7758e08474d8c1f87d73d7b259a00e8a9e9cf36f70a9d397459`.

Build and native controls/input/aim suites pass, including all 21 level guards.
New controls callback tests exercise an exact partial final step, repeated held
stop, reverse, retreat from X 8418, and unaffected other lanes/types. Existing
Vanilla, support and collision tests pass.

Private evidence is under ignored `recomp/analysis/d08t4-20261023/` (directory
named before discovering D08T4 was already occupied; stable job ID is D08T5).
Real keyboard/mouse, Xvfb, OpenGL, dummy audio and private profiles/cards only.
`verify.json`: UI slot 12 dumpster moves from X 2582 to exactly 7292, holds there,
reverses to 6662 and returns exactly to the boundary. Slot 9 block still stops
at X 8700, remains there under held W and reverses to 9240. The first-person preference at 120 FPS repeats the exact stop, held boundary,
reverse and return (`first120.json`); grabbing retains its established third-person
interaction view. A private save/load retains the dumpster position. All 36 source save/sidecar/thumbnail files match
the intake SHA-256 manifest. Player settings and cards were not used for writes.

## Existing regrab restriction and acceptance limits

The initial `verify.py` regrab and reload checks only assert object position;
the traces show idle animation 63, not an acquired grab. They are NOT evidence
of successful regrabbing. Focused `regrab.json` and `regrab-baseline.json` reproduce
that refusal on both the candidate and accepted D08L1 executable using the same
private boundary save. The local floor record has grab permission bit clear,
which the existing D08T4 gate honors. No new exemption was added. Reversing while
still holding is verified; unrestricted regrab/pull from every approach is not.

Automated ladder routes are recorded separately below. Player acceptance
of the candidate followed on 2026-10-24 (see below). D08L1 crouched
walk-offs, mantle behavior and original grab-floor rules are unchanged.

Launch: `python3 recomp/tools/local/run.py`. Capture is automatic.

## Ladder route

`ladder-route-tap.json` uses real input from private slot 7: move around the
ladder-side end, turn toward the ladder, tap jump while holding E/W, release E
once attached. Duke attaches to ladder 0x801d9fe4 (animation 185), then reaches
187/mode 3 at (9025,-10505,13769), without attaching to the dumpster. The final
capture was visually checked. This establishes ladder acquisition, not completion
of the climb onto the upper platform. Earlier broad E/W/Space holds sometimes
mantled the dumpster or repeatedly left the ladder; they are retained as route
history, not successful approaches. The change limits object travel; it does
not disable intentional dumpster mantles or guarantee every approach angle.

## Final user acceptance

User: "lock it in!! close us out". D08T5 is Done. This accepts the candidate;
it does not broaden the automated ladder evidence or remove the existing
floor-based regrab restrictions recorded above. No gameplay edits or rebuild
followed acceptance. Authored implementation, tests, manual and evidence are
included in the authorized MusicMonsterMod commit/push closeout.
The runtime patch was re-exported unchanged and verified against the clean
pinned framework: all 51 changed/new files match the live dependency.
No player save writes or next job started.

```sh
python3 recomp/tools/local/run.py
```
