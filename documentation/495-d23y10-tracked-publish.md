# 495 - D23Y10: publishing a redraw image copies only what changed

Autonomous three-machine session of 2026-11-16 (desktop lane, then the
field laptop). User: "D3. If time remains: D23Y5's image publish without a
copy (copy-on-write fork or a double-buffered image the emulation thread
writes in place), exact by construction, measured on the laptop."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. What a publish did (CODE)

Each published frame (`psx_mod_replay_publish`, render_worker.c) copied the
whole redraw image into its shared slot on the emulation thread:
`render_replay_image_save`, 2 MB of RAM, the scratchpad and the used parts
of mod memory and both DMA apertures, 3.5-5.5 MB (note 493 section 3). The
slots are reused oldest first, eight of them, so a slot is republished
every eighth publish. A whole publish took 656 us on the desktop (UI 2,
180, one worker); note 490 measured 3.4% of the emulation thread at 120 on
the field laptop for it.

## 2. Choice

- A copy-on-write fork per publish: the redraw workers are long-lived
  processes reading shared slots; a fork per frame would copy the page
  tables of a process with hundreds of MB mapped every publish (INFERRED:
  dearer than the copy) and change the worker model.
- A double-buffered image the emulation thread writes in place: the guest
  memory would have to live in the shared slots, the slot rotation and the
  "still being read" rule rebuilt; a large framework change.
- Copy only what changed since the slot's last publish, with the kernel
  saying what changed: the same asynchronous userfaultfd write protection
  and `PAGEMAP_SCAN` that D23N5's restore uses (note 493). Exact by
  construction and a small change; chosen.

## 3. The change (CODE; developer `PSX_PUBLISH_TRACK=1`, off by default)

The emulation process write-protects its RAM, mod memory and DMA window
(registered at the first publish, Linux 6.7+). A write to a protected page
is resolved in the kernel and leaves the page "written". At each publish
one scan lists the pages written since the previous publish and protects
them again; the list is added to every slot's pending set. The slot being
published receives only its pending pages and any range an extent (mod
memory used, DMA aperture used, the extra aperture's high-water mark) grew
over since that slot's last publish; the header and scratchpad are copied
whole; its pending set is cleared. A page nothing wrote since the slot's
last publish already holds the bytes a whole save would copy, and bytes
past an extent are left as a whole save leaves them, so the slot equals a
whole save. A slot's first publish, and any kernel failure, saves whole.
memory.c (`render_replay_image_save_tracked`), render_worker.c. The
workers are forked without the parent's registration (the kernel drops it
in a child without fork events) and register their own for D23N5.

Developer check `PSX_PUBLISH_TRACK_VERIFY=N`: every Nth tracked publish is
followed by a whole save of the same moment into a scratch buffer, compared
byte for byte with the slot over the extents a load copies.

## 4. Desktop (MEASURED)

UI 2, 180, one worker, 30 s, every tracked publish checked: 1,016 of 1,016
identical to the whole save; a tracked publish 91-94 us (113-118 pages
copied, 71-73 pages written between publishes) against 656 us whole.

Interleaved A/B, two runs each, the emulation process's CPU (the game's
processes less the workers, % of a core):

| View | off | on | publish (tracked) | pages written / copied a publish |
| --- | --- | --- | --- | --- |
| UI 2 | 70.0, 69.0 | 68.2, 67.3 | 103 us | 85 / 133 |
| UI 5 | 62.1, 63.1 | 61.7, 61.5 | 61 us | 49 / 62 |
| UI 9 | 37.2, 37.2 | 34.6, 33.8 | 62 us | 50 / 63 |
| FJ start | 67.2, 67.9 | 67.0, 66.0 | 75 us | 65 / 89 |

Every run with the tracked publish is lower than its pair: 1-3 points of a
core.

Proof (`proofy.sh`, every tracked publish checked, 40 s a view):

- UI 2, 5, 9 and the FAMILY JEWELS start at one worker, and again at two
  workers with D23N5's restore on as well: 8 runs, 1,016 tracked publishes
  compared a run (the last periodic report), 0 differing.
- Render-thread validator (`PSX_RT_VALIDATE=1`, tracked publish on, one
  worker, UI 2, 8, 9, 40 s each): 908 of 908 images exact.
- Vanilla (`--vanilla`, UI 8 and UI 2, shots at +60, +200, +400 frames),
  `ab/now5` against this build: byte-identical, 6 of 6. (Vanilla publishes
  nothing: the tracking is never registered there.)
- Framework patch exported; the pinned framework (`08ec704`, from its public
  upstream) plus the patch reproduces all 90 patched files; the three
  compiled framework tests CI runs pass.

## 5. Field laptop

To be measured (the acceptance): the emulation thread at 120 with the
tracked publish off and on.

## 6. Reproduce

`recomp/analysis/d23n5/` (desktop, local): `abx.sh` (`ON=...`), `proofy.sh`;
`recomp/analysis/d23ab/` (field laptop).
