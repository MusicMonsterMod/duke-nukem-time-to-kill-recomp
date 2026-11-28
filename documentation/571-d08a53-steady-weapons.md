# D08A53 - Steady weapons while walking, running and turning (first person)

2026-11-27, rounds 2 and 3 2026-11-28. Status: **Done, user-accepted 2026-11-28** ("lock it in and close it out"). User, 2026-11-26: "a review of all the
weapons for stabilization because i noticed a few vigorous shakes here and
there, so we just wanna smooth everything out a bit and go for more 'realism'
rather than jerkiness". Scope narrowed by the user on 2026-11-27: "this is basically gonna
be entirely a first person piece - third person is totally unaffected by this
stuff. so the positioning on everything is nice. our idle anims are absolutely
perfect and super realistic. the only thing is really, when walking around.
when duke walks around there are unusual little shakes, at his wrist, or the
gun may very slightly (slightly) vibrate", and "walking and running, usually i
run everywhere so i see it everywhere".

Evidence labels: S source (recomp source, generated code read only), L private
runtime (offscreen lab), U user report. The D36 audit
([note 257](257-d36-first-person-presentation-audit.md)) applies. Related:
D48 ([note 423](423-d48-gatling-steady-hand.md)), D08A52
([note 570](570-d08a52-steady-grenade-hand.md)), D17AY (redraw blending).
Modernized only. Third person draws the native skeleton and is unchanged.

## Measurement

L: lab `recomp/analysis/d08a53/` (local, ignored). `probe.py` runs a private copy
of the cards (UI slot 2, level 6) offscreen at 180 Hz on the GTX 1080 Ti, gives
all weapons with `dnstuff`, and for each weapon traces 120 ticks idle, 90
walking forward, 90 back, 90 running forward, 90 back (`walk`). The `act`
case covers firing while walking, after firing, looking down while walking,
looking up while backing, and running. `DNTTK_PRESENT_TRACE` logs each drawn
point relative to the view of the redraw that shows it. New developer points:
every display arm segment (`arm10`-`arm15`, and `x` points off each along all
three axes, so a rotation shows too), the weapon's axes (`weapon_axis`), and
the translation the GTE gets for the weapon (`gte_weapon`) and the arms (`garm10`-`garm15`).
`jitter.py` measures each point's screen path at 2560 wide as the residual
from a 9-redraw centred mean, plus the share of steps that reverse direction.
A vibration reverses about half its steps; motion does not. `summary.py`
gives the worst arm point per weapon and phase, `placement.py` the mean drawn
position (rest look), `series.py` single paths, and `plot.py` the
before/after picture. The player's saves and profile were hash-checked
unchanged after every run.

## Four causes

### 1. The drawn arms took the live skeleton's rounding (walking, running)

S: `display_arm_matrix` (`first_person.inc`) draws each arm segment with its
rotation taken from the live joint matrices relative to the live hand, and the
support wrist from the live skeleton. The game builds those matrices in fixed
point: Q12 rotations and whole-unit positions, with GTE products down the
spine and arm chain. That chain also carries a scale of about 0.99.

L: standing still, the rounding is the same on every tick: idle arms are
exactly still (0 px), and they come back to the same pixel at any other spot
(0.00 px after walking elsewhere). Walking moves the root and legs every tick,
so the rounding up the chain changes from tick to tick, while the upper clip
stays the same (Shotgun clip 20, Eagle clip 5). Old build, walking:

| Weapon | Worst arm point, rms / p99 px | Reversing steps |
| --- | --- | --- |
| Shotgun | 2.3-2.6 / 5.7-5.9 | 54-70% |
| Buffalo Rifle | 1.1-1.4 / 2.9-4.4 | 33-39% |
| Gatling | 1.3-1.4 / 3.0-3.3 | 47-55% |
| Desert Eagle | 0.6-1.4 / 1.5-3.5 | 5-39% |
| Pipe Bomb | 2.5-3.2 / 6.6-7.4 | 50-62% |
| Knife, Axe, Crossbow | 2.8-3.3 / 5.8-7.5 | 54-62% |

Change: `exact_arm_pose_live` rebuilds both arm chains relative to the right
wrist (as `arm_reference` does) from the **joint records**. Those are the local
rotations the game composes this frame, so they follow its own blend and
timing, and a joint without a key keeps its record (retail ready clip 29's
right wrist). The model's bone offsets give the chain. A first attempt that
rebuilt the pose from the clip keys at the timer fraction was 28 degrees off
the live pose at clip changes (Gatling run start): the game does not blend
key to key there.

The live pose's fixed-point bias is part of the accepted rest look, and the
D36H1/H2 grips depend on it (painter order within a unit). So a **rest bias**
is kept per weapon slot: per joint, a rotation and the live scale; the
support wrist's shift; and the upper-arm and forearm bones as drawn. It is
taken on live draws only, while Duke's ground position and heading hold still
(the idle clip moves the root height, which adds no noise), both arm channels
hold one key, and no run key is involved. It is added to the pose, so at rest
the arms are drawn exactly as before, and moving they keep that look without
the per-tick noise. Redraw workers get the per-tick pose with the render
state and blend it between ticks (D17AY). Standing ground modes only: crouch
and swim keep their D08K10 / D11E channel paths. A/B: `DNTTK_FP_EXACT_ARMS=0`,
and `DNTTK_FP_EXACT_BIAS=0` for the pure record pose.

### 2. The run blend swung the support hand out and back

S/L: running, the seeded weapons keep about a tenth of the native run swing,
blended toward their ready chain (D36H3/D08A29). The weight ramped with the
timer over the first run key, while the native arm crossed from the ready key
to the run key. Midway the arm had moved but the weight had not, so the
display went `slerp(native(t), ready, 0.9 t)`. The Shotgun's support hand
swung about 100 px out and back in three ticks at every run start, and the
reverse at every stop.

Change: from a ready or run key the larger of the two keys' weights applies
at once, so the arm moves straight to its run pose while the native pose
crosses. From another key (a firing key) the weight still ramps, so that pose
does not snap. The same per-weapon chains apply: the Buffalo's right chain,
and the Gatling and Freezer support hands with their D48/D36H11 holds on top.
The ready grip the run blends toward gets the same rest bias.

### 3. The Knife, Axe and Crossbow showed the live hand

L: slots 1-3 have no seeded ready pose. Standing, they showed the live
skeleton's hand, as the throwables did before D08A52: the weapon itself
jittered 1.8-3.5 px, with a third or more of the steps reversing.

Change: they take the arm-channel hand standing too (`throwable_channel_hand`,
as D08A52's slots 12-14). The Crossbow shares the Shotgun's ready (20) and run
(18, 19) clips, but with no seed it showed the whole native run swing: the tip
held still for about five ticks, then whipped 200-250 px across the view. It
joins the run blend (slot 3 in `run_arm_eligible`), and its hand is taken
from the right arm's records with the same per-key run weight toward clip 20's
ready key.

### 4. The gun and arms hopped whenever the view turned

L: walking straight, the gun was exactly still against the view. Looking down
while walking, the grip hopped 2-3 px once per tick (redraw fraction about 0.30),
then returned. S: the weapon matrix is placed from the eye (`eye + C^T v`),
but its world translation is stored in whole units. `0x800292a0` then forms
the GTE translation as the view rows times (translation - eye). As the view
turns, the rotated offset meets the unit grid at a new point, so the gun moves
up to a unit against the view: about 2.5 px at 2560 wide at the grip, more
nearer the eye. Mouse look and turning while walking or running both do it;
holding still does not. The offset `v` itself was constant (traced). The arms
inherited the same rounding through the weapon matrix's translation.

Change (`view_locked_transform`, entry of `0x800292a0`): for the hand joint's
transform and the display arm segments (return `0x80034c6c`), and for the
weapon mesh's own transform (return `0x80033f14`, `0x80033e40`'s scaled stack
copy, used when a weapon has a display scale), the call gets two private
inputs:

- a camera whose view rows are only the projection scale;
- a matrix `M C^T` whose translation is the intended view offset, rounded once
  in view axes.

The GTE gets the same rotation, and a translation that no longer changes as
the view turns. Each matrix is matched by the exact bytes it was written as,
so other writers of the same scratch (contact arms, hidden arms) are left
alone. The arm rig hangs from the unrounded grip. A/B:
`DNTTK_FP_VIEW_EXACT=0`.

## Results

L: same build, all changes against `DNTTK_FP_EXACT_ARMS=0
DNTTK_FP_VIEW_EXACT=0 DNTTK_FP_THROW_HAND=live`. Worst arm point, rms px (reversing steps):

| Weapon | Walking before | Walking after | Running before | Running after |
| --- | --- | --- | --- | --- |
| Desert Eagle | 0.57-1.39 (7-37%) | 0 | 1.3-2.2 (31-67%) | 0 |
| Shotgun | 2.4-2.6 (60-70%) | 0 | 2.1-3.3 (10-20%) | 0.96 (8-13%) |
| Gatling | 1.3-1.4 (47-50%) | 0 | 2.4 (37-39%) | 1.6 (1%) |
| Buffalo Rifle | 1.1-1.4 (33-38%) | 0 | 8.5-8.6 (35-36%) | 5.0-5.9 (1-2%) |
| Energy Weapon | 0 | 0 | 0.7-1.1 (0-2%) | 0 |
| Flamethrower, Freezer, RPG | 0 | 0 | 0 | 0 |
| Pipe Bomb | 2.9-3.2 (53-62%) | 0 | 3.5 (61-65%) | 0 |
| Throwing Axe | 3.0-3.3 (54-57%) | 0 | 3.4-3.8 (65%) | 0 |
| Crossbow | 2.8-2.9 (58-62%) | 0 | 8.2-8.6 (16-21%) | 1.6 (10-20%) |

What is left running has few reversing steps. It is the run swing the D36H3
design keeps on purpose (Buffalo and Gatling), not a vibration. The Knife's
blade traces leave the measured screen area, but its grip and axis trace 0
walking and running. Shotgun run start, support hand: steps of 11-24 px out
and back, then smooth to the run pose.

View turning, the drawn translation (`gte_weapon`, `garm`), rms px (reversing
steps):

| Weapon, part | Before | After |
| --- | --- | --- |
| Desert Eagle, grip | 1.1-1.2 (18-20%) | 0.25 (0-1%) |
| Shotgun, grip | 1.6-1.8 (23-26%) | 0.31 (3%) |
| Energy Weapon, grip | 1.7 (22-29%) | 0.28 (2-6%) |
| Buffalo, grip | 1.8 (23-28%) | 0.27-0.29 (2-6%) |
| Buffalo, support hand | 1.5 (21-22%) | 0.20-0.23 |
| Gatling, support arm | 0.9-1.0 (12-14%) | 0.4-0.5 (4-8%) |

The remaining 0.2-0.3 px is the trace's own conversion back through the
camera rows.

Rest look: idle arms land on the live pixel (Shotgun support arm 0.00 px). The
grip and arms are drawn at their intended position rounded once, where before
the whole-unit world rounding put them anywhere within about a unit. That unit
depends on the view angle, so the old rest spot was only defined to within it
(up to about 4-5 px). The Knife and Crossbow now use the channel hand, as the
throwables do: 4-7 px, one to two units.

Regression checks (L): firing while walking is the same recoil and fire clips,
with smaller single steps (Gatling 146 to 106 px, Shotgun 103 to 51 px); the
Shotgun's pump after a shot follows the same path, without the old 40 px jump.
Vanilla has no first-person display and no number keys: no first-person points are drawn
with the changes on or off. Native: `ttk-controls-test` 169 groups pass (new
D08A53 check of the locked transform: scale-only private view rows, the same
GTE rotation and translation). Group expectations moved with the design:

- D36H3: the run twist is the wrist record's, and the run weight applies at
  once from a ready key.
- D08A52 / D08K10: slots 1-3 keep the channel hand standing.
- D11E and the worker-eligibility group: now use empty hands.

`ttk-near-test` and `ttk-input-test` pass; Python 292 OK (2 skipped).

Picture (local): `research/before-after/D08A53/steadiness-trace.png`. It
shows the Shotgun support hand walking and at a run start, and the Desert
Eagle grip looking down while walking, before and after.

## Round 2 (2026-11-28): jumping, wrist roll

U, the user's playtest of round 1: "the weapons themselves, including the holy
hand grenade, are all noticably smoother, but the issues i see are in like,
arms and wrists ... like how the wrist on the gatling gun twists around".
Their run, turning and bunny-hopping test, one weapon at a time:

| Weapon | Report |
| --- | --- |
| Throwing Knife | Vibrates a little when running and turning, acceptable, low priority |
| Throwing Axe, Shotgun, RPG | Acceptable |
| Shotgun | The support hand under the gun "lacks robustness" (a design matter: D36N) |
| Pipe Bomb, Holy Hand Grenade, Dynamite | "Dukes wrist specifically vibrates", most when turning, stronger when jumping |
| Energy Weapon | "crazy looking wrist" when jumping |
| Buffalo Rifle | Both wrists vibrate when jumping and turning |
| Crossbow | The same, milder |
| Freezer | "the wrist is just going crazy" (right) |

The lab adds a `hop` case: running with Shift, turning with the mouse and
jumping every 36 ticks, 150-300 ticks per weapon. Jumping is movement mode 9
(`player+0x22c`); the upper clip stays the same (Shotgun 20, Pipe Bomb 39).
The new `twist.py` reads a developer trace of each forearm's roll about its
own axis against the hand (`tw11`, `tw14`, degrees, by swing-twist
decomposition).

### 5. Jumping fell back to the rounded skeleton

S: the exact arms (cause 1) and the run blend took ground modes only (0, 1),
so in a jump the arms went back to the live skeleton's rounding, with a pose
switch at take-off and landing. Now mode 9 is included (`exact_arm_mode`).
The rest bias and roll are sampled on the ground only.

### 6. The arm rig re-entered the camera rows' rounding

L: a forearm drawn near a half-unit flipped one view unit (3 px) back and
forth while turning. S: the Q12 camera rows are not exactly orthonormal, so
going to the world and back (`C C^T`) moved the view offset about 0.1 unit,
differently as the view turned. The support wrist was also built through the
weapon matrix's Q12 rotation, rounded again on each redraw. With the view lock,
`view_exact_camera` orthonormalizes the camera rows for the view-locked
geometry: weapon placement, arm rig and locked transform. The arm rig uses the
weapon's unrounded rotation, kept with its view offset.

### 7. The forearms rolled around the fixed hand

L, roll range in degrees:

| Case | Roll |
| --- | --- |
| Gatling and Freezer gun wrist, run and hop | 29-41 (their gun arm keeps the full native run, by design since D36H3) |
| Buffalo support wrist, run and hop | 27-89 |
| Looking up and down while walking | 38-117 (Knife 117, Axe 99, Shotgun support 76) |
| Fire clips | 40-230 |

The arm kept bending and swinging correctly, but corkscrewed around a hand
that does not move.

Change: each forearm keeps a **rest roll** against its hand, per weapon slot
and side. It is turned about its own axis to that roll; its bend and swing stay
animated. The rest roll:

- eases (30% a tick) toward the drawn roll only while Duke stands still on the
  ground, with no run key, so idle settles on the old look in about 0.3 s;
- holds while he moves, jumps, looks around or fires.

The lock's strength also eases (30% a tick):

- toward 1 while a weapon is drawn with a rest roll;
- toward 0 in the empty hand of a draw, holster or switch (slot 32);
- back to 0 after any gap in the exact pose (crouch, swim, climbs).

A Pipe Bomb redraw had turned the forearm 65 degrees in one tick when the
lock simply switched off. Both values go to redraw workers with the render
state and are blended between ticks. The exact arms now also blend across a
weapon change (the first tick of a draw had stepped 20 degrees). A/B:
`DNTTK_FP_WRIST_ROLL=0`.

### Round 2 results

L: the `hop` case, same build, everything from D08A53 against
`DNTTK_FP_EXACT_ARMS=0 DNTTK_FP_VIEW_EXACT=0 DNTTK_FP_THROW_HAND=live
DNTTK_FP_WRIST_ROLL=0`. Drawn arm segment origins as the GTE gets them,
rms px (reversing steps):

| Weapon | Upper arm (13) | Forearm (14) | Support arm (11, 12) |
| --- | --- | --- | --- |
| Pipe Bomb | 5.5 (69%) to 0.51 | 4.1 (68%) to 0.35 | - |
| Holy Hand Grenade | 7.3 (65%) to 0.58 | 5.6 (68%) to 0.41 | - |
| Throwing Knife | 7.6 (63%) to 0.61 | 5.6 (62%) to 0.43 | - |
| Throwing Axe | 7.0 (62%) to 0.54 | 5.3 (65%) to 0.38 | - |
| Desert Eagle | 5.9 (67%) to 0.41 | 3.4 (66%) to 0.29 | - |
| Shotgun | 6.0 (56%) to 0.73 | 3.7 (54%) to 0.49 | 3.9-4.4 (56-59%) to 0.59-0.65 (2%) |
| Gatling | 3.4 (61%) to 0.39 | 2.5 (64%) to 0.31 | 1.1 (46-50%) to 0.20-0.22 (1%) |
| Buffalo Rifle | 5.7 (61%) to 0.62 | 3.5 (59%) to 0.40 | 1.6-2.2 (45-58%) to 0.19-0.23 (0-1%) |
| Energy Weapon | 5.2 (65%) to 0.57 | 3.2 (70%) to 0.38 | 1.2-1.3 (52-62%) to 0.18-0.20 (2%) |
| Flamethrower | 5.3 (57%) to 0.72 | 3.3 (55%) to 0.49 | 1.4-1.6 (44-45%) to 0.22-0.27 (1%) |
| Freezer | 5.9 (64%) to 0.62 | 3.7 (65%) to 0.42 | 1.4-1.6 (60-61%) to 0.20-0.24 (1-2%) |
| Crossbow | 6.2 (54%) to 1.15 | 4.2 (58%) to 0.89 | 5.2-6.5 (51-55%) to 0.72-0.76 (5-6%) |
| RPG | 7.8 (56%) to 1.03 | 4.9 (59%) to 0.66 | 5.7-7.9 (64-65%) to 0.43-0.65 (5-13%) |

After, reversing steps are 0.4-24%: what is left is the motion of the run and
jump, not vibration.

Forearm roll, running and hopping, range / largest step in degrees, lock off
to on:

- Gatling gun wrist: 29-41 / 3.4-5.7 to 0.2 / 0.
- Freezer gun wrist: 29-39 / 2.2-4.3 to 0.1-0.2 / 0.
- Buffalo support wrist: 27-89 / 3.8-19.8 to 0.1-0.2 / 0.
- Every other weapon: at most 7 to 0-0.3, except one Energy Weapon run at 1.9.
- The Pipe Bomb right after a throw eases back over about a second (31
  degrees, at most 3.6 a step), with no snap.

Native: `ttk-controls-test` 169 groups, `ttk-near-test` and `ttk-input-test`
pass. They do not reach the roll lock, which needs real still ticks from the
host frame counter; the lab measures it. Picture (local):
`research/before-after/D08A53/hop-trace.png`. Its "before" has all of
D08A53 off.

Tier 3 (the user: "I think we could do so much better with these weapon
animations") is its own job: [D36N](../jobs/first-person.md#d36n---first-person-viewmodel-motion-and-grips),
the arms and weapon as one rig with deliberate motion and camera-designed
grips, research first.

## Round 3 (2026-11-28): support hands, upper arms, kick order

U, the user's playtest of round 2. They found no vibration on any weapon any
more:

- **Perfect or good:** the Knife, Axe, Desert Eagle, Gatling, RPG,
  throwables, Energy Weapon and Crossbow.
- **Shotgun:** "Duke still does this kind of like weird rubbing thing with his
  left hand", and they would like it "the same kind of thing" as a stable
  Buffalo.
- **Buffalo:** "that left arm looks like it wants to move but it doesn't".
- **Freezer:** "the right arm looks like it was to fiddle".
- **Quick kick:** "should always be below his hands and the guns". It drew in
  front of the weapons, and with the weapon holstered over the hands while
  bunny-hopping.

L: new developer traces `tw10`/`tw13` (each upper arm's roll about its own
axis) and `slide.py` (a drawn point against the grip in view space), lab
`hop`, `walk` and `act` cases:

| Weapon | Measured, running and hopping |
| --- | --- |
| Freezer, right upper arm | Rolls 92-183 degrees |
| Gatling, right upper arm | Rolls 93-99 degrees |
| Buffalo, left upper arm | Rolls 75-340 degrees |
| Shotgun, support hand | Slides 12-13 px along the gun when running: the tenth of the native run swing the D36H3 blend kept on the support hand (0 px walking straight) |

The forearm was locked in round 2. The upper arm was the next segment up.

### 8. The upper arms rolled; the Shotgun's support hand slid

- **Upper arms:** each one now holds its roll against its (locked) forearm. It
  uses the same mechanism as the forearm: a rest value per slot and side,
  eased only while still; the side's eased lock weight; values carried to
  redraws (`ExactArms` rolls widened to four). The segment code is factored
  into `segment` (the pose turned onto its target direction), `twist` and
  `turn`; the forearm lock is applied in the upper arm's draw as well, so the
  reference is exact.
- **Shotgun:** the support chain's run weight is 1 (held on the ready grip,
  like the Gatling and Freezer support hands) instead of 0.9.

| Weapon | Before | After |
| --- | --- | --- |
| Buffalo, left upper arm roll, running | 75-340 degrees | about 1 degree |
| Freezer, right upper arm roll, running | 92-183 degrees | about 1 degree |
| Shotgun, support hand slide, running | 12-13 px | 0 px |
| Gatling, support hand slide, running | - | 0 px |

Hopping, the support hands move 1-1.6 px against the gun: the jump itself.

### 9. The kick leg drew over the guns and hands

S (`near_clip.cpp`): the weapon, display arms and the kick leg (`LimbOwner::Kick`)
share the deferred first-person layer (`held_packets`, OT slot 1). That layer
was drawn farthest-first, and the foot (anchor depth about 160) is much nearer
than the gun (about 520), so it was drawn last, on top.

The holstered hands (`LimbOwner::Body`, D36H4) are world-depth packets in the
world's slots. Slot 1 is drawn after every world slot, so a kick always
covered them. D36J3 and the D36 audit had accepted the overlap; the user now
asks for the leg behind.

Change:

- **Rank inside the layer:** each held packet gets a rank, 0 for the kick leg
  (quick and door kick) and 1 for the rest. The flush sorts by rank, then
  farthest first, so the leg is drawn first and the weapon, arms and hands
  cover it. One slot and one flush, so the HUD, Bio Mask and frozen tint
  keep their order.
- **Holstered hands during a quick kick:** while a quick kick plays
  (`quick_kick_live`, carried to redraws as `kick_active`), the holstered
  hands (parts 10-15) take the first-person layer's depth, so they draw
  after the leg. During the kick they lose world occlusion.
- **A/B:** `DNTTK_FP_KICK_BEHIND=0`.
- **Test input:** a `scroll` word (Scroll Lock) for the lab's holster step.

L: frames during a kick, A/B
(`research/before-after/D08A53/kick-order.png`):

- With the Shotgun, the boot covered the support hand and the gun's lower
  half; now the gun and hand are drawn over it.
- Holstered while hopping, the leg covered the left fist; now both fists are
  drawn over it.

Native: new `ttk-near-test` group D08A53. A far display-hand mesh and a near
kick mesh are flushed, and every kick packet must come before every hand
packet; it fails with `DNTTK_FP_KICK_BEHIND=0`. It sits in the legacy-geometry
contracts (`DNTTK_UNIFIED_GEOMETRY=0 ttk-near-test disc/SLUS_005.83`).

Finding, not from this job: that legacy run already aborts afterwards, in the
D17AH sky-painter group (verified with the D08A53 changes set aside). No
documented command runs that path.

## Limits

- Offscreen runs; the playtest on the real display at the player's rate is the
  acceptance.
- The Gatling's right forearm: old redraws drew it 0.2 units shorter than the
  live draw that the rest bias is taken from (2.6 px at rest).
- The drawn positions are still whole view units: slow motion (a recoil's
  return) moves in even 1-unit steps (about 1.4 px at the grip).
- Crouch, swim, climb, contact and other actions keep their own paths. The
  muzzle flash and held items are not view-locked (the flash lasts one frame).
- With the roll lock, aiming up or down while standing still still rolls the
  forearm as before (the rest roll follows the native pose at rest); moving,
  it holds.
- The arms still follow the original third-person animation's bend and swing
  from the eye; a purpose-built viewmodel is D36N.
- While a quick kick plays with the weapon holstered, the hands do not get world
  occlusion (a wall right in front of them does not hide them for that moment).
- The Crossbow's fire and reload clips are fast native motion, unchanged.
