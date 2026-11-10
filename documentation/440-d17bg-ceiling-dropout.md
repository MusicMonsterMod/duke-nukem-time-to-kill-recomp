# D17BG - Sandstone room ceiling face drops out to black

2026-11-10. Status: Done (user-accepted 2026-11-10: not a player fault;
software OpenGL notice added, section "Notice"). Polymorph, Modernized. Found during D41F (note 439, section 4): in the sandstone room
(UI slot 8) a ceiling face showed black in some frames, with shadows and
dynamic lights on or off.

## Result

**The player's GPU never shows it.** The dropout happens only in the
offscreen test runs (Xvfb, Mesa llvmpipe software OpenGL). On the real
display (NVIDIA 580, OpenGL 3.3) the same save and view showed the face in
every sample: 16 VRAM samples of both display buffers, 14 presented frames
at 30 Hz and 21 at 180 Hz.

## What the offscreen runs showed

Harness: `recomp/analysis/d17bg/lab.py` (local; private card copy, player
files hash-checked unchanged after every run).

- The face is the ceiling quad `0x80185008` of mesh `0x80184d10`, cut by the
  near clip and drawn as four smooth-shading triangles (D17AV). Near-clip
  trace: the same emits in good and black frames. GPU command stream: the
  same packets reach the GPU in every frame.
- The black frames are exactly those drawn into one of the two display
  buffers (VRAM y 0). There the face's triangles reach above and left of
  the framebuffer, so OpenGL has to clip them; in the other buffer (y 240)
  they only meet the scissor. Same triangle, colours, texture page, CLUT,
  UVs, depth and perspective values in both (a temporary trace in the GL
  renderer, since removed).
- Not the cause: dynamic lights, D41F shadows, host depth
  (`PSX_HOST_DEPTH=0`), host UVs, smooth quads, coplanar ranking,
  widescreen (4:3 also showed it), the 16-bit texture cache, the texture
  and CLUT contents (the sampled VRAM copy is identical in both phases),
  depth clears.
- Drawn as untextured red the face appears in both buffers. Drawn textured
  without perspective-correct UVs it appears in both. So llvmpipe returns
  wrong (transparent) texels for a perspective-textured triangle that it
  clips against the framebuffer edge.

## Consequence

- Players on a GPU driver: no change needed as far as tested (NVIDIA).
  Other hardware drivers (AMD, Intel) were not tried.
- Offscreen evidence: a face that drops to black at the top or left edge in
  alternate frames of an Xvfb run is a test-renderer artefact; confirm on
  the real display before treating it as a game fault.

## Notice (closeout)

User choice: no renderer workaround; tell the player instead.

- `gpu_gl_renderer.c` (framework patch): when `GL_RENDERER` names a
  software rasterizer (llvmpipe, softpipe, Software Rasterizer, SWR, GDI
  Generic), the launch log prints `software OpenGL detected (...)`, and
  `gl_renderer_is_software()` reports it (main renderer only; the render
  thread copies keep the flag private).
- `game_menu.cpp`: the first time the main menu opens in a launch, a
  centred notice for 8 s: software OpenGL, slow, some surfaces may draw
  wrongly, install the graphics driver. Vanilla gets the log line only.
- Checked: notice under llvmpipe (Xvfb); none and no log line on the real
  NVIDIA display; Vanilla launches. `ttk-near-test`, `ttk-flash-test`,
  `ttk-live-env-test` pass. Patch exported and applied to a clean
  checkout of the pinned framework (`08ec704a`).

## Possible follow-ups (not done)

- Make the offscreen harness match the player: run the test window on a
  GPU (headless EGL or a hidden real-display window) instead of llvmpipe.
- Or work around llvmpipe in the renderer (for example, clip such triangles
  to the framebuffer before submission). Only worth it if players without
  a GPU driver matter.
