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

- Modernized is the default profile. Vanilla stays available, unchanged in play, with `run.py --vanilla` (one launch, not saved).
- Modern controls mean independent movement, camera, and weapon aiming, not keyboard remapping alone.
- Job board: `MODERNIZATION_JOBS.md`. Status evidence: `documentation/00-status.md` and `documentation/09-handoff.md`.
- For new custom meshes or mesh/material art revisions, read the accepted style guide `documentation/265-native-mesh-art-style.md` first. Use D08A28 revision 5 as the reference for angular geometry, effective texel density, UV mapping and painted materials; validate in game, not only Blender.
- For first-person body/arms, weapon/item presentation, interactions, clipping or quick-kick visual work, read the accepted D36 audit `documentation/257-d36-first-person-presentation-audit.md` first. Preserve its evidence labels, ownership boundaries and regression requirements. D36G-L are report-only implementation proposals until explicitly selected.
- For future first-person arm reports, start with the accepted D36H5 design in `documentation/305-d36h5-contact-arm-rig.md` and the repair guide in `documentation/306-d36h5-interaction-arm-coverage.md`. Identify the actual presentation owner and current slot contents; preserve failing private saves. D36H6 retains unverified broader coverage.
- When the user reports performance problems, promptly check the computer's state as well as gameplay logs: Timeshift/rsync and other backups, updates, CPU load, disk I/O pressure, memory/swap and relevant system events. Correlate historical activity with the reported session; a later idle machine does not establish conditions during the slowdown. Do not assume a game regression or change code before investigating host contention. Distinguish confirmed slowdowns, suspected causes and reproducibility; background activity alone does not prove causation. User instruction, 2026-10-24; see the handoff's slot 12 mantle report.
- `$continue-duke-recomp` (Codex) or `/continue-duke-recomp` (Claude Code) lists jobs and waits for a choice. Listing does not start a job.
- Do not mark a job In progress, edit gameplay code, build, or launch the game just to show the list.
- Never hand-edit generated recompilation C, overwrite original media, or use the player's memory cards.
- If the player's game is open when work needs to build, test or launch, close it (debug `quit` on port 9123, or end the process) and continue. The user gave standing permission on 2026-10-01. Never touch their saves or memory cards.
- During selected coding work, agents may launch the game as needed for testing without asking again. Prefer background/offscreen diagnostic runs with private settings and test save/card copies. Use a foreground window only when visual verification genuinely requires it. The user gave standing permission on 2026-10-04.
- After a selected job, give a launch command such as `python3 recomp/tools/local/run.py` from this workspace.

## Writing

Use ASCII hyphen-minus `-` only. Do not use Unicode em dashes (U+2014) or en dashes (U+2013).

## Private paths

Never write the local account name or an expanded personal home directory into
tracked documentation, Markdown, reports, logs, asset metadata or examples.
Use repository-relative paths for workspace files, `~/` for home-relative
references, and `$HOME` in shell commands. Redact copied diagnostic output before
saving it. For other machines use `<redacted>` as the username placeholder.
Run `python3 scripts/ci/check_repo.py` before an authorized commit; fix privacy
failures without recording the offending username in the fix or its notes.
Give launch commands from the workspace root: `python3 recomp/tools/local/run.py`.
