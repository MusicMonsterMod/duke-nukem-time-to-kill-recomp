# Running weapon selection, running presentation and frozen view intake

2026-10-26. Backlog only. User requested three jobs and permitted "a touch of
investigation now". No new gameplay implementation, build or game launch.
D36H1 is separately accepted; its closeout does not start these jobs.

Evidence labels follow note 257: U/H = user report or unverified hypothesis;
S = inspected source; A = accepted prior behavior. No fresh L reproduction.

## D08A29 - Wheel weapon selection while running

**U:** allow scroll-wheel weapon changes without stopping the run.
**S:** `pc_input.cpp:input_event` accepts captured SDL wheel notches and queues
`weapon_previous` / `weapon_next`. Alt diverts the wheel to camera distance;
there is no Shift/run exclusion in this branch. Diagnostic input already supports
`wheelup` / `wheeldown`, so a private reproduction can exercise the real event path.

`shortcuts.inc:select_weapon` acknowledges commands at entry and advances
`command_seen` before later eligibility checks. Ground selection requires
`movement_ready()` (lease plus identity), permissible equipment, no held
fire/interact/original aim, clear flags 0x2c5 and a stable upper-body idle loop
(5/20/29/35/39). Flight/swim have dedicated eligibility. **H:** a run-state upper
animation or eligibility gate could consume a notch without switching. This is
an investigation point, not a demonstrated cause or permission to remove gates.

Next selection: compare idle/walk/run wheel input, command serial/count, lower
and upper animations, flags, lease and requested/actual equipment. Include Shift,
RUN MODE, diagonal/backward/strafe movement, both wheel directions, number-key
comparison and an eligible inventory. Preserve genuine reload/throw/fire safety,
Alt-wheel, menu navigation, swimming/flight and Vanilla. Acceptance is in the board.

## D36H3 - Restrained two-handed running motion

**U:** excessive right-hand motion while running, specifically shotgun key 3,
Gatling 4, Energy Weapon 7, Flamethrower 8 and Freezethrower 0. These correspond
to base display slots 5, 7, 10, 9 and 11 respectively; verify actual records live.

**S:** `first_person.inc:display_pose` normally uses each known firing seed for
the ready weapon pose; `DNTTK_FP_WEAPON_POSE=animated` is an existing diagnostic
alternative. `prepare_display_matrix` owns the shared right-hand/weapon anchor;
`display_arm_matrix` obtains the native relative arm geometry and solves connected
segments toward offscreen shoulders. Thus apparent hand motion must be separated
from forearm/elbow motion, camera bob, native firing animation and replay before
choosing a fix. A blanket reduction of camera bob would not establish the cause.
Crouch/swim reconstruction and the accepted Buffalo reload have separate paths.

**A:** preserve D36H connected endpoints and the newly accepted D36H1 underside
contact. **H:** native segment motion or a presentation transition may explain the
report, but source inspection alone does not identify the offending transform.
Next selection: matched idle/walk/run/start/stop/strafe captures with joint and
weapon matrices, actual upper states and 60/120 samples; assess the five families
and upgrades. Tune only proven visual motion, keeping aim/muzzle, recoil/events,
bone connection, view choice, lighting, save/replay and Vanilla contracts intact.
Read note 257; read style guide 265 if later work requires new mesh/material art.

## D35J5 - Frozen first-person blue vision

**U:** freezing switches the view to third person; user wants to stay first person
with blue vision, citing Duke 3D as the visual target. This describes requested
behavior; no external reference implementation was inspected in this intake.

**S:** `first_person.inc` documents fallback for unsupported states;
`modern_controls.cpp:state(camera_only, presentation)` enumerates supported
animations/modes and checks actor/camera ownership and inhibit bits. Its existing
presentation-only exceptions distinguish eye continuity from movement permission.
This is the right boundary to investigate, rather than enabling player movement
while frozen. No authoritative frozen animation, flag or timer was identified in
this small pass. **H:** frozen-state eligibility may trigger the reported fallback;
verify with real frozen and thawing state captures before widening any predicate.

Next selection: reproduce native enemy freezing using private saves, identify
entry/hold/thaw/death/shatter and load behavior, trace the perspective rejection,
and establish where a blue full-viewport tint belongs. Keep frozen gameplay
restrictions, duration/damage and shattering native. Clear tint reliably on exit,
Continue/load/view changes; respect explicit third person and Vanilla. Test HUD,
widescreen, water/gas/night vision coexistence and 60/120 replay. No frozen-state
address, visual implementation or new preference is prescribed without evidence.
