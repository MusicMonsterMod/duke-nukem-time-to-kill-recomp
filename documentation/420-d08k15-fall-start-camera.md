# D08K15 - View snaps level when the box under a crouched Duke is kicked apart

Job: [D08K15](../MODERNIZATION_JOBS.md). Status: **Done** (2026-11-08, user-accepted after playtest round 1: "thats a winner. lock it in and close it out").
Modernized only; Vanilla never reaches this code.

## Report

User: "for slot 9, when you crouch on this box, look down, and then quick kick
to break it, theres some weird glitch that happens when duke selfrights really
quickly, like jerks him somehow. this is all boxes of this type in fact."
UI slot 9 is savestate file `slot08` (MINER 69ER, Duke on a type 303 crate,
object flags `0x90c2`, Gatling ammo inside).

## Reproduction (private copy, before the fix)

Lab `recomp/analysis/d08k15/` (local only): `probe.py` loads a copy of
`slot08` with copied cards and private settings, offscreen, port 9715, and
records per sample the animation, modes, feet, object contact `+0x174` and the
camera (`ttk_input`: yaw, pitch, first-person policy). First person, 180 Hz,
Ctrl held, Q:

| Time | Animation | Mode | First person | Pitch |
| --- | --- | --- | --- | --- |
| kneeling | 178 | 0/0 | active, ground | 1.046 (looking down) |
| crate breaks | 108 | 0/0 | **dropped: unsupported-camera, blend 0** | 1.046 |
| next update | 108 | 9/0 | active, fall | **0.039 (level)** |
| landed | 178 (D08K9 crouch landing) | 0/9 | active, ground | 0.039 |

The 60 Hz capture shows the view swinging from the debris to the horizon in
one frame. That snap is the jerk. Duke's body itself goes 178, two updates of
108, then back to 178 through the existing crouch landing. The feet drop
256 (crate height) while the root eases down to the kneel height over about
five updates.

## Cause

The fall initializer sets 108 one update before the dispatcher sets mode 9.
`unowned_fall_early()` (`modern_controls.cpp`) accepted 107/108 only in mode 9,
and `state()` had no other rule for 108 at 0/0 unless it was an owned short
fall (`short_fall`, which needs recent movement and no held crouch). So on that
one update the camera hook found the camera ineligible, released first person
and invalidated the orbit. On the next update the orbit was seeded again from
the native follow camera, whose pitch is level.

This is not crate-specific: any unowned fall that starts with Duke still and
his support gone (a crate under him destroyed, crouched or standing) went
through the same update.

## Fix

`unowned_fall_early()` also returns true for 108 at mode 0, previous 0, when
it is not an owned short fall. That makes the first fall update a
camera-only traversal state, as the later fall updates already were. The
original keeps the fall's motion. Movement input stays with the original
(`locomotion_input_ready` excludes traversal states), and owned short falls
are unchanged.

Test-only: `p` (the view key) added to the `DNTTK_TEST_INPUT` key words.
`ttk-controls-test` gained the missing `replay_beam_roots()` stub (D49's
symbol had broken that harness's link).

## Playtest round 1 (2026-11-08): the left leg pops while turning

User: "ok while this has essentially fixed it. if you continue to crouch, and
after you break the box, just simply move the mouse left and right to turn,
duke's left leg pops in and out".

Lab trace of the crouched-legs test (`first_person_body_eye_inside`): after
the crouch landing the eye sat 353-354 above the pelvis against a limit of
352 (an ordinary crouch: 225), so turning flipped the leg on and off. The
eye's height is measured from the native camera pivot (`player+0x7c0`)
through a +-64 dead band. Around the drop that pivot leaves Duke's root:
about 280 above it on the fall's first update (the eye rose about 200 for that
update), and already on the floor at the landing while the D08K9 crouch
landing still eases the root down over its six-update ramp. The eye followed
the higher neck out of the band and settled at the opposite edge, 128 units
above an ordinary crouch. The same happened on any jump landed with Ctrl held
(baseline captures: the leg flickers while turning after a plain crouch
landing).

Fix (`first_person.inc` `first_person_anchor`, `crouch_landing.inc`): on the
fall's first update (108 at 0/0) and while the crouch landing's ease runs
(`crouch_landing_eye_root()`: set at the landing, cleared when the ramp ends,
the crouch ends or the animation leaves 178), the eye's height is measured
from the root. The existing rebase keeps the eye in place when the reference
switches, the dead band and its easing are unchanged, and the root-anchored
action eyes keep their full convergence. The eye now descends with the body
and settles 220-240 above the pelvis (ordinary crouch 223-225). The leg is
steady while turning at 60 and 180 Hz, after the crate break and after a
jump's crouch landing.

Executable `d63d61048b3a4aaf298a54ce46aba88cc000ed069f20856d0f504ed68c291957`;
`ttk-input-test` and `ttk-aim-test` pass, `ttk-controls-test` as below.
Captures: `research/before-after/D08K15/turn-after-landing-*.png`,
`jump-crouch-landing-turn-*.png`.

## Evidence, first round (executable `38bdab2ea9e68371ab8e5b130ef52885c5c918c5ebd63058be857be08fe15fc2`)

| Run | Result |
| --- | --- |
| Slot 9 copy, first person, 180 Hz, Ctrl held, Q (`cam2`) | first person active on every update. Policy ground, fall, fall, ground. Pitch 1.046 throughout. Lands in 178 and stands on release |
| Same, 60 Hz frame captures (`fpshots2`) | view stays on the debris as the crate breaks and Duke drops. Compare `fpshots` before the fix |
| C toggle crouch, Q (`toggle`) | pitch 1.046 throughout. The D21 latch ends in the fall, so Duke lands with the normal soft landing 105 and stands (63) |
| Baseline executable, same input (`cam1`) | unsupported-camera on the 108 0/0 update, pitch 1.046 -> 0.039 |
| `ttk-input-test` | PASS |
| `ttk-aim-test` (PS-X EXE) | PASS |
| `ttk-controls-test` | fails at the D14 `hud_intact()` HUD-margin assertion with or without this change (pre-existing, unrelated) |
| Player profiles and saves | hash-checked unchanged by every lab run |

Before/after frames: `research/before-after/D08K15/` (local).

## Limits

- Third person: the third-person quick kick (D12E1) runs only standing,
  walking or running, so a crouched Duke cannot kick the crate there. Gunfire
  at the 60 degree pitch limit misses the crate underfoot, and a standing
  first-person kick does not reach it. So in the lab only the crouched
  first-person kick broke the crate under Duke.
- Another crate of the type: three more type 303 crates exist on this level
  (rooms 19 and 59). Teleporting Duke onto one with RAM writes left the lab
  game stuck, so no second crate was tested in the lab. The fix does not depend
  on the crate.
- Duke's body still shows the original fall pose 108 for about two updates
  between kneel and crouch landing (visible in third person only, if such a
  fall is reached there).
- With C toggle crouch Duke stands after the drop (D21 latch ends in the
  air). Held Ctrl stays low.
