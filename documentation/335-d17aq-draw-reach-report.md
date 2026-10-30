# D17AQ - Draw reach report: jetpack-height pop-in, moat far end, enemy spawns

2026-10-30. Report only (job acceptance is the user's approval of a proposed
fix). No player-facing code changed. A developer-only experiment was built
locally, measured and removed again; the player executable is byte-identical
to the one before the job (SHA-256 prefix `92acbe4b8c61396b`).

Evidence is local under `recomp/analysis/d17aq/` (scripts `pan.py`,
`climb.py`, `vanilla.py`, `rooms.py`, `actors.py`, `spawn.py`, `perf.py`,
`mdis.py`; shots and JSON). Every run used private copies of the player's
card and savestates (UI slot 10 = file slot 09, saved 2026-10-30 12:24; UI
slot 2 = file slot 01), offscreen video, dummy audio and private debug ports;
each script hash-checks the player's saves and profile before and after.
Both slots are level 6 (the castle moat).

## Summary

| Report item | Cause | Original game? |
| --- | --- | --- |
| Slot 10: level pieces pop in and out while turning at jetpack height | Room/portal visibility: the eye is above the moat walls but the game only draws neighbouring rooms seen through their portals, clipped to the portal's screen rectangle | Yes. Vanilla reaches the same height and shows the same black void beside the castle |
| Slot 10: large black areas that never fill in | No geometry is authored there: the moat floor and walls end in empty space below the sky backdrop | Yes (authored) |
| Slot 2: moat far end completely black | Room far limit: the far end lies about 50000-63000 units away, past the Extended limit 51104 | Original limit is 25552, so the original shows even less |
| Enemies appear only as Duke gets close | Spawn/despawn radius is the original mesh far limit 25552, set at level load; Extended draw distance now shows twice that range | By design: the original spawns enemies just inside its own fog edge |

Not FOV-related: the same rooms drop out at FOV 60, FOV 90 16:9 and FOV 90
4:3 (only the yaw at which they leave changes slightly).

## 1. Slot 10 at jetpack height

The save has Duke hovering with the jetpack at Y -1898 (Y grows downward),
in room 1, a sky room whose bounding box reaches Y -2048. The moat walls
around him top out near Y 1000: a descending sweep (`climb.py`) shows the
wall top entering the view only below about Y 1000. From the save's height
Duke therefore looks over every wall.

The original visibility is a portal walk (`0x8006276c`): starting from the
camera room it lists each room seen through a portal, with the integer screen
rectangle of that opening (list `0x800d68b0`, count `0x800d68a8`). The room
walk (`0x80062b48`) then draws each listed room clipped to its rectangle.
Room 1's portals are full-height openings into rooms 2 and 21 (the moat
continuing left and right) and a low gate into room 0. From above:

- Geometry of other sky rooms seen over a wall, not through a portal, is never
  listed. Rooms 8, 12, 16, 18, 19, 20 and 3/4 (all sky rooms bordering the
  moat) were absent from the list in most headings.
- Rooms reached through a portal are clipped to that portal's rectangle, so
  the castle front is cut along a vertical line mid-screen and its
  continuation in room 21 appears only inside room 21's rectangle.
- Turning moves each portal in and out of the view, so whole rooms drop out
  (rooms 2 and 29 at yaw about 0.8-1.0 rad, everything but room 1 by yaw
  about 1.1).

The existing D17Y rule (`open_upper_portal`) opens only the top edge of a
portal into a taller sky room; here source and target share the same top, so
it does not apply.

**Vanilla comparison.** Loading the same state with `--vanilla` and holding
the original jump button keeps Duke at the ceiling (player Y -1723, camera Y
-2173). The third-person views show the same black void beside the castle,
with the castle ending in a hard edge against it. The narrower view and the camera behind Duke
keep most of it off screen; Modernized first person at the eye, a wider FOV
and free hover-turning expose it.

**Experiment (developer-only, removed).** A local hook appended every sky
room to the list with the full view rectangle whenever the camera room is a
sky room (`DNTTK_D17AQ_SKY_ALL=1`). The castle front, both moat banks and the
neighbouring bastions then stayed whole and stable while turning; nothing
popped. What remained black is the authored void: the moat's outer floor
edges end in empty space below the sky backdrop's mountain line.

Cost at 120 Hz, slow turn, two runs each (current settings, 16:9, FOV 90):

| Run | Emulation thread | Presents/s | Host geometry ring peak (of 139744) |
| --- | --- | --- | --- |
| Current | 0.667 / 0.662 | 120.2 / 120.2 | 64540 |
| All 12 sky rooms listed | 0.816 / 0.816 | 120.0 / 120.0 | 115872 |

Side effect seen: with the experiment, Duke's hover height settled at -1829
instead of -1898 in every run, so something in gameplay reads the visible
list. A real fix must not change the list the game itself uses.

## 2. Slot 2 moat far end

Duke stands in room 3 at X about -23300 looking along the moat (rooms 3, 2, 1,
21, 20). Room 20 starts about 50000 units away and its far wall about 63000.
The Extended room/mesh limit is 51104 (the original 25552 doubled, D17P), so
the end is not drawn and the cleared background shows. With the developer
override `DNTTK_DRAW_DISTANCE_FAR=65535` (the GTE's 16-bit depth maximum)
the moat's real end wall appears; some black remains above it, the same
authored void as in slot 10.

Cost at 120 Hz in slot 2 (the heaviest D23G scene), after a 6-9 s settle,
four runs each: emulation thread 0.67-0.74 current versus 0.70-0.74 at 65535,
host ring peak 104900 versus 106260. The difference is within run-to-run
noise. Presents/s dipped to 98-110 in some runs of both configurations; that
is the open D23G slot-2 pacing item, not this limit.

## 3. Enemy spawn distance

Disassembly of the owned executable:

- Level load (`0x80023984`/`0x80023990`) stores the level fade start in
  `0x800da450` and fade start + `0x13d0` in `0x800da474`: the original mesh
  far limit.
- Spawner setup (`0x8009934c`) squares `0x800da474` into `0x800dd9e0`
  (levels 27-30 use an effectively unlimited value), resets the live count
  `0x800c56a4` and loads a per-level cap of live spawned enemies into
  `0x800c56a8`.
- `0x80099164` returns "too far" when the squared distance from Duke to a
  spawner exceeds that radius (three spawner types are exempt).
  `0x800991bc` and `0x8009926c` spawn (`0x80099048`) only when in range and
  under the cap. `0x800992fc` removes a spawned enemy (`0x80098d70`) once it
  is out of range, unless its flag bit 1 is set.

Live values (both slots, level 6): radius 25552, 51 spawners, cap 12, 3-4
live at load. Only four targetable enemies existed at load, none in the moat.

So the original spawns and removes enemies at the edge of its own visible
range, where the fog hid them. Extended draw distance keeps the radius
(D17P deliberately left the level globals alone because gameplay reads them)
but draws rooms to 51104, so Necros appear in the open at 25552 and would
also vanish there when Duke moves away.

## Proposed fixes (for approval; nothing implemented)

All three are Modernized with Extended draw distance only. Original draw
distance and Vanilla stay unchanged. Each is game-wide, keyed to room flags
and level globals, never to a level, slot or coordinate.

**A. Open-sky visibility (slot 10).** When the camera room is a sky room,
draw the other sky rooms whose bounding boxes are in the view frustum and
within the far limit, and give sky-room entries the full view rectangle.
Make it render-only: build the extended list for the room and props/actors
passes and restore the game's own list afterwards, so gameplay (and the hover
height) keeps the original. Host depth from the unified path (D17AM) keeps
far rooms behind near walls. Frustum and distance culling should bring the
cost well below the all-rooms experiment (+0.15 thread). The geometry ring
headroom must be checked (the experiment peaked at 83% of the ring). The
authored void stays black: filling it would need new art, so it is out of
scope unless you want a separate job (for example, extending the sky
backdrop below the horizon).

**B. Longer Extended limit (slot 2).** Raise the Extended room/mesh limit
from double the original (51104 here) to the GTE maximum 65535 in every
level, with the fade start moved by the same amount. It cannot go further
on the current projection path. Measured cost in slot 2 is within noise.

**C. Enemy spawn radius to match (enemies).** Two options:

- C1 (recommended): with Extended draw distance, spawn at the Extended limit
  and despawn a little beyond it (hysteresis), so enemies are already in
  place when their room comes into view and do not vanish on screen. This
  changes gameplay: enemies spawn and start reacting earlier, and with the
  per-level cap (12 in level 6) distant enemies can occupy slots that nearer
  spawners would otherwise use. CPU cost is bounded by the cap. Needs a
  playtest across levels.
- C2: keep the original radius (gameplay identical) and fade newly spawned
  enemies in from the fog colour over their first second. This hides the pop
  but leaves enemies absent beyond 25552 in long open views.

Suggested order: B (small), then A, then C. A and C each need playtesting.

## Implementation (2026-10-30, user approved B, A, C1 in that order)

User: "proceed with your exact recommendation ... we really need to open her
up". All three are Modernized with Extended draw distance only
(`recomp/src/ttk/draw_distance.inc`); Original and Vanilla are unchanged.

**B. Maximum limit.** Extended now sets the room/mesh limit to 0xffff in
every level (D17P doubled it). The fade start moves by the same amount, and
the OT depth limit becomes 16383, inside the mesh guards. The moat's far end
is drawn in slot 2.

**A. Open sky, gated by sight over the walls.** The room-walk hook
(`0x80062b48`, after the D17P/D17Y portal widening) runs only while the
camera room is a sky room. A sky room is opened to the whole view rectangle
(when listed) or appended (when in front of the camera, on screen and inside
the room far limit) only when Duke can see over the walls. The test uses the
mesh-piece bounds the room walk already uses (room +0x32 count, +0x40
pointers, piece +4 int16 min and +0xc max, relative to the room's minimum
corner). A sight line from the eye to the top of one of the room's pieces
must leave the camera room above the highest camera-room piece at that point
of its edge (or where none stands). Without that test, open sky also listed
rooms from the ground that the walls hide, at about 2 game frames/s in slot 2
for no visible change. The list stops at 31 entries (the walk's own limit is
32). Only the camera render's three passes read the list; the portal walk's
room bitmap is untouched.

**C1. Spawner range (withdrawn after playtest; see Revision 2).** New entry hook `0x80099164` (added to
`game.local.toml`, regenerated; codegen hash unchanged, existing savestates
load). For the spawn callers (ra `0x800991ec`, `0x80099288`) the range is the
extended mesh limit; for removal (ra `0x80099310`) it is 4096 more. The hook
computes the exact distance (the original saturates once an axis differs by
32768 or more) and answers by pointing a0 at the spawner's own position or
at a far point in enhancement memory. No game variable is written, so a
savestate taken mid-check holds nothing of ours. Exempt types 0x3d, 0x385 and
0x3e and every other caller are left to the original. The live cap is
unchanged.

Developer A/B: `DNTTK_OPEN_SKY=0`, `DNTTK_SPAWN_RANGE=0`,
`DNTTK_DRAW_DISTANCE_FAR=51104` (the previous doubled limit in level 6). The
controls report's `draw_distance` object adds `open_sky` (frames, opened,
added) and `spawn` (in, out, kept, removed, refusals, live, cap).

### Verification

- Build passes. `ttk-controls-test` passes with the owned EXE, both clean
  LEVEL00 fixtures, LEVEL01 and all level overlays (117 PASS lines). New
  groups cover open sky (above the wall, on the ground, behind the camera,
  indoors, non-sky room, refused limit, portal-listed room opened only when
  seen, caller and Vanilla) and the spawner range (extended reach, removal
  hysteresis, exempt types, other callers, no write to the squared radius).
  `ttk-near-test` and `ttk-input-test` pass, as do the profile tests (43).
- Slot 10 at hover: the castle front, both moat banks and neighbouring yards
  stay whole while turning. The gated list matched the ungated experiment in
  every heading.
- Slot 2 on the ground: the open-sky gate adds only rooms 8 and 22 (castle
  towers above the moat walls) and the moat end is drawn.
- Enemies, slot 2 walking along the moat with god mode: the original keeps
  4-7 alive and creates more at about 25000 units. With C1 all 12 (the
  level's cap) are alive at once, the nearest at 15300, the rest out to
  35000, and close in on Duke.
- 12-slot sweep (private copies, new vs old, short turns): no lost geometry
  beyond animation and timing differences (at most 0.9% outside slots 2 and
  10, where the view gains the far end and the yards).
- Vanilla, slots 2 and 10: original limits (25552), zero open-sky and spawn
  activity.
- Player saves and profile hash-checked unchanged in every run.

### Cost (120 Hz, CPU 150%, private profile copy, slow turns)

| Scene | Before (game frames/s, thread) | After |
| --- | --- | --- |
| Slot 10, hover (open sky showing 4-8 more rooms) | 30.0, 0.65 | 22.0, 0.80 |
| Slot 2, ground (limit, enemies, gate) | 30.2, 0.66 | 27.9-28.2, 0.71-0.78 |

Presents stay at 120/s (frame replay), apart from the known slot-2 load-time
dip in either build. In slot 2 the parts are about 1.3 frames/s for the limit,
1 for the 12 live enemies and 0.7 for the gated open sky. The open-sky rooms
are drawn by the game's own room walk, and CPU 200% only reaches 24 frames/s
at hover because the emulation thread then falls behind real time. Recovering
that belongs to the remaining D23G steps (dispatch and emulation-thread
cost). Fitting tighter rectangles to the visible pieces is a possible
follow-up.

Limits: the authored void (no geometry beyond the moat's outer edges) stays
black. Rooms past 65535 cannot be drawn on this projection path. With 12
enemies alive from the start, nearer spawners wait for a free slot; raising
each level's cap would be a separate gameplay decision.

## Revision 2 (2026-10-30): C1 withdrawn, dormant enemies instead

User playtest: "the flow of the game has been altered quite considerably with
the different enemy reactions ... In Duke3D ... they were just present and
they didnt react at all until you were in proximity ... If you shoot them, it
wakes them up". C1 was removed (hook, tests, notes) and replaced by dormant
enemies (`recomp/src/ttk/enemy_reach.inc`), Modernized Extended only. B and A
are unchanged.

Original mechanism, read from the owned executable:

- Spawner tick `0x80099530` (called from the world update `0x80025ac8` with
  Duke's position, single player only) visits four spawners a frame. Records
  are at `*0x800de720`, 0x30 bytes: +0 type, +4 actor, +8 position, +0x14
  flags (bit 0 spawned; 0x16 skip). A spawned record goes to the removal check
  `0x800992fc`, an empty one to the spawn check `0x8009926c`. When the cap is
  full, that check replaces the farthest spawned enemy (`0x80099048`) if the
  new spawner is nearer. Creation is `0x80098f3c`; a failed creation sets flag
  0x10, which disables the spawner for good.
- The removal handler saves the enemy's flags and position back into its
  record, so the next spawn starts where it left off.
- Actor loop (same world update): for each actor in the lists `*0x800c5694`
  and `*0x800c5698` it runs the prep `0x80025944`, then
  `handlers[actor+0x15](actor)`, table `*0x800dd850`. Byte +0x15 is the
  class, copied at creation from byte 6 of the 28-byte type entry
  (`*0x800d2660`). Class 0 is the empty handler `0x800632b8`; moat enemies
  are class 6 (controller `0x80065040`) or 7. Health is int16 +0x32.

Design:

- Range: the range test `0x80099164` uses the exact distance against the
  extended mesh limit to spawn and that plus 4096 to remove. It answers by
  pointing a0 at the record's own position or at a far point, so no game
  variable is written. The cap, the live count, the nearest-first replacement
  and the creation path are the original's, so no more enemies exist than the
  original allows and actor memory stays within its design.
- Dormant: a spawned enemy farther than the original radius (`0x800dd9e0`)
  that has not lost health since it was spawned. The tick hook decides this
  once per frame. The prep hook gives a dormant enemy class 0 and the empty
  handler's hook restores it immediately, so its controller does not run
  (no movement, sound, sight or attack) and nothing else sees the swap.
  `0x8006cefc`, right after the loop, restores any left over. On the first
  tick after a savestate load, a spawned enemy found at class 0 gets its
  class back from the type table (a save taken inside that short window).
- Wake: within the original radius it is simply the original enemy. A health
  drop (shot, kicked, caught in a blast) wakes it for good.
- When the cap is full and a nearer spawner comes in range, the original
  replacement removes the farthest enemy, now a dormant distant one. Near Duke
  the same enemies spawn and act as in the original.

New entry hooks (regenerated, codegen hash unchanged, savestates load):
`0x80099164`, `0x80099530`, `0x80025944`, `0x800632b8`, `0x8006cefc`.
Developer A/B: `DNTTK_ENEMY_REACH=0`. The controls report's `draw_distance`
object has `enemies` (spawn_in/out, kept, removed, refusals, held, woken,
healed, spawned, dormant, live, cap).

Verification:

- `ttk-controls-test` passes with both clean LEVEL00 fixtures, LEVEL01 and
  all level overlays (117 PASS lines). The dormant group covers range and
  hysteresis, no radius write, exempt types and callers, hold and release,
  release after the loop, hurt wake, a new enemy in the same record, the
  post-load heal (only after a load) and the empty spawner table.
  `ttk-near-test`, `ttk-input-test` and the profile tests pass.
- Slot 2 walking the moat (god mode): 12 spawned (cap 12), 8 dormant. Every
  enemy beyond 25552 stayed exactly in place, and nearer ones approached as
  in the original. Spawner 0 woke at 24404 and moved. As Duke advanced,
  dormant distant enemies were replaced by nearer spawners.
- Forced hurt (health -1 on the farthest dormant enemy, 36401 away): it woke
  at once, kept its class (7) and moved 2156 units in 6 s; the other 7
  dormant enemies did not move.
- 12-slot sweep (levels 0, 1, 2, 3, 6, 7): live counts never exceed the cap
  (10, 12 or 20), and creation was never refused. A 40-sample check in slots
  1, 2, 10 and 12 found no enemy left at class 0 (one earlier sighting was an
  unsynchronised debug read inside the hold window).
- Cost, slot 2, 120 Hz, CPU 150%: 29.0-29.3 game frames/s without dormant
  enemies, 28.9 with them (their controllers do not run).
- Vanilla, slots 2 and 10: original limits and zero range/hold activity.
- Slot 12 (file slot 11) loads in about 21 ms and runs at 30 game frames/s
  (the user later corrected the report to slot 10; see Revision 3).

Limits: a dormant enemy may hold its last pose, as its controller drives
its behavior (not checked close up in game). Enemies past 65535 do not exist. Dormancy
covers spawner enemies; enemies placed directly in a level (without a
spawner) keep the original behavior.

## Revision 3 (2026-10-30): slot 10 slowdown

User: the slow slot was 10, not 12, "the issue still persists". The user's
16:52 session log shows the slot 10 savestate applied in 21 ms; the slowness
is the game running slowly after the load. Host checks found no contention.
With the player's settings (4x, 16:9, 120 Hz, CPU 150%) hovering in slot 10:
30.0 game frames/s with D17AQ off, 23.6 with it, 30.1 with only open sky off
(dormant enemies 24.3 vs 23.6 and the 65535 limit cost little). Fewer game
frames make the game itself run slower, so the jetpack view ran about 20%
slow.

Three changes:

1. Each added or widened sky room gets the screen rectangle of its pieces
   seen over the walls (projected piece bounds, 8-pixel margin), not the
   whole view, so the room walk's piece test skips the rest: 23.6 to 25.0.
2. `near_clip.cpp`: when the unified path draws every face of a world mesh
   piece, the original routine `0x80011020` gets a null mesh and returns at
   once instead of transforming every vertex of an empty copy (its only
   other effect, the colour-index clear, is already applied to the real
   mesh). This applies to every scene: 25.0 to 26.1 here.
3. The object pass `0x800374cc` (new entry hook, regenerated) gets the
   portal walk's own list back. For the added rooms it drew nothing visible
   in matched views but cost 2 game frames/s: 26.1 to 28.1.
   Developer: `DNTTK_OPEN_SKY_OBJECTS=1` keeps the added rooms for it.

Final, player settings, slow turn: slot 10 hovering 28.1-28.2 game frames/s
(30.1 without D17AQ), presents about 115/s; slot 2 on the ground 30.1 in both.
The remaining hover cost is the GPU work of drawing the extra scenery (host
vertex store and triangle setup in the profile). CPU 175% gives 28.7 and the
emulation falls behind real time (57.7 fields/s), so 150% stays.

Verification: `ttk-controls-test` (117 PASS, both clean fixtures, all level
overlays; open-sky rectangle and object-pass assertions updated),
`ttk-near-test` (unified and `DNTTK_UNIFIED_GEOMETRY=0`; the world takeover
contract now expects the null mesh) pass. A 12-slot sweep of the new build
with D17AQ off against the previous build found at most 1.0% changed pixels,
weapon bob and animation only (slot 5 checked by eye). Slot 10 hover views
with and without the object-pass cut are identical at matching headings.
