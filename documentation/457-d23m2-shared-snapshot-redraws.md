# D23M2 - Redraws read the frame's VRAM from its snapshot, and repeat no uploads

Status: Done (user-accepted 2026-11-12 after playing UI slots 9 and 1 at 180 Hz). Job board: D23M2 (direct redraws, step 2;
step 1 is D23M, [note 448](448-d23m-direct-redraws.md)). Framework only
(`recomp/psxrecomp/runtime/src/gpu_gl_renderer.c`, `include/render_thread.h`),
exported in `recomp/patches/time-to-kill-accepted-source.patch`. The
emulation thread's own redraws (validation, the single-thread path), Vanilla,
ORIGINAL and Classic draw as before. Evidence labels: CONFIRMED (our
measurement or code), INFERRED.

## 1. What a render thread paid that the redraw timeline did not show

A redraw on a render thread (D17AW, D23O) draws into that thread's own copy of
the emulated PS1 VRAM surfaces: the colour surface at the internal scale with
its depth-stencil, the 1 MB raw mirror the textured shader samples, and the
widescreen surfaces. The D23J/D23M figures per redraw (`PSX_GPU_PHASES=2`) are
the GPU timeline from the redraw's begin to its end. Two copies sit outside
that window (CONFIRMED in code):

- **capture**: when the game publishes a frame, the emulation thread copies
  all its surfaces into a snapshot (`gl_rt_capture`);
- **load**: before its first redraw of a snapshot a render thread copies all
  of them again into its own (`gl_rt_load`), and since D23M it copies back,
  before the next redraw of the same snapshot, what the previous one wrote.

New developer timers around both (`[gpu copies]`, `PSX_GPU_PHASES=2`),
offscreen, turning at 180 Hz, the player's profile (CONFIRMED):

| View | Capture | Loads, thread 0 | Loads, thread 1 | All copies |
| --- | --- | --- | --- | --- |
| UI slot 2 | 0.34 ms x 30/s | 0.44 ms x 30/s | 0.46 ms x 9/s | about 27 ms of GPU a second |
| UI slot 8 | 0.20 ms x 30/s | 0.26 ms x 30/s | 0.27 ms x 7/s | about 16 ms a second |

At about 150 drawn redraws a second that is 0.1-0.18 ms per redraw on top of
the redraw itself (GPU timestamps from two contexts interleave, so these are
upper bounds; the whole-GPU figures in section 5 are the cleaner measure).

## 2. What a redraw really reads

The job asked to measure first how many of a redraw's uploads repeat the
snapshot's bytes. Developer counters on the render threads (`[upload
repeat]`, UI slots 2 and 8, about 1,600 redraws, CONFIRMED): every redraw
makes 7 uploads (452 pixels in all: the game's CLUT and small texture
updates), and **all 7 repeat the snapshot's bytes**, over pixels the redraw
has not drawn yet.

Then, with copy-on-read in place (section 3), the tiles a redraw actually
loads (`[lazy load]`, `[lazy rows]`): 16 colour tiles and 22 wide-surface
tiles a redraw, all in tile row 7 (VRAM rows 224-255, the bottom of the drawn
buffer the frame's clearing fill does not reach), while 266 tiles a redraw
are written whole by a fill, an upload or a wide clear before anything reads
them. Of the 60 MB a load copied, a redraw reads well under 2 MB.

## 3. Change

### 3.1 Copy-on-read

The snapshot stays referenced, unchanged, for the whole session (D23M), so a
render thread's surfaces now start every session invalid and are loaded from
the snapshot tile by tile (32 x 32 VRAM pixels; rows of 32 on a wide surface)
the first time the session reads a tile or writes it in part:

- every write already reports its rectangle (`rr_cow_hr`, `rr_cow_wide`: the
  draw area at a primitive, fills, VRAM copies, uploads, depth clears, wide
  clears, the band); a tile under a write that covers it whole (an upload, a
  non-wrapping fill, a wide clear: colour, stencil and depth replaced) becomes
  valid without a copy, any other is copied first;
- every read that is not a draw goes through `lz_ensure`: the pack pass (also
  behind the 16-bit texture check), the centre copy into the wide surface,
  the band capture, VRAM copy sources; the stencil rebuild reads whole
  surfaces and loads everything first;
- the raw mirror (1 MB) is copied whole with a new snapshot, and again for a
  later session of the same snapshot only when the last one wrote it (an
  upload it kept, a pack);
- nothing is put back at the end of a session: the D23M deferred restores
  are no longer needed on this path;
- depth-stencil is copied with a tile only when the session can observe the
  stencil (D23J rule `s_rr_ds_skip`), and so the capture copies colour only
  while the stencil is stale (rebuilt from colour before any mask test, and
  depth is cleared by every redraw before its first host triangle);
- a remembered fully valid rectangle per surface keeps the per-primitive
  check to one comparison.

Developer: `PSX_REPLAY_LAZY_LOAD=0` keeps the full loads and restores;
`PSX_REPLAY_CAPTURE_DS=1` keeps the full capture; `PSX_REPLAY_COW_CHECK=1`
turns copy-on-read off (its check compares restored surfaces).

### 3.2 Uploads that repeat the snapshot

A byte comparison alone would not prove an upload redundant: the colour
surface there could hold a later draw over the same bytes. The renderer now
keeps, in every instance, a short list of rectangles whose colour, stencil and
raw mirror are exactly an upload of the CPU array's current bytes (an upload
adds its rectangle once it has landed; every draw, fill, copy, depth clear and
pack over it removes it). A snapshot carries the list (`GlRtSnap.coh`). A
render thread's redraw skips an upload when the rectangle lies inside one of
the snapshot's, its bytes equal the snapshot's, and the redraw has not written
there yet: the surfaces already hold exactly what it would write (and with
copy-on-read, the tile simply stays to be loaded from the snapshot).
Developer `PSX_REPLAY_UPLOAD_SKIP=0` keeps every upload.

### 3.3 Developer measurements added

With `PSX_GPU_PHASES=2`: `[gpu copies]` (capture and load GPU time),
`[upload repeat]`, `[upload skip]`, `[lazy load]`, `[lazy rows]`, a `load`
kind in `[gpu kinds]`, and `[gpu overdraw]` (samples passing all tests over
each redraw's scene, against its output pixels).

## 4. Pictures (CONFIRMED)

`PSX_RT_VALIDATE=1` (every 8th render-thread image compared, RGBA exact,
with the same redraw drawn by the emulation thread), turning, walking and
firing at 180 Hz:

| Build | UI slot 2 | UI slot 8 |
| --- | --- | --- |
| copy-on-read | 303 of 303 | 603 of 604 |
| + upload skip | 536 of 537 | 605 of 606 |
| + colour-only capture | 554 of 554 | 602 of 603 |
| + fast path (final) | 581 of 582 | 610 of 610 |
| baseline (matched, unmodified) | - | 607 of 608 |

Every exception is the first compared redraw at the moment the level's lamp
bake is uploaded to the GPU (`[gl bake] ... generation 1` on the line before
it), where the two draws compared saw different bake generations: 194,366 or
196,445 pixels in UI slot 8 (the identical difference appears with the
unmodified baseline), 985-1,073 pixels one level apart in the sky band of UI
slot 2 (the rounding recorded in note 454 before this work). No difference
anywhere else. 0 full loads, 0 whole-surface reads, 0 tiles without a
snapshot surface; 11,837 of 11,837 redraw uploads skipped.

## 5. Cost (CONFIRMED)

Matched builds, the unmodified binary of this session's start against the
new one, alternating, 15 s each, turning all the time (`SPIN=dx=3`) at
180 Hz, offscreen on the GTX 1080 Ti, the player's profile, private card
copies; whole GPU clock-weighted (`nvidia-smi`), redraws on the busiest
render thread (`PSX_GPU_PHASES=2`). Final build (2 runs each) and the build
before the CPU fast path (3 runs each, same GPU work):

| View | Whole GPU | GPU a redraw (timeline) | of it uploads | Copies outside redraws |
| --- | --- | --- | --- | --- |
| UI slot 2, before | 23.8% / 23.1% | 0.835 / 0.870 ms | 0.039 ms | about 27 ms a second |
| UI slot 2, after | 22.0% / 21.5% | 0.757 / 0.789 ms | 0.005 ms | 3.2-3.4 ms a second |
| UI slot 8, before | 20.8% / 20.2% | 0.952 / 1.005 ms | 0.038 ms | about 16 ms a second |
| UI slot 8, after | 19.3% / 19.4% | 0.937 / 0.858 ms | 0.005 ms | 3.5-5.1 ms a second |

Every run held 178-180 presents a second. The "before" copies are from the
timers of section 1, whose build drew exactly like the baseline. GPU timeline
figures vary by up to 0.2 ms between single runs in UI slot 8 (one run of each
build read 1.05-1.07 ms); the whole-GPU figure is steadier: 1.5-1.8 points
lower in UI slot 2 and 0.8-1.5 lower in UI slot 8, about 7% of the GPU's work.
Counting each redraw's share of the copies, GPU a redraw went from about 1.0
to 0.78 ms in UI slot 2 and from about 1.05-1.1 to 0.88-0.97 ms in UI slot 8
(INFERRED from the columns).

## 6. CPU (CONFIRMED)

The first build of copy-on-read cost each render thread 8-12 more points of
a core: every primitive reports its draw area, and the check walked all 128
tiles of it each time. A remembered fully valid rectangle per surface ends
almost every call at one comparison. Final build, same runs: UI slot 2 total
266% against 268%, render threads 41% / 15% against 41% / 16%; UI slot 8 242%
against 243%, render threads 31% / 3% against 32% / 3%; workers and the
emulation thread unchanged.

## 7. Against the acceptance

D23K's baseline (note 448, before D23M) was 1.008 ms a redraw in UI slot 2
and 0.990 ms in UI slot 8, without the copies. Now about 0.76-0.79 and 0.86-0.94 ms with the
copies' share mostly gone: 22-25% and 5-13% less on the redraw timeline,
about a fifth less in UI slot 2 counting the copies. The third is not
reached. What is left of a redraw is almost all the scene itself: textured
batches 0.58-0.60 / 0.72-0.78 ms, and `[gpu overdraw]` measures **2.8-2.9 samples passing per
output pixel**: the PS1's painter order (back to front, the ordering table)
shades every layer of every pixel through the full lighting shader. Drawing
opaque host geometry front to back under its exact host depth (the ordering
table kept for translucent and depthless primitives) is the next large GPU
saving and is on the board as its own job (D23U). Removing the remaining
VRAM-shaped work (band copy 0.03, depth clear 0.015-0.02, mirror 0.01, wide
clear 0.01, fill 0.008 ms) by drawing straight into an output-sized target
would save about 0.07 ms more; it is on the board as D23M3.

## 8. Limits and next

- The capture still copies the whole colour surfaces (0.09-0.1 ms, 30 a
  second); redraws read only tile row 7 and what they pack. A capture of only
  what redraws read would need a guaranteed exact fallback for a redraw that
  reads elsewhere; not built.
- Playtested by the user at 180 Hz in UI slots 9 and 1 and accepted
  (2026-11-12): "performance/gameplay-wise ... its perfect"; the GPU still
  reaches 49% for a very short time in the heaviest part, with no impediment.
