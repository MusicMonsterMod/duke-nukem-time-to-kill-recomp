# D08T3 - Manual push/pull and supported edges

Status: Done. Selected and user-accepted 2026-10-23. Explicit documentation,
commit and push closeout authorized.

Accepted executable:
`1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.

## Player behavior

Hold Grab (RMB, or Alt) to take hold. W/S move the object along its original
cardinal axis, relative to the camera. Releasing movement stops the object;
Grab remains held. Reverse without letting go. Releasing Grab ends the
interaction and restores the weapon. A blocked object stays held, allowing a
reverse. E still leaves the grab and mantles a climbable object.

Movement stops where the object's footprint loses level support, meets a wall
or another collision box, or Duke would lose standing clearance/support. There
is no falling-object simulation. Existing fixed-by-design grab restrictions
remain. Vanilla uses the original shove.

The player's slot 8 already records a floating block. This change prevents new
unsupported movement; it does not relocate objects in existing saves. Use slot
9 to test the approach to that edge. All supplied player files are unchanged.

## Investigation and implementation

Private copies of current UI slots 8-12 live under ignored
`recomp/analysis/d08t3-20261023/cards/openbios/`; file indices are 7-11.
`source-hashes.json` records their identities. Slots 9-11 contain type 924
symbol blocks; slot 12 is the type 165 alley dumpster. Slot 8 records the
unsupported result for slot 9's block.

The original grab is animation 121. Push 120 / pull 119 run a whole clip;
`0x80048e84` integrates Duke's root and derives object travel from a hand/bone
coordinate. `0x800910a0` applies that horizontal displacement, updates the
object's room membership through `0x80095cb0`, and requests sound `0x7008`.
That displacement path has no destination support check. The previous modern
adapter ran the fixed shove even after the directional key was released.

`recomp/src/ttk/push.inc` now owns travel after the original grab settles:

- At the existing player animation hook, select hold/push/pull and loop the
  working frames of all four original animation channels. No generated C edits
  or new entry hooks.
- At the existing ground dispatcher, suppress original root and bone travel,
  then move Duke and the held object together in steps of at most 16 units.
  Speed follows guest simulation ticks, not redraw frequency. Axis and object
  rotation remain the original line-up's.
- Read the object's rotated collision box. Original room floor/clearance
  queries check nine footprint points; original wall rays check the corners
  at upper and lower heights. Support tolerance is 48 units, footprint samples
  are inset 16 units. Duke also needs standing clearance and support. Nearby
  collision boxes stop overlap; the original non-colliding type bit is honored.
- Original object room membership and positional push sound still run when
  movement succeeds. Query fields, stack and scratch are restored through the
  existing guarded call helpers.
- Release, capture epoch changes and pause restart the original hold/release
  channels and clear travel. World changes discard host pointers without
  writing to the new world. Loading a saved moving pose without Grab ends it.
- New resident code guards cover the original push dispatcher branch and the
  object room-update routine. `DNTTK_PUSH=original` is a diagnostic comparison;
  the player defaults to manual movement. `DNTTK_PUSH_TRACE=1` logs step results.

No framework source, pins, original media, player settings or player saves
changed. Existing framework modifications remain untouched.

## Evidence

Build and native `ttk-controls-test`, `ttk-input-test`, `ttk-aim-test` pass.
The added controls fixture exercises short travel, neutral hold, reversal,
unsupported-floor and wall refusal, query-field restoration, capture-epoch
cleanup and Vanilla isolation. Existing grab/floor gates remain covered.

Real keyboard/mouse tests use private Xvfb displays, private profiles/cards,
OpenGL and dummy audio. Evidence scripts and captures are in the directory
above; they are local research, not distributed game assets.

| Check | Observation |
| --- | --- |
| Slots 9, 10, 11, 12 | Short W/S movement, neutral hold and reverse work on symbol blocks and dumpster |
| Slot 9 supported edge | Block X 11760 -> 8700; another second of held W makes no change; S returns it to X 9120 without regrabbing |
| Slot 8 existing result | Same block at X 7125, Y 5632; inspected, not relocated |
| Slot 11 long push | Stops at X -9340 from -13840; holding still retains grab |
| Slot 12, first-person preference, 120 FPS setting | Short push/pull and sustained movement; original third-person interaction view retained |
| Grab release during movement | Object position remains unchanged after release |
| Pause/resume | Object stops; old shove animation ends; weapon and selected first-person view return |
| Focus loss/return | Object stops and original idle returns |
| Private save made mid-push | Loading without Grab returns to idle and does not move the object |
| Repeated loads | Slots 10, 11, 12, 9, 11 load in one process after the interaction tests |
| Grab -> E mantle | Alley dumpster enters original mode 8 and Duke reaches its top |
| Vanilla, original pad route | A short directional input still completes the fixed shove: dumpster X 2582 -> 3714 |

An initial candidate allowed Duke's old push animation to continue after pause
although the object stopped. Corrected capture-epoch cleanup is covered by a
native assertion and the final live route. An earlier multi-load run timed out;
subsequent repeated-load routes completed. No exact cause is claimed for that
earlier timeout.

## Review revision - held input and ladder cutoff

The user rejected the first candidate: holding W while taking hold could run
one original shove and release, and the dumpster could pass through its ladder.
Both reports reproduced on private copies. The earlier live tests waited for
grab to settle before pressing W and therefore missed the timing failure.

The animation update can complete grab between the prepare and dispatcher
hooks. The dispatcher now acquires manual ownership at that transition before
the original code consumes held Up. A native regression simulates that exact
hook order with movement already held.

Ladders have zero-depth collision panels, which the volume-only box reader
skipped. Push collision now includes those panels with a small thickness.
Duke's nearby-object list also discovers the ladder too late for the dumpster's
front: the first panel-only revision stopped at X 8612, already overlapping it.
The final revision additionally walks the complete room object lists used by
`0x80095cb0` (table `0x800da454`, count `0x800da47c`, 88-byte records, list +0x48).
It stops at X 8418, with the dumpster's front at X 9216 before the ladder panel
at X 9225, and permits reversal while Grab remains held. A native regression
puts the flat ladder in a room list while omitting it from Duke's nearby list.

Revision evidence is under ignored `recomp/analysis/d08t3-review-20261023/`:
`baseline.json` reproduces the failures; `revision.json` tests the timing and
panel change; `verify.json` records the final room-list candidate. Build and
controls/input/aim native suites pass. Final real-key routes on slots 9-12 pass held-W acquisition, neutral hold,
reversal, nine seconds of resumed pushing plus a further one-second boundary
hold, reversal from that boundary, and release with no further object travel.
The stops are X 8418 (slot 12), 8700 (slot 9), 15734 (slot 10) and -9340
(slot 11). Final captures include `verify-slot12.png` showing the ladder stop.
The 15 source save/sidecar/thumbnail
hashes for slots 8-12 remain unchanged. Earlier evidence above belongs to the
first candidate unless explicitly retested here; it did not establish the
held-W handoff or ladder cutoff.

## Final user acceptance and separate follow-up

User accepted the final revision after further testing across slots 9-12 and
explicitly requested closeout, commit and push:

| Slot | Final user finding |
| --- | --- |
| 12 | Dumpster pushes freely toward the ladder and stops correctly |
| 11 | Box push and pull work throughout permitted travel |
| 10 | Cube stops against an enemy and remains stable |
| 9 | Block stops at the ledge instead of floating beyond support |

D08T3 is Done on this acceptance and the recorded regression evidence. No
additional gameplay changes or rebuild were made during closeout. Authored
implementation, tests, manual and evidence are included in the authorized
MusicMonsterMod commit/push. The runtime patch was re-exported unchanged and verified against pinned clean
dependency `08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 patched files match.

A separate user-reported softlock remains: push slot 12's dumpster to its
limit, release it and mantle onto it. Low overhead clearance forces crouching,
and Duke cannot crouch-walk off the edge. This is **D08L1 (Todo)** in
[the job board](../MODERNIZATION_JOBS.md#d08l1---modern-locomotion---allow-walking-and-crouch-walking-off-ledges),
covering Duke's standing and crouched walk-off behavior. Its cause is not yet
established. It is not a Free Push defect, and no implementation was started.
D08L1 must preserve the accepted box collision and support restrictions.

Coverage limits remain: supplied objects and user tests do not establish every
campaign puzzle; automated dummy-audio tests do not establish sound quality.
Support checks do not add gravity, stacked-object motion or rescue an already
unsupported block. First-person grabbing retains the established third-person
interaction view. These are evidence limits, not unaccepted Free Push work.

Launch with the player's saved preferences:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Capture is automatic. No F10 activation step is required.
