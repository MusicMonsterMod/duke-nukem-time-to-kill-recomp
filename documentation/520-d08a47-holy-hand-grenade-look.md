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
