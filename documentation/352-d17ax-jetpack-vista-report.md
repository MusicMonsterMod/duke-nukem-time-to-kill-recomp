# D17AX - Whole level from jetpack height: report

2026-11-01. Report only; nothing player-facing changed. The job's acceptance
starts with the user's choice of fix.

Evidence is local under `recomp/analysis/d17ax/` (scripts `circle.py`,
`crashprobe.py` and `levels.py`, plus shots, sheets and JSON). The user video is
`research/screencaps/Video_2026-11-01_18-52-03.mp4`, with frames in `video/`. Every
run used private copies of the player's card and savestates (UI slot 10 =
file slot 09, saved 2026-10-30 12:24, level 6), offscreen video, dummy
audio and private debug ports. Each script hash-checks the player's saves
and profile before and after the run.

## Developer mode added

`DNTTK_HOST_VIS_ALL` (Polymorph host visibility, `host_visibility.inc`) adds
rooms to the host selection and draws them through the whole guard-banded
view. Piece boxes are still tested against the view. It takes `1` (every
room), `sky` (every sky room) or a comma list of room numbers. These rooms
stay out of the props/actors list and the shown-room set, because their
objects belong to the game. The `draw_distance.host_visibility` debug object
now includes `list`: the rooms selected in the last frame, with bit 8 set
for rooms that had a piece in view. The default build behaves as before:
the switch is off, `ttk-controls-test` passes all 119 groups, and
`ttk-near-test` and `ttk-input-test` pass.

## Level 6 layout (room table)

Level 6 has 58 rooms, 12 of them sky rooms (flag bit 3). Duke hovers at
X 5166, Y -1900, Z -4580 in sky room 1 (the jetpack ceiling is Y -2048).
Every room's pieces lie inside its box. All rooms together span X -29696 to
39936 and Z -6144 to 68608. Only room 1 reaches below Z 0, down to -6144.
The castle interior is rooms 28-57 (not sky rooms; towers reach Y -14336).

## Per region

Hover, a full turn in 16 headings (`shots/base`), compared with the
developer mode (`shots/sky`, `shots/outer`, `shots/inner`, `shots/high`,
`shots/low`):

| Region | Cause | Proof |
| --- | --- | --- |
| Behind the camera (yaw about 2.5 to -2.5) | Authored void | No room exists past room 1's outer bank (Z -6144). With every sky room or every other room drawn whole-view, the same 0-1 pieces are in view. |
| Both side banks (past the moat's outer edge) | Authored void | The moat rooms (2, 3, 4, 20, 21) end at the level's outer bounds. Forcing any room group adds nothing beside the moat. |
| Castle interior band: towers and gatehouse | Geometry the host pass does not select | With the interior rooms drawn, the keep tower and the gatehouse above the gate appear above the front wall (`crop-inner.png`, `crop-hl.png`). Rooms 33, 34, 38 and 43-49 carry them. The over-the-walls test (D17AQ/D17AR) considers sky rooms only, so these rooms are never candidates. |
| Rest of the band above the wall | Authored void plus the backdrop's dark lower part | The yard floor lies below the wall top. Past the castle's far end nothing exists. The sky model's mountain ring is a black silhouette with only the clear colour below it, so the band is backdrop and void, not missing geometry. |

The host selection already covers every sky room with geometry in view. At
heading 0 it selects 14 rooms with 103 pieces in view. Drawing all 12 sky
rooms whole-view gives the same picture. Neither the guard band, the 65535 far
limit nor the original room rectangles hide anything here.

As found in D17AQ, Vanilla shows the same void from this height, and the
PlayStation camera never looked over these walls.

## Side finding: arena exhaustion

With every room (`1`) drawn whole-view, the near-clip packet arena filled
(139616 of 139744 bytes per frame). About 430 meshes a second then fell back
to the game's original renderer. That overran the game's packet area:
textures turned to garbage, Duke dropped from the hover, and the emulator
stopped on GPU command 0x1F. This reproduced twice. Peak arena use for
comparison:

| Selection | Arena peak per frame |
| --- | --- |
| Current build, slot 10 hover | 41264 (30%) |
| Plus all 30 interior rooms whole-view | 83140 (59%) |
| Every room whole-view | 139616 (full; crash) |

No current player path comes near this. Any wider selection, though, must cap
itself before the arena fills and must not hand its pieces back to the game's
renderer.

## Cost at 180 Hz (player settings: 4x, 16:9, CPU 150%, slow turn)

| Run | Presents/s | Fields/s | Emulation thread |
| --- | --- | --- | --- |
| Current, run 1 / 2 | 142 / 91 | 60.5 / 59.8 | 0.82 / 0.72 |
| Plus all interior rooms, run 1 / 2 | 180 / 180 | 60.0 / 60.1 | 0.83 / 0.82 |

The difference is within run-to-run noise: governor steps decide the presents
rate, and fields held 60 in every run. That is an upper bound; a gated
selection draws fewer of those rooms. D17AW (render thread) remains the
route to more headroom.

## Other outdoor levels (room tables, all 21 levels)

Sky rooms exist in levels 1 (20 of 78 rooms), 5 (11 of 77), 6 (12 of 58),
8 (1 of 64), 10 (10 of 48), 11 (26 of 58), 21 (5 of 10) and 28 (3 of 3).
Non-sky rooms bordering a sky room and rising above its floor (a rough
count of geometry the over-the-walls test cannot offer): level 1 27,
level 5 12, level 6 12, level 10 4, level 11 9, level 21 1. Level 6's towers
do not border a sky room, so the true count per level needs the in-game
test. Each level's authored area also ends at its room bounds, so every
outdoor level has a void beyond them.

## Proposed fixes (for the user to choose)

All are Polymorph only, Modernized with Extended draw distance; Classic is
frozen and Vanilla unchanged. All are game-wide, keyed to room flags and
level data, never to a level or slot.

**A. Over the walls for every room (missing towers and roofs).** From a sky
room, offer every room, not only sky rooms, to the existing over-the-walls
sight test (piece tops seen above the camera room's walls, rectangles from
the pieces seen). Host selection only: the game's own room list, the props
pass and the object pass stay as today. Guard: rooms added this way stop
before the arena passes a set share (for example 75%), farthest first,
and are never passed back to the game's renderer. Risk to check in play:
an interior room whose outer shell is not modelled could show its inside
through a culled roof or wall from above. The gatehouse needs a close look.

**B. Fill the authored void** (choose one or combine):

- **B1. Backdrop to the ground.** Continue the sky model's mountain ring
  below the horizon with a dark ground band and haze in the backdrop's
  colours, so nothing below the mountains is the clear colour. No geometry.
  Every outdoor level at once, at almost no cost. The moat banks then end
  at a distant haze, not at black.
- **B2. Distance haze.** Fade far geometry towards the backdrop's lower
  colour so the island's edges blend into B1 rather than ending in a hard
  line. Small per-vertex cost; needs a choice of strength per level type.
- **B3. Horizon skirt.** A host-drawn ground ring from each level's outer
  room bounds out to the horizon, at the level's outer ground height, using
  a ground texture from the level. Reads as land continuing to the
  mountains. Automatic heights and textures can mismatch (moat banks sit at
  several heights), and it needs per-level checks.
- **B4. Authored terrain.** New meshes per outdoor level (hills, banks,
  far walls) per the D08A28 style guide. Best result, but per-level art for
  8 levels.

Recommendation: A, then B1 with B2 game-wide. B3 or B4 could follow as a
separate job if the haze still reads as a floating island. Acceptance for
whatever is chosen stays as on the board: slot 10 full circle at hover,
slot 2 moat, the other outdoor levels from the jetpack, a 21-level sweep
without lost geometry, and a user playtest.

## Implementation (2026-11-01, user chose A, then B1 and B2) - removed, kept as notes

This first implementation was removed after the playtest (Revision 2 below).
The design and measurements stay here for a future, better-integrated mist
and haze job.


User: "i think B4 is too extreme, at least at this point in time. but other
than that, let's go with your most educated and informed recommendation".
That is A, B1 and B2. B3 stays a possible later job; B4 is not wanted now.
All of this is Polymorph only, Modernized with Extended draw distance and
host visibility. Classic and Vanilla never reach it (Vanilla slot 10 was
checked: original view, no mist, no host pass).

### A. Towers and roofs over the walls (`host_visibility.inc`)

- While the camera room is a sky room, the over-the-walls test offers every
  room, not only sky rooms. A non-sky room it adds is a scenery room.
- Scenery pieces pass a per-piece sight test: a line from the eye to the
  centre or a corner of the piece's top must not pass through the bounds of
  any regularly drawn piece (walls, banks, towers in front). Bounds
  over-cover their geometry, so a little scenery behind a gap can stay
  hidden, but nothing a wall hides is drawn. In the slot 10 vista, 37 of
  52 scenery pieces in view are hidden this way.
- Scenery pieces are drawn after all other pieces. If the frame cannot
  afford one, it is dropped and never handed back to the game's walk, which
  is what overran the game's packet area in the report's all-rooms test.
  Scenery rooms stay out of the props/actors list and the shown-room set,
  because gameplay reads them. A room the game itself lists is never
  scenery.
- Developer: `DNTTK_OVER_WALLS_ROOMS=0` (sky rooms only, as before) and
  `DNTTK_SCENERY_OCCLUSION=0` (no per-piece test). The debug object
  `host_visibility` gains `scenery` (rooms, pieces, hidden, dropped).

### More packet space (framework and `near_clip.cpp`)

The slot 10 vista already exceeded the 350 KB host region per frame before
this job. About 18 far pieces a frame went back to the original renderer,
which draws them dark, and there was no room for scenery. The 1 MiB GPU
DMA aperture is full: host arena 512 KB, Bio Mask 188 KB, animation
overrides 320 KB.

- Framework (`mod_memory.h`, `memory.c`, `gpu.c`, `mod_runtime.cpp`,
  `mod_plugins.h`): a second 1 MiB aperture directly below the first
  (0x00E00000; guest 0x80E00000), allocated with
  `psx_mod_alloc_gpu_dma_extra_memory`. Together the two form one 2 MiB
  window, which the host-vertex direct table now covers.
  - Savestates carry the new aperture as an optional tail of
    `BS_SEC_MODMEM` ("XTRA", used, bytes). Older builds stop reading before
    it. A load restores it when this process allocated the same amount;
    otherwise it fills it with end-of-list headers. The first aperture's
    records and layout are unchanged, so existing saves load as before
    (`modmem=restored`).
  - Render replay images carry it too.
  - The codegen hash is unchanged (0x8bab543c).
- Each near-clip region gains 512 KB from the new aperture after its host and
  Bio Mask blocks (862 KB per frame). No packet straddles a block boundary.
  Developer: `DNTTK_EXTRA_ARENA=0`.
- Two faults were found and fixed while testing:
  - Without saving the new aperture, the frame a savestate resumes (D20H)
    lost its world, weapon and HUD for a few frames after every load.
  - Without it in the replay images, redraw workers recorded garbage after a
    load, and textures were corrupted on the third save/load cycle.
  - With both fixed, three save/turn/load cycles show the full frame at once
    and one second later.

### B1. Mist below the horizon (`near_clip.cpp`)

- The mist colour is a running average of the level's sky texels, read from
  VRAM as the sky is drawn (corners and centre of each sky face, near-black
  texels left out, modulated as the GPU would).
- The band is an eye-centred ring from 4 degrees above the horizon to
  straight down: 55% of the mist colour at the horizon, falling to 3% below.
- It is emitted in the farthest ordering-table slot, painter order, with no
  depth test, so the sky's mountain ring and every surface cover it. It is
  clipped to the view in view space.
- It is drawn while the camera room is a sky room, easing in and out over
  0.5 s. Developer: `DNTTK_MIST=0`.

### B2. Distance haze (GL renderer and `host_visibility.inc`)

- `psx_mod_gpu_set_haze` (framework) sets a colour, amount and distance
  range. The GL shaders recover view distance from host depth
  (`z = 16 / (1 - depth)`) and mix towards the colour.
- It applies only to primitives with host depth (world, props, actors). The
  sky, HUD, weapon and 2D layers are never hazed, and neither are
  semi-transparent texels.
- Range: from the level's original fog start (original mesh limit less
  0x13d0; 20480 in level 6) to the extended limit, at most 55%, in the band's
  horizon colour.
- It is on only while the camera room is a sky room, and off whenever the
  host pass does not run. Developer: `DNTTK_HAZE=0`.

### Verification

- Slot 10 at hover, full turn: the keep tower and gatehouse show above the
  front wall, and the band below the mountains is a dim mist falling to dark
  ground instead of black (`shots/b2`, `sl-new-1b.png`). Hover height is
  steady.
- Slot 10 counters: 153 pieces drawn a frame, none left to the original
  renderer, none dropped. Original-renderer mesh fallbacks went from 1404 to
  0. Peak packets are 477 KB of 862 KB.
- Remaining black notch: the gatehouse opening above the gate. With
  `DNTTK_SCENERY_OCCLUSION=0` a lit gatehouse interior shows there, at
  655 KB a frame and 2 ms more hook time, so the test stays on.
- Slot 2 (moat, on foot): unchanged; the moat walls hide the horizon.
- 21-level sweep, four headings each, new against the same build with every
  D17AX switch off: room and piece counts equal or higher. The largest pixel
  differences were checked by eye and are turn position and weapon bob. No
  lost geometry.
- Other outdoor levels: their starts look normal. Scripted flights only
  reached the automatic lift height (Space does not climb through test
  input), so their jetpack vistas were not seen.
- `ttk-controls-test` (119 PASS), `ttk-near-test` (unified) and
  `ttk-input-test` pass, as does the framework aperture test with three new
  checks. `ttk-near-test` with `DNTTK_UNIFIED_GEOMETRY=0` fails a packet
  contract, and fails the same way with the committed `near_clip.cpp`, so
  it predates this job.
- The exported patch applies to the pinned framework commit and reproduces
  the working tree exactly.
- Player saves and profile were hash-checked unchanged in every run.

### Cost (player settings: 180 Hz, 4x, 16:9, CPU 150%, slot 10 hover)

| Build | Raw fields/s (shedding off) | Presents/s with the governor |
| --- | --- | --- |
| D17AX switches off | 56.8 / 57.1 / 57.0 | 142 / 132 |
| D17AX | 54.4 / 55.1 / 53.7 / 54.5 | 95 / 87 |

The vista already ran below 60 fields/s at 180 Hz. Drawing the whole world
host-side costs about 2 fields/s more (around 4%), so the governor now steps
it to every second refresh (90 presents/s) on this machine. Before, it
stepped in some runs and not others. Game fields held 60 with the governor
in every run, and slot 2 is unchanged (150 presents/s, 60 fields/s). D17AW
(a host render thread) is the route back to 180 here.

Limits: the mist colour adapts over about a second after a level change.
Mist and haze key on the camera room being a sky room, so a covered
courtyard room flagged as sky gets them too. Each new save in a vista is a
little larger (about 90 KB). A build from before D17AX loading a new save
ignores the tail and shows the resumed frame without its far pieces for a
few frames.

## Revision 2 (2026-11-01): flicker fix only

User after the playtest: "the fog doesnt work for me ... visually its not it.
and being able to see the gatehouse too is probably unnecessary ... all i
really wanted was for a couple of those draw distance issues i could see in
the previous iteration to not flicker". Then: "let's keep the high KB per
frame, and if anything go a lot higher". Mist and haze will come back later,
"more elegantly".

### Removed

All of A, B1 and B2 above is removed from the code:
- the non-sky rooms in the over-the-walls test, and the per-piece occlusion
  test;
- the mist band, the sky colour sampling, and the outdoor step in the host
  pass;
- the framework haze API and its shader terms.

The host pass is back to its committed selection. Two developer aids remain:
`DNTTK_HOST_VIS_ALL` (its rooms drawn last and dropped rather than handed to
the game's walk, and kept out of the props list), and the debug room `list`
with `developer_dropped`.

### The flicker and its cause

In the slot 10 vista the 350 KB host region overflowed in about half the
frames. The distant pieces that did not fit (about 18 a frame) went back to
the original renderer, which left them as black holes. Which pieces fit
changed with the view, so while turning a far wall section came and went.

- Shot evidence, old memory size (`DNTTK_EXTRA_ARENA=0`): a lit section of
  the far castle wall is missing in one frame and present in the next three
  (`tn4.png`, left).
- A still view is stable in both versions; its old/new difference is that
  same section, black in the old one (`oldnew-crop.png`).
- With the larger region the section is present in every frame, and the host
  pass leaves nothing to the original renderer (`left 0` against 1501 over
  about 100 frames).

### What ships

**Second aperture (framework).** 2 MiB of GPU DMA memory at 0x00D00000
(guest 0x80D00000), directly below the first aperture. The two form one
3 MiB window for the host-vertex direct table.
- A 3 MiB aperture at 0x00C00000 stopped the BIOS at boot (unknown dispatch
  to 0x0000C000, ra 0xBFC06CF4, twice): the BIOS reads that range during
  boot. 0x00D00000 boots, and 0x00E00000 had booted earlier.

**Bigger frame regions (`near_clip.cpp`).** Each frame region gains 1 MiB,
1.35 MB in all: about 3.9 times the old region and 2.8 times the vista's
measured peak (about 480 KB).
- The two regions take alternate 64 KB chunks, so what a frame uses sits at
  the start of the aperture.
- No packet crosses a chunk or block boundary.

**Copies only what was written (framework).** The framework tracks the
highest byte ever written in the aperture; replay images and the savestate
tail copy only up to that point. In slot 10 that is a few tens of KB, not
2 MB.

**Savestates.**
- The tail is "XTR2": allocated size, written size, then the bytes.
- Saves without a tail (all saves before this) never link into the
  aperture, so nothing is touched.
- A tail from another layout, such as the first D17AX build's "XTRA", fills
  the aperture with end-of-list headers.
- The first aperture's layout and the codegen hash (0x8bab543c) are
  unchanged.

### Verification

- Slot 10: three save/turn/load cycles are clean right after each load and
  one second later (`modmem=restored`). Save files are back to normal size
  (about 2.42 MB).
  - The weapon is missing in the very first shot after the first load. The
    same happens with `DNTTK_EXTRA_ARENA=0`, so it is post-load timing, not
    this change.
- 21-level sweep, new against `DNTTK_EXTRA_ARENA=0`:
  - Heading 0 is identical in every level.
  - Later headings differ only by turn position; the largest were checked by
    eye.
- Tests:
  - `ttk-controls-test` (119 PASS), `ttk-near-test` and `ttk-input-test`
    pass.
  - The framework aperture header test passes (10 checks).
  - The exported patch applies to the pinned commit and reproduces the tree.
- Player saves and profile were hash-checked unchanged in every run.

### Cost (180 Hz, 4x, 16:9, CPU 150%, slot 10 hover, shedding off, three runs)

| Build | Fields/s |
| --- | --- |
| Old region (`DNTTK_EXTRA_ARENA=0`) | 56.7 / 57.3 / 55.2 |
| Larger region | 55.2 / 56.7 / 54.6 |

That is about 1 field/s, within run-to-run spread. It is the cost of drawing
the distant pieces that used to be dropped. The vista still needs D17AW to
hold 180 on this machine.

Limits:
- The other outdoor levels' jetpack vistas were not flown.
- The authored void (past the level's edges, below the mountains) stays
  black, as before D17AX.
