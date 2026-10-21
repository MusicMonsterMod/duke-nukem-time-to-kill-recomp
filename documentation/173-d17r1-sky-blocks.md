# D17R1 - stair-stepped holes in the sky clouds

2026-10-21. **Done (user-accepted 2026-10-21: "lock it in and close it out").**
Executable `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f` is the regression baseline. User report (2026-10-21, D17R acceptance):
sky artifacting while looking up/down and turning in medieval UI slot 2 (with a
possible camera jerk) and LEVEL01 UI slot 8. The user's recording
(`research/screencaps/Video_2026-10-21_07-02-29.mp4`, 13 s, medieval canal,
first person) shows the clouds cut into screen-aligned, stair-stepped blocks:
where a block is missing, only the flat sky-colour fill and the backdrop
gradient show. Looking further up, the blocks fill in again.

## Finding

The clouds are not covered by anything: the polygons are not drawn. A GP0
frame dump at the reproduced view (slot 2 save, first person, 60 FPS, so no
redraws are involved) shows the pixels of a block are painted only by the sky
fill F4 and the backdrop G4. The cloud band's near quads are missing.

The cause is the D11B near-clip takeover's packet budget, not the sky code or
D17R:

- In the eye view, sky cloud quads have corners nearer than H, and the object
  renderer `0x80010000` drops a polygon whose corners are all nearer than H.
  The near clip (`near_clip.cpp`) draws such polygons itself (clipped and
  subdivided). That is what normally keeps the clouds whole in first person.
- Its host packets were written into the game's own render ring (context
  `0x800d67a8` +4..+8, 139744 bytes), capped at 60% of the ring per frame so
  the ring could not wrap onto the same frame's packets.
- With the **extended** draw distance (D17P, the player's setting) the game
  itself uses about 61% of the ring in the canal view (frame peak 85608 bytes;
  50812 with original distance). The cap was already spent before the sky,
  which the composition draws last. Every sky mesh then hit
  `mesh_budget_fallback` (about 60 a second) and went to the original path,
  which drops the near cloud quads: the stair-stepped holes.

| Canal view (slot 2 save, pitch up 15) | Fallbacks / 2 s | Ring peak | Host peak | Clouds |
| --- | --- | --- | --- | --- |
| Before, draw distance extended | 123 | 85608 | 4736 | blocks missing |
| Before, draw distance original | 0 | 50812 | 16160 | whole |
| After, extended, 60 and 120 FPS | 0 | 80944 | 15968 | whole |

The same budget applied to every mesh drawn late in a heavy frame, so this is
a game-wide first-person (and widescreen near clip) limit, not a sky-only one.

## Change

The near clip's host packets get their own arena (purpose-built, instead of
retuning the 60% cap):

- `near_clip.cpp`: a 512 KB arena in the runtime's GPU DMA aperture
  (`psx_mod_alloc_gpu_dma_memory`, `0x80f00000`, reachable by 24-bit OT
  links), four 128 KB regions used one frame each in turn, so a region is
  rewritten only three frames after the GPU walked it. The frame budget is
  now the region size alone; the game's ring, its cursor (context +0) and its
  packets are never touched. A frame that wrote nothing keeps its region.
  Replay workers receive the region index with the existing frame state.
- Framework `render_replay.c`: the high-refresh redraw workers walked the
  ordering table with their own loop, which also stopped at any link with
  bit 23 set (the PS1 end marker). Every aperture address has bit 23 set, so
  each redraw ended at the first arena packet. The live GPU walker
  (`dma_gpu_ll.c`) ends only at `0xFFFFFF`. The redraw walk now ends there,
  or at a bit-23 link that is not an allocated aperture address.
- Framework `gpu.c`: the host precise-vertex table keyed every address by
  `& 0x1FFFFC`, which would fold an aperture address onto main RAM. Aperture
  addresses now keep their own key (`hpv_key`). Both framework changes are
  exported in `recomp/patches/time-to-kill-accepted-source.patch`; the patch
  applies to the pinned framework and reproduces both files exactly.
- Tests: `ttk-near-test` checks that host packets land in the aperture and
  the game's cursor is unchanged, that a spent host budget still falls back
  whole, and that a nearly full game ring no longer forces the fallback. Its
  fixture keeps the aperture apart from RAM, and the packet digest covers it.
  `ttk-controls-test` links near_clip and got the allocator stub.

Vanilla, third person without the widescreen near clip and the original draw
distance take the same paths as before (the near clip does not run in
Vanilla). The D17R sky widening and D17O camera-relative sky are untouched.

## Regression found in the first playtest

The first build (`d632379...`) broke the picture while moving: the user loaded
slot 2 and reported "quite a lot of graphically breaking issues". Every
high-refresh redraw stopped at the first host packet (the near clouds, drawn
first), so in-between images showed only the sky fill over stale picture, with
no weapon or HUD. Still screenshots show the real frames, and the redraws were
not counted as failed, so the first round of checks (still sweeps) missed it.
A forced-turn redraw (`DNTTK_LATE_TEST_YAW=10`) and a worker-side walk trace
found the bit-23 end test above. Motion sequences (replay dumps while walking
and turning) are now part of the evidence.

## Evidence

Offscreen OpenGL 4x, 16:9, first person, the player's saved Modernized
settings (copied once; extended distance, 120 FPS, Corrected precision),
private cards under `recomp/analysis/d17r1-sky` (player saves only copied,
hashes verified: slot 2 = file 01 `756b0e15...`, slot 8 = file 07
`115c923c...`; the player's files unchanged).

- Repro: slot 2 canal view, pitch up 15: stair-stepped block before, whole
  clouds after at 60 and 120 FPS. Draw distance original was already whole.
- Sweeps at 120 FPS, 45-degree steps (90 for the last two), all with 0 mesh
  fallbacks, 0 packet skips, 0 failed redraws: slot 2 at pitch 0/-20/-40
  (host peak 50784 bytes), LEVEL01 slot 8 at 0/-25 (32280), D17R slot 9
  (file 08 `f88cd1a6...`, 22200), D17O slot 10 (file 09 `2307f0f6...`, 43440).
  Sky continuous to both edges in all sampled views.
- Redraws after the walker fix (executable `9b296ae1...`): a forced
  10-degree turned redraw on slot 2 has the clouds, world, weapon and HUD;
  24-present replay sequences while walking and turning on slot 2, LEVEL01
  slot 8 and D17O slot 10 have no blank or broken images and 0 failed
  redraws. The few one-image pops while turning match the accepted baseline
  executable on the same sequence (316 vs 306 pixels at 682x240), so they
  are the existing turn cadence. Sweep of slot 2 again: 0 fallbacks.
- Vanilla slot 2 loads and draws as before (no replays, no near clip).
- `ttk-near-test` (original and Corrected precision), `ttk-sky-test`,
  `ttk-aim-test`, `ttk-input-test` (Xvfb) pass. `ttk-controls-test` builds but
  stops at its missing level fixture, as recorded before this job.
- Player executable
  `9b296ae1b7d828c5d9ca5880257cd24e4443dd5919d9b2e5c1097dc98adece3f`.

## Limits

- The camera jerk the user mentioned was not reproduced or isolated here; the
  recording shows the blocks, not a separate jerk. If it remains after this
  fix it belongs with D17B.
- The sky's own geometry is unchanged: a view that shows the original's black
  overhead (city levels without a fill colour, note 172) stays as it was.
- Host packets past 128 KB in one frame still fall back to the original path;
  the heaviest sampled frame used about 50 KB.
