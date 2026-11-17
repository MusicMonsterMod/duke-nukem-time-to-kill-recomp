# D08A45 - FAMILY JEWELS in blue, green and red

Status: **Done**, user-accepted 2026-11-17 ("lock it in and close it out"),
after a reopening the same day for the jewels in the wheel and in Duke's hand
during the hand-in (both fixed, below). Modernized with the
Polymorph world renderer; Classic and Vanilla keep every jewel pink.

## Request

User, 2026-11-14 playthrough: the three jewels in FAMILY JEWELS (level 6) are
all pink. Give each a clearly different colour, same shape and style,
consistent for the pickup, its inventory icon and wherever it is placed.
Gameplay unchanged. Acceptance: the three jewels distinct in the world and in
the inventory.

Colours chosen by the user, 2026-11-17: the energy crystals' colours, "blue,
green, red".

## Audit

Measured live in level 6 (debug port, spawned jewels):

| Type | Mission item | Model | Colour |
| --- | --- | --- | --- |
| 854 | 14 (`player+0x38c`) | 1595 | blue |
| 855 | 15 (`player+0x390`) | 1596 | green |
| 856 | 16 (`player+0x394`) | 1597 | red |

The three models are the same jewel: 5 FT4 and 8 FT3 textured faces, 4-bit,
four palettes (CLUTs 0x26f9, 0x2739, 0x27fa, 0xeb9 in level 6, in first-use
order). Every palette is a single-hue pink ramp (hue 322-348, 6-12 used
entries, the rest opaque black). The mission row names all three "FAMILY
JEWEL" (the game's own select-inventory name, kept).

## Design

As D08A46's key cards ([note 500](500-d08a46-coloured-keycards.md)): the
jewel's texels stay the game's own, and only the palettes are swapped. Each
pink entry gets the crystal icon's hue (blue 216, green 127, red 2; red nudged
off 5 so it never leans pink), its saturation lifted (`s * 1.25 + 0.18`,
capped) so the pale facets do not read pink again, and its brightness kept
(green x0.88, so the three weigh the same). 4 palettes x 3 colours = 12 ramps
at x 256-447, y 481 of the second texture bank, painted at boot beside the key
card palettes (y 480).

The test is the mesh being drawn, not the object type: a mesh that is the
model of type 854, 855 or 856 takes that type's colour. So a jewel shown by any
object (pickup, placed, dropped) is coloured, with no per-level data. Its
palettes are found from the mesh's own faces in first-use order; a model with
another palette count or an 8/15-bit page is left pink.

Mission inventory: three recoloured copies of the jewel icon (same transform),
chosen by item (14 blue, 15 green, 16 red) while the colours are on.

## Placed jewels (the wheel)

User, 2026-11-17: "slot 7 - when you put the jewels in the nuke symbol wheel,
they show as their original pink models". The wheel does not draw the jewel
models: Use at the wheel sets the next jewel's used bit (`0x4000`, items in
order 14, 15, 16) and the wheel switches to its own model with 1, 2 or 3
jewels set in (1632, 1633, 1634 in level 6: 47 vertices, 51 faces; 4, 8 and 12
jewel faces). The sockets are top (vertices 33-39), right (26-32) and left
(40-46), filled in that order, the same vertices in every model; their faces
use three of the jewel's palettes.

Rule (no model numbers): a mesh that is not a jewel model but has 4-bit faces
on the jewel model's palettes is a holder. Its jewel faces are grouped into
sockets by shared vertices (at most three). Fill order comes from the model
before it: when that is a holder with one socket fewer on the same vertices,
its order comes first and the new socket last (recursively); otherwise lowest
vertex first. The k-th socket in fill order shows the k-th used jewel in item
order, and sockets past the used ones the colours not yet shown. Each mesh's
result is cached per thread until the level, a savestate load or its polygon
list changes; levels without jewel models skip all of it.

## The hand-in

User, 2026-11-17: "the animation when duke is handing the crystals into the
device, shows the blue crystal 3 times". The game draws the first jewel's
model (type 854) in Duke's hand for every jewel. Measured at the wheel: Use
sets the jewel's used bit, then equipment `player+0x3b8` is 1 with the upper
pose 47/48 while he holds it, then pose 266 pushes it in; `+0x3b9` holds the
item (14, 15, 16) throughout. A jewel model drawn as part of Duke in that
state (equipment 1 or pose 266) takes the held item's colour, the same rule as
the held key card.

## Implementation (title only; no framework change)

- `recomp/src/ttk/jewel_colours.inc` (in `modern_controls.cpp`): setting gate
  (`input_modernized`, Polymorph; developer `DNTTK_JEWEL_COLOURS=original`),
  the 12 ramps, `jewel_paint_bank`, `jewel_frame(mesh)`.
- `near_clip.cpp`: a jewel mesh's textured 4-bit faces with one of its four
  CLUTs take the bank palette with `PSX_MOD_HOST_CLUT_BANK` (D08A46's path).
- `sky_borrow.cpp`: the bank painter calls `jewel_paint_bank`.
- `inventory_hud.cpp` / `shortcuts.inc`: `mission_jewel_colours()`; `art_for`
  takes the item.
- `assets/ui/items/`: `item-family-jewel-blue.png`, `-green.png`, `-red.png`
  and their `items.json` entries (`colour_of: FAMILY JEWEL`).
- Tests: `ttk-near-test` D08A45 group (a jewel mesh's faces take their own
  palette's replacement; a face with another CLUT, or a non-jewel mesh,
  never); `ttk-inventory-test` (found item 15 shows green, no pink; off shows
  the pink drawing); `test_ui_art` names.

## Evidence

Private offscreen runs on the NVIDIA GPU, private card copy, debug port,
player files hash-checked (`recomp/analysis/d08a45/probe.py`, local):

| Check | Result |
| --- | --- |
| Level 6, spawn `family jewel`, `2`, `3` | blue, green and red in the world, third and first person |
| `DNTTK_JEWEL_COLOURS=original` | all pink, as before |
| Walk into a spawned 855 | item 15 set (14 and 16 untouched), as the original pickup |
| Mission row (unit test dump) | the found second jewel shows the green icon |
| Vanilla | loads and draws as before |
| UI slot 7 at the wheel, E three times | blue, then green, then red set in; colours kept as the wheel turns; `original` all pink |
| Same, shots while Duke holds each jewel | blue, then green, then red in his hand |

Suites: `ttk-near-test`, `ttk-inventory-test`, `ttk-input-test`, Python 260 OK
(2 skipped). Before/after: `research/before-after/D08A45/` (local).

## Limits

- Polymorph only, as D08A46: Classic draws the original packets (pink).
- The wheel cannot know the order the jewels went in beyond the used bits;
  E sets them in item order, so the sockets match. A jewel used out of order
  (the mission inventory's Enter) colours the sockets in item order.
- Not established: the colours under every light of level 6; a full
  playthrough of the level.
- In warm corridor light the blue reads slightly cyan; clearly apart from the
  green.

## Player check

Level 6 FAMILY JEWELS, backtick console: `spawn family jewel`, `spawn family
jewel 2`, `spawn family jewel 3`; walk over one, then `.` for the mission row.

```sh
python3 recomp/tools/local/run.py
```
