# D08T6 - Push/pull lock-zone investigation

Status: Needs review. Research and backlog scoped on 2026-10-24.
No gameplay implementation, build or new live test in this pass.

## Requested behavior

The user considers the current game playable. This is a flexibility and feel
improvement: a movable object should remain grabbable and reversible throughout
its permitted lane, including after releasing it at the end. Preserve the
imaginary rail and its guardrails. Slot 12's alley dumpster is the main example.
Owning the interaction could support future level editing, but no editor or
free-physics system is requested here.

Separate report: walk into the dumpster's side, press E, then W. Duke assumes a
mantle/hang pose but does not climb; Space does not drop him, S does. This exact
report has not been reproduced or assigned a cause in this pass.

## Evidence reviewed

Read current push/input/traversal source, native floor-gate assertions, notes
163, 208 and 210's recorded findings (163 through the board's detailed account),
and the retained D08T5 JSON and output logs. These are historical live results,
not fresh tests or proof that every recent slot has the same restriction.

Local evidence directory: `recomp/analysis/d08t4-20261023/` (ignored).
Its name predates the assignment of D08T5; do not mistake it for the level-5
fixed-dumpster D08T4 investigation.

| Evidence | Result |
| --- | --- |
| `verify.json` | Slot 12 dumpster X 7292 at boundary, held reverse to 6662, return to 7292; grab animation 121 |
| `first120.json` | Same boundary/reverse/return with first-person preference and 120 FPS |
| `regrab.json` | Three attempts at boundary: grab remains idle 63; S becomes walking 74; dumpster stays X 7292 |
| `regrab-baseline.json` | Same refusal on pre-D08T5 accepted D08L1 executable |
| `regrab-baseline-output.log` | Recorded floor at 0x8017be4e, bytes 6800ff0000850568: byte 4 is zero |
| `verify.json` slot 9 | Block stops at X 8700, stays held and reverses to 9240 |

Note 208 also records held movement/reversal for slots 9-12. That establishes
held travel, not release/reacquisition coverage at every destination. Historical
slot numbers are not stable object identities: D08T4's old slot 10 was a level-5
fixed dumpster; the recent slot 10 is a type-924 symbol block. Match save hashes,
level, type and placement before using any fixture.

## Ownership and cause boundary

1. **Eligibility:** `push_contact_ready()` in `push.inc` requires a pushable type,
   grounded valid state/contact, and `push_floor_allows()`. The latter reads bit
   0 at byte 4 of Duke's current floor record (`*(player+0x48)`). It describes
   where Duke stands, not an immutable mobility property of the object.
2. **Acquisition:** the original idle-action routine `0x80051890` still performs
   floor gating and original line-up (`0x113`) into grab animation 121. Removing
   only the host prompt gate would not establish a successful original grab.
3. **Held travel:** D08T3 owns incremental movement, working animation loops,
   stop/reverse and displacement suppression. It uses the original cardinal
   line-up axis and calls original room membership/sound routines. The floor
   grab bit is not checked by `push_step_clear()` or the held-state predicate.
4. **Travel constraints:** object footprint support, walls, other collision
   boxes, flat ladder panels, Duke support/standing clearance, and D08T5's
   positive-X cap at 7292 remain independent checks. Reverse is permitted by
   the cap, including older saves already beyond it, subject to collision.
5. **Release and transitions:** host ownership clears on release/world changes;
   original hold/release and mantle handlers remain involved. We therefore own
   much of travel, but not the entire push/pull and climb stack.

The recorded boundary regrab refusal is consistent with the explicit floor
eligibility gate, and that gate was documented during D08T5. It is not evidence
that the object loses its type flag or that the travel limit itself prevents
reverse. The user's broader observation still needs a fresh matrix separating
continuous hold, release/regrab, opposite-face approach and reload.

## Proposed D08T7 implementation boundary

Separate object mobility from the floor permission used to acquire it. Identify
an authenticated movable placement and its permitted axis/lane; allow safe
reacquisition there even when Duke's current floor lacks the original grab bit.
Preserve collision, support, alignment, prompts and the accepted ladder limit.

Do not globally set floor bits or treat every type-165 dumpster as movable:
D08T4 established a fixed level-5 placement with the same type flags. Do not use
raw RAM addresses or UI slot numbers as persistent identity. A session-only
"previously grabbed" latch is also insufficient for loading an object already
at the boundary. Determine stable placement provenance and save/load behavior
before choosing a cache, sidecar or explicit placement policy. This is a design
recommendation, not an implemented metadata format or a proven generic rule.

First increment should diagnose eligibility, line-up and held-step refusal
separately, then extend acquisition for verified movable placements. A wholesale
rewrite of original animation/mantle logic is not necessary to test that design.
Future editor-authored mobility metadata can build on it as separate scope.

## D08T8 mantle investigation boundary

Relevant paths are `push_obstacle_action()` / original `0x80051cf0`,
`traversal_input_ready()`, input pad routing and `object_hang_update()` in
`ledge_reach.inc`. Existing object-hang release specifically recognizes mode 7,
animations 149-151 and the attached object's climb flags. Do not assume the
reported pose is that state: obtain mode/previous mode, all animation channels,
contact/attachment pointers, pad input, clearance and E/Grab ownership first.

Compare long side and ends, original and moved placement, tap/held E, W release
and repress, Space and S independently, armed/stowed and both view preferences.
Use private copies and an original-controls comparison. Establish whether W is
missing, the original pull-up rejects geometry, or an animation/ownership state
is stale. Preserve successful mantles, safe blocked-headroom behavior, D08L1
crouched escape, ladders and ceiling hangs. No causal link to the floor grab bit
has been established.

## Next validation matrix

- Slot 12 and slot 7 reference: initial position, midpoint, X 7292, and a private
  older beyond-limit save if available. Hold reverse; release/regrab/pull; leave
  and return; save/reload; both reachable lane ends. Assert actual acquisition
  and signed object displacement, not merely unchanged object position.
- Recent slots 9-11: same tests on symbol blocks, including supported-edge and
  enemy stops. Slot 8 is already unsupported and is an inspection/control case,
  not permission to relocate it or add object gravity.
- Fixed level-5 dumpster: remains fixed with no misleading Grab prompt.
- Pause/capture loss/load while interacting, camera-relative controls, first
  and third person, 60 and high refresh, Vanilla unchanged.

Research delivers the ownership map and bounded implementation jobs. Full lane
mapping, new live reproduction and gameplay changes remain in D08T7/D08T8.
No player files, game source, framework changes, builds or settings were written.
Existing modified framework worktree was preserved. No commit or push.
