# 492 - D23Y5: OSD images uploaded only when they change; D23Y8 step 1 not built (field laptop)

Autonomous overnight session of 2026-11-16 (laptop lane). User: "If time
remains: D23Y8 step 1 behind a developer switch (its own small GL path, OSD
as data, present_shot moved, other present paths drained), offscreen checks
only, real-display check queued for me."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Where the present's emulation-thread time goes (MEASURED, note 485)

At the player's 120, `gl_swap_with_osd` was 3.8% of the emulation thread,
`gl_draw_osd_image` 2.1% of it. D23Y8 step 1 would move that work to a
presentation thread; this session first looked at why drawing the OSD costs
that much.

## 2. Why (CODE)

Every present composes several host OSD images into the window: the
crosshair (9 x 9, every present in first person), the level stats, the
inventory strip and the message text, besides the camera frame and menu
images when they are up. All of them were uploaded into one shared texture
(`glTexSubImage2D`) and scanned for alpha (`host_osd_text_has_alpha`) at
every present, 120 times a second, though they rarely change; each upload
also overwrote the previous image's texture, so nothing could be kept.

## 3. The change (CODE; on by default, developer `PSX_OSD_UPLOAD_CACHE=0` off)

Each OSD image source (its buffer, size) keeps its own texture and a copy of
the pixels the texture holds; an image equal to that copy, compared byte for
byte, is drawn from its texture without an upload or a scan. Exact by
construction: the texture holds the same pixels, drawn with the same state.
Images over 256K pixels keep the shared texture as before. The textures and
copies are dropped with the GL context (renderer reset). Developer
`PSX_OSD_UPLOAD_CACHE=2` reads the texture back at every hit and compares it
with the image. `gpu_gl_renderer.c` (`gl_draw_osd_image`), exported into the
tracked patch.

## 4. Proof (MEASURED)

- The check (`=2`): field laptop, UI 2 at 120, 30 s: 6,144 hits read back,
  0 differing, 1,040 uploads; desktop, UI 2 and 8 at 180, 40 s: 6,144 hits,
  0 differing, 1,966 uploads.
- Vanilla (`--vanilla`, case `vshots`, UI 8 and UI 2, +60/+200/+400): the
  lane's build against the committed `5226b66` build: byte-identical, 6 of 6.
- Whole-frame matched shots of the Modernized profile cannot judge it: two
  runs with the same build already differ (note 488 section 4).

## 5. Cost (MEASURED)

Field laptop, the player's settings (FRAME RATE 120), UI 2, case `cad` 40 s,
emulation-thread profiles (`osdq.sh`, `incl.py`), interleaved, two runs a
configuration:

| | upload every time (`=0`) | cached (default) |
| --- | --- | --- |
| `gl_swap_with_osd` | 3.4%, 3.5% | 1.9%, 1.8% |
| of it `gl_draw_osd_image` | 1.8%, 1.9% | 0.6%, 0.6% |
| `gl_draw_osd_bound` | 0.6%, 0.7% | 0.4%, 0.3% |

About 1.6 points of the emulation thread at 120 (of D23Y5's 62%), the
larger part of what D23Y8 step 1 would have moved to another thread (about
4 points, note 485), without a thread.

Host: idle before the runs (load under 1.3 after the build, no game,
update or backup).

## 6. D23Y8 step 1: not built (CODE, INFERRED)

What remains movable of the present after this cut is about 2 points at
120 (the picture quad, the OSD draws themselves, the tint and the swap). A
presentation thread needs its own GL path on a shared context (the
renderer's state cache and VAOs belong to the emulation thread's context),
the eight present paths (`gl_swap_with_osd` callers: VRAM presentation,
held frames, the wide surface, the cached redraws) drained into it, the OSD
callbacks turned into data and `present_shot` moved with it; its timing and
pacing can only be judged on the real display, which this session could not
use. With 2 points left to win it was not started; the real-display
measurement it would need is queued for the user.

## 7. Status

D23Y5 stays Blocked on its target (under 50% at 3.1 GHz at the original 60;
62% at 120 less these 1.6 points); this cut is on by default and Needs the
user's look (nothing in the picture should change). D23Y8 Blocked
(measured), unchanged.
