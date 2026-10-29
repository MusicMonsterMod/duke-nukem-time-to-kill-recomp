# D20H - Slot-5 intermittent colour corruption

2026-10-29. Investigation in progress; no fix or acceptance claimed.

## Fixture and reported evidence

Current player UI slot 5 is `recomp/saves/local-play/openbios/state_800AB6FC_slot04.pst`,
with sidecar `state_800AB6FC_slot04.pst.ttk` identifying level 1. Its SHA-256 is
`0b0976d72279c424dbfa1b488357cca3bb7bd3fd9c3b2c2bfa1960d92d1a0335`.
The save timestamp is 2026-10-29 10:32:14 UTC; the reference screenshot is
10:37:59 UTC. The scene is the Western street outside the saloon. Private copies, input files,
profiles, logs and captures are in ignored `recomp/analysis/d20h/`.

Reference found: `research/screencaps/crazy-colors.png`, SHA-256
`5c9292e0beea1115d627b09f6978325bf93c8619c2981b52a5d5f171e2c4ca46`.
Visual inspection confirms severe fine-grained colour/texture corruption across
world geometry, weapon and HUD, with recognisable scene geometry. This appearance
is consistent with invalid texture/CLUT data but does not establish its cause.
A still image cannot establish whether the condition persists.

## Prior work and source inspection

Read [D20C](178-d20c-savestate-mod-memory.md): that earlier defect was omitted
mod-memory packet arenas in savestates, allowing stale ordering-table links to
execute garbage GPU packets and overwrite VRAM. Existing code serializes those
arenas and allocation records, reserves packet arenas before load and fills
unrestored DMA storage with end-of-list tags. Current slot 5 contains MODMEM,
and baseline loads report `modmem=restored`, not absent or partial. This does
not establish that every renderer-local lifecycle is correct.

The saved resume PC is `0x80010364`, within the native object renderer; its
ordering-table bitmap is populated and contains host-arena links. Host-region
load detection currently occurs at composition or empty-OT boundaries. Whether
a mid-frame restore can reuse stale host bookkeeping before those boundaries
is a source-inspection hypothesis, not a reproduced cause; no speculative
change is made. GL full-VRAM upload/restaging and replay timeline reset paths
were also inspected. No demonstrated failure in those paths yet.

## Baseline evidence

Executable SHA-256:
`17d3e4e8c1b51e2a87fa36743e2da95ec5f4a57c9786ca3f5af77a260deabde2`.
Xvfb/OpenGL, first person, corrected geometry/textures, 16:9, 90 FOV,
120-Hz setting; all writable state private. Debug readback and concurrent
diagnostics disturb pacing, so these are restoration observations, not a
sustained-120-FPS performance result.

- `baseline`: 20 repeated slot-5 loads, 2x internal scale, short walking/turning
  between loads. Three synchronous native captures and staged presented captures
  per attempt. No reference-style colour corruption in inspected captures; no
  fatal GPU command. Presented capture completion was not polled, so their exact
  temporal assignment is not acceptance evidence.
- `scale4-present`: 24 loads at 4x with varied pre-load movement and two staged
  presented captures. No colour corruption in inspected captures, but these
  retain the same asynchronous capture timing limitation.
- A synchronized follow-up now polls `savestate_status` for completed successful
  load and `present_shot_seq` with `wrote=1` for each capture before continuing.
  `sync90` stopped during its first attempt: COW validation at 4x triggered the
  starvation watchdog after 4.64 seconds without a heartbeat. It produced no
  usable corruption evidence and is excluded from passing-load counts. The
  copied-baseline `sync90b` retry disables that costly check: 20 successful
  synchronized loads, 40 completed presented captures, no extreme colour
  corruption, zero replay worker failures/deaths. No initial-frame corruption
  was observed at sampled times; unsampled earlier frames remain unverified.

All 41 intake player files/settings retained hashes and modification times after
the first two runs. The visible sky wedge is independently reported to the
shared renderer investigation; it is not evidence of the colour-corruption cause.

## Candidate repeat-load matrix

Candidate executable SHA-256:
`89961fef6005895165c50075aaa8f4679319c4d9b86a87b1761f1b6ba99aef61`.
The private profile copies the user's settings, including strong view bob,
borderless HUD and corrected geometry/textures; tests override view, FOV,
120-Hz rate and aspect explicitly. Each load waits for successful status, then
each presented capture waits for completion and `wrote=1`.

| Run | Completed loads | Colour result | Other outcome |
| --- | --- | --- | --- |
| `candidate64`, 4:3, 4x | 8 | No extreme corruption in 16 completed captures | No worker deaths/failures |
| `candidate90`, 16:9, 4x | 12 | No extreme corruption in 24 completed captures | No worker deaths/failures; Western sky wedge remains separately reported |
| `candidate120`, 16:9, 4x | 7 | No extreme corruption in 14 completed captures | Watchdog after subsequent movement, before eighth load request |

The last run logged guest pacing at 1.1 frames/s before a 4.10-second heartbeat
timeout; its starvation ring is preserved locally. Immediate host pressure
sampling found recent CPU pressure and no memory pressure, but post-halt host
state does not establish conditions or causation at the halt. Xvfb uses software
rendering on this setup; these costly readback runs cannot establish the actual
GPU's 120-Hz gameplay performance. No GPU unknown-command diagnostic or extreme
colour corruption was captured. A subsequent 2x attempt recorded five clean
loads before intentional shutdown while switching investigation scope; it is
not included in the completed matrix and is not labelled a second watchdog.

All original player file hash/mtime checks pass after these runs. No renderer,
savestate or game implementation was changed for D20H.

## Remaining work

The synchronized baseline and candidate matrix supply 47 completed load
observations with no reference-style corruption, plus earlier exploratory runs
with the timing limitations above. The corruption remains unreproduced; no
confirmed cause, corrective implementation or fixed claim. Initial-frame-only
versus persistent behaviour remains unresolved. Preserve any future failing
private state, log, VRAM and rendered captures before reloading.
