# D17BA - Open models show their inside

2026-11-02. Status: Done, accepted by the user 2026-11-02 ("fixed! accepted").
Polymorph only; Classic and Vanilla are
unchanged.

Evidence is local under `recomp/analysis/d17ba/` (`probe.py`, `dump.py`,
`sweep_open.py`, `rule2.py`, `perf.py`, `diff.sh`; shots, sheets, JSON,
`changed.json`). Every run used a private copy of the player's card and
savestates (UI slot 12 = file slot 11), offscreen video, dummy audio and
private debug ports. Each script hash-checks the player's saves and profile
before and after the run.

## Cause (confirmed)

The user's hypothesis holds, with cause 3 of the job folded in: the barrel has
no inside geometry, so its inside can only be the back sides of its outer
walls, which are culled.

- **The model.** Object `0x801d9864`, type 220, renderer kind 1, mesh
  `0x8012cb90` in the object renderer `0x80010000` layout: 20 vertices, ten
  FT4 faces. Six one-sided faces form a hexagonal shell from y -122 (rim) to
  374 (floor), with no lid, no bottom and no inner faces. Four double-sided
  animated faces (y -122 to -374) form the flame cross standing on the rim.
- **The cull.** A primitive trace of the up-close view (`shots/a`) shows the
  host drawing the two near shell faces and the four flame faces, and culling
  the four far shell faces as back faces. This is the original rule: the
  faces have no double-sided bit, and NCLIP drops them on the PlayStation as
  well. Through the open top the floor and wall behind the barrel showed
  where its far inside wall should be.
- Cause 2 of the job (a host culling or near-clip difference) is not
  involved: the host decision is the original one.

## Change

`recomp/src/ttk/near_clip.cpp`, `open_model_faces_of` and the back-face test
in `object()`.

While unified geometry draws a static scene prop (Polymorph; not actors,
Duke, the first-person layer or the sky), an **open** model's inner faces
skip the back-face cull and are drawn from both sides:

1. **Open.** An edge of a one-sided face (by corner position, quads in the
   GPU's 0-1-3-2 order) has no reversed twin in another one-sided face, and
   the edge is not at the model's base (its lowest corners, standing on the
   floor). Double-sided faces do not close a model.
2. **Inner face.** Of an open model, a one-sided face gets its back side when
   that side faces the model's centre (an inner wall; a flat panel or sign
   does not), it is opaque, and no opposite-facing face lies within 8 units
   of its plane (a back-to-back panel, which would fight it).

The inside shows the face's own texture, colour, ordering slot and host depth,
so a back side stays behind the model's front faces and behind nearer walls.
The result is cached per mesh with a checksum of its corner positions and
face corner lists, so a level load that reuses the address rebuilds it.
Developer switch `DNTTK_OPEN_MODELS=0` keeps the original cull. Counters:
`open_models` in the near-clip JSON (`builds`, `open`, `faces`, `drawn`).
Trace event `open_back` marks each back side drawn.

When a mesh falls back to the original renderer (a frame that cannot afford
it), NCLIP culls its back faces as before. Classic does not run unified
geometry (`run.py` sets `DNTTK_UNIFIED_GEOMETRY=0`), and Vanilla never reaches
this code.

## Evidence

| Run | Result |
| --- | --- |
| Slot 12, walking up and looking down, before (`shots/a`) | Floor and wall show through the open top. |
| Same, after (`shots/b`, `shots/ab-crop.png`) | The far inside walls are drawn, with the barrel's texture (mirrored). |
| Walking around it, first person, 180 Hz (`shots/around`) | Inside drawn from every side seen. |
| Walking around it, third person, 180 Hz (`shots/around-tp`) | Inside drawn from every side, including from above. |

Level sweep (`sweep_open.py` then `rule2.py`, all 21 selectable levels, the
mesh +0x44 of every object in the room lists, the rule applied offline):
1054 level-mesh entries (4393 objects); 488 are open by rule 1; 413 get inner
faces (9162 faces, 1946 objects). All are kind-1 static objects. Per level
(meshes / objects / faces): 0: 33/90/641, 1: 46/160/887, 2: 29/219/750,
3: 23/229/562, 5: 54/140/876, 6: 32/90/574, 7: 20/216/366, 8: 34/158/589,
9: 60/155/1597, 10: 27/244/633, 11: 27/176/525, 12: 27/68/1125, 29: 1/1/37;
none in levels 21-28. The full list is `changed.json`. Most of these are
low-poly props without a back or lid; their inner faces are hidden by their
front faces and show only through an opening.

On/off screenshots, all 12 fixture saves, four headings each (`diff.sh`):
the stationary differences are the barrel (UI slot 12), a hanging lamp's
inner glass (UI slot 1), a wagon's wheel boxes (UI slot 6) and a few single
pixels. No
see-through, z-fighting or wrong-side face was found. Whole-frame differences
in some turned shots come from turn and timing nondeterminism (one is a HUD
message), not from the rule.

Cost at 180 Hz, slow turn, 15 s (`perf.py`, on / off): host hook time per game
frame UI slot 12 (D17AZ fixture) 505/467 us, UI slot 11 572/559 us, UI slot 3
375/305 us; prop
triangles per frame +6% to +22%; game frames and fields unchanged (60 fields
per second; game frames within 2%); no mesh fallbacks or packet skips.

Tests: `ttk-near-test`, `ttk-input-test` and `ttk-controls-test` (SLUS
executable and the LEVEL00 guard fixture) pass.

Executable `d8e1cac1c50bb38558ac65b83d8434e8233c06cb5e9ddba888b2a96f3385f107`.
No framework change and no regenerated code.

## Limitations

- The user confirmed the barrel in game; the other 412 models were checked
  only through the sweep and 48 sample shots.
- The inside uses the outer texture and lighting; a decal on the outside
  (the barrel's grey spot) shows mirrored on the far inside wall.
- Actors keep the original cull (open limb ends stay open, as before).
- A model whose opening is at its base is treated as closed; a prop hanging
  from a ceiling with an open top counts as open, which costs draws but shows
  nothing wrong.
