# D23M3 - Output-sized redraws: measured first, not worth building yet

Status: measured and designed (2026-11-12), Needs review; the build is split
into D23M3B, ordered after D23V. Job board: D23M3. Evidence labels:
CONFIRMED (our measurement or code), INFERRED.

## 1. What the job would remove

A render thread's redraw draws into VRAM-shaped surfaces (the colour surface
at the internal scale with its depth-stencil, and the widescreen surface),
then copies the canonical centre into the wide surface (`wide_blit_center`),
copies the displayed band into the cache image (`rp_copy_band`), mirrors every
batch that reaches a margin into the wide surface, and clears depth over the
VRAM draw area and the wide rows (`hz_clear_area`). D23M2's GPU timeline put
that at about 0.07 ms a redraw (note 457 section 7). Measured again here, one
render thread, per redraw (CONFIRMED, `PSX_GPU_PHASES=2`):

| Kind | UI slot 2 | UI slot 8 |
| --- | --- | --- |
| band (centre and band copies) | 0.030 ms | 0.030 ms |
| depth-clear | 0.023-0.030 ms | 0.016-0.017 ms |
| mirror | 0.009-0.011 ms | 0.010-0.011 ms |
| wide-clear | 0.009 ms | 0.009 ms |
| fill-copy | 0.008 ms | 0.008 ms |

## 2. Do redraws read back what they draw? (CONFIRMED)

An output-sized target has no VRAM around the band, so it must fall back to
the VRAM path for any redraw that packs drawn pixels into a texture, copies
from them, reads them back to the CPU, or draws in more than one draw area.
New developer counters (`[m3 reads]`, `PSX_GPU_PHASES=2`), turning at 180 Hz:
about 3,400 redraw sessions in UI slots 2 and 8, and **none** had a pack, a
VRAM copy, a CPU readback or a second draw area. The fallback would almost
never be taken.

## 3. What it would save (CONFIRMED)

The bound, measured directly: a developer switch (`PSX_GL_DEV_SKIP=96`,
wrong pictures) drops the centre and band copies and the redraw depth clear
altogether, which is more than an output-sized target could remove (it still
needs one output-sized depth clear). Same binary, alternating, two render
threads, turning at 180 Hz, offscreen, the player's profile:

| View | Normal | Copies and depth clear skipped |
| --- | --- | --- |
| UI slot 2 | 22.7 / 25.2 / 24.6% | 23.0 / 25.2 / 25.3% |
| UI slot 8 | 21.7 / 20.6 / 20.4% | 21.2 / 20.6 / 21.5% |

No difference that rises above run-to-run noise (about one point). On the
timeline the removable part is about 0.05 ms a redraw at full clock, about
0.9% of the GPU at 180 Hz (INFERRED), which is consistent with not resolving it.

## 4. The design, for when it is worth building (D23M3B)

From reading the code (CONFIRMED in code, not built):

- **Centre pixels must rasterize exactly as now.** Drawing the centre with the
  wide surface's projection (`u_xoff`, `u_xhalf` of the wide width) changes
  the float mapping (a division by a width that is not a power of two), so
  edge pixels could differ from the VRAM surface's. The exact way is to keep
  the canonical projection (`u_xoff` 0, `u_xhalf` 512, a 1024 x 512 viewport
  at the scale) and move the viewport origin by whole pixels: `-(base_x -
  wide_off) * S, -band_y * S`. The same floats then land on the same samples,
  shifted.
- **Margins keep the mirror's mapping** (with the backdrop-stretch gates),
  scissored to the margins, as today; only their target changes.
- **One target per cache image**: the cache texture itself (wide width by
  band height at the scale) with one output-sized depth-stencil per render
  thread. The band copy and the centre copy disappear, and one depth clear
  replaces the VRAM area's and the wide rows'.
- **Copy-on-read for the output**: the frame's clearing fill does not reach
  tile row 7 (VRAM rows 224-255, note 457), so those output tiles must be
  loaded from the snapshot's colour (centre) and wide surface (margins) on
  first touch, as the VRAM surfaces' are.
- **Fills, wide clears and lines** map through the same viewport and scissor;
  uploads stay on the VRAM surfaces and the raw mirror (they are never in the
  band in these views; one in the band falls back).
- **Fallback**: a pack, a VRAM copy, a CPU readback, an upload into the band
  or a second draw area marks the session, and the render thread draws the
  job again on the VRAM path.
- **Proof**: `PSX_RT_VALIDATE` compares the render thread's output with the
  emulation thread's VRAM-path image, RGBA-exact.

## 5. Decision

Not built: the measured saving is below the noise of the whole-GPU figure,
the change reaches every draw path of the wide pipeline (D50 mirror deferral,
overlay rectangles, backdrop gates, copy-on-read, the upload skip), and the
GPU floor found in D23U (note 459 section 3: 14.4-19.2% whole GPU with no
textured scene drawn, against a bar of 7.2%) is far larger. The board now has
D23V for the floor, then D23M3B for this design.

Developer additions: `[m3 reads]` counters, `PSX_GL_DEV_SKIP` bits 32 (skip
the centre and band copies) and 64 (skip the redraw depth clear).
