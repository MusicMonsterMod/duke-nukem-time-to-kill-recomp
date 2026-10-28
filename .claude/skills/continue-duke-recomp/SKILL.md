---
name: continue-duke-recomp
description: Resume the Duke Nukem Time to Kill PC recomp project by listing its modernization jobs for selection or implementing a specifically selected job, with Modernized as the default profile and Vanilla preserved. Use when the user asks to continue the Duke recomp, see the job board, or work on a job ID such as D08L.
argument-hint: "[job ID or task, e.g. \"work on D08L\"]"
---

# Continue Duke recomp

Claude Code variant of the Codex `$continue-duke-recomp` skill. Invoke as `/continue-duke-recomp` to list jobs, or `/continue-duke-recomp work on D08L` to select one.

Selection passed with this invocation: `$ARGUMENTS`

## Locate the project

Use the current workspace when it contains `MODERNIZATION_JOBS.md` and the Time to Kill `recomp/` checkout. Otherwise check `~/CODE/duke-nukem-time-to-kill-recomp` and the legacy `~/Desktop/dn-ttk`, expanding `~` locally only. If neither is available, ask for the project location; do not create a replacement project or obtain another disc.

Follow `AGENTS.md` in the workspace root. It overrides anything here that conflicts.

## Show the jobs, then let the user choose

Read `MODERNIZATION_JOBS.md` as the canonical backlog, plus `documentation/00-status.md` and `documentation/09-handoff.md` for current evidence. Do not duplicate job descriptions or statuses in this skill. The board is large; read the job table first and only the work-log sections you need.

When the selection above is empty:

- Present the current job IDs, short names and statuses, grouped by domain. Include all unfinished jobs; summarize completed and cancelled jobs separately.
- Layout (user preference, 2026-10-20): one `###` heading per domain, then one bullet per job, `- **ID** - short name`, with any non-Todo status or blocking dependency in brackets after the name, e.g. `(**Blocked**)` or `(waits on D21)`. No tables and no several jobs crammed onto one line. After the unfinished jobs, a short `## Finished` section (Done count with recent IDs, Accepted-not-closed count, Cancelled IDs), then `## Suggested starting points` with two or three short bullets, then the question.
- Identify ready jobs from their dependencies and suggest a useful starting point. Explain any existing In progress job or blocker.
- Ask which job the user wants to work on and **wait for their choice**. End the turn there. Listing jobs does not authorize starting one. Do not edit files, build, launch the game or mark a job In progress just to show the list.

When the selection names an ID or an unambiguous task, use it without asking again. If the requested task is absent from the board, scope it and add a stable job entry. If a dependency materially prevents the selected work, explain the dependency and ask the user to select that prerequisite; do useful investigation within the selected scope meanwhile. Do not silently switch jobs.

## Work on the selected job

Read the job's acceptance criteria and relevant linked notes, then inspect Git state in both repositories:

- The workspace root tracks implementation under `recomp/` as well as documentation.
- Inspect root Git state and pinned framework/UI submodules. Preserve local changes. Export framework edits into the tracked complete patch; a submodule pointer alone does not preserve local edits.

Mark the selected job In progress and implement a bounded, reviewable result. Use the existing local build/import/patch workflow described in `documentation/04-build-and-run.md` and `documentation/10-tooling.md`. Never hand-edit generated recompilation C, overwrite original media or use the player's memory cards for tests. Check for a running game process before building over or launching anything; if the player's game is open, close it (debug `quit` on port 9123, or end the process) and continue, under the user's standing permission (2026-10-01). Never touch their saves or memory cards.

Keep this game-specific. Modernized is the default profile; Vanilla must remain available (`run.py --vanilla`) and unchanged in play. Modern controls mean independent movement, camera and weapon aiming, not merely keyboard remapping. Third-person comes before the first-person foundation is considered complete. Verify addresses and active overlays before applying game hooks. Existing renderer/framework capabilities are not proof that an option works in the player build.

Test according to the selected acceptance criteria and regression risk. Distinguish build/test results, observed gameplay and user reports. The earlier security-card freeze did not reproduce on the user's successful retest; do not claim its exact cause was established. A linked executable or first-level pass does not establish full campaign fidelity.

Update the board and append a work-log entry with implementation, evidence and remaining limitations. Use Needs playtest when required gameplay acceptance remains unverified, Blocked with a concrete reason when appropriate, and Done only when the job's actual criteria are met. Update status/handoff and focused engineering notes as needed; update `GAME_MANUAL.md` when player-facing behavior or controls change.

Write with ASCII hyphen-minus `-` only; never U+2014 or U+2013.

## Git

Commit or push only when the user asks. When they do, commit the public repo as MusicMonsterMod only. Never commit retail media, dumps, BIOS, memory cards, `research/`, generated game code, build output, extracted retail assets, or anything under `game/` except `game/README.md`. `scripts/ci/check_repo.py` enforces this; run it before committing.

## Finish

Finish with what changed, what was verified and what remains. Do not automatically start another job. The user chooses the next job on a future invocation or explicitly authorizes continued work.

After working on a selected job, always include a copy-pasteable command to launch the current player build, including when the job is Needs playtest:

```
python3 recomp/tools/local/run.py
```

Run this from the workspace root. It uses the player's saved settings. If testing the job requires different mode or control options, include the verified launcher flags and explain that they update saved preferences. Mouse capture is automatic during gameplay and after resuming or loading a save. F10 has no action; do not include an F10 activation or capture reminder. Use the current GAME_MANUAL.md for control instructions rather than historical handoff notes. Providing the command does not authorize launching the game; the user can run it themselves with `! <command>` or in their own terminal.

When you have finished working on a selected job (including Needs playtest or Blocked), play a chime as the very last step, just before the final summary, so the user knows you are done while looking away (user request, 2026-10-07):

```
paplay /usr/share/sounds/freedesktop/stereo/complete.oga || canberra-gtk-play -i complete
```

Play it once per finished job, not for job listings or intermediate steps. A failed chime never blocks the summary.

## Private paths

Never write the local account name or an expanded personal home directory into
tracked documentation, Markdown, reports, logs, asset metadata or examples.
Use repository-relative paths for workspace files, `~/` for home-relative
references, and `$HOME` in shell commands. Redact copied diagnostic output before
saving it. For other machines use `<redacted>` as the username placeholder.
Run `python3 scripts/ci/check_repo.py` before an authorized commit; fix privacy
failures without recording the offending username in the fix or its notes.
Give launch commands from the workspace root: `python3 recomp/tools/local/run.py`.
