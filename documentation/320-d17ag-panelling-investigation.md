# D17AG - Slot-6 wooden panelling investigation

2026-10-29. In progress; reproduced, native painter-order cause confirmed. Linked to the
D17AK quality sweep, without an established shared cause.

## Fixture and reproduction

Private copy of current UI slot 6, file
`recomp/saves/local-play/openbios/state_800AB6FC_slot05.pst`.
SHA-256 `7835791aef22f3c76634b02f736e2e17fb2ace4b3417c41b7fd7cab57c13046f`.
Original player saves/cards/settings retain hashes and modification times.
Local evidence is under ignored `recomp/analysis/d17ag/`.

At FOV 90, 16:9 and 60 setting, pan right then left without movement.
Baseline 24-pan capture shows rectangular tan gaps in the wood canopy underside
above the caravan, particularly turn09/10/11. A controlled yaw sweep reproduces
at yaw -0.20 and pitch -0.122522 radians; nearby yaw 0 is continuous. Fixed
positions remain stable while captures complete. Later diagnostic captures
explicitly poll present_shot_seq and require wrote success before proceeding.
Initial pan captures used a shorter settle and lack that completion assertion.

## Evidence and negative controls

The same gaps remain at controlled yaw -0.20 with each independent diagnostic:

- DNTTK_PRECISE_CULLING=0 (native NCLIP decisions).
- DNTTK_WORLD_SUBDIVISION=1 (original world subdivision).
- Original geometry with Corrected textures.
- Corrected geometry with Original textures.
- Both geometry and textures Original.
- DNTTK_NEAR_CLIP=0 (host near takeover disabled).

These are diagnostic exclusions, not proposed shipped settings. Disabling near
clipping also changes weapon presentation and is not an acceptable workaround.

Primitive trace at affected pose (local trace file, host frame 169) identifies
three source faces overlapping native screen coordinate (200,-60):

| Mesh | Face | Decision | Corner depth range |
| --- | --- | --- | --- |
| 0x8019ddfc | 0x8019e034 | native_kept | 3656-4854 |
| 0x8019ddfc | 0x8019e05c | native_kept | 4653-4979 |
| 0x8019e0c8 | 0x8019e2e8 | native_kept | 3625-4823 |

The first and third use source texture index 0x15; the second uses 0x0e.
All recorded corners are projection-safe. No packet skips, mesh fallbacks or
budget hits occurred in these captures. A follow-up GP0 ring capture confirms native face-wide ordering causes the
visible gap. At fixed yaw -0.205654, frame 185, pixel (185,-61):

- Roof quad packet `0x0011482c`, sequence 99931, covers the pixel.
- Tan wall packet `0x00114a9c`, sequence 99933, also covers it and draws later.
- The adjacent roof packet, sequence 99940, does not cover this pixel.

The corresponding traced roof is mesh `0x8019d3bc`, face `0x8019d504`, with
corner depths 3827-5025. The wall is face `0x8019e05c`, depths 4653-4979.
Native world sorting uses maximum corner depth shifted by 5: roof slot 157,
wall slot 155, so the wall is painted later. But intersecting the view-space
planes with the pixel ray gives approximate depths 4039 (roof) and 4813 (wall)
at the shared pixel: the roof is physically in front. Adjacent projected
panels cover different pixels as yaw changes, producing the observed popping.
The trace and GP0 captures differ slightly in yaw (-0.20 versus -0.205654);
the packet coverage and depth ordering agree in both close views.

This is direct evidence of an original painter-order limitation exposed by
this view. It is not a missing-face, packet-budget, texture-precision or NCLIP
failure. It does not establish that the other batch symptoms share the cause.
Raw GP0 evidence: local `gp0-target0-gpu0.json`; corresponding completed
presented capture: `gp0-slot6-target00.png`. A correction must preserve the
original order where valid while resolving actual overlap, rather than use a
scene-specific face exception or globally disable culling.

## Changes and remaining validation

No gameplay or renderer change made for D17AG yet. The root coordinator is evaluating coherent shared depth/order corrections. After any correction, repeat matched 64/90/120 pan
routes at 16:9 and a contrasting aspect, plus 120-Hz replay and affected shared
fixtures. Current 60-setting diagnostics do not certify that acceptance matrix.
