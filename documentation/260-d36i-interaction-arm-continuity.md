# D36I - Native interaction arm continuity

2026-10-26. Done, user accepted and lock-in/closeout authorized.
The accepted [D36 audit](257-d36-first-person-presentation-audit.md) and
[D36G ownership contract](258-d36g-limb-ownership.md) remain authoritative.
D36J-L are unstarted. D37A/D19Q3 retain their separate Needs playtest statuses.

## Accepted implementation

Push and Use now admit native arm parts 10-15 through the existing guarded
joint-depth bridge. The original actor loop otherwise rejects an entire joint
whose origin is nearer than GTE H, even when part of its mesh is visible.
The bridge changes only the loop's saved depth; the existing polygon clipper
clips real geometry at its near plane and preserves world depth and lighting.

Native joint matrices, arm poses, object travel, collision, callbacks and event
timing are unchanged. This is not a weapon-overlay arm pose: hands remain
occluded by scene geometry. D36H combat/held-item arms still own equipment-1/2
presentation, and hidden arms retain their exclusion. The existing caller,
primary Duke draw, camera, stack, effective perspective and part gates remain.
Ladder/traversal admission, Mirror policy and Vanilla keep their existing paths.
There are no new allocations, assets, serialization/replay fields, hooks or
framework changes.

## Diagnosis and verification

**S - source/native:** the complete controls, input, aiming and near-clipping
suites pass, including the original LEVEL00 fixture, LEVEL01 and all overlay
fixtures. D37A animation restoration/GPU clearing checks pass. New assertions
exercise all six semantic arm parts versus torso/head, origins behind the eye
and on both sides of H, original matrices/projection, world-depth packets,
wrong actor/caller/stack camera, draw-boundary retirement, action Third Person
and Vanilla. Existing tests retain replay/load ownership, attached equipment,
weapon/kick, action safety, movement and ladder contracts.

**L - slot-8 baseline:** reproduced 121 hold, 120 push and 119 pull with a private
copy of the current LEVEL05 save. In one forward sample the six native origins
were approximately 64/173/260 and 113/239/267 against H=256. In a reverse sample
they were -11/81/139 and -123/6/130. The unmodified bridge excluded Push, matching
the detached hand/forearm captures. Depths are derived from sequential debug
matrix reads, not atomic GTE samples. They support the source-confirmed cutoff
mechanism, not attribution of every native pose motion to culling.

**L - candidate:** local evidence is `recomp/analysis/d36i/`. Runs use copied
saves/cards, private settings, dummy audio and Xvfb OpenGL on port 9489.
`candidate60` and `candidate120` cover slot-8 entry, push, reverse, hold,
look/yaw, attempted weapon switch, private save/load and release. Visible
forearms now continue to the hands instead of ending at a rejected joint.
The original motion can still move arms out of the view. No constrained contact
pose was introduced: native motion/framing remains a human visual acceptance
item, distinct from the corrected origin cutoff.

The corrected routes reach wheel 270, vending 277, keycard upper 47/item 6 then
266, crystal upper 47/item 11 then 267, and ladder 186 as the control. Reviewed
wheel/vending captures retain their native contact geometry. `candidate120`
saves/loads during wheel, vending, crystal handoff and ladder; three replay
workers report zero failed jobs and zero plugin failures. The first attempts
that only stowed/redrew a weapon do not establish use coverage; the later
`coverage60` approach/hold route reaches the actual keycard callback and weapon
return. Slot numbers and save identities are recorded by the local copies.

The first 120-setting motion script used fixed wall-time delays during a slow
private startup. Its recording did not reliably reach the intended push action,
although later debug samples did. It is not accepted motion coverage.
`motion120-synced` waits for guest frames instead, reaches 121/119/120 and records
two forward/reverse/hold cycles, then release, without intermediate screenshots.
Its replay workers also report zero failures. Reviewed motion shows connected
native arms; this is not proof of 120 distinct frames/s (pacing reached divisor
3). The diagnostic process was CPU-heavy; a contemporaneous host check found
ample available memory and no backup process among the leading CPU consumers.
Those observations do not prove the cause of every diagnostic delay or establish
a player-session performance regression.

`coverage-third60` verifies actual Third Person push/use preferences, global
view commands and pause/release recovery with guest-frame waits. Earlier `p`
and `escape` diagnostic words were unsupported; they are not counted as view
or pause tests. The corrected run uses `act=camera_view` and `esc`. Native tests
also exercise both choices and Vanilla independently of that input harness.

**L - bounded cost/restore checks:** three 3-second private 60-setting windows
with the same slot-8 hold route measured 59.94-60.19 guest frames/s for the
candidate versus 59.90-60.28 for the saved baseline executable. The last two
aggregate frame averages were 16.683 ms in both runs; primitive averages were
425-426 versus 415. These capped Xvfb samples include pacing/driver variation
and do not establish uncapped cost or campaign-wide performance. The initial
relocated baseline lacked its adjacent BIOS and never booted; the valid
comparison places the copy beside the same build assets.

Across the sampled candidate routes, packet-arena peak was 138144 of 139744
bytes, host geometry peak 19960 bytes, with zero world/object budget hits, packet
skips or copy overflows. This does not certify unsampled scenes. All 41 player
file hashes and mtimes match intake; saves/cards/settings were not modified.
Private game and recording processes are stopped.

## Acceptance and evidence limits

The user confirms the arms are solid, native animation is unaffected, and the
result is complete and foundational. Original body motion and shoulder framing
are accepted. This acceptance preserves the bounded verification above. Natural damage interruptions, every costume/campaign interaction,
dark/gas/power-up combinations and a live software-renderer sweep were not
newly certified. The full combined presentation matrix remains D36L's scope.
The native tests and sampled live cases are bounded evidence, not full-campaign
certification. The user explicitly authorized lock-in and closeout, including
implementation, tests and documentation commit/push as MusicMonsterMod.

Launch with the player's normal saved settings:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Gameplay capture is automatic. P selects first person; PUSH/PULL VIEW and USE
VIEW should be FIRST PERSON for this visual check. Check slot 8 while holding
Grab (right mouse or Alt), W/S, look down/up, release and regrab. Then check
wheel/vending and card/crystal use, save/load and the third-person choices.
`--vanilla` remains the original profile for one launch.

Accepted executable:
`e6e6922f51217d2383d56458b31dfd2784952a4c192b6d62cf54e6b9641565ea`.


## Closeout - 2026-10-26

User: "correct, the arms are solid. native anim is unaffected as mentioned.
this is perfectly done and foundational. lock it in and close us out".
D36I is Done. No gameplay changes, rebuild or game launch during closeout.
Authored implementation, tests, manual, board, status and handoff are included
in the authorized commit/push. Existing framework modifications remain untouched.
D36J-L are unstarted; D37A/D19Q3 retain their separate Needs playtest statuses.
No next job started.
