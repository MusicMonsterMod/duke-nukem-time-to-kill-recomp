# D20G - Slot 11 reload fatal halt investigation and recurrence lessons

Status: Done (user-accepted 2026-10-24). Closed as an investigation, not a fix:
fatal not reproduced, cause and regression origin unresolved. User explicitly
authorized closeout and asked that these lessons be consulted if similar symptoms recur.

## Recurrence checklist for future Done-job reviews

If a savestate reload causes another GPU GP0 halt, wrong colours or scrambled
geometry, revisit D20G and [D20C](178-d20c-savestate-mod-memory.md) before
starting a new diagnosis. A matching opcode alone does not establish the same cause.

- Preserve the halted process long enough to capture RAM **and live mod-memory
  aperture/Expansion 1**, plus GPU/DMA state, failing packet source, load status,
  logs, executable/settings identity and private copies of the relevant saves.
- Reconstruct the load order and intervening play; compare baseline/candidate
  with identical private copies. Check host history as well as game evidence.
- Do not fold mod-aperture links into main RAM when walking the DMA chain.
  The missing live aperture prevented a reliable reconstruction in this incident.
- A successful restore or `modmem=restored` does not prove the following frame
  is safe; a clean short replay does not prove absence or a corrupt save.
- Continue from the bounded diagnostic plan below, including mid-composition
  host rendering state. Do not suppress an opcode or assume D08T7 caused it.

The 82-load comparison and its limits remain the evidence baseline. Reopen or
scope follow-up work when similar symptoms provide new evidence; no automatic
follow-up is started by this closeout.

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

## 2026-10-24 - Investigation result (accepted)

The user selected D20G in fresh context, then accepted this investigation and
authorized closeout. No gameplay/framework code, executable, player settings,
player saves or cards were changed. D08T8 remains separate.

### Comparative method and identities

Local evidence: `recomp/analysis/d20g-20261024/`. `compare.py`, `menu.py`, their
logs and JSON results retain the routes. Both binaries were copied into private
runtime directories with the existing assets/cache linked alongside them. No
rebuild. SHA-256 verification matched the two executable hashes above.

Private slot 11/12 copies came from the incident intake, including sidecars and
thumbnails. UI slot 10 came from the earlier D08T7 private intake; its identity
at the incident cannot be independently established from the log. Save hashes:

| UI slot (file index) | State SHA-256 |
| --- | --- |
| 10 (09) | `6351d3888fc406667429c74a2ae603299a666f3666fc5d2635133a5d04fc44eb` |
| 11 (10) | `fd3a9d17b92ec731879e741fd8dd161d2b5a4873325ebcfd710332b47bad1ce4` |
| 12 (11) | `9b9e762b63042a3f174a17beadbcb6dc482f63001a8de8ac9a173bfcd76be884` |

Both builds used the same Modernized profile: first person, independent camera,
view weapon aiming, corrected geometry/textures, 16:9, extended draw distance,
manual jump, fast CPU timing, 150% overclock, strong view bob, camera distance
2304. Each run used a separate writable-state directory, fresh cards, copied
saves and private settings on port 9275. Audio was dummy.

Diagnostic route: OpenGL 4x, NVIDIA 580.178.04 offscreen, fixed 120 FPS preference.
Eight debug loads of UI slot 11, followed by six repetitions of UI slots
12 -> 11 -> 10 -> 11. After each load: 0.5 s settle, 1 s idle or synthetic
W/mouse-turn input on alternating iterations, 0.5 s settle. Every operation
checked successful load generation and subsequent frame progress. This is 32
loads per build. The input diagnostic confirmed capture and synthetic movement
were active. All loads reported `modmem=restored`; neither log contains FATAL.

Menu route: private Xvfb display, OpenGL 1x/Mesa, `auto` FPS preference, actual
xdotool W/mouse input, F3, Down to the target slot, Enter, Enter. Starts with one
debug load of UI slot 12, then two repetitions of 12 -> 11 -> 10 -> 11 through
LOAD GAME. Successful load generation and frame progress are checked after each
confirmation. The captured confirmation visibly names SLOT 11 - FAMILY JEWELS.
A 4x Xvfb trial was stopped and excluded: it ran at about 3.2 emulated frames/s,
with the game consuming roughly nine host CPU cores; current pressure showed
CPU/I/O contention but ample memory. This is a test-environment limitation,
not evidence about the original incident or a new game regression.

### Results

| Build | Diagnostic loads (4x, 120 preference) | Menu route (1x, auto -> 60 Hz target) |
| --- | --- | --- |
| Accepted D08T7 | 32/32 successful, frame progress, no FATAL | 8/8 menu loads + setup load successful, frame progress, no FATAL |
| Pre-D08T7 D08T5 | 32/32 successful, frame progress, no FATAL | 8/8 menu loads + setup load successful, frame progress, no FATAL |

That is 82 successful restores across the two builds. Log sequences were
checked against the scripted routes, not merely counted. Both final menu-route
screenshots show the expected first-person room without obvious corruption;
this is not visual verification of every restored frame. All six original
slot-11/12 save/sidecar/thumbnail files still match the incident intake hashes.
`summary.json` and `source-verification.json` retain these checks.

### What the captured failure establishes

- Incident sequence is UI 12 -> 11 -> 10 -> 11, with substantial intervening
  gameplay over about eleven minutes. The final load restored PC `0x8001188C`,
  SP `0x801FFE98`, RA `0x80011894`: a world-polygon rendering continuation.
- Read-only parsing of the affected save finds all 18 sections, including
  987012 bytes of mod memory. GPU parser is idle (`gp0_state=0`); DMA linked-list
  state is inactive. Thus the fatal is not simply continuation of an active
  DMA transfer stored in this save.
- The fatal dump records a newly started DMA2 list at `0x800F40E4`, with the
  last DMA start attributed to retail PC `0x800B2A24`. Its reported 78-word
  count belongs to the currently active node, not necessarily that start
  address. The RAM header at the start address describes a nine-word packet;
  that difference alone is not proof of corruption.
- The first 389 nodes of the post-mortem RAM walk reach a link from `0x112E8C`
  into the mod DMA aperture at `0xF22AD0`. The intake's RAM-only `dma-chain.json`
  then folds the address into ordinary RAM. Its resulting bad chain is not a
  valid reconstruction. The fatal capture omitted the live aperture bytes.
  Substituting saved aperture bytes would mix different points in execution,
  so that exploratory walk also cannot prove the path.
- The fatal word occurs at main RAM offset `0x120394` in both the save and halt
  RAM. This nominates a possible unwanted data read; it does not establish the
  GPU's source address, since the live aperture was not captured.
- D20C's mod-memory serialization and startup reservation are present in both
  builds, and the incident reports `modmem=restored`. Missing section data, the
  earlier D20A failure, is not demonstrated here. The GPU snapshot includes
  parser state and DMA snapshot includes list progress. These facts do not
  prove all title-side transient rendering state is restored correctly.

### Assessment and bounded next step

The strongest supported description remains a post-restore GPU command-stream
failure. Its exact producer and regression origin are unresolved. Mid-frame
restore interacting with host-side renderer bookkeeping is a plausible lead:
near-clip allocation cursor, region, held packets and subdivision restoration
include host-side state; region reconciliation occurs at later drawing hooks.
The saved PC can resume inside an already-started composition. This is a code
inspection hypothesis, not a diagnosed defect or evidence against D08T7.
D08T7's tracked implementation changes are confined to push acquisition/guards;
that narrows review but does not rule out an indirect effect.

The captured host history was reviewed: upgrades ended at 08:41:21, PackageKit
ended 08:46:19, and the later snapshot was idle. No captured OOM/GPU reset or
host event establishes a cause for the guest command failure. The earlier
08:04 mantle slowdown is a separate incident and is not causal evidence here.

A behavioral fix is not justified by the present evidence. Do not suppress GP0
0x1F, discard a player's save, roll back D08T7, or describe this as fixed.
The bounded follow-up, if the failure recurs or deeper investigation is chosen:

1. Before quitting the halted process, capture main RAM plus the live 1 MiB
   aperture at `0x80F00000` and Expansion 1 at `0x9F000000` using `read_ram`,
   together with frame, DMA/GPU state, savestate status and the fatal dump.
   Keep all raw bytes local. The missing aperture is the immediate evidence gap.
2. In a private diagnostic build, retain the failing GP0 source address and
   complete DMA current/next node, parser collection state, and a small ring
   of packet headers around the first post-load DMA. Log near-clip region,
   cursor, pending subdivision restore and load-generation reconciliation.
   Compare the same load boundary on both builds. Avoid changing opcode behavior.
3. If this identifies a stale host-state write, scope the fix to that owner's
   load boundary and add a failing-before/passing-after test with this private
   save, mid-composition loads, alternating slots, fresh-process loads,
   first/third person, 60/high refresh and Vanilla. Preserve old-format D20C
   coverage and verify/export any framework changes against the pinned source.

Limits: these routes are shorter than the incident, do not replay its exact
push/pull/mantle input history, and do not recreate its approximately 90
presents/s cadence. The incident's exact settings were not captured. A clean
comparison cannot prove absence, save corruption, or a pre-existing defect.
Vanilla was not exercised in this investigation; its code and build are unchanged.
