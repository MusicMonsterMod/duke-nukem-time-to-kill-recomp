# D17BJ - GOLD AND GUNS cabin shaft shows the borrowed sky

2026-11-17. **Done, user-accepted** ("lock it in and close it out"). User report (2026-11-14 playthrough, UI slot 9,
the 13:09 save, GOLD AND GUNS): a cabin's ceiling area is bright or open and
should show the same sky as the level's start (D17AO1, [note 398](398-d17ao1-borrowed-sky.md)).
Private runs only: a hash-checked copy of the player's cards in
`recomp/analysis/d17bj/cards` (UI slot 9 = runtime slot 8); every run checks
the player's saves and settings before and after (unchanged).

## What the ceiling draws

The save looks up a vertical shaft whose walls are brightly lit timber and rock.
Its top is open: nothing is drawn there, so the frame's cleared background
shows, black. Unlike the level's start shaft, it has no face painted with the
small sky picture (CLUT `0x1438`, page `0xe`, UV 64..86 x 232..255): a
primitive trace of the view emits no such face and processes none; the level's
texture table holds the picture once (index 125, table `0x801c0454`). D17AO1
requested the borrowed sky only while a painted face had been left out in the
last two compositions, so this opening never asked for it.

## Change

A level with a painted sky and no sky of its own (the `painted_skies` table:
GOLD AND GUNS only) now draws the borrowed Duke Hill sky behind every
composition, not only while a painted face is in view
(`sky_borrow_wanted()` is `sky_borrow_ready`; `sky_borrow.h`). The sky
routine draws behind the world, so it can only appear where no world
polygon covers the pixel: true openings. No per-opening data was added; the
start shaft's painted face is still left out as before. Modernized only, one
player, OpenGL, as D17AO1; Vanilla is unchanged (the readiness test requires
the Modernized profile). `DNTTK_SKY_BORROW=0` still turns the whole borrowed
sky off.

## Is anything else behind the level open?

Concern: drawing a sky behind a mine could show it through cracks, seams or
beyond the draw distance, where black was intended. Measured with a developer
gate (removed again) that switched the always-on sky per composition, so each
view was shot off and on from the same camera 3 frames apart, 180 Hz first
person:

| Views (8 yaw steps each) | Black pixels with the sky off | Black that became sky |
| --- | --- | --- |
| Cabin shaft, view levelled | 0.00-1.19% | All of it: the shaft opening, nothing else |
| Lava cavern beside the cabin, looking down | 0.30-0.63% (stone seams) | 0 pixels |
| Level start (`level 3`), timber tunnel | 2.78-10.40% (dark texels, tunnel end) | 0-8 pixels |

The black seams and tunnel ends are drawn dark texels, not background.

## Verification

| Check | Result |
| --- | --- |
| Cabin, first person 180 and 120, third person 180 | The opening shows the borrowed sky (fill colour with the slow clouds); 0.00% black pixels. |
| Start shaft (D17AO1's UI slot 11 copy) | Unchanged: Duke Hill's clouds through the painted opening. |
| Level 2 save | Not a painted-sky level; the rule does not apply. |
| 180 Hz cost, start tunnel (sky now drawn with no opening in view) | On: 59.88 fields/s, 1442 distinct images in 8 s, 0 misses. Off (`DNTTK_SKY_BORROW=0`): 60.0, 1441, 0. |
| `ttk-sky-test` | ok. |
| `ttk-controls-test` | Does not link before or after this change (pre-existing undefined references: `mission_keycard_colours`, `mission_jewel_colours`, `psx_host_physical_cores`, `replay_load_bake`); not run. |
| Local unit tests | 262 run, OK (2 skipped). |

Before/after: `research/before-after/d17bj/` (local).
Executable `dadfbdea2c1a365654b63b4c817e8ccf70a5442edb827f6ac22076ceb7e0e544`.

## Limits

- Verified offscreen on the desktop GPU; the user accepted the result.
- Survey: three areas, 24 views. Other GOLD AND GUNS rooms were not visited; any
  other true opening there now also shows the sky (intended), and any pit or
  void open below would show the sky's fill colour instead of black.
- The sky seen through this shaft is mostly the fill colour with faint clouds,
  as at the start shaft (Duke Hill's clouds sit 10-31 degrees above the
  horizon; the D17AO1 deck covers overhead).
