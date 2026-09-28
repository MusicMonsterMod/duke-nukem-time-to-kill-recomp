# D07A — View-facing and weapon presentation

> Superseded behavior note (2026-09-27): D07C/D08H implementation and current
> coverage are in [feedback implementation](40-feedback-implementation.md).
> Earlier held-aim exclusions, weapon-family limits, and apartment non-reproduction
> below are historical evidence, not the current implementation.


Current follow-up: [D07 controls, assistance and timing](24-d07-controls-and-aim-options.md). The checkpoints below are historical; the follow-up replaces repeated SHA hashing with authenticated full live-word comparisons and adds independent aiming/display preferences.

D01–D07 remain accepted Done. D07A adds presentation to the bounded LEVEL00
controls; its automated evidence and playtest status are recorded below. It does
not extend weapon families, maps, traversal or campaign acceptance.

## Behavior and ownership

With Modernized, independent camera, view weapon aim and capture active, Duke's
horizontal facing follows the actual camera forward vector in supported normal
locomotion. This applies both holstered and with a supported drawn weapon (slots
4–8 and 11). Holstering does not switch to movement-facing: W/S/A/D still move
relative to the camera, including sideways and backward movement. Facing samples
the last completed camera matrix at the heading boundary, so rapid look can have
one simulation update of phase lag; responsiveness still needs playtesting. Original
animation speed, deceleration and collision timing remain in use. No tank-turn
button is generated. Feet still use the existing locomotion animation; bespoke
strafe/backpedal footwork is not implemented.

Body pitch is untouched. Existing upper-body aiming animations receive the actual
view direction, including pitch, through their direction argument. This is not a
new skeleton/IK system: draw/holster, recoil and animations that do not invoke the
original arm-aim routine keep their poses. The arm points along the view axis;
D07's shot still converges from the offset muzzle to the resolved view target.
Close-range parallax, spread, gravity and recoil can therefore visibly differ.

Original camera or original weapon-aim selection disables the new presentation
adapter. Mouse2 original precision aim suspends view-facing, arm retargeting,
view-shot replacement and the modern reticle while held. Release returns control
only when the normal guarded state is available. Vanilla, unsupported drawn
weapons, unsupported states/maps, capture loss, and stale camera leases receive
no presentation writes. No profile schema or saved preferences are changed.

## Verified code boundaries

Original disassembly is from the already identified owned SLUS-00583 executable;
private disassembly remains in `recomp/analysis/d07a/`.

- `0x80041C34` calls heading updater `0x80057230`; the next call at `0x80041C3C`
  enters `0x80058120(player)`, RA `0x80041C44`. This is the new heading boundary,
  after original heading logic and before original aim/model calculation.
- Only actual/target yaw halfwords `player+0x1C/+0x24` are written. Yaw is
  `atan2(camera forward X,Z) * 4096 / tau`, wrapped to 0–4095. Original pitch,
  displacement, clocks and position stores are untouched.
- `0x80097C04` builds the model using actor yaw and invokes `0x80097A44` at
  `0x80097F20` for existing aiming bones. Arguments are scratchpad bone matrix
  and `player+0x124`, with s2 holding the player, RA `0x80097F28`.
- That helper constructs orthogonal axes from a signed-word Q12 direction.
  The hook substitutes a dedicated guest argument vector copied from the actual
  camera's Q12 forward row. It never overwrites `player+0x124`, auto-aim state,
  model flags or animation identifiers.

D07A research corrects the earlier interpretation of `player+0x224 & 2`:
`0x8004179C..0x800419EC` sets it from movement magnitude/run preference, and a
live Shift hold clears it while the same pistol stays drawn. Equipment state is
instead at `player+0x3B8` (0 empty, 1 held item, 2 weapon), with slot at +0x3B9.
The new presentation guard and reticle eligibility use equipment state, so walking
no longer hides the reticle simply because the run flag cleared. This does not
establish the cause of D07's earlier missing pellet reticle frame or certify
reticle continuity in all firing animations.

The existing per-invocation whole-LEVEL00/code identity checks and camera lease
apply. Added SHA-256 ranges cover the new heading caller/callee and arm helper;
the model caller is covered by the existing model guard. Wrong actor, caller,
stack or scratchpad matrix arguments fail closed. Hooks are emitted through
`build.py`, never by hand-editing generated C. Runtime submodule modifications
remain outside this change.

## Reproduce

```sh
python3 -m unittest discover -s recomp/tests/local -v
cmake --build recomp/build-local --target psx-runtime ttk-controls-test ttk-aim-test ttk-input-test --parallel 4
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --controls facing --renderer opengl
recomp/build-local/ttk-controls-test recomp/disc/SLUS_005.83 recomp/analysis/pc-input/UNIQUE/level00-guard-fixture.bin
recomp/build-local/ttk-aim-test recomp/disc/SLUS_005.83
xvfb-run -a recomp/build-local/ttk-input-test
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE2 --controls camera
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE3 --controls movement
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE4 --controls weapons --weapon-aim original
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE5 --controls presentation
python3 recomp/tools/local/vanilla_regression.py --name UNIQUE6 --mode vanilla
```

Run the isolated facing route before using its overlay fixture in the native test.
Each replay refuses an existing game and owns only its own process, cards and
preferences. Raw retail data and captures remain private under ignored analysis.

## Verification and required playtesting

See [D07A evidence](reports/d07a-facing.json). Native harnesses model callback
contracts and owned code/overlay bytes, not animation or terrain fidelity.
The `--controls facing` replay uses real SDL input on a private X display with
fresh cards/preferences; it reads guest state and makes no synthetic weapon or
position writes. The existing camera route covers capture/focus/pause, and
movement/original-aim/Vanilla replays remain separate checks.

Research/failure provenance: `d07a-facing-01/02/03` mistakenly waited for the
run flag to change after H; they are failed routes, not holster acceptance.
`d07a-state-research` records the independent run/equipment distinction and
original arm-helper calls during pistol fire. Long H holds open weapon selection;
the successful natural draw/holster samples in `d07a-facing-04` use a short tap.
That run subsequently failed a heading comparison during camera settling (41.3
angular units, about 3.6 degrees); it is not an accepted complete replay.

Required user playtesting: standing/moving/firing yaw coherence and responsiveness;
body/gun agreement at high/low pitch and yaw wrap; sideways/backward footwork;
draw/holster, Mouse2 plus arrows, focus/pause/recapture; walls, slopes and steps.
Some retained collision responses still consult actor facing (D05 documented
this boundary); changing that facing makes terrain playtesting especially relevant.
Inspect naturally acquired supported weapons and original options. Automated
checks do not replace this visual/gameplay acceptance.

All D07 gaps remain: natural weapon acquisition/switching and enabled-family
animation/trajectory coverage, damaging hitscan/beam integration, moving actor
hits, reticle continuity during firing, around-corner/near-cover coverage,
original-auto-aim comparisons and broader progression/platform testing. The
previous actor-selected shot reaching a wall has no established explanation;
the missing Software pellet reticle frame remains unresolved. D08 is not started.


## Final candidate evidence — 2026-09-26

Executable SHA-256: `1cb5b7be3e9c6d351292f1ec479bbb90db366616885ca78e7137fa0a5e307f1a`.
All six final isolated replays use this identity and exit 0: facing (OpenGL),
camera, movement with original camera, original weapon aim, pitch/walking
presentation (Software), and Vanilla. The normal generation/build/movie step,
50 Python tests (two additional skips), and final native input/control/aim
harnesses pass. Existing runtime diff and saved settings/card hashes are unchanged.

Reviewed facing captures show natural equipment 2→0→2→0 and actual pistol fire
(ammo199), then original precision-view fire (ammo198). Heading/arm/modern-shot
counters stay unchanged during Mouse2 plus arrows/fire; normal ownership resumes
on release. The camera route shows stationary fire (ammo198), moving fire with a
muzzle flash (ammo197), and unchanged look count24 across release/focus/pause
recapture. Original-camera movement records zero new facing callbacks. Original
aiming fires (sampled ammo200→198) with zero facing/arm/modern-shot callbacks and
no modern crosshair in the reviewed composed image.

The additional presentation route observes actual view pitches -16.51° and
+23.29°, with body pitch0 and active arm callbacks. Composed Software captures
show the crosshair during upward fire (ammo199) and Shift-walking downward fire
(ammo197, equipment2, run flag clear), then removed on release. This establishes
those sampled cases, not full-range or all-animation acceptance. The upward
capture also shows Duke partly obscuring the screen center; existing camera/body
occlusion remains. Raw snapshots and presented images span different frames.

Reviewed Vanilla fire (ammo196), jump (ammo195), inventory and turning preserve
the original route. Debug quit reported `emu busy or frozen` before the owned
process exited0; this is not new graceful-window-close, audio/save or campaign
certification. D01–D07 remain accepted Done. D07A remains **Needs playtest** for
the gameplay/animation/terrain criteria above; D08 was not started.

## D07A user-feedback pass — wall recovery and EDuke32 control reference

User reported camera/movement sticking near walls, especially the strip-club
entrance/interior, and requested another pass. This is not D07A sign-off and does
not reopen D01–D07 or start D08. EDuke32 is the selected control reference;
[external research](23-external-research.md) separates current controls from later
HRP, first-person weapon and menu work.

Three bounded changes retain original collision results and position integration:

- Idle `0x8005201C` calls forward clearance at `0x80052220`, before selecting walk
  or run. At the existing world-query hook `0x800780B4`, RA `0x800794B8`, saved
  parent RA at SP+0x54 `0x80052228` identifies that probe. For supported idle 63
  with WASD intent and no original-aim hold, redirect both XZ vector and endpoint
  to requested camera-relative direction. Preserve length and Y. Previously only
  already-running/walking probes were redirected, so facing a wall could prevent
  starting a backpedal or strafe into clear space. No collision result is forced.
- Original run collision explicitly enters bump animations 94/95 (`0x800537F4`
  and `0x80053858`). Their dispatch entries resolve to `0x800490C0`, which retains
  vertical integration. Permit the normal camera lease/orbit in these two states;
  movement, body-facing and gun adapters remain excluded until normal locomotion
  resumes. This prevents a bump from resetting mouse orbit to the original camera.
  No broad traversal/death whitelist expansion. Added SHA guards cover idle
  clearance code, the two dispatch entries and the bump handler.

- A tighter replay then reproduced a separate camera pin: requested yaw changed
  from -81° through -9° to +63°, but actual yaw stuck near -111° before jumping.
  The original final look-at stage `0x80028F2C` overwrote the requested orientation
  using the collision-constrained boom position. At matrix-to-quaternion entry
  `0x8002A7FC`, guarded RA `0x80029220` (smoothed local matrix) or `0x8002920C`
  (direct camera matrix), the normal camera's exact nested stack and S2 owner
  permit substitution of the requested yaw/pitch matrix. Original quaternion
  conversion, smoothing, derived camera transforms, room queries and collision
  positions remain active. No target-pointer swap or final position write. Added
  code guards and native tests reject other callers/owners and changed code.
  Recapture seeds yaw/pitch from the visible matrix instead of the displaced boom.
  The first runtime attempt refused the smoothed path because its guard used the
  delay-slot address; corrected to the actual return `0x80029220`, with live hook
  activity and per-step yaw agreement now required by the wall replay.
  This is a view-orientation fix; full confined-space framing remains playtesting.

Speed policy is walk at process launch; the existing saved `walk` action becomes
speed modifier in supported Modernized locomotion. Default Shift reverses the
speed, Caps Lock toggles autorun only while focused/captured, repeat is ignored.
Autorun survives capture/focus/pause in the session and resets off next launch.
No profile schema change or saved binding rewrite; unsupported/original controls
remain available. EDuke32's default run-key mode uses the same XOR policy, with
our walk-on-launch default chosen explicitly by the user. Caps Lock is reserved.

The original red dot is an autoaim target indicator, not a laser. Current `view`
aiming intentionally bypasses original correction for supported shots. `original`
aiming retains that option; independent assisted-view-aim configuration is D07B
later research. D07 weapon-family/acquisition, actor-hit, reticle and cover gaps
are unchanged by this pass.

Evidence and required playtesting are recorded in the [follow-up report](reports/d07a-feedback.json). Automated
wall recovery is not proof of all strip-club corners, camera clearances, campaign
states or high-refresh behavior. User must retest the reported tight-space route.
