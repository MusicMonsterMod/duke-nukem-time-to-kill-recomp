# D33 - Modernized by default, Vanilla behind `--vanilla`

2026-10-20. **Done (user-accepted 2026-10-20);** the executable below is the regression baseline. User request: "modernized btw, should be the
default profile of the game. --vanilla i think should be the special flag".
Executable `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2`.

## Decisions (user, 2026-10-20)

| Question | Decision |
| --- | --- |
| Plain `run.py` | Always Modernized |
| Vanilla | `run.py --vanilla`, this launch only, never saved |
| `--mode vanilla\|modernized` | Removed (no `--modernized` either) |
| OPTIONS > MODERNIZED > PROFILE | Removed (it was a one-way trip: Vanilla's original menus had no way back) |
| A saved `active: vanilla` | Moved to Modernized once, with a notice; both profiles' settings kept |
| Version 0 file without `mode` | Loads as Modernized (was Vanilla) |

## Behaviour

- `player_profiles.save()` always writes `active: modernized`; the field stays
  in the file (schema 33 unchanged) so older launchers still read it.
- `load()` keeps Modernized for a stored `vanilla`, prints
  `Modernized is now the default profile; your Vanilla settings are kept.
  Launch with run.py --vanilla to play the original game.` and rewrites the
  file (the usual `.recovered-*` backup holds the old bytes). Only that notice
  appears; the "invalid fields" notice is not added for it.
- `run.py --vanilla` sets the active profile in memory. Presentation options
  (`--renderer`, `--display`...) and `--settings` / `--show-settings` /
  `--show-bindings` then apply to Vanilla, saved in its profile; Modernized
  control flags still save to Modernized. `--bind` with `--vanilla` is refused.
  The exit-time display state is saved to the profile that ran.
- `describe()`: `Profile: Modernized (run.py --vanilla plays the original game
  for one launch)` or `Profile: Vanilla (this launch only)`.
- The terminal `--settings` menu lost choices 1 Vanilla / 2 Modernized; it
  edits the launched profile.
- In-game: the PROFILE row is gone from OPTIONS > MODERNIZED (CAMERA is
  first). The side file keeps version 1 with `active` fixed to `modernized`;
  `absorb_menu_settings()` ignores `active`, so an older build's PROFILE
  VANILLA cannot switch the next launch.
- Probes: `pc_input_probe.py` and `vanilla_regression.py` keep their own
  `--mode` argument and pass `--vanilla` to `run.py`.

## Evidence

Offscreen on the real GPU, private profile and cards
(`recomp/analysis/d33-default-profile`, cards and savestates copied from
`d19-menu`; slot 5 gameplay).

| Check | Result |
| --- | --- |
| Pause > OPTIONS > MODERNIZED (`t.py`) | rows start at CAMERA; no PROFILE |
| Change VIEW, CONFIRM, quit | `Saved the in-game OPTIONS settings.`; file `active` modernized, view saved |
| `--vanilla` launch (`v.py`) | `Profile: Vanilla (this launch only)`, original 4:3 picture and HUD; file stays `active: modernized` |
| Python unittest | 152 OK (2 skipped); new: one-launch Vanilla, saved Vanilla moved once, version 0 without mode, old side file with `active: vanilla` ignored |
| Native | `ttk-controls-test` (LEVEL00 fixture), `ttk-aim-test`, `ttk-input-test`, `ttk-near-test`, `ttk-live-env-test` pass |
| `scripts/ci/check_repo.py` | OK |

Accepted by the user 2026-10-20. The player's settings file already
held `active: modernized`, so no migration notice is expected there.

## Docs and rules changed

`AGENTS.md`, both `continue-duke-recomp` skills (Claude Code and Codex),
`.cursor/rules/dnttk.mdc`, `README.md`, `GAME_MANUAL.md` (profiles section,
examples, in-game OPTIONS), notes 04, 10, 17, 19 and 145.
