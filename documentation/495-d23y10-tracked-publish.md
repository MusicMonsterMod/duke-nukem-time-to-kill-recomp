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

Desktop, D23N5's restore and the tracked publish both on, against both
off, one worker, two interleaved runs (the emulation process, % of a
core): UI 2 70.0, 70.0 / 67.0, 67.2; UI 5 63.6, 62.4 / 61.3, 60.9; UI 9
36.0, 36.5 / 34.1, 33.6; the FAMILY JEWELS start 67.7, 68.1 / 66.6, 64.5.
Both on, both checks, 120 s in five views: 17,880 tracked publishes, 0
differing (note 493 section 6b).

## 5. Field laptop (MEASURED)

The build at `2e10fa8`, offscreen, host checked before every run, 30 s
walks:

- The check (every tracked publish), UI 2 and the town at SIMPLE 120:
  2,032 of 2,032 identical to a whole save; a tracked publish 138-148 us
  (90-93 pages copied, 64-68 written between publishes). At the player's
  4x the redraws mostly stay off (note 494), so too few publishes ran to
  report.
- Off against on, two interleaved runs, the emulation thread (% of a
  core, medians): SIMPLE 120: UI 2 59 / 58, the town 72 / 72, UI 8 72 / 71,
  UI 9 50 / 49, the FAMILY JEWELS start 63 / 60; pictures 120 in every
  second either way. At the player's 4x the runs follow the package limit
  (note 494): the town's game rate 49.8 against 47.3, UI 9 presenting
  more redraws with the publish on (57 pictures against 44) and so more
  emulation-thread work; no conclusion there.

So on the field laptop the tracked publish removes 0-3 points of the
emulation thread at 2x and 120 (about what its 3.4% publish share of note
490 allows), and nothing at 4x, where redraws do not run. The D23Y5 target
(under 50% at the original 60) is not touched: at 60 nothing is published.
It stays a developer switch, off by default; default on is the user's
decision.

## 5b. Citadel (MEASURED)

The build at `2e10fa8`, AUTO's 3 workers, offscreen, host checked before
every run:

- The check (every tracked publish), UI 2 and the town at AUTO (2x) and
  ULTRA (4x), 120: 3,040 tracked publishes, 0 differing; 146-240 us a
  tracked publish (89-95 pages copied, 64-68 written between publishes).
- Off against on, two interleaved runs, the emulation thread (% of a
  core): AUTO 120: UI 2 62 / 60, the town 74 / 72, UI 8 75 / 74, UI 9
  56 / 53, the FAMILY JEWELS start 62 / 60, pictures unchanged (UI 9 110
  either way); ULTRA 120 (GPU-bound): 1-13 points lower, but the runs with
  the publish on also presented fewer redraws in UI 2 and UI 9, so only
  the town and UI 8 (71 / 74, 73 / 74) compare.

## 6. Reproduce

`recomp/analysis/d23n5/` (desktop, local): `abx.sh` (`ON=...`), `proofy.sh`;
`recomp/analysis/d23ab/` (field laptop).
