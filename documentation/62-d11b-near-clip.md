# D11B - First-person near-wall clipping

**Done - user accepted 2026-09-29:** "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available") In the Modernized eye view (D11), walls,
floors and props right beside or in front of the eye are now clipped and drawn
correctly instead of tearing or turning black. Third person and Vanilla are
unchanged: nothing here runs unless the eye view is live.

## Cause

The D11 note blamed `0x8002f1e0` (outcode bit `0x10`). That routine
(`0x8002ef90`) draws a horizontal 1024-unit grid surface; it is not the walls.
The GTE projection ring on the club slot showed the level going through two
hand-written mesh renderers at the start of the executable:

| Renderer | Draws | What it does with near vertices |
| --- | --- | --- |
| `0x80011020` world | Rooms: walls, floors, ceilings. Vertices on a 1024 grid (x/z bytes x 1024, y byte x 128), GT3 (0x60) and GT4 (0x61) records, fog by DPCS from fog start `ctx+0x6c`, light table, texture animation. Callers ra `0x80037344`, `0x80037c54`, `0x80062db8`. | Drops a polygon if any vertex has SZ 0 (at or behind the eye). Vertices nearer than H/2 saturate the GTE divide and get wrong screen positions. Polygons with every SZ under 0x2000 go to a screen-space subdivision (`0x80012960`) that reuses the wrong points. |
| `0x80010000` object | Props and actor joints: F3/F4/G3/G4/FT3/FT4/GT3/GT4 records, OTZ from AVSZ3/4 with a +-1 bias. About 20 callers. | Drops a polygon only if every vertex is nearer than H; otherwise draws it with saturated coordinates. |

Both take a0 = mesh, a1 = ordering table `0x800d27a0` (2048 head/tail pairs),
a2 = render context `0x800d67a8`, a3 = OT bitmap `0x800d26a0`. Context fields
used: `+0` packet cursor, `+4`/`+8` arena start/end, `+0x48`/`+0x4c`
command/tpage bits, `+0x50` flags (`0x400` flips the NCLIP sign, `2` flat
color, `0x80` screen outcodes), `+0x54`/`+0x5c` flat colors, `+0x58` texture
table (12-byte entries), `+0x60` animation table, `+0x68` light table,
`+0x6c` fog start, `+0x78` OT depth limit.

That is why a shorter projection distance or moving the eye back in D11 could
not fix it: the fault is in how these two routines treat any vertex near the
eye plane.

## Implementation

`recomp/src/ttk/near_clip.cpp` (header comment lists the layouts), hooks
`0x80010000` and `0x80011020` (new entries in `game.local.toml`
`mod_function_entry_funcs`, regenerated: one generated line each). SHA-256
guards cover both routines (`0x80010000`/0xd18, `0x80011020`/0xd4c).

At each renderer entry, only while `first_person_view_live()` (Modernized,
orbit lease, first-person blend > 0), with the known OT/context/bitmap
arguments and (world) a known caller:

1. Project every mesh vertex exactly as the GTE does (RT/TR from
   `cpu->gte_ctrl`, UNR divide, IR and SXY saturation). A vertex is unsafe if
   the divide saturates, IR clamps or its true projection is beyond +-1000.
2. Take over every polygon with an unsafe corner or a corner nearer than 1536
   units (props: 3072, see the second pass). Object polygons with every corner nearer than H stay dropped, as in
   the original. Duke's own model (between his actor draw at `0x800348d8` and
   the next object-list step `0x8001ca4c`) is left to the original (D12).
3. Resolve its attributes the way its renderer does: world vertex colors with
   the color-index table `0x800c37e4` or the flat color, fog DPCS, light-table
   DPCS when every corner's index byte has bit 4, texture table and animation;
   object flat/palette colors, semi-transparency and `ctx+0x48`/`+0x4c` bits,
   animation (16-byte entries), OT bias. Back faces use the sign of
   det(v0,v1,v2) in view space, which is NCLIP's sign for points in front of the
   eye, with the renderer's `0x400` rule and double-sided flags.
4. Clip in view space against z >= 16 and a guard band of +-480 x +-240 pixels
   (every piece stays inside the GPU's 1023x511 primitive limit).
5. Textured pieces are cut along whole-texel lines (u or v integer) until they
   are at most 96 pixels, or 16 pixels when their depth ratio exceeds 1.25.
   Packets carry integer UVs and near the eye one texel covers 15-25 pixels,
   so cutting at fractional coordinates made straight texture lines kink.
   Untextured pieces are bisected geometrically.
6. Emit GT3 (textured) or G3 packets from the render arena with the renderer's
   own wrap rule, and insert each into an OT slot from the piece's own average
   depth (world: average SZ >> 5; object: AVSZ3 >> 3 with ZSF3 341, the same
   scale, plus the polygon's bias bits and -4 for props; both skip past
   `ctx+0x78`). Vertices that are safe original corners keep the GTE's exact
   screen position, so seams with original polygons match; new corners round
   away from the piece centre so neighbouring pieces overlap instead of
   cracking.
7. Point a0 at a copy of the mesh (Expansion 1, 64 KB) whose polygon list
   omits the taken polygons, so every other polygon still takes the original
   path. The world copy keeps the header and every vertex record the original
   reads (it reads in threes), and the host applies the original's one-shot
   color-index nibble clear (`0x80011170`) to the real mesh.

Developer switches (environment, not saved preferences): `DNTTK_NEAR_CLIP=0`
off, `=world` or `=object` for one renderer; `DNTTK_NEAR_TINT=1` paints host
world pieces red and host prop/actor pieces blue; `DNTTK_NEAR_OBJECT_BIAS`
(default -4) and `DNTTK_NEAR_SORT=min` (nearest-corner sort, the first pass)
for comparison; `DNTTK_NEAR_DEPTH_SPAN` is experimental (it overran the arena
at 32/64 before the frame budget existed).
Debug JSON: `ttk_input` -> `controls` -> `fp` -> `near` (`world`/`object`:
`seen`, `taken`, `polys`, `culled`, `triangles`, `budget_hits`; `refused`,
`copy_overflows`, `duke_skips`, `zsf3`, `arena_size`, `frame_used_peak`,
`host_bytes_peak`, `packet_skips`, `mesh_fallbacks`).

## Second pass (user playtest, apartment wardrobe)

User: "it looks very good, but i think there is a kind of popping or
artifacting ... inside the hooker's apartment ... the closet ... the back wall of
the closet ... pops through" (the door itself was fixed). Reproduced on fresh
private cards (street spawn -> alley ladder -> window, god mode, monsters
hidden, debug state saved in `recomp/analysis/d11b-near-clip/apartment-cards`).
Findings and changes:

- **Comb between wardrobe and wall.** The wardrobe is a prop (object
  renderer) standing flush against a world wall. The first pass sorted every
  piece by its nearest corner, so wall and wardrobe pieces alternated in the
  OT (sawtooth). Pieces now sort by their average depth; host prop pieces sort
  4 slots (128 units) nearer, which removed the teeth that -2/-3 left where the
  wardrobe door meets the wall. Actors (drawn between `0x800348d8` and the
  next list step `0x8001ca4c`) get no bias, so an enemy just behind a wall edge
  keeps the original rule.
- **One key per prop polygon was tried and rejected:** a long door then lost to
  nearby wall pieces.
- **Original prop polygons vs host wall pieces:** the wardrobe's inner door
  (about 2000 units, drawn by the original with its whole-polygon average)
  was covered by a host wall piece for a frame. Props are now taken over to
  3072 units.
- **Hairline cracks** (single-frame 1-2 pixel slivers while walking) came from
  T-junctions between pieces split differently; new corners now round away
  from the piece centre.
- **Packet arena.** The render arena is a 139,744-byte ring filled continuously;
  per frame the original uses about 13 KB (apartment) to 29 KB (club). Heavier
  splitting overran it within a frame (screen garbage). Host packets are now
  limited per frame (frame start = OT bitmap empty): 64 KB, and never past 60%
  of the ring in total; short of budget pieces stop splitting and a mesh that
  cannot be afforded stays on the original path. Measured peaks: 42-46 KB of
  host packets, 0 skipped packets, 0 mesh fallbacks.

## Third pass: conservative default (user report at 1080p)

User: "everywhere I go now has like black lines over the floor ... a black grid
when you're outside ... some of the objects have black lines over them quite
faint ... the floor texture is still popping ... this does not happen in third
person ... I do wonder whether the previous iteration was actually better",
asking for the options, and to run tests at 1080p.

Reproduced with the presented window captured from the X display at
1920x1080 (the runtime's `screenshot_file` reads the internal software raster
and did not show it): with clipping on, dotted dark lines run along host piece
edges on pavement and grass. Causes: the outward rounding of new corners drew
pixels past a floor tile's UV range; T-junction seams of the subdividing mode
remained with floor rounding; the -4 prop bias could pull a wardrobe interior
over a wall. Three modes were compared at 1080p (off / full / conservative):
conservative matched the original wherever the original was right and fixed
its holes.

**Default is now conservative:** only polygons the original would draw wrongly
or drop are taken (unsafe corner, wider than 1023 or taller than 511 pixels,
or a prop polygon entirely nearer than H, which the original drops and which
made props see-through up close); each is clipped and emitted as a fan of
triangles, never subdivided, sorted as one polygon by the average depth of its
visible part (props keep their own bias bits, no extra bias). Corners are
floored like the GTE. It uses at most about 1.2 KB of packets per frame. The
subdividing, perspective-correct mode stays available as `DNTTK_NEAR_MODE=full`
(with `DNTTK_NEAR_ROUND=outward` for its old rounding).

Evidence on binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`: 1080p
street (4 views): no dark lines (remaining differences are the animated neon
sign and single pixels); apartment 1080p views and five-spot roam: no holes,
wedges or combs (clip off: torn, see-through wardrobe); walking into the
wardrobe: solid door where clip off is see-through; club side wall and
corridor: gaps filled; 60.01 fps clipped, 59.97 off, 59.71 third person, 0
underruns; Vanilla `d11b-vanilla-3` exit 0; native tests and Python 74 OK;
movie shard current. Captures: `recomp/analysis/d11b-near-clip/third-pass/`.
Accepted trade-off: near-eye polygons keep the PS1's affine texture bend (as
in third person and the original), and polygons sort whole, as in the original.

## Occluder fade in the eye view (user review)

User verdict: conservative default "looks the very best"; `DNTTK_NEAR_CLIP=0`
stays as Vanilla rendering; `full` is research only. The subway card-reader
door turned invisible up close in first person; the user's DuckStation
screenshot shows the original doing the same in third person. It is the
original occluder fade: the prop loop `0x80031fa0..0x80032078` sets the GPU
semi-transparency bit (`ctx+0x48 = 0x02000000`) and dims a prop by distance
when it is nearer than camera+0xa0 (about 291) and `0x8002ee50` finds its
screen rectangle overlapping Duke's (camera+0xa8; halves x,y,w,h). In the eye
view Duke's rectangle covers the screen, so every prop walked up to faded.
Hook `0x8002EE50`: in the eye view only, the call from `0x80032008` (ra
`0x80032010`) gets an empty off-screen rectangle, so the prop draws solid.
Guards: `0x8002ee50`/0x140 and the caller `0x80031fa0`/0xe0.
`DNTTK_FP_OCCLUDER_FADE=1` keeps the original fade. Debug JSON adds
`fades_skipped` and `fade_tests`. Both switches are planned D19 menu toggles.

## Evidence

Second pass (current) binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`:
apartment closet sweep (225-315 degrees), walk toward the wardrobe (24
frames), and five-spot room captures (8 headings + 4 up) are clean with
clipping on; the unclipped build shows the original's torn/see-through
wardrobe and black ceiling wedges at the same spots. Club side wall and entry
corridor sweeps unchanged. Frame budget in the club: 59.92 fps clipped, 59.94
unclipped, third person 59.94, 0 underruns. Vanilla route `d11b-vanilla-2`
exit 0, captures normal. `ttk-near-test`, `ttk-controls-test` (D07 fixture),
`ttk-input-test`, `ttk-aim-test` PASS; Python 74 OK (2 skipped); movie shard
current. Captures: `recomp/analysis/d11b-near-clip/apartment/`,
`club-final.png`.

First pass binary `d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa`:
Isolated Xvfb runs on private route cards (slot 1, club main room, first
person), captures in `recomp/analysis/d11b-near-clip/`. Duke was placed with
debug position writes on those private cards only, then walked into the wall.

- Club red-panel side wall (5486, -4096), view swept -90..+90 in 15 degree
  steps with `DNTTK_NEAR_CLIP=0` and on. Off: looking along the wall (+15 to
  +45) the near wall is missing (black), head-on the frame is bent. On: the
  wall is continuous, texture lines straight and perspective-correct.
- Club entry corridor side wall (5442, -800), same sweep. Off: large black
  gaps at -30..-15 and +15..+30. On: continuous wall.
- Stage block at the slot's spawn: floor near the eye now draws to the bottom
  edge (off: missing, showing what is behind).
- Counters on the side-wall sweep: 0 refusals, 0 copy overflows, budget hit
  on 2 pieces.
- Frame budget (software renderer, 15 s of walking into the stage block with
  mouse sweeps): first person 59.96 fps with clipping, 60.04 without; third
  person 60.03; 0 audio underruns each.
- Third person (P toggle in a live run): 0 near-clip calls, projection 386 and
  minimum distance 768 intact; back to first person resumes.
- Vanilla regression route `analysis/vanilla-regression/d11b-vanilla` exit 0;
  captures show the original camera, fire, jump, inventory, forward, turn.
- Native: `ttk-near-test` (GTE-exact projection, safety classes, DPCS),
  `ttk-controls-test` (with the D07 LEVEL00 fixture), `ttk-input-test`,
  `ttk-aim-test` PASS; Python 74 tests OK (2 skipped). Movie shard current.

## Limits

- Tested in the club (side wall, stage block, entry corridor) and the
  apartment, not the whole first level or the campaign.
- Sorting is still a painter's algorithm (the PS1 has no depth buffer): the
  -4 prop bias means a prop up to 128 units behind a nearby wall piece could
  draw over it; actors are excluded from the bias.
- Affine texture mapping remains inside each piece; pieces are small near the
  eye, so it is only visible as slight bending at moderate distance.
- Polygons whose corners are all nearer than H in the object renderer stay
  dropped, as in the original.
- Duke's own body is not clipped here (D12 owns hands and weapon).
- If the runtime's widescreen X squash were enabled, host-drawn pieces would
  not be squashed (the project runs 4:3; D14 owns FOV and widescreen).
