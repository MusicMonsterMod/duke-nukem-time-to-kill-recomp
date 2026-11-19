# D11E1 - First person kept through a lapse of the water plane while swimming

Job: [D11E1](../jobs/first-person.md#d11e1---first-person-drops-while-swimming-underwater-level-6-moat).
Status: Done (user-accepted 2026-11-19).

## 1. The report

Field laptop playtest, 19 November 2026 (18 November build copy, real screen,
first person, Double Duke active): first person dropped to the third-person
camera a few times while Duke swam underwater in the FAMILY JEWELS (level 6)
moat. The session log has ten refusals of the camera lease in about 5 s
(game frames 7100-7404), all of one kind:

```
[TTK lease] inactive (unsupported-camera) anim=128 state=5/5 flags=00000002 cam=800d7198 ident=1 ctx=1
[TTK lease] inactive (unsupported-camera) anim=127 state=5/5 flags=00000002 cam=800d7198 ident=1 ctx=1
[TTK lease] inactive (unsupported-camera) anim=127 state=5/5 flags=00000102 cam=800d7198 ident=1 ctx=1
```

Native underwater swim (mode 5, thrust 128 and idle 127), the normal camera
owned by Duke, no script or underwater-inhibit bit. `+0x224` bit 1 is the
running bit; it stays set after a running entry into the water (reproduced),
so it says nothing about the cause. Double Duke only doubles Duke's damage.

## 2. What refuses

The camera hook (`0x8003ade4`) refuses when `camera_eligible()` is false,
which is `state(true,true)` (the presentation lease). For a native swim pose
(122-133) the only branch that admits it is `swimming = in_water_early()`.
With the owner and flags in the log, the refusal leaves three candidates:
Duke's word 0 bits `0x20002` (dead, frozen), the back-pointer `+0x7d4`, or
`in_water_early()` false. For a Duke below the surface, `in_water_early()`
fails only when the water plane `+0x834` reads `0x40000000` (none).

The plane is not fixed per level. The executable stores `+0x834` at three
places (`0x80048128`, `0x80051ca8`, `0x80056008`), each from the same query
`0x8005b85c(0, &root, room byte +0x2e, &+0x838, &+0x83c)`, which searches the
current room's water polygons above and below the root and returns
`0x40000000` when it finds none. The native swim state machine continues
through such a result; the host refused the camera on it.

## 3. Reproduction

About 25 minutes of private offscreen swimming in the level 6 moat (walk and
run entries, the whole south and east moat at surface, mid and bottom depths,
firing, hits through Duke's damage handler, a corner grid) never produced a
natural lapse; the spot and cause of the player's 5 s remain unknown.

The mechanism reproduces directly: writing a dry room into `+0x2e` under
water (the game restores it on its next update) gave the player's exact line
(`anim=127 state=5/5 flags=00000102`, unsupported-camera). That write races
the game's update (one of five runs hit), so the A/B uses a developer test
word instead: `nowater` in `DNTTK_TEST_INPUT` (held) writes the query's
no-water result into `+0x834` at the start of the camera update; the game's
own update writes the plane again. The extended refusal trace on the base
build then reads `word=00000000 back=1 room=1 plane=40000000 y=9658
mask=0007ffff`: the plane is the failing input.

## 4. The change (presentation only)

- `native_swim_pose_early()` (`modern_controls.cpp`): mode 4 or 5 with
  animation 122-133, the native swim states (nothing else selects them).
- `state()`: the presentation lease (`presentation=true`) counts a native swim
  pose as swimming when the plane lapses (`swim_presentation_early()`); the
  action leases keep `in_water_early()`. Death, frozen, script, owner and
  back-pointer guards are unchanged.
- `first_person_swimming()`: the same native pose keeps the swimming eye and
  weapon view; the surface eye (mode 4) uses the body offset (294 above the
  root) when there is no plane, instead of the plane height. Normally they
  agree within a few units (8094 against 8096 in the test).
- Diagnostics: each lapse episode logs once (`[TTK swim] water plane lapsed
  under the native swim (episode N: ...): view kept`, at most every 2 s); the
  `fp` debug JSON has `swim_plane_lapses` (episodes) and
  `swim_plane_lapse_checks`; every `[TTK lease] inactive` line now also names
  word 0, the back-pointer, the room, the plane, root y and the water mask.

Not changed: the swim movement lease (`swim_lease_ready` needs `+0x20c` and a
plane) and the depth helpers that read `+0x834`. During a natural lapse the
modern swim steering may still pause; that has not been reported and could
not be observed without a natural lapse.

## 5. Evidence (ripspace, offscreen, private profile and cards)

Lab `recomp/analysis/d11e1/` (local). `lapse.py`, level 6 start, four phases
with 2 s of held `nowater` each; samples with first person effective:

| Phase | Base (test word, no fix) | Fix |
| --- | --- | --- |
| Underwater idle | 1 of 54 | 58 of 58 |
| Underwater thrust | 1 of 57 | 58 of 58 |
| Surface idle | 1 of 56 | 53 of 53 |
| Surface swim | 0 of 57 | 57 of 57 |
| Lease refusals, first-person fallbacks | 4, 3 | 0, 0 |

Third person (same test): lease refusals 5 base, 1 fix; the one left
(`anim=105 state=0/10`, a landing after jetpack mode 10, on land without
water) is on both builds and outside this job.

Regression without lapses, base against fix: the free-swim pattern
(strokes, turns, dives, surfacing; about 2,450 samples each) identical, no
refusals; a first-person water exit (swim, water-exit, mantle, ground) in
first person throughout on both. `python3 -m unittest discover -s
recomp/tests/local` 266 tests OK; `ttk-input-test` passes; `ttk-aim-test`
does not link on this tree (`psx_mod_replay_active` undefined in
`weapon_aim.cpp`, unrelated and not built by CI).

Field laptop (HD 5500, real screen, the fix built there from the same
sources, 79 s): `lapse.py` first person 213 of 213 lapse samples effective, no
refusal, no fallback, three episodes logged; third person one refusal, the
same jetpack landing; the no-lapse free-swim pattern without a drop or a
refusal. The laptop's player build now carries the fix; its previous binary
is kept beside it as `Duke_Nukem__Time_to_Kill_Recompiled.d11e1-before`.

Vanilla: the camera hook refuses before `state()` without Modern controls,
and the test word needs `DNTTK_TEST_INPUT`; neither path changes.

## 6. The playtest (accepted)

Field laptop, real screen, 2026-11-19 (`play-20261119-150713.log`). User:
"great work. it's very clean playing". The log has three natural lapses,
all kept in first person: room 20, underwater thrust 128 at 5/5, root y
11932, 11990 and 12694 (3700-4500 below the 8192 surface; the lab swims
reached about 2650), water mask 0. So the native water query finds no water
near the bottom of the south-east moat room. One other refusal, unrelated
to water: anim 191 at state 8/0 (a pole or chain step-off clip entered from
ground mode, word 0x200) in dry room 42.

## 7. Remaining

- The natural trigger is the deep part of room 20; why its water polygons
  miss there was not traced further (the view no longer depends on it).
- The swim movement lease still needs a plane during a lapse.
