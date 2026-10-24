# D35G - First-person mantles and ledge hangs

2026-10-24. Done, user-accepted. User: "lock it in, close it out!".
Explicit implementation, documentation, commit and push closeout authorized.

## Behavior

MANTLE VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers FIRST PERSON
(default) / THIRD PERSON. CONFIRM applies and saves the choice. Global Third
Person wins. Modern controls and Independent camera remain prerequisites.
Profile schema 46 adds `mantle_view`; `run.py --mantle-view first|legacy-third`
saves it. Vanilla retains its original presentation and controls.

First Person covers ground/object mantles, airborne catches, ledge hangs,
shimmying, pull-ups, drops and water climb-outs. The eye follows native body
travel over the lip with a stable 294-unit upright offset, without neck lean
or walking bob. Native hand poses retain world-depth occlusion and clipping.
Mouse orientation remains independent of the animation. Existing E, W, A/D,
Space and S behavior, attachment, collision, movement and weapon restoration
are unchanged.

## Implementation

- Presentation-only Mantle classification covers modes 6/7 with ledge poses,
  mode 8 poses 134-142, their mode-0 initialization, and 108 drops from 6/7/8.
  Shared mode-3 catches and exclusive ladder/pole exits retain D35E/F ownership.
- Early mode-8 exits with previous mode 4/5 resolve WaterExit and use MANTLE
  VIEW. Lingering water flags cannot pin their eye to the swimming plane.
- Only first-person presentation widens the camera state gate for early
  entry/drop transients. This does not grant movement, aim or firing leases.
- The existing interaction-arm clip path and actor-scoped native near-cull
  bridge now cover ledges and water exits. No joint matrices, projection or
  physical collision queries are replaced.
- Existing render snapshots carry the resolved action. No new gameplay latch,
  code-generation hook, framework change or submodule pin is needed.

## Verification

Accepted executable SHA-256:
`96cb138081ecd17feda384a95f41ff7d26a49d9721563da34c898e6838d9f41d`.

- Player build passes. Native controls, input, aiming and near-clipping suites
  pass. Controls uses the owned EXE, clean LEVEL00 fixture, LEVEL01 and all 21
  selectable overlays. Policy checks cover all 4096 fact combinations,
  denials, lifecycle poses, both choices, global view, suspended input and
  save reacquisition. Existing ladder/pole ownership, replay, scoped arm
  clipping, failed pull-up rollback and Vanilla refusal checks still pass.
- All 96 profile tests pass: schema-45 migration/backup, retained choices,
  menu sidecar merge, validation, launcher flags and Vanilla environment.
- Private Xvfb/OpenGL alley route, 120 FPS setting: dumpster/platform climb,
  saved platform hang, A shimmy, pull-up, Space drop and land return. All 158
  sampled states retain full first-person blend. The 60-setting Third Person
  action comparison resolves 77 Mantle samples with effective first person
  false and restores global First Person afterward.
- Private sewer ground climb, 120 setting: 70 samples, including 139/140,
  all fully first person, ending on land with the weapon restored.
- Private airborne catch into the sewer ledge, 120 setting: 55 samples,
  including jump 109, catch 148 and pull-up 140, all fully first person;
  normal movement and weapon restoration follow.
- Final-build natural water exit, private historical level-6 save, 120 setting:
  surface, swim south, E stow/climb, 140 pull-up, land and weapon restoration.
  All 117 samples retain full first person, including 16 Mantle samples.
  The brief 8/4 and 8/5 entry windows are covered by native tests; the live
  sampling captured the subsequent 8/8 phase.

- Final-build 60-setting water comparison: 138 samples, 17 Mantle samples
  with effective first person false, then selected first person and weapon
  restoration on land.
- Final-build 60-setting saved-hang menu checks: P both ways, cancel with zero
  writes, CONFIRM to Third Person and back to First Person, live while hanging.
  Menu layout and looking up/down/sideways inspected. An 18-image pull-up
  sequence shows native hands over the lip and the eye clearing onto the
  platform; final effective action is Ground with full first-person blend.

All diagnostic sessions are stopped.

The initial level-12 route never reached swimming and does not count as water
exit evidence. Cardinal-direction searches in the two private water saves
preceded the successful level-6 south exit. The alley/sewer/air runs preceded
only the final restriction of the new presentation bridge to global First
Person and MANTLE VIEW First Person; the final build's native suite passes.

Private scripts, copied historical saves, settings, traces and images are in
`recomp/analysis/d35g/`. No player's writable settings, saves or cards are used.
Original media is read-only. No framework source, generated C or pins changed.
Implementation and documentation closeout explicitly authorized; see below.

## Acceptance limits

The user accepted the candidate; their acceptance does not enumerate additional
playtest routes. The recorded routes
are not whole-campaign or every-costume certification. Fresh live blocked
ceiling pull-ups, all furniture/crate variants, damage interruptions and
ladder/pole-to-ledge transfers remain coverage limits; existing native guards
and movement checks are retained, not claimed as fresh live coverage.
Frame-rate settings are not cadence benchmarks; dummy audio is not an audible
sound test.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

P selects global First Person. MANTLE VIEW defaults to First Person; try both
choices and CONFIRM. Gameplay captures automatically. Check a ground mantle,
airborne E catch, hang/shimmy, W pull-up, Space/S drop and E water climb-out.


## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export is
unchanged; application to the clean pinned framework index reproduced all 51
changed/new files. No pins changed. No post-acceptance gameplay edits, rebuilds
or launches; no player data touched. No next job started.
