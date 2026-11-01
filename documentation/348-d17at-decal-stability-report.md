# D17AT - Ground decals and small props on any surface

2026-11-01. Report (sections 1-6), then the approved implementation
(D17AT2 blood, D17AT3 water, D17AT4 checks; from "Implementation" on).
Design approved by the user 2026-11-01 ("i approve the recommended design of
the proposed fix in 3 stages"; water option A). Status: Done, user-accepted 2026-11-01 (see the end of this note).

**Correction to the report.** The report attributed the blood to the game's
per-frame sprite queue (`0x8009b7fc` / `0x8009b1c0`). Instrumented hooks
during D17AT2 showed that queue is not reached in these scenes: the blood is
a small flat 3D mesh drawn by the small-model renderer `0x8002e02c`
(section "Implementation"). The emitters and ordering key in the report are
the shared ones and are right; the queue description stays below as
reference, and the conclusion (painter-only native packets over host floors)
holds.

User: the shells on the ground in slot 12 (file 11, medieval canal) clip in
and out as the water texture animates; ground blood in slot 11 (file 10) pops
in and out when walking left and right over it; the blood in slot 7 (file 6,
castle tower, across the jump) flickers. "I want our renderer to absolutely
own that ... to never have this problem whatsoever."

## Method

Private copies of the three states and both cards in
`recomp/analysis/d17at/fixture` (player files hash- and mtime-checked by
every run). Offscreen, 16:9, first person, FOV 90, Corrected geometry and
textures, Polymorph (default), 60 Hz. Harness in `recomp/analysis/d17at/`:

- `route.py` - load a slot, follow steps, shoot every step (or bursts).
- `gp0.py` - GP0 packet dump of one live frame (needs `PSX_FORENSICS=1`).
- `wt.py` - write trace of the packet ring, to find which routine builds a
  packet.
- `decals.py` - reads the decal queue; `mdis.py`, `xref.py` - disassembly
  and call references in the owned executable.
- `sheet.py` - contact sheets of crops.

## Reproduced

| Slot | What shows | Route |
| --- | --- | --- |
| 12 (file 11) | The two shotgun-shell boxes lose their lower halves in some frames and not others, standing still (`s11.png`) | `still11`: 12-24 shots, no input |
| 11 (file 10) | The blood pool is cut along a straight floor edge; how much is cut changes as Duke walks and strafes (`w10b.png`, `w10b2.png`) | `walk10`: forward, then strafe left and right |
| 7 (file 6) | The thin blood on the ledge floor across the jump comes and goes during a slow turn; the wall splat is stable (`s6slow.png`) | `slow6`: turn one count per frame |

Standing still in slot 7 shows no change in 24 frames; it needs camera motion.

## Three paths, two causes

### Blood: a decal queue that nothing on the host owns

Blood is not drawn by either mesh renderer the host has taken over. It is a
**per-frame decal queue**:

- `0x8009b7fc` queues one decal: a 32-byte record (+0 x, +4 y, +8 z world
  position, +0xc angle, +0x10 flags, +0x14/+0x16 half sizes, +0x18/+0x1c
  texture words). Count `*0x800deb40`, capacity `*0x800c57a0` (192), list
  `*0x800c5798`. Eleven call sites queue decals (`0x80030340`, `0x80030760`,
  `0x8003084c`, `0x8003d190`, `0x8004a694`, `0x8006d074`, `0x8008ef9c`,
  `0x8009cb68`, `0x8009cd14`, `0x8009d3dc`, `0x8009df6c`); which effect each
  one is (blood, scorch, shadow...) is not yet identified.
- The camera render calls `0x8009b750` once (`0x8002e808`), which runs
  `0x8009b1c0` over the queue and then clears it (`0x8009b7b8`).
- `0x8009b1c0` builds each decal as a flat quad in its own plane (corners
  (+-w, +-h, 0), rotated by the record's angle and orientation, translated by
  position minus eye), projects it with RTPT and emits it through the
  generic face emitters `0x8002d508` -> `0x8002cff0` (FT4, `0x2c`). Close to
  the eye it is split into four quads.
- The ordering slot comes from `0x8002bdc8`: per-face mode in flags bits
  12-14 (first, last, average, farthest or nearest corner SZ), bit 15 moves it
  two slots nearer; `0x8002bd48` links it at SZ >> 5, the same scale as world
  polygons.

Traced in slot 11: every blood packet (texture page `0x1c`, CLUT `0x57ac`,
the identities D17K saw) is written at `0x8002d0d8`/`0x8002d0e8`, inside
`0x8002cff0`, called from the decal queue.

So since D17AM the floor under the blood is host geometry with depth, and the
blood is a native painter-only packet with no depth. Whether a piece of
floor covers it depends only on which ordering slot is nearer: a floor
polygon whose farthest corner is nearer than the blood's key is painted
after it, and wins. The blood in slot 11 spreads over a floor edge: the
polygons beyond it sort in front at some positions and behind at others, so
the cut appears and disappears as Duke moves. D17K fixed the case where host
subdivision made floor pieces sort nearer than the whole floor polygon; it
cannot fix a different floor polygon that the original itself sorts nearer.

**Not a Polymorph regression.** The same route with the game's own packets
(`DNTTK_NEAR_CLIP=0`, `w10nc.png`) cuts the same blood along the same edge,
and worse (whole triangles). The original game has the problem; the host
renderer has never owned decals.

Whether the floor beyond that edge is slightly higher than the blood's plane
(a real lip) is not established. If it is, a true-depth rule hides the blood
consistently there instead of flickering; see the contact allowance below.

The slot-7 ledge blood is the same mechanism at a grazing angle and long
distance: thin coplanar decal against floor polygons whose keys swap order as
the view turns.

### Shells in the canal: water writes depth over things just under it

The slot-12 floor in that spot is the canal's animated water surface (water
tile builder `0x8002ef90`, caller `0x8002fbb4`). Since D17AN every water tile
is host-drawn with depth, pushed 32 units deeper so scenery just under the
waterline stays over it, as the original paints it.

The shell boxes are an ordinary static prop (`0x8012249c`, caller
`0x80032288`, host-drawn with depth). Measured against the traced water
plane, its 24 vertices lie from **15 units above to 140 units below the
water surface** (eye 674 above it). The 32-unit allowance covers the top;
the lower half is deeper and fails the depth test against the water.

The water writes depth only where its texel is not transparent (texel 0 is
discarded), and the water texture animates. So the cut follows the moving
texture: "clip in and out as the water texture animates" exactly.

Confirmed by switch: with `DNTTK_WATER_DEPTH=0` (water in painter order, as
the original) both boxes are complete in every frame (`s11wd0.png`); with
the default they are cut.

## The rule the host should own

Requirements from the job: a decal or small prop lying on a surface is drawn
over it at every distance, angle and frame rate, in every level, and never
shows through walls.

### 1. Decals become host geometry (blood, and whatever else uses the queue)

Hook `0x8009b1c0` (authenticated like the other renderers) while the near
path is live. For each queued decal, compute its four corners in view space
from the record and the GTE state the routine sets up (exact positions, the
same way `world()`/`object()` get theirs), and emit it through the existing
host path:

- same texture, CLUT, colour, semi-transparency and native ordering slot
  (the original's own key), so it interleaves with sprites and effects as
  before;
- clipped by the host near plane and guard band (no four-way split needed);
- **depth at its own plane**, moved toward the eye by a **decal contact
  allowance**, and written: texel-0 pixels are already discarded, so the
  splat's transparent parts never write depth. A floor drawn later then fails
  against the decal; a wall nearer than the decal still hides it (no
  draw-through).

The allowance must exceed what separates a decal from its surface: the
game's integer world position and fixed-point rotation, plus depth-buffer
resolution at long range (slot 7 is far). Proposed: the D17D prop contact
rule widened with distance (a constant 2-4 units near, growing in
proportion to view depth far away, both measured before choosing). It stays
small enough that an actor standing on blood is not drawn behind it.

Once the decals are host geometry, the game's packet path draws nothing for
them (as with meshes), so there is no duplicate.

### 2. Water becomes a surface layer: tested, not written

Water tiles keep their depth test, so a bridge deck or walkway nearer than
the water still hides it (D17AN's bridge start, slot-10 walkway grating).
But they stop writing depth. Then whatever the game paints after the water
(props, pickups, debris lying in shallow water) draws over it as the
original painted it, at any depth below the surface, and nothing changes as
the texture animates. The 32-unit push is no longer needed.

This needs one framework addition: a host-vertex "test but do not write
depth" mode (the GL path today either tests and writes, or does neither).
That is a small change to `gpu_gl_renderer.c` and the replay stream,
exported into `time-to-kill-accepted-source.patch`; the hashed framework
headers that would reject savestates are not touched.

Alternative without a framework change: keep water depth and raise the push
until the deepest prop in shallow water is covered. That is a per-level
number, not a rule, and still breaks in deeper water. Not recommended.

### 3. What already holds and stays

Host-drawn props on world floors keep D17D's plane depth and contact rule
(the shelf, the cup, the ladder ammo stay as accepted). Translucent host
faces stay painter-only. The first-person layer, sky and Bio Mask are
untouched. Vanilla never reaches any of this; Classic (D17AR6) gets the same
rule wherever its near path is live, since the decal and water paths are
shared.

## Proposed stages

- **D17AT2 - Host decals.** Identify the eleven queue callers; hook
  `0x8009b1c0`; emit decals as host geometry with the contact allowance;
  measure decal-to-surface separations and depth resolution to set the
  allowance; prove matched-view pixel equality against the native packets on
  a flat floor at mid distance. Fixtures: the three slots plus the D17K
  blood. No framework change.
- **D17AT3 - Water as a surface layer.** Framework test-only depth mode,
  patch export and clean-dependency check; water tiles use it; regression on
  D17AN (slot-12 bridge start), slot-10 walkway grating, slot-2 moat and
  canal (UI 2), and the D17AS route.
- **D17AT4 - Sweep.** All three user saves, 60/120/180 Hz, FOV 60/90/120,
  first and third person, Classic and Polymorph, other levels' decals
  (scorch marks, shadows once identified), performance at 180.

## Limits of this report

- Reproduced at 60 Hz offscreen only; the fixes are frame-rate independent by
  construction (the redraw workers run the same hooks), to be verified at
  120/180 in D17AT2-4.
- The floor edge under the slot-11 blood was not measured; whether it is a
  real lip is open.
- The queue's other users are not identified yet.
- No player saves, cards or settings were written (hash-checked each run).

Local evidence (not distributed): `recomp/analysis/d17at/` - `s11.png`
(default, shells cut), `s11wd0.png` (water painter order, shells whole),
`s11sd.png`, `w10b.png`/`w10b2.png` (Polymorph blood cut), `w10nc.png` (game
packets, same cut), `s6slow.png` (slot-7 ledge blood), `wt*.json` write
traces, `gp0/` packet dumps, `runs/tr10`, `runs/tr11` primitive traces.

## Implementation

### Blood: the small-model renderer (D17AT2)

What actually draws the pools: `0x8002e02c` (called from `0x800a27bc`) sets
a model's GTE transform (`0x8002df20`: full rotation, position minus eye),
projects its vertices with RTPT into 8-byte records (`0x8002dce0`: SXY, SZ >>
2, clip flags at +6: 1/2/4/8 screen sides, 0x10 nearer than the near limit,
0x20 past the far limit) and draws the faces with `0x8002d848` through the
generic emitters (`0x8002d43c` -> `0x8002cff0` for the blood's FT4), each
linked by `0x8002bd48` at SZ >> 5. Model layout: +0 face count, +2 vertex
count, +4 face flags, +8 index lists, +0xc vertices (SVECTOR), +0x10/+0x14
texture lists, +0x18 colours. Face flag 0x400 drops a face with any 0x10/0x20
corner (`0x8002d7a8`), 0x800 drops a face wholly off one screen side
(`0x8002d7f8`), 0x200 is the NCLIP back-face test. Each blood blob in slot 11
is one four-vertex model lying 4 units above its floor.

`model_depth.inc` (title code, included in `near_clip.cpp`), two new entry
hooks in `game.local.toml` (`0x8002D848`, `0x8002BD48`, regenerated C, codegen
hash unchanged at `0x8bab543c`, player slots still load):

- At `0x8002d848` (from `0x8002e06c` only, Modernized, near path live,
  unified geometry, both code ranges byte-authenticated): every vertex is
  projected again from the model data and the live GTE state with
  `near_project`; any mismatch with the game's record leaves the whole model to
  the game. Near flags (0x10) are cleared so faces under Duke's feet are built
  rather than dropped; far rejection and the screen cull of safe faces stay.
- At `0x8002bd48` from an emitter while that model is live and the GTE
  transform is still the model's: the packet's corners are matched to the
  model's records (a position shared by vertices of different depth counts as
  ambiguous), and the face is drawn as a host prop polygon: the game's colours,
  texture, CLUT, page, semi-transparency and ordering slot, host clipping,
  plane depth and the D17D two-unit prop contact allowance. The native packet
  keeps a zero-length link (as water tiles do). A face the host cannot take
  stays native, except one with a near corner, which is dropped (its native
  corners are saturated).
- Developer switch `DNTTK_MODEL_DEPTH=0` leaves these models to the game.
  Counters: `near.models` in the debug JSON.

Visible consequence, by geometry and not a defect: in slot 11 the pool
spreads under a floor face 128 units higher (a step; the blood lies 126-130
units below that face's plane, measured from the trace). The painter order
sometimes drew the pool over the step and sometimes not; true depth now hides
the part under the step every frame, so the pool ends in a straight line at
the step edge.

### Water: tested, not written (D17AT3)

Framework addition (exported in `time-to-kill-accepted-source.patch`):
`psx_mod_gpu_host_vertex_test_only` (`mod_plugins.h`, not a codegen-hashed
header). The host-vertex entry carries a test-only flag; the render-replay
stream carries it as a tenth word on such vertices; `gl_set_host_depth` takes
a mode (1 test and write, 2 test only); textured runs and flat batches split
on it and set `glDepthMask` per run, restoring it afterwards. Other renderers
never used host depth and are unchanged.

`water_clip.inc`: depth-tested tiles are test-only, keeping the D17AN
32-unit push. A bridge deck nearer than the water still hides it; whatever the
game paints after a tile (the slot-12 shell boxes, up to 140 units under the
surface) draws over it as the original paints it. Developer switch
`DNTTK_WATER_WRITE=1` restores depth writes.

## Verification (D17AT4)

Private fixture copies, offscreen, player files hash- and mtime-checked
unchanged by every run. Harness `recomp/analysis/d17at/` (`route.py` now also
saves `near.models`, `near.water` and replay counters; `cover.py` traced
cover with depth at shot points).

| Case | Before | After |
| --- | --- | --- |
| Slot 12 shells, 12-frame still bursts, 60 and 120 Hz | lower halves cut, changing frame to frame (`s11ww.png`, water writes) | both boxes whole in every frame (`s11new.png`, `s11-120.png`) |
| Slot 11 blood, walk then strafe, 60 and 120 Hz | cut along the floor edge, amount changing as Duke moves (`w10b2.png`) | identical in every frame; ends at the step edge (`w10new.png`, `w10-120.png`) |
| Slot 11 walking onto and around the pool | - | pool complete around Duke, no gaps (`over2.png`) |
| Slot 7 ledge blood | painter-only packets | drawn by the host path (tint run `tint6.png`); a slow turn sheet looks steady (`s6new.png`); not separable by a pixel metric, player check needed |

- Counters over these routes: `models.mismatch` 0, `faces_native` 0,
  `faces_dropped` 0, `faces_budget` 0; every face taken. Third person: 440
  near flags cleared, no drops. Vanilla: 0 models and 0 water tiles taken.
- Water regressions with the old rule as the comparison
  (`DNTTK_WATER_WRITE=1`): slot-2 moat wade and dive (D17AS route) identical
  to the pixel in 9 of 10 shots (2 pixels in the other); D17AN slot-12 bridge
  pan identical until the pan's timing drifts, deck clean in both; slot-10
  walkway grating identical except jump-timing frames.
- 120 Hz: replay failures and misses 0 on every route. 180 Hz, slot-12 canal
  turning, two runs each against both switches off: cadence mean 5555/5543
  us, p95 5849-5922 / 5883-5962 us, worker job average 4779-4798 / 4639-4773
  us, hook 336-341 / 331-333 us per frame: no measurable cost.
- `ttk-near-test PASS`, with new contracts: water tiles take the test-only
  path; a small model is re-projected, near flags cleared, its face drawn by
  the host and its native packet emptied; another transform or Vanilla leaves
  it native.
- Python suite: 221 tests, 2 skipped, 1 failure,
  `test_primitive_trace.test_observational_trace_and_gate` (static-prop
  instances absent from the native fixture's trace). It fails identically with
  the committed `near_clip.cpp`, `water_clip.inc` and test fixture, so it is
  pre-existing and not caused by this job.
- Framework patch exported; applied to a clean checkout of the pinned
  dependency it reproduces all changed files byte for byte.

First implementation executable SHA-256 (superseded by D17AT5 below):
`c3b6902549e3049ca51222425cd7158c9dd4f8576efab5650c8455a04db74e72`.

## Follow-up: shimmer where blobs overlap (D17AT5)

User playtest, 2026-11-01: "it's flickering only when there is blood decals
on top of other blood decals ... more of a shimmer. the blood decals directly
on a surface do not flicker." Slot 7 re-saved closer to the pools (new private
copy; the earlier copy kept under `fixture/openbios/old-slot06`).

Cause, measured: every blob wrote its own depth. Blobs of one pool lie on the
same surface within a fraction of a unit, and each model has its own rounded
GTE translation, which changes as the camera moves. In the traced slot-7 view
289 of 514 overlap samples had the two blobs less than 1 unit apart. Over a
13-frame slow pan with fixed screen points, the blob winning on depth changed
1432 times against 279 changes for a fixed draw order (edges moving under the
points): the shimmer.

Rule (like Build/Polymost's floor- and wall-aligned sprites: an offset
against the surface plus a fixed sprite order): a small model whose vertices
all lie within half a unit of one plane is a decal. Each opaque face is drawn
twice in its native slot: a depth-only pass (`psx_mod_gpu_host_vertex_depth_only`,
new framework mode: depth test and write, colour mask off) 2 units toward the
eye, then the colour pass test-only 6 units toward the eye. A later blob of the
same surface always passes (blobs keep the game's painter order among
themselves), a surface drawn later (2 units behind) never does, and anything
nearer than 6 units hides it. Translucent faces stay painter-only. Developer
switches: `DNTTK_DECAL_LIFT` (6), `DNTTK_DECAL_WRITE` (2); `DNTTK_DECAL_LIFT=0`
restores the single-pass prop rule. Framework: the host-vertex depth mode
(0 normal, 1 test only, 2 depth only) travels in the replay stream's tenth
word; GL runs set depth mask and colour mask per run and restore them
(including after the stencil fix-up pass). `depth_push` now honours its sign
(negative moves toward the eye; water's positive push is unchanged).

Checks: every blood face in the three saves takes the decal path (no
mismatches, drops or native faces), 120 Hz without replay failures, 180 Hz
cost within noise (cadence mean 5555 us both; worker job 4758-5103 against
4808-5082 us; hook 358-385 against 352-373 us). Shells (water) unchanged and
whole. `ttk-near-test PASS` with a contract for the two passes; the GL flat
batch shim test updated for the new run-state helpers and passing; Python
suite otherwise as before (the one pre-existing `test_primitive_trace`
failure). Patch re-exported and byte-identical on a clean pin. Executable
SHA-256 `a1756b9b3524ba5f05aa18b30933ff1b07c3d41531ac51fc59cd5f9b4e1ee92c`
(superseded by D17AT6 below).

Known difference from the original, deliberately kept: some floor blobs near
the slot-7 wall corner run on under the wall at a shallow angle. The game's
painter order smeared them over the wall; true depth cuts them at the wall
line. The hidden parts are well behind the wall (median 101 units in depth,
only 40 of 337 samples within 16), so restoring them would mean drawing blood
through walls.

## Follow-up: blood vanishing at the sides of the view (D17AT6)

User playtest of D17AT5, 2026-11-01, slot 7 re-saved closer: "the blood
shimmers less, but also disappears in the peripherals of the view" when
looking left and right.

Cause: the blood objects are drawn by `0x800a2690`, which first tests the
object's room (object +0x38) against the room bitset `0x800e1f10`. The portal
walk (`0x8006276c`, at `0x800627ec`) fills that bitset with the rooms it
lists. Polymorph (D17AR) shows more rooms than the portal walk lists (seen
over walls, or through an opening at or beyond the edge of the view) but
leaves the portal walk, and so the bitset, to the game. So a pool in such a
room was skipped whole while its floor was still drawn. This predates D17AT;
the larger, steadier pools made it obvious. On the user's save, turning left
and right, 434-840 blood-object draws per route were in rooms the host showed
but the bitset did not have.

Fix: host visibility records every room it showed in the frame
(`host_visibility_room_shown`, game-listed rooms included). A new entry hook
on `0x800a2690` (regenerated C, codegen hash unchanged) sets the object's room
bit for that object's test only, when the host showed the room and the game
had not set it, and clears it again when the model's faces start
(`0x8002d848`), at the next object or at the next scene. Nothing else in the
frame, gameplay included, sees a changed bitset. Polymorph only (Classic and
Vanilla never reach it). Counter `near.models.room_lifts`.

Also in this round:
- The decal offsets became depth scales of each face's own plane
  (`Poly::depth_scale`): a plane moved toward the eye by s scales every depth
  on it by 1 - s/|d|. Taken after the raster fallback, the colour pass can no
  longer end up behind its own depth pass where a shifted plane would fall
  back (checked on 7452 corner pairs in turning views: colour always nearer,
  minimum margin 1.19). Faces nearer than twice the lift keep the prop rule.
- Ruled out: the game's screen-edge cull of model faces. Redraws re-run the
  composition with the late camera (`apply_late` sets the rotation first), so
  that cull already uses each image's own camera. Clearing it was tried and
  reverted (it only added off-screen faces: worker time +14%).

Checks: `ttk-near-test PASS` with a contract for the room test (set only for a
host-shown room, cleared when the faces start, untouched for unshown rooms,
Vanilla and bits the game set). Slots 11, 12 and 7 at 120 Hz: every face
taken, no drops, no replay failures. 180 Hz on slot 7 turning: cadence mean
5555 us both, worker job 4654 against 4804 us, no misses. Python suite as
before (one pre-existing failure). Patch re-exported, byte-identical on a
clean pin. Executable SHA-256
`8afae3ace928ee8e7272932764f6f9eeaf08b7dea27eeffadcba68d9e502556d`
(superseded by D17AT7-9 below).

## Follow-up: the user's clip (D17AT7-9)

User playtest of D17AT6, 2026-11-01: "there is still some shimmering left,
and the decals disappear at the peripheries of the vision when you look right
to the edge of left and right". Recorded `research/screencaps/bloody.mp4`
(slot 7, 180 Hz). At 8.4 s the left-wall blood is visible out to the screen
edge; at 8.8 s, turned slightly further, the whole patch is gone. The harness
reproduces it from the same spot by pure rotation (which cannot change what
hides what), so a screen-position test was dropping faces. Three causes, each
measured and fixed:

- **Side cull (D17AT7).** The game culls a face wholly beyond one side of its
  cull rectangle, built from the draw rectangle (`0x800a27d4`). Inferred from
  the flags: horizontal half-width about 341, the original 4:3 view, while
  the 16:9 view reaches about 480. The left/right flags (1/2) are now cleared
  for live models; top/bottom stay with the game, since the view is wider,
  not taller. `DNTTK_MODEL_SIDE_CULL=1` keeps the game's cull.
- **Oversize cull (D17AT8).** The facing test `0x8002d604` (face flag 0x200)
  drops a face whose screen NCLIP is under 2 (back-facing) but also over
  120000: a face too big on screen, a limit for the original view. A large
  wall blob stretched across the wide FOV 90 view, partly off screen with
  clamped corners, crossed it and vanished whole: the 8.4/8.8 s case. While
  a live model's faces are built, 0x200 is cleared in its face flags (the
  face walk of `0x8002d848`: the flag word advances unless 0x20, the index
  list unless 0x40, the top two bits give 1-4 corners) and restored when the
  model ends. The host tests facing itself in view space (det(v0,v1,v2) > 0,
  as NCLIP for corners in front of the eye, and right beside or behind it),
  without a size limit. Packets are now matched to whole walked faces, which
  also stops dropping near faces whose clamped corners were ambiguous
  (71 dropped before, 0 after, on the turned slot-7 view).
- **Order flips (D17AT9, the remaining shimmer).** The decal colour pass
  keeps painter order among blobs, but the game sorts each face by its view
  depth, which changes as the view turns, so overlapping blobs in
  neighbouring slots swapped while turning. Measured on a traced 18-frame
  turn: the winning blob at fixed screen points changed 1631 times, against
  863 for any fixed order (edges moving under the points). Decals now take
  their model's slot from its straight-line distance (256-unit buckets, far
  side): turning never changes it, blobs of one pool share it, and inside a
  slot the order is the game's object order. Same turn: 831 against 802.
  `DNTTK_DECAL_SLOT=0` keeps the game's slots. (A depth-rank alternative was
  tried and rejected: each model's plane jitters about half a unit from frame
  to frame, more than any usable rank step; 3183 changes.)

Also confirmed with a GP0 dump: no blood packets are left on the game's own
path in slot 7 (only HUD and weapon packets are native).

Checks: `ttk-near-test PASS` (contracts updated: decal passes, room test,
facing); slots 11, 12 and 7 at 120 Hz: every face taken, none native or
dropped, no replay failures; 180 Hz turning on slot 7: cadence mean 5555 us,
worker job 4864 against 4892 us for the previous rules, no misses. Python
suite as before (the one pre-existing failure). Patch re-exported,
byte-identical on a clean pin; codegen hash unchanged. Executable SHA-256
`09a5a67496ff6258ff2883ba0f98d7fe92438a8724a21c5a1ceea5227bc05c78`
(superseded by D17AT10 below).

## Follow-up: harsh flicker while turning (D17AT10)

User playtest of D17AT7-9, 2026-11-01: edge disappearance gone, "but now the
flickering overall is present quite harshly". Clip
`research/screencaps/Video_2026-11-01_12-31-22.mp4`: still, the blood is
steady; while turning, a large straight-edged patch of a wall splat vanishes
for single images (frame 415, back at 416).

Reproduced offscreen: fast turns at 180 Hz, counting images whose blood drops
by over 10% against both neighbours. With the blood work off and with the
game's slots (`DNTTK_DECAL_SLOT=0`), 0 of 5 runs; with D17AT9's distance
slots, about one run in three. Cause: splats sit within about 2 units of
their surface, some slightly inside it (traced: per-splat median surface gap
0.5-2.2 units in view depth with the 2-unit contact shift). The colour pass
(lift 6) shows such a splat over its wall only if the wall was drawn first;
the depth pass, 2 units up, does not stop a wall drawn later. The game's
slots usually drew the wall first; the distance slot (straight-line distance,
never less than view depth) sometimes put the splat first, angle by angle.
Fix: the depth pass is written 4 units up (`DNTTK_DECAL_WRITE`, default 4),
so a surface drawn later loses to a splat up to about 4 units inside it,
whatever the order; blobs keep their order among themselves (the colour test
at 6 leaves a 2-unit margin over the half-unit plane jitter). Fast turns both
ways and a slower turn: no dropouts in 15 runs; blob order still stable
(winner changes 831 against 802 for edge motion).

Ruled out on the way (measured, not the cause here): leftover face flags at
frame boundaries (all images draw the same blood faces with or without the
new cleanup, which stays as a guarantee: any other hook and every flip now
end a model); the room check (passing every room's check made no difference
over 8 runs).

Checks: `ttk-near-test PASS`; Python suite as before (one pre-existing
failure); 180 Hz turning: cadence mean 5556 us, worker job 4629 us, no misses
or failures, no native or dropped blood faces. Patch re-exported,
byte-identical on a clean pin. Executable SHA-256
`284ce8ef9d2738284b210b67862ab11d0f1b5287bdeba41fe03230628103a978`
(superseded by D17AT11 below).

## Follow-up: blob plane jitter (D17AT11)

User playtest of D17AT10, 2026-11-01, slot 7
(`research/screencaps/Video_2026-11-01_12-57-03.mp4`): "significantly more
stable ... minimal glitching of the blood decals now, and nothing is
disappearing. please continue the robustness". The clip has no dropouts
(1029 frames); the remaining glitching is speckle where a splat and a
surface are within a few units (the wall-floor corner looking down).

Cause: `0x8002df20` places each model with TR = the camera MATRIX times
(object position - camera position) through ApplyMatrixLV (`0x800b444c`),
truncated to whole units; the truncation changes as the camera moves, so each
blob's plane moved by up to a unit from frame to frame. The blood-object hook
(`0x800a2690`) now records the object and the camera MATRIX (context +0xc;
the model is placed from object +0xc, its position at object +0x20); when the
model starts, the host computes the exact translation and moves its view
positions by the difference (screen positions stay the game's). Measured on
the user's save: every blob refined, largest correction 1.0 unit, as the
truncation predicts; what remains is the rounding of the rotation matrix,
about 0.05 units for a blob this size. `DNTTK_MODEL_REFINE=0` keeps the game's
translation.

Checks: `ttk-near-test PASS`; slots 11 and 12 at 120 Hz and slot 7 at 180 Hz:
no native, dropped or unrefined blobs, no replay failures; fast turns both
ways, no dropouts; blob order stable (829 against 802); 180 Hz slow pan:
cadence mean 5556 us, worker job 4842 against 4794 us without refinement.
No framework change this round. Executable SHA-256
`838ba4a4b64517463a1dd996cea4b282b6da8459ad0c68ef30b292bc3693656d`
(superseded by D17AT12 below).

## Follow-up: blobs swapping while panning (D17AT12)

User report on D17AT11 (`research/screencaps/Video_2026-11-01_13-27-51.mp4`,
slot 11): "the decals are reordering themselves and some come to the surface
some go below ... not invasive but it's not how I want this to ultimately
look"; present in all three saves, most in slot 7. In the clip the small
gaps where floor shows through the top splat change shape from frame to
frame while panning.

Measured: with D17AT9's distance slots, about 7% of blob comparisons between
consecutive images changed sort slot (the eye moves while strafing and, in
first person, while turning, so a blob near a 256-unit bucket boundary
crosses it), and each change can reorder it against its neighbours. Gaps
opening or closing inside the slot-11 pool per image at 180 Hz (after
aligning for motion; includes texture crawl): game drawing 365, distance
slots about 150, one shared slot about 159, a ranked depth rule about 180.
The measure is dominated by texture crawl and cannot show the swaps
themselves; the slot changes are real.

Rule now: every blood decal goes into one ordering slot (the farthest, drawn
first), so the order among blobs is always the game's object order: no
slot change, no reorder. Depth still resolves everything else (the depth pass
4 units up: a surface drawn later never covers a decal; anything nearer
hides it). Known risk, not seen in the three saves: a painter-only
translucent face behind a decal on screen is now drawn after it and over it
(water is depth-tested, D17AT3, so it is not affected).
`DNTTK_DECAL_SLOT=1` restores distance slots, `0` the game's slots.

Tried and kept off: a ranked depth rule (each blob lifted 4 units plus a
fixed rank from its object, 16 levels 0.125 apart; one depth-tested pass,
game slots), viable only because D17AT11 removed the plane jitter; it
measured slightly worse (about 180). `DNTTK_DECAL_RANK=1` enables it.

Checks: `ttk-near-test PASS` (contract back to the two-pass rule);
fast turns both ways at 180 Hz, no dropouts; slots 7, 11 and 12: every face
taken, none native or dropped, no replay failures. Python suite as before.
No framework change. Executable SHA-256
`259cd504454f885d3b09f38ce6ccd83c7ac486a91149113cd9b5e63f1c15f565`.

## Remaining

- Player playtest at the player's frame rate (180): slots 12, 11 and 7, the
  pools from the original spot and walking over them, and water views
  (moat, canal bridge).
- The real sprite queue (`0x8009b1c0`, 11 callers) is still drawn by the game
  in painter order; nothing in these three cases uses it. If a remaining
  flicker turns out to be one of its sprites, it needs its own rule
  (camera-facing quads; plane depth would cut them).

## Acceptance (2026-11-01)

User, after D17AT12: "I want to now accept this this is 100% acceptable it's
exactly working fine ... it's not perfect"; "slot 7 does not exhibit any
issues anymore however in slot 11 and 12 you can see a very very slight
shimmering"; "this is literally the best it's ever looked it looks better
than the base game". D17AT is Done. The remaining very slight shimmer in
slots 11 and 12 (`research/screencaps/Video_2026-11-01_14-22-44.mp4`) is the
new job D17AU, which starts from the findings above.
