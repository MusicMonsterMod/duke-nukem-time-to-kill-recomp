# D04 — PC action input, bindings and capture

Current controls, schema 7 and the guest-acknowledged shortcut queue are described
in [the D08A engineering note](33-controls-shortcuts.md). The details below record
the historical D04 checkpoint. [D05/D06](20-modern-movement-camera.md) now
apply movement/look data in verified first-map states and extend profiles to schema 3.

The player launcher now selects an actual native input path. Vanilla retains
`keybinds.ini` and controller translation. Modernized enables the game-specific
PC input module, with keyboard and mouse actions and an explicit captured
Gameplay context. This is an input foundation: movement, camera transforms and
shot direction are not replaced. No guest code, addresses or overlays are patched
by D04. D05/D06 still need their guarded game hooks.

## Player behavior

Modernized starts with the mouse released, in Menu context. Arrow keys, X
(Cross/confirm), Z (Triangle/back), C (Circle), Enter (Start/pause/skip) and Right
Shift (Select/inventory) remain available in both contexts. They cannot be
rebound by the action editor. Arrows retain original tank movement in gameplay.

F10 toggles gameplay capture. Escape releases it. Enter, inventory, focus loss,
minimization and opening a host overlay also release it. Regaining focus or
clicking does not recapture; use a fresh F10. Held action state and pending mouse
deltas are cleared at release/capture boundaries. Mouse buttons act only while
captured; the click used to focus a window cannot also fire. Function keys and
Ctrl/Alt/GUI shortcuts release capture. Capture failure leaves Menu context.

Captured action defaults:

| Action | Input | Current effect |
| --- | --- | --- |
| Independent movement axes | W/S/A/D | Exposed to future D05 hooks only; no PS1 turn buttons |
| Fire / holstered action | Mouse1 | Cross |
| Jump | Space | Square |
| Draw/holster | H | Circle |
| Quick turn / crouch | V | Triangle; original game semantics |
| Walk | Left Shift | L1 |
| Original precision aim | Mouse2 | R1; still use arrows |
| Original strafe left/right | Q/E | L2/R2 |
| Inventory | I | Select, then releases capture; Right Shift closes inventory |
| Look delta | Relative mouse motion | Frame data only; no camera or aim changes yet |

Historical D04 table. Since D08T1 (schema 17) Mouse2 is **Grab / Manipulate**
(`grab`, plus `grab_alt` on Alt) and `original_aim` is Unbound in Modernized;
with legacy aiming the `grab` input serves precision aim. See
[70-d08t1-grab-manipulate.md](70-d08t1-grab-manipulate.md).

Original physical controllers remain on their existing runtime path in either
profile. Keyboard/mouse activity is identified in the action snapshot. Modern
controller axes/dead zones/hot-plug action semantics remain D09; no tested hardware
claim is made here. Custom runtime host hotkeys should stay separate from the
listed PC bindings; D04 reserves the default host keys. Arbitrary custom host
hotkey conflicts are not imported into the terminal editor.

## Persistence and editor

`run.py --settings`, choice 6, edits Modernized bindings. The terminal lists
implemented actions, accepted input tokens and reserved controls. Multiple
`ACTION=INPUT` assignments perform an atomic swap. Conflicts name both actions
and leave bindings unchanged; there is no silent unbinding. Command-line edits
also save and return without launching:

```sh
python3 recomp/tools/local/run.py --mode modernized --bind jump=J
python3 recomp/tools/local/run.py --show-bindings
python3 recomp/tools/local/run.py --bind jump=W --bind move_forward=J
```

Bindings use physical SDL scancodes (the labels describe US key positions).
Accepted tokens are listed in the editor. Mouse1/2/3 mean left/right/middle,
with Mouse4/5 for side buttons. Wheel bindings, modifiers/chords and multiple
bindings per action are not implemented. Fixed menu keys are additional aliases.
Restoring Modernized defaults resets its action bindings and renderer. Vanilla
rebinding continues to use the original runtime configuration.

Profile schema 2 adds `bindings` to each profile; only Modernized consumes them.
Version 1 selections and renderer preferences migrate with a byte-preserved
backup. Invalid input maps recover as a whole, with a notice and backup, so a
partial recovery cannot accidentally introduce duplicate controls. Higher
schema versions are refused. The launcher sends a validated versioned numeric
binding payload in child-process environment variables, avoiding temporary
shared input files and allowing simultaneous independent preference files.
A directly launched executable defaults to Vanilla unless explicitly configured
with this launcher contract. Headless probes have no SDL action source.

## Native contract and runtime seams

`src/ttk/input_bindings.def` defines stable action order, default scancodes and
optional PS1 bits; both the Python editor and C++ input module consume it.
Movement actions carry zero PS1 bits. `input_snapshot(Context::Gameplay)` returns
held actions, unit-normalized movement axes, mouse delta and last keyboard/mouse
device. `Context::Menu` returns no gameplay actions. `input_frame()` latches
accumulated relative motion once per vblank and clears the pending accumulator;
read-only snapshots do not consume it. D05/D06 must consume on an appropriate
simulation boundary, apply their own game-state/overlay guards and avoid applying
one frame's look delta multiple times. No sensitivity/FOV option is advertised yet.

Events are handled on the runtime pump thread, including stopped-window and
host-overlay loops; no event watch mutates state from another SDL thread. The
keyboard-to-pad seam selects the PC map only in Modernized. The final physical
pad sample gates both buttons and analog axes on focus for both profiles.
Keyboard analog translation is suppressed in Modernized, preventing an old
keyboard-stick map from leaking into the new actions. Controller pause/inventory
also releases capture. Diagnostic input overrides remain deliberately separate.

The read-only debug command `ttk_input` reports mode, focus, capture, action axes,
look delta, last device and PC pad word. `pad_status` separately verifies the
actual SIO state, including original controllers and Vanilla input.

`time-to-kill-z-pc-input.patch` preserves the narrow runtime seams after the CD
seek and stopped-window patches. Because those patches overlap, the applicator
checks an already installed stack by reversing copies in a temporary directory,
never by undoing live source. Pristine/partial stacks retain sequential checked
application. Conflicting source fails instead of discarding local edits.

## Local commands

```sh
python3 -m unittest discover -s recomp/tests/local -v
cmake --build recomp/build-local --target psx-runtime ttk-input-test --parallel 4
xvfb-run -a recomp/build-local/ttk-input-test
python3 recomp/tools/local/pc_input_probe.py --name UNIQUE --mode modernized
```

The current configure retains the earlier OpenBIOS emitter-fingerprint and
`dev`/VERSION warnings. D04 does not regenerate or hand-edit guest C; the
existing movie shard was checked and remains valid for all 19 entries.

## Verification and scope

See [D04 evidence](reports/d04-input.json) for the final build identity and runs.
The Python tests cover migration, persistence, atomic swaps/rejections, reserved
inputs, menu editing and patch-stack preservation. The native SDL test covers
movement independence, diagonal normalization, context isolation, one-frame
mouse deltas, focus/repeat handling, Escape/pause/inventory/quit release and
blocked recapture inside overlays.

`pc_input_probe.py --name UNIQUE --mode modernized` uses an isolated Xvfb
server, real X keyboard/mouse events, separate cards/preferences, the actual
player executable, and both input diagnostics and SIO assertions. It refuses an
active game or occupied port. A second Vanilla run checks its original mapping
and focus release. Navigation captures require visual review, not just a zero
exit status. `--menu-back` ends the navigation route after returning from the
difficulty screen to the main menu, keeping the return and level-entry routes
separate. Interrupted probes flush their completed assertions incrementally. Virtual-display tests do not establish desktop-compositor behavior,
audible output, controller hardware, Windows or full-campaign fidelity.

The user's high-standard playtest report applies to the preceding candidate.
It is preserved as regression confidence, not labelled as user testing of D04.
