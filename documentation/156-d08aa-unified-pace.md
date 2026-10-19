# D08AA - one walk/run pace on land and in water

2026-10-19. **Done (user-accepted 2026-10-19);** executable `5e48b2c7...` is the
regression baseline. Follow-up D08AB: SWIMMING MODERN / ORIGINAL menu row.
User request (2026-10-09): unify Duke's
horizontal walking and running speeds across land, shallow water, surface
swimming and underwater swimming; Shift released = walk, held = run, at the
land baselines, through one shared speed path. Modernized only; Vanilla
unchanged; land baselines unchanged.

## Measured before (executable `114a7753...`, private lab)

Units per vblank (the debug `frame` counter, ~60 a second), horizontal only.
Lab `recomp/analysis/d08aa-20261019/` (ignored): private card copies, dated
copies of the player's savestates (files 00-09; UI slot N = file N-1) plus
the D08P mid-depth ledge save as file 10, profile copy, Xvfb `:97`, port 9361.
`m.py MODE FILE 'keys;keys'` holds xdotool keys and samples Duke's position.

| State | Save | Walk (no Shift) | Run (Shift) |
| --- | --- | --- | --- |
| Land (72/74, 76/78) | files 03, 06, 09 | 10.5-10.8 | 49.7 |
| Ankle-deep subway (depth 128) | file 07 (UI slot 8) | 49.8 (forced run) | 50.2 |
| Mid-depth wade (80/81) | file 10 | ~11-22 band, walk 2x land | 53.5 |
| Surface W / S / A-D (state 4) | file 02 (UI slot 3) | 25.5 / ~14 / 6.0 | same |
| Underwater (state 5, thrust 128) | file 02 + Ctrl dive | 27.6 | same |

(D17V had measured land at ~10.4 walking and 49-52 running the same way.)

## Causes

- **Shallow water** (depth < 0x200, land gaits): `input_running()` and the
  pad walk bit were forced to run by `wade_full_speed_ready()` (a D08P choice
  from when walk-by-default made wading feel like a crawl). The gait itself is
  the land gait, so nothing else was needed.
- **Mid-depth wade** (0x200..0x281, clips 80/81, handler `0x800539f8`): the
  host retargets the root, but its walk band (gain 2, 40..60 per update) was
  twice the land walk.
- **Surface and underwater** (native swim states, `0x800455bc`): Shift never
  reached them. The common tail (`0x80046e90`) calls `0x800451a0`, which
  writes a body-local velocity to `player+0x1f4` from fixed speeds: surface
  forward 125 `0x1333`, back 126 `0xaaa`, strafe 123/124 `0x444` (local X),
  underwater thrust 128-130 (and Square) `0x149f`, each scaled down by the
  pad's pressure (`0x8001e5b4`, full at 150). `0x80066e34` (ra `0x80046ec8`,
  a0 = the stack copy of yaw / pitch / roll, a1 = `player+0x1f4`) rotates it
  to world space; the tail then moves Duke by `v*dt/1024` with the original
  collision (`0x8003f040` for forward strokes) and writes `+0xfc..+0x100`.

## Change

`modern_controls.cpp`: `modern_walk_pace = 2030`, `modern_run_pace = 9520`
(velocity units, `v*dt/1024` per player update) are the measured land
baselines; `modern_pace(run)` and `modern_pace_step(run)` (per update,
effective dt 10.5) are the one shared path. Every water state reads it; a
later global walk/run retune changes those two numbers (land itself is still
the original gait, so that job also has to bring land onto them).

1. Shallow water: the forced run is gone (`input_running()`, the pad walk bit;
   `wade_full_speed_ready()` removed). Shift/Caps Lock decide, as on land.
2. Mid-depth wade: root length = clip x step / 28, clamped to 0.8..1.2 x step
   (28 = the clip's effective average against land; 25 and 26.5 measured 5-7%
   fast).
3. Free swimming on land-side water anims (70/80/81/175-177 in deep water,
   before the original hands over to its swim states): root step = pace step
   instead of a fixed 48.
4. New entry hook `0x80066e34` (`game.local.toml`, regenerated; one generated
   line, `generated/SLUS_005.83_full_34.c`; codegen hash unchanged - all
   savestates load). A shared rotation (87 call sites), so the plugin compares
   ra first, like D08O2A's matrix hook. `swim_pace()` (`swim.inc`), with the
   swim lease and a WASD direction held:
   - surface 123-126: the stroke's local axis becomes `modern_pace()` with its
     sign and pressure share, so forward, back and sideways are all walk/run;
   - underwater 128-130: thrust along the view pitch becomes `modern_pace()`
     (level swimming = land speed); with Space or Ctrl held (explicit
     vertical) the original vertical speed is kept and only the horizontal
     part becomes the pace, by rewriting the pitch in the stack copy (the body
     pitch Duke is drawn with is not touched);
   - untouched: no direction (idle, Space / Ctrl alone), the dive 133, other
     callers, Vanilla / no lease.

## Measured after (final executable `5e48b2c7...`)

| State | Walk | Run |
| --- | --- | --- |
| Land (file 09) | 10.95 | 50.0 |
| Ankle-deep subway (file 07) | 10.5 | 49.7 |
| Mid-depth wade 80/81 (file 10) | 10.6-11.1 | 49.4-50.1 |
| Surface W / D / S (file 02) | 9.9 / 10.7 / 10.7 | 50.7 / 50.2 / 50.5 |
| Underwater W / A (file 02 + dive) | 10.1 / 10.1 | 49.0 / 51.1 |

Earlier fix rounds with longer windows show the same speeds; runs over 120
updates in the small sewer pool include wall stops.

Regression (same route as D08O4, `reg.py`): Ctrl dive 133 -> 128 -> 127 and
the D08O4 stop on release, Ctrl descend, swim-fire masks / aims / shoulder
aims and shots, Space surfacing to 122, Ctrl+W dive all unchanged; plain W
underwater covers 423 units in 30 updates instead of 987 (walk). Subway wade
jumps (`j.py`): Space alone -> 98 (the D08W forward climb jump), W+Space and
Shift+W+Space -> 98, the walking jump shorter than the running one, as on
land. Falling into deep water (file 00, Shift) enters 108 -> 127/128.
Vanilla (pad input): surface Up 23.7 (the original `0x1333` stroke), subway
Up 52.4 (original run), Square tap 122 -> 133 -> 127 with the D08O4 numbers
(hz 1301, dy 1060).

Suites: `ttk-controls-test` (48 groups; new D08AA pace cases: surface walk /
run / pressure / back / strafe, underwater level and Space-vertical, dive,
foreign caller, no direction, Vanilla; mid-wade numbers updated),
`ttk-input-test`, `ttk-aim-test`, `ttk-near-test`, Python unittest (146, 2
skipped) pass. `ttk-inventory-test` / `ttk-font-test` need asset-pack
arguments and test untouched HUD code; not rerun.

## Limits

- Not verified: the user's playtest (feel of walking strokes, the run speed
  in tight water, transitions).
- Walking now stops at a drop's edge in water as it does on land (file 00:
  the pool rim at depth 0 used to be crossed by the forced run). Run or jump.
- The dive 133, Space / Ctrl alone and the wade jumps' own horizontal top-ups
  (3600 walking / 4800 running, now following Shift) keep their speeds.
- Underwater view-pitched swimming keeps its slope, so looking down at walk
  speed moves 10.5 along the view, less horizontally.
- Land is still the original gait; `modern_*_pace` documents its speed.
