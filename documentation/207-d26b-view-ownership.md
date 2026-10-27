# D26B - View and control ownership across transitions

Status: Done (user-accepted 2026-10-23). User expanded D26B from console
behavior to view/control ownership across menus, focus and loads, then accepted
the result and explicitly authorized closeout: "perfect! lock it in, close it out."

## Confirmed problem

On the accepted D17Y build, real SDL console and pause input both reduced the
first-person blend from 1 to 0. Closing them blended back through third person.
The `fp.requested` diagnostic also incorrectly read false during suspension,
although the persistent first-person preference had never changed.

The host input module cleared the entire action snapshot on capture release,
including view/shoulder preferences. The camera lease required that snapshot's
`active` flag and input epoch. Its refusal path invalidated the orbit and called
`first_person_release`. Recapture then seeded direction from the guest camera
that had already been allowed to retreat to third person. This was a camera
ownership failure, separate from D04B's fixed permanent capture hold. No evidence
here establishes that the saved Modern/Legacy setting changed implicitly.

## Ownership contract

| State | Authority | Transition behavior |
| --- | --- | --- |
| First/third-person and shoulder selection | Host input preferences, explicitly changed by P/shoulder actions or confirmed settings | Present even in inactive input snapshots; guest saves cannot replace them |
| Modern/Legacy selection | D04B live host setting | Only explicit configuration changes it; release is never Legacy |
| Movement, look, firing and interaction | Captured, focused input plus existing gameplay and identity leases | Suspended in overlays, menus or lost focus; existing capture recovery retained |
| Camera presentation | Authenticated normal camera, actor/state and independent-camera selection | Continues without active input; input epochs and action freshness do not revoke it |
| New guest world | Savestate load generation and authenticated overlay tag | Old orbit, draw tokens, flight ownership and eye smoothing discarded; acquire the new world's direction and selected view |
| Unsupported/special camera | Existing title camera/state contracts | Temporary override, never a preference write; diagnostics identify the reason |

`pc_input.cpp` keeps view/shoulder in cleared and inactive frames while actions
remain empty. `modern_controls.cpp` separates the presentation lease from the
action lease. The original-camera option retains its existing movement lease;
independent camera is still required to own the rendered orbit. Pause context
is admitted only for presentation, never movement or capture offers.

Input epoch changes reset accumulated deltas and pending camera-key motion,
without reseeding yaw/pitch from the original camera. Camera constraints,
orientation conversion and draw-time head hiding use the presentation lease.
Frame replay still captures its draw lease in `RenderState`; late mouse input
still requires active captured input. Its existing load-generation timeline
reset prevents interpolation across savestate worlds; no replay/framework edit
was required.

A load can resume midway through a draw, before the normal-camera callback.
`camera_world_reset` therefore observes load generation at hook entry, discards
old head/boom ownership tokens without writing through stale pointers, and
clears old orbit/eye/flight state. The existing D11C draw path reclaims hidden
joint flags actually present in the loaded model. A changed authenticated level
tag also starts a new camera world. The existing authenticated teardown call
contract at `0x8002b9f4` also resets it for same-level restarts/new games. Known
jump animations can acquire presentation directly from a loaded airborne state;
action consumers retain their existing jump/input continuity checks.
The first supported camera update acquires
the requested view immediately; it does not animate from a previous save's view.
Ordinary explicit P switches and existing traversal transitions retain blending.

## Deliberate overrides and limits

- Legacy controls and the Original camera option retain their accepted original
  camera policy. The first/third-person selection remains dormant and returns
  with Modern plus Independent camera. D26B does not redefine Legacy controls.
- Swimming, jetpack flight, pushing and attached traversal still use the
  established temporary third-person views. D11E/D11F remain separate jobs.
  Rolls, slides and supported airborne movement retain first person.
- Scripted/turret cameras, death, frontend and unrecognised code retain their
  existing refusal/ownership rules. Reasons now distinguish `legacy`,
  `original-camera`, `frontend`, `scripted`, `identity`, `unsupported-camera`,
  `swimming`, `jetpack`, `push-object`, `traversal` and `input-suspended`.
- An explicitly paused debug save remains paused. Its stored background is
  displayed until resume; host render state may correctly report `world-change`
  before the first normal-camera update. The preference remains authoritative.
  This is not automatic unpausing or permission to run gameplay behind a menu.

## Verification

Evidence is local in `recomp/analysis/d26b-view/`. Runs use private profiles,
ports, cards and copies of existing diagnostic fixtures. Real SDL keyboard and
mouse input is supplied under Xvfb with OpenGL and dummy audio. No player saves,
player profile, original media or generated source were edited. Existing patched
framework worktree and pinned submodule revisions are preserved.

- Baseline reproduction: `baseline.json` / `baseline.txt`.
- Incremental player build and native movie/math shard checks pass.
- Native input and aim suites pass. Input tests verify preference retention with
  inactive actions, host overlays, load notification and explicit Legacy.
- Native controls suite passes with owned EXE, clean LEVEL00, LEVEL01 and all
  21 selectable overlay identities. Added cases cover input epochs, paused
  redraws without fresh camera updates, preserved direction, loaded direction,
  mid-draw load-token invalidation, immediate selected-view acquisition,
  explicit Legacy return, changed camera owner and unknown code rejection.
  Existing movement, traversal, aiming and Vanilla tests remain in the suite.
- The old menu-key fixture now explicitly marks the guest paused before testing
  Z as menu-back. D04B deliberately suppresses that fixed key in uncaptured
  gameplay. Camera fixture replacement now declares a load instead of using an
  input epoch to simulate a new world.
- `menus.json`: real Modern -> Legacy -> Modern settings, F2/F3 save/load,
  F6/F9, F7, console and post-resume movement/mouse checks pass.

- `matrix.json`: six real-input runs at 60/120/240 FPS, starting in first and
  third person, with 1,608 sampled states. Two console/pause cycles per run,
  focus loss/regain, F7 open/close, gameplay saves loaded after switching to the
  opposite view, three rapid F9/Escape sequences per run (0/20/120 ms delays),
  and paused debug save/load/resume pass. No scheme change or selected-view
  change occurred. Paused debug loads deliberately defer render-state
  acquisition until resume, while keeping the stored background and preference.
- `worlds.json`: live console level transitions 0 -> 1 -> 2 -> 6 -> 0, plus
  a same-level 0 restart, regain captured first person; console checks pass in
  each destination. This exercises the original restart/load path through the
  existing debug level command, not natural campaign exits.
- `vanilla.json`: gameplay save/load, original pause, paused save/load and
  explicit resume pass. Paused saves were not auto-resumed.
- Console and paused-load captures were visually inspected. The debug server
  is not serviced inside F7's blocking host modal loop, so F7 is sampled after
  closing, not claimed as a continuous debug trace inside the modal loop.

The six-run matrix preceded the final bounded teardown and airborne-acquisition
additions. Final native controls tests include both additions; final live level
transitions/same-level restart and Vanilla checks use the final executable.
The menu route was repeated on that executable. No broad test result is
presented as a per-frame hardware trace or a full-campaign fidelity claim.
Swimming/jetpack first person, every death/scripted-camera sequence and physical
monitor play feel remain outside the automated verification. The user accepted
the candidate; these coverage limits remain documented.

Final executable SHA-256:
`a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec`.

No new function hooks, generated C edits, media edits or framework edits.
The user authorized implementation/test/documentation commit and push as
MusicMonsterMod. The runtime patch was re-exported unchanged and applied to the
clean pinned framework `08ec704a974b1f3a16335b4afeb340b9eff19926`; all 51
patched files match the current framework worktree. No submodule pin changed.
No further job was started.

Launch with the player's saved preferences:

```sh
python3 recomp/tools/local/run.py
```

Automatic capture remains in effect; F10 is not needed.

