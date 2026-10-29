# D17AM1 - Unified host geometry path: inventory, rule design and cost

2026-10-29. Done: approved by the user as recommended (all four decisions),
implemented in D17AM2 ([note 325](325-d17am2-unified-geometry.md)), which
refines the budget plan (Bio Mask block shared per frame, reserve sized from
the built mesh). This report changed no renderer behaviour. Parent: D17AM
(job board, renderer plan). Prerequisite D17AL is Done
([note 323](323-d17al-prop-contact-depth.md)).

The goal of D17AM is that every world and prop polygon is drawn by the host at
every distance, so the 1536/3072 handover and the rules that exist only
because of it can go. This note catalogues what `recomp/src/ttk/near_clip.cpp`
does today, proposes the single rule set, and measures what full takeover
costs on the heaviest fixtures. D17AM2 should not start until the rule set
and the budget plan below are approved.

## What was added for this report

One developer measurement switch, default off, in `near_clip.cpp`:

- `DNTTK_NEAR_TAKEOVER=all` sends every world, prop and actor polygon through
  the host path at any distance. Sky meshes, the first-person layer and
  fixed-projection limbs are unchanged. Split and whole-mesh fallback
  decisions ignore the arena budget; packets that do not fit are counted and
  not written, so pictures past the budget are not representative (the sky,
  drawn last, loses its clouds).
- `DNTTK_NEAR_MEASURE=1` times the current renderer the same way.
- New `measure` block in the `near` debug JSON: per-frame demand peak/average
  (bytes the host path wanted, written or not), frames over the 128 KB region,
  triangle peak, hook time per live frame, and demand by owner
  (world, prop/water, actor, sky, first-person layer).

With neither variable set, behaviour is unchanged: `ttk-near-test` passes all
contracts on the built player executable. Probe:
`recomp/analysis/d17am1/measure.py` (private copies of the D17AK fixture set,
offscreen 16:9 first person, Corrected geometry/textures, FOV 90 unless
stated; route: load, 12 turn steps with pitch, 8 walk and 8 back steps).
Player saves, cards and settings were hash- and mtime-checked unchanged on
every run.

## Inventory of current thresholds and special cases

Line numbers are `near_clip.cpp` at this report. "Fate" is the proposal.

| Item | Where | What it protects | Evidence | Fate |
| --- | --- | --- | --- | --- |
| World takeover depth `perspective_z=1536`, mesh `any` floor 2048 | 152, 964 | Affine kink where native and host halves meet; near-eye saturation | D11B (62), D17A (81) | Retire: all polygons host |
| Prop takeover `object_perspective_z=3072` | 156, 1291 | Native prop sorted by whole average lost to host wall pieces (wardrobe inner door) | D17A | Retire |
| Safe projection (`sz*2>H`, IR unclamped, inside +-1000) | `near_project` | Which native corners can be trusted as snap reference | D11B | Keep, as the vertex snap reference only |
| Oversize primitive (1023x511) | `oversize` | GPU drops large primitives (wardrobe up close) | D11B | Subsumed by clipping |
| Object all-corners-nearer-than-H drop | object() | Native object renderer drops close faces (see-through props) | D11B | Subsumed |
| Guard band 3.0/1.5, near plane 16 | 87 | GPU primitive limit at any FOV, behind-eye geometry | D17AE (322) | Keep: the clip rule |
| Backface rules (world 0x200 double-sided, second triangle when 0x400 clear; object bit 1) | 798 | Retail NCLIP behaviour | D17AE revert (322) | Keep |
| Precise split 192 px / depth ratio 1.5 / max depth 12 / `split_reserve` | 112, `subdivide` | Keep a piece's sort key local against nearby items | D17A | Retire (see sort key rule) |
| Conservative mode, non-precise `refine` texel-line splits, `round_outward` | 117, 130, `refine` | Developer fallback when precise vertices are off | D11B, D17A | Retire with the switch-over (unified path requires precise vertices) |
| Object OT bias -4 slots | 106 | Prop pieces losing to wall pieces in a comb (wardrobe) | D17A | Retire: depth plus contact rule decides |
| Thin platform depth bridge (`platform_depth_z=8192`, 128/256 slabs, overhead only) | 158, 995 | Native platform between 1536 and 3072 cannot occlude host props | D17M (200) | Retire |
| Compact prop depth (256 extent, 320 for pickups) and shared `compact_key` | 1320 | Distant cup/ammo painter-only beside a host table/platform | D17L (95), D17M | Retire (shared key replaced by native key) |
| Static box tops depth-only | 1339 | Distant box top overwrote a depth-tested item | D17L | Retire |
| Ladder depth | 1317 | Tall ladder crosses the boundary; upper panels vs host platforms | D17M1 (201) | Retire |
| Native order for safe scenery faces | 1431 | Prop face entering host before an intervening native wall | D17AA (267) | Becomes the general sort-key rule |
| World source order (max corner SZ key shared by pieces) | 781 | Near floor piece overwrote native decals/props ordered in front | D17A follow-ups | Becomes the general sort-key rule |
| Native 1-2 diagonal for warped quads and Corrected world textures | 722 | Texture changing at takeover; intersecting snapped pieces | D17Z (266) | Keep, as "quads keep the native diagonal" |
| Plane depth at raster position (`DNTTK_PLANE_DEPTH`) | emit | Snapped corners turning shared planes into intersecting triangles | D17A | Keep |
| Prop contact allowance 2 units, phone back 16 | 681 | Matrix quantisation between world and prop; embedded payphone | D17AC (292), D17AL (323) | Keep: the single contact rule |
| Coplanar rank (same-mesh overlapping faces) | 1143 | Minecart textured panel vs flat face | D17W (status log 2026-10-20) | Keep: the decal/coplanar rule |
| Scene depth for native packets | 829 | Painter order vs real overlap at distance (club sign post, canopy) | D17AE, D17AG, D17AL | Retire: no native world/prop packets remain |
| World subdivision bypass (Corrected textures) | 1509 | Native screen-space subdivision jitter | D17N (98) | Moot for host-owned meshes; keep for anything left native |
| Sky painter order | 1261 | Sky layers composed in painter order | D17AH (319), D17R1 (173) | Keep, sky excluded |
| First-person layer slot 1, fixed projection, Bio Mask, frozen tint | 246, viewmodel flush | Weapon never cut by walls, HUD on top | D12 (63), D36, D35J5 | Keep unchanged |
| Third-person body coverage | object() | Duke's body fade near the camera | D36 | Keep |
| Missing-texture animation filter | 1091 | Zero descriptor sampling the framebuffer | D17Y (206) | Keep (orthogonal) |
| Occluder fade suppression | 1479 | Doors fading in the eye view | D11B | Keep (orthogonal) |
| Water tile clip | `water_clip.inc` | Native water builder outside the mesh renderers | D17AF (321) | Keep separate (D17AM4 re-triage) |
| Far limit (`ctx+0x78`), fog, light table, texture animation | `slot_index`, world()/object() | Native draw distance and colour | D11B, D17P | Keep: attributes resolved exactly as now |
| Arena: 4 x 128 KB regions, `takeover_reserve`, mesh fallback, 64 KB copy, 256-vertex world cap | 147 | Budget safety, D20C savestate region rule | D17R1, D20C (178) | Restructure (budget plan) |

## Proposed single rule set

1. **Ownership.** In Modernized, while the near path is live (eye, third
   person, widescreen), every polygon from the world renderer (its three
   authenticated callers) and the object renderer is drawn by the host,
   except sky callers, the first-person layer and fixed-projection limbs.
   The game still decides which meshes are drawn and how they are
   transformed, lit and animated; the original routine runs on the filtered
   copy, which is then empty of drawn faces. Vanilla never reaches this code.
2. **Clip and cull.** Near plane 16 and the angular guard band; each
   renderer's own backface rule.
3. **Vertices.** In-band corners keep the GTE's integer position (PGXP
   position with Corrected geometry), new corners are exact, every corner
   carries view depth and perspective UVs.
4. **Quads keep the native diagonal.** An unclipped quad is emitted as one
   four-corner packet (the GPU splits it along 1-2, like the original); a
   clipped one is cut as two native halves. This replaces the D17Z split and
   the fan triangulation.
5. **Sort key is the original's.** Every piece of a source polygon goes in
   that polygon's native ordering slot: world, the farthest corner SZ >> 5;
   objects, AVSZ3/4 with the face's own +-1 bias. No per-piece keys, no
   object bias, no depth-locality subdivision. Host geometry then
   interleaves with sprites, particles, blood and effects exactly as the
   original painted them, and the depth buffer resolves host against host.
6. **Depth.** Opaque pieces test and write depth at the raster plane. One
   contact rule: non-actor props move toward the eye by 2 units (16 for the
   verified payphone back); world and actors 0. Same-mesh coplanar overlaps
   keep the D17W rank. No other depth exceptions.
7. **Translucency (decision needed).** Today a translucent face drawn by the
   host writes depth (the GL path writes depth for every host-depth batch),
   while scene depth leaves translucent native packets painter-only. Proposed:
   translucent host faces take host clipping and precise vertices but no depth
   test or write, keeping native slot order, which matches the original and
   the current distant behaviour. No framework change: a negative host depth
   already means "no depth" (the weapon and sky use it).
8. **Overflow.** If a frame cannot afford a mesh, that mesh falls back to
   today's path (native packets plus scene depth), not to plain painter order.

## Cost of full takeover

Current renderer ("base") against `DNTTK_NEAR_TAKEOVER=all` ("all"). Bytes are
host packet bytes per live frame; budget is one 128 KB region (131072 bytes).
"Ring" is the game's own packet ring peak (139744 bytes available). Hook time is
wall time inside the near hooks per live frame on this machine (idle host,
load average under 1). Worker time is the 120-FPS redraw job average.

| Fixture | Mode | Rate | Peak bytes | Avg bytes | Frames over | Peak tris | Hook avg / peak ms | Ring peak | Worker avg ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Canal (UI 2), FOV 90 | base | 60 | 73616 | 9129 | 0 | 1889 | 0.48 / 4.9 | 92784 | - |
| | all | 60 | 166568 | 24354 | 31 | 4205 | 0.80 / 7.4 | 17848 | - |
| | base | 120 | 73336 | 8848 | 0 | 1871 | 0.49 / 5.1 | 94148 | 12.1 |
| | all | 120 | 164976 | 30476 | 34 | 4158 | 1.04 / 8.0 | 18408 | 10.1 |
| Canal (UI 2), FOV 120 | base | 60 | 74744 | 7381 | 0 | 1901 | 0.48 / 5.1 | 106684 | - |
| | all | 60 | 178704 | 23770 | 25 | 4500 | 0.91 / 7.5 | 19928 | - |
| Western street (UI 5), FOV 120 | base | 60 | 53676 | 16057 | 0 | 1473 | 1.02 / 6.5 | 72236 | - |
| | all | 60 | 163576 | 52270 | 116 | 4138 | 1.75 / 7.5 | 2600 | - |
| Western canopy (UI 6), FOV 90 | base | 60 | 49384 | 15429 | 0 | 1381 | 0.64 / 3.9 | 29388 | - |
| | all | 60 | 105564 | 25760 | 0 | 2805 | 0.88 / 4.2 | 1080 | - |
| | base | 120 | 52480 | 16369 | 0 | 1483 | 0.69 / 4.1 | 29416 | 5.6 |
| | all | 120 | 110000 | 26312 | 0 | 2921 | 0.95 / 4.5 | 932 | 5.5 |
| Club exterior (UI 11), FOV 90 | base | 60 | 36000 | 7734 | 0 | 900 | 0.32 / 3.0 | 30456 | - |
| | all | 60 | 86160 | 12355 | 0 | 2154 | 0.42 / 4.2 | 40 | - |
| | base | 120 | 24600 | 6940 | 0 | 615 | 0.32 / 3.1 | 53660 | 5.9 |
| | all | 120 | 125760 | 12504 | 0 | 3144 | 0.45 / 4.2 | 40 | 5.7 |

Peak demand by owner, FOV 120 (world / prop and water / actor / sky /
first-person layer, bytes): canal base 52520 / 4560 / 3880 / 18144 / 2560, all
151440 / 14400 / 2640 / 18744 / 2560; Western street base
28800 / 8160 / 2000 / 18156 / 5320, all 120280 / 37584 / 2440 / 18156 / 5360.

Findings:

- **Packet bytes.** Club and canopy fit one region with full takeover. The
  canal and the Western street do not: peaks of 164-179 KB, 25-34 canal
  frames and 116 street frames over 128 KB. World geometry is most of it.
  Takeover empties the game's own ring (canal 107 KB to 20 KB), so total
  packet memory per frame only grows about 1.5x. Host packets are triangles
  of 40 bytes, so a native 52-byte GT4 quad currently costs 80 bytes. The
  host path barely splits at distance (canal: about 0.75 triangles per drawn
  source polygon or native half), so rule 4 (one packet per unclipped quad)
  is the main saving; I estimate roughly a third of world bytes, to be
  measured in D17AM2.
- **The GPU DMA aperture is full.** It is 1 MiB and ends at the 24-bit OT
  link limit (0x00F00000-0x00FFFFFF). Allocated: host arena 4 x 128 KB,
  Bio Mask arena 4 x 47 KB, animation overrides 320 KB, plus small menu and
  loading regions: about 1020 KB. The host region cannot simply grow.
- **CPU.** Live-frame hook time rises 0.1-0.7 ms on average (worst average
  1.75 ms, Western street) and the peak from 3-6.5 ms to 4.2-8.0 ms. Peaks
  are single frames and may include load or first-draw work; the averages
  are the reliable figure. 120-FPS worker job time showed no measurable
  increase (differences within run-to-run noise), no replay failures and no
  misses. Offscreen present cadence is not comparable to the D23 budgets
  (p95 about 25 ms in both modes in this harness), so D23 compliance still
  needs D17AM3's runs.
- **Replay stream (risk, not yet hit).** Each host vertex adds a 10-word
  record to a 120-FPS worker's output, which is capped at 192K words
  (768 KB, `render_worker.c`). At the canal's full demand (about 4500
  triangles) host records alone would be about 540 KB plus 180 KB of
  packets and the native stream. These runs could not reach that because
  packets past 128 KB were not written. D17AM2 must measure stream
  occupancy and size it (framework, unhashed file) or shrink the records.
- **Images.** Static load views with takeover against the current renderer:
  club 0.002-0.005% of pixels differ, canopy 0.01%. The canal and street
  differ (7-9%) only where the budget ran out (the sky's clouds). Turn and
  walk images are not an A/B (input timing differs between runs).

## Budget plan (proposal)

In order, all inside D17AM2:

1. Emit unclipped quads as one packet (rule 4) and drop the depth-locality
   subdivision (rule 5). Measure the canal and street again.
2. Put the Bio Mask and frozen-tint packets in the host region (they are
   already linked per frame in the same rotation), freeing 188 KB of the
   aperture: four regions of 175 KB each, with no framework change, no
   change to the 24-bit links and the D20C region mirror kept.
3. Keep four regions; do not cut to three without evidence about GPU and
   replay reuse.
4. Size the worker output stream against measured occupancy.
5. Keep rule 8 (fall back to today's path per mesh) as the safety net, and
   count fallbacks in the debug JSON so D17AM3 can require zero on the
   fixtures.

Growing the aperture (moving its base below 0x00F00000) is possible but needs
framework and savestate-section work (D20C) and is not recommended unless
steps 1-2 fall short.

## Decisions requested

1. Approve the rule set, in particular rule 5 (native ordering slots for all
   pieces; retire the object bias and per-piece keys).
2. Rule 7: translucent host faces painter-only (no depth), or keep today's
   depth-writing translucent pieces.
3. Actors: include them (measured here; they are a small share), or keep
   actors on their current boundary for D17AM2.
4. Approve the budget plan, including moving the Bio Mask arena into the host
   regions.

## Limits

Offscreen 16:9 first person only; no third person, 4:3 or live-window runs.
Four fixtures (no level-select sweep, no mirror room). Timing is one machine,
one run per cell. Demand past 128 KB is counted, not drawn, so the cost of
actually writing those packets is not included in the hook times. The
rule-set fidelity claims (sprites interleaving as the original) are design
reasoning to be verified in D17AM2/AM3, not measured here.
