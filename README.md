# Duke Nukem: Time to Kill - native PC recompilation

[![CI](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/workflows/ci.yml/badge.svg)](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/actions/workflows/ci.yml)

Faithful Vanilla play plus an opt-in Modernized mode (WASD, mouse aim, and quality-of-life). **No retail game data is included.** You need your own USA SLUS-00583 disc. Europe/PAL (SLES-01515) is a different SKU and will not work.

- [Game manual](GAME_MANUAL.md) - controls and player-facing behavior
- [Documentation index](documentation/README.md) - engineering notes and evidence
- [Current status](documentation/00-status.md)
- [Modernization jobs](MODERNIZATION_JOBS.md) - the canonical backlog

The implementation is tracked under **[recomp/](recomp/)**: title source,
controls, renderer integration, runtime patches, build tools and tests. Pinned
upstream framework/UI sources are Git submodules. Our framework modifications
are included in the complete tracked patch, applied automatically by the build.
No ROM, generated game code, compiled game, player save or extracted retail art
is distributed.

## Build and play (Linux)

```bash
git clone --recurse-submodules https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp.git
cd duke-nukem-time-to-kill-recomp
python3 -m venv .venv
.venv/bin/pip install -r recomp/requirements-local.txt
```

Install the compiler/CMake and graphics development prerequisites described in
[build instructions](documentation/04-build-and-run.md). Put your owned USA
SLUS-00583 dump in `game/` (a subfolder is fine): `.cue`/`.bin`, CloneCD
`.img`/`.ccd`/`.sub`, or a raw MODE2/2352 image. **Everything in game/ except
README.md is gitignored.** Then:

```bash
.venv/bin/python recomp/tools/local/build.py --jobs 4
python3 recomp/tools/local/run.py
```

To select the accepted Modernized quality target, use
`python3 recomp/tools/local/run.py --mode modernized --frame-rate 120`
(these options save your preferences). Vanilla remains available.

For an existing clone, run `git submodule update --init --recursive` first.
Builds generate the game code locally from your disc. Research, generated code,
build outputs, personal settings, captures and saves remain ignored. Optional
locally extracted fonts/inventory art are not distributed; see the build notes.
Linux is the validated platform; Windows execution remains unverified.

**D17A and D17B are accepted.** 120 FPS is the primary quality/regression target;
180 FPS+ remains excellent high-refresh support, 240 FPS+ robustness/compatibility,
and Unlimited stress/debug. There is no 120 FPS ceiling. See the
[accepted baseline and remaining focused bugs](documentation/86-d17-acceptance-and-regression-baseline.md).

Third-party licenses remain applicable: the title scaffold has its license in
`recomp/LICENSE`; framework/UI dependencies retain their own licenses. The
root MIT notice does not relicense those dependencies.

Invoke `$continue-duke-recomp` (Codex) or `/continue-duke-recomp` (Claude Code, from `.claude/skills/`) to list jobs from `MODERNIZATION_JOBS.md` and choose one.
