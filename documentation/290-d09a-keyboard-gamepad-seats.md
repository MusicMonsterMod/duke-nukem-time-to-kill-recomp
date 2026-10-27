# D09A - Keyboard and gamepad seats

Status: Done, user-accepted 2026-10-27. User confirms controller/keyboard
routing "100% works" and local two-player works as a foundational first step.
Implementation, tests and documentation commit/push closeout explicitly authorized.
Candidate-stage verification and remaining coverage below retain their scope.

## Player setup

Launch normally. In OPTIONS > CONTROLS set TWO PLAYER INPUT to KEYS AND PAD
and confirm the page. Choose TWO PLAYER. Keyboard arrows choose P1's character
and Enter confirms it. The first assigned gamepad chooses P2's character with
D-pad/left stick and confirms with Cross/A. Confirm again once both are ready.
The character screen identifies P1 keyboard and P2 first gamepad, or requests
a gamepad when none is attached. TWO PADS retains the existing configuration.
The preference is saved independently of guest savestates.

Matches retain original turning and gameplay rules. Keyboard P1 can use
W/S to move, A/D to turn, Space to jump, Ctrl to crouch, Shift to walk,
X or bound Fire (default left mouse) to fire/action, C or bound Holster
(default Scroll Lock) to draw/holster, E for the original action, and
Alt or right mouse for original precision aim. Hold C with left/right to
cycle weapons. Arrow keys remain available. Escape pauses; Enter resumes.
P2 uses the original controller controls, including Start to pause.
This does not add independent split-screen cameras or mouse look.

## Implementation and ownership

The audit found that runtime `capture_pad_slot` merges each seat's keyboard
bindings into its assigned gamepad. D09A adds a Modernized, explicitly selected
two-player branch: P1 reads only the title keyboard input, P2 reads only the
first assigned physical controller, and spare controllers contribute neither.
The controller menu path receives seat 2, so it cannot confirm P1's character.
Existing D34B frontend/game player-count fields determine scope; no new guest
hooks, unverified addresses or generated-code edits were introduced.

Physical handle assignment stays intact. The initial hotplug experiment found
that closing every handle after any removal could promote the spare pad into
the active seat. Mixed mode now closes only detached handles. Unplug leaves a
neutral guest seat and reconnect reuses the vacant host assignment. Rumble
routing follows P2's guest seat to the first pad; physical rumble was not tested.
A routing-entry release mask prevents held pad buttons becoming fresh input.
Focus and savestate guards still neutralize input.

The keyboard's existing bound original-action path also runs without mouse
capture in mixed matches. Bound mouse buttons are recorded without executing
single-player camera/traversal commands. Modern campaign input remains mixed
keyboard/mouse/gamepad; Vanilla does not enter any D09A branch.

Profile schema 56 adds `split_devices`, with `two-pads` as the migration/default
value and `keyboard-pad` as the explicit choice. Existing controller preferences
are retained. OPTIONS uses the existing confirmation and persistence workflow.
The diagnostic virtual-pad file now accepts decimal 4294967295 to unplug a
seat, allowing normal SDL hotplug testing without touching physical devices.

## Verification

- Player build succeeds. Native SDL input, controls and aiming suites pass.
  Input regressions cover uncaptured mixed-match movement/jump, mouse fire,
  releases, focus loss, frontend routing and unchanged campaign behavior.
- 106 profile tests pass, including schema-55 migration, retained controller
  speed, saved mixed-seat choice and invalid-value rejection.
- Private Xvfb/X11 keyboard events and two SDL virtual gamepads exercise MESA.
  Keyboard Right changes only P1's character; first-pad Right changes only P2.
  Separate confirms set lock bits 1 then 3, and a fresh confirm starts play.
- Keyboard and gamepad travel change only their respective player's X/Z.
  The spare pad has no effect. Vertical settling is excluded from movement
  comparisons. Keyboard jump affects P1. Held keyboard mouse fire and gamepad
  fire reduce their own ammunition; captures show the separate weapon states.
- Escape/Enter and gamepad Start/Cross pause and resume. A private match
  save/load succeeds. MAIN MENU from pause reaches results; gamepad Triangle
  exits results to the main menu.
- With P2 moving, virtual unplug stops movement after native deceleration;
  subsequent idle samples retain X/Z. Keyboard remains usable, the spare pad
  does not take over, and reconnect restores P2 movement.
- Changing TWO PLAYER INPUT through the real menu to TWO PADS restores
  independent first-pad/P1 and second-pad/P2 movement. The final Controls
  screenshot was inspected; the unsupported plus glyph was replaced with
  the readable KEYS AND PAD label.
- A private campaign load supports gamepad movement/look followed by keyboard
  movement. Vanilla campaign load/movement is checked separately.
- The exported complete framework patch applies to a clean pinned archive;
  all 45 modified tracked framework files match. Dependency pins are unchanged.

Private artifacts: `recomp/analysis/d09a/` and
`recomp/analysis/d09a-vanilla/`. These use private settings/cards and port 9497;
physical controllers are excluded from virtual-pad runs. Host device inventory
lists a Sony PLAYSTATION(R)3 Controller; no physical button/stick or unplug test
is claimed. Player settings, saves and memory cards were not written.

Executable SHA-256:
`6fb3e0fbaa00440cd438f466ff1d34df0c94e14a0691a48ade1130987bc90d91`.

## Candidate-stage acceptance checklist (historical)

User playtest with keyboard and the physical gamepad on the real display:
select both characters, play a match, fire/use actions independently, pause,
resume, disconnect/reconnect, and exit results. This pass is bounded to MESA
and existing private savepoints, not every arena or a natural full-length
match. Original alternating viewport presentation remains; isolated pause
captures can contain only one updated half. This is not presentation acceptance
or a diagnosis of the historical Red Duke / Blue Duke report.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

## User acceptance and closeout, 2026-10-27

The user accepts this input-seat foundation and requests lock-in/closeout.
D09A is Done. Their request for the next modernization step opens D34C on the
job board as Todo; it does not start its implementation. D34C adds per-player
modern movement, camera and aiming plus per-viewport presentation while keeping
classic local play available. Networking remains separate.

The checklist above records what was pending when the candidate was delivered.
User acceptance is not a claim that every listed hardware combination or arena
was newly observed by the agent. No gameplay edits, rebuild or launch in this
closeout; the executable hash and existing evidence remain unchanged. The
historical character-selection report's cause remains undiagnosed.

Closeout verification: complete framework export applies against the pinned
clean dependency and matches all 55 authored files (45 modified tracked files
and 10 added framework sources already carried by the patch). Dependency pins
and the accepted executable hash remain unchanged. Repository privacy and
whitespace checks pass. GitHub authentication and commit identity are
MusicMonsterMod.
