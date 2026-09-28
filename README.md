# Duke Nukem: Time to Kill - native PC recompilation

[![CI](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/workflows/ci.yml/badge.svg)](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/workflows/ci.yml)

Faithful Vanilla play plus an opt-in Modernized mode (WASD, mouse aim, and quality-of-life). **No retail game data is included.** You need your own US SLUS-00583 disc.

- [Game manual](GAME_MANUAL.md) - controls and player-facing behavior
- [Documentation index](documentation/README.md) - engineering notes and evidence
- [Current status](documentation/00-status.md)
- [Modernization jobs](MODERNIZATION_JOBS.md) - the canonical backlog

This GitHub tree holds the notes, job board, player manual, and validation reports. The nested `recomp/` checkout, original CloneCD dump, `research/` references, and local `.venv/` stay on the development machine and are gitignored.

Invoke `$continue-duke-recomp` to list jobs from `MODERNIZATION_JOBS.md` and choose one.
