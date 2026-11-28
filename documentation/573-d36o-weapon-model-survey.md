# D36O - first-person weapon model survey and repair

2026-11-28. Status: **Done, user-accepted 2026-11-28** ("lock it in and close it out!! its working!!"). Board: `MODERNIZATION_JOBS.md` D36O,
[jobs/first-person.md](../jobs/first-person.md#d36o---first-person-weapon-model-repair).
Follows [note 572](572-d36n-viewmodel-motion.md) (D36N round 3 limited the
Energy Weapon's motion short of a vanishing face).

Evidence labels: **S** source read, **L** offscreen lab (private settings and
card copies, NVIDIA EGL, motion off unless stated), **U** user report.

## 1. The report

U (D36N rounds 2-3, and a new video 2026-11-28 13:00): part of the Energy
Weapon's housing, the outer right section down the barrel beside the green
panel, disappears while running and turning, and for a moment at the very
end of the idle gesture (in the video from 7.05 to 7.117 s the housing
beside the panel ends in a straight edge with the street behind it; at
7.133 s the triangle pops back). The user asked for "a real fix that would
allow us to view the weapon at any angle" and a review of every weapon.

## 2. Cause: a folded quad culled by its first half

S: the held weapon is drawn by the object renderer `0x80010000` through the
host (`near_clip.cpp` `object()`, every viewmodel face host-taken). A
one-sided face is dropped when it faces away; like the original's NCLIP, a
quad is judged by its first triangle (corners 0 1 2) alone.

L, lab switch `DNTTK_VIEWMODEL_CULL=both` (draw the held weapon two-sided)
and a log of culled faces: the vanishing section is one textured quad (FT4)
of the Energy Weapon's housing that is folded along its diagonal. A few
degrees from ready its first half (0 1 2) turns away while its other half
(1 3 2) still faces the eye, so the whole face was dropped and the street
showed through. The model itself is closed: its 38 faces share every edge
exactly twice with consistent winding. It was not a missing face but the
original's single-triangle test, which its own cameras never exposed.

D17BE had already met this on world props (the crushed car on TIME TO KILL's
street): unified scene meshes draw such a quad as its front half. That rule
excluded the viewmodel.

## 3. Fix (Modernized; the renderer, every weapon)

`near_clip.cpp`: a held-weapon quad whose first half faces away but whose
second half faces the eye is drawn as that front half (the D17BE rule, now
also for the viewmodel). Faces that face away whole stay culled; planar
faces are unchanged. Modernized only (`input_modernized()`); Vanilla has no
first-person weapon. Developer A/B: `DNTTK_VIEWMODEL_FOLDED=0` keeps the
original cull. The near report counts them (`viewmodel_folded`).

`viewmodel_motion.inc`: D36N's soft limit on the Energy Weapon's muzzle-down
pitch, muzzle-left yaw, top-left roll and rightward travel (round 3, a
safety net) is removed. Its D53G rule (the gun only moves down, so the
support shoulder stays out of view) is kept.

No model data changes, no new art.

## 4. Hole detector (lab)

`DNTTK_VIEWMODEL_CULL=mark` draws every held-weapon face the renderer still
culls as an untextured face, first in the viewmodel layer (before any other
weapon or arm face). Visible mark colour in the final picture therefore means
no front face covers that pixel: the background shows through the weapon's
silhouette. Painter-order noise is excluded by drawing them first (an earlier
pass that sorted them normally reported false holes). The colour encodes
the record: q = (face-list offset)/4, red 196+8*(q&7), green 4+8*((q>>3)&7),
blue 196+8*(q>>6), each centred in a 5-bit step so dithering keeps it.
`DNTTK_VIEWMODEL_CULL_LOG=1` prints the culled faces per mesh when they
change (`F`: its second half faces the eye). Lab scripts:
`recomp/analysis/d36o/` (local): `probe.py` cases `survey`, `gestures`,
`eseq2`, `posesweep`, `models`; `meshdump.py`, `orient.py`, `decode.py`.

## 5. Survey of every weapon

L, the street savestate, `dnkroz` and `dnstuff`, every weapon held still by
`fpidle pose`: 35 poses each (pitch, yaw and roll -15 to +15 in 5-degree
steps one axis at a time, the eight +-10 corners together, the motion
layer's eight travel corners x -9/4, y 0/20, z -5/5) and the three D53G
gestures (inspect, heft, rest). Hole pixels at 1024x576, with the original
cull and with the fix:

| Weapon (record, model) | Original cull, max | With the fix, max |
| --- | --- | --- |
| Throwing Axe (2, 115) | - | 0 |
| Throwing Knife (1, 122) | 0 | 0 |
| Desert Eagle (4, 121) | 0 | 0 |
| Combat Shotgun (5, 125) | 72 (pitch +10/+15) | 0 |
| Gatling Gun (7, 120) | 4 | 0 |
| RPG (8, 1080) | 88 (pitch +10) | 1 |
| Pipe Bomb (12, 123) | 0 | 0 |
| Holy Hand Grenade (13, 945) | 1 | 1 |
| Dynamite (14, 946) | 0 | 0 |
| Energy Weapon (10, 167) | 897 (most poses) | 0 |
| Flamethrower (9, 195) | 3 | 0 |
| Buffalo Rifle (6, 116) | 0 | 0 |
| Crossbow (3, 117) | - | 2,378 (also at ready) |
| Freezer (11, 974) | - | 837 (yaw +10/+15) |

(-: not reached in that run.) Values of 1-4 pixels are single edge pixels.
The upgrades (records 27, 28, 29) use the Flamethrower, Gatling and RPG
models (195, 120, 1080). The Mighty Boot has no held mesh. The Crossbow and
Axe models are resident on the street level; key 1 and key 9 reach them
with `dnstuff`.

Energy Weapon gestures, every second game update to the end (270 frames):
the original cull opens the housing in about 90 of them (up to 1,133 px),
including the closing frames of all three gestures (inspect 62-80, heft
63-66, rest 45-48); with the fix 0 in every frame.

Full motion, D36N limit removed (`eseq2`: motion on, running, a jump every
10 updates, hard turns both ways, looking up and down; 60 frames per
weapon): Energy Weapon 0 in every frame; every weapon 0-2 px except the
Crossbow (every frame, up to 1,763) and the Freezer (17 frames, up to 13).

### The two remaining weapons

- **Freezer: no defect.** The marked pixels are shaded, not the flat mark
  colour: they are the Freezer's translucent ice crystals (11 two-sided
  records) on both sides of its head, seen over the culled faces behind
  them. Drawn normally they are pale blue and see-through, as designed.
  The mesh has 8 open edges, all at those crystals.
- **Crossbow: an opening around the trigger.** Decoded faces 65, 62, 27, 6,
  63, 64 and 49 show through a wedge beside the trigger, and through a
  sliver along the top edge of the stock. The mesh has no open edges (8
  edges are shared by more than two faces: thin plates modelled as two
  back-to-back sides) and those faces are wound consistently. Drawn
  normally the wedge reads as a trigger window with the ground behind it.
  Not changed; whether it should be closed is the user's call (an
  authored face would follow note 265).

## 6. Round 2 (2026-11-28): the corner above Duke's right hand

U, after the fix (video 15:20, walking down the street and turning): "it
flick[s] on and off almost constantly"; "a small triangle ... at the corner,
above the top left of Duke's right hand". In the video the inner corner of
the white bracket beside the glove changes from one 60 fps frame to the
next.

L: the hole detector stays at 0 there (fast turns, walking and turning, 60
frames each): no face is culled. The viewmodel had no depth test; it was
painted farthest mean depth first, and the bracket and the brown housing
face beside it overlap on screen at almost the same mean depth, so their
order could change between the game's frames and the redraws (which re-run
the weapon at an in-between camera). With the gun pinned to the view
(motion and view bob off) while walking, that corner is the one patch of the
gun that changes between frames.

Change (Modernized, `near_clip.cpp` `emit`): the held weapon and arms are
depth-tested among themselves. Their view depth is compressed into a thin
slice just beyond the depth curve's 16 (16 + z/10000; the renderer stores
1 - 16/z, so about 0.0003 to 0.003), nearer than the room: they stay in
front of it as before (checked against a tree and pressed flat to a wall)
and their own faces meet per pixel, the same in every frame. Translucent
faces test without writing depth; the kick leg stays painted first, behind
(D08A53), and the Bio Mask last, in front (D08A26). Developer A/B
`DNTTK_VIEWMODEL_DEPTH=0` (painter order alone).

L, rest pictures of every weapon: unchanged in silhouette; where faces had
overlapped wrongly they now meet as their depth says (the Buffalo's barrel
over the top of its fore-end, the Shotgun's support glove over the pump);
a plane-ordered painter sort tried first (Newell's tests) gave the same
Buffalo and Shotgun results and was dropped for the exact depth test. The
quick kick's leg is still drawn behind the gun and hands. The blink itself
was not measured away in the lab (walking changes the lighting of every
pixel); the user's playtest decides it.

## 7. Tests

- `ttk-near-test disc/SLUS_005.83`: new group "D36O held-weapon folded quad
  draws its front half and the weapon is depth-tested in its own slice
  (Modernized); back faces culled, flat faces, kick leg and Vanilla
  painter-only" (fails with `DNTTK_VIEWMODEL_FOLDED=0` or
  `DNTTK_VIEWMODEL_DEPTH=0`).
- `ttk-controls-test` 170 groups, `ttk-input-test`, Python 293 OK.
- Found, not caused: `DNTTK_UNIFIED_GEOMETRY=0 ttk-near-test` (the legacy
  pass) fails at the D17AH sky painter group, line 784, on the committed
  code as well.

## 8. Limits

- Lab evidence is offscreen at 1024x576; the user's playtest decides the
  Energy Weapon at any angle.
- The survey covers the street level's resident models; a level whose MOD
  file carries a different weapon model was not checked.
- Third-person Duke and world objects are unchanged (D17BE already covers
  world props; actors and Duke's body keep the original cull).
