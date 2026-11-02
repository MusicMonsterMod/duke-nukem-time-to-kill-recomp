# D17AZ - Objects that reach out of their room stay drawn

2026-11-02. Status: Done, accepted by the user 2026-11-02 ("excellent work").
Polymorph (host visibility) only;
Classic and Vanilla are unchanged.

Evidence is local under `recomp/analysis/d17az/` (`turn.py`, `objs.py`,
`sweep_objs.py`, `spill.py`, `perf.py`, `mips_dis.py`; shots, sheets, JSON).
Every run used a private copy of the player's card and savestates (UI slot 12 =
file slot 11), offscreen video, dummy audio and private debug ports. Each script
hash-checks the player's saves and profile before and after the run.

## Cause (confirmed)

The user's EDuke32 comparison was right: this is the "sprite owned by a culled
sector" case.

- **Owner.** The train is object `0x801d5d64`, type 77, renderer kind 1 (the
  static-object renderer `0x80031d10`), position (7399, -5784, 39932), local
  box x -9361..9404, bounding radius (+0x5a) 9573. It sits in room 82's object
  list (X 7168-14336). Duke stands in room 19 (X 14336-21504), about 1700 units
  from the room boundary. The train runs from room 82 into room 19, up to
  about X 16800.
- **The object pass is a draw loop over the portal list.** `0x800374cc`
  disassembled: for each entry of the portal walk's list (`0x800d68b0`) it
  walks the room's object list (+0x48; nodes +4 next, +8 object), its
  targetables (+0x4c) and the global list `*0x800c5698` (filtered by the
  object's room byte +0x2e), and calls the renderer table `0x800c0bf8` by kind
  (+0x14) with the camera, the object and the entry's rectangle. An object in
  an unlisted room is never visited.
- **The frame it vanishes.** Turning right from the save in steps of about
  0.025 rad, the game's list drops room 82 at step 9, the same frame the train
  disappears (`shots/base`). Until then room 82's rectangle was the whole view,
  so no per-object rectangle or radius cull was involved (cause 2 in the job
  ruled out). The host list keeps room 82 three steps longer (guard band). That
  does not help: D17AR still hands the object pass the game's own list.
- Cause 3 (far reserve or far limit) is not involved: the train is about
  1700-9000 units away and fails only on room membership.

## Change

`recomp/src/ttk/host_visibility.inc`, `host_vis_reach_draw`, called at the
object pass entry after the pass has the game's list back
(`host_vis_object_pass`; `draw_distance.inc` and `modern_controls.cpp` pass the
CPU through).

In a frame where host visibility owns the world (Modernized, Extended draw
distance, unified geometry, Polymorph), for each room the portal walk did
**not** list, each kind-1 object in its object list is drawn through
`0x80031d10` when:

1. its bounding sphere (position +4, radius +0x5a) reaches out of its own
   room's box;
2. that sphere overlaps the box of a room the host shows this frame;
3. the sphere's box passes the host's guard-banded view test (the same test
   and guard band as the world pieces).

At most 32 such draws are made per frame. The call uses the whole view
rectangle `0x800d2210` and runs on a copy of the CPU with the stack and
scratchpad restored, the method D35F1's pole draw uses. The renderer keeps its
own far, near, sphere and box tests, clipping, colours and depth, and host
depth keeps it behind nearer walls. Nothing else changes: the game's room
list, object membership, the props/actors pass and the object pass are
untouched, and the pass still draws listed rooms as before, so nothing is
drawn twice. Actors (kind 2) and the other kinds stay on the game's list, as
D17AQ/D17AR decided. Redraw workers run the same pass, so high-refresh frames
follow; transform interpolation is keyed by object and call index, so the
extra draws interpolate like any other object.

Guards: `0x800374cc` (object pass) and `0x80031d10` (renderer) were added to
the host visibility code guards, and the renderer table's kind-1 entry is
checked each frame. Developer switch `DNTTK_OBJECT_REACH=0` turns it off.
Counters: `draw_distance.host_visibility.reach` = [drawn, tested, drawn last
frame, selection microseconds].

## Evidence

Slot 12 (private copy), turn right 30 steps and back 30, `turn.py`:

| Run | Result |
| --- | --- |
| First person, FOV 90, 60 Hz | The train stays drawn and slides off the left edge. |
| First person, FOV 120, 180 Hz, fix off | The train vanishes at step 11 and returns at step 46 on the way back. |
| First person, FOV 120, 180 Hz | Stays drawn; its rear end leaves the edge gradually (last sliver at step 21, `shots/fp120/edge.png`); returns smoothly. |
| Third person, 180 Hz | Stays drawn and slides off the edge. |

Level sweep (`sweep_objs.py`, all 21 selectable levels, 4627 objects in
room lists): 1009 objects have a bounding sphere that reaches more than 256
units out of their room's box, and 542 of those reach into another room. Of
those 542, 491 are kind-1 static objects in room object lists, which this rule
covers. The remaining 51 are listed under Limitations.

Cost at 180 Hz with player-like settings, slow turn, 15 s (`perf.py`): game
frames 30.0/s and fields 60/s with the fix on and off in slot 12, slot 10 and
slot 2. The reach selection takes 30-67 us per game frame, with 0.6-2.3 extra
draws per frame. Presents per second follow governor steps (1360 or 2720)
independently of the switch, as repeat runs show.

Tests: `ttk-controls-test` with the LEVEL00 fixture
(`analysis/pc-input/d07-final-camera/level00-guard-fixture.bin`) exits 0;
`ttk-near-test` (default) and `ttk-input-test` pass. `ttk-near-test` with
`DNTTK_UNIFIED_GEOMETRY=0` aborts at a packet contract (line 572). That target
builds only `near_clip.cpp`, which this job did not change, so the abort is not
caused by it.

Executable `868256dc4ea4d963a20769d802442750a32fdcdf9139b22a9b8dd9aeb66fabf9`.
No framework change and no regenerated code (the object pass entry hook already
existed).

## Limitations

- Actors are not covered: 48 kind-2 and 2 kind-3 records (enemies and Duke,
  radius about 900-2400) whose sphere crosses into a neighbouring room. Their
  radius is much larger than their body, so most never show. One that really
  stands in a doorway can still pop when its room's opening leaves the view.
  Drawing actors outside the pass needs a check of the actor renderer
  `0x80032e78` for side effects first; that is a possible follow-up.
- One kind-1 object sits in a targetable list (+0x4c) and is not covered.
- The test uses the bounding sphere, not the rotated box. An object whose sphere
  reaches a shown room while its geometry does not is still culled by the
  renderer's own box test, so this costs a call but never shows wrongly.
- Not played by the user yet: real-time slot-12 turns at 180 Hz on the player's
  display; other levels' large props (bridges, vehicles, long pipes) were
  found by the sweep but checked only through this rule, not one by one in
  game.
