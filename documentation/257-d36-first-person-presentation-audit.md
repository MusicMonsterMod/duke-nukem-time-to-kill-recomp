# D36 - First-person body, arms and interaction presentation audit

2026-10-26. Done, research accepted. User: "ok, lock this research job in, and
close it out. i think it's gonna be integral to working on the rest of the jobs".
D36 and D36A-F are accepted, with documentation commit/push closeout authorized.
This is the reference for subsequent first-person presentation work. Acceptance
preserves the evidence limits below; it does not authorize implementation of
D36G-L or certify all unobserved gameplay combinations. D35 remains closed.

## Findings and recommendation

The existing presentation systems are reusable. The main problem is inconsistent
limb ownership between three rendering paths, not generally absent arm meshes:

1. Combat weapons and held items move the right hand into a camera-relative
   display pose. Five other arm/hand parts are deliberately hidden. Adding their
   original geometry unchanged would not connect them to that moved hand.
2. Holstered interactions use the original world-space skeleton. The polygon
   clipper admits their arms, but the earlier whole-joint depth rejection is
   bypassed only for traversal actions, not Push, Use or Mirror. A hand can remain
   visible while its parent arm is rejected. Native alignment and turning still
   animate the body below a stabilized eye.
3. Quick kick redirects the right-leg joints into the weapon layer. It also turns
   the thigh end-for-end and lowers that thigh separately to bypass the native
   joint cutoff. Ordinary body parts retain a different clipping/depth policy.

Recommend a small presentation contract around the existing draw hooks, with
connected combat arms and world-space interaction arms as distinct pose owners.
Keep native gameplay and animation events authoritative. First investigate and
correct slot-8 interaction continuity, then implement combat arms and kick/body
refinement as separately reviewable changes. Do not rewrite the renderer, force
third person, hide every limb, or constrain mouse pitch as a blanket solution.

## Evidence, baseline and limits

Evidence labels used below:

- **S**: current source or read-only disc structure verified during this audit.
- **L**: current private-run state/capture evidence, with the route identified.
- **A**: previously accepted behavior and its documented bounded verification.
- **U/H**: user report or hypothesis not established by this audit.

Baseline player executable:
`736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
Framework pin `08ec704a974b1f3a16335b4afeb340b9eff19926`; UI pin
`be8ac1d03ee19d55394b5a5f2d9d1506edd56659`. Existing framework modifications
were present at intake. No gameplay, framework, generated C, assets, animation
packs or build definitions are changed. No build, commit or push is part of this
research. The player build remains the D37A/D19Q3 candidate, not newly accepted.

Local evidence: `recomp/analysis/d36-audit/` contains intake hashes/mtimes,
private settings and save copies, scripts, JSON traces, source-geometry census
and PNG captures. Runtime uses offscreen OpenGL, dummy audio, separate writable
cards, loopback port 9486 and Modernized first person at a 60 presentation setting.
These are visual/state diagnostics, not sound checks or a performance benchmark.
Captures and retail-derived geometry remain local. The source geometry census
reads DB00-DB09 from the owned disc without modifying it.

Samples are sequential RAM/debug reads, not atomic frame snapshots. Sparse stills
can establish visible discontinuity but cannot certify all animation frames.
A requested weapon key is not proof of selection: the matrix below uses actual
held record IDs. Early failed grab binding and post-death weapon samples were
excluded; raw evidence remains for reproducibility. Slots are current reproduction
pointers, not permanent level identities. The user's phrase "new Time to Kill
level" is not assumed to identify a particular level. Weapon probes here use the
verified current slot-8 LEVEL05 scene, not an invented identification of that phrase.

The audit is comprehensive in identified source paths and proposed coverage;
it is not a claim of exhaustive campaign visual observation. Unreproduced events,
costumes and transition combinations remain explicit below.

## D36A - System ownership catalogue

Source links are relative to this report. Function names identify the relevant
contracts even when later work moves line numbers.

| Layer | Current source and responsibility | Entry, active interval and restoration |
| --- | --- | --- |
| Preference and input | [pc_input.cpp](../recomp/src/ttk/pc_input.cpp): `camera_action`, `input_set_view`; [player_profiles.py](../recomp/tools/local/player_profiles.py), [game_menu.cpp](../recomp/src/ttk/game_menu.cpp) | Global first/third selection is separate from effective view. Pause/focus loss suspends actions; it must not silently replace the selected perspective. Action preferences default to first and global Third Person wins. |
| Action policy | [action_perspective.h](../recomp/src/ttk/action_perspective.h): `resolve_perspective`; [first_person.inc](../recomp/src/ttk/first_person.inc): `first_person_update` | Priority: Mirror, Use, WaterExit, Push, Wall, Ladder, Pole, Overhead, Mantle, committed movement, fall, other traversal, swim, jet, ground. Only the resolved owner consumes its preference. This policy does not authorize movement or weapon fire. |
| Authentication and camera | [modern_controls.cpp](../recomp/src/ttk/modern_controls.cpp): `state`, `camera_eligible`, `orbit_begin`, normal-camera hook `0x8003ade4` | Validate executable/active overlay, gameplay, reciprocal player/camera ownership and supported state. Player `0x800d7198`, camera `0x800d6eb0`; owner +0xa4, back-pointer +0x7d4, pivots +0x4c/+0x50. On refusal `first_person_release` restores head/boom and invalidates eye state. |
| Eye and projection | `first_person_anchor`, `first_person_constraint`, `first_person_follow`, `first_person_projection` | Ground uses camera pivot plus smoothed neck offset and deliberate bob. Actions use stable root-relative anchors, commonly 294 units above root. Native obstruction queries remain. Projection defaults H=256; temporary minimum boom is restored to 768. Acquisition starts directly in selected view, not a blend from an unrelated save. |
| Skeleton lookup | [duke_model.inc](../recomp/src/ttk/duke_model.inc): `duke_model`, `duke_part_joint`, `duke_joint_part` | 19 joints; descriptor +0x24 part-to-joint map; records at +0x44, stride 0x28. Part and reverse map must agree. Costume-specific record ordering is not a semantic part ID. Matrix array is player+0x3c; model descriptor player+0x40. |
| Head/body visibility | `first_person_draw`, `head_restore`; `near_clip.cpp:object` | Head part 0x10 is hidden only during the primary Duke draw. Saved stale bit-0 hides are reclaimed across all joints. Ordinary body geometry stays on the native path; allowing interaction arms into the host clipper is not an instruction to render the whole body. |
| Combat and held-item display | `first_person_weapon_joint`, `display_pose`, `first_person_weapon_attached` | Intercept transform `0x800292a0`, return `0x80034c6c`. Move right hand 0x0f to a private matrix; hide parts 0x0a-0x0e at the eye. Equipment 2 draws weapon, equipment 1 draws item. Attach weapon, flash and item to the same display hand. Original gameplay matrices remain authoritative. |
| Combat pose variants | `weapon_view_slot`, `animated_weapon_channel`, `crouch_weapon_active`, `swim_weapon_view_active` | Seeded firing poses for gun records 4-11; animated throw/blade/crossbow channels. Crouch/swim rebuild the right-arm channel to avoid double-applying shoulder compensation. Separate upgrade records 27/28/29 map to display slots 9/7/8. |
| Interaction arms | `first_person_weapon_joint`, `first_person_ladder_near`; `near_clip.cpp:object` | Tag native arm parts 0x0a-0x0f for supported actions. No camera-relative weapon relocation when holstered. Traversal alone raises the saved loop depth to H before polygon clipping. The guard requires Duke, normal camera and the verified actor-loop stack/caller. |
| Mesh clipping and depth | [near_clip.cpp](../recomp/src/ttk/near_clip.cpp): `object`, `emit`, `near_clip_viewmodel_flush` | Clip unsafe polygons at near_z=16. Weapon/item/kick packets use viewmodel ordering-table slot 1 and precise negative-depth metadata; HUD slot 0 remains later. Native interaction arms retain world depth and occlusion. Ordinary body joints are excluded from this takeover. |
| Lighting and fade | `fade_clear`, `fade_restore`, `third_person_mesh_begin` | Correct only native proximity fade, preserving intentional translucency/power-ups. Restore render-context command, tpage, flat colour and GTE back colours at the next joint/boundary. Reflected Duke uses separate draw scope; it must not inherit primary-body hiding. |
| Kick | [kick.inc](../recomp/src/ttk/kick.inc): `kick_request`, `kick_update`, `kick_leg_joint`, `leg_pose` | Q animation is host presentation over native damage queries. Right parts 5-8 are substituted during their existing draw, not a second complete skeleton submission. 27 host frames; hit interval 8-20. Cancellation checks eye readiness. Door 116 uses `door_kick_frame` and native channel timing. |
| Bio Mask | `weapon_draw_bounds`, `near_clip_biomask`; [note 247](247-d08a26-bio-mask-mesh.md) | Draw after weapon, before HUD, only with live settled first person, active charged mask and mask face model. Native per-hand environment RGB if available; brightness fallback when holstered. Recompute each draw, including replay. |
| Animation overlays | [anim_override.cpp](../recomp/src/ttk/anim_override.cpp), [ttk_anim.py](../recomp/tools/local/ttk_anim.py), crouch/swim pose helpers | Original animation channels remain separate from presentation. Override packs authenticate source clip digests. D37A recovers saved allocation metadata and rebuilds against native tables; never preserve stale packet memory to retain a pose. |
| Save/world boundaries | `camera_world_reset`, hook load-generation check, scene teardown hook | Forget old pointer ownership before reusing guest addresses; do not restore old draw bytes into newly loaded RAM. Clear orbit, policy, eye/bob history and fade ownership. Draw-time stale head flags are reclaimed separately. |
| High-refresh replay | `RenderState`, `render_state_prepare/save/load/interpolate`; [frame_replay.cpp](../recomp/src/ttk/frame_replay.cpp): late view path | Snapshot perspective, weapon pose seeds, crouch/swim eligibility, eye, matrices and kick frame. Reserve scratch before worker forks. Workers recompute joint draw scope; they cannot infer current eligibility from stale simulation statics. |

Whole-tree search covered title sources, local launcher/profile tools, framework
runtime and UI for first-person, blend, viewmodel and developer-override names;
results are retained in `search-files.txt`. Adjacent traversal, aim, crouch,
animation and lifecycle code was inspected by ownership, not only string matches.
Generated guest routines are read-only evidence. Indirect script behavior and
unvisited level callbacks cannot be proven exhaustive by text search.

### Activity and transition catalogue

Animation numbers below are lower-body unless explicitly marked upper. They are
current classifier/source IDs, not claims of having newly played every clip.
Shared IDs require mode/history and object ownership; never whitelist a range alone.
All rows preserve global/action Third Person and Vanilla choices.

| Activity | Verified identifiers and owning path | Entry / active / exit and visual disposition | Evidence |
| --- | --- | --- | --- |
| Standing, walking, running, strafe | Idle 63; gait handlers in `state`, `grounded_water.inc` | Ground eye/bob; weapon hand display; normal legs use native joint culling. Stop returns to idle and damps bob. | S/A; current ground/crouch probes |
| Idle gestures | 64-69, mode/history 0/0; `idle_fidget_eye` | Stable upright root eye while native fidget plays; return to ground. Not all six gestures newly observed. | S/A [233](233-d35j-transient-perspective-research.md), [234](234-d35j-use-and-idle-first-person.md) |
| Crouch and crouch walk | Stationary 178; `crouch.inc`, `crouch_walk.inc`, `crouch_hull.inc` | Low eye/hull and upper-channel compensation; safe stand-up; viewmodel reconstruction bypasses that compensation once. Preserve custom gait and events. | S/A [187](187-d08k4-crouch-walking.md), [192](192-d08k10-crouched-viewmodel.md); L crouch |
| Jump, landing, fall and wall bumps | 96/97 preparation, 98/103/104/105/109 and owned 108; `state`, `manual_jump.inc`, `landing.inc` | Camera-only and movement leases differ. Preserve takeoff ownership, epochs and continuation checks. Fall/land restore ground presentation. | S/A [218](218-d08z6-blocked-run-jumps.md) |
| Push/pull/grab | 119/120/121, 0/0, holstered, valid push object +0x290; `push_state_early` | Native alignment/grab, sustained push/pull, release/weapon return. Eye anchored at root -294; arms remain native, world-depth. | S/A [226](226-d35d-first-person-push-pull.md); U/L current slot 8 |
| Plain ladder | 147/148/156, 185-190, 207/210/211 with plain-ladder owner | Mount, traverse, exits 185/190, detached 108 at 9/3 retain owner history. Root eye, native hands, pre-polygon near-cull bridge. | S/A [227](227-d35e-first-person-ladders.md); L slot 12 pose 186 |
| Chains/poles | 147/148/154, 191-196 with pole-family owner | Mount/climb/turn, exit/drop and return. `pole_visibility_draw` also submits an attached pole whose room is absent from portal list. Do not regress it. | S/A [228](228-d35f-first-person-poles.md), [231](231-d35f1-chain-visibility.md) |
| Climbable walls | 147/148/155, 197-206; wall object flags 0x100 in 0x700 mask | Native wall body motion and accepted diagonal input; root eye, arm bridge; exclusive exits/drop retain policy. | S/A [232](232-d35i-wall-climbing.md) |
| Overhead traversal | 147/148 at mode 6, 149-151 at mode 7; climbable flags 0x80 | Shared hang poses resolved by attachment, not names; release 108 at 9/7; independent overhead preference. | S/A [230](230-d35h-overhead-perspective.md) |
| Mantle, ledge hang, shimmy | 134-142, 147-153 with allowed mode/history; `first_person_mantle` | Root travel follows ledge; eye avoids animated neck lean. Armed step-ups keep hidden viewmodel limbs excluded from interaction bridge. | S/A [229](229-d35g-first-person-mantles.md), [242](242-d35g1-jump-edge-artifact.md) |
| Water climb-out | 134-142, mode 8 after 4/5 | WaterExit wins before lingering swim flags; root anchor rebases, then ground. | S/A [229](229-d35g-first-person-mantles.md) |
| Surface/submerged swimming | Water predicates, modes 4/5; native dive 133 and stroke states | Water-relative eye; swim firing shoulder correction restored after model build; display hand reconstructs animated arm channel once. Preserve limited underwater weapon selection rules. | S/A [216](216-d11e-first-person-swimming.md), [224](224-d35b-swimming-perspective.md), `swim.inc` |
| Shallow water | `shallow_land_water_early`, grounded-water/crouch helpers | Land eye/crouch path; transitions restore scoped native water vetoes. Do not confuse with free swim. | S/A [193](193-d08k11-shallow-water-crouch.md), [194](194-d08ac-shallow-water-locomotion.md) |
| Jetpack | Mode 10, `jetpack_anim` 163-170 | Root-relative flight eye; body/head/weapon presentation; off enters fall then landing. Keep scheme and perspective independent. | S/A [220](220-d11f-first-person-jetpack.md), [225](225-d35c-jetpack-perspective.md) |
| Keycard and held mission items | Upper 47/48, equipment 1, then full-body 266 | Held item first uses moved right hand; committed use then native arms when hand empties; native callback and redraw own completion. | S/A [234](234-d35j-use-and-idle-first-person.md); L slot 11 |
| Crystal take/return | Upper 47, held item 11 in current route; 267/268 | Item-display phase differs from full-body use; preserve pedestal callback and inventory accounting. | S/A [236](236-d35j4-time-crystal-perspective.md); L slot 9 insertion 267 |
| Vending/NPC use | 277, committed bit 0x40, mode/history 0/0 | Holster, native gesture, callback, weapon return; root eye and native interaction arms. | S/A [234](234-d35j-use-and-idle-first-person.md); L slot 10 vending |
| Wheel | 270, class 7, type 206 in accepted route | Native alignment and cyclic hand path; stable eye; retain turning event and exit. | S/A [241](241-d35k4-wheel-first-person.md); L slot 5 |
| Lever | 118, selector class 10 | Native timing/effect and stow/return; Use perspective only. | S/A [234](234-d35j-use-and-idle-first-person.md) |
| Hinged door kick | 116, selector class 5 | Native action/callback remains; display leg uses native key progression through `door_kick_frame`; no second hit calculation. | S/A [237](237-d35k-slot5-door.md) |
| Toilet | 275, selector class 8 | Preserve one-time effect and committed animation; Use policy then weapon recovery. | S/A [239](239-d35k2-toilet-first-person.md) |
| Statue | 279, class 13, accepted LEVEL09 type 926 | Native alignment/animation/callback; Use policy, then original restoration. Other class-13 types not newly certified. | S/A [240](240-d35k3-statue-first-person.md) |
| Mirror use and reflection | Use pose 278, own `mirror_view`; reflected actor caller 0x8003816c | Interaction eye and full reflected body are separate. Clear only unwanted reflected proximity fade, restore render context at boundaries. Missing voice D35N1 remains separate. | S/A [235](235-d35n-mirror-interaction-and-opacity.md) |
| Held original inventory | Bit 0x200, native upper-body state | Presentation-specific exception preserves eye without authorizing actions; release restores normal controls. | S/A [234](234-d35j-use-and-idle-first-person.md) |
| Gadgets, night vision, Bio Mask | Native equipment/face bits; mask face model 2 | Instant/automatic gadget behavior is accepted. Mask geometry samples current light, no independent animation clock. Keep goggles/mask replacement and depleted/death gates. | S/A [244](244-d08a23-instant-night-vision.md), [246](246-d08a24-auto-bio-mask.md), [247](247-d08a26-bio-mask-mesh.md) |
| Damage/death/respawn | Death flags and state rejection; native death poses | Third-person death/respawn is explicitly accepted, not a presentation defect. Recover selected view only when a valid new world/player returns. No blanket damage-pose admission. | S/A [222](222-d35-first-person-audit.md); limited accidental death in discarded first sweep |
| Scripted/foreign cameras | +0x224 bit 0x20000000; entry 0x8003b36c / exit 0x8003b474 | Preserve native alternate-camera ownership, intentional cinematics and guard failures. Direct callers known in LEVEL00/05/09 do not prove all indirect events. | S/A [238](238-d35k-interaction-inventory.md) |
| Pause, load, restart, level transition | `camera_world_reset`, load generation, `RenderState` | Keep selected preference; retire old pointers/history; reconstruct from loaded state. D19Q3 resumes gameplay, separate from visual quality. | S/A [255](255-d37a-animation-save-restore.md), [256](256-d19q3-always-resume.md) |

Unmapped selector cases remain research gaps: class 9 pose 269, class 12 pose
176's exact interaction, underwater class-6 pose 131, other class-7/class-13
objects, class-14/16/17/18 callback variants, indirect alternate-camera events,
level openings/boss/endings and two-player ownership. See note 238. Existing
support for a numeric pose does not establish all its object-specific outcomes.

## D36B - Weapon and source-geometry matrix

Read-only DB00-DB09 hierarchy-1 census confirms 19 joints in every group.
All have arm parts 10-15 with nonempty meshes. Forearms have 15 vertices and
hands 8 vertices; right upper arms have 23 vertices, left upper arms 23 or 30.
DB00 example: left models 98/99/102; right 106/107/110. DB01 example:
left 150/151/154; right 158/159/162. These are group-local indices, not globally
interchangeable asset IDs. Part 21 is a separate accessory under the right-leg
branch (model 212 in the census); it is not included in kick parts 5-8.

Thus the current source can supply arm segments, but reuse still needs connected
transforms and pose-specific joints. Original segment availability does not prove
seam quality at first-person scale, correct UVs under an altered pose, or matching
shoulder placement. No replacement asset has been created.

| Key / record | Weapon / variant | Current display mechanism and audit disposition |
| --- | --- | --- |
| 1 / 0 | Mighty Boot | No combat hand weapon; Q uses the independent kick. Body/kick overlap belongs to D36C. |
| 1 / 1 | Throwing Knife | Animated channel; right glove/blade displayed, arm chain suppressed. L idle/crouch/attack samples; connected forearm missing by design. |
| 1 / 2 | Throwing Axe | Same suppression, separate tilt. L selected axe verified; retain draw/throw motion, not a static gun pose. |
| 2 / 4 | Desert Eagle | Seeded firing grip; offset intentionally keeps cut wrist near/below screen edge. Base and upgrade share display record. |
| 3 / 5 | Combat Shotgun | Seeded grip, accepted yaw/roll/raise. Left arm/hand suppressed even though weapon is two-handed. Preserve accepted barrel framing. |
| 4 / 7, 28 | Gatling / Laser Gatling | Seeded grip; long mesh has large slot offset. Detached right glove is structurally expected. Upgrade display maps 28 to 7. |
| 5 / 8, 29 | RPG / Incendiary RPG | Seeded grip, scale 1.2 and low twin-barrel framing. Upgrade display maps 29 to 8. Arms must not fill the central sightline or undo accepted framing. |
| 6 / 12 | Pipe Bomb / detonator | Throw channel; remote becomes equipment 1, item 0. Both need a connected hand solution. Do not confuse remote state with a firearm record. |
| 6 / 14 | Dynamite | Throw channel and automatic next-stick redraw; transitional equipment 0 is real. Observe release as well as held-fire. |
| 6 / 13 | Holy Hand Grenade | Throw channel, same five-part suppression. Distinct selection verified in corrected sweep. |
| 7 / 10 | Energy Weapon | Seeded pose, scale 1.4; upgrade retains record 10. Preserve aim/beam behavior, change display only. |
| 8 / 9, 27 | Flamethrower / HiTemp | Seeded pose, same base mesh/animation framing; upgrade display maps 27 to 9. Flame effects remain native. |
| 9 / 6 | Buffalo Rifle | Seeded pose with slight raise; two-handed continuity needs shared support-arm solution. |
| 9 / 3 | Crossbow | Animated right-arm channel, not seeded gun path. Selection must wait for native switching to settle; early repeated key samples remained record 6 and were excluded. |
| 0 / 11 | Freezer | Seeded grip, scale 1.4; preserve accepted barrel/tip framing. |

Weapon names and slot mappings are authenticated in [note 33](33-controls-shortcuts.md)
and [shortcuts.inc](../recomp/src/ttk/shortcuts.inc). Six campaign upgrades apply
to pistol, shotgun, Gatling, RPG, energy and flame; only flame/Gatling/RPG have
separate resolved records. [Note 199](199-d12d-upgraded-viewmodels.md) previously
verified direct challenge records 27-29 and unchanged framing. This audit does
not relabel campaign base record IDs as failed upgrades.

Corrected current sweep reloads before each weapon, grants inventory with the
existing cheat and records actual equipment. For successfully selected records in that pass it covers idle, crouch, downward
look, quick-kick overlap and attack/release samples in LEVEL05; Crossbow was
verified separately at idle only. It does not certify
every weapon's full throw/reload cycle, upward extreme, swimming, jetpack,
damage interruption, every costume or every upgraded state combination. These
are explicit test cells in the implementation contract below, not inferred passes.

Recommended combat solution: preserve the accepted right-hand/weapon transform
as the grip target; solve or author a connected right shoulder/elbow/wrist chain
in the same display space. Add the support hand only where the weapon requires
it, with per-weapon grip anchors. Share original mesh families and animation
channels first. Use a small original replacement forearm/hand only where existing
segment seams or posing cannot meet the standard. Simply unhiding native arms
would leave them attached to the old world-space hand position.

## D36C - Body and kick diagnosis

| Observation | Evidence and cause status | Recommended direction / alternatives |
| --- | --- | --- |
| Legs appear/disappear while looking down | S: normal body joints retain native origin and polygon rejection; head alone is explicitly hidden. Eye stabilization separates camera motion from animated neck/root motion. A joint crossing H can disappear as a unit. Attribution of every reported gait pop requires synchronized pose/depth captures. | Define an explicit body visibility policy and let admitted leg geometry reach safe clipping. Keep intentional torso anti-obstruction if it reads well. Compare selective upper-body exclusion and camera-relative lower body before considering a restricted downward look. |
| Foot versus long gun depth | S: kick and gun share the viewmodel layer but use different anchors. Gun default (110,150,520), kick (-170,80,160); Gatling adds (110,80,290). Mesh extent, tilt and pose determine actual overlap. A closer foot is not intrinsically wrong. | Judge anatomical reach with short/long weapon captures; use one depth convention and grip/hip anchors. Do not force the boot behind every barrel or change damage reach to move its picture. |
| Apparent third leg | S: `kick_leg_joint` substitutes parts 5-8 in the original joint loop, so code does not simply draw another complete right leg over an unchanged right leg. Left leg, pelvis and accessory part 21 remain on their native path. Mixed origins, the inverted thigh and remaining attachment are concrete leads, not proof of three anatomical legs. | Explicit right-leg/accessory presentation ownership. First identify the visible pieces per frame; suppress or transform the corresponding attachment only while owned. Restore on cancel/load/view switch. Do not delete the entire body to conceal it. |
| Mirrored/incorrect holster | S: thigh is rotated 180 degrees end-for-end around an axis perpendicular to its length, repositioned at knee, then dropped by 13% of screen height. This is a proper rotation, not proof of a reflection/negative determinant. Part 21 is not redirected with the leg. U: user's precise holster interpretation. | Remove the geometric workaround only after proper near clipping is available. Reuse the correct right leg with anatomically continuous transforms; compare an original jeans/boot mesh without holster if the retail shape cannot frame cleanly. |

Preserve `kick_update` timing, `kick_impact`, radius 96, original damage formula,
world/contact tests, sound timing and Q responsiveness. Door kick 116 is a separate
native-timed consumer of the display leg and must be checked independently.
A presentation change must not call damage twice or move the gameplay hit point.

## D36D - Interaction evidence and coverage contract

| Current slot / activity | Observed identity and path | Diagnosis and remaining work |
| --- | --- | --- |
| 11 keycard | LEVEL00, upper 47 with equipment 1/item 6, then lower 266, weapon returns | L floating card/hand phase; S held-item path suppresses five arm parts. After equipment empties, native interaction path applies. Audit both phases rather than applying a single arm rule throughout. |
| 10 vending machine | LEVEL00, lower/upper 277, holstered, then pistol returns | L cut-off arm in action captures. S Use lacks the traversal pre-polygon bridge. Exact framewise contribution of origin culling versus native arm pose requires depth trace. |
| 9 crystal | LEVEL00, upper 47 with item 11, then 267 and weapon return | L crystal/glove without continuous arm; S same isolated held-item path as keycard. Preserve crystal callback and use/return direction. Current repeat newly covers insertion, not every 268 removal phase. |
| 8 dumpster | LEVEL05 current checkpoint; final detailed grab route recorded locally | Prior accepted native push system stays authoritative. Test alignment, 121 grab, 119/120 motion, reversal, neutral hold and release with actual attached object. See final run results below; failed initial binding trials are not push evidence. |
| 5 wheel | LEVEL00, sustained lower/upper 270, holstered | L separate hands/forearms intermittently visible around wheel. S native world-space arm chain and joint cutoff, not weapon-hand relocation. Captures alone do not justify replacing wheel timing. |
| 12 ladder | LEVEL00, lower/upper 186, mode 3/3, holstered | L hands/legs at downward look; S traversal bridge already present. User reports arms generally work. Preserve this baseline; torso invisibility alone is not classified as a defect. |

For each interaction, test entry, sustained use, exit, attempted interruption,
weapon-switch request, both look extremes and save/load where the game permits
it. A blocked switch during a committed action is expected, not failure. Current
key/card/crystal/vending/wheel captures establish selected intervals and recovery;
they do not fill every combination in that contract. Natural damage interruption,
mid-use save/load and 120/180/240 rendering remain unverified in this research run.
Previously accepted functional tests in notes 226-241 remain evidence, not newly
rerun results or blanket visual acceptance.

Native arms should remain world-occluded for contacts with dumpsters, walls and
ladders. Moving all interactions into the weapon overlay would let hands paint
through the object and obscure the contact problem. For slot 8, first separate
origin rejection from actual native turning/repositioning, then compare native
pose plus clipping against a constrained contact pose. A dedicated push/pull
pose is viable if it preserves the original movement and callback phase; it is
not yet justified as the only solution.

## Current-run results and defect traceability

The four retained passes contain 382 samples, including deliberately retained
failed harness trials. That total is not a count of valid passing test cases.
Corrected evidence is in `followup.json`, `detail.json` and `push-final.json`.

- All 14 base weapon records 1-14 were selected and identified, plus Boot.
  Crossbow was finally confirmed as equipment 2/record 3 after allowing the
  native switch to settle (`crossbow-cycle-1`). Its final check is idle only;
  earlier record-6 captures cannot stand in for crossbow firing or crouching.
- All six campaign upgrade flags were read as 9 (owned bit 1 plus upgrade bit 8)
  for records 4, 5, 7, 8, 9 and 10. Six-upgrade captures retain actual held base
  IDs, as expected from prior D12D evidence. Direct challenge IDs 27-29 were
  not newly loaded in this audit; their prior acceptance is cited separately.
- Keycard upper 47/equipment 1/item 6 proceeds to lower 266 and weapon return;
  crystal upper 47/item 11 proceeds to lower 267; vending reaches 277; wheel
  reaches 270; ladder starts in 186. These establish current slot identities.
- Final slot-8 route reaches 121 hold, 120 push, 119 pull, then 121 release and
  63 with pistol returned. During forward movement root Z advances from -17073
  to -16143, then reverses to -16983. Captures show detached wrist/forearm pieces,
  disappearance during pull, and return during hold. Mouse yaw changes which
  pieces remain visible; the object and native movement remain functional.
- Approximate camera-space origin depths support the source diagnosis. At
  `push-final-0`, left upper arm/forearm/hand are 174/260/372 and right are
  126/216/325, with H=256. At `push-final-4` they are 230/340/450 and
  169/278/383. At `push-final-8` all six origins are below H. The source bridge
  excludes Push, so this predicts the observed whole-part disappearance.
  These are calculations from sequential native matrix/eye reads, not captured
  GTE registers at the exact draw instruction. They establish a strong matching
  mechanism, not an atomic proof of every pixel or of all native pose awkwardness.
- Downward walk samples at pitch about 0.917 radians move the right-hip origin
  across H: approximately 264, 276, 263, 238, 225, 258, 241, 221. Captures show
  the leg entering/leaving view. Run input also reaches wall-bump 95 beside the
  dumpster, so that sequence is not a pure unobstructed running test. Crouch
  samples reach 178. Walking backward away from the object provides a separate
  unobstructed short body-view sample, not a full locomotion acceptance route.
- Detailed Boot and Gatling Q sequences show active kick frames and return;
  a later single Q with pistol returns to inactive. The boot can visibly overlap
  the Gatling. No discrete third anatomical leg was confidently identified in
  these captures. The thigh reorientation is source-confirmed; a literally
  mirrored mesh and the precise user-observed holster arrangement are not.
- SHA-256 and mtime checks after all runs match the original player saves/cards,
  settings and executable. All diagnostic processes have stopped. No source
  implementation or binary was rebuilt. Documentation links and whitespace are
  checked separately; existing native tests were inspected, not represented as
  newly executed tests.

| Defect / concern | Evidence | Cause confidence | Proposed job | Specific acceptance route |
| --- | --- | --- | --- | --- |
| Knife/axe/Gatling/throwing-item disconnected hand | D36B census, verified weapon records and captures; `first_person_weapon_joint` | Confirmed arm suppression plus moved hand; pose seam design still needed | D36H | All base records, six upgrades, throw/reload/switch, crouch and both look extremes |
| Keycard/crystal floating item hand | Slots 11/9, upper 47 then 266/267; equipment 1 path | Confirmed isolated display-hand policy during item phase | D36H with D36I handoff | Insert/remove item, native callback, weapon return, mid-use load |
| Slot-8 detached/popping arms | `push-final-*`, 119/120/121, per-part depths | Culling mismatch source-confirmed and supported by matching depths/captures; residual pose motion not fully attributed | D36I | Hold/push/pull/reverse/yaw/release, world occlusion, both view choices |
| Vending/wheel cut limbs | Slots 10/5, poses 277/270; Use excluded from bridge | Confirmed policy difference; individual visual cut attribution remains partly inferential | D36I | Entry/active/exit, up/down, pause/load and switch refusal |
| Walking leg popping | `gait-w-*`, native-body exemption and hip depths straddling H | Source mechanism plus matching observed transition; not every gait/costume certified | D36J | Unobstructed walk/run, look sweep, stairs, crouch, jump and save/load |
| Kick depth and thigh orientation | `kick-4-*`, single-kick sequence, `kick_leg_joint` | End-for-end rotation confirmed; universal depth fix and literal third-leg cause not established | D36J | Short/long weapons, all costumes, door 116, Q timing/hit equivalence |
| Existing ladder arms / torso omission | Slot 12 pose 186 and accepted D35E | Working baseline / intentional body exclusion, not automatically a defect | D36K | Preserve arms and control, review down-look body choice explicitly |
| Cross-scene/replay restoration | Source lifecycle, D37/D37A and native test contracts | Known historical allocation hazard, not a proven cause of these visual defects | D36G/L | Old/new save allocation layouts, replay, restart, mirror and Vanilla |

## D36E - Architecture and asset options

| Option | Reuse / requirements | Trade-offs and boundary |
| --- | --- | --- |
| Extend existing hooks with explicit part ownership | Keep perspective resolver, primary-draw scope, part map, world/overlay paths, fade restoration and replay snapshot | Smallest coherent foundation. Make owner, part set, space/depth, light source and cleanup explicit. Avoid a second camera/controller system or a giant per-animation allowlist. |
| Reuse retail arm segments | Census proves geometry in all ten groups; reconstruct or solve chains relative to accepted grips | Lowest asset burden and native style. Must validate seams, elbows, left-hand grip, costume ordering and animated throws. Presence of geometry alone is insufficient. |
| Dedicated original arms | Blender-authored low-poly meshes and grip/pose sets, original textures or disc-derived runtime sources | Better seam control, higher authoring/test cost and potential costume mismatch. Do not distribute extracted retail textures or geometry. Keep hands at existing weapon grips. |
| Native interaction animations plus proper clipping | Keep native matrices/events and extend guarded arm admission where justified | Best contact fidelity and least gameplay risk. Cannot repair a genuinely unsuitable body turn or wrist pose; inspect before adding IK. |
| Interaction-specific display poses / retargeting | Map native phase to display pose, preserve callback/event timing | Useful for push/pull and close work. Requires contact anchors and smooth entry/exit; camera-facing overlays alone would lose object occlusion. |
| Small original leg replacement | Dedicated jeans/boot, optional no-holster design, same display timing | Removes end-for-end thigh workaround; extra asset and costume review. First compare correct native leg with clipping, which may suffice. |
| Camera changes | Existing stable eye, projection and obstruction solve | Last resort for residual framing. Pitch restrictions reduce player control; require explicit visual evidence and user choice. Do not change global FOV to hide a local seam. |

The demonstrated [Bio Mask pipeline](../recomp/assets/models/biomask/README.md)
contains editable Blender geometry, exporter, Photon atlas project, provenance
and original source textures. Blender supports mesh/UV/rig authoring; Photon
prepares textures/atlas layers, not a substitute for a connected skeleton or
animation pose. Existing `ttk_anim.py`/`ttk_anim_blender.py` decode HIE/MOD and
ANM/FRM/ROT, export rigged references and produce authenticated animation packs.
No tool was run to modify those assets in this audit.

The mask demonstrates custom mesh packets and lighting, not a ready-made skinned
arm renderer. It emits roughly 1174 visible triangles from 2400 source triangles;
its four packet regions already share a constrained 1 MiB aperture with world
packets and a 320 KiB animation buffer. New limb meshes should start with the
existing much smaller retail segment counts. Measure packet usage, allocations,
worker reforks and frame time before increasing geometry; reserve scratch before
forking, not on the first visible action. Do not assume a mask-sized allocation
can simply be repeated for every weapon.

Lighting contract: combat arms follow the weapon's environment/shading and
intentional effects; interaction arms keep native world lighting. Preserve fade
scope and restore both RAM render context and GTE state. Custom geometry must
not remain full-bright in dark rooms or retain gas tint after load/holster.

Serialization contract: prefer draw-local matrices and phase derived from native
animation. Any new host latch must reset on world/load and be included in replay
state where needed. Any new guest allocation changes save-layout compatibility
and must pass D37A recovery. [D37](254-d37-movement-failure.md) established an
animation allocation restoration failure; it does not establish the cause of
floating arms. [D37A](255-d37a-animation-save-restore.md) is the relevant recovery
baseline. Never weaken GPU-packet clearing to preserve animation state.

## D36F - Quality standards and proposed implementation roadmap

The public [Zero Hour Overclocked project page](https://dnzh-overclocked.com/)
was reviewed on 2026-10-26. It documents first-person presentation, optional body
visibility and visual recoil independent of aim. This audit did not run Zero Hour
or review a frame-by-frame reference capture. It therefore uses the user's stated
quality target and those public feature descriptions, not an inferred implementation
or a claim that every Overclocked interaction meets these standards.

Observable acceptance standards:

- When visible, a hand connects plausibly to its forearm and elbow; no wrist cap
  floats in free view. Offscreen continuation is acceptable when intentional.
- A stationary action does not pop limbs solely because a joint origin crosses
  a rejection plane. Partial clipping follows the actual geometry and scene.
- Two legs remain anatomically interpretable during Q; right-leg accessories do
  not remain as an unexplained third limb. Holster/boot orientation is correct.
- Weapons keep accepted framing, aim, muzzle attachment and responsive switching.
  Support hands contact the weapon; throw/draw motion remains readable.
- Interaction hands plausibly contact their object and respect world occlusion.
  Native movement, collision, callbacks and event timing are unchanged.
- Part ownership changes smoothly on entry, exit, interruption, pause/load and
  first/third preference changes. Reflections keep a complete correctly lit Duke.
- Geometry, nearest-sampled textures, colour and shading retain the PS1 style.
  Lighting follows the environment, including dark rooms, gas and power-ups.
- No visual correction changes save format, hit detection, damage, timing or
  Vanilla play incidentally. Performance must be measured, not inferred from FPS settings.

The following stable IDs are **proposals in this report only**, not selected or
implemented jobs. Promote a chosen proposal to the board with its final scope
before coding. Research acceptance does not select any of them.

### D36G - Shared limb presentation ownership

1. **Problem:** draw ownership, clipping, depth and cleanup are spread across
   hand, kick and interaction branches; extending them independently risks leaks.
2. **Evidence:** D36A functions; `first_person_weapon_joint`, `kick_leg_joint`,
   `near_clip.cpp:object`, `RenderState`; slots 8/10/11 exercise different owners.
3. **Cause:** confirmed split presentation contracts, not a new gameplay failure.
4. **Solution:** small shared descriptor/helper for owner, semantic parts,
   transform space, depth domain, lighting and scoped cleanup. Preserve the
   existing resolver and draw hooks; first deliver behavior-equivalent plumbing.
5. **Alternative:** local helpers in each branch are cheaper initially but make
   ownership/restoration harder to audit; full renderer replacement is excessive.
6. **Dependencies:** D36 review; D37A/D19Q3 candidate limitations acknowledged.
7. **Risks:** actor/reflection leakage, stale replay tokens, changed allocations.
8. **Acceptance:** existing captures and native hook outputs remain equivalent;
   no changed gameplay or preference semantics, no extra first-use allocation.
9. **Tests:** existing controls/input/aim/near suites; primary/reflected/other actor
   boundaries, global and action third-person, pause/load, scene restart, replay.

### D36H - Connected combat and held-item arms

1. **Problem:** detached weapon and mission-item hands, especially knife/axe,
   Gatling and throws; source arms exist but are hidden.
2. **Evidence:** D36B census and held-record matrix, current slot-8 weapon captures,
   slot-11 item 6 and slot-9 item 11; parts 10-15, upper 47/48 and throw channels.
3. **Cause:** confirmed hand relocation plus suppression of parts 10-14.
4. **Solution:** retain current weapon grip/framing, add connected display-space
   arm chains and per-weapon support grips; include equipment-1 item/remote states.
5. **Alternatives:** reuse native meshes first; original dedicated arms when seam
   quality demands it. Simply unhiding world-space arms is insufficient.
6. **Dependencies:** D36G contract, or equivalent narrowly scoped helpers reviewed
   with this change; per-weapon grip and pose approval before asset completion.
7. **Risks:** crouch/swim double pitch, clipping, left-hand placement, muzzle drift,
   first-shot pose learning, costume mismatch and extra packet cost.
8. **Acceptance:** connected visible arms through idle/fire/throw/reload/switch;
   accepted weapon framing and all projectile/damage behavior unchanged.
9. **Tests:** all records in D36B, all six upgrades and direct 27-29 challenge
   records; stand/crouch, up/down, walk/run, swim/flight, quick kick, held items,
   interrupted switches, load while drawn/throwing and both aspect ratios.

### D36I - Native interaction arm continuity, slot 8 first

1. **Problem:** detached/popping arms during push/pull and close-use animations.
2. **Evidence:** current slots 8/10/5, native parts 10-15; 119-121, 277 and 270;
   traversal-only pre-polygon bridge excludes Push/Use/Mirror.
3. **Cause:** confirmed culling-policy difference; contribution of native
   alignment/body turns to every erratic slot-8 frame remains to be isolated.
4. **Solution:** admit eligible arm geometry through guarded safe clipping while
   retaining world depth; inspect residual motion and add constrained contact
   poses only where native pose plus clipping fails the visual standard.
5. **Alternatives:** dedicated phase-driven push/interaction pose costs more but
   can improve contact. Weapon-overlay arms would break world occlusion.
6. **Dependencies:** D36G or equivalent scoped ownership; validate slot-8 cause
   with depth/part trace before altering the pose. Can precede D36H.
7. **Risks:** arms through objects, duplicate limbs, native alignment fights,
   incorrect stow/return, callback or movement changes, regression of ladder arms.
8. **Acceptance:** slot-8 entry/hold/push/reverse/release has connected stable
   arms and unchanged travel/collision; wheel/vending also retain plausible contact.
9. **Tests:** current slots 8/10/5, plus 11/9 across equipment transitions and 12
   as control; look extremes, both view preferences, committed switch refusal,
   pause/load, damage where valid and 60/120 with replay restoration checks.

### D36J - Coherent body and quick-kick leg

1. **Problem:** body visibility discontinuity, odd thigh orientation and reported
   third-leg/depth appearance while retaining the successful Q mechanic.
2. **Evidence:** D36C, current downward/gait/kick captures, parts 1-8 and accessory
   21, `leg_pose`, end-for-end thigh code, separate gun/kick anchors.
3. **Cause:** thigh reorientation and split body/viewmodel policies confirmed;
   exact anatomical count and every holster interpretation remain visual hypotheses.
4. **Solution:** explicit lower-body visibility and right-leg/accessory ownership;
   correct near clipping allows natural thigh orientation. Calibrate hip/grip
   relationship using short and long weapons without changing damage reach.
5. **Alternatives:** original dedicated boot/jeans mesh when native leg cannot
   frame well; limited torso exclusion remains permissible. Pitch restriction
   requires evidence and user review and is not the starting remedy.
6. **Dependencies:** D36G; coordinate framing with D36H but preserve current guns
   as the initial reference. Identify the apparent extra piece before removing it.
7. **Risks:** swapped costume parts, mirrored equipment, disconnected knee, changed
   door kick, clipping at crouch height, stale hide after kick cancellation/load.
8. **Acceptance:** interpretable two-leg body and one moving right kick, connected
   knee/hip, correct orientation and convincing depth; exact Q damage/timing retained.
9. **Tests:** Boot/pistol/Gatling/RPG and variants; every costume; idle/walk/run/
   crouch/jump, up/down, kick start/hit/end/repeat/cancel, door 116, save/load and
   global view switch. Compare hit counters and callback arguments to baseline.

### D36K - Traversal and special-action visual coverage

1. **Problem:** accepted functional coverage does not prove presentation quality
   for all traversal poses, costumes, scripted events and handoffs.
2. **Evidence:** D36A state catalogue, current ladder control, notes 227-242 and
   the explicitly unresolved selector/alternate-camera cases from note 238.
3. **Cause:** coverage gap, not a blanket confirmed defect in those systems.
4. **Solution:** natural-route capture matrix; only identified visual defects get
   bounded pose/attachment corrections using the established ownership contract.
5. **Alternatives:** synthetic pose tests help guards but cannot establish reachable
   contact geometry or justify whitelisting unknown interactions.
6. **Dependencies:** D36G and relevant D36H/I/J changes; retain current routes as
   baselines, especially arms the user already considers good.
7. **Risks:** undoing accepted camera choices, chain visibility, armed mantle
   exclusion, native event timing, intentional death/cinematic views.
8. **Acceptance:** documented entry/active/exit/interruption coverage, no new
   disconnected limbs or uncontrolled geometry; unresolved routes remain named.
9. **Tests:** ladder/pole/wall/overhead/mantle/water-exit, swim/flight, mirror,
   door/lever/toilet/statue, gadgets, death/respawn and level transitions; both
   action choices, global third, Vanilla, and 60/120 plus high-rate stress.

### D36L - Presentation acceptance and restoration regression pack

1. **Problem:** static screenshots and pointer assertions alone cannot certify
   moving limb continuity, restoration, full weapon coverage or performance.
2. **Evidence:** sparse-sampling limits here, existing D35/D12 tests, D37 allocation
   failure and mask packet budget history.
3. **Cause:** incomplete combined validation, not an established new runtime bug.
4. **Solution:** reproducible private route/capture manifest with actual level,
   slot hash, part/pose/equipment IDs, view, rate, aspect and pass criteria; add
   focused semantic tests for ownership, transforms and cleanup where code changes.
5. **Alternatives:** manual-only review misses restoration/serialization errors;
   image-only golden tests are brittle across lighting and animated scenes.
6. **Dependencies:** selected implementation proposals complete; no certification
   of unimplemented work. Human visual acceptance remains necessary.
7. **Risks:** calling screenshots performance evidence, using stale saves, counting
   failed selections as coverage, or accessing player files for writes.
8. **Acceptance:** source/hash preservation, no gameplay/Vanilla regressions,
   reviewed motion captures, bounded packet/allocation use, compatible old/new saves.
9. **Tests:** D37A warm/reverse-order/cross-level allocation loads, per-action
   enter/leave/cancel, dark/gas/mirror/power-up lighting, 4:3/16:9, OpenGL/software,
   60/120 normal play, 180 support and 240/Unlimited robustness where supported.

Priority: D36G small foundation, then D36I slot-8 continuity, D36H combat/item
arms, D36J body/kick, D36K remaining quality coverage and D36L combined acceptance.
D36I and D36H have separate visible outcomes; neither requires rewriting gameplay.
Do not batch all assets and interactions into an unreviewable implementation.

## Review boundary

This is accepted research, not a new playable candidate or an acceptance of
D37A/D19Q3. Remaining uncertainty is part of the accepted audit. No implementation
proposal starts until explicitly selected. No player-facing behavior changed,
so the game manual does not need an update. Launch the unchanged current build:

```sh
python3 recomp/tools/local/run.py
```

Modernized is the default; P selects first person if needed. Gameplay capture is
automatic. `--vanilla` remains the one-launch original profile.

## Acceptance and closeout - 2026-10-26

The user accepted this research as integral to subsequent jobs and explicitly
requested lock-in and closeout. D36 and its research tasks D36A-F are Done.
Future presentation work should start from the state/part ownership catalogue,
traceability matrix and proposed job contracts above, preserving accepted gameplay
and investigating named gaps instead of treating hypotheses as proven causes.

D36G-L remain stable report-only proposals. No implementation job is selected or
started. Closeout changes documentation only; no source/asset change, rebuild,
game launch, player-file write or dependency-pin change. Existing framework
modifications remain untouched. Authored research, board/status/handoff and agent
reference updates are included in the authorized MusicMonsterMod commit/push.
