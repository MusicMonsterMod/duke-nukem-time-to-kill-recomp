# Gatling grip, Buffalo grip flicker and medieval corridor wall intake

2026-10-26. Research and backlog setup only, explicitly requested by the user.
D36H1, D36H2 and D17Z are Todo, unselected for implementation. No code, asset,
build, game launch, player settings/save/card write, commit or push is included.
Existing dirty framework files and dependency pins are preserved.

## Evidence and boundaries

Use the accepted [D36 audit](257-d36-first-person-presentation-audit.md) labels:
S = inspected source; A = accepted prior behavior; U/H = user report/hypothesis.
V below denotes reviewed frames from the user's video, not a fresh private-run
L reproduction. No new live reproduction was performed during this intake.
The user's slot-12 load/walk instruction is retained as the reproduction route
for the queued wall job. A save slot is not a permanent level identity.

A: latest accepted D36K executable is
`ba5f474f1af044de08ca9e71691d6e32eb766d4b86ae5dfc974814377f7de803`
(note 262); that is a documented baseline, not a new hash measurement here.
[Connected arms and Buffalo motion](259-d36h-connected-arms.md) remain accepted.
These narrow follow-ups do not reopen the entire D36H or renderer work.

## D36H1 - Gatling left-hand underside contact

U: in first person at idle, the left hand should actually hold the bottom of
the Gatling. No new Gatling recording was supplied with this report.

S: `first_person.inc:weapon_view_slot` maps upgraded record 28 to slot 7;
key 4 selects the Gatling family. `first_person_weapon_joint` enables support
arms for slot 3 and slots 5-11. `display_arm_matrix` obtains left wrist part 12
through the semantic model map, transforms the native wrist relative to the
right-hand display matrix, and adjusts support position for weapon scale.
The Gatling has slot offset (110,80,290) and tilt (-12,-26,16), but no dedicated
underside grip correction. The ready right hand can use a seeded firing pose
while the support relation comes from native matrices. This is an investigation
target, not proof of the reported gap's cause. Connected endpoints alone do
not prove palm contact with the gun surface.

When selected: capture actual base/upgrade records at idle before/after firing,
identify the underside contact point and compare wrist orientation and position
in weapon-local space. Prefer a bounded Gatling support-grip adjustment if the
measured native relation is unsuitable; preserve the accepted right-hand anchor,
weapon framing, muzzle, bone lengths and connected chain. Do not move every gun
or add another skeleton/animation system to solve one grip.

Acceptance: the visible palm plausibly supports the underside at settled idle,
without a floating gap, deep penetration or a pop on fire/release/switch.
Verify base Gatling and Laser Gatling, fresh selection and post-fire idle,
look extremes, stand/crouch, movement, swim/flight where permitted, Q, view
switch, save/load and 60/120 replay. Preserve other weapons, items, reflection,
third person, Vanilla, damage and timing. Apply D36 ownership/lighting contracts.

## D36H2 - Buffalo Rifle grip-area flicker

U: a small section where Duke grips the Buffalo Rifle flickers. The exact hand,
polygon and idle/fire/reload phase are not established by this report. Do not
assume this is the earlier rejected whole-rifle roll or reload vibration.

S/A: key 9 is record/slot 6. The accepted `rifle_reload_pose` uses upper 27's
native elapsed clock for smooth lift/pull. `rifle_ready_arm` resolves final
reload channel keys for both arms; `display_arm_matrix` builds the left-hand
relative grip from them, including a 70-unit pull stroke and wrist orientation.
The weapon/right hand share `weapon_matrix`; `display_hand_done` prevents arm
previews advancing recoil twice. Render snapshots interpolate rifle lift/pull
and recoil. `near_clip.cpp` supplies the viewmodel clipping/depth path.
Existing native controls tests cover rifle motion, endpoint return, missing-data
fallback and replay state, but do not establish stability of every visible pixel.

H: overlapping hand/weapon polygons, a wrist/forearm seam, clipping or depth
ordering, texture sampling and mismatched presentation timing are alternatives.
None is a confirmed diagnosis. Identify the affected triangles and compare
adjacent frames before changing pose, mesh or renderer depth policy.

When selected: record idle and repeated fire/reload/settle at close inspection,
confirm actual equipment 2/record 6, identify costume/part/triangle and phase,
then compare base and replay frames at 60/120. Inspect draw ordering, shared
matrix, source polygon ownership and texture/depth metadata at that location.
Use private copied slot 9 from the earlier rifle work only after verifying its
current contents; the user did not supply a new Buffalo-specific save route.

Acceptance: no intermittent disappearing/flashing patch at the identified grip
through idle/fire/reload/settle, pitch extremes and crouch; preserve the accepted
raise/cock/return and support stroke. Check save/load during upper 27, switching,
replay, relevant costumes, other weapons/items, lighting and both view choices.
Native events, ammo, aim, damage, third person and Vanilla remain unchanged.
The older `research/screencaps/buffalo.mp4` documents a superseded reload defect;
it must not be presented as proof of this new small-section flicker.

## D17Z - Slot-12 medieval corridor wall glitch

U: load current UI slot 12 and walk down the corridor to observe glitching walls.
Reference: `research/screencaps/medieval-glitch-wall.mp4`, present locally;
11.4 seconds, 1462x1020, nominal 60 FPS. SHA-256:
`93c8a4c2e7ee4e8c3738783df4f5ca175394d7ea8ceaa8ed861da1ddc2bf2b8c`.
V: sampled frames show the first-person rifle view moving along a stone corridor,
looking toward a dark arched feature and into the adjoining stone passage,
then facing the corridor again. Overview samples at 0/3/6/9 seconds and a denser
8 FPS sample beginning at 1 second were reviewed. These identify the route and
surfaces; they do not isolate a faulty primitive or establish the temporal cause.
Local derived sheets: `recomp/analysis/visual-backlog-intake/`; never commit the
video, sheets, saves or extracted assets. Renderer/profile/rate settings and the
recording's executable are not established from the video metadata.

S/A: `near_clip.cpp:world` owns guarded world-mesh interception (0x80011020,
callers 0x80037344/0x80037c54/0x80062db8), safe projection, polygon takeover,
clipping, packet budgets and native fallback. Its world path and viewmodel path
are distinct; the presence of the rifle is not proof of a shared arm defect.
[D17N](98-d17n-world-subdivision.md) and [D17Q](99-d17q-uv-seams.md) are accepted
history. D17Q fixed per-triangle mirrored-sprite UV correction on perspective
world triangles in GL/VK. Its historical UI slot 12 was a club panel, not proof
that this current medieval corridor has the same cause or save contents.

When selected: hash/copy the current slot-12 save into a private writable test
setup, verify active level/overlay, record backend/profile/texture/geometry/rate,
and follow the user's corridor route. Capture matched advance/retreat/turn and
stationary intervals. First classify texture seam versus missing geometry,
depth overlap or visibility change. Trace affected world polygons, clipping/native
handoff, UV/precision validity and packet budgets; compare 60/120 replay and
Corrected/Original settings only as controlled diagnostics. Portal/room culling
and interpolation are hypotheses to investigate if whole wall sections vanish.
Do not disable accepted precision/near clipping globally or assume D17Q recurred.

Acceptance: the identified wall glitch is absent along the same slot-12 route,
including approach/retreat/turn, with matched before/after motion evidence.
Preserve D17N/Q precision and seams, nearby world/prop occlusion, sky and
platform behavior, first/third view, Original/Vanilla, simulation rate and packet
budgets/replay reliability. Add focused regression coverage based on the proven
cause; export and verify the accepted-source patch if framework code changes.

Mandatory delivery reminder when fixed: tell the user that
`research/screencaps/medieval-glitch-wall.mp4` is still there to compare against
the fix. Include that reminder in the playtest handoff and closeout, together
with the slot-12 corridor route. Keep the original recording intact.

## Dependencies and verification plan

D36H1/H2 depend on accepted D36G/H and follow note 257; D17Z depends on accepted
D11B/D17N/D17Q as regression baselines. No unresolved prerequisite requires
starting another job, and none of these jobs selects D36L. D19Q3 remains its
separate Needs playtest entry. Each future implementation requires its own
selection and human visual acceptance.

Research validation: inspected current source, accepted notes, native test
coverage and video metadata/frames. No tests/build were run for documentation
intake; no new live reproduction or root-cause proof is claimed.
