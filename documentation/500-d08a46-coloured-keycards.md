# D08A46 - Duke 3D-style coloured key cards

Status: **Done**, user-accepted 2026-11-16: "its solid! lock it in, close it
out". Modernized with the
Polymorph world renderer; Classic and Vanilla keep every card blue.

## Request

User, 2026-11-14 playthrough: levels with several key cards should use Duke
3D's colours (blue, red, yellow). Work: audit the key cards per level, decide a
colour for each, update the pickup, the inventory icon and the in-world
representations; door logic unchanged. Acceptance: different cards visibly
distinct, every level still completes.

## Audit

Every key-slot key in TTK is the same object: types 153-157 all use model 420,
one flat textured quad (FT4) whose 32 x 24 4-bit texture is a blue
radiation-symbol card drawn with a 16-colour blue ramp (CLUT 0x23ac at VRAM
704,142; page 0x62b, UV 72-103 x 0-23 in level 0). The pickup dispatcher sets
mission item 6 for type 153, 7 for 154, 8 for 155 (note 114). The game's own
mission names (note 114) give the per-level cards:

| Level | Item 6 (type 153) | Item 7 (type 154) |
| --- | --- | --- |
| 0 | SUBWAY SECURITY KEY | TRANSPORT ROOM ID |
| 5 | WAREHOUSE KEY | - |
| 7 | GANTRY KEY | VALVE KEY |
| 9 | LAB KEY | VALVE ROOM KEY |

Levels 0, 7 and 9 have two cards; no level names an item-8 card. Skeleton keys
(medieval, western) are different objects and are not touched.

## Design

The colour follows the mission slot, a game-wide rule with no per-level data
(custom levels keep working): **type 153 / item 6 blue** (the retail card,
unchanged), **154 / item 7 red**, **155 / item 8 yellow**. Types 156 and 157
(items 9 and 10, unused by the campaign) stay blue. So every two-card level
shows a blue and a red card, as Duke 3D's first levels do.

The card's art stays the game's own. Because its texture is a single-hue
16-entry palette, a red and a yellow palette of the same brightness index for
index recolour it completely:

- red (B, 0.55 R, 0.45 R), yellow (B, 0.78 B + 0.12 G, 0.5 R) of each blue
  entry (R, G, B), 15-bit with the mask bit set as the original's, so index 15
  stays opaque black. Compiled into `keycard_colours.inc` (our numbers, no
  retail data).

The palettes sit in the second texture bank (D17AO1), at x 256-287, y 480,
painted once at boot beside the TV atlas and prop lights. A red or yellow
card's polygons keep their retail texels in VRAM and read only the palette from
the bank. Nothing in guest VRAM or RAM is written; redraw workers record the
mode word like any other host vertex.

The mission inventory uses a red card icon for the item-7 cards
(`item-keycard-red.png`, a hue-shifted copy of the blue card icon, the user's
own drawing). With the colours off (Classic, or developer
`DNTTK_KEYCARD_COLOURS=original`) every card and icon is blue.

## Implementation

Framework (`recomp/psxrecomp`, exported into the tracked patch):

- `PSX_MOD_HOST_CLUT_BANK` (16) for `psx_mod_gpu_host_vertex_mode`
  (`mod_plugins.h`); the entry point's mode mask widened from 4 to 5 bits.
  `gpu.c` turns a triangle whose three vertices carry it into
  `gl_set_host_clut_bank()`.
- `gpu_gl_renderer.c`: such a 4- or 8-bit triangle (never one already in the
  bank) gets `v_depth + 8`; the pack check uses the texel page only. In both
  textured fragment programs (`TEX_FS` and the depth pre-pass) texel reads take
  the bank on `v_depth & 4` and palette reads (`clut_at`) on `v_depth >= 4`,
  the same single test each as before.
- `render_copy/rc*_names.h` regenerated for `gl_set_host_clut_bank`.

Title (`recomp/src/ttk`):

- `keycard_colours.inc` (in `modern_controls.cpp`): the setting gate, the
  palettes and `keycard_paint_bank`, the card texture read from the type-153
  model each time (CLUT, page, UV rectangle), and `keycard_frame`: the colour
  of the object the object loop is drawing (D08A44's `nv_object`: type 154
  red, 155 yellow) or, for Duke's frames, of the held card: equipment
  `player+0x3b8` 1 or upper pose `+0x74` 266, item `+0x3b9` 7 or 8 (measured at
  a level-0 reader: the card comes out at upper 47 with equipment 1 and item 6;
  equipment drops to 0 while pose 266 pushes it into the reader).
- `near_clip.cpp`: a textured face of that frame with the card's CLUT, page
  (position and depth bits; the context adds blend bits 0x60) and UVs inside
  the card's rectangle takes the bank palette and `Poly::clut_bank`, emitted
  with the mode bit.
- `sky_borrow.cpp`: the bank painter calls `keycard_paint_bank`.
- `inventory_hud.cpp`: `mission_keycard_colours()` (fed every gameplay update
  from `shortcuts.inc`); off, the item-7 card names show the blue icon.
- `assets/ui/items/items.json`: TRANSPORT ROOM ID, VALVE KEY and VALVE ROOM KEY
  use `item-keycard-red.png`.

## Evidence

Lab in the job's scratch directory (private card copies, Xvfb or SDL
offscreen, debug port; every run asserts the player's saves and profile
unchanged). Worktree build of `5b01798` plus this job.

| Check | Result |
| --- | --- |
| Level 0 (UI slot 4), `spawn 153`, `154`, `155` | blue, red and yellow cards in the world, full texture detail |
| `DNTTK_KEYCARD_COLOURS=original` | all three blue |
| Walk into a spawned 154 | TRANSPORT ROOM ID message, item 7 flag set; mission inventory and item card show the red icon (blue with `original`) |
| Archived level-0 reader route (D36 card case, item 6) | blue card in hand at upper 47, insertion 266, reader turns green |
| Same route, held item forced to 7 (test only) | red card in hand, first and third person (the reader refuses item 7, as expected) |
| Vanilla | loads and draws as before |
| GPU at 180 Hz (UI slot 4, first person, turning, CUSTOM, two runs each, clock-weighted `nvidia-smi`) | player build 11.1 / 10.7 %, this build 10.3 / 9.9 %: no measurable cost |
| `PSX_GL_PROGRAM_DUMP` | four textured programs 15 instructions larger each (the palette read's own test in every inlined fetch), no `lmem` |

Suites: `ttk-near-test` (new D08A46 group: a card's faces carry the clut-bank
mode and the bank palette, a face with another CLUT or any face of a non-card
object never), `ttk-inventory-test`, `ttk-input-test`, `ttk-flash-test`,
Python 260 OK (2 skipped), the three CI runtime tests, the patch on a clean
pinned `08ec704a` (1038 files match). Archived savestates still load (no
codegen hash change).

Before/after: `research/before-after/D08A46/` (local): world, mission
inventory, held card.

## Limits

- Polymorph only: Classic draws the original packets (blue cards, blue icons).
  A card face left to the original renderer (a mesh downgraded by the frame
  budget, D17AM2) stays blue for that frame.
- No real item-7 reader was used: the red held card was checked by forcing the
  held item in a private test. Every level's completion with the new colours
  (the acceptance) is not established by this pass; door logic is untouched
  (no game state is written).
- Yellow (type 155) is only reachable by spawning or in custom levels; no
  yellow icon exists, since no level names an item-8 card.
- Door readers keep TTK's red-to-green look; they are not colour-coded.

## Player check

Levels 0, 7 and 9 have a blue and a red card. Try it with `spawn key 1` and
`spawn key 2` in the console in level 0, then `.` for the mission inventory.
From the workspace root:

```sh
python3 recomp/tools/local/run.py
```
