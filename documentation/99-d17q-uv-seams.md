# D17Q residual wall flicker (per-triangle UV seams)

Status: **Accepted** (user playtest, 2026-10-04). After extended play the
user accepted the whole job: "the whole job is 100% accepted", and
praised how cleanly the game now plays. The user authorized
documentation, commit and push.

Accepted executable SHA256 (new regression baseline):
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960`.
Previous baseline (D17N):
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`.

## Summary

With D17N's subdivision zigzag gone, a different, sharper artifact remained:
a single diagonal line through some textured walls. It is visible standing
still (UI slot 12, the patterned club panel at the crosshair on load), and it
flickers on and off while walking. Along the line, the texels on one side are
offset by one texel from the other side.

Cause: the GPU backends apply a 2D-sprite correction to each triangle
separately. Some world-wall triangles qualify for it and their partner
triangles don't, so the two halves of one wall polygon sampled the texture one
texel apart. Fix: perspective-corrected 3D triangles no longer get that sprite
correction. Original textures, Vanilla and the software renderer are
pixel-identical to before.

## Cause, with evidence

Reproduced offscreen (120 Hz, 4x, first person, private card copies, port
9242). UI 12 savestate SHA256 is unchanged since D17N
(`1db85905...236dd`).

1. **Not geometry or depth.** A temporary trace of the seamed panel's
   world quad recorded exact PGXP positions, integer SZ and UVs for all four
   corners. In homogeneous form (x*z, y*z, z) the corners satisfy the
   single-homography condition to about 0.03 texel, and z0+z3 = z1+z2
   exactly. The two triangles' perspective maps agree to 0.04 texel. Integer
   SZ, PGXP positions and the GTE divide are not the cause. An offline
   exact-perspective render of the same data, using the real texture and CLUT
   read from VRAM, is seamless. The panel art itself has no diagonal.
2. **The GL backend received different UVs for the two halves.** A trace at
   the final GL vertex append showed triangle A with UVs (216,192),
   (216,231), (255,192) and limits 216..255, and triangle B of the same quad
   with (256,192), (217,231), (256,231) and limits 217..255. Every U in
   triangle B was bumped by +1.
3. **Source of the bump.** `gpu_uv.h` `psx_uv_tri_mirror_offset` (used by GL
   and VK) is the Beetle/parallel-psx compensation for X/Y-flipped 2D sprites
   on centre-sampled rasterizers. For each triangle it checks the integer
   screen corners. If the mapping is axis-aligned (U depends on x only) and
   decreasing, it adds +1 to U and tightens the sampled range.
   - A world-wall triangle that contains a whole vertical wall edge (two
     corners with the same x and the same U) always counts as axis-aligned.
     When the texture runs right-to-left, it counts as mirrored and gets the
     bump.
   - The partner triangle in the same quad usually contains the other
     vertical edge. That edge rounds to a one-pixel slant, so it counts as
     diagonal and gets no bump.
   - Result: a one-texel discontinuity along the quad's diagonal (see the
     staircase steps in `evidence/ui12-seam-pixels.png`).
   - As Duke moves, the integer corners decide frame by frame which triangles
     count as axis-aligned, so the seam appears, moves and disappears: the
     residual flicker.
4. **Scope.** The same mechanism appears wherever textured world polygons
   have an exactly vertical or horizontal integer edge. In the 12-slot sweep
   it is visible, for example, as a diagonal seam across the UI 11 dance-stage
   canopy (`evidence/ui11-canopy-legacy-vs-fix.png`). The idle UI 12 packet
   census has 85 textured quads, 9 with any bump and 6 where exactly one half
   was bumped.

## Implementation

Framework only (`recomp/psxrecomp`, exported into
`recomp/patches/time-to-kill-accepted-source.patch`):

- `runtime/src/gpu_uv.h`:
  - `psx_uv_tri_is_perspective(valid, q)` is true when the triangle carries
    perspective weights that are not all 1.0, i.e. a Corrected-texture
    world-polygon half.
  - `psx_uv_tri_limits_3d` gives the full min..max sampled range, the same
    as the existing diagonal model, including page wrap.
- `runtime/src/gpu_gl_renderer.c`, `runtime/src/gpu_vk_renderer.c`: in
  `gpu_textured_triangle`, a perspective triangle uses the 3D limits and no
  mirror bump. Everything else keeps the original 2D model.
- `PSX_UV_3D_LEGACY=1` (developer) restores the old per-triangle model for
  comparisons. It is not a player setting.

Unchanged by design:
- Original textures. The mesh path sends uniform q = 1, so these take the
  legacy path bit for bit.
- Vanilla.
- The software renderer, whose DDA never used the bump.
- 2D sprites and rects.
- Near-clip host polygons with float UVs, which already bound sampling by
  their own range.

No game hook, codegen-hash input (still `0x8bab543c`) or save format changed.
Existing savestates load.

## Evidence (private, local)

`recomp/analysis/d17q-20261004/` (ignored). The cards are a dated copy of the
player's current files; the player's files were never used.

- `walk-base/`, `walk/`: 120 Hz idle plus 24-present W/S sequences, baseline
  versus candidate. `evidence/ui12-panel-legacy-vs-fix.png`: the UI 12 panel
  seam is gone.
- `pk/`, `panel-vram.json`, `offline-panel.png`: packet dumps, the panel
  texture/CLUT, and the offline exact-perspective render.
- `sweep/`: all 12 slots at 120 Hz, legacy versus fix, idle screenshot plus a
  strafe:
  - Zero replay misses and zero failed jobs in all 24 runs.
  - Changed pixels per slot 0.00-0.77% (UI 11 2.30%, mostly the animated
    dancer).
  - The changes are one-texel corrections on world surfaces and the removal
    of the UI 11 canopy seam. No missing surfaces were seen.
- `modes/`:
  - UI 12 at 60 and 180 Hz with the fix: zero misses, clean walking frame.
  - Original textures and Vanilla, legacy versus fix: **pixel-identical**.
- Tests: Python 106 (104 pass, 2 conditional skips); `ttk-near-test
  disc/SLUS_005.83` including whole-polygon contracts; `ttk-input-test`;
  `ttk-aim-test disc/SLUS_005.83`.
- The VK backend edit passes a syntax check. VK is not built by the
  `local-dev` preset and was not run.
- The patch applies to a clean worktree of the pinned framework commit and
  reproduces the edited files exactly.

## Remaining limits and other candidates

- Accepted on the user's extended playtest; the private checks above are the
  recorded automated evidence.
- The automated walk sequences pitched toward the floor, so the
  before/after motion evidence is mainly the idle and strafe captures plus
  the mechanism. The flicker claim rests on the mechanism (integer-corner
  toggling), not on a measured flicker metric.
- Measured and found negligible here: integer-SZ perspective weights (about
  0.03 texel bend across a quad).
- Not investigated in this pass:
  - Polygons still on the original subdivided path (`subdivided_kept`,
    0-127 per run in D17N).
  - The 1/64-texel centre-sampling offset on mirrored 3D mappings, now
    uncompensated. It is below a texel and uniform, so it can't produce a
    seam.
  - Texel aliasing (shimmer) of distant high-contrast textures, which is
    inherent to point sampling and not addressed.
- Campaign-wide coverage is not established.
