# D08O6 - BREATH RESTORE: air refills at once on surfacing (option)

Renamed 2026-11-20 at the user's request (first build: UNDERWATER BREATHING, key `underwater_breathing`; Duke is not breathing under water). A saved old key is carried over quietly.

Status: **Done, user-accepted and closed out 2026-11-20** ("fix accepted! great work king", "close it out big boy!"). Executable
`77eba92b84da4060fd4b466a645b1031910f57306ac7fa4627a69b82a6e9e04e`
(also carries D17BL, D08A43 and D08A47).

## Request

User, 2026-11-14: a choice between Time to Kill's gradual air recovery and
Duke 3D's instant refill: ADVANCED, BREATH RESTORE, CLASSIC or INSTANT.
Air use under water unchanged in both; consistent on surfacing, quick dips
included; the air box (D20J) follows.

## The original

The player update, at `0x80041fd0`: while `player+0x22c` (the movement mode)
is 5, under water, air `player+0x236` drops by the frame step
(`0x800d21fc`) each frame, and at 0 the drowning damage starts. In every other
mode, air below 12000 rises by 8 frame steps a frame, clamped at 12000 (about
a quarter of a second for each 10% of air at 60). The status bar shows the air
box while the mode is 5 or the air is below 12000 (note 406).

## Change

- `recomp/src/ttk/swim.inc` `breathing_update()`, run each game frame in the
  player update beside the debug spawn (Modernized only): with INSTANT, the
  first frame Duke is not in mode 5 and his air is below 12000, it is set to
  12000. Under water nothing changes. The air box hides by the original's own
  rule.
- Option `breath_restore` (`instant` default, `classic`), profile schema
  80 (older profiles get `instant`), `run.py --breath-restore`,
  `DNTTK_BREATH_RESTORE` (Vanilla always `classic`), OPTIONS > ADVANCED
  row BREATH RESTORE under SWIMMING, live.
- Tests: `tests/local/test_player_profiles.py` `UnderwaterBreathingTest`
  (migration, launcher, environment, Vanilla, validation); the schema
  assertions moved to 80.

## Evidence

Lab `recomp/analysis/d08o6/` (local only; `probe.py` case `air`: UI slot 2
copy (level 6), `dnkroz`, the developer `warp` into the castle moat at
(11680, 10500, 8428), hold Space to swim up). Private cards and profile; the
player's files hash-checked unchanged by every run.

| Setting | Under water | Surfacing frame (mode 5 -> 4) | After |
| --- | --- | --- | --- |
| INSTANT | 10 air a frame step, as before | air 10420 -> 12000 | full; no air box |
| CLASSIC | the same | 10420 -> 10500 | +80 every 2 frames, air box shown (97% two seconds later) |

Pictures two seconds after surfacing: `research/before-after/D08O6/`.

## Limits

- Vanilla was not exercised in game (the lab's warp is Modernized only); the
  launcher test shows Vanilla always gets `classic`, and the rule never runs
  outside Modernized.
- A real dive and surfacing on the player's display is the playtest.
