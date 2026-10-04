# D17P distant horizontal black bands (subway, UI slot 8)

Status: **Done, user-accepted** (2026-10-04): "i fully accept this fix." The
user authorized documentation, commit and push.

Accepted executable SHA256:
`5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
Accepted D15 baseline (for comparison):
`35756df57b9a6ddd31ee0dabdffb51876faf7a3b0c05effb78be285dfd9dc9a7`.

## Result

The new Modernized **Draw distance** option (`extended`, the default for
Modernized) removes the bands: the subway corridor now ends in its real wall,
door and parked train, and the distant ceiling is continuous. `original` keeps
the original limits. Vanilla always uses the original limits.

## Cause: three original rendering limits, not a D15 regression

Vanilla shows the same bands, and so do Original and Corrected geometry and
textures at 60 and 120 Hz. Each band was traced to one of three original
mechanisms. Packet dumps (`gpu_frame_dump`) were rasterized offline to separate
undrawn pixels from drawn black pixels.

1. **Far limit (black box at the end of the corridor).** Each frame the camera
   render `0x8002e48c` copies the level's view limits into the render context
   `0x800d67a8`: `+0x6c` fade start (level header global `0x800da450`, 23000
   in the subway), `+0x70` mesh far limit (fade start + `0x13d0` = 28072),
   `+0x74` room/prop/actor far limit (the alternate global `0x800da34c` =
   33280 during the portal walk, otherwise `+0x70`) and `+0x78` = `+0x74`/4.
   Rooms past `+0x74` are not drawn at all; the region is the cleared
   background (no primitive covers it). Mesh colours fade toward the fog
   colour (black) over `0x1000` units past the fade start (DPCS at
   `0x800113e8`). At these distances that fade spans a pixel or two, so the
   cut-off reads as a hard black box with a dark line on its upper edge.
   Raising the limit at runtime revealed the corridor's real end.
2. **Portal rectangles (thin full-width black lines).** The portal walk
   `0x8006276c` lists each visible room with the integer screen rectangle of
   the opening it is seen through (`0x800d68b0`, 12-byte entries x, y, w, h,
   room; count `0x800d68a8`). The room walk `0x80062b48`/`0x80062b50` copies it
   to context `+0x18`/`+0x1c` and, when the bounds test `0x800365e8` reports a
   partial room, sets context flag `0x80` so the mesh renderer `0x80011020`
   records per-vertex rectangle outcodes. A face whose corners all lie beyond
   one edge is skipped (`0x8001160c`). A per-mesh trace showed the room at
   depth 27000 clipped to rectangle top -22 while its ceiling projects to
   about -24: every ceiling strip wholly above -22 was dropped, and the nearer
   room's ceiling ends about 1.3 native pixels higher, leaving the background
   showing through. The same happened at depth 33000. The faces are
   continuous in precise view space; the gap comes only from the rectangle.
3. **Thin distant faces (jagged partial lines).** A ceiling or floor cell one
   native pixel tall can have integer corners whose NCLIP determinant is zero
   or has the wrong sign. The renderer branches `0x80011894` (triangles) and
   `0x80011ab4` (quads) then skip it. Native rows with no submitted strip
   (for example 95-96 then 97-98) were found in the packet stream.

A fourth original path was identified and left alone: a quad with any corner
past `+0x70` is emitted as a flat black `0x28` quad (`0x80011cf4`), the
renderer's fully fogged surface.

## Implementation (Modernized `extended` only)

`recomp/src/ttk/draw_distance.inc`, entered from three new function-entry hooks
(regenerated `game.local.toml`): `0x8006276C` (ra `0x8002e650`), `0x80062B48`
(ra `0x8002e688`) and `0x8002FFEC` (ra `0x8002e6dc`). They run right after the
render writes its limits and before any reader. Two guarded ranges
(`0x8002e610`+`0xd0`, `0x80062b48`+`0x2c0`) must match the owned executable.

- Each far limit is doubled, capped at `0xffff` (the GTE depth range). The fade
  start moves by the same amount as the mesh limit, so the original fade still
  ends just short of the new limit. Values are derived from the level globals,
  never from the context, so applying twice is harmless. Mesh guards accept the
  larger values: the OT insert `0x8002bc18` clamps buckets to 0..2047 and
  `0x8002f754` rejects average depths past `0x4000`*4. The game itself writes
  `0xffff` limits on another path (`0x80025614`).
- The level globals are untouched. `0x800993d0` and `0x800a5b84` use them for
  squared-distance checks that may affect gameplay.
- Every portal rectangle grows by 2 native pixels, within the view rectangle
  `0x800d2210` (already widened by D14). Nearer walls around an opening are
  drawn later in OT order and still cover anything behind them. The
  props/actors pass reads the same, wider list.
- Precise culling (framework, `gte.cpp`, exported patch): when every NCLIP
  input has a validated PGXP shadow and the exact sign disagrees with the
  integer determinant, MAC0 takes that sign (+1/-1). Untracked inputs, stale
  shadows and suppressed passes keep the native value. It needs exact
  projection tracking, so it acts only when geometry or texture precision is
  Corrected; with both Original nothing is tracked and NCLIP stays native. The
  declarations live in `pgxp.h`, not `cpu_state.h`: the latter feeds the
  codegen hash, and changing it would reject every existing savestate.

Developer environment (not player options): `DNTTK_DRAW_DISTANCE_FAR=N`
(4096..65535) replaces the doubled limit, `DNTTK_PORTAL_MARGIN=0..8` (default
2), `DNTTK_PRECISE_CULLING=0` keeps native NCLIP. The near-clip primitive trace
now records context `limits` and `rect` per mesh. The `ttk_input` controls
report has a `draw_distance` object (mode, frames, refusals, limits, widened
portals, NCLIP checks and flips).

Player option: profile schema 24 adds Modernized `draw_distance`
(`original|extended`, default `extended`, migrated as `extended`),
`run.py --draw-distance`, settings menu **D**, launcher variable
`DNTTK_DRAW_DISTANCE` (always `original` for Vanilla).

## Evidence (private, local)

`recomp/analysis/d17p-subway-bands/`: four-combination and 60 Hz intake,
24-present walk sequences at 2x and 4x, Vanilla/aspect/view comparisons, packet
dumps and rasters, live limit reads, the far-limit experiment, per-mesh face
traces (`trace/`), the matched-view A/B (`proto/`), the slot sweep (`sweep/`),
the replay oracle (`oracle/`) and cost runs. Player files were never used; all
runs used private card copies, a private profile and port 9231.

- Matched views at 4x (Corrected/Corrected, 120 Hz): the margin alone removes
  the clean lines, precise culling alone removes the jagged lines, and both
  with the extended limit give a continuous ceiling and the real corridor end.
- All 12 private slots, Original vs Extended at 120 Hz, 4x, first person, CPU
  100%: game rate identical idle and turning (UI 4 and UI 11 were already
  20-22 fps idle in Original); ring peak at most 69584 of 139744 bytes (UI 11
  later club; subway 67016); zero near-clip budget hits, fallbacks, packet skips,
  replay misses or failed jobs. Idle images are unchanged except UI 8 (0.74%,
  the corridor end) and animation timing (UI 4 pig cop, UI 11 dancer).
- 120 Hz cost, 8 s turning: UI 8 112.8 vs 118.8, UI 11 119.1 vs 120.1, UI 4
  118.0 vs 120.0 distinct images/s (Original vs Extended), 30 fps game rate,
  zero misses, equal p95 present interval. Third person checked in UI 8.
- Vanilla with `extended` forced: original limits 23000/28072/28072/7018, no
  hook activity, no culling checks.
- Replay oracle (interpolation off): copy-on-write checks pass in both modes.
  Redraws occasionally differ from their real frame by a thin sliver
  (Extended 57-1034 pixels at 2x in some walking samples). Original shows the
  same class (41-174 pixels while turning), and Extended with neither the
  margin nor precise culling also does (374), so this is an existing replay
  limitation, not caused by D17P.
- NCLIP flips are 0-8% of tracked checks in most slots and 39% in the subway,
  whose distant cells are mostly one pixel tall.

Tests: Python 106 (104 pass, 2 conditional skips) including new schema 24
draw distance tests; `ttk-controls-test` (31 PASS lines with the Level 0
fixtures), `ttk-near-test` (owned packet contracts, digest
`b6dfbb5aad8fd86a`), `ttk-input-test`, `pgxp_test` and `gte_register_access_test`
(new precise culling case) pass. The framework's previously unregistered
`test_host_vertex_depth.py` was repaired (stubs for the current replay
recorder) and registered, because the recompiler's test-registration guard
otherwise blocks reconfiguration.

## Limits

- Accepted on the user's playtest; the automated and private checks above are
  the recorded evidence. Campaign-wide coverage is not established.
- A longer corridor can still end in black beyond twice the original limit.
- Thin-face rescue needs Corrected geometry or textures.
- Distant rooms now draw actors that the original never showed; whether those
  far actors animate depends on the game's own update rules.
- Campaign-wide coverage is not established.
