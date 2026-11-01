# D17AU - Polymorph decals: what the remaining shimmer is

2026-11-01. Report first, as the job asks. Starts from note 348 (D17AT).
Status: Cancelled 2026-11-01 after the report. User, after playing: "lets
shelve this. it looks so good right now ... its better than ever". The
proposed stages were not built. Two developer switches stay, off by default
(`DNTTK_LATE_TEST_YAW_FILE`, `DNTTK_DECAL_EXACT`); nothing changes in play.

Reference: the user's clip `research/screencaps/Video_2026-11-01_14-22-44.mp4`
(slot 7 clean; slots 11 and 12 "a very very slight shimmering" while panning).

## Method: a fixed camera path instead of video

Video compression hides changes of a pixel or two, and live mouse input never
repeats exactly. New developer switch `DNTTK_LATE_TEST_YAW_FILE` (title code,
`frame_replay.cpp`, off unless set): a file holding degrees that every redraw
adds to the late camera's yaw. With Duke standing still, the harness writes
0, 0.02, 0.04 ... degrees (about 0.2 internal pixels per step at 4x), waits,
and captures. Two runs of the same path are pixel-identical above the weapon,
so variants can be compared on exactly the same views.

Harness `recomp/analysis/d17au/` (local): `pan.py` (private fixture copies
from `d17at/fixture`, player files hash-checked; offscreen, 16:9, first
person, FOV 90, Polymorph, corrected geometry and textures as the player
uses them, 180 Hz, internal scale 4 like the player), `tflick.py`, `jit.py`.

Measure (`tflick.py`): over the path, a blood pixel that changes by more than
60 (RGB sum) and changes back by more than 60 in the next step. Smooth motion
moves an edge across a pixel once; a pop back is aliasing or jitter. Reported
as a share of the pool's pixels per step; the floor beside it as reference.
The bobbing weapon is excluded where it overlaps the pool.

## Findings

### 1. Decal corners still wobble on screen (fixable now)

D17AT11 placed each blob exactly in depth, but its corners are still drawn at
the game's GTE screen positions (`draw()` keeps the game's projected position
as `ex/ey` for every original vertex). With corrected geometry that position
has a sub-pixel divide but view X/Y truncated to whole units first, plus the
blob's own truncated translation. Every blob gets a different error that
changes as the camera moves, so overlapping blobs slide against each other
and the speckle gaps between them open and close.

Traced on slot 11 (`jit.py`, 18280 corners, focal 160 native px): corners sit
up to 0.20 native px (median 0.04) from the exact projection of their own
view positions; blobs slide against each other by up to 0.1 native px from
one image to the next. At 4x internal scale that is up to about 0.8 internal
pixels.

Measurement switch `DNTTK_DECAL_EXACT=1` (default 0): decal faces are
projected from their exact view positions (`Poly::exact_screen`), so blobs
move only with the camera. Depth, slots, order, UVs and colours unchanged.

| Save, view | Default | Exact corners |
| --- | --- | --- |
| Slot 11, standing at the pool (walk 0.9 s, then the path) | 0.09% of blood px per step | 0.06% |
| Slot 11, from the save spot (far, view depth ~8700) | 0.11% | 0.08% |
| Slot 12, the path | 0.96% | 0.77% |
| Slot 12, no camera motion (control) | 0.70% | 0.65% |

Slot 12 after subtracting its control (see 2): about 0.26% falls to 0.12%.
Same path at internal scale 2: 0.11% against 0.08% (slot 11), 0.92% against
0.80% (slot 12). Every blood face still takes the decal path (no native,
dropped or mismatched faces, every blob refined), no replay failures or
misses; `ttk-near-test PASS` with the switch off and on.

### 2. Slot 12: the canal water over the pool (authored, not a defect)

In slot 12 the pool lies under the canal's translucent animated water. The
water's band paints over the blood the same way in Vanilla
(`--vanilla`, same save) and with the game's own decal slots
(`DNTTK_DECAL_SLOT=0`). With the camera still, the animation alone toggles
0.70% of the pool's pixels per step (0.40% of the floor). Most of the change
seen over that pool in slot 12 is this water, which the original draws the
same way. Changing it would change the level's look; not proposed.

### 3. What remains: aliased speckle edges

With exact corners, what still pops in slot 11 is single pixels scattered
along the edges of the blood's speckles (texels against transparent texel 0,
red against dark floor). The floor beside it, low in contrast, shows
practically none (0.00%). That is texel-edge aliasing: an edge passing near
a pixel centre flips that pixel with tiny changes of the view. Supersampling
alone (scale 2 against 4) barely moves the count, and the game's optional
bilinear filter would not help: it keeps cutout edges hard (texel 0 still
discards).

Ruled out on the way: depth precision (24-bit `1 - 16/z`, about 0.004 units
at 1000, far below the 2-unit decal margin); order changes among blobs (one
shared slot since D17AT12, order fixed); rotation rounding of each blob's
transform (about 0.07 units across a blob, under 0.01 px at these
distances).

## Proposed stages

- **D17AU2 - Exact decal corners.** Make `DNTTK_DECAL_EXACT` the default
  (developer switch to compare), with a `ttk-near-test` contract, 180 Hz
  check and player playtest. Small, title code only, no framework change.
  Expected: blobs no longer slide against each other or the floor; about a
  third to half of the pan-induced pops go.
- **D17AU3 - Anti-aliased blood edges (framework).** For decal colour
  passes only: the fragment shader takes several samples of the cutout
  across the pixel's texture footprint and turns the share of opaque samples
  into coverage, blended over what is beneath, so a speckle edge fades by
  fractions instead of flipping whole pixels. It needs the surface under the
  decal to be drawn first, so the decal colour pass moves after the opaque
  surfaces of its ordering slot range (a true decal composite: blobs of one
  pool still in the game's object order), while translucent surfaces such as
  the canal water keep painting over it as now. New host-vertex mode in
  `mod_plugins.h` and the replay stream (not a hashed header), patch export
  and clean-pin check. Larger; report its exact design before building.
- Not proposed: changing the slot-12 water, blood-only bilinear (does not
  touch the cutout edges), higher internal scale (no measurable gain).

## Limits

- Measured offscreen at 180 Hz on private copies of slots 11 and 12; not yet
  seen by the player. The pop measure is a threshold count; it shows
  direction and size of the change, not what the eye notices.
- Slot 7 (accepted as clean) was not re-measured.
- The 1024-wide capture is smaller than the player's 1473-wide window.
