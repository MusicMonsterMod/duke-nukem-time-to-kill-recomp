# D08A28 - Bio Mask PS1 authenticity pass

2026-10-26. Done. User accepted revision 5: "This is the one", requested the
style be documented as a foundation for future meshes, and authorized closeout.
The accepted result is plain five-sided tubing with dark pump-style art and
Buffalo-barrel-style graded steel. The first ribbed/flat-grey demake was rejected.
Historical entries below retain the evidence and limitations of each revision.
The reusable production guide is [Native mesh art style](265-native-mesh-art-style.md).

## Artistic objective

Make the biomask less technically detailed and more artistically consistent with
the original 1998 Time to Kill assets. Preserve its recognisable design, broad
colour scheme, silhouette, position and screen coverage. It must remain real 3D
geometry with the accepted environmental lighting, transforms, attachment,
visibility, gameplay and performance behavior. A pixelation filter or arbitrary
triangle reduction is not sufficient. Do not replace it with a HUD sprite,
rebuild from scratch without a compelling technical reason, change the weapon
renderer, or change global texture filtering to accommodate it.

## Initial inspected baseline

S means inspected source; A means accepted prior evidence; U means user report.
Follow the accepted [D36 presentation audit](257-d36-first-person-presentation-audit.md)
for ownership, lighting, replay, restoration and regression boundaries.
[D08A26](247-d08a26-bio-mask-mesh.md) remains accepted; this job refines its art.

- S/A: `recomp/assets/models/biomask/` contains `biomask-lowpoly.blend`,
  `build_model.py`, `prepare_textures.py`, the PNG/Photon atlas, CC0 source
  photographs and provenance. The documented evaluated mesh is 2400 triangles;
  accepted runtime backface rejection emits 1174. Recount both for the revision.
- S: the procedural hoses use eight sides and 16 ribs per side, each rib with
  four longitudinal intervals; one hose is mirrored. The central housing is an
  extruded outline with a single-segment bevel, gasket, collars, collar rims,
  six-sided bolt heads and separately modelled bolt-slot strips. The existing
  housing is already angular; do not assume it has many bevel subdivisions.
- S: the 256x128 atlas has two 128x128 source regions derived from CC0 rubber
  and rusty-metal photographs. The metal is desaturated and both images are
  box-resampled. This is not evidence of original TTK hand-painted texture style.
- S: generated UVs use dominant-normal planar projection, axis-specific scales
  (X .25, Y 1.5, Z 1.25), modulo wrapping and half-atlas placement. Inspect actual
  evaluated UV islands and discontinuities; these scales are a plausible source
  of uneven density, not proof of the reported horizontal stretching.
- S: `export_biomask_mesh.py` reads the fixed Blender path, evaluates modifiers,
  triangulates, exports UVs in 256x128 coordinates and bakes flat-face directional
  colour. `export_biomask_atlas.py` asserts 256x128 and emits 15-bit RGB texels.
  Headers are `recomp/src/ttk/bio_mask_mesh.h` and `bio_mask_atlas.h`.
- S/A: nearest sampling is already used for this immutable host atlas. Blender
  roughness/specular settings are preview materials, not a runtime PBR shader.
  `near_clip.cpp:near_clip_biomask` applies the established aspect-aware transform,
  restrained bob, backface rejection and per-channel native RGB modulation.
  Preserve weapon/HUD ordering and the reserved packet allocation contract.
- S/A: `first_person_weapon_attached` samples native weapon environmental light;
  holstered fallback uses Duke's brightness. This known fallback limitation must
  not be silently changed or described as newly verified by an art-only pass.
- U: the accepted mask looks too refined and dense beside the weapons, with
  apparent horizontal pixel stretch. These visual judgments motivate the work;
  a fresh in-game comparison and causal diagnosis have not yet been performed.

Blender is discoverable at `/usr/bin/blender`; this is availability evidence,
not a successful model-generation or game-validation run.

## Reference study required before revision

Use owned retail assets locally, with their actual weapon records authenticated:

| Reference | Selection / record | Required observations |
| --- | --- | --- |
| Energy Weapon | Keyboard slot 7 / record 10 | Sparse detail, broad faces, simplified silhouette, polygon count, texture/UV footprint and geometry-versus-texture economy. Its existing display scale is 1.4, which affects apparent texel density. |
| Flamethrower | Keyboard slot 8 / record 9; upgrade 27 | Angular pipes, low-sided cylinders, squared-off nozzle and connectors; identify how geometry and texture imply roundness. |
| Buffalo Rifle pump/grip | Keyboard slot 9 / record 6 | Primary tubing reference: chunky ridged form, broad shading and texture-based material cues; distinguish true geometry from painted ribs. |

The desired visual traits above come from the user's brief. They are not newly
measured weapon mesh counts or texture dimensions. Inspect the actual source
meshes, texture regions/CLUTs and captured runtime presentation before deciding
how many sides/ribs or which atlas resolution to use. Do not confuse the Energy
Weapon's keyboard slot with internal record 7 (Gatling), or accept a requested
selection key as proof the switch completed. Keep extracted retail art local.

## Work sequence when selected

1. Preserve a separately named accepted reference revision of Blender, generator,
   atlas/Photon source, exporters and exported headers. Record hashes and exact
   rebuild/restore commands before changing anything. Current files remain
   untouched at intake. The present scripts write fixed accepted paths, so add
   explicit revision/output selection or equivalent isolated paths first.
2. Measure both mesh and screen-space reference evidence under equivalent
   camera, aspect, resolution, lighting and texture/geometry settings. Record
   polygon/triangle counts, actual texture regions and dimensions, palette,
   UV proportions and texels per visible surface. Compare horizontal/vertical
   screen footprints, including original pixel aspect and projection. Atlas
   dimensions alone do not establish density or explain stretch.
3. Adapt the existing Blender generator reproducibly. Explore lower-sided,
   fewer-segment, more angular hoses, reduced ridge frequency, a few large ribs
   and painted corrugation. Borrow the Buffalo grip's chunky economy and flame
   nozzle construction. Do not automatically remove every ridge.
4. Retain the housing outline and open shape. Review tiny bolt slots, fasteners,
   bevels, gasket and transitions against original-resolution visibility. Use
   larger faces and simpler metallic regions where justified, without turning
   the design into an unrecognisable primitive.
5. Revise UVs and texture art together. Prefer broad hand-painted shading,
   limited wear/colour variation and deliberate low-resolution features over
   photographic noise or sophisticated material effects. Preserve broad colours
   unless the reference study justifies a change. Match apparent weapon density;
   correct unnecessary horizontal stretching but retain plausible period UV
   distortion. A lower-resolution source inside the existing atlas may suffice;
   changing atlas dimensions requires auditing every consumer, not just the PNG.
6. Export with the established mesh/atlas workflow, preserve the runtime contract,
   and compare accepted/revised mask against all three weapons inside the game.
   Use equivalent conditions, 4:3 and widescreen, original-resolution inspection
   and normal player presentation. Include motion, dry/dark/gas illumination,
   weapon drawn/holstered and look/stance changes, not only Blender studio shots.
7. Verify visibility/lifecycle: equip, auto gas entry/exit, charged/depleted,
   goggles replacement, death/load, first/third view, Vanilla, save/load and
   60/120 replay. Run relevant native packet/lighting and controls tests; check
   allocation/packet budget and performance against the accepted baseline.
   Do not alter gameplay, native light sampling, global filtering or weapons.

## Deliverables and acceptance

Deliver a brief evidenced inconsistency analysis and findings for all three
references; separately named revised Blender model, textures and reproducible
scripts/exports; geometry and texture before/after counts; in-game matched
captures; exact rollback instructions and proof the accepted asset is preserved.
Report lighting/runtime regression results and any remaining validation limits.

The revision must retain the design, genuine 3D rendering and working first-person
implementation, but look plausibly native to 1998 TTK: simpler angular tubing,
Buffalo-like grip economy, Flamethrower-like pipe construction, comparable apparent
weapon texel density and no conspicuous modern fine detail. Preserve environment
lighting and introduce no gameplay/rendering regressions. Numeric reduction alone
is not acceptance. Human in-game visual review is required; no in-game acceptance
may be claimed without actual comparison. If tools cannot run, deliver only
verified work and list the outstanding Blender/game validation explicitly.

D08A26, D08A24 and the accepted D36 ownership contract are baselines. The new
D36H2 Buffalo flicker job remains separate; studying the grip does not authorize
fixing that weapon. No other backlog job is selected by this scope.

## Implemented candidate - 2026-10-26

Candidate executable SHA-256:
`666587f7aef9cc24ffa9c404352e27dc2e0096dd1d602a4c0595b4cb08db31ce`.
No acceptance closeout, commit or push. D36H1/H2/D17Z remain Todo.

### Reference findings

S: read-only USA executable disassembly at 0x800352bc-0x80035318 confirms the
weapon draw obtains the model index from weapon record +0x0e. DB00 mesh census:

| Weapon | Record / model | Referenced vertices | Polygons | Triangles |
| --- | --- | --- | --- | --- |
| Energy | 10 / 167 | 38 | 38 | 72 |
| Flamethrower | 9 / 195 | 73 | 72 | 123 |
| Buffalo Rifle | 6 / 116 | 52 | 67 | 95 |

The existing offline parser reads a halfword vertex count; the Flamethrower's
high byte includes non-count data (1353 is not a credible vertex count).
This report counts unique vertex indices actually referenced by the primitives
instead. No shared parser was changed as part of the art job.

Texture regions are small, often 16x16, 24x16 or 24x24, with 4-bit texture-page
sampling and separate CLUTs. These are used UV bounding regions, not separate
image files or a total atlas budget. Buffalo includes a 48x8 metal strip and
15x24 wood regions; Flamethrower includes a 48x16 nozzle/detail strip. Exact
per-page/CLUT bounds are retained in the local `reference-census.json`.

L: baseline and candidate routes verified actual selected records 10, 9 and 6.
The Energy weapon reads through a small number of broad angular surfaces and
contrasting green/metal regions. The Flamethrower uses a visibly squared mouth,
angular tank/connector faces and large painted highlights. Buffalo's grip reads
as a few chunky wood steps and broad dark/light bands. These support reducing
hose ribs and small hardware, rather than smoothing a lower-polygon mesh or
preserving every corrugation as fine texture detail. Retail assets stay local.

### Asset and texture changes

The revision lives in `recomp/assets/models/biomask/ps1/`, including its Blender
model, atlas, generator, Blender pixel-art script, statistics and provenance.
The established generator was adapted; this is the same housing outline and
hose centerline. No replacement implementation or new runtime render path.

| Property | Accepted original | PS1 candidate |
| --- | --- | --- |
| Evaluated triangles | 2400 | 488 |
| Visible emitted triangles/draw | 1174 | 236 |
| Packet bytes/draw | 46960 | 9440 |
| Hose sides / ribs per side | 8 / 16 | 6 / 5 |
| Runtime atlas | 256x128 | 256x128, unchanged interface |
| Material source detail | Two 128x128 photo-derived regions | Two original 32x32 tiles, nearest-expanded 4x |
| PNG unique RGB colours | 161 | 13 |

The housing loses its small bevel, bolt/slot meshes and thin extra collar rims.
Broad faces and collar shapes remain. Overall X extent changes by about 0.1%;
Z extent changes by less than 1%. Removing protruding tiny hardware changes Y
bounds, not the camera transform. Larger, angular rib spacing is intentional.
The new atlas has broad steel patches, a few coarse chips and simple rubber
bands; no photograph or retail pixels, random noise, gloss or normal map.

S/L: the old front-plane mapping uses about 31.36 U versus 156.8 V texels per
model unit. The new projection is approximately 15.68 effective texels/unit in
both directions. For a representative front plane at a 1024x576 16:9 presentation,
the unchanged projection predicts original texels around 5.23x1.05 screen pixels,
versus about 10.46x10.45 for the revised coarse art. This is an analytical
front-plane footprint, not a universal measurement for foreshortened hoses or
all weapon surfaces. It explains a genuine source of horizontal-looking texels;
reducing the atlas size alone would not have corrected the directional ratio.

Actual game comparisons show larger, quieter texture regions beside all three
weapons, without the original fine photographic streaking. Weapon surfaces vary
substantially in density and distortion; exact equality is neither asserted nor
the target. The original broad dark rubber/steel colours are retained. Human
judgment of the coarse metal treatment and five-rib spacing remains the acceptance
gate, not the triangle or palette reduction percentages.

### Rendering and verification

No `near_clip.cpp`, first-person light/transform/visibility implementation,
framework code, global filter, native weapon asset or gameplay code changed.
Only exported mask geometry/texels feed the existing runtime path. Existing
packet regions and allocation sizes remain unchanged; less of each is used.
Exporters now accept `--asset-dir` and `--output`; default paths restore the
accepted art. The packet test's obsolete >1000 visible-triangle assertion was
lowered while retaining a nonempty geometry floor, accepted allocation ceiling,
packet shape, UV/depth, weapon/HUD ordering, bob and lighting checks.

- Build and native controls/input/aim/near suites pass. Controls include original
  LEVEL00, LEVEL01 and all overlay fixtures. D37A animation-save recovery passes.
- Private offscreen OpenGL 60 comparisons include base Energy, Flamethrower and
  Buffalo, mask off/on, firing/settling and crouch. Actual equipment IDs checked.
- Physical-key Xvfb lifecycle passes at 120 / 16:9 and 60 / 4:3: equip, movement,
  crouch/jump, both views, pause/resume, goggles replacement, depletion, death,
  reload, held fire, gas save/load and automatic gas exit. Diagnostic depletion
  and death use private guest-memory writes, not player saves or game-code edits.
- 120 run: three replay workers, zero failed workers/plugin jobs and zero misses.
  Presentation averaged about 23.4 ms in the trailing sample; this is replay
  coverage, not proof of sustained 120 unique frames/s. Short capped guest-rate
  samples and lower packet use do not establish a campaign-wide performance gain.
- The lighting route samples dry `515151`, gas `b0d8b0`, holstered brightness
  fallback `b0b0b0`, automatic exit and reload to the dark room. Captures inspected;
  no previous tint survives reload. The known holstered tint limitation remains.
- Software-renderer smoke/capture passes; Vanilla emits zero mask packets.
- Sampled 60 route has zero packet skips, copy overflows and world/object budget
  hits. No new allocation or save-format field was introduced.

All player save/card/settings intake hashes and mtimes match; accepted source
model/texture/generator hashes match `ps1/accepted-reference.json`. Diagnostic
games are stopped. Existing dirty framework files remain untouched.

Excluded attempts: the first diagnostic token list used unsupported bare B/P
names, so its screenshots are not mask/view-switch evidence. Corrected captures
use semantic actions with explicit reset. A 4:3 test initially supplied invalid
profile value `4:3`, causing control recovery/default third-person; it was rerun
with the actual `off` widescreen setting and passed. Current player slot 11 had
changed since the original gas work; comparisons use the earlier private D08A26
slot-11 copy, not an assertion about the player's current slot numbering.

### Review artifacts and rollback

Local evidence root: `recomp/analysis/d08a28/`.

- `baseline-valid/` and `candidate60/`: matched settings and save location, actual
  reference records. Enemy animation/timing is not synchronized pixel-for-pixel.
- `comparison-buffalo.png`: original above, revision below; canonical captures
  are nearest-resampled to the same 16:9 presentation ratio for review.
- `verify-120-16x9-*.png`, `verify-60-off-*.png`, `light-*.png`: actual presented
  captures for aspect, lifecycle and light checks; software/Vanilla captures too.
- `reference-census.json`, `texel-footprint.json`, `geometry-comparison.json`,
  `tests.json`, `preservation.json`, logs and scripts preserve measured limits.

The accepted source directory is intact. The private `accepted-asset/`, original
exported headers and `accepted-game` also retain the intake working reference.
[Revision README](../recomp/assets/models/biomask/ps1/README.md) gives exact
candidate rebuild and accepted-asset restore commands. Rollback exports the
unchanged parent Blender model/atlas and rebuilds; no player-state migration.
Restoration was verified to private output paths: mesh header byte-identical to
intake, atlas texel data identical (only the exporter provenance comment differs).

Launch `python3 recomp/tools/local/run.py`, use Modernized first person (P),
then equip a charged Bio Mask (B) or enter gas. Compare beside keys 7/8/9 when
those weapons are available. D08A28 is Needs playtest for artistic approval;
full campaign/costume coverage and sustained high-rate performance are not claimed.

## Missing mask after loading current slot 3 - corrected candidate

User reports no mask at all. The 17:50:44 session loaded UI slot 3 and logged
Bio Mask on/off successfully. A fresh private copy of that save and the player's
settings reproduces the failure on `666587f7...`: active mask, charge and face 2,
valid first-person draw gate, but `mask_arena=00000000`. This was missed by the
prior private gas-save coverage; those checks did not certify this allocation layout.

Cause: only the world packet arena was reserved at startup. Loading this save
imports its remaining DMA allocation records (including the previous mask buffer)
into the process allocator, while the title's host `mask_arena` pointer stays zero.
A later fresh 188 KiB mask allocation cannot fit in the 1 MiB aperture, so the
otherwise eligible draw returns without emitting geometry. This is not a missing
asset export, an inactive gadget or a stale executable.

Correction: reserve the existing mask packet arena alongside the world arena in
`register_ttk_near_clip`, before any save can import an unowned allocation tail.
Capacity and mesh are unchanged. Old allocation-order mismatches continue through
D37A's authenticated saved-animation recovery; GPU packet clearing is not weakened.
No framework change, player-save rewrite, new save format or mask-visibility bypass.

New regression checks establish startup ownership and no draw-time allocation;
the real memory.c snapshot test now exercises old world/animation/mask order
against startup world/mask, including animation capacity and repeated/warm loads.
Native controls/input/aim/near and save-restoration tests pass. Private slot-3
screenshots show the revised mask, including downward look. Semantic view-toggle
probes remained effectively first person and are not counted as new switch coverage.
60- and 120-setting save/reload, cross-level slot 8 and return to slot 3 passes, with
three workers and zero worker/plugin failures or replay misses. Clean-air reload
correctly auto-removes the mask; the test re-equips it before checking rendering.
The first warm-load assertion incorrectly expected it to stay active in clean air;
that assertion was corrected without changing the accepted auto-mask policy.

Corrected executable:
`2c86ce64a4ddfa42a6d7fa7d45aedef342c105b9ccb0480ac07e84dc6cbe549e`.
Evidence: `recomp/analysis/d08a28-missing/`, with before/fixed captures, copied
saves, draw gates, allocation traces and preservation manifest. D08A28 remains
Needs playtest. No commit/push or accepted closeout.

The final 60-setting route additionally asserts increasing mask-draw counts at
each eligible sample (including warm load and return from slot 8). All 41 current
player save/card/settings hashes and mtimes match this follow-up intake. All
diagnostic games stopped. The user can retest their existing slot 3 directly.

## Revision 2 - plain pentagonal tubes and defined textures

User clarification: tubes should be about five-sided, without ridges; the first
revision's textures looked like a grey amorphous block. That art direction is
rejected and superseded. The slot-3 allocation correction remains in place.

Changes:

- Five-sided, constant-radius tubing with three broad bend spans. No modelled
  ribs or corrugation. The mirrored centerline, connection and housing outline
  remain; painted bands suggest the grip material without changing the profile.
- Re-inspected all extracted Flamethrower/Buffalo texture-region crops, not just
  atlas dimensions. Flame metal uses pronounced bright edges, dark inset bands
  and large highlights. Buffalo's 16x8 pump/grip region (page 43, CLUT 13996)
  uses dark bands and narrow highlights. These guide original painted art.
- Housing receives a dedicated 32x16 front texture with edge strips, inset panel,
  broad slats and coarse fasteners; separate 32x16 steel art covers the sides and
  collars. The tube strip is 32x16, unwrapped along/around its five sides.
  Removing dominant-normal planar mapping on the tubes avoids irregular strip
  direction. No retail pixels were copied into the asset.
- Stronger light/dark texture contrast and less severe baked metal attenuation
  restore material readability. No change to the native environmental light
  multiplier, runtime shader, global filter, geometry placement or visibility.
- Atlas remains 256x128, with 22 original RGB colours and nearest-expanded art.
  The housing's front-plane texel footprint is about 7.2x7.3 pixels at the prior
  representative 1024x576 view, rather than the superseded revision's 10.5 square
  pixels. Foreshortened tube/side footprints differ deliberately.
- Mesh is 184 triangles, of which 90 are emitted per draw (3600 packet bytes).
  Packet allocation capacity is unchanged for restoration of the accepted art.
  Native packet tests use a source-relative visible-geometry floor instead of
  assuming the rejected ribbed model's >100 triangles; all semantic checks stay.

Built in Blender 4.0.2 and exported through the existing selectable asset path.
Build and native controls/input/aim/near tests pass; real-memory old/warm snapshot
and animation-save recovery regressions pass. Private current slot-3 load,
look, save/reload, cross-level slot 8 and return pass at the 120 setting, with
zero replay misses, worker failures or plugin failures. Effective per-sample draw
counts are checked; semantic perspective probes that stayed first-person are
not claimed as new toggle coverage.

Matched 60-setting captures with actual Energy 10, Flamethrower 9 and Buffalo 6
show the mask on/off, firing/settling and crouch. Reviewed in-game images show
plain faceted tube outlines and readable dark/light details rather than the
previous blank grey face. Native gas/dry/holstered/reload lighting, software
renderer and Vanilla smokes pass. Original high-detail source hashes match;
all 41 player save/card/settings hashes and mtimes are unchanged. No framework
change or alteration to the already fixed startup reservation.

Current executable:
`f65e677e517829f5af253c71bc383eff4112e56a4734e1894f10e22cd68f3ad7`.
Local evidence: `recomp/analysis/d08a28-revision2/`; `comparison-slot3.png` places
the rejected revision above the new one, equally resampled to 16:9. Actual
reference captures are in `references60/`; physical lighting captures and logs,
`slot3-120/`, test results and the preservation manifest are alongside them.
`previous-ps1/` retains the rejected revision for comparison. Current editable
Blender model/atlas/scripts remain under `recomp/assets/models/biomask/ps1/`.

Needs playtest for the user's judgment of the plain tube shape and new textures.
No visual acceptance, commit or push is inferred. Launch the normal player build,
load existing slot 3 and equip B in first person. No save recreation is needed.

## Revision 3 - dark pump-style tubing and stronger painted steel

User likes the direction and requests more texture, darker tubing matching the
Buffalo pump, and metallic treatment like the Flamethrower. This selects texture
refinement, not acceptance closeout. The five-sided plain geometry is retained.

Inspected the Buffalo pump's actual 16x8 texture region: it uses neutral greys
8, 16, 24 and 33. The new original tube pattern uses exactly those four RGB tones,
with near-black grooves and one narrow highlight per band. The preceding wide,
bright silver-looking bands are gone. The pattern is newly authored, not a copy
of extracted retail pixels. The final PNG palette was checked, not merely the
numbers in the authoring script.

Housing and collar textures now use stepped painted steel reflection bands,
bright edge strips, deep recessed shading, lit fastener rims and deliberately
placed short scuffs/edge nicks. This follows the Flamethrower's broad contrasting
metal treatment while retaining the low-resolution style. No photographic noise
or new runtime material/shader. Atlas remains 256x128 with effective 32x16 art
regions; 41 source RGB colours total. The packed Blender texture is updated.

The entire exported mesh header is byte-identical to revision 2: geometry, UVs,
per-face colours, 184 triangles and the 90-triangle visible draw are unchanged.
Runtime transforms, lighting, visibility and the slot-3 buffer reservation fix
are untouched. Original accepted source hashes still match. Revision 2 is backed
up locally under `recomp/analysis/d08a28-revision3/previous-ps1/`.

Build and native near/packet/lighting suite pass. Private current-slot-3
120-setting load/save/reload, cross-level slot 8 and return pass with zero replay
misses, worker or plugin failures. Actual record 10/9/6 captures at 60 cover the
Energy/Flamethrower/Buffalo beside the mask, firing/settling and crouch. Physical
lighting route confirms dry/gas, holstered fallback, redraw, exit and reload;
software-renderer smoke passes. All 41 player-file hashes/mtimes are preserved.
These checks establish the new texture rendering, not user artistic acceptance.

Current executable:
`bf1f526dfe23c6feb5847dbab6313ee16b89a5a23eb24d1492195c8c746067cd`.
Evidence: `recomp/analysis/d08a28-revision3/`, including `comparison-slot3.png`
(previous above/new below, same aspect correction), `references60/`, physical
lighting captures, `slot3-120/`, logs and preservation checks. Diagnostic games
are stopped. Needs playtest; no commit/push or other job started.

## Revisions 4 and 5 - accepted final steel and foundation style

Revision 4 darkened only the central housing material to (0.30,0.32,0.35),
retaining collar gains and adding six small flecks within the tube's existing
8/16/24/33 palette. Geometry and UVs stayed fixed. Build/native tests, private
slot-3 load/save/reload and cross-level return, and physical dry/gas/holstered/
drawn/exit/reload checks passed. Original source and 41 player files were preserved.
Evidence: `recomp/analysis/d08a28-revision4/`.

Revision 5 answered the request to match the Buffalo barrel's gradient more
closely. Actual Buffalo record 6 was brought up and captured before/after in
private Modernized first-person runs, including firing, settling and crouch.
Housing front and grille now have a narrow cool reflection falling into deep
steel, instead of broad uniformly bright bands. Mesh header is byte-identical
to revision 4; tubing and secondary steel pixels are byte-identical. Atlas has
48 source RGB colours; effective grids, runtime transforms and lighting unchanged.
Build and native near tests pass; live render replay plugin failures: zero.
Screenshots were inspected, then the user accepted the visual result.
Evidence: `recomp/analysis/d08a28-revision5/`, including `comparison-buffalo.png`,
`buffalo-before/`, `buffalo-after/`, `results.json` and the previous-source backup.

Accepted executable SHA256:
`93cf0b9bbe3c88cd26ab3ec131ac5dd3923698feacef39a5c477bcf51a526c89`.
The original D08A26 source remains in the parent asset directory; its recorded
hashes match. The accepted PS1 source has its own `accepted-revision.json` manifest.
Source, packed Blender model, atlas, generated runtime headers, exporters,
startup allocation fix and regression tests are part of this closeout.

[Note 265](265-native-mesh-art-style.md) captures the exact recipe, density and
mapping measurements, reference lessons, material gains, RGB555 conversion,
reproduction commands and visual acceptance workflow. D36 ownership/evidence
boundaries remain in force. This is artistic acceptance of the mask and the
bounded recorded tests, not fresh all-campaign or performance certification.
D36H1, D36H2 and D17Z remain Todo; no implementation of those jobs is included.
