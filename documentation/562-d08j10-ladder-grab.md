# D08J10 - Onto a ladder through the climb (LADDER CLIMBING)

2026-11-26 to 2026-11-27. Done (user-accepted 2026-11-27: "great!! the best yet"; follow-up D08J11, a one-armed hang while armed).

## The complaint

User, 2026-11-14 playthrough: Duke climbs unusually slowly at the start of a
ladder, until his feet reach it. Proposed: ADVANCED, LADDER CLIMBING, MODERN
full climbing speed from the grab, CLASSIC the original start.

First candidate (same day): the entry clips played 2.5x faster. User: "i dont
like the animation at all here unfortunately. it's just the low rungs anim
speeded up. i think duke could be animated properly climbing more like his
normal hand climb style like muscling himself up, rather than reusing this
animation just speeded up." That candidate is gone; this note describes its
replacement.

## Measured (original)

First alley ladder, LEVEL00 object type 46 at (9225, -10239, 13769), yaw
1024, climbed on its west face (attached x 9025). Offscreen, empty cards, the
player's settings copied. The debug port's frame counter runs at 60 a second;
the player update (and so the animation) at 30.

| Entry | Clips (mode) | Fields | Seconds | Height |
| --- | --- | --- | --- | --- |
| W + E at the ladder from the floor | 185 (8, after 0) | 64 | 1.07 | 266 up |
| Running jump with E (tap Space) | 148 (3) then 156 (3) | 32 + 24 | 0.93 | catch at -10013, 230 down as the hands take the weight, 90 up |
| Standing jump with E | 147 (3) then 156 (3) | 34 + 22 | 0.93 | |
| Climb steps afterwards | 188 / 189 (3) | 22 each | 0.37 | 299 / 219 up |

The motion is the clips' root motion. At the hand-over to the climb (end of
185 and of 156) x snaps by up to 145 units onto the climbing line, 200 from
the panel; the rest poses 186/187 snap the same way. A catch below a high
ladder bottom hangs in 207, which waits for W; W then plays 210 into 156.

## Change

`recomp/src/ttk/ladder_grab.inc`, once per player update at the animation
update 0x8005a210 (after the game's mode handler for that update, before the
animation runner). Modernized with LADDER CLIMBING MODERN, plain ladders only
(type flags 0x200 without 0x500):

- **Ground mount.** On the first update of a 185 whose previous update was on
  the floor (mode 0), Duke is attached as the original catches attach him
  (+0x17c the ladder, box +0x180 = 0, sub-state +0x1c4 = 0, mode 3/3) in the
  climb step 188, frame 0, at his standing height. The step lifts him onto the
  rungs: he reaches up and pulls himself up in his normal climb. The game's
  mode history (+0x22d) already reads 8 at that point, so the host keeps the
  mode it saw at the last update; the step-off at the bottom is a 185 after
  mode 3 and is left alone.
- **Catches.** 147 and 148 play at their own pace while the body swings down
  under the hands. Once the root stops going down (from the second update;
  at most 10 updates) Duke starts 188 from that low point and is lifted on top
  of the step to where the original's climb would have started (classic,
  measured: 102 below the catch height after 148, 67 above it after 147), so
  his hands keep their rung while he muscles up. 156 and 210, if reached,
  start 188 at once, 85 higher.
- **Ease onto the line.** Over 3 updates (smoothstep; 8 in the second candidate) the host moves x/z from
  where Duke is to the climbing line and turns him to face the panel, instead
  of the original's snap; height changes are added in steps on top of the
  step's own root motion. If the original puts him in a rest pose (a climb
  refused near the top or under an enemy there), the original's placement
  stands and the ease stops.

The game interpolates from the frame shown into 188's first frame, so the
start blends from the standing or hanging pose. Everything after the first
step is the original handler: the climb, the top exit, the step-off. Not
changed: the D08U top mount (156 under its own host blend), the hang 207,
poles, chains and climbing walls.

Developer A/B: `DNTTK_LADDER_SETTLE` (updates, default 3),
`DNTTK_LADDER_CATCH_MAX` (default 10).

Setting: OPTIONS > ADVANCED > MOVEMENT > LADDER CLIMBING (MODERN default /
CLASSIC), live on CONFIRM; profile schema 82 (`ladder_climbing`); `run.py
--ladder-climbing modern|classic`; Vanilla always classic.

## The legs (third candidate)

User, 2026-11-27, on the second candidate: "ok the top part of his body is
great!! but now you need to sort the legs out, because hes miming as if he's
on the rungs when hes not ... refine the lower half until his legs get on the
ladder".

Measured offline (`ttk_anim.py` poses of Duke's clips, `recomp/analysis/d08j10/feet.py`)
against the classic climb: the climb puts the toe joints on rungs at floor -
234 - 259 n, and reaches about 200 forward of the pelvis, so the rungs are
under the toes only on the climbing line. The original 185 places Duke on the
ladder's centre 275 from the panel (snapping him up to about 200 sideways
after an angled approach), so for the first step his feet climbed in the air
75 short of the ladder. Note: joints 1-4 are Duke's left leg, 5-8 his right
(x negative is his left).

- **Step-in legs.** `recomp/tools/local/ttk_ladder_entry.py` builds host
  extra clip 61 (in `ttk-anim.pack`): four keys, one per player update of the
  step's opening hold, while the ease brings the pelvis the 75 units onto the
  line. The right ankle is solved with two-bone IK onto its standing position
  (planted, 15 above the floor throughout); the left leg lifts from the
  standing pose onto the bottom rung (188's own first-frame leg, the toe at
  226 over the floor and 9 past the panel face), with the knee raised mid-way.
  `--render DIR` draws side and quarter sheets with the floor and rungs.
- **Hand-back.** `ladder_grab.inc` writes those keys over joints 1-8 at
  0x800411b8 (after every channel blend; pelvis, root and upper body are never
  written), then blends to 188's live legs over 4 updates: they keep the left
  foot on that rung and bring the right one up from the floor.
- **No snapped frame.** The original sets 185 after the animation update, so
  the entry was converted one update later, after a frame drawn at the
  original's snapped placement. The conversion now also runs at 0x800411b8 of
  that same update, from where Duke stood (within 400 units); the ease starts
  there, so an angled approach turns and steps in instead of jumping.
- The ease takes 3 updates (was 8) for the catches too, so a catch's lift is
  done before 188's feet start climbing.

## Hanging legs after a catch (fourth candidate)

User, 2026-11-27, testing at level 0 room 7 (4346, -9726, 13754), under the
second alley ladder (bottom about 2060 above the floor; caught with a jump
and E): "unfortunately i see no change at all. when duke grabs the ladder and
he hasnt got his whole body on it yet, his legs climb up invisible rungs.
basically the legs can dangle or sway naturally until they are high enough
for him to put his feet on the rungs". The third candidate only changed the
ground mount; after a catch 188's legs climbed the air below the ladder.

- After a catch (and 156/210) each leg is held on the original bottom-hang
  loop 207 (its four frames cycled every 1.3 s, a slight sway) and eased in
  from the catch pose over 3 updates.
- Each leg swings onto 188's leg over 4 updates, and stays there, once the toe
  188/189 would place it at is no lower than the ladder's lowest rung: toe
  height bottom - 234 (+30), the bottom read from the ladder's collision box.
  The toe heights below the pelvis per keyframe of 186-189 (left ROT 4, right
  ROT 8) are a table measured with `feet.py table`, interpolated with channel
  0's own frame position.
- Measured at the user's spot (standing jump with E, then W): catch 147 at
  -10992, legs hanging through the first steps, the right leg on the climb at
  about -11500, the left a step later, then the original climb to the top
  exit 190. A catch near a floor ladder hangs only until the lower foot
  reaches (or the climb starts on the original lattice).

## The ledge hang at rest, a slight sway when climbing (sixth candidate)

A fifth candidate solved a straight-down pose (host clip 62); user: "no, the
legs are now unnaturally close together like a pencil dive, and duke's ass is
twisted to the side ... what i really wanted was a natural, human hang ... the
exact hang that duke should do is when you mantle something but leave it
idle ... this hang while idle, and a slight sway while climbing" (reference:
`research/screencaps/Video_2026-11-27_01-34-43.mp4`, the ledge hang 149).
Clip 62 is gone.

- The ledge hang 149 and the bottom hang 207 share one legs clip (249), so the
  sway already had the right legs; the twist came from the pelvis. The climb
  clips roll the pelvis 10-12 degrees (187 q z 375, 188 frames 1-2 362/433 of
  4096), and the hang's legs written under it pointed to one side.
- While hanging, the pelvis is now the hang's own (149's four frames, 150
  ticks each, cycled every 1.8 s: its slight idle), eased in from the catch
  over 3 updates, and the chest is turned back by the same amount so the
  climbing upper body keeps its world pose exactly (as the D12E1 kick keeps
  the aim). During a climb step (188/189) 35% of the climb's own pelvis roll
  comes through, eased over 300 ms: the slight sway with the pulls. As the
  legs reach the rungs the pelvis goes back to the climb's.

## Measured (after)

| Route | MODERN | CLASSIC |
| --- | --- | --- |
| Alley, W + E from the floor | 185 shown 1-2 fields, 188 lifts from -9739; first step ends -10038 | 185 for 64 fields, climb from -10008 |
| Alley, running catch | 148 for 18 fields, 188 from -9792 lifted 119: 189 at -10210 | 148 + 156 56 fields, 189 at -10210 |
| Sewer (room 68), standing catch | 147 for 6 fields, 188, 189 at -670 | |
| Sewer, both faces, W + E | climb on at once | |
| Sewer, W held to the top | top exit 190 at -4871 after 478 fields, stands at -5615 | 190 at -4858 after 524, stands at -5615 |
| Alley, S down to the bottom | step-off 185 from -10000, floor -9737 | from -10005, floor -9737 |

Third-person (alley, room 68 sewer from an angle) and first-person frame sequences were reviewed offscreen
(`recomp/analysis/d08j10/shots/`): from the floor Duke reaches up and steps
onto the first rung in the climb; from a jump he catches, hangs, then pulls a
knee up and climbs on; first person shows the hands taking the rungs with the
D35E steady eye.

## Tests

- `ttk-controls-test` (owned EXE, LEVEL00 guard fixture): the D08J10 group
  covers the ground mount (attachment, sub-state, ease to the line and the
  facing), the step-off, the running and standing catch hand-overs, 156/210,
  and the refusals (207, a pole, no owner, CLASSIC, Vanilla). Whole suite
  passes.
- `python3 -m unittest discover -s recomp/tests/local`, including
  `LadderClimbingTest` (migration 81 -> 82, CLI, env, Vanilla, validation).

## Limits

- Offscreen only (llvmpipe, dummy audio): whether it looks seamless on the
  player's display, in both views, is the user's playtest.
- The alley ladder has an enemy on its platform that holds Duke below the top
  in both modes (D08U2); top exits were measured on the sewer ladder.
- The D08U top mount and the 207 -> 210 hang climb were not reproduced live;
  the top mount is excluded in code, 210 is covered by the native test.
- The step-in legs assume the original grab distance (275 from the panel,
  the same on every ladder measured) and the rung heights above; the right
  foot slides sideways a little after an angled approach (the ease turns and
  moves Duke in 0.1 s). The catches keep 188's legs (the feet swing onto the
  rungs during the opening hold, as the original's 156 did).
- One single-field height outlier (+125) appears in the probe's RAM samples
  late in the first step; the step otherwise rises a steady 29-30 per
  update. Taken as a read during the update, not a drawn position.

Private probes and captures: `recomp/analysis/d08j10/` (local).
