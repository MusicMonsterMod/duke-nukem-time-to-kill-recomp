# D10 - Third-person camera polish: recenter, shoulder, saved distance

**Done - user accepted 2026-09-29** ("it's done, fully accepted", after about five
minutes of play).

Second bounded D10 pass (2026-09-29). The earlier pass added Alt+wheel
distance (see [33-controls-shortcuts.md](33-controls-shortcuts.md)). The
user chose this scope: no specific camera complaint; an optional shoulder
offset, off by default; a recenter key, not automatic recentering; and a saved
Alt+wheel distance.

Modernized with the independent camera only. Vanilla, and the `original`
camera option, keep the original camera; the new keys do nothing there.

## Controls

| Action | Default | Behavior |
| --- | --- | --- |
| `camera_recenter` | V | Smoothly swings the orbit to Duke's heading (`player+0x1c`) and the rest pitch. Mouse motion cancels the swing. |
| `camera_shoulder` | H | Cycles centered -> right shoulder -> left shoulder -> centered, with a centered notice. |
| Alt+wheel | - | Unchanged distance control; the value is now saved. |

**Rest pitch** is the pitch seeded from the original camera the first time
the orbit starts in a session, which is the original follow view. On the
ground in Modernized, Duke's body already faces the view (D07A/D07C). Probes
on the turret-room wade and the ledge, with `view` and `original` weapon aim,
standing and strafing with A/S/D, all read heading = camera yaw within 0.1
degrees. So V mostly levels the pitch. The yaw swing matters only where the
facing differs from the view.

**Shoulder offset** is 0.22 x the current boom length, clamped to
192-640 units. It eases in with the distance blend (0.25 per host frame). It
moves only the requested eye along the view's right row
(`cos yaw, 0, -sin yaw`) inside `orbit_constraint`, before the original
four-candidate constraint solve. The look target and view matrix are
unchanged. Original collision still shortens or shifts the solved camera.
View aiming traces from the solved eye (`camera+0x14`), so shots still
converge on the crosshair.

## Persistence

Profile schema **12** adds Modernized controls `camera_distance` (0 =
original follow distance, else 768-6144) and `shoulder`
(`center/right/left`), plus the two actions. Migration from v8-v11 adds new
actions without taking inputs that are already bound. A v11 profile with
holster on H gets another free key for `camera_shoulder`. Custom bindings
always win.

The launcher passes `DNTTK_CAMERA_DISTANCE`, `DNTTK_CAMERA_SHOULDER` and
`DNTTK_CAMERA_STATE_FILE` (`<settings>.camera-state`). The runtime never
edits the profile. After a distance or shoulder change has been stable for 30
host frames, the runtime writes that side file atomically
(`{"camera_distance": N, "shoulder": "..."}`). `run.py` validates it, merges
it into the Modernized controls after the game exits, and again at the next
launch in case the launcher was killed. It then deletes the file. An invalid
side file is discarded with the profile unchanged. A saved distance starts in
place on the first orbit, with no zoom on entry.

Launcher flags: `--camera-distance original|768..6144`,
`--shoulder center|right|left`. `--show-settings` lists the saved values and
keys.

## Evidence

Binary `84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`.

- Python: 71 tests pass. New tests: v11 migration without stealing inputs;
  side-file absorb, validation and deletion; CLI to env round trip; post-exit
  merge; Vanilla env; invalid distances rejected.
- Native: `ttk-input-test` covers V counting (auto-repeat is one press), the
  H cycle from the profile side, no guest commands, uncaptured and
  `original`-camera keys ignored, and the side kept across release.
  `ttk-controls-test` covers the recenter swing to heading and rest pitch,
  mouse cancel, shoulder offset magnitude, the constraint delta moving the eye
  to Duke's right, no position commits, the side-file content and the 30-frame
  settle. `ttk-aim-test` also passes.
- Live isolated SDL probes (Xvfb, private profile and card copy under
  `recomp/analysis/d10-camera`, savestate slots 1-2 from the D08Q fixture):
  - Recenter from -0.155 rad pitch returned to rest 0.0127 within about 40
    frames; yaw error 0.09 degrees; a mouse move cancelled a swing.
  - Right shoulder: solved lateral 637.5 of 640 in the turret room.
  - Left shoulder: the original solve pulled in to 544 lateral at 2687
    distance against the wall.
  - On the ledge, right 370 / left 465 lateral (constrained).
  - Screenshots show Duke left of center for the right shoulder and right of
    center for the left, with no geometry clipping in the reviewed frames.
  - Firing with the right shoulder: 1 shot, 0 rejected, world impact.
  - Alt+wheel 3 notches = +576; side file written. After a clean game exit the
    launcher printed `Saved camera distance 3627, shoulder left.` The
    relaunch started at radius 3627 with the saved side.
- Vanilla regression route (`analysis/vanilla-regression/d10-vanilla`)
  exit 0.

The debug-server `quit` answered "emu busy or frozen" in these probes. The
probe therefore closes the game with SIGTERM, which SDL turns into a normal
quit, so the launcher's after-exit merge could run. Real window closes take the
same launcher path.

## Remaining

The user accepted the whole job. No per-location playtest of doors, corners,
tight rooms or vertical traversal was recorded beyond the two probed rooms;
later campaign camera issues should be opened as new jobs. Automatic recentering was not requested and is not implemented.
