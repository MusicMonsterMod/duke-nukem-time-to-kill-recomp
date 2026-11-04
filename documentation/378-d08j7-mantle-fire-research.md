# D08J7 - Shooting while mantling: research report

2026-11-04. Research report (the job's first deliverable), then D08J7A,
accepted and closed 2026-11-04 (see Closeout).

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

## Closeout

2026-11-04. User: "excellent work. lock it in and close it out!". D08J7A is
Done. The mantle stage (2b: weapon up on landing; 3: pull-up about 1.0-1.2 s)
is the new job D08J7B. The runtime patch re-export was unchanged (no framework
edits). No gameplay edits, builds or launches after acceptance; no player data
touched.
