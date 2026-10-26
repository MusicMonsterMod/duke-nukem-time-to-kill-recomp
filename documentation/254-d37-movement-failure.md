# D37 - Movement failure around 07:26 UK

Date: 2026-10-26. Status: Done. Research accepted and closed by user instruction; repair selected as D37A.

## Finding

The strongest lead is a reproducible save-load animation-memory failure. A new-game
session followed by loading the private copy of UI slot 9 produces `modmem=partial`
and leaves the animation globals pointing into cleared enhancement memory. A fresh
process loading the same file produces `modmem=restored` and a valid animation header.
This matches the before/after restart distinction in the incident logs. There was
no live memory capture during the original incident, so historical attribution is
strongly supported rather than directly proven. D36 visual defects are not implicated.

## Historical chronology

Local Europe/London time was GMT (+00:00), equal to UTC. The launcher names logs with
`datetime.datetime.now()` in `recomp/tools/local/run.py:345`; individual stderr lines
have no timestamps. File modification times bound the last recorded output, not
individual actions. UI slots are one-based; runtime slot 8 is UI slot 9.

- `session-20261026-071555.log`: starts 07:15:55, last write 07:26:12.748712 UTC.
  Initial frontend lease refusals precede gameplay and are not evidence of the later
  incident. Gameplay, traversal, saves, and camera switches follow.
- Lines 89-110: UI slot 9 loads twice, both `modmem=partial`, followed by capture and
  Escape/resume messages. Lines 111-121: UI slot 8 also loads partially, with camera
  switching afterward. This is compatible with the reported persistent failure.
- Last pacing summary is 90.0 presents/s. Earlier summaries include 77.7 and 82.5
  during save/menu activity. These count presentation, not advancing simulation or
  animation, and cannot certify healthy movement.
- `session-20261026-072626.log`: restart at 07:26:26, last output 07:31:17.457622 UTC.
  First load is the same UI slot 9 (same size and saved PC), now `modmem=restored`,
  followed by animation override application. UI slot 8 also restores, followed by
  pushable-object and ladder activity. User reports normal play after restarting.
- No fatal GPU error or identity refusal at the failing loads was recorded.

Original logs were copied to `recomp/analysis/d37-investigation/evidence/` before
any test launch. The launcher retains only five session logs, so preservation matters.

## Host correlation

Journal, Timeshift log, APT history and historical atop records were inspected.
The atop 07:30:02 sample covers 07:20:02-07:30:02, including the incident:

- CPU pressure: 1.800 seconds of some-stall over 600 seconds (about 0.30%).
- Memory pressure: 0.00596 seconds some-stall; 0.00544 seconds full-stall.
- I/O pressure: 0.203 seconds some-stall; 0.191 seconds full-stall.
- System disk busy time: 1.686 seconds over 600 seconds (about 0.28%).
- Available memory approximately 23.1 GiB. Swap occupancy is not evidence of
  active thrashing; the pressure samples do not indicate sustained thrashing.

The 07:00 Timeshift check finished in the same second, with the daily snapshot
not yet due. No rsync backup overlap is established. APT's last recorded package
transaction finished October 25 at 00:10:38. Earlier update/index activity and
07:36 cron.daily do not coincide with the incident. The inspected journal has no
OOM kill, GPU Xid/reset or disk I/O error in the incident window. Repeated
playerctld messages are present; they are not proof of causation.

Current host readings at 07:44:59 showed load 0.58/0.63/0.92, about 23 GiB available
and 714 MiB swap occupied. These are explicitly not historical incident evidence.
Atop's ten-minute aggregation cannot exclude a short CPU/core stall, and there is
no per-frame host recording from 07:26. Sustained host contention is poorly
supported compared with the reproduced memory failure.

## Source mechanism

- `recomp/psxrecomp/runtime/src/memory.c:226`: `modmem_region_restore` restores only
  the matching allocation prefix when current and saved allocation layouts differ.
- `memory.c:270`: the un-restored DMA aperture is filled with `0x00FFFFFF`, an
  ordering-table terminator. This protects GPU traversal from stale packet links.
  `partial` alone is not an error and can be harmless for rendering arenas.
- `recomp/src/ttk/anim_override.cpp:204`: animation overrides also allocate in that
  DMA aperture. They contain copied frame/command tables, not just GPU packets.
- `anim_override.cpp:272`: after loading, an invalid animation header causes an
  early return. The guest globals remain outside main RAM and still point at the
  invalid tables. The file's introductory recovery comment is broader than the
  actual invalid-header implementation.

The private new-game animation table was at `0x80FAF230`; the saved table is at
`0x80F80230`, with header at `0x80F80010`. On warm load its magic becomes
`0x00FFFFFF`; on fresh load it is valid `0x414B5454` (TTKA). This is direct evidence
of animation data destruction in the partial-restore path, beyond the log label.

## Private reproduction and limits

Used the unchanged accepted executable, SHA-256
`9a81112c57148d928c00f8cfbd7aed33a88365640cc766395d45d1305c6beb04`.
Private settings, empty private memory cards and copied savestate files only;
no player card was read or written. Test route and JSON are local under
`recomp/analysis/d37-investigation/`. SDL offscreen OpenGL and dummy audio were used.
No build, source edit, memory patch or change to the accepted binary was made.

Route: start a new game, load UI slot 9, repeat that load, then load UI slot 8;
compare a fresh process loading UI slot 9 then UI slot 8. Resume paused saves with
menu Enter before injecting W/S/A/D and a mouse sweep. Early paused-only runs are
not valid movement tests; their header observations remain valid. Final probe
records pause flags, frames, input, XYZ, animation and table/header pointers.

The warm-load test reproduced the key behavior after pause became zero: injected
W/S/A/D changed input values but X/Z stayed fixed; animation stayed 63 on slot 9,
while frame counts advanced and mouse sweep changed yaw from -0.195156 to
-0.012943. Repeating the load reproduced this. Slot 8 remained horizontally stuck
with limited animation changes (63/72). This is a simulation/animation failure,
not merely a frozen displayed frame. The historical incident's exact pre-load
allocation history and perceived stutter were not captured.

The fresh-process diagnostic comparison confirmed full restoration and valid header,
but its loaded paused menu did not resume through the injected menu keys (including
a 25-second delayed-load retry). Those stationary fresh samples are not movement
passes. Fresh-process normal gameplay is supported by the user report and historical
post-restart activity, not by this harness. `warm-confirmed-results.json` preserves
the resumed failing run; `results.json` contains the final fresh comparison.
All diagnostic processes exited. Offscreen tests do not assess audible or visual
quality, and the precise original allocation mismatch has not been reconstructed.

## Recommended next job

A separately selected fix should make animation save restoration safe across
allocation-layout differences. Preserve valid saved frame/command data and metadata
before generic DMA clearing, or relocate/rebuild the animation tables using verified
original pointers and clip records. Do not simply disable the GPU safety fill or
blindly reinterpret cleared memory as animation metadata.

Acceptance should cover both load orders, repeat loads, new-game/attract allocation
history, cross-level loads, both perspectives, crouch overrides and quick kick;
prove movement and animation advance and retain D20C/D20G GPU safety. Preserve
Vanilla and player files. Add a focused regression that exercises layout mismatch
with animation data, not just a `partial` status assertion.

Until fixed, the user-confirmed workaround is restarting before loading the save.
If it recurs, retaining the live process long enough to capture `ttk_input`, frame,
FRM/ROT/header and host pressure data is more useful than trying random inputs.
No need to overwrite the save. No commit, push or acceptance closeout performed.
