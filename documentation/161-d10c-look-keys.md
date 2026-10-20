# D10C - Duke 3D look keys

Job: [D10C](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "awesome!! ... lock this current one in, and close it out"). Executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19` is the regression baseline. Follow-up: D10D (arrow-key turning).

## Reference

The user's EDuke32 capture (`research/screencaps/Video_2026-10-20_06-21-12.mp4`,
34 s), stepped at 15 and 8 images/s:

- Page Up / Page Down held: the view tilts up / down and reaches the limit in
  about 0.4 s; on release it eases back to level in about 0.3 s.
- Home / End held: the view tilts at about half that speed (about 0.75 s to
  the limit) and stays there after release.
- A Page Up or Page Down tap after a lock brings the view back to center
  (about 0.8 s from the limit).

## Implementation

Four held actions, appended to `recomp/src/ttk/input_bindings.def` (wire
order unchanged for every older action): `look_up` PageUp, `look_down`
PageDown, `lock_look_up` Home, `lock_look_down` End. They are host-only: no
pad bit, never a guest command (the edge-command loops now stop at
`mission_next`). A Look press is also counted per input epoch
(`InputFrame::look_press_total`, like V's `recenter_total`), so a tap shorter
than the gap between game camera updates (3 vblanks or more) still counts.

`recomp/src/ttk/modern_controls.cpp` (`key_look`, `key_pitch_at`) runs in
`orbit_begin`, the same place as the V swing and mouse look:

- Look held: pitch moves at 150 degrees/s; on release (or a counted tap) it
  eases back to `rest_pitch` (the original follow pitch V also returns to) at
  V's rate, 0.75 per vblank (17.26/s).
- Lock held: pitch moves at 80 degrees/s and stays on release.
- A Look key wins over a Lock key. Any key stops a V swing; mouse motion stops
  the ease back, as it stops V. Pitch stays within the existing +-60 degree
  clamp. Yaw is never touched.
- The model is timed on the presenter clock (`input_clock_ms()`, the clock
  `input_live_look` uses). `late_view` evaluates it at each redraw's look time,
  so the D17B late camera tilts smoothly between game frames at high refresh.
  A dt bound (+-100 ms) keeps pauses and hitches from jumping the view.

KEY BINDINGS (CAMERA) gains LOOK UP, LOOK DOWN, LOCK LOOK UP and LOCK LOOK DOWN
with one-line hints. The menu and the launcher know the new tokens (`Home`,
`PageUp`, `End`, `PageDown`). Launcher schema 34: older profiles gain the four
keys where free (`add_missing_actions`; a key the player already uses stays
theirs and the new action takes a free input). `--show-settings` lists them.

Vanilla is unchanged by construction: bindings are parsed only in Modernized,
the actions carry no pad bit and the camera code runs only on the
independent Modernized camera.

## Verification

Executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19`.

Lab `recomp/analysis/d10c-look/look.py` (private profile and card copies,
offscreen on the real GPU, port 9381, level 1 savestate slot 9), third and
first person. Pitch in degrees, positive down; rest 7.7:

| Step | Third person | First person |
| --- | --- | --- |
| Page Up held | 7.7 to -60 in about 18 frames | same |
| released | back to 7.7 in about 12 frames | same |
| Page Down held / released | to 60, back to 7.7 | same |
| Home held / released | to -60 at about half the speed, stays | stays at -58 (released early) |
| walking with the lock | stays | stays |
| Page Down tap (0 or 2 frames) | back to 7.7, counted as a tap at 0 frames | same |
| End held / released | to 60, stays | same |
| mouse after a lock | moves the view; lock kept | same |
| Page Up tap | back to 7.7 | same |

First-person screenshots: rest, Home lock (sky) and End lock (Duke's feet)
confirm the directions. `recomp/analysis/d10c-look/keys.py` (D19G harness):
KEY BINDINGS shows the four rows and hints; binding LOCK LOOK DOWN to Page Up
swaps with LOOK UP (End) and the profile saves `PageUp` / `End` (schema 34).

Tests: `ttk-controls-test` (D10C group: hold rate, ease back, lock, tap,
mouse stops the ease back; all levels pass); `ttk-input-test` (held, never
commands, presses counted, repeats and Lock not); Python 155 OK (2 skipped):
schema 34 migration with a taken Home, `--bind`, wire order.

## Limits

- The rates are taken from the video; the user accepted them in play.
- Key state is read at game camera updates (20-30 per second), so a press or
  release takes effect up to one update late; the late camera hides this for
  smoothness, not for latency.
- The center is `rest_pitch` (7.7 degrees down in this save), the same angle V
  returns to, in first person too. Duke 3D centers on level; change it if the
  user prefers level.
- Weapon aim follows the view pitch at game rate as before.
