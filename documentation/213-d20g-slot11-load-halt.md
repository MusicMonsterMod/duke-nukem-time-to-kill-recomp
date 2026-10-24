# D20G - Slot 11 reload fatal halt investigation intake

Status: Todo. Backlog only, 2026-10-24. Explicitly deferred to fresh context.
No new investigation or fix in the D08T7 acceptance closeout.

## Confirmed incident

At 08:46:44 BST, session `session-20261024-083523.log` restored UI slot 11
(internal/file index 10), then halted on GPU GP0 opcode 0x1F, word 0x1F20161D.
Restore applied successfully in 20.2 ms, total 22.9 ms. Debug frame 40732;
savestate generation 4, last load successful. GPU DMA channel 2 was active.
Same slot had loaded successfully earlier in this session. This establishes a
post-restore fatal emulation halt, not a corrupt-save diagnosis or host crash.

The app exited after post-mortem capture. Debug quit initially said busy/frozen;
by the subsequent TERM attempt all recorded game PIDs were already absent.
No code was changed in immediate inspection. User separately accepted D08T7's
mechanic and requested this investigation next, before returning to push/pull.

## Local evidence and comparisons

Ignored directory: `recomp/analysis/slot11-halt-20261024/`.
Contains the session log, automatic fatal dump
`psx_freeze_dump_psx-runtime_1792828004_0.json` (about 28 MB), full 2 MiB
`halt.ram`, debug status/input, system journal and process snapshot, six copied
save/sidecar/thumbnail files in `intake/`, and `source-hashes.json`.
All six player-source files matched their intake hashes after inspection.
`dma-chain.json` is an exploratory RAM walk, not proof of a causal path.
Some attempted debug commands were unsupported; inspect each JSON's ok field.

Affected state file: `state_800AB6FC_slot10.pst`, SHA-256
`fd3a9d17b92ec731879e741fd8dd161d2b5a4873325ebcfd710332b47bad1ce4`.
Do not confuse this with file slot11 (UI slot12). Use fresh private copies;
never write the player's cards, saves or settings.

Incident/current accepted D08T7 executable:
`0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.
Pre-D08T7 D08T5 executable:
`bf634052fc9cf7758e08474d8c1f87d73d7b259a00e8a9e9cf36f70a9d397459`,
locally preserved as `recomp/analysis/d08t7-20261024/baseline-bin`.
Verify hashes before comparative tests. D08T7's native/live tests passed and
user reports solid push/pull; none of that rules out a load-related regression.

## Host evidence

08:47:44 snapshot: no Timeshift/rsync process found; CPU/I/O/memory pressure
avg10/avg60 zero; about 25 GiB available RAM, negligible swap. Journal records
automatic upgrades 08:41:10-08:41:21, Bottles scope ending 08:46:18, PackageKit
ending 08:46:19 and firmware metadata refresh at 08:47:06. No OOM/GPU reset
appears in the captured interval. A later idle snapshot cannot establish host
conditions at every frame; none of these events proves the fatal's cause.

## Fresh-context task

Read the board's D20G acceptance criteria. Review earlier D20A/D20C save/restore
fixes before changing anything. Reconstruct the incident's sequence from the
log; compare isolated load/reload and relevant multi-load routes on both builds
with matched settings. Inspect restored GPU/DMA command state and capture the
first divergence if reproducible. Keep raw observation, inference and unknowns
separate. Do not hide an opcode simply to stop the fatal report.

Deliver a reviewable regression assessment and fix decision. If a fix is
justified, scope its cause, change and regression tests; if not reproduced,
record evidence limits and a concrete diagnostic next step. A clean run does
not prove the defect absent. The user intends to review/lock this investigation
before returning to push/pull (D08T8 side mantle remains Todo).
