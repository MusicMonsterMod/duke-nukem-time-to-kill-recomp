# D19U / D19U1 - Live HUD levels

Status: Done, user-accepted; closeout authorized 2026-10-28.
Selected D19U followed by D19U1. The closeout includes the already-accepted
D19X/D19Y camera sliders.

## Player behavior

Modernized gameplay uses number-row Minus for less HUD and Equals (with or
without Shift) for more HUD. Full -> Borderless -> Hidden and the reverse
clamp at their endpoints. Holding a key does not auto-repeat the step.
Options > Gameplay > Status Bars offers the same three choices. Full is the
default. Confirm applies menu changes, Cancel discards them and Defaults
restores Full. Shortcuts apply immediately and save the preference.

Key Bindings exposes LESS HUD and MORE HUD. Keypad Minus/Plus and mouse
buttons are supported rebindings, not additional fixed shortcuts. Console,
menus, binding capture, focus loss and load boundaries cannot carry a pending
HUD press into gameplay. Existing bindings retain their IDs; the actions are
appended. A conflicting custom binding wins and leaves a new HUD action unbound.

## Ownership and implementation

- The native settings word at `0x800c3f9c`, bit 3, remains the visibility owner.
  Full and Borderless set it; Hidden clears it. This retains the existing Off
  semantics and does not change the crosshair or inventory browsing.
- The menu owns the saved `status_bars` preference and `DNTTK_STATUS_BARS` live
  value. It reapplies visibility after loads and transitions. Profile schema 60
  adds the preference and bindings. Pending older menu confirmations migrate
  their binding list without discarding accepted FOV/distance preferences.
- Original memory-card status-bar and auto-aim values are restored by the card
  packing path, keeping the Modernized preference separate from Vanilla.
- Owned executable inspection identifies all seven native status-box calls as
  using sprite record `0x800c44a4`, width 46 and height 16, independently of
  icons and digits. Borderless temporarily sets its width to zero only in the
  authenticated one-player HUD composition scope. This produces a degenerate
  frame quad without changing texture data, icons, values or their positions.
  The original record word must match `0x102e3a3f` before substitution.
- Both normal and alternate composition exits restore the width. The original
  widescreen layout scope restores positions. The selected-gadget, steroids,
  timed-power-up and challenge boxes omit only their separate frame draw.
  The challenge icon's alignment is calculated using the original dimensions.
- RenderState carries the borderless choice with each captured frame. Replay
  workers must not use their fork-time copy of live settings. Full/Hidden
  rendering and HUD depth order otherwise keep their original paths.
- Input presses use a bounded atomic queue, preserving order without a mutex
  or allocation inherited by forked render workers. Loads and release clear it.

No framework edit, new hook, generated-C edit or retail asset change is needed.
Split-screen retains its native border layout; the new borderless scope is
single-player, matching the existing modern gadget/widescreen HUD scope.

## Verification and rejected attempts

D19U's initial two-state candidate was built and checked separately: minus
hid the native HUD and plus restored it, with endpoint presses harmless.
Evidence: `recomp/analysis/d19u/`.

D19U1 evidence:

- `recomp/analysis/d19u1/`: 60-setting, 1280-pixel 16:9 window; Full/Borderless/
  Hidden in both directions, endpoints, save load, actual Gameplay menu,
  menu-key isolation, Confirm/Cancel, Defaults and full process restart.
- `recomp/analysis/d19u1-final/`: final 120-setting, 1280-pixel 21:9 window;
  steps, endpoints, reload, active jetpack/goggles/steroids, a second level's
  private save, console isolation and replay telemetry. Full-window captures
  include the widescreen HUD margins omitted by canonical screenshots.
- `recomp/analysis/d19u1-reload-atomic/`: three consecutive borderless reloads
  at the 120 setting, with advancing frames, intact restored border record
  and successful screenshots after each load.
- `recomp/analysis/d19u1-small/`: 60-setting, 640-pixel 4:3 window; all
  three appearances, both directions and endpoint clamping.
- Native input regressions cover ordered edges, repeat suppression, menu and
  binding-capture isolation, keypad rebinding, old-key removal and load reset.
  Native actual-hook regressions cover the exact record, alternate exit,
  unrelated caller, Vanilla, changed-record rejection and a forked replay
  whose environment disagrees with the captured HUD mode.
- Player build and controls/input/aim/near suites pass. Python profile/input
  checks (111 profile tests and 17 input tests) cover migration, persistence,
  old menu confirmations and Vanilla
  launch exclusion. Every private live route compares hashes and mtimes of
  all 41 player files; none changed.

The first enum implementation reversed the environment/JSON fields and failed
sidecar parsing. It was corrected before the successful lifecycle route.
The first high-refresh implementation kept borders in replay frames because
workers retained the launch-time setting. The captured render-state field and
forked regression address that observed defect.

Three intermediate 120-setting attempts stalled after reload (debug requests
timed out and the fresh-frame wait expired). Preserve the private saves and
logs in `d19u1-cover-final/`, `d19u1-cover-retry/` and `d19u1-reload/`.
Alternate-exit restoration was added, then the input queue was made lock-free.
The final repeated-reload and complete coverage routes passed. Fork-inherited
locking is a suspected contributor, not a proven root cause; do not describe
this as an established fix for any earlier campaign or overhead save halt.
A later host check found available memory and no current I/O pressure; it
cannot establish host conditions during those stalls.

## Acceptance and retained limits

User accepted the result and authorized closeout: "lock it in, close it out!"
The checks remain bounded: not an
exhaustive campaign, every power-up combination, every aspect/DPI or Windows
certification. No new full-motion acceptance is inferred from screenshots.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Implementation, tests, manual and evidence are included in the authorized
MusicMonsterMod commit/push closeout. No next job selected.
