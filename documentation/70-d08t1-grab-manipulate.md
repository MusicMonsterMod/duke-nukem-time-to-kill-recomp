# D08T1 - Hold to grab: push/pull separate from mantling

Status: **Done** (user-accepted 2026-09-30). Modernized only; Vanilla keeps the
original Action rules. Brief: [69-d08t1-grab-manipulate-brief.md](69-d08t1-grab-manipulate-brief.md).
Replaces the D08T control contract in [64-d08t-pushable-objects.md](64-d08t-pushable-objects.md);
the original rules recorded there are unchanged.

## How the D08T exception was built (before this change)

- **Climb mask** (`push.inc`, generated entry hook `0x80051CF0`): whenever Duke
  walked into a pushable object with Cross held, this tick's Cross bit was
  cleared unless a Space climb was pending. So E + W could never climb a
  pushable object, only a non-pushable one.
- **E grab request** (`pc_input.cpp`): E while touching a pushable object
  (`push_contact_ready`) started `push_request`: no directions, Cross only at
  idle 63, which the original idle Action `0x80051890` turned into grab 121.
  E again let go (`push_released` then blocked E's Cross until E was up).
- **Space climb**: Space on a climbable pushable object ran a host climb
  (holster pulse, Up + Cross, then Up through anims 134..142) instead of a jump.
- HUD: `W/S PUSH/PULL - E LET GO - SPACE CLIMB`.

So a hidden object flag (`0x08000000`) swapped the meaning of E and Space.

## Original rules that matter (verified)

`0x80051890` (idle Action, called only from `0x800467F4` and `0x80052C14`,
both behind a fresh-Cross test) grabs a touched pushable object and otherwise
handles switch/use objects (`0x00400000`, `0x00800000`, `0x01000000`). It never
climbs. Climbing a climbable object (`0xC0`) happens only in `0x80051CF0`,
walking or pressing Up into it with Cross held. So at an ordinary climbable box,
E alone while standing does nothing and W + E climbs.

## Mouse2 audit

`original_aim` (Mouse2, R1) was read in Modernized by: the pad path (R1), view
facing / presentation (`presentation_ready`), the airborne jump redirect
(original arc while held), jetpack face-view (`jetpack.inc`), shortcuts,
crouch, terrain and the weapon-aim shot hook (all "step aside while held"). With
`view` aiming, `view_aim_input_ready()` already removed it in guarded
locomotion. What Modernized with `view` aiming loses now that Mouse2 is Grab:
R1 precision aim where view aim was not ready (airborne, unsupported weapons),
the "hold RMB to keep the original jump arc" case, and holding RMB in jetpack
flight to stop Duke turning to the view. All three are still reachable by
binding `original_aim` to another input. No Modernized path reads Mouse2 as
`original_aim` by default any more.

Alt audit: plain Alt does not release capture (only GUI and Alt+Tab do);
Alt + wheel still sets camera distance; Alt + Enter never uses inventory; W/S/
A/D still register while Alt is held. Window-manager Alt shortcuts outside SDL
(for example Alt + drag) were not tested.

## Binding model

| Profile / aiming | Right mouse | Alt (either) | Grab / Manipulate |
|---|---|---|---|
| Modernized, `view` aiming, independent camera (default) | Grab | Grab | RMB or Alt |
| Modernized, `original` aiming or original camera | Original precision aim (R1) | Grab | Alt |
| Vanilla | Original precision aim | unchanged | Original Action rules |

Two actions, both rebindable: `grab` (default Mouse2) and `grab_alt` (default
Alt, meaning either Alt key). With legacy aiming the `grab` input serves
precision aim instead, so nobody loses precision aim where it still means
something. `original_aim` defaults to Unbound. Profile schema 17 migrates:
`original_aim` Mouse2 moves to `grab`; a customized precision-aim input is
kept and `grab` then takes Mouse2 only if free; `grab_alt` takes Alt if free.

## Contract (Modernized)

| Input | Result |
| --- | --- |
| E (+ W into it) | Normal mantle: the original climb, exactly as at any climbable object. E alone at a pushable object does nothing |
| Hold Grab touching a pushable object (W may be held) | Automatic holster if armed, original line-up and grab 121 |
| W / S while Grab is held | Camera-relative push 120 / pull 119 (original motion, unchanged) |
| Release Grab | Let go; weapon redrawn as after E interactions |
| E while grabbing | Let go, then E is the normal interaction (W + E climbs) |
| Space, fire, Circle, R1 while grabbing | Ignored until Duke lets go; no buffered jump |
| Grab against anything else | Nothing: no R1, no Cross, movement unchanged |
| Blocked object, hit, pause, focus loss, capture loss | The grab ends; a fresh Grab press is needed |

A push or pull cycle that has started runs to its original end (about one
1200-unit shove) even after letting go; directions stay neutral until the
original leaves 119..121, so a held W cannot extend it.

## Implementation

- `input_bindings.def`: `original_aim` Unbound; new `grab` (Mouse2) and
  `grab_alt` (Alt, scancode 226; `down()` also accepts Right Alt).
- `pc_input.cpp`: `grab_down()`/`aim_down()` resolve the model above;
  `frame.held[original_aim]` and R1 follow `aim_down()`. Grab request/latch:
  request while Grab is held and touching; holster pulse when armed; Cross at
  idle 63 after 6 frames; latch on 119..121; let go on release, E, a 12-frame
  loss, timeout, pause or focus loss; `push_released` (fresh press) and
  `push_letting_go` (E's Cross and directions wait until the original lets go)
  are separate. Space climb code removed. Prompts: `HOLD RMB TO GRAB` on the
  first two touches, `W/S PUSH/PULL - RELEASE RMB TO LET GO` on the first three
  grabs (live binding name: RMB, ALT or the key).
- `push.inc`: the `0x80051CF0` mask now applies only while Grab owns Cross
  (`input_push_grab_owns_cross`), so E's Cross climbs. New generated entry hook
  `0x80051890` (`game.local.toml`, regenerated C, not hand-edited) clears this
  tick's Cross for Duke from the two original callers when Grab is not asking
  and the touched object is pushable with no switch/use flag (`0x01C00000`),
  so E never grabs. `0x80051890` was already SHA-guarded.
- Debug: `controls.push_idle_masks` beside `push_masks`.

## Evidence (binary `66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`)

Private copy of the D08T slot-5 alley state in `recomp/analysis/d08t1-grab`
(`grab.py`, `vanilla.py`); Xvfb with real xdotool keys and mouse. Runs:
third person + `view`, first person + `view`, third person + `original`
(Alt grabs).

| Case | Result (all three runs) |
| --- | --- |
| W into the dumpster, then E held | Climbs (mode 8) to the top (Y about -10767), never grabs |
| Stop at contact, E alone | No grab, no climb, idle 63; 16-24 idle masks |
| Hold Grab at contact, W | Grab 121, push 2582 -> 3539 then the original shove continues to the alley end (3795), original lets go (82); held Grab does not regrab; release redraws the weapon |
| Fresh grab, S | Pull 2582 -> 1639; mouse look counted while held; release lets go |
| Release Grab, then W + E | Climbs to the top |
| Grab held while walking in with W | Grabs and pushes, never climbs; release lets go |
| Grab, then W + E while still holding Grab | Lets go, then climbs (after the shove already started) |
| Grab, Escape (pause), resume | Grab ended, not re-grabbed |
| Grab against a non-pushable object / open space | Nothing grabbed, Duke walks, no R1 |
| `original` aiming: RMB | R1 (precision aim), no grab |
| Vanilla, original pad, gapless route | Grab 121, push +1185, pull -923, no climb (D08T: +1202 / -928) |

Native: `ttk-input-test` (E mantles, RMB/Alt latch, push/pull, release, fresh
press, holster pulse, neutral directions during release, legacy precision aim,
focus loss), `ttk-controls-test` (grab-only climb mask, E-only idle mask,
caller/actor/flag/Vanilla/capture guards), `ttk-aim-test` PASS; Python 87 OK
(2 skipped) including schema-17 migration.

## Limits

- Verified on one object, the alley dumpster. The rules key on the original
  flags, so other pushables should follow them.
- A pushable object that is also a switch/use object keeps the original
  precedence (E's Cross grabs it); none is known in the first map.
- An original push/pull shove that has started finishes before Duke lets go.
- Grab still needs roughly square contact (original line-up).
- `build.py` currently stops in its runtime patch-stack check (the reviewed
  patches no longer apply cleanly over the live runtime edits); generation was
  run directly with the same command. Not changed here.
- No controller support yet (D09).
