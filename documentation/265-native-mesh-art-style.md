# Native mesh art style - accepted Bio Mask reference

2026-10-26. Accepted foundation for new custom Time to Kill meshes. D08A28
revision 5 is the reference: the user said "This is the one" and explicitly
requested that its style be documented and the job closed. This is a production
recipe derived from an accepted in-game result, not a claim about the original
studio's internal art process. Read alongside the [D36 presentation contract](257-d36-first-person-presentation-audit.md).
Implementation history and evidence: [D08A28](264-d08a28-biomask-ps1-authenticity.md).

## What made it fit

Spend geometry on the silhouette and broad changes of direction. Spend texture
pixels on material identity, recesses and large light/dark transitions. Judge
both at the size the object occupies in the actual game. A modern detailed mesh
with a pixelated photograph is not this style. Neither is a featureless grey
primitive. The accepted result combines economical forms with deliberately
painted, readable materials.

The successful references each supplied a different part of the answer:

| Reference | Lesson applied |
| --- | --- |
| Energy Weapon, key 7 / record 10 | Broad angular surfaces and sparse detail communicate a recognisable object. |
| Flamethrower, key 8 / record 9 | Squared-off pipes/nozzle, strong painted edges, recessed shadows and simple metallic components. |
| Buffalo Rifle pump, key 9 / record 6 | Chunky form; near-black grip with narrow painted highlights, rather than individually modelled hose ridges. |
| Buffalo Rifle barrel | A narrow cool highlight falling into deep blue-grey steel gives metal its volume. Broad uniformly bright stripes looked too flat. |

Use actual selected record IDs to validate references. Keyboard slot labels
alone are not evidence. Retail textures and meshes are reference material kept
locally; the new mask's art is original and does not embed copied retail pixels.

## Geometry: coarse shape, deliberate construction

Keep a strong silhouette, large flat faces and angular connections. The accepted
mask has plain five-sided constant-radius hoses, three broad bend spans, no
geometric corrugation, an extruded angular housing, and simple collars. Painted
bands suggest grip material without changing the silhouette. It retains the
original hose centerlines, housing outline and camera-relative coverage.

Do not add bevel loops, bolts, slots or ridges just because Blender can produce
them. Model a feature when it contributes a meaningful silhouette, occlusion or
large plane change at the intended game size. Otherwise paint it coarsely or
omit it. Flat faces and baked face shading are intentional. Do not smooth the
mesh into a modern low-poly product render.

The mask is 184 evaluated triangles; the observed first-person draw emits 90
triangles after backface rejection. The previous high-detail design was 2400 /
1174. These are evidence, not universal budgets. Inspected DB00 references were
72 triangles for Energy, 123 for Flamethrower and 95 for Buffalo. Different
coverage, visible surfaces and functions make a single numerical cap misleading.

## Texture density: author for the screen, not the atlas dimensions

The accepted storage atlas is 256x128, but its effective art is much coarser:

| Region | Effective painted grid | Storage region | Expansion |
| --- | --- | --- | --- |
| Housing front | 32x16 | 128x64 | 4x4 nearest |
| Secondary steel: sides/collars | 32x16 | 128x64 | 4x4 nearest |
| Tubing | 32x16 | 128x128 | 4x8 nearest |

All three regions use original authored pixels. The complete PNG has 48 unique
RGB colours in revision 5. This is not a requirement to give every future mesh
48 colours or a 256x128 atlas. Retail UV regions inspected here were often
16x16, 24x16 or 24x24, with 4-bit texture-page/CLUT sampling. Examples include a
48x8 Buffalo metal strip, a 16x8 pump region and a 48x16 Flamethrower detail strip.
These are used texture regions, not entire independent image files.

For a nearly front-facing patch, estimate effective texel footprint as its
screen width divided by the number of distinct source texels across that width,
and similarly for height. Count painted texels, not duplicated atlas pixels.
The accepted housing mapping was estimated at about 7.2x7.3 screen pixels per
effective texel in a representative 1024x576 view. Revision 5 retains that mesh,
UV layout and resolution. This is a view-specific estimate, not a guarantee for
every surface, display or field of view, and not an exact equality with all weapons.

A practical density check:

1. Capture the new mesh and the reference weapons under equivalent camera,
   aspect ratio, resolution, lighting and presentation settings.
2. Compare apparent pixel size, feature frequency and occupied screen area.
   Compare the silhouette and materials at normal viewing size, not only zoomed.
3. Check horizontal and vertical footprint separately. A square atlas texel is
   not necessarily square on screen. UV island proportions, physical dimensions,
   view angle, projection and display aspect all contribute.
4. Change the local UV coverage or authored grid when needed. Do not globally
   change filtering, blur the renderer or use a screen pixelation effect.
5. Retain plausible period distortion on oblique surfaces. Do not demand perfect
   mathematical UVs if they erase the reference character.

The tube's 4x8 atlas expansion is intentional storage, not proof of unwanted
horizontal stretching. Its screen footprint depends on the five-sided surface
and longitudinal unwrap. The housing uses a dedicated front island; sides and
collars use secondary steel. Tubes unwrap along their length and around their
five faces. This replaced the earlier dominant-normal planar/modulo mapping
that made strip direction and density difficult to control. Avoid accidental
island spill, repeated slats on side faces, and arbitrary axis-dependent scaling.

Canonical renderer screenshots may be anamorphic. For this evidence, 1364x480
captures were displayed at 1024x576 with nearest resizing for a 16:9 comparison.
Use an actual presented-window capture when checking aspect correctness. Do not
mistake raw framebuffer proportions for the player's final image.

## Painted materials: exact accepted recipe

The reproducible source of truth is
`recomp/assets/models/biomask/ps1/paint_atlas.py`, with geometry and material gains
in `build_model.py`. Preserve both scripts and the packed `.blend` plus PNG.

**Grip/tubing:** neutral RGB levels 8, 16, 24 and 33. The authored band sequence
is 8,16,33,24,24,16 across the strip, with sparse interruptions, shaded ends and
six small wear flecks within the same palette. This creates dark material with
narrow highlights, not alternating silver hoops. The tubes remain geometrically
plain. The approved revision 4 tubing is byte-identical in revision 5.

**Central steel:** deep cool shadows, a narrow blue-grey reflection, then a falloff
back into dark steel. Revision 5's exact 16-row authored ramp is:

```text
18,24,32    26,34,43    40,51,63    62,77,92
96,114,132  144,163,181 188,202,214 121,143,162
73,94,113   43,59,75    29,41,55    68,87,106
145,165,183 204,216,226 110,133,154 32,45,60
```

Rows follow the generator's bottom-up image coordinates. The ramp is modified
locally by panel recesses, coarse fasteners and restrained nicks. Grille slats
also have a shaded reflection across their width instead of a uniformly bright
fill. The pattern provides material volume on a flat plane. It is painted light,
not dynamic specular reflection. Secondary collar steel retains the stronger
Flamethrower-inspired bright edge and oily recess contrast.

Do not copy only the PNG and expect the same result. Housing material gain is
(0.30,0.32,0.35); collars use (0.65,0.65,0.65); tubing uses (1,1,1). The mesh
exporter bakes per-face colour with `128 * shade * gain**0.45`, rounded to bytes,
where `shade = 0.55 + 0.45 * max(0, normal dot light)` and the normalized light
direction is (-0.25,-0.8,0.6). Runtime environmental RGB modulation is then applied
by the existing mask path. Blender roughness/specular settings are previews,
not runtime PBR. Inspect the final game result after all these stages.

The atlas exporter quantizes RGB to 5 bits per channel with `channel >> 3`, packs
RGB555, and clamps an all-zero packed texel to 1. Source RGB palette counts are
not runtime palette precision. Nearest sampling preserves the coarse art. Avoid
high-frequency photographic noise, many tiny chips, smooth modern procedural
materials and additional gloss/shader effects. Do not darken everything uniformly:
the failed grey-block pass showed that removing material contrast loses the form.

## Production and acceptance workflow

Start by studying comparable original assets in game. Save the current accepted
source and headers before a new pass. Adapt the existing workflow reproducibly;
change one visual concern at a time and keep comparisons. For this accepted asset:

```sh
blender -b --python recomp/assets/models/biomask/ps1/build_model.py
blender -b --python recomp/tools/local/export_biomask_mesh.py -- --asset-dir recomp/assets/models/biomask/ps1
python3 recomp/tools/local/export_biomask_atlas.py --asset-dir recomp/assets/models/biomask/ps1
cmake --build recomp/build-local --parallel 4 --target Duke_Nukem__Time_to_Kill_Recompiled ttk-near-test
recomp/build-local/ttk-near-test recomp/disc/SLUS_005.83
```

Both exporters still default to the older parent asset for backward compatibility.
Always pass `--asset-dir recomp/assets/models/biomask/ps1` for the accepted current
look. Use `--output` for non-destructive export comparisons. See the asset README
for explicit rollback instructions. Keep retail media, extracted references,
captures, private saves and build output out of Git.

Preserve camera-relative positioning, true 3D rendering, attachment, visibility,
lighting ownership, gameplay and packet allocation/lifecycle. A correct Blender
preview does not prove that the asset loaded in game. D08A28 needed an independent
startup mask-buffer reservation fix for older save loads; do not remove that fix
when restoring art. Follow D36 for actual runtime changes and relevant regressions.

Compare in the actual renderer with the reference weapons and previous accepted
mesh. Check dark and gas-lit conditions, holstered fallback, equipment changes,
save loading and the affected lifecycle paths using private save/card copies.
Use native tests to protect packet/lighting behavior; do not claim unobserved
campaign coverage from screenshots. Record source counts, effective grids, gains,
exported headers, executable hash, settings and evidence locations. Final visual
acceptance belongs to the in-game comparison and user review, not Blender beauty
renders, arbitrary polygon reductions or an automatic palette-count target.
