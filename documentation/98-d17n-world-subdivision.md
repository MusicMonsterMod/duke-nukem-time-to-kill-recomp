# D17N diagonal wall artifacts (world subdivision)

Status: **Accepted** (user playtest, 2026-10-04). After substantial play the
user reported a major improvement to the zigzag/vibrating texture effect, most
obvious in UI slot 12 and apparent across the game: "The result is excellent."
The user's message labelled the job D17P; this is D17N (D17P is the separate,
earlier accepted draw-distance job, note 97). The user authorized
documentation, commit and push.

Accepted executable SHA256 (new regression baseline):
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`.
Previous baseline (D17P):
`5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.

## Summary

The original game subdivides nearby walls, floors and ceilings into smaller
screen-space pieces. Those pieces use rounded pixel and texel coordinates and
lack the precise position data the Corrected texture path needs, so even with
Corrected textures they fell back to PS1-style affine (warped) mapping.
Because the subdivision and rounding change as Duke moves, the diagonal
distortions changed from frame to frame: the zigzag/vibration seen on surfaces
throughout the game. With Corrected texture precision, a nearby polygon whose
precise data is available is now drawn whole with proper perspective.

Compatibility and fallback behaviour (preserve):

- Corrected texture precision: whole-polygon path wherever possible.
- Original texture precision: original subdivision, unchanged.
- Vanilla mode: unchanged.
- Software renderer: unchanged (it never enables mesh precision).
- Polygons lacking exact projections, or exceeding the GPU primitive limit,
  keep the original subdivision.
- Existing savestates remain compatible (no codegen-hash input changed).

## Report

UI save slot 12 (file 11, SHA256
`1db85905d93a910ea746aacbc48d042bece5657bcd9a62fe9919dd3b690236dd`, unchanged
since note 89): walking forwards/backwards, the club's panelled wall shows
jagged diagonal lines and appears to vibrate. The user reports the same effect
throughout the game, most pronounced here.

## Cause: the original screen-space subdivision

Reproduced offscreen at 120 Hz, 4x, first person. The panel borders break and
step sideways along diagonal lines. Matching a packet dump against the near-clip
per-mesh trace identified the wall as world mesh `0x801bbd68` (about 5000 units
away), drawn by the world renderer `0x80011020`, the D15-instrumented path.
Original and Corrected textures looked identical there: the correction never
reached these polygons.

The world renderer projects every vertex into the buffer at `ctx+0x10` (8-byte
records: SXY, then SZ | flags). Bit 21 is set when SZ < `0x2000`
(`0x80011278`). Per polygon, the fetch helpers `0x800114ec` (triangles) and
`0x8001160c` (quads) AND the corner flags; with bit 21 still set the polygon
branches (`0x800118b0`, `0x80011ad0`) to the screen-space subdivision
`0x80012960` (triangles) or `0x8001205c` (quads). The quad routine writes the
whole polygon first (it fills T-junction cracks underneath) and then 2x2 or
4x4 pieces. Midpoints are integer averages of corner screen positions
(`0x800127c8`), UVs are averaged after clearing each coordinate's low bit
(`0x80012880`, `0x800128ec`), and colours are averaged. They are never
projected, so they have no PGXP shadow; every piece then fails D15's
"all corners exact" rule and is drawn affine per triangle. Border lines bend
along the piece diagonals, and because the integer midpoints and the 2x2/4x4
choice change as the eye moves, the bends move from frame to frame. Every
world surface within `0x2000` units is affected, which is why the effect
appears throughout the game. Vanilla PS1 has the same subdivision (it exists to
limit affine warping); it is not a recomp regression.

## Implementation

`recomp/src/ttk/near_clip.cpp`, two new function-entry hooks (regenerated
`game.local.toml`): `0x800114EC` (ra `0x80011894`) and `0x8001160C` (ra
`0x80011ab4`), with the render context in a2. The existing `0x80011020`
SHA-256 guard (0xd4c bytes) covers both helpers and both branches.

When Modernized texture precision is Corrected (the GPU maps each polygon with
exact perspective), a world polygon is drawn whole, without the subdivision,
if all its corners have bit 21 set, every corner's SXY word has an exact,
current-generation projection (`pgxp_mesh_vertex` on the buffer record), and
its screen bounds are at most 1000 x 500 native pixels (within the GPU
primitive limit). The hook clears bit 21 on that polygon's first corner record
and restores it at the next fetch, so neighbours sharing the vertex decide for
themselves. A new mesh (`0x80011020` entry) drops a pending restore because the
vertex pass rewrites the buffer. Anything else keeps the original subdivision.

Texture precision Original (affine GPU mapping), Vanilla and the software
renderer never reach the bypass. Game simulation, collision, OT ordering and
the near-clip host path are unchanged. `DNTTK_WORLD_SUBDIVISION=1` (developer)
keeps the original subdivision for comparisons. The `ttk_input` near-clip
report adds `whole_polygons` and `subdivided_kept`. No framework file, codegen
hash input or save format changed; existing savestates load.

## Evidence (private, local)

`recomp/analysis/d17n-20261004/` (private cards copied from the player's
current files; player files never used; port 9241):

- `intake/`: Original and Corrected textures show the same diagonal breaks on
  the baseline; the candidate's borders are straight (`cropfix-*`).
- `pk/`, `trace/`: packet dump outlines, the per-mesh trace and the matched
  wall quad (whole quad plus four 2x2 pieces with UV `0x0f`/`0xaf` midpoints).
- `walk/`: 24-present 120 Hz sequences walking W and S, old subdivision
  (`DNTTK_WORLD_SUBDIVISION=1`) versus candidate. The old path breaks the
  borders at changing places; the candidate keeps them straight in every
  present (`sheet-w2.png`).
- `sweep/`: all 12 private slots, 120 Hz, 4x, first person, idle and turning,
  old versus candidate. Exact-projection share of mesh polygons rose from
  0.29-0.43 (0.68-0.71 in UI 9/10) to 0.73-1.00. Game rate unchanged (UI 4
  22.0 vs 24.2 idle, UI 11 20.0 vs 19.7, all others 30). Packet ring peak fell
  in every slot (for example UI 12 46252 to 30356, UI 11 69584 to 50704
  bytes). Zero near-clip budget hits, fallbacks, packet skips, replay misses or
  failed jobs. Polygons kept subdivided: 0-127 per run. Idle images contain no
  missing surfaces or background cracks; the darkened-pixel differences are
  remapped dark texels and a straighter building-corner highlight (UI 4).
- `modes/`: slot 12 at 60 and 180 Hz uses the same path (no replay misses);
  Original textures and Vanilla report zero whole polygons.

Tests: `ttk-near-test disc/SLUS_005.83` adds whole-polygon contracts (first
corner only, restore, stale restore after a new mesh, already-whole, missing
projection, oversize, wrong caller, foreign context, altered code), passing
with Original, Corrected, geometry-only and the developer override; the packet
digest stays `b6dfbb5aad8fd86a`. Python 106 tests (104 pass, 2 conditional
skips), `ttk-input-test` and `ttk-aim-test` pass. `scripts/ci/check_repo.py`
passes.

## Limits

- Accepted on the user's playtest; the private checks above are the recorded
  automated evidence.
- Subtle residual wall/surface flicker remains in places. It is tracked as
  independent job D17Q; its cause is not assumed to be this one.
- Applies only with Corrected texture precision. With Original textures the
  original subdivision (and its diagonal breaks) remains by design.
- Polygons whose corners lack an exact projection, or that exceed the
  primitive limit, keep the original subdivision.
- Object/prop meshes (`0x80010000`) have no such subdivision and are
  unaffected. Campaign-wide coverage is not established.
