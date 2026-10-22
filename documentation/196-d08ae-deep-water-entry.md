# D08AE - Restore deep-water entry resistance and timely swim handoff

2026-10-22. Status: Done, user-accepted.

User: "Fully accepted as working, tested in the appropriate locations, and
again in shallow water, all solid. lock this job in."
Implementation and documentation commit/push explicitly authorized after
adding the unrelated D17R2 sky backlog. The executable below is the accepted
regression baseline. Framework export is unchanged and verified against the
pinned clean dependency (all 50 patched files match). No rebuild needed for
acceptance closeout.

## Accepted correction

`grounded_water.inc` now requires the native local water volume depth
(`player+0x838`) to be at most 640 as well as the existing support-based
shallow-water predicate before protecting an airborne dispatch or transition.
The final nested-hook checks reapply the same gate after native water queries.
Grounded gait and stride-completion eligibility are unchanged. There are no
new hooks, velocity writes, drag constants, water-mask changes or framework
edits. Native descending body/neck checks, entry animation 127, effects and
swim dispatch regain ownership in deep water.

The native query `0x8005b85c` calculates a local top/bottom interval and
consults geometry through `0x8005d358` before storing its depth. The added
check supplements the support predicate; it does not replace all grounded
classification with a volume-only rule. Deep entry itself still requires
the native body, neck, direction and animation conditions.

Accepted executable SHA256:
`c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.

Private tests/evidence remain in `recomp/analysis/d08ae-research/` and
`recomp/analysis/d08ad-lab/`. The six initial candidate entry routes reproduce
the earlier comparison: running-jump entry is at sampled body depth 380 in
the pool and 577 in the moat, matching the earlier ORIGINAL samples, instead
of 2452/4378. The other walk/run-off entries span 352-589.

A further 24 entry trials cover 60 Hz assisted/first-person selection,
120 Hz manual/first-person selection and 120 Hz assisted/third person,
both locations, held direction/Space and released run-jump input. Every trial
entered swimming at sampled body depth 352-589; the air-steer counter stayed
constant throughout subsequent swimming. Traces include body, foot and neck,
water surface, native volume, support, modes/animations, velocities and input.
These are sampled observations, not exact simulation-boundary timestamps.
Some assisted run-jump attempts at the moat reached the edge as run-offs;
this distinction is retained in the animation traces. First-person swimming
still uses its existing camera fallback (D11E is a separate open job).

Native controls tests pass with all 21 level overlays, along with input and
aim suites. Added contract cases cover shallow/deep thresholds, support equal
to surface, refresh from shallow to deep within either native scope, preserved
water identity, and grounded stride selection inside a deep volume. Live regressions confirm complete walk clips 72/74 (frames 16-19/32-35)
and run clips 76/78 (105-107/117-119) at 60/120 Hz. Dive, underwater
movement/fire (three shots), ascent to surface mode 4, crouch/stand-up,
SWIMMING ORIGINAL and Vanilla were checked separately on private copies.
Five hook-contract tests also pass.

A controlled shelf case extends the native water record's bottom far below
the local floor: the native query still clips the local volume to 512.
Neutral/directional/run jumps and crouch remain on land, demonstrating why
the queried local depth matters more than the authored region's full depth.
The updated slot-10 route also slopes from shallow toward deeper water;
raising its surface makes a run cross 640 into 768 before landing, where a
native swim transition is expected. An initial blanket 'never swim' harness
assertion was rejected on those recorded depths. Older hard-coded-region
physics runs did not alter the current save's water and are excluded from
the depth-boundary evidence. The corrected harness resolves `+0x83c` and
asserts the requested starting depth before each trial.

The corrected depth harness passes 24 routes: starting depths 128/512/640,
neutral/walk/run jumps and held-crouch landing, manual/third person at 60 Hz
and assisted/first-person selection at 120 Hz. No sample swims in a queried
local volume of 640 or less. Additional moat trials with earlier Space presses
confirm actual assisted jump clips 98 and 103 before native swim entry, not
only run-offs. Final build, native controls and hook contracts pass.

The high-refresh controlled crouch/depth fixture logs 38 temporary identity
refusals for the existing owned crouch branch at `0x800548e8`; the same branch
warning occurs in the pre-D08AE `d08ad-lab/physics120.log`. It is not a new
water-entry code change and was not silently fixed in this job. Grounded
animation cycles and the deep-entry matrix have zero reported refusals.

All 39 files in the original player manifest have unchanged hashes. Dummy audio establishes native effect
routing, not audible splash quality. User playtest confirms the two reported locations and shallow-water behavior;
broad campaign shorelines, object shelves and
all exit geometries are not exhaustively established by these routes.

## Original investigation (before implementation)

## Report and reproduction

On accepting D08AD's animation fix, the user reports that jumping into deep
water feels like jumping through air, with no resistance. Reproduce from
UI slot 9 by jumping into the pool ahead, and UI slot 2 by jumping into the
moat. Preserve the accepted shallow/mid-water locomotion, jumps, crouching
and complete third-person animation cycles.

Private copies of both current saves were tested offscreen on accepted
executable `9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
UI slots 9/2 correspond to files 08/01. Evidence is local/ignored under
`recomp/analysis/d08ae-research/`: paired-modern/original traces and screenshots.
The harness checks travel direction toward the water after loading, since
host camera recapture can change it. Early slot-9 jump trials went away from
the pool and are excluded from the paired findings below.

## Observed findings

Both requested locations reproduce an abnormally late airborne-to-swimming
handoff with SWIMMING MODERN. Running jumps, walking jumps and running off
the edge were sampled at 60 Hz with the same private settings apart from
SWIMMING MODERN versus ORIGINAL. ORIGINAL here is the swimming option in
Modernized, not the Vanilla profile.

Representative running-jump traces (world Y increases downward):

| Location | SWIMMING MODERN | SWIMMING ORIGINAL |
| --- | --- | --- |
| Slot 9 pool, water volume depth 3072 | Native mode 9 remains active down to body Y 2452 below surface; first sampled mode 5 also at 2452 | Mode 9 last sampled at 266 below surface; mode 5 first sampled at 380 |
| Slot 2 moat, water volume about 4851..5120 on this route | Mode 9 remains active down to body Y 4378 below surface; first sampled mode 5 also at 4378 | Mode 9 last sampled at 339 below surface; mode 5 first sampled at 577 |

In the pool running jump, downward velocity rises from 10586 to 18406
while the body is submerged and mode 9 remains active. In the moat it rises
from 10816 to 22776. Approximate sampled submerged-airborne spans were
0.57 s and 0.87 s respectively. These are observations from wall-clock RAM
sampling, not exact transition timestamps or a calibrated drag measurement.
Walking jumps and run-offs show the same late handoff: pool maximum sampled
airborne body depth 2521..2533; moat 4095..4519.

The key classification mismatch: during the submerged airborne interval,
`player+0x1c8` (cached support height) equals `player+0x834` (water surface),
so support-minus-surface is zero. Meanwhile `player+0x838`, the native water
volume depth, is thousands of units. The support cache is not a reliable
physical bottom-depth measurement during this entry.

## Source findings and likely cause

- `modern_controls.cpp::shallow_land_water_early()` excludes native modes
  4/5 but accepts mode 9 if water is present/submerged and cached
  support-minus-surface is less than 641. It has no actual grounded-state
  requirement. `grounded_water_modern()` uses this predicate when SWIMMING
  MODERN is enabled.
- `grounded_water.inc` deliberately applies shallow landing protection
  during flight. Its transition scope bypasses native 0x80056240, suppressing
  neck/body-based swimming entry inside 0x80055e80. Its dispatcher flight
  scope bypasses 0x80054cbc, which otherwise compares volume depth +0x838
  with 0x280 and enters mode 5 / clip 127 for deep water.
- Both scope checks use the cached support-based predicate, including the
  later rechecks at the neck lookup and landing sound call. Refreshing that
  predicate does not help when the refreshed support still denotes the
  water surface rather than the bottom.
- This strongly implicates D08AC's shallow-water flight classification and
  entry bypasses in the delayed handoff. Switching to ORIGINAL disables
  that policy and restores much earlier entry in both paired locations.
  A targeted fix or patch-isolation experiment was not performed, so the
  exact minimal correction remains to be established.
- D08AD's new completion selector only admits grounded modes 0/1 and clips
  72..79. The two flight bypasses predate D08AD. Do not attribute introduction
  of the regression specifically to the animation fix without a historical
  binary comparison. The user reported it during D08AD acceptance.
- `manual_jump.inc::manual_air_control()` acts during an owned ballistic
  update without its own water gate. It may retain air steering while the
  native swim handoff is delayed. This is a secondary consequence to trace,
  not evidence that changing the air-control constants would fix entry.

## Future implementation scope and acceptance

Investigate and correct the distinction between grounded shallow water and
an airborne body entering a genuinely deep volume. Restore prompt native
water-entry response/resistance and relinquish airborne ownership to the
swim state at the appropriate boundary. Establish that boundary using native
water volume, geometry and body/neck checks; do not merely add a guessed drag
multiplier or globally remove shallow-water jump protection.

Acceptance requires:
- UI slots 9 and 2: walking/running jumps and run-offs enter deep water with
  a convincing water response, without continuing an air-like ballistic
  descent almost to the bottom. Trace animation/modes, body/foot/neck heights,
  support/surface/volume, velocity and airborne ownership before/after entry.
- Compare native entry behavior via SWIMMING ORIGINAL/Vanilla while keeping
  Modernized's accepted post-entry swim speeds and controls. Test held and
  released movement/Space, manual and assisted jumps, both cameras, 60/120 Hz.
- Preserve D08AC grounded shallow/mid-water jump arcs and landing, D08K11
  crouch, D08AD full walk/run cycles and smooth depth transitions. Include
  shallow shelves inside deeper volumes, slopes, pool edges, exits and
  jumping from shallow into deep water; volume depth alone may be insufficient.
- Preserve surface/underwater movement, dive/ascent, swim-fire, water effects,
  ORIGINAL semantics and Vanilla. Require user playtest of entry feel.

Dependencies: D08AC and D08AD (accepted). Related: D08AA/AB shared swim pace
and option, D08O native swimming, D08Z air control, D08K11 crouching.
No gameplay code or binary changed during this investigation. The new job
remains Todo until the user selects implementation.
