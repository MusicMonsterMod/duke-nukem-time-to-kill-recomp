# D08T - Pushable objects: modern grab, push/pull and climb

Status: **Done** (2026-09-30, user: "it works so much better than the original now. this is it rock solid. confidence level is very high."). Modernized only; Vanilla keeps the
original Action rules. Opened from the user's report about the green dumpster in
the first map's alley, with a savestate in UI slot 5 in front of it.

## The report

In Modernized, first or third person, wedging Duke into the dumpster and
holding **E** never grabbed it. It did nothing or climbed on top. On DuckStation
the user could almost start the push animation, but Duke then climbed. The user
wants both push and mount, with an easy modern control for every pushable
object, not only this one.

## Original rules (verified)

Static reading of the owned executable plus isolated headless replays from a
private copy of the slot-5 state (original pad input, Vanilla profile):

| Piece | Address / value | Meaning |
| --- | --- | --- |
| Object type flags | `*(0x800d2660) + 28 * object[+0x2c]` | `0x08000000` pushable; `0xc0` climbable; `0x200` ladder |
| Dumpster | object `0x801d9264`, type 165, flags `0x081090e2` | Pushable **and** climbable, 1024 units high |
| Idle Action | `0x80051890` (from idle `0x8005201c`) | Fresh Cross (`word & 3 == 1`), holstered, object pushable: `LineUpWithObject(0x113)`, anim **121**, object stored at `player+0x290` |
| Walk/Up into obstacle | `0x80051cf0` (callers return `0x80052544`, `0x8005349c`, `0x8005384c`) | Cross held: ladder `0x200` -> 185, `0x400` -> 191, `0x100` -> 197, climbable `0xc0` -> climb 134..139 (mode 8) |
| Grab 121 at track 3 | `0x80048c74` (dispatcher `0x80048410`) | Cross held + Up -> push **120**; + Down -> pull **119**; Cross released -> let go |
| Push/pull 119/120 | `0x80048e84`, `0x800910a0` | Moves the object along one world axis chosen from Duke's heading quadrant |
| Action word | `*(0x800d1b50[player+0x233])` = `0x800d147c` | Per-tick shift register: bit 0 this tick, bit 1 the previous tick. Ticks run about every 3 video frames |

Original replay results: holstered Cross alone grabs (yaw snaps 1050 -> 1024,
Duke steps back to about 215 units from the face). Cross + Up pushes the
dumpster +1202 units; Cross + Down pulls it -928. Left/Right do nothing while
grabbed. Up + Cross into it, or Up during the grab after Cross was lost,
climbs (139/140, mode 8) and ends on top. A plain jump (97) at it does not
mount it. So the original needs "stand still, hold Action, then Up/Down", and
any direction held with Action climbs. That is the DuckStation behavior the
user saw.

Why Modernized failed: E holds Cross only while `interaction_ready()`, which
needs the ground or ladder lease. Anims 119..121 were in no lease, so Cross
dropped the moment the grab started and the original let go. W held into the
dumpster plus E reached `0x80051cf0` and climbed. During a grab the camera also
fell back to the original camera and WASD to the tank fallback.

## Modernized contract

| Input | Result |
| --- | --- |
| **E** touching a pushable object (W may still be held) | Grab it: automatic holster if needed, then the original line-up and grab |
| **W / S** while grabbed | Push / pull, relative to the camera (W pushes when the camera looks at the object, pulls when it looks at Duke) |
| **E** again | Let go (the weapon is redrawn as after other E interactions) |
| **Space** touching or grabbing a climbable pushable object | Climb onto it (automatic holster, original climb, redraw on top) |
| Space on a grabbed object that is not climbable | Let go; that press never jumps |
| Mouse | Camera stays live while grabbing, pushing and pulling |
| First person (P) | Blends to the third-person orbit while grabbing (the eye would be inside the object); back to the eye after letting go |

Walking into a pushable object **with E held** no longer climbs it; Space does.
Ladders, ledges and other climbables keep their existing E behavior.

## Implementation

`recomp/src/ttk/push.inc` (included by `modern_controls.cpp`) and
`recomp/src/ttk/pc_input.cpp`:

- **Climb mask** - new generated entry hook on `0x80051cf0`. For Duke only,
  from the three original callers, in captured Modernized play, when
  `player+0x174` is pushable and no Space climb is pending, it clears bit 0 of
  this tick's Action word. The original then falls through to its wall bump,
  and the next tick sees a fresh press. Nothing else is written.
- **Camera-only grab lease** - `state(camera_only)` accepts 119..121 when
  `player+0x290` is pushable, Duke is holstered and in mode 0/0.
  `locomotion_input_ready()` excludes it, so there are no facing writes, gait
  injection or Forward.
- **Grab request** - E while touching a pushable object: no directions and no
  Cross until original idle 63 with the weapon stowed and at least 6 frames of
  released Cross, then Cross (a fresh press for `0x80051890`).
- **Latch** - once 119..121 is live, Cross is held by the host. Camera-relative
  input dotted with Duke's facing gives original Up (> 0.38) or Down (< -0.38).
  The latch ends on E, Space, focus/capture loss, or 12 frames outside the
  grab (hit reaction, or the original letting go when the object is blocked).
  After letting go, E's own Cross stays off until E is released and the grab
  ended, so a held E cannot regrab.
- **Climb** - Space: optional holster pulse, then Up + Cross from settled
  ground (along Duke's facing), then Up alone while mode 8 anims 134..142 run;
  ends when he stands again. Square never reaches the pad for that press.
- The first three grabs of a session show a short hint:
  `W/S PUSH/PULL - E LET GO - SPACE CLIMB`.
- New SHA-256 guards: `0x80051cf0` (812 bytes), `0x80048c74` (240),
  dispatcher table entries 119..121 at `0x80014188` (12). `0x80051890` was
  already guarded. `game.local.toml` adds `0x80051CF0` to
  `mod_function_entry_funcs` (regenerated C, not hand-edited).

## Evidence (binary `941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`)

Private copy of the user's slot-5 state in `recomp/analysis/d08t-push/cards`
(savestate only; no memory cards). Scripts `probe.py`, `research*.py`,
`modern.py`, `modern_d.py` there; Xvfb with real xdotool keys and mouse.

| Check | Result |
| --- | --- |
| A: W held into the dumpster, then E held (third and first person) | Holster, grab 121, push 120; never mode 8; dumpster 2582 -> 3713 (the alley end stops it; the original then releases with 82) |
| B: stop at contact, tap E, turn the mouse, S | Grab 121; S pulls it 2582 -> 1668; mouse look counted |
| D: grab, camera swung to face Duke, W | Pull 119, dumpster 2582 -> 1681 (camera-relative); 59.5 fps |
| C: armed, W into it, Space | Pistol stowed, climb 139 -> 140, stands on top (Y -10740, top at -10752), pistol redrawn |
| A: after letting go, walk back in, Space | Climb 139/140 (mode 8) |
| First person during the grab | View blends to the orbit showing Duke pushing; screenshot `m-A-push-first.png` |
| Vanilla original push sweep on the final binary | Grab 121, push +1202, pull -928, unchanged |
| Vanilla regression route `d08t-vanilla-final` | Exit 0; spawn/draw/fire/jump/inventory/forward/turn/holster captures reviewed |
| Native `ttk-input-test`, `ttk-controls-test`, `ttk-aim-test` | PASS, including new D08T checks (mask only for Duke, only from the original callers, never Vanilla/uncaptured/climb request/plain climbables; camera-only grab lease; latch, push/pull, let-go without regrab, Space climb without jump, focus loss) |

## Limits

- Verified on one object: the alley dumpster. The rule keys on the original
  object flag, so other pushables should follow it, but none were checked.
- Grab needs roughly square contact: the original `LineUpWithObject(0x113)`
  still rejects about 24 degrees or more off the face, as in the original.
- Push speed, the axis, and when a blocked object makes the original let go
  are unchanged original behavior.
- The climb itself takes about 2 seconds of original animation.
- Space next to a climbable pushable object climbs instead of jumping.
- No controller support yet (D09).
