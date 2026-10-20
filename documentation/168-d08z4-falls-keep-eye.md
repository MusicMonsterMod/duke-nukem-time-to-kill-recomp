# D08Z4 - Falls keep the first-person eye (slot 6 cop car)

Status: **Done** (user-accepted 2026-10-20: "thats the one! lock it in and close it out!!"). Modernized first person only; Vanilla and third person unchanged.

## Report

User, 2026-10-08: at UI slot 6, standing in front of the cop car and jumping
into it briefly forces third person while playing in first person.

## Reproduction

Private copy of UI slot 6 (file `slot05`, SHA-256
`724c3cb83d252f586ed2c9583e5890b7fdf3cbb171a80eea8905da5a138de272`) and of
the player's settings (first person, manual jump, `jump_walls slide`), in
`recomp/analysis/d08z4-20261020/` (ignored): `common.py` (Xvfb :97, real
xdotool keys, port 9271), `jump.py` (gait, run-up length, heading and held
keys from the environment; samples `ttk_input` fp state and player
animation/mode about every 9 ms). Standing and walking jumps hit the car side
and land back on the street in the eye view. A short run-up lands on the
sloped car body:

| Animation (mode/previous) | Before | After |
| --- | --- | --- |
| 104 jump, 78 landing on the roof | active | active |
| 145 (2/9) slope slide, 108 (9/2), 105 (0/9) | active (D22B slide latch) | active |
| 108 (9/0), 108 (9/9) | **unsupported**, blend 0.49 | active |
| fall off the car to the street, 108 (9/9) | **unsupported**, blend to 0.0 for ~0.7 s | active |

## Cause

`first_person_update()` supported the eye only when
`traversal_camera_early()` was false or a committed move (roll, slope slide)
was running. `traversal_camera_early()` includes D08V's
`unowned_fall_early()` (107/108 in mode 9 that no owned jump started). D08V
sent mantles, hangs and those falls to the orbit because "the eye would be
inside the ledge", which is true for mantles and hangs but not for a fall: 107
and 108 are the owned flight's own animations, already shown from the eye for
owned jumps. Once the slide latch ends, every later 108 on or off the car was
an unowned fall.

## Change (`recomp/src/ttk/first_person.inc`)

`supported` also accepts `unowned_fall_early()`. Mantles (mode 8), hangs
and shimmies (6/7), ladder mounts and climbs (3) keep the orbit. Control
leases, pad input and the third-person camera are untouched; only the
first-person blend target changes.

## Evidence (executable `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778`)

| Case | Result |
| --- | --- |
| Before, run-up 0.2 s | `unsupported`, blend 0.0 (the report) |
| After: stand, walk 0.25/0.6 s, run 0.1..0.5 s; straight, mouse left 250, right 250 | 24 jumps, always `active`, blend 1.0 |
| After: assisted jump; `jump_walls original` | 16 jumps, always `active`, blend 1.0 (includes 107 bounces) |
| E held (D08X jump mantle) | running mantles onto the car (137, mode 8) from the orbit, as before |
| Captures mid-slide (145) and mid-fall (108) | eye over the roof, no clipping |
| Third person, same runs | blend stays 0.0 |
| Vanilla | first-person code idle (`updates` 0) |
| `ttk-controls-test` (LEVEL01 and all levels), `ttk-aim-test`, `ttk-input-test` | PASS |

## Limits

- Only the slot-6 car was driven. Other unowned falls (off edges, failed
  grabs, drops into water) use the same rule but were not exercised.
- A thin black wedge at the top-left edge in some fall frames looks like the
  sky-edge issue tracked as D17R; not part of this change.
- The user's playtest decides the feel.
