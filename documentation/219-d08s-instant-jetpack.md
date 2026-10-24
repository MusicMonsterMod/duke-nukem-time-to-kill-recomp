# D08S - EDuke32 jetpack candidate

2026-10-24. Done. User accepted revision 4 and authorized closeout, commit and push.

## Acceptance

User: "this is it!!! lock it in, close it out". The final revision is accepted.
Ground lift and midair height hold are locked in. Build/native and live test
evidence below remains valid; closeout changes documentation only. Historical
candidate sections retain the acceptance state at the time of each test.
Runtime patch re-exported unchanged and verified against the pinned framework.
No next job started. Broader campaign coverage remains limited as recorded.

## Revision 4 - Hold activation height in midair

User clarified that airborne activation must not replay the ground boost.
Modern and EDuke32 now arrest vertical movement and capture the activation
height during jumps/falls. The extra host floor-clearance climb is bypassed
for this hold; native collision still applies. Explicit Space/Ctrl input
ends the special hold and resumes the existing vertical controls. Ground
startup retains the accepted accelerating lift.

Private live checks showed zero root-Y variation after activation: rising
jumps at 60 FPS and falling restarts at 120 FPS, in both Modern and EDuke32.
A separate Modern 120 FPS jump also held exactly. The ground-start regression
check rose 642 units, preserving the accepted lift. Build and full native
controls suites pass with both schemes, including a low airborne activation
case that checks unchanged height and zero ballistic velocity.

Candidate `8e976893091271ddbaaa019878cba08f5acede6874433336fcdfca69a611b548`.
D08S remains Needs playtest. No closeout, commit or push.

## Revision 3 - Modern midair activation

User accepted the lift/control feel and requested Modern activation in
midair too. Modern now routes J/item-use from mode 9 (jump/fall) through the
same guarded startup. Lift, velocities, fuel and the accepted in-flight off
behavior are unchanged. A native pack close can be reversed while airborne
only for its own pending/off record, closing flag and original model state;
other pending item transitions remain blocked.

Private live 60/120 FPS checks: J from rising jump 98 enters mode 10. A rapid
off/on sequence initially reproduced a dropped activation while the pack was
still closing (`0x8001`, mesh 2/3); the candidate now returns directly to
flight and the open endpoint. Full native controls and D08S suites pass,
including new Modern falling/closing cases.

Candidate `d57f64b3c33bdff4182b5bc66f81f5a933abc26e26c14428f0018389214e7ae6`.
Needs playtest for this final addition; no closeout, commit or push authorized.

## Revision 2 - J starts Modern and gives a short lift

User correction: Modern must not wait for Space. The supplied local video
`research/screencaps/eduke32-jetpack.mp4` shows immediate activation followed
by a short rise. Read-only EDuke32 `P_DoJetpack` research confirms ten startup
updates with increasing upward steps before idle flight (30 updates/second).
No EDuke32 assets or source files are included in the player build.

Modern now enters flight on a grounded J request. Both Modern and EDuke32
apply a short accelerating lift over 20 input fields, targeting 640 TTK units,
then hold the resulting height. The lift goes through the existing velocity
and collision handler; it does not teleport the root. Space/Ctrl take over
immediately. Off, lost flight ownership, load/world changes and input epochs
cancel the startup. Modern retains its existing fuel and in-flight toggle;
Classic retains its original J-arm/Space-lift behavior. This explicitly
supersedes the initial candidate's unchanged-Modern activation below.

Private J-only tests (no Space) at 60/120 FPS, UI slots 1/10:
Modern rose 640-642 units; EDuke32 rose 641-642. Both remained in flight and
settled into hover (8-9 units sampled variation over the second second).
Classic remained grounded. The first 480-unit lift left the original floor
clearance controller climbing slowly afterward; 640 clears that region.
The full native controls suite and D08S fuel/gating suite pass, with added
J-only velocity and Ctrl-cancels-startup checks. A 120 FPS ceiling run
retained native ceiling limits when reactivating near the ceiling, and then
held position. Subjective height/feel still
requires user acceptance.

Current candidate:
`6564b4e4101957de920a0773dedc4237a25ba761c593462cd635097dee7ed89c`.
Playtest Modern with `python3 recomp/tools/local/run.py --jetpack modern`;
this saves the scheme. No commit or push.

## Initial candidate behavior (startup superseded above)

A third persisted scheme, `eduke32`, alongside unchanged `modern` (default)
and `classic`. Select with `run.py --jetpack eduke32`, or the JETPACK row
under OPTIONS > ADVANCED, MOVEMENT heading (next launch). The CLI saves the
choice. Existing profiles retain their selected scheme; no schema migration
or change to Vanilla.

J immediately opens the pack and enters flight on normal ground or during
a jump/fall. J again closes it and enters the ordinary 108 fall with a live
camera. Space/Ctrl and camera-relative WASD reuse the accepted Modern flight
velocity/collision layer. Idle hovers. Native landing/water collision keeps
its result and closes the pack; submerged activation is refused. This does
not implement D11F: flight still uses the existing third-person view.

## Original-code research and implementation

Read-only disassembly of the owned SLUS-00583 confirmed:

- `8004aaf8` enters flight by clearing actor flag 0x20, selecting lower-body
  163, zeroing its two progression words and selecting mode 10. Its original
  Square, ground/air and water gates prevent immediate midair activation.
- `8004ade0` retains flight integration, ceiling/wall/floor/water collision,
  weapons and lean. Its cut-out chooses 108 and calls `8003ff6c(player,1)`.
- `800402d0`, `8003ff6c` and `8004001c` own pack opening, closing, sound and
  model endpoints. The candidate invokes those guarded originals and takes
  the model straight to endpoint 5/0, rather than waiting through deployment.
- Fuel is signed 16-bit at player +0x35a. The original capacity/pickup table
  at `800c2716` is 9000. Existing HUD percentages use that capacity. The
  original subtracts animation dt, not milliseconds of real flight; note
  109 measured about 278 fuel units/second. Storing 61000 would overflow the
  original signed representation and break the existing capacity/HUD contract.

`jetpack.inc` keeps the original fuel/save/HUD units and charges capacity per
3660 input fields (61 nominal 60 Hz seconds). Presentation frames do not
count. A fractional unit carries across updates/toggles; capture/load/world
changes reset the fraction, losing at most one fuel unit of precision. A
paused guest clears the billing interval. Original skip-fuel and god mode
remain unlimited. Native pickups or god-mode top-ups during a flight update
are preserved. A scoped pre/post-handler fuel value prevents the original
dt threshold from cutting out before the new timer expires.

`shortcuts.inc` takes the new scheme's J/item-use request before the old
ground-only gate. Normal camera ownership, captured Modern controls, code
and overlay identity, living primary player, permitted state, ownership,
fuel, water and guest-call guards apply. Attached traversal remains owned by
the original. Old takeoff/velocity latches are cleared at an immediate
transition; load/world reset discards host fuel/flight state without writing
it into newly loaded RAM. No generated C, framework, submodule pin or asset
changes.

## Evidence

Private copies, offscreen OpenGL and port 9420; evidence is local under
`recomp/analysis/d08s/`. All 39 player save/card data-file hashes match the initial candidate baseline.
The separate `writable-state.log` differs from that earlier baseline; the
revision tests used only private writable-state directories. Diagnostic games
are stopped. No player save/card data writes.

- Full-tank hovering ended in mode 9 / anim 108 / zero fuel / closed pack
  after 61.09 seconds at 60 presentation FPS and 61.15 seconds at 120 FPS.
  NTSC fields run at about 59.94 Hz; sampling resolution adds a small margin.
- Ground J, J-off and reactivation during the resulting fall observed in
  private UI slots 1 and 10. A separate 120 FPS run activated from a normal
  rising jump (mode 9 / anim 98 -> mode 10). Pack model endpoints are 5/0 immediately.
- UI slot 1, 120 FPS: W/S/A/D, mouse turn plus movement, fire, weapon request,
  idle hover and low-ceiling climb. The native wall/ceiling limits remain.
- UI slot 9, 120 FPS: fly over the water then Ctrl down enters native swim
  mode 5, closes the pack, refuses underwater J, and retains swim movement.
- Actual original Start/pause menu: position and fuel unchanged over the
  pause, resume has no accumulated fuel debt. Ctrl lands and closes the pack;
  J can start again. Health unchanged in the sampled route.
- The first diagnostic runs were invalid: a saved pause menu was not resumed,
  and a scripted Escape run released capture without entering pause. They do
  not establish flight/pause results. Subsequent runs checked the guest pause
  word and resumed with the original menu confirm action.
- Native D08S checks cover immediate ground/air toggles, exact 3660-field
  exhaustion, empty/water/death/code/Vanilla gates, god mode, a native refill,
  paused fields and retaining the native landing pose while closing the pack.
- Modern live regression retains J-arm/Space-lift, hover and original fuel
  drain. Classic retains Space boost, gravity on release and the original
  landing/armed-pack behavior.
- Existing controls, input and aim native suites pass. Profile suite: 89
  tests pass, including CLI persistence and Vanilla environment isolation.

Candidate executable:
`de95de02b313f248375862ab39bcc46405807473f5bf96dd135c670dc1a1f887`.
The full-duration runs preceded the final refill-preservation guard; the
unchanged timer and new refill branch pass the final native suite, and the
final candidate has the movement/weapon/Modern/Classic live checks above.

Wider campaign geometry, every damage/traversal interruption and subjective
feel are not exhaustively tested. The user must confirm the new scheme.

## Playtest

```sh
python3 recomp/tools/local/run.py --jetpack eduke32
```

This saves the scheme. J toggles immediately; try it from the ground and in
midair, then Space, Ctrl, WASD, mouse, release/hover, fire, J-off and J-on
again during the fall. Test without god mode for fuel-out. Capture is automatic.
Use `--jetpack modern` to restore the existing default scheme.
