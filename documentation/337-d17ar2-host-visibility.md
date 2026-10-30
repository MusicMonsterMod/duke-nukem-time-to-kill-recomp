# D17AR2 - Host-owned world visibility behind a developer switch

2026-10-30. Implements the approved D17AR1 design
([note 336](336-d17ar1-host-visibility-design.md)) behind
`DNTTK_HOST_VISIBILITY=1` (developer; default off). Status: Needs playtest
(revision 2, below). The default switch is D17AR3.

Code: `recomp/src/ttk/host_visibility.inc` (selection and the room-walk
replacement, included from `draw_distance.inc`), `recomp/src/ttk/near_clip.cpp`
(`near_clip_host_world_live`, `near_clip_host_world_piece`,
`near_clip_host_walk`, the walk filter). Evidence is local under
`recomp/analysis/d17ar2/` (`ab.py`, `sweep.py`, `perf.py`, `portals.py`,
`bfs.py`; shots and JSON). Every run used private copies of the D17AQ fixture
card and savestates, offscreen video, dummy audio and private debug ports, and
hash-checked the player's saves and profile before and after.

## What runs

Only in Modernized, with Extended draw distance, unified geometry and the near
path live. Vanilla and Original never reach it. At the room walk's entry
(`0x80062b48`, ra `0x8002e688`), after the draw-distance limits:

1. **Rooms.** A host portal walk from the camera room over the level's own
   room graph, in floating point, starting from the view rectangle widened by
   15% of the half view on each side (`DNTTK_HOST_VIS_GUARD`). Portals the game
   treats as closed (flag bit 1) are skipped too. From a sky room, sky rooms
   seen over its walls are added with the D17AQ open-sky test, guard-banded.
   Every room the game's portal walk listed is kept as well, its rectangle the
   union of both, so the host never draws less than the game.
2. **Pieces.** Each piece of a selected room whose bounds project into that
   room's rectangle, or reach behind the eye without lying wholly outside one
   side of its frustum.
3. **Drawing.** Each piece goes through the D17AM emission exactly as the room
   walk's call would: GTE rotation from the camera matrix, translation computed
   as `0x80029660`/ApplyMatrixLV does (hi/lo split, halving of offsets of 32768
   or more), context `+0x50 = 0x300` with bit 1 and `+0x54` for flat-coloured
   rooms (or the global flat colour `0x800c0bb8`), the rectangle test off, and
   the walk's own preamble values (`+0x10`, `+0x14`, animation table `+0x60`).
4. **The game's walk** then gets only the rooms of pieces the frame budget sent
   back (see Cost), with the whole view rectangle, and the near-clip hook
   empties every other call it makes (null mesh). The props/actors pass gets
   the host room list (game-listed rooms first, at most 31). The object pass
   gets the portal walk's own list back, since its handlers affect play
   (D17AQ). The portal walk itself and its side effects are untouched.
5. D17P portal widening and D17AQ open sky are skipped while the host pass owns
   the frame (D17AR4 retires them).

Developer console: `hostvis on|off|default` overrides the switch for the
session (main process only; with 60 Hz presentation there are no redraw
workers, which is how the matched views below were taken). Counters are in
`ttk_input` under `controls.draw_distance.host_visibility`.

## Found during the work

- **Portal corners are a 2x2 grid** (like a PS1 quad: 0, 1, 2, 3 = top-left,
  top-right, bottom-left, bottom-right), not an outline. Walked as 0-1-2-3, the
  near-plane clip saw a bowtie and narrowed rectangles, so the host walk alone
  missed a room the game lists (level 21, room 6 seen through 2-3-0: floor and
  a barrel disappeared). Fixed with the outline 0, 1, 3, 2. Afterwards the host
  walk reaches every room the game lists in 83 of 84 sweep views (one level-2
  view still relies on the union rule).
- The view rectangle `0x800d2210` is centred (x from -341 at 16:9). The
  camera's `+0x42` is not the projection in use; the host takes the GTE H
  register at the walk.
- Level 28: the existing draw-distance code refuses its limits on every frame
  (refusals rise each frame), so the host pass, like D17P/D17AQ, never engages
  there and the game's walk runs as before. To check in D17AR3.

## The five proofs

1. **Per-room state.** Matched views, one process, host off/on/off within
   about 0.6 s (`ab.py`, `sweep.py`). Slot 2 (level 6, six views): host-vs-game
   difference 0.4-1.3% of pixels against off/off animation noise of 1.1-4.6%.
   All 21 selectable levels, four views each (84): 81 within noise; the three
   above it are an explosion going off during the host shot (level 9), and
   geometry the portal walk had clipped now drawn (level 6 view 1, level 26
   view 1: a far opening filled). The level-21 regression above was found and
   fixed by this sweep. Slot 10 (jetpack height, level 6): the host view fills
   the right-hand void with the moat bank, completes the battlements and the
   castle wall the game cut along a vertical line, which is the pop-in this job
   is about.
2. **Moving or changing pieces.** In every selectable level, two RAM images
   4 s apart: no world piece's header, vertices (colour-index nibble masked, as
   the renderer clears it) or polygon list changed, and no portal is flagged
   closed at load. Doors and lifts are objects. The host reads the live data
   every frame anyway, so a piece that did change would be drawn as changed.
3. **Second camera renders.** On the whole disc image, the room walk and the
   camera render each have exactly one caller (the composition). All host
   logic is keyed to the walk's call from the camera render (ra `0x8002e688`)
   and the mesh calls from the walk (ra `0x80062db8`); other world-renderer
   callers (`0x80037344`, `0x80037c54`) are untouched.
4. **Overlays and the list limit.** No refusals in any level (code guards on
   `0x80029660`, `0x800b444c`, `0x8006276c` and the walk). The props list never
   exceeded 15 rooms (cap 31). Room table and pieces are read live each frame,
   so level overlays need no cache invalidation.
5. **Cost** (120 Hz, FOV 90, slow turn, two runs each, `d17aq/perf.py`):

   | Fixture | Game frames/s off | on | Main thread off | on |
   | --- | --- | --- | --- | --- |
   | Slot 10 (jetpack) | 28.0 / 27.9 | 26.5 / 26.7 | 0.776 / 0.772 | 0.789 / 0.798 |
   | Slot 2 (moat) | 28.5 / 28.4 | 27.3 / 27.2 | 0.773 / 0.779 | 0.814 / 0.779 |

   Host visibility costs about 4-5% of game frames per second in these views.
   Selection is about 25 us a frame; drawing is about 2.6 ms a frame (about
   65 us a piece), and it draws more pieces than the game would (that is the
   point). The guest saving the design expected is small: since D17AQ the game's
   renderer is already handed a null mesh for pieces the host draws. Distant
   pieces the far reserve sends back (D17AM3 rule, about 7 a frame at slot 10;
   the frame holds about 80 KB of host packets when they arrive) are still
   drawn by the game's walk with scene depth, as today. No present misses; the
   presented-frame p95 at slot 10 rose from about 9 to about 18 ms in the
   turn/walk route (`d17ar2/perf.py`). Redraw-worker frames at 120 Hz drew
   correctly.

## Limitations and open items for D17AR3

- Performance: the cost above must be recovered or accepted before the
  default switch (candidates: cheaper emission of far pieces, a tighter guard,
  skipping pieces fully behind nearer walls of the camera room).
- Level 28 refusal (above).
- The context ends the walk at `+0x50 = 0x300` (count 0), where the game's
  walk left the last room's bits; no difference was seen in the props pass.
- A savestate taken between the room walk and the object pass carries the
  host's list; after loading, the object pass sees it once (as with D17AQ).
- Not yet covered: FOV 60/120, 4:3, third person, jetpack/walk routes in every
  level, Bio Mask and night vision (all D17AR3).

## User review 2026-10-30: far-end black holes remain

User: "when you look down the end at the far side of the moat and you still see
those massive black holes at the end when you look left and right ... there's
still a lot of stuff popping in and out of existence in slot 10". The session
log does not record whether the switch was on (neither the environment value
nor the console reply is logged).

Cause of the slot-2 far-end holes (confirmed offscreen, fixture slot 2 view
looking down the moat): not visibility. The host selects those far pieces,
but the D17AM3 far reserve (`DNTTK_UNIFIED_FAR_RESERVE_KB`, 48 KB) sends every
mesh whose nearest vertex is 4096+ away back to the original renderer once
the frame holds about 80 KB of host packets, and the original renderer draws
them black at that range. With the reserve at 0 the same view shows the far
moat wall textured; the developer tint shows the host drawing it. The default
path has the same holes, which is why host visibility looked no different.

Slot 10: in a 24-step turn the black share of the frame is identical with and
without the reserve (most of it is the area beside the castle with no authored
geometry, as D17AQ found). The remaining pop-in there is not yet identified.

## Revision 2 (2026-10-30): distant pieces drawn by the host, larger frame budget

User: "yes, i want to have a professional rendition of this working properly!"

Live session (user playing slots 10 and 2 with `hostvis on`, counters read
over the debug port): 24% of host-selected pieces over the session, about a
third looking down the moat and 37% at jetpack height, went back to the
original renderer and were drawn black.

Changes (`near_clip.cpp`, `host_visibility.inc`):

- **Nearest first.** The host pass collects every selected piece and draws
  them by distance from the eye, so any shortfall falls on the farthest ones.
- **Reserves for a host frame.** Each host world piece leaves room only for
  what follows the room walk: the props and actors, sized from what they drew
  last frame plus what refused meshes would have needed (a quad a face), plus
  a quarter, two meshes' takeover allowance and 4 KB, between 24 and 64 KB
  (`after_walk_reserve`). Sized from what they drew alone, the reserve shrank
  once they were squeezed and every prop at slot 10 fell back. In such a frame
  props and actors no longer leave the 48 KB far reserve. A distant host piece
  (every vertex 4096+ away) is costed at its packets (one quad a face, half
  again for edge splits, 512 bytes) instead of the 6 KB near-clip allowance.
  The sky (32 KB) and first-person (8 KB) reserves are unchanged; a dynamic sky
  reserve was tried and dropped because the sky then fell back.
- **Rectangle test on** (context flag 0x80) for host pieces against the host
  room rectangle: faces wholly off one edge cost no packets (slot 2 world peak
  108 to 93 KB).
- **Frame budget: two 256 KB regions** instead of four of 128 KB, in the same
  512 KB allocation, each with two of the four Bio Mask blocks: about 350 KB a
  frame instead of 175 KB. The GPU walks a frame's list serially after the
  frame is built, so a region is rewritten only while the frame after next is
  built. This applies to every Modernized frame, host visibility or not. The
  1 MB GPU DMA aperture is full (host arena, Bio Mask blocks, 320 KB animation
  overrides; about 3.6 KB free) and cannot grow: 24-bit list links end at
  0xFFFFFF. Allocation sizes and order are unchanged, so savestates keep their
  layout. The D20C mirror word now holds the region plus 4: values 0-3 are
  states saved with four regions, 4-5 with two; after a load the next region is
  one the loaded frame's packets do not occupy, in either layout
  (`region_after`). Native test updated.
- `[TTK hostvis] host world visibility on|off` is written to the session log
  when the switch changes.

Results (fixture slots, 60 Hz unless noted):

| | Pieces handed back | Prop/actor fallbacks | Downgrades |
| --- | --- | --- | --- |
| Slot 10, before | 33-37% | 0 | 0 |
| Slot 10, revision 2 | 0 of 19384 | 0 | 0 |
| Slot 2, revision 2 | 0 of 13161 | 0 | 0 (4138 with host visibility off) |

- Slot 2, looking down the moat: the far-end black hole is filled with the
  textured far wall (`far-3rcmp.png`); with host visibility off it remains.
- Sweep, all 21 selectable levels, 84 matched views: 82 within noise, 2
  gameplay events (an explosion, an enemy shot); no pieces handed back; no
  piece data changes.
- Bio Mask with host visibility: drawn (10980 mask triangles, no fallbacks).
  A savestate saved and loaded in the new layout, and an old-layout state
  (slot 10), resume without corrupt frames.
- Cost at 120 Hz, FOV 90, slow turn, two runs each:

  | Fixture | Game frames/s, off | on | Before this job |
  | --- | --- | --- | --- |
  | Slot 10 | 29.2 / 29.5 | 28.4 / 28.3 | 28.0 |
  | Slot 2 | 29.1 / 29.3 | 29.1 / 29.4 | 28.5 |

  The larger frame budget made the default path faster as well (fewer
  downgraded meshes). With host visibility, slot 10 costs about 3.4% for
  drawing all the distant geometry; slot 2 costs nothing measurable.

Still open (D17AR3): level 28 never engages (draw-distance refusal); the black
areas left at slot 10 are where the level has no geometry (D17AQ: Vanilla
shows the same); FOV 60/120, 4:3, third person and full routes.

## Revision 3 (2026-10-30): the switch reaches the redraw workers

User playtest of revision 2: "the far end of the moat as viewed from slot 2
was blank at the peripherals. and the panning left and right in slot 10 was
the same ... i couldnt tell that anything has even been done."

Cause: the player runs at `auto` frame rate (up to 120 Hz), where most
presented frames are redraws by worker processes forked at startup. The
session was launched without `DNTTK_HOST_VISIBILITY` (absent from the game's
environment; `run.py` copies the environment unchanged), and `hostvis on`
changed only the main process, so the workers kept the original walk. The
live counters read only the main process. All earlier matched views were taken
at 60 Hz (no workers); the 120 Hz cost runs set the environment variable, which
workers inherit.

Fix: the live frame's switch is carried to the workers in `RenderState`, the
draw-time state each redraw job loads (as the session FOV already is).

Check at 120 Hz, console toggle only, same process: slot 2 looking down the
moat, host visibility off shows the far-end black hole and on fills it
(`ab120-s1.png`); slot 10, off cuts the castle wall along a vertical line as
the view turns and on continues it (`ab120-s9.png`). The large black area at
slot 10 is where no geometry is authored and is the same either way. Native
tests pass.

## Playtest result (2026-10-30)

User, revision 3 with the switch reaching every process: "Visually,
fantastic. performance-wise, it needs a lot of work to make this silky
smooth! Visually you've got it though, nothing is popping in and out of
existence anymore, so this is certainly the right direction." Live counters
over the session: about 312000 host pieces, none handed back, no fallbacks;
most minutes 75-90 presents/s, with brief drops behind real time that also
occurred with the switch off (main emulation thread at 100% of one core, no
other load). D17AR2 is accepted on visuals; smoothness is D17AR3's first
item.
