# D35 - First-person transition audit and modernization roadmap

**D35K slot-5 finding (2026-10-25):** The reported door drop is native pose
116 in the ordinary reciprocal camera, not an alternate scripted view. A
bounded USE INTERACTION VIEW fix and visible right-leg kick are accepted as
D35K1 Done; see [note 237](237-d35k-slot5-door.md).
The broader scripted-event inventory remains open.


**D35L closeout (2026-10-25):** The user accepts and prefers the existing
third-person death/respawn presentation. D35L is Done with no action needed;
its historical research/option proposal below is superseded. No new death
variant testing is implied. D35K remains Todo and this parent remains Needs review.


2026-10-24. Research candidate, Needs review. No gameplay implementation.

## Scope and evidence

The goal is uninterrupted first-person gameplay when selected, with per-action
legacy third-person choices and unchanged global third-person/Vanilla play.
This note inventories the current host transition mechanisms and their native
inputs. It is not a claim that every campaign script has been played or that
all indirect guest calls have been resolved. Unknown event-to-state mappings
are explicit research deliverables below, not fabricated confirmations.

Audited sources: all title-source perspective selection/blend/release users,
normal camera and input ownership, traversal classifiers/handlers, weapons and
render replay, menu/profile/launcher configuration, relevant native tests,
resident generated camera dispatch (read-only), framework camera-related APIs,
and accepted D11E/D11F/D26B/D10B evidence. Searches included `first_person`,
`fp_blend`, `input_set_view`, `camera_world_reset`, camera owner/pivot fields,
traversal/animation predicates, interaction dispatch, and camera setting names.
Generated code and local analysis are evidence only; neither is a deliverable.

Baseline: root implementation was clean apart from existing modified pinned
`recomp/psxrecomp` sources. Framework pin `08ec704a974b1f3a16335b4afeb340b9eff19926`,
UI pin `be8ac1d03ee19d55394b5a5f2d9d1506edd56659`. Those changes were preserved.
Latest accepted player build in status: D10B, SHA-256
`4396ea72a48d95b6de13999501ff8b4f53f46515533b8aa7d7a6ca42d54429eb`.
No build, game launch, player settings/save writes, new gameplay test, commit or
push was performed for this audit. Existing test source is evidence of intended
coverage, not a new passing run.

Evidence labels: **C** = confirmed current source mechanism; **A** = previously
accepted implementation with bounded recorded tests; **U** = user-observed
transition; **S** = suspected concrete event, requiring a trace. A source guard
can be C while its attribution to a particular door or damage event remains S.

The user added **opening doors** and **mirror interactions** during this audit.
Both are U findings. Neither is dismissed because no named host predicate exists.
Exact location/animation/owner traces are not supplied yet. Reflecting Duke in a
mirror is a separate render pass, not proof of mirror-interaction support.

## 1. Camera architecture and complete host transition boundary

Source links use stable function names; line numbers below describe this audit
snapshot. Paths are relative to the repository root.

| Layer | Source and contract |
| --- | --- |
| Selected view | `recomp/src/ttk/pc_input.cpp:99`: `first_person`; initialized from `DNTTK_CAMERA_VIEW` at 286, changed by `camera_action(camera_view)` at 104 and `input_set_view` at 346. `InputFrame.first_person` transports the request. The only production caller of the setter found is menu `apply_live`. Temporary fallback never changes this boolean. |
| Global prerequisites | `modern_controls.cpp:222` `independent_camera`: Modern controls plus Independent camera. `camera_eligible` at 735 requires Modern controls and `state(true,true)`. Vanilla, explicit Legacy controls or Original camera suspend the eye view, retaining the preference. |
| Original camera ownership | Normal camera is `0x800d6eb0`, player `0x800d7198`. `state` checks `camera+0xa4 == player`, `player+0x7d4 == camera`; `pivot` at 233 checks both `camera+0x4c/+0x50 == player+0x7bc`. These are separate ownership and anchor tests. |
| Original dispatch | Resident loop `0x80025e88..0x80025ee8` tests actor `+0x224 & 0x20000000`: set selects alternate camera `0x800d7010` and `0x8003b4f4`; clear calls normal follow `0x8003ade4` with return `0x80025ee8`. An alternate path can bypass the modern normal-camera hook altogether. |
| Presentation eligibility | `modern_controls.cpp:655` `state(camera_only,presentation)`: scene words `0x800bcbb0 == 1`, `0x800d2540 == 0`, actor flags `&0x20002 == 0`, supported animation/mode/history, owner pointers and inhibit mask. It deliberately ignores ordinary pause/input capture for presentation. Action leases remain stricter. |
| Hard release | Normal-camera hook at 1409: on ineligibility/identity failure, invalidates orbit and calls `first_person_release`. Reasons: `legacy`, `identity`, `frontend`, `scripted`, `unsupported-camera`. `scripted` specifically means owner mismatch here; other script flags can report `unsupported-camera`. |
| Orbit acquisition failure | `orbit_begin` at 295 rejects Original camera, invalid pivot or initial radius outside 256..8192. Hook releases with `original-camera` or `orbit`. `orbit_constraint` at 398 can also invalidate the orbit if the pivot changes later. |
| Deliberate action fallback | `first_person.inc:104` `first_person_update`: rejects push; rejects traversal except committed roll/slide and unowned fall; explicitly rejects early water-to-mantle. `wanted = selected_first && supported`. Blends with `1-pow(0.7,ticks)`; acquisition starts directly at 0 or 1. Reasons `push-object`/`traversal`. |
| Eye positioning failure | `first_person_anchor` at 207 can return false if neck/model matrices are invalid. `orbit_constraint` then uses the orbit anchor even with positive blend. This is an indirect visual fallback, not necessarily a logged `fp_fallbacks` event. Model/identity safety must not be removed. |
| World reset | `camera_world_reset` at 1014 clears blend, orbit, eye offsets and transient draw ownership on savestate load, authenticated overlay change, and guarded level teardown `0x8002b9f4` (callers at 1077). It does not clear selected view. Next supported acquisition is immediate. A load resuming inside draw can temporarily precede reacquisition; native tests explicitly cover reset then recovery. |
| Drawing | `first_person_draw`, projection, constraint/follow and weapon-joint hooks in `first_person.inc` depend on blend/ownership; head hiding is scoped to Duke's primary draw. Losing the draw lease can reveal Duke even before another normal camera update. Missing head data is distinct from changing the selected view. |
| High-refresh replay | `modern_controls.cpp:1635..1705` `RenderState` save/load copies blend, orbit, eye and weapon eligibility; `frame_replay.cpp:655` `apply_late` rotates about the eye only at blend >=0.999, otherwise the orbit pivot. Late-view rejection retains the interpolated camera, not a deliberate third-person setting. New policy must be snapshotted, not inferred from stale worker statics. |

These are the discovered production writers/consumers capable of changing or
invalidating host first-person presentation. No second automatic writer to the
selected-view boolean was found in weapons, animation overrides, frontend,
level-select, generic runtime or UI code. Animation and guest scripts affect
presentation indirectly through the fields above. A substring search cannot
prove all runtime-generated guest addresses or all level events are known.

### Native script evidence and unresolved call graph

Read-only generated evidence lives in `recomp/generated/SLUS_005.83_full_06.c`
(dispatch loop) and `_13.c` (camera routines):

- `0x8003b36c` sets actor `+0x224` bit `0x20000000` at `0x8003b398`, initializes
  the alternate camera from an object transform, assigns its target at
  `camera+0x50` and owner at `+0xa4` (`0x8003b458`). This is a confirmed native
  special-view entry, not yet a confirmed mapping to any particular mirror.
- `0x8003b474` tests and clears that bit at `0x8003b4b4`, sets `0x20`, and
  calls `0x800286b8` on the actor's camera to restore it. Resident direct calls
  found in `func_800434FC` (return `0x80043588`) and `func_800412A4`
  (return `0x800427d8`, including its generated alias).
- `0x8003b4f4` updates the alternate view using `0x800286b8`, `0x80028f2c`
  and `0x8002b3c4`, conditional on globals `0x800c3d28`/`0x800c3c2c`.
- A co-occurrence scan for the script-bit constant and actor-offset stores also
  finds `0x800434fc`, `0x8005201c`, `0x80094730`, `0x800412a4`. This is a
  candidate list, not proof each sets the bit or changes perspective. Masks,
  initialization, clearing and unrelated bit operations can match it.
- No resident direct call to `0x8003b36c` was found by the generated CPS-call
  search. Indirect callbacks/overlays must be traced before naming all events.
  Do not remove the script bit globally to force an eye view.

### Classifier details needed by implementation jobs

The lower animation is actor `+0x60`, upper animation `+0x74`, current/previous
modes `+0x22c/+0x22d`. `state` permits land 63, 72..79, 178..184 and original
steps 82..85/88..91; camera-only also admits 70/71, 80/81, landing 94/95/106,
175..177 and the specialized branches in the matrix. A pose outside that union
is an unsupported-camera candidate unless water/flight/traversal branches admit
it. Do not label every unsupported numeric clip as damage or door opening.

`in_water_early` requires a valid water plane (`+0x834 != 0x40000000`) and
submerged root or nonzero water flag `+0x20c`. Shallow land water excludes modes
4/5 and uses floor-minus-water depth below 0x281. Thus swimming presentation is
not determined by animation alone. The general inhibit is `0x20000200`; shallow
land water and qualifying FP swim presentation remove only 0x200 from that mask.
Upper-body interaction bit 4 blocks several action/restoration predicates but
is not itself a universal FP rejection in `state`. Door/mirror diagnosis must
therefore inspect lower pose, modes and ownership rather than assume bit 4
explains the user's report.

## 2. Action audit matrix

There are currently **no action-specific perspective toggles**. In the Toggle
column, 'none' means no such toggle; existing movement settings are listed
separately below. Global VIEW applies to all supported first-person actions.

| Action / event | Current behavior and trigger | FP support / evidence | Toggle | Next step |
| --- | --- | --- | --- | --- |
| Ground gait, crouch and crouch walk | Allowed land/step poses, D08K posture support | A, stays FP | Global VIEW / VIEW BOB | Regression only |
| Ordinary/assisted/manual jump, airborne fire | `jump_animation`, `eye_air`, owned flight | A, stays FP | None needed for gap repair | Regression only |
| Unowned fall / wall deflection | 107/108, mode 9, history 0/9; exception to traversal fallback | A, stays FP for recognized states | None | Regression; unusual histories in D35J |
| Rolls, slope slide and original backflip | 157..162; slide mode 2 / 99/100 and landing latch; `committed_move_early` | A, keeps FP, original motion owns committed move | Movement toggles are not view toggles | Regression/comfort under D21 |
| Landing / wall bump | 94/95/106 and recognized 105 transitions | A/C, supported histories | None | Regression; unknown reactions D35J |
| Push, pull, grab | 119..121, mode/history 0/0, holstered, valid pushable object; explicit exclusion | C, third-person orbit to avoid eye inside object | None | D35D |
| Ladder mount/climb/exit | Host top-mount; mode 3 186..189, mount/exit 185/190; airborne/bottom transfers 147/148/156/207/210/211 | C, orbit; no action FP | None | D35E |
| Chain/pole/lamppost climbing | Mode 3 192..195; catch/top mount 154; exit 191/196 | C for pole-class objects; lamppost classification must be verified per object | None | D35F |
| Climbable walls | Mode 3 198..205, broad climb predicate includes neighboring transfers | C predicate, historic wall route not played | None | D35I |
| Ledge catch/hang/shimmy/pull-up | Mode 6 147..153; mode 7 149..153; mode 8 134..142, guarded previous modes | C, explicit third-person traversal policy | None | D35G |
| Overhead/monkey-bar traversal | Mode 7 149..151, attached object climbable flags ==0x80 | C/A movement, no FP presentation | None | D35H |
| Surface swim, dive, underwater travel/fire | D11E surface water-plane eye, underwater root eye | A, supported FP | None; SWIMMING is pace | D35B configuration only |
| Water climb-out | Mode 8, previous 4/5, 134..142; explicit early `swim_exit` rejection even with lingering water flag | C, still third person | None | D35G owns this, no duplicate swim implementation |
| Jetpack all three schemes | Mode 10 163..170, stable root eye; off/fuel-out 108 and landing history 10 bridged | A, supported FP | None; JETPACK is scheme | D35C configuration only |
| Door opening | User reports forced third person; common E path stows, interacts, then redraws | U; exact lower/upper pose, inhibit bit or special owner not yet established | None | D35M, dedicated reproduction and implementation job |
| Mirror interaction | User reports forced third person | U; interaction callback/animation/owner mapping unresolved | None | D35N, separate from reflection rendering |
| Switches, locks, mission-item insertion | Finder `0x80077f28` through `0x80051890`, native object callback; mission item upper animation 47 | C routing, S unwanted view switch for these variants | None | D35M investigation; split only if distinct mechanism confirmed |
| Ordinary weapons, reload/draw/holster, throws, quick kick | Display hooks do not change selected view. Kick converts 112..115 initial request when eye-ready; root state still matters | A supported set, no universal weapon fallback found | FP weapon display is not action perspective | Regress D12/D12A/D12D; transient failures D35J |
| Native held inventory/use transient | `+0x224 & 0x200` rejects presentation except shallow water / D11E qualifying swim eye | C guard; occurrence depends on native interaction | None | D35J; preserve action suspension |
| Damage, stagger, knockdown, hazards | Actor flags/unsupported lower pose/history can reject `state`; not every damage hit changes pose | C guard, S event-specific coverage | None | D35J traces before widening |
| Death, drowning, dismemberment | Actor `&0x20002` and unsupported/death states revoke modern camera; original owns presentation | C/A general death fallback, variants unverified | None | D35L research/design, no automatic gameplay lease |
| Scripted/special camera, potential turret use | Foreign owner/pivot or `0x20000000` native alternate dispatch | C mechanism; individual event list incomplete | None | D35K inventory; mirror/door examples owned by their jobs |
| Respawn/Continue/restart/level travel | World teardown/reset, then immediate selected-view acquisition when eligible | A/C mechanism, not blanket campaign certification | Global saved VIEW | D22 route coverage, D35L recovery cases |
| Save/load | Reset stale pointers, reclaim hidden head bits, reacquire supported eye | A/C; pre-update draw gap is observable in native fixture | Global saved VIEW | D22 regression; do not reopen cancelled D11G without reproduction |
| Pause, console, focus/capture loss | Presentation lease distinct from action lease | A D26B, should retain FP | Not an action view option | Regression; no new fix job |
| Frontend, attract/demo, statistics, cutscene/FMV | Guest scene/ownership supersedes player presentation | C; noninteractive views are not playable first person | No blanket override | D35K define scope; preserve authored cinematics |
| Foreign code/model, bad pivot/radius | Identity or anchor validity fails | C intentional safety fallback | Never user-overridable | Diagnose actual failure, preserve guards |
| Mirror reflection draw | Primary Duke head/arm edits scoped to primary draw; mirror caller `0x8003816c`; D11F flames rearmed | A/C rendering, not interaction support | No interaction toggle | D35N regression, no duplicate D11F job |
| Two-player/split-screen | Host eye code uses fixed primary player/camera; not generalized per player | C architecture limitation | No per-player FP policy | Existing D34B owns classic split-screen; no expansion here |

## 3. Existing swimming and jetpack coverage

**Swimming can remain first person throughout the swim itself.** D11E is Done.
`first_person_swimming` uses `in_water_early && !shallow_land_water_early`;
`first_person_anchor` pins X/Z to the root, Y to surface minus 96 in mode 4,
or root minus 294 underwater. Swimming bypasses land gait bob/glide and
reconstructs animated weapon channels without double shoulder compensation.
The D11E presentation exception permits native equip bit 0x200 for modes 4/5,
animations 122..133; script, death and ownership guards remain. A water exit
that becomes a mantle still deliberately leaves FP. This is a mantle gap.

**Jetpack can remain first person through supported takeoff, flight and exit.**
D11F is Done for Modern, Classic and EDuke32. Root minus 294 eye avoids lean
sway; pack-off/fuel-out pose and mode-9 previous-10 fall/landing transients retain
presentation without enabling movement. Flight-ending water/contact/death can
enter another policy; entry into an unsupported climb still leaves FP.

See [D11E accepted evidence](216-d11e-first-person-swimming.md) and
[D11F accepted evidence](220-d11f-first-person-jetpack.md). Those notes retain
bounded route, costume, weapon, water-exit, death and renderer limitations.
Their acceptance is not revoked by this audit. No new defect is inferred merely
from untested combinations. Existing native fixtures at
`recomp/tests/local/modern_controls_native.cpp:3016..3189` exercise ownership,
loads, swim anchor/flags/early mantle, flight poses/schemes/death and recovery;
these were inspected, not rerun. Historical D11 docs/launcher descriptions
claiming universal swim/jet third-person fallback are superseded by source.

## 4. Existing settings and proposed shared architecture

Current schema is 40 in `recomp/tools/local/player_profiles.py`. Defaults include
`view=third`, `camera=independent`, `scheme=modern`, `swimming=modern`,
`jetpack=modern`. Preserve the user's global view; this roadmap does not change
it to first person by default.

- `game_menu.cpp:349` VIEW is on GAMEPLAY; ADVANCED CAMERA selects Independent
  or Original. VIEW changes live through `apply_live`/`input_set_view`.
- ADVANCED MOVEMENT SWIMMING is Modern/Original pace, live. JETPACK is
  Modern/Classic/EDuke32 physics/control/fuel scheme, next launch. Neither is a
  first-person/legacy-third-person choice.
- Menu `write_settings` at 651 writes committed rows to the atomic
  `DNTTK_MENU_SETTINGS_FILE` side file. Launcher/profile absorption validates
  and persists it. Runtime `camera_persist` separately records view/shoulder/
  distance; `absorb_camera_state` merges it before/after launch. Global view
  and existing movement settings are persistent. Nonexistent per-action view
  settings cannot be described as persistent.
- `DNTTK_FP_PROJECTION`, weapon pose/placement overrides are developer/display
  tuning, not persistent action perspective settings.
- `player_profiles.py:469..470` still says swimming and jetpack use third
  person. Correct in D35A without treating it as a camera implementation gap.

### Proposed contract (design only)

Add a single presentation-only action policy resolver after world authentication
and before `first_person_update`. It returns an action category, capability,
requested/effective presentation and reason. Actions supply eye anchors and
presentation constraints, not independent save/restore copies of global VIEW.
Keep movement/aim/fire permission entirely separate.

Proposed precedence:

1. Invalid identity/world/owner: safe original ownership; never force FP.
2. Authenticated gameplay and explicit global third person: stay third person,
   irrespective of any action's First-Person setting.
3. Global first person + supported action: apply its First-Person or Legacy
   Third-Person choice. Nested actions use the currently owning state (e.g.
   swimming -> mantle); transitions must not oscillate on stale previous-mode
   bytes. Transfer states use authenticated action context, not animation alone.
4. Exit/abort/landing: resolve against the **current** global preference. A P
   press during the action must win; do not restore a stale saved boolean.
5. Pause/focus/menu suspension freezes actions but retains presentation;
   load/restart clears only transient action ownership and reacquires safely.

Use flat enum fields such as `swim_view`, `jetpack_view`, `push_view`,
`ladder_view`, `pole_view`, `mantle_view`, `overhead_view`, `wall_climb_view`,
`door_view`, `mirror_view` in the existing controls section; values `first` and
`legacy-third`. Names are proposed, not implemented flags. Add an ADVANCED
ACTION PERSPECTIVE heading, readable labels and help explaining that they apply
only when global First Person and Modern/Independent camera are selected.
Validate/migrate schema once for infrastructure, extend it as action fields ship;
share enum validation and row construction without replacing the profile system.
Apply confirmed menu changes live at a safe presentation update; preserve draft
cancel/CONFIRM and atomic persistence. Test older/malformed profiles, defaults,
side-file merge ordering and restart. Do not let a stale camera side file erase
new action settings. Snapshot resolved state for replay workers.

New supported actions should default to First-Person once accepted. Existing
swim/jet defaults remain FP. Unsupported actions retain today's fallback until
implemented; do not expose a false promise by adding inert First-Person rows.
For legacy action view, preserve the accepted Modernized third-person orbit
(including centered climbs and D10B collision), not silently revert all controls
to PS1 or remove free look. If exact original fixed cinematic framing is needed,
research it separately in D35K. Original global camera and Vanilla stay intact.

## 5. Implementation requirements and risks

Every action needs a real eye trajectory, not just model hiding or a deleted
fallback condition. Keep native root motion, collision sweeps, attachment and
animation events authoritative. Use stable body/interaction references and
bounded posture easing; do not attach the camera blindly to a swinging neck.
Maintain eye clearance near object faces, ladder panels, ceilings and water
planes. Do not move Duke or bypass collision to fix the camera.

Climbs need entry/steady/exit coverage and clear look-versus-body orientation.
Pole A/D currently changes meaning from the camera's front/back side via
`pole_view_from_front`; first-person classification must not reverse held input.
Mantles change root height and orientation, can fail, drop, shimmy or transfer
from swimming. Overhead input already rotates Duke toward camera-relative WASD;
a first-person eye must not create a self-reinforcing turn or rhythmic roll.
Push/pull locks direction, stows weapons, suppresses fire and has bounded lanes;
retain those gameplay contracts. Door/mirror callbacks can align or lock the
actor; preserve interaction timing, item use, one-shot events, sound and weapon
redraw while giving a comfortable eye path. Interaction hands may require new
presentation; raw weapon-ready arms are not appropriate for every action.

Camera, projection, head/body visibility, hand/weapon draw and reflection must
agree in both canonical and replay frames. Do not hide the reflected head or
leave stale joint flags in saves. Near-clip fixes must not make doors transparent
or reveal geometry through a ceiling. Retain D10B third-person compression and
body fade. Test missing model data and denied ownership as safe fallbacks.

Damage/death/script cases are not permission to enable normal input. Research
which actions are playable, which are committed, and which are authored
noninteractive sequences. D21 owns broader comfort/accessibility controls;
reuse VIEW BOB and propose extra controls only for a demonstrated need.

## 6. Backlog and order

The canonical board contains full scopes for D35A-D35N. All children are Todo;
this audit starts none of them. D35 remains Needs review until user acceptance.

| Order | Job | Boundary |
| --- | --- | --- |
| 1 | D35A | Shared action-perspective policy/configuration foundation, preserve current behavior |
| 2 | D35B, D35C | Independent swim and jetpack perspective choices; existing FP implementation reused |
| 3 | D35M, D35N research stages | Trace user-reported door and mirror transitions early; classify callbacks before estimating implementation |
| 4 | D35D | Push/pull, bounded known state set |
| 5 | D35E, then D35F | Ladder lifecycle, then chain/pole orbit/input and transfers |
| 6 | D35G | Mantle/hang/shimmy and water climb-out; complex eye path |
| 7 | D35H, D35I | Overhead and climbing-wall refinements, using verified transfer infrastructure |
| As evidence permits | D35M, D35N implementation stages | May move ahead of climbs if research shows simple, safe interaction-only presentation |
| Research alongside selected work | D35J, D35K, D35L | Damage/inventory gaps, other special views, death/recovery; do not widen guards speculatively |
| Continuous final evidence | D22, with D21/D25 | Campaign coverage, comfort, then release acceptance; no duplicate giant FP job |

D11E/D11F, D12, D11B, D26B, D10B and accepted traversal-control jobs stay
closed. D11G remains Cancelled/cannot reproduce. D34B remains classic two-pad
split-screen, not a multiplayer-first-person commitment. D35K must not duplicate
door/mirror work; it records their findings and owns other native camera events.

## 7. Acceptance and reproducible validation plan

For each implementation job, use private settings, cards and save copies.
Historical slot numbers are references, not stable fixtures: record level,
position, object type, overlay identity, animation and state when copying them.
Do not overwrite player saves. No diagnostic launch is part of this research.

Capture `ttk_input`/camera diagnostics before, during and after the action:
selected view, effective blend/reason, normal/alternate owner, pivot pointers,
actor flags, lower/upper animation plus track, current/previous modes, object
pointer/type/callback, look angle, world position and frame sequence. Include
canonical and presented images. Current `fp_fallbacks` counts hard releases,
not every smooth traversal fallback or anchor failure; blend/reason/owner are
necessary to detect the latter. New diagnostics must be bounded and cheap.

Each action tests First-Person and Legacy Third-Person with global first person;
global third person with both action settings; P changes mid-action; pause,
console/focus loss, save/load, abort/death and reacquisition; default/reset,
menu CONFIRM/cancel and relaunch persistence; Independent/Original camera,
Modern/Legacy controls and Vanilla. Include 60 FPS and replay 120+ FPS, 4:3 and
widescreen, supported software/OpenGL paths, multiple costumes/levels, armed/
holstered and relevant weapon draw/fire restrictions. Confirm that perspective
choice does not alter root trajectory, collision, health, event order or items.

Complete first-person gameplay acceptance requires:

- Every controllable action in this matrix has a verified eye presentation and
  coherent entry/exit, or an explicit reviewed exception with a focused job.
- No unexplained FP-to-third transition on a recorded beginning-to-end campaign
  route including bonus/challenge/boss content, natural death/Continue, save/
  load and level travel. D22 owns route coverage, not this static audit.
- Door, mirror, key/lock and special-script callbacks are mapped from actual
  examples; every new mechanism is added here with reproducible evidence.
- Legacy per-action transitions return to the current selected view; global
  third person and Vanilla retain gameplay and presentation behavior.
- Comfort, interaction visibility, clipping, hands/weapons, reflections and
  responsiveness are accepted by the user, not inferred from a linked binary.
- No hidden identity/owner bypass or input permission widening is used to
  maintain the eye. Noninteractive FMVs/frontend/cinematics are documented
  exemptions, not silently counted as completed first-person gameplay.

## 8. Research result and remaining uncertainty

Confirmed explicit action gaps are push/pull and attached traversal; confirmed
policy/safety exits include native special-camera ownership, death/inhibit/state
rejection, world reset and invalid anchors. Swimming and jetpack already work in
first person and lack only the requested independent perspective choices.
Doors and mirror interactions are user-confirmed missing cases, with precise
native causes still unresolved. Damage/lock/script variants need event traces;
they are not all claimed broken. The host boundary audit is complete for the
searched production sources; exhaustive campaign event coverage remains D22 and
the focused research stages. This distinction is essential to review the roadmap
without mistaking static evidence for a full gameplay certification.
