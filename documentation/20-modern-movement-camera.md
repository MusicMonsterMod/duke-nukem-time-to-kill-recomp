# D05/D06 — Bounded Modernized controls

D05 and D06 are Done following successful user playtesting and explicit sign-off.
The implementation and remaining coverage limits are recorded below. These adapters target the verified SLUS-00583 executable and
exact LEVEL00 overlay only (D22A, 2026-10-04: the authenticated LEVEL01 body is
also accepted; see [note 100](100-d22a-portal-level-identity.md)). Vanilla returns before guest reads or writes in the
control hooks. All original media, generated C and player cards remain untouched.
Generated source is rebuilt through the normal local build wrapper.

## D05 movement contract

Captured PC movement requests the original forward locomotion action only while
a recently invoked, identity-checked normal camera owns a standing/walking player.
The camera lease expires after four input frames and is tied to capture epoch.
Neither keyboard axes nor the adapter emit original left/right tank-turn bits.
Original animation supplies horizontal speed, including the slower L1/Shift
walk. There is no host-time multiplier and no extra simulation iteration.

The adapter normalizes camera-horizontal forward/right input, takes the magnitude
of the original animation displacement and redirects it before the original
handler. Vertical displacement is preserved. Diagonal keys therefore have the
same continuous horizontal magnitude; rounding to signed guest integer units
has at most the normal sub-unit quantization error. Animation deceleration keeps
the last movement direction until idle, rather than reverting to actor facing.
The first correction did not yet cover the separate stop-animation handler;
D06 review identified and added that handler as a D05 follow-up.
Actor actual/target headings are not overwritten.

Running uses D03 entry `0x80053500` with callers returning to `0x80048664/A8`.
Shift-walking uses `0x80053404`, returning to `0x800485DC/620`: disassembly confirms
its mode-4 query before XYZ integration. The game computes a facing-based world
probe inside `0x8007926C`. At `0x800780B4`, return `0x800794B8`, the adapter
redirects that vector and its already-computed endpoint together, preserving the
original probe length and vertical component. It checks the nested stack, actor,
argument and saved mode-4 return (`0x80053548` or `0x80053424`). Stop-animation `0x80054218` uses the same query, guarded by returns
`0x800487C4/0x80048810`, arguments `(player,63,4)`, frame size 0x20 and
collision return `0x80054240`. Later mode-6 slide
queries keep their original delta-based path. Original collision responses,
floor queries, slope handling and final position commits all still run.

Every mutation rehashes all guarded code regions and every byte of LEVEL00 using
SHA-256. Reciprocal links, state bytes, alternate-camera ownership and active
capture are checked at invocation. Unknown states/maps retain original behavior;
arrow keys remain the original fallback. This is a bounded implementation, not
campaign or traversal certification. Side-step animations and weapon rays have
not been replaced; modern weapon aim belongs to D07.

## Evidence discipline

The first SDL movement probe (`d05-movement-01`) proved directional displacement
and matched run/probe callback counts, but failed Shift-walk. It also exposed
forward drift from unredirected deceleration. That run is retained as failed
evidence; the handler extension and deceleration correction follow it.

Pure geometry checks establish normalization and speed invariants. The owned-data
native harness tests Vanilla no-writes, live-code/overlay rejection, wrong actor,
wrong caller, unsupported states, and displacement/probe consistency. These tests
cannot establish reliable collision on actual walls, slopes and steps. SDL routes
use separate Xvfb displays, settings and fresh cards and read guest state without
patching it. Screenshots and gameplay results must be reviewed separately.


## D06 orbit and ownership

The normal-camera entry seeds orbit yaw/pitch and radius from the existing
anchor around the player-linked target (`camera+0x4C/+0x50 == player+0x7BC`),
including the original vertical look-target offset. Captured mouse counts
advance yaw continuously, with default 0.12 degrees/count and a ±60° pitch
limit. The launch profile can select original camera behavior instead.

At `0x8003AA48`, with the D03 caller and stack argument checks, a Q12 view
rotation and requested anchor delta enter the original four-candidate constraint
routine. The original anchor integration, camera position conversion, room lookup,
height queries and look-at update still execute. No final position store,
world-query return value, simulation clock or guest instruction is replaced.
Obstructions may limit the actual view. Desired orbit remains separate from actor
heading and movement; weapons still use original aiming.

Relative mouse counts accumulate across input frames. Each camera update uses
the difference from the last consumed totals, so slower simulation does not
lose intervening mouse samples and repeated callbacks cannot multiply input.
Capture epochs and unsupported-state gaps reset consumption. There is no
framerate multiplier. Normalization, batching, duplicate samples, epoch resets,
yaw wrap and pitch limits have native arithmetic tests; real display refresh
rates still require playtesting.

While captured and supported, no automatic recenter is applied. Escape/F10,
pause/inventory, focus loss and host overlays release capture. The original
camera then owns control; a fresh F10 seeds from its current view, discarding
stale motion. Jump/traversal, unknown animations, alternate camera and map changes
also relinquish ownership. This may produce the original camera's recentering
motion; it is not a promise of a polished transition through every state.

**Death guard correction:** the first D06 combined route reached death while
normal camera callbacks continued and state bytes remained 0/0. Screenshots
exposed this despite a passing numerical route. It is rejected as camera gameplay
acceptance. Controls now require the guarded `0x80048410` animation dispatch
subset: idle 63 and locomotion/stop 72–79, plus clear observed death flag bit 2 and the original heading-inhibition flags
(player `0x20000`, player+0x224 `0x200`).
Animation 218 and any unrecognized animation fail closed. The route now rejects
death and separates camera checks from the longer movement route. This is why
numerical callback counts alone cannot establish correct ownership.

## Persistence and reproducing checks

Profile schema 3 adds `controls` (camera, sensitivity, invert_y). Versions 0–2
migrate with byte-preserved backups, retaining existing bindings, renderer and
selection. Invalid camera preferences recover with a visible notice; newer
schemas are refused. `--settings` choice 7 and the explicit launch flags edit
these values. Resetting Modernized restores camera defaults as well as bindings.

```sh
python3 -m unittest discover -s recomp/tests/local -v
cmake --build recomp/build-local --target psx-runtime ttk-controls-test ttk-input-test --parallel 4
xvfb-run -a recomp/build-local/ttk-input-test
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls movement
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls camera
# Owned-data harness: use the isolated movement route's exact overlay fixture.
recomp/build-local/ttk-controls-test recomp/disc/SLUS_005.83 recomp/analysis/pc-input/UNIQUE/level00-guard-fixture.bin
```

The fixture, screenshots and RAM stay ignored under analysis. Reports contain
hashes, counts and observations. No player memory card is used by these tools.
The movement route explicitly selects the original camera to verify movement
with both camera options across the separate runs.


## Reviewed camera evidence

The short fresh-spawn route `d06-camera-final` passes standing look, moving look,
firing input while looking, release/recapture, real X focus loss and pause/resume.
Reviewed standing-fire capture shows a muzzle flash and ammo 198 (starting 200);
post-pause capture shows ammo 197 and resumed first-level gameplay. This proves
successful firing during the route, not shot direction matching the mouse.
Actor yaw remains 1537 while requested camera yaw changes independently.
Look-event count remains 26 across release, refocus and pause recapture, despite
large mouse movements outside capture. Earlier `d06-camera-02` also passed live
movement/camera checks, with ammo still 200; it is not successful-fire evidence.

Still required: representative walls/slopes/steps; long directional/diagonal speed
and animation review; original-aim combinations; high-refresh desktop playtesting;
full yaw/pitch and obstruction/near-wall sweeps; recentering and progression/script
transitions. Original look-at constraints can modify the requested orbit, so the
±60° clamp is a requested pitch range, not a guarantee of unobstructed viewing.
Other maps and traversal remain original and require D08 adapters. Forced-interpreter
execution, hardware controllers and Windows are not newly certified by these runs.


## Final movement replay

`d05-final` exercises the final candidate with the original-camera option and
passes all directional, walk, diagonal, rotated-camera and release assertions.
Player animations remain in the live locomotion set throughout; the rotated
capture shows Duke alive in the first level (health 62), not a death screen.
The final adapter also covers the separate stop-animation handler; earlier D05
reports remain provenance, and this replay supersedes their build identities.


## Final acceptance checkpoint

`d05-final`, `d06-acceptance` and `d05-d06-final-vanilla` all exit 0 and match the
final binary hash in the machine-readable reports. The final camera route keeps
actor yaw 1537 while looking/moving/firing; reviewed ammo is 198, 197, then 196.
Its look-consumption count stays 27 across release, focus and pause/resume.
Vanilla's reviewed spawn, fire (200→196, later 195), jump, inventory, movement and
turn captures preserve the original route. These are automated routes with visual
review, not a fresh user playtest or full-campaign sign-off.

The final movement replay also records deceleration continuing left (+338,+365 XZ)
and right (-367,-335 XZ), instead of reverting to actor-forward (+X,-Z).
The 48-test Python suite passes with two skips; native input and owned-data control
harnesses pass. Pre-existing runtime submodule changes are byte-preserved.
At this pre-user-playtest checkpoint, both jobs were Needs playtest. The later
user sign-off below supersedes that status.


## User acceptance — 2026-09-26

The user confirms that WASD movement and the independent third-person mouse
camera both worked in their playtest and explicitly requests D05/D06 be marked
Done. This is user-reported gameplay acceptance, distinct from the automated
checks and capture review above. The individual coverage limits remain recorded;
no additional terrain, refresh-rate, platform or campaign tests are inferred.
D07 is ready and remains unstarted pending the user's separate skill invocation.

## D07A follow-up

The heading-independence observations above describe the accepted D05/D06 build.
[D07A](22-view-facing.md) now couples body yaw to the view under its own opt-in
camera/aim guards. Movement still uses camera-relative intent independently of
body orientation; original camera selection retains the D05 movement path.

## D07A feedback follow-up

The original run-default/Shift-walk description above is historical. The current
Modernized policy is walk on launch, Shift speed inversion and Caps Lock autorun.
Idle clearance now follows WASD direction, and bump states 94/95 retain camera
orbit only. See [D07A details](22-view-facing.md) and the current manual. D05/D06
remain accepted; additional corridor/terrain playtesting is still required.
