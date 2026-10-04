# R01 - DisruptorRecomp reference research

2026-10-04. Research complete. No implementation experiment authorized or started.

**Recommendation: keep the local reference.** The strongest leads are precision
provenance diagnostics, an optional geometry/texture precision mode, and bounded
CPU-accounting experiments. Preserve TTK's accepted D17/D17B/D17O implementation.
Disruptor's advertised features are useful research leads, not proof of a better
solution or of applicability to TTK.

## 1. Reproducible scope and acquisition

| Item | Inspected identity |
| --- | --- |
| Upstream | `https://github.com/Phroster/DisruptorRecomp.git` |
| Project revision | `408f214d3109dbc6cbdded7edd128cbf8de6466a` |
| Commit metadata | 2026-09-25T04:56:28+02:00, `Add a Ko-fi link: sponsor button and README badge` |
| Local reference | `research/DisruptorRecomp/` |
| Framework URL | `https://github.com/RetroPortingToolKit/psxrecomp.git` |
| Framework gitlink | `193a60b805e1eaa853129d6ccf63022440d4b143` |
| Framework changes used by their build | Project `patches/001` through `018`, applied by `build.ps1`; reviewed as diffs, NOT applied locally |
| TTK root baseline | `7e9c85a7d6993ed454651d10370f995ab5a8a51b` |
| TTK framework pin | `08ec704a974b1f3a16335b4afeb340b9eff19926` plus existing local accepted patch stack |
| TTK UI pin | `be8ac1d03ee19d55394b5a5f2d9d1506edd56659` |
| TTK accepted framework patch SHA-256 | `85bc8b943ddf9df8384e7bd34494df9f6aba6839a16804614e907466de1021aa` |

Commands actually used, from the TTK workspace:

```sh
git check-ignore -v research/DisruptorRecomp/probe
git ls-files research
git clone --no-recurse-submodules https://github.com/Phroster/DisruptorRecomp.git research/DisruptorRecomp
git -C research/DisruptorRecomp submodule update --init --depth 1
git -C research/DisruptorRecomp rev-parse HEAD
git -C research/DisruptorRecomp submodule status
```

The first command resolved to `.gitignore:65:/research/`; `git ls-files research`
returned nothing. Final checks also cover real files and the nested framework.
The external checkout is neither a TTK submodule nor vendored source. Leave it
unmodified; do not run its build scripts in this reference checkout. It is
read-only by workflow, not a filesystem permission change. Clone initialization
created ordinary Git metadata; no upstream tracked file was edited.

For reproduction elsewhere, clone the URL, check out the full project revision
above, then initialize its pinned submodule. Do not substitute today's main or
silently update the retained reference. The framework directory is the clean
pin; understand effective behavior by reading it together with the project
patches. Other nested dependencies were not recursively obtained or audited.

Evidence is static source/configuration/patch review. No Disruptor disc was
obtained; neither game was built or launched. Upstream measurement comments are
identified as upstream reports, not independently reproduced benchmarks. Original
Disruptor addresses below come from its authored hooks, expected-opcode patches
and tests, not independent disassembly of its retail executable. TTK comparison
uses current source plus accepted evidence in notes [86](86-d17-acceptance-and-regression-baseline.md),
[89](89-d17-playtest-followup.md) and [92](92-d17o-sky-intake.md), rather than the
historical proposed design at the start of note 80.

Source links at the end are pinned permalinks. `DR` means the external project
root; `FW` means its pinned `psxrecomp/`. Each finding gives files and actual
function/patch names so it can be revisited without guessing from UI labels.

## Reference-use policy confirmed by the user

During R01, the user confirmed that the retained checkout is for research and
learning techniques, with purpose-built TTK implementations rather than 1:1
copies. Apply that policy to future jobs: document the lesson, establish the
TTK-specific producer/consumer and failure case, then design and test our own
implementation. Do not introduce foreign source by transcription or mechanical
translation. This can shorten investigation while preserving TTK architecture
and accepted behavior. It does not establish blanket legal clearance; retain
provenance and the license findings below. Source adaptation is not the planned
route and would require a separately authorized change of scope.

## 2. Architecture comparison

| Layer | Disruptor | TTK and implication |
| --- | --- | --- |
| Translation/runtime | Generated C, psxrecomp CPU/GTE/GPU/devices, native overlay cache with interpreter fallback | Same family, different pin and accepted local modifications. Shared ancestry is not a drop-in patch contract. |
| Title integration | `game.toml` expected-opcode patches; three title C files; activation, function-entry and VBlank plugins | Guarded title hooks, near clipping, independent cameras/aiming, worker redraw integration. Preserve active code identity and scene epochs. |
| Renderer | OpenGL native-wide surfaces, PGXP shadows, ordered primitive submission; project crack/filter patches | Same renderer foundation plus title-produced precise UV/depth, selective host depth testing and live/replay isolation. |
| Geometry | GTE world plus CPU-projected billboards using Q8 trig and reciprocal tables | World and polygonal actors/props, mesh viewmodel, camera-relative sky. TTK cups/bowls are meshes, not sprites (note 89). |
| Timing | One gameplay update/render pass per VBlank after game patches; tick loop consumes elapsed fields | Original variable-timestep compositions around 30/20 Hz; high-refresh views redrawn separately. |
| Extra presentations | Optional completed-image crossfade | Worker-rendered intermediate world/camera/object transforms and late mouse orientation. |
| Product | Windows launcher/installer, fixed 16:9, independent graphics controls | Vanilla/Modernized profiles, Linux baseline, wider aspect choices; modern settings UI and packaging remain backlog. |

`DR/CMakeLists.txt` explicitly builds one PGXP-enabled target by default, disables
the shared UI, Vulkan, rewind and netplay, and force-includes its fast-timing
header into generated game sources. It does not hand-edit generated C. Their
framework's `runtime/src/frame_interpolation.c` is byte-identical to TTK's local
copy; its presence is not a new discovery to import. PGXP/GTE sources differ.

## 3. Geometry wobble and the proposed Original / Corrected setting

**Classification: Directly applicable technique; Potential future feature (D15).**
Scalar table registrations are **Disruptor-specific**.

Sources: [GTE][gte], [PGXP][pgxp], [GPU consumer][gpu], [scalar patch 016][p16],
[title hooks][hooks], [game configuration][game].

The base GTE path computes fixed-point transformed/projected coordinates, then
reduces screen X/Y to integer pixels for SXY and the GPU packets. Losing that
fraction is one source of movement wobble. Earlier quantization in rotation,
translation, fixed-point matrices and game state remains a separate source.
In `gte.cpp`'s RTPS/RTPT projection path, `sx16`/`sy16` exist before the shift by
16; `pgxp_gte_push_sxy` retains them and SZ while the original integer SXY still
reaches guest code. This is retention of precision before final screen rounding,
not smoothing already rendered images and not a floating-point replacement of
all PS1 transforms. The shadow feed occurs after the guest SXY push in code order,
but uses the retained pre-rounding values.

`PGXPValue` shadows registers, RAM words, scratchpad and GTE registers.
`psx_pgxp_load/store/cop2` propagate provenance; CPU mode extends propagation
through supported arithmetic and coordinate packing. `pgxp_get_precise_vertex`
validates packet address, value, generation and coordinate agreement, falling
back through an ambiguity-gated screen-position cache or native integer values.
`gpu.c::prepare_precise_triangle` applies draw offsets to the selected 16.16
positions and passes them to the backend. The GL path consumes and clears the
per-triangle override. Guest collision/AI never read the shadow values.

`game.toml` enables geometry correction, perspective texturing and CPU tracking,
with `pgxp_tolerance=-1`. The ordinary 0.5-pixel clamp rejects many valid
fractions; their comments report much higher coverage with the clamp removed.
These percentages are scene-specific upstream measurements. Mixed precise and
integer edges can open seams, so disabling the clamp in TTK without measuring
coverage would be premature.

Patch 016 adds another tier because enemies, bodies and pickups are projected
on the CPU using Q8 trig, shifts, integer division and a reciprocal size table.
`pgxp_set_trig_tables`, `pgxp_set_recip_table`, `pgxp_set_word_alias` register the
known formulas and coarse/fine camera aliases. `psx_pgxp_alu` and
`psx_pgxp_muldiv` carry an unrounded scalar alongside the real guest integer;
halfword stores can produce subpixel packet coordinates. The title's
`disruptor_mouselook_vblank` registers these tables and aliases. This can remove
precision loss earlier than final projection, but only along recognized paths.

A useful source-review trap: the introductory patch comment mentions a 2-pixel
scalar allowance; the operative `kScalarRelax16` is **16 pixels**, with an extra
half-pixel comparison offset. Do not transplant the comment or tolerance as a
TTK requirement. `pgxp_peek_scalar_vertex` requires usable data for all four
corners before `gp0_exec_textured_quad` diverts a billboard from the integer
rectangle shortcut to the triangle path. This avoids a partly precise skewed
sprite. Scalar stores discard depth validity; scalar position correction alone
does not prove perspective UV coverage.

Coverage is data/provenance-driven, not a player-selectable world/actor/weapon
mask. GTE polygons qualify when tracked; recognized CPU billboards get the new
tier; ordinary HUD/text/weapon rectangles without such provenance retain their
native path. The review does not establish every weapon/effect path. There is no
separate category toggle in the launcher.

**Toggle semantics:** the launcher writes geometry and texture booleans separately
and applies settings on the next launch. `gte_geometry_correction_set` supports
runtime enable/disable, invalidates generations and rederives tracking needs;
that API is not proof of an exposed, validated hot-switch UI. Scalar controls
and several environment settings are cached. Disabling geometry also does not
undo the independent smooth-yaw table correction.

**TTK application:** retain the accepted PS1-character baseline as Original.
`recomp/src/ttk/near_clip.cpp::emit` deliberately keeps original mesh corners
integer-snapped, gives new clipped corners precise coordinates, and supplies
perspective depth/UVs. A future Corrected mode must make those original corners
and their native neighbors agree; simply enabling a global PGXP flag could leave
a visible boundary between host and native paths. Preserve plane-depth evaluation,
near clipping, viewmodel ordering and replay epochs. Original must not mean
reintroducing accepted clipping/depth/idle-sky bugs. Treat D15's geometry and
texture options independently and measure actors, world, weapons, sky and effects
before claiming global correction. No TTK compile/config toggle was changed.

## 4. Gaps, clipping, culling and depth

**Classification: Directly applicable diagnostics; Conceptually useful crack
coverage; not an established fix for D17L/M/N.**

Sources: [patch 014][p14], [patch 015][p15], [seam census][seams], [game patches][game].

Three different operations must be separated:

1. **Coverage cracks:** `crack_fill_px`, `crack_fill_amount`, `crack_fill_grow`
   in patch 015's `gpu_gl_renderer.c` offset triangle edges outward in screen
   space. `gpu_geometry` and `gpu_textured_triangle` draw an enlarged underlayer
   immediately before the unchanged primitive. Opaque precise triangles qualify;
   semitransparent primitives are excluded. The textured underlayer retains
   original corner attributes, stretching mapping only in its exposed rim.
   Earlier attempts to extrapolate perspective mapping failed at steep depth
   ranges. This is overlap, not welded geometry or recovered missing polygons.
2. **Texture/lighting tile borders:** `gpu_tile_blend_prepass`,
   `tile_quad_from_words`, `tile_blend_describe` recognize opaque modulated
   Gouraud-textured quads and match edges by packet coordinate words. Neighbor
   UV/CLUT/page/color data lets patch 015's shader mix both border colors toward
   the same half-and-half result. This reduces texture/lighting discontinuities,
   not visibility holes. Exact screen-coordinate matches are not a general
   world-space adjacency proof.
3. **Missing submissions/culling:** title patches widen portal/frustum/sprite
   bounds. Neither crack fill nor tile blending brings back a culled surface.

The launch default rim is 0.45 native pixels. `crack_fill_amount` grows toward
1.2 at a 112-pixel longest edge, starting its ramp at 32 pixels. Whole-pixel
corners trigger a minimum 1.0 where the near-width setting allows it, based on a
heuristic for game clipping. Matched tile-blend borders and quad diagonals can
suppress the rim to prevent doubled stripes. This costs extra geometry and can
expand silhouettes, overpaint foreground edges or alter texture borders.

The underlying renderer preserves PS1 primitive ordering and mask/STP behavior.
Its perspective shader supplies clip W for UV interpolation but clip Z remains
zero: perspective texturing is not a scene depth buffer. The reviewed project
patches do not add an equivalent of TTK's near-geometry host-depth system.
Framework `gte.cpp::nclip` can compute a checked precise winding sign, but is
gated by `gpu_ws_precise_nclip_enabled`; API existence is not evidence that all
Disruptor culling uses it. Game clipping can still emit rounded/untracked corners.
PGXP cannot prevent a guest from omitting a polygon before GPU submission.

The most valuable diagnostic is `seam_census_note` / `gpu_seam_census_dump` plus
`tools/seam_census.py::cracks` and `tjunctions`: record actual final positions,
packet address, primitive and precision provenance, then identify nearby distinct
vertices and vertex-to-edge gaps. Its frame boundary uses GP0 clears rather than
VBlank to avoid splitting a draw pass. Its spatial proximity tests are **candidate
finders**, not proof of shared topology or visible cracks. TTK should add object,
source polygon, live/worker identity and depth/ordering information, rather than
adopt the screen-space heuristic as a bug classifier.

TTK already rejected outward rounding after dark dotted UV-edge artifacts
(`near_clip.cpp::round_outward`). Its accepted depth/ordering fixes address
problems this fill does not solve. D17L's disappearing cup, D17M's visible-through
ammo and D17N's diagonal wall artifact need captured failing primitives first.
Use a census to determine whether geometry was submitted, clipped, depth-rejected,
misordered or separated by a genuine subpixel gap. Do not apply rims globally.

## 5. Perspective-correct textures

**Classification: Directly applicable; Potential future feature (D15). Existing
near-eye correction is already implemented in TTK.**

Sources: [GPU][gpu] `prepare_texture_triangle`, `gpu_texture_correction_stats`;
[GL renderer][gl] `TEX_VS`, `TEX_FS`, `glb_set_perspective_triangle`.

PS1 GPU packets contain screen coordinates and UVs without the depth needed for
perspective interpolation. The framework retains projected SZ with provenance.
`prepare_texture_triangle` requires a valid packet source and a matching nonzero
Z at all three vertices via `gte_precision_load_word`. It builds normalized
reciprocal depths q=(1/Z)/max(1/Z). The vertex shader uses W=1/q and premultiplies
screen NDC by W: screen location stays the same after division, while `smooth`
UV varyings receive perspective interpolation. A separate `noperspective` UV
varying retains affine mapping when disabled or depth is unavailable. Vertex
color remains noperspective. This requires no replacement textures.

The function's historical SWC2-only comment is narrower than the current
`gte_precision_load_word` implementation, which delegates to validated PGXP
word shadows. Follow the actual producer/consumer chain when auditing coverage.
No reliable depth means affine fallback, not guessed perspective. Scalars that
repair sprite positions do not automatically supply Z. CPU-built UI and flat
billboards must not inherit arbitrary world depth.

TTK's `near_clip.cpp::emit` already submits exact UV, view depth and separately
evaluated raster-plane depth through `psx_mod_gpu_host_vertex_depth`;
`gpu_gl_renderer.c::gl_set_precise_uv` / `gl_set_host_depth` consume that path.
Thus renderer support exists, but a complete optional full-scene mode still
needs provenance coverage across native packets, clipped packets, replay and
state load. Do not market the existing near-eye path as a global player option.
Separate geometry precision from texture precision in D15 experiments so their
appearance and seam effects can be evaluated independently.

## 6. Skies

**Classification: Disruptor-specific representation; Conceptually useful
clipping/wrap tests; Potential future feature only for a separately scoped TTK sky.**

Sources: [game.toml][game] `ws-sky-*`, [sky test][skytest] `Machine.run`.

Disruptor's documented routine `0x8003AF68` projects a portal opening, clips a
horizontal span, and emits textured rectangles from a circular **1280-pixel
panorama**. The patches widen portal and full-background paths to [-64,384].
Negative panorama starts are normalized modulo 1280; the identical shift is
applied to the end to preserve span and phase. Existing positive wrapping handles
the remainder. `tools/test_sky_window.py` executes the relevant MIPS blocks,
including branch/load delays, against a locally extracted executable and the
expected-opcode patches. It tests off-screen rejection, near defaults and wrap
phase. We read this test; we did not run it without their disc.

TTK's sky is not that panorama. Note 92 establishes `0x800388e4` drawing a
camera-facing colored backdrop and two textured curved cloud meshes, with
separate integer phase rotations and eye-relative matrices. D17O now exempts
three authenticated matrix calls from world-object interpolation
(`sky_render.h::sky_transform_call`, `SkyRenderIdentity::valid`). Keep that fix.
There is no Disruptor cubemap implementation here to transplant. Useful lessons
are to identify sky coordinate space explicitly, keep clipping and animation
phase coherent at wider bounds, and test seams at wrap/near-plane transitions.
A conventional TTK skybox would require art/projection/design work; flattening
its meshes into a panorama would lose behavior and is not recommended by R01.

## 7. High refresh and interpolation

**Classification: Already solved better in TTK for this project's requirements.
Timing measurements and negative findings are Conceptually useful.**

Sources: [game patches][game], [activation hook][hooks]
`disruptor_widescreen_activate`, [scheduler][interp], [GL][gl]
`interp_capture`, `interp_present_source_interval`, `INTERP_FS`.

Disruptor has two distinct features:

- Real approximately 60 Hz engine passes: remove redundant waits, retain ONE
  blocking VSync per pass, and consume elapsed VBlank ticks in the gameplay logic
  loop (`func_80039450` via the loop at `0x80043404`). The old 30 Hz pass consumed
  two ticks; normal new passes consume one. The attract demo is explicitly kept
  at two fields because it consumes one recorded input per pass and two fixed
  logic steps. Weapon/psionic double-tap windows change from 30 to 60 passes.
- Optional high-refresh image blending: captures completed source images into
  GL textures, schedules presentations between source deadlines and crossfades
  previous/current pixels. The change-adaptive shader uses RGB difference to
  switch high-change pixels toward the previous or current frame at alpha 0.5.
  It has no motion vectors, actor identities, interpolated camera transform or
  new visibility walk. Actor/camera geometry still comes from source passes.

It avoids intermediate-geometry inconsistency by not generating intermediate
geometry. Disocclusions can double-image or step; this is not a cure for culling
flicker. Source dimensions, source path, mode suspension and rate changes reset
history. The scheduler preserves an anchored source deadline, coalesces missed
output deadlines and reanchors after sustained overruns. Presentations run in
the render interval on the existing context; extra image blends do not decouple
heavy guest work into a TTK-style worker renderer.

Their source explicitly keeps blending **off by default**, describing ghosting
and uneven spare-time presentation. Also, `PSX_VBLANK_DIV` in patch 007 is a
retained diagnostic experiment: `run.ps1` rejects using it because it accelerated
game speed. A patch existing in the tree is not a recommended production path.

TTK's `frame_replay.cpp` snapshots endpoints, interpolates camera and keyed object
transforms, reruns composition in isolated worker processes, rebuilds visibility
for the replay eye and uses recent mouse orientation (`apply_late`, `submit_due`).
Discrete membership/animation changes still need careful handling. Its
`render_worker.c` and `render_replay.c` isolate state and GPU surfaces. Note 86
records accepted 120 FPS quality and the distinction among Game/Guest/Unique/FPS.
Retain all of that. Raising TTK logic pass rate to 60 is especially inappropriate:
TTK has per-update effects as well as delta-scaled behavior, unlike Disruptor's
specific elapsed-tick loop. D17 already rejected that tradeoff.

## 8. Mouse and low latency

**Classification: late sampling is Directly applicable but already present in the
shared framework; fractional-angle consistency is Conceptually useful;
TTK's current late camera is Already solved better for high-refresh full aiming.**

Sources: [mouse patch][p04], [runtime frontend][main], [title hooks][hooks].

`main.cpp::sdl_vblank_present_body` initially samples input, waits for the frame
pacer, then with `g_low_latency_input` pumps events, updates controllers and
resamples SIO before `mod_call_frame_hooks`. The mouse event watch from patch 004
feeds accumulated relative motion, independent of which SDL event loop consumes
it. The post-wait pump makes newly arrived events available to the title callback.
This reduces stale sampling for the NEXT guest pass; it does not rotate a finished
image or provide a new camera for each image-blended present. The latency-ring
mark is restamped; it is not an end-to-end input-to-photon measurement.

`disruptor_mouselook_vblank` accumulates fractional yaw rather than rounding away
slow mouse motion in the game's 256-direction byte. `trig_slot_publish` restores
the previous table slot, writes the fractional angle's Q8 sine/cosine into the
current slot, and `disruptor_pitch_hook` refines the camera-only matrix to Q12
using a return-address check. This aligns portal traversal, CPU sprites and
movement with the camera. The UI's smooth-turning wording suggests spreading
motion over frames, but the key source technique is **angle precision and shared
consumers**, not a timestamped high-refresh resampling algorithm.

Crucially `DISRUPTOR_PITCH_ENABLE` is **0**: the header's initial full-mouselook
comment is stale. Pitch was disabled because rotating only the world matrix
separated ground sprites from it. TTK already supports independent yaw/pitch,
first/third-person presentation and weapon aiming. Do not copy the yaw-table
writes or focus-only capture behavior into TTK.

TTK's `pc_input.cpp` timestamps mouse reports in the presenter clock;
`modern_controls.cpp::late_view` and `frame_replay.cpp::apply_late` consume a
coherent look timeline. Its SDL3 X11 timestamp repair, capture/pause leases and
world-history policy are accepted. The useful comparison experiment is to measure
sample age at poll, guest consumption, worker submission and actual presentation
under load, not to replace the input path. D17G/H/I need this distinction between
simulation translation, late rotation and presentation misses.

## 9. CPU throughput and host cost

**Classification: overclock concept already implemented in TTK; fast-accounting
experiments Conceptually useful for D23 / D17G/H/I / D18C.**

Sources: [overclock patch 010][p10], [fast timing header][fast],
[game wait patches][game], [run defaults][run].

`psx_cpu_oc_scale` scales CPU charges by 100/percent, carrying a remainder.
`psx_advance_cycles_raw` handles absolute guest-clock waits without scaling;
GTE/muldiv completion waits and idle skips use that path. Devices retain the
ordinary guest clock, so more CPU work can fit between fields. The default 300%
is chosen from upstream heavy-scene measurements, not a portable optimal value.
This cannot make expensive host instructions free; it may increase host work.

TTK already implements `psx_overclock_compress` / `psx_overclock_renew` in
`runtime/src/psx_cycles.c`, with a gameplay lease renewed by
`modern_controls.cpp::overclock_lease`, idle/explicit-skip exemptions and adjusted
completion stamps. `run.py` enforces stock 100 for Vanilla; Modernized supports
saved choices. TTK compresses the unsynchronized interval at device service to
avoid changing generated-code/cache identity. Disruptor scales at the charge
boundary. Neither static comparison establishes that replacing one is safer.
Preserve TTK's lease, which avoids menu/FMVs spinning at an unnecessary overclock.

The separate `disruptor_fast_timing.h` is more interesting: cumulative compile-time
levels remove per-block diagnostics, disable game instruction-cache simulation,
charge static basic-block cycles, directly read eligible RAM loads with a flat
charge, and inline the common branch-edge check. `disruptor_fast_edge` services
pending interrupts, accumulated charges (96-cycle threshold) and repeated polling
edges; it also handles debug/save maintenance starvation introduced by reducing
check frequency. These are timing approximations with explicit fallbacks, not
merely an overclock slider. Device/MMIO/BIOS paths keep their separate contracts.

`game.toml::vsync-wait-skippable` rewrites Disruptor's timeout/store loop into a
single store-free Vcount polling block recognized by idle skipping. That exact
patch is game-specific and removes a timeout; do not copy it into TTK. It teaches
us to distinguish useful game execution from expensive waiting before increasing
CPU throughput. Offline `warm_cache.ps1` and disabled in-game autocompile avoid
compiler hitches; TTK already has checked native movie/math shard preparation.

TTK workers already use `g_psx_render_untimed`, disable cache/load-delay modeling
for frozen rendering, and special-case cycle accounting (`psx_cyc_step`). TTK
also batches charges and has `test_pipeline_batch_deadlines.py`. Therefore the
next step is an attribution measurement of residual live/worker overhead, not
assuming Disruptor's five levels are five missing TTK optimizations. Only try a
small, measured accounting fast path after checking MMIO, IRQ, save/debug polling,
overlay fallback and worker equivalence. No global accurate-timing removal.

## 10. Widescreen, display, supporting systems

| Finding and exact sources | Mechanism and applicability | TTK mapping |
| --- | --- | --- |
| `game.toml::ws-*`; `disruptor_terrain_window_hook` [hooks][hooks] | Bias portal windows by +64, widen to 448, subtract bias on renderer entry; widen frustum rays and four sprite families. Fixed 16:9 reveals real side geometry at unchanged central projection. **Conceptually useful**, literal offsets **Disruptor-specific**. | D14 already widens TTK's root view rectangle and supports 16:10/21:9/auto. Do not replace with dozens of immediate patches. Keep category coverage tests for future FOV changes. |
| `nw_flat_backdrop`, [patch 003][p03] | Route backdrop flat triangles through normal batching instead of one immediate draw per primitive. **Conceptually useful**, existing shared batching must be checked first. | D23; preserve accepted order-sensitive batching. |
| `smart_filter_mode`, `smart_filter_note` [patch 015][p15]; `tools/test_filter_selection.py` | Primitive-kind metadata travels per vertex without splitting batches. Gouraud terrain stays bilinear even when axis-aligned or precision provenance is absent; billboard/HUD-like paths use an edge-directed filter. **Potential future feature**; title classification is **Disruptor-specific**. | D16: categorize by verified producers, not merely GP0 opcode or screen shape. TTK's 3D cups demonstrate why. |
| `disruptor_pgxp_autopause_vblank`, `disruptor_pgxp_note_3d` [hooks][hooks] | Pause hook work after 30 fields without known 3D calls; invalidate shadows on resume. **Conceptually useful**. | D15/D18/D23, if full PGXP tracking becomes expensive. TTK needs its own authenticated world/menu/FMVs signals and replay lifecycle. |
| `gl_dxgi_present_init`, `gl_dxgi_present_frame` [patch 012][p12] | D3D11/DXGI flip-discard swapchain, tearing capability and GL/DX interop; fallback to GL swap if unsupported. **Potential future feature**, Windows-specific. | D24. Not a Linux latency fix. Driver and monitor validation required. |
| [patch 009][p09] present-vsync ownership policy | Permit integer refresh multiples for ordinary 60 Hz presentation. **Conceptually useful**, no new intermediate geometry. | D17/D24 display tests; not a substitute for TTK's high-refresh scheduler. |
| [patch 017][p17] FMV content-fit/filter; `run.ps1` | Disc-specific 320x180 image inside 320x240, separate bicubic video filter and aspect-preserving fit. **Potential future feature**. | D18; identify TTK's actual encoded content before any crop. Menus and movies remain separate from 3D view. |
| `recomp_audio_drc.h::rab_config_defaults` [patch 006][p06] | Increase target queue from 180 to 320 ms and ring from 280 to 520 ms. **Conceptually useful evidence of a tradeoff**, not an audio correctness fix. | D18C: can mask short stalls at added latency; cannot solve sustained starvation. Do not copy as a low-latency improvement. |
| `boot_state_check_buffer` [patch 013][p13] | Compare low ABI half while allowing base/PGXP flavor change; restore invalidates precision shadows. **Conceptually useful** only after compatibility proof. | D15/D20. Preserve TTK codegen, identity, host metadata and state-load guards; never remove broad validation to make a save load. |
| [patches 008/011][p08] / [overlay reset][p11] | Recover native clean-text candidates and rearm code watches after boot reset. **Conceptually useful** lifecycle cases. | D22/D23: compare existing TTK dirty-code restore and worker invalidation before proposing changes. |
| `disruptor_wheel_select_vblank` [weapon][weapon] | Queue wheel requests and drive the game's button/selection protocol; retain game-owned state. **Already solved in TTK** for existing weapon/inventory controls. | D08A family; retain current controls and ownership rules. |
| `release/launcher.cpp::defineSettings` and settings persistence [launcher][launcher] | Independent display, geometry, texture, filtering, input and timing options; next-launch application and per-page defaults. **Conceptually useful** for player choice. | D19 still requires its own plan/artifact first. Do not confuse independent options with separate per-object precision controls. |
| `release/builder.cpp`, `release/build_release.py`, [release process][release] | Build from owned media locally, verify recipe/generated objects/executable, prepare native cache, exclude game data from installer. **Conceptually useful** reproducibility pattern. | D24, with TTK's own disc hashes, licensing and Linux/Windows toolchains. No binaries imported. |
| `tools/pass_cost.py`, `frame_perf_dump.py`, `pace_report.py`, `motion_log_report.py`, `frame_diff.py` [tools][tools] | Separate source passes, host cost, present spacing and motion; source instrumentation includes bounded census output. **Directly applicable measurement concepts**. | D17G/H/I, D18C, D23. TTK already has richer FPS categories and frame traces; add missing attribution rather than duplicate counters. |

## 11. Licensing and provenance

The [project license][license] and [framework license][fwlicense] identify
PolyForm Noncommercial 1.0.0. Project scope explicitly includes patches, mods,
launcher, scripts, configuration and documentation; retail Disruptor is excluded.
Their notices name Phroster/contributors and Matthew Stanley/PSXRecomp. Public
visibility is not a permissive copying license. Noncommercial study/experimentation
is within the stated personal-use scope; commercial use is not generally granted.

A material discrepancy: both inspected LICENSE bodies differ from the
[official PolyForm text][polyform], omitting its separately headed distribution,
notices, changes/new-works and patent-grant sections and rewriting the copyright
grant. The project still includes Required Notice lines. Record the exact files;
do not silently treat this as a verbatim standard license or resolve the
interpretation by assumption. Before importing code, clarify intended terms and
check compatibility, including distribution notices. No imported code is needed
for this research result.

| Activity | Research policy |
| --- | --- |
| Read and learn | Keep revision/source citations and distinguish evidence from inference. This job did that. |
| Independently implement a general technique | Derive TTK requirements, algorithms and tests independently; do not translate their source line by line and call it independent. Record inspiration. No implementation was undertaken here. |
| Adapt source | Treat as third-party code with applicable restrictions, copyright/notices and dependency review; seek clarification of the license discrepancy first. |
| Copy source verbatim | Same obligations, plus exact file/revision attribution. No code was copied into TTK. |

`FW/runtime/src/pgxp.cpp` asserts an independent implementation from public PS1
and PGXP descriptions, with DuckStation/Beetle used as behavioral references.
That is an upstream provenance assertion, not an audit we independently verified.
Their dependency and release license inventory includes separate component terms;
the root license does not relicense SDL, toolchain/runtime libraries or retail
assets. We did not audit every transitive dependency or establish legal clearance
for future distribution. The immediate outcome is an original technical review
and a private reference checkout, not a code reuse decision.

## 12. Ranked experiments for a future selected job

These are recommendations, not newly started implementation jobs. R01 is complete;
D15 and the focused renderer/performance jobs retain their existing statuses.

| Rank | Bounded experiment | Value / risk and acceptance |
| --- | --- | --- |
| **1** | **Primitive/provenance census on the remaining reported artifacts** | High value, low behavioral risk. Independently record final screen coordinates, native/host origin, object/face, epoch, clipping and depth/OT decision for D17L/M/N. Private reproducible states; stable bar props as controls. Accept only an identified failing primitive and cause category, not a proximity count. No rim or culling change in this experiment. |
| **2** | **D15 Original / Corrected geometry A/B, with separate perspective toggle** | High feature value, medium risk. Start with one known static scene and coherent host/native corner policy, then actor, weapon, sprite and sky coverage. Four geometry/texture combinations; 60 and primary 120, 180 robustness; state-load/scene transitions. Require off-path image equivalence to the accepted baseline, coverage counters, no new seams/occlusion errors, unchanged timing and a measured performance budget. Original remains default. |
| **3** | **CPU-accounting and wait-cost attribution, then one small fast-path trial if justified** | High performance value, low risk for measurement, medium/high for timing changes. Separate live guest, frozen worker, idle waits, dispatch, GL and audio. Compare 120 first and heavy-scene 180. TTK already skips substantial worker timing: demonstrate residual cost before editing. Require unchanged guest cadence and deadlines, worker endpoint equivalence, no audio starvation and intact save/debug responsiveness. |
| 4 | Crack underlayer only on a confirmed coverage defect | Medium value, medium/high visual risk. Only after rank 1 proves a crack. Test thin silhouettes, UV borders, transparency, near-eye depth and existing coplanar blood; withdraw if it hides a deeper submission bug. |
| 5 | Category-aware filtering sample | Medium value, medium risk. D16, one verified world/actor/weapon/HUD set with palette and transparency changes. Do not port Disruptor's opcode classifier as universal truth. |
| 6 | Windows presenter and reproducible installer prototype | Valuable for D24, platform/driver and packaging risk. Defer until that job's dependencies and licensing are addressed. |

For D17G/H/I and D18C, rank 3 is an investigation lead, not evidence of a shared
cause. For D17O, retain the accepted repair and original sky character; no new sky
job is started. A future full skybox is lower priority than the three experiments
above and has no directly reusable implementation in this reference.

## 13. Completion evidence and limits

R01 acceptance is a code-review deliverable, so no gameplay playtest is needed to
close it. Verified project/framework revisions, ignored checkout boundaries,
clean external tracked trees, stable source references and documentation diffs.
Existing TTK framework changes remain untouched. No build, launch, generated C,
media, save/card, profile, gameplay or renderer edit occurred. No source from the
reference is staged in TTK. This report does not certify Disruptor campaign
behavior or claim that any proposed experiment improves TTK before it is tested.

Current player launch, when desired:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

## Pinned source index

[gte]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/gte.cpp
[pgxp]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/pgxp.cpp
[gpu]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/gpu.c
[gl]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/gpu_gl_renderer.c
[interp]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/frame_interpolation.c
[main]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/runtime/src/main.cpp
[fwlicense]: https://github.com/RetroPortingToolKit/psxrecomp/blob/193a60b805e1eaa853129d6ccf63022440d4b143/LICENSE
[hooks]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/src/disruptor_widescreen.c
[game]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/game.toml
[fast]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/src/disruptor_fast_timing.h
[run]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/run.ps1
[seams]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/tools/seam_census.py
[skytest]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/tools/test_sky_window.py
[weapon]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/src/disruptor_weapon_select.c
[launcher]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/release/launcher.cpp
[release]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/release/README.md
[license]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/LICENSE
[tools]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/tools/pace_report.py
[p03]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/003-flat-backdrop-batched.patch
[p04]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/004-mouse-wheel-turn.patch
[p06]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/006-audio-cushion.patch
[p08]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/008-overlay-clean-text-miss-candidates.patch
[p09]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/009-integer-multiple-vsync.patch
[p10]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/010-cpu-overclock.patch
[p11]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/011-rearm-overlay-watches-after-boot-reset.patch
[p12]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/012-gl-dxgi-flip-model-present.patch
[p13]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/013-savestate-accept-other-overlay-flavor.patch
[p14]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/014-gpu-seam-census-and-tile-blend.patch
[p15]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/015-gl-crack-fill-and-smart-filter.patch
[p16]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/016-pgxp-scalar-tracking.patch
[p17]: https://github.com/Phroster/DisruptorRecomp/blob/408f214d3109dbc6cbdded7edd128cbf8de6466a/patches/017-fmv-content-fit-and-filter.patch
[polyform]: https://polyformproject.org/licenses/noncommercial/1.0.0
