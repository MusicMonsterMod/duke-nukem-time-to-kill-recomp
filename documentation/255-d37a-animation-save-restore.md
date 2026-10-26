# D37A - Animation recovery across save allocation layouts

## Acceptance - 2026-10-26

Done. User: "i have tested it and accept fully". The user requested the status
correction during D36J selection. The implementation and bounded verification
below remain unchanged; no new build or test is claimed for this correction.
Earlier Needs playtest entries below are historical. D19Q3 is separate.
No commit or push was requested.

Session closeout update (2026-10-26): user authorized commit and push of the current
work. Needs playtest status remains; D19Q4 records the new UI-flash reports as
backlog only. Earlier no-commit statements below describe the implementation turn.

Date: 2026-10-26. Status: Needs playtest.
D37 research is closed by explicit user instruction. The repair is implemented and
built for review; repair acceptance, commit and push have not been requested.

## Result

Existing saves, including UI slot 9, can recover their animation tables when loaded
into a process whose enhancement allocations were created in a different order.
The loader retains its protection against incompatible GPU packet lists. No save
format migration, slot deletion, replacement art or control change is required.

The reproduced route is new game -> load slot 9 -> resume -> move. Before repair,
partial mod-memory restore cleared the animation header and Duke could turn the
camera but not move horizontally. The repaired build reconstructs valid tables in
its own allocation and movement/animation advance. Repeat loads and cross-level
loads are covered by the private checks below.

## Implementation

- Runtime `memory.c` keeps a read-only copy of the latest validated saved DMA
  allocation image. `psx_mod_memory_saved_gpu_allocation` exposes only a bounded
  prefix of an exact recorded allocation. It does not install saved packet arenas
  or alter live allocation ownership. Missing, malformed and unknown-version
  mod-memory sections invalidate the retained image. Render replay cannot replace it.
- `anim_save_header.h` validates the TTKA v1 header, main-RAM original table ranges,
  counts, frame spans, clip records and saved/live pointer relationship before any
  recovery writes. Original clip indices and extra host IDs are separate namespaces.
- `anim_override.cpp`, at the guarded player-update hook, retrieves that saved
  header, restores the original clip records and table pointers, rebases active
  original-clip channels, and reapplies the current animation pack in this process's
  buffer. A compatible fully restored allocation may be adopted; a mismatched one
  is not. Vanilla restores the original tables without applying the modern pack.
- Rejected metadata is not reinterpreted through the old weaker fallback on later
  ticks. It produces a diagnostic; the implementation does not invent missing data.

The saved source costs a fixed 1 MiB of host storage plus allocation records. The
copy happens on save load, not each frame. Wire format and GPU end-of-list clearing
are unchanged. This does not claim to reconstruct a save whose animation metadata
was already destroyed before it was written.

## Verification

Native regressions:

- `python3 recomp/tools/local/test_animation_save_restore.py`: PASS. Compiles the
  actual runtime memory implementation with the configured build flags. Tests
  mismatched layouts, immutable saved allocation lookup, exact bounds, compatible
  restore, replay isolation, missing/malformed/unknown-version invalidation, and
  confirms live incompatible GPU memory remains filled with end-of-list packets.
- The same runner checks animation metadata validation, malformed pointers/counts,
  duplicate records, invalid spans and independent original/extra ID namespaces.
- `python3 -m unittest discover -s recomp/tests/local -p test_ttk_anim.py`: 15 PASS.
- Incremental player build: PASS; movie and geometry shards current.

Private live evidence: `recomp/analysis/d37a-repair/` (not tracked).
`final-results.json` has 13 passing Modernized routes and one Vanilla route;
`third-results.json` has five passing third-person routes. All use private settings,
private cards and copied player savestates; no player card was read or written.

- Warm/new-game allocation order: slot 9 twice, slot 8, newly saved slot 1, slot 9;
  level-select 6, save private slot 2, return to slot 9 and reload level 6.
- Fresh/load-first allocation order: slot 9, newly saved slot 1, slot 9, level-6
  private slot 2, slot 8. This exercises both allocation layouts in one process.
- Both views: live first-person effective flag true in the first suite and false
  in the third-person suite. Every movement check has four distinct X/Z samples,
  pause cleared and valid TTKA header. All 18 modern routes record crouch entry;
  first-person quick-kick counters advance in all 13 first-person routes.
- Vanilla: fresh new game, original pad movement and original main-RAM animation
  tables, no modern override application. This is a bounded regression check.
- The broad suite ran on intermediate build
  `eec962f14647e6bcfbf861bee14fd92d4832a2b2885c7779ba3a48725ac9525d`.
  The final build only relaxes the decoder to correctly distinguish the original
  and extra ID namespaces, plus comment/unreachable-condition cleanup. The native
  regression covers that difference; final-build warm-load smoke evidence is in
  `smoke-results.json` and `smoke-warm.log`: slot 9 twice and slot 8 all PASS.

Final candidate SHA-256:
`b94b58e4be7cbeda84b60494d3a9e814c08b4160bfcea6feba0c9ea96040a738`.

Harness corrections: debug loads of these saves can expose original paused menus;
original confirm (`x` in the Modernized test input file) resumes them. Initial
paused samples were not counted as movement passes. Temporary slot numbers above
the supported range were refused, then replaced with private slots 1/2. A diagnostic
`act=camera_view` command does not exercise the physical camera binding; separate
first/third launch profiles establish view coverage instead.

Framework patch exported with `export_runtime_patch.py`, applied to a clean archive
of pinned `08ec704a974b1f3a16335b4afeb340b9eff19926`, and all 52 changed framework
files compared byte-for-byte with the working source. Existing framework changes
are preserved. No dependency pointers changed.

## Player acceptance remaining

The checks use offscreen OpenGL, dummy audio and sampled state, not a visible
campaign playthrough. Please load slot 9 after playing another save or starting a
new game, then try normal movement, turning, crouching and quick kick. Repeat a
load without restarting. Existing saves should work without deletion/recreation.
Visual feel and broader campaign acceptance remain for the user.

Launch with existing player preferences:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Modernized gameplay capture is automatic; F10 is not an activation step.
