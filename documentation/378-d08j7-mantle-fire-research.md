# D08J7 - Shooting while mantling: research report

2026-11-04. Research report (the job's first deliverable), then D08J7A,
accepted and closed 2026-11-04 (see Closeout), then D08J7B and D08J7C
(D08J7C user-accepted 2026-11-04 at revision 3, D08J7B closed with it; the
last sections).

## Request

User, closing D08J6A: "is it gonna be possible to add in the ability to
shoot when mantling, like that's just huge because currently it prevents you
from shooting whatsoever and you might take quite a lot of damage during
that essentially offenseless time - so if we could also add it in for
mantling". Duke stows before he grabs a ledge (D08J/D08X E stow) and cannot
fire through the hang, the pull-up or the mantle. Vanilla unchanged.

## How current games handle it

| Approach | Examples | Fit for TTK |
| --- | --- | --- |
| Sidearm while hanging from a ledge, one hand on the lip | Uncharted 2-4 (pistol from ledge hangs), Tomb Raider 2013+ (sidearm on climbs and ropes) | Closest to the request, and the same rule D08J6A uses on climbs |
| No shooting during the pull-up itself; the pull-up is short | Uncharted, Tomb Raider, The Last of Us (roughly 0.5-1.0 s over a lip) | The defenceless window is kept short rather than made armed |
| Fire buffered through the mantle, weapon up the moment the feet land | Common in modern third-person shooters (input is queued, not dropped) | Fits TTK's original deferred weapon request (below) |
| Cancel the pull-up back into the hang | Uncharted (drop back), Tomb Raider (hold position) | Useful: a player caught mid-climb can stop and fight from the hang |

The professional pattern is: armed in the hang (sidearm, one hand on the
lip), no shooting while both hands push Duke over the lip, a short pull-up
and a weapon that is ready the moment he lands. The player may decide to stay
in the hang and fight instead of climbing on.

## What the original game already does

Evidence: private offscreen lab `recomp/analysis/d08j7/` (local only),
accepted D08J6A executable
`c5c6a30b010df99ad7981cf3452d6105ba02811d2cc2a050e0bff7b6abed9a56`, 60 FPS
setting, dummy audio, port 9612, private copies of the D35G alley fixture
(slot 0 saved platform hang, slot 6 before the dumpster and platform climb),
holster rebound to Z in a private profile only. Player saves, cards and
settings were hash-checked unchanged after every run.

### Phases and timings (W + E walk-in, slot 6)

| Phase | Animation, mode | Time | Hands |
| --- | --- | --- | --- |
| E stow on the approach (armed start) | upper 5 -> 6 -> 0 -> gait, mode 0 | about 0.2 s, while walking | weapon away |
| Climb up the face | 139, mode 8 | 0.47 s | both on the lip |
| Pull-up over the lip and stand | 140, mode 8 | 1.74 s | both pushing |
| Hang between the two climbs (platform only) | 149, mode 6, 2-4 frames | under 0.1 s | both on the lip |
| Saved hang, W released (slot 0) | 149, mode 6 | as long as the player waits | both on the lip |

So one ground climb is about 2.2 s without any action, and the dumpster then
platform route about 4.5 s. Every walk-in climb with E is the full climb
139 -> 140; the dumpster climb never hangs, the platform climb passes through
the hang for a few frames because W is still held.

### Live probes

| Test | Ledge hang 149 (mode 6) | Climb 139 / pull-up 140 (mode 8) |
| --- | --- | --- |
| Hold left click holstered | nothing | nothing during the climb; Modernized fire-draw (D08E) draws once on the ground |
| One Circle tap | draws: upper 149 -> 3 -> 0 -> 6 -> 5, equipment 2, about 0.4 s, still hanging | ignored, the tap is lost |
| Original weapon request (`+0x224 |= 4`) | draws at once, as the Circle tap | kept pending through the mantle and served on the first ground frame: weapon up about 0.2 s after landing (both from 139 and from mid-140) |
| Hold fire, armed | real rounds: 8 (ammo 200 -> 192); upper 9/11, flag 0x100; not adapted to the camera (aim `shots` 0) | - |
| Armed: A or D (shimmy), W (pull-up), S, Space (drop) | all refused, Duke stays hanging | - |
| Second Circle tap | stows: 5 -> 6 -> 3 -> 149, about 0.4 s | - |
| W after stowing | pull-up 140 and on to the platform | - |
| Holstered: A, Space | shimmy 152; Space lets go (108), lands | - |
| One `hurt=` hit through Duke's damage handler | not repeated (see Limits) | during 139: health word `+0x32` 9250 -> 9249 (armour 100 absorbed most), the climb and pull-up ran on unchanged |

Pose (third person, `hang-armed.png`, `hang-fire.png`): with the pistol out
the original keeps **one hand on the lip** and holds the pistol low in the
other, pointing into the wall in front of Duke. That is already close to the
Uncharted hang-and-aim; what is missing is the aim toward the crosshair.
First person (`fp-armed.png`, `fp-fire.png`): the ground pistol viewmodel
appears at the lower right and fires into the wall; no hand is seen on the
lip.

Static: the hang code around `0x8004c8b8` (mode 6, sets the upper-table
entries 149/152/153 bit 0x40 again, D08X) reads the buttons through the
player's history word `0x800d1ce8 + 4 * +0x233`. Which test refuses the
armed shimmy, pull-up and drop was not located; the climb dispatcher's
upper-table flag 8 test (note 377) is the likely equivalent, not confirmed.
Note 119: the skeleton build's one-arm aim of record 5 covers modes 3/6/7,
so the original authors planned hang shooting.

### What is missing

1. **Input.** Left click does nothing in a hang; fire during the climb is
   dropped (unless held until landing) and a Circle tap during it is lost.
2. **Aim.** Hang shots go along Duke's heading into the wall; `shot_ready()`
   admits neither the hang nor the climb.
3. **Leaving the hang armed.** Shimmy, pull-up and drop all refuse while
   armed; only a second Circle tap frees Duke. A Modernized player has no
   reason to know that.
4. **Length.** The pull-up is 1.74 s, roughly twice the modern norm, and the
   weapon then still has to be drawn after landing.

## Recommendation

Do not make Duke shoot while both hands push him over the lip (no game the
user cited does that, and TTK's mantle hands are visible in both views).
Instead make every part of the mantle either armed or short:

### A. Hang and fight (ledge hangs, mode 6)

- Left click in a ledge hang (149, settled grab 147/148 included) draws the
  sidearm, swapping out another selection, exactly as D08J6A on climbs (the
  same lease rules, swap and returns). Held fire fires; a short click draws
  only. Rounds go to the crosshair within the hang arc (shot adaptation).
- W (pull-up), A/D (shimmy), S and Space (drop) while armed spend the
  original stow tap at 2x speed and then do what was asked, with no second
  press. The swapped weapon comes back on the ledge top or on the ground.
- Pose: keep the original one-hand-on-the-lip armed hang for now; the aim
  toward the crosshair (gun arm and chest turn) belongs with the D08J6B
  hang-and-aim pose work.

### B. Fight your way up (walk-in and airborne climbs, mode 8)

- **Fire stops the climb in the hang.** Fire pressed during the climb up the
  face (139, before Duke's chest reaches the lip), or during the brief hang
  between climbs, stops him in the ledge hang (mode 6) instead of pulling up,
  and A applies. Releasing fire and pressing W climbs on.
  Needs a feasibility check: entering the original hang from 139 at the
  lip height the catch uses (the D08X settle at ledge top + 456), on
  geometry ledges and on climbable objects (the dumpster climb never
  hangs; a crate hang is mode 7, released at once by D08X).
- **Ready on landing.** Fire held or clicked during the pull-up (140)
  queues the original weapon request, served on the first ground frame (the
  original already defers it), and the weapon drawn before the E stow is
  the one that comes back (not the sidearm). Draw on landing at the fast
  rate D08J6A uses on climbs.
- **Shorter pull-up (Modernized only, decision below).** Play 140 faster,
  about 1.0-1.2 s instead of 1.74 s, through the same upper and lower
  track timing the original animation runner uses, with the root motion
  unchanged so Duke ends where he does today.

### Not recommended

- Firing during 139/140 with the ground upper body over the mantle legs:
  this is the legs-only pose D08Z2 removed ("some static pose with his legs
  and looks weird"), now with both arms off the lip.
- Leaving the weapon out through the E stow: the original reach and mantle
  refuse an armed Duke (D08X), and D08Z2 keeps the upper track on the
  climb; changing that touches every grab rule.

## Overlap with D08J6D

D08J6D (monkey bars and ledge hangs) depends on this job. If part A ships
here, D08J6D keeps the ceiling monkey bars (mode 7) and object hangs and
reuses part A for them. Their pose for all hangs stays with D08J6B/D08J6D.

## Staged plan

1. **D08J7A - Hang and fight.** Part A: hang lease (Modernized, normal
   gameplay and camera ownership, mode 6, a ledge hang clip, settled, not a
   crate), fire-to-draw through the original request, sidearm swap and
   returns, view-aimed hang shots, automatic stow on W/A/D/S/Space. Native
   tests for the lease, refusals and Vanilla. Test at 60/120/180 on the D35G
   saved hang, the slot-12 sewer wall catch (D08X) and the user's saves.
2. **D08J7B - Fight your way up.** Part B: fire during 139 stops in the hang
   (after the feasibility check), queued draw on landing with the weapon
   drawn before the E stow, and the shorter pull-up if chosen. D08Z2 handoff,
   D35G first person and the D08X catches regression-checked.
3. Pose and first person (one hand on the lip, gun arm at the crosshair, in
   both views) with D08J6B and D08J6C, sharing their Blender work.

Acceptance needs enemies near a ledge: enemies cannot be spawned yet, so a
user save state where enemies attack during a mantle is requested.

## Decisions for the user

1. Fight from the hang (part A), with the D08J6A sidearm swap: yes
   (recommended) or another weapon rule.
2. Fire during the climb up the face stops Duke in the hang (recommended),
   or the climb always completes and only the landing draw is added.
3. Shorter pull-up in Modernized: about 1.0-1.2 s (recommended), keep the
   original 1.74 s, or another length.
4. Leaving the hang armed: W/A/D/S/Space stow automatically and then act
   (recommended), or a drop (S/Space) lets go with the weapon still out.
5. Part A in this job and D08J6D keeping monkey bars and object hangs
   (recommended), or all hangs in D08J6D.
6. A save state where enemies attack during a mantle or near a ledge hang.

## User decisions (2026-11-04)

User: "1 - yes, 2 - let's talk about this one before moving forward, 3 -
recommended, 4 - need more info again, 5 - recommended, 6 - check my slot 9
for this, you can jump directly up and hang, dnkroz is on".

- 1: hang and fight with the D08J6A sidearm swap.
- 3: shorter Modernized pull-up, about 1.0-1.2 s.
- 5: D08J7 covers ledge hangs; D08J6D keeps monkey bars and object hangs.
- 2 and 4: discussed below; then, from the options quoted back: "2b. Fire
  never interrupts. The climb always finishes, with the shorter pull-up you
  chose, and fire pressed during it raises your weapon the moment Duke lands"
  and "4b. Drop with the pistol still out. Duke lets go at once and lands
  ready to shoot".

### User slot 9 (private copy, runtime slot 8)

Copied byte for byte to `recomp/analysis/d08j7/cards-slot9` (local only);
the player's files were hash-checked unchanged. Duke stands armed under a
ledge, facing the wall. A vertical jump with E held (after stowing) catches
it: 97, grab 147 (mode 6), hang 149 after about 0.5 s. A Circle tap draws in
the hang (0.4 s) and the rounds go into the ledge face. A LARD pig cop stands
on the lip directly above Duke (`slot9b-fire.png`); health stayed 100 with
dnkroz. Pressing E only 0.3 s before the jump while armed did not stow in
time: no catch, landing 105 (the known D08X limit).

User correction: "slot 9 has enemies to the left, there just also happens
to be an enemy up the top too ... sorry, to the right!". The main threat is
to Duke's right along the wall; the pig cop on the lip is extra. Turning the
camera about 90 degrees right in the hang (`slot9c-right*.png`) did not frame
them in the private run; the armed firing pose there holds the pistol arm out
level with one hand on the lip, and the rounds still followed Duke's heading
(aim `shots` 0).

So the enemies are beside Duke along the wall and above the lip, not in
front of it.

## D08J7A - hang and fight (accepted)

### Behavior (Modernized only)

- In a settled ledge hang (149, mode 6) held left click draws the sidearm
  (slot 4), swapping out another selection as on climbs (D08J6A); held fire
  fires at the crosshair, a short click draws only.
- Armed, W (pull-up) and A/D (shimmy) spend the original stow tap at the
  D08J6A 2x rate and then act; the swapped weapon returns once holstered.
- Armed, S or Space lets go at once with the pistol out; the swapped weapon
  is drawn back once Duke is settled on the ground.
- No firing during the climb over the lip (decision 2b, part of D08J7B with
  the queued landing draw and the shorter pull-up).

### Why the armed hang refused every move

Probe (`gate.py`): with the pistol out, writing the hang pose 149 to the
upper track made S release at once (108). So the hang's moves wait for an
upper body without a weapon pose, as the rung handler does on climbs; the
hang's own pose/button selector in `0x8004b644` (the 149 cases around
`0x8004c8e4`) has no equipment test.

### Implementation

- `climb_fire.inc`: `hang_weapon_ready()` (Modernized, normal gameplay and
  camera ownership, mode 6/6, animation 149, identity) joins the climb lease
  (`climb_weapon_ready()`), so fire-hold, draw, stow, shot and the upper-track
  boost cover the hang. The draw request and the swap return are shared
  (`climb_fire_request()`): at the climb dispatcher as before, and once per
  player update in a hang (`hang_fire_update()`). Armed drop: for one player
  update the upper track takes 149 (`+0x7c/+0x7e` = 0), the original releases
  through Down, and at the next update the kept 0x14-byte weapon block comes
  back (in the fall, or still hanging if the original refused); not after a
  savestate load, a changed equipment byte or a weapon pose of its own. Debug
  `controls.climb_fire`: `hang`, `hang_drops`, `hang_drop_restores`.
- `pc_input.cpp`: armed, the hang is no traversal state (that lease needs a
  holstered weapon), so S/Space in an armed hang send original Down
  explicitly and keep the D08T8 Space consumption; they never spend the stow
  tap. W/A/D stow through the existing D08J6A path.
- Ground return of the swap (`climb_fire_ground_restore`): if the original
  reports the swapped weapon unusable for a moment, the return now waits
  instead of being dropped. Seen in the first runs, then traced to the test
  fixture (a reload removed the cheat-given shotgun); kept because a held
  return is harmless (a holster or another selection still ends it).
- No new hook address, guard, generated code, framework or codegen change;
  existing savestates load. Vanilla: everything is behind `input_modernized()`.

### Verification

Final executable SHA-256:
`0ccd15e8ba4fd9ff24f43362cd87e0be416b6c19b25837517d177b8074bb025c`.
Private lab `recomp/analysis/d08j7/` (offscreen, dummy audio, port 9612,
private copies only; player saves, cards and settings hash-checked unchanged
after every run).

| Route (saved platform hang, shotgun selected) | 60 | 120 | 180 |
| --- | --- | --- | --- |
| Hold fire 1.2 s: sidearm drawn, rounds, view shots | 5, 5 | 5, 5 | 5, 5 |
| Then A: holstered, shimmied 481, shotgun selection back | yes | yes | yes |
| Fire, then W: holstered, pull-up onto the platform | yes | yes | yes |
| Fire, then S: let go armed, landed, shotgun drawn back | yes | yes | yes |
| Fire, then Space: the same | yes | yes | yes |
| Short click: drawn, no round | yes | yes | yes |

Fire to equipment out 0.15-0.17 s, first firing pose 0.22-0.23 s (all three
settings). First person (60): the same outcomes; the rounds hit the wall at the
crosshair (`final-fp-armed.png`). User slot 9 copy: vertical E catch, camera
turned right, fire drew the sidearm and all 9 rounds were view-adapted
(aim `rejected` 0); Space dropped and landed armed. Vanilla (`--vanilla`,
the same hang): left click and Space do nothing, as in the original. D08J6A
regression on this binary (alley ladder, D35F chain, D35I1 wall): the same
outcomes as at its acceptance.

Native suites pass: `ttk-controls-test` (new D08J7A group: lease, gates
147/148/150/152/153/140, entry frame, mode 7, Vanilla, draw and return at
the player update, armed stow/shot, the drop handoff and restore, refused
drop, fire/W/holster/E blocking the drop, savestate load), `ttk-input-test`
under Xvfb (S and Space send Down with no stow tap, W still stows),
`ttk-aim-test`, `ttk-near-test`.

### Limits

- The pose is still the original armed hang: one hand on the lip, the gun arm
  not turned toward the crosshair (D08J6B/D08J6C work). Rounds follow the
  view regardless, within the existing muzzle cover check.
- Only the settled hang 149: not during the grab (147/148) or a shimmy step.
- Enemies were not seen hit: none were framed in the slot 9 runs (the
  user's enemies to the right); shots were measured by the aim counter and
  impact marks.
- An enemy on the lip above may be out of reach of the low gun hand (muzzle
  cover check), not measured.
- D08J7B (queued landing draw, shorter pull-up) is not built yet. Hang shots need an
origin and arc that clear the lip upward (Duke's head is about level with it
in the hang, ledge top + 456), or the D08J6A cover check will stop them at
the edge.

## Limits

- One fixture area (the alley dumpster and platform). The slot-12 sewer wall
  catch, crate mantles, height mantles 134-138 and water climb-outs were not
  probed live; they share modes 6 and 8 and were exercised in D08X, D08Y,
  D08Z2 and D35G.
- Only one damage hit was delivered (the test word acts once per change, and
  the later cases repeated it). A hit does not interrupt the climb in that
  case; stagger or knock-off rules during hangs were not measured.
- No enemies near a ledge; shot landing points in the hang were not
  measured beyond the aim counter and screenshots.

## D08J7B - weapon up on landing, shorter pull-up (Done, closed with D08J7C)

User decisions 2b (fire never interrupts the climb; weapon up the moment
Duke lands) and 3 (Modernized pull-up about 1.0-1.2 s).

### Behavior (Modernized only)

- Fire held or clicked while Duke climbs over a lip queues his weapon: the
  climb up the face (139), the pull-up (140), a height mantle (134-138), or
  the hang a platform climb passes through with W held (149). The weapon is
  the one selected before the E stow (a hang sidearm swap's own weapon, if
  one is pending).
- The climb never stops for it. W held in a ledge hang now climbs on through
  held fire: no sidearm draw there (D08J7A draws when W is not held).
- On the first ground frame the original weapon request draws it, with the
  draw and its opening generic blend (0/1, two-handed weapons) at 2x. Held
  fire then reaches the weapon; unarmed Cross stays off while the draw waits.
- The pull-up 140 plays 1.6x (both tracks): 104 game frames (1.74 s) become
  63-67 (about 1.05 s). Only the animation runner's time is scaled; events,
  root motion and collision stay the original's.

### Implementation

- `climb_fire.inc`: `mantle_fire_update()` once per player update (after
  `hang_fire_update()`). The queue is kept in the host during 134-139 and the
  hang; the request (`+0x224 |= 4`, selection written) is set once the
  pull-up 140 runs in mode 8, where the original keeps it and serves it on the
  first ground frame before the upper track blends out of the mantle (set one
  update later on the ground, the draw waited about 18 frames behind that
  blend). After a height mantle it is set on the ground. Not during 139 or a
  hang: the original serves the request at once in a ledge hang, which would
  stop a platform climb in its brief hang. A holster, an own weapon, an
  equipment change or anything but the ground (a fall) drops the queue.
- `pullup_advance()` at the animation runner `0x80059db0`: lower track (ra
  `0x8005a3d4`, delta at the update's sp-0x30, a3 0) and upper track when it
  plays 140 (ra `0x8005a5a8`, sp-0x2e, a3 1), each once per update, as the
  D08Z quick takeoff and crouch do. The D08J6A transition boost gains the
  landing draw (2x) through `mantle_draw_boost`, until a weapon idle pose.
- `pc_input.cpp`: the D08J6A climb hold yields to W in a ledge hang; Cross is
  masked and the D08E Circle fire-draw is held off while the queued draw
  waits (`mantle_fire_cross_masked()`). Debug `controls.mantle_fire`:
  `queued`, `weapon`, `boost`, `queues`, `requests`, `drops`, `pullup_boosts`.
- No new hook address, guard, generated code, framework or codegen change;
  existing savestates load.

### Verification

Executable SHA-256
`ca7d1a22bf575a6ee4d7a3c010338e6f6d641e698e416a6b1bfa540fb90aa9ab`.
Private lab `recomp/analysis/d08j7/` (`p7b.py`, `endpos.py`, `sewer7b.py`,
`slot9e.py`; offscreen, dummy audio, port 9612, private copies only; player
saves, cards and settings hash-checked unchanged after every run).

| Route | 60 | 120 | 180 |
| --- | --- | --- | --- |
| Alley W+E dumpster then platform, no fire: pull-up frames | 63, 60 | 64, 61 | 64, 62 |
| Fire held from 139 to landing: shotgun (selected before the stow) out after landing | 0.17 s | 0.17 s | 0.17 s |
| One 60 ms click a third into 140: shotgun out after landing | 0.20 s | 0.17 s | 0.17 s |
| Fire through the platform's second climb and brief hang: no stop, shotgun out | 0.17 s | 0.20 s | 0.20 s |
| User slot 9 copy, jump catch, then W + fire: pull-up, pistol out on landing, hang draws 0 | 67 frames, 0 | 66, 0 | 66, 0 |

"Out" is equipment 2; the ready pose follows about 0.25 s later for the
shotgun. Before: fire during the climb was dropped (the weapon came out only
if fire was still held on the ground, through D08E), and the pull-up was 104
frames. First person (60): the same outcomes. Slot-12 sewer ledge (D08V copy,
turn 180, W+E): pull-up 64 frames, pistol out 2 frames after landing, end
position identical with and without fire.

End position, A/B against the same build with the speed-up off (W+E until
140 starts, then all keys released, standing position 2.5 s later, twice
each): dumpster `[6667, -10739, 13823]` in both; platform pull-up from the
saved hang `[7262, -11763/-11764, 13735]` without and `[7264, -11762, 13749]`
with the speed-up (14 units along one axis, reproducible).

Regressions on this binary: D08J7A hang routes (`cand.py`: 5 rounds and 5
view shots, A shimmy 481 with the shotgun selection back, W pull-up, S and
Space armed drops with the shotgun drawn back, click draws only) and D08J6A
(alley ladder W, D35F chain S, D35I1 wall S) match their previous runs.
Native suites pass: `ttk-controls-test` (145 groups; new D08J7B group: queue
in 139 without a request, no hang draw with W held, request in 140, 2x
landing draw until the idle pose, height mantle served on the ground,
refusals for armed, no weapon and Vanilla, drops on a fall and the holster,
pull-up 1.6x on both tracks once per update, other clips/modes and Vanilla
unchanged), `ttk-input-test` under Xvfb (W climbs on in a hang through held
fire; Cross masked while the draw waits), `ttk-aim-test`, `ttk-near-test`.

### Limits

- Vanilla was not driven live: the test input file works only in Modernized.
  Every change is behind `input_modernized()` and covered by the native
  Vanilla checks.
- The faster pull-up moves the platform end 14 units along one axis (not
  seen on the dumpster or in the sewer); the cause (per-update collision
  versus animation time) was not traced.
- Climbing-wall and chain top exits (206, 196) and ladder exits are not
  mantles: fire there is not queued (D08J6A owns those climbs).
- No enemies were framed during a mantle; the landing draw was measured by
  equipment, pose and timing only.
- The armed hang's W pull-up without fire (D08J7A) still lands holstered.

## Closeout

2026-11-04. User: "excellent work. lock it in and close it out!". D08J7A is
Done. The mantle stage (2b: weapon up on landing; 3: pull-up about 1.0-1.2 s)
is the new job D08J7B. The runtime patch re-export was unchanged (no framework
edits). No gameplay edits, builds or launches after acceptance; no player data
touched.

## D08J7C - one-handed armed pull-up and the sidearm return (Done, user-accepted 2026-11-04)

User, reviewing D08J7B: "i think Duke could mantle up with 1 hand with the
sidearm out you know". Decisions (2026-11-04): W in an armed hang (sidearm
only) always pulls up one-handed, firing or not; the D08J7B speed; shots in
the late phase only (knee on the lip to standing); one sidearm return rule
for the armed pull-up, the D08J7A armed drop and the D08J6A armed let-go
(option C with D's cancel).

### Behavior (Modernized only)

- In a settled ledge hang with the sidearm out, W pulls Duke up one-handed:
  no holster step, firing or not. The body, legs, head and off hand play the
  pull-up 140 (1.6x, D08J7B); the gun arm keeps the sidearm's pose. Another
  weapon drawn in a hang (the original Circle draws the selection) still
  holsters first; A/D still holster and shimmy; holstered pull-ups are
  unchanged.
- Shots: refused while the chest is below the lip (pull-up frames 0-5 of 10,
  and the hang once W is held), allowed and view-aimed from frame 6 (knee on
  the lip) until Duke stands. He lands with the sidearm out.
- Sidearm return: back on the ground with a swapped sidearm out (armed
  pull-up, D08J7A armed drop, D08J6A armed let-go), Duke keeps it while the
  player keeps shooting; 0.75 s after the last fire (real time, the same at
  60/120/180) the swapped weapon is requested. Taps closer than that never
  start it. A number key, the wheel, previous/next/last weapon cancel it.
- Fire during the swap: the original switch is not interrupted (holster 6,
  draw 21 run to the end); held fire then fires the new weapon as soon as it
  is up (shotgun: first shot about 0.3 s after its draw starts, 0.84 s after
  the swap began). No host change was made there.

### Why the original refuses, and the per-channel pose

- The hang handler `0x800471ec` (anims 147-153, jump table `0x80014070`)
  starts the pull-up for 149 at `0x8004728c`: it refuses when the upper
  animation (`+0x74`) carries table flag 8 (a weapon pose), `+0x224 & 0x200000`
  or `& 0x241`; with Up in the history word it calls the pull-up test
  `0x8007d240` and writes 140 to the lower track. The equipment byte is not
  tested.
- The model mixes four channels per joint group (note 188): 0 root and legs
  (`+0x60`), 1 right arm (`+0x74`, the "upper" track), 2 left arm (`+0x88`), 3
  chest/head (`+0x9c`). The sidearm poses fill channel 1 only, so with the
  sidearm block on `+0x74` and 140 on the others, the body and the off hand pull
  up while the gun arm holds the pistol: no new animation was needed for the
  prototype. Channels 2 and 3 stay in step with channel 0 (frame offsets 30 and
  10 throughout the 1.6x pull-up).

### Implementation

- `climb_fire.inc`: the D08J7A drop handoff also serves the pull-up. W held
  (`hang_pullup_armed_ready`: hang lease, equipment 2, base weapon 4, a sidearm
  pose other than draw/stow 6/7, no request) writes the hang pose 149 to the
  upper track for one player update; the original then starts 140; at the
  next update the kept 0x14-byte sidearm block comes back (or, refused, Duke
  still hangs with it). The pull-up's first lower frame is kept;
  `armed_pullup_fire_masked()` (handoff or frames 0-5) and
  `armed_pullup_shot_ready()` (frame 6 on, camera owner) gate fire, and
  `climb_shot_ready()` admits the late phase for view-aimed shots.
  `hang_fire_update()` now runs before `ledge_pullup_retry()` (D08T8), which
  accepts the handoff update: the alley platform is a climbable object whose
  original pull-up test fails and the host retry pulls up.
- `climb_fire_ground_restore()`: waits `sidearm_return_ms` (750) after the
  last fire seen with a swap pending (`input_clock_ms`); `climb_swap_cancel()`
  from `select_weapon` on any weapon choice command, before the return check.
- `pc_input.cpp`: `hang_pullup_input_ready()` sends original Up while W is
  held in the armed sidearm hang (the pose test would miss the handoff update
  itself, where Up must reach the game); no stow tap; Cross masked while the
  handoff or the early pull-up runs.
- Debug `controls.climb_fire`: `armed_pullup`, `armed_pullup_late`,
  `armed_pullups`, `armed_pullup_restores`, `return_waits`, `return_cancels`.
- No new hook address, guard, generated code, framework or codegen change;
  existing savestates load. Vanilla: every path is behind `input_modernized()`.

### Verification

Executable SHA-256
`908d46a985d9ceb4ec56ae61ef3c655f7d7f1627d09cdf7e2a0072d39e62b3d6`.
Private lab `recomp/analysis/d08j7/` (`p7c.py`, `geo7c.py`, `ret7c.py`,
`slot9f.py`, `sewer7c.py`, `van7c.py`; offscreen, dummy audio, private copies
only; player saves, cards and settings hash-checked unchanged after every
run).

Lip height (`geo7c.py`, joint world positions `[player+0x3c]` against the
ledge top `+0x1c8`, alley platform): the chest crosses the lip between
pull-up frames 4 and 5, the knee between 5 and 6.

| Saved platform hang, shotgun selected, fire draws the sidearm, then W + fire | 60 | 120 | 180 |
| --- | --- | --- | --- |
| Pull-up game frames (third / first person) | 62 / 65 | 64 / 65 | 62 / 64 |
| Rounds in frames 0-5 / from frame 6, view-aimed | 0 / 1, 1 | 0 / 1, 1 | 0 / 1, 1 |
| Lands with the sidearm out (weapon 4) | yes | yes | yes |
| Swapped shotgun requested after fire release (pull-up route) | 0.81 s | 0.82 s | 0.80 s |
| Five taps 0.53 s apart: no swap; after the last | 0.79 s | 0.78 s | 0.78 s |
| `1` pressed 0.3 s after release: return cancelled | yes | yes | yes |
| D08J7A armed drop (S): shotgun back after landing | 1.47 s | 1.47 s | 1.47 s |

Without fire: W pulls up armed at once (no holster), lands with the pistol,
and the shotgun comes back on landing (fire released long before). Before
the Cross mask covered the handoff update, one round fired at frame 2 (the
hang's Cross reached the game during the handoff); none since. First person:
the sidearm viewmodel stays up through the whole mantle view
(`p7c-fp-sheet.png`). Side view (`p7c-side-sheet.png`): the off hand on the
lip and the pistol arm pointing forward; before the mask, the early rounds
hit the platform face.

User slot 9 copy (`slot9f.py`, jump catch, fire draws, W + fire): pull-up 66
frames at 60/120/180, lands armed, 6 rounds, all view-aimed (the pistol was
already selected: no swap). The slot-12 sewer route is a direct mantle (139
into 140, no hang), so the armed pull-up does not apply there; its D08J7B
route was re-run unchanged (64/63 frames, pistol 2 frames after landing, the
same end). Vanilla (`van7c.py`, raw pads): Circle draws in the hang and Up
leaves Duke hanging armed, as in the original; no handoff.

Regressions (on the candidate before the choice cancel moved ahead of the
return check in `select_weapon`; nothing else differs):
D08J6A alley ladder W, D35F chain S and D35I1 wall S match their D08J7B runs
(shotgun back after landing); D08J7B `p7b.py` (pull-up 59-64 frames, shotgun
up 0.17-0.20 s after landing); D08J7A `cand.py` (5 rounds and view shots, A
shimmy with the shotgun back, S/Space armed drops, click draws only). Its W
case now pulls up armed instead of holstering (the intended change). Native:
`ttk-controls-test` 146 groups (new D08J7C group: input gate, handoff gates
and blocks, mid-shot handoff, block restore over 140, the frame-6 mask and
shot boundary, the end of the pull-up, refused handoff, the 0.75 s return
with a tap restarting it, the choice cancel, Vanilla), `ttk-input-test` under
Xvfb (W in the armed sidearm hang is Up with no stow tap, Cross masked),
`ttk-aim-test`, `ttk-near-test`.

### Limits

- Pose prototype: the gun arm plays the sidearm's own idle/fire poses over
  the pull-up (records 5-7 not aimed at the crosshair, no lean); the Blender
  one-handed pull-up clip (D08K1/D08K4 extra clips, shared with D08J6B) is
  not built. Judge it in game first.
- The late-phase boundary (frame 6 of 10) is measured on the alley platform;
  other ledges share the clip, not checked geometrically.
- At most one or two rounds fit in the late window at the pistol's rate.
- Fire during the swap waits for the new weapon (the original switch is not
  interrupted).
- No enemies were framed: shots measured by ammo, the aim counter and
  screenshots.

### Revision 1: SIDEARM RETURN option (2026-11-04)

User, on the automatic return: "i'm in 2 minds about whether we switch the gun
back automatically", then chose to make it a setting ("option 3"), defaulting
to the built behavior.

- OPTIONS > ADVANCED > ITEMS > SIDEARM RETURN: AUTO (default, the 0.75 s rule
  above) or NEVER (on the ground the swap is forgotten and Duke keeps the
  sidearm until the player chooses; a holster while still attached gives the
  selection back as before). Live (`DNTTK_SIDEARM_RETURN` through the live
  environment), saved with CONFIRM. Profile schema 64 (`sidearm_return`,
  migration from 63 adds `auto`), `run.py --sidearm-return auto|never`,
  terminal `--settings` choice A. Vanilla receives `never` and has no swap.
- Evidence: Python profile/input suites (138 tests, new
  `SidearmReturnOptionTest`: menu, invalid values, migration, launcher,
  Vanilla, in-game sidecar); native D08J7C group adds the NEVER case; live
  private alley hang with the profile set to never: armed pull-up and armed
  drop keep the pistol (no return requested), and auto still returns the
  shotgun about 0.73 s after release. Executable
  `87d8b01d982d8a872c17b0770f749214cfe1a4e5b9e2bdae9f8fa521e042dbaa`.

The arm flicker and legs-only body the user reported in the slot 9 first-person
hang predate this job (the same on the D08J7B build); they are job D36H9
(torso to be added, user decision).

### Revision 2: one-handed pull-up clip, NEVER default (2026-11-04)

User: "i wanna go for a proper one handed look to this. And i think never
should be the default :( ... we should leave the option there".

- SIDEARM RETURN defaults to NEVER. Profile schema 65: a schema 64 `auto`
  (the revision 1 default) migrates to `never` once; a schema 65 `auto` is
  the player's choice and stays. The menu row, `run.py --sidearm-return`,
  terminal choice A and the native fallback (unset = NEVER, `climb_fire.inc`)
  follow. AUTO is unchanged.
- Clip: `recomp/tools/local/ttk_onehand_pullup.py` builds the upper body
  (ROT ids 9, 16, 10-15) of a one-handed pull-up frame for frame with 140 on
  Duke's skeleton, from 140's own pose (frames that leave a joint out carry
  the previous frame's value, as the channel blend does): the chest yawed 22,
  rolled 12 and pitched 6 degrees onto the left arm (full to frame 4, then
  0.8/0.55/0.3/0.12/0 as he stands); the left hand two-bone IK-solved onto
  140's lip contact moved 35% of the way to the hands' midpoint, keeping
  140's hand orientation and elbow direction; the head keeping 140's world
  orientation; the gun wrist at chest-space (135, -235, 25) with the muzzle
  12 degrees forward of straight up and the sights toward Duke (elbow out
  and down), blending into the pistol idle (animation 5) to reach it at frame
  6. It is host extra clip 41 in `ttk-anim.pack` (`ttk_crouch_walk.py
  --host-pack`, before the padding clip). `--render DIR` writes side, front
  and quarter comparison sheets; `--set KEY=VALUE` tries variants.
- Runtime `onehand_pullup.inc`: at `0x800411b8` (after the channel blend and
  the crouch walk) while `armed_pullup_active()`, sample clip 41 at channel
  0's previous/current frame and t (so the D08J7B 1.6x timing holds) and
  slerp the joint records toward it with a weight ramped over 140 ms of real
  time, in and out. Gun arm weight 1 to frame 5, 0 at frame 6 (the late shots
  aim from the channel's own pose); chest and head left to the original aim
  build while `+0x224 & 0x100`. Legs and root are never written. Debug
  `controls.climb_fire`: `onehand_weight`, `onehand_phase`, `onehand_writes`.
  `DNTTK_ONEHAND_PULLUP=0` keeps the revision 1 pose (developer A/B).
- Evidence (private lab `recomp/analysis/d08j7/` `p7d.py`; sheets
  `p7d-s150-sheet.png`, `p7d-s-300-sheet.png`, `p7e-s150-sheet.png`,
  `p7d-fp-sheet.png`; offline renders `recomp/analysis/d08j7c-clip/r2/`):
  33 pose writes per pull-up, full weight reached; left hand on the lip and
  the pistol up beside the head in the climb, the sidearm pose from the knee
  on the lip; fire held at 60/120/180: no round before the late phase, one
  view-aimed round in it, lands with the sidearm; first person at 180: the
  sidearm viewmodel stays up and the raised body arm never enters the view;
  user slot 9 copy: 66-frame pull-up, lands armed; A/B off: 0 writes; Vanilla
  (raw pads): Up still refused in the armed hang. Native `ttk-controls-test`
  147 groups, `ttk-input-test`, `ttk-aim-test`, `ttk-near-test`; Python
  profile/input 139; `test_ttk_anim.py` 17 (new: frame count, joint set,
  encodable, left hand within 8 units of the lip target, gun wrist at the
  high ready, gun arm equal to the pistol pose from frame 6, chest and left
  arm back on 140 at frame 9). Executable
  `c89d98930383a099dd60fc39fa2652d975e265d5dc5448b1ea68675e88d60b18`.
- Limits: a procedural IK clip, not hand-keyed in Blender; the pistol mesh is
  not in the offline render, so its muzzle direction was judged in game
  screenshots only; one ledge geometry (alley platform) and the slot 9 catch
  checked; no enemies framed.

### Revision 3: shooting the whole way up (2026-11-04)

User: "with the one handed pull up, duke actually never needs to stop
shooting"; "proceed" on the proposal (drop the early mask, let the aim's
cover handling decide, aim the gun arm while firing).

- `climb_fire.inc`: `armed_pullup_shot_ready()` covers the whole armed
  pull-up; `armed_pullup_fire_masked()` only the handoff update (the hang's
  Cross must not reach the game while the original starts 140). The view
  shot leaves from the real muzzle; the body-to-muzzle trace
  (`weapon_aim.cpp`) pulls a muzzle behind the lip back to Duke's side, so a
  round aimed into the ledge face hits it, over the lip or along the wall it
  flies. `armed_pullup_view_aim()` gives the original aim look-at
  (`0x80097a44`) the view direction during the armed pull-up, as
  `presentation_ready()` does on the ground (view aiming, independent camera,
  no original-aim hold); without it the chest and pistol pointed along
  Duke's heading while the round went to the crosshair (seen with the camera
  turned 90 degrees). The D08J7A hang is unchanged.
- `onehand_pullup.inc`: while `+0x224 & 0x100` the gun arm's weight ramps
  to the channel's aimed sidearm pose over 70 ms (`onehand_aim_ms`) and back
  to the high ready after; the left arm stays on the clip, the chest and head
  on the original aim.
- Evidence (`p7d.py` with `FIRE=1`; sheets `p7f-fire60-sheet.png` before the
  look-at, `p7g-fire60-sheet.png` camera turned, `p7g-n60-sheet.png` chase
  camera, `p7g-fp-sheet.png` first person at 180): 4 view-aimed rounds through
  the pull-up at 60/120/180 (revision 2: 1), pull-up 56-61 frames, the same
  end position at every rate, lands armed; muzzle flash toward the crosshair;
  user slot 9 copy with fire: 67 frames, 9 rounds (8 view-aimed), lands armed;
  Vanilla: Up still refused. `cand.py` (D08J7A): one run fell behind real time
  (about 149 frames in about 5 s of script time after a reload; the draw click
  did not register; then the debug port stopped answering at a screenshot),
  on a host at load 2.3 with an unrelated eduke32 running; it passed 5
  reruns with identical results (not reproduced). The stalled step does not pass through the
  pull-up code. Not established as a game fault. Native suites, Python suites
  as revision 2. Executable `a040b2c15b2bf7426c7c9752c3e5c2e9f1565214727a401c44dbe87edfbbeaaf`.
- Limits: the chest look-at can twist Duke's chest far from his hips when the
  camera looks well to the side (the ground behaves the same); the left hand
  is the clip's, relative to the aimed chest, so it can leave the lip
  while firing sideways; no enemies framed; first person is job D08J7D.

### D08J7C closeout (2026-11-04)

User: "the work here is absolutely quality. legendary work. i'm really happy with it. lock it in close it out". D08J7C is Done at revision 3 and D08J7B
is closed with it (same accepted build). First-person presentation of the
one-handed pull-up is job D08J7D.


## D08J7D - the one-handed pull-up in first person (Done, user-accepted 2026-11-04)

User, on D08J7C revision 2: "do we have a backlog job in to get this
animation into first person too? it's very close already isnt it".

### Owner and behavior (Modernized, first person)

- Before: an armed Mantle (`fp_policy.action` Mantle, equipment 2) hid the
  pistol's free arm (parts 10-12) at the eye for the viewmodel (D35G1). The
  climb showed only the pistol.
- Off hand: in the armed sidearm hang (`hang_weapon_ready()`) and the
  one-handed pull-up (`armed_pullup_active()`, clip weight above 0), parts
  10-12 take the D36H5 contact rig (`first_person_contact_arm`, eye-anchored
  shoulders, native hand target) while the hand is on the lip
  (`offhand_on_lip()`): native hand within 250 units of the ledge top
  `+0x1c8`, and the drawn hand at least 128 units ahead of the eye. The drawn
  hand is pinned to within 40 units of the lip height; the native matrices
  are never written.
- Measured (alley, `geo.py`): the clip's left hand stays within 40 of the lip
  to pull-up frame 3.8, then 140 lifts it 400-860 units up by frame 5. Before
  the lip gate the eye rig stretched it into a close glove beside the eye.
  While firing, the aimed chest lifts it 70-190 units and, on the slot 9
  copy, draws it level with the eye (eye depth -50 to +66; 90-245 without
  fire), hence the pin and the depth rule.
- Gun arm: the viewmodel takes a high ready weighted by the clip's own
  gun-arm weight (`onehand_view_raise`, written in `onehand_pullup_pose`):
  50 degrees muzzle up, 15 rolled out, 40 right, 120 up, 60 nearer, about the
  grip. While firing it eases to the aimed pose in 70 ms (the D08J7C rule) and
  it is gone from the late phase. Developer tuning:
  `DNTTK_FP_ONEHAND_RAISE=pitch,roll,x,y,z`.
- Replay: `OnehandView` (off hand, raise) is in RenderState and the raise
  blends between ticks. No hook, guard, generated code, framework or save
  change. `DNTTK_FP_ONEHAND=0` gives the D08J7C view (A/B).

### Verification

Executable `396f207794e73cf19a59dfd2c4fe7da89bb7e276f4c3871f4d4d893bd0bec862`. Private lab `recomp/analysis/d08j7d/` (`fp7d.py`, `rec.py`,
`frames.py`, `geo.py`, `slot9.py`; offscreen or Xvfb, dummy audio, private
copies only; player saves, cards and settings hash-checked after every run).

- D35G alley, fire held, 60/120/180: 61-frame pull-up at every rate, lands
  with the pistol, view-aimed rounds. Sheets `s60/s120/s180-sheet.png`,
  `ap-sheet.png` (pin) and `ar-sheet.png` (depth rule).
- Recording `rec/r60b.mp4` (no fire, 60 Hz), sheet `r60b-a.png`: hang (hand
  above the view), off hand on the lip sliding down the view as Duke rises,
  pistol at the high ready, hand gone under the bottom edge near frame 4,
  pistol down. Before the lip gate (`r60-a.png`) a glove came back near the
  eye late in the pull-up.
- Looks at 120 (`looks.png`): right clean; left and down show near forearm
  slabs for a few frames, as in the accepted holstered pull-up (`ulooks.png`,
  larger there). Up in the armed hang (`up-ab.png`): off hand on the lip, A/B
  none.
- User slot 9 copy (`slot9.py`): 65-69 frames at 60/120/180, lands armed, 9
  rounds with fire. Without fire the hand and the raised pistol show. With
  fire the hand is hidden. `s9ab-half.png` (A/B) shows the tan slab in one
  frame is the pig cop's weapon, present without the change.
- Third person at 120: 61 frames, unchanged. Vanilla (`van7c.py`): Up still
  refused in the armed hang.
- Native: `ttk-controls-test` 148 groups with LEVEL00, LEVEL01 and all levels
  (new D08J7D group: off hand on the contact rig, pin, off-lip, at-eye and
  shotgun support-grip cases, through a render-state save and load),
  `ttk-input-test`, `ttk-aim-test`, `ttk-near-test`.
- A 180 Hz Xvfb recording (`rec/f180.mp4`) fell behind real time. The host
  had an unrelated eduke32 at 100% CPU (load 4.7) plus ffmpeg. The unrecorded
  180 runs played the 61-frame pull-up normally. Host contention, not
  established as a game fault.

### Limits

- The lip reach, rest and eye depth are measured constants (alley platform,
  slot 9), not per-ledge geometry.
- On slot 9, firing hides the off hand (the aim brings it to the eye); the
  pose itself is unchanged.
- The hand leaves the view below the bottom edge at about frame 4 without a
  fade.
- The high ready is a viewmodel offset judged on screenshots; no enemies were
  framed for aiming checks; only Polymorph checked.

### D08J7D closeout (2026-11-04)

User: "i'm really happy with this progress! lock it in and close it out" D08J7D is Done at executable `396f207794e73cf19a59dfd2c4fe7da89bb7e276f4c3871f4d4d893bd0bec862`. The remaining limits
above stay as recorded. Broader first-person arm coverage is still D36H6.
