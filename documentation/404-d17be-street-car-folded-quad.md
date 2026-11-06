# D17BE - TIME TO KILL street car: the sliver under the cabin

2026-11-06. Status: **Done, user-accepted** ("lock it in and close us out").

User: "can you check slot 10, there's a small sliver that disappears near the
rear left wheel of this car. i'm not sure if it's a culling thing or what."
UI slot 10 (runtime slot 9, saved 16:41) is TIME TO KILL (level 0). Duke
stands on the ledge above the street and looks down at the wrecked car, with
the crosshair on the gap between the cabin's lower corner and the bonnet's
side.

## Method

Private copies of the player's cards and settings, offscreen, player files
hash-checked by every run. Harness `recomp/analysis/d17be/probe.py` (from the
D17BD probe; new case `looktr`: a primitive trace and a shot per camera
step), plus `meshes.py` (meshes with emits in a screen box), `draw.py`
(traced emits over a shot) and `faces.py` (each face's screen NCLIP of both
halves against the host decision). Fine camera sweeps of 1-2 mouse units per
step, left, right, up and down from the saved view.

## Cause: a folded quad culled by its first half

The car is one static prop mesh, `0x80126350` (24 vertices, 20 quads and 4
triangles; instance `0x801d8124`, static-prop loop `0x80032288`), drawn by the
unified renderer. Face `0x8012655c` (corners 2, 3, 12, 13) is the panel in the
gap under the cabin. It is not planar: it is folded along its diagonal. At
the saved view its first half (0 1 2) faces away (screen NCLIP -110) and its
second half (1 3 2) faces the eye (+633).

The original culls an object quad by its first half alone, and the object
path (`front_facing(..., false)`) copies that rule. So the whole quad was
dropped, including the half that faces the camera, and the street showed
through the gap. As the view turns, the first half flips sign and the panel
pops in and out: the disappearing sliver. The game's own packets
(`DNTTK_NEAR_CLIP=0`) show the same gap at the saved view, so this is the
original game's cull, not a renderer regression. The world path already
accepts a quad by its second half (the original world routine's own rule) and
draws a repeated-corner quad as one triangle (D17AV). The object path had
neither.

The car's other 12 culled faces have both halves facing away (genuine back
faces).

## Fix (Modernized, unified renderer)

`near_clip.cpp` object path: when a quad of a unified scene mesh (not actors,
Duke's body, the first-person layer or the sky) fails the first-half test but
its second half (1 3 2) faces the eye, it is drawn as that one triangle,
with that half's corners, UVs and colours. The back-facing half stays culled,
so no surface is shown from behind. Trace event `folded_half`, counter
`folded_quads` in the near-clip stats. `DNTTK_FOLDED_QUADS=0` (developer) keeps
the original cull. Vanilla and Classic do not use the unified renderer and are
unchanged. No framework edit, no new hook, codegen unchanged.

## Verification

Executable `bb86f81ba91c4aa25dde2859cab250d6422ef08874c90e6e9ab094b648d453ec`.

| Check | Result |
| --- | --- |
| Saved view, fix off/on | street in the gap before; the car body's side panel after |
| Fine sweeps (16 steps right, 16 up) | before: the gap shows the street at every step; after: the panel in every step, no popping |
| Faces caught at the saved view | one, `0x8012655c` (main pass and each redraw worker) |
| Other save slots (runtime 0, 2-8, 10), fix on vs off | no change except animation timing (fire, blood, the boss, the axe swing) |
| Vanilla, slot 9, on vs off | identical |
| `ttk-controls-test` (owned EXE, LEVEL00 fixture, LEVEL01, all levels) | PASS |

`ttk-near-test` does not link (missing `cowboy_hat_mesh` symbols, as noted
in D08A36); not run.

Before/after images for the README: `research/before-after/d17be-street-car/`
(local).

## Limits

- Judged at the saved view and its sweeps. Street level around the car was not
  walked.
- The rule is game-wide for static props. Other folded prop quads now draw
  their front half too. None showed up in the other slots' saved views.
