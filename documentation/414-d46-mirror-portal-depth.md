# D46 - Apartment mirror shows the outside when Duke turns left

Status: Done (user-accepted 2026-11-08: "fully accepted. excellent work!
absolutely superb. lock it in, close it out").

## Report

User, after the D41H playtest: "slot 8 will load you into the apartment, if
you turn duke to the left, you'll notice that the mirror pops into the
outside. i never noticed this before". UI slot 8 is file index 7 (saved
2026-11-08 09:50, LEVEL00 apartment, first person).

## Reproduction

Offscreen, private copy of the slot (intake hashes recorded; the player's
files were hash-checked unchanged after every run). Facing the bathroom
mirror (camera yaw 84) the mirror reflects Duke and the door. Turning left,
from about yaw 55 the mirror shows a brick wall with a barred window instead.
That wall is outdoors, behind the bathroom wall. Same result with Classic,
dynamic lights off, original draw distance (host
visibility off), unified geometry off, host near clipping off, portal margin
0, open portal tops off, precise culling off and widescreen near off. So it
is not a Polymorph, lighting or D17 host visibility problem.

## Owner

The game's own portal walk (`0x8006276c`) listed the outdoor rooms. At yaw 84
it lists room 8 (the apartment) alone; at yaw 48 it lists 8, 83 (the doorway
to the left), 7 and 5 (the street) with full-screen rectangles, then 6 with
a rectangle over the right of the screen, where the mirror is. Room 6 is the
street beyond the bathroom wall. Setting room 6's piece count to 0 in a
private session restored the reflection in both renderers. So room 6's
pieces were drawn over the reflection.

Why it was listed: the portal rectangle routine (`0x80062258`, called with
`a5 = camera+0xa0`) gives a portal with any corner nearer than `camera+0xa0`
its parent room's whole rectangle instead of its own projection, so rooms
around Duke never pop. `camera+0xa0` is the depth of the camera's target
(Duke), written by the actor draw as `max(depth, GTE H)`
(`0x80034d04..0x80034d40`). The actor draw returns before that store when the
actor is nearer than H (`0x80034c84..0x80034c90`). The original camera never
gets that close to Duke, but the Modernized eye view always does. The value
stayed at the last third-person depth, 5386 units. Every portal within about
5400 units of the eye then passed its parent's full screen on: doorway 83,
then the front door to street 7, then the street portal to 5 (corners about
4850 deep), and 5's portal to room 6 kept only its own projection. Third
person tracks the value correctly (1654-1908 here), lists only 8, 83 and a
narrow strip of 7, and never had the fault.

D35E (`first_person_ladder_near`) already raises Duke's joint depth to H in
some first-person arm states, which lets the original store H there. In
ordinary first-person play it does not apply, so the stale depth stayed.
Older builds were not bisected, so when the fault first appeared is not
established; the mechanism applies to any first-person build in which
Duke's own draw returns early.

## Fix

`camera_target_near` (`recomp/src/ttk/first_person.inc`, called from the
`0x800b4d2c` hook with ra `0x80034c8c`): when the main camera's own target
(`camera+0xa4`) is skipped as nearer than H, write `camera+0xa0 = H`, which
completes the original's formula `max(depth, H)` for the case it never
reaches. This is game-wide (any level, any Modernized camera close to Duke),
not tied to this mirror. Only the main camera at `sp+0x98` counts, so the
stacked mirror camera's reflected Duke is ignored. Live frames only (not
redraw-worker replays, which see the written value in guest memory). Vanilla
never reaches it (`input_modernized()`, code identity). Developer
`DNTTK_TARGET_DEPTH=0` keeps the stale value for A/B tests.

## Evidence

- First person after: `camera+0xa0` = 160 (H) instead of 5386; yaw 48 lists
  8, 83 and 7 (a left strip through the front door); rooms 5 and 6 are no
  longer listed there.
- Matched shots, before and after, yaw 84, 69, 62, 55, 48, 41, 34, 20: the
  mirror keeps the reflection at every heading after the fix; the street seen
  through the doorway is unchanged. Also 180 Hz (redraw workers) and Classic.
- Third person: identical values and lists before and after.
- Vanilla: identical image at load (pixel diff empty), value untouched.
- Native: `ttk-near-test` and `ttk-input-test` PASS, `ttk-aim-test` passes.
  `ttk-controls-test` stops at a pre-existing assertion (`hud_intact()`,
  line 1755 with the fixtures used) with and without this change, so no
  native case was added there.
- Shots: `research/before-after/d46-mirror/` (local). Scripts and private
  copies: `recomp/analysis/d46/` (local).

## Limits

User playtest of the slot 8 turn. Other mirrors (the slot 5 club mirror) and
first-person pop-in near doorways elsewhere were not surveyed; Polymorph's
host walk still adds every room it reaches, so it cannot draw less than
before, while Classic's first-person list is now the original's own rule for
a camera at Duke.
