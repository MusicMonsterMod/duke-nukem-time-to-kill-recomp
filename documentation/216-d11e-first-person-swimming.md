# D11E - First-person swimming candidate

2026-10-24. Status: Done (user-accepted). User authorized implementation after
reviewing [the research](215-d11e-first-person-swimming-research.md), then
accepted first-person swimming and explicitly requested lock-in and closeout.
The candidate evidence below is preserved with its original coverage limits.

## Behavior

Modernized with Independent camera and first person selected now keeps the eye
view at the surface, through the dive, underwater and while surfacing. Movement,
looking, firing, oxygen and water ownership retain their existing systems.
Third person and Vanilla retain their existing camera policies.

The swimming eye follows real body travel, without following the head-first
dive or the pitched swimming stroke. It is centered over the body horizontally.
Underwater its target is 294 units above the root, derived from the measured
floating neck height (about 198) plus the existing 96-unit eye offset. Surface
mode targets 96 units above the water plane. These are presentation offsets,
not changes to Duke's collision shape or buoyancy. The target offset converges
by half each camera update; body travel is not smoothed away. Switching between
the land pivot and swim root rebases the previous relative eye offset before
easing the posture difference. Land bob/glide is disabled and cleared while swimming; shallow land movement
retains its existing eye behavior.

The normal camera still solves the requested anchor and updates room/geometry
state. Camera yaw/pitch remain independent of the body's thrust yaw/pitch.
No direct post-solve camera teleport, new underwater effect, water ownership
write, swim velocity change, generated-C edit or new hook is introduced.

Mantles remain the established third-person traversal view. The first
water-exit frame, mode 8 / animations 134-142 with previous mode 4/5, yields
first person immediately on acquisition, including when the water flag remains
set. Supported land movement regains the selected view. This deliberately does
not implement first-person climbing, jetpacking or scripted cameras.

## Implementation

- `recomp/src/ttk/first_person.inc`: remove the deliberate swimming refusal;
  add the stable swim anchor and land/swim reference conversion; suppress swim
  bob/glide; handle the early swim-to-mantle state explicitly.
- The crossbow exposed the expected display-transform problem: its firing
  model could rise vertically across the view. Animated swimming weapons now
  reconstruct the hand from the original right-arm animation channel and the
  current costume's part/offset table, using the existing D08K10 helper. This
  avoids applying the swim shoulder's world-space compensation again inside
  the first-person display. Draw/throw/reload remain animated. Seeded gun poses
  and all third-person matrices retain their existing paths.
- `modern_controls.cpp::RenderState` publishes swim weapon eligibility to
  render workers, independently of crouch eligibility. The draw path does not
  rely on a worker's fork-time gameplay statics. Existing world resets invalidate
  the eye history; the added reference bookkeeping is only used with valid
  eye history and is refreshed on every anchor evaluation.
- Real holster input exposed an original swim transient with `+0x224 & 0x200`
  (mode 5, animation 127) that revoked camera presentation. A first-person-only
  presentation exception now covers native swim animations 122-133 in modes
  4/5. Action/capture eligibility, third-person behavior, actor/death guards,
  camera ownership and the script inhibit bit `0x20000000` are unchanged.
- `GAME_MANUAL.md` describes swimming in first person and the mantle exception.

## Tests and observations

All runs used private settings/cards and earlier private diagnostic fixtures
under `recomp/analysis/d11e-research/`. No player card, save or profile was used
by the game. No original media was changed. Tests used offscreen OpenGL or a
private Xvfb display with dummy audio. Configured FPS is not a measured
physical-monitor refresh rate or a performance certification.

### Native and build checks

- Final player build succeeds; native movie and geometry shards are current.
- Final `ttk-controls-test` passes with the owned EXE, clean LEVEL00 fixture,
  LEVEL01 and all 21 selectable overlay identities.
- New anchor cases cover native surface/underwater and land-side deep-water
  transition states, two costume layouts, large changes in the neck matrix,
  input suspension/pause, loaded first person, explicit third person, the swim
  holster transient, retained script inhibition, and early water-to-mantle
  previous modes 4/5. Original water, velocity and root fields remain unchanged
  in those controlled camera tests.
- New animated-weapon cases cover crossbow, pipe bomb and held item: changing
  the world hand cannot change the corrected display; changing the arm channel
  animates it. Forked worker tests load enabled/disabled/enabled snapshots and
  verify restoration of the ordinary ground display independently of crouch.
- Existing crouch/ground weapons, upgrade framing, input/camera ownership,
  traversal and Vanilla assertions remain in the passing controls suite.
- `ttk-input-test`, `ttk-aim-test` and `ttk-near-test` pass. These ran after the
  initial camera change; their source did not change afterward. The controls
  suite and player build were rerun after the weapon and transition changes.
- `git diff --check` passes. No framework source was edited or patch re-export
  needed for this job; existing patched submodule state was preserved.

### Live routes

- `matrix.json`: 415 samples across level 2 and level 6 at configured
  60/120/240 FPS. Every sampled surface/dive/thrust state retained requested
  first person and blend 1. Forward/back/strafe, pitched look, moving fire and
  ascent to mode 4 were exercised. Canonical and presented captures were
  inspected, including the water surface and nearby walls. No new body-hiding
  rule was needed for the inspected views.
- Weapon captures exercise pistol, shotgun, Gatling, rifle, pipe bomb and
  crossbow. The corrected crossbow firing sequence stays in front of the eye;
  its original across-body carry remains animated. The pipe bomb's throw is
  retained. These are visual/functional checks, not every weapon/ammo/target
  combination. Initial mislabeled crossbow attempts still had the pistol;
  corrected captures explicitly checked held slot 3. Short holster attempts
  that retained equip 2 do not count as unarmed coverage.
- `real.json`: 136 real-keyboard samples include a successful holster (equip 0),
  P to third and back, Escape pause/resume, console open/close, and loading
  surface/underwater saves while the opposite view had been selected. Suspended
  input retains first-person blend 1. Holstered settled capture is unobstructed.
  Intermediate samples taken during the explicit P blend are not fallbacks.
- The same real-input run travels through the original level command to
  BLOOD BATHS (12) and regains ground first person. A further 70-step route
  tested land movement/jumps there but did not reach swimmable water. It is
  ground/level-transition regression evidence, not a third swimming level.
- Attempts to reach a shore/climb-out from the two historical water fixtures
  remained in water. They do not establish a successful live mantle or full
  natural entry/exit route. The early mantle handoff is covered by native
  fixtures; natural two-level entry/exit remains a playtest requirement.

The 415-sample and real-keyboard runs used the candidate immediately before
the final narrowly scoped early-mantle guard. The final executable additionally
received the complete native controls pass and the 4:3 regression run below.
No broad earlier route is presented as a fresh full-matrix run on the final hash.

### Final executable regression run

`regress.json` verifies Modernized third-person surface/dive/resurface, Vanilla
surface view, first-person surface/dive/resurface, an underwater load in level 6
and corrected crossbow fire. Presented captures were inspected. The first
attempt to request 4:3 used an invalid private-profile value and was normalized
to 16:9; it is widescreen evidence only. `regress-4by3.json` repeats the final
first-person routes with the verified setting `widescreen=off` (4:3), including
surface/dive/resurface and level 6 crossbow fire. Its presented images were
inspected and the private profile retained `off` after shutdown.

All diagnostic game processes have stopped. The private Xvfb server is stopped.
No player-facing game was launched or left running for this handoff.

## Accepted executable and recorded coverage limits

Accepted SHA-256:
`6725addce03114878375d08d81331a4bd4032aa5169d21fff74a586602b73ea6`.

The user accepted the feature. The acceptance criteria covered entering,
swimming, diving, surfacing and leaving water in at least two levels, with
third person and Vanilla unchanged and the view reading well. The user's
acceptance did not enumerate individual routes, so the earlier automated/live
coverage limits remain recorded rather than upgraded to new test claims. Wider costume/room coverage, all weapon
upgrades, every reload/throw, death/oxygen exhaustion, focus-loss behavior in
water and all renderer paths are not established by this bounded run.

The original water rendering is reused; no generalized claim about all water
surfaces, portal boundaries or underwater visual effects is made. Camera feel
and the chosen eye height are ready for user evaluation. No known GPU fatal
halt occurred in these runs. On some private shutdowns `quit` reported
`emu busy or frozen` while the process exited; logs and process checks showed
normal termination rather than the D20G GP0 halt.

Launch with saved player settings, then press P if needed for first person:

```sh
python3 recomp/tools/local/run.py
```

Automatic capture remains in effect. Space ascends, Ctrl dives, E climbs out.
User authorized commit and push as MusicMonsterMod. No additional gameplay
changes, rebuild or new live tests were needed for acceptance closeout.


## Acceptance and closeout

User: "awesome!! in that case, we can now accept the first person swimming.
lock it in and close it out". D11E is Done. Implementation, tests, manual,
research and this evidence record are included in the authorized closeout.
The executable hash above is unchanged. Runtime patch re-export was byte-for-byte
unchanged; application to the clean pinned framework succeeded and all 51
patched files match the existing framework worktree. No submodule pin changed.
D08Z6 and D08AF remain Todo; neither backlog addition was actioned.
