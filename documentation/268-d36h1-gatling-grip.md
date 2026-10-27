# D36H1 - Gatling underside grip

2026-10-26. Done, user accepted and closeout authorized. Selected as a small hand-placement fix.
The accepted D36 evidence/ownership contract in [note 257](257-d36-first-person-presentation-audit.md)
and narrow intake in [note 263](263-weapon-grips-and-medieval-wall-intake.md) apply.

## Change and evidence

**U:** the left hand should support the Gatling underside at first-person idle.
**S:** the native left wrist is transformed relative to the accepted right-hand
weapon matrix. There was no Gatling-specific contact offset.
**L:** matched private idle captures show an air gap before the change and palm
contact after a small translation. Evidence is local in `recomp/analysis/d36h1/`:
`before`, `candidate`, `final120` and `swim120`, including cropped grip comparisons.

`display_arm_matrix` shifts the Gatling left chain 12 camera-space units right
and 12 up before its connected endpoint solve. This is a 3D placement change,
not a fixed screen-pixel offset. It preserves wrist rotation, native bone lengths,
the right-hand/weapon anchor and the existing native firing motion. The normalized
slot gate includes base record 7 and challenge record 28. No geometry, texture,
animation event, save field, allocation, gameplay, framework or preference change.
Other weapons, native interactions, reflections, third person and Vanilla retain
their existing ownership paths. The Buffalo flicker job remains separate.

## Verification and limits

**S:** player build and existing controls, aim and near native suites pass.
Controls used the original LEVEL00 guard fixture, owned LEVEL01 and all level
overlays. Existing endpoint tests include base/upgraded Gatling and both costume
orders, plus unchanged native matrices, ownership, replay and Vanilla guards.
No new test duplicates the numeric visual tuning.

**L:** offscreen OpenGL, dummy audio, isolated settings/cards copied from prior
private fixtures, port 9488. Verified record 7 at base and campaign upgrade
flags (0x0009 versus 0x0001); challenge record 28 is native-test coverage only.
60-setting captures cover idle, fire, settle and crouch. The 120-setting pass adds
pitch, movement, Q/return samples, switching to pistol/back, private save/load
and jetpack (observed animation 164). Separate swimming captures show 127/128,
Gatling firing upper 33, tinted lighting and save/load. The swimming harness
confirms actual first -> third -> first changes; the earlier diagnostic
`act=camera_view` attempt did not toggle and is excluded from that claim.
Both 120 runs report three replay workers with zero worker/plugin failures.
These are bounded visual/lifecycle checks, not a performance benchmark.

The existing firing pose raises the hand above the barrel in baseline and
candidate captures. This narrow idle-contact adjustment preserves that motion;
it does not redesign the firing grip. Sparse captures do not certify continuous
motion, every costume, mirror/gas/power-up combination or the full campaign.
No fresh software/Vanilla live sweep. User visual acceptance was subsequently received; these coverage limits remain.
Player save/card/settings hashes and mtimes are unchanged; private runs stopped.

Candidate SHA256: `8783c0b34f51f73255a0dcf5192dbff00ccd392ad0906009b1b86dd47ffe51a3`.
The user subsequently authorized implementation/documentation commit and push.

## Playtest

Launch with saved preferences, select first person with P if needed, then key 4.
Judge the underside contact at idle and after firing, crouching and switching.

```sh
python3 recomp/tools/local/run.py
```


## Acceptance - 2026-10-26

User: "lock it in and close it out, this current work is a winner". D36H1 is
Done on that acceptance, with the candidate hash above now the accepted build.
No further code change, build or game launch for closeout. D08A29, D36H3 and
D35J5 were requested as backlog only, with a small source investigation permitted.
