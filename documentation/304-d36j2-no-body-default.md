# D36J2 - No-body as the Modernized first-person default

2026-10-28. Done: user accepted the integration and explicitly authorized
lock-in and closeout, including commit/push. D36H5 and D36J3 remain unstarted.
Prior verification below retains its original coverage limits.

## Change and ownership

S: ordinary Modernized first-person draws now suppress semantic body parts
0-9/20/21 without an environment flag. Only exact `DNTTK_FP_BODY=visible`
selects the earlier diagnostic body policy; omitted, empty, `hidden` and unknown
values select no-body. There is no saved preference, menu entry or launcher
schema change. Direct executable launches use the same default.

This changes the accepted D36J1 visibility gate only. Q and native door-kick
owners still execute first. Arms, weapons and mission items retain their owners.
The primary-player/camera/caller/first-person guards exclude reflections, other
actors, third person and Vanilla. D14E1 still protects unsafe empty-handed arm
views, and protects body geometry in the diagnostic visible comparison.
Native matrices, collision, aim, attack timing/damage/sound, camera/FOV, allocation,
replay layout and save format are unchanged. No generated C or framework edits.

The accepted D36 audit's evidence labels apply: S source/native tests, L private
runtime observations, A prior acceptance, U user report. A: D36J1's direction
was accepted. U: exposed shoulder/upper-arm ends remain the priority D36H5
concern. This default integration does not resolve or accept that concern.

## Native verification

S: player executable builds; controls, input, aim and near suites pass. The
controls suite uses the owned executable, LEVEL00 fixture, LEVEL01 and all level
overlays. Default, visible, hidden, empty and unknown policies are tested across
two costume maps, projection inputs and armed/unarmed displaced-eye cases.
The visible comparison retains torso fit and D14E1 tests. Default Q has the same
display matrix as visible Q, suppresses the opposite leg and returns to hidden
body after recovery. Door presentation matches the visible comparison. Existing
hit count, callback arguments, impact sound/timing and gameplay-matrix checks pass.

Action tests check hidden torso/root/neck/accessory ownership alongside contact
arms, with third/Vanilla, pause/load, interruption and replay restoration. Mirror
isolation now checks all semantic parts 0-21, not just arms. The legacy cowboy
comment conflated neck joint 14 and head joint 15; tests now distinguish the
body suppression from the existing separate head owner. Native tests are not
retail-map collision or visual acceptance evidence.

## Private runtime evidence

L: captures and recipes are local under `recomp/analysis/d36j2/` and
`recomp/analysis/d36j2-d35*/`. Each run uses private profiles/cards and no body
override unless named `visible`. Existing route cards are private historical
fixtures; current-slot comparisons use copies of the player's current saves.
OpenGL with Xvfb and dummy audio; no audible-quality claim. Raw state samples
and images are sequential, not atomic snapshots. Retail-derived assets remain
local. The standalone [artifact](artifacts/quick-kick-realism.html) links selected
images and the motion recording without a server.

- `default60`: default/60/80/90, level/down/up, idle and Q display frozen at 13,
  current slot 4, equipment 2/record 4, 16:9. Inspected FOV-90 down samples show
  ordinary body absent and the unchanged temporary Q leg present.
- `motion120`: unfrozen Q/movement/jump, poses 63/72/76/96/97/105/178, equipment
  2/4. Airborne pose 97 has mode 9/9. Pause/resume, first/third/first and private
  save/load recover. Additional current slots 2/6/7/9/12 are scene samples only.
  1103 replay sessions, 1247 completed worker jobs, zero session/worker failures.
  Requested 120 is not sustained measured 120 FPS; video capture is 30 FPS.
- Door: native pose 116, no Q attacks/hits/sounds, right-leg draw counter active,
  return to 63. The inspected door-contact capture is occluded by the door;
  it does not certify the entire visible leg motion.
- Mirror: near/far and grooming 278. Inspected near capture shows complete
  reflected head/torso/arms; feet are outside this crop. Native isolation checks
  cover all mapped body parts. No claim of every mirror/costume combination.
- Pole: 154/191/192/194/195/196, modes 3/8, mount, descent/top exit, drop and
  pause/load. Ladder: 148/156/185/186/188/189/207, mount/climb/descent/exit,
  pause/load and drop. Their trace files retain actual reached states.
- Mantle: 139/140 and ledge 149/152, modes 6/8. Overhead: 149/150/151 mode 7,
  directional motion, look and release. Wall: 147/155/197/198/199/200/202/203/206,
  mode 3/8, top/bottom/drop, pause/load; 790 wall-owned samples.
- Use fixtures: 275 and 279 return to 63. The 275 fixture had already consumed
  its one-time effect; unchanged effect byte is not a fresh reward test.
- Water climb-out: swim 122/125/127/128 mode 4/5 -> 140 mode 8 -> ground.
  This route resolves to `mantle`, not `water-exit`; separate live water-exit
  branch coverage remains unverified. Native water-exit policy tests pass.
- Swim: two private scenes reach mode 4/5 with armed record 4, directional and
  vertical input, firing requests, view changes and save/load. Input requests
  alone do not establish every projectile's outcome.

- Jetpack at requested 120: 163/164/165/170, mode 10/10, equipment 2/4,
  actual `jetpack` owner/effective first person. Private load restores 164;
  switching off returns to ground 63. A level-5 -> level-0 transition and a
  separate twice-repeated level-0 restart retain effective first person.
- Equipment at FOV 90: equipment 2 records 4/8/12 and equipment 0, idle/movement/
  Q/recovery. Z attempts remain armed and are not counted as holstering; Boot
  selection provides the actual equipment-0 case.
- Aspect runs: 4:3 (`--widescreen off`) at 60 and 21:9 at 120, FOV 60/90,
  downward idle/Q. Unfrozen early-Q captures include phases 3/6 and do not
  reliably show the leg in frame; separate `*-pose13` runs fix display phase 13
  for inspection. In the first 21:9 phase-13 sample the native capture contains
  Q while the presented capture still shows the prior frame. `aspect219-settled`
  holds Q for 0.3 seconds instead of 0.12; inspected presentation then contains
  the leg. This demonstrates the sequential-capture limitation, not an atomic
  renderer comparison. The unfrozen 21:9 run records 470 replay sessions and 553
  completed worker jobs, zero session/worker/plugin failures.
- Diagnostic `visible` launch retains the body route at FOV 60/90. No new
  player body setting is introduced. `items` initial slot-9/11 attempts remain
  equipment 2/4 and are excluded from held-item coverage. The corrected
  `items-use` route approaches and uses the object: actual equipment 1/record 11
  and 1/record 6, followed by native poses 267 and 266 respectively. Inspected
  item-6 capture retains the card and connected right arm; close wall occludes
  much of the scene. This is bounded item handoff coverage, not all items.

Accepted executable SHA-256:
`c3757ea848040724af4707f35764d9dcddb90a55709e07dc8060240b3fd48543`.


All 41 player save/card/settings files retain SHA-256 and mtimes through the
completed runs. Diagnostic processes are stopped. `git diff --check` and
`python3 scripts/ci/check_repo.py` pass. Existing framework modifications and
submodule pins are untouched. The implementation verification above preceded
acceptance; commit/push is now authorized for closeout.

## Review limits and launch

The user accepted this bounded default integration. This does not certify
every untested motion/action combination. Broad arm/shoulder redesign is D36H5;
kick motion/overlap refinement is D36J3. No claim of all weapons/upgrades/items,
all costumes or every lighting/collision/action combination. Existing black
edge triangles are not fixed by this policy. Software rendering and a fresh
live Vanilla route are not newly certified; native Vanilla guards pass.

Normal launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Diagnostic comparison, one launch without saving a preference:

```sh
DNTTK_FP_BODY=visible python3 recomp/tools/local/run.py
```

Modernized remains default; `--vanilla` remains the one-launch original profile.


## Acceptance and closeout - 2026-10-28

User: "teriffic, accepted, done, lock it in and close it out". D36J2 is Done.
Closeout records this acceptance and commits/pushes authored implementation,
tests, manual and documentation as MusicMonsterMod. Prior build/native/runtime
results stand; no new gameplay change, build, launch or player-file write.
The existing framework patch is re-exported and verified against the pinned
clean dependency. D36H5 and D36J3 remain Todo; no next job starts automatically.
