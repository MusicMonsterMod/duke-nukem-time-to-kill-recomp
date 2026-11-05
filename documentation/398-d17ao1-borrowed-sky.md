# D17AO1 - A real sky through painted skylights (Duke Hill's sky)

2026-11-05. **Done, user-accepted** at revision 2: "lock it in and close it
out. its beautiful". Revision 1 report: User report, UI slot 11 (savestate 10): the
little patch of "sky" above "looks so unbelievably fake" at high FOV and
resolution; "at minimum just paralax this, or at maximum, add a real skybox in
(the one from duke hill)". The user also asked how EDuke32 parallaxes its skies
("the effect is really spectacular in eduke"), then chose the real Duke Hill
sky. Private runs only; no player save, card or setting was a test destination
(every run checks the player files' hashes before and after).

## What the patch was

The first look on the board read an older copy of the save. The player's
current UI slot 11 (saved 2026-11-05 22:13) is GOLD AND GUNS (level 3), a mine,
and that level loads no sky: the sky routine's four layer selectors and its fill
colour (`0x800da344`, `0x800da45c`, `0x800da464`, `0x800da468`, `0x800da46c`,
note 92) are all zero. The patch is one world quad at the top of a vertical
shaft, texture page `0xe`, CLUT `0x1438`, UV 64..86 x 232..255: a 23x24-texel
blue-and-cloud crop inside a level 3 texture atlas (`DB03A.RAW`, VRAM
(912,232)). The atlas holds unrelated tiles beside it (an arrow sign, numbered
tiles), so sliding its UVs cannot parallax it. No other level's VRAM images
contain that picture.

## How EDuke32 does it

A parallaxed ceiling is not drawn as a surface. The engine wraps a ring of sky
tiles (8 by default; DEF `multipsky`, `lognumtiles`) around 360 degrees and
shows it wherever that ceiling is: view yaw scrolls it, the horizon shifts it,
walking never moves it, so it reads as infinitely far away. Time to Kill's own
sky is the same kind of thing: eye-centred cloud rings whose matrices translate
to the eye (note 92). Sources: EDuke32 wiki "Multipsky (DEF)" and "Editing
Parallaxed Skies".

## Duke Hill's sky

DUKE HILL (level 1) selects all four layers: backdrop `0x611` (8 G4, vertex
coloured), fixed horizon ring `0x612` (36 FT4, the mountains, Y -83..0), slow
and fast cloud bands `0x613`/`0x614` (72 FT4 each: 36 columns by 3 rows at
about 10, 20 and 31 degrees of elevation) and fill colour `0x661f29`. Their
4-bit pages are at VRAM (704,0) and (704,256) with CLUTs near y 386, all inside
`DB01A.RAW`/`DB01B.RAW`, the level's raw VRAM images (uploaded to (512,0) and
(512,256), note 184). The cloud strip is a 256-texel panorama (page `0x3b`,
v 0..63).

`DB01MOD.BIN` loads unchanged at the model table address (`0x800ddb18`): 2048
32-bit offsets counted from the end of the table (`0xffffffff` none), then the
meshes, whose `+8`, `+0x10` and `+0x14` fields are offsets from the mesh. At
load `+0` takes the id and `+0xc` is cleared. The four meshes rebuilt this way
match a level 1 RAM dump byte for byte.

## Design

Modernized only. In a level with a painted sky and no sky of its own, while a
painted face is in view:

1. **The face is left out.** `near_clip.cpp`'s world path skips a face whose
   resolved texture matches `painted_skies` in `sky_borrow.h` (level, CLUT,
   page, UV rectangle). That table holds GOLD AND GUNS only. The world is drawn
   (room walk `0x80062b48`) before the props pass, so the same composition knows.
2. **The sky is requested.** `sky_widescreen.cpp`'s D17R2 `sky_background`
   sets the render's sky request although the level's own selectors are zero.
3. **The native sky routine draws Duke Hill's sky.** At `0x8001fc44(0)` with
   return `0x800265c4`, the call just before the routine `0x800388e4`, the
   five level words take Duke Hill's values, and the model table pointer points
   `4*0x611` bytes before a guest copy of Duke Hill's meshes, so the table
   entries for `0x611..0x614` are those meshes. At the next call `0x8002e850`
   (return `0x800265d4`) all six words are restored. A save or any other code
   never sees them (60 RAM samples while running: 0 injected). The routine,
   its timer `0x800c0c64` (read and written only by that routine), the D17O
   camera-relative matrices, D17R widescreen and D17AH painter rules are
   unchanged.
4. **A second texture bank.** Every polygon of the borrowed meshes is
   host-drawn (`near_clip`, `f.borrowed` forces the host path) with its
   vertices sent through the new `psx_mod_gpu_host_vertex_foreign`, which sets
   bit 4 of the host vertex's mode. A triangle whose three matched host
   vertices carry the bit samples the title's second bank
   (`gl_set_host_bank`): a 1024x512 image of Duke Hill's `A.RAW`/`B.RAW`,
   read by the GL shader (`u_foreign`, v_depth +4) with the same 4/8/15-bit
   CLUT rules. Retail packets cannot carry the mark. Revision 1 marked the
   copy's records with CLUT bit 15 (ignored by the PS1 GPU) instead, but 175
   of level 3's own textured model records already set that bit, so while the
   borrowed sky was wanted they sampled Duke Hill's sheet: the user saw "a
   texture map" where distant hills usually are, looking right again whenever
   the game was paused (no composition, so the sky was no longer wanted).
5. **A cloud deck overhead (the project's own geometry).** Duke Hill's bands
   end 31 degrees above the horizon, so a skylight overhead first showed only
   the flat fill colour. A first attempt added rows up to 88 degrees repeating
   each column's tile, which converged into a radial vortex, and was rejected.
   The pack builder now closes each band's top ring (36 vertices, y -250,
   radius about 410) with a flat deck in its plane: an 8x8 grid cut to the ring
   and fanned into FT3 faces, each cell one panorama tile (page `0x3b`, UV
   0..63), mirrored in alternate cells so the tiling is seamless. Each deck
   turns with its band, so the slow and fast decks drift against each other,
   and the slow band's Y scale applies to its deck automatically.

The guest copy (about 11 KB) is allocated once in the live process; replay
workers are re-forked when mod memory grows and never allocate it themselves.
It is rewritten at every use, because a savestate load restores mod memory to
the saved image. `near_clip` accepts that range for the sky routine's draws
(`sky_borrow_owns`); without that, third person showed only the fill colour,
because the native path dropped the deck's polygons nearer than H.

## Files

- `recomp/tools/local/build_ttk_sky.py`: the build step (CMake, with a
  prepared disc), `build-local/ttk-sky.pack` (TTKSKY1: meshes with relative
  offsets, then the two VRAM images). Retail-derived build output, never
  committed. Without a pack nothing changes.
- `recomp/src/ttk/sky_borrow.h` / `sky_borrow.cpp`: the table, the state shared
  with `near_clip`, pack loading, injection and restore, and the bank for the
  runtime (`ttk_foreign_vram`, `ttk_foreign_vram_ready`).
- `near_clip.cpp`: painted-face skip, borrowed mesh range; the primitive trace's
  `emit` records now also carry `clut`, `tpage` and `uv` (diagnostic).
- `sky_widescreen.cpp`: the sky request.
- Framework (in the accepted-source patch): `gpu.c` (host vertex mode bit 4),
  `gpu_gl_renderer.c` (the bank), `mod_plugins.h`, the regenerated
  render-thread rename headers (`render_copy/rc_names.h`, `rc2_names.h`).

Developer: `DNTTK_SKY_BORROW=0` turns it off; `DNTTK_SKY_PACK=path` picks
another pack. Not a saved setting.

## Verification

Private harness `recomp/analysis/d17ao1/` (`probe.py`, `meshes.py`), card copy
taken 2026-11-05 22:20, UI slot 11.

| Check | Result |
| --- | --- |
| First person, 60 | Face gone; Duke Hill's clouds over the purple fill through the opening, no vortex (cloud texel std 13.7). |
| Third person, 60 and 120 | Same after the `sky_borrow_owns` fix (std 13.0); before it, flat fill only. |
| First person, 120 and 180 | Clouds present; a 180 replay dump's redraw frame shows the sky like the real frame. |
| Injected words | 0 of 60 samples while running showed the swapped words or table pointer. |
| Vanilla | Original camera behind Duke; no Modernized path runs. |
| Level 1 and level 0 skies | Console travel on the same build: level 1 within 3.2% of the pre-change capture (enemy motion); level 0 differs only by a damage tint (Duke was hit on arrival). |
| 180 FPS cost under the skylight | On: 1440 distinct images in 8 s, 59.94 fields/s, 0 misses or failures. Off (`DNTTK_SKY_BORROW=0`): 1442, 59.99. |
| Native suites | `ttk-controls-test` (d08-camera-final fixture, LEVEL01, all levels) 153 PASS; `ttk-sky-test` ok; `ttk-aim-test`, `ttk-input-test` pass. `ttk-near-test` does not link since `3b7d1e5` (D08A35 `cowboy_hat_mesh` symbols), before this job; not run. |
| Framework patch | Exported; applied to a pristine pinned checkout, all 78 files match the working tree. |
| Revision 2 (host-vertex mark) | Clouds as before: first person std 13.7, third person 13.0, 180 real and redraw frames 16.6 / 15.9. Third-person routes with the borrowed sky on and off differ only in the opening and in turn-timing offsets; no stray textures. Controls 153 PASS, sky suite ok. |

Revision 2 executable
`95867765e3c80af34643cefbeeeab565b66dfa15765f39710fc5dd522bb28718`.

## Limits

- Offscreen captures. The user's look at their own display remains.
- Duke Hill's clouds are soft and low in contrast, as authored. Deck cell size,
  mirror tiling and any brightness are presentation choices open to tuning.
- The deck meets the band at 31 degrees with a texture change, visible only
  when the opening shows that elevation.
- Only GOLD AND GUNS's painted sky is in the table; other levels were not
  surveyed for different painted skies.
- OpenGL only: the software renderer draws the borrowed layers from the
  level's own VRAM. The user's exact hills view was not reproduced on the
  private run; the retest is theirs.
- If a frame cannot afford a world mesh and draws it natively, its painted face
  is drawn as before.
