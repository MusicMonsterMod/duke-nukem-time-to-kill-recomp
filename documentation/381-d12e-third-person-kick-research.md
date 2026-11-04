# D12E - Third-person quick kick (research report)

2026-11-04. **Done (research closed by the user 2026-11-04; implementation is D12E1, then D12E2).** Research only: no gameplay code, hook, build or executable
change. Executable measured:
`ac4634201d59a1b6133043becb570c55aec461db5631df071c31a9667a5136d4`
(the accepted D10F1 build).

User (2026-11-04): the third-person kicks "are so slow that you will
basically get [hurt] if you actually take [it] seriously"; "we want the quick
kick to be quick ... exactly the same thing as what we do spam it in first
person and you have more offense"; it must look "absolutely legit", not a
sped-up spin.

## 1. The original third-person kick (static read and measurement)

### Who may kick

The Modernized Q shortcut (`recomp/src/ttk/shortcuts.inc`, `quick_kick`
action) calls the original Boot attack `0x800517a4` in third person only
when **all** of these hold: the Boot is selected and holstered
(`player+0x3b8 & 0x00ff00ff == 0`, no pending selection), lower and upper
animation are both idle 63, no movement input and no jump. With a weapon
drawn, Q does nothing in third person; while walking or running it does
nothing. That is the first half of the problem: to kick, the player must
holster, select the Boot and stop.

### What it plays

`0x800517a4` picks `112 + rand() % 4` (disassembly: remainder 0 -> 112,
1 -> 113, 2 -> 114, 3 or negative -> 115) and zeroes the lower track. Only
these four reach the kick case `0x80049044` of the dispatcher `0x80048410`
(table `0x800140a8`, index = animation - 63), which holds the executable's
only call to the damage sphere `0x800a979c` (ankle point, radius 96, type
10). Animation 116 is the door kick (D35K), not an attack.

Clip timing decoded from the disc (`recomp/analysis/d12e/clips.py`,
`foot.py`; all four channels of each animation share the clip length, so the
kick is a full-body move: legs, both arms, chest and head):

| Anim | Keys x ticks | Total ticks | Sound event 0x800 | Ankle at the strike key (from pelvis, z ahead, y up negative) | What it is |
| --- | --- | --- | --- | --- | --- |
| 112 | 8 x 40 | 320 | key 4, tick 160 | z 417, y -830 | Lean-back high kick |
| 113 | 9 x 40 | 360 | key 5, tick 200 | z 415, y -892 (head height) | High front kick |
| 114 | 11 x 30 | 330 | key 7, tick 210 | z 462, y -754, after the ankle swings through x -366 / z -396 behind | **Spinning kick** (the "jiu jitsu" one) |
| 115 | 9 x 40 | 360 | key 5, tick 200 | z 446, y -656 (hip height) | Straight front snap kick (D12A's source) |

Before the strike key the foot is still at the knee: for 113 the ankle sits
at y about -77 for keys 0-3 (ticks 0-160), so the first 0.47 s is wind-up
with no reach.

### Measured in game

Private copy of the D10B apartment fixture (runtime slot 5), offscreen,
port 9617, third person, Boot selected and holstered, 60 Hz presentation,
game update delta 10 throughout (`recomp/analysis/d12e/tpkick.py`,
`runs/`, `tp.json`, `wall.json`, local). Player files hash-checked unchanged.

| Measure | Result |
| --- | --- |
| Q to kick animation start | 35-92 ms (next game update) |
| Kick length 113 / 115 | 1.04-1.06 s (64-65 frames at 60 Hz), 7 kicks |
| Kick length 114 | 0.92-0.97 s (57-58 frames), 4 kicks |
| One tick | about 2.94 ms (360 ticks in 1.06 s) |
| Foot reaches out (strike key) | about 0.47 s (112), 0.59 s (113, 115), 0.62 s (114) after the start, plus the start delay |
| Recovery after the strike | about 0.35-0.47 s, nothing can be done |
| W held from 0.1 s after Q | Duke does not move until the kick ends (1.08 s), then runs |
| Q held 6 s | **one** kick; the original never repeats a held Q |
| Root drift during 115 | about 60-130 units (the clip's own step back and forward) |

The random pick alternates in practice (this fixture gave 115, 113, 115,
113 ... and 114 four times in a row in another run), so a player meets all
of them, including the spin.

For comparison, the accepted first-person quick kick (D12A, `kick.inc`):
snap at 117 ms (frame 7 of 60 Hz host frames), extension 183 ms, hits on
frames 8-20 (133-333 ms), done at 450 ms, a held or queued Q repeats every
450 ms, any weapon stays drawn, movement continues. Four contacts per kick
at the normal delta deal 1500 to a pig cop (three kicks kill one). The
original third-person kick deals more per kick (1875-3750 point-blank) but
takes 2.3 times as long and locks Duke in place.

**So the third-person kick costs about 0.6 s of standstill before the foot
can land and 1.0-1.1 s in total, from a holstered Boot only.** The
first-person kick lands in about 0.15 s, from any weapon, on the move.

## 2. How third-person action games do it (sourced brief)

A web research pass (2026-11-04). Official frame data for most shooters
(Gears, Resident Evil 4 / Remake, Max Payne 3, Uncharted, The Last of Us,
Dead Space, DOOM Eternal) is **not published**; numbers below are only what
sources give.

- **Duration.** Duke Nukem 3D's quick kick is a counter: "setting
  quick_kick to 15 will cause the player to perform a quick kick"
  (https://wiki.eduke32.com/wiki/Quick_kick), about 0.5 s at Build's 30 Hz
  logic (inference: the page does not state the rate). It is a
  secondary attack over the held weapon. For Honor, a slow read-and-react
  PvP game, uses 400 ms attack startups and sped a dodge bash from 500 to
  433 ms (Ubisoft posts). A God of War design article's worked example is a
  60-frame attack with hit frames 15-20 and cancels from frame 21
  (https://www.gamedeveloper.com/game-platforms/combat-canceled-i-god-of-war-i-action-game-design).
  Halo players cancelled "the remainder of the melee animation (and the
  delay after the melee)" (https://www.halopedia.org/BXR): recovery is a
  separate, cancellable phase.
- **Layering.** Unreal's Layered Blend Per Bone is documented for "a melee
  attack, reload animation or any other animation that you want to be able
  to do while moving around"
  (https://dev.epicgames.com/documentation/en-us/unreal-engine/using-layered-animations-in-unreal-engine);
  Unity uses body masks with Override layers, the manual's example being
  lower-body locomotion with upper-body shooting
  (https://docs.unity3d.com/460/Documentation/Manual/AnimationLayers.html).
  A kick inverts the usual split: the leg layer owns the legs and pelvis, the
  aim keeps the spine and arms.
- **Root motion.** Motion warping bends a melee's root motion so the
  character arrives at the target
  (https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-warping-in-unreal-engine);
  Halo's lunge shows the trade-off: reach, but a later hit
  (https://www.halopedia.org/Melee).
- **Hit-stop and camera.** Sakurai: hit-stop scales with damage and the
  victim shakes; keep the attacker moving slightly
  (https://sourcegaming.info/2015/11/11/thoughts-on-hitstop-sakurais-famitsu-column-vol-490-1/).
  Community guides give 2-4 frames for light hits and 50-150 ms camera shake
  (not authoritative). God of War's developers made up for reduced camera
  shake with audio and animation. No source was found for shooter FOV punch
  on a melee.
- **Cancels and buffering.** A buffered press is stored and fires when the
  window opens; most fast melees accept the next input before the current
  one ends (God of War article above). The Last of Us Part II tags hit
  frames and target tracking on the timeline
  (https://gamedeveloper.com/design/how-naughty-dog-defined-melee-attacks-and-behaviors-in-i-the-last-of-us-part-ii-i-).
- **Looking authored at speed.** Keep the key poses but compress the
  anticipation; hold the active pose; put the saved time into follow-through
  that does not block input (Jonathan Cooper,
  https://www.jikguard.com/news/the-12-principles-of-animation-in-video-games).
  Smears belong between two strong keys, not on the impact pose. God of War
  2018 relied on 3-5 frame blends between attacks
  (https://gamedeveloper.com/production/creating-fluid-combat-animations-for-the-new-i-god-of-war-i-).
  Overgrowth (GDC 2014 Animation Bootcamp) built its combat from a few keyed
  poses with procedural interpolation, close to what this project already
  does with Duke's clips.

Takeaway: a spammable armed melee is about 0.4-0.6 s in total, lands in the
first 0.1-0.2 s, keeps the weapon and aim on the upper body, and spends its
remaining time on a recovery the next press can cut into. A uniform speed-up
of a full-body clip is what reads as "sped up".

## 3. Options

| Option | Time to contact / total | Weapon and aim | Moves | Looks | Cost |
| --- | --- | --- | --- | --- | --- |
| A. Play 112-115 faster (x2.3) | 0.26 / 0.45 s | Lost: full-body clip, holstered Boot only | No | Sped-up spin and flailing arms; 114 still spins | Small |
| B. Cut 115: start at the chamber key, play keys 4-8 | about 0.15 / 0.5 s | Lost (full body) | No | A jump cut into the chamber; still a holster | Small |
| C. **Legs layer: an authored quick-kick clip on channel 0 only, keyed from 115** | 0.13 / 0.45 s (D12A cadence) | **Kept**: channels 1-3 (both arms, chest, head) stay on the original aim and fire | Yes, at a capped pace | Authored snap kick, pelvis and support leg move | Medium |
| D. Host leg override (private matrices on the right leg only, as the first-person leg) | 0.13 / 0.45 s | Kept | Yes | Leg pokes out of a running or idle body; no hip or support leg; reads mechanical | Small |
| E. New full-body clip in Blender with an upper-body correction | 0.13 / 0.45 s | Kept only with an extra aim correction | Yes | Best possible | Large; C delivers most of it |

The engine already supports C. D08K4 crouch walking plays host-owned
authored clips (`ttk-anim.pack`, built by `recomp/tools/local/ttk_crouch_walk.py`,
loaded by `anim_override.cpp`) on channel 0 (legs and pelvis) while channels
1-3 keep the original crouch aim and fire, with an upper-body compensation
pass so the chest stays steady when the pelvis moves (`crouch_walk.inc`).
The quick kick would be the second user of that path.

## 4. Proposal: D12E1 - third-person quick kick (legs layer)

First proposal; section 6 is the locked design where they differ.

Modernized, independent camera, third person (`fp_blend == 0`) only.
Vanilla, first person (D12A) and the ORIGINAL setting keep today's
behaviour.

**Input and eligibility.** Q in third person, any weapon or holstered,
standing, walking or running on the ground. Boot-selected requests (Q, a
held left-click) are converted as D12A does at `0x800493a4`, so 112-115
never start in QUICK mode. Not in the air, crouched, swimming, climbing,
pushing or in interactions (Q does nothing there, as now). A view switch
during a kick cancels it, as in first person.

**Timing budget (host time, frame-rate independent, the D12A cadence so
both views feel the same):**

| Phase | Time | Pose |
| --- | --- | --- |
| Anticipation | 0-67 ms | Weight onto the left leg, pelvis drops about 20 and tips back about 5 degrees, right knee starts up |
| Chamber | 67-117 ms | Knee to hip height (115 key 4) |
| Snap | 117-183 ms | Shin whips out; one in-between with the knee leading the foot |
| Hold (active) | 183-283 ms | 115 strike pose (key 5): ankle about 445 ahead at hip height, toes pointed; hits on 133-333 ms |
| Retract | 283-350 ms | Back to the chamber (overlap: the foot trails the knee) |
| Settle | 350-450 ms | Foot down, small pelvis overshoot, then blend into the current locomotion (about 100 ms) |

Chaining as in first person: a Q during a kick queues the next, a held Q
repeats every 0.45 s, and the queued kick may start from the settle phase
(after 350 ms), so a spammed kick runs at about 0.35-0.45 s per kick.
Fire stays available throughout (the arms are not in the clip).

**Animation.** A new authoring tool (`ttk_quick_kick.py`, the
`ttk_crouch_walk.py` pattern) writes one legs-channel clip (about 14 keys)
into `ttk-anim.pack`, keyed from 115's rest, chamber and strike poses with the
anticipation, in-between and settle above, right leg kicking, left leg
planted with a bent knee. Channel 0 carries the pelvis, so the upper body's
compensation keeps the chest, head, arms and weapon on the aim. Review in
Blender (allowed) and in game at 60 and 180 Hz, plus a contact sheet. No new
mesh; the D08A28 style guide does not apply. D36 ownership: this is native
channel animation, not a `LimbOwner` presentation; the first-person leg
stays `LimbOwner::Kick`, and only one of the two runs at a time.

**Movement.** Duke keeps facing the aim (D07A). Ground travel during
the anticipation-to-retract phases (0-350 ms) is capped at the walking pace
so the planted foot does not skate, then the configured pace returns. No
root motion beyond a small forward step on the snap (about 30-40 units, the
original's own step, through Duke's normal collision). Running Q kicks
therefore slow Duke briefly instead of stopping him. If the user wants full
run speed, a later running-kick clip variant is the honest fix.

**Hit.** The D12A host sphere (original `0x800a979c`, radius 96, type 10,
the same damage formula, Duke as attacker) on every player update of
133-333 ms. Its point is the posed ankle in the world, but pulled to the
aim: the original segment query from Duke's hip along the aim, and where it
meets an actor or a surface within the foot's reach (about 480), the sphere
goes just short of it (the D12A low-object rule, measured from Duke rather
than the eye). D12C impact sound on the first real contact only. Same damage
per kick as first person (1500 to a pig cop at the normal delta).

**Feel.** No global hit-stop (it would freeze enemies and the shared update
rate). On contact the strike pose holds for one extra game update (about
33 ms of the 100 ms hold, no time added), and the camera gets an optional
small forward punch (under 1 degree, about 80 ms, default off, judged with
the user). The original kick's occasional enemy shove (reaction 84) stays
out of scope as in D12A.

**Toggle.** ADVANCED > a new COMBAT heading > **KICK: QUICK / ORIGINAL**
(profile `controls.third_person_kick`, override `DNTTK_TP_KICK`),
QUICK default in Modernized; hint "QUICK: A FAST FRONT KICK IN THIRD
PERSON. THE WEAPON STAYS UP". ORIGINAL restores today's holstered Boot kick
(112-115) exactly. Vanilla never sees the row's effect. GAME_MANUAL.md
updated with the row and the Q behaviour.

**Tests planned.** Native controls cases (conversion, eligibility per state,
the clip only on channel 0, channels 1-3 untouched, hit window and sphere
arguments, cancel on a view switch, ORIGINAL and Vanilla untouched); the
`tpkick.py` probe for latency, length, chaining and the held-Q repeat rate
at 60/120/180 Hz; contact against an enemy (a pig cop fixture, or one the
D26E spawn work adds) and a crate; capture sheets of the clip standing,
walking and running with the pistol, shotgun and a holstered Boot; frame
budget unchanged; the Vanilla regression route.

## 5. Second research pass: ARPGs, character action, fighting games

User (2026-11-04, after reading sections 1-4): the inspiration for "twitchy
third person visuals" is the action RPG, "not just exclusively ARPG's"; "let's
make this custom kick absolutely spectacular"; "we seriously want knowledge".
Two further sourced passes, summarized here. Official frame data is rare;
community and wiki numbers are marked as such.

### Fighting games (readability at speed)

- Street Fighter 6 standing light kicks (community frame data, seen only in
  search snippets): Terry 5 frames startup, 2 active, 12 recovery, 9 frames
  hitstop (https://srk.shib.live/w/Street_Fighter_6/Terry/Data); Cammy 5/3,
  Lily 4/3 (GameSkinny tables). At 60 fps a 4-5 frame startup is 67-83 ms
  and the whole move about 317 ms, with about 150 ms of hitstop on a hit. A
  kick reads in under a tenth of a second when its key poses are strong.
- Smash: hitlag = floor((damage x 0.65 + 6) x multipliers), at most 30 frames
  (Ultimate); the attacker freezes on the contact frame, the victim on its
  first flinch frame and shakes; hitboxes stay active through the freeze
  (https://www.ssbwiki.com/Hitlag). Sakurai: shake the victim more than the
  attacker, let the shake decay, horizontal on the ground, scale it with
  camera distance, keep the attacker creeping forward, blend the victim into
  the hurt pose over about 4 frames
  (https://sourcegaming.info/2015/11/11/thoughts-on-hitstop-sakurais-famitsu-column-vol-490-1/,
  https://nintendowire.com/news/2022/12/12/this-week-in-sakurai-12-5-12-11-fine-tuning-hit-stop-and-cheating-the-system/).

### ARPGs

- Path of Exile 3.7 (official): a skill can be cancelled in the first 20% of
  its attack time; the swing's damage cone matches the animation; the basic
  swing was rebuilt for "a more consistent hit rhythm"
  (https://pathofexile.com/forum/view-thread/2512662). PoE 2 reportedly
  authors several variations of an animation so it still reads at high attack
  speeds (secondhand, unverified).
- Diablo II removed whole frames per attack-speed step; Diablo III scales
  continuously (fan wiki, https://di.diablowiki.net/Breakpoint). No Blizzard
  talk on hit reactions was found.
- Lost Ark is praised for "meaty" combat from a hitstop on each hit, against
  Diablo's weightless feel (Kotaku review,
  https://kotaku.com/lost-ark-review-amazon-steam-diablo-mmo-rpg-hands-on-1848518837).
- Monster Hunter Wilds' team lightened the hitstop in the beta, overshot, and
  restored it with added sound for launch
  (https://gamingbolt.com/monster-hunter-wilds-weapon-hitstops-will-be-closer-to-what-people-are-looking-forward-to):
  hitstop is tuned with the player, not set once.
- Hades: no developer source for its combat timings was found (Supergiant's
  GDC 2021 talk is about dialogue).

### Character action

- Devil May Cry 5 (Itsuno, via Kotaku): even a few-frame attack sells a hard
  hit by "displaying certain parts of the animation for a little longer";
  "when you press a button, there should be a clear result on the screen"
  (https://kotaku.com/devil-may-cry-5s-director-tells-us-how-they-made-combat-1833642299).
- God of War 2018 enters stances with 3-5 frame full-body blends rather than
  authored transitions (gamedeveloper.com breakdown).
- Sifu (Sloclap) post-animates its mocap to add "real impacts", because
  gameplay constraints rule out raw capture
  (https://www.gamesradar.com/sifu-developers-talk-about-its-160-moves-each-was-designed-for-a-particular-situation).
  Absolver: players read an attack's first frames, so its readability is
  controlled deliberately.
- Kingdom Under Fire's programmer (Jiesang Song, 2005,
  https://www.gamedeveloper.com/audio/improving-the-combat-impact-of-action-games):
  freeze both for a couple of frames on the best-looking pose (attacker only
  for light quick blades); shift the camera for a couple of frames at the peak
  in the swing direction; directional impact particles and per-hit flashes;
  whoosh, impact and victim sound layers; a hit rumble unlike a miss; at
  30 fps impacts closer than about 5 frames merge into one.

### Shooters with a gun up

- Bulletstorm's boot: "the kick is always the same thing - the same
  animation, the same sound, the same button - but its purpose can change"
  (https://gamedeveloper.com/audio/classic-postmortem-people-can-fly-s-i-bulletstorm-i-);
  a kicked enemy drifts in slow motion with a glow while the world runs on.
- Resident Evil 4: the gun earns the stagger and the kick spends it (player
  guides, no official timing).
- Halo 3: a melee contest window of "three frames, or approximately 100ms"
  (Bungie, https://www.halo.bungie.org/bwu/index.html?item=163); a lunge adds
  reach but delays the hit.
- Duke Nukem 3D's quick kick uses a different leg from the Mighty Foot
  (fan wiki); DOOM Eternal's Blood Punch only spends its charge on a real hit.

### Animation and audio craft

- Jonathan Cooper (Game Anim): minimal anticipation for player moves; a strike
  that reaches extension in 2 frames feels faster than one taking 5; fast in,
  slow out; the hips and torso lead and the limb snaps last; a regain-control
  frame lets follow-through play only when no input arrives
  (https://gamedeveloper.com/production/the-12-principles-of-animation-in-video-games).
- Kick breakdown (Pixelblog 53,
  https://www.slynyrd.com/blog/2024/11/25/pixelblog-53-punches-and-kicks):
  the support foot pivots outward, the hips twist into the kick, the torso
  leans back, the hit effect starts on the contact frame, then follow-through
  and a small overshoot. Smears (2-4 frames, community advice) go between
  strong keys, never on the impact pose; at 180 Hz a smear mostly reads as
  noise.
- Audio: a whoosh before contact plus 2-4 layered impacts, more low end for
  kicks; 3 or more variations with no immediate repeat and a few percent of
  pitch jitter avoid the "machine gun" effect of spam (vendor and blog
  sources, not studio talks).

### What the engine already offers (verified this session)

- Spark burst: original `0x8006d594(6, position, brightness)`, called from
  six places in the executable (the dynamite fuse, `0x8007120c` and others).
  A candidate contact spark at the foot; its look must be checked in game.
- Sounds: the original kick sound `0x1000` (bank 1 entry 0, unverified
  whether a whoosh or a vocal) and the D12C impact `0x2007`, which the host
  already plays at a chosen pitch and volume.
- Duke's legs channel is host-timed in the crouch walk (D08K4), so a pose
  hold on contact (attacker hit-pause) costs no global time.
- No rumble exists in the host today (no SDL rumble call); the original game
  has a vibration setting only for a DualShock.
- No knockback: the original kick's shove (enemy reaction 84) is chosen inside
  each enemy's AI callback, not traced (D12A).

## 6. Locked design (user: "let's try whatever it is that you recommend")

The user selected the recommended option C on 2026-11-04 and asked for the
research above to shape it. These defaults are locked for D12E1; the user
judges them in play.

**D12E1 - spectacular quick kick, core:**

1. Legs-and-pelvis authored clips on channel 0 (the D08K4 path), arms,
   chest, head and weapon on the original aim. Any weapon or holstered;
   standing, walking or running on the ground. Default **QUICK**; ADVANCED >
   COMBAT > **KICK: QUICK / ORIGINAL** (`controls.third_person_kick`,
   `DNTTK_TP_KICK`). ORIGINAL and Vanilla keep 112-115 exactly.
2. Budget (host time, same as first person): coil 0-67 ms (support foot
   pivots outward, hips twist into the kick, torso counter-leans so the
   chest stays on the aim), chamber 67-117 ms, snap to full extension in about
   2 game frames (117-150 ms), strike held to about 283 ms, retract with the
   foot trailing the knee, settle with a small overshoot and a regain-control
   point at 350 ms, done at 450 ms. Hits 133-333 ms. Fire stays available.
3. **Alternating legs:** chained kicks go right, left, right (one authored
   clip and its mirror), so spam reads as a combination, not a loop.
4. **Hit-pause on Duke only, on a real hit only:** the strike pose holds
   about 50 ms (3 frames at 60 Hz) at the first contact, with a slight forward
   creep; the time comes out of the recovery so the 450 ms total and the
   350 ms chain point never move. The aim, camera control and enemies never
   freeze. A whiff gets none.
5. **Camera punch on a hit only:** a few units along the kick direction for
   about 2 frames, then a short decaying shake under 1 degree, scaled down
   with camera distance. None on a whiff.
6. **Impact effects:** the original spark burst at the contact point on an
   actor or wall hit; a dust puff under the support foot only if an original
   effect fits (otherwise none, no new assets).
7. **Sound layers:** a whoosh at the snap (the original `0x1000` if it is
   not a vocal; otherwise a whoosh the game already has), the D12C impact on
   contact with about +/-5% pitch variation that never repeats the same step
   twice in a row; whiffs whoosh only.
8. **Hit** through the D12A sphere (radius 96, type 10, same damage per
   contact), its point at the posed ankle pulled to the aim within the foot's
   reach (the low-object rule measured from Duke's hip). D12C sound rule
   unchanged: impact only on a real contact.
9. Running: the pace is capped at walking from 0 to 350 ms so the planted
   foot does not skate; the configured pace returns after. A small forward
   step on the snap (30-40 units) through Duke's own collision. No magnetism
   in D12E1 (a lunge delays the hit).
10. Clip authoring in a new `ttk_quick_kick.py` (the `ttk_crouch_walk.py`
    pattern): key poses from 115's rest, chamber and strike; foot IK keeps
    the support foot planted; reviewed in Blender and in game at 60, 120 and
    180 Hz with contact sheets. No new mesh.

**D12E2 - finisher and enemy payoff (after D12E1 is accepted):**

- A heavier third kick in a chain (a roundhouse turn keyed from 114's strike
  pose, about 520 ms), with a longer hit-pause (about 83 ms).
- Knockback and stagger through the game's own reactions: trace what
  selects enemy reaction 84 (the original kick's shove) and use it for the
  finisher; no host-side position shove that bypasses collision.
- A draw-only shake of the struck enemy for the hit-pause (no gameplay
  position change), if the renderer allows it per actor.
- Gamepad rumble on a hit (short, distinct from nothing on a whiff).
- Optional short magnetism (step-in toward an enemy within about 100-150
  units of the strike point) if D12E1 play shows near misses.

## Limits of this report

The sections 5 and 6 sources are secondhand for most commercial games (frame
data wikis were partly blocked; Hades and Diablo combat timings were not
found). The locked numbers are this project's choices, set from the measured
first-person kick and those principles.

One fixture (the apartment), so no enemy contact was timed: the contact
window comes from the decoded strike keys and the measured tick length,
not from a struck enemy. The wall run started inside an object's box and
recorded its contact at the first update, which says nothing about reach.
112 was not drawn by the random pick in these runs; its length is from the
clip (320 ticks, about 0.94 s). The web sources give few official frame
numbers for third-person shooters; the budget above is ours, set to match
the accepted first-person kick.
