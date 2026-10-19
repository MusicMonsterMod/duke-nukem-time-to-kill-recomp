# D08AB - SWIMMING option: modern or original water speeds

2026-10-19. **Done (user-accepted 2026-10-20);** executable `c7557a21...` is the
regression baseline. User request on accepting D08AA: "I want one
job in the backlog to add a toggle between modern / original swimming in our
menu". Executable
`c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508`.

## Behaviour

A SWIMMING row in OPTIONS > MODERNIZED, after LANDING: MODERN (default) /
ORIGINAL, live (`ttk_live_env`), saved in the profile (`swimming`, schema
33), `run.py --swimming modern|original`, terminal settings key **U**,
environment `DNTTK_SWIMMING`. Vanilla always gets `original`.

- MODERN: D08AA unchanged ([note 156](156-d08aa-unified-pace.md)).
- ORIGINAL: modern controls with the pre-D08AA water speeds. User choice
  (2026-10-19): this includes the D08P forced run in shallow water.

## Change

`swimming_modern()` (`swim.inc`) reads `DNTTK_SWIMMING`; with `original`:

- `wade_full_speed_ready()` is back (Modernized, gameplay, shallow land-gait
  water below depth `0x281`) and forces `input_running()` and the pad walk
  bit to run, as before D08AA;
- the mid-depth wade root uses the old band (walk 2x clip in 40..60, run 4x
  in 80..120) instead of the pace step;
- the land-anim free-swim root is the old fixed 48 per update;
- `swim_pace()` (the `0x80066e34` hook) leaves the original stroke and
  thrust velocities alone.

Steering, swim-fire, dive, vertical movement and transitions use the same code
in both. No new hooks or guards; generated code untouched.

## Measured (private lab `recomp/analysis/d08ab-20261019/`, ignored)

Dated copies of the D08AA lab cards and savestates, private profile, Xvfb
`:97`, port 9361. Units per vblank, horizontal; walk (W) / run (Shift+W).

| State | ORIGINAL | MODERN | D08AA "before" |
| --- | --- | --- | --- |
| Land (file 09) | 10.55 / 50.0 | - | 10.5-10.8 / 49.7 |
| Ankle-deep subway (file 07) | 49.5 / 49.8 (forced run) | 10.4 / 50.1 | 49.8 / 50.2 |
| Surface W / D / S (file 02) | 26.1 / 6.05 / 15.1, Shift same | 10.4 / 10.6 / - | 25.5 / 6.0 / ~14 |
| Underwater W (file 02 + Ctrl dive) | 27.6 / 26.8 | 10.6 / 47.9 | 27.6 |

File 10 (the D08P mid-depth save) starts Duke in water shallow enough for
the land gait, so with ORIGINAL it measured the forced run (49.4 / 49.8)
rather than the 80/81 wade clip; the wade band itself is covered by the native
test (walk 44, run 88 from a 22-unit clip tick, the pre-D08AA numbers).

Live change, through the real pause OPTIONS > MODERNIZED > SWIMMING while
surface swimming (`live.py`): MODERN -> ORIGINAL 9.8 -> 25.1, ORIGINAL ->
MODERN 24.7 -> 9.8; the profile file then held the new value.

Regression route (`reg.py`, D08O4 route) in both modes: Ctrl dive 133 ->
128 -> 127 and the D08O4 stop on release, swim-fire masks / aims / arm aims
and 4 shots, Space surfacing to 122, Ctrl+W dive all as before; plain W
underwater 30 updates: ORIGINAL 987 (the pre-D08AA figure), MODERN 383.
Vanilla (`--mode vanilla`, pad Up): surface 25.4, subway 53.9 (original run).

Suites: `ttk-controls-test` (fixture `analysis/pc-input/d07-final-camera`;
new D08AB cases: wade band, forced run on / off / Vanilla, debug field,
strokes and thrust kept, live switch back), `ttk-input-test`, `ttk-aim-test`,
`ttk-near-test`, Python unittest (149, 2 skipped; new `SwimmingOptionTest`:
migration 32 -> 33, CLI / env, Vanilla, invalid value, terminal menu).

## Limits

- The deep-wade (80/81) ORIGINAL band was checked by the native test, not
  measured in the lab (the save starts in land-gait water).
- The menu description shows the MODERN meaning only, as the neighbouring
  rows do.
