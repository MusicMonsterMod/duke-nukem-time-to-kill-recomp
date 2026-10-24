# D08T7 - Regrabbing movable objects along their lane

Status: Done, user-accepted 2026-10-24. User selected implementation after D08T6.
Explicit documentation, implementation commit and push closeout authorized.

Accepted executable: `0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.

## Implementation

`push.inc` now separates extra acquisition permission from the original floor
bit. A small placement policy authenticates the active overlay, object type,
height, lane Z and rotation for the LEVEL00 alley dumpster and the three
LEVEL06 symbol-block lanes in recent slots 9-11. X is the travel coordinate,
not identity. No RAM address, UI slot number, remembered prior grab or new
save sidecar is required. A loaded boundary save is eligible immediately.

Unlisted placements retain the original floor rule. In particular LEVEL05's
fixed type-165 dumpster is not made movable. This is a bounded set of verified
placements, not a campaign-wide mobility classifier or an editor implementation.
Future placements can extend the data independently of acquisition/collision.

At the existing idle-action hook, only a fresh Grab-owned Cross, holstered
idle, clear original action flags and no attached use object can attempt the
extension. The guarded original `0x8007ec4c` aligns Duke. The resulting target
facing must be along the X lane, and the existing footprint/player support,
clearance, wall and object checks must accept the stationary aligned position.
Success enters the original grab animation 121 and records its object just as
`0x80051890` does. Failure restores all player alignment/query fields. The same
press is masked from the original action handler to prevent duplicate actions.
Floor records, original media and generated C are never modified.

Held movement, speed, animation loops, stop/reverse, room membership and sound
remain D08T3's. The X 7292 ladder cap remains D08T5's. No object is repositioned
on load; a supported older beyond-limit placement can acquire and retreat.
E/mantle behavior is unchanged; D08T8 remains separate.

## Verification

Local private evidence: `recomp/analysis/d08t7-20261024/` (ignored).
Private profiles/cards, Xvfb, OpenGL, dummy audio, real keyboard/mouse input.
Source copies and baseline executable were preserved before testing.

- Build succeeds; controls/input/aim native suites pass. Controls include
  boundary acquisition without prior host state, beyond-limit acquisition,
  original line-up rejection, wall/support/axis/state/capture/Vanilla refusal,
  unchanged floor bytes, and lane eligibility across all 21 level guards.
- `regrab.json`: old boundary save acquires at X 7292, pulls back, regrabs at a
  midpoint, returns to exactly 7292, saves privately and reloads there, then
  reacquires and pulls back again. Actual animation and displacement asserted.
- `matrix.json`: recent slots 9-11 reach stops, release, reapproach, reacquire
  and reverse. Slot 9 stops at X 8700 and slot 11 at -9340. Slot 10's stop is
  dynamic because an enemy is an obstacle; no fixed coordinate is asserted.
- `east-fixed.json`: corrected real-input approach grabs the dumpster from its
  ladder-side end at X 7292 and pushes it back to 6752. The historical level-5
  fixed dumpster remains idle 63, weapon drawn, at its unchanged position.
- `first120.json`: first-person preference, 120 FPS setting, slot 12 initial
  position to X 7292, release/reapproach/regrab, reverse, exact return, pause with
  no object movement, resume and reacquisition. Interaction keeps its established
  third-person view; final framebuffer inspected (`boundary.png`).
- All 40 files in the intake source directory match their SHA-256 manifest.
  Only private test copies were loaded/written. No framework edits or patch
  export were needed; the pre-existing modified dependency was preserved.
- Native coverage retains held reverse, supported edges, flat ladder panels,
  Vanilla isolation, pause/capture cleanup and guard refusal.

Initial test-route limitations are retained in local logs: attempt 1 repressed
Grab without returning to contact after release; the original release animation
had moved Duke back. The corrected route explicitly reapproaches. The first
opposite-end route turned before approaching and missed the dumpster; it is not
an acquisition failure. A local harness file named `inspect.py` also shadowed a
Python standard module during exception reporting; it was renamed. These
attempts do not count as passing routes.

User accepted the feel as "absolutely solid" after several tests and explicitly
authorized locking in D08T7. Broader approach/placement coverage remains an
evidence limit, not a claim that every campaign object was tested.
The native older-beyond-limit check is not a fresh live test of every legacy save.
Dummy audio does not establish sound quality. No new falling-object physics,
unsupported-save rescue, all-object free movement or mantle fix is claimed.

Launch with saved player preferences (automatic capture):

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

## Acceptance and separate investigation

User requested a backlog-only investigation of the UI slot 11 load-time fatal
halt, then commit/push to lock in the most recently completed task. D08T7 is
Done on that instruction. D20G is Todo for fresh context; see note 213. No causal
relationship between the halt and D08T7 has been established. D08T8 is unchanged.
No gameplay edits or rebuild followed acceptance. Runtime patch re-export and
clean-pin verification: export unchanged; all 51 files match pinned clean
dependency `08ec704a974b1f3a16335b4afeb340b9eff19926`.
