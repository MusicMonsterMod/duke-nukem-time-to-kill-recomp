# D53F - TTK-look security monitor lamps turned down

2026-11-21. Done (user-accepted at 60%: "its perfect. lock it in and close it out"). Labels: U user request, S source reading, L
isolated runtime observations.

## Request

U, after D53E: "let's start by turning down the vanilla monitor lamps, which
feels like the simplest option", "so only the vanilla ones. our ones are
perfect". The "vanilla" monitors are the TTK look of SECURITY MONITORS in
Modernized; the Duke 3D TVs stay as they are.

## Cause (note 528)

- S: the D41D1 sign scan gave each monitor a lamp 300 in front of its screen:
  pure red (1, 0, 0), strength 1.0, radius 1576 (`sign type 858` in
  `assets/lights/level00.lights`, 818 in `level05.lights`, 1000 in
  `level09.lights`). Three side by side flood the room. They are baked
  (D41P2), so `lamp off` does not hide them.
- Why the Duke 3D look shows them weaker was not traced; not needed here.

## Change

- `level_lamp_monitor_scale` (`level_lights.inc`): a lamp whose prop is drawn
  with the TTK monitor model (`security_monitor_object`, the D53 model test,
  either look; cached per lamp) is scaled while the TTK look is on
  (`security_monitors_ttk`: Modernized, Polymorph, SECURITY MONITORS TTK).
  Applied to the live lamp list (`level_lamps_publish`) and to the bake's lamp
  colours, which are set every game tick (`lamp_bake_levels`), so it follows a
  live option switch with no rebake and reaches the redraw workers in
  `LightView`.
- Default 0.6. Developer `DNTTK_TTK_MONITOR_LAMP` (0-1; 1 the old light).
  First 0.15; U: "now i think a little too dim. if this was a percentage -
  let's say you went from 100%, this feels like about 10%. Let's try 60%".
- Found by model, not level or type: any level using the monitor model gets
  it. No lamp file, framework or generated code change.

## Checked (L, offscreen, real GPU, private card copy)

Frame mean RGB, TTK look, Duke 900-1100 in front of the monitors
(`recomp/analysis/d53e/d53e.py lamps`, local):

| Level | 1.0 (before) | 0.6 (default) | 0.5 | 0.3 | 0.15 | 0 |
| --- | --- | --- | --- | --- | --- | --- |
| LEVEL05 | 214, 52, 53 | 188, 52, 53 | 175, 52, 53 | 134, 52, 53 | 90, 52, 53 | 45, 52, 53 |
| LEVEL00 | 101, 23, 21 | 82, 24, 21 | | | 47, 25, 22 | |
| LEVEL09 | 80, 10, 10 | 54, 10, 10 | | | 22, 10, 10 | |

- DUKE 3D look, LEVEL05: 152.8 / 152.5 red mean with scale 1 / 0.3
  (unchanged within noise).
- Live switch at 180 Hz (DUKE 3D to TTK and back, play, pause, resume): the
  TTK picture matches the launched TTK look (centre 97.4); back to DUKE 3D as
  before.
- `ttk-near-test`, `ttk-aim-test`, `ttk-controls-test`, `ttk-input-test`,
  `ttk-live-env-test` pass.

Pairs: `research/before-after/D53F/` (local).
