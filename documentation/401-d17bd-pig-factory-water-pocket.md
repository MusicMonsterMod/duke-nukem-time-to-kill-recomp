# D17BD - PIG FACTORY: the pocket beside the pillar has no water surface

2026-11-06. Status: **Done, user-accepted** ("great work. you can close it out now"). Option 2, Modernized only, all four surfaces.

User, after the report: "i can confirm that the surface is indeed treated as
water", then chose option 2 and asked for before/after shots for the future
README.

User clip `research/screencaps/Video_2026-11-06_10-44-51.mp4` (from UI save
slot 10, PIG FACTORY): from about 16 s, Duke stands on the far ledge and looks
back at the pool. The square pocket between that ledge and the brick pillar,
right under the crosshair, shows the dark pool walls and floor with no water
surface, while the pool beside it has the animated water.

## Method

Private copies of the player's cards and settings, offscreen, player files
hash-checked by every run. Harness `recomp/analysis/d17bd/probe.py` (from the
D44C probe), plus helpers:

- `ov.py`: traced emits over the matching shot (native x maps 964 to the
  shot width; y is 240 lines about the centre).
- `world.py`: traced water corners back to room-local space through the
  inverse room matrix.
- `gap.py`, `animgrid.py`: per-pixel max-min over 10 shots, which marks the
  animated water texture.

Case `tele` loads slot 10, moves Duke 2300 units along his facing (world
38952, 26268, room 62), turns him round and looks down. That gives the clip's
view of the pocket.

## Not a renderer regression

At the pocket view the surface is missing the same way in the default build,
with `DNTTK_NEAR_CLIP=0` (the game's own GPU packets) and with
`DNTTK_WATER_CLIP=0` (`tele3.png`). Default run counters: every water tile
the game builds is taken (`seen` = `taken`), with no guard, grid, dropped or
budget fallbacks.

## The water grid stops at the pocket

Traced water corners in room-local space (both views): x 2048-6144, z
1024-6144, surface y 3584. At the pocket view the eye is at (2088, 2651,
2721) looking down and towards -x. The view meets the surface at about x
1660, z 3040, outside the grid's first column. The near ledge in the saved
view has its lip on the same x = 2048 line.

In level RAM there is a run of 24-byte boxes (`0x80191760` onwards, field
order: two words, then x0, y0, z0, a field, x1, y1, z1). The water renderer's
region pointer is the box's x0 (it reads x at +0, y at +2 and the end x at
+8, as `water_clip.inc` does). The relevant boxes:

| Box | x | y | z | field |
| --- | --- | --- | --- | --- |
| `0x80191760` | 2048-6144 | 3584-5632 | 1024-6144 | 62 |
| `0x80191778` | 6144-7168 | 3584-5632 | 2048-4096 | 62 |
| `0x80191790` | 1024-2048 | 3584-6656 | 2048-4096 | 0 |
| `0x801917d8` | 1024-2048 | 3584-3968 | 5120-6144 | 62 |

The first box is exactly the rendered grid. `0x80191790` is the pocket: the
same surface height and as deep as the pool, but its field is 0 where every
box drawn as water in this room has 62. So the level data defines the pocket
as part of the volume, but not as a rendered water surface.

Conclusion: an authored gap, the same class as D17AS (stray moat face) and
D17AO (slot-11 ceiling hole). The original camera sits behind and above Duke
and rarely looks straight down into the pocket. The independent first-person
camera stands on the far ledge and does.

## Other findings at this spot

- With the default build, the saved view's pool reaches the near ledge's lip
  and the floor covers it correctly. With the game's own packets, the near
  stretched (saturated) tile bleeds a band of water over the ledge. That is
  the native artifact, not a missing surface.
- The channel behind the corner (turn left from the save) has very dark
  water, with a hard straight seam where the lit water starts. That is the
  game's per-tile flat colour, and the native packets show it the same way.
- The dark patch at the pillar's foot in the saved view does have water
  (88% of its pixels animate). The floor under it is dark.

## Options

1. Leave it as authored (as D17AS). No code change.
2. Level patch, Modernized only: draw a water surface over the pocket's box
   top (one 1024 x 2048 patch at y 3584, room 62), using the neighbouring
   tile's texture, colour and animation, at the water's depth (like the
   D17AO1 tunnel sky patch). Small and contained. It would be identified by
   level tag, room and box values, not by address.
3. A game-wide rule: give a surface to any field-0 box whose top matches an
   adjoining rendered water box. Not recommended without a survey of every
   level. Field 0 may mean a dry pit or a volume that is not water, and the
   rule could flood places that are meant to be dry.

## Decision and implementation (option 2)

The user confirmed in play that Duke swims in the pocket: the box is water,
only its surface is missing.

How the surface passes choose boxes (resident code, matched by digest):
each visible room with water (room +0xc bit 0) lists its water boxes at
room +0x3c (count +0x31, 24 bytes each). Two loops, `0x80030090` (the
surface pass, return `0x800300c4`) and `0x80037a48` (a second pass that adds a
few pixels on nearby walls, return `0x80037a88`), call the builder
`0x8002ef90(box, view)` for each box whose first halfword has **bit 0
clear**. The builder reads only the box's material index (+4, checked against
the table at `0x800c597c`), x0/y0/z0 and x1/z1 at +8; it never reads the 62/0
field. Room 62's list: the pool (`0x8019175c`, bit 0 clear) and six boxes
with bit 0 set, including the pocket (`0x8019178c`, third in the list).

Room 62's shaft piece (`0x801bfd8c`, room-local; vertex y is byte x 128)
has walls at x 1024 and 2048 and at z 2048 and 4096, from the rim at y 3456
down to 6656, and floor at y 3456 around it. So the pocket box is exactly the
shaft opening, and the water at 3584 sits 128 below the rim, as in the main
pool. (The report's "635 below Duke's feet" was wrong: Duke's position is
his centre, about 510 above the floor.)

`recomp/src/ttk/water_pocket.cpp`, Modernized only, entry hook on
`0x8002ef90` (added to `game.local.toml`'s `mod_function_entry_funcs`; one
generated line, codegen hash `0xebce51ea` unchanged, saves still load):

1. When the builder starts on room 62's first box in PIG FACTORY (level tag
   `0xc`), called from either loop, and the list matches (pool box values,
   second box bit 0 set, third box the pocket's values with bit 0 set and
   the same material), it clears the pocket's bit 0.
2. The same loop then builds the pocket. At the builder's next entry (the
   pocket itself) the bit is set again. Nothing else in the frame sees the
   change. If a savestate load came in between, the restore is skipped
   (the loaded RAM is authored).
3. The pocket's tiles take the ordinary water path (D17AF clip, D17AN depth).

Guards: the builder and both loops by SHA-256, both return addresses, the
level tag, the room table and the box values. `DNTTK_WATER_POCKET=0`
(developer) keeps the authored data. No framework edit.

## Verification

| Check | Result |
| --- | --- |
| Pocket view (far ledge, looking down) | 99% of the pocket's pixels animate with the patch, 47% without it (only the wall light) |
| Matched before/after, first person (yaw 0.15 and 0.40, steered to the same view) | dry shaft before, water across the opening after; the floor beyond the rim stays dry |
| Third person (Modernized) | water across the shaft beside Duke |
| Pocket flag in RAM, 60 samples while playing, and after a reload | always 1 (authored) |
| Vanilla | flag always 1, no change |
| Water counters | every built tile still taken; no guard, grid, dropped or budget fallbacks |

Executable `f850e6e356ca7a38434b20d235d6ad1bcab2e6861faf83f31ca05fc1b3b876ac`.
`ttk-controls-test` does not link this plugin; not run.

## Second pocket (opposite the pillar)

User, after accepting the first pocket in play ("it's working!"): "the one
on the other side needs doing". The new save is in UI slot 10 (runtime slot 9,
saved 12:49). UI slot 7 still holds the 09:39 MOLOCH save. Duke stands at
room-local (7150, 4749). Room 62's second box `0x80191774` (x 6144-7168, y
3584-5632, z 2048-4096, bit 0 set, same material) is the second shaft. Piece
`0x801c04cc` has walls at x 6144 / 7168 and z 2048 / 4096 from the rim at
y 3456 to its floor at 5632, open to the pool on its x 6144 side above 4608.
Again the box is exactly the opening. Room 62's other bit-0 boxes are deeper
or beside it and were not touched.

The plugin now takes a table of pockets (list index 1 and 2, each matched by
its values, bit 0 and material). When the builder starts on the pool's box,
both bits are cleared. Each is set again when the builder starts on that box,
and any other box restores both.

Save safety: a staged savestate can be serviced at interrupt checks inside
compiled code, so the window is now closed to saves and loads with
`psx_mod_host_call_begin/end` (the API used by D19J and weapon aim) from the
clear to the last restore. A second entry hook on the per-frame `0x8001fba0`
(already a hook point; the builder never calls it) restores anything left
over and ends the hold. The debug port reads RAM from another thread, so 2
of 400 mid-frame reads saw a cleared bit (before the hold, 1 of 120). That is
expected, not a save. The debug port refuses save requests in these runs
(`LLE run cannot load states`), so no save inside the window was attempted;
the guarantee rests on the API's contract.

| Check (executable `62f98ace...`) | Result |
| --- | --- |
| New save as left, the shaft in front of Duke | 43% of its pixels animate with the patch, 12% without |
| First pocket, matched view | water across the opening, as before |
| Vanilla | both flags always 1 |

Executable `62f98ace28d40d3fb9ebaebcae170ee8425353967abcc6aebde307542098d506`.

## Beach corners (third and fourth patches)

User: "it's working!" (the second shaft), then the next piece from the same
slot, and "the final piece is also the opposite to this on the other side of
the room". New UI slot 10 save: Duke at room-local (1532, 4735), yaw 0.35,
facing +z. Room 62's boxes 5 and 6, `0x801917d4` (x 1024-2048) and
`0x801917ec` (x 6144-7168), both z 5120-6144, y 3584-3968 (384 deep), bit 0
set, same material. Geometry (pieces `0x801bfd8c` / `0x801c04cc`, and the
wall piece `0x801c088c`): the room's far edge is a diagonal wall across each
cell (from y 2432 to the floor), and inside it the floor is the pool's
sloping beach, y = 3456 + 384 * (z - 5120) / 1024. That puts it under the
surface (3584) for z above about 5461. The main pool already covers the
beach between them (x 2048-6144). So each box is the beach's triangular
end. The builder draws a whole square tile. Beyond the diagonal it is behind
the wall, and on the beach above 3584 it is under the floor, and depth hides
both. The only visible change is the wet triangle against the wall.

Both added to the plugin's table (index 5 and 6). The clear/restore order is
1, 2, 5, 6 in the same loop. Boxes 3 and 4 (deeper volumes) stay skipped.

| Check (executable `2ed37ff6...`, rebuilt after a comment-only edit) | Result |
| --- | --- |
| New save as left (pillar-side corner) | the pool's surface now runs into the corner beside the wall; no water through the wall |
| Mirror spot (6660, 4735), yaw -0.35 | the same triangle fills on the opposite corner |
| Flags of all four boxes, Modernized and Vanilla | as authored (1 of 240 asynchronous mid-frame reads in Modernized saw a cleared bit, as before) |

Executable `2ed37ff6b4eebd76c7286200cd3cb87980b6b4c4d6f4684835fdb16f4d6f08c3`.

## Before and after (README material)

Kept local (retail imagery, never committed):
`research/before-after/d17bd-pig-factory-pocket/`. It has `side-by-side-ledge.png`,
`side-by-side-down.png`, the matching single frames (`ledge-*`, `down-*`),
`third-person-*`, for the second pocket `opposite-*`, and for the beach
corners `beach-pillar-side-*` and `beach-opposite-*`. Regenerate with `recomp/analysis/d17bd/probe.py`,
case `show` (`PLAYER=1 SLOT=9 TY=0.15 TP=0.7`, with `XENV=DNTTK_WATER_POCKET=0` for
"before").

## Limits

- The field's meaning is inferred from this room only, not decoded in the
  renderer or the physics.
- The user confirmed the pocket is water in play; my `write_ram` teleport
  could not show it (floor support is not rebuilt).
- The other bit-0 boxes in room 62 (two at the surface height beside the
  pool, deeper ones below) were left alone and not checked against the
  room's floor. No other level was surveyed for similar boxes.
- Offscreen captures at the player's settings; the user's own display and
  180 Hz look remain.
- The original packets in this setup are the recomp's GPU path with the
  Modernized camera, not real hardware or the Vanilla camera, which does not
  frame this view on the same inputs.
