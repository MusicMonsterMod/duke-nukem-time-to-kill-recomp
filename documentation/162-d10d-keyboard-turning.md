# D10D - Keyboard turning (arrow keys)

Job: [D10D](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "perfect, lock it in, close it out").
Executable `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892` is the regression baseline.

## Decision

User (2026-10-20): "basically i just wanted a simple turn left, turn right,
for the arrow keys ... this would be global. it doesnt affect the wasd keys,
it will just always be there as the functionality of the arrow keys". Rates:
Duke 3D's (chosen from the offered options).

## Reference

Duke 3D `getinput` (game.c; EDuke32 keeps the same constants): while a turn
key is held, `PREAMBLETURN` 5 build angle units per 30 Hz game tic until the
key has been held `TURBOTURNTIME` (`TICRATE/8`, 1/8 s), then `NORMALTURN`
15, doubled when running. 2048 units are a full turn, so about 26 degrees/s
for the first 1/8 s, then 79 degrees/s walking and 158 degrees/s running. A
clean-room implementation of those rates; no Duke 3D code is used.

## Implementation

- `recomp/src/ttk/input_bindings.def`: two held host actions appended
  (wire order unchanged for every older action): `turn_left` Left,
  `turn_right` Right. No pad bit, never guest commands (the edge-command loop
  stops at `mission_next`).
- `recomp/src/ttk/modern_controls.cpp` (`key_turn`, `turn_yaw_at`) runs in
  `orbit_begin` beside D10C's `key_look`. It integrates the rate model on
  the presenter clock (`input_clock_ms()`), so `late_view` evaluates the yaw
  at each redraw's look time and the D17B late camera turns smoothly between
  game frames at high refresh. Mouse yaw adds on top; a turn key stops a V
  swing; pitch is never touched. The running state is `input_running()`
  (Shift, Caps Lock autorun). Duke follows the view as he does for the mouse
  (D07A facing), so standing, walking, swimming and jetpack flight all turn.
  A dt bound (+-100 ms) keeps pauses and hitches from jumping the view.
- `recomp/src/ttk/pc_input.cpp`: captured, Left / Right are no longer the
  fixed escape-hatch D-pad Left / Right (which started the original tank turn
  anims 70/71, fighting the modern camera). Jetpack flight no longer reads
  them as strafe (`InputFrame::arrow_x` removed; Up / Down still fly forward
  / back). Without the modern camera lease (tank fallback: turrets, scripted
  cameras, identity loss) the bound turn keys feed the original D-pad turn,
  so the arrows still turn Duke there. Uncaptured (menus) nothing changed.
- KEY BINDINGS (CAMERA) gains TURN LEFT / TURN RIGHT with a one-line hint;
  the menu shows the keys as LEFT ARROW / RIGHT ARROW. The launcher knows the
  tokens `Left` / `Right`; schema 35 adds the keys to older profiles where
  free (`add_missing_actions`). `--show-settings` lists them.

Vanilla is unchanged by construction: bindings are parsed only in Modernized,
the actions carry no pad bit, the pad path above returns early outside
Modernized, and the camera code runs only on the independent Modernized
camera.

## Verification

Lab `recomp/analysis/d10d-turn/turn.py` and `jet.py` (private profile and
card copies, offscreen on the real GPU, port 9381, level 1 savestate slot 9):

| Step | Third person | First person |
| --- | --- | --- |
| Right held 1.8 s, walking pace | 130 degrees, about 73 degrees/s incl. the slow start | 137 degrees |
| Left + Shift held 1.8 s | 261 degrees (running rate) | similar |
| W + Right | turns while walking forward | same |
| A + Left | turns while strafing | - |
| Right + mouse | adds (208 degrees/s together) | - |
| Page Up + Left | look up and turn at once; pitch back on release | - |
| V then Right | the turn key stops the V swing | - |

Duke's heading follows the view in every case; the pitch never moves from
turning; the pad never carries D-pad Left / Right (0x20 / 0x80) in captured
play. Jetpack (`dnstuff`, J, Space): Right turns 70 degrees in 1 s,
Left + Shift 126 degrees; with only an arrow held Duke hovers (0 horizontal
movement), W + Right flies forward while turning.

KEY BINDINGS (D19G harness, `analysis/d10d-turn/keys2.py`, slot 5): TURN LEFT
/ TURN RIGHT rows show LEFT ARROW / RIGHT ARROW and the hint; binding TURN
RIGHT to Q swaps with QUICK KICK (QUICK KICK on RIGHT ARROW), saved as the
profile tokens `Q` / `Right` (the harness profile was restored afterwards).
A copy of the player's profile migrates from schema 34 to 35 with
`turn_left=Left`, `turn_right=Right`.

Tests: `ttk-controls-test` (D10D group: walk rate with the slow start,
release, running doubles, left back, mouse adds, V swing stopped, pitch
untouched; all levels pass); `ttk-input-test` (arrows held as turn actions,
never D-pad bits on foot or in flight, never commands; D-pad turn in the
tank fallback; Classic flight); Python 157 OK (2 skipped): schema 35
migration, `--bind` and conflicts, wire order.

## Limits

- Key state is read at game camera updates (20-30 per second), so a press or
  release takes effect up to one update late (the late camera can draw up
  to that much past a release); as for D10C's look keys.
- Turning is not in Vanilla, by design.
- Not verified: feel on the real display, and every scripted-camera state
  using the tank fallback (only the unit path is tested there).
