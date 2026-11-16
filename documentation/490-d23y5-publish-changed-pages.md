# 490 - D23Y5: publishing only the worker image's changed pages is slower (field laptop)

Autonomous overnight session of 2026-11-16 (laptop lane). User: "D23Y5:
the published worker image (7.2% of the emulation thread at 120, note 485):
publish only what redraws read (dirty RAM pages, mod memory in use), exact
by construction; re-profile with y5n.sh before/after."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. What a publish copies (CODE, MEASURED)

`psx_mod_replay_publish` saves the guest image into a worker slot
(`render_replay_image_save`, `memory.c`): main RAM (2 MB), the scratchpad,
the mod memory in use, the GPU DMA aperture in use and the extra aperture up
to its high-water mark; 871-1,248 pages of 4 KB a publish in UI 2, 8 and 9
(3.5-5.0 MB). Mod memory and the apertures are already copied only as far
as they are used.

## 2. Which writes could be tracked (CODE)

Generated code reads main RAM inline but stores through `psx_write_*`; RAM
is also written by DMA, the overlay loader's shadow copies
(`overlay_loader.c`), savestate loads and host plugin writes. A dirty-page
record would have to see every one of these paths to be exact, so it is
not exact by construction; it was not built.

## 3. The exact-by-construction variant: copy only differing pages (MEASURED)

Built behind developer `PSX_REPLAY_PUBLISH_DIFF=1`: each 4 KB page of the
image is compared with the slot's copy (8 publishes old) and copied only
where it differs, so the slot holds byte for byte what the save writes.
`PSX_REPLAY_PUBLISH_STATS=1` timed both.

| Machine, view | whole copy | changed pages only | pages copied |
| --- | --- | --- | --- |
| desktop, 180, UI 2 / 8 / 9 (two runs) | 601-609 / 592-597 / 643-646 us | 583-585 / 568-571 / 605 us | 116 / 94 / 82 of 1,148-1,248 |
| field laptop, 120, UI 2 (two runs) | 951-953 us | 1,163-1,188 us | 85-86 of 1,148 |
| field laptop, 120, UI 8 | 680-706 us | 803-818 us | 77 of 871 |
| field laptop, 120, UI 9 | 803-821 us | 929-976 us | 78-80 of 1,033 |

Emulation-thread profiles, field laptop, UI 2 at 120, interleaved, two runs
each (`y5q.sh`, `incl.py`): `psx_mod_replay_publish` 7.0-7.2% (whole copy)
against 7.3% (changed pages); its image part `render_replay_image_save`
3.3-3.4% against `render_replay_image_update` 4.5-4.6%.

Although only 7-10% of the pages differ, reading the slot's old copy (cold,
in shared memory) costs more than the writes it saves: 4% less on the
desktop, 15-25% more on the field laptop's two-core, dual-channel memory.

## 4. Decision

Not cheaper where it matters: reverted (not committed). D23Y5 stays
Blocked; what is left of the publish is the copy itself (3.4% of the
emulation thread at 120). The next lever is not copying at all: workers
reading the live image copy-on-write (a fork per published frame) or a
double-buffered image the emulation thread writes in place, each a larger
change with its own exactness proof.

## 5. Reproduce

Field laptop, `recomp/analysis/d23y6/` (local): `y5p.sh` (the A/B with the
timer), `y5q.sh` (the profiles), both against the reverted code in this
note's diff (local lab).
