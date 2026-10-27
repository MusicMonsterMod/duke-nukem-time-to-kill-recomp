# D08Z6 - Forward carry after a blocked running jump

2026-10-24. Done, user-accepted revision 2.

## Acceptance and closeout

User accepted revision 2: "this is perfect! lock it in and close it out!!!!"
D08Z6 is Done. Accepted behavior includes forward carry after clearance and
first-tap response during the grounded stop. Earlier candidate entries below
are the implementation/test history, superseded as status by this acceptance.

Accepted executable: `e250b5b5ec43c2a2a8fd1c39e6ab414b89d90d3b0329979dbdb42ba05a7fa78f`.
No gameplay edits, rebuild or new live testing after acceptance. Existing
bounded tests and the separate diagnostic reload halt retain their stated
limits; acceptance does not establish an unrelated halt cause or exhaustive
campaign coverage. Runtime patch re-exported unchanged and verified against
clean pinned framework `08ec704a974b1f3a16335b4afeb340b9eff19926`: all 51 files
match. Framework/UI pins unchanged. Source, tests, manual and documentation
are included in the explicitly authorized MusicMonsterMod commit/push.
No next job started.


## Reproduction and cause

Private copies of the current player saves reproduce the UI-slot-10 report
in LEVEL06. Player settings select manual jumping, slide wall contact,
Modern controls and first person. No player settings or saves are written.
The middle approach and a short right strafe followed by Shift + W stop at
the step. Space selects preparation 96, vertical jump 97, then the existing
clearance adapter permits directional jump 98 as the feet clear the lip.

The accepted build seeds manual steering from that standing handoff's
horizontal speed, approximately 2734, for the entire flight. An ordinary
running jump at this location starts at approximately 10100. The low cap,
not an absent forward input, explains the measured loss of forward carry.
At a nearby right-corner approach the original can select 98 directly;
that separate ordinary directional jump remains unchanged by this patch.

Existing guards for 0x8005492c (standing handler), 0x800797f4 (clearance),
0x80055904 (airborne handler), and 0x8003ebf4 (ballistic integration) match
the owned US executable. The live LEVEL06 controls identity passes. No new
addresses, overlay hooks, generated C or framework changes are required.

## Candidate behavior

In modern_controls.cpp, only the existing cleared 97 -> 98 handoff can
raise the manual air-steering cap to 10000. It requires the current owned
takeoff/epoch, active directional input, running, manual jump style, no
precision aim and dry ground. The original initializer's horizontal speed
is retained, then ordinary bounded air steering accelerates toward the cap.
The first nominal update changes 2734 to 4134, not directly to 10000.

The existing support-top and forward-clearance tests still decide when the
handoff is allowed. No position, clearance, gravity or vertical impulse is
added. Original swept collision and the existing wall-slide solver still
run after steering. No movement input means no added acceleration; releasing
before clearance leaves the original vertical jump. Walking, assisted jumps,
ordinary running jumps and swimming do not receive this promotion. Vanilla
continues through its existing unchanged path.

## Verification

- Player build, controls, input and aim suites pass. Controls includes the
  owned executable, clean LEVEL00/LEVEL01 and all 21 level overlays.
- New native cases verify refused clearance, delayed vertical preservation,
  bounded acceleration, unchanged position, release, walking and assisted
  exclusions. Existing suites exercise wall slides, mantles, landing,
  stale ownership, code identity and Vanilla rejection.
- Six initial candidate routes cover middle/right/left, open run, release
  and steering. Four additional nearby right-corner trials all reach the
  upper surface with a 10000 horizontal speed cap.
- Sixteen further routes cover first/third person at configured 60/120 Hz:
  middle and saved right-corner held runs, early release and early diagonal
  steering. All eight held routes have matching paths across cameras/rates.
  Early release/diagonal input can deliberately retain vertical jump 97;
  the existing forward-direction clearance gate is unchanged.
- A separate accepted/candidate comparison uses the identical private saved
  right-corner position, held input, release after clearance, and lateral
  steering after clearance. Additional routes use player UI slots 4, 6 and 7.
  See local analysis/d08z6 for traces, screenshots and test logs.

The matched right-corner held comparison ends airborne at X 7818 in the
accepted build and X 9950 in the candidate, reaching the upper surface.
Both have the same minimum root Y, 4952. Release after clearance retains
momentum; lateral steering changes the flight direction. The UI-slot-4
running route and UI-slot-6/7 blocked obstacle routes have identical sampled
airborne start/end positions, apex and final positions in both builds. The
latter two remain vertical jumps; neither is forced through its obstacle.

All 40 player save/card files retain their initial SHA-256 hashes, and
the player profile is byte-identical to the intake copy. Diagnostic games
are stopped and the tested candidate is restored after comparison.
Candidate executable SHA-256: `eff4617cd7d7558db0f1308c3d742bbbc6d4e0655f53d5007bca8348eaa56045`.
Framework changes present at intake are preserved; no patch re-export is
needed because this job makes no framework edits. No commits or pushes.

These are bounded geometry tests. Full campaign traversal and subjective
feel remain user acceptance work, especially the right-hand corner.
The patch does not force every wall-adjacent jump onto a platform.

## Player check

Launch normally with the existing saved settings. Load UI slot 10, approach
the tall step near its right-hand corner, hold Shift + W until stopped, then
press Space while continuing to hold Shift + W. Check forward carry and
steering after clearing the lip. P switches camera views. Capture is automatic.

```sh
python3 recomp/tools/local/run.py
```

The implementation turn did not authorize commit/push; the later acceptance
above explicitly authorizes closeout.


## Revision 2 - First Space press during the stop

User reported that the first Space press did nothing and a second was needed.
Fresh-process reproduction confirms it: at 0.8 s into the slot-10 approach,
Duke is in grounded stopping animation 94. A 60 ms Space tap expires before
94 returns to idle at about 1.1 s. The second press starts 96. At 0.95 s the
same first press can survive long enough to reach idle; this is timing-sensitive,
not a requirement for two presses in every stance. Earlier tests waited 2 s
and therefore missed the stopping interval.

The existing preparation branch now also accepts a fresh manual-running press
from grounded 94/95 with a fresh camera-only lease, matching identity, normal
camera ownership, dry ground and current mode/previous mode both zero. Existing
crouch, precision aim, movement, jet thrust and flag exclusions remain. It
starts the same original 96 preparation and consumes the press once; all
clearance, vertical arc and forward-carry logic stays as in revision 1.
Assisted, walking, landing transitions and other committed animations are
excluded. No broader movement lease was granted to stopping animations.

Player build and the full native controls suite pass. Added cases cover both
94 and 95, plus assisted/walking/landing/crouch/aim/epoch exclusions. Fresh
single-tap live tests cover first/third person at 60/120 Hz at 0.8 s, plus
0.95 s and settled 2 s presses. Preparation starts within one nominal logic
update (roughly 30-40 ms), without a second press. Right-corner single-tap
routes pass in both views at 120 Hz, recorded in corner-verify.out.
All diagnostic games are stopped. The 40 player save/card files retain their
revision-check hashes, and player settings are unchanged.

One preliminary private multi-load run halted after its second load with an
unknown dispatch to 0x800c0000; its later trace is invalid and was discarded.
first-press.log and firstpress.out retain the incident. D20G note 213 was
consulted. The process had been stopped before live aperture/RAM collection;
that evidence is unavailable. Cause is unresolved and is not attributed to
this jump change. Fresh launches were used for the valid reproduction and
verification; no unrelated runtime fix was attempted.

Revision-2 executable:
`e250b5b5ec43c2a2a8fd1c39e6ab414b89d90d3b0329979dbdb42ba05a7fa78f`.
Status remains Needs playtest; no commit/push.
