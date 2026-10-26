# D36H - Connected combat and held-item arms

2026-10-26. Done, user accepted and closeout authorized. User: "fully accept! lock it in and close it out!!!!!!".
The accepted [D36 audit](257-d36-first-person-presentation-audit.md)
and [D36G ownership contract](258-d36g-limb-ownership.md) remain authoritative.
D37A/D19Q3 retain their separate Needs playtest statuses. D36I-L are unstarted.

## Accepted behavior

Combat and held-item hands now have connected retail arm segments in the same
display space. The accepted right-hand matrix remains the weapon, muzzle-flash
and held-item attachment target. Native two-handed weapon poses supply their
support grip relative to that hand; display weapon scaling also moves the support
grip about the right hand. One-handed weapons and equipment-1 items/remote use
only the right chain. All six segments resolve by semantic part, not costume
joint index. No new asset, extracted geometry or replacement texture is added.

The first private prototype rigidly moved the native arm chains with the hand.
It exposed shoulder caps in the view. The candidate instead preserves each
bone's length and swings the forearm and upper arm toward a downward/backward
camera-space continuation. Child endpoints meet their parent segment endpoints;
the wrist and weapon framing stay fixed. This is a display pose, not a new
native animation or a claim that the arm is attached to the world-space torso.
The user accepted these poses and grips, including the revised Buffalo cycle.

`DisplayArm` extends the existing ownership descriptor. Its viewmodel depth and
proximity-fade correction match the weapon. The verified joint-depth bridge
admits those segments to polygon clipping, while hidden unused arms remain
excluded. Native Push/Use/Mirror/traversal ownership and world occlusion are not
changed; their remaining visual work belongs to D36I/K.

Only the original hand draw advances recoil and pose counters. Arm transforms
preview that grip; arms drawn after the hand reuse it rather than applying the
next recoil step. This matters for cowboy models whose right branch precedes
the left. Original skeleton matrices, gameplay, damage, projectile, animation
events, callbacks and weapon selection are untouched. No new guest allocation,
serialized field, replay field, player preference or framework edit is added.
The existing weapon matrix and hidden-arm scratch are reused. Draw/load/replay
boundaries retire the draw-local hand-completion flag.

## Verification and evidence limits

**S - source/native:** player build and controls/input/aim/near suites pass.
Controls use the original LEVEL00 fixture, owned LEVEL01 and all level overlays.
New checks exercise connected endpoints for all 14 base records, direct challenge
records 27-29 and equipment-1, in both original and cowboy joint orders. They
check semantic ownership, depth admission, source-matrix preservation and cleanup.
Existing tests retain base/upgrade weapon matrices and scale, attachments,
crouch/swim display reconstruction, kick mechanics, native interactions, fade,
reflection isolation, third-person/Vanilla and world/replay reset contracts.
The D37A native save-allocation recovery and GPU-clearing tests also pass.

The controls harness previously returned one address for every guest allocation.
A new endpoint test exposed that unrealistic alias between arm scratch and the
weapon matrix. Its mock now returns distinct aligned allocations; the full suite
passes with that stricter model. A synthetic camera rotation was also explicitly
initialized for the geometric endpoint assertion.

**L - private captures:** ignored evidence is `recomp/analysis/d36h/`. Separate
settings and copied writable saves/cards, offscreen OpenGL, dummy audio, debug
port 9488. `corrected-120` confirms actual records 1-14 plus Boot and samples
idle/crouch/up/down/attack/release. It captures equipment-1 keycard 6 and crystal
11, quick kick, jetpack mode 165 and a flight save/load. Three replay workers
ran without failed jobs in its final sample. `final-43` verifies all six campaign
upgrade flags and actual selected base records (4/5/7/8/10/9), idle and attack,
in 4:3. Direct challenge 27-29 mapping is native-test evidence, not a new live
challenge-stage playthrough.

`final-checks` samples pipe-bomb throw/load and the remote transition.
`swim-view` uses an older private D11E swimming save copy, verifies actual
first -> third -> first effective-view changes, swimming 127/128, firing upper
10 and an underwater save/load at 120. Reviewed stills show connected forearms
with the original weapon framing, including the crossbow, axe, Gatling and
keycard. Water lighting remains visible on the hand/weapon.

The initial capture scripts did not re-trigger identical diagnostic action/cheat
strings, and invalid/conflicting private view bindings fell back to defaults.
Those attempts are not counted as missing-weapon, upgrade, swimming or view-toggle
coverage. The corrected runs above use actual equipment/perspective/state fields.
One `final-43` telemetry request lacked the GL timing flag; its completed weapon
captures remain useful, and timing/remaining checks were rerun separately.

**L - bounded cost check:** same candidate, 4:3, 60 Hz, slot-8 Gatling, three
three-second windows. Arms enabled measured 59.88-59.95 guest frames/s; diagnostic
`DNTTK_FP_ARMS=0` measured 59.94-59.98. Both reported 16.683 ms average frame time.
The last aggregate had 514 versus 505 primitives; scene GPU averages were 2.301
versus 2.153 ms. These capped offscreen samples include pacing/driver variation;
they do not establish an uncapped CPU cost or campaign-wide performance.
`DNTTK_FP_ARMS=0` is a developer A/B switch, not a saved player option.

Across the 120 sweep, observed packet-arena peak was 121620 of 139744 bytes,
host geometry peak 23120 bytes, with zero world/object budget hits, packet skips
or copy overflows. No new packet buffer or animation allocation is requested.
These observations do not certify every scene or the higher refresh settings.

**Remaining:** user review of per-weapon support grips, wrist/elbow seams and
continuous draw/fire/throw/reload/switch motion; every costume, interruption,
dark/gas/power-up/mirror combination and full campaign are not visually certified.
Sparse stills do not establish every transition frame. There is no new live
software-renderer sweep. Existing native tests and the sampled live states are
bounded evidence, not replacements for these visual acceptance checks.

## Playtest

Use P for first person if needed; gameplay capture is automatic. Check knife/axe,
Gatling, shotgun/crossbow and throwables, including the remote, then card/crystal
use. Look up/down, crouch, fire/reload/switch, quick kick, swim/fly, change view and
save/load. Please judge the connected wrists, support-hand contact and whether
any shoulder cap or abrupt arm movement appears. Native interaction defects
covered by D36I remain separate.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Modernized is default; `--vanilla` remains a one-launch original profile.

Candidate executable: `9bffb4f704fbf22c02d0b203c6995c067f18f6e237b0fdd66462b9251f1da806`. Player saves/cards/settings hashes and
mtimes match the 41-file intake manifest. All private game processes stopped.

## Buffalo Rifle reload follow-up - 2026-10-26

User reports D36H is "99.9% perfect" except weapon key 9's Buffalo Rifle: the
hand cocks/reloads while the rifle stays fixed. This is a bounded follow-up to
D36H, not acceptance closeout or authorization to commit/push.

**S/L:** reproduced with actual equipment 2 / record 6, using a private copy of
save slot 9. Its native firing sequence uses upper 25 and reload upper 27; the
reload right-arm keys in the observed DB00 route are 4113-4118. The display
previously kept the seeded firing grip throughout that cycle while the support
hand followed the native skeleton.

Record 6 / upper 27 now reconstructs its right-hand pose from the native channel
and applies the reload motion relative to the clip's final ready key, around
the accepted firing grip. The clip and final key are resolved from the loaded
animation tables, not hardcoded global frame numbers. Both arms and weapon
attachments share that moving grip. The original idle/firing presentation and
other weapon records retain their existing policy. Channel reconstruction avoids
applying crouch/swim world-pose compensation twice. Missing data retains the seed.
No animation events, firing mechanics, guest skeleton, allocation or save/replay
fields change.

Player build, full controls suite (original LEVEL00, LEVEL01 and all overlays),
aim and near checks pass. New native assertions cover reload motion, final-key
return within fixed-point rounding, stance/world-hand independence, unchanged
idle/other guns and missing-data fallback. The private `d36h-rifle/final-120`
run captures standing and crouched firing/reload cycles and a save/load during
upper 27, followed by native upper 23. Three replay workers report no failures.
The native final-key reference removes the pose jump from switching directly
between the first-person seed and the different native ready grip.

Evidence stays under `recomp/analysis/d36h-rifle/`. Initial raw-channel captures
are intermediate; `final-120` is the calibrated candidate. User should retest
key 9's Buffalo Rifle firing/cocking motion; D36H remains Needs playtest for that
visual result. The broad positive user report is retained without claiming
unobserved per-costume or campaign coverage.

Reload follow-up executable: `4cd7269e8251f99bcfb2d1ceb8c5f4b2c0d46449d61add7edbc2708b3df5afac`. All 41 player-file hashes
and mtimes match this follow-up intake; private processes are stopped.


## Buffalo video review and revised candidate - 2026-10-26

**U/L:** The user rejected `4cd7269e...` after recording
`research/screencaps/buffalo.mp4` (21.3 seconds, 60 FPS). Extracted sequences
show the rifle rolling down while the left hand rises independently. The user
also reports a vibrating finish. The preceding sparse capture checks did not
establish acceptable motion; that candidate is superseded.

**S:** The native reload wrist delta includes large sideways roll/yaw, suitable
for the original body pose but unsuitable around the fixed first-person grip.
That delta was also reconstructed from the base frame's discrete channel while
replay interpolated the native matrices used by the support arm. This was a
source-level timing inconsistency; its individual contribution to the user's
vibration has not been isolated by a separate A/B recording.

The new rifle-only display pose uses native clip elapsed time (frame durations
and remaining ticks), with smooth raise/hold/return keys and a short support-hand
stroke. Both ends return to the seeded firing grip. The left wrist uses the
retail reload's final two-hand relative grip, so native idle/return wrist motion
cannot shake it independently. Connected elbow/shoulder lengths and endpoints
are retained. The render snapshot carries lift/pull, and the existing blend
hook interpolates these and rifle recoil between snapshots. No guest animation,
event timing, projectile, damage, save format or new animation asset is changed.
Other weapons retain their paths; Vanilla and third-person presentation do not
enter this display path.

**L:** Evidence: `recomp/analysis/d36h-rifle-review/` (intake manifest, user
video sheets, build/test logs, capture scripts and review sheets), plus
`recomp/analysis/d36h-rifle/review-motion60/` and `review-motion120/` containing
continuous Xvfb recordings with six seconds of held fire and two seconds of
settling per standing/crouched stance. The recordings show the rifle lifting
with the hands, the short stroke and the return; 60 consecutive captured
settling images were inspected at the 120 setting without the prior independent
hand movement. Native controls with original LEVEL00, LEVEL01 and overlay
fixtures, aim and near tests pass. Added checks cover native timing, endpoints,
stance/world-matrix independence, malformed frame fallback, other records and
worker snapshot/intermediate-pose restoration. Early synthetic checks failed
because they started at phase zero or compared two points on the hold plateau;
the corrected test samples the rise as well as hold and return.

The 120-setting Xvfb recording uses three replay workers, with zero worker or
plugin failures, but pacing reached divisor 3. It is motion evidence with replay
active, not proof of sustained 120 unique frames per second. Original cowboy
costume, private copied slot 9, standing/crouched were exercised here; broader
costume/campaign acceptance and the user's preferred visual feel remain open.
The initial `review-60` images are an intermediate pitch-direction trial, not
the final candidate. No framework change, commit or push.

Candidate executable: `db2e95b17b2faff913f4ba2893bb9d1272c6dfa3fc4d2e8c7d007de286541517`.
D36H remains Needs playtest. Retest weapon key 9, repeated fire and its return to
idle with the player's normal settings.

The final private `review-load120` run saved and restored during upper 27,
continued through the reload, and returned to subsequent native weapon states.
All 41 player-file hashes and mtimes match this revision intake; private game
processes have stopped. The original media and player saves were not written.


## Acceptance and closeout - 2026-10-26

User: "fully accept! lock it in and close it out!!!!!!". D36H is Done.
Acceptance includes the connected combat/held-item arms and the final Buffalo
Rifle motion revision, executable
`db2e95b17b2faff913f4ba2893bb9d1272c6dfa3fc4d2e8c7d007de286541517`.
Earlier Needs playtest entries above are historical; the rejected native-roll
rifle candidate remains superseded. The documented coverage and performance
limits remain evidence limits, not pending acceptance of this job.

Authored implementation, regression tests, manual, board, status and handoff
are included in the authorized MusicMonsterMod commit/push closeout. No gameplay
change, rebuild or game launch is needed for closeout. D36I-L remain unstarted;
D37A/D19Q3 retain their separate Needs playtest statuses. Player saves, captures,
retail assets and build output remain local.
