# 464 - D23P2: GRAPHICS, four gradings and AUTO

User, 2026-11-13: "i think it is more realistic to think we have some graphics
profiles like simple, pretty etc, maybe 4 gradings. i want the most out of
this small machine". Built and measured on the field laptop (note 461).

## 1. The grades

A grade sets the five settings that cost the GPU most (D23W: lighting is most
of the GPU work, the internal resolution the rest); every other setting stays
the player's.

| Grade | Resolution | LIGHTING | LIVE SHADOWS | DRAW DISTANCE | FRAME RATE |
| --- | --- | --- | --- | --- | --- |
| SIMPLE | 1x | ORIGINAL | SIMPLE | original | 60 |
| BALANCED | 2x | ENHANCED | SIMPLE | extended | AUTO |
| PRETTY | 3x | RELIT | SHARP | extended | AUTO |
| ULTRA | 4x | RELIT | SHARP | extended | AUTO |

FRAME RATE AUTO keeps its own CPU limits (D23P, D23W: the original 60 on 2
cores), so on the laptop every grade presents at 60.

## 2. Measured on the field laptop

The user's other settings, real display, 30 s walks turning and firing,
`recomp/analysis/d23x/grades.sh`:

| Grade | UI slot | Game frames/s | Lowest tenth | GPU busy | Underruns | Behind seconds |
| --- | --- | --- | --- | --- | --- | --- |
| SIMPLE | 2 / 8 / 9 | 60 / 60 / 60 | 60 / 59 / 59 | 38 / 45 / 46% | 0 / 0 / 0 | 0 / 0 / 0 |
| BALANCED | 2 / 8 / 9 | 60 / 60 / 60 | 60 / 60 / 60 | 53 / 54 / 51% | 0 / 0 / 0 | 1 / 0 / 1 |
| PRETTY | 2 / 8 / 9 | 59 / 60 / 59 | 57 / 59 / 59 | 67 / 69 / 64% | 1 / 1 / 1 | 2 / 1 / 1 |
| ULTRA | 2 / 8 / 9 | 57 / 56 / 59 | 54 / 51 / 54 | 79 / 78 / 74% | 1 / 2 / 0 | 4 / 5 / 2 |

BALANCED is clean here; PRETTY (the user's own 3x Relit) plays at 59-60 with
one short dip and an audio underrun a walk; ULTRA is for faster GPUs.

## 3. AUTO

AUTO is one of the four, picked from the D23W GPU probe (a synthetic 4x frame,
measured once per GPU, driver and build, cached in `config/gpu-probe.json`):
at or under 2.5 ms ULTRA, 5.0 ms PRETTY, 14.0 ms BALANCED, slower SIMPLE
(`gpu_auto.grade_for`). The laptop's probe reads 6.3-6.9 ms: BALANCED. A GPU
about 1.3 times faster gets PRETTY; one about 2.6 times faster ULTRA, where
RESOLUTION AUTO already chose 4x. A failed probe gives BALANCED. Only AUTO
runs the probe; a fixed grade or CUSTOM never does (the menu's AUTO then uses
the last measurement).

## 4. Where it lives

- Profile schema 74: `graphics` (auto, simple, balanced, pretty, ultra,
  custom). Fresh installs start at AUTO; profiles from before schema 74 become
  CUSTOM, every choice kept (the user's 3x Relit stays as it is).
  `player_profiles.GRAPHICS_PRESETS`, `apply_graphics`.
- `run.py --graphics GRADE`. At a launch with a grade the launcher applies its
  rows for that launch (they are what OPTIONS shows and saves back); a flag
  for one of the rows (`--lighting`, `--shadow-edges`, `--draw-distance`,
  `--frame-rate`, `--internal-scale`) makes the profile CUSTOM. The launcher
  prints `Graphics: balanced (AUTO)`.
- OPTIONS > DISPLAY > GRAPHICS (first row). Confirming a grade sets its rows on
  every screen (LIGHTING, LIVE SHADOWS and FRAME RATE at once; RESOLUTION and
  DRAW DISTANCE at the next launch, as before); changing one of those rows by
  hand makes GRAPHICS CUSTOM (`game_menu.cpp`: `graphics_grade`,
  `graphics_custom`; the table is checked against the Python presets by
  `test_menu_table_matches_the_presets`).
- Vanilla is never graded.

Checked in game (real keys, a private profile): VSYNC OFF and a grade
confirmed in OPTIONS > DISPLAY applied live and were saved with the grade's
rows.

## 5. Limits

- One laptop calibrates the limits; the probe is synthetic (fill and shading,
  not draw calls). PRETTY's 5.0 ms limit is inferred from the laptop's
  PRETTY being slightly over budget, not measured on a GPU at 5 ms.
- On a 4+ core machine at a high refresh rate a grade's redraws cost GPU
  time per picture; ULTRA there is the D23K desktop's own settings.
- The 30-second walks are short; a long session's heaviest views (UI slot 9's
  corridor, blasts) can still dip at PRETTY.
