# D26J - The console holds input; Duke keeps his state

Status: Done (user-accepted 2026-11-02: "excellent work, accepted"). Board: `MODERNIZATION_JOBS.md` D26J.

## Cause (confirmed)

Opening the backtick console (`debug_console_event`, runtime `main.cpp`)
called `ttk::input_release()`, and the main loop then called
`ttk::input_allow_capture(false)` on every frame while the console was open,
which released again (a new input epoch each frame). With capture off,
`input_frame()` published gameplay frames with `active = false`. Every
Modernized owner treats an inactive frame as "not mine": `crouch.inc` reads
`f.active && f.held[crouch]` (the C latch is only published in an active frame),
and `jetpack_input_ready()` requires an active frame, so the host flight owner
stepped back and the original pack, without its hold, came down.

Same build, same private save, `MIMIC_OLD=1` (the probe drops capture while
the console is open, the pre-D26J frame): the toggle-crouched Duke stood up
(animation 178 -> 63) on the first sample with the console open, and the
jetpack at a held altitude (y -12515) fell to the ground within about 2 s
(-12386, -10922, then landing animation 105 and standing 63). The C latch
itself was never cleared (crouch returned after close); the inactive frame
was the cause.

## Change

The console is an overlay with its own keyboard focus, not a capture loss.

- `ttk::input_console(bool open)` (`recomp/src/ttk/pc_input.cpp`). Opening over
  Modernized captured gameplay frees the mouse (relative mode off) but keeps
  `captured`, the input epoch, the look totals and the C crouch latch. It drops
  held keys, mouse buttons, look motion, queued commands, cheat typing, HUD
  steps, a pending E interaction, and blocks held pad buttons/triggers until
  released. While it holds, `input_frame()` publishes an **active frame with
  no new input** in the same epoch, and the pad does not feed gameplay.
- Closing recaptures the mouse and takes still-held gameplay keys from the SDL
  device state (`resync_held()`, as Escape/resume does). No new epoch.
- Not captured when it opens (menus, Legacy uncaptured, Vanilla): the old
  `input_release()` path, unchanged. Automatic capture waits while the console
  is open.
- Focus loss while the console holds keeps the hold (round 2, below).
- Runtime `main.cpp` (framework patch): open calls `input_console(true)`,
  `debug_console_close()` calls `input_console(false)` (Escape, backtick and
  commands that close it: `level`, `spawn`, `endlevel`, ...),
  `input_allow_capture()` no longer includes the console, and window/focus
  events now reach the input layer while the console is open.
- Debug JSON (`ttk_input`): `console_open`, `console_hold`, `active`, `epoch`.
  Diagnostic input scripts (`DNTTK_TEST_INPUT`) gained `grave` and `1`, and
  their direct key writes are dropped while the console holds.

Held keys: a key held when the console opens is released for gameplay (its
key-up goes to the console). If it is still physically held when the console
closes, it acts again at once; otherwise it stays released. Momentary Ctrl
crouch is therefore released (Duke stands up through normal clearance if Ctrl
is not held at close); the C toggle is kept. Space held while flying is
released, so the jetpack holds its current altitude.

## Evidence (executable `ddcfbd583309d54e8b5ff9355a3c3c61961152df4bc35249ce9dcd687f5c0284`)

Probe `recomp/analysis/d26j-console-hold/probe.py`: private copy of
`saves/local-play` (savestate slot 11, level 0 alley), Xvfb with the x11
driver, debug port, the real console opened and closed with backtick key
events through the SDL event pump. Player saves, cards and profile hashes
unchanged after every run.

- Crouch (third and first person): C, console open 10 s while typing W, A, S,
  D, C, Space, E, 1 and a left click: animation 178 throughout, position
  unchanged, frame active, epoch unchanged, equipment unchanged. Closed: still
  178; D moved Duke on the first hold (crouch-walk); C stood him up.
- Escape closes the console with the same result (178 kept).
- Jetpack (EDuke32 flight, third and first person): lift-off, Space climb to
  y -12515, console 10 s with typing: y unchanged, animation 164 / mode 10,
  fuel kept draining as while hovering
  (the game's hover drain; rate not separately measured). Closed: same altitude; W flew forward at once.
- W held through open/close: Duke stops while the console is open (no input),
  walks again on close while W is still held, stops on release.
- `level 1` from the open console: travel, then captured and active on the new
  level, as the same travel without the console (Duke Hill starts in the same
  scripted pose in both runs).
- `MIMIC_OLD=1` reproduction above (both user symptoms).
- Native: `ttk-input-test` new D26J group (active frame and epoch kept for 600
  frames with offers arriving, C latch kept, no movement/look/commands/jump/use/
  fire, pad shows only the kept crouch, recapture on close, held W released,
  first press after close, Space released in flight, focus loss while open
  releases and no capture until close, uncaptured open keeps the old release).
  `ttk-controls-test` (122 groups), `test_pc_input.py`, `test_player_profiles.py`
  pass. Framework patch exported and applied cleanly to the pinned clean
  dependency, reproducing the working tree.

## Round 2 (user playtest, 2026-11-02)

User: "it still seems to be taking mouse input and it just vibrates around
when the console is open", and with the console open, defocusing the window
made a crouched Duke stand and a jetpack Duke land.

- Vibration: the presenter's late look (`input_live_look`) peeks queued SDL
  mouse motion so a late camera sees it early. The console swallows those
  events, so the camera leaned toward motion that never reached the look totals
  and snapped back. The peek is now skipped while the console holds input. The
  native test fails without this guard (a pushed motion event moved the live
  look) and passes with it.
- Defocus: round 1 deliberately made focus loss end the hold (a real release),
  which published inactive frames again. Now focus loss or minimizing while the
  console holds only notes it (`hold_focus_lost`); the hold already gives
  gameplay no input. Refocusing continues the hold without a new epoch. If the
  console is closed while the window is still unfocused (only possible through
  the debug port), that close is a real release.
- Diagnostics: while the console holds, `DNTTK_TEST_INPUT` no longer forces
  `focused`/`captured`, so a real X focus change is visible to the probe;
  `ttk_input` reports `hold_focus_lost`.

Evidence (executable `d28b1a8e8cbc23c2bfc4933d4e48d9d4050d12011c778956a03cd6219d5b34bf`):
probe cases `defocus` and `jetdefocus` (focus moved to another X window with
xdotool for 5 s with mouse motion, then back; `hold_focus_lost` true while
away): crouch 178 kept at the same position through open, away, back and
close, D crouch-walked after; jetpack kept its held altitude (the usual 32-unit
hover-hold wobble) through all of it. Native D26J group extended (defocus and
refocus keep active frame, epoch and latch; the live look ignores queued motion;
close while unfocused releases). `ttk-controls-test` (122 groups) passes. The
vibration itself was not observed offscreen (it is a presentation effect); the
user's check confirms it.

## Audit (by code reading; not separately played)

With the console open every Modernized owner sees an active frame with all
inputs released, the same as a player letting go of every key: swimming drifts
as on release, ladders/poles hang, a push/pull grab lets go (Grab is released),
precision aim ends (Mouse2 released), steroids and other timers keep running
(the game is not paused). D26K (holster while the console is open, [note 363](363-d26k-console-idle-hold.md)) shares this
path; equipment did not change in these 10 s windows, but D26K's 60 s check
and its cause are not part of D26J.

## Remaining

Accepted after the round 2 build. Outside scope: focus loss without the
console still releases gameplay; D26K is a separate job.
