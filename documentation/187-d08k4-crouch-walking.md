# D08K4 - Modernized crouch walking (prototype)

Job: [D08K4](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-22; animation quality continues in D08K6). User request
(2026-10-22): "the options should be vanilla (roll) OR modern crouch walking
with modern crouch being the default", for precise combat movement.

## Behaviour

- **CROUCH: MODERN** (default; profile schema 39 `crouch`, `run.py --crouch`,
  OPTIONS > ADVANCED > MOVEMENT, live): hold crouch (LCtrl) and a direction to
  walk low, camera-relative, Duke facing the view (strafe and back up while
  aiming). **ORIGINAL**: crouch plus a direction rolls (181-184), as before.
  Vanilla: always the original (`DNTTK_CROUCH=original`).
- Release stands through crouch.inc where the 0x370 headroom allows.

## Implementation

- `recomp/src/ttk/crouch_walk.inc` (player update, after the stance request):
  while crouch.inc owns the stance (animation 178), crouch is held and the
  crouch-walk clips are loaded, it
  - tells `pc_input.cpp` (`crouch_walk_input_ready()`) to withhold every
    direction bit, so the original crouch handler never starts a roll;
  - faces the view and moves Duke 9 units a player update (walk is ~21)
    through the D08K2 crouch probe (hull 0x2c0, walls, objects, edges, steps),
    sliding along a blocked axis;
  - keeps channel 0 on animation 178 but sets its previous/current frames and
    ticks left each update to the crouch-walk clip of the dominant input axis
    (forward, backward, +x, -x), from a gait phase that advances with the
    distance moved (250 per cycle), so the planted feet follow the travel;
    entry, direction change and stop blend through the channel's own slerp;
  - holds the pelvis at the clip's height (330 + 8 cos(4 pi phase)), ramped
    from and back to the kneel's 282.
- Clips: `ttk_crouch_walk.py --host-pack` writes `ttk-anim.pack` in the build
  tree from the owned disc (CMake `ttk-anim-assets`), four extra clips (pack
  group 0xffff) of 8 frames with 1024-tick frames so the original stepper never
  advances them, no root commands. `anim_override.cpp` appends them and records
  their ranges in its buffer header (savestates keep them);
  `anim_extra_clip()` finds them. Every costume's legs share the same joint
  geometry by ROT id (DB00-09), so one clip set serves all levels.
- Arms and torso (channels 1-3) stay original: crouch idle, aim, fire.

## Evidence

Lab `recomp/analysis/d08k3-lab` (port 9404, private cards/savestate slot 5,
diagnostics input file): held LCtrl then LCtrl+W/D/S/A in third person:
animation stays 178, channel 0 plays the crouch-walk frames (5224-5255),
Duke moves about 5.3 units a frame at 60 Hz in each direction, pelvis 323-336;
releasing the direction returns to the kneel; releasing crouch stands (63).
Same run with `crouch=original`: animation 181 (the roll). Screenshots
`shots/cw1-sheet.png`. Tests: Python 177 OK (2 skipped; profile schema 39 and
`test_schema_38_gains_modern_crouch`), `ttk-input-test` (directions withheld
while the crouch walk owns), `ttk-controls-test`, `ttk-near-test`,
`ttk-aim-test` PASS.

## Revision 2 (2026-10-22): the user's glitch, walking pace, Duke3D feel

User: "the crouch walk is very glitched currently. i would prefer if crouch
speed was the same as walking pace ... I'd like the feel to be generally that
of how they did it in duke3d" (videos `research/screencaps/crouching-glitch.mp4`
and `crouching-duke3d.mp4`).

Causes found (lab, armed with the Desert Eagle, as in the video):

- **Standing legs with a weapon drawn.** With a weapon out, the original
  armed path leaves Duke's leg joints at identity after the channel blend, so
  he showed straight, standing legs while crouch walking, with occasional bent
  frames: the "glitch" in the video. The pelvis rotation was never taken from
  the channel either. The host now writes the crouch pose (the two frames and
  blend factor it gave channel 0, slerped like `0x8002a0c8`) onto the legs and
  pelvis records at `0x800411b8`, the last call before the model build.
- **Hang.** With the weapon drawn the original stepper (`0x80059db0`) advanced
  the host-driven channel past the extra clips (it loops within animation 178's
  own clip), into the command table; `0x80059004` then never ended its command
  loop. Host frames are now 30000 ticks (the host rewrites them every update),
  each clip starts with a sentinel frame, and 16 padding frames follow the
  clips.
- **Kneel / walk popping.** Stopping returned to the one-knee kneel and
  starting left it again. Held Modern crouch now keeps one low stance: still
  shows a squat idle (extra clip 5), every clip change blends over 6 updates.

Changes:

- Speed: the Modernized walking pace (`modern_pace_step(false)`); measured
  631 / 644 / 623 units/s crouch forward / back / strafe against 634 walking.
- Pose (Duke3D reference): pelvis 300, leaning 25 degrees forward, 240 stride,
  55% stance, stance width 0.95, 12 keyframes (planted-foot slide max 4.2, mean
  0.3; top 593). Frame events removed (none fire host-driven anyway).

Evidence: armed, 120 Hz and 60 Hz: crouch, walk, stop, strafe, release stay
low throughout and stand on release (`d08k3-lab/shots/d6-sheet.png`); armed
and unarmed squat identical (`k-cmp.png`); no hang. Python 177 OK (2 skipped),
`ttk-controls-test`, `ttk-input-test` PASS. Executable `e3b41dfa105e8dd17b47381ef5a5fa4a28e819b8666be073e24de6a0f184dbf7`.

## Revision 3 (2026-10-22): a captured crouch walk

User (video `crouch-glitch-new.mp4`): "it's closer, but still has a way to go",
offering a CC-BY model with animations ("Modern Army Soldier with +24 pro
Animations" by abdlilah ben, Sketchfab; files under `research/crouchwalk-anim`,
not committed). Revision 2 already kept Duke low; what remained was the motion
itself (a stiff, wide straddle from behind).

- `recomp/tools/local/extract_crouch_mocap.py` (Blender) samples the rig's
  "walk crouch" (28 frames, 24 fps, Mixamo skeleton) and writes
  `recomp/assets/anim/crouch-walk-mocap.json` (tracked, attribution inside):
  per frame the hips height, torso direction, ankle paths relative to the hips
  and foot rotations relative to standing, in leg lengths, axes mapped to the
  game's. No mesh or source bone data is kept. (The import carries a 0.01
  scale; bone matrices are orthonormalised before the deltas.)
- `ttk_crouch_walk.py` retargets it: resampled at even steps of planted-foot
  travel (the host advances the phase with distance), the ankles on the
  captured paths (leg-length scaled, recentred to Duke's stance width, the
  stride scaled to a 640-unit cycle; the capture's own cycle is 658), feet
  rolling as captured (the ankle lifted where Duke's longer foot would put the
  toe through the floor), the pelvis at 300 with the capture's bob and torso
  lean (about 31 degrees), solved with the same two-bone IK. 24 frames.
  Backward plays the cycle reversed; strafes turn the stride sideways with a
  380-unit cycle (quicker, smaller side steps; a 640 sideways stride read as a
  split).
- Host clips carry each frame's pelvis height in a root command (y); the
  runtime interpolates it (`crouch_frame_pelvis`) instead of a fitted cosine.
- Planted ankle slide 0.4-0.5 units a frame; top 577; speed 632-636 units/s
  (walk 646).
- Credit in README.md (CC BY 4.0 requires it), the asset and the extractor.

Evidence: lab armed at 120 Hz, crouch / walk / stop / strafe / release
(`d08k3-lab/shots/m1-sheet.png`, `m2-strafe.png`); `test_mocap_crouch_walk`;
Python 178 OK (2 skipped); `ttk-controls-test`, `ttk-input-test` PASS.
Executable `3548c78dc3bd6724d004998940870e62e9c011fbc91043c6010e88be295a6239`.

## Revision 4 (2026-10-22): static legs after crouching

User (video `Video_2026-10-22_10-02-58.mp4`): "dukes animation becomes static
with his legs after crouching ... only duke's animation ... not fixed upon the
release of ctrl ... in third person it is clearly broken."

Cause: animation 178 keeps its upper channels (2, 3) in step with the legs by
frame offset, so while channel 0 played a crouch-walk frame the game moved
them onto crouch-walk frames too. They then played leg joints, and their joint
masks (+0xc / +0x10, only ever OR-ed) kept the leg bits; from then on, in
every animation, the blend's fade (`0x80059324`) set joints in a channel's mask
but not in its frame to identity: straight, static legs while walking, until
the level reloaded. This was also the real reason the armed squat showed
standing legs in revision 2.

Fix (`crouch_channels_sanitize`, every player update and again before the
model build): an upper channel on a crouch-walk frame goes back to its own
clip's first frame, and upper channels never keep a leg joint (ROT 0-8, 21)
in their masks (no original upper track animates one). Channel 0 is handed
back to its animation's own clip whenever the crouch walk lets go.

Evidence (lab, armed, 120 Hz): walk, crouch walk, walk again: upper channels
stay on their own frames (513 / 518) during the crouch walk, masks keep no leg
bits, the legs animate afterwards (joint values change every sample;
`d08k3-lab/shots/q-cmp.png`). Suites pass. Executable `993c2c705d1f86fa27c1608ed92ba0574a1ade50097c909918b656b65e1c46c5`.

## Limits (prototype)

- No footstep sounds while crouch walking (the host sets the frames, so the
  original's frame events never fire).
- Diagonals use the dominant axis's clip (the feet slide on the cross part).
- Crouch speed is the walking pace (Shift does not run while crouched).
- Not yet judged by the user in play: look, speed, aiming while moving, first
  person, slopes, combat.
