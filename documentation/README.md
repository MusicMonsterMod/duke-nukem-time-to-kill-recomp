# Documentation index

This is the working engineering notebook for the local **SLUS-00583** recompilation project. It describes this workspace and the owned USA disc. Drop that dump in `game/` (created on first run). Europe/PAL is out of scope.

## Read first

1. [Current status and acceptance boundaries](00-status.md)
2. [Scope, architecture, and milestones](01-architecture-and-roadmap.md)
3. [Disc identity and integrity](02-disc-and-integrity.md)
4. [Executable, assets, and overlays](03-reverse-engineering.md)
5. [Build and run instructions](04-build-and-run.md)
6. [Validation matrix](05-validation.md)
7. [First-person research design](06-first-person.md)
8. [Decisions, risks, and open questions](07-decisions-and-risks.md)
9. [Reproducibility and dependency provenance](08-provenance.md)
10. [Next-session handoff](09-handoff.md)
11. [Tool contracts and maintenance](10-tooling.md)
12. [Complete overlay inventory](11-overlay-inventory.md)
13. [Development log](12-development-log.md)
14. [Movie playback diagnosis and native overlay](13-fmv-fidelity-pass.md)
15. [Gameplay voice-to-music seek correction](14-gameplay-voice-seek.md)
16. [Security-key freeze investigation and window-close fix](15-security-key-and-window-close.md)

17. [Vanilla regression route and recorded baseline](16-vanilla-regression-route.md)

18. [Persistent launch profiles](17-player-profiles.md)

19. [Player and camera research](18-player-camera-research.md)

For players: [game manual and controls](../GAME_MANUAL.md).

For the next development job: [modernization backlog and acceptance criteria](../MODERNIZATION_JOBS.md). Invoke `$continue-duke-recomp` (Codex) or `/continue-duke-recomp` (Claude Code) to review the current list and choose work.

## Machine-readable evidence

- [Disc manifest](reports/disc-manifest.json): every ISO file, logical and raw-extent hashes, submodes, overlay duplicates, and candidate extent-table references.
- [Full sector validation](reports/sector-integrity.json): EDC/ECC and structural totals.
- [Executable analysis](reports/executable-analysis.json): candidate diagnostic-string references, seed audit, startup disassembly.
- [Dependency pins](reports/provenance.json): exact Git identities and local tool versions.
- Build and diagnostic logs are in `logs/`. They are local evidence, excluded from source tracking because they can contain private paths and generated-code details.

- [PC action input](19-pc-action-input.md): D04 bindings, capture, contexts and native test contracts.

## Evidence language

**Observed** means reproduced from this disc or this build. **Candidate** means a plausible static interpretation that still needs loader/control-flow/runtime proof. **Upstream-reported** is a statement in the source project, not reproduced here. **Untested** means no claim of success.

A generated function count does not measure correct gameplay. A native link does not demonstrate boot. A headless boot does not verify graphics or audio. A first level does not verify all overlays or campaign progression.

Update `00-status.md` after meaningful tests. Keep raw evidence and exact commands; do not replace a failure with an unqualified success when only a narrower check passed.

- [D07 weapon aiming, crosshair and coverage](21-modern-weapon-aiming.md)

- [D07A view-facing and weapon presentation](22-view-facing.md)

- [External research register](23-external-research.md): EDuke32 controls and later HRP/menu investigations.

- [D08A shortcuts, held crouch and bounded D10 distance](33-controls-shortcuts.md): verified original paths, current limitations and automated evidence.

- [Accepted standing directional jumps, both input orders (D08C Done)](34-standing-directional-jumps.md)

- [D08G1/D07C/D08H/D19A feedback implementation and verification](40-feedback-implementation.md)

- [Latest playtest acceptance and next-job plan](41-playtest-follow-up-plan.md): silent/centered cheat results, run/edge jumps, armed ladders, crouch walking, inventory feedback and deferred menu responsiveness.

- [D08P crystal-2 turret / scripted-camera WASD recovery](56-scripted-camera-controls.md)

- [D08T pushable objects: modern grab, push/pull and climb](64-d08t-pushable-objects.md)
- [D12A first-person quick kick](65-d12a-first-person-kick.md)
