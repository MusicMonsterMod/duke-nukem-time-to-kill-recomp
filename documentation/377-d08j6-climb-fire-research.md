# D08J6 - Firing while climbing: research report

2026-11-03. Research report (the job's first deliverable). D08J6A accepted and
closed 2026-11-04 (see Closeout). No gameplay code,
build or executable change. User direction while the research ran: "I'm open
to any new animation if you ever wanna do that. we need to aim for absolute
professionalism! you can use blender if you so need".

## Request

User: "the ability to shoot while climbing ... Duke stopping and then shooting
and then you would be able to resume climbing". Example: slot 7 (file 6), the
castle tower with two floating enemies near the climb. Expected: fire while
climbing stops the climb, draws the current weapon, aims and fires; climbing
input stows the weapon and resumes the climb. Vanilla unchanged.

## How current games handle it

| Approach | Examples | Fit for TTK |
| --- | --- | --- |
| No weapons on climbs | many stealth/action games | Safe but not what the user asked for |
| Two-handed weapon kept while climbing | Half-Life (user, 2026-11-04, after playing it: "it's crazy really, having you just hold the weapon with 2 hands while climbing the ladder") | The counter-example this job avoids |
| Hang and aim, one-handed sidearm | Uncharted (ladders, ledges, pipes), Tomb Raider 2013+ (pistol on ladders/ropes), The Last of Us (none) | Closest to the request: one hand keeps the grip, the other aims |
| Full weapon kept, climb freezes | Older third-person shooters, TTK's own original (see below) | Works mechanically, looks wrong when both hands leave the rung |
| First-person overlay unchanged on ladders | Doom-likes, EDuke32 | Fits first person only; TTK first person shows real hands on rungs (D35E/F) |

The professional pattern is the second one: the climb stops, the off hand
stays on the rung or chain, the torso turns toward the target within a limit,
and the gun arm aims at the crosshair. Climbing input (or a short timeout)
holsters and resumes. Two-handed weapons are either swapped for the sidearm
(Uncharted, Tomb Raider) or not allowed. Holster/draw times are short
(roughly 0.3-0.5 s) and inputs during them are buffered, not dropped.

## What the original game already does

Evidence: private offscreen lab `recomp/analysis/d08j6/` (local only), 60 FPS
setting, dummy audio, copies of the D35E alley-ladder, D35F chain and D35I1
climbing-wall fixture savestates; holster rebound to Z in a private profile
only. Player saves, cards and settings were hash-checked unchanged after
every run.

### Static facts (owned EXE)

- Climb dispatcher `0x80043eb8` (mode 3, from `0x8004b60c`) dispatches on
  `anim - 147` through `0x80013dd0`: 147/148 catches, 154-156, ladder rungs
  186/187 (`0x80043fd0`) and 188/189 (`0x80044128`), chain/pole 192-195
  (`0x8004421c`, `0x80044398`), climbing wall 198-205, 207-211.
- Its entry treats Square without Cross (and no `+0x224 & 0x241`) as let go
  (`0x8003f060`, fall 108). The handler never reads equipment (`+0x3b8`).
- The rung handler refuses to climb when the upper-body animation (`+0x74`)
  has animation-table flag 8 (`0x800c2824`). Weapon-holding upper animations
  carry it, so an armed Duke does not move: this is the original "stop".
- Note 119: the skeleton build already has a one-arm aim for record 5 (gun
  arm, `0x800978f8`) during upper animations 8/11 in modes 3/6/7, i.e. the
  original authors gave climbing (mode 3) and the hang modes partial
  shooting support.

### Live probes (Modernized build, original paths)

| Test | Ladder (186/187) | Chain (192/194) | Climbing wall (198) |
| --- | --- | --- | --- |
| Hold left click while holstered | nothing (0 rounds) | - | - |
| One Circle tap (holster key) | draws: upper 3 -> 0 -> 6 -> 5, equipment 2, about 0.45 s, still attached | draws | draws |
| Hold fire, armed | real rounds: 9 in 1.5 s (ammo 164 -> 155); upper 9/11, flag 0x100 | 6 rounds | 6 rounds |
| W or S while armed | no movement (flag-8 refusal) | no movement | no movement |
| Second Circle tap | stows: 6 -> 3 -> climb clip, about 0.4 s | stows | stows |
| W or S after stowing | climbs again; continued to the top exit 190 | climbs (194) | climbs to the exit (206) |
| Space while armed | lets go (108), weapon stays out | - | - |
| Circle held too long | hold-to-select mode (`+0x224 & 0x200`), no draw | - | - |

So stop, draw, fire, stow and resume all exist in the original game on all
three climb families, driven by Circle. What is missing:

1. **Input.** Left click does nothing on a climb. Modernized fire-to-draw
   (`fire_draw_ready()`, D08E; flight D08Q7; swim D08O5) admits the ground,
   flight and swim leases only. Climbing keys do not stow; the player must
   know the holster key.
2. **Aim.** `shot_ready()` (`weapon_aim.cpp`) excludes climbs, so shots are
   not adapted to the camera. The original fires along Duke's own heading,
   which faces the ladder, chain or wall: in the chain probe the rounds hit
   the wall in front of Duke. With independent camera on climbs (D08J3) the
   player can look at an enemy and still hit the wall.
3. **Pose.** The armed pose is the ground upper body pasted onto the climb
   clip: both hands leave the rung (`pose-sheet.png`: rung, armed idle,
   firing). The native one-arm aim points the gun up along the ladder. The
   free hand hangs; nothing holds Duke on.
4. **First person.** On the ladder the normal weapon viewmodel appears
   (hands on rungs disappear); on the chain the weapon was not visible at all
   in the sampled frames. D35E/F presentation was built for holstered climbs.

Not probed live: ceiling monkey bars (mode 7) and ledge hangs (mode 6); the
static one-arm aim covers those modes too. Enemies near a climb: none spawned
(enemy spawning is not available yet); the user's slot 7 castle tower is the
intended acceptance location.

## Recommendation

Use the original mechanics (they already stop, fire with real ammo and
resume) and build the missing input, aim and presentation around them, with
a purpose-built hang-and-aim pose rather than the pasted ground pose.

### Behavior (Modernized, all climb families)

- Left click on a ladder, chain, pole or climbing wall: the climb stops,
  the weapon is drawn and held fire fires. A short click draws only (as on
  land). Rounds go where the crosshair is (shot adaptation, limited to the
  hang-aim arc).
- W/S (and A/D circling on chains/poles) while armed on a climb: stow
  automatically, then the held direction resumes the climb with no extra
  press. Space/Ctrl let go armed, as the original allows.
- Exiting or letting go keeps the weapon state the player chose (no forced
  redraw, matching D08J restore rules).
- Number keys/wheel select only weapons allowed on climbs (see below).

### Which weapons

Option A (recommended): one-handed weapons fire as they are; two-handed
weapons are swapped for the best owned one-handed weapon while on the climb
and restored on stow (Tomb Raider/Uncharted). Option B: every weapon,
with a braced one-arm pose for the heavy ones (more animation work, less
believable). Option C: only one-handed weapons; others do not draw. The
weapon classification needs a short inventory pass against the actual
models (pistol, Desert Eagle and similar sidearms one-handed; shotguns,
Gatling, rifles two-handed).

### Pose (new animation)

- Composite: pelvis, legs, head and off-hand arm (records 0, 2-4, 9-18) from
  the climb clip, so the hand stays on the rung or chain; chest (record 1)
  turns toward the aim within a clamp (about +-70 degrees yaw, +-45 pitch,
  beyond that Duke leans out or the shot is refused); gun arm (records 5-7)
  aims at the crosshair, using the existing look-at helpers (`0x80097a44`,
  `0x800978f8`) as D08O2A did for swimming.
- Blender-authored additions in the D08K1 pipeline: a one-arm draw and holster
  (gun from the holster with the other hand still gripping), an aim idle and
  recoil, and per-family off-hand grips (ladder rung, chain wrap, wall hold).
- First person: keep the native climbing off-hand visible on the rung
  (D35E/F clipping path) and show the gun arm aiming from the opposite side.
  Follow note 257 (D36) ownership and notes 305/306 for arm coverage.

## Staged plan

1. **D08J6A - Fire draws, climbing stows (mechanics).** Admit the climb
   lease (attached mode 3 on authenticated ladder/chain/pole/wall families,
   settled upper body, no transition flags) in fire-to-draw and drive the
   original Circle tap; W/S/A/D while armed send the original stow tap and
   then the held direction; adapt shots to the camera within the hang arc;
   weapon-on-climb rules (option A/B/C). Uses the native pose. Native tests
   for every climb anim and family, Vanilla refusal. Test at 60/120/180.
2. **D08J6B - Hang-and-aim pose, third person.** Joint composite above,
   torso clamp, off hand on the rung; Blender one-arm draw/holster and recoil.
   Validate in game on ladder, chain and wall from several angles.
3. **D08J6C - First person.** Gun arm and off hand on the rung in the
   ladder/pole views; no disappearing weapon on chains; D36 regression set.
4. **D08J6D - Remaining climbs.** Ceiling monkey bars (mode 7) and ledge
   hangs (mode 6), after a live probe of each.

Acceptance location: the user's slot 7 castle tower (two floating enemies)
through a user-made private save state, plus the alley ladder, D35F chain
and D35I1 wall fixtures.

## Decisions for the user

1. Weapons on climbs: option A (sidearm swap, recommended), B or C.
2. Resume: climbing input stows and climbs (recommended), or also an
   automatic stow after a few seconds without firing.
3. Whether to start with D08J6A alone (playable sooner, native pose) or
   build D08J6A and B together so the pasted ground pose is never shipped.
4. A save state at the slot 7 castle tower climb, near the floating enemies,
   for testing.

## User decisions (2026-11-04)

User: "let's totally go with your recommendation. Uncharted, and Tomb Raider
are BRILLIANT comparisons ... of course, we go total cleanroom". So: option A
(sidearm swap), climbing input stows and climbs, hang-and-aim pose with new
animation, stages A-D. After playing Half-Life the user added: "it's crazy
really, having you just hold the weapon with 2 hands while climbing the
ladder" (the counter-example above).

## D08J6A - mechanics (Needs playtest)

### Behavior (Modernized only)

- Held left click on a ladder (186-189), chain/pole (192-195) or climbing
  wall (198-205) stops the climb and draws the sidearm (slot 4); held fire
  fires; a short click draws only. Mid-step, the draw starts at once and the
  step ends at its rest.
- Another selected weapon is swapped for the sidearm and given back once Duke
  holsters on the climb, or, after letting go armed, on settled ground through
  the original weapon request.
- W/S/A/D while armed spend one Circle tap to stow (from the idle or the
  one-arm aim hold 8 that lingers after the last shot); the held direction
  climbs on once holstered. Held fire keeps the weapon out.
- Shots follow the view (crosshair shown), with the existing cover checks.
- On the climb the draw and stow run 2x and the aim lowering 4x once fire is
  released; the firing rate is unchanged.

### Implementation

- New `climb_fire.inc` (controls adapter): climb lease
  (`climb_weapon_ready`: Modernized, normal gameplay and camera ownership,
  mode 3/3, a climb clip, an attachment, identity), the draw request at a
  second plugin on the existing guarded climb-dispatcher hook `0x80043eb8`
  (Duke from `0x8004b60c`), the swap and its returns, debug JSON
  `controls.climb_fire`.
- `pc_input.cpp`: held fire with a sidearm to draw withholds climb directions
  and unarmed Cross; the stow Circle tap uses the existing one-tap request.
- `wall_climb.inc`: held fire counts as zero input for the host wall motion.
- `shortcuts.inc` (`select_weapon`): the ground return of the swap.
- `modern_controls.cpp`: the upper-track boost at `0x80059db0` admits the
  climb transitions. `weapon_aim.cpp`: `shot_ready()` admits the armed climb.
- No new hook address, guard, generated-code, framework or codegen change;
  existing savestates load. Vanilla: everything is behind `input_modernized()`.

### Verification

Private lab `recomp/analysis/d08j6/` (local only): offscreen OpenGL, dummy
audio, port 9611, copies of the D35E alley-ladder (slot 0), D35F chain
(slot 0) and D35I1 climbing-wall (slot 11) fixture savestates, holster on Z
in a private profile only. Player saves, cards and settings hash-checked
unchanged after every run.

| Route (shotgun selected) | Ladder | Chain | Wall |
| --- | --- | --- | --- |
| Hold fire 1.5 s | sidearm drawn, 6 rounds, 6 view shots | same | same |
| Then W/S | holstered, shotgun selection back, climbed (top exit 190) | climbed | climbed (top exit) |
| Space armed, land | shotgun drawn back on the ground | same | same |

Game-frame timings at the 60, 120 and 180 FPS settings (identical): fire
pressed mid-step to equipment out 9-17 frames, first shot 15-25; fire
released with a climb key held to holstered 17-21, climbing again 21-25
(before the speed-up: 28 to the first shot and 65 to climbing again).
Short click: drawn, 0 rounds. Looking away from the ladder, the rounds land
at the crosshair on the far wall. Vanilla: Cross held on the rung leaves
Duke holstered, Up climbs. Modern land fire-draw (D08E) unchanged.

Native suites pass: `ttk-controls-test` (D08J6A group: lease, clips, draw
request and guards, swap and both returns, stow/shot gates, Vanilla),
`ttk-input-test` under Xvfb (directions/Cross withheld, stow tap),
`ttk-aim-test` (D08J6A climb shots) and `ttk-near-test`.

Executable SHA-256:
`c5c6a30b010df99ad7981cf3452d6105ba02811d2cc2a050e0bff7b6abed9a56`.

### Limits

- The pose is still the original: both hands leave the climb and the gun arm
  points along Duke's heading (D08J6B). In first person on the ladder the
  normal viewmodel appears; on the chain no weapon shows (D08J6C).
- No enemies near a climb were tested (enemies cannot be spawned); the slot 7
  castle tower needs a user save state.
- Monkey bars and ledge hangs are not included (D08J6D).
- Number keys and the wheel do nothing on climbs. A save made with the swap
  pending keeps the sidearm selected.
- Functional state, shot and timing checks, not feel or visual acceptance.

## Closeout

2026-11-04. User: "great progress so far and i love the direction ... lock us
in and close us out". D08J6A is Done. Stages B-D continue as D08J6B (third
person hang-and-aim pose), D08J6C (first person) and D08J6D (monkey bars and
ledge hangs). User test saves for them, used only as private copies: UI slot
12 ladder, UI slot 11 chain, UI slot 6 monkey bars, UI slot 8 climbing wall.
The user also asked for shooting while mantling, now D08J7 and the next job.
The runtime patch re-export was unchanged (no framework edits). No gameplay
edits, builds or launches after acceptance; no player data touched.

## D08J6B - hang-and-aim pose, third person (Needs playtest)

2026-11-04. Prototype on the original poses, judged in game before any
Blender clip (the D08J7C order; see the user decisions under Limits).

### What the original does (probes)

- Armed on a climb, only channel 1 (right arm, `+0x74`) carries the sidearm
  pose; legs, the left arm and the chest/head (channels 0, 2, 3) stay on the
  climb clip (ladder: 187 on 0, 2, 3; upper 3 -> 0 -> 6 -> 5 on the draw,
  9/11 firing). So the off hand already holds the rung, chain or wall.
- The aim build `0x80097c04` in modes 3/6/7, with a weapon pose (table flag
  8) and `+0x224 & 0x200100` (firing): the head gets the look-at
  `0x80097a44` and, in poses 8/11, the gun shoulder the one-arm aim
  `0x800978f8` (the direction as the bone's Y axis), both from `+0x124`
  (set from the heading at `0x8003fcc8`), which faces the climb. The chest is
  never aimed. Hence the D08J6A limit: rounds went to the crosshair while the
  pistol pointed at the ladder or wall.
- `0x800978f8` has no entry hook (entry hooks are fixed at codegen; adding one
  would change the codegen hash and reject savestates).

### Behavior (Modernized, view aiming, third person, sidearm out, mode 3)

- Chest: turns toward the view about the world vertical, clamped to 50
  degrees each way.
- Off hand: the left arm is solved back onto its climb-clip contact. Shoulder
  swing and elbow bend (the D36H5 two-bone solver, bend plane from the clip's
  own elbow), hand orientation kept, so the grip looks the same.
- Head: looks toward the view, clamped to 70 degrees.
- Firing (the original aim): the head look-at and the one-arm aim take the
  view direction, clamped to the gun arc (110 degrees to Duke's right, 60
  across the body to his left, pitch 70).
- Idle (pose 5): the pistol stays in the original high ready beside the head,
  turned with the chest.
- Weight ramps over 160 ms of real time (the same at 60/120/180) and out for
  the stow (upper 7). Hangs and monkey bars (modes 6/7) are D08J6D; first
  person (`fp_blend > 0`) is D08J6C and unchanged.

### Implementation

- New `climb_aim.inc` (controls adapter, after `swim.inc`):
  `climb_aim_begin` at the existing `0x800411b8` hook (ra `0x80042634`)
  computes the turn and the clamped direction; `climb_aim_arm` at the
  existing `0x800b42ec` composition hook (after `swim_aim_arm`) writes the
  chest, left shoulder, forearm, hand and head local matrices in the build's
  scratch (`0x1f800020..0x1f8000a0`), checked by actor, caller, part, depth
  and scratch slot. The forearm's local rotation for the solve comes from the
  previous build (the climb rests while armed); without it the shoulder only
  keeps its orientation.
- Head look-at: the existing `0x80097a44` hook replaces its direction
  argument (as on the ground). One-arm aim: `+0x124` holds the clamped
  direction (its own length kept) only for the gun shoulder's build in poses
  8/11 and is restored at the next joint's composition or the first restore
  point after the build (`0x800411b8`, `0x80055e80`, `0x800493a4`,
  `0x8003ade4`, `0x800455bc`). Its readers outside the build (`0x80057088`,
  `0x800a16d8`) never see the lent value.
- New guard `0x800978f8` (332 bytes; the one-arm aim reads `+0x124`). No new
  hook address, generated code, framework or codegen change; existing
  savestates load. Vanilla: behind `input_modernized()`.
- Debug `controls.climb_aim`; A/B `DNTTK_CLIMB_AIM=0`; developer limits
  `DNTTK_CLIMB_AIM_LIMITS=chest,head,gun_right,gun_left,pitch` (degrees).

### Verification

Private lab `recomp/analysis/d08j6b/` (local only: `probe.py`, `geo.py`,
`pose.py`, `sweep.py`, `van.py`; offscreen, dummy audio, port 9613, copies of
the D35E ladder, D35F chain and D35I1 climbing-wall fixtures; player saves,
cards and settings hash-checked unchanged after every run).

| Camera turns, idle and firing (`geo.py`) | Ladder | Chain | Wall (cowboy model) |
| --- | --- | --- | --- |
| Off hand from its holstered contact, max | 12 units (86 before the IK) | 6 | 12 |
| Firing gun arm (shoulder to hand) vs the clamped aim, max | 13 degrees | 13 | 13 |
| Same at 120 / 180 (ladder) | 12 / 12, 13 / 13 | - | - |

Before (A/B off) the firing arm pointed along the heading (chain: at the wall
Duke faces); now it points along the view, the chest and head turn and the off
hand stays on the chain (`ab-chain.png`, `ab-chain-zoom.png`; wall
`sweep-wall-grid.png`). `+0x124` lends and returns balance in every run.
D08J6A regression (`d08j6/cand.py` ladder W, chain S, wall S): draw, 6 view
shots, stow and climb, let go armed, identical to the D08J7C runs except that
the pistol is kept on landing, the accepted SIDEARM RETURN NEVER default.
Vanilla (`van.py`, `--vanilla`, raw pads): the original Circle draw and
Cross fire (5 rounds) on the ladder, no climb-aim build. Native:
`ttk-controls-test` 149 groups with LEVEL00, LEVEL01 and all levels (new
D08J6B group: chest turn, off-hand IK onto the contact with its grip in a
synthetic chain, clamps, `+0x124` lend and both returns, head look-at, stow
ramp, foreign actor/caller/slot, refusals, Vanilla), `ttk-input-test` under
Xvfb, `ttk-aim-test`, `ttk-near-test`.

Executable SHA-256 `b293c25036ef3124f1666c76c1a21a3fc531e80e11e71e2d7532a937354c9806`.

### Limits and decisions for the user

- Prototype: no Blender clips yet (one-arm draw/holster, aim idle, recoil,
  per-family grips). The draw and stow are the original upper-body clips on
  the gun arm.
- Idle keeps the original high ready; the arm aims only while firing, as on
  the ground. An aim idle (arm on the crosshair whenever the sidearm is out)
  is possible.
- Beyond the arc the arm stops at its limit while rounds still go to the
  crosshair (behind Duke). Alternatives: refuse shots beyond the arc, or let
  Duke swing out.
- The user saves (UI slots 12, 11, 8) and the slot 7 castle tower were not
  used; fixtures only. No enemies framed. Only Polymorph checked.
- The off-hand solve uses the previous build's forearm rotation (one update
  old; the climb is at rest while armed).

### D08J6B closeout

2026-11-04. User: "duke can clearly shoot in the right direction here and it
looks superb. that one is a winner", then "i love it all, this animation works perfectly fine". D08J6B is Done at executable
`b293c25036ef3124f1666c76c1a21a3fc531e80e11e71e2d7532a937354c9806`. The arc
limit (the arm stops, rounds still go to the crosshair) and the high-ready
idle stay as built; no Blender clips for now. First person is D08J6C.
