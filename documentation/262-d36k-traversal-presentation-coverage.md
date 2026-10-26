# D36K - Traversal and special-action visual coverage

## 2026-10-26 - D36K accepted and closed

User confirmed "fixed, lock it in, and close it out." D36K is Done. The
traversal/special-action coverage and the slot 12 swimming backpack correction
are accepted. Only the mapped backpack joint is hidden during the effective
first-person swimming draw; native motion and third-person presentation remain.
This note preserves the coverage,
regression evidence and bounded limits. Accepted executable:
`ba5f474f1af044de08ca9e71691d6e32eb766d4b86ae5dfc974814377f7de803`.
Implementation, tests and documentation closeout, commit and push are authorized
as MusicMonsterMod. Existing framework edits remain untouched. No rebuild,
game launch or player-save/settings writes during closeout. D36L remains
unselected; D19Q3 remains Needs playtest. No next job started.
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-26 - Swimming backpack candidate (subsequently accepted)

The user's only critique of the coverage pass was the backpack bobbing across
first-person swimming in `research/screencaps/backpack.mp4`. Work resumed only
after the user finished testing and authorized it. This supersedes the initial
pass's conclusion that no implementation change was needed.

- L: the clip shows the pack crossing the eye during submerged ascent/descent.
  A private copy of current slot 12 reproduces it after Ctrl dive and downward
  mouse look, then alternating Ctrl/Space. Earlier surface look tests included
  the nearby ceiling and are not clean evidence of the reported pack defect.
- S/L: the visible piece is semantic part 20 (0x14), mesh 96 in the audited
  costume census. Native attachment lookup uses descriptor +0x38. Hiding this
  mapped joint removes the same pack in the before/after route; third-person
  captures retain it. This is separate from hip accessory part 21 and D36J's Q.
- S: `first_person_draw` hides that joint only for the validated primary Duke
  eye draw with effective Swimming policy. It uses the existing temporary
  joint bit-0 mechanism, preserves other flag bits, and restores with the head
  scope before other actors/reflections and at draw end. World/load/replay
  resets forget stale host pointers; the existing joint sweep reclaims saved
  hide bits. Invalid part mapping fails closed. Native matrices, movement,
  physics, camera pitch, weapon presentation, flight and save format are unchanged.
- S: player build and controls/input/aim/near suites and animation save recovery
  pass. Added controls assertions cover both costume orders, swim modes,
  reflection/end-draw cleanup, flag preservation, invalid maps, stale loads,
  replay/action preferences, global third, Vanilla, land exit and flight exclusion.
- L: private `baseline-deep60` versus `candidate-deep60` recordings and sampled
  frames show the pack obstruction removed. `candidate-deep120` repeats the
  submerged motion, physical P third/first switches and private save/load.
  Third person retains the pack; the reloaded eye remains clear. The 120-setting
  run exercised 1,066 replay sessions with zero replay/plugin/worker failures.
  This diagnostic recording is not a sustained-120-FPS performance claim.

Local evidence: `recomp/analysis/d36k-backpack/` contains the probe, recordings,
model/matrix samples, native-test/build logs, baseline executable and integrity
manifest. Baseline-launch setup failures (Python path, then missing adjacent BIOS)
were corrected before the successful comparison; they are not gameplay evidence.
All 42 intake player-save/card/settings/runtime-patch files retain hashes and
mtimes. Diagnostic games are stopped. Existing framework changes are untouched.
Candidate executable SHA-256:
`ba5f474f1af044de08ca9e71691d6e32eb766d4b86ae5dfc974814377f7de803`.

At candidate handoff, D36K awaited the user's slot 12 visual retest. Acceptance
and closeout authorization are recorded above. The broader coverage limits
below still apply. D36L remains unselected. Launch `python3 recomp/tools/local/run.py`,
load slot 12, use P for first person, then dive and move up/down while looking down.

## Initial coverage pass (before the backpack report)

2026-10-26. Needs playtest (human motion/appearance review). Selected by the user
after D36J acceptance. No new demonstrated presentation defect was found in
the inspected routes, so the accepted player build and gameplay code are unchanged.
This is the bounded coverage proposal from the accepted [D36 audit](257-d36-first-person-presentation-audit.md),
not a reopening of accepted D35 functionality or selection of D36L.

## Scope and evidence rules

Preserve the accepted D36G/H/I/J presentation ownership, action preferences,
native animations and callbacks. Investigate reachable entry, active movement,
exit and interruption; only demonstrated visual defects warrant pose or
attachment corrections. S means source/native-test evidence, L means observed
private-run evidence, A means previous acceptance, and U/H means an unverified
report or hypothesis. A requested key, filename, successful script exit or
frame-rate setting alone does not establish that an action or rate occurred.

The baseline executable is the accepted D36J plain-left-leg revision:
`1f4ee0da21ddc0e5659a17dfaad8d80b8ccf0a9b1cf86dd681e1fd061e11b6d2`.
Framework pin: `08ec704a974b1f3a16335b4afeb340b9eff19926`; UI pin:
`be8ac1d03ee19d55394b5a5f2d9d1506edd56659`. Existing framework edits were
present at intake and are outside this job. No gameplay, pose, renderer,
asset, save-format or player-setting change is currently justified by the
inspected evidence.

## Natural-route matrix

Private copies of archived D35 saves identify the earlier accepted routes;
current player slots are separately copied for present-day reproductions.
All writes go to diagnostic cards/settings. Debug reads are sequential, not
atomic snapshots; short recordings and representative frame inspection do not
certify every moving frame. No pose, position or health writes
are used to manufacture action coverage. Native input and callbacks remain
in charge. Xvfb/OpenGL recordings use dummy audio: this is not sound testing.

| Route | Observed coverage | Evidence boundary |
| --- | --- | --- |
| Ladder | Ground mount 185, rung/climb 186-189, top exit 190, top remount 156, bottom exit; pause/load; high catch 148/207 and held-input drop | L, 60/120 settings. One early airborne catch missed; later successful catches are identified separately. |
| Chain/pole | Attached 192, turning, descent, upper exit 196, lower exit and drop; saved attachment and pause | L, 60/120 settings. Uses the archived accepted chain route, not every pole-class object. |
| Mantle/ledge | Entry, 149 hang, shimmy, 140/152 pull-up and release/landing | L, 60/120 settings. Native close-surface camera passages remain; this does not certify every ledge's contact geometry. |
| Overhead | Natural jump/E catch, 149-151 traversal in five directions, look, saved hang, drop/fall | L, 60/120 settings. Ownership requires the actual mode-7 attachment, not animation number alone. |
| Climbable wall | Natural jump/E catch, vertical/diagonal movement, upper/lower exits, saved attachment, pause and drop | L, 60/120 settings. Both runs assert wall ownership and successful upper/lower exits. |
| Door | Native 116, right-leg door presentation, return to 63; no host Q attack/hit/sound invocation | L, 60 setting; separate from the accepted plain-left-leg Q. |
| Toilet | Native 275, committed action and return to 63 | L, 60 setting. The archived save has already consumed its one-time effect; the effect byte remains 1. |
| Statue | Native 279, committed action and return to 63; effect byte changes 0 to 1 | L, 60 setting, accepted type-926 route. Not every class-13 type. |
| Mirror | Near/far reflection, repeated native 278, return to ground | L, 60 setting. Missing voice remains separate D35N1. |
| Keycard/crystal | Actual equipment-1 item IDs 6/11, 266/267 use, native return to weapon | L, current copied slots 11/9 at 120 setting. Held-item and committed-use phases are distinguished. |
| Wheel | Actual 270 with connected native arms | L, current copied slot 5 at 120 setting. |
| Flight | Actual 163/164/165/170, hover, forward/strafe/fire/look, private airborne save/load, off/landing | L, current copied slot 8 at 120 setting. |

Additional L coverage:

- Archived swim saves start in real surface mode 4 and submerged mode 5.
  Dive, underwater movement/fire/look, saved swimming and physical P changes
  were checked at 120 with First Person and action Third Person. Brief samples
  labelled `surface` remained submerged and are not evidence of surfacing.
- Corrected water-exit route uses `d35g/cards-water`, not its default land cards:
  124 samples include 122/125 surface movement, 128 swimming, 140 pull-up,
  15 mode-8 samples, then ground and weapon return. The earlier default-card
  attempt never reached water and is excluded. The short water-exit phase
  resolves as Mantle in these samples; this does not prove every 8/4 or 8/5
  WaterExit boundary was sampled.
- The archived gas route verifies face 2 and active mask drawing, a private gas
  save/load, physical first/third changes, goggles face 3, mask replacement,
  and clean-air exit (face 0, active mask bit cleared). The initial current-slot
  gadget attempt never drew the mask and is excluded from mask coverage.
- A genuine `level 1` console transition replies with the accepted travel
  request and reaches level 1, pose 63, live first person. Copied cross-level
  slots 6/9/12/8 also restore their expected ground/ladder presentation.
- Attached-state perspective checks cover all five traversal families with
  First Person, action Third Person and global Third Person. Corrected pole
  and mantle retries remain attached during pause and both P changes; held
  release restores the appropriate global perspective.
- A 240-setting overhead stress run completes 792 replay jobs with no worker
  or plugin failures (final pacing divisor 3). It is a robustness check, not
  sustained 240 FPS. Vanilla private ladder/pole/overhead loads plus original
  pad movement retain native presentation; these are smoke checks, not full
  Vanilla campaign acceptance.

The 120 traversal runs processed 13,250 replay worker jobs in total, with
zero worker or plugin failures. Final pacing divisors were 2 (ladder/pole),
3 (mantle/overhead) and 1 (wall). These are bounded replay checks during
capture, not a sustained-120-FPS result or a performance improvement claim.

## Final regression checks

S: incremental native-test build, full controls suite with original LEVEL00
fixture and all owned level overlays, Xvfb input suite, aim suite, owned-EXE
near-clipping suite and D37A animation/save-restoration tests all pass.
The player executable was not rebuilt. These test results establish their
contracts, not visual certification of the unvisited routes below.

L: native RPG damage after disabling the copied save's god mode through
`dnkroz` reaches health 0 and releases first-person presentation. The final
physical-input run waits for the owned Death page, chooses Continue, waits
for the confirmation page, confirms, then observes health 10000, active first
person and movement pose 72. Earlier fixed-delay attempts sent confirmation
before the death menu was ready and are not successful respawn coverage.
No health, animation or position writes were needed. Records are in
`d36k-death/physical-death-ready120.json` and its motion/screenshots.

The current-slot special-action run completes 6,917 replay jobs without
worker/plugin failures. Its sampled near-clip counters record zero packet
skips, copy overflows or world/object budget hits, with a frame-used peak of
179600 bytes against the reported 180704-byte arena. This is a bounded
observation, not a general capacity/performance guarantee.

All 43 intake files retain their hashes and mtimes, including player saves,
cards/settings, the accepted executable and runtime patch. Diagnostic game
processes have stopped. Existing framework edits are preserved. No commit or
push is authorized or performed. D36K awaits human visual acceptance; no next
job is started and no player-facing manual change is required.

## Body policy and visual review

S/A: native interaction arms keep world transforms, native contact animation
and world occlusion. D36H combat/held-item arms retain their display owner;
D36J ordinary locomotion legs and the separate Q/door leg owners remain intact.
The joint bridge is not permission to render every torso/body part in every
traversal pose. Global Third Person wins over action preferences.

L: inspected captures retain the existing ladder/overhead arms, connected wheel
arms, complete reflected Duke, and separate door kick. Some native limbs extend
offscreen or are occluded by nearby geometry; those facts alone do not prove a
missing mesh. The accepted ladder torso/down-look exclusion is preserved.
No new clipping takeover, forced torso, fabricated limb, pitch restriction or
blanket pose admission is introduced to make a screenshot look fuller.
Human motion/appearance review remains required; representative images are
not frame-by-frame or all-costume certification.

## Reproduction and retained limits

Local-only evidence is under `recomp/analysis/d36k/`, `d36k-d35*` and
`d36k120-d35*`. `intake.json` records player-file and binary hashes/mtimes;
`route-sources.json` records archived source-save hashes; `route-files.json`
also records sidecars and private card files. `prepare.py`,
`batch.py`, `matrix.py`, `choices.py`, the copied route scripts, JSON state
traces, screenshots and motion recordings retain the actual inputs/results.
Do not publish the saves, captured retail geometry or videos as source assets.

Early short xdotool taps sometimes failed to trigger release at 120. Held-input
retries distinguish missed synthetic input from a presentation failure. Some
initial choice routes walked out of the attachment before P was pressed;
those ground-view toggles cannot certify attached-view transitions. Corrected
routes keep the attachment active. Current slot 2 is a land scene: its labelled
swim/dive attempt reached crouch 178 and is excluded from swimming coverage.
The current slot-10 use attempt did not establish vending pose 277.

Still unverified as exhaustive live visual coverage: every costume and lighting
combination, every native damage interruption, all lever/callback variants,
underwater class-6 crystal use (131), class 9 (269), type-179 toilet,
class-12/type-368, alternate class-13/type-838, class-14/16/17/18 callbacks,
and the nine alternate-camera event subtypes listed in [note 238](238-d35k-interaction-inventory.md).
Previously accepted type-206 wheel and type-926 statue observations do not
certify those other variants. No unknown interaction is whitelisted.
D36L's combined renderer/aspect/costume/lighting/restoration acceptance pack
remains unselected. D19Q3 remains separately Needs playtest.

## Player review

Use the existing saved preferences and accepted player build:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Capture is automatic; P changes first/third person. Review ladder, chain,
overhead, wall and mantle entry/movement/exit with normal mouse look, then
wheel/mirror/door and swim/flight. Compare each action's First Person and
Third Person setting and verify that global Third Person still wins.
`--vanilla` selects original play for one launch without saving that choice.
