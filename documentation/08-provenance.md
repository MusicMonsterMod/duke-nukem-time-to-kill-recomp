# Provenance and reproducibility

## Source pins

Base repository: https://github.com/alexbeavs-ps1-ports/duke-nukem-time-to-kill-recomp

Base commit: `b2d21e3231d19958b7b04af065ce5397265b8559`

Local branch: `local/native-pc-bringup`

| Dependency | Commit |
|---|---|
| PSXRecomp | `08ec704a974b1f3a16335b4afeb340b9eff19926` |
| recomp-ui | `be8ac1d03ee19d55394b5a5f2d9d1506edd56659` |
| recomp-net | `268e74fe718b38fe38643c358588bbc1e0f0af70` |
| retcomm-rbengine | `ebd94a4729abe2c0615070cef3ffe05b3f9ebf28` |

The nested submodule Git commits are authoritative. Branch names in `.gitmodules` do not authorize floating upgrades. The current upstream repository may have newer architecture documentation than this pinned tree. Inspect source before using a current-web command or configuration key.

The clone reported an existing line-ending normalization difference in `.github/workflows/setup-name.yml` immediately after checkout. It is not a game behavior change. Preserve that fact when reviewing diffs rather than attributing it to a local gameplay edit.

## Host tools

Initial host: Linux, GCC 13.3, CMake 3.28.3, Python 3.12.3. The workspace virtual environment pins Ninja 1.13.2 and optional Capstone 5.0.9. No system packages were changed. Exact machine metadata and recursive submodule output are in `reports/provenance.json`.

SDL and other upstream dependencies were resolved through the pinned CMake configuration. Build logs record download/configure choices. Generated game C and OpenBIOS C are local build products. Keep source and generated artifacts distinct.

## Upstream claims consulted

- Main repository and `disc_probe.json`: executable/disc identity and initial seed count.
- `docs/FEASIBILITY.md`: bootstrap gameplay reported upstream; remaining package gates.
- Pinned `psxrecomp/docs/BUILDING.md`: toolchain and OpenBIOS generation.
- Pinned `psxrecomp/runtime/src/debug_server.c`: actual debug commands and request schema.
- Pinned `psxrecomp/psxrecomp_cli.py`: generation, hashing, and BIOS selection behavior.

External research links:

- https://github.com/RetroPortingToolKit/psxrecomp
- https://github.com/RetroPortingToolKit/psxrecomp/blob/master/docs/AOT_SHARDING.md
- https://github.com/PS1Recomp/ps1-recomp
- https://forums.duke4.net/topic/12150-duke-nukem-time-to-kill-and-land-of-the-babes-modding-thread/

These are references, not substitutes for local verification. No external ROM or retail BIOS was downloaded.

## Source tracking and sharing

Original media and generated retail translations remain excluded from the game repository. The documentation reports contain identities, counts, references, and a short startup disassembly; framebuffer captures and raw runtime diagnostics remain local under ignored analysis/log directories.

The inherited source project and dependencies retain their existing licenses; see their LICENSE files before redistributing changes. This work creates no release, remote fork, pull request, or publication. It does not alter the original disc.

For backups, retain both `documentation/` and the `recomp/` source changes, plus the exact original input privately. Do not rely on logs alone: reproduce them using the documented commands.
