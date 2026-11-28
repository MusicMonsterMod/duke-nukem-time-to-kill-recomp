# D49G - Shots and muzzle effects at the drawn barrel tip

2026-11-28. Status: Done (user-accepted 2026-11-28: "the fix is in! now we have solid looking energy weapons, and freezethrower. lock it in close it out"). User report: "the energy weapon blast
doesnt line up with the tip of the gun. probably worth a quick review too to
line everything up after the new animations". After round 1: "i tested both
and see absolutely no difference whatsoever. the energy weapon seems to fire
in front of where the weapon is"; asked, the user chose "the blast drawn over
the gun". Round 2 (below) is the fix for that; round 1's scaled muzzle point is
kept (it is right but changed little on screen).

Before/after (local, not committed): `research/before-after/D49G/`
(`before-*` with `DNTTK_FP_MUZZLE_SCALE=0`, `after-*` default; still, turning
right, strafing; `idle-tips-*` marks the recorded tip, red before and cyan
after, on idle Energy Weapon, Freezer and Gatling frames).

## Cause

Not the D36N motion, and not the D36N quarter-unit view lock. The barrel
point D49 records (`first_person_drawn_muzzle`, `first_person.inc`) is taken
after the tilt, idle gesture and motion pose are applied, from the same
display matrix the gun is drawn with. That matches the code and the lab: the
recorded point follows the gun through every move.

The gap is the mesh's display scale. The eye view draws some weapon meshes
larger than the original (`weapon_slot_scale`: slot 8 at 1.2, the Energy
Weapon (slot 10) and Freezer (slot 11) at 1.4; it scales `0x80033e40`'s own
scale argument, so the mesh grows about the hand). The muzzle vector (shorts
at `0x800c45b0 + slot*0x2c`) is in the mesh's units at the original scale:
for the Energy Weapon (-8, 350, -16), the barrel's front end at about
(18, 353, -12) in the mesh (D36O mesh dump). The muzzle flash `0x800341e4`
(hand matrix, slot, camera, a3 = pointer to that vector; disassembly: it
adds the transposed hand matrix times the vector to the hand's translation)
and the recorded point both used the vector unscaled. The Energy Weapon's
tip therefore sat about 140 units further out than the point the flash, the
beam's root (D49), the Laser Gatling streak (D49E) and its launch (D49F)
use. That point is about 30% of the gun's length back inside it.

This has been the case since the scale was set (October). It started to show
with D36O: the held weapon is now depth-tested in its own slice in front of
the room, so it always covers the beam. Before, the beam was painted over
the gun's front, and a root inside the barrel still read as attached. Now the
first part of the beam is hidden inside the gun, and the visible beam seems
to come out of the barrel's side.

## Change

`first_person.inc`, Modernized first person only (the paths are reached only
through the eye-view weapon draw):

- `drawn_muzzle_scale()`: the same `slot_scale(weapon_view_slot())` the mesh
  is drawn at (1 for every weapon not enlarged).
- The recorded barrel tip uses the muzzle vector times that scale. The beam
  root (`frame_replay.cpp`), the D49E streak (`bolt_barrel.inc`) and the D49F
  launch (`weapon_aim.cpp`) read this one point and follow without changes.
- The native flash call (hook at `0x800341e4`, ra `0x80035348`) gets `a3`
  pointing at a scaled copy of the vector in guest memory (allocated with the
  weapon matrix, so redraw workers see it). The game's table is not written.
  The flash mesh's own size is unchanged.
- A/B: `DNTTK_FP_MUZZLE_SCALE=0` at launch keeps the unscaled vector.
- Debug: `ttk_input` -> `controls.fp_motion` gains `muzzle_scale`,
  `muzzle_flashes` (scaled flash draws), `muzzle_ok` and `muzzle_tip`
  (view frame).

Hits, damage, rounds' launch points (the game muzzle `player+0xbc`), the
third-person view and Vanilla are unchanged.

## Evidence

Lab `recomp/analysis/d49g/` (local; private copy of UI slot 8, D49 cards,
offscreen, first person, `dnkroz`/`dnstuff`): `lab.py moves` holds fire
still, turning both ways, strafing and running per weapon; `idle.py` takes
idle frames with the recorded tip.

| Weapon (slot, scale) | Recorded tip, view frame, before | After |
| --- | --- | --- |
| Energy Weapon (10, 1.4), idle | (82, 134, 757) | (66, 113, 894) |
| Energy Weapon, firing still, 180 Hz | (81, 177, 746) | (66, 172, 885) |
| Freezer (11, 1.4), idle | (91, 86, 780) | (71, 48, 930) |
| Gatling (7, 1) | (232, 224, 1083) | identical |
| Flamethrower (9, 1), firing | (76, 238, 908) | identical |

Projection check: with the screen centre (512, 288) of the 1024x576 lab image
and f = 572 px (fitted on both axes from the Gatling's tip, where both builds
agree), the marks land on the Gatling's barrel tip and, after the change, on
the front end of the Energy Weapon and Freezer meshes. Before, they land
inside the gun bodies. Firing frames: the beam's root and its burst sit on the
recorded point in both builds (D49), so after the change they start at the
drawn front end. 120 Hz: the same tip and scaled flashes.

Tests: `ttk-controls-test` new group D49G (flash `a3` is a 1.4 copy for slot
10 and the game's vector is untouched; the recorded tip equals the weapon
matrix's translation plus its transpose times the scaled vector; scale-1
weapons keep the native pointer; fails with `DNTTK_FP_MUZZLE_SCALE=0`), 171
groups; `ttk-near-test` (D36O group), `ttk-aim-test`, `ttk-input-test` pass.

## Review of the other muzzle effects

- Muzzle flashes: every weapon's flash goes through the same hook. Only
  slots 8, 10 and 11 are enlarged. Slot 8's muzzle vector is (0, 0, 0), so
  scaling does not change it.
- Laser Gatling bolts (D49E streak, D49F launch): slot 7, scale 1. They read
  the recorded point, so they would follow a future scale change.
- Rounds in flight, rockets, fireballs and the Flamethrower stream (D49B):
  launched from the game muzzle and drawn as world objects. They are covered
  by D10H8 and do not use the drawn barrel. Not changed.
- Impact glows (D49A): at the beam's far end, not the muzzle. Not changed.
- Weapon motion (D36N): already part of the recorded point. No change needed.

## Limits

- The flash keeps whole-unit placement (the weapon matrix, note 572). The
  recorded tip also comes from that matrix, under 1 unit off the view-locked
  gun (well under a pixel at 900 units).
- Offscreen lab under llvmpipe. The user's display at 180 Hz decides.

## Round 2 - the flash painted over the gun (2026-11-28)

User, after round 1: no visible difference; the blast is drawn over the gun.

Identified by elimination in the lab: with `DNTTK_BEAM_FOLLOW=0` the beam's
streak moves away but the green and white blast stays on the gun; with the
flash vector thrown behind the eye (a temporary probe, removed) the blast is
gone, the gun is whole and the beam starts at its front end. The blast is the
native muzzle flash mesh (Energy Weapon mesh 1087: blades from the muzzle 348
units forward, 175 out).

Cause: D36N draws the view-locked gun and arms in a camera space 4 times
finer (`view_locked_transform`, the frame's factor; projection unchanged), so
their view depth is 4 times larger. The flash's transform (`0x800292a0` at ra
`0x8003437c`, its stack matrix built from the weapon matrix) and the held
item's (ra `0x8003414c`) were not locked (note 572) and kept factor 1. D36O's
depth slice (`16 + z/10000`) and the held packets' farthest-first sort compare
those raw depths: the flash at about 885 read nearer than every gun face
(about 3500), passed the test everywhere and was sorted last, so it was
painted over the whole gun. D36N's own comment names the risk ("a part at
another factor would sort as if nearer or farther"). This also hid round 1's
change.

Change (`first_person.inc`, Modernized first person): `view_locked_transform`
also takes the flash and held item transforms while the weapon is live: their
view offset comes from their own matrix's translation, then the same private
camera and the same frame factor as the gun. Picture unchanged; the depth
slice now sees them at the gun's scale. A/B `DNTTK_FP_ATTACHED_LOCK=0`; debug
`fp_motion.attached_locks`.

Evidence (same lab, 180 Hz, still and turning):
`research/before-after/D49G/flash-over-gun-before-top-after-bottom.png`. With
the lock off the flash covers the Energy Weapon; with it the gun is drawn in
front and the flash and beam come out of its front end. Pistol, Shotgun,
Gatling, Freezer and Flamethrower checked in the same run: flashes at their
barrels, guns in front. Tests: `ttk-controls-test` 172 groups (new D49G flash
lock group, which replaces D08A53's "flash transform left alone" check; fails
with the lock off), `ttk-near-test` 16, `ttk-aim-test` 11, `ttk-input-test`,
Python 293 OK.

Limits: the flash's blades still reach past the gun's sides (the native mesh,
not scaled with the 1.4 gun); held items were not separately tested in
the lab (the same lock, guarded the same way).
