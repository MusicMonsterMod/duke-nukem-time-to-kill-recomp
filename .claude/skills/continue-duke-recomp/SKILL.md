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

Read `MODERNIZATION_JOBS.md` as the canonical backlog, plus `documentation/00-status.md` and `documentation/09-handoff.md` for current evidence. Do not duplicate job descriptions or statuses in this skill. The job table in `MODERNIZATION_JOBS.md` holds every ID and status; each ID links to its description and work log in `jobs/<category>.md`. The dated session log is in `jobs/log-YYYY-MM.md`, newest first. List jobs from the table; read only the category sections you need.

When the selection above is empty:

- Present the current job IDs, short names and statuses, grouped by domain. Include all unfinished jobs; summarize completed and cancelled jobs separately.
- Layout (user preference, 2026-10-20): one `###` heading per domain, then one bullet per job, `- **ID** - short name`, with any non-Todo status or blocking dependency in brackets after the name, e.g. `(**Blocked**)` or `(waits on D21)`. No tables and no several jobs crammed onto one line. After the unfinished jobs, a short `## Finished` section (Done count with recent IDs, Accepted-not-closed count, Cancelled IDs), then `## Suggested starting points` with two or three short bullets, then the question.
- Jobs marked **Monitoring** wait on a user report (for example D07H, the red dot). List them with `(**Monitoring** - waits for your report)`, never suggest them as starting points, and do not investigate, build or test them until the user reports a recurrence or selects them with new evidence.
- Identify ready jobs from their dependencies and suggest a useful starting point. Explain any existing In progress job or blocker.
- Ask which job the user wants to work on and **wait for their choice**. End the turn there. Listing jobs does not authorize starting one. Do not edit files, build, launch the game or mark a job In progress just to show the list.

When the selection names an ID or an unambiguous task, use it without asking again. If the requested task is absent from the board, scope it and add a stable job entry. If a dependency materially prevents the selected work, explain the dependency and ask the user to select that prerequisite; do useful investigation within the selected scope meanwhile. Do not silently switch jobs.

## Work on the selected job

Read the job's acceptance criteria and relevant linked notes, then inspect Git state in both repositories:

- The workspace root tracks implementation under `recomp/` as well as documentation.
- Inspect root Git state and pinned framework/UI submodules. Preserve local changes. Export framework edits into the tracked complete patch; a submodule pointer alone does not preserve local edits.

Mark the selected job In progress and implement a bounded, reviewable result. Use the existing local build/import/patch workflow described in `documentation/04-build-and-run.md` and `documentation/10-tooling.md`. Never hand-edit generated recompilation C, overwrite original media or use the player's memory cards for tests. Check for a running game process before building over or launching anything; if the player's game is open, close it (debug `quit` on port 9123, or end the process) and continue, under the user's standing permission (2026-10-01). Never touch their saves or memory cards.

Keep this game-specific. Modernized is the default profile; Vanilla must remain available (`run.py --vanilla`) and unchanged in play. Modern controls mean independent movement, camera and weapon aiming, not merely keyboard remapping. Third-person comes before the first-person foundation is considered complete. Verify addresses and active overlays before applying game hooks. Existing renderer/framework capabilities are not proof that an option works in the player build.

To test, set up the scenario directly instead of hunting for it in saves (user, 2026-11-03: "to test stuff you can just spawn anything you need"). Spawn what the test needs in front of Duke on a private run: the backtick console `spawn <item>` (D26E, note 111: pickups, weapons, ammo, keys), the debug cheats, `DNTTK_TEST_INPUT` words (`flash=`, `hurt=type,amount` for a hit through Duke's own damage handler) and the debug port. If what you need cannot be spawned yet (enemies, as of 2026-11-03), say so and propose adding it rather than spending long on slot surveys.

Test according to the selected acceptance criteria and regression risk. Distinguish build/test results, observed gameplay and user reports. The earlier security-card freeze did not reproduce on the user's successful retest; do not claim its exact cause was established. A linked executable or first-level pass does not establish full campaign fidelity.

Update the job's table row and append a work-log entry to its section in `jobs/<category>.md` with implementation, evidence and remaining limitations; add a dated entry at the top of the current month's `jobs/log-YYYY-MM.md`; run `python3 scripts/ci/check_jobs.py`. Use Needs playtest when required gameplay acceptance remains unverified, Monitoring when a job stays open only to catch a recurrence, Blocked with a concrete reason when appropriate, and Done only when the job's actual criteria are met. Update status/handoff and focused engineering notes as needed; update `GAME_MANUAL.md` when player-facing behavior or controls change.

Write with ASCII hyphen-minus `-` only; never U+2014 or U+2013.

## GitHub status

Keep the repository's CI green (user request, 2026-11-12, after it had failed
unnoticed on every push for four days):

- At the start of every invocation (job listing or selected work), check the
  latest CI runs with the MusicMonsterMod account:
  `gh run list --repo MusicMonsterMod/duke-nukem-time-to-kill-recomp --limit 5`.
  If `gh`'s active account is another one, prefix the command with
  `GH_TOKEN="$(gh auth token --user MusicMonsterMod)"` (never print the token).
  Report the result in one line, with the job listing or at the start of the work.
- If the latest run on `main` failed, read the failing job's log (`gh run view
  <run id> --log-failed`, or `gh api repos/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/jobs/<job id>/logs`),
  find the cause and reproduce it locally. Fix it in the same session even when
  it is unrelated to the selected job, and report it. A fix still waits for the
  user's commit and push instruction, like any other change.
- Before ending a session that changed code or tests, run the local
  equivalents of CI: `python3 -m unittest discover -s recomp/tests/local`,
  `python3 scripts/ci/check_jobs.py` and `python3 scripts/ci/check_repo.py`.
  Tests skipped in CI (they need the owned EXE) still run locally; report their
  failures separately.
- After every authorized push, wait for that push's CI run (`gh run watch
  <run id> --exit-status`) and confirm it passed. If it fails, fix it, report
  it and ask before pushing the fix.

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

## Close what you opened

Close every program you started once you are finished with it (user request, 2026-11-08): reference engines and ports (EDuke32, UZDoom, Ironwail and so on), Photon Studio, offscreen test games, Xvfb displays, ffmpeg captures, background watch loops, browsers or viewers opened for the work, and anything else. Leftover programs compete with the player's game for the CPU and GPU. Track the process IDs you start (pid files, not broad `pkill -f`), stop them when the step that needed them ends, and before the final summary check that nothing you launched is still running (`pgrep -a Xvfb`, the game, the reference ports). Never close programs the user started themselves unless they ask.

When the user asks to close a job out, that is also the signal to tidy up and pack up shop: stop every program and background task from the work, remove scratch captures and temporary files that are not wanted evidence, and leave the machine as it was before the job started.

## After a closeout

When the user accepts a job and authorizes the closeout (documentation, commit and push), tidy up first (see Close what you opened), then end the closeout reply with up to three suggested next jobs and a ready-to-run command for each (user request, 2026-11-01). Pick them from the current board: unfinished jobs whose dependencies are met, preferring follow-ups the closed job uncovered and related open work. Give each a one-line reason, then the command, for example:

```
/continue-duke-recomp work on D36H6
```

Suggesting jobs does not start one. Do not mark any job In progress; wait for the user to run a command or choose.

## Private paths

Never write the local account name or an expanded personal home directory into
tracked documentation, Markdown, reports, logs, asset metadata or examples.
Use repository-relative paths for workspace files, `~/` for home-relative
references, and `$HOME` in shell commands. Redact copied diagnostic output before
saving it. For other machines use `<redacted>` as the username placeholder.
Run `python3 scripts/ci/check_repo.py` before an authorized commit; fix privacy
failures without recording the offending username in the fix or its notes.
Give launch commands from the workspace root: `python3 recomp/tools/local/run.py`.
