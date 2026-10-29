# D17AL - Embedded prop contact depth (slot-12 payphone)

2026-10-29. Done, user-accepted. User: "absolutely perfect. lock it in and
close us out". Regression from D17AE scene depth
([note 322](322-d17ae-ceiling-investigation.md)); related D17AC
([note 292](292-d17ac-phone-wall-depth.md)).

## Symptom and cause

User slot 12 (new save): walking toward the payphone, its front unit is
missing at a distance and pops in close. The phone's rear panel is embedded
about 10-11 units in its wall (D17AC). Two paths draw it:

- Close (any vertex under 3072): host prop pieces. `draw()` moves each
  piece's depth plane toward the eye by the prop contact allowance: 2 units
  (`DNTTK_PROP_CONTACT`), or 16 for the verified phone back from its open
  side (D17AC). The panel wins over its wall.
- Farther: the game's own packets with D17AE scene depth, which gave every
  corner its exact view depth and no allowance. The embedded panel lost to
  the wall, so it disappeared until the near path took over.

## Change

`recomp/src/ttk/near_clip.cpp`:

- The allowance is one function, `prop_contact_shift(d, phone_back)`, used by
  both host pieces and scene depth. Host-piece behaviour is unchanged.
- `scene_depth_begin` records whether the mesh is a non-actor object mesh
  (the same set whose host pieces get the allowance) and whether it is the
  verified payphone shape at the static-prop caller (the existing D17AC
  `phone_mesh` check).
- At flush, each matched opaque prop packet gets the plane of its corners in
  the host piece's corner order (0,1,3 for quads). Moving that plane by the
  allowance along its normal scales every corner's depth along its eye ray by
  `1 - shift/d`, so the packet's depth matches what its host piece would get.
  The rear panel is the FT4 packet whose corners match phone vertices
  0,4,2,6. Warped quads, like host pieces, get no allowance.
- Packets, ordering slots, UVs, colours and perspective weights are
  unchanged. World meshes, actors, Duke's body, the first-person layer, sky,
  translucent packets and Vanilla are unchanged.
- Developer comparison: `DNTTK_SCENE_PROP_CONTACT=0` restores exact depth for
  prop packets (the D17AE behaviour). `DNTTK_PHONE_BACK_DEPTH=0` and
  `DNTTK_PROP_CONTACT` now apply to both paths.

No framework, generated C or asset changes.

## Verification

Private copies only: current UI slot 12 (state SHA-256 prefix
`6a46686597251c91`) and current UI slot 11 copied into
`recomp/analysis/d17al/fixture`; the other slots come from the D17AK fixture
set. Every probe run checked player saves, cards and settings by hash and
mtime before and after: all unchanged. Offscreen 16:9 captures, first person,
corrected geometry/textures, probe `recomp/analysis/d17al/probe.py`
(`APPROACH=1`: 10 forward steps toward the phone).

- Reproduced in the new build with `DNTTK_SCENE_PROP_CONTACT=0` (FOV 90,
  60 FPS): the phone's front unit is missing in steps 0-3 and appears at
  step 4; only the surround is drawn. With the fix, the same route shows the
  whole phone at every step (`crops.png`, `old`/`new`).
- Fixed approach routes, whole phone at every step: FOV 64, 90 and 120 at
  60 FPS, and FOV 64, 90 and 120 at 120 FPS.
- Same-build static A/B (fix off/on) at FOV 64/90/120: slot 11 (D17AE sign
  post) pixel-identical, post hidden; slot 6 (D17AG canopy) differs by at
  most 48 pixels, a one-pixel shift along a building silhouette against the
  sky; slot 12 differs only at the phone.
- Slots 11 and 6 forward/back rocks at FOV 64/90/120 (60 FPS) and FOV 90
  (120 FPS): post hidden, canopy continuous, as in D17AE.
- 12-step turn sweeps on slots 5, 6, 8, 10 and 12 (FOV 90): no prop showing
  through a wall, no z-fighting or missing faces seen. Turn timing differs
  between runs, so this is visual inspection, not a pixel A/B.
- 120-FPS replay: 0 misses and 0 failures on all four 120-FPS runs.
- `ttk-near-test recomp/disc/SLUS_005.83`: new contract "D17AL scene-depth
  prop contact" (a distant native phone's rear-panel packet carries 16 units,
  another face 2, a modified shape 2, Vanilla no host depth, ordering table
  untouched) plus all existing contracts pass, including with
  `DNTTK_PHONE_BACK_DEPTH=0` (rear panel then 2) and corrected geometry with
  original textures.

## Limits

- Pre-existing, not caused by this change: with
  `DNTTK_TEXTURE_PRECISION=corrected` the test stops earlier, at the D17Z
  assertion `clipped==(shift!=0)` (line 252). It fails the same way at HEAD
  without this change, so the new contract was not reached in that
  combination. `DNTTK_SCENE_DEPTH=0` also stops at the D17AE contract, which
  expects scene depth on.
- Offscreen 16:9 first person only; no 4:3, third-person or live-window
  playtest. Other phone placements and other embedded props were not
  individually checked. Vanilla is covered by the contract gate, not a
  campaign play.
- Distant props now get the same 2-unit contact allowance their host pieces
  already had. A prop within 2 units behind a surface could in principle
  show through at a distance; none was seen in the sweeps.
