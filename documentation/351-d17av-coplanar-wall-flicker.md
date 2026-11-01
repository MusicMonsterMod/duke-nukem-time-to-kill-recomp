# D17AV - Polymorph: flickering triangles on corridor walls

2026-11-01. Status: Done, user-accepted 2026-11-01 after the second pass (see "Follow-up" and "Decision"). Polymorph only (Classic frozen at the
user's request, 2026-11-01); Vanilla never reaches this code.

User: in a new state in UI slot 8 (file 7, level 6), looking left and right
shows a flickering triangle on a section of corridor wall.

## Method

Private copy of the state and both cards in `recomp/analysis/d17av/fixture`
(player files hash- and mtime-checked by every run). Offscreen, 16:9, first
person, FOV 90, Polymorph, 60 Hz. Harness in `recomp/analysis/d17av/`:

- `route.py` - load the slot, pan left and right, shoot every step; `TR`
  traces one step, `TRALL=1` from that step to the end.
- `copl.py` - coplanar overlapping emitted faces of one traced frame with
  equal rank (the ones the depth test cannot separate).
- `ov.py` - outline traced faces over a shot.
- `ranks.py` - which face is on top on each of the two slot-8 wall panels, per
  traced frame.
- `scan.py`, `sweep.py` - static scan of every room mesh of a level (room
  table `0x800da454`, count `0x800da47c`, 88-byte rooms, pieces at +0x40) for
  coplanar overlapping faces, run over every selectable level.

## Reproduced

Panning in slot 8 shows two 1024 x 1024 wall panels, one each side of the
corridor (room 22, mesh `0x801aafa8`, at z 9216 and z 7168). Each panel is
built from several faces in the same place: quads and triangles with
different textures (left: quad tex 4 twice, triangles 1116/1117/1118 and
1933; right: quad tex 1112, triangles 1931 and 1932). In the shots
(`shots/pan`), the left panel shows a triangle of diagonal stripes or
stipple, the classic sign of two faces at the same depth fighting, and on
the right a triangle that comes and goes as the view turns.

## Causes

Polymorph draws world faces with the depth test. Faces in exactly the same
plane get the same depth, so D17W/D17AM2 give overlapping coplanar faces of
one mesh a small depth step by rank (`rank_coplanar_world`, `apply_coplanar`
in `recomp/src/ttk/near_clip.cpp`), ordered as the original paints them.
Three faults:

1. **A repeated-corner quad was never ranked (left panel).** The triangle
   that wins on the left (`0x801ab258`, tex 1933) is stored as a quad with
   indices 60, 79, 78, 60. In the edge order used to build its plane
   (0, 1, 3, 2) the first three corners are 60, 79, 60: no plane, so the face
   was treated as not flat and left out of every pair. It drew at rank 0
   beside the rank-0 triangle `0x801ab43c` in the same place: the stripes.
   (Such a quad passes the world cull from both sides, `front_facing`.)
2. **Counted ranks could tie (chains).** A face's rank was the number of
   faces painted over it. With A over B and B over C (A and C apart), B and C
   both got rank 1 and fought.
3. **Ties swapped with the view (right panel).** The original orders world
   faces by their farthest corner. A triangle made from three of a quad's
   corners can never sort farther than the quad, so it is on top unless both
   share the farthest corner; then they tie and record order puts the quad
   on top. Turning changes which corner is farthest, so the triangles came
   and went. The trace confirms it: quad `0x801ab320` ranked over its
   triangles in one frame (`pmt`), the triangles over the quad in another
   (`pan2`). Classic (the game's own painter order) shows the same symptoms
   (`shots/cls`).

## Fix (Polymorph)

`near_clip.cpp`:

- `coplanar_face`: a quad that repeats a corner keeps each distinct corner
  once and is ranked as the triangle it is.
- `apply_coplanar`: a face's rank is one more than the highest rank of the
  faces painted over it (longest path), so a chain never ties.
- `apply_coplanar`: on a tie in the original's slot, a face made from some of
  the other face's corners wins, so it stays on top in every view (the
  original's result whenever it does not tie). Otherwise record order, as
  before. The relaxation is bounded by the number of pairs.

Same depth step as before (`depth_plane.d *= 1 + rank/4096`). No framework
changes; no new switches (`DNTTK_NEAR_COPLANAR=0` still disables ranking).

## Evidence

- Slot 8 pans before and after (`z3.png`, `z4.png`): the stripes and
  stipple are gone; each panel shows one steady result.
- `ranks.py` over a traced 56-step pan (`pan4`, every frame from step 1):
  left panel the same winner in all 93 traced frames (`0x801ab258` on top,
  ranks 0/0/1/1/2), right panel the same in all 37 (triangles `0x801ab404`
  and `0x801ab424` over quad `0x801ab320`).
- `ttk-near-test` PASS; `ttk-controls-test` (`d08-camera-final` fixture,
  LEVEL01, all levels) exit 0.

## Other places (static sweep, every selectable level)

| | Levels | Count |
| --- | --- | --- |
| Repeated-corner faces newly paired (fault 1) | 6 only | 5 pairs |
| Overlap chains where counted ranks could tie (fault 2) | 1, 2, 5, 6, 9, 11, 25 | 20 meshes |
| Differently textured subset pairs that could swap with the view (fault 3) | 1, 2, 3, 5, 6, 7, 8, 9, 11, 25 | 144 pairs |

Levels 10, 12 and 21-24, 26-29 have none. Level select excludes 4 and 13-20,
so they were not scanned. All of these use the same three rules, so the fix
covers them. Only slot 8 was looked at in game.

## Limits

- Only slot 8 was checked visually; the other 144 places were found by the
  static scan and not visited. Not seen at 120/180 Hz or in third person.
- What the panel triangles are meant to be (shading, damage, variant art) is
  not known. The fix keeps the original's usual winner, not a judgement of
  which art is intended. If the user would rather a steady triangle were not
  shown at all, that would need a separate rule.
- Faces of different meshes (adjacent rooms) are not ranked against each
  other; the slot-8 frame had only sliver overlaps there (under 500 square
  units), and none showed.

## Follow-up: the triangle was a shading break (2026-11-01)

User playtest of the first pass, clip
`research/screencaps/Video_2026-11-01_16-30-27.mp4`: "the issue is still
present but it's not as flickery". Frame by frame (60 fps, crops in
`vid/`), each panel now kept one winner, as the trace said, but a bright
triangle with a hard diagonal edge stayed in view. Overall brightness was
steady (no light flicker), and the face entries are not animated
(`tex.py`).

What the triangle is:

- **The extra faces are duplicates.** Right panel: quad tex 1112 and
  triangles 1931/1932 have the same palette (`0x0cf8`), page and texture
  coordinates at every corner. Whichever wins, the texture is the same.
- **One lit corner, two shading triangles.** Corner 50 of the right panel is
  `0x524833`, the others about `0x323232` (`col.py`; static over 20
  samples). The GPU shades a quad as two triangles on its 1-2 diagonal, so
  all the light sits in one triangle and stops at the diagonal. The original
  does the same.
- **The left panel's front is two triangles.** From the front, the quads
  (tex 4) are back faces; the front is triangles (79,60,78) and (60,79,61)
  (corner 60 `0x393939`), so the break is in the level data. Over half of it
  lay `0x801ab258`, a back-side triangle (its winding faces away, its
  texture tile and palette `0x0d38` are the back quad's) stored as a quad
  that repeats a corner. The world cull accepts a quad from behind when its
  second half faces the eye (`front_facing`), so this triangle showed from
  the front too, with the back's tile: a texture seam on the diagonal.

### Fix, second pass (`near_clip.cpp`, Polymorph world geometry)

- **Smooth quad shading.** A flat world quad whose corner colours do not
  blend evenly (twist `c0+c3-c1-c2` of 2 or more in any channel), and whose
  fourth corner's texture coordinates follow from the other three (one
  texture map over the face), is drawn as four triangles meeting at its
  centre, which takes the mean position, texture coordinates and colour of
  the corners. The shading blends across the face instead of breaking on
  one diagonal; outer edges and texture are unchanged. Quads whose two
  halves map the texture differently (D17Z) keep the native halves: a first
  version without that check visibly moved the texture on a near pillar.
- **Triangle pairs joined.** Two opaque triangles of one world mesh with a
  shared edge, in one plane, forming a convex quad, with one texture map and
  the same rank and draw state, are drawn as that quad when the shading
  would break (so it is smoothed as above).
- **Duplicates under the whole face.** In the coplanar ranking, a face that
  only repeats part of a bigger one (subset of its corners; same palette,
  page, colours and texture coordinates at shared corners) loses to the
  bigger face, so the smoothly shaded whole is on top. A subset with
  different art still wins on a tie (first pass).
- **Repeated-corner quads culled as their triangle.** Such a record is drawn
  as the GPU triangle it makes and culled by that triangle's winding, so a
  back-side triangle no longer shows from the front.
- Developer: `DNTTK_SMOOTH_QUADS=0` turns off smooth shading and joining;
  console `smoothquads on|off|default` overrides it in live frames (A/B
  shots at 60 Hz; redraw workers keep the launch value).

### Evidence

- Slot 8, same views (`sm10.png`, `sm11.png`): both panels show one even
  wall; no triangle.
- Same-view A/B with the console toggle over a 49-step pan (`ab3`) and two
  other levels (1 and 5, `lv1`, `lv5`): the only changes beyond soft
  shading are moving actors and an effect sprite; no texture or edge
  changes.
- Cost, slot 8 corridor: host packet bytes peak 76 to 96 KB, average 32 to
  43 KB, no packet skips; at 180 Hz the redraw worker job average 9.0 to
  9.5 ms, maximum unchanged (19.4-19.6 ms), cadence p95 unchanged
  (6.3-6.4 ms), no misses or failures (`perf0`, `perf1`).
- `ttk-near-test` PASS; `ttk-controls-test` (`d08-camera-final` fixture,
  LEVEL01, all levels) exit 0.

### Limits (second pass)

- A dotted hairline at a boundary between two rooms' ceiling faces
  (`0x801ab370` and `0x801ab698`; their edges miss by about 0.05 native
  pixels) showed in three shots taken right after a turn with smoothing on.
  A shot from the same view 0.3 s later showed no hairline, with smoothing
  either on or off (`abl1`, `abl2`). It was not reproduced in the 49-step
  same-view A/B either, so smoothing is not shown to cause it. If the user
  sees ceiling hairlines, treat it as a cross-room seam.
- The first pass's level sweep counted subset pairs by texture entry
  number. Slot 8's turned out to be identical texture maps under different
  numbers, so many of the 144 are likely duplicates (now smoothed under the
  whole face) rather than different art.
- Other levels' lit corners were not visited one by one; the change applies
  to every Polymorph world quad that meets the conditions.
- Not seen on the user's display; 180 Hz was measured offscreen.

## Decision

User, 2026-11-01, after playing the second pass: "it is now fixed! great
work. superb". D17AV is Done and closed. Classic is frozen; renderer work
continues on Polymorph only.
