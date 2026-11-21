# D53E - Live options reach the redraw workers

2026-11-21. Done (user-accepted: "it's working! looks great"); follow-up D53F (note 530). Labels: U user request, S source reading, L
isolated runtime observations.

## Report

U, UI slots 9 and 10: "changing the security monitor type back to ttk in
settings does not seem to revert these, weirdly the original ones show in the
pause menu! but when changing the setting, the monitors stay as they are and
you see their original lamp color."

## Cause

- S: OPTIONS rows marked live store their value with `ttk_live_env_set`
  (`live_env.cpp`): a table of process statics. The in-game pictures at any
  FRAME RATE other than ORIGINAL are drawn by the forked redraw workers
  (`render_worker.c`), and a worker's table is the one it was forked with.
  Paused frames are drawn live in the game process, so the pause menu showed
  the TTK monitors while play kept the TV. The game process itself switched
  (the TV bounds went back, so you saw the monitors' baked red sign lamps).
- S: `leave_screen` (`game_menu.cpp`) re-forks the workers when MODEL SHADOWS,
  GLOWS, LIVE SHADOWS or DYNAMIC LIGHTS change (D41S); SECURITY MONITORS was
  never on that list. A GRAPHICS grade that changes those rows does not
  re-fork either (its rows change in `committed` directly).

## Change

The fix is for the whole system, not one row: every published frame now
carries the confirmed option table, and the worker that draws it takes that
frame's values.

- `ttk_live_env_pack` / `ttk_live_env_unpack` (`live_env.cpp`): the table as
  `name\0value\0` pairs ending with an empty name; unpack sets only values
  that differ and keeps its strings for the life of the process, as
  `ttk_live_env_set` requires.
- `RenderState` (`modern_controls.cpp`, the draw-time state every redraw job
  already carries, as the D40 world renderer does) gains a 1536-byte
  `live_env`, saved with each frame and unpacked in `render_state_load`. A
  `static_assert` keeps `RenderState` inside `Frame::render` (16 KB).
- So any live option read at draw time through `ttk_live_env` (SECURITY
  MONITORS, GLOWS, MODEL SHADOWS, LIVE SHADOWS, WIDESCREEN, CROSSHAIR...)
  shows on the next frame in play, also after a GRAPHICS grade. The D41S
  re-fork stays for worker state outside this table.
- Developer `DNTTK_REPLAY_LIVE_ENV=0`: workers keep their fork-time values
  (the old behaviour, for A/B).
- Developer console `option KEY VALUE` (`game_menu.cpp`): confirms one live
  choice row through the menu's own `apply_live` (and the D41S re-fork for
  its rows) for this session; nothing is saved. Example:
  `option security_monitors ttk`.

No framework, generated code or codegen hash change; the D53D savestates
still load. Vanilla: no menu change reaches it and the look stays TTK.

## Checked (L, offscreen, real GPU, private card copy)

Lab: `recomp/analysis/d53e/d53e.py` (local; `switch LEVEL`, `use LEVEL`).
Shots in `recomp/analysis/d53e/shots/` (local).

- A/B in LEVEL05 at 180 Hz, same build, 1.2 s after `option security_monitors
  ttk` in play: the centre of the picture is (176, 43, 46), the original
  monitors in their lamps' red, with the fix; (55, 42, 40), still the Duke 3D
  TV, with `DNTTK_REPLAY_LIVE_ENV=0`. This reproduces the report.
- LEVEL00, LEVEL05 (UI slot 9) and LEVEL09 (UI slot 10) at 180 Hz, each:
  DUKE 3D, then TTK in play, paused and resumed, a TTK camera use (bare view,
  no frame), back to DUKE 3D in play (TV at once), a camera use with the REC
  frame, the TV's live feed after it, paused and resumed. Redraw jobs ran
  throughout (submits rising after each resume).
- LEVEL05 at 60 Hz: both directions in play and paused.
- TTK use distance is the same launched in TTK (`security_monitors: ttk`) and
  switched live: the use opens the camera at Duke x -17120 (the 275 line-up)
  in both. The original panel is 140 flatter than the TV, so it takes a step
  closer than the DUKE 3D TV.
- Unit: `ttk-live-env-test` (pack, unpack, a value from freed memory, nothing
  fits, unterminated input); `ttk-near-test`, `ttk-aim-test`,
  `ttk-controls-test`, `ttk-input-test` pass; local unit tests 268 OK.

## Not changed

- The TTK look's red light from the monitors' baked sign lamps (note 528), turned down since in D53F (note 530):
  U 2026-11-21, "i dont mind that red lamp when on regular mode. lets leave
  for now and i'll check it when you're done and refine from there".
- Not checked on the real display or in the real OPTIONS screen by hand; the
  console runs the menu's own apply path.
