# D08X - Hold-E airborne ledge grab

Status: **Done** (user-accepted 2026-10-01).

## Original rules (owned executable, private replays)

- The airborne handler for standing directional and running jumps
  (`0x80055904`, animations 98/103/104/109) first calls the reach
  `0x800557f8`. An animation event (`+0xb8 & 0x2000`) arms the reach once per
  jump by setting `+0x224 |= 0x800000`. While armed, a held Cross with the
  weapon stowed (upper-table bit 8 clear) switches Duke to reach 109. Every 109
  update then runs the surface acquisition `0x80055208` **before** the flight's
  own collision (`0x8007a98c`). Acquisition catches a ladder or climbable
  object (mode 3), a climbable object top (mode 7) or a ledge with headroom
  (mode 6, grab 147/148, hang 149, pull-up 140), subject to the original facing
  tests (0x1e5 for 109, 0xeb for the vertical jump 97) and height probes.
- The arming event comes late in a jump. A standing jump at a wall whose ledge
  is in reach hits the wall first (impact 107) and bounces off. This happens in
  **Vanilla too** (Square, then Up + Cross at the 1024-high wall beside slot 12:
  98, 107, landing 78) and in Modernized with W + E + Space before this change.
  Duke then only gets up by walking back into the wall with W + E (ground
  mantle 139/140).
- A fall off an edge (108) arms the reach as well: with E held the original
  already reaches (109) in a fall.

## Change (`ledge_reach.inc`, `pc_input.cpp`, `modern_controls.cpp`)

While E asks for a reach (held, its press still pending, or an airborne reach
already started) during an original jump (98/103/104, mode 9), the owned
ballistic hook (`0x8003ebf4` from `0x80055934`) sets the original arming bit at
once, unless one of the original blocking bits (0x241) is set. From there the
original takes over unchanged: the held Cross enters 109 when the weapon is
stowed, and the original acquisition decides what, if anything, is caught.
With nothing in reach Duke flies, lands or bounces exactly as before. Jumps
without E, precision aim, Vanilla and all other animations are untouched. The
first-person eye jump (which owns no host flight lease) is covered too.

## Evidence (binary `bd153dd540359b1fa5eafc84a9ea04492099abd20e7e8479d511b33d36b065b3`)

Private Xvfb runs, real keys and mouse, private copies of slot 12 and the alley
state (`recomp/analysis/d08x-ledge-grab`):

| Case (slot-12 wall, 1024 high, 425 away) | Before | After |
| --- | --- | --- |
| W + E held, Space (pistol drawn) | 98, 107 bounce, walk-in mantle | 98, 109, grab 148 (mode 6), pull-up 140, pistol redrawn: **3/3** |
| W + Space, no E | 98, 107 bounce | same: 3/3 no grab |
| Holstered, E pressed at takeoff | - | grab and pull-up 2/2 |
| Holstered, E pressed 12 frames after Space | - | grab and pull-up 2/2 |
| Armed, E pressed 12 frames after Space | - | no grab 3/3: the stow is not finished at impact |
| First person, W + E, Space | bounce | grab and pull-up 2/2, back to the eye view |

- Jumping with E held off the walkway into the pit, and at the walkway end:
  reach, no catch, fall into the water as without E (2/2 each). A street wall
  too tall to have a ledge: reach, impact 107, landing, nothing odd (2/2).
- Running jump (E held from before the run-up) at the 1024-high alley object:
  6/6 reach right after takeoff, but Duke meets it low (feet about 370 above
  the floor) and the original acquisition does not catch; he bounces and, with
  W + E still held, gets up with the ordinary mantle. Same end as without the
  change (6/6), where there is no reach at all.
- D08J regression: the D08 alley transfer (climb the first ladder, run-jump
  with E held toward the second ladder) attaches at X 4590, climbs, exits and
  redraws, 2/2 valid runs (a third failed its first-ladder climb before the jump).
- Vanilla wall jump on the final binary: identical bounce.
- Native `ttk-controls-test` (new D08X fixture) and `ttk-input-test` PASS.

## Limits

- Only the slot-12 walkway wall (a ledge 1024 high) produced a mid-air ledge
  catch in these runs. The apartment exterior and crystal-2 ledges named in the
  job were not reached from the available private states. No private savestate
  exists there.
- Armed, E pressed late in a short jump: the original will not reach until the
  automatic stow finishes (D08J's 4x budget), so a close wall is hit first. E
  held a moment before the jump works.
- Ledges the original acquisition rejects (too high, too low at contact, wrong
  facing, no headroom) are still rejected: this job opens the reach earlier, it
  does not widen the catch.
- Holding E through any jump now shows the original reach pose for the whole
  flight instead of from its apex.

## Follow-up: running jumps into crates (boxes room, 2026-10-01)

User report: in the boxes room (user UI slot 11; private copy in
`recomp/analysis/d08x-boxes`), Shift + W + Space with E held bounced off the
wooden crates. Crates are objects (type 20, flags 0x2090c2: climbable 0x80,
ledge 0x40), 1024-unit cubes (box index 0 via `+0x44`), top 1072 above the
floor; walking into one with W + E climbs it with 139/140.

Cause: with E held the reach (109) now starts at takeoff, but the crate only
becomes the contact (`+0x174`) in the flight collision, after the acquisition
has run, and the next update is the bounce 107. Measured at impact: the crate
top is 600..740 above Duke's feet (his root is 507 above the floor value
`+0x1c8`).

Change (`ledge_reach.inc`, `ledge_reach_mantle()` from the player update): in
the first four updates of a 107 bounce with E asking for a reach, holstered,
if the contact (or, when the collision left it empty, a climbable object in
Duke's cell list, in front of him and within 1100) is a climbable object
(0xc0) whose top is 0x100..0x3ff above his feet, start the original
height-matched mantle 134..138 from where he is: floor `+0x1c8` = feet, ledge
`+0x1c4` = top - feet, velocity cleared, mode 0. The original mantle plays and
ends by raising the floor to the crate top (`0x8004c728`). A jump from right
against a crate (top about 1070 above the feet) is left alone: starting the
full climb 139 there stalled in the hang 149, so that case keeps the bounce and
the ordinary walk-in climb.

Evidence (binary `9bade2a5c7202edaba9ff848f9ee4604487249a265772f380ed93fb4247fba9c`,
real keys, private copy of the user's save):

| Crate | E held, jump after 4/10/16/22 run frames | No E |
| --- | --- | --- |
| Straight run (crate A) | on top 3/4 (136, 137, 137); the miss is the jump from contact | bounce 4/4 |
| Diagonal (crate B) | on top when the jump reaches it (22: 137); earlier jumps land short | bounce 4/4 |

Ends on top at Y -7713 (the ground climb ends at -7719). Earlier rounds with
keys held longer: 6/6 mantles (W held then runs on across the crate). Slot-12
wall ledge grab still 148/140 (2/2); alley ladder transfer still attaches 2/2.
Native fixture: mantle choice and fields, too-high/too-low, no E, facing
away, non-climbable and Vanilla refusals. Suites PASS.

Remaining limits: a jump started while already touching a crate bounces (walk
in with W + E instead); ledges rejected by the original rules stay rejected.

## Playtest fix: crate hang and lost modern controls (2026-10-01)

User report: jumping with E into the crates under the opening, Duke grabbed a
crate, hung there, and modern controls were lost for the rest of the session.
Session log: `108 9/8` then `149 7/9` (original object hang, mode 7), then
`inactive (identity) ... ident=0` until exit.

- **Identity loss (pre-existing bug).** The original object hang
  (`0x80055438`) clears bit 0x40 of the upper-body flag table entries for 149,
  152 and 153 (`0x800c2a78`, `0x800c2a84`, `0x800c2a88`); the ledge hang
  (`0x8004c8b8`) sets it again. The controls guard covered that table as code,
  so any object hang disabled every Modernized feature until a later ledge
  hang. The guard is split to exclude entries 149..153 (game state); a new
  diagnostic logs `[TTK identity] guard N ... changed at ADDR` on the first
  failure so a future report names the cause.
- **Stuck hang.** Reproduced on the stack at (12824, 77305): the reach catches
  the upper crate (mode 7, 149/150/151); with a crate (or ceiling) in the way
  no input climbs, only Square lets go (Up, Up+Cross, Cross, Down tested). In
  Modernized, S in that hang lets go, and W held for 40 updates without a
  climb lets go on its own (`object_hang_update()`, original Square). The
  hang's entry frame (mode 7 from 9) now keeps the camera lease too.

Evidence (binary `bf368b6814ddff4cd76db6d48303a33c72351638db6f89f6b8f0326c0bebbd45`):
stack jumps with W + E held 3/3 catch, release (108), land and end with the
modern lease ready; no identity reports, no `ORIGINAL MOVEMENT`. The other
stack: mantle then the original climb to the top 2/3, one landing. Crate A
mantle 3/3, slot-12 wall grab and ladder mount unchanged. Native: the three
toggled words keep identity, a neighbouring word still breaks it; suites PASS.

## Playtest fix: first-tier crate hang spins (2026-10-01)

User: grabbing the first tier of a stacked crate glitches, and A/D turns Duke
round "like a ballerina with his arms in the air". Reproduced at the stack
(12824, 77305): from 700..1100 away the reach catches the upper crate right at
the first tier's top and enters the original object hang (mode 7). That hang
is a pole-style hang: A/D walk Duke round the object (heading swept 2583..3907
in the capture). On a crate it never makes sense, so in Modernized a mode-7
catch on a crate-type climbable object (flags 0xc0) lets go at once through the
original Square: the hang shows for 2..3 frames and Duke drops as from a
bounce; A then strafes normally. Single-crate mantles (3/3) and the offset
stack climb are unchanged; binary
`c1c49119b728c262924d372165f996901cacffd252c218138c25ebce68586fe7`.

## Higher grab confidence: lifted reach retry (2026-10-01)

User: "for jumping to grabbing ledges ... I just want higher confidence"
(earlier: the misses were ledges "a bit too high").

The reach's acquisition probes for a ledge at a fixed point 425 above Duke's
root (`0x8007b2f4`: root + rotated (0, -0x1a9, 0)), so a ledge a little above
the jump's apex is missed. `ledge_reach_lift()` (player update) retries the
same original acquisition `0x80055208` during an E reach (109, and the first
updates of a bounce 107) with Duke raised 160, 320 and 480 (about an arm's
length), through the existing isolated `original_call`. The original decides
the catch, its type and the hang; a miss restores height and probe fields.
After a catch Duke is put back at his real height and eased up the lifted
distance over the grab (a third per update, minimum 40), so there is no pop.

Evidence (binary `43eda09cfc8b7c7045dc1bd39aa5aac71967c22933fa3e12e2b2328dd9e20f61`):
- Mechanism: on a 2048-high wall in the boxes area (too high for the original
  from every approach), an experimental 960 limit caught and pulled up 7/8
  (catches needed 640..960) with a smooth ~18-frame rise; the shipped 480 limit
  does not grab that wall (8/8 no catch), as intended for a ledge twice Duke's
  height.
- The slot-12 1024 wall still grabs and pulls up 3/3 (one catch now comes from
  the lifted retry, earlier in the arc); crate mantles 3/3; stacked-crate
  release 3/3; alley ladder transfer 2/2; ladder-top mount unchanged.
- Native: retry order (+160/+320/+480), full restore on misses, catch at +320
  keeps the original hang and eases up exactly 320, no retry without E, with
  precision aim or armed.

Limit: no ledge between about 1300 and 1800 high was found from the available
private states, so the gain on "a bit too high" ledges is for the playtest
(drained-water area).

## Angle forgiveness (2026-10-01)

User: jumping from the crate top (new private copy of UI slot 11) to the ledge in
front works only when Duke is lined up square; be more forgiving.

The original geometry-ledge facing test `0x8007eaf0` compares the wall normal
(from `+0x194`) with the body heading `+0x24` and accepts within +-0x200 (45
degrees), then squares Duke to the wall; the probe points along `+0x1c`. In
Modernized the body follows the camera in flight, so a jump at a slight angle
plus any look round leaves the body outside 45 degrees although Duke flies
straight at the ledge. Measured: jump line about 26 degrees off the normal,
camera turned after takeoff: +20..+65 caught, -35, -50, -65 missed.

Change: the reach retry (`ledge_reach_lift()`) now also turns the heading
(`+0x1c`/`+0x24`) by +-290 and +-580 (about 25 and 51 degrees) for the original
acquisition, at the original height and each lift (19 tries at most per
update); a miss restores both. On a catch the original squares `+0x24` to the
wall and the probe yaw follows.

Evidence (binary `8bb0138554bd3d5f51c70387cb973f77dca577d6669e75b1f4886e12e8651701`): camera turned after takeoff -35 and -50 now catch
(only -65, body about 90 degrees off, still misses); approach sweep from the
crate catches from -15 to +15 (-15 missed before); +20/+25 and -20 hit wall
outside the ledge section (full-height wall / past its end). Without E: no
catch. Crate mantles, stack release, slot-12 wall and alley ladder transfer
unchanged. Native retry test updated (order, restore of heading).

## Crate-to-crate mantles from any angle (2026-10-01)

User: mantling onto a crate from another crate (new private copy of UI slot 11:
Duke on a crate next to a second-tier crate 1024 higher) is low confidence,
especially at an angle; and the mantle confidence must be game-wide.

Findings: the crate mantle only fired on a bounce (107) roughly facing the
crate; at an angle the reach (109) slides along the face without a bounce; a
crate beside the one Duke stands on is hit just after takeoff with its top
about 1140 above his feet (beyond the 134..138 mantles); during a bounce the
velocity is already reversed; at contact Duke's centre is ~370 off the face
(his collision radius).

Change (`ledge_reach_mantle()`, still generic: original object flags 0xc0,
the object's own box via the 0x80073ae8 rules for box lists, multi-box models
and single model boxes, no room-specific data):
- runs through any E jump or fall (97/98/103/104/108/109 in mode 9) and the
  first updates of a bounce, not only the bounce;
- proximity against the object's rotated box (Duke's centre within 450 of a
  face), moving toward it within about 75 degrees (heading during a bounce);
- skips an object with another object sitting on it (a stack: no room);
- every jump mantle first runs the ground action's original line-up
  `0x8007ec4c` (0x113) on the box, which places and squares Duke; if the
  original refuses, no mantle;
- top 0x100..0x33f above the feet: the height mantles 134..137; up to 0x4c0:
  the original full climb 139 (138's band does not complete from mid-air).

Evidence (binary `ccb4eb2aa39aed322acd53a4742832a6bdbc435b8fb7ed006c81381168e90107`): crate-to-crate from the user's spot, walking and
running jumps at -45, -30, -15, -5, 0, 5, 15, 30, 45 degrees: 18/18 end on the
higher crate (before: only straight-on, and not reliably). Without E: no
mantle 6/6. Floor to crate 3/3; aligned stack: brief catch and drop, modern
controls kept 3/3; alley object on the first map (another level area) 6/6 on
top; slot-12 wall grab and alley ladder transfer unchanged. Native: mantle
choice incl. 139, line-up refusal, refusals as before; suites PASS.

## Flush hang after a jump catch (2026-10-01)

User: mechanically superb, but Duke sometimes hangs with his hands grabbing the
air above the ledge until a shimmy re-aligns him.

Measured: the original catch keeps the height at which the probe found the
ledge. On the slot-12 sewer wall a +160 lifted catch hung at ledge top
(`+0x1c8` in the hang) + 294 and my old easing raised him the lift distance;
an unlifted turned catch on the boxes-room ledge hung at +417. The shimmy
re-aligns the root to +454..463 on both ledges.

Change: the rise-by-lift easing is removed (a lifted catch returns to the real
height), and `ledge_hang_settle()` eases the root to ledge top + 456 over a
few updates whenever an E jump has just caught a ledge (mode 6 entered from
flight, 147..149). Evidence (binary `60161f81ab42a013ebfa326d6b80b97772b29b9d8fb051b598ddba45cbde369c`): hang now +456 (sewer) and +452
(boxes ledge), against +454..461 after a shimmy; captures show the hands on the
ledge. Pull-ups, crate-top ledge catches 5/5, crate-to-crate 8/8 unchanged;
native settle check; suites PASS.
