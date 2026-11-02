# D35D1 - First-person push/pull: a steady grip

2026-11-02. Done: revision 2 accepted by the user ("absolutely solid work!
fantastic, and accepted"); closeout, commit and push authorized. Labels: U user report, S source/native tests,
L isolated runtime observations.

## Problem

U: in first person, pushing the slot-12 alley dumpster shows Duke's arms
going through the third-person push animation, where he turns around and
shoves with his back. The user wants a stable first-person grab: hands on the
object in front of him, no turning or twisting, for grab, push, pull and
release.

L: the presentation owner is the D36H5 contact rig (`PerspectiveAction::Push`,
`LimbOwner::Interaction`); see [note 306](306-d36h5-interaction-arm-coverage.md).
Its contact targets were the native hands. Sampled in Duke's body frame
(gameplay facing +0x1c, position +4..+12), pull 119 and push 120 move the
hands by several hundred units: during push the hands cross to one side and
the native shoulders swap sides, so the rig's virtual shoulders and solved
arms swept across the view. Grab 121 at phase 3 is a static hold.

## Implementation

`push_grip` in `recomp/src/ttk/first_person.inc` supplies a first-person grip
for the Push owner only:

- Hand targets sit on the held object's near face: its box (`push_bounds`,
  the same rotated box used by push collision) gives the face distance along
  Duke's facing, with an 85-unit inset that keeps the glove out of the retail
  dumpster face (inset 0 sinks it; checked in captures). Height is chest level
  (150 units above Duke's position), kept 60 below the object's top; hands
  sit 150 either side, kept inside the face's width. Forward is clamped to
  180..360.
- Rotations come from the native grab-121 hold, expressed in the body frame,
  with both sides averaged into one pose mirrored for the right chain (local
  X is the mirror axis, checked against idle 63). Hands pitch fingers 55
  degrees up so the knuckles rest on the face.
- Pull 119 and push 120 use the grip fully. Grab 121 blends from native by how
  near its hands are to the native hold (smoothstep over 300 units), so the
  reach-in and the let-go stay continuous.
- The D08T3 release replays 121 from its first keyframe; entry and release
  share the same keyframes (4296 then 4297) in guest memory, so a release
  looked like grip, drop, grip, let go. `push_letgo` (set by
  `push_manual_release`, cleared when 121 ends or a new hold starts) keeps the
  grip until the native let-go phase (+0x6a = 1), then the distance blend lets
  go. It is the only host state and is carried in the render-replay snapshot.
- The contact rig then solves both arms from its eye-relative virtual
  shoulders to these hands. Bone lengths stay the skeleton's; each segment's
  source rotation carries its bone axis (measured in its own frame), so the
  rig's swing is correct for gripped rotations. Hands now draw with the grip
  matrix; outside the grip they keep native matrices as before.

Unchanged: animation, movement, collision, push physics, the eye, third
person (still the native turn-and-shove), reflections, other contact owners
and Vanilla. `DNTTK_FP_PUSH_GRIP=native` is a one-launch comparison that
restores the D35D/D36H5 native-hand presentation; it is not saved.

## Revision 2: vibration

U: "functionally, it works perfectly. but what is happening is the arms appear
to vibrate". L: render replay interpolates the camera, objects and Duke's
joint matrices between 30 Hz game ticks, but host code reading gameplay state
at draw time sees the last tick. The grip was anchored to Duke's position
(+4..+12); an in-draw trace showed redraw eyes up to 29 units ahead of it while
pushing, so the hands stepped against the smoothly moving view. While turning,
D14H draws Duke through the body camera (interpolated yaw) while the object
uses the late mouse camera, so world-fixed hands also slipped.

Fix: the grip anchors horizontally to the eye (the push eye sits straight
over Duke's position, so live frames are unchanged), and in late redraws its
positions and rows are mapped by B^T L (body camera B, late camera L, from the
new `replay_late_rotation` in `frame_replay.cpp`), so the body camera projects
them where the late camera sees the world. Trace after the fix: the hand's
screen position is constant across push redraws as the eye advances; in a
still hold it moves with a fixed world point to about 0.2 px (live frames and
redraws differ globally by a small pitch amount that also moves the world).
Screen-recording jerk metrics at 60 fps alias the 180 Hz stream and were not
used as evidence. Executable
`6872f692f956bd70fda32c29c9fc099246cdbb3e878f0c5d7ee09489f3946bab`.

## Verification

S: player build; native controls (new D35D1 group), near, aim and input
suites pass. The D35D1 group uses the actual draw hook in both costume joint
orders and two object heights: hand placement on the face, top clamp, width,
mirrored hand rows, connected elbow/wrist chains, identical output while the
native chains swing (steadiness), 121 hold blend versus native far away, the
native comparison, third person untouched and native matrices unchanged.

L: private copies only, Xvfb/OpenGL, dummy audio, debug-port driver
(`recomp/analysis/d35d1/probe.py`), player save/card/profile hashes checked
before and after every run (unchanged).

- Player's UI slot 12 (LEVEL00 alley dumpster), 120 Hz: native vs grip
  comparison through grab, push, hold, pull and release. Native hands sweep
  across and out of view; the grip holds both hands on the face throughout.
  Continuous recordings checked frame by frame for grab entry and release
  (release dip fixed by `push_letgo`).
- Slot 12 at 180 Hz with a private mid-grip save/load: steady grip; replay
  reported 3 workers, 0 failed jobs, 0 deaths. Loading ends the grip as in D35D.
- Third person (VIEW third, 120 Hz): native turn-and-shove animation intact.
- Archived D35D fixtures (`recomp/analysis/d35d/cards`, UI slots 9-11, LEVEL06
  Duke-symbol blocks, medieval costume) at 120 and 180 Hz: hands steady on
  the block face.
- The board's slots 5 and 8 no longer hold push scenes (they now load a
  western street and a castle courtyard), so the archived D35D fixtures stood
  in for them.

## Limits

- Sampled screenshots and recordings, not frame-perfect proof; dummy audio.
- The LEVEL01 box (type 271) and the LEVEL05 movable dumpster were not rerun;
  placement follows each object's box, but their faces were not inspected.
- The black triangles visible over the dumpster at close range also appear
  in the native comparison; they come from the existing camera/object
  geometry, not the arms, and are outside this job.
- Looking far away still leaves the hands on the object (world contact), as
  accepted in D35D.
- Hand pose (pitch, height, width) is an authored choice for review.

```sh
python3 recomp/tools/local/run.py
```
