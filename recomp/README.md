# Time to Kill implementation

This directory is tracked source, not an external unpublished game project.
Use the [root README](../README.md) and [build guide](../documentation/04-build-and-run.md)
for the supported owned-disc build. The game ROM and generated recompilation C
are intentionally absent from Git.

- `src/ttk/`: title-specific controls, camera, renderer, audio/movie and UI hooks.
- `tools/local/`, `tests/local/`: import/build/launch tools and regressions.
- `patches/time-to-kill-accepted-source.patch`: complete authored framework delta.
- `psxrecomp/`, `recomp-ui/`: pinned upstream source submodules.
- `game.local.toml`, CMake files and preloaded mods: game/build integration.

The upstream scaffold came from alexbeavs-ps1-ports/duke-nukem-time-to-kill-recomp
at `b2d21e3231d19958b7b04af065ce5397265b8559`; its license is retained here.
Framework pins are recorded in `framework_pins.txt`. Historical incremental
patches are archived under `patches/history/`; only the complete top-level
patch is applied. Original upstream Git metadata is preserved locally outside
the tracked source, not needed by another clone.
