# D35K2 - First-person toilet use

2026-10-25. Done, user-accepted. User selected implementation of the
confirmed toilet gap from [note 238](238-d35k-interaction-inventory.md).

## Acceptance

User: "its perfect! lock it in." The user also closes the overarching D35 and
will report further first-person drops if encountered. Future reports belong
in focused D35 child jobs; no further fix or campaign sweep is auto-started.
No post-acceptance gameplay edits, rebuilds or launches.

## Change

`modern_controls.cpp:use_interaction_eye` now admits native pose 275 under
USE INTERACTION VIEW. It retains mode/history 0/0, committed-action bit 0x40,
identity, death, script and ordinary-camera ownership guards. This is a
presentation-only exception: no locomotion/fire permission, native callback,
healing logic, animation, action timing, weapon restore or save format changed.
No new hook, settings schema, framework edit or generated-code edit is needed.
The player manual includes toilet use in the existing action perspective option.

## Validation

Player build and controls/input/aim/near native suites pass. Controls use the
owned executable, original LEVEL00 guard fixture and all 21 selectable overlay
fixtures. Pose 275 joins the transient presentation tests for action isolation,
view choice, pause/input suspension, load reset, replay and death/script/owner
guards, with additional wrong-mode/history rejection checks. Adjacent unknown
poses remain rejected.

Private natural slot-5 E routes at 60 and 120 retain blend 1 throughout sampled
pose-275 intervals, complete to idle 63, clear the action bit and redraw the
weapon. Mid-action captures were inspected. Action Third Person uses blend 0
and recovers the selected first person afterward. Global Third Person also
completes the action at blend 0 (72 sampled action frames).

The source save has already consumed the toilet's one-time effect: object+0x35
is 1 before and after. This confirms no reset of that state, not new healing
coverage. The initial 60-run harness wrongly expected a fresh flag (0 -> 1)
and failed that assertion with (1,1); its camera/completion assertions passed,
and the retained trace independently confirms 65 first-person action samples.
The corrected 120-run check passes with 72 action samples and unchanged flag.

Pause/resume and loading a private mid-action save also retain first person
and complete the original action with its effect flag intact. Vanilla loading
of that private mid-action save also completes pose 275 to idle in third
person. Fresh Vanilla activation from the supplied checkpoint did not respond
to the attempted native pad/keyboard inputs; no fresh-activation result or
cause is claimed. This is saved-action completion coverage only.


All diagnostic settings/cards are private copies under
`recomp/analysis/d35k2/`; native input is delivered through Xvfb or original
pad requests. No player save/card use, teleport, forced action or RAM patch.
Diagnostics are stopped. All 40 source player save/card hashes and modification
times match the research intake manifest. Existing framework changes and both
dependency pins remain untouched.

Original one-time healing on an unused toilet remains unverified because the
supplied checkpoint is already used. Type 179 and other unobserved pose-275
routes are not newly certified.

Accepted executable SHA-256:
`d819cd7ad757ff3f3adf04b1382393b2cbcc70d3e822f3e84754439e753d996a`.

## Player check

Launch with saved preferences:

```bash
python3 recomp/tools/local/run.py
```

Load UI slot 5, select global First Person and USE INTERACTION VIEW First
Person, then press E at the toilet. The user confirmed the result is perfect and explicitly authorized lock-in
and closeout. D35K2, D35K bounded research and the overarching D35 are Done.
Implementation, tests, manual and documentation are included in the authorized
MusicMonsterMod commit/push. Existing coverage limits above remain recorded.
