# D36N - First-person viewmodel motion and grips

2026-11-28. Status: **Done, user-accepted 2026-11-28** ("lock this in and close it all out"). Follow-up: D36O (weapon model repair). Board: `MODERNIZATION_JOBS.md` D36N,
work log in [jobs/first-person.md](../jobs/first-person.md#d36n---first-person-viewmodel-motion-and-grips).
Dependencies: D08A53 (Done, [note 571](571-d08a53-steady-weapons.md)).
References: the [D36 audit](257-d36-first-person-presentation-audit.md), the
D36H5 arm rig ([305](305-d36h5-contact-arm-rig.md), [306](306-d36h5-interaction-arm-coverage.md)),
the D53G idle gestures ([note 532](532-d53g-first-person-idle-gestures.md)).

User, 2026-11-28, D08A53: "I feel like we could do so much better with these
weapon animations ... this is one of the most important visual aspects of
the game and people are going to remember this"; on the Shotgun: "duke holds
the shotgun with his left hand under the gun you know I'd like to see how
other games do that same pose ... there is a lack of robustness to it". At
selection: "a deep research piece ... which seriously analyzes in detail how
these or similar weapons are done in other games ... realistic motion with
the weapons when running/walking/jumping etc. ... all decisions will be
yours." The board asked for options before implementing; the user handed
every decision to the agent, so the research chose the design.

Evidence labels: S source, L private runtime (offscreen lab), R research
(with its source class: code, disk, sourced, inferred). Modernized only;
Vanilla has no first-person weapon and is unchanged; third person draws the
native skeleton and is unchanged; the frame rate never changes.

## 1. Research

Three studies, written up locally in `research/d36n/` (not tracked; about
19,000 words with code excerpts and citations):

- `engines-source.md`: the released source of Duke Nukem 3D / EDuke32
  (local copy), Quake, Quake III, Half-Life 1, Source SDK 2013, Doom 3 BFG,
  GZDoom and UT99/2004: every formula and constant of the weapon bob, turn
  lag, landing, take-off, idle drift and firing interaction.
- `local-games.md`: readable logic in games installed on this PC, read only
  (no game launched): Selaco's ZScript (`Selaco.ipk3`), Doom 3 BFG's weapon
  defs (`base/_common.resources`), Ion Fury's CON scripts (`fury.grp`), Deus
  Ex's UnrealScript (`Engine.u`), and the settings of Black Mesa, Half-Life,
  Portal, Dusk, Duke Nukem Forever, HROT and the Unreal Engine 4 games.
- `aaa-practice.md`: studio talks, interviews and articles (Overwatch, Halo 2,
  Crysis 3, Call of Duty Modern Warfare 2019, Apex, Destiny, Doom 2016 and
  Eternal), animator guidance on camera-designed poses, the real pump-shotgun
  grip, and the support-hand poses of shooters with a pump shotgun.

A private research page (artifact, in the user's gallery) summarises the three with plots of the formulas.

### What every reference agrees on (R)

| Layer | Consensus | Best models |
| --- | --- | --- |
| Assembly | Arms and gun move as one rigid unit; authored fire, reload and draw play underneath, procedural motion on top | every engine; Halo 2, Overwatch (studio talks) |
| Walk bob | Side sway at half the vertical rate (2:1, a figure 8 or arc), on a step clock, amplitude from speed | Quake III / Doom 3 (step clock), Duke 3D (constant "U" arc) |
| Bob shape | Duke 3D and Doom: lowest crossing the centre, rising at the sides ("U"); Quake III and Selaco: the reverse (arch) | the "U" is the Duke look |
| Look lag | A bounded lag that settles without overshoot: Doom 3 averages the last 10 frames of view angles (x0.25, cap 10 degrees); Source follows the view at 5/s (tau 0.2 s); Selaco and Ion Fury use leaky integrators with caps | none of them bounce |
| Movement | Doom 3: a 400-500 ms raised-cosine pulse on each movement change; Selaco: strafe start/stop kicks; Ion Fury: strafe slide | inertia, short and bounded |
| Jump | Doom 3: a 400 ms sag on take-off; Selaco: the gun sinks from vertical speed (cap 11, decay 0.71/tic); Ion Fury: offset from vertical speed (cap 32) | vertical speed drives it |
| Landing | Quick down, slower back: Quake III / Doom 3 150 ms down, 300 ms up, the gun 25% more than the eye; Duke 3D 8 px per fall-speed step; Deus Ex 0.055 x speed with an exponential return | lopsided, scaled by impact, capped |
| Idle | Very small: Doom 3 / Quake III about 0.4 degrees over 6.3 s; Selaco 3.4 s breaths; CoD MW2019 "not on the deck of a ship" | faint |
| Firing | The bob keeps running reduced (Selaco 67%); GZDoom stops it by default; studios keep the muzzle still as a shot leaves | ease it down |
| Sprint pose | Absent in every classic engine; AAA games with a separate sprint key lower and turn the gun | none for a game played running |
| Precision | Float transforms everywhere | - |

### The pump shotgun support hand (R)

- Real technique (sourced): push-pull; the support hand wraps the fore-end,
  fingers curled round it, the thumb on the near side or over the top. A
  flat palm under the fore-end reads weak.
- Duke 3D (code): the pump swings the whole gun sideways (32 then 64 px)
  so the rack reads, starting about 0.3 s after the shot.
- Doom 2016 / Eternal (sourced excerpt): the gun turns over toward the
  centre to pump. Half-Life 2 (sourced): the pump is its own activity with a
  fully articulated left hand.
- Screen composition (sourced, Overwatch): the support hand may come toward
  the centre, below the aim zone; knuckles on the near side read as a firm
  grip.

## 2. Design (S, `recomp/src/ttk/viewmodel_motion.inc`)

One pose about the grip in the level frame, the D53G gesture's convention
(pitch muzzle up, yaw muzzle right, roll top left in degrees; x right, y down,
z forward in view units), added to the gesture in `prepare_display_matrix`.
The arms hang from the weapon matrix (`display_arm_matrix`), so the gun and
both arms move as one assembly and every D08A53 lock (exact arms, roll locks,
view lock) still applies. Time is the game's frame step (`0x800d21fc`, 300
units a second; pause holds it). The pose is computed once per live frame, at
its first build (the arms build before the hand), and published in
`RenderState.fp_motion`; redraw workers blend it between ticks like
`fp_idle`. Layers:

| Layer | Model | Tuning |
| --- | --- | --- |
| Look sway | The view's angular velocity (this frame's forward against last frame's right and up axes), low-passed 50 ms, times a 22 ms lag, soft-capped (tanh) at 3.2 degrees, through a near-critical spring (3.6 Hz, damping 0.9). Yaw and pitch lag, the gun trailing 1.6 units per degree, roll 0.55 per degree of yaw lag | Source / Doom 3: bounded, no wobble |
| Inertia | Side and forward: a mass on a spring (3 Hz, damping 0.7) held by the view, kicked by Duke's velocity change each frame (soft limits 9 and 10 units), with roll from the side offset and a strafe tilt (2 degrees at running strafe speed) | Doom 3 pulses, Selaco strafe kicks |
| Air and landing | In the air the gun is offset by vertical speed (0.0032 units per unit/s, cap 6, eased 80 ms): it lags low on the rise and floats up falling. A landing (air to ground) dips it by 0.0065 per unit/s of the fall beyond 200 (cap 14), 150 ms down (ease out), 300 ms back (ease in-out), the muzzle 0.25 degrees down per unit; a landing during a dip continues from its depth | Quake III / Doom 3, Ion Fury |
| Bob | On the D17V view bob's phase (the eye and the gun step together; its own copy when VIEW BOB is off): one side sway per stride, one dip per step, lowest crossing the centre (Duke 3D's arc), 0.35 rad behind the step; amount 1 walking, up to 2 running (the view bob's own speed curve). Walking: x 4.5, y 4.0 units, roll 1.2, yaw 0.9, pitch 0.5 degrees. Firing eases it to 35% (40 ms in, 250 ms out) | Duke 3D arc, Quake III rotations |
| Run carry | Running (over 2.5 x walking speed) on the ground, no shot for 0.45 s: muzzle 1.5 degrees down, yaw 1 in, roll 2.5, x -3, y +5, z -3; in over 0.22 s, out in 0.05 s (a shot or the fire button) | subtle: the player runs everywhere |
| Breath | Standing still and not looking around, no gesture playing: a 4.6 s breath, 0.3 degrees, 0.9 units; in over 1.2 s, out in 0.25 s | Doom 3 idle size |

Eligibility: standing or jumping (movement modes 0 and 9), perspective
Ground/Fall/None, no crouch, swim or one-handed climb path; the weight eases
over 0.12 s. A weapon change starts from rest. Per slot: the RPG's rotations
are halved (it fills the lower view); the Energy Weapon only moves down (its
support shoulder shows above ready, D53G); other two-handed weapons rise at
most 6 units and 3 degrees.

**OPTIONS > GAMEPLAY > WEAPON MOTION:** ON (default), SUBTLE (half), OFF
(held still). Profile schema 84 (`weapon_motion`), `run.py --weapon-motion`,
`DNTTK_WEAPON_MOTION`; Vanilla `off`. Live: the OPTIONS change applies at
once. Developer: `DNTTK_FP_MOTION_PARTS=sway,inertia,bob,carry,breath`,
`DNTTK_FP_MOTION_LOG=1`; the debug port's `controls` JSON has `fp_motion`.

### The Shotgun grip and rack

L (stills): the native pose put the support fist against the back of the
wooden fore-end, under the receiver, and the native pump (upper clip 26,
keys 79-90, after fire clips 24 and 25, then 23 back to ready 20) lifted the
fist off the gun toward the top of the receiver. That is the "weird rubbing
thing" of the D08A53 playtest.

- **Grip:** the support chain is held on the ready grip (clip 20 key 0)
  through ready, run (18, 19), fire, pump and return (23-26), eased in 60 ms,
  so a draw keeps its native arm. The whole chain is moved in the gun's own
  axes (local y is the barrel, the muzzle vector is (-1, 363, 60)) to
  (-8, 72, 12) and the hand rolled 15 degrees: under the front of the
  fore-end, a little to its left so the knuckles wrap it. Chosen from three
  still sweeps (21 placements).
- **Rack:** progress through clip 26 from its key and the key's countdown.
  The hand pulls 34 units back along the barrel (smooth, 2-38%), holds, and
  drives home (52-86%); the gun cants top-left 7 degrees, lifts the muzzle
  2.5 degrees and moves 6 units toward the centre, peaking with the pull
  (Duke 3D's swing, Doom's turn toward the centre). The real stroke is about
  10 cm.
- The grip and rack are a pose fix, not a motion preference: they apply at
  every WEAPON MOTION setting. A/B `DNTTK_FP_SHOTGUN_GRIP=0`. Developer:
  `fpgrip ox oy oz [pitch yaw roll]` / `fpgrip off` (console, session only).

### Sub-unit viewmodel placement

L: with deliberate motion the trace showed the drawn gun stepping 2.5 px at
a time (2560 wide). S: the GTE keeps camera space in whole units (MAC >> 12),
and the PGXP mesh path accepts only a fraction that agrees with the native
integer pixel, so every viewmodel vertex sat on a whole-unit grid: the
D08A53 limit "slow motion moves in even 1-unit steps", which a slow bob or a
settling sway shows as stair steps (the D53G gestures too).

Change (`view_locked_transform`): the private matrix the view lock passes to
`0x800292a0` has its rotation scaled by k and its offset given in 1/k units.
The private camera rows stay the projection scale, so the GTE sees camera
space k times larger: the projection (x/z, y/z) is unchanged and positions
are 1/k of a unit apart. k = 4 (quarter units); a call whose rotation would
reach the 16-bit Q12 limit (7.9) uses less (`subunit_limited` counts it; 0
seen). Lighting uses the real weapon matrix, which is unchanged. The depth
the viewmodel sorts by grows by k for all of it together. A/B
`DNTTK_FP_SUBUNIT=1`.

L, same poses with k 1 and 4 (`rest` and `look` cases, motion off, 10
weapons): identical pictures except edges and texture lines moving by under
a unit (0.26-5.7 mean colour difference); no missing parts, no lighting or
fog change, the muzzle flash, beam and kick order unchanged.

## 3. Evidence

Lab `recomp/analysis/d36n/` (local): `probe.py` (from the D08A53 probe:
private card copies, offscreen at 180 Hz on the GTX 1080 Ti, debug port,
player files hash-checked after every run) with cases `stills`, `grip`,
`rest`, `look`, `firelog`, `clip` (Xvfb + ffmpeg, 60 fps), `walk`, `hop`;
`amp.py` (peak-to-peak travel per phase), the D08A53 `jitter.py` and
`series.py`; `sheet.py`.

L, the drawn grip (`gte_weapon`) at 2560 wide, peak-to-peak over the middle
of each phase (`amp.py`), Desert Eagle / Shotgun:

| Phase | Side to side | Up and down |
| --- | --- | --- |
| Standing (breath) | 0 | 2-3 px |
| Walking | 34-47 px (1.3-1.8% of the width) | 8-10 px (before the final dip increase, 3.0 to 4.0 units) |
| Running | 55-70 px | 15-30 px |
| Running, jumping and turning | 35-45 px | 54-87 px |

Duke 3D's own sway is about 6.7% of the width; modern games about 1%.

Smoothness, all 14 weapons (`walk` and `hop` cases, 180 Hz,
`full-jitter.txt`): every phase 0.88 px rms or less against a 9-redraw mean,
and at most 8.7% reversing steps (D08A53's vibration was 50-70%). What
reverses is the tracer's own re-projection noise, about 0.2 px, in slow
phases; the old 2.5 px stair steps are 0.6 px. Before the quarter units,
the same motion measured 1.0-2.1 px rms. A motion-off A/B of the hop case
(`DNTTK_WEAPON_MOTION=off DNTTK_FP_SHOTGUN_GRIP=0`) gave the same tracer
noise (46-58% on the re-projected `weapon_hand`, 1-2% on `gte_weapon`), so
the traced reversals while turning are the measurement, not the gun.

Stills of every weapon (`fin-all-a.png`, `fin-all-b.png`, `fs-crossbow.png`,
local) at rest, walking, running, through a jump and turning: the Knife,
Axe, Crossbow, Desert Eagle, Shotgun, Gatling, RPG, Pipe Bomb, Holy Hand
Grenade, Dynamite, Energy Weapon, Flamethrower, Buffalo Rifle and Freezer
all draw with both arms connected; the Energy Weapon's support arm stays off
the bottom edge. Shotgun firing (`firelog`, `grip`): the fist stays on the
fore-end through clips 24-26 and 23, the rack visible.

Vanilla (`MODE=vanilla`): the original third-person view, no first-person
point traced. Player saves, cards and profile unchanged after every run.

Pictures and clips (local, `research/before-after/D36N/`): `shotgun-grip.png`,
`shotgun-pump.png`, `shotgun-before-after.mp4`, `eagle-before-after.mp4`
(Xvfb software rendering at 60 fps: shape, not pacing).

## 4. Tests

- Native `ttk-controls-test`: 170 groups pass. New D36N group: look lag
  soft-capped and settling without a wobble, walk and run bob, run carry cut
  by a shot, jump lag, fall float, landing dip and recovery, OFF and Vanilla
  still, the Shotgun grip hold and rack at any setting. The D08A53
  view-locked group had compared 0 with 0 (the fixture's camera had no view
  rows); it now gives the camera rows and checks the GTE gets the same
  rotation and translation times one factor, 4.
- `ttk-near-test`, `ttk-input-test` pass. Python 293 OK (2 skipped): profile
  schema 84 with a new `WeaponMotionTest`, the console table has `fpgrip`.

## 5. Limits

- Offscreen runs and Xvfb clips; the playtest on the real display at 180 Hz
  is the acceptance (feel, sizes, the grip from the eye).
- The arms are still the native third-person animation from the eye: the
  motion moves the assembly, it does not re-author poses. Grips other than
  the Shotgun keep their D36H/D48 placements.
- Crouch, swim, climbs, contact actions and the one-handed raise ease the
  layer out; they keep their own paths.
- The muzzle flash and held items are not view-locked (D08A53): they keep
  whole-unit placement and now sort in front of the finer viewmodel depth.
- The bob tempo is the D17V view bob's fixed tempo (about 1.5 steps a
  second walking and running), not the leg animation's footfalls.

## 6. Round 2 (2026-11-28): playtest fixes

U, the user's playtest: "the new weapon motion looks fantastic". Two faults:

- **Gatling and Buffalo Rifle right wrist twisting** after switching to
  them while running without stopping (wheel, from slot 4 with `dnmonsters`
  and `dnstuff`, running loops and jumping); it stopped for good after one
  stop.
- **Energy Weapon:** "tearing on the back ... I don't know if Duke's hand is
  meant to be there like a handle", and parts of the gun's side popping in
  and out.

### 7. A weapon first drawn on the move had no roll lock (pre-existing, D08A53)

S: the D08A53 lock learns each slot's rest roll only while Duke stands still
with the weapon; until then nothing holds the segment and the native run
twists it. The lab always stood after a switch, so it never saw this. L, new
`runswitch` case (switch while running, keep running, jumping and turning):
the Gatling's right forearm rolled over 19 degrees and the upper arm 26; after
one stop, 0.

Change: per-slot seed rest rolls (`arm_roll_seed`) for the four segments of
all 14 weapons, the values the still sampler learns. They are the D08A53 idle
traces, confirmed in a fresh session per weapon (selected alone, then still;
within 0.2 degrees, the Desert Eagle 0.7). A first table taken while
cycling through the weapons in one session had learned transitional values
(up to half a turn off on the Gatling's upper arm) and was discarded, as was
a version that learned during the draw (the draw left the Gatling's upper arm
167 degrees from rest). The still sampler refines the seed as before. A/B
`DNTTK_FP_ROLL_SEED=0`; developer `DNTTK_FP_ROLL_LOG=1`; the learned values
are in `fp_motion.rolls`.

L, `runswitch`: every segment of the Gatling, Buffalo, Shotgun and Freezer
0.0-0.6 degrees while running from a fresh switch (Energy Weapon upper arm
8 degrees, smooth); at the first stop the arm moves at most 5.6 degrees
(Buffalo upper arm, eased over about 0.3 s).

### 8. The Energy Weapon's mesh at another sub-unit factor

S/L: the Energy Weapon and Freezer meshes are drawn at display scale 1.4
(the RPG 1.2). At factor 4 their transform passed the 16-bit limit, so the
guard dropped the mesh call alone to a coarser factor; the mesh then sorted as
nearer than the hand and arms, and the rear housing covered Duke's right
hand (the grip the user saw as a handle), flickering as the order changed.
Change: one factor per frame for every view-locked part, chosen in
`prepare_display_matrix` from the view's projection scale and the weapon's
display scale (`view_subunit_frame`, `fp_motion.subunit`).

L, rest pictures against whole units: Energy Weapon 12,710 to 5,923 changed
pixels (mean 2.85 to 1.23), Freezer 19,526 to 4,163 (5.72 to 0.90), RPG
13,231 to 7,509 (3.08 to 1.54), now the same edge-shift level as the other
weapons; the hand shows at the back of the Energy Weapon again. The first
pass had put those larger differences down to the scene; they were this.

Also: the controls and input debug JSON buffers are larger (20 and 24 KB)
for the motion fields.

Tests: `ttk-controls-test` 170 groups, `ttk-near-test` pass. Lab `walk` and
`hop` for the Gatling, Buffalo and Energy Weapon: at most 5.5% reversing
steps, roll ranges at most 2.1 degrees running and hopping.

## 9. Round 3 (2026-11-28): the Energy Weapon's outer right face

U, the user's video (`research/screencaps`, local): "part of the outer
right-most section down the barrel of the energy weapon continues to
disappear ... that seems to be the only issue left!"

L, still sweeps with the gun held in poses (`posesweep`, `fpidle pose`,
motion off): the housing's outer right face, beside the green panel, draws
from muzzle yaw -2 to +4 degrees and vanishes at -3 and -4; with the muzzle
3 degrees down, or the top rolled 3 left, it goes at a yaw of only -1.5;
with the rotations together, 3 units of travel to the right is enough. The
model has nothing behind that face, so the street shows through: the ready
pose sits about 2.5 degrees from the angle where it turns away. The look
lag (to 3.2 degrees), the bob's yaw and the run carry (muzzle down, top
left) all crossed it.

Change (`motion_slot_shape`, Energy Weapon only): the motion's muzzle-down
pitch, muzzle-left yaw and top-left roll are softly limited (tanh) to 0.8,
0.5 and 0.8 degrees, and its travel to the right to 4 units. That corner
together still draws the face at the layer's full travel (x -9 to 4, y 0 to
20, z -5 to 5). Its other directions, and all its up, down and leftward
travel, are unchanged.

L, running, jumping every 10 updates and turning hard both ways, 247 updates:
pitch at least -0.80, yaw at least -0.50, roll at most 0.75 degrees, x at
most 2.8 units; the face draws in every frame. `ttk-controls-test` 170
groups pass.

Not changed: the D53G Energy Weapon idle gestures turn the gun much further
(inspect to 26 degrees) and may show that side for a moment, as before.

## 10. Closeout

User, after round 3: "i can confirm it appears good on my end right now";
on the limit: "i would have preferred a real fix that would allow us to view
the weapon at any angle", having seen "the disappearing chunks happen in the
same spot in the idle animations". Accepted: "lock this in and close it all
out". The round 3 limit stays as a safety net until
[D36O](../jobs/first-person.md#d36o---first-person-weapon-model-repair)
repairs the models (survey of every weapon first) and removes it.

D36O (2026-11-28) found the cause, a folded quad culled by its first half,
and removed the round 3 limit; see [note 573](573-d36o-weapon-model-survey.md).
