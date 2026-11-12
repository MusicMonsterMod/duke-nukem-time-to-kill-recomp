# D23U - An exact depth prepass for the opaque host scene, and why it is off

Status: built and verified exact (2026-11-12), Needs review; off by default
(developer and opt-in `PSX_GL_ZPREPASS=1`). Job board: D23U. Evidence labels:
CONFIRMED (our measurement or code), INFERRED.

## 1. The idea and the rule that keeps pictures identical

D23M2 ([note 457](457-d23m2-shared-snapshot-redraws.md) section 7) counted
2.8-2.9 samples passing per output pixel in a redraw and proposed drawing the
opaque host-depth geometry nearest first, so the GPU rejects hidden fragments
before the full textured and lit shader runs. Reordering the ordering table is
not exact in general (painter-order primitives interleave with host geometry,
and equal depths must resolve as before), so this job keeps submission order
and shades each pixel once instead:

A run of opaque host-depth triangles (depth test and write, no translucent
prim, no destination mask check) is drawn twice
(`gpu_gl_renderer.c`, `tex_run_draw`):

1. depth only, with a prepass program: the same vertex shader and a fragment
   shader holding only the decisions that remove a fragment (screen-door
   coverage, texel 0 in the nearest and bilinear paths). The depth buffer then
   holds exactly what the LEQUAL pass leaves: the minimum of the run's depths
   and what was there before;
2. colour, in submission order, depth test EQUAL, no depth writes, blending
   off (an opaque prim's dual-source factor is 0). A fragment shades only at
   the final depth; among equal depths the later primitive still draws last
   and wins; a fragment equal to depth from before the run still passes, as
   under LEQUAL. Alpha (the mask bit) and stencil come from the same winning
   fragments.

Translucent and depthless primitives, depth-test-only (water) and depth-only
(decal) runs and destination masking keep the ordering table exactly.

A developer variant (`PSX_GL_ZPRE_SEGMENT=1`) extends one prepass over a
whole segment of batches: painter-order and batch-key flushes are logged with
their vertices and state, and at any coherency point (another flush, a
framebuffer bind, a mirror drain) one prepass covers every eligible run of
the segment, then every batch is replayed in order through the unchanged
flush bodies. The argument in the code comment shows painter, depth-test-only
and flat host primitives may sit inside a segment; depth-only runs,
translucent runs that write depth, mask checks and anything that reads what
was drawn end it.

## 2. How the discards and STP passes limit early rejection here (CONFIRMED)

Measured on the GTX 1080 Ti with `GL_FRAGMENT_SHADER_INVOCATIONS` next to
`GL_SAMPLES_PASSED` over each redraw (new, `PSX_GPU_PHASES=2`):

- Today almost nothing is rejected early: 2.94 invocations against 2.78
  passing samples a pixel in UI slot 2, 3.09 against 2.92 in UI slot 8. The
  0.15-0.17 difference is texel-0 and coverage discards. Painter order passes
  nearly every fragment, so there is nothing to reject.
- In the EQUAL colour pass the GPU does reject hidden fragments before the
  shader, even though the textured shader can discard: the pass writes no
  depth, so the discard does not force late depth testing. In UI slot 8 the
  textured draws fell from 0.89 to 0.78 ms on the timeline with the
  invocation count rising only by the prepass's own fragments.
- The STP stencil fix-up pass runs only under a destination mask check,
  which no triangle in UI slots 2, 8 or 9 uses (0.0% in the stats below);
  translucent prims keep their two passes and are not prepassed. The STP
  passes therefore do not limit anything in these views.
- What does limit it is the prepass itself. Drawn with the full lit program
  and an early return it cost 0.13 ms (UI slot 8) and 0.77 ms (UI slot 2) a
  redraw: the program's register use sets its occupancy whatever the branch.
  The dedicated program cut that to 0.035 and 0.375 ms.

## 3. Where the samples really are (CONFIRMED)

Developer counts (`PSX_GL_ZPRE_STATS=1`) and a developer switch that skips one
class of runs (`PSX_GL_DEV_SKIP`, wrong pictures, cost attribution only), one
render thread, turning at 180 Hz:

| | UI slot 2 | UI slot 8 |
| --- | --- | --- |
| Textured triangles in opaque host-depth runs | 82-84% | 83% |
| Runs a redraw, triangles a run | about 143, 20 | about 7.4, 176 |
| Depth-test-only (water) triangles | 12.5-13.5% | 3.2% |
| Samples a pixel, all / without opaque host runs | 2.76 / 1.98 | 2.92 / - |
| Whole GPU, normal / no opaque host runs / no textured runs | 24.0 / 20.8 / 19.2% | 19.9 / 17.2 / 14.4% |
| GPU timeline "tex", normal / no opaque runs / none | 1.17 / 1.01 / 0.89 ms | 1.04 / 0.63 / 0.54 ms |

So the premise was too optimistic. In UI slot 2 the opaque host geometry is
only 0.78 of the 2.76 samples a pixel; water splits it into about 143 short
runs a redraw. Even removing every opaque host run outright saves 3.2 points
(UI slot 2) and 2.7 points (UI slot 8) of whole GPU, of which overdraw is
only a part. Most of the GPU's work remains with no textured scene drawn at
all: 14.4% and 19.2%. That floor is now its own job, D23V (section 7).

## 4. Pictures (CONFIRMED)

`PSX_REPLAY_VALIDATE_ZPRE=1` (new, with `PSX_RENDER_THREAD=0`): every redraw is
drawn with the prepass, then again without it, and the two images compared
RGBA-exact. Walking, turning and firing at 180 Hz:

| Build step | View | Redraws compared | Exact |
| --- | --- | --- | --- |
| per run, full program prepass | UI slot 2 | 2,880 | all |
| per run, prepass program | UI slot 2 | 2,304 | all |
| segments | UI slot 2 | 2,560 | all |
| segments | UI slot 2 / UI slot 8 | 2,624 / 1,856 | all / all but 1 |
| final (per run, 64-triangle rule) | UI slot 2 | 2,624 | all |
| final | UI slot 8 / UI slot 9 | 2,176 / 2,112 | all but 1 / all but 1 |
| final, segments | UI slot 8 | 2,112 | all but 1 |
| control: prepass off on both sides | UI slot 8 / UI slot 9 | 2,176 / 2,048 | all but 1 / all but 1 |

About 21,000 redraws compared. Every exception is the first redraw compared
at the moment the level's lamp bake is uploaded (`[gl bake] ... generation 1`
follows it): 4,990 pixels one level apart in UI slot 8, 90 pixels up to 24
levels apart in UI slot 9 (level 0, 90 lamps). The control runs, with the
prepass off for both draws, show the identical difference at the same redraw:
the two draws compared saw different bake generations, the artifact D23M2
recorded with its unmodified baseline (note 457 section 4).

Also: `PSX_RT_VALIDATE` (render thread against the emulation thread) 691 of
691 exact on the default build; Vanilla screenshots byte-identical to this
session's starting binary; `ttk-input-test` and `ttk-near-test` pass.
Modernized screenshots cannot serve as matched shots: two launches in the
same mode differ by 296,152 pixels (the presented redraws follow the wall
clock), as much as a launch with the prepass does, so the in-process
comparison above is the proof.

## 5. Cost (CONFIRMED)

Without a length rule both modes broke 180 Hz in UI slot 2 with the player's
two render threads: 84-100 presents a second in all six runs. The prepass
adds draws and state changes per run on the render thread (about 0.33 against
0.25 of a core per 100 presents), redraws missed their deadlines, redraw-only
switched off ("workers behind") and the emulation thread had to draw. UI slot
8 held 180 in every mode. A prepass is therefore now drawn only for runs of at
least 64 triangles (`PSX_GL_ZPRE_MIN`); shorter runs draw as before, equally
exact.

With that rule, same binary, `PSX_GL_ZPREPASS=0` against `1`, alternating,
turning at 180 Hz, offscreen, the player's profile:

| Measure | UI slot 2 | UI slot 8 |
| --- | --- | --- |
| Whole GPU, two render threads (3 pairs) | 22.7 / 20.0 / 23.1 -> 22.8 / 20.2 / 22.2% | 18.5 / 19.3 / 20.1 -> 17.9 / 17.7 / 18.7% |
| Whole GPU, one render thread (6 pairs in all) | 22.7 / 22.1 -> 24.0 / 23.7% | 19.5 / 18.1 / 19.0 / 19.2 / 20.8 / 21.1 -> 20.9 / 18.4 / 17.0 / 16.9 / 17.4 / 17.0% |
| GPU a redraw, one thread, at 1,974 MHz | 1.68 / 1.66 -> 1.85 / 1.82 ms | 0.95 / 0.89 / 0.92 / 0.93 -> 0.91 / 0.91 / 0.82 / 0.82 ms |
| Render thread CPU | 41-43% -> 40-42% | 30-32% -> 30-31% |
| Presents a second | 180 (159 in one pair, both modes) | 180 |

UI slot 8 is about 1.5 points of whole GPU lower (9 pairs, from +1.4 to -4.1)
and 0.06 ms a redraw (6%) lower; UI slot 2 is equal with two render threads
and up to 0.17 ms a redraw higher with one. Against D23M2's figures (UI slot
2 22.0%, UI slot 8 19.3-19.4% whole GPU) the baselines here agree.

Two cautions on method found on the way (CONFIRMED):

- The GPU timeline in milliseconds depends on the clock the GPU chose; a
  lighter load clocks down and lengthens every interval, so raw timeline
  means can move against the whole-GPU figure. The redraw figures above are
  scaled to the full clock by each run's mean clock (an approximation).
- Timestamps serialize the GPU: with per-kind marks (`PSX_GPU_PHASES=2`) the
  prepass's extra marks inflated its own timeline; level 1 marks only the
  redraw.

## 6. Decision and Deck-class estimate

Exact, but not a reliable saving: worth a few percent where opaque runs are
long and overdrawn (UI slot 8), nothing or a little worse where water breaks
the scene into short runs (UI slot 2, the heaviest view of D23K). It is left
off by default; `PSX_GL_ZPREPASS=1` turns it on (Vanilla never draws host
depth, so it is unchanged either way), and the segment variant stays a
developer option.

Deck-class estimate (note 444's factor of 7 on this GPU's time, INFERRED): UI
slot 8 about 0.4 ms a redraw less (6.5 -> 6.1 ms); UI slot 2 no gain. Neither
reaches the D23K bar of 4 ms a redraw on such a GPU; the floor of section 3 is
the larger part of what stands in the way.

## 7. Next

- D23V (new): the GPU floor. With no textured scene drawn, the whole GPU at
  180 Hz is still 14.4% (UI slot 8) and 19.2% (UI slot 2), about 0.8-1.05 ms
  a presented frame against the 0.4 ms bar; the GPU timeline still shows
  0.54-0.89 ms of "tex" per redraw with nothing drawn. Find what it is
  (per-batch setup and vertex-ring copies, presents, captures and loads,
  lighting and shadow setup, the emulation thread's own frames) with the
  same skip-and-measure method.
- Developer switches added: `PSX_GL_ZPREPASS`, `PSX_GL_ZPRE_MIN`,
  `PSX_GL_ZPRE_SEGMENT`, `PSX_GL_ZPRE_STATS`, `PSX_GL_DEV_SKIP`,
  `PSX_REPLAY_VALIDATE_ZPRE`; `[gpu overdraw]` now reports fragment shader
  invocations; the GPU kinds gain `zpre`.
