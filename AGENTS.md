# Agent instructions

Duke Nukem: Time to Kill PC recompilation (owned US SLUS-00583).
GitHub: https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp

## What this tree is

This GitHub repo holds `GAME_MANUAL.md`, `MODERNIZATION_JOBS.md`, `documentation/`, and agent rules (including the Claude Code skill in `.claude/skills/`).
The nested `recomp/` checkout, player dumps under `game/`, `research/`, and `.venv/` are local only. `game/README.md` is tracked.

## Git and GitHub

- Use only the MusicMonsterMod account on this repository.
- Commit and push as MusicMonsterMod. Do not attribute work to any other GitHub user.
- Never commit retail media, GRP/ISO/BIN dumps, BIOS, memory cards, `research/`, `recomp/`, or anything under `game/` except `game/README.md`.

## Game work

- Vanilla stays available. Modernized is an optional profile.
- Modern controls mean independent movement, camera, and weapon aiming, not keyboard remapping alone.
- Job board: `MODERNIZATION_JOBS.md`. Status evidence: `documentation/00-status.md` and `documentation/09-handoff.md`.
- `$continue-duke-recomp` (Codex) or `/continue-duke-recomp` (Claude Code) lists jobs and waits for a choice. Listing does not start a job.
- Do not mark a job In progress, edit gameplay code, build, or launch the game just to show the list.
- Never hand-edit generated recompilation C, overwrite original media, or use the player's memory cards.
- Do not interfere with an active game session.
- After a selected job, give a launch command such as `python3 recomp/tools/local/run.py` from this workspace. Do not launch the game unless asked.

## Writing

Use ASCII hyphen-minus `-` only. Do not use Unicode em dashes (U+2014) or en dashes (U+2013).
