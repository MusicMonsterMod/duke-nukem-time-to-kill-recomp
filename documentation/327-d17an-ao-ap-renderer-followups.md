# D17AN/D17AO/D17AP - Renderer follow-ups after unified geometry (and D14F re-check)

2026-10-29/30. Selected batch after the D17AM3/AM4 playtest: D14F, then
D17AN, D17AP, D17AO. Private copies only: the D17AM3 playtest fixture
(`recomp/analysis/d17am3/playtest-fixture`, UI slot N is savestate N-1) and
the D17AM3 sweep fixture. Harness: `recomp/analysis/d17an/` (`repro.py`,
`trace_hole.py`, `cover.py`, `border.py`, `faces.py`) and the D17AM3 sweep
scripts. Frozen executables: `build-local/*.an-head` (HEAD before this
work), `*.an-cand` (first candidate), `*.an-cand2` (final). No commit, push
or acceptance implied.

## D14F - already implemented (Needs playtest)

The board still said "Changes: none at registration", but D14F landed in
afd4ad6 (note 317). Re-verified: `first_person_fov.h` default 90, range
60-120; the DISPLAY slider 60-120, default 90, one-degree steps; launcher
and profile fallback 90 with explicit saved values preserved; Vanilla still
forces 64; manual consistent; Python profile suite 111 tests pass. No code
change. Remaining: the user's in-game check.

## D17AN - slot 12 water over the bridge deck (fixed)

Reproduced on a first-person pan from the load view: water ripples painted
over the bridge deck, most at the start of the bridge. Also seen with
`DNTTK_UNIFIED_GEOMETRY=0` and `DNTTK_NEAR_CLIP=0`, and in Modernized third
person.

Cause: water tiles come from their own FT4 builder (0x8002ef90) and keep
painter order in their native slot. A tile under the bridge whose nearest
corner is close to the eye sorts nearer than the deck above it, so painter
order lays it over the deck. Only the D17AF tiles beside Duke (skipped by the
original) were depth-tested.

Fix (`water_clip.inc`): with unified geometry (Modernized, live eye view),
every water tile is taken and depth-tested at its surface pushed 32 units
back, the D17AF rule. Scenery above the water now wins, and scenery just
under the surface (the slot 10 grating) is still drawn over it, as the
original paints it. `DNTTK_WATER_DEPTH=0` (developer) restores painter order
for tiles the original builds itself.

Budget: the first candidate took about 2.7 times as many tiles in the canal
(UI 2), used up the host packet budget and caused 387 near-mesh fallbacks
in a 30 s route (0 at HEAD). A tile taken only for its depth (safe, not
oversized, not skipped) now leaves the D17AM3 48 KB far reserve to near
meshes, like a distant mesh. Short of that it stays native in painter order
(new counter `near.water.budget_native`). Canal, final build: 0 fallbacks in
two runs, about half the tiles left native there. Slot 12 bridge: 0 left
native, deck clean.

## D17AP - slot 9 grave soil corners (fixed)

Reproduced: a slow pan with the view tilted down shows triangular notches
at the soil mound's near corner (baseline). Cause: unified geometry draws a
quad wholly inside the clip planes as one packet. For a warped quad (four
corners not on one plane) that packet has no depth plane
(`if(warped) local.depth_plane={}`), so snapped corners take raw vertex
depth and cut into the floor. This is the D17X mechanism, which the unified
whole-quad path bypassed.

Fix (`near_clip.cpp` `draw()`): with unified geometry a warped quad is drawn
as its two native halves (0-1-2, 2-1-3), each with its own plane depth, as
D17X does elsewhere. Planar quads stay one packet. `DNTTK_WARPED_QUADS=0`
(developer) restores the single packet. Same pan, final build: corners
clean.

## D17AO - slot 11 ceiling hole (diagnosed; authored gap, needs decision)

Reproduced: jumping in first person shows a thin wedge of sky in the stone
ceiling. The same wedge appears with unified geometry, the earlier renderer
(`=0`), native (`DNTTK_NEAR_CLIP=0`), this morning's pre-afd4ad6 build
(`d20h-baseline`), and with each of `DNTTK_PRECISE_CULLING=0`,
`DNTTK_OPEN_PORTAL_TOP=0`, `DNTTK_PORTAL_MARGIN=0`, draw distance Original
and `DNTTK_SKY_PAINTER=0`. Modernized third person shows it even standing.
It is not a regression from the D17AM work.

Primitive trace of a frame mid-jump (frames 204-205 form one game frame):
the wedge pixels are covered only by the sky dome (mesh `0x8013f870`,
painter order). No world face covers them, drawn or culled. The wedge lies
between two room meshes, `0x80192f94` and `0x801bb994`. Around vertex 11 of
the first mesh, the face fan is open between edges 11-4 and 11-3. Around
vertex 54 of the second, it is open between 54-65 and 54-64. Vertices 3/64
and 4/65 are the same two points, one about 105 units above the other at
about 2200 units from the eye. Together they leave a lens-shaped strip,
about 3260 units long and up to 105 tall, that no face closes. The room
walk draws the same nine room meshes in every traced frame, so no room is
being skipped.

Conclusion: an authored gap in the level data. The original camera
evidently never sees it from its position (Vanilla's camera stays behind
Duke against the wall here). The independent first-person and third-person
camera does. Closing it would need a level-data patch or a generic
gap-sealing rule; neither is attempted without the user's choice.

## Validation

- `ttk-near-test disc/SLUS_005.83`: unset and `DNTTK_UNIFIED_GEOMETRY=0`,
  each with Corrected and Original precision, pass. New contracts:
  "D17AN unified water" (a safe distant tile is depth-tested at the surface
  32 units deep; Vanilla untouched) and the warped prop quad now requires its
  two native halves with positive plane depth. Both fail with
  `DNTTK_WATER_DEPTH=0` and `DNTTK_WARPED_QUADS=0` respectively.
- `ttk-input-test`, `ttk-aim-test`, `ttk-sky-test`, `ttk-gas-test`,
  `ttk-live-env-test`, `ttk-loading-test`, `ttk-controls-test` (clean
  `d08-camera-final` LEVEL00 fixture, LEVEL01, all levels) pass. The
  `d08p-turret` LEVEL00 copy is modified and fails the controls guard
  check; do not use it.
- A/B sweep, HEAD against the first candidate, 33 scenes, fp60 (FOV
  64/90/120 and Bio Mask) and tp60, 656 image pairs: median difference 0.0%
  of pixels; 65 of 524 fp60 and 9 of 132 tp60 pairs at or over 0.5%. The
  worst pairs reviewed are game-state divergence (death screen against the
  continue menu, a moved camera, explosion and projectile phase) and the
  water's animation phase. No water, floor or geometry was missing. Counters
  are clean in both.
- Routes, final build, all 33 scenes, 120 FPS, FOV 120, plus Bio Mask legs:
  47200 frames, 0 near fallbacks, packet skips, copy overflows, replay
  failures or misses (16665 downgrades of distant meshes).
- Timing, one game at a time, 30 s route, 120 FPS, FOV 120 (emulated fps,
  hook ms avg/peak, worker ms avg/max):

| Scene | Build | Emulated fps | Hook ms | Worker ms | Near fallbacks |
| --- | --- | --- | --- | --- | --- |
| Canal (UI 2) | HEAD | 58.35 | 1.30 / 7.6 | 10.1 / 27.5 | 0 |
| | First candidate | 59.12 | 1.46 / 8.2 | 10.0 / 28.7 | 387 |
| | Final (2 runs) | 58.3 / 58.7 | 1.74 / 8.0, 1.71 / 7.8 | 12.1 / 23.8, 12.6 / 28.2 | 0 / 0 |
| Slot 10 channel (UI 10) | HEAD | 59.94 | 0.38 / 3.4 | 4.7 / 15.6 | 0 |
| | First candidate | 59.94 | 0.46 / 3.6 | 4.9 / 15.2 | 0 |

The final build's canal hook and worker averages are about 0.4 ms and 2 ms
above HEAD (more host water packets). Emulation keeps up.

## Limits

- Offscreen captures; the user's live 120 Hz playtest remains.
- In scenes over budget (the canal) part of the water keeps painter order,
  so a deck-over-water fault like D17AN could still appear there.
- The 32-unit water allowance is still tuned on slot 10 only.
- D17AO is diagnosed from one traced frame and the geometry around it; the
  original console camera's view of this spot was not captured.
