# Agent instructions

Duke Nukem: Time to Kill PC recompilation (owned US SLUS-00583).
GitHub: https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp

## What this tree is

This GitHub repo holds the implementation under `recomp/`, plus the manual, job board, documentation and agent rules. Title source, patches, tools, tests and build definitions are tracked. `recomp/psxrecomp` and `recomp/recomp-ui` are pinned upstream submodules; our framework changes are tracked in `recomp/patches/time-to-kill-accepted-source.patch`.
Original media under `game/`, research, generated game code, builds, captures, player saves/cards and extracted retail assets stay local. Only `game/README.md` is tracked under `game/`.

## Git and GitHub

- Use only the MusicMonsterMod account on this repository.
- Implement and test selected work, then present it for user approval. Acceptance alone does not authorize the documentation/commit/push closeout: wait for the user to explicitly instruct it. Do not automatically commit or push.
- When authorized, commit authored implementation changes as well as documentation. Never restore a blanket ignore of `recomp/`. After framework edits, run `python3 recomp/tools/local/export_runtime_patch.py` and verify the patch against the pinned clean dependency.
- When authorized, commit and push as MusicMonsterMod. Do not attribute work to any other GitHub user.
- Never commit retail media, GRP/ISO/BIN dumps, BIOS, memory cards, `research/`, generated game code, build output, extracted retail assets, or anything under `game/` except `game/README.md`.

## Game work

- Vanilla stays available. Modernized is an optional profile.
- Modern controls mean independent movement, camera, and weapon aiming, not keyboard remapping alone.
- Job board: `MODERNIZATION_JOBS.md`. Status evidence: `documentation/00-status.md` and `documentation/09-handoff.md`.
- `$continue-duke-recomp` (Codex) or `/continue-duke-recomp` (Claude Code) lists jobs and waits for a choice. Listing does not start a job.
- Do not mark a job In progress, edit gameplay code, build, or launch the game just to show the list.
- Never hand-edit generated recompilation C, overwrite original media, or use the player's memory cards.
- If the player's game is open when work needs to build, test or launch, close it (debug `quit` on port 9123, or end the process) and continue. The user gave standing permission on 2026-10-01. Never touch their saves or memory cards.
- During selected coding work, agents may launch the game as needed for testing without asking again. Prefer background/offscreen diagnostic runs with private settings and test save/card copies. Use a foreground window only when visual verification genuinely requires it. The user gave standing permission on 2026-10-04.
- After a selected job, give a launch command such as `python3 recomp/tools/local/run.py` from this workspace.

## Writing

Use ASCII hyphen-minus `-` only. Do not use Unicode em dashes (U+2014) or en dashes (U+2013).
