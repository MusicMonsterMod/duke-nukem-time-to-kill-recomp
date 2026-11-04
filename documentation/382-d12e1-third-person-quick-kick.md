# D12E1 - Third-person quick kick (legs layer)

2026-11-04. **Done (accepted by the user 2026-11-04, revision 3: "LOCK IT IN AND CLOSE IT OUT ... accepted").** Implements the locked design in
[note 381](381-d12e-third-person-kick-research.md) section 6. Candidate
executable `674bf8c7ff7ecf130e5fe6acffd29ded6c45598b0dbdc2123c5527416ca2d02a` (revision 1); revision 2 `eea4298d3c9c5b0356b3b06378b4ebe829331006a1b745cbbdf71181213a83fa`.
Labels: S source/native tests, L isolated runtime observation (private
fixture copies, offscreen), U user judgement (pending).

## Revision 3 (2026-11-04): the left leg

User on revision 2: "10000000% that's our kick big boy. but left leg. LEFT
LEG!". The clip is solved for the right leg as measured, then mirrored onto
the left (`KICK_LEG = 'left'` in `ttk_quick_kick.py`: quaternion
`(x, -y, -z, w)`, legs swapped; Duke's leg rig is symmetric within a few
units). The hit point's side offset follows (-10). Timing, hips, jumps and
effects are unchanged. Ground, held and jump kicks re-measured offscreen; `ttk-controls-test` PASS.
Executable `86a5e664e2b495bbdc5b5ff50e91182eba326505d5602705d08ba0b8375f72d8`.

## Revision 2 (2026-11-04): one leg, MMA roundhouse, jumps

User after the first candidate: "let's make it always the same leg, and let's
try and go for more of the MMA kick which, well i'm imagining like a faster
version of this" (a reference roundhouse motion in `research/mma-kick`, local
only, never committed); "the version you have right now is functional but the
animation is totally off"; "i want this to work while in a jump too just like
in first person mode".

- **Clip:** one right-leg clip (51; clip 52 and the alternation are gone).
  `ttk_quick_kick.py --measure REF.fbx` (Blender) samples the reference at 16
  frames (11-51 of 65 at 30 fps) and prints the `KEYS` table now in the tool:
  pelvis rotation from a square stance, the ankle and knee from the right hip
  joint, foot rotation, support-foot pivot, mapped to Duke's axes and scaled
  to his leg (485 units, 5.45 per reference unit). The table is our key sheet;
  no reference file, mesh or texture is in the repository. The reference's
  first hip move to the strike (16 frames, 0.53 s) plays in 150 ms; its
  retract (0.47 s) in about 120 ms.
- **Retarget:** the hips turn at `hip_turn` 0.7 of the reference (side-on and
  tipped: about 63 degrees of turn at the strike), the ankle by two-bone IK
  with the knee turned toward the reference knee (it comes across the body,
  the shin whips out to about pelvis height, about 460 ahead), the support foot
  planted and pivoting at `support_turn` 0.45 (up to about 50 degrees). The
  first and last keys ease from and back to Duke's standing pose (the reference
  starts bladed). Keys are interpolated linearly (dense reference samples).
- **Chest:** follows the hips' turn by 0.35 (`tp_chest_follow`); firing still
  aims the chest exactly through the original look-at.
- **Jumps:** allowed in mode 9 and in the standing jump 96/97 (mode 0 after a
  jump), excluding jetpack, push, mantle, ladder and climb states; the support
  leg keeps the jump's pose, no forward step; a kick running into the landing
  continues through the landing poses.
- **Evidence (L):** ground Q to the kick about 60 ms, held Q 10 kicks in 3.4 s;
  standing jump and forward jump kicks run airborne for the whole kick (they
  were refused before the air rule); a spawned pig cop down in three kicks;
  `ttk-controls-test` PASS; stills in game (`shots/sheet-r2.png`) and key sheets
  (`sheets2/`, side/front/quarter/behind). Needs the user's look and feel
  judgement.

## What the player gets (revision 1, superseded where revision 2 differs)

Modernized, third person, **ADVANCED > COMBAT > KICK: QUICK** (default):

- Q kicks with any weapon or holstered, standing, walking or running on the
  ground. With the Boot selected a held attack kicks too; an original Boot
  request (Action at idle) no longer starts 112-115 and does nothing, as in the
  eye view.
- Only the pelvis and legs play the kick. Arms, chest, head and weapon stay on
  the original animation and aim; firing works throughout.
- Chains: a Q during a kick queues the next one, which starts at the regain
  point (350 ms) with the other leg; a held Q repeats.
- On a real hit (actor, breakable object or a wall within reach): the D12C
  thud with a pitch step that never repeats twice in a row (+/-4 steps, about
  +/-5%), the original spark burst at the contact, a 50 ms hold of the strike
  pose taken from the recovery, and a camera punch (14 units along the kick
  over 67 ms, then a decaying 6 unit side shake, 140 ms in all). A whiff only
  whooshes.
- Travel is capped at the walking pace for 0-350 ms; a 35 unit step on the
  snap through the standing hull probe.

**KICK: ORIGINAL** restores the holstered Boot's full-body 112-115 exactly.
First person keeps D12A. Vanilla never reaches any of it
(`DNTTK_TP_KICK=original`, `input_modernized()` gate).

## Implementation

| Piece | Where |
| --- | --- |
| Clip authoring: clips 51 (right) and 52 (mirror), 9 keys at 0/67/117/150/200/283/350/400/450 ms | `recomp/tools/local/ttk_quick_kick.py`, added to `ttk-anim.pack` by `ttk_crouch_walk.py write_host_pack` (CMake dependency added) |
| Runtime | `recomp/src/ttk/tp_kick.inc` (included after `kick.inc`) |
| Q | `shortcuts.inc`: `kick_request(true) || tp_kick_request(true)`; a refused request falls through to the original Boot path unchanged |
| Boot request conversion | `tp_kick_convert` at `0x800493a4` after `kick_convert` |
| Update (sound, step, hits, effects, chains) | `tp_kick_update` at `0x80058120` (ra `0x80041c44`) after `kick_update` |
| Pose | `tp_kick_pose` at `0x800411b8` (ra `0x80042634`) after `crouch_walk_pose` |
| Pace cap | `tp_kick_pace` in the walk/run handler block (`0x80053404`/`0x80053500`) |
| Camera punch | `tp_kick_camera_offset` added to the orbit target in `orbit_constraint` (the constraint and collision solve it; the view direction and aim never move) |
| Impact pitch | `impact_pitch_offset` in `kick.inc` `kick_impact` (0 for every other caller) |
| Setting | `game_menu.cpp` COMBAT heading, `player_profiles.py` schema 66 `third_person_kick` (`quick`/`original`), `run.py --third-person-kick`, `DNTTK_TP_KICK` |

No new hook address, no guest code change, no codegen or framework change.

Clip (`ttk_quick_kick.py`): keys from 115's legs channel (clip 71): rest key 0,
coil key 2, chamber key 4, strike key 5 with 6 degrees of extra hip flex on the
snap, strike exact, a 4 degree knee give at the end of the hold, a retract that
re-chambers the knee (65% of the chamber's bend, the foot trailing the knee),
115's key 7 for the landing. The support foot is solved with two-bone IK onto
the standing idle 63's ankle, turning out up to 14 degrees; the kicking foot is
planted the same way at rest and lands 24 units ahead. The pelvis keeps 115's
rotation (the hips twist and lean into the kick); its height is never moved,
so the clip carries no root command. Toes keep the live pose. The left kick is
the right one reflected through x = 0 (quaternion `(x, -y, -z, w)`, legs
swapped). Sheets: `--render OUT_DIR` (side, front, quarter; local
`recomp/analysis/d12e1/sheets/`).

Runtime pose: the clip pose at clip time is slerped between keys (the snap
key pair eases out, the rest smoothstep), blended in from the displayed pose
over 50 ms (a chained kick starts from the pose shown last) and back to the
live locomotion from 350 to 450 ms. Joints 0-3 and 5-7 are written; the chest
record is corrected so its world rotation is unchanged under the new pelvis
(`chest' = chest (x) pelvis_live (x) pelvis_new^-1`). A cancel (leaving the
eligible state, a view switch) fades the last pose out over 80 ms. Redraws
interpolate Duke's joint matrices as for every animation (D17AY), so no
`RenderState` field is needed.

Hit: every player update of 133-333 ms (host time) calls the D12A sphere
(`0x800a979c`, radius 96, type 10, `delta * 5 * (4, or 16 on steroids)`,
attacker Duke) at the posed ankle (445 ahead of the pelvis, 60 above, 90 to the
kicking side), pulled to where the view ray from the hip meets a surface or an
actor within 480 horizontally. Walls use the D12C reach test. The first
contact triggers the impact, the spark `0x8006d594(6, point, 0xff)` (the
dynamite fuse and other original callers use the same call: three particles),
the hold and the punch.

Developer only: `DNTTK_TP_KICK_FREEZE=0..449` holds every kick at that time.
Debug: `ttk_input` > controls > fp > `tp_kick`.

## Evidence

| Check | Result | Label |
| --- | --- | --- |
| `test_player_profiles.py` | 123 OK (new `ThirdPersonKickOptionTest`: terminal Q, invalid values, schema 65 -> 66 migration, CLI save, Vanilla env `original`, in-game menu sidecar) | S |
| `ttk-controls-test` (PS-X EXE, LEVEL00 fixture, LEVEL01, all levels) | PASS; QUICK converts a third-person Boot request to idle 63 with no kick (suppressed), ORIGINAL keeps 112 and the D12C original-sphere cases | S |
| `ttk-input-test` | PASS | S |
| Latency, length (apartment fixture copy, pistol drawn, 60 Hz) | Q to the kick 37-75 ms; active about 0.46 s; one whoosh and about 6 sphere calls per kick | L |
| Held Q 3 s | 10 kicks, about 0.35 s each (chained at the regain point), legs alternating | L |
| 120 / 180 Hz | same counts and times (host-time clock) | L |
| Walking with Q | pace cap applied (8 capped updates), steps 11, 2 refused at a wall | L |
| Door in front (apartment) | struck every kick: thud, spark, hold | L |
| Pig cop type 57 (first street, spawned, god mode) | first contact 135-155 ms after the kick started; 3 kicks and it went down (the D12A count); spark and hold recorded | L |
| ORIGINAL, Boot holstered | 114 played, as on the pre-change build (both builds drop the same scripted taps that land inside the previous 1 s kick) | L |
| Vanilla route `vanilla_regression.py --mode vanilla --state-probe` | completed, event frame counts as the D08G8 baseline | L |
| Stills of each key in game (frozen) | chamber, snap and strike on the legs; pistol and torso stay on the aim | L |
| Player profiles and saves | hash-checked unchanged by every probe | L |

Probes and stills are local: `recomp/analysis/d12e1/` (`probe.py`,
`enemy2.py`, `shots/`, `sheets/`).

## Limits

- Not yet judged by the user (U): the look at 60/120/180 Hz on the real display,
  the strength of the punch and hold, the forward step (35 units, so a 3 s
  spam advances Duke about 315 units), the spark's look.
- From the default behind camera the extended foot is partly hidden by Duke's
  own body at the strike; the side and front sheets show the clip in full.
- The scripted test input drops the first key after a load (any key first
  fixes it); not seen with real input, but not tested with a real keyboard.
- No native pose-level test: the harness stubs the extra clips with dummy
  frames; pose writes are covered by the runtime probe and stills.
- Frame cost not measured separately (a few slerps per player update).
- Running kicks slow Duke to a walk for 350 ms by design; a running-kick clip
  would be the fix if that feels wrong.
- Knockback, finisher, enemy shake and rumble are D12E2.
