# D08A47 - The Holy Hand Grenade held and thrown looks like its pickup

Status: **Done, user-accepted and closed out 2026-11-20** ("fix accepted! great work king", "close it out big boy!"). Executable
`2b152660b12e78d17111267bef9a3748fe1333b25ef0f0e9ee7c32a0224ec4bf`
(also carries D17BL and D08A43).

## Report

User, 2026-11-14 (playthrough commentary): the Holy Hand Grenade's held or
thrown model does not look like its pickup.

## Three models, two looks

| Use | Where the game names it | Model | Mesh |
| --- | --- | --- | --- |
| Pickup | object type 608, type table `+0x10` | 1129 | 35 vertices, 23 faces, gold (CLUTs `0xc7a`/`0xcba`, texels u 176-231 v 48-71) |
| Held | weapon slot 13 (`0x800c4590 + 44*13`) `+0xe`, drawn on the hand by `0x80033e40` | 945 | 42 vertices, 26 faces, dark (CLUT `0x193a`, texels u 216-232 v 11-25) |
| Thrown | the slot's projectile type (`+0` = 11), projectile type table `0x800c4bc0 + 64*11 + 6`, drawn by `0x8003177c` from the projectile loop `0x8006d08c` | 944 | the held mesh again |

The held and thrown meshes are one mesh in two frames: the held one is the
thrown one scaled by 0.672 and turned a quarter turn about x, into the hand
(fitted over all 42 vertices, 5 units at worst). All three use texture page
`0x2b`. No other weapon slot, projectile type or object type names 944 or 945.

## Fix (Modernized; the game's own pickup mesh, no new art)

`recomp/src/ttk/hhg_look.inc`, checked every game frame beside the debug
spawn (Modernized only):

- The model ids come from the game's tables above (slot 13, its projectile
  type, type 608).
- The thrown mesh is rewritten in place with the pickup's faces (texture and
  CLUTs as they are) and vertices scaled to the thrown mesh's original height
  (scale 0.836), its base at the thrown mesh's base, centred across. The held
  mesh gets the same, carried into the hand by the thrown-to-held transform,
  solved each time as a least-squares affine fit over the original 42 vertex
  pairs (refused unless every vertex fits within 8 units). Bounds, the radius
  across and the corner radius (also the header's) are recomputed.
- The pickup fits inside each mesh's own storage (vertices 280 of 336 bytes,
  faces 380 of 428), so nothing moves and the game's main-RAM pointer checks
  still pass. Models load with every level, so the check runs again after a
  level or savestate load; a converted mesh is recognised and left alone.
- It refuses rather than guesses when the meshes differ from the expected
  shape (different topology, no room, a poor fit).

New mesh-art rules (note 265) do not apply: no new mesh is made. The
first-person presentation (note 257) is the existing D12 hand draw, unchanged.

Developer: `DNTTK_HHG_LOOK=0` keeps the original meshes. Log line
`[TTK hhg]` when a level's meshes are converted (or why not, once).

## Evidence

Lab `recomp/analysis/d08a47/` (local only; `probe.py` case `hhg`: spawn the
grenade, pick it up, select it with 6, hold fire 40 frames to throw). Private
card copies of UI slot 2 (level 6); the player's saves and profile
hash-checked unchanged by every run. The log reports
`held 945 and thrown 944 now the pickup 1129 (scale 0.836, fit 4.3)`.

- First person, held: before a dark cross-topped cage, after the gold
  grenade with its crosses, upright in the hand.
- Third person, held and in flight: gold, the pickup's shape.
- Pictures (matched before/after): `research/before-after/D08A47/`.

## Limits

- A savestate made in Modernized carries the converted meshes in its RAM; the
  same savestate loaded with `--vanilla` shows them until the next level load.
- Checked in level 6 only (the grenade loads in levels 1, 2, 3, 6, 8 and 9 of
  the player's saves; the rule reads each level's own tables).
- Offscreen runs; size and light in the hand on the real display are the
  playtest.

## D08A51 - ORIGINAL / ACCURATE setting

OPTIONS > ADVANCED > ITEMS > HOLY HAND GRENADE (`hhg_look`, profile schema
83, `run.py --hhg-look`, environment `DNTTK_HHG_LOOK` read live; Vanilla
always `original`). ACCURATE (default) is the conversion above. ORIGINAL keeps
the game's own meshes. Because the conversion writes over the meshes in
place, `hhg_look_update` keeps, just before converting, the original bytes of
both meshes (vertex words, face words, vertex count at +6, the 16-byte bounds
block, the radius half at +2) together with an FNV-1a checksum of each
converted mesh (address, vertex count, vertices, faces). With ORIGINAL chosen,
a level whose meshes still match those checksums gets its originals written
back; choosing ACCURATE converts again. Up to eight levels' copies are kept,
one per pair of mesh addresses. The log says `back to the original model` on
a restore.

Lab `recomp/analysis/d08a51/` (local; `probe.py` case `toggle`, from the
D08A47 probe): live `option hhg_look` switches in level 6 (UI slot 2) gave held
mesh vertex counts 35 / 42 / 35 / 42 for ACCURATE / ORIGINAL / ACCURATE /
ORIGINAL and the matching pictures; a launch with ORIGINAL saved never
converts. Pictures: `research/before-after/D08A51/`.

Limit: a savestate made in an earlier session with converted meshes has no
kept copy; ORIGINAL applies there from the next level load (which loads the
game's meshes again).

### Game-wide: the pickup's art where a level lacks it

User, 2026-11-27, with ACCURATE on in a Pig Factory load: the grenade stayed
the original one. The log said `not converted (6)`, the D61B rule: the pickup
loaded untextured. A survey of every level index (0-31, `probe.py` case
`survey`, from RAM) found the pickup model in all but level 0 (see below);
untextured (all 23 faces CLUT `0xffff`, page `0x20`, local
UVs on two 16x24 boxes) in 9 Pig Factory, 13 Modern 4, 14 the animation test
and 30 the empty room; textured in every other level (page `0x2b` u 176-191 /
216-231 v 48-71, level 29 elsewhere on `0x2b`). In those four the local UVs
are exactly the shipped UVs minus each box's origin (faces 0-14 one box and
CLUT, 15-22 the other).

`build_ttk_look.py` (the D61B look pack) now gives those four levels level 6's
two blocks and CLUTs, copied from the disc's level 6 images into free VRAM
(zero in the level's images with a 2-texel margin, and zero in VRAM ten seconds
into the level with the look off), and `hidden_level_look.cpp` textures the
faces with their UVs shifted onto the blocks (its mode 0, no code change). The
builder now also refuses two uploads of a level that overlap. The grenade rule
then converts on the next frame (the log still shows `not converted (6)` once
at the level's first frame, before the faces are textured).

Evidence (offscreen, private cards): all four levels convert (`held 945 and
thrown 944 now the pickup 1129`); the uploaded blocks and CLUTs equal level 6's
words in VRAM in each (case `vramcmp`); Pig Factory from the player-style
savestate (slot 0): the floor pickup and the held grenade gold, ORIGINAL / ACCURATE
switch live, gold in the throw. Levels 13, 14 and 30 were checked from RAM and
VRAM only (the console spawn did not place a grenade at their starts). Pictures:
`research/before-after/D08A51/pigfactory-*.png`. The pickup on the floor in
those levels is fixed too (it drew the frame buffer before).

### TIME TO KILL: a level without the pickup

User, 2026-11-27, after the fix above: in a TIME TO KILL save (savestate slot
3, level index 0) the old grenade still showed. The log said `meshes not ready
(held 945 thrown 944 pickup 1129)`: level 0 loads the held and thrown models
but not the pickup (DB00MOD.BIN has no model 1129; the table entry is 0). The
survey's reading of level 0 as "no grenade" was wrong.

New `tools/local/build_ttk_hhg.py` -> `build-local/ttk-hhg.pack` (TTKHHG01,
CMake target `ttk-hhg-assets`): for such a level, level 6's model 1129 from
DB06MOD.BIN (its faces already carry level 6's CLUTs and page) with each
textured face's CLUT, page and UVs moved onto the two boxes copied, with their
CLUTs, from level 6's raw images into free VRAM of the level: level 0 block
(852,136) 8x24, CLUTs (784,122) and (784,123) (zero in the raw images with a
margin and in VRAM ten seconds in). `hhg_look.inc` reads the pickup into a
value (`HhgPick`) from the model table or, when the level has none, from the
pack, sends the pack's blocks with GP0 `0xa0` just before converting, and logs
`now the pickup 1129 from the pack`. ORIGINAL's restore no longer needs a
pickup. Checked offscreen on a copy of the player's slot 3: converts from the
pack, ACCURATE / ORIGINAL / ACCURATE live, gold in hand and in the throw
(`research/before-after/D08A51/timetokill-*.png`). Every level index 0-31 now
gives the gold grenade: 27 from their own model, 4 through the look pack, 1
through this pack.
