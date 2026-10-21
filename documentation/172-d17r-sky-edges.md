# D17R - sky across the widescreen columns

2026-10-20. **Done (user-accepted 2026-10-21: "Perfect!! Lock this in.").**
Follow-up sky artifacts while looking up/down and turning are D17R1. User report (2026-10-04, D22A playtest): in
LEVEL01 UI slot 9, looking up, the central sky is right but it turns
black/dark toward the left and right edges. Also reported in the first
medieval level (UI slot 2) and HOG HEAVEN (level 10).

## Finding

The defect is the D14 widescreen columns, not the sky art, first person or
high refresh. With widescreen off (4:3) the same views are clean. In 16:9 the
black areas start exactly at the 4:3 boundary (native x -256 / 256, 85 native
pixels inside each edge).

The original sky routine `0x800388e4` (note 92) runs after the screen-mode
call `0x8001fc44` at `0x800265bc`, whose `0x8001f9f4` rewrites the portal root
rectangle `0x800d2210` to the 4:3 `(-256,-120,512,240)`. D14 widens that
rectangle only for the world render (`0x800b4d9c` from `0x8002e4c8`). Three
parts of the sky are therefore still sized for 4:3:

| Part | Original code | 16:9 symptom |
| --- | --- | --- |
| Flat sky-colour fill | When the RGB at `0x800da468` is non-zero, an F4 (`0x800b39bc`) from the root rectangle's x/y down to the horizon, drawn at OT 0x7ff by `0x8002bc18` (ra `0x80038bb4`). | Black rectangles in both top corners, vertical edges at the 4:3 boundary. |
| Camera-facing backdrop | Selector `0x800da464`, 18 vertices / 8 full-width rows, X -416..416, elevation 2-38 degrees, rotated to the camera's look direction (`+0xc/+0x10`), matrix load `0x800292a0` ra `0x80038c68`. | A slanted edge past which the cloud layers are drawn over black: the "night" edges. |
| Object-renderer clip rectangle | The routine copies `0x800d22a0` (always 4:3) into context `0x800d67a8` +0x18..+0x1e and sets flag 0x80. `0x80010000` ORs per-vertex outcodes against it and rejects a polygon whose corners are all outside one side. | Cloud and horizon-ring polygons wholly inside a margin are dropped. |

The horizon ring (`0x800da344`, radius 480) and both cloud bands (`0x800da45c`,
`0x800da46c`, radius 474) cover 360 degrees; only the backdrop is partial.

Levels whose sky has no fill colour (the city levels 0, 5 and 9: fill
`000000`) skip the fill and are black overhead in 4:3 as well (note 92); that
original look is unchanged.

## Change

`recomp/src/ttk/sky_widescreen.cpp` (plugin `ttk.sky.widescreen`), on three
entry hooks that already existed (no regeneration):

- `0x8002bc18` ra `0x80038bb4`: the fill polygon's x pairs become
  `-256-m..256+m`.
- `0x800292a0` ra `0x80038c68`, primary camera: the backdrop matrix's row 0
  (the model's local X, `+0/+2/+4`) is scaled by `(256+m)/256`. A trace of
  the matrix against camera `+0x20` showed the model's forward axis in row 2
  (the look direction); scaling column 0 instead, the first attempt, skewed
  the backdrop off the view yaw everywhere except yaw 0. Because the backdrop
  faces the look direction, its X axis is the view's horizontal right axis, so
  its screen edges move out by exactly the widescreen factor and its vertical
  colour gradient is unchanged.
- `0x80010000` from the four sky draws (ra `0x80038c94`, `0x80038cf0`,
  `0x80038da8`, `0x80038e44`) with context `0x800d67a8`: clip x becomes
  `-256-m..256+m`.

`m` is `psx_mod_widescreen_x_margin()` (85 at 16:9, 192 at 21:9). Each
rectangle is only widened from the exact 4:3 pair, so nothing is widened
twice. Gates: margin live (Vanilla and widescreen off are always 4:3), one
player (split screen keeps its viewports), and the D17O `SkyRenderIdentity`
digest of the whole resident routine. The D17O camera-relative replay
exemption, sky timer, textures, layer rotations, draw order and 4:3 output
are untouched; the routine still builds its matrices in every worker redraw,
and the hooks run there too. `DNTTK_SKY_WIDE=0` (developer, not saved) keeps
the 4:3 sky for comparison.

The near clip (`near_clip.cpp` object takeover) does not read the context
rectangle, and the original renderer runs after every entry hook, so plugin
order does not matter.

## Evidence

Offscreen OpenGL 4x, 16:9 unless stated, Modernized first person, 60 FPS,
private copies under `recomp/analysis/d17r-sky-edges` (cards copied from the
D11D/D12B intake; the player's slot files 07/08 have since been overwritten).

- UI slot 9, file 08, SHA-256
  `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`:
  before, headings 0/90/180/-90 at pitch -30 and -50 show the corner
  rectangles and the slanted edge; widescreen off shows neither. After, the
  fill, gradient and clouds run to both edges at every heading tried
  (including 70/-20 and -120/-25, where the column-scaling attempt failed).
- UI slot 2, file 01, SHA-256
  `756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`
  (medieval, fill `060a0d`): top-corner pixels change from (0,0,0) to the
  fill's (0,8,8). The report of "strange behaviour with the sky directly
  ahead" was not separately reproduced; no other sky artifact was seen in
  these views.
- Level-select survey of every selectable level start (0-3, 5-12, 21-29;
  `surv-0-15.json`, whose 13-15 rows repeat level 12, and `surv2.json`).
  Sky with a fill colour: 1 (`291f66`), 6, 8 and 28 (`060a0d`), 10 and 11
  (`000099`). The city levels 0, 5 and 9 have layers but no fill; the others
  load no sky at their start. HOG HEAVEN uses the
  same authenticated routine with fill, backdrop, ring and both cloud layers;
  its start area is indoors and the outdoor HOG HEAVEN view was not reached
  offscreen, so that view rests on the shared code and the user's playtest.
- Third person and 21:9 on slot 9: continuous to both edges. 120 FPS on
  slot 9: 849 replays, 0 failed, 0 misses; replayed frames match. D17O slot
  10 (file 09 of the D17O work copy) at 120 FPS with `DNTTK_SKY_TRACE=1`:
  all 360 `[sky-native]` records at offset 0,0,0, no `[sky-xf]` substitution.
- `ttk-sky-test EXE` (new): margin 0 / 85 / 192, exact rectangle widening,
  row-0 rounding, other callers, cameras, contexts and the cloud layer left
  alone, split screen, changed sky code.

Native near, aim, input and sky suites pass. `ttk-controls-test` aborts on
its stale level fixture (`code_writes==0` guard), as recorded before this
job; it does not build this file. Player executable
`fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d`.

Remaining limits: the backdrop is stretched horizontally, not re-authored, so
at 21:9 its gradient rows are wider than the art intended (the gradient is
vertical, so this is not visible as distortion). The city levels' original
black overhead is not changed. 240 FPS / Unlimited and every optional camera
combination were not individually captured.
