# D17BB - Other rooms' wall pieces showing over the castle walls: intake report

2026-11-02. Status: Needs playtest. The user chose option A; the
implementation is at the end of this note.

Evidence is local under `recomp/analysis/d17bb/` (`look.py`, `dump.py`,
`rooms.py`, `objs.py`, `seen.py`, `sheet.py`; shots, RAM dump, `evidence.png`). Every run
used a private copy of the player's card and savestates (UI slot 12 = file
slot 11, saved 2026-11-02 21:07, level 6), offscreen video, dummy audio and
private debug ports. Each script hash-checks the player's saves and profile
before and after the run.

## What the user sees

Load UI slot 12: Duke stands in the castle courtyard (room 8, a sky room).
Above the far battlements are shapes that do not belong there: a thin spire,
a tilted block that looks like a merlon seen from an odd angle, small spikes
between the merlons and a dark roof stub (`evidence.png`, top row). They read
as pieces of castle wall drawn in the wrong place.

## Cause (confirmed)

They are real level geometry from **another room**: the moat outside the
wall.

- **Who owns the pixels.** `rooms.py` rebuilds every room's world pieces from
  a RAM dump (vertex = room min + (1024 b0, 128 b1, 1024 b2), GT3/GT4
  groups, back faces culled) and renders a room-ID image from the dumped
  camera. It matches the shot edge for edge. The spire and the tilted block
  are room 2 pieces 6 and 1 (the moat south of the castle, also a sky
  room); the slab at the top right is room 4 piece 1 (`evidence.png`, bottom
  row). The only placed objects in view are two in room 8 near the floor, so
  objects are not involved.
- **What those pieces are.** Room 2 pieces 1 and 6 span Z 8192-11264 and
  Y -2048..5120: the outer, moat-side skin of the south wall, with its own
  battlements and spire. The courtyard side of the same wall belongs to room
  8 (Z 12288 onwards). Each side was built to be seen only from its own room,
  and the two skins do not line up: the outer one is taller in places and its
  merlons sit elsewhere. Seen from the courtyard, only their tops and inner
  sides are front-facing, hence the "tilted" look.
- **Why they are drawn.** The game's portal walk lists only room 8 from this
  view (no portal leads into room 2). Polymorph's host visibility (D17AR) also
  runs the over-the-walls test inherited from D17AQ: from a sky room, any
  other sky room whose piece tops can be seen above the camera room's wall
  tops is added. The test is coarse: it compares bounding boxes only at the
  point where the sight line crosses the camera room's edge. Run offline on
  the dump (`seen.py`), room 2 piece 1's box top (Y -2048) passes over a low
  room-8 section there (piece 1, top Y 1024), although the real wall a little
  further in hides most of it. Where no room-8 piece stands at the crossing
  point at all, the test counts the edge as open: that is how 10 of room 3's
  11 pieces pass. Room 2 is then drawn through the rectangle of its seen
  pieces, which takes in the spire piece too. Host depth hides the parts
  behind the inner wall; what rises above it shows.
- **Proof.** Same save, same view, `DNTTK_HOST_VISIBILITY=0` plus
  `DNTTK_OPEN_SKY=0` (the game's own room list): the wall top is clean
  (`evidence.png`, middle row). `hostvis off` alone looks the same as
  Polymorph, because Classic's D17AQ open-sky rule admits room 2 the same
  way (Classic is frozen; noted only).

So this is not a draw-distance problem and not a broken model. The original
designers relied on the portal walk to never draw the moat from inside the
courtyard; our over-the-walls rule, written for jetpack views (D17AQ, slot
10), also fires on the ground here.

## Courtyard turn (16 headings)

The host list adds moat rooms 2, 3 and 4 at most headings and room 22 at two.
Room 28 (castle interior, not a sky room) comes in through a real portal and
is not part of this problem.

## Options

All are Polymorph only (Modernized, Extended draw distance); Vanilla is
unchanged and Classic stays frozen.

**A. Look over walls only when the eye is above them (recommended first).**
The over-the-walls test exists for raised views: jetpack, ramparts, towers.
Gate it on height: a sky room is offered only when the eye is above the top
of the camera room's own walls between it and that room (with hysteresis so
nothing pops while climbing). On the courtyard floor the eye (Y 5347) is
thousands of units below the wall tops (Y -2048), so the frame is the
designers' portal view. On the battlements or on the jetpack the outer side
really is in sight and the rule still applies. Game-wide, computed from the
level's own data, cheap, no files. Small code change in
`host_visibility.inc`; most of the work is the sweep.

**B. Shared-wall skins stay hidden from inside.** In an over-wall room, skip
the pieces that sit against the camera room's boundary (within a wall
thickness, facing it): they are the far skin of a wall the camera room
already draws. Complements A for raised views where the outer skin still
pokes above the inner one. Needs care for genuine neighbours (a tower
standing next to the wall).

**C. A visibility rules file (the user's idea).** Per level, a small list of
room-pair rules such as "from room 8, never show room 2 unless the eye is
above Y" or "always show room N", loaded at level start, with a developer
console command to toggle a pair live while looking and write the file. It is
our own data (room numbers only, no retail content), so it can be tracked.
Best as the override layer for whatever A and B miss, not as the main fix:
21 levels with up to 88 rooms each would take a lot of hand authoring.

**D. A precomputed visibility set.** Offline, sample eye positions over
each room's walkable floor and record which rooms can really be seen
(Quake-style PVS), then restrict the over-the-walls test to those pairs.
Most automatic and exact for the ground, but a large tool for a problem A
mostly removes. Not recommended now.

Recommendation: A, then B if raised views still show skins, with C kept as
the escape hatch for single cases. The job is smaller than it looks: the
renderer already decides room by room, and the cause is one rule firing where
it was not meant to.

## Acceptance (proposed)

1. Slot 12: no foreign wall pieces above the courtyard walls at any heading,
   first and third person, FOV 90 and 120, 180 Hz.
2. Slot 10 jetpack vista and slot 2 moat: what D17AQ/D17AR/D17AX fixed stays
   fixed (no pop-in, no lost towers or banks).
3. Sweep of the outdoor levels (1, 5, 6, 8, 10, 11, 21, 28) at ground level
   and from raised spots: every remaining over-the-walls admission listed and
   checked; no regression in the other levels.
4. Polymorph frame rate holds; Classic and Vanilla unchanged.

## Implementation: option A (2026-11-02, Needs playtest)

User: "A it is. let's go for it".

**Rule.** `open_sky_clear` (`draw_distance.inc`) takes an eye slack. Polymorph's
host selection (`host_vis_select`) passes it; Classic's open-sky caller does
not and is unchanged. A sight line to an over-wall room's piece now also needs
the **eye** to be no more than the slack (1024 units, about one storey) below
the camera room's wall top where the line leaves the room. Where no piece
stands at that point, the highest camera-room piece within 1024 of it counts;
a target inside the camera room's footprint is gated by the pieces around the
target. With no camera-room piece near either, the gate does not apply.

The slack came from the views: on the courtyard floor the eye is 3300 units
or more below the wall top at every crossing, while in the slot 10 moat
trench, where the castle over the rim is wanted, it is about 160 below. A
strict "eye above the wall" rule would have removed that view.

**Developer.** `DNTTK_OVER_WALLS_EYE=<slack>` (default 1024, `-1` off).
Console `hostvis eye <slack>|default` overrides it in the main process (for
60 Hz A/B tests; redraw workers keep the environment value). Debug object
`draw_distance.host_visibility.eye_gated` counts gated sight lines.

No framework change. No new state across frames, so redraw workers and the
live frame decide alike.

### Evidence

- **Slot 12 courtyard.** The saved view matches the game's own room list
  (`shots/a-still/cmp.png`); a full turn admits only room 8 and its portal
  neighbours (rooms 2, 3, 4 and 22 no longer appear). Clean battlements at
  180 Hz first person, third person and FOV 120 (`shots/a-180-all.png`,
  `shots/a-tops.png`). Gate off (`DNTTK_OVER_WALLS_EYE=-1`) still admits
  rooms 2, 3, 4 and 22.
- **Slot 10.** The D17AX jetpack-hover fixture: room lists equal at matching
  headings with the gate on and off. The current slot 10 save (moat
  trench): identical list and pixel-identical frame.
- **Slot 2 moat.** Gate removes rooms 8 and 22 (inside the castle); the
  frame is pixel-identical (they were hidden).
- **Ground sweep, all 21 selectable levels, 8 headings each** (`sweep.py`,
  gate on and off in the same process and view, 168 views): rooms gated in
  20 views (levels 0, 1, 5, 11, 28). Their pixel difference is at most
  0.54%, all sky animation, a moving enemy or fire flicker
  (`gated-views.png`); no geometry lost. Larger differences occur only in
  views whose room lists are equal (animation).
- **Jetpack sweep** (levels 1, 5, 6, 10, 11, 21, hover about 1000 units up,
  48 views): gating in levels 1, 5 and 11, in narrow streets where the gated
  rooms lie behind tall walls; side-by-side views show the same scenery
  (`jet-gated.png`).
- `ttk-controls-test` (LEVEL00 fixture), `ttk-near-test` and
  `ttk-input-test` pass. Player saves and profile hash-checked unchanged in
  every run.

Executable `dfd2cd8751ccc0fef29004a704f5dc05c5ebb5dad50e5d49ed54a3ebeec33028`.

### Limits

- A room hidden by the gate appears when the eye rises to within 1024 of the
  wall top (climbing a ladder or stairs, lifting off). What it reveals is
  then genuinely over the wall, but it can show as a pop at that height.
- Raised views over a shared wall (on the ramparts) can still show the outer
  skin where it rises above the inner one; that is option B, not done.
- Not yet played by the user at 180 Hz on the real display.

## Playtest round 1 (2026-11-02): region rule tried and withdrawn

User: "an AMAZING start". New saves: slot 12 on top of a stable (eye
Y 2324, near wall-top height), slot 11 moatside. From the stable roof the gate
lets other courtyards (rooms 12, 16, 18-21) over the walls, which show as
chopped wall fragments; from the moat it lets the courtyards (8, 12, 16,
19, 20) over the outer wall, whose wall tops show as tips. With
over-the-walls rooms off, both views are pixel-identical to the game's own
room list.

The user then asked to keep the view beyond the walls but not the castle
peaks. A second rule was built and measured: over-the-walls rooms only from
the camera room's open-air region (sky rooms joined through the level's own
portals). Level 6 splits into the moat ring (rooms 1-4, 18-21) and separate
courtyards (8; 12 and 16; 22). Both saves then matched the game's list
exactly, and the jetpack hover kept the moat, banks and outer walls, losing
only courtyard wall tops behind the front wall (`hover-region.png`).

User after seeing it: "this version is extremely stable, but it just isnt
it ... something about making this area feel grand when you can see some of
the castles over the wall. let's just go back one version". The region rule
was removed and option A restored; the rebuilt executable is byte-identical
to the option A build above
(`dfd2cd8751ccc0fef29004a704f5dc05c5ebb5dad50e5d49ed54a3ebeec33028`), so its
tests and sweeps stand. The region rule's partial ground sweep (56 of 168
views) was stopped. The stable-roof chopping and the moatside tips remain
known with option A; they are the cost of seeing castle parts over the
walls from near wall-top height.

## Round 2 (2026-11-02): region rule restored, milestone

User, after seeing option A again: "can you now go forward to the version
we were just on. i want to see it one more time", then "i think we can work
with this". The region rule is back (source above; option A's gate code is
removed again, `draw_distance.inc` is unchanged from before D17BB).
Executable `f52724f03a01672b6b9abf00a2c2668562bf6111db220ae2b2b373801691bcc3`. Check after the rebuild: slots 12 and 11 pixel-identical
to the game's own room list (as in round 1).

Developer: `DNTTK_OVER_WALLS=0` (none), `all` (every sky room, the
pre-D17BB behaviour), default region; console
`hostvis walls off|region|all|default` (main process only). Debug object
`draw_distance.host_visibility.region_skipped` counts skipped rooms. Round 1
measured: jetpack hover over the moat keeps the moat, banks and outer walls;
only courtyard wall tops behind the front wall go (`hover-region.png`).
`ttk-controls-test`, `ttk-near-test` and `ttk-input-test` passed on the
region build. Not done for the region rule: the full 21-level sweep (stopped
at 56 of 168 views with no lost geometry in those).

### Missing piece from the stable roof (new slot 12, 2026-11-02 22:32)

From the stable roof (room 8, eye Y 2368) a V-shaped notch in the far wall
shows the black mountain backdrop (`missing-zoom.png`). It is the south
wall continuing into the neighbouring courtyard, room 22 pieces 0 and 3
(their tops at Y -2048). Room 22 comes through a portal (via room 23), so
only its pieces inside that portal's screen rectangle are drawn; the wall top
above room 8's wall lies outside it. The game's own room list shows the same
notch; the PlayStation camera never looked from there.

The same mechanism, reversed, made the moatside tips: walls of a room behind
the camera room's wall. Pieces are about 5000 units deep, so a per-piece
rule cannot tell "the wall continuing" from "the back skin of the wall in
front". Next step (user chose it): option 1, per-face back-skin hiding (faces
in the narrow gap directly behind the camera room's own wall stay hidden;
everything else of nearby rooms may show over the walls).
