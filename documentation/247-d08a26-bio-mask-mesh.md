# D08A26 - Bio Mask first-person mesh (accepted)

The user approved the low-poly Blender shape and requested game integration on
2026-10-25. The user accepted the mesh and final native-lighting pass and authorized
commit, push and closeout on the same date. D08A26 is Done.

The original 2400-triangle model uses exact mirrored eight-sided hoses, ribbed
couplings and flat-shaded steel. Photon prepares a 256x128 atlas from CC0 Poly
Haven photographs. Editable Blender/Photon sources, source photographs, hashes,
credits and exporters are in `recomp/assets/models/biomask/`. No generated-image,
retail or UltimateDoomVisor pixels ship with this asset.

## Rendering

`weapon_draw_bounds` appends the mask after Duke's weapon and before the native
HUD. It requires Modernized first person, a settled first-person blend, live
player identity, normal gameplay, positive health/charge, native active-mask bit
and mask face model. It does not change item behavior, protection or sounds.
It follows the existing `bob_dip` at restrained amplitude. Replay workers load
and interpolate this bob with the existing presentation alpha. Bob off gives
zero motion; no separate animation clock is introduced.

The mesh emits ordinary textured shaded triangles with precise vertex depth.
An exact reserved tpage/clut pair selects an immutable host texture in the GL
and software renderers, without occupying retail VRAM or latching texture state.
The atlas uses nearest sampling regardless of world texture filtering. Normal
weapon ordering and the HUD's later slot remain intact. Framing adjusts for
4:3 versus widescreen without stretching the model.

Backface rejection emits 1174 triangles / 46960 packet bytes. The four 47 KiB
packet regions coexist with the world's four 128 KiB regions plus its 16-byte
mirror and the 320 KiB animation override allocation within the 1 MiB aperture.
Allocation occurs after animation setup, before worker fork or first mask draw.
The first attempt over-reserved that aperture; old-save animation setup failed.
This was fixed before the final lifecycle tests. The packet test reserves the
animation budget explicitly to guard against recurrence.

## Verification and limits

The native packet test verifies weapon/HUD ordering, packet shape, atlas marker,
UV bounds, precise depth and bob coordinates; controls, input and aiming suites
also pass. Private live tests use copied cards/settings and an offscreen X server.
Evidence lives locally under `recomp/analysis/d08a26/`.

The 60 FPS / 4:3 lifecycle route verifies manual equip, movement, crouch/jump,
first/third person, pause/resume, goggles replacement, native depletion, death,
reload, held fire, active gas save/load and automatic gas exit. Screenshots were
inspected at 4:3 and 16:9. High-refresh and supplementary regression results are
recorded below. Field-rate samples are brief diagnostics, not a benchmark or a
claim about sustained presentation frequency.

User play acceptance remains: size/height, visual obstruction, motion strength,
and readability across weapons, costumes, campaign scenes and traversal views.
No commit or push is authorized by the integration request.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Use Modernized first person (P), then equip the Bio Mask (B) or enter gas with
an owned, charged mask. Vanilla does not emit this mesh.

### Final supplementary checks

- `verify120-final.log`: full lifecycle PASS at 120 target / 16:9, 1653 replay
  captures rendered, zero failures. Offscreen present cadence averaged about
  23.4 ms in its trailing sample; this is not a sustained-120-FPS acceptance.
- `verify60-off.log`: full lifecycle PASS at 60 / native 4:3.
- `regress-opengl.out`: bob-off smoke PASS; `DNTTK_BOB_LOG` reports zero dip
  for all 103 sampled updates, including movement.
- `regress-vanilla.out`: Vanilla loads and emits zero mask packets.
- `regress-software.out`: software renderer smoke PASS; screenshot inspected.
- Final packet test passes with the animation allocation reserved. Build and
  whitespace checks pass. Exported framework patch applies to the clean pinned
  dependency and matches all 43 modified tracked framework files. Existing
  framework changes were preserved. No dependency pins changed.

Remaining coverage includes all motion strengths/traversal combinations, other
weapons/costumes, cutscenes/restarts throughout the campaign, and sustained
normal-window high-refresh performance. No player save/card was used for writes.


## Native lighting revision - 2026-10-25

The user accepted the integrated mesh visually and requested native lighting
before closing the job. `first_person_weapon_attached` samples render context
`0x800d67a8 + 0x54` at `0x80033e40`, caller `0x8003531c`, for Duke's main
first-person weapon. The original actor loop has prepared that RGB, including
room brightness and per-joint environmental tint, and the existing weapon code
has already corrected close-camera fade. The mesh multiplies its baked face
colours by that RGB / 128, truncating and saturating each channel to 0..255.
No additional light source, gamma adjustment or change to weapon lighting.

Each Duke draw initializes the light from `player+0x1b`, the native brightness
byte. This is also the fallback when no weapon is drawn, including holstering.
The actual weapon sample replaces it in the same draw. Replay workers execute
the same reset/sample sequence using their restored RAM; no previous-frame tint
is carried across loads, perspective changes or a holstered interval. The
fallback follows brightness but does not reproduce per-hand gas tint without
a drawn weapon.

`lighting.out` / `lighting.log` and `light-*.png` record the private slot-11
120-target route: dry RGB 0x515151, gas RGB 0xb0d8b0, changing brightness along
the route, firing, holstering/drawing, automatic exit and reload. Reload restores
the darker tunnel light. Screenshots were inspected; replay reports zero failures.
The packet test checks black/neutral/dim/tinted/bright output and restoration,
with geometry and bob unchanged. Native controls tests pass.

D08A27 records the user-reported 4:3 gas cutoff in widescreen (also visible in
this route). It is the selected next job after D08A26 closeout. No gas renderer
change, commit or push is part of this revision.

The rebuilt software-renderer lighting smoke also passes (`lighting-software.out`).
No framework code changed in this lighting revision.


## Acceptance and closeout - 2026-10-25

User: "awesome, commit and push!! lock it in and close us out". The mesh,
framing/motion and final native-lighting candidate are accepted. Documentation
and implementation commit/push are explicitly authorized as MusicMonsterMod.

Accepted executable SHA-256: `c775837d26203c05f46d50503f6a657b7ae1b85e01bbb0ec062c1e388fe15e62`.
No post-acceptance gameplay edits, build or launch. Prior test coverage limits
remain recorded above; acceptance does not imply a full campaign or sustained
120-FPS benchmark. Framework changes are exported against the unchanged pinned
dependency. Retail media, extracted assets, research, saves/cards and build
output are excluded from the commit.

D08A27 is the selected next job, ready after this closeout: reproduce the gas
coverage cutoff with a private copy of UI slot 11 and fill the widescreen
viewport. No gas fix is included in D08A26.
