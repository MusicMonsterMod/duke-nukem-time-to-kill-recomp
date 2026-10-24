# D08T8 - Apartment alley platform pull-up and drop

Status: Done - user-accepted 2026-10-24; commit/push closeout explicitly authorized.
The reproduction below records the pre-fix executable. Verification and
limitations below record the candidate testing before user acceptance.

## Corrected report

The user confirms the dragging/regrabbing checks pass and the dumpster mantle
works. The remaining failure is the platform directly ahead after climbing the
dumpster from current UI slot 7. This supersedes the earlier dumpster-side
attribution in note 211. D08T6 remains a research ticket awaiting review;
D08T7 remains accepted.

## Fresh reproduction

Accepted executable SHA-256:
`0a4f3a5a5837e867936095107c2ae6f5f20dc33e4007ab88113ae3708e117078`.
Private copy of current player cards/saves/settings, UI slot 7 (file slot 6),
Modernized, third person, 60 FPS, OpenGL 1x, Xvfb, real xdotool keyboard input,
dummy audio, debug port 9273. No player session was running at intake.

Route: load slot 7, resume, hold W+E to climb the dumpster and reach the platform
in front. Two runs reached the same hang; the second tested each escape input.
The first diagnostic process ended with its shell before the separate follow-up
could connect, so the complete route and checks were repeated in one process.

- Dumpster climb progresses through animations 139/140 in mode 8.
- Platform hang settles in animation 149, mode/previous mode 6/6, root
  (6993, -10834, 13779), heading 1062. Screenshot confirms hands on the
  platform and the dumpster underneath.
- Continued W and a fresh W press both leave that state and position unchanged.
- Release E and W, then hold Space alone: same state and position.
- Release Space, then hold S: Duke releases and falls (108, mode 9), then lands.
- Capture remains active. W maps to original Up (active-low pad 65519),
  Space to Square (32767), S to Down (65471). This establishes host pad routing,
  not the exact original handler's reason for rejecting pull-up/Square.

Contact/use fields +0x174 and +0x17c still name the dumpster at the platform
hang. This is a diagnostic lead, not proof that stale attachment causes it.
The mode-7 object-hang auto-release path is not this observed mode-6 hang.
Do not force a climb or rewrite attachment fields without checking original
ledge dispatcher, height/headroom and transition ownership.

## Evidence and limits

Local ignored evidence: `recomp/analysis/d08t8-20261024/`:
`route.py`, `route.json`, `route-output.log`, `route.log`, per-step PNGs,
private profiles/cards and intake/source-check manifests. `held-w.png` and
`held-space.png` show the stalled platform hang. Both diagnostic game runs
have ended. All 41 source save/card/profile files match their intake hashes.

No gameplay/framework edits, build, player settings changes, commit or push.
No original-controls/Vanilla comparison, first-person or high-refresh matrix
was run. Root cause and correction remain open. Next implementation work
should inspect the original mode-6 pull-up/drop decisions on this exact route,
then verify safe clearance and ordinary hangs, ladders and dumpster climbing.


## Selected implementation

User explicitly requested logging the job and then implementing it. D08T8 was
marked In progress before gameplay edits. The original mode-6 handler already
uses Up for pull-up and Down for release; Square has no drop branch here.
Its pull-up query at 0x8007d240 uses the attached object's top when +0x17c is
nonzero. At this hang the dumpster top is -10240, while the platform height
stored in +0x1c8 is -11264. Pausing, clearing stale contact/support in private
RAM, then resuming and pressing W completes original animation 140. This
isolates the stale lower support as the cause on this route.

The Modernized candidate retries that query only in a settled mode-6/149 hang,
with forward input actually routed as Up, a validated climbable object below
the recorded ledge, no subpart attachment, and the normal action guards.
It temporarily clears contact/support and calls the original pull-up query.
Failure restores the hull/query state and ledge height. Success starts the
original animation 140; the original dispatcher performs the climb. There are
no forced position writes or bypassed landing/headroom checks. The helper's
owned-disc code receives a SHA-256 identity guard.

Space in a settled hang requests original Down. The press is consumed through
the release so held Space/W/E cannot jump off the dumpster or immediately
regrab. Releasing the keys allows a new grab. S retains original Down.
Ladder, ceiling and mode-7 object-hang ownership remain separate. Vanilla and
legacy controls do not use the retry or new drop mapping.

## Candidate verification

- Player build and native controls/input/aim suites pass. Controls fixture
  covers all 21 overlay guards. New checks cover query refusal rollback,
  stale versus valid support, input/capture/Vanilla gates, unsupported modes,
  altered helper identity and drop input consumption. Existing ladder,
  ceiling and crouch checks also run; these are not fresh campaign playtests.
- Third-person 60 FPS: complete UI-slot-7 dumpster/platform route, saved
  baseline hang with neutral then W, Space drop, S drop, Space with W/E held,
  and pause/resume followed by W all pass in private live runs.
- Dragging regression: grab, pull dumpster away, push it back, release, climb
  dumpster and platform passes. E-owned weapon restoration returns equipment 2.
- First-person 120 FPS: full slot-7 route reaches the platform. A subsequent
  saved-hang reload halted with GP0 word 0x1F323232. This is a reload failure,
  not evidence that pull-up failed. The harness quit before live RAM/aperture
  capture, so post-mortem data is incomplete. Logs and save are retained.
- Following note 213, baseline and candidate were compared with the same
  private hang save at first-person 120 FPS. Both restored successfully;
  recurrence cause and relationship to this change remain unresolved. The
  comparison harness now captures RAM, mod aperture, Expansion 1, GPU/DMA and
  savestate status before quitting on any recurrence. No GPU code was changed.
- Vanilla private saved hang: original Up and Square remain hanging; Down
  releases. This agrees with the original handler and preserves its behavior.

Local evidence adds `attachment.py`, `candidate-third-60.json`,
`candidate-first-120.json`, `regress.json`, `vanilla.json`, native/build logs,
`reload_compare.py` and baseline/candidate reload logs. First-person 120's full
saved-hang matrix is incomplete. Wider side/end placements, blocked ceilings
and fresh live ladder/ceiling/crouched-escape regression remain acceptance
limits. User feel and the reported slot-7 route still require playtest.


Additional final checks: deliberate stow plus E tap/fresh E reaches the platform
and stays holstered; Space drop followed by released keys and fresh W/E climbs
again. The Xvfb Scroll Lock injection did not stow in two attempts, so this
check used a private C binding (player binding untouched). See `stow.py`,
`stow-output.log`, `stow-reclimb.json`. The dragging run stopped at that stow
assertion; its dragging/climbing portion passed. Do not count the whole
`regress.py` run as passing.

Final controls/input/aim logs all pass after removing an unintended duplicate
mask from the legacy input return path. Build has existing binding-display
format-truncation warnings. All 41 original save/card/profile hashes remain
unchanged. Test processes stopped; no framework edits, commit or push.
Candidate executable SHA-256: `95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.


## User acceptance and closeout

User: "completely accepted. this works exactly as intended now. lock it in and close it out".

The reported slot-7 dumpster-to-platform route and intended controls are
accepted. D08T8 is Done. The tested implementation is unchanged at closeout;
no rebuild or gameplay retest is needed for these status-only updates.
The incomplete wider matrix remains a coverage limit, and the isolated GPU
reload halt remains unresolved. No framework edits were made for D08T8; the
existing local framework modifications are preserved. Implementation and
documentation are committed together as MusicMonsterMod.
