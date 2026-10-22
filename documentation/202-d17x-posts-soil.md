# D17X - White triangles on boat supports and grave soil

2026-10-22. Done, user-accepted; closeout authorized.

## Revision 2 - Slot 7 soil corners

The user confirmed slot 6 is fixed and reported slot 7 still affected. The
first pass missed the relevant defect: continuous walking captures show
triangular cutouts in the soil, exposing the brighter ground below. The
initial statement that soil did not reproduce is superseded by this evidence.
The same recorded slot-7 hash is used; no replacement save was needed.

The soil consists of warped textured quads. The host near renderer treated
each as one outline, used the opposite diagonal when triangulating the fan,
and disabled its plane-depth correction because the four vertices do not
share a plane. At the mound's contact with the ground, independently snapped
triangles therefore intersected the floor and disappeared in triangular
pieces. Disabling near clipping removes the cutouts.

`near_clip.cpp` now splits warped precise quads on the original PS1 1-2
diagonal before clipping and subdivision. Each triangle keeps its own UVs,
colors and plane, so raster-depth correction and the existing static-prop
contact allowance apply normally. World halves retain the source quad's
maximum-depth ordering key. Existing planar and depth-only paths are
unchanged; Vanilla does not enter this host path. There is no soil/model/level
address gate. `DNTTK_WARPED_QUADS=0` is the developer comparison.

Continuous 24-present sequences while walking forward/back and sideways show
the soil boundary retained. The accepted slot-6 GL correction is unchanged.
New owned-EXE contracts check native diagonal, UV corner mapping, per-triangle
plane depth and preservation of the world quad's ordering key. The user subsequently confirmed slot 7 fixed and fully accepted D17X.

Revision-2 validation: build and near tests pass with Original textures,
Corrected textures, both geometry/texture corrections, and the developer
split disabled. GL batch tests still pass. All four 60/120 FPS and 16:9/4:3
movement routes cover soil, boat and the preserved cart. Inspected captures
retain the soil boundary and show no boat/cart regression. Six older scenes
remain clean in inspected captures (119.1-119.9 submissions/s in short samples).
The initial two-second 4:3 soil sample was 115.5/s; a settled six-second
comparison measures 119.7/s enabled versus 119.9/s disabled, both with zero
dummy-audio underruns. Third-person smoke captures are clean. All three
Vanilla smoke screenshots are pixel-identical to the prior candidate and
report zero host-depth packets. These remain bounded offscreen checks.
No additional framework changes were made in revision 2; the previously
exported and clean-pin-verified patch is retained.

Accepted revision-2 executable:
`e5a6a69cf7bf8ff9d72581ee2171b1b8c7f64546f71809b846b0b3de3f3e7b4a`.

## First-pass reproduction and diagnosis (historical)

Private copies under `recomp/analysis/d17x-posts-soil/` match the board:

- UI slot 6, file 05, level 6:
  `9d995e41fe1a9ddda0b500664941ea965970dbc7557c483757f05491019fe594`.
- UI slot 7, file 06, level 1:
  `85b08d702946a955ec1f118f53d518dcc0cee8d4107636e63027cacf4a93323e`.

The D17M1 accepted build reproduces white slivers on the wooden boat supports
immediately and while walking. Disabling near clipping removes them; the
near-piece tint identifies host object pieces. Primitive traces identify the
support mesh `0x801253d8`, including flat F4 faces `0x80125778` and
`0x80125780`. These are internal surfaces, not missing wood textures. Unlike
the D17W cart's coincident faces, these are not overlapping coplanar faces,
so D17W's coplanar ranking correctly does not group them.

The title already supplies host depth for flat/gouraud pieces. The GL flat
pipeline discarded it, whereas textured host pieces used depth testing.
Subdividing and sorting the internal flat faces could therefore paint small
white triangles over the textured wood. Enabling depth for the opaque host
flat pieces removes the reproduced slivers. Two fixed screenshot regions
around the left/right supports contain 35/96 bright pixels in the baseline
versus 0/0 with near clipping disabled and 0/0 with the candidate (screenshots
normalized to 2048 x 720 for this comparison).

The grave-soil mesh `0x80149eb8` has 18 textured quads and no flat faces.
The reported white corners did not reproduce on the D17M1 baseline during
short and longer forward/back walks with level and downward views. This
change is not claimed to establish or fix the soil report's cause. Player
rechecking of that save remains necessary.

Addresses above identify local evidence, not conditions in the correction.

## First-pass implementation

`gpu_gl_renderer.c` carries host depth through the flat/gouraud vertex stream
and shader. Opaque, precise host triangles with valid positive depth use the
same depth buffer and LEQUAL rule as textured host triangles. The flat batch
flushes when depth ownership changes and disables depth after drawing both
canonical and widescreen targets. Colors retain their existing interpolation.

Native polygons, lines, rectangles, translucent effects and first-person
weapons retain painter ordering. No title hooks, model data, generated C,
codegen hash, controls or preferences change. The developer comparison
`PSX_HOST_FLAT_DEPTH=0` restores the previous flat-polygon behavior.
The authored framework delta is exported in the tracked accepted-source patch;
its filename does not imply player acceptance of this candidate.

## First-pass verification

The player build passes. The new compiled GL batch test executes the actual
batch functions against an observing GL shim. It covers host/native batch
boundaries, depth cleanup, canonical/wide draws, translucent exclusion,
negative-depth weapon exclusion, lines, coordinates/colors, and the developer
comparison disabled/enabled. It does not replace actual shader/raster checks.
Owned-EXE `ttk-near-test` passes with Original and Corrected textures.

Offscreen first-person routes cover both affected saves and the preserved
D17W cart at 60/120 FPS in 16:9 and 4:3, with forward/back and lateral movement
and two view pitches. Inspected captures show no white support slivers or
cart regression; soil remains without the reported white corners. The 120
samples range from 119.0 to 120.6 distinct submissions/s with zero dummy-audio
underruns. The 60 mode does not use the replay distinct counter; its zero
value is not a measured frame rate. Six older scenes (table, shelf, closet,
apartment, blood, subway) show no obvious new defect in inspected captures,
with short samples from 118.2 to 120.1 submissions/s.

Third-person smoke captures of the three states show no obvious new defect.
Vanilla reports zero host-depth packets and zero replay workers. Its three
smoke screenshots are pixel-identical with the new flat-depth path enabled
and disabled. This is a bounded Vanilla rendering check, not campaign proof.

The full exported framework patch applies to the clean pinned dependency and
matches all 50 patched files. Original player save hashes remain unchanged.
All live probes use private profiles/cards/savestate copies and offscreen GL
with dummy audio. Capture and timing results are bounded diagnostic evidence,
not monitor cadence or listening acceptance.

Candidate executable SHA-256:
`8ef0ab59d8889f80a1f89e58282c8e30a4c62a602359ebc34d3bd60ae060c4a3`.

## Player check

Load UI slot 6 and walk slowly forward/back beside the boat supports in first
person. Repeat on the soil corners in UI slot 7. Check 16:9 and 4:3, at 60 and
your saved 120 FPS. F10 captures the mouse if needed. No new option is needed.
The old minecart regression uses a preserved private state; the player's
current slot 10 has since changed.

```sh
python3 recomp/tools/local/run.py
```

## Acceptance and closeout

2026-10-22: user confirmed "the fix is in! fully accepted. lock it in and close
it out". D17X is Done on user gameplay acceptance of both fixes. No further
implementation changes, rebuild or repeated gameplay checks were needed.
The framework patch was re-exported and verified against the clean pinned
dependency, matching all 50 patched files. Implementation, tests and
documentation are included in the authorized commit/push as MusicMonsterMod.
The automated limits above remain recorded. No next job started.
