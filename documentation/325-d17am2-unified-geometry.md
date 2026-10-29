# D17AM2 - Unified host geometry path behind a developer switch

2026-10-29. Done, user-accepted ("slot 12's texture glitch looks fixed to me. Let's commit and push this work now as we stand and close us out"). Design and approval:
[note 324](324-d17am1-unified-geometry-design.md) (D17AM1, approved by the
user as recommended: "i'm gonna say i want to go entirely with your
recommendation, end to end").

## What the switch does

`DNTTK_UNIFIED_GEOMETRY=1` (developer, not a saved preference; default off)
makes Modernized draw every world, prop and actor polygon through the host
path at every distance, with the approved rule set:

- **Ownership.** World renderer (its three callers) and object renderer
  faces are all host-drawn while the near path is live. Sky meshes, the
  first-person layer, fixed-projection limbs, the Bio Mask and water tiles
  keep their own paths (`Frame::scene` marks what the rules apply to).
- **Sort key.** Every piece goes into its source polygon's native ordering
  slot: world, farthest corner SZ >> 5 with the native far-limit test;
  objects, AVSZ3/AVSZ4 with the face's own +-1 bias. No per-piece keys, no
  object bias, no compact-prop shared key.
- **Quads.** A quad wholly inside the clip planes and the GPU size limit is
  one GT4/G4 packet, which the GPU draws on the native 1-2 diagonal; any
  other quad is drawn as its two native halves. Pieces are split only past
  the GPU primitive limit (1000x500), never for depth locality.
- **Depth.** Opaque pieces test and write depth at the raster plane, with
  the one contact rule (2 units for non-actor props, 16 for the verified
  payphone back) and the D17W coplanar rank. Translucent scene faces carry
  no depth (painter order in their native slot).
- **The original's own per-polygon rules**, now needed because the host
  draws faces the original used to draw itself (found while verifying,
  from the retail world renderer): with context flag 0x80, a world polygon
  whose exactly projected corners all lie past one edge of the clip
  rectangle `ctx+0x18..0x1e` is dropped (0x80011634..0x800116f0); one whose
  corners are all beyond `ctx+0x70` is drawn as an untextured black
  silhouette (0x800118a4, 0x80011a80). The object renderer has neither rule.
- **Overflow.** A mesh the frame cannot afford (estimate: one 52-byte packet
  per face plus the existing reserve) falls back whole to today's path
  (native packets with scene depth).

Developer comparisons: `DNTTK_UNIFIED_OWNERS=world|object` applies unified
ownership to one renderer only; `DNTTK_UNIFIED_QUADS=0` draws every quad as
two halves. The D17AM1 measurement switches (`DNTTK_NEAR_TAKEOVER=all`,
`DNTTK_NEAR_MEASURE=1`) remain.

## Packet budget (both renderers)

Each frame's region is now that frame's 128 KB host block plus its 47 KB
Bio Mask block, filled through one cursor; no packet straddles the two. The
two GPU DMA allocations, their order and size and the D20C region mirror are
unchanged, so savestates keep their layout. With the switch off, host
pieces and the weapon keep exactly the 128 KB they had and the Bio Mask and
frozen tint use the rest, so the current renderer's packets are
byte-identical (contract digest unchanged). With the switch on, scene
geometry may use the region except 8 KB kept for the first-person layer, 32 KB
kept for the sky (drawn last; the canal's sky peaks near 25 KB at FOV 120), and
while the Bio Mask is worn (the player flags its draw tests, read from guest RAM
so in-between images agree) what the built mask mesh needs, one packet per
triangle of its 184 (7.4 KB; about 90 face the eye and are drawn; a static
assert keeps it within its 47 KB block); otherwise 256 bytes for the frozen
tint. Scene limit: 134.75 KB (137984 bytes), 127.6 KB with the mask worn.

A mesh the frame cannot afford whole is first downgraded: it is drawn exactly
as the current renderer would (its near polygons here with the current rules,
the rest native with scene depth), so it never loses near clipping. Only if
even that does not fit does it fall back whole to the original path, as
today. Both are counted in the debug JSON (`measure.unified_downgrades`,
`measure.fallbacks` by world/object/sky).

## Fix for both renderers: wide-FOV guard band

At FOV 120 the projection H is 92, and the D17AE angular guard band
(x/z <= 3.0) ended 276 pixels from centre, just past the 4:3 edge (256),
so host pieces in the 16:9 margins were clipped away. The current renderer
showed it as a black near floor in the right margin at FOV 120; unified
mode lost whole margins. The guard band is now the wider of the angular
ratio and the native 480x240-pixel band. At H >= 160 (FOV 90 and narrower)
nothing changes.

## Verification

Build: player executable SHA-256 `2cd55c0e9ba1ed030df6ee64dd185bf5b5faa9e4c0afcc0050eede117afc495e`.
No framework, generated C, header or asset changes.

Contracts (`ttk-near-test recomp/disc/SLUS_005.83`):

- Switch off: all existing contracts pass and the whole-run packet digest is
  unchanged from HEAD (`a451f43de35cf84b`), so the current renderer's packets
  are byte-identical, including Bio Mask and frozen-tint packets now written
  through the shared cursor.
- Switch on (`DNTTK_UNIFIED_GEOMETRY=1`, also with Corrected geometry): new
  contract "D17AM2 unified geometry": a distant world quad becomes one GT4 in
  its native slot with four host vertices and depth; a far quad becomes a black
  G4 silhouette; a quad wholly outside the clip rectangle with flag 0x80 is
  not drawn; two prop faces keep their own AVSZ3 slots and a translucent face
  carries no depth; a warped prop quad is one packet in PS1 corner order;
  Vanilla keeps the original path. Bio Mask, frozen tint and water contracts
  pass in this mode. The current-renderer contracts are skipped in this mode
  because they assert its splitting.

Private fixture copies only (D17AK set, current UI slots 11/12 from D17AL);
every run hash- and mtime-checked all player saves, cards and settings
unchanged. Offscreen, Corrected geometry/textures, first person unless
stated. Probe `recomp/analysis/d17am1/measure.py`.

Static A/B against the current renderer (same build, switch off/on), share of
pixels differing by more than 24 levels:

| Fixture | FOV | Differing pixels |
| --- | --- | --- |
| Club exterior (UI 11), 16:9 | 90 / 120 | 0.002% / 0.001% |
| D17AE sign post (current UI 11) | 90 | 0.002% (post hidden) |
| D17AL payphone save (current UI 12) | 90 | 0.011% |
| Western street (UI 5) | 90 / 120 | 0.012% / 0.052% |
| Western canopy (UI 6) | 90 / 120 | 0.014% / 0.004% |
| Third person, club / canopy | 90 | 0.001% / 0.001% |
| 4:3, club / canal | 90 | 0.002% / 0.1% |
| Medieval canal (UI 2), 16:9 | 120 | about 3% (see below) |

The canal's difference is not a rendering rule. Its light tables animate,
and with unified geometry the original renderer does far less emulated work,
so the game draws more often (every third field instead of every fourth in
this view) and the captures fall on different lighting phases. The current
renderer differs from itself by 1.2% (2.9% in the brightest area) when
captured 0.4 s later, and a GP0 dump of the area shows the same two
primitives in the same order in both modes (floor quad, then the
translucent water) with only corner colours differing; the current
renderer's own colours for that quad change between its captures.

D17AL payphone approach route (10 steps): phone whole at every step in both
modes. Sign post (D17AE) hidden, canopy (D17AG) continuous.

Cost, turn and walk routes (host bytes per frame peak; skipped packets;
whole-mesh fallbacks; live hook time average/peak; 120-FPS worker time per
in-between image; replay failures/misses):

| Fixture | Mode | Rate | Peak | Skips | Fallbacks | Hook ms | Worker ms | Fail/miss |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Canal FOV 120 | current | 120 | 98384 | 0 | 0 | 0.86 / 5.0 | 15.1 | 0/0 |
| Canal FOV 120 | unified | 120 | 134552 | 0 | 0 | 1.00 / 6.6 | 10.1 | 0/0 |
| Canal FOV 120, Bio Mask | unified | 120 | 128768 | 0 | 1 (1 downgrade) | 0.86 / 6.7 | 10.7 | 0/0 |
| Canal FOV 90 | unified | 60 | 107408 | 0 | 0 | 0.64 / 6.5 | - | - |
| Western street FOV 120 | current | 120 | 80232 | 0 | 0 | 1.29 / 5.4 | 17.4 | 0/0 |
| Western street FOV 120 | unified | 120 | 129472 | 0 | 0 | 1.66 / 6.3 | 13.2 | 0/0 |
| Western street, Bio Mask | unified | 120 | 125704 | 0 | 0 | 1.62 / 7.3 | 13.2 | 0/0 |
| Western canopy FOV 90 | unified | 120 | 65568 | 0 | 0 | 0.77 / 4.0 | 5.4 | 0/0 |
| Club FOV 90 | unified | 120 | 46284 | 0 | 0 | 0.33 / 3.1 | 5.3 | 0/0 |

With the Bio Mask worn the mask drew all 90 triangles every frame (same as
the current renderer). Live hook time rises 0.1-0.4 ms per frame on average;
the 120-FPS worker time per in-between image falls 3-5 ms because the
emulated renderer has little left to do. Offscreen present cadence is not a
D23 measurement; D17AM3 owns that.

Wide-FOV guard band: with the switch off, the canal at FOV 120 now draws its
near floor in the right 16:9 margin (previously black); the centre is
unchanged (0.03%). At FOV 90 and narrower the band is unchanged.

## Limits

- Developer switch only; Modernized default unchanged. Needs a live
  playtest with the switch before D17AM3 (sweep and default switch).
- Fixtures: the D17AK set and current slots 11/12. No level-select sweep,
  no mirror room, no D17W minecart, D17Z corridor, D17AA block or D17M
  platform fixtures yet; those belong to the D17AM3 sweep.
- The canal comparison cannot be made pixel-exact (animated lighting and a
  changed draw cadence); the GP0 evidence above is the comparison.
- Superseded special cases (handover thresholds, platform/compact/top/ladder
  depth, object bias, scene depth for taken meshes) are still in the code
  and still used with the switch off; D17AM3 removes them after the default
  switch.
- The faster draw cadence in heavy views is a side effect of taking work off
  the emulated renderer. Whether game logic timing is affected was not
  measured; D17AM3 should check it in play.

## User playtest and follow-up (2026-10-29)

User playtest with the switch: stable; Bio Mask fine; blocks, climbing,
mantling, monkey bars, enemies, NPCs, combat and the medieval crystal machine
fine; no wall-edge glitches seen. Reports: UI slot 12 wall panel artifacting
while turning, and UI slot 10 water vanishing near Duke (D17AF, independent of
the switch).

Follow-up change: overlapping coplanar faces of one world mesh are ranked as
D17W ranks prop faces (the earlier record, which the original paints last,
wins under the depth test). Prompted by slot 12's load view, where world mesh
`0x801a1a38` defines a panel twice on the same corners with opposite winding,
both drawn. World meshes compare only faces in the same or a neighbouring
plane bucket (normal to 1/64, distance to 4 units). Unified only; the current
renderer's digest is unchanged. New unified contract: a twice-defined world
panel keeps the earlier record in front. The reported artifact was not
visibly reproduced offscreen, so this needs the user's confirmation.
Build `d5341463174b683b72c586dd51e898ee68b9d7ba0c5ba205303107e42346cd2a`
(also includes D19AA, Tab fast-forward removed).
