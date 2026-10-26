# D17Z - Medieval corridor wall texture continuity

2026-10-26. Status: Done. User reported it works perfectly, then explicitly
authorized lock-in and closeout after accepting the D17AA follow-up. The final
accepted build includes both fixes: `9342a18490c3b9304371f3f2d2d3cabce077f000a13465f326b1392ebfa9eb32`.

## Cause and change

L: reproduced the reported corridor using a private copy of current UI slot 12,
LEVEL09 (overlay tag 9), room 7. Save SHA256:
`032eca1255d740d515c8f7a789ba344bf7f506075f63e4f9287950722b252f56`.
The visible issue is texture remapping as walls enter the host near-clip path,
not an established portal/whole-room visibility failure.

S/L: primitive traces identify wall mesh `0x801b7370`, including faces
`0x801b7690`, `0x801b7650` and `0x801b75f8`. The first face uses texture descriptor
2529 with UV corners `(127,64), (64,64), (126,119), (64,110)`. The wall geometry
is planar, but its UV mapping is not affine across the quad. The native renderer
splits PS1 corners along 1-2. The host clipper ordered the outline 0,1,3,2 and
triangulated it as a fan along 0-3. Those choices produce different interpolation:
at the centre of this rectangular face the difference is 0.5 U / 4.5 V texels.
Crossing the takeover threshold therefore changes the painted wall, even with
Corrected textures. Clipping the entire perimeter could also change its internal
triangle boundaries. This is separate from D17Q's mirrored-sprite correction.

`recomp/src/ttk/near_clip.cpp:draw` now splits Corrected-texture world quads into
the original two triangles before clipping/subdivision. Each half retains its
source UVs/colours and the whole quad's maximum-depth ordering key. This extends
the existing warped-quad split to planar textured world quads. Original-texture
behavior, depth-only bridge faces and object/viewmodel split policy are unchanged.
No geometry, retail textures, generated C, framework, save format, gameplay,
settings schema or hook list changes.

## Verification

Candidate executable SHA256:
`0d8950aca24488efa223b7238f169762409180564e5c714fbb739d7b691093cd`.
Baseline executable SHA256:
`93cf0b9bbe3c88cd26ab3ec131ac5dd3923698feacef39a5c477bcf51a526c89`.
Private evidence and harnesses: `recomp/analysis/d17z/`. Offscreen OpenGL, 4x,
16:9, Corrected geometry/textures, player's copied Modernized preferences,
private cards and port 9496. Player saves/cards/preferences retain their intake
hashes and mtimes. The supplied video is preserved.

- Incremental player build passes.
- `ttk-near-test disc/SLUS_005.83` passes all four combinations of Original /
  Corrected geometry and textures. New test evaluates the actual emitted UVs
  against the original two-triangle interpolation, including subdivision and
  guard-band clipping, source ordering and Vanilla exclusion. It fails against
  the old implementation. Original-texture before/after packet digests match.
- `close/` versus `fixed-route/`: same load/advance/retreat/turn script at 60.
  `fixed120/`: same candidate route at 120. Reviewed captures preserve the wall
  mapping rather than flipping its diagonal at takeover. No missing wall patches
  observed in these samples. These are sampled routes, not frame-exact replay of
  the user's recording, and they do not eliminate native texture distortion or
  point-sampling shimmer.
- The 60/120 corridor runs have zero packet skips, mesh fallbacks, copy overflows
  and subdivision budget hits. Candidate host-packet peak is 28,920 bytes versus
  28,360 in the baseline route; these are observed peaks, not identical-frame
  performance measurements. The 120 run records 2,292 replays, zero misses,
  zero failed sessions/jobs and zero worker failures.
- Original-texture and Vanilla before/after slot-12 load captures have a
  pixel-identical matching pair. The original executable is run through a private
  harness with the same current asset directory and private state.
- Third-person 120 smoke covers current slots 12/8/10 and the archived D17Q
  club-panel save copied into private slot 1, including load/move/retreat/turn.
  Zero replay misses or failures. Both baseline and candidate record the same
  seven subdivision-limit hits during slot-12 third-person movement, with no
  dropped packets or mesh fallbacks; these are not a new candidate regression.
  This is bounded smoke coverage, not campaign certification. Native near tests also retain warped-world, prop/platform,
  source-order, precision, whole-polygon and identity-guard contracts.
- `ttk-sky-test disc/SLUS_005.83`, repository policy check and `git diff --check` pass.
- Local sampled motion sequences are `close.mp4`, `fixed-route.mp4` and
  `fixed120.mp4`. They are assembled from diagnostic stills at a common sampling
  rate for visual comparison; they are not cadence/performance recordings.

## Player review

Launch `python3 recomp/tools/local/run.py`, load current UI slot 12, and walk
along the medieval corridor, back away and turn into the adjoining passage.
Check the stonework while approaching and retreating. Keep the player's saved
Corrected texture setting for this fix; Original textures retain their own path.

Compare with the preserved `research/screencaps/medieval-glitch-wall.mp4`.
Human visual acceptance was received on 2026-10-26. Original/Vanilla and smoke checks
above are bounded; no full campaign, Vulkan, audio or long-session performance
claim is made. No framework edits were needed, so the existing accepted-source
patch and dirty submodule baseline are untouched.
