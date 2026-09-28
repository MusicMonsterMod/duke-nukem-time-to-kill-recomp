# Decisions and risks

## ADR-001 — extend the existing project

Accepted by user. Reuse its pinned build/runtime setup and matching executable seed list. Keep independent evidence rather than inheriting its gameplay claims. This saves bootstrap work without assuming full compatibility.

## ADR-002 — preserve the original CloneCD dump

Original files remain untouched. Prepared runtime media is a byte-identical copy. Hashes, descriptor structure, and full sector checks gate import. Original subchannels remain available even though the initial CUE runtime does not consume them.

## ADR-003 — separate local identity profile

`game.local.toml` describes this validated image. Upstream `game.toml` remains a reference. Matching executable hashes permit reuse of resident-code analysis; different disc hashes prevent declaring all media equivalent. The local profile carries no unverified upstream netplay identity.

## ADR-004 — pinned OpenBIOS first

The pinned source includes OpenBIOS, so use it for initial generation and boot. No retail BIOS substitution is needed for the first attempt. Future retail comparisons require an appropriate user-supplied input and separate evidence.

## ADR-005 — reproducible native developer build before enhancements

Original behavior, 4:3, no new netplay, no FPS patch, no overclock. Debug-enabled native executable with the launcher disabled makes startup and diagnostics direct. Final product UI is deferred.

## ADR-006 — source and research live separately

Implementation lives in the nested upstream Git checkout. Detailed notes live in `documentation/` at workspace root. The workspace `.gitignore` excludes the nested checkout, original media, local environment, and raw logs if the workspace is later put under Git. It does not create a second Git repository automatically.

## Risk register

| Risk | Evidence | Mitigation / next evidence |
|---|---|---|
| Different raw image from upstream | Whole-image hashes disagree; executable identical | Register exact local identity; parity validated; do not claim asset equivalence |
| Streamed executable overlays | 50 files / 30 unique contents; MIPS calls/returns | Recover loading and coexistence rules; observe real memory writes |
| Data misidentified as code | Generator reports 426 suppressed reserved-opcode sites | Audit mixed code/data and actual callers; keep exception behavior |
| Incomplete seed boundaries | JAL-derived seeds; out-of-function warnings | Ghidra CFG work, indirect-target recovery, runtime comparison |
| Movie/audio data loss in conversion | XA Form 2 and interleaved channels observed | Retain raw sectors and subheaders |
| BIOS-specific behavior | OpenBIOS first; retail not tested | Record backend; controlled comparison if a concrete failure appears |
| Linux success generalized to Windows | No Windows runtime available here | Windows build/run matrix; explicit untested status |
| First-person room visibility | Camera-start-room diagnostic; third-person design | Recover culling/room semantics before FPS patch |
| Untracked knowledge | Separate requested docs directory | Index and handoff; exact reproducible commands and reports |

## Questions not yet answered

Where do all overlay families load? Is the four-byte prefix copied or skipped? Are base overlays embedded in resident code or loaded via another table? What dispatch paths use runtime interpretation? Does OpenBIOS behave correctly through every level? What exactly differs between local and upstream raw images? Which asset groups represent campaign, bonus, and multiplayer contexts?

Do not convert an unanswered question into a configuration constant. Prefer a bounded experiment and record its result.
