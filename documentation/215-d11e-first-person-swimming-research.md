# D11E - First-person swimming research

2026-10-24. Research requested before implementation. No gameplay change.
Status at research handoff: Needs review. Implementation was subsequently authorized;
see [accepted implementation note 216](216-d11e-first-person-swimming.md).
D11E was subsequently user-accepted and closed; the research evidence below is historical.

## Conclusion

The existing system provides a credible implementation path. Swimming already
has independent camera ownership, camera-relative movement, view-aligned shots,
costume-aware joint lookup, first-person draw hooks, near clipping and replayed
camera presentation. There is no demonstrated need to replace swimming physics,
create new assets, or add a renderer/framework feature to begin a prototype.

This establishes readiness for a bounded prototype, not proof that enabling the
view produces a finished feature. The main work is a stable swim eye position,
waterline behavior, body visibility and weapon presentation. Near-wall and
water-surface rendering require visual verification with the actual eye camera;
an orbit-camera baseline cannot answer them.

## Scope and baseline

User selected D11E and requested deep research and sufficient information to
make it real. Existing prerequisites D08O, D11 and D22B are complete.
Modernized plus Independent camera owns this feature. Third person, Vanilla,
Original camera, original oxygen/drowning, swim physics and allowed weapons
retain their existing contracts. No new scuba item or underwater weapon rules.

Accepted executable used throughout:
`95b61d027a5b0a3bf152d8fb59a1ab3881a6bc9dc173ef4eb8c879902de9e88f`.
No rebuild, generated-source edits, framework edits or commit/push.
The pre-existing patched framework worktree and both submodule pins were left
alone. All new runs used a separate settings file and card directory beneath
`recomp/analysis/d11e-research/`, copied from earlier private diagnostic fixtures.
The player's cards, saves and settings were not used by these runs.

## Current ownership and hook path

| Area | Verified source behavior | D11E implication |
| --- | --- | --- |
| Presentation ownership | `modern_controls.cpp::state(true)` admits water; `camera_eligible` and identity checks gate the normal camera. D26B separates presentation from captured action input. | Do not use the action-only swim lease to decide whether a paused eye camera remains owned. |
| Explicit refusal | `first_person_update` computes `in_water_early() && !shallow_land_water_early()` and refuses that state, reporting `swimming`. | This is the direct feature gate. Removing it is necessary but insufficient. |
| Camera anchor | `orbit_constraint` supplies the requested anchor before the original solve. `first_person_anchor` uses neck translation relative to root/pivot, dead bands, ground bob and glide. | Add a swim-specific presentation policy here; preserve the solved camera/room path. |
| Camera orientation | Host yaw/pitch drives `view_matrix`; underwater steering separately rotates Duke toward thrust. | Eye orientation must remain view-owned, never copied from the swimming body's rotation. |
| Original camera | Owned EXE disassembly of `8003ade4` confirms eye construction via `80039c7c`, room walk via `80039dd0`, and subsequent geometry queries. Mode changes to 5 also select a camera configuration through `800286b8`. | Keep these operations. Verify the original underwater configuration does not undo eye placement/projection during transitions. |
| Model identity | `duke_model.inc` maps parts through the model table at `+0x24`; head is part `0x10`, not a fixed joint index. | Reuse the lookup on every costume. |
| First-person visibility | Draw-scoped head hiding; arm suppression while a weapon/item is held; the original near cull otherwise handles much of the body. | Horizontal swimming can expose torso/legs or unarmed arms differently. Inspect held and holstered cases before choosing additional draw-only hiding. |
| Aiming | `weapon_aim.cpp::shot_ready` already includes `swim_weapon_ready()`. D08O1/O2A/O2B own movement-plus-fire and arm presentation. | Preserve shot origins, collision, damage and third-person arm behavior. |
| Replay | `RenderState` publishes first-person blend, ownership, weapon poses and crouch eligibility to workers; late camera rotates about the eye. | Publish any new draw-time swim state explicitly; workers do not rerun simulation updates. |
| World changes | `camera_world_reset` clears eye smoothing, bob history and ownership on loads/level changes. | Clear new swim presentation history there too. Do not blend across unrelated saves. |

Source locations are `recomp/src/ttk/first_person.inc`, `modern_controls.cpp`,
`swim.inc`, `duke_model.inc`, `weapon_aim.cpp`, `frame_replay.cpp`, and
`recomp/tests/local/modern_controls_native.cpp`.

Six relevant existing guards were independently checked against the owned EXE:
`8003ade4`, `80097c04`, `800348d8`, `800455bc`, `800987cc`, `800b42ec`.
All match their declared sizes and SHA-256 values. This checks static hook
identity; it does not replace runtime overlay authentication. Baseline live
samples also retained authenticated swimming camera ownership.

## Water states and transitions

| State | Current behavior | Proposed first-person contract |
| --- | --- | --- |
| Grounded shallow/mid water | Land movement, including crouch; existing first person | Keep current land eye and crouch behavior. |
| Surface, mode 4 | 122 idle, 125 forward, 126 back, 123/124 strafe; body pinned to surface | Stable surface eye, independent look, no walking bob. |
| Dive, mode 5 / 133 | Ctrl starts the original dive; D08O4 handles release | Smooth eye transition without following the head-first body tumble or changing dive duration. |
| Underwater idle, 5 / 127 | Original floating pose | Stable underwater eye; no breathing-driven viewpoint motion. |
| Underwater thrust, 5 / 128-130 | Original thrust/collision along body yaw and pitch; host steers from view and input | Same view yaw/pitch during W/S/A/D, diagonal, Space and Ctrl; bounded positional changes between idle and stroke. |
| Surfacing | Original integrator returns to mode 4 | Stable crossing and return to surface eye; no orbit flash. |
| Land-side deep-water transitions | Existing free-swim branch includes 70/80/81/175-177 before native handoff | Investigate and admit valid transition states rather than gating solely on modes 4/5. |
| E climb-out / attached traversal | E stows weapon and requests original mantle; existing first person temporarily yields to orbit for traversal | Recommended initial scope retains this deliberate mantle override and immediately regains first person on supported land. Verify with user during prototype review; uninterrupted first-person mantle would expand the existing traversal policy. |
| Death, scripted camera, unsupported identity | Existing original-camera ownership | Keep deliberate fallback and retain the saved first-person preference. |

Water surface is `player+0x834`, with `0x40000000` meaning no water. Game Y
grows downward. Water ownership/oxygen uses original `+0x20c`; floor is
`+0x1c8`. A camera crossing the water plane must not change oxygen, buoyancy,
body position or water ownership. Presentation classification and gameplay
classification are separate concerns. Do not infer all transitional states
from animation numbers alone: some numbers also occur in ground/wade contexts.

## Live baseline measurements

Private OpenGL runs on the unchanged accepted build used diagnostic input via
`DNTTK_TEST_INPUT`, dummy audio, an offscreen surface and port 9411.
These are functional research measurements, not physical-monitor feel or
performance tests. No foreground window was needed.

`survey.py` recorded 180 samples at a configured 60 FPS in level 2 and level 6.
The historical private file slot 02 starts at the level 2 surface; file slot 11
starts underwater in level 6. They are not the player's current UI slots.
Some script labels say surface for slot 11, but the recorded mode proves that
fixture starts underwater. Interpret the recorded state, not the label.

| Observation | Evidence | Consequence |
| --- | --- | --- |
| First-person request survives water | All 180 samples report `fp.reason=swimming`, blend 0; request stays selected | Current third-person view is deliberate policy, not lost preference. |
| Surface neck is already near the water plane | Level 2 surface neck minus 96 is roughly 89-103 units above the plane in sampled idle/forward motion | A small stable surface eye clearance has an anatomical starting point; these are not final tuning constants. |
| Underwater posture changes eye position greatly | Level 2 rest neck Y relative to root about -198; forward stroke +15 to +56; dive samples +197 to +256 | Raw neck-follow would move vertically by hundreds of units solely with pose, in addition to real travel. |
| Forward reach changes too | Level 2 underwater rest neck X/Z about 11/14; forward stroke about 149-173 / 126-146 | A neck camera can move ahead of the collision body near walls. Bound the eye offset and test collision/room visibility. |
| Different costume agrees on the issue | Level 6 rest neck Y -198; sampled stroke about -74 to +18, with forward Z near 188-209 | The problem is not specific to one pool/model. |
| Movement and firing already coexist | Level 2 W+fire traveled about 468 units over the sampled interval and counted five shots | No new swim combat mechanic is needed to begin first-person work. This does not independently verify every projectile/hit. |
| Fixture geometry matters | Level 6 forward cases travel little or stop while back/strafe travel over 500 units | The fixture faces nearby geometry. Do not mistake this for a newly broken forward-swim mechanic or use it for speed comparisons. |

Samples use multiple debug reads and are not atomic per-frame captures. Joint
and actor values may straddle an update; ranges establish large pose differences,
not exact per-frame derivatives. No prototype eye was rendered in these runs.
The existing first-person filter would attenuate some raw motion; measurements
above describe raw anchor inputs, not a measured defect in a new camera.

A second 120 FPS baseline run records longer ascents and view-pitched movement
in `transition-120.json`. See the results addendum below. Baseline framebuffer
captures were visually inspected; they show the expected orbit view and nearby
water/geometry, not first-person rendering acceptance.

## Proposed implementation

1. Add a narrowly gated swim presentation classification within authenticated
   Modernized independent-camera ownership. Preserve shallow land behavior and
   D26B's suspended-input presentation. Include entry and exit transitions.
2. Add a stable swim anchor. Start from the body's real world travel and a
   costume-aware reference height; use the anatomical neck as a bounded posture
   reference rather than copying every stroke. Keep view orientation independent
   of body yaw/pitch. Compare a bounded neck-relative solution with a stable
   body-relative solution in private visual captures before locking constants.
3. Give surface/dive/underwater transitions explicit smoothing and reset rules.
   Retain immediate real translation, limit pose-induced travel, remove residual
   land bob/glide in swim states, and do not filter through a wall or across a
   save/level change. Keep camera geometry queries and room walking intact.
4. Reuse first-person weapon framing. Seeded guns already use a standing firing
   reference, independent of the live torso. Crossbow/pipe bomb and other
   unseeded/animated poses need examination: `display_pose` reads live
   torso-relative hand animation, while the swim arm fix deliberately cancels
   the pitched torso in world space. Reapplying a standing torso can therefore
   produce a different displayed rotation. If reproduced, reconstruct the
   intended animation channel for display, analogous to D08K10, without undoing
   third-person shoulder aiming. Do not tune one weapon to conceal a general
   transform error. Keep recoil, reload, throw and muzzle-flash attachments.
5. Publish any new draw eligibility/reference state in `RenderState`; reset it
   on loads/teardown. Keep third-person and Vanilla paths unchanged.
6. Add diagnostic eye target/solved position, swim presentation state and
   waterline delta for validation. Remove or gate verbose logging for play.

The initial route can reuse current hooks and title source. A new hook should
only be added after a specific missing observation/control point is demonstrated,
with verified callers and guards. No generated C hand edits.

## Questions that need the prototype, not further speculation

- Which bounded anchor reads best at the surface, during dive and under thrust?
  Exact offsets, smoothing rates and hysteresis are intentionally not fixed yet.
- Does the original water surface draw correctly from both sides at eye height,
  including near-plane intersections, shallow edges and neighboring rooms?
  No custom tint, fog or refraction is justified by this baseline. The original
  underwater-effect selection has not been fully traced; classify any mismatch
  from captures before changing rendering or camera environment flags.
- Does horizontal Duke expose chest/legs or holstered arms inside the eye?
  Hide additional parts only in the appropriate first-person draw scope.
- Do unseeded weapons double-apply body compensation, and can their original
  reload/throw animation be retained with a display-only transform?
- Does mantle exit need uninterrupted eye view for the user's intended feel?
  Existing traversal fallback is explicit, but D11E acceptance must not silently
  claim that a third-person mantle is continuous first-person play.

## Verification and acceptance plan

| Area | Required checks |
| --- | --- |
| Native ownership | Surface, dive, thrust, surface return, shallow/deep transition; requested first/third, inactive input, pause, focus, Original camera, Legacy, Vanilla, identity failure, scripted/death ownership. |
| Anchor | Translation follows body; posture/heading change bounded; no land-bob carryover; water sentinel and valid model handling; load/level reset; no writes to gameplay water/velocity fields. |
| Draw state | Costume part lookup, held and holstered visibility, head restoration, load during draw, weapon/muzzle attachment and worker-state round trip. |
| Two live levels | Level 2 sewer and level 6 water as repeatable starting fixtures, with fresh entry/exit routes; add BLOOD BATHS level 12 for different pool geometry if practical. |
| Water movement | Stand/wade/fall entry, idle, W/S/A/D, diagonals, Shift, look up/down, vertical Space/Ctrl, brief Ctrl release, floor/ceiling contact, repeat surface crossing, near-wall turns. |
| Weapons | All permitted swim weapons including upgraded Gatling: ready, tap/hold fire, release tail through surfacing, reload, switch, holster, E without a ledge, pipe-bomb throw and crossbow. Compare crosshair hits, not just the displayed barrel. |
| Exit | E mantle, aborted mantle, shallow return and landing; preference/capture recovery, no lingering hidden body or swim smoothing. |
| Presentation | 60/120/240 FPS, 4:3/16:9, relevant renderer paths, views facing/along walls and through room boundaries, surface from above/below. Inspect actual presented images as well as canonical framebuffer captures. |
| Continuity | Pause/console/focus, P toggles, save/load underwater/surface/dive and saves from the opposite selected view, natural level transitions where available. |
| Regression | Accepted third-person swimming/firing, ground first person and crouch weapons, jetpack fallback, traversal, Vanilla. |

Use native tests for meaningful ownership/transform invariants and live routes
for actual camera geometry and water rendering. Re-run controls/input/aim and
near-clip suites when the implementation touches their behavior. Functional
research runs do not count as those suites or a campaign pass.

If a private restore produces the known GPU halt, capture the live aperture and
RAM per [D20G](213-d20g-slot11-load-halt.md) before stopping; do not classify it
as a swim-camera regression solely because D11E was being investigated.

User visual acceptance in at least two levels remains required. Documentation
and commit/push closeout still require an explicit instruction; this research
entry is not completion acceptance.

## Related evidence

- [Original first-person foundation](61-d11-first-person.md), historical; current source supersedes its old anchor description.
- [View/control ownership](207-d26b-view-ownership.md).
- [Swimming research](55-swim-controls-research.md); early failed iterations are historical.
- [Swimming combat](107-d08o1-swim-fire.md).
- [Natural swim-fire pose](119-d08o2a-natural-swim-fire-pose.md).
- [Costume-aware swim aiming](146-d08o2b-swim-fire-costumes.md).
- [Crouched weapon presentation](192-d08k10-crouched-viewmodel.md).

Local evidence: `recomp/analysis/d11e-research/`: scripts, logs, sample JSON,
owned-EXE disassembly, guard results and baseline PNGs. These remain local.

## 120 FPS baseline addendum

The second run completed with 180 additional samples, 360 total. In both
levels, longer Space ascents transitioned from mode 5 (127/128) to surface
mode 4 (122). Final raw neck-minus-96 height was 94 units above water in
level 2 and 96 in level 6. Up/down look inputs changed the independent camera
pitch while underwater: approximately -0.38 to +0.70 radians across the level 2
runs and -0.55 to +0.55 across level 6 runs. Every sample retained requested
first person, `swimming` fallback and zero blend. This verifies baseline
ownership through these routes, not a first-person feature or achieved refresh
rate. The level 6 wall-facing limitation also applies to these runs.

Both diagnostic processes exited normally and no game process remained.
No live first-person prototype, natural land entry/exit route, mantle route,
240 FPS check, Vanilla replay or native suite was run in this research phase.
Those are implementation acceptance work, not claimed research results.
