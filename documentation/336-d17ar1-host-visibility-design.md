# D17AR1 - Host-owned world visibility: findings and design (report first)

2026-10-30. Report for approval; no implementation. The user chose option C
after the D17AQ moat playtest: "I want option C so badly, that's truly exactly
what i want."

## Why owning the renderer did not remove pop-in

Since D17AM every world and prop polygon the game submits is drawn by the
host: precision, depth, near clipping, widescreen. But the game still decides
which polygons to submit. Each frame the camera render `0x8002e48c` runs:

1. the portal walk `0x8006276c`: from the camera room, list each room seen
   through a portal with the screen rectangle of that opening (list
   `0x800d68b0`, at most 32 entries);
2. the room walk `0x80062b48`: for each listed room, set the room transform
   (`0x80029660`), then test each mesh piece's bounds against the room's
   rectangle and the far limit (`0x800365e8`) and draw the survivors
   (`0x80011020`);
3. the props/actors pass `0x8002ffec` and the object pass `0x800374cc`, over
   the same list.

A room whose opening leaves the screen is not listed, even when parts of it
are still visible over a wall: that is the moat's edge pop-in, the slot-10
jetpack pop-in and the portal cut-offs. D17P, D17Y and D17AQ (open sky)
patched the game's inputs case by case. Option C replaces steps 1-2 for world
geometry with a host decision. CPU speed only gives the emulated CPU more
cycles; it changes nothing about what is culled.

## Verified on the live game (slot 2, level 6)

- Room table `*0x800da454`, count `*0x800da47c`, 88-byte records: +0 min
  corner (3 words), +0xc flags (bit 3 sky; bit 1 flat room colour, with the
  colour at +0x54), +0x20 max corner, +0x30 portal count, +0x38 portals,
  +0x32 mesh piece count, +0x40 piece pointer array, +0x48 object list,
  +0x4c targetable list.
- Piece = world mesh: +4 int16 bounds relative to the room corner (min +0,
  max +8), +0x14 vertex count, +0x18 polygon list offset, +0x1c vertices
  (8 bytes: x, y, z, colour-index byte; then an RGBC word). Polygon groups
  0x60 GT3 / 0x61 GT4, 8-byte records, 0xff ends.
- Room transform: the GTE rotation is the camera view matrix (camera +0x20,
  row 0 scaled for the aspect) and the translation is R * (room min - eye).
  Measured on 134 traced meshes in 7 rooms: maximum error 2.8 units
  (integer rounding and the halving used for large offsets).
- Vertex decode, from the traced view-space vertices solved back through the
  live GTE state: room-local x = byte0 * 1024, y = byte1 * 128,
  z = byte2 * 1024. So every world vertex is
  **room min + (1024 * b0, 128 * b1, 1024 * b2)**: the whole level's world
  geometry can be built once in world space from the game's own data.
  (Level 6: 58 rooms, 301 pieces, 17325 vertices in the 51 rooms checked.)
- The 120 Hz redraws (D17 render replay) re-run the game's own frame
  composition in worker processes on a frozen machine copy, through the
  same hooks; host visibility would run in every redraw with that redraw's
  camera.

## Design

**Host world pass.** At level load (and after a savestate load or overlay
change), read every room's pieces and build a host world cache: per piece,
world-space bounds and a reference to the guest mesh. Each frame, at the room
walk:

1. Select pieces: in front of the real camera, inside a guard-banded view
   frustum (wider than the screen, so late-camera turning never reaches an
   undrawn edge), inside the room far limit, and in a room reachable from the
   camera by the level's own room graph or seen over its walls (portals and
   sky rooms, so indoor rooms behind solid walls stay out, as the game
   intends).
2. Draw each selected piece through the existing D17AM emission
   (`near_clip.cpp` `world()`): the same colours, colour-index animation,
   flat room colour, fog, texture and animated-texture lookup, translucency
   and depth, driven by a synthesized GTE state (R = camera matrix,
   TR = R * (room min - eye), H) instead of the room walk's. Nothing about how
   a polygon looks changes; only which pieces are drawn.
3. The game's room walk then draws nothing (it is given an empty list for
   world geometry), so its guest cost disappears; that is what slowed
   slot 10 (23.6 to 28.1 game frames/s after D17AQ revision 3).

**What stays with the game.** All simulation, and everything that moves or
is an object: Duke, enemies (including D17AQ dormancy), props, doors,
pickups, effects, blood, sprites, the sky, the first-person layer, the Bio
Mask. The props/actors and object passes get the list of rooms the host
selected (whole view rectangle), so props and enemies in every visible room
are drawn and nothing pops at room boundaries. The sky request (room flag
bit 8) follows the same list.

**Guard band and late camera.** Selection uses a frustum wider than the
displayed one by the turn the late camera can add within a game frame (about
one game frame of the fastest look), so turning never exposes an undrawn
edge. Host depth and the GPU clip the rest.

**Vanilla and Original.** Unchanged: the host pass runs only with Modernized
Extended draw distance and the unified path; `DNTTK_HOST_VISIBILITY=0`
(developer) restores the game's walk.

## What D17AR2 must prove first (stop if any fails)

1. The emission reproduces the room walk's per-room state for a piece drawn
   outside the walk: context flags (flat room colour `0x800b4d3c` from
   +0x54, flag 0x80 rectangle test off), light and colour tables, fog start,
   texture and animation tables. Test: a host-drawn piece must match the
   walk-drawn piece pixel for pixel in a matched view, in every level.
2. Pieces that move or change: animated or scripted world pieces (doors,
   lifts, breakable walls, water levels, secret walls) if any are world
   meshes rather than objects. Inventory all 21 levels' rooms for pieces whose
   data changes between frames; those stay on the game's walk.
3. Mirrors, reflections and any second camera render (the composition runs
   other renders through the same walk; only the main view is replaced).
4. Level overlays and the 32-entry list limit for the props pass.
5. Cost: host emission of every selected piece per present at 4x and 120 Hz
   in the heaviest views (slot 2 moat, slot 10 jetpack, the town and club
   levels) against the current numbers; the guest saving from skipping the
   walk.

## Stages

- **D17AR2** (after approval): host world cache and pass behind
  `DNTTK_HOST_VISIBILITY=1`, the five proofs above, matched-view pixel
  comparisons against the current path in every level.
- **D17AR3**: regression sweep (all private fixtures and level-select
  levels; static, walk, turn and jetpack routes; FOV 60/90/120; 4:3 and 16:9;
  60 and 120 Hz), performance, then the Modernized default.
- **D17AR4**: retire what host visibility supersedes (D17AQ open sky, D17P
  and D17Y portal widening).

## Risks

- A per-room state the room walk sets that the emission does not yet know
  about (the proofs above are designed to catch it before any default
  change).
- Rooms the designers relied on the portal walk to hide (closets, rooms
  behind one-way walls, areas outside the playable space). Selection by the
  room graph and sight over walls keeps the game's intent; anything authored
  to be seen only through a portal stays clipped to it.
- Host draw cost grows with what is visible; the guest saving offsets it.
