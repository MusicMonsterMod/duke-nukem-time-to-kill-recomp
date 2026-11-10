# Time to Kill jobs - First person

Job descriptions and work logs for D11, D12, D14, D35, D36, D47 and D48.
Status and dependencies are on [the job board](../MODERNIZATION_JOBS.md); the
dated session log is in the `log-YYYY-MM.md` files beside this one. Moved
verbatim from the board on 2026-11-10 (D58), one section per job in ID order.

## Weapon-grip and medieval-wall follow-ups - 2026-10-26

Research and backlog creation authorized; implementation explicitly not started.
[Note 263](documentation/263-weapon-grips-and-medieval-wall-intake.md) records source
findings, video review, uncertainty, investigation routes and regression criteria.

## User-reported follow-ups queued 2026-10-25

All five entries are backlog only, not investigated or reproduced in this intake.
The user explicitly requested no job work yet and authorized documentation,
commit and push only. **D35J4 (bug 5) is first when work is later authorized.**
Other reports have no requested execution order. Modernized is the target;
Vanilla must remain unchanged in play. Slot numbers describe the user's current
saves and are not immutable fixtures; future tests must use private copies.

## Jobs

### D11 — First-person playable prototype

Build an optional eye-level camera using the verified movement and aiming foundation. Investigate head/body visibility, near-plane clipping and room/portal culling. Keep third-person available as a fallback for unsupported states.

**Acceptance:** a documented first-level route is playable with correct collision and shot direction, no obstructing head geometry and no missing rooms caused by the camera offset. Mark unsupported states explicitly; moving the camera alone does not complete the job.

**Done - user accepted 2026-09-29** after an extended play session: "i have been playing it for a while, and im insanely happy with it. mark all as accepted. this is phenomenal, and it's like a dream come true."
**P** toggles an eye-level view in Modernized
(independent camera); saved as profile schema 13 `view`, `--view first|third`.
The eye follows Duke's neck joint through the original camera solve, the head
joint is skipped only during Duke's draw, and the render gets a shorter
projection distance (about 64 degrees). An isolated first-level route (street,
club door fight, doorway threshold, entry hall, main room, jump, crouch, P
toggle) passed its 14 checks. Swimming and jetpack flight blend back to third
person. Known limit: steep-angle wall contact can drop wall polygons (D11B).
See [D11 first person](documentation/61-d11-first-person.md).

### D11A — Scroll-wheel zoom lock into first-person

**Cancelled 2026-09-29 by the user:** the D11 `P` toggle works well enough for
entering first person, so a scroll-wheel lock is not wanted. Kept for reference.

**Todo from 2026-09-27 playtest.** Player wants Fallout/Skyrim-style entry: scroll
the existing third-person distance inward until the view locks into first-person,
then scroll out to return. Coordinate with D10 distance and D11 eye-level camera;
do not break Alt-wheel distance, weapon wheel, or Vanilla.

**Acceptance:** continuous distance scroll reaches a documented first-person lock
without FOV hacks alone; unlock returns to the preferred third-person distance;
unsupported states fall back cleanly; aiming/movement remain coherent.

### D11B - First-person near-wall polygon clipping

**Done (2026-09-29, user accepted).** Host near clipping for the two level mesh
renderers (`0x80011020` world, `0x80010000` object) while the eye view is live;
see [the engineering note](documentation/62-d11b-near-clip.md) and the work log.
Original report from D11: pressed against a wall and looking along it at
a steep angle, the first-person view loses or tears the wall polygons that
reach beside or behind the eye (club side wall, about 60 degrees off its
normal). The D11 attribution to `0x8002f1e0` was wrong; that routine draws a
horizontal grid surface. A shorter projection distance (192-256) and moving
the eye back 64-128 did not fix it.

**Acceptance:** the documented D11 wall-contact cases (head-on and 30-90 degree
oblique in the club and a first-map corridor) render without black or torn
wall polygons; third person, Vanilla and the frame budget are unchanged.

### D11C - Savestates can keep Duke's first-person head hidden (slot 12)

**Done - user accepted 2026-09-30:** "duke's head is back, mark as complete! well done". A stale head-hide bit is reclaimed at Duke's
next draw in either view, so loading slot 12 (or any save taken in first
person) shows the head again. Verified that the original never sets bit 0 on
a joint record. The save file itself is not rewritten. See the work log.

**Found 2026-09-30 (user noticed it was specific to slot 12):** in the eye
view, `first_person_draw` (`recomp/src/ttk/first_person.inc`) sets bit 0 of the
flag byte on Duke's head-joint record (joint 9, `[player+0x40]+0x44+0x28*9`)
from Duke's draw entry to the next object-list step, and `head_restore`
clears it. The user's slot-12 savestate was captured inside that window: on a
private copy, the head record flag reads `01` right after loading (all other
joints `00`; slot 1 reads `00`). After a load the host `head_flag` is 0 and
`first_person_draw` deliberately refuses to take ownership of an already-set
bit, so nothing ever clears it and Duke is headless in every view.

**Acceptance:** a save taken in first person can never carry the hidden head.
Two options: clear the bit before a savestate is written, or safely reclaim a
stale bit after a load. First verify whether the original game ever sets bit 0
on Duke's joint 9 itself, so a real hide is never undone. Loading the existing
slot-12 copy shows Duke's head in third person, while first person still hides
it. Add a native test case. Vanilla is unchanged; slot 12 itself is repaired
only if the user asks.

### D11D - First-person eye height from Duke's real proportions (game-wide)

**Accepted - user confirmed 2026-10-05:** "i can confirm that i accept both jobs as complete!" Cause: the cowboy costume (levels 1, 2, 3
and 27, the Old West levels) orders Duke's 19 joints differently. The eye
followed joint 9, which is the neck in every other costume but a hip-height
part in the cowboy model, so the eye sat about 300 units low (waist height).
The eye, head hide, weapon torso reference and arm hiding now find their
joints through the model's own part table (`desc+0x24`, the same table the
game reads the right hand from). The neck is now 691-705 above the toes in
all 21 selectable levels. Shared with D12B; see the work log.

**User report, 2026-10-04 (D22A playtest).** UI slot 8: stand next to
the dancer in first person and Duke appears to be at about her waist height.
Press **P**: in third person Duke is about her height or slightly taller. The
first-person eye/camera height does not match Duke's actual size.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Investigate Duke's actual actor/model height, the first-person camera origin
and eye anchor (`first_person_anchor`, D11/D11C), our view offsets and
projection, stance/crouch state, and whether the result varies between levels,
costumes or player states. Derive a believable eye height from Duke's real
proportions.

**Game-wide:** fix the shared first-person system. Do not add a LEVEL01 or
per-level camera offset.

**Regression evidence, 2026-10-05 (user, D22B game-wide playtest):** Duke's
first-person height looks correct in the medieval levels and in the Roman /
HOG HEAVEN area, believable against the surrounding characters. The LEVEL01
report above is still open; any fix must keep those levels looking right.

**Acceptance:** in slot 8 the dancer and Duke read as similar heights in both
views; the first map's accepted first-person routes (apartment, club, subway,
crouch, ladders, swim) still look right; the user confirms.

### D11E - First person while swimming (game-wide)

**Done (user-accepted 2026-10-24).** User accepted first-person swimming and explicitly requested lock-in and closeout. Stable surface/underwater eye, animated weapon display, replay state and swim holster presentation are accepted. Mantles retain the established third-person traversal view. [Note 216](documentation/216-d11e-first-person-swimming.md) preserves implementation, test evidence and coverage limits; [note 215](documentation/215-d11e-first-person-swimming-research.md) records the research. Accepted executable: `6725addce03114878375d08d81331a4bd4032aa5169d21fff74a586602b73ea6`.

**Found by the D22B survey, 2026-10-04.** In BLOOD BATHS (and any deep
water) Duke swims in modes 4/5 (anims 122-128). The Modernized swim controls
and mouse camera own it (D08M/D08O), but `first_person.inc` deliberately treats
swim modes as unsupported, so a first-person player is shown the orbit view
until Duke leaves the water. Reach it with `level 12` and walk into the pool.

Give swimming a first-person eye view (head/eye anchor while the body pitches,
near-surface and waterline behavior, weapon/hands presentation, the D08O
mantle-only exit), or document why a state must stay in the orbit.

**Game-wide:** the shared swim/first-person system, not one level's pool.

**Acceptance:** first person stays active through entering, swimming, diving,
surfacing and leaving the water in at least two different levels; third person
and Vanilla unchanged; user confirms it reads well.

### D11F - First person while flying the jetpack (game-wide)

**Done (2026-10-24).** User accepted the final slot-12 correction and explicitly
requested closeout: "lock it in and accept! close it out!" First person stays
active in Modern, Classic and EDuke32. Modern/EDuke32 keep the pack running at
floor/platform hover, descend below departed platforms and retain the chosen
height on release; reflected flames are visible. Native suites and private
flight/platform routes are recorded in
[note 220](documentation/220-d11f-first-person-jetpack.md), with coverage limits.
Implementation and documentation closeout authorized as MusicMonsterMod.

Original user request, 2026-10-06: "add another job in for first person jetpack."

Give jetpack flight a first-person eye view for every jetpack scheme (Modern,
Classic, and the EDuke32 scheme from D08S once it exists): eye anchor through
take-off, hover bob, flight lean and landing; weapon/hands presentation and
the D08Q3 view aiming and crosshair in first person; ceilings and near-wall
clipping (D11B); clean hand-off at J on/off, fuel-out and death. Document any
state that must stay in the orbit.

**Game-wide:** the shared jetpack/first-person system, not one level.

**Acceptance:** first person stays active through take-off, flight, hover,
firing and landing in at least two levels with each jetpack scheme; third
person, ground first person and Vanilla unchanged; the user confirms it reads
well.

### D11G - New level can start in third person with first person selected

**Cancelled 2026-10-21: cannot reproduce.** User: "cancel the job. i cant
seem to replicate it either now. if it comes back i'll let you know but close
it as cant replicate". Reopen if it is seen again. The user later clarified
that the left click was simply how they restored the view, not the only way.

**Work log (2026-10-21, no repro; no code changed).** Private copy of the
player's UI slot 11 (file slot 10, SHA-256
`436ae18d35c2287783b9324ae363c3aa1b3939ed095311435237b668c1a466c0`, saved
on the LEVEL 0 statistics screen), private cards and port 9391, harness in
`recomp/analysis/d11g-fp-start/` (local only: `x1.py` real keys on private
Xvfb with the real capture logic, `g1.py` real GPU offscreen). Every route
started the next level (level 1) in first person with no input: debug load
then Enter on CONTINUE; pause menu LOAD GAME; title LOAD GAME after launch;
F3 load menu during play; statistics SAVE GAME then CONTINUE; the player's
own profile copy (120 fps, distance 1728, right shoulder, corrected
precision) in a window and on the real GPU at 120. In each, the mouse was
captured automatically when gameplay appeared (about 6.5 s after Enter
under Xvfb) and the eye view blended in within about 0.6 s. Code reading:
the gameplay input frame carries the first-person preference only while
the mouse is captured, so uncaptured play shows the original third-person
camera; recapture follows the automatic offer (needs `initial_capture`,
window focus, no Escape hold) or F10. Candidate causes if it returns: the
window losing focus during the level load, an F10 opt-out, or a level-start
state that keeps the camera lease inactive. A session log from the
occurrence would decide it.

Opened 2026-10-21 (user, when accepting D20D; not actioned): "starting a new
level might result in duke loading in third person when first person was
already selected. an example of this is loading our slot 11 save, and
starting the next level, and duke starts in 3rd person. when you click the
left mouse button, it goes back to first person."

Reproduction to confirm: player UI slot 11, finish the level and CONTINUE
from the statistics screen (or `endlevel`), with first person selected.
Expected: the next level starts in first person, as in D22A / D22B. Left
click (fire) bringing first person back suggests the view is restored by a
later event rather than kept across the level start. Fix it game-wide, not
for one level, and check other level starts (new game, level select,
CONTINUE after a death) and the D11C head-visibility state.

**Acceptance:** with first person selected, every level start reached by
finishing a level (and the other starts above) begins in first person with no
input needed; third person is kept when it is selected; Vanilla unchanged;
the user confirms from slot 11.

### D12 — First-person weapons and state polish

**Done - user accepted 2026-09-30:** "finally, we can mark this as accepted!!"
after six framing passes. Duke's own right hand and the
original weapon meshes are drawn in front of the eye in first person, held in
each weapon's own firing pose, drawn over walls, with the original muzzle flash
and a short kick, each weapon framed per the user's playtests (Devastator-style
twin cannons, HUD on top). No new assets. See
[D12 first-person weapons](documentation/63-d12-first-person-weapons.md) and
the work log. Known limits: instant holster/draw, no left hand, reload motions
not shown, projectile origin still at the real hand. The kick leaving first
person is D12A (quick kick, Done).

Resolve held-weapon presentation, body visibility, eye height, recoil, traversal, death and scripted sequences. Decide whether existing geometry is sufficient before proposing new assets.

**Acceptance:** tested weapon/state combinations are usable without clipping or misleading muzzle placement; camera transitions remain coherent; third-person and Vanilla still pass their routes. Document any optional new assets and their provenance.

### D12A - First-person kick without leaving the eye view

**Done - user accepted 2026-09-30:** "its done! accepted. ... this has been yet another amazing feat of engineering." Queued by the user on 2026-09-29: in first
person, kicking (Q, the original Boot melee) dropped out of the eye view to
the original third-person camera. On 2026-09-30 the user asked for no "duke-fu
spin around" in first person, but a Duke Nukem 3D style kick with Duke's leg,
"more accurate to aim". Delivered: in the eye view Q is a short quick kick with
any weapon, moving or standing, drawn with Duke's own right-leg meshes posed
from the original front kick (animation 115), hitting along the crosshair with
the original kick's damage sphere, radius and damage formula. Boot-selected
original kick requests (Q, E, held attack) are converted to it. See
[D12A first-person kick](documentation/65-d12a-first-person-kick.md).

**Acceptance (revised with the user's direction):** in first person a
standing and a moving kick, holstered and armed, stay in first person with the
view aim intact and no full-body spin; the leg reads as a kick; the kick hits
what the crosshair is on through the original damage path; third person and
Vanilla keep the original kick. The user accepted no knockback and the lower
damage per kick after the first playtest; the kick must hit low props under
the crosshair without crouching, kick on a held left-click with the Boot while
moving, and show a tuned thigh (second pass, Needs playtest).

### D12B - Costume-aware first-person kick leg (game-wide)

**Accepted - user confirmed 2026-10-05** (with D11D). Rule: the kick draws Duke's right-leg
*parts* 5-8 (hip, knee, ankle, toe) from whichever model he currently wears,
found through the model's part table. Medieval and Roman were right because
their models share the first map's joint order (leg joints 14-17); the
cowboy model has the leg at joints 5-8, where joints 14-17 hold other parts.
Later levels inherit the rule. Shared cause with D11D; see the work log.

**User report, 2026-10-04 (D22A playtest).** UI slot 8 or 9: Duke wears
a different costume in LEVEL01, but the first-person quick kick (D12A) still
shows the first map's leg appearance.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Find how the original game represents Duke's per-level costume/appearance
(model, texture page/CLUT, part tables) and make the first-person kick leg
follow Duke's current visual state rather than a fixed leg. Do not hard-code
"level 2 uses leg B" if the game has a meaningful current-costume concept to
derive from.

**Acceptance:** the kick leg matches Duke's third-person costume in the first
map and in LEVEL01, by the same rule; the user confirms. Record the rule so
later levels inherit it.

**Regression evidence, 2026-10-05 (user, D22B game-wide playtest):** the
first-person kick already shows Duke's medieval costume ("working beautifully")
and his Roman / HOG HEAVEN costume ("looks fantastic"). Find out why those eras
are right and LEVEL01 is not before changing anything, and keep them right.

### D12C - Kick impact sound on a real hit only

**Done - user accepted 2026-10-08:** "perfect!! fully accepted." Implemented 2026-10-08. A kick that connects plays the
game's own wall-bump thud `0x2007` (bank 2 entry 7, SPU sample `0x1BAA0`, the
bump call `0x8006b270(0x2007, Duke+4, 0x800)` at `0x800538bc`, no vocal in
it), played one octave up at three times its table volume (user tuning), once,
at the first contact (a still-playing `0x2007` from the wall bump is stopped
first); empty air, missed and out-of-range kicks stay
silent. Contact is the damage sphere's own hit result (it stores the attacker
in its frame only on a hit); walls are the original segment query's world hit
where the boot reaches. First-person quick kick and the third-person original
kick (new entry hook `0x800A979C`, ra `0x80049098` only; the host makes the
same sphere call and the original call is left touching nothing), Modernized
only. See [note 131](documentation/131-d12c-kick-impact.md).

User request, 2026-10-08. Small audio/game-feel job.

**Problem.** When Duke's kick connects (a wall, a breakable crate, an enemy or
another object that takes melee damage) there is little or no physical impact
sound, so the hit feels weightless. It is most noticeable with crates that need
several kicks: the individual kicks are nearly silent.

**Rule.** The sound comes from the authoritative kick hit result, never from the
button press or an animation frame:

- kick -> no collision -> silent;
- kick -> wall -> impact sound;
- kick -> crate/object -> impact sound;
- kick -> actor/enemy -> impact sound.

Do not build a second, audio-only approximation of the kick collision if the
existing melee/kick logic already says whether the attack connected.

**Candidate sound (research clue, verify).** When Duke runs or jumps into a
wall, two sounds can play: Duke's grunt/vocal, and a generic physical
impact/thud under it. The user wants that **non-vocal thud**, never the grunt.
It may be a Duke 3D-inherited sound; treat that only as a clue. It already
exists in TTK's resources (the game plays it on wall collision): find the sound
the game is already playing; do not add a new Duke 3D asset. Useful tools from
D08A10/D27A: the `0x8006b73c` entry log (`controls.steroid_beat.log`), SPU
KEYON capture (`spu_events`) to tell which sample is which, the debug
`sfx <id>` console command, and the note that non-positional sounds use
`0x8006bbd8` instead ([note 129](documentation/129-d08a10-steroids-heartbeat.md),
[note 130](documentation/130-d27a-silent-shift.md)). D12A has the first-person
quick kick's hit sphere and sound notes.

**Avoid duplicates.** Some targets may already play their own impact/damage
sound. Trace the kick/damage/collision paths and place the sound at the most
appropriate common point, so one kick never plays the same sample two or three
times through different paths. Where a target already gives fitting impact
feedback, keep it rather than layering an identical sound on top. Enemy pain
sounds, vocals and damage sounds stay as they are; the new impact complements
them.

**Scope.** Audio only. Do not change kick damage, range, timing, animation,
quick-kick controls, object health, enemy damage reactions or collision
geometry. Vanilla stays original; Modernized-only unless the user decides
otherwise when selecting it.

**Acceptance.**
1. The existing wall-collision impact sound is located.
2. It is separated from Duke's accompanying grunt/vocal.
3. Its sound resource/ID (bank, index, SPU sample) is recorded.
4. The authoritative successful-hit point in the kick logic is identified.
5. The cleanest trigger point is chosen and documented.
6. Empty-air kicks stay silent.
7. A wall kick plays the sound once, at contact.
8. Each successful kick on a breakable object (e.g. a multi-kick crate) plays
   the sound; damage and destruction unchanged.
9. A successful kick on an enemy/actor gives fitting impact feedback.
10. Failed/out-of-range kicks stay silent (seeing a target in front of Duke is
    not enough; the boot must connect).
11. No duplicate playback where the target already has related impact/damage
    audio.
12. All existing kick damage and gameplay behaviour is preserved.

Goal: "If Duke's boot actually hits something, I should hear the impact. If his
boot hits nothing, I should hear nothing."

### D12D - First-person rocket launcher blocks the view in Challenge Stage 4

**Done (user-accepted 2026-10-22).** Separate upgrade records 27/28/29 now share
the complete base first-person framing, including firing seeds. Challenge
stages 23/24/25 and all six campaign upgrades checked; native suites pass.
User fully accepted and authorized closeout. [Note 199](documentation/199-d12d-upgraded-viewmodels.md).

**Original user report, 2026-10-08:** "In challenges, level 24, you spawn in with
the rocket launcher, however in first person mode, it's view is obscuring your
vision. We came into a similar bug like this early on when we created first
person mode for the first time." Evidence: `research/rocket-launcher-challenge.png`
(the launcher fills the middle of the screen in first person).

Historical lead (resolved in note 199): challenge stages 21-26 set upgrade bit (level - 21)
in `player+0x85f` (`0x800950cc`), so stage 4 (level 24) upgrades weapon 8,
the RPG, which `0x8003df40` resolves to record 29 (Incendiary RPG). The D12
first-person tables in `recomp/src/ttk/first_person.inc` (`weapon_slot_offset`,
`weapon_slot_tilt`, `fire_poses`) only cover slots 0-11, so 27 (HiTemp), 28
(Laser Gatling) and 29 (Incendiary RPG) may get no framing at all. If so, the
same view would appear after `dnupgrade` in any level. Check every upgraded
form, not just the RPG.

**Game-wide:** upgraded weapons in first person anywhere (challenge stages,
`dnupgrade`), not only level 24.

**Acceptance:** in first person the upgraded weapons (27, 28, 29 and the
in-place upgrades 4, 5, 10) are framed like their base weapons and never block
the view, at rest and firing, in Challenge Stage 4 and after `dnupgrade` in a
campaign level; base weapons, third person and Vanilla unchanged; the user
confirms.

### D12E - Third-person quick kick (research report first)

**Status: Done (research closed by the user 2026-11-04; [note 381](documentation/381-d12e-third-person-kick-research.md); implementation D12E1, D12E2). Dependencies: D12A, D12C, D10F1.** User (2026-11-04, playing
in third person after D10F1): the original third-person kicks ("the like jiu
jitsu kicks and stuff that he does in third person mode are so slow that you
will basically get [hurt] if you actually take [it] seriously") make
third-person play hard; "we want the quick kick to be quick ... exactly the
same thing as what we do spam it in first person and you have more offense".
"I underestimated the importance of third person ... it's original identity
... I want to get third person looking right ... and feeling right and
playable."

Scope: a Modernized option (toggle) between the original kick and a quick
kick in third person, with the D12A cadence, reach, damage path and D12C
impact sound; Vanilla, first person and the original-kick setting unchanged.
It must look professional ("absolutely legit"), not a sped-up spin.

Report first (deep research): how third-person action games present a fast
melee kick or boot (duration, wind-up/recovery, upper-body layering so the
weapon and aim stay up, root motion, hit-stop, camera response); what the
original animation set offers (animation 115 front kick used by D12A, the
existing kick clips and their timing); options from retiming or cutting an
original clip, a partial-body leg layer over the current pose, to a new
authored clip (Blender allowed, following the D08A28 style guide and the
D36 first-person presentation audit for ownership). Measure the current
third-person kick timings in game, propose one approach with its timing
budget, and the toggle's menu home. No implementation until the user
selects it.

### D12E1 - Spectacular third-person quick kick (legs layer)

**Status: Done (accepted by the user 2026-11-04, revision 3; [note 382](documentation/382-d12e1-third-person-quick-kick.md)). Dependencies: D12E, D08K4.** Implement the locked design in
[note 381](documentation/381-d12e-third-person-kick-research.md) section 6
(user: "let's make this custom kick absolutely spectacular ... try whatever it
is that you recommend"; inspiration ARPGs and character action, "twitchy third
person visuals").

- Authored legs-and-pelvis clips on channel 0 (the D08K4 crouch-walk path,
  `ttk-anim.pack`, new `ttk_quick_kick.py`), keyed from 115, foot IK on the
  support leg, plus the mirror for the other leg; arms, chest, head and
  weapon stay on the original aim. Any weapon or holstered; standing, walking,
  running on the ground.
- Budget: coil 0-67 ms, chamber 67-117, snap in about 2 frames, strike held
  to about 283, settle with overshoot, chain point 350, done 450 ms; hits
  133-333 ms through the D12A sphere aimed from Duke's hip; fire available.
- Alternating legs when chained; Duke-only hit-pause (about 50 ms, taken from
  the recovery) and a camera punch only on a real hit; the original spark
  burst `0x8006d594(6, ...)` at the contact; whoosh at the snap (verify
  `0x1000` is not a vocal), D12C impact with about +/-5% pitch variation;
  whiffs whoosh only.
- Pace capped at walking during 0-350 ms; 30-40 unit step on the snap through
  Duke's collision; no magnetism.
- ADVANCED > COMBAT > KICK: QUICK / ORIGINAL (`controls.third_person_kick`,
  `DNTTK_TP_KICK`), QUICK default; ORIGINAL and Vanilla keep 112-115
  exactly; first person unchanged. GAME_MANUAL.md.

**Acceptance:** in third person Q kicks with any weapon while standing,
walking and running; first contact within about 150 ms of the press and
about 0.45 s per kick, chaining and a held Q repeating; the aim and weapon
stay up and firing works during the kick; legs alternate; the clip reads as
an authored snap kick at 60, 120 and 180 Hz (contact sheets plus the user's
judgement: "absolutely legit"); hit-pause, camera punch and spark only on real
hits; damage per kick as in first person; ORIGINAL, first person and Vanilla
unchanged (native suites, Vanilla route); frame budget unchanged.

### D12E2 - Third-person kick finisher and enemy payoff

**Status: Cancelled (user decision 2026-11-04: "I'm actually happy with the
kicks! i wanna leave the kicks at this point"; rumble moved to D09C).
Dependencies: D12E1.** Scoping found the finisher spec stale (D12E1 revision 2
replaced the alternating front kick with a single left-leg roundhouse) and the
reaction 84 shove untraced. Original scope, from
[note 381](documentation/381-d12e-third-person-kick-research.md) section 6:
a heavier third kick in a chain (a roundhouse keyed from 114's strike pose,
about 520 ms, hit-pause about 83 ms); knockback and stagger through the game's
own enemy reactions (trace what selects reaction 84, the original kick's
shove; never a host shove that bypasses collision); a draw-only shake of the
struck enemy; gamepad rumble on a hit; optional short magnetism if D12E1 play
shows near misses. Each piece judged with the user; Vanilla and ORIGINAL
unchanged.

### D14 — Widescreen, FOV and visibility

**Done (user-accepted 2026-10-01):** "im very happy with it! i accept!" Modernized opens in true widescreen (default
16:9; `run.py --widescreen off|16:9|16:10|21:9|auto`), rendered natively wide
with the original projection, so nothing is stretched. TTK's portal root
rectangle is widened so the revealed columns are drawn, the D11B near clip runs
in third person for near walls, and the status bar sits in the wide corners
(restored after it draws, so saves and 4:3 never see it). Movies and 2D menus
stay 4:3. The broken preview came from `nw_hud_corners`, which shifted world
polygons. Vanilla and `off` are unchanged. See
[the D14 notes](documentation/76-d14-widescreen-first-pass.md).

Explicit user priority: widescreen support. Render a wider view without stretching actors. Correct aspect, FOV, HUD anchoring, menus and room/portal visibility; define how original movies and fixed compositions are framed.

**Acceptance:** 4:3 and 16:9 pass representative indoor/outdoor tests without geometry popping at the added edges, misplaced aiming or stretched UI. Additional aspect ratios may remain explicitly unsupported.

### D14A - Implement first-person field-of-view control

**Status: Done (user-accepted 2026-10-28). Dependencies: D11, D14. Related: D11B, D12, D36.**
Investigate camera projection and widescreen support before adding a menu
setting. Establish the current first-person FOV and the horizontal/vertical
degree convention; implement a configurable value that changes real projection,
not image stretching. Preserve the current view exactly as the default. Scope
to first-person gameplay, leaving third-person cameras, scripted shots, menus,
FMVs and Vanilla unchanged.

Provide a temporary developer control. Verify aiming alignment, visibility and
portal/object culling, near clipping, widescreen behavior, first-person weapons,
held items and arms across a practical range. Include view transitions and
high-refresh replay. Read the accepted D36 audit and preserve its ownership
boundaries and regression requirements; D36G-L remain separately unselected.
Recommend safe slider limits from actual results. Document limitations and the
configuration interface for D19X, including units, default, limits, validation,
live application, and persistence integration points.

Acceptance: measured baseline and declared degree convention; observable real
projection changes; current default preserved; unaffected camera/presentation
modes verified; practical-range evidence for aim, culling, clipping, widescreen
and first-person presentation; safe range and menu-facing contract documented.

Intake source finding: `first_person.inc` defaults to GTE H=256
and accepts cached developer-only `DNTTK_FP_PROJECTION=160..386`. This is neither
a saved degree setting nor evidence that its entire range is safe. The source
comment's approximate 64-degree width is not a measured display convention.

Candidate: session-only `fov [60..90|default]` console control and
`DNTTK_FP_FOV` launch override, horizontal degrees at 4:3. Nominal default 64
preserves H=256 exactly. Replay snapshots carry projection; a scoped correction
prevents native interaction limbs disappearing under the wide-FOV proximity fade.
Build/native controls/input/aim/near suites pass. Private 4:3/16:9/21:9 captures,
60/120-setting replay and D36 held-item/interaction checks are documented in
[note 296](documentation/296-d14a-first-person-fov.md), including failed harness
attempts and remaining visual/campaign coverage. D19X remains unselected.

### D14B - Decouple first-person weapon presentation from world FOV

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependency: D14A foundation. Related: D12, D36G/H/I/J/K, D19X.**

User reports that adjustable world FOV looks excellent, but increasing it makes
first-person weapons and arms too small and incorrectly positioned. Reproduction:
load player UI slot 4, use `dnmonsters` and `dnstuff`, set `fov 90`, and cycle
through weapons. This is user-reported evidence; the intake did not launch the
game or independently reproduce it. When selected, identify the saved scene and
use private save/card copies; slot numbers alone are not stable scene identities.

Investigate how world FOV affects weapon and arm geometry. Establish whether
projection alone explains the size/placement change or whether existing placement
calculations also contribute. Read the accepted D36 presentation audit and the
D14A implementation/evidence before choosing an approach. Use the accepted
presentation at original world FOV (H=256, nominal 64 degrees at 4:3) as the baseline.

Implement a separate first-person weapon/arms projection, or an equivalent
renderer-appropriate correction, preserving their intended apparent size and
screen placement as world FOV changes. Identify geometry by reliable semantic
ownership, including weapon attachments and muzzle flashes across costumes and
shared-skeleton paths. Resolve held-item and interaction-arm boundaries explicitly;
preserve native world-space contact and occlusion where appropriate. Avoid
per-weapon placement adjustments unless investigation demonstrates their necessity.

Preserve Duke's natural world-space body/legs, third-person rendering, animation
events, environmental lighting and intentional effects. Preserve crosshair
alignment/targeting, projectile trajectories, hit behavior, nearby-wall clipping,
and the accepted world-FOV improvement. Keep Modernized scope and Vanilla intact.
Carry any presentation state correctly through high-refresh replay, load and
view transitions; do not alter gameplay matrices merely to reframe the viewmodel.

Acceptance: verify every weapon slot, including alternate/upgraded variants,
across the supported world-FOV range (currently 60-90, plus original/default).
Compare matched captures of apparent size and placement to the original-FOV
baseline. Cover idle, fire/recovery/reload where applicable, muzzle flashes,
switching, look up/down, standing/crouching and nearby walls, relevant aspect
ratios, first/third-person transitions and 60/120 replay. Include native body/leg
and lighting/occlusion regressions and distinguish observed results from user
acceptance. Report shared-skeleton or render-pipeline limitations and any
necessary per-weapon exceptions with evidence.

A future standalone HTML comparison artifact may help the user approve the
weapon presentation. Create it only when requested; no artifact is requested by
this intake. Use plain locally opened HTML, like the ttk-fonts document, without
a web server. The original backlog intake authorized no weapon design, code, build or
playtesting. The user subsequently selected D14B on 2026-10-28.

Accepted result: DisplayHand/DisplayArm meshes and attached equipment use local H=256,
while the world keeps the selected FOV. Original framing, grips, UV/depth and
lighting are preserved without per-weapon retuning or gameplay-matrix changes.
Build, native controls/input/aim/near suites and private multi-weapon/aspect,
interaction and replay samples pass within the limits in
[note 297](documentation/297-d14b-weapon-projection.md). User confirms the arms
are solid and accepts D14B, with documentation/implementation commit and push
closeout authorized. D14C separately records the leg-view concern; it is Todo.

### D14C - Investigate first-person leg presentation across world FOV

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D14A, D14B (Done). Related: D36J/K, D12A/B, D08K.**

User accepts D14B's arms but reports an uncertain view alteration involving
Duke's legs. Investigate before choosing any correction. First compare FOV 60,
original/default and 90 from the same private save, position, camera pitch,
stance, lighting and animation phase. Identify ordinary body/legs separately
from Q quick kick and native door kick; do not assume the report concerns a
specific one of those paths or that all should use the weapon projection.

Bounded intake research: source inspection confirms Body and Kick are excluded
from D14B's fixed projection. Ordinary legs retain native poses/world depth;
quick kick has separate camera-relative presentation and viewmodel depth but
still uses world H. Existing D14B default/90 downward-look captures show changed
leg presentation, but differing animation/lighting states prevent a controlled
diagnosis. No new FOV 60/90 comparison or game launch was performed. This is a
source finding and limited existing-image observation, not a confirmed cause.

When selected, read the accepted D36 audit and D36J implementation first.
Trace projection, eye/body relationship, near-plane and joint admission, depth,
proximity fade, pose and replay ownership. Distinguish expected perspective
changes from unwanted deformation, popping, overlap or apparent camera movement.
Recommend an evidence-backed correction only if needed, preserving natural
world-space body contact and the accepted D14B weapon/arm presentation. Avoid
blanket fixed-FOV legs or camera changes without demonstrating their necessity.

Acceptance: report the actual cause or explicitly unresolved hypotheses with
matched 60/default/90 evidence. Verify any selected correction standing,
crouched, walking/running, jumping/landing, looking down and through pitch
changes, plus Q and door kick where relevant. Cover supported aspects, 60/120
replay, costume mappings, view/load transitions, nearby floors/walls, lighting
and occlusion. Preserve kick reach/damage/timing, gameplay matrices, third
person and Vanilla. Separate observed results from user visual acceptance.
The intake authorized no implementation, build, new art or further research.
The user selected D14C on 2026-10-28. Matched geometry traces establish expected
world-projection framing; native per-joint shading was reapplying faded brightness
at wide FOV. The candidate corrects that register within the verified limb draw,
preserving projection, camera, pose and gameplay. Build/native suites and bounded
private visual/replay checks pass. See [note 298](documentation/298-d14c-leg-fov.md)
for evidence and limits. The user accepted the result and authorized lock-in
and commit/push closeout on 2026-10-28.

### D14D - Correct sky artifacts at wider first-person FOV

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D14A, D17O, D17R/R1/R2 (Done).**

User selected immediate investigation and implementation. Report: current UI
slots 2, 5 and 6 show sky artifacts, especially at the left/right at FOV 80/90.
Compare private copies at default/60/80/90 and trace the earlier widescreen
coverage, native sky layers, clipping and replay paths. Prefer coverage driven
by the actual view projection and aspect over restricting the accepted FOV.
Keep original art, gradients, cloud motion, world visibility and Vanilla intact.

Acceptance: matched reported-scene captures and yaw/pitch motion at 4:3, 16:9
and ultrawide; preserve default FOV and third person, load/view transitions,
60/120 replay, prior sky fixes and packet-budget behavior. Add focused native
regressions for the proven cause. Distinguish sampled verification from user
visual acceptance. User confirms all working and authorizes lock-in and
closeout on 2026-10-28.

Accepted result: the partial sky backdrop now adapts to actual render H as well as
aspect. Matched side seams are removed; build, sky/near tests and bounded
private 4:3/16:9/21:9 and replay sweeps pass. User playtest is accepted.
See [note 300](documentation/300-d14d-adaptive-sky-fov.md) for evidence and limits.

### D14F - 90-degree default and 120-degree maximum world FOV

**Status: Done (user-confirmed 2026-10-30: DISPLAY slider, default 90; re-verified 2026-10-29). Related: D14A, D14B, D19X.**
Inspect and retain the existing FOV convention. Fresh/unset defaults become 90; explicit saved preferences survive. Retain minimum; update slider, validation, persistence and descriptions. Preserve fixed weapon/hand presentation.

Implemented in commit afd4ad6 ([note 317](documentation/317-d14f-fov-settings.md));
the "none at registration" lines here were stale. Re-verified 2026-10-29:
C++ default 90, range 60-120 (`first_person_fov.h`); OPTIONS > DISPLAY slider
60-120, default 90, one-degree steps; launcher and profile fallback 90 with
explicit saved values kept; Vanilla still forces 64; manual matches; Python
profile suite 111 tests pass. Visual coverage at FOV 64/90/120 came from the
D17AM3 sweep (note 326). Remaining: the user's in-game check of the slider
and default.

### D14G - Holstered swimming arms look disembodied in first person

**Status: Done (user-accepted 2026-11-01). Dependencies: D11E, D36H4, D14B, D08O5.** User: in
first-person swimming with the weapon holstered "the arms looks disembodied"
(a "weird projection"). Read note 257 (D36) and notes 305/306 (D36H5) first.

Intake research (2026-11-01, private slot-2 moat copy, first person, 60 Hz,
offscreen; `recomp/analysis/d08o5/shots/sheet.png`, local only): holstered
underwater idle and strokes showed no arms in the sampled frames. During
surface strokes, separate forearm/sleeve and hand pieces appear low on the
screen with no shoulder, upper arm or torso connecting them. Source lead:
the D36H4 unarmed rule in `first_person.inc` (`ra == 0x80034c6c`) claims
the spine/torso parts and unarmed arm parts 10-15 as `Body` (world clipping
admission plus the 120-unit screen-down framing). It only runs for the Ground,
Committed, Fall and Jetpack actions, not `PerspectiveAction::Swimming`. So
swimming holstered arms keep native ownership while the D11E eye is
root-anchored inside the pitched swim body. These are world-projection
limbs; the D14B fixed projection (DisplayArm/DisplayHand) applies only to
armed display arms, so a high-FOV projection miss is possible but unconfirmed.

Scope: identify the actual owner of each visible piece per stroke phase
(surface 122-126, underwater 127-130/133), then give holstered swimming arms a
connected, intentional presentation: either an arm chain connected from the
shoulder (D36H5 contact-arm design) or hidden at the eye where the stroke
cannot be shown plausibly. Check the default and maximum world FOV, 120 and
180 Hz, the D08O5 draw into the armed swim pose, and the D36K backpack hide.
Armed swimming, land, flight and Vanilla unchanged. Report findings before a
design choice if more than one option fits.

Work log 2026-11-01: joint sampling confirmed the cause. At the surface the
shoulders sit at the eye plane, so the native cull dropped upper arms (and
often forearms) and kept hands/cuffs; looking down puts a shoulder joint
near screen centre, ruling out plain Body admission (open shoulder ends).
Holstered swimming now uses the accepted D36H5 contact rig unchanged (native
hands, eye-anchored shoulders, authored elbow pole); a drawn weapon keeps its
display arms. Native controls/aim/near pass with a new swim group (mutation
checked). Live 60/FOV 90, 120 Hz and FOV 120: connected arms from the screen
edge, D08O5 draw, restow, private save/load, dive; replay zero failures.
Limits: one moat route, default costume. [Note 346](documentation/346-d14g-swim-contact-arms.md).

Accepted 2026-11-01. User: "perfect. lock it in, close it out". Closeout,
commit and push authorized.

### D14H - Jetpack first-person arms: FOV-independent viewmodel projection

**Status: Done (user-accepted 2026-11-02). Dependencies: D14B, D14G, D36K1, D11F.** In first-person
jetpack flight Duke's disembodied arms do not appear to use the safe
viewmodel projection introduced with the wider field of view. Review the arms
during flight and give them the same projection.

Initial research: the D14B note
([note 297](documentation/297-d14b-weapon-projection.md)) records that
jetpack's unarmed arms retain world projection (only `DisplayHand` and its
connected arm chains are FOV-independent); D14G already moved holstered
swimming arms onto the fixed projection, and D36K1
([note 273](documentation/273-d36k1-jetpack-arms.md)) owns unarmed jetpack
arm continuity. Follow D14G's approach; read the D36 audit and the D36H5
notes (305/306) first, per AGENTS.md.

**Acceptance:** jetpack arms (unarmed and armed, every flight pose and lean)
keep the same size and framing at every world FOV, with no disembodied or
clipped segments; ground, swim and other arms unchanged; the user's playtest.

Work log 2026-11-02: cause confirmed. D36K1 claims empty-handed flight arms as
`Body` (world projection) while flight legs and armed display arms use the
fixed H=256 projection; since FOV 90 became the default (H=160) the arms draw
at about 62% of their accepted framing (36% at FOV 120), shrinking toward the
centre and exposing open upper-arm ends when looking down. New owner
`FlightArm` keeps Body's world depth/clipping/fade and takes the fixed
projection; armed flight was already correct. Native suites pass (flight
group and D14B packet group extended). Live slot-8 LEVEL05 flight, FOV
60/90/120 at 60 Hz and FOV 120 at 120 Hz: identical arm framing at every FOV,
connected with the legs, armed unchanged; replay zero failures. Limits: one
route, default costume, corner slices at steep pitch as in D36K1.
[Note 358](documentation/358-d14h-jetpack-arm-projection.md).

Revision 2 (2026-11-02), after the user's first look: "they dont seem to move
with duke if you turn left and right quickly", "duke's whole body in fact
really", "and crouching i suppose too". Gameplay heading already follows the
mouse each game update; the late camera turns each redraw while Duke's
skeleton is interpolated at 30 Hz, so his body trailed fast turns. Late
first-person redraws now draw Duke alone through his interpolated yaw at the
late pitch (equivalent to turning his whole body to the mouse yaw), in every
first-person state, holstered or armed. Fixed-offset redraw test at 120 Hz:
old build swings arms/legs, new holds them; replay zero failures; native
suites pass. Needs the user's live turn playtest.

Accepted 2026-11-02 (revision 2). User: "that's a winner. lock it in and
close it out". Closeout, commit and push authorized.

### D35 - Comprehensive first-person camera transition audit

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D11, D11E, D11F, D10B.

**Closeout:** User accepts the first-person work and explicitly closes the overarching D35 job. D35K bounded research and D35K2 toilet use are accepted. D35 remains the category for future reported first-person drops: open a focused child job, reuse notes 222/233-239, and investigate the actual event before widening support. No autonomous campaign sweep or next job is started. Unplayed routes remain evidence limits, not a claim of exhaustive certification. D35N1 is a separate missing-voice report and remains Todo.

Historical research scope follows; accepted child-job statuses above supersede the original planning language.

Research, documentation and planning only. Trace every discovered direct and
indirect perspective entry point across camera ownership, animation/state,
input, rendering, scripts and world lifetime; distinguish code-confirmed,
user-reported and suspected events. Verify accepted swimming/jetpack support
and existing settings before proposing work. Doors and mirror interactions
were explicitly added by the user during the audit.

**Deliverable:** [Note 222](documentation/222-d35-first-person-audit.md), including
source inventory, action matrix, configuration architecture, dependencies,
risks, acceptance criteria and implementation order. D35A-D35N below are
independent Todo jobs; none is started by delivering this research.

**Acceptance:** source findings are reviewable and evidence limits explicit;
existing features are not duplicated; new jobs have current/desired behavior,
source references, perspective policy, dependencies, reproduction/tests and
regression criteria. Campaign event tracing remains explicit work rather than
an unsupported claim of complete gameplay coverage. No gameplay, camera,
animation or settings edits, commit or push are part of this research result.

### D35A - Shared action-perspective policy and persistent Advanced Settings foundation

**Status:** Done. **Dependencies:** D35.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** Presentation eligibility and fallback are embedded in first_person_update/state; global VIEW persists, but no action view choices exist. Launcher text still incorrectly says swimming/jetpack force third person.

**Desired result:** Centralize presentation-only action classification, capability, effective view and reason without changing current action support. Preserve global view, third-person behavior and safety guards. Provide shared enum validation/menu-row helpers and replay-safe state; correct stale launcher wording.

**Perspective/configuration:** No inert user-facing action rows. Define first / legacy-third schema contract for the action jobs; defaults must preserve existing global view and current supported FP behavior.

**Source references:** recomp/src/ttk/first_person.inc:first_person_update; modern_controls.cpp:state, camera_world_reset, RenderState; pc_input.cpp:camera_action; game_menu.cpp:make_screens/apply_live/write_settings; player_profiles.py:validate_controls/load/absorb_camera_state; run.py.

**Reproduction and tests:** Compare current action matrix before/after refactor using private fixtures; profile migration, malformed values, side-file merge, menu cancel/confirm, reset and relaunch; P during temporary fallback; replay snapshots.

**Acceptance:** Existing supported/unsupported actions retain behavior; presentation cannot grant movement/aim/fire; one resolver explains effective view; preference remains intact across reset; settings helpers integrate existing atomic persistence; stale text corrected.

**Regression risks:** Accidental state() side effects, stale replay workers, schema loss, rewriting VIEW, broadening owner/identity permissions.

### D35B - Swimming perspective option using accepted first-person swimming

**Status:** Done. **Dependencies:** D35A, D11E.
**Accepted:** [Note 224](documentation/224-d35b-swimming-perspective.md).
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** D11E supports surface/dive/underwater FP. SWIMMING selects pace only; no temporary legacy perspective option.

**Desired result:** Reuse D11E eye and weapons. Add independent swim presentation choice; mantle exit remains governed by mantle policy.

**Perspective/configuration:** SWIMMING VIEW: First-Person / Legacy Third-Person, default First-Person when global FP is selected; persist and apply live.

**Source references:** recomp/src/ttk/first_person.inc:first_person_swimming/first_person_anchor; modern_controls.cpp:state swim_eye; game_menu.cpp; player_profiles.py; documentation/216-d11e-first-person-swimming.md.

**Reproduction and tests:** Private surface and underwater saves in two levels; dive/resurface/holster/fire; switch view and setting in water; water-to-mantle boundary; both pace modes; pause/load/relaunch.

**Acceptance:** FP reproduces accepted D11E behavior; legacy shows accepted third-person swim and returns to current global view on exit; no pace/physics changes; no false claim that mantle support ships here.

**Regression risks:** Water-plane crossings, equip-bit transient, double shoulder compensation, legacy view accidentally forcing FP on exit.

### D35C - Jetpack perspective option using accepted first-person flight

**Status:** Done. **Dependencies:** D35A, D11F.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** D11F retains FP across Modern/Classic/EDuke32 flight. JETPACK chooses movement/fuel scheme, not perspective.

**Accepted:** Live, persistent JETPACK VIEW with First Person / Third Person labels. User accepted and explicitly authorized closeout. [Note 225](documentation/225-d35c-jetpack-perspective.md) records verification and coverage limits.

**Desired result:** Add independent flight presentation choice without changing flight, fuel, platform hovering or reflected flames.

**Perspective/configuration:** JETPACK VIEW: First-Person / Legacy Third-Person; default First-Person under global FP; live and persistent independently of next-launch flight scheme.

**Source references:** recomp/src/ttk/first_person.inc:first_person_anchor; modern_controls.cpp:jetpack_flying_early/state jet_eye/settle; jetpack.inc; game_menu.cpp; player_profiles.py; documentation/220-d11f-first-person-jetpack.md.

**Reproduction and tests:** Two levels, all three schemes; J start/off, takeoff/landing, fuel exhaustion, flight save/load, water/contact interruption, platform descent and mirror flames; switch perspective while airborne and relaunch.

**Acceptance:** FP retains accepted transition bridges; legacy returns gracefully after off/fall/landing; movement/fuel/root and audio remain unchanged; setting persists and global third person wins.

**Regression risks:** One-frame pack-cleared pose, previous-mode-10 landing, stale floor/hover state, reflection flags and replay.

### D35D - First-person push/pull and grab presentation

**Status:** Done (user-accepted 2026-10-24). **Dependencies:** D35A, D08T3, D08T7.
**Accepted:** [Note 226](documentation/226-d35d-first-person-push-pull.md). Stable body eye, native hands and saved First Person / Third Person choice; user accepted and explicitly authorized closeout.

**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** Holstered pushable 119..121 in 0/0 explicitly yields the eye to avoid the object face.

**Desired result:** Implement stable eye/interaction hands with object-face clearance through alignment, grab, continuous push/pull and release. Keep native movement lanes and input restrictions.

**Perspective/configuration:** PUSH/PULL VIEW: First-Person / Legacy Third-Person; accepted modern orbit as legacy, FP default once supported.

**Source references:** recomp/src/ttk/push.inc:push_state_early/push_manual; first_person.inc:first_person_update; pc_input.cpp:push_owned pad bridge; documentation/208-d08t3-manual-push-pull.md.

**Reproduction and tests:** Private alley dumpster plus later-era Duke-symbol block; grab from both directions, travel to limits, release/regrab, obstruction, E-to-mantle and weapon redraw; camera pitched/turned near face.

**Acceptance:** No third-person frames in FP through complete interaction; eye never inside block; hands read correctly; original displacement, limits, fire suppression and redraw preserved; legacy and global-third routes match baseline.

**Regression risks:** Camera-relative push direction feedback, face clipping, free-push lanes, weapon restoration and object ownership.

### D35D1 - First-person push/pull: a steady grab, not the native turn-and-shove arms

**Status: Done (user-accepted 2026-11-02). Dependencies: D35D, D36D, D36H5, D08T3.** Fixture: the
player's UI slot 12 (pushable block nearby; test on private copies only). In
first person, pushing the dumpster shows Duke's arms going through the
third-person push animation, where he turns around and shoves with his back;
it looks good in third person but in first person the arms flail. The user
wants a stable first-person grab: Duke has hold of the object in front of
him, with no turning or twisting, for push, pull, grab and release.

Initial research: D35D ([note 226](documentation/226-d35d-first-person-push-pull.md))
anchored the eye for push/pull (anims 119/120/121) but kept the arms native at
world depth (D36 audit, [note 257](documentation/257-d36-first-person-presentation-audit.md),
push row). Start from the D36H5 contact-arm design and repair guide
([note 305](documentation/305-d36h5-contact-arm-rig.md),
[note 306](documentation/306-d36h5-interaction-arm-coverage.md)); identify the
actual presentation owner per frame before changing anything. Third person
keeps the native animation.

**Acceptance:** in first person, grab, push, pull, reversal and release show
both hands steadily on the object (slot 12 and the other accepted push
fixtures, slots 5 and 8), at 120 and 180 Hz; third person and push physics
unchanged; Vanilla unchanged; the user's playtest.

Work log 2026-11-02: owner confirmed as the D36H5 contact rig (Push), whose
targets were the native 119/120 hands; in Duke's body frame they sweep several
hundred units and the native shoulders swap sides. New first-person grip
(`push_grip`, first_person.inc): hands on the held object's near face from its
push box (85-unit inset, chest height under the top edge, 150 each side within
its width), rotations from the native grab-121 hold mirrored for both sides,
fingers pitched up; arms solved by the existing rig. 119/120 grip fully, 121
blends by nearness to the hold; `push_letgo` (render-replay snapshot) keeps the
grip through the release's replay of 121 until its let-go phase, removing a
grip-drop-grip dip. `DNTTK_FP_PUSH_GRIP=native` compares. Animation, physics,
eye, third person and Vanilla unchanged. Native controls (new D35D1 group,
both costumes), near, aim and input suites pass. Private runs: slot-12
dumpster at 120/180 Hz (with mid-grip save/load; replay 0 failures), native
comparison, third person, archived D35D LEVEL06 symbol blocks at 120/180 Hz.
Board slots 5 and 8 no longer hold push scenes; the archived D35D fixtures
stood in. Not rerun: LEVEL01 box, LEVEL05 movable dumpster. Executable
`84fa6e7fb6966343175c03307c1ef20339b2ceafb32d9a2568cde07a6cc69e77`.
[Note 359](documentation/359-d35d1-first-person-push-grip.md). Needs the
user's playtest.

Revision 2 (2026-11-02), after the user's first look: "functionally, it works
perfectly. but what is happening is the arms appear to vibrate". Cause: in
120/180 Hz redraws the eye (and objects, Duke's joints) are interpolated
between 30 Hz game ticks, but the grip was anchored to Duke's last-tick
position (+4..+12); traced in redraws the eye ran up to 29 units ahead of it,
so the hands stepped against the view while pushing. Turning added a second
mismatch: D14H draws Duke through the body camera while the object uses the
late camera. The grip now anchors to the interpolated eye and, in late
redraws, is placed through the late camera (new `replay_late_rotation`). In-draw
trace: hand screen position constant across push redraws; still-hold hand
moves with the world within about 0.2 px. Suites pass; slot-12 route at
180 Hz, replay 0 failures. Executable
`6872f692f956bd70fda32c29c9fc099246cdbb3e878f0c5d7ee09489f3946bab`.

Accepted 2026-11-02 (revision 2). User: "absolutely solid work! fantastic,
and accepted" and "lets lock it in and close us out". Closeout, commit and
push authorized. Follow-up D17AY recorded for draw-time reads of tick-rate
state.

### D35E - First-person ladder climbing lifecycle

**Status:** Done. **Dependencies:** D35A, D08J3, D08U, D08J1.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** Attached ladder and top mount use camera-only orbit; traversal policy suppresses FP, including mount/exit transfers.

**Desired result:** Implement comfortable panel-relative eye across ground/airborne/top entry, climb, bottom hang, drop and top/bottom exits; retain look without rotating collision or root motion.

**Perspective/configuration:** LADDER VIEW: First-Person / Legacy Third-Person; centered accepted orbit for legacy.

**Source references:** recomp/src/ttk/modern_controls.cpp:climb_state_early/mantle_state_early/traversal_state_ready; ladder_top.inc:ladder_mount_early/ladder_bottom_hang; pc_input.cpp:traversal pad bridge.

**Reproduction and tests:** Historical alley slot-5/slot-6 routes using verified private copies; top mount, floor-level and high-bottom ladder, enemy at top, failed grab, Space drop and save on rung; check transfer 147/148/156/207/210/211.

**Acceptance:** Full ladder lifecycle remains FP when requested; safe near-panel eye and readable hands; no lost movement or unexpected view frame during shared transfer states; existing top-exit enemy handling and third person unchanged.

**Regression risks:** Transfer classification shared with poles/hangs, top-exit wall penetration, auto shoulder return and held-input direction.

**Accepted evidence:** [Note 227](documentation/227-d35e-first-person-ladders.md). LADDER VIEW, stable eye, native hands, guarded mount/drop bridges; build/native suites and 94 profile tests pass. Private lifecycle and menu checks recorded; user accepted and explicitly authorized closeout.

### D35F - First-person chain, pole and pole-class lamppost climbing

**Status:** Done. **Dependencies:** D35A, D35E, D08J5.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Accepted behavior:** Pole-class 0x400 attached objects retain first person through shared catches 147/148, transfer 154, climb 192..195 and exits 191/196. CHAIN/POLE VIEW also offers the existing third-person orbit. Lampposts are classified by object flags, not scenery name.

**Desired result:** Provide axis-aware eye and hand placement during climb/circle/top mount/descent/drop. Adapt view-relative input interpretation without altering native attachment.

**Perspective/configuration:** CHAIN/POLE VIEW: First-Person / Legacy Third-Person.

**Source references:** recomp/src/ttk/pole_climb.inc:pole_axis/pole_mount; ladder_top.inc:pole_sidestep_ready/pole_view_from_front; modern_controls.cpp:climb_state_early/mantle_state_early; pc_input.cpp:1198.

**Reproduction and tests:** Private historical slot-3 chain, another pole and confirmed climbable lamppost if present; circle both ways with held A/D and mouse look, top mount, lower floor, no-floor drop, top exit and failed catch.

**Acceptance:** FP remains stable and clear of column; input never reverses unexpectedly on view-side change; mount/descent positioning and native exit logic preserved; list unlocated object variants honestly.

**Regression risks:** Front/back A/D latch, 225-unit axis offset, mount half-turn comfort, phantom upper-floor exit and pole/ladder transfer ambiguity.

**Accepted evidence:** [Note 228](documentation/228-d35f-first-person-poles.md). Build, native suites and 95 profile tests pass. Two private chain types, top/airborne catch, circle/look, descent/exits/drop, save/load, menu and Third Person comparison checked. User accepted and explicitly authorized closeout. Separate solid-pole/lamppost and natural no-floor-bottom coverage are not claimed.

### D35F1 - Chain disappears near the top while climbing

**Status:** Done. **Dependencies:** D35F.

**Accepted:** [Note 231](documentation/231-d35f1-chain-visibility.md). The attached chain spans rooms; it now receives its own native draw when its owning room leaves the visible list. Both perspective choices, upper exit, descent, Ctrl drop and replay checked. User accepted and explicitly authorized closeout on 2026-10-25.

**User report (2026-10-25):** Load current UI slot 10, climb up the chain,
and the chain disappears as Duke gets closer to the top. The user requests
that this be fixed later. This entry is backlog only; no investigation,
fixture capture, gameplay edit, build or launch is authorized by recording it.

**Desired result:** Keep the chain visibly coherent as Duke approaches and
reaches its top, without breaking first-person presentation or climbing.

**Reproduction and investigation when selected:** Copy the then-current
slot-10 save into an isolated test directory and verify its identity against
the reported location. Trace where the disappearance starts while ascending;
compare First Person / Third Person, looking up/down, descent and the top
exit. Establish the cause from rendering/camera evidence before choosing a
fix; no cause or specific culling failure has been established.

**Acceptance:** The reported chain remains visible where it should be through
upper-chain approach and exit; no new visibility-through-walls artifacts;
chain movement, attachment, drop/exit and other traversal stay intact.
Compare both action perspective choices and preserve Vanilla. Record verified
coverage and any remaining limits, then present the result for user playtest.

### D35G - First-person mantles, ledge hangs and water climb-outs

**Status:** Done, user-accepted 2026-10-24. **Dependencies:** D35A, D35E, D08V, D08X, D08T8.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Accepted implementation:** MANTLE VIEW keeps the eye and native hands through ledge hangs, shimmy, pull-up and water climb-outs, or selects the existing third-person view. Schema 46; see [note 229](documentation/229-d35g-first-person-mantles.md) for verification and coverage limits.

**Desired result:** Implement an eye path over the lip with hang/shimmy/pull-up hands and comfortable orientation; retain native motion/events for ground/object/airborne/water entry and abort/drop.

**Perspective/configuration:** MANTLE VIEW: First-Person / Legacy Third-Person, including water climb-out. Ladder/pole exit ownership follows the shared transfer classification.

**Source references:** recomp/src/ttk/first_person.inc:first_person_update swim_exit; modern_controls.cpp:mantle_state_early; ledge_reach.inc; swim.inc; pc_input.cpp:ledge drop/restore; documentation/214-d08t8-alley-platform-hang.md.

**Reproduction and tests:** Private sewer and alley platform, crate/furniture and level-geometry ledges; hold-E airborne catch, shimmy, pull-up, Space/drop, blocked pull-up, water exit and save in hang; both view choices.

**Acceptance:** Continuous FP through lip crossing without ceiling/ledge penetration or invalid hands; interrupted moves restore current selected view; input, root/collision and weapon stow/redraw match baseline; water-to-land never briefly uses orbit in FP.

**Regression risks:** Large root/posture changes, head inside lip, stale previous water mode, shared ladder/pole exits, near-wall clipping and visual nausea.

### D35G1 - Slot-5 jump edge/corner screen artifact

**Status:** Done, user-accepted 2026-10-25. User: "lock it in and close it out!!".
Implementation, tests, backlog research and documentation closeout authorized.
Explicitly selected for immediate work, 2026-10-25.
Report: after loading current slot 5, jumping into roughly the ledge edge/corner,
rather than cleanly landing on top or holding E to mantle, puts an artifact on
screen. Reference: local `research/screencaps/jump-artifact.mp4` (37.9 seconds).
The video shows a brief broad skin-colored polygon across the first-person view
on several edge contacts, alongside separate deliberate mantle sequences.
Use a fresh private slot-5 copy; distinguish body/arm geometry, clipping, camera
and replay causes before changing code. Acceptance: repeated edge/corner jumps
without the artifact at 60/high refresh; preserve ordinary landings, E catches,
first-person mantle hands, weapon presentation, global/action Third Person and
Vanilla. The gadget jobs remain Todo. User acceptance and closeout are recorded above.

**Candidate (2026-10-25):** Reproduced native armed step-up 134. Its hidden
viewmodel arms were incorrectly admitted by the mantle near-cull bridge. Clear
the interaction-arm flag when hiding those limbs. Regression fails before and
passes after; player build and four native suites pass. Repeated 60/120 routes
remove the skin-colored artifact; E-stowed native hands remain visible.
[Evidence and limits](documentation/242-d35g1-jump-edge-artifact.md).
User accepted the candidate and authorized closeout; no gameplay/settings change
to either gadget.

### D35H - First-person overhead and monkey-bar traversal

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D35A, D35G, D08J4.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** D35G already supplies first-person eye/hands for ceiling hangs. The accepted D35H implementation separates OVERHEAD VIEW from MANTLE VIEW by ceiling-object ownership, retaining existing presentation and movement. [Note 230](documentation/230-d35h-overhead-perspective.md).

**Desired result:** Implement below-ceiling eye and reachable hand presentation through catch, turn, advance, transfer and drop, without converting body turns into camera roll/sway.

**Perspective/configuration:** OVERHEAD VIEW: First-Person / Legacy Third-Person.

**Source references:** recomp/src/ttk/ceiling_hang.inc:ceiling_hang_state/ceiling_hang_update; modern_controls.cpp:mantle_state_early; pc_input.cpp:ceiling_hang_ready bridge.

**Reproduction and tests:** User reports working monkey bars from current slot 9; its copied save loads at the moat walkway and the route remains unverified here. Private historical overhead slot-3 grate fixture verified; diagonal travel, turn-in-place, mid-span hold, low ceiling, edge/drop and regrab; compare body trajectory under both settings.

**Acceptance:** FP eye clears ceiling and arms; no feedback loop between camera-relative movement and body heading; no mid-span accidental drop; legacy/Vanilla movement and handoffs preserved.

**Regression risks:** Mode-7 overlap with ledge shimmy, camera/body feedback, ceiling clipping and excessive animation sway.

### D35I - First-person climbing walls

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D35A, D35E.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Current behavior:** Mode-3 climb predicate covers wall animations 198..205, but historical free-camera testing did not include a climbing-wall route.

**Desired result:** Verify the user-provided current UI slot-10 wall and transfer states; add panel-clear first-person climbing presentation. User selection 2026-10-25 also authorizes fluid Modernized WASD and diagonal wall climbing, retaining native attachment, boundary/transfer checks and Vanilla behavior.

**Perspective/configuration:** WALL CLIMB VIEW: First-Person / Legacy Third-Person.

**Source references:** recomp/src/ttk/modern_controls.cpp:climb_state_early; ladder_top.inc:pole_sidestep_ready comments; documentation/124-d08j3-climb-camera.md.

**Reproduction and tests:** Locate native climbable-wall object and record flags/overlay; test vertical/lateral movement, top/bottom/side boundary, transfer/drop, armed approach and saved attachment.

**Acceptance:** Actual wall route documented before implementation; continuous safe FP for supported lifecycle, independent mouse look and coherent hands; fluid Modernized WASD with normalized diagonals and prompt stop/reversal; retain native wall boundaries, transfers and exits, with ladder/pole and Vanilla behavior unchanged.

**Regression risks:** Unverified object mapping, side edges, assumed ladder equivalence and broad animation ranges.

### D35I1 - Climb down from a climbing-wall top (slot 7)

**Status: Done (user-accepted; closeout authorized 2026-10-28).** Selected 2026-10-28. User reports no climb-down
prompt while standing above the climbing wall reached in current UI slot 7.
Investigate the actual saved location and native attachment/entry rules using
private copies; provide a bounded Modernized climb-down entry and prompt.
Preserve existing wall movement, exits, ladders, poles, camera policy and Vanilla.
Acceptance: reproduce the missing prompt, verify deliberate entry and downward
travel plus cancellation/reload and unrelated-edge rejection; run relevant
native regressions and obtain user gameplay acceptance. The report that this
may be the only such location remains unverified.

Work log: private slot-7 reproduction confirmed the missing wall-family entry.
Accepted implementation adds E prompt and native 155-to-198 wall attachment, preserving lateral
position. Player build, controls/input/aim/near suites and private 60/120-setting
descent/reload/release probes pass. Existing local camera/settings changes retained.
Evidence and limitations: [note 313](documentation/313-d35i1-wall-top-descent.md).
User: "lock it in, close it out". Implementation, documentation, commit and push
closeout authorized. No next job selected.

### D35J - Damage and native inventory transient perspective gaps

**Status:** Done. **Dependencies:** D35.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The bounded research result is accepted; enemy-hit variants, knockdown, heavy-fall and water-hazard behavior remain unverified, with no speculative damage fixes or broader acceptance.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**User-supplied reproduction leads (2026-10-25):** Idle animations may leave
first person: load any state and stand still. Current player UI slot 10:
interact with an NPC. Slot 11: Miner 69er using a lever. Slot 8: use a key.
Slot 9: use a crystal. These are investigation leads, not independently
reproduced causes. Use private copies of these current saves. Include idle
animation coverage in the transient-view investigation; trace the interaction
cases before assigning ownership. Lever/key/crystal use may belong to D35M,
and NPC/scripted camera ownership may belong to D35K. Do not silently expand
D35J into implementation of those jobs or presume a shared cause.

**Research evidence (2026-10-25):** [Note 233](documentation/233-d35j-transient-perspective-research.md). All five supplied cases reproduced on private copies; held native inventory also reproduced. Idle 64/65/66, NPC 277, key/crystal 266 and lever 118 fail the supported-pose gate; native held inventory fails the separate 0x200 guard. Normal camera ownership and the selected first-person preference survive, and each observed case recovers. One survivable RPG blast retained first person; enemy-hit variants, knockdown, heavy-fall and water-hazard coverage remain unverified. The investigation itself made no gameplay changes. D35J1-J3 and D35M subsequently implemented the reproduced gaps.

**Current behavior:** state rejects unknown lower poses/mode histories and +0x224 bit 0x200 outside documented water exceptions. Event-specific hit reactions are not comprehensively mapped.

**Desired result:** Research exact damage/stagger/knockdown/hazard and held-inventory transitions with traces. Produce narrowly scoped fixes only for reproduced gaps; do not pre-authorize broad guard relaxation.

**Perspective/configuration:** Propose DAMAGE REACTION VIEW only if meaningful committed reactions need a choice. Ordinary inventory/pause should preserve view without a redundant toggle.

**Source references:** recomp/src/ttk/modern_controls.cpp:state/hold_button_sampled; pc_input.cpp:Circle tapping; first_person.inc:first_person_update; documentation/222-d35-first-person-audit.md sections 1-2.

**Reproduction and tests:** Private natural enemy hit, explosive knockback, heavy fall, water hazard and held holster/inventory interaction; capture lower/upper anim, flags, modes and owner before/during/after. Separate death from survivable damage.

**Acceptance:** Research maps every reproduced event to reason and ownership with baseline evidence; identifies retained input restrictions and recovery; creates focused implementation child jobs only where necessary. No guessed damage animation IDs or unproven fixes.

**Regression risks:** Granting input during committed hurt, suppressing damage/death logic, confusing equipment flag with script bit, overselling no-repro cases.

### D35J1 - First-person idle fidgets without perspective drops

**Implementation evidence (2026-10-25, before acceptance):** [Note 234](documentation/234-d35j-use-and-idle-first-person.md). First-person idle and held inventory retained; verified NPC, lever, key/door and crystal actions use the shared USE INTERACTION VIEW choice. Player build, four native suites and 99 profile tests pass. Private routes at 60/120, live setting, global third person, pause/load and a Vanilla action-load check pass. User subsequently accepted the delivered result and authorized closeout. Broader variant coverage is not claimed.

**Status:** Done. **Dependencies:** D35J.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The delivered implementation is accepted. Scope and test limits remain recorded in [note 234](documentation/234-d35j-use-and-idle-first-person.md) and [note 235](documentation/235-d35n-mirror-interaction-and-opacity.md).
**Evidence:** [Note 233](documentation/233-d35j-transient-perspective-research.md).

Keep selected first person through native idle fidgets without widening the
movement lease for unrelated states. Slot 10, holster, Caps Lock run mode,
then stand still reproduces lower poses 64/65/66; 67-69 are source-backed
selector candidates requiring coverage. Native idle selection, animation and
movement cancellation must remain intact. Acceptance: comfortable eye/body
presentation across verified variants, normal movement/weapon recovery,
first/third-person and Vanilla regressions, including high-refresh replay.
Investigate stationary Walk-bit timer reset separately before changing input.

### D35J2 - Preserve first person through held native inventory

**Implementation evidence (2026-10-25, before acceptance):** [Note 234](documentation/234-d35j-use-and-idle-first-person.md). First-person idle and held inventory retained; verified NPC, lever, key/door and crystal actions use the shared USE INTERACTION VIEW choice. Player build, four native suites and 99 profile tests pass. Private routes at 60/120, live setting, global third person, pause/load and a Vanilla action-load check pass. User subsequently accepted the delivered result and authorized closeout. Broader variant coverage is not claimed.

**Status:** Done. **Dependencies:** D35J.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The delivered implementation is accepted. Scope and test limits remain recorded in [note 234](documentation/234-d35j-use-and-idle-first-person.md) and [note 235](documentation/235-d35n-mirror-interaction-and-opacity.md).
**Evidence:** [Note 233](documentation/233-d35j-transient-perspective-research.md).

Slot 10, hold Scroll Lock, then release: native +0x224 bit 0x200 drops the eye
view while normal ownership and lower idle 63 remain. Keep presentation in
the selected view without giving movement, fire or interaction permission
through the inventory lock. Acceptance: native selection UI and hold/release
semantics remain usable; taps still holster/draw; input recovers correctly;
no redundant view toggle; verify pause/load, third person and Vanilla.

### D35J3 - First-person NPC interaction presentation

**Implementation evidence (2026-10-25, before acceptance):** [Note 234](documentation/234-d35j-use-and-idle-first-person.md). First-person idle and held inventory retained; verified NPC, lever, key/door and crystal actions use the shared USE INTERACTION VIEW choice. Player build, four native suites and 99 profile tests pass. Private routes at 60/120, live setting, global third person, pause/load and a Vanilla action-load check pass. User subsequently accepted the delivered result and authorized closeout. Broader variant coverage is not claimed.

**Status:** Done. **Dependencies:** D35J.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The delivered implementation is accepted. Scope and test limits remain recorded in [note 234](documentation/234-d35j-use-and-idle-first-person.md) and [note 235](documentation/235-d35n-mirror-interaction-and-opacity.md).
**Evidence:** [Note 233](documentation/233-d35j-transient-perspective-research.md).

Slot 10 E reproduces native lower/upper 277, action bit 0x40 and ordinary
camera ownership. Provide physically appropriate eye/hand presentation for
this committed interaction without changing its callbacks, quotes, timing,
alignment or weapon return. Acceptance: armed/holstered entry, repeated use,
view recovery, first/third-person and Vanilla checks; retain native action
restrictions and verify the desired action-view policy before exposing a row.
Other NPC variants require explicit evidence rather than numeric-range leases.

### D35J4 - Slot-9 time crystal forces third person (bug 5)

**Status:** Done. **Dependencies:** D35J, D35J1-J3, D35M (Done).
**Selected (2026-10-25):** User instructed "let's go with D35J4 straight away".
Only D35J4 is selected; the other four reports remain backlog.

**Accepted and closed (2026-10-25):** User instructed "lock it in and close it out!".

**Evidence:** [Note 236](documentation/236-d35j4-time-crystal-perspective.md).
Slot-9 crystal take/return poses 268/267 now use the existing first-person
interaction policy. Build and four native suites pass; private 60/120 routes,
third-person choices, pause/load and Vanilla action-load checks pass within
the recorded limits. The delivered candidate is user-accepted; broader crystal
and campaign coverage is not implied.

**Report:** The time crystal in current slot 9 puts Duke into third person.
The user requests the same first-person treatment as the accepted D35J/J1-J3
work. This is a new reported gap despite earlier bounded crystal-use coverage;
retain that historical evidence without claiming this case is already fixed.

**Acceptance:** Reproduce the reported time-crystal action on a private slot-9
copy, identify its lifecycle, and preserve first person under the applicable
USE INTERACTION VIEW policy. Retain the deliberate Third Person choice,
native crystal events/progression, action restrictions and view recovery.
Verify entry, completion, weapon return and prior idle/inventory/use cases.
Reference: [note 233](documentation/233-d35j-transient-perspective-research.md)
and [note 234](documentation/234-d35j-use-and-idle-first-person.md).

### D35J5 - First-person frozen effect with blue vision

**Status: Done (user-accepted 2026-10-26). Dependencies: D35J, D36G (Done).** User reports freezing returns
the view to third person. Requested result: remain first person with blue vision,
using Duke 3D as the desired visual reference. Verify the real frozen-state flags,
animations and thaw/death transitions before changing presentation eligibility.
Acceptance: blue frozen vision across the full viewport, no unwanted third-person
cut, clean restoration on thaw/death/continue/load and view changes; native frozen
movement/fire restrictions, duration, damage and shattering remain authoritative.
Preserve explicit third-person choice, Vanilla, HUD readability, water/gas/night
vision effects and replay at 60/120. Slot-11 reproduction and candidate evidence:
[note 275](documentation/275-d35j5-frozen-first-person.md). Natural death/continue
and water/gas coverage remain open.

### D35K - Native scripted and special-camera event inventory

**Status:** Done (bounded research accepted with D35 closeout, 2026-10-25). **Dependencies:** D35.

**Selected scope (2026-10-25):** User supplied current UI slot 5, where opening a door returns Duke to third person, and authorized investigation and a fix. Investigate this event within D35K, reuse accepted use-action policy where supported, and record broader event inventory limits. D35 review does not prevent this bounded investigation.
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Accepted follow-up (2026-10-25):** User: "lock it in, close it out!". The slot-5 perspective fix and visible right-leg kick are Done as D35K1. The broader scripted-event inventory remains Todo; no next job is started.

**Slot-5 result (2026-10-25):** Reproduced native hinged-door pose 116 with normal camera ownership. Added it to the existing USE INTERACTION VIEW presentation-only policy; no script override. Player build, four native suites, private 60/120 routes, view-choice comparisons, pause/load and Vanilla action-load completion pass. User accepted the delivered result. [Note 237](documentation/237-d35k-slot5-door.md).

**Current behavior:** Alternate camera dispatch and native entry/exit are confirmed; indirect overlay callback-to-event mapping remains incomplete.

**Desired result:** Research per-level gameplay special views versus intentional cinematics/frontend; enumerate owner/pivot/inhibit writes, callbacks and return paths. Feed D22 coverage and create bounded per-mechanism jobs.

**Perspective/configuration:** Only propose SCRIPTED GAMEPLAY VIEW for verified controllable events; do not add a blanket cutscene override. Door/mirror choices remain in D35M/N.

**Source references:** Generated resident addresses 0x80025e88..0x80025ee8, 0x8003b36c/0x8003b474/0x8003b4f4, original callbacks/overlays; modern_controls.cpp:state/hook_body; documentation/222-d35-first-person-audit.md native evidence.

**Reproduction and tests:** Use private route captures for level openings, interaction camera objects, boss/event views, potential turret use and endings; trace writes to camera owner/pivot, player camera pointer and script bit; cross-check event names against gameplay.

**Acceptance:** Catalog confirmed entry/exit addresses, callers, exact triggers and intended player control; distinguish cinematic exemptions from missing playable FP; every unresolved mapping remains named; no unsafe global script-bit override.

**Regression risks:** Indirect overlays, transient object lifetime, two-player cameras, bypassed normal hook, misclassifying a cinematic as player camera.

**Accepted research batch (2026-10-25):** [Note 238](documentation/238-d35k-interaction-inventory.md) confirms toilet pose 275 fallback at 60/120, inventories all shared selector classes, and resolves nine alternate-camera trigger subtypes in LEVEL00/05/09. Exact event naming and remaining natural routes are explicit coverage limits. D35K2 is now Done. Further work is driven by user reports in focused D35 child jobs; no exhaustive campaign pass is pending as a condition of this accepted closeout.

### D35K1 - Slot-5 hinged-door first person and visible right-leg kick

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D35M.

**Scope:** Bounded follow-up identified during selected D35K investigation.
Pose 116 retains first person under USE INTERACTION VIEW and visibly kicks
the door with Duke's right leg. Existing kick meshes/poses are reused with
native channel timing; no added damage, sound or door callback changes.

**Acceptance:** User confirms the perspective fix works and explicitly closes
out the right-leg polish: "lock it in, close it out!". Build, four native
suites, 60/120 routes, Third Person comparisons, pause/load and Vanilla
saved-action completion pass. [Note 237](documentation/237-d35k-slot5-door.md)
retains evidence and limits. Broader D35K event mapping is not completed.

### D35K2 - Toilet first-person use presentation

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D35M, D35K.

Current UI slot 5's type-80 toilet uses committed native pose 275 and falls
back to third person with ordinary camera ownership, then recovers. Reproduced
at 60/120; [note 238](documentation/238-d35k-interaction-inventory.md).

**Accepted:** User: "its perfect! lock it in." [Note 239](documentation/239-d35k2-toilet-first-person.md). Pose 275 retains first person under USE INTERACTION VIEW; user accepts the delivered framing/comfort. Explicit closeout includes implementation and documentation commit/push.

**Scope:** Add bounded presentation support under USE INTERACTION VIEW. Preserve native action lock, event timing, one-time toilet
effect and weapon stow/redraw. Do not widen locomotion/fire permissions or
script/ownership guards. Type 179 shares metadata class 8 but is unverified.

**Acceptance:** Natural toilet use retains first person when selected; action
and global Third Person choices work; native callback/effect and completion
remain intact; pause/load, 60/120 and successful Vanilla activation comparisons
are verified. Use only private copies. User playtest required.

### D35K3 - Statue interaction drops first person

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D35K, D35M, D35K2.

**User report (2026-10-25):** "slot 5, stand up a new background job,
interacting with the statue goes into 3rd person". Current UI slot 5 is the
reproduction reference. Do not assume it is the earlier slot-5 door or toilet
checkpoint. Statue identity, level, pose, trigger route and camera ownership
remain unverified; this is a user report, not an agent reproduction.

**Scope:** Investigate the statue interaction as a focused D35 follow-up.
Use a private copy of the current save and ordinary interaction input. Trace
lower/upper pose, mode/history, action flags, owner/pivots/back-pointer and
script bit through entry and recovery. Reuse the accepted interaction policy
if supported by evidence; preserve native alignment, timing, effects, callbacks,
weapon restoration and action restrictions. No blanket pose-range or script
bit override. D35/D35K stay Done; no wider audit is reopened.

**References:** [D35 follow-up method](documentation/222-d35-first-person-audit.md),
[interaction selector and camera inventory](documentation/238-d35k-interaction-inventory.md),
[accepted toilet pattern](documentation/239-d35k2-toilet-first-person.md).
Do not identify this statue with an unmapped selector class without tracing it.

**Acceptance:** Naturally reproduced statue use preserves the selected first
person under the appropriate existing action choice; action/global Third Person
and Vanilla retain their intended behavior. Verify action completion/effects,
recovery, pause/load, 60/120 and adjacent accepted interactions using private
copies. Record limits and obtain user playtest acceptance.

**Accepted:** Current slot 5 is LEVEL09 type-926 statue, native pose 279.
The bounded USE INTERACTION VIEW fix and validation are recorded in
[note 240](documentation/240-d35k3-statue-first-person.md). User instructed
"lock it in and close it out". D35K3, D35 and D35K are Done.

### D35L - First-person death and respawn presentation research

**Status:** Done (user-accepted 2026-10-25; no action needed). **Dependencies:** D35 (research prerequisite waived by the user's no-change decision).
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Accepted behavior:** Actor death flags revoke the modern view and the original third-person death presentation runs. Continue/restart/load reset host state and reacquire the selected view when eligible.

**User decision (2026-10-25):** "i like that this one goes into third person and absolutely no issue with how this works"; explicitly requested closing D35L with no action needed. Preserve the existing death and respawn presentation. The proposed first-person death research, DEATH VIEW option and implementation follow-ups are no longer requested.

**Evidence and limits:** Closed on the user's acceptance of existing behavior, not a new agent test of every death variant or recovery route. No gameplay changes, build or launch required. D35 remains Needs review and D35K remains Todo; this decision closes D35L only.

### D35M - First-person door opening and related use interactions

**Implementation evidence (2026-10-25, before acceptance):** [Note 234](documentation/234-d35j-use-and-idle-first-person.md). First-person idle and held inventory retained; verified NPC, lever, key/door and crystal actions use the shared USE INTERACTION VIEW choice. Player build, four native suites and 99 profile tests pass. Private routes at 60/120, live setting, global third person, pause/load and a Vanilla action-load check pass. User subsequently accepted the delivered result and authorized closeout. Broader variant coverage is not claimed.

**Status:** Done. **Dependencies:** D35A.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The delivered implementation is accepted. Scope and test limits remain recorded in [note 234](documentation/234-d35j-use-and-idle-first-person.md) and [note 235](documentation/235-d35n-mirror-interaction-and-opacity.md).
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**D35J baseline evidence (2026-10-25):** [Note 233](documentation/233-d35j-transient-perspective-research.md) reproduces current slot 8 key/door use and slot 9 crystal use (lower 266), plus slot 11 Miner 69er lever use (lower 118). Item upper pose 47 retains first person; the full-body action drops it. All retain normal camera ownership and recover. The implemented presentation-only use path covers these verified selectors.

**Current behavior:** User confirms door opening switches to third person (2026-10-24). E stow/action/redraw and native finder/callback routing are known; the exact door animation, script flag or owner causing this report is not yet traced.

**Desired result:** Reproduce door opening, then provide an aligned, comfortable eye/hand interaction through completion and weapon return. Survey switches/key locks/mission-item insertion for the same mechanism; split materially different systems into children.

**Perspective/configuration:** DOOR VIEW: First-Person / Legacy Third-Person. Share a broader USE INTERACTION VIEW only if research proves the same lifecycle; do not conflate with push/mantle or mirror preference.

**Source references:** recomp/src/ttk/pc_input.cpp:update interaction_pending/restore_owned and input_pad; modern_controls.cpp:interaction_ready/interaction_alive/interaction_restore_ready/state; apartment_interaction.inc:apartment_switch_query; shortcuts.inc:mission_use; native 0x80051890 -> 0x80077f28 and object callback table 0x800dd84c.

**Reproduction and tests:** Locate a user-reported ordinary opening door using private save copy; record E entry, lower/upper animation+track, owner/pivot and flags. Compare armed/holstered, locked/unlocked, repeat press, obstruction, key use, moving door and exit. Test switches separately before grouping.

**Acceptance:** Exact reproduced cause recorded before gameplay edits; FP option retains a physically correct eye and action animation/events; legacy returns to current selected view; no early item use, duplicate callback, collision bypass or lost weapon redraw. Suspected switch/key cases are not claimed fixed without tests.

**Regression risks:** Door callback scripts, alignment root motion, event timing, closed-door transparency, animation-only fix missing alternate camera ownership; old security-card freeze cause remains unresolved.

### D35N - First-person mirror interactions

**Status:** Done. **Dependencies:** D35A.

**Accepted and closed (2026-10-25):** User explicitly requested "lock it all in and close us out" after delivery of the combined idle, inventory, use and mirror candidate. The delivered implementation is accepted. Scope and test limits remain recorded in [note 234](documentation/234-d35j-use-and-idle-first-person.md) and [note 235](documentation/235-d35n-mirror-interaction-and-opacity.md).
**Research:** [D35 central audit](documentation/222-d35-first-person-audit.md).

**Scope added by user (2026-10-25):** Current UI slot 12 reproduces the mirror interaction. Current slot 7 reproduces unwanted translucency when Duke approaches a mirror. Both are explicitly part of the ongoing D35J/M candidate work. Keep reflected Duke opaque at close range while preserving intentional power-up effects and Vanilla. The user reported the previous idle/use fixes solid so far, then accepted the combined result and requested closeout.

**Implementation evidence (2026-10-25, before acceptance):** [Note 235](documentation/235-d35n-mirror-interaction-and-opacity.md). Slot 12 uses pose 278 with normal camera ownership; the eligibility fallback caused the perspective switch. The candidate keeps this action in first person with an independent MIRROR VIEW preference. Slot 7 reflected Duke now bypasses only native proximity fading, retaining intentional effects and Vanilla. Build/native/profile tests and private 60/120 interaction/reflection routes pass; the user subsequently accepted the delivered framing. Broader mirror coverage is not claimed.

**Desired result:** Reproduce the mirror interaction and identify whether it is an animation-owned interaction or native special view. Implement appropriate first-person eye/hands/interaction timing while retaining full reflected Duke and optional legacy presentation.

**Perspective/configuration:** MIRROR VIEW: First-Person / Legacy Third-Person, independent of door preference unless user later chooses consolidation.

**Source references:** recomp/src/ttk/pc_input.cpp:E interaction lifecycle; modern_controls.cpp:normal-camera hook and actor draw 0x800348d8 (primary 0x8003769c, mirror 0x8003816c); first_person.inc:first_person_draw/weapon_draw_bounds; native callback and 0x8003b36c special view are tracing candidates, not proven mirror cause.

**Reproduction and tests:** Locate an interactive mirror with private setup; distinguish passive looking/reflection from E-triggered action; trace object type/callback, lower/upper animations, flags and both camera owners; test repeated/aborted interaction, weapon redraw, first/third global views and reflected head/arms/jet flames.

**Acceptance:** Report exact cause before implementation; complete interaction remains first person with correct timing and readable reflection; primary-view hiding never hides reflected Duke; legacy restores current preference; no duplicated event/quote or ownership leak.

**Regression risks:** Confusing mirrored drawing with perspective ownership, head flags leaking to reflection, scripted transform and animation timing; do not infer cause solely from special-camera routine proximity.

### D35N1 - Missing mirror voice line at slot 6 (bug 3)

**Status:** Done (user-accepted 2026-10-30: "working perfectly"; closeout authorized; with D35N2; [note 329](documentation/329-d35n1-n2-mirror-reach.md)). **Dependencies:** D35N (Done).

**Cause and fix:** the mirror and its line (pose 278, random line 50-52) are
native and intact. The mirror stands behind a counter, so the 512-unit E use
finder ray never reaches it (732 units) and E does nothing. In Modernized the
finder's first ray is aimed through a mirror's centre (interaction class 11/15
with the use flag) up to 896 units within a 35-degree view cone, in Duke's
room only. The original callback, line-up, pose, line choice and repeat rules
are unchanged; Vanilla is unchanged.

**Report:** Duke does not say the mirror voice line at current slot 6. The
user identifies this as an original-game level-design oversight, rather than
a recomp regression; that attribution has not been independently verified.

**Acceptance:** Identify the intended mirror interaction and missing quote
trigger, then restore the appropriate line in Modernized with correct timing
and repeat behavior. Preserve mirror perspective/reflection behavior and
other quotes. Keep the original Vanilla behavior.

### D35N2 - Missing mirror voice line at new slot 8

**Status:** Done (user-accepted 2026-10-30; closeout authorized; reported 2026-10-29; fixed with D35N1, [note 329](documentation/329-d35n1-n2-mirror-reach.md)). **Dependencies:** D35N (Done); do with D35N1.

**Cause and fix:** as D35N1. The user's recollection was right: the dresser
keeps Duke 541 units from the type-750 mirror, beyond the 512-unit ray. Fixed by
the same mirror-only reach.

**Report:** In the Old West (user's new UI slot 8, 22:33), pressing E at a
mirror does not make Duke say his usual mirror line. The user recalls a
similar case where a table or other object was in the way of the mirror.
Private copy: `recomp/analysis/d17am3/playtest-fixture`.

**Acceptance:** As D35N1: find the interaction and quote trigger, restore the
line in Modernized with correct timing and repeats, keep Vanilla.

### D36 - First-person body, arms and interaction presentation quality audit

**Status: Done (research accepted 2026-10-26).** User explicitly requested
"lock this research job in, and close it out". D36A-F and the roadmap in
[note 257](documentation/257-d36-first-person-presentation-audit.md) are accepted
as the reference for subsequent first-person presentation work. Source findings,
private observations, hypotheses and coverage gaps retain their evidence labels.
Acceptance does not establish every reported cause or authorize implementation. D35 and its accepted children remain
Done; this is presentation refinement, not a reversal of their accepted behavior.

**Mission:** establish a cohesive, visually convincing, professional first-person
experience across the entire game while retaining the original PlayStation style.
Preserve successful first-person gameplay and interaction work. Research existing
systems, identify actual causes, evaluate the smallest coherent architecture, and
present independently implementable follow-ups for review before fixing anything.
The player should not need to excuse floating hands, detached limbs, popping or
inappropriate poses as implementation limitations.

**Dependencies:** accepted D11/D12 and D35 presentation work, D08K animation work,
and D08A26 custom first-person equipment. These are reference baselines to inspect,
not jobs to reopen. D37 is separate; D36 must cross-reference its eventual findings
without assuming a common cause or waiting to begin independent visual research.

**Research task breakdown:** D36A-F below are accepted research deliverables,
not authorized implementation jobs. D36A establishes the inventory; D36B-E investigate domains;
D36F consolidates evidence and proposes the implementation roadmap. Future coding,
asset and animation jobs must follow the evidence, receive stable IDs and explicit
selection, and must not be silently started as part of this audit.

### D36A - Existing first-person systems and state catalogue

**Status: Done (research accepted 2026-10-26). Catalogue in [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on accepted D11, D12, D35, D08K and D08A26 work.**

Read relevant repository instructions, source and prior engineering notes before
proposing changes. Catalogue camera/viewmodel rendering; weapons, hands and arms;
body visibility and suppression; legs and locomotion; quick kick; ladders, chains,
poles, walls, overhead traversal and mantles; swimming/underwater movement and
jetpacking; push/pull; keycard, crystal, vending-machine, wheel and other object
interactions; scripted activities, death/respawn and other transitions. Include
first-/third-person user preferences and accepted fallback mechanisms.

Trace model hierarchy, transforms and attachments, animation selection, geometry
visibility, culling, clipping, projection/depth and lighting. Search the whole
codebase for every first-person override/workaround and state transition, including
activities not listed in the user's examples. Establish which behavior is accepted,
intentional, incomplete or suspect; do not duplicate existing systems or undo prior
work. Identify shared ownership and pose/visibility restoration paths.

**Acceptance:** a source- and documentation-linked state catalogue covers all
identified activities, entry/active/exit paths and overrides, with function names,
verified animation identifiers and relevant model parts where available. Explicitly
list coverage gaps instead of claiming comprehensive observation without evidence.

### D36B - Weapon arms and hands: complete geometry audit

**Status: Done (research accepted 2026-10-26), [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on D36A.**

Reproduction reference: load the new Time to Kill level and use the existing
`dnstuff` cheat. Verify the actual level and weapon identities during research;
"new Time to Kill level" is the user's description, not a confirmed level ID.
Inspect every weapon, alternate weapon sharing an inventory slot, and upgraded
variant. Do not limit coverage to these reported examples:

| Inventory slot | User observation | Research question |
| --- | --- | --- |
| 1 | Throwing axe and knife show hands/weapons but no forearms/arms | Missing source geometry, suppression, transforms, clipping or another cause? |
| 4 | Gatling gun looks good but its hand lacks a connected arm | How should hand, arm and weapon join in the existing pose? |
| 6 | Grenade-type weapons show hands but no arms | Can shared geometry/attachments serve all variants? |

Distinguish original model composition from our first-person suppression,
attachment/animation transforms, near-plane clipping and renderer limitations.
Evaluate a unified arm solution against weapon-specific geometry, attachments and
poses. Arms must connect plausibly wherever naturally visible, without adding
geometry that obscures the weapon or creates an unnatural pose.

**Acceptance:** a complete weapon/variant matrix records observed visibility,
source evidence, confirmed causes or labelled hypotheses, viable solutions and
trade-offs. Cover idle, fire/throw, switching, crouch, look extremes and relevant
movement/interruption states. Preserve accepted weapon framing and mechanics.

### D36C - Body/legs and quick-kick presentation audit

**Status: Done (research accepted 2026-10-26), [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on D36A.**

Investigate normal walk/run leg visibility while looking down: limbs reportedly
appear/disappear unpredictably. Trace body-part rules, camera-relative clipping,
near-plane intersections, transforms, culling, animation frames and locomotion
transitions. Slot 12 ladder arms generally work; disappearing torso when looking
straight down may be an intentional anti-obstruction choice, not a required fix.
Evaluate retaining that behavior, body placement, mesh adjustments, selective
visibility and restricted downward look. Camera restrictions are permissible only
with justification and comparison to alternatives, not the default remedy.

Preserve the successful Q quick kick and investigate three distinct observations:

- Depth/overlap: with long weapons, especially Gatling, the foot appears closer
  than the barrel. Inspect coordinate systems, projection, depth and animation
  placement. Establish convincing relationships across weapon sizes rather than
  indiscriminately pushing the foot behind every weapon.
- Third leg: normal legs remain visible looking down while an additional kick leg
  animates. Compare replacing/suppressing the corresponding normal leg with other
  rendering approaches; restore normal visibility reliably afterward.
- Mirrored equipment: the kick appears to mirror Duke's right leg, exposing an
  incorrect holster arrangement. Verify the actual mesh/orientation; compare the
  correct original leg, attachment/animation correction, mesh modification and a
  dedicated first-person jeans-and-boot mesh without the holster.

**Acceptance:** each observation has evidence and a recommended approach with
alternatives. Test planning covers short/long weapons and variants, costumes,
standing/crouching, walk/run, look extremes, kick start/impact/end/interruption and
save/load restoration. Quick-kick timing, damage, hit detection and responsiveness
must remain unchanged. No cause is considered confirmed merely from appearance.

### D36D - Interaction arms, poses and attachment audit

**Status: Done (research accepted 2026-10-26), [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on D36A. Slot 8 push/pull is a high-priority example.**

Current user save-slot references (2026-10-26) are reproduction pointers, not proof
of cause; verify actual loaded scene/state and use private copies for research.

| Save slot | Interaction | Reported observation |
| --- | --- | --- |
| 11 | Keycard insertion | Floating hand; partial arm appears/disappears |
| 10 | Vending machine | Arm appears abruptly cut off |
| 9 | Crystal insertion | Only hand holding crystal is visible |
| 8 | Dumpster push/pull | Detached arms move erratically |
| 5 | Wheel turn | Hands lack properly connected arms |
| 12 | Ladder | Arms generally work; downward body visibility needs review |

For every interaction inspect animation, hierarchy, attachments, visibility and
camera behavior. Determine whether existing third-person animation can be adapted
reliably or a dedicated first-person pose/animation is preferable. Expand coverage
to every relevant activity found by D36A: chains/poles, walls, overhead traversal,
mantles, water climb-outs, swimming, flight, scripted uses and object manipulation.
Audit limb/object/camera clipping, inappropriate poses, unexpected geometry,
visibility changes, state-transition instability and pose restoration after use.

Slot 8 requires particular attention to original turning/repositioning/body motion
and whether those transforms explain the reported erratic arms. Evaluate a stable,
plausible first-person pushing/pulling pose or animation. This is a proposed avenue
to investigate, not a confirmed cause or selected implementation. Any future visual
fix must preserve movement, collision and interaction mechanics.

**Acceptance:** a per-interaction evidence matrix covers entry, sustained activity,
exit, interruption, weapon switching, look extremes and saving/loading where valid.
Record intentional suppression separately from defects. Preserve working animations,
perspective preferences and accepted fallbacks; do not hide arms or force third
person merely to avoid doing the presentation work.

### D36E - Geometry, animation and shared-presentation architecture evaluation

**Status: Done (research accepted 2026-10-26), [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on D36A; informed by D36B-D.**

Evaluate the demonstrated Blender custom low-poly asset pipeline, Photon Studio
asset creation/processing workflow, and runtime mesh manipulation. Review D08A26's
first-person gas-mask geometry and environmental lighting as existing capability;
verify reusable integration points rather than assuming assets alone solve poses.
Original Duke geometry is an option, not an exclusive constraint.

Compare reuse of original meshes; dedicated arms; interaction-specific poses;
retargeted/reused animations; attachment corrections; selective mesh visibility;
camera/clipping corrections; small original replacement assets; and shared arm/hand
conventions. A hybrid of weapon-specific combat geometry and interaction poses may
be best, but must follow the evidence. Evaluate a shared body-presentation layer
for consistent ownership, transforms, lighting, visibility and restoration, versus
extending existing systems. Prefer the smallest coherent architecture that meets
the quality target; do not default to rewriting the renderer or animation system.

**Acceptance:** alternatives and trade-offs are grounded in D36A-D evidence, with
reuse opportunities, asset requirements, performance implications, integration
boundaries and serialization risks explicitly identified. Separate architectural
foundation work from asset creation, animation refinement and isolated corrections.

### D36F - Visual standards, traceability and implementation roadmap

**Status: Done (research accepted 2026-10-26), [note 257](documentation/257-d36-first-person-presentation-audit.md). Depends on D36B, D36C, D36D and D36E.**

Use Duke Nukem: Zero Hour Overclocked's first-person presentation as a comparative
quality reference where accessible; cite what was actually reviewed and disclose
unavailable references. Do not assume its implementation or architecture transfers.

Define observable quality standards: anatomically plausible connected limbs,
consistent intentional visibility, stable weapons, convincing interactions, correct
orientation, sensible camera-relative placement, no floating/detached parts or
unexplained extra limbs, no distracting clipping/popping, consistent environmental
lighting and PS1 style, smooth state transitions and responsive gameplay.

Deliver a comprehensive technical audit in the existing numbered documentation
structure. Allocate the next unused document number at research time; intake does
not reserve a number or create an empty report masquerading as findings. Include
state and weapon catalogues, defect root-cause analysis, geometry/animation option
evaluation, source/function/model/animation references and prior documentation.

Build a traceability matrix linking each observed defect to evidence/source,
confirmed cause or hypothesis, proposed solution, future sub-job and test case.
For every proposed implementation job provide:

1. Problem and its visible/technical impact.
2. Evidence: source, model, animation state and save-slot reproduction.
3. Confirmed root cause or clearly labelled hypothesis/remaining investigation.
4. Recommended technical solution.
5. Viable alternatives and their trade-offs.
6. Dependencies and prerequisites.
7. Risks and possible regressions.
8. Observable acceptance criteria.
9. Specific tests: slots, weapons, interactions and state transitions.

Prioritize by visual severity (including slot 8), architectural dependencies,
complexity and regression risk. Consolidate shared causes where justified rather
than prescribing isolated patches. The roadmap must be reviewable before any
implementation starts. Cross-reference the separate D37 report without conflating
movement failure with presentation defects.

**Regression requirements for all proposed work:** preserve save compatibility,
single-player behavior, weapon mechanics/switching, quick-kick damage/timing,
interaction functionality, first-/third-person settings, camera/input response,
rendering/lighting, accepted performance/refresh behavior and Vanilla gameplay.
Explicitly identify changes that could affect serialized player/animation state.
Recommend practical automated or reproducible visibility, animation-restoration
and transition tests, including enter/leave, look up/down, weapon switching and
save/load in relevant states. State limits where automated tests cannot certify
visual quality; require appropriate captures and gameplay review.

**D36 completion:** the full audit, standards, traceability matrix and dependency-
aware roadmap are delivered for user review, with gaps and uncertainty explicit.
Research must leave the accepted single-player build untouched. No gameplay,
renderer architecture, player model, animation asset or save-format modifications;
no silent third-person fallback replacement and no discarded working interaction
animations. Research acceptance is not implementation or commit/push authorization.

### D36H1 - Gatling left-hand underside contact

**Status: Done (user-accepted 2026-10-26). Depends on D36G/D36H (Done).** Make the left hand visibly hold
the bottom of the Gatling at first-person idle. Research finds existing support
geometry using the native wrist relation, without a dedicated underside anchor;
contact placement still needs live measurement. Preserve accepted weapon framing
and connected arms. Acceptance: convincing underside contact for base/Laser
Gatling at idle and across fire/return/switch, stance/look and replay transitions,
with the note 263 ownership, gameplay and Vanilla regressions intact.

### D36H2 - Buffalo Rifle grip-area flicker

**Status: Done (user-accepted 2026-10-26). Depends on D36G/D36H (Done).**
User accepted the left-hand turning fix and right-wrist running follow-up, then
explicitly authorized lock-in and closeout. The left grip has a small rifle-local
seating correction; the right cuff retains its ready alignment during running.
Accepted Buffalo lift/cock/return, weapon/right-hand anchor and native gameplay
remain preserved. [Note 277](documentation/277-d36h2-buffalo-grip.md) records
source diagnosis, measured ordering stability, tests and bounded visual coverage.

### D36H3 - Realistic two-handed weapon/hand motion while running

**Status: Done (user-accepted 2026-10-26). Dependencies: D36G, D36H, D36H1 (Done).** User reports excessive
right-hand movement with shotgun (3), Gatling (4), Energy Weapon (7), Flamethrower
(8) and Freezethrower (0). Make the running motion more restrained and plausible,
with both hands coherently attached. First compare idle/walk/run/strafe and
start/stop, using actual equipment records and upper-channel states. Follow the
D36 audit and preserve accepted framing, D36H1 contact, camera bob, recoil,
reload/fire timing, muzzle/aim, lighting, save/replay and Vanilla. Acceptance:
user-approved natural motion for all five families and upgrades, no hand/weapon
separation or start/stop pops at 60/120, crouch/swim/flight and switching intact.
Display-only running-arm correction implemented; build, native suites and private
60/120 checks pass. Final revision accepted; lock-in and closeout authorized. [Note 271](documentation/271-d36h3-running-arm-motion.md).

### D36H4 - Connected unarmed locomotion arms

**Status: Done (user-accepted 2026-10-26). Dependencies: D36G/H/I/J and D36H3 (Done).**
Selected 2026-10-26: stand up, research, then immediately implement. User reports
floating hands with slot 1 (Boot/unarmed) while running and looking slightly down.
Research the actual equipment state, semantic arm transforms and pre-polygon
culling before choosing the smallest presentation correction. Reuse accepted
limb ownership and native arm geometry. Acceptance: connected visible hands and
arms through idle/walk/run/strafe/start-stop and pitch sweeps; preserve weapon
switching, Q, native motion/events, world occlusion, third person, Vanilla and
60/120 replay/load restoration. User accepted the lowered revision and authorized
implementation, tests and documentation commit/push. Research and bounded
verification evidence: [note 272](documentation/272-d36h4-unarmed-running-arms.md).


**Latest closeout (2026-10-26): D36H3 is Done. User accepted the final running-arm and support-grip presentation and explicitly authorized lock-in/closeout. [Note 271](documentation/271-d36h3-running-arm-motion.md). No next job started.**



**Latest closeout (2026-10-26): D08A30 is Done. User accepted reversed weapon-wheel direction and explicitly authorized lock-in/closeout. Wheel up selects next; wheel down selects previous. No next job started.**

**Latest closeout (2026-10-26): D08A29 is Done. User accepted running weapon selection and explicitly authorized lock-in/closeout. D08A30 (reverse weapon-wheel direction: up = next, down = previous) is Todo only. [Note 270](documentation/270-d08a29-running-weapon-selection.md). No next implementation started.**

**Latest closeout (2026-10-26): D36H1 is Done. User accepted the Gatling underside grip and explicitly authorized lock-in/closeout. D08A29 (wheel weapon changes while running), D36H3 (restrained two-handed running motion) and D35J5 (first-person frozen blue vision) are Todo, with bounded source investigation only. No next implementation started. [Note 268](documentation/268-d36h1-gatling-grip.md), [note 269](documentation/269-running-weapons-and-frozen-view-intake.md).**

**Latest closeout (2026-10-26): D17Z and D17AA are Done. User accepted both fixes and explicitly authorized lock-in and closeout. D19Q3 is also Done by the user's completion correction. Accepted executable `9342a18490c3b9304371f3f2d2d3cabce077f000a13465f326b1392ebfa9eb32`. [Note 266](documentation/266-d17z-medieval-wall-textures.md), [note 267](documentation/267-d17aa-wall-block-ordering.md). No next job started.**

**Earlier backlog intake (2026-10-26): D36H1 Gatling underside grip and D36H2 Buffalo grip-area flicker remain Todo. D17Z was subsequently implemented and accepted; see closeout above. [Note 263](documentation/263-weapon-grips-and-medieval-wall-intake.md). Preserve `research/screencaps/medieval-glitch-wall.mp4` and remind the user to compare it when D17Z is fixed.**

**Latest accepted research (2026-10-26): D36 and D36A-F are Done. [Note 257](documentation/257-d36-first-person-presentation-audit.md) is the accepted first-person presentation audit and roadmap. D36G is Done (user-accepted); D36H is Done (user-accepted); D36I is Done (user-accepted); D36J is Done (user-accepted); D36K is Done (user-accepted); D36L remains unstarted.**

**Latest closeout (2026-10-26): D23D is Done. User could not reproduce the Level 9 slowdown and reports a problem-free playthrough. Closed on user playtest evidence; no build change or new profiling.**

**Latest accepted job (2026-10-26): D23C moat slowdown recheck is Done. User approved closeout; original sustained slowdown no longer reproduced, brief water-entry cadence dip retained as an unresolved observation. No player-build change or next job started. [Note 253](documentation/253-d23c-moat-recheck.md).**

**Latest accepted job (2026-10-26): D23 performance baseline is Done. User accepted the findings and authorized closeout. [Note 252](documentation/252-d23-performance-baseline.md) preserves the measurements and limits. Player build unchanged; D23C is ready for selection, not started.**

**Latest accepted job (2026-10-25): D08Q9 jetpack walk/run and steroid pace is Done. User authorized lock-in and closeout. [Note 251](documentation/251-d08q9-flight-pace.md). No next job started.**

**Latest accepted job (2026-10-25): D08Q8 smooth jetpack surface following is Done. User accepted and authorized lock-in/closeout. [Note 250](documentation/250-d08q8-flight-clearance.md). No next job started.**

**Latest accepted job (2026-10-25): D08A24 automatic/instant Bio Mask is Done. User: "lock it in, close it out". Automatic entry/exit, instant use while firing and on/off sound accepted; implementation and documentation closeout authorized. [Note 246](documentation/246-d08a24-auto-bio-mask.md). No next job started.**

**Latest accepted job (2026-10-25): D08A25 night vision on/off sound is Done. User: "its perfect! lock it in and close us out". Implementation and documentation closeout authorized. [Note 245](documentation/245-d08a25-night-vision-sound.md). No next job started.**

**Latest accepted job (2026-10-25): D08A23 instant night vision is Done. User: "lock it in, close it out!!". Implementation and documentation closeout authorized. [Note 244](documentation/244-d08a23-instant-night-vision.md). D08A24 remains Todo; no next job started.**

**Latest accepted job (2026-10-25): D08T9 level-5 dumpster repeat grab is Done. User: "perfect! lock it in and close it out". Implementation and documentation closeout authorized. [Note 243](documentation/243-d08t9-level5-dumpster.md). No next job started.**

**Latest accepted job (2026-10-25): D35G1 slot-5 jump edge artifact is Done. User authorized lock-in and closeout. D08A23 instant night vision and D08A24 automatic Bio Mask remain Todo with initial research. [Note 242](documentation/242-d35g1-jump-edge-artifact.md). No next job started.**

**Latest accepted job (2026-10-25): D35K4 slot-5 wheel turn is Done. User authorized lock-in and closeout. [Note 241](documentation/241-d35k4-wheel-first-person.md). No next job started.**

**Latest closeout (2026-10-25): D35K2 toilet use, D35K bounded research and overarching D35 are Done by user acceptance. Future first-person drops become focused D35 child jobs; no next job started. D35N1 missing mirror voice remains Todo.**

**Latest accepted job (2026-10-25): D35J4 slot-9 crystal take/return perspective is Done. User: "lock it in and close it out!". [Note 236](documentation/236-d35j4-time-crystal-perspective.md). Other intake jobs remain Todo.**

**Backlog intake (2026-10-25): Five user reports queued as Todo only. D35J4 (bug 5, slot-9 time crystal) is first for future work; no job is started. D08Q7, D08T9, D35N1 and D08Q8 record bugs 1-4. Implementation awaits a later instruction.**

**Latest accepted job (2026-10-25): D35I first-person walls and fluid diagonal WASD is Done. User: "fully accepted! lock it in and close it out". [Note 232](documentation/232-d35i-wall-climbing.md).**

**Latest accepted job (2026-10-25): D35F1 chain visibility is Done. User: "totally accept. lock it in and close us out". [Note 231](documentation/231-d35f1-chain-visibility.md).**

**Latest accepted job (2026-10-24): D35G first-person mantles, ledge hangs and water climb-outs is Done. User: "lock it in, close it out!". [Note 229](documentation/229-d35g-first-person-mantles.md).**

**Latest accepted job (2026-10-24): D35F first-person chains/poles is Done. User: "so so good! let's lock it in and close it out!". [Note 228](documentation/228-d35f-first-person-poles.md).**

**Latest accepted job (2026-10-24): D35E first-person ladder lifecycle is Done. User: "lock it in, close it out!!!". [Note 227](documentation/227-d35e-first-person-ladders.md).**

**Latest accepted job (2026-10-24): D35D first-person push/pull/grab is Done. User: "lock it in, and close it out!!". [Note 226](documentation/226-d35d-first-person-push-pull.md).**

**Latest accepted job (2026-10-24): D35C jetpack perspective option is Done. User: "lock it in, close it out". [Note 225](documentation/225-d35c-jetpack-perspective.md).**

**Latest accepted job (2026-10-24): D35B swimming perspective option is Done. User fully accepted the First Person / Third Person choice and explicitly requested closeout. [Note 224](documentation/224-d35b-swimming-perspective.md).**

**Latest accepted job (2026-10-24): D08Z6 blocked-run ledge jumps and first-press response are Done. User accepted revision 2 and explicitly authorized lock-in/closeout. [Note 218](documentation/218-d08z6-blocked-run-jumps.md).**


**Latest accepted job (2026-10-24): D08AF plunge-to-swimming handoff is Done. User accepted and authorized lock-in/closeout. [Note 217](documentation/217-d08af-plunge-swim-handoff.md). D08Z6 is now Done; see its acceptance entry above.**

**Latest accepted job (2026-10-24): D08T8 apartment alley platform pull-up/drop is Done. User confirms it works exactly as intended and authorizes closeout. [Note 214](documentation/214-d08t8-alley-platform-hang.md).**

**Latest accepted job (2026-10-24): D20G reload-halt investigation is Done (not reproduced; cause unresolved). If a similar save/load GPU halt or corruption recurs, revisit D20G and [note 213](documentation/213-d20g-slot11-load-halt.md) before starting from scratch. D08T8 is now accepted; see note 214.**

**Latest accepted job (2026-10-24): D08T5 alley dumpster ladder limit is Done. Slot 7's X 7292 is the maximum push toward the ladder. [Note 210](documentation/210-d08t5-dumpster-limit.md).**

**Latest accepted job (2026-10-23): D08L1 walking and crouched walk-offs is Done. Executable `8de27e4cae38b5053c5b456c3fc60ef96a2d2a46df4630dbb7309a8c01a63097` is the accepted baseline. [Note 209](documentation/209-d08l1-walk-offs.md).**


**Latest accepted job (2026-10-23): D08T3 Free Push is Done. Executable `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118` is the accepted baseline. New Todo: D08L1, walking/crouch-walking off ledges. [Note 208](documentation/208-d08t3-manual-push-pull.md).**

**Latest accepted job (2026-10-23): D26B view/control ownership is Done. Executable `a7ef7350de0b11fcb8235786e8a790faff19ddcc12dc9c186f6b320b4cc2caec` is the regression baseline. [Note 207](documentation/207-d26b-view-ownership.md).**

**Latest accepted job (2026-10-22): D17S AUTO frame rate is Done. Executable `482a3479813ddca0da3a5237034b2b84edf22c77ede8061f339a64c313eba1e9` is the regression baseline. [Note 203](documentation/203-d17s-auto-frame-rate.md).**

**Latest accepted job (2026-10-22): D12D upgraded first-person weapon framing is Done. Executable `bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0` is the regression baseline. [Note 199](documentation/199-d12d-upgraded-viewmodels.md).**

**Latest acceptance (2026-10-22): D08K5 final coverage and D08K crouch-walking parent are Done. No gameplay changes; D17R2 executable remains the regression baseline. [Note 198](documentation/198-d08k5-crouch-coverage.md).**

**Latest accepted job (2026-10-22): D17R2 (angle-dependent black sky); executable `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318` is the regression baseline. [Note 197](documentation/197-d17r2-angle-dependent-black-sky.md).**

This is the canonical job list for our **Duke Nukem: Time to Kill** PC project, targeting the owned US SLUS-00583 disc. The ambition is a polished, game-specific PC edition: faithful original play plus an optional modern experience. This is a plan, not a list of features already available.

Invoke **`$continue-duke-recomp`** (Codex) or **`/continue-duke-recomp`** (Claude Code) to see the current jobs and choose one. You can also request a job directly: **`$continue-duke-recomp work on D01`** or **`/continue-duke-recomp work on D01`**. The skill reads this file rather than keeping a second backlog. It must not automatically start the next job.

**Latest accepted job (2026-10-22): D08K11 (Modern crouch and crouch walking in grounded shallow/mid-depth water); executable `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8` is the regression baseline, [note 193](documentation/193-d08k11-shallow-water-crouch.md).** Earlier: **Latest accepted job (2026-10-22): D08K10 (first-person crouched weapons retain standing framing, including high-refresh crouch walking); executable `72edb8aec75b3786e92359e29b654ef6e18911ae27c7edda0ea46049698d623f` is the regression baseline, [note 192](documentation/192-d08k10-crouched-viewmodel.md).** Earlier: **Latest accepted job (2026-10-22): D08K9 (held crouch takes over on soft touchdown; CROUCH owns the feature, LANDING hidden from Advanced). Executable `a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91` is the regression baseline, [note 191](documentation/191-d08k9-crouch-on-landing.md).** Earlier: **Latest accepted job (2026-10-22): D20E (the statistics screen reads ENEMIES KILLED X OF Y, Y by the level load's own enemy rule; accuracy dropped, damage taken parked); executable `156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef` is the regression baseline, [note 183](documentation/183-d20e-statistics-research.md).** Earlier (2026-10-21): **D20F (the statistics screen's CONTINUE / SAVE GAME are a selectable list under the lines: Up / Down / wheel, Enter chooses, V still saves; the original's wait is about 1 s); executable `3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496` is the regression baseline, [note 180](documentation/180-d20f-statistics-list.md).** Earlier (2026-10-21): **D20B (the one-player statistics screen is ours in the original look; SAVE opens our SAVE GAME, nothing in Modernized writes a game save to the memory card; LOAD GAME shows LEVEL COMPLETE saves with the level's last picture; console `endlevel` for testing; menu host image follows the presented 4:3 / 16:9 rect); executable `f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866` is the regression baseline, [note 179](documentation/179-d20b-statistics-screen.md). New Todo: D20F (selectable CONTINUE / SAVE GAME), D20D (richer statistics), D20E (statistics research).** Earlier (2026-10-21): **D17R (the sky now runs to both edges in widescreen: the shared sky routine's colour fill, camera-facing backdrop and clip rectangle are widened by the widescreen margin; 4:3 and Vanilla unchanged); executable `fbab9bc39f40bc820e4acf06dc161b6ada7ae730814ba174cb35f1807606552d` is the regression baseline, [note 172](documentation/172-d17r-sky-edges.md).** Earlier (2026-10-20): **D19Q1 (SAVE GAME's picture read from stale CPU VRAM, the image at the previous save; the grab now syncs the GPU renderer first); executable `92178369fc5153178009edf801bd5c07299fee5d6d9bab927ea6ca2b6fc3e043` is the regression baseline.** Earlier (2026-10-20): **D19Q (SAVE GAME on the pause list, LOAD GAME's page with every slot open, OVERWRITE THIS SAVE? with NO first, SAVED / COULD NOT BE SAVED, gameplay thumbnail, LOAD GAME resumes play; atomic slot writes; menu identity guard no longer covers card-restored settings); executable `15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b` is the regression baseline, [note 171](documentation/171-d19q-save-game.md).** Earlier (2026-10-20): **D19R (startup memory card wait runs at host speed: the black gap after the movies went from about 2 s to about 0.35 s, the same guest fields, settings still loaded, Vanilla unchanged; closeout delegated by the user); executable `2fb1bd3cc0e8ee78fd94c41da6c66c39a5fe7d5ff4eddd813e321bc2f2833408`, [note 170](documentation/170-d19r-card-wait.md).** Earlier (2026-10-20): **D19K (LOAD GAME on the title, pause and death menus lists our savestates in the menu style, thumbnail beside, Console steel save time, LOAD THIS GAME?, opens on slot 01; startup memory card check runs unseen in Modernized; new Todo D19Q SAVE GAME, D19R startup card hang); executable `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9` is the regression baseline, [note 169](documentation/169-d19k-load-game.md).** Earlier (2026-10-20): **D19G1 (in-game FRAME RATE applies at once from any launch rate: a 60 launch starts replay presentation mid-session, 60 stops it and returns to the original presentation; software renderer says NEXT LAUNCH); executable `bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29` is the regression baseline, [note 164](documentation/164-d19g1-live-frame-rate.md).** Earlier (2026-10-20): D10D (keyboard turning: Left / Right arrows turn the view and Duke everywhere in Modernized play at Duke 3D rates, smooth at high refresh; arrows no longer reach the original D-pad in gameplay; KEY BINDINGS CAMERA rows, schema 35); executable `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892` is the regression baseline, [note 162](documentation/162-d10d-keyboard-turning.md).** Earlier (2026-10-20): **D10C (Duke 3D look keys: Page Up / Page Down look while held and ease back, Home / End lock the look, a tap recenters; first and third person, smooth at high refresh, KEY BINDINGS CAMERA rows, schema 34); executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19` is the regression baseline, [note 161](documentation/161-d10c-look-keys.md). New Todo D10D (arrow keys turn left / right).** Earlier (2026-10-20): **D07F (airborne shots follow the crosshair: jumps and falls use the view aim on the camera-only lease; RPG, shotgun, pistol, manual and assisted jump, first and third person); executable `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef` is the regression baseline, [note 160](documentation/160-d07f-airborne-shots.md).** Earlier (2026-10-20): **D19G (in-game KEY BINDINGS: every Modernized action, bound at once and saved, swap on conflict, DEFAULT asks; mouse, display, widescreen, camera, view, shoulder, distance and weapon aim live on CONFIRM, frame rate too unless launched at 60); executable `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da` is the regression baseline, [note 159](documentation/159-d19g-options-stage-2.md). New Todo: D07F (airborne shots ignore the crosshair), D10C (Duke 3D look keys), D19G1 (frame rate live after a 60 launch).** Earlier (2026-10-20): **D33 (Modernized by default; `run.py --vanilla` plays Vanilla for one launch; `--mode` and the in-game PROFILE row removed); executable `fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2` is the regression baseline, [note 158](documentation/158-d33-default-profile.md).** Earlier (2026-10-20): **D08AB (SWIMMING MODERN / ORIGINAL in OPTIONS > MODERNIZED; ORIGINAL restores the pre-D08AA water speeds, forced shallow run included); executable `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508` is the regression baseline, [note 157](documentation/157-d08ab-swimming-option.md).** Earlier (2026-10-19): **D08AA (one walk/run pace on land, in shallow water, on the surface and underwater; Shift decides); executable `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366` is the regression baseline, [note 156](documentation/156-d08aa-unified-pace.md). New Todo D08AB (SWIMMING MODERN / ORIGINAL menu option).** Earlier (2026-10-19): **D07E (modern shots and movement keep their lease for one game frame, so slow scenes keep crosshair aim; `original` weapon aiming faces the camera); executable `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5` is the regression baseline, [note 154](documentation/154-d07e-frame-lease.md).** Earlier (2026-10-19): **D19P (death screen QUIT TO MENU beside QUIT GAME, via the game over screen; identity guard no longer covers the selected item `0x800c3f94`); executable `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63` is the regression baseline, [note 153](documentation/153-d19p-quit-to-menu.md).** Earlier (2026-10-19): **D22D (energy weapon locks the enemy under the crosshair and damages it; base and upgraded, first and third person); executable `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98` is the regression baseline, [note 152](documentation/152-d22d-energy-beam.md). New Todo: D07E (stale-lease shots use the original auto-aim), D19P (death screen QUIT TO MENU).** Earlier (2026-10-19): **D17V (modern view bob: Duke 3D-style steady tempo, deeper not faster when running, no sway, even forward glide; accepted on strong too); executable `a1fea19a0a437d9637211d993747a55a8767f8238d1d7773e1ceed6fe9a06193` is the regression baseline, [note 151](documentation/151-d17v-view-bob.md).** Earlier (2026-10-19): **D19J (title main menu, difficulty, two player and character pages re-implemented in the original style, QUIT GAME on the title; saves wait for plugin guest calls); executable `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d` is the regression baseline, [note 150](documentation/150-d19j-title-menu.md). New Todo D19O (frontend redraw flash-back).** Earlier (2026-10-19): **D19M (QUIT GAME on the pause and death lists closes the program, QUIT GAME? YES / NO); executable `de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43` is the regression baseline, [note 149](documentation/149-d19m-quit-game.md). Title main menu item in D19J.** Earlier (2026-10-19): **D19I (in-game menu text at full brightness: drawn at the pause dispatch, after the paused world's darkening); executable `f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c` is the regression baseline, [note 148](documentation/148-d19i-menu-brightness.md). New Todo: D19M (QUIT GAME closes the program), D19N (one OPTIONS screen, the rest under ADVANCED).** Earlier (2026-10-19): **D19H (Escape opens a list-style pause menu, death screen too, in the original style); executable `681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2` is the regression baseline, [note 147](documentation/147-d19h-pause-menu.md). New Todo: D19I (dimmed in-game menu text), D19J (main menu re-implemented), D19K (integrated savestate LOAD GAME), D19L (menus during demo loads).** Earlier (2026-10-18): **D08O2B (swim-fire aims at the crosshair on every Duke model; shoulders found by part); executable `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25` is the regression baseline, [note 146](documentation/146-d08o2b-swim-fire-costumes.md). New Todo D08O4 (brief Ctrl dive drifts forward). Queued: D08O4, D19G, D19F, D08AA, D24D, D26G.** Earlier (2026-10-18): **D08G5 (partial cheat entry keeps mouse look; a breaking key acts at once); executable `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79` is the regression baseline. Queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.** Earlier (2026-10-18): **D08A22 (switcher strips and mission card scale with the game picture like the native HUD); executable `67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546` is the regression baseline. Queued: D19G, D19F, D08O2B, D08AA, D24D, D26G.** Earlier (2026-10-18): **D19 (in-game OPTIONS: PC CONTROLS, DISPLAY, MODERNIZED in the original menu style, title and pause); executable `fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b` is the regression baseline, [note 145](documentation/145-d19-game-menu.md). Queued: D19G, D19F, D08O2B, D08AA, D08A22, D24D, D26G.** Earlier (2026-10-18): **D19D (menu keys: Escape/Backspace back, Enter confirms, V Square, wheel moves; Escape never quits); executable `08bba860a9171fe2d147e211e569a8de90e434d262600749c476f4caf6b7e9fe` is the regression baseline, [note 144](documentation/144-d19d-menu-keys.md). Queued: D08O2B, D08AA, D08A22, D24D, D26G.** Earlier (2026-10-18): D30 (recomp.board listing: `.recomp.json` claims our row; README badge). Earlier: **Latest accepted job (2026-10-18): D19E (menu close-out: taps never lost during loads or on LOAD GAME, menu over the spinner one image sooner); executable `0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf` is the regression baseline. Queued: D08O2B, D08AA, D08A22, D24D, D26G, D19D.** Earlier (2026-10-18): D19C (more responsive original menus over the demo); executable `1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828` is the regression baseline. Queued: D19E (loading-spinner menus and full menu sweep), D08O2B, D08AA, D08A22, D24D, D26G, D19D.** Earlier (2026-10-18): D17U (smooth loading screens); executable `e1d32dc00d9b2e870f5c1aab4b3e4a21bb74b242c20c7483d4f6eeed7011f9fc`. Earlier (2026-10-18): D17T (smooth title menu and attract demo); executable `7ab8877528b05a46a1e793556660ab6cf09c538d93c7b2f1e6cb5bb744ce9829`. Earlier (2026-10-09): D08G6 (`dntlod` fun cheat); executable `74cec66e398217530d0b8abc3fa0ccfe06308f6b73a17e5aafa628b3f0f2a540`. Earlier (2026-10-09): D08A20 (challenge item HUD, option A - 14x14 icon in a square); executable `a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95`.** Earlier (2026-10-09): D08A21 (14x14 HUD item icons; the 16x16 versions kept in `recomp/assets/ui/items/16px/`); executable `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21`.** Earlier (2026-10-08): D08A12 (modern dynamite: unlit until fire, switch away freely, next stick drawn after a throw); executable `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d` is the regression baseline. Queued: D24D, D26G.** Earlier (2026-10-08): D08A19 (used mission items: USED and the green tick); executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8`. Earlier (2026-10-08): D08Z2 (mantle pose; frozen legs after a corner bump); executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression baseline. D08Z3 (modern landing) accepted 2026-10-08; executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e` is the new regression baseline. Queued: D24D, D26G, D08A19.** Earlier (2026-10-08): D08Z1 (jump wall slide); executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. Earlier (2026-10-08): D08A17 (power-up countdowns); executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`. Earlier accepted jobs (2026-10-08): D08A14 (death ends steroids), D08A15 (heartbeat restart; a pickup mid-run stops steroids) and D08G4 (inventory cheats give full armor); executable `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.** Earlier: D08J2 (pole/chain A/D direction), D08U2 (climb off a ladder past an enemy), D08O3 (E in water keeps the weapon) and D08O2A (natural swim-fire pose v2), 2026-10-07.** Previous baseline `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e` (D08U2). **D08J2 (pole/chain A/D direction) Accepted 2026-10-07**: executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the current regression baseline ([note 122](documentation/122-d08j2-pole-chain-sidestep.md)). Todo: D08J4 (ceiling monkey-bar drops, UI slot 3), D08J3 (free camera while climbing). Previous baseline `e75b50f156137fa2377e4643407b2efbd16f2e976fa4d697afcbceb228e2e554` (D08O3). **D23E (western-town stutter) Accepted 2026-10-07**; **D08O2A Accepted** ([note 119](documentation/119-d08o2a-natural-swim-fire-pose.md)), **D08O3 Accepted** ([note 120](documentation/120-d08o3-e-in-water.md)); **D23F (fast CPU timing) Accepted 2026-10-05 and now the Modernized default**; executable `e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966` (includes D23E), [note 104](documentation/104-d23e-busy-scene-stutter.md), [note 105](documentation/105-d23f-fast-timing.md). Next suggested: D23G (finish the fast path) and D17S (auto frame-rate default).

**Accepted 2026-10-22: D08AD, full third-person walk/run cycles in deeper
shallow water. Baseline executable `9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
D08AE Done: deep-water entry and shallow-water regression accepted by the user.
D17R2 Done: current slot 8 sky-pass omission fixed and user-accepted; note 197.**

### D36H5 - No-body arms and weapons polish, exposed shoulders first

**Status: Done (accepted contact-arm scope; closeout authorized 2026-10-28). Dependency: D36J2 (Done). Related: D36G/H/H1-H4/I/K, D14B.**

User's highest concern: camera shots that expose Duke's disembodied shoulders or
upper-arm ends. No solution is assumed. Begin with a reproducible shot/motion
inventory across look extremes, FOV, weapon poses and camera displacement; name
the actual part and owner producing each exposed end before changing geometry.

Compare bounded remedies such as pose/framing adjustments, limited geometry
coverage and authored first-person arms only where evidence justifies them.
Do not conceal defects by narrowing FOV, restricting free look, restoring an
unwanted body or globally hiding useful arms. Read accepted notes 257 and 265
before presentation or mesh/material work. Preserve accepted weapon grips,
silhouettes, aim and native interaction contacts.

Original broad acceptance matrix (unrun portions carried to D36H6): no
unexplained shoulder/upper-arm cut ends in the verified shots;
connected readable arm chains, stable support-hand contact and controlled elbows
through idle, walk/run, crouch/jump, kick overlap, draw/holster/switch, fire/reload/
throw and equipment-to-interaction handoffs. Cover all base weapon records,
upgrades, empty hands and held mission items; include default/60/80/90, aspects,
lighting/effects, near-world clipping and 60/120 replay. Retain labelled before/
after stills and continuous motion, rejected experiments and explicit untested
routes. Native gameplay, third person, reflections and Vanilla stay unchanged.
Human visual acceptance is required; screenshots alone do not certify motion.

2026-10-28: user accepted the dedicated traversal contact rig and selected
expansion to all interactions, highlighting current slots 6 and 7. The shared
rig now covers all nine established contact owners, adding Push/Use/Mirror to
Ladder/Pole/Wall/Overhead/Mantle/WaterExit. Native hand contacts and world depth
remain authoritative. Native-hook tests cover the added actions and both
costume maps; private natural routes verify use gestures and traversal.
See [note 305](documentation/305-d36h5-contact-arm-rig.md) for the accepted design
and [note 306](documentation/306-d36h5-interaction-arm-coverage.md) for expanded
coverage and the future-report repair guide. The user accepted the expanded
contact-arm result and explicitly authorized closeout. Broader unverified
weapon/item/effect/campaign coverage transfers to D36H6.

### D36H6 - Remaining first-person arm coverage and reported exceptions

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependency: D36H5 (Done).**

Retain the unrun portions of D36H5's original broad matrix: all weapon records
and upgrades, held items, empty-hand locomotion, swim/flight and lighting/effect
variants. Investigate rare interaction selectors/callbacks and scripted camera
variants only after establishing their actual natural routes and owners.
The accepted contact rig is the baseline, not a reason to rewrite unrelated
presentation domains. A future user report can identify an activity or current
slot; begin with the repair guide in [note 306](documentation/306-d36h5-interaction-arm-coverage.md).

Preserve the isolated overhead 120-Hz save/load halt as an unresolved
observation. If it recurs, retain the failing private save and compare that
exact state before assigning a cause. Twelve restoration cycles and a full
route repeat passed; neither a fixed bug nor an arm regression was established.
Acceptance for any selected slice requires labelled natural-route evidence,
appropriate ownership/native regressions and user visual review. No work starts
merely because this follow-up is registered.

2026-10-28: selected by the user. A fresh 60-Hz unarmed load omitted the accepted
arm lowering until a weapon draw allocated scratch, exposing upper-arm ends.
The accepted repair allocates that existing scratch at the first eligible arm draw.
The new cold-start actual-hook regression fails before and passes after the
repair, across both costume maps. Player build and controls/input/aim/near/save
suites pass. Private base/upgrade, held-item, empty-hand/flight and swim routes
are recorded in [note 307](documentation/307-d36h6-arm-coverage.md), with fresh
60/120 restoration comparisons. User accepts significantly improved quality,
superb weapons, and brief costume/held-item/interaction testing, and explicitly
authorizes lock-in and closeout. D36H6 is Done for this accepted repair and
coverage pass. Unrun combinations and the isolated overhead halt remain recorded
limitations, not claims of exhaustive verification. No next job selected.

### D36H7 - Proportional contact arms with shoulder reach

**Status: Done (user-accepted; closeout authorized 2026-10-28).** Selected 2026-10-28. Reduce excessive interaction and
traversal arm extension by allowing bounded virtual-shoulder travel toward
native hand contacts. Preserve hands, connected elbows, native gameplay,
world depth, hidden shoulder ends, accepted combat arms, legs and camera policy.
Verify both costume mappings, all nine contact owners, look/FOV extremes and
private gameplay/replay samples. User visual acceptance received 2026-10-28.
Evidence: [note 312](documentation/312-d36h7-proportional-contact-arms.md).

### D36H8 - Run-jump mantle across water: extended arms flicker (slot 11)

**Status: Done (user-accepted 2026-11-03; fix: empty arms also admitted over the neck, [note 306](documentation/306-d36h5-interaction-arm-coverage.md) D36H8 section). Dependencies: D36H5, D35G, D08X.** Fixture: the player's UI
slot 11 (re-saved 2026-11-02 in front of a body of water); test on private
copies only. Route: run (Shift + W) to the ledge, Space to jump, then hold E
at once to mantle onto the far ledge. While Duke's arms are extended for the
grab they flicker; the user wants the arm model robust in this situation.
Read the D36 audit ([note 257](documentation/257-d36-first-person-presentation-audit.md))
and the D36H5 design and repair guide
([note 305](documentation/305-d36h5-contact-arm-rig.md),
[note 306](documentation/306-d36h5-interaction-arm-coverage.md)) first:
identify the actual presentation owner of the extended-arm pose (airborne
ledge grab D08X, first-person mantle D35G or the contact rig) and the slot
contents per frame before changing anything, then fix the cause (clipping,
near plane, depth, pose handoff or a frame where the arms are dropped) for
every airborne grab and mantle, not this ledge only.

**Acceptance:** the slot 11 route shows no arm flicker at 120 and 180 Hz,
repeated grabs from several approach speeds; other mantles, ledge hangs and
water climb-outs (D35G, D08X, D08V coverage) unchanged; Vanilla unchanged;
the user's playtest.

### D36H9 - Ledge hang first person: upper arm flicker and legs-only body (slot 9)

**Status: Done (user-accepted 2026-11-04: "thats it, thats a winner. lock it in, close it out"; the flicker fix plus legs moved back to a sliver; [note 306](documentation/306-d36h5-interaction-arm-coverage.md) D36H9 section). Dependencies: D36H8, D35G, D08J7A.** User, reviewing D08J7C
(video `Video_2026-11-04_08-54-07.mp4`): "when mantling now, the arms
flicker. for example, from the same save slot, simply jump and grab the
ledge, and W to climb up, and duke's arms flicker. furthermore you can see
the disembodied legs in this view too." Slot 9, first person, unarmed: jump
catch, hang 149, look down and turn right along the wall; an upper arm pops
in and out at the side of the view, and below the eye only the legs (and the
holstered pistol) are drawn, no torso.

Reproduced 2026-11-04 on private copies at the 180 setting (Xvfb, 60 fps
recording, `recomp/analysis/d08j7/flickrec.py`): the same intrusion on the
D08J7C candidate and on the D08J7B build
(`ca7d1a22bf575a6ee4d7a3c010338e6f6d641e698e416a6b1bfa540fb90aa9ab`), so it
is not a D08J7C regression. Likely the mechanism of note 257 and D36H8: a
whole limb part shown or hidden as its origin depth or the eye envelope test
changes with the view, here the hang's raised arms beside the eye. Identify
the actual owner first (D35G Mantle presentation, D36H4 body path or the
D36H5 contact rig; notes 257, 305, 306), then make the hang arms continuous
(hidden or shown, never popping) and decide with the user what the hang
shows when looking down (legs alone read as disembodied: hide them, or add
the torso). Test the slot 9 copy and the D35G alley hang at 60/120/180,
looking down and sideways; shimmy, pull-up and drop; third person and
Vanilla unchanged.

**2026-11-04 decision.** User, on what the hang shows when looking down: "he
could get a torso in this case too"; after the options (D14E already draws the
pelvis and vest on the ground, not in hangs) the user chose instead to keep the
dangling legs and fade them out only while looking down in a hang: "i kinda
wanna try this. when duke goes transparent when he's in a tight spot, it's
really the same kind of thing i think". Plan: in first-person ledge hangs
(grab, hang, shimmy) the legs and pelvis use the game's own close-camera fade
(`first_person.inc` notes it: render context `0x800d67a8` +0x48 semi-transparent,
+0x4c blend, colors and GTE back color darkened), scaled by downward pitch so
they are solid looking ahead or sideways and fade smoothly to nothing looking
straight down; back to solid as the pull-up brings the knee onto the lip. The
upper arms go through the near-plane polygon clip path instead of whole-part
rejection, so they never pop. Thresholds tuned in game at 60/120/180 and 60-90
FOV; Polymorph and Classic checked; third person and Vanilla unchanged.

**2026-11-04 implementation (Needs playtest).** Owner: the D36H5 contact rig
(Mantle). Three causes, each fixed game-wide for the Mantle owner: (1) the hang
draw calls the list iterator 0x8001ca4c inside Duke's draw (ra 0x80075810) and
the hook took it for the object loop's step, dropping first-person ownership
mid-draw; a call below the actor-entry stack frame now keeps it. (2) The rig's
shoulders were anchored in camera yaw, so turning the view in a hang swept an
upper arm across the eye as a near slab; in Mantle they follow Duke's heading
with the camera pitch. (3) The eye itself alternated 84 units per tick: the
native solver's ray from Duke's pivot to the point ahead of the eye hit the lip
at alternating positions. The fully blended Mantle eye now takes the requested
delta through the existing 0x8002a038 easing hook (no new hook, no codegen
change, savestates unaffected), and ledge hangs place the eye 128 units behind
the root along Duke's heading, keeping the accepted framing (both hands on the
lip). Legs: the third-person body's screen-door coverage, solid to 45 degrees
down, gone at the 60-degree look limit, in hangs and pull-ups; solid standing.
Rejected: the game's own B+F/4 close-camera blend (a binary hide at this pitch
range) and the solver bypass without a setback (hands left the view).
Evidence: slot 9 slow pans at 60/180 without the per-tick corner flicker
(per-frame skin-pixel counts on Xvfb recordings); D35G alley hang 60/120/180
look-down, pan, shimmy, pull-up; third person and `--vanilla` hang unchanged;
D36H8 slot-11 mantle 120/180 continuous and identical to an A/B run with the new
switches off. Native controls 147 groups (new D36H9 group), input, aim and near
suites, Python profile and input suites pass. Limits: pitch-based fade; legs
return solid when Duke stands, not exactly at the knee; the setback is a
measured constant; only Polymorph checked (the plan's Classic and 60-90 FOV
checks were not run). Developer A/B switches: `DNTTK_FP_HANG_EYE=native`,
`DNTTK_FP_CONTACT_ARMS=camera`, `DNTTK_FP_HANG_LEGS=solid`. Executable
`a87d304b36c72636bab815b3367ecc7ec3e974701ce7d22d63f17a9034a78ec4`.

**2026-11-04 revision 1 (Needs playtest).** User: "the leg view just isnt it.
can you revert it back to the hanging legs while i make a decision. and the
arms seem to have changed too because now i can really badly see Duke's
disembodied shoulders". Both reverted to the earlier presentation: the legs
are solid dangling legs again (fade kept only as developer A/B
`DNTTK_FP_HANG_LEGS=fade`), and the contact-rig shoulders are camera-anchored
again (body-heading frame only as `DNTTK_FP_CONTACT_ARMS=body`). Kept: the
draw-ownership fix and the steady hang eye. Slot 9 slow pans at 60/180 still
show no per-tick arm flicker; native controls 147 groups pass. The leg
treatment waits for the user's decision. Executable
`0434285d96aba58fdb4980f75fd122a3fdb76f5206127a32120f0bb22c5b4ed6`.

**2026-11-04 revision 2 trial (Needs playtest).** User: "when hanging, all i
want you to do is try: moving the legs back a few inches on my screen. lets say
4 inches back as i perceive it." In a ledge hang the dangling leg rig moves a
further 140 units back along the view heading (the D36J5 leg setback, plus
140): at FOV 90, the legs' depth and a 27-inch 16:9 screen, about 4 inches
toward the bottom of the screen. Looking straight down (the 60-degree limit)
they now sit just below the bottom edge; before, their top was about 29% up
the screen. Developer tuning only: `DNTTK_FP_HANG_LEGS_BACK=<units>`. Pull-up,
standing legs, arms and eye unchanged. Executable
`074ace53ff13ae7826f85864ace7ae2af884c2254a248c187ef365d6dd08ed7e`.
Iterated the same day: "bring em forward an inch", then "basically a sliver
of leg is all it needs to show". Measured leg top above the bottom edge at the
60-degree look: 0 units 25.5%, 30 15.3%, 60 9.7%, 80 4.5%, 88 2.3%, 92 1.0%,
105 and 140 none. Now 88 units (a sliver). Executable
`89fe6df1b40ee15028a96d25041714ba39a988eba9c7c01de6d1d3d2e47b2ebe`.

**2026-11-04 accepted and closed.** User: "thats it, thats a winner. lock it
in, close it out", adding "the job was way simpler than we thought". Shipped:
the nested-draw ownership fix, the steady Mantle eye (requested delta through
the easing hook, 128-unit hang setback) and the hang legs 88 units back (a
sliver at the lowest look). The leg fade and body-heading shoulders were
rejected and stay off (developer A/B only).

### D36H10 - Water exit first person: the arms vibrate climbing out of the water (UI slot 12)

**Status: Done (user-accepted 2026-11-05: "fixed! lock it in and close it out"; fix: the pull-up's four animation channels in step, [note 306](documentation/306-d36h5-interaction-arm-coverage.md) D36H10 section); user (2026-11-05): the very next job (after the D10H4 /
D10H6 closeout). Dependencies: D36H9, D35D1.** User: "my slot 12, when
climbing out of the water, duke's arms vibrate around ... i feel like this
may be a small side effect? i am not sure". Quick look on a private copy of
UI slot 12 (runtime 11; `recomp/analysis/d10h4/slot12.py`, W+E): Duke stows
the weapon (upper 6, 0, 122), then the water exit runs (mode 8, anim 140,
first-person action water-exit then mantle, about 1 s) and he walks off. No
fire is held and the WEAPON FEEL code does not act (0 boosted updates), so
it is not a D10H side effect. Unconfirmed hypothesis: the D35D1 revision 2
class, arms anchored to the last 30 Hz tick while 120/180 Hz redraws
interpolate the eye. Start from notes 305 / 306 (AGENTS.md first-person arm
rule): identify the presentation owner of the water-exit and mantle arms,
trace hand screen positions across redraws at 180 Hz, fix game-wide, keep
D36H9 and D35D1 unchanged. Preserve the user's slot; test on copies.

**2026-11-05 implementation (Needs playtest).** Owner as drawn: the contact
rig (WaterExit, then Mantle), but the rig was not at fault: its native hand
targets rocked. Duke's pose comes from four animation channels (legs, right
arm, left arm, chest/head). D08J7B scaled the runner time of channels 0 and 1
only (1.6x pull-up); channels 2 and 3 ran at the original rate and were then
synced to channel 1 by frame offset (0x8005a11c), keeping a stale previous
frame each time channel 1 crossed a frame first. Per tick the body-frame hand
went 199, 118, 226, 224, 22, 112 units forward (native speed: one smooth heave
per frame). `pullup_advance` now scales channels 2 and 3 too (ra 0x8005a75c /
0x8005a7bc, sp+0x14 / +0x16, a3 2 / 3), once per update, only while that
channel plays 140 (armed, the gun arm's own clip stays unscaled). Fixes every
pull-up, ledge or water, first and third person. Evidence: private slot-12
copy at 180 Hz (present trace, channel dumps, Xvfb recordings fix vs
`DNTTK_PULLUP_SYNC=off`), D35G alley ledge channel dumps; end position, yaw
and clip length identical; native controls (152 groups, D08J7B group now
covers all four channels and Vanilla), input, aim and animation-save checks
pass. Not done: 60/120 recordings (the defect is per game update, independent
of the presentation rate), slot 9 hang and armed one-handed pull-up recordings,
a readable third-person view (the slot-12 spot is dark under water). Needs
the user's playtest. Scripts and evidence: `recomp/analysis/d36h10/` (local).

### D36H11 - Freezer first person: the left (support) hand does not touch the weapon

**Status: Done (user-accepted 2026-11-07: "perfect, its working! lock it in and close it out"; fix: Freezer support grip held on ready clip 29 key 0 and seated 30 up / 40 toward the eye, implementation below). Dependencies: D36H, D36H1.** User,
2026-11-07: "the left arm on the freezethrower doesnt connect to the weapon".
Read the D36 audit (note 257) and the D36H5 notes (305, 306) first, per
AGENTS.md.

Initial research (2026-11-07, no code changed):
- Owner: in first person the left arm is a `DisplayArm` support grip
  (`recomp/src/ttk/first_person.inc`, `support=slot==3 || (slot>=5 && slot<=11)`),
  posed from the Freezer's own two-handed pose with its display scale (1.4,
  note 257 "seeded grip") about the unchanged right-hand anchor. Running uses
  the Gatling's reference clip (29) and the full walking grip (slot 7 / 11
  branch). Unlike the Gatling (D36H1 palm shift onto the underside) and the
  Energy Weapon (housing clearance), the Freezer has no per-weapon contact
  correction.
- Seen in a private offscreen capture (UI slot 12 copy, LEVEL0, 16:9, FOV
  90, 60 Hz; `recomp/analysis/d08a32/integ/shots/freezer-arm/`, local): at
  idle the left glove floats left of and below the gun body with a visible
  gap; while firing the left arm swings further away to the screen's bottom
  left. Running looked close to idle. Not checked: third person, crouch,
  swimming, jetpack, other FOVs, high refresh.

Likely route (to confirm): a Freezer support-palm contact like D36H1 (move the
whole left chain onto the housing, endpoints solved, the right hand and weapon
framing untouched), and a hold of the ready support grip through the firing
keys if the firing pose is what pulls it away (as the Flamethrower / Energy
contact does with key 24/2). Try a measured offset with the user first
(simple tuning before rigs).

**Acceptance:** the left hand stays on the Freezer at idle, walking, running,
firing (tap and held) and looking up/down, at FOV 60/90/120 and 180 Hz; the
accepted barrel/tip framing and every other weapon unchanged; Vanilla
unchanged; the user's playtest.

**2026-11-07 implementation (Needs playtest).** Measured on private runs: the
Freezer's ready upper clip is 29 (the Gatling's), firing switches to clip 34,
and clip 34 is what throws the left arm to the bottom-left of the screen.
`display_arm_matrix` (`recomp/src/ttk/first_person.inc`) now, for slot 11
only: (1) holds the left chain on the ready grip, clip 29 key 0, through idle,
firing, running and crouching (the same mechanism the Flamethrower / Energy
contact uses with 24/2; right arm, events and recoil stay live), and (2)
translates the whole left chain 30 camera units up and 40 toward the eye
before the endpoint solve (D36H1 style), seating the glove against the
housing's left side. Moving it right instead hides the glove behind the
housing (the weapon paints over the arm), so contact is side contact.
Developer overrides: `DNTTK_FP_FREEZER_HOLD=anim,key` (`-1,-1` = native) and
`DNTTK_FP_FREEZER_SHIFT=x,y,z`. No other weapon, third person, geometry,
save, preference or framework change.

Evidence (private offscreen runs, UI slot 12 copy, LEVEL0, 16:9; local
`recomp/analysis/d36h11/`, before/after in `research/before-after/D36H11/`):
idle, fire (0.15 s and 0.5 s held), settle, look down, look up, run, crouch
and crouch-fire at FOV 60/90/120 (60 Hz) and FOV 90 at 180 Hz all keep the
glove on the housing; the native control row (overrides off) shows the
glove leaving the weapon while firing and crouch-firing. `ttk-input-test`
passes. Executable
`5f5e4eaefdd742983930274ffea1344260b374d2371461dfb5f5054a8d83d83e`.
Limits: sparse frames, not continuous motion; tap fire, swimming, jetpack,
ladders and the third-person view not checked; no live Vanilla sweep (the
first-person display arm path is a Modernized feature and Vanilla code is
untouched). The left arm no longer runs its native running swing with the
Freezer (it holds the grip, like the Energy Weapon).

### D36I1 - Slot-5 hair-adjustment hands popping in first person

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D36G/I (Done). Related: D35N, D17AB, D14A-C.**

User selected immediate implementation: current UI slot 5 faces a mirror;
interacting shows Duke adjusting his hair in the reflection while the primary
first-person hands pop in and out. Register and investigate this bounded polish
job using private save/card/settings copies and the accepted D36 audit.

Scope: diagnose native Mirror arm admission and clipping; correct abrupt
whole-part disappearance while preserving native animation, reflection, world
occlusion, interaction timing, weapon return, third person and Vanilla. No new
arm assets or unrelated presentation redesign. Acceptance: compare slot-5
entry/active/exit and repeat use; verify all six arm parts, reflected versus
primary ownership, look/FOV changes, load/view transitions and 60/120 replay.
Keep evidence limits and user visual acceptance separate. The user accepted
the candidate and authorized lock-in and closeout on 2026-10-28.
[Note 299](documentation/299-d36i1-mirror-arms.md) records verification and limits.

### D36J - Coherent body and quick-kick leg presentation

**Status: Done (user-accepted 2026-10-26). Depends on D36G/D36H.**

Implement the accepted [D36J audit contract](documentation/257-d36-first-person-presentation-audit.md#d36j---coherent-body-and-quick-kick-leg).
Explicit lower-body polygon clipping and plain left-leg Q presentation
(user correction), with separate right-leg door ownership; preserve weapon framing, Q hit/damage/timing, door 116,
view choices, restoration, lighting, performance and Vanilla. Candidate evidence
and limits: [note 261](documentation/261-d36j-body-and-kick.md). Build/native
checks and private live routes pass within their documented bounds. User accepts
the plain left-leg revision and authorizes closeout, commit and push. D36K/L
are not selected.

### D36J1 - Full-body first-person quick-kick investigation

**Status: Done (no-body prototype user-accepted; closeout authorized 2026-10-28). Dependencies: D36G/J, D14A-E/E1 (Done). Related: D12A-C, D36K/L.**

User requests a new deep investigation and evidence artifact before choosing a
refinement. At FOV 80/90, downward Q appears to originate near Duke's shoulder;
jump + Q exposes torso and appears to originate above the belt. These are user
reports, not newly reproduced results. Preserve accepted plain left-leg Q and
separate native right-leg door kick. D36J acceptance remains historical.

Initial deliverable: stand up the job and standalone investigation artifact.
Research scope: trace camera/pelvis/leg ownership, projection, depth and pose;
review published modern FPS/full-body first-person practice with named examples
and source limits; capture matched baseline and controlled experiments for fixed
repositioning, hip-relative angling, pitch/phase blending and airborne clearance.
Evaluate the user's low/downward versus high-kick proposal without assuming a
pitch-only rule or mesh replacement is the answer.

Acceptance: artifact contains reproducible screenshots and motion sequences,
source-backed diagnosis, experiment results including rejected alternatives,
recommended solution with tradeoffs, regression evidence and explicit gaps.
Cover default/60/80/90, down/level/up sweeps, stand/walk/run/crouch, jump ascent/
apex/descent/landing, kick start/extension/retraction/repeat/cancel, short/long
weapons and costume mapping. Sample 4:3/16:9/21:9, 60/120 replay, near walls,
D14E1 corner guard, lighting, view/load transitions and door kick. Preserve Q
hit reach/damage/timing/sound, native matrices, weapon framing, free look,
reflections, third person and Vanilla. Record all untested combinations.

User subsequently selected a no-body prototype: hide ordinary first-person
body while preserving arms, weapons and temporary Q/door legs. Compare unchanged
kick geometry at FOV 60/80/90 before proposing pose changes. This is an opt-in
prototype. User playtested and accepted the no-body direction. Closeout covers
this bounded prototype and its evidence; default integration and remaining
polish/research are split into D36J2, D36H5 and D36J3 above. The original broad
research matrix below is historical scope, not a claim of exhaustive completion.
Use private test state; captures and retail-derived assets remain local.
[Investigation contract](documentation/303-d36j1-quick-kick-research.md) and
[standalone artifact](documentation/artifacts/quick-kick-realism.html).

### D36J2 - Integrate no-body as the Modernized first-person default

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependency: D36J1 (Done, accepted prototype).**

Promote the accepted no-body direction into ordinary Modernized first-person
launches without requiring the experimental environment flag. Decide and document
any retained diagnostic body comparison route; do not add a player setting
without a concrete need. Preserve arms, weapons/items, temporary Q and native
door-kick presentation, complete third-person/reflected Duke and Vanilla.

Acceptance: normal launcher uses no-body first person; verify mirror reflection,
door kick, swim, jetpack, ladder/pole/wall/overhead/mantle/water-exit, held items,
view changes, pause/load/restart and kick cancellation. Check default/60/80/90,
supported aspects and 60/120 replay. Retain gameplay matrices, collision, aim,
attack timing/damage/sound, save compatibility and D14E1 unsafe-eye protection.
Record actual action/equipment states and visual coverage limits. Update manual,
artifact and tests appropriate to the final integration. Broad shoulder/arm
redesign belongs to D36H5; integration must not claim that concern resolved.

Accepted implementation and evidence: [D36J2 integration note](documentation/304-d36j2-no-body-default.md).
Ordinary launches use no-body; exact `DNTTK_FP_BODY=visible` is a one-launch
diagnostic comparison. Build/native suites and bounded live action/restoration
routes pass; user accepted the integration. Named coverage limits remain.

### D36J3 - No-body quick-kick motion and weapon-overlap polish

**Status: Done (revision 3 user-accepted; closeout authorized 2026-10-28). Dependency: D36J2 (Done). Coordinate with D36H5 (Done).**

Review the accepted Q under no-body presentation before deciding whether further
changes are useful. Preserve the actual plain left leg and separate native
right-leg door kick. Inspect chamber/extension/recovery, held/repeated Q,
cancellation and downward airborne views with short and bulky weapons. The
current visibility-only result is the baseline; no pose rewrite is presumed.

Acceptance: readable leg entry, strike and recovery without exposed cut ends,
objectionable thigh framing, sudden popping or confusing weapon overlap in the
verified matrix. Compare default/60/80/90, level/down/up and continuous pitch,
stand/walk/run/crouch, jump ascent/apex/descent/landing, costumes and supported
aspects. Keep exact hit reach/damage/callbacks, attack/impact timing, sound,
weapon framing, camera freedom and native gameplay joints. Check door action,
view/load/replay restoration and Vanilla. Retain motion/stills, source diagnosis,
measured cost if changed, and explicit limits. If unchanged Q passes review,
document that finding rather than introduce unnecessary animation changes.

2026-10-28: user accepted revision 3 ("thats it! lock it in and close it out").
Q uses the same fixed H=256 presentation as weapons, keeping its size/framing
stable across world FOV 60/75/80/90. Entry/recovery easing remains; the rejected
adaptive hip translation is removed. Native door projection, weapon framing,
gameplay matrices, hit reach/damage/timing and sound remain unchanged.
Build, controls/input/aim/near suites, exact mesh-packet projection regressions
and bounded private motion/replay checks pass. [Note 308](documentation/308-d36j3-quick-kick-transitions.md)
records the accepted result, rejected revision and evidence limits. Closeout
includes authored implementation, tests, manual and documentation as
MusicMonsterMod. Remaining unrun combinations are not claimed certified.
No next job selected.

At the D36J1 closeout these three jobs were registered only. D36J2 was
subsequently selected; its current status is above. D36H5 owns the priority
shoulder concern. Original D36J1 research questions that remain useful transfer
to these scopes; unrun pose experiments are not claimed completed.

### D36J4 - Door-kick fixed display FOV

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D36J3 (Done).**
User reports the door-opening right leg stretches at wide FOV in current slot 10.
Route this camera-relative leg and its thigh attachment through the weapons/Q
fixed projection. Preserve native door timing/events, gameplay matrices, Q,
contact arms, third person, reflections and Vanilla. Reproduce using private
copies of current slot 10; compare FOV 60/default/80/90 and 60/120 replay.
Acceptance requires packet-level projection invariance, native guard regressions,
real door entry/action/recovery observation and user visual review.
Evidence and work log: [note 310](documentation/310-d36j4-door-kick-fov.md).

### D36J5 - Optional first-person legs with fixed display FOV

**Status: Done (user-accepted; closeout authorized 2026-10-28).** Selected 2026-10-28. Restore ordinary legs without
torso, sharing the fixed H=256 projection used by weapons and kicks while
retaining native animation and safe world clipping. Advanced > First-person
Legs defaults On, applies on Confirm and persists. Off retains kicks/arms.
Verify FOV stability, costume part mapping, view/action isolation, settings
round trips and unchanged native skeleton. User visual acceptance received 2026-10-28.
Evidence: [note 311](documentation/311-d36j5-first-person-legs.md).

### D36K1 - Connected unarmed jetpack arms

**Status: Done (user-accepted 2026-10-26). Dependencies: D36G, D36K, D36H4 (Done).**
Selected 2026-10-26: investigate backward-flight floating hands, stand up this
job, then implement. Source identifies the missing Jetpack arm clipping owner;
private slot-8 flight reproduces equipment 0 and backward animation 166.
Scope: admit native unarmed arm chains to safe world clipping during first-person
flight, preserving native flight poses and existing grounded arm framing.
Acceptance: connected visible arms backward/forward/strafe/hover and pitch;
preserve armed display, fuel/motion, global/action third person, Vanilla,
semantic costume mapping, draw cleanup, save/load and 60/120 replay.
User accepted: "lock it in, close it out". Implementation, tests and documentation
commit/push authorized. [Note 273](documentation/273-d36k1-jetpack-arms.md)
records source/depth evidence, implementation and bounded verification.

### D36M - First-person power-up looks: invisibility, invincibility, Double Duke

**Status: Done (user-accepted 2026-11-03; user direction: a light Quake/Doom-style screen tint per power-up on the D39 tint, [note 370](documentation/370-d39-screen-flash-framework.md) D36M section). Dependencies: D36J, D08A17.** In third person Duke turns
translucent while invisible and changes colour with invincibility and Double
Duke; the first-person arms, legs and weapon do not. The user wants the
first-person view to show the same: translucent while invisible, the power-up
colour for invincibility and Double Duke, for as long as each lasts (the
D08A17 countdowns already show the time). Find how the game marks the model
for each power-up (render flags, semi-transparency, colour), apply the same
to the first-person body and weapon presentation (D36 ownership, note 257's
fade rules preserve intentional translucency), and keep the Bio Mask and
night-vision overlays correct. Vanilla unchanged.

### D47 - First shotgun shot smokes from Duke's left shoulder

**Status: Done (user-accepted 2026-11-08: "lock it in, close it out! good work"; closeout authorized; [note 422](documentation/422-d47-first-shot-smoke.md)). Dependencies: none.**
User: "the first shoot from the shotgun seems to 'fire' (smoke etc) from
duke's left shoulder (when viewed from first, and third person). you can see
the smoke appear for no reason there. why does that happen? small glitch?"
Only the first shot is reported; seen in both views. User: it has been
happening "for a good hot minute", before D41B, so D41B did not cause it.
Find what places the
first shot's smoke (and whether the round or flash also starts there), and
whether it is original behaviour or a regression from the recomp's aim,
muzzle or first-person work. Acceptance: the first shotgun shot's smoke
appears at the muzzle like later shots, in both views, confirmed by the user.

**Work log (2026-11-08, Needs playtest).** The status line had been
overwritten by D51's closeout text (af34de4); D47 was never started before
this. Lab `recomp/analysis/d47/lab.py`, private copy of UI slot 8. Cause:
the puff (effect `0x12`), D41B's muzzle light and the rounds use the muzzle
point player `+0xbc`. Duke's draw writes it (`0x80032364` in `0x800348d8`)
from the pose it draws. The gun call runs earlier, in the update, so a press
that finds the shotgun at rest reads the rest pose's muzzle, at the left
shoulder (Duke-relative -203, -257, -246). The next frame draws the gun aimed
(-374, -120, +68), so held shots are right. This happens on every press from
rest. It is the original code order, not a recomp regression: the same with
the original weapon timing, and the recomp never writes `+0xbc`. Fix
(`muzzle_settle.inc`, Modernized only): the gun call notes the old muzzle and
the live effects. At `0x80032bd8`, entered from Duke's draw right after the
muzzle write, the shot's puff moves onto the new muzzle, and its muzzle light
with it. Effects are drawn after the actors, so no live frame shows it at the
shoulder. The state is in guest memory, so the 120/180 Hz redraw workers'
copies (taken before that draw) make the same move. Only shots that left a
puff there are moved, i.e. the shotgun; the Desert Eagle, Gatling and RPG are
unchanged. Rounds and hits are unchanged. New entry hook `0x80032BD8`: one
generated file, codegen hash unchanged, saves load. After: the puff is at the
aimed muzzle on every press, first and third person, at 60 and 180 Hz (15
consecutive 180 Hz frames, none at the shoulder). Vanilla: disabled. Input
and aim suites pass; the controls suite stops at the known D14 HUD check.
Remaining: at 120/180 Hz the first game frame's redraws still light from the
old point. User playtest accepted 2026-11-08: "lock it in, close it out! good work"

### D48 - Steady left hand on the Gatling gun

**Status: Done (accepted 2026-11-08: "im very happy with it"; [note 423](documentation/423-d48-gatling-steady-hand.md)).**
User: "can we stabalize duke's left hand with the gatling gun, i dont think
it needs to violently shake backwards and forwards. you can observe that by
simply opening the gatling gun and firing with it. is that really how the
game wants to do it? i think we could aim for a much more professional
animation". First establish whether the shake is the original firing
animation or comes from the recomp (first-person presentation, D36 arms,
recoil or aim work), in which views it shows, and what the right hand and
gun do meanwhile. Then hold the left (support) hand steady on the gun, with
at most a small recoil that follows the gun. Modernized only; Vanilla
unchanged. Read the D36 audit (note 257) and the D36H5 arm notes (305, 306)
first if the first-person arms own it. Acceptance: user playtest firing the
Gatling in first and third person.

**Done.** First-person left hand held on the ready grip (as D36H11).
