# D08K6 - Crouch-walk animation quality, and how to animate Duke

Job: [D08K6](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-22, after revisions 2-4 the same day). User: "that's it!! i'm happy with this end to end. the anim is now fully complete. the technical implementation is now fully complete. this is totally playable in every way at this point. lock it in, close it out".
Builds on D08K1 ([note 184](184-d08k1-animation-pipeline.md): formats, tools,
runtime override), D08K3 ([186](186-d08k3-crouch-walk-loops.md): IK gait) and
D08K4 ([187](187-d08k4-crouch-walking.md): host-driven crouch walk).

User (2026-10-22): "go from this 'on the toilet' look to a real solid snake
type look", fix "a slight shake in duke's left arm when crouch walking", and
"document all of our discoveries in this job. I think this will make it
possible to add in future custom animations, and even custom models ... this
is real foundational groundwork". This note is written to be the reference for
that future work: part 1 is what changed in D08K6; part 2 is the reusable
knowledge, including the mistakes worth not repeating.

## Part 1 - What D08K6 changed

### Why D08K4 looked like "sitting on a toilet" (measured, not guessed)

`ttk_pose_study.py metrics` puts the source capture ("walk crouch" of the CC-BY
"Modern Army Soldier with +24 pro Animations") and Duke on one scale (leg
lengths from hip joint to ankle, degrees, game axes):

| Metric (cycle mean) | Capture | D08K4 Duke | D08K6 Duke (default) |
| --- | --- | --- | --- |
| Hip joints over the ankles | 0.69 | 0.47 | 0.63 |
| Thigh pitch from vertical | 56-60 | 83-85 (level) | 70-72 |
| Knee flexion | 81-88 | 106-109 | 94-96 |
| Ankles from the body centre | 0.02 / 0.19 | 0.28 / 0.28 | 0.11 / 0.10 |
| Pelvis pitch | 0-5 | 31 | 12 |
| Upper chest pitch | 61-68 (Spine2) | ~44 (pelvis 31 + kneel chest 13) | 48 (designed) |
| Arms | rifle up at eye line | the kneel's arms, hanging to the knees | fists up front / weapon pose kept |

Four things made the "sitting" read: the pelvis was forced to 300 (thighs
level, deepest knee bend), the feet were placed at 0.95x Duke's *idle* stance,
which is itself wide (about 4x the hip joints' spread, a straddle), the lean
was put in the pelvis (the capture's pelvis stays nearly level and bends at the
spine), and the upper body stayed on the one-knee kneel's channels, so the arms
hung down to the knees. The capture's stance yaw (hips 24 and shoulders 38
degrees, a rifle blade) was deliberately not copied: Duke faces the view and
aims forward. Its swing (hips +-7 yaw, +-3.5 roll) was.

### The new gait (`ttk_crouch_walk.py`, `mocap_frames`, parameters `TACTICAL`)

- Legs: hip joints at `hip_ratio` x leg length over the ankles (capture 0.69;
  default 0.62, a little lower so the crouch reads from the chase camera); the
  capture's ankle paths and heel-to-toe foot roll (scaled to Duke's leg, stride
  scaled to 640 units a cycle, resampled by planted-foot travel); feet at
  `width` x the hip joints' spread (1.5) with 6 degrees toe-out, knees solved
  forward over the toes. Strafes keep the feet under the hips
  (`STRAFE_FORWARD` 0.3 of the capture's forward stance offset; with the full
  offset the knees came forward and the strafe sat).
- Pelvis: 12 degrees forward plus the capture's yaw and roll swing.
- Chest (Duke's torso is ONE joint, ROT 9, covering the capture's three spine
  bones): 48 degrees forward in the world (default; 62 matches the capture's
  upper chest but hides the head behind the jetpack from the chase camera) with
  a 0.6x counter-swing against the hips. Head (ROT 16): level (0) in the world.
- Arms (when no weapon owns them): two-bone IK with the elbow hinge about local
  x bending negative (knees bend positive), fists 150 forward, 70 down, 40 in
  from each shoulder, elbows back/down/out, 12 units of counter-swing.
- Upper-body joints are written to **separate host clips 11-15** (same frame
  count and sentinel as legs clips 1-5). They are never placed on a channel,
  because a channel's joint mask only grows (see part 2) and leg/upper joint
  sets must never mix.

Results (Python, `test_tactical_crouch_posture`): every direction under the
0x2c0 hull (highest vertex 651; idle 637), thighs pitched down (mean thigh drop
72 forward/backward, 82 strafes; D08K4 about 20), chest lean over 45, upper
joints separate from leg commands and encodable.

### Runtime (`recomp/src/ttk/crouch_walk.inc`, `crouch_walk_upper`)

At `0x800411b8` (the player update's last call before the model build;
`crouch_walk_pose`), after the legs are written:

1. The upper frame matching each legs frame (`crouch_upper_frame`: legs clip c
   frame i -> upper clip c+10 frame i) is slerped with the same `t` as the legs.
2. Chest and head are always ours. An arm is ours when its channel still
   follows the legs (channel 1 / 2 animation == 178: unarmed, or a one-handed
   weapon's free left arm).
3. An arm a weapon pose owns keeps the world orientation the pose was authored
   for (an upright pelvis and the pose's own chest), so the gun does not tip with
   our lean: `A' = A (x) C (x) P'^-1 (x) C'^-1` (game quaternion order, Hamilton
   product, see part 2). Verified numerically (8e-16).
4. Entry and exit ramp the override over 6 updates (`crouch_upper_weight`),
   so standing up blends back instead of popping.

### The left-arm shake: cause and fix

Measured in the lab with a weapon drawn: channels 2 (left arm) and 3 (chest)
are on animation 178, which keeps its upper channels in step with channel 0 by
frame offset. Every update the original stepper pulled them toward crouch-walk
frames and `crouch_channels_sanitize` (D08K4 revision 4) put them back on their
clip's first frame; the blend in between left the chest and left arm 2-6
degrees off every few updates, and up to 26 degrees on the forearm when channel
3 landed on a crouch-walk frame. That was the shake (unarmed it was 1 degree, so
it was mostly seen armed). Because the upper joints are now written last, after
every blend, the churn never reaches the model. Rendered matrices (relative to
the pelvis) over 60 frames: left arm and chest change only at the ramp-in; the
weapon arm moves 2-5 degrees per update, which is the compensation holding the
gun's world direction while the hips swing.

### Revision 2 (2026-10-22): arms straight down after crouching

User: "duke's arms are now pointing straight down after crouch". Reproduced
armed in the lab (`s11.py`): after release every arm joint stayed at identity
in every later animation. Two causes, both fixed in `crouch_walk.inc`:

1. **Channel masks.** Armed, animation 178's frame-offset sync walked the left
   arm and chest channels onto neighbouring clips (the right arm's and the
   chest's), so channel 2's mask grew from `0x00c00` to `0x1fe00` (chest, head,
   right arm) and channel 3's to `0x11fe00`. Masks only grow, so every later
   animation faded both arms to identity. `crouch_channels_sanitize` now keeps
   each upper channel's masks to the joints its original clips animate, from a
   scan of every clip in DB00-09: channel 1 right arm 13-15, channel 2 left arm
   10-12, channel 3 9, 16-18, 20, 22, 23 (it used to strip only leg joints).
   This also repairs a save already in the broken state.
2. **Compounding compensation.** The weapon-arm compensation read the arm and
   chest from the joint records. While Duke holds still the weapon channel's
   frame does not change (48/48) and the records can still hold our previous
   update's values, so the correction compounded and the right forearm and hand
   went to identity during the still crouch. The weapon arm's pose now comes from
   its own channel frames (`crouch_channel_pose`, the blend `0x80059004` makes)
   and the reference chest from channel 3's frame.

Evidence (armed and unarmed, 60 Hz): masks stay at their original values
through crouch, crouch walk, a 2-second still hold and release; after release
the arms match their pre-crouch values (armed shoulder/forearm 56.6/114.4
degrees, unarmed 17.8/33.8); the still crouch holds the pistol arm at 90.5/114.4
with no drift. Shots `shots/p-arm.jpg`: the pistol is held up by the shoulder,
as standing, through crouch, still, release and walking. The earlier
observation that the pistol "hangs at the thigh" was this bug, not the pose.
Executable `6ca0122ee58dce894263394fa0d1ba7623763e8bba5aa36f6967bf9c6d77dbcf`.

Lesson for part 2: never derive a correction from a joint record you also
write; recompute from the channel's frames, because the game does not rewrite
every record every update.

### Revision 3 (2026-10-22): shooting crouched pointed the gun up

User: "when duke shoots crouched, he shoots directly up ... we did some great
work with shoot direction being corrected during swimming so can you use that".
Cause, from the build code documented for swimming (notes 118, 119): firing
sets `+0x224` bit `0x100`, and the aim build `0x80097c04` replaces the chest's
world rotation with the `0x80097a44` look-at of the view (D07A supplies the
direction); the arms and head hang from that chest. Our weapon-arm
compensation and designed upper locals assume the chest leaned 48 degrees, so
under the aimed chest the gun arm rotated back and pointed 49-58 degrees up.

Fix (`crouch_walk_upper`): while bit `0x100` is set, the chest, head and arms
get their own channels' pose (computed fresh from the frames with
`crouch_channel_pose`, never from the records), no lean and no compensation;
the original aim then does exactly what it does standing. After the shot the
lean ramps back in over 6 updates. The legs keep the crouch walk.

Evidence (lab `s12.py`, pistol, third person; gun elevation = shoulder to hand
above horizontal): standing fire -2 degrees; crouched still fire, before 49 to
58, after -9 to 0; firing while crouch walking back and strafing -1 to -7, the
muzzle flash forward and the torso squared to the view, the lean back after
the shot, release stands (`shots/f4.jpg`). Suites: `ttk-input-test`,
`ttk-controls-test` (LEVEL00 fixture `analysis/pc-input/d07-final-camera/`,
LEVEL01), `ttk-aim-test`, `ttk-near-test`, `test_ttk_anim` pass. Executable
`cd48d3e3d7cd5f0b35f3364b2179d0a4209b1ca3ac79b67ec6354312f92db900`.

The shot itself was never wrong: D07 view aim sends it from the muzzle to the
crosshair target; only the arm pose was.

### Revision 4 (2026-10-22): variant C, head up, no upright snap when firing

User: "i liked C ... in our game he is constantly looking down ... when duke
starts shooting, his torso doesnt jerk upwards like bolt upright ... a slight
raise, if any raise at all".

- **Default is variant C** (`TACTICAL`: hip ratio 0.56, chest 62, head 5;
  strafes as before): hip joints 0.57 leg lengths over the ankles, knees
  102-104 degrees, highest vertex 580. Pack
  `6df281683f8d47769569b299429d3b855b8f642a196f3a9584b72483758becce`.
- **Looking down: a bug, not the design.** The upper-body loop treated every
  ROT id from 10 up as an arm (`j>=10`), so the head (16) counted as the gun
  arm: with a weapon drawn it was never written and kept the kneel's head under
  our 62-degree chest, 56-62 degrees down. Arms are now 10-15. Measured in game
  (forward = row 2 of the actor's transposed matrix): head 5 degrees down
  crouched, armed and unarmed, as designed.
- **Firing stays hunched.** Instead of handing the chest to the original aim
  (upright look-at), the `0x80097a44` hook gives the chest's look-at (scratchpad
  `0x1f800040`, depth 2) the view direction tilted down by `crouch_fire_lean`
  (50 degrees) about its right axis (`crouch_fire_direction`), and both
  shoulders are turned back by the same tilt, `q' = q (x) (sin(-lean/2), 0, 0,
  cos(lean/2))`, so the arms and gun keep the original aim, as the swim fire
  pose does (D08O2A). The head keeps its own look-at. The lean weight is not
  reset by a shot, so the chest returns from 56 to 62 degrees, not from upright.
  Measured: chest 62 degrees crouched, 56 firing (the view's pitch is the small
  raise); gun -8 to +1 degrees; head 6 firing.

Suites: Python 179 OK (2 skipped; the posture guard is now a mean thigh drop
over 40, C is about 48, D08K4 about 20), `ttk-input-test`, `ttk-near-test`,
`ttk-aim-test`, `ttk-controls-test` (LEVEL00 + LEVEL01). Executable
`789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03`.

### Variants and how to try them

`ttk_crouch_walk.py --host-pack OUT --set key=value` builds a variant pack;
`DNTTK_ANIM_PACK=/abs/path/OUT python3 recomp/tools/local/run.py` plays it
(diagnostic; it replaces the built-in pack). Built locally in
`recomp/analysis/d08k6/packs/` (not committed):

| Pack | Settings | Read |
| --- | --- | --- |
| old | D08K4 tool (git HEAD before this job) | the straddle being fixed |
| a | hip 0.69, chest 62, head 5 | capture-exact height; head hidden from behind |
| b | hip 0.62, chest 62 | lower; head hidden |
| c | hip 0.56, chest 62 | lowest; deepest knees |
| d | hip 0.62, chest 48, head 0 | lower, hunched, head visible over the jetpack |
| default (revision 4) | variant C with the strafe fix | the user's choice |

### Evidence

- Lab `recomp/analysis/d08k3-lab` (offscreen, port 9404, private cards, slot 5),
  scripts `s6`-`s10.py`; study `recomp/analysis/d08k6/` (renders `r/`, sprite
  sheets `art/`, packs `packs/`).
- In game (armed, third person): crouch 178 held, pelvis 382 over the ankles
  with variant a, knees 84-87 degrees still, ankles 102 apart (D08K4: 270);
  crouch walk back/strafe, release stands (63). Shots `shots/g-*.png`
  (variants), `shots/h-def-*`, `h-un-*` (default, armed and unarmed).
- Python: `test_ttk_anim` 14 OK (new `test_tactical_crouch_posture`; host pack
  now 11 entries: 4 walks, idle, 5 upper clips, padding).
- Executable `dda1d28f2626ffa5b0464d0adcc83bb99483dc3879ef9e1009069d342382a3be`,
  pack `8fffb73ccf9f5055c0812b175f6a3082084f129176cdb5acf286dfa2377841ad`.

### Limits

- Weapon poses are kept as authored (the pistol held up by the shoulder, as
  standing); a different crouched carry would need a per-weapon choice of idle
  versus fire/reload upper animations.
- First person: the eye follows the neck joint, so the lean moves it forward;
  the FP view was seen working in the lab, not judged for comfort.
- Not judged by the user in play; footstep sounds still absent; diagonals use
  the dominant clip.

## Part 2 - Reference: animating (and eventually remodelling) Duke

### The data, where it lives, what is proven

| Piece | Where | Status |
| --- | --- | --- |
| Skeleton (HIE) | `DBnnHIE.BIN`, hierarchy 1 = Duke, 19 records, 0x28 bytes each | decoded, verified live (note 184) |
| Meshes (MOD) | `DBnnMOD.BIN`, one rigid mesh per joint, textured quads/tris, VRAM pages from `DBnnA/B.RAW` | decoded and rendered offline; **replacement never tried** |
| Animations (ANM/FRM/ROT) | per level group; 9,481 clips round-trip byte for byte | proven: decode, edit, re-encode, load at run time |
| Animation -> 4 clips | executable `0x800c2d64` (336 animations), flags `0x800c2824` | decoded; new animations need host mapping |
| Runtime override | `anim_override.cpp`, `ttk-anim.pack` | proven (replace a clip, or add extra clips read by the host) |

Duke's records (record: ROT id, part): 0:0 pelvis (root), 1:9 chest, 2-4:10-12
left shoulder/forearm/hand, 5-7:13-15 right, 8:20 chest attachment (jetpack),
9:16 head, 10-13:1-4 left hip/knee/ankle/toe, 14-17:5-8 right, 18:21
attachment on a leg. Offsets (parent space, y down): thigh 227, shin 258,
upper arm about 133, forearm about 128; hip joints 47 below and +-31-36 beside
the pelvis; chest pivot just above the pelvis; shoulders 163 and head 186 above
the chest pivot. Standing pelvis 561 over the floor, ankles 36; **Duke's idle
stands on bent knees (about 52 degrees)**, so a crouch lowers him less than a
straight-legged rig would suggest. All costumes in DB00-09 share these joint
offsets by ROT id.

### Coordinate and quaternion conventions (get these right first)

- Game axes: x right of the body, **y down**, z forward. A pose's world matrix
  for a joint is `S_child = S_parent * M(q)`, `t_child = S_parent * offset + t_parent`.
- `M(q)` (`0x8002a5ac`, `ttk_anim.quat_matrix`) is the **transpose** of the
  textbook matrix of (x, y, z, w). So `M(a) M(b) = M(b (x) a)` with textbook
  Hamilton products, and composing in matrix order means reversing quaternion
  order. The runtime compensation formula above follows from this; check any
  new formula numerically against `quat_matrix` before porting to C++.
- `ttk_crouch_walk.game_q(R)` returns the game quaternion whose `M` is a plain
  rotation matrix `R` (world = R @ local). Work in plain matrices, convert last.
- Q12 fixed point; quaternion fields signed 14/14/14/13 bits; the encoder keeps
  w >= 0 and scales by 4095 when w would overflow.
- Blender (glTF Mixamo rigs) to game: x = -X, y = -Z, z = -Y; Blender's
  armature import carries a 0.01 scale, so orthonormalise bone matrices (SVD).
  The source rig's actions start at frame 0.8, are in place (no root motion),
  and its rifle poses yaw the hips 11-40 degrees: measure facing from the toes,
  not the hip line.

### Channels: who owns which joint

- Four channels per actor (player `+0x60/+0x74/+0x88/+0x9c`, 0x14 bytes each:
  animation, ticks left, previous frame, current frame, state, reverse, two
  joint masks): 0 legs + root (ROT 0-8, 21), 1 right arm (13-15), 2 left arm
  (10-12), 3 chest/head/jetpack (9, 16, 20).
- Weapon "upper" animations fill only some channels: many are right arm only
  (pistol 5, 8, ...), two-handed ones fill 1-3 (e.g. 1, 2, 12, 13). A channel
  not used by the weapon follows the legs' animation.
- **Joint masks only grow** (OR-ed). If a channel ever plays a frame with
  joints outside its set, it keeps those bits, and the blend's fade
  (`0x80059324`) later forces them to identity in every animation: the D08K4
  "static legs" bug and the D08K6 "arms straight down" bug. Never put mixed
  joint sets on a channel; the host trims player channels 1-3 to their
  original sets every update.
- The game does not rewrite every joint record every update (a channel on an
  unchanging frame can be skipped), so host code must not read back a record
  it wrote earlier as if it were the game's value.
- Animation 178 keeps its upper channels in step with channel 0 by frame
  offset; host-driven channel-0 frames therefore drag the upper channels with
  them (the source of the left-arm shake).
- The original armed path resets leg joints after the blend; the pelvis
  rotation is never taken from a channel. Anything the host must guarantee is
  written at `0x800411b8` (`ra 0x80042634`, `a0` = player), after all of that.

### Recipe: a new custom animation for Duke

1. Decide the owner. Replacing an existing clip everywhere: a normal pack entry
   (group, clip, original digest), loaded by `anim_override.cpp`. A new motion
   the host drives: extra clips (group 0xffff, your own ids), read with
   `ttk::anim_extra_clip`, frames set by the host (the D08K4 pattern), long
   frame ticks (30000) so the original stepper never advances them, a leading
   sentinel frame and a padding clip after the last one.
2. Author: in Blender (`ttk_anim.py blender-export` -> edit ->
   `blender-import`), procedurally/IK (`ttk_crouch_walk.Leg`/`Arm`), or
   retarget a capture (extract a skeleton-independent description, as
   `extract_crouch_mocap.py` does, then solve on Duke). Retarget by
   proportion (leg-length ratios), not by absolute heights, and compare with
   `ttk_pose_study.py metrics`; Duke's proportions (short thigh, bent idle
   knees, one-joint torso, big jetpack) change what reads well.
3. Keep leg/root joints and upper joints in separate clips.
4. Root: the first frame's root y is the pelvis offset (relative to -561), later
   frames add to it; x/z are travel per frame. Host clips here carry the
   absolute pelvis height in root y and the host applies it.
5. Check: every command encodes (`t.encode_command`), highest vertex under the
   relevant hull (`pose_top`), planted-foot slide (`metrics` in
   `ttk_crouch_walk.py`), render (`ttk_anim.py render`, `ttk_pose_study.py
   flipbook`), then the lab.
6. Licence: only motion we may use (CC-BY with credit in README, the asset and
   the extractor, or our own); commit derived descriptions, never retail data
   or the source files.

### Toward custom models (what we know, what is missing)

Known: the MOD layout (vertex count +6, bbox +8, optional block +0xc, 8-byte
vertices at [+0x10], primitive groups at [+0x14]: GPU code, count, records;
0x2c/0x24 textured quads/tris are 16 bytes with vertex indices, UV+CLUT,
UV+tpage), that each joint has exactly one rigid mesh (no skinning), that the
loader replaces the record's MOD index with a model pointer, and where the
texture pages land in VRAM. Not yet done: writing a MOD, fitting new geometry
and textures into the level's VRAM pages/CLUTs, runtime replacement of a model
(the pack loader only handles animation tables), and checking the level's
memory budget. A future job should start with a round trip of Duke's own
meshes through a MOD writer, as D08K1 did for animation.

### Lab lessons (save the next session hours)

- The lab profile in `d08k3-lab` defaults to **first person**: set `VIEW=third`
  for body work.
- Slot 5 (the street) has a pig cop who shoots Duke after about a second: his
  hit reactions (animations 212/215) look like crouch bugs. Walk backward or
  sample the first second; there is no god mode.
- Joint **records** can be read mid-update (one read returned identity); judge
  what is drawn from the matrix array `[player+0x3c]` (0x20 bytes per record:
  3x3 Q12 shorts, translation int32 at +0x14), which the game rebuilds only
  every few presented frames.
- From the chase camera the torso lean is mostly hidden by the jetpack; the
  hunch reads as the head sinking. Judge side views offline, the chase view in
  game.
- Blender renders: aim the camera with a track-to constraint, not Euler guesses.
- The actor matrix array `[player+0x3c]` holds transposed matrices: a joint's
  forward axis is row 2, not column 2 (symmetric terms such as the up-axis
  lean read the same either way, which hides the mistake).
- Firing in an offscreen lab: the scripted input word `mouse1` holds the left
  button (`DNTTK_TEST_INPUT` only, D08K6); `act=fire` is a one-shot command,
  not a held trigger. Writing health at `+0x32` mid-run can put Duke straight
  into a hit/death animation; keep runs short instead.
- `ttk-controls-test` needs a LEVEL00 fixture that matches the current guards;
  `analysis/pc-input/d07-final-camera/level00-guard-fixture.bin` does
  (`analysis/d08p-turret/` does not).
- Any host pose that the original aim build can override (bit `0x100`: the
  chest becomes a view look-at) must stand down while it is set.

### Tools

| Tool | Purpose |
| --- | --- |
| `ttk_anim.py` | decode, render, round trip, Blender bridge, packs (note 184) |
| `ttk_crouch_walk.py` | crouch-walk clips; `--host-pack`, `--set` variants |
| `extract_crouch_mocap.py` | capture -> tracked motion description |
| `ttk_pose_study.py` | `sample`, `metrics`, `flipbook`, `source-flipbook` |
