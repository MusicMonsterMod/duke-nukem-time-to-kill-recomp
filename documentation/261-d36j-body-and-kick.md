# D36J - Coherent body and quick-kick leg presentation

2026-10-26. Done, user accepted and closeout authorized under the
accepted [D36 audit](257-d36-first-person-presentation-audit.md). D36G/H/I remain
the accepted presentation foundation. D36K/L are unstarted. D37A is separately
Done following the user's full playtest acceptance in this session; D19Q3 remains
Needs playtest. D36J implementation, tests and documentation commit/push are
authorized as MusicMonsterMod. Earlier candidate acceptance limits below are
historical; the recorded coverage limits still apply.

## Revision 2 - plain left-leg quick kick

The user rejected the first candidate's leg choice: Q should display Duke's
plain left leg, with no holster. Natural orientation on the holstered right
leg was not the requested result. That earlier candidate is superseded.

Q now owns semantic left-leg parts 1-4 from the current costume. The recorded
kick pose is retargeted using the left/right hierarchy offset differences;
actual left meshes are used, with no reflection or copied right-leg texture.
The ordinary right leg and part-21 attachment remain native during Q. Native
door 116 retains the separate right-leg/attachment path. Body clipping/fade,
weapon framing, kick gameplay callbacks and timing remain as before.

The controls suite passes with explicit left-leg ownership, untouched right
leg/attachment and both costume orderings, exact hit-call count/arguments,
end-of-kick restoration and the existing native door tests. The following
first-candidate evidence is historical where it concerns right-leg Q images.

## First candidate (superseded leg choice)

Ordinary first-person ground/committed/fall presentation now explicitly owns
native leg parts 1-8 and the right-thigh attachment, part 21. Their original
matrices and world depth remain authoritative. The guarded joint-depth bridge
admits their geometry to near-plane polygon clipping instead of rejecting a
whole segment when its origin crosses GTE H. Traversal/interaction body policy,
torso/head policy and accepted weapon/arm framing remain separate.

Quick kick keeps its native right-leg geometry and hip-to-knee orientation.
The old end-for-end thigh rotation, knee-origin relocation and separate 13%
screen drop are removed. Kick parts 5-8 reach the same safe clipping bridge,
using their existing camera-relative viewmodel depth. Part 21 follows the
displayed thigh using its native relative rotation and offset. It reuses the
existing hip matrix scratch after that mesh has consumed it; no allocation,
extra skeleton submission, model asset or serialized state is added.

The admitted ordinary legs also use the existing guarded close-camera fade
correction. This removes native distance-induced transparency/dimming while
preserving intentional actor transparency flags. Render context and GTE back
colours restore at the next joint, intervening transform or draw boundary.
The correction does not force the body into weapon-overlay depth.

Q's 27-frame cycle, frames 8-20 damage window, damage/radius/type/attacker,
view-directed hit point, reach, collision and impact sound are unchanged.
Door animation 116 still derives the display phase from native channel timing
and does not start a quick-kick attack. Third person and Vanilla retain native
presentation. No framework edits, dependency changes, save-format changes,
generated C edits or player-setting changes are part of this job.

## Evidence and verification

**S - source:** the audit's end-for-end thigh mechanism was verified directly.
The retained retail geometry census covers DB00-DB09: part 21 is a depth-3
child of the depth-2 right hip, offset (42,63,8), mesh 212, in all ten groups.
This identifies a real attachment that previously stayed on the world thigh;
it does not retrospectively prove every reported apparent third leg had that
cause. Semantic maps, rather than costume-specific joint indices, drive ownership.

**S - native:** player build and controls/input/aim/near-clipping suites pass.
Controls runs use the original LEVEL00 guard fixture, owned LEVEL01 and the
full overlay fixture directory. New checks cover both joint orderings, body
versus arm/head ownership, original matrix preservation, depths across H,
actor/caller/stack-camera gates, world-depth packets, natural hip-to-knee spacing,
attachment transforms and scope retirement. Proximity-fade correction restores
RAM context and GTE state and preserves each intentional transparency flag.
The kick test asserts exactly 13 sphere calls across frames 8-20 and retains
radius 96, type 10, damage 200 for the fixture, attacker and contact arguments.
Existing door-116 tests retain the no-extra-attack contract. Existing suites
also cover cancellation, replay/load ownership, camera policy and Vanilla.
D37A animation metadata recovery and incompatible GPU-packet clearing pass.

**L - private captures:** evidence and scripts are local under
`recomp/analysis/d36j/`. Tests use copied saves and separate settings/cards.
Frozen frame-13 comparison covers Boot, pistol, Gatling and RPG at level/downward
looks. Corrected captures retain the natural thigh and its moving attachment.
Non-frozen sampled routes cover walking, running, crouching, jumping, repeated
kicks, upgraded Gatling/RPG and private save/load. Some walking/running samples
meet the slot-8 dumpster; those are contact transitions, not an unobstructed
full gait certification. Static/sparse captures do not certify every motion frame.

**L - bounded cost/replay:** three 3-second Xvfb/OpenGL downward-view windows
measure 59.93-59.94 guest frames/s on the saved baseline and 59.94-60.26 on the
candidate. The final aggregate frame average is 16.683 ms for both. This is a
capped single-route comparison, not an uncapped cost or full-campaign benchmark.
The 120-setting Xvfb run completes 1088 worker jobs with zero failed jobs/plugin
failures and pacing divisor 2; it does not certify 120 distinct frames/s.
Candidate packet peak on that route is 21512 of 139744 bytes, host geometry
peak 14160 bytes, zero packet skips/copy overflows/world or object budget hits.

Harness limits: an early offscreen pass displayed the pre-existing load-menu
overlay and is excluded from visual acceptance. The SDL offscreen input suite
cannot enable relative mouse mode; its Xvfb rerun passes. An offscreen frame-perf
request had no GL timer samples; the Xvfb comparison supplies the measurements
above. `act=camera_view` is a queued gameplay command, not the physical camera
event, so those labelled toggle attempts do not establish view switching.
The first added fade-test fixture omitted the required stack pointer; correcting
the fixture exercises the real hook and passes. These failures remain in local
logs rather than being counted as successful coverage.

## Playtest and remaining limits

Review downward standing/walking/running/crouch movement and Q start/extension/
retraction with Boot, pistol, Gatling and RPG. Check repeated kicks, view changes,
save/load and the hinged-door kick. Native anatomical framing is the candidate;
no camera-pitch restriction or changed weapon framing hides remaining concerns.
Every costume/action/lighting combination, stairs, natural damage interruptions,
software renderer, 4:3 and 180/240/Unlimited stress are not newly certified.
The combined acceptance pack remains D36L, and traversal/body coverage D36K.

```sh
python3 recomp/tools/local/run.py
```

Gameplay capture is automatic. P selects first person; Q quick-kicks. Existing
player preferences are used. `--vanilla` selects original play for one launch.

## Final verification - 2026-10-26

`coverage-xvfb` uses the physical P key in the isolated Xvfb window: first ->
third -> first is asserted from effective perspective, then private save/load
returns to first person and movement reaches pose 72. Current copied slots
2/6/9/12 also retain live kicks and safe return; slot 12 is ladder pose 186,
which does not certify all traversal-body presentation. The run completes 2178
replay jobs with zero worker/plugin failures. Its packet peak is 81172/139744,
with zero budget hits, packet skips or copy overflows. The offscreen synthetic
X-key attempt failed to toggle and is not counted. Earlier console-level
attempts while input was suspended did not prove costume/level transitions.
All ten costume groups have source mapping evidence; live images do not certify
all ten. Ordinary native body geometry remains outside the screen when dictated
by its pose; this change does not pin two boots to the bottom of the view.

All player save/card/settings hashes and mtimes match intake. Diagnostic game
processes have stopped. No framework modifications were made; the existing
runtime patch and submodule worktree edits were preserved.

Candidate executable: `9186ed9531217dcedaeb9da3c410290649609565d33650e0a250403602563a61`.

Revision-2 executable: `1f4ee0da21ddc0e5659a17dfaad8d80b8ccf0a9b1cf86dd681e1fd061e11b6d2`.

Revision-2 live verification: `left-leg` captures actual plain left-leg Q at
frozen frame 13 with Boot/pistol/Gatling/RPG, level and downward views.
`left-coverage120` uses non-frozen Q, physical P view changes, private save/load
and movement, plus copied slots 2/6/9/12 at a 120 presentation setting. Effective
third/first transitions are asserted; restored movement reaches pose 72. Replay
workers report 0 failed jobs and 0 plugin failures.
These are bounded diagnostic checks; user motion/appearance acceptance remains.
Diagnostic processes exited; no player settings or saves were written.

## Acceptance and closeout - 2026-10-26

User: "lock it in and close us out". The plain left-leg revision is accepted;
D36J is Done. This accepts the actual left-leg meshes without a holster for Q,
with the separate right-leg door kick and body visibility corrections retained.
The first right-leg Q candidate remains rejected and superseded. Existing
verification and untested combinations remain explicitly bounded above.

Authored implementation, tests, manual, board, status and handoff are included
in the authorized MusicMonsterMod commit/push. The separately accepted D37A
status correction is included. No framework edits, rebuild, game launch or
player-file writes during closeout. D19Q3 remains Needs playtest; D36K/L remain
unstarted. No next job started.
