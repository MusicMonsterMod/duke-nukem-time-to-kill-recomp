# D09 - Modern controller candidate

Status: Done, user-accepted 2026-10-27. User reports "controller is working
solidly" and authorizes lock-in/closeout, including implementation, tests and
documentation commit/push. The candidate-stage limitations below retain their
evidence scope; acceptance does not imply unobserved tests were performed.

## Scope and implementation

The assigned first controller now supplies camera-relative stick direction,
independent right-stick look and actions to the existing Modernized campaign
input layer. This is direct SDL input and action state, not synthetic keyboard
remapping. Keyboard bindings do not change pad actions, and keyboard/mouse
remain usable alongside the pad. Walking and running retain accepted game
speeds; stick magnitude is not a new variable-speed locomotion implementation.

The runtime samples its assigned seat handles before publishing the input
frame. During Modern campaign gameplay it suppresses the duplicate original
controller button stream, except Start, while retaining keyboard/action input.
Menus, explicit Legacy, Vanilla and two-player matches retain the native pad
route. No guest addresses or generated C were changed.

Radial deadzone defaults to 20 percent. Right-stick look integrates elapsed
host time at 180 degrees/second, bounded against long frame gaps, into the
existing timestamped camera accumulator. Conversion compensates for mouse
sensitivity and inversion, so controller preferences remain independent.
OPTIONS > CONTROLS adds PAD LOOK, DEADZONE and INVERT CONTROLLER. Profile schema
55 migrates existing preferences and adds these defaults. Integer sliders are
serialized as integers so the profile validator accepts saved controller values.

Buttons/triggers held across capture or focus transitions need release before
acting again. This fixes an observed resume-confirm carrying into a jump in the
first candidate. Disconnect clears controller movement/look and, when the pad
was active, transient action ownership. Held directional input can resume after
capture, as with keyboard movement. Action mappings are fixed for this candidate;
controller rebinding is not provided. See the manual for the complete map.

Character selection now says CONNECT TWO CONTROLLERS, CONFIRM ON EACH
CONTROLLER or BOTH READY - CONFIRM TO START. Independent seat ownership remains
unchanged. Runtime assignment logs identify seat and transient SDL instance,
which distinguishes virtual devices whose SDL mapping displays the same name.

## Evidence

- Player build succeeds. Native input suite passes with actual SDL virtual-pad
  sampling: radial deadzone, normalized diagonals, action edges, independent
  inversion, held-trigger release on focus/capture and device detach cleanup.
  The existing keyboard/mouse, traversal and capture regression cases pass.
- Existing native controls and aiming suites pass with the owned executable and
  local guard fixture. These are bounded module checks, not campaign completion.
- Profile suite: 105 tests, including schema-54 migration preserving mouse
  preferences, independent controller defaults, menu persistence and invalid
  settings rejection.
- Private Xvfb runtime routes at 60 and 120 presentation: old one-seat campaign
  save load, left-stick travel, simultaneous strafe/right-stick turn, pistol
  fire, crouch/release, Start pause, Cross resume and keyboard movement after
  controller use. The corrected 120 route resumes at idle animation 63, rather
  than carrying the confirmation into a jump. Similar turn intervals ended at
  yaw -2.678 and -2.672 respectively; this is bounded cadence evidence, not a
  precision benchmark. God mode and pistol selection isolate the route from
  incidental damage; an earlier route entered death after firing and was not
  used as crouch/pause acceptance evidence.
- Two virtual seats independently change their own character, set lock bits
  1 then 3, and start MESA. P1 travel changes its X/Z while P2 X/Z stays fixed;
  then P2 travel changes its X/Z while P1 X/Z stays fixed. Vertical settling is
  excluded from that comparison. P2 Start/Cross pauses/resumes. MAIN MENU goes
  through results; P2 Triangle returns to the main menu.
- Physical enumeration recognizes one PS3 Controller. A two-second idle sample
  reads LX 128, LY -386, RX -129, RY 642, triggers 0 and no buttons. These values
  are inside the default deadzone. This establishes recognition and sampled
  neutral values only, not a physical movement or button acceptance test.
- Framework export applies against the clean pinned dependency and matches the
  modified tracked framework files. The submodule pin is unchanged.

Private artifacts: `recomp/analysis/d09/` and `recomp/analysis/d09-split/`.
Tests use copied cards/profiles and private debug ports. Player settings,
saves and memory cards are untouched. Menu/character captures were inspected;
clipped slider labels found during inspection were shortened, and the final
Controls screenshot was inspected with all displayed labels inside the viewport.

## Remaining acceptance

The reported Red Duke / Blue Duke blocker is not reproduced with two virtual
pads. Only one physical pad is currently detected; the number and models used
when the original blocker occurred are unconfirmed. Do not claim that a missing
second controller caused the historical report. Each seat still requires its
own controller confirmation.

User playtest is needed for physical stick feel, movement and simultaneous aim,
all buttons, idle drift, disconnect/reconnect, focus and menu transitions, and
two physical controllers through character selection, a complete match and
real-display presentation. This pass does not repeat the prior natural timed
match, all arenas, exhaustive campaign traversal or full Vanilla campaign
coverage. Preserve note 288's bounded earlier evidence. Modern controller
controls apply to the single-player campaign; classic split screen stays classic.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

## User acceptance and closeout

User accepted controller behavior and explicitly requested lock-in and closeout.
D09 is Done. No further gameplay edits or rebuild during closeout. Repository
privacy/whitespace checks and clean-pin framework export verification are the
closeout checks. D09A records the separately requested keyboard/gamepad seat
assignment job as Todo; it is not implemented or started here. The historical
physical two-pad character-selection report remains undiagnosed.
