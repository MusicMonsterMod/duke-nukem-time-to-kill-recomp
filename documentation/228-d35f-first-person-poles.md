# D35F - First-person chains and poles

2026-10-24. Done, user-accepted. User: "so so good! let's lock it in and close it out!".
Explicit implementation, documentation, commit and push closeout authorized.

## Behavior

CHAIN/POLE VIEW in OPTIONS > ADVANCED > ACTION PERSPECTIVE offers FIRST
PERSON (default) / THIRD PERSON. CONFIRM applies and saves the choice. Global
Third Person wins; Modern controls and Independent camera are prerequisites.
Profile schema 45 adds `pole_view`; `run.py --pole-view first|legacy-third`
saves it. Vanilla retains its original camera and controls.

The eye follows the native attached body, 294 units above its root, without
neck-animation sway or walking bob. Looking around does not rotate Duke.
The existing top-mount half-turn still moves Duke around the column; the
view keeps its world heading. Hands retain native climbing poses and normal
world-depth occlusion, rather than becoming camera-facing weapon overlays.

The existing view-side A/D latch remains: a held direction does not change
its pad mapping when mouse look crosses sides. Releasing A/D lets the next
press use the new view side. Native attachment, circling, descent, floor
queries, top exit, let-go and phantom upper-floor rejection are unchanged.

## Implementation

- Shared presentation policy adds Pole. Attached 147/148/154 and 191-196
  require pole-family flags 0x400 with 0x100/0x200 excluded. Scenery names do
  not classify objects. Pole-exclusive exits 191/196 can already own their
  landing surface in mode 8, previous 3/8.
- An attachment-cleared 108/9/3 drop can retain preceding Pole presentation
  until the ordinary fall policy takes over. This is presentation only and
  grants no movement, aiming or firing permissions.
- Eye positioning uses the native 225-unit attachment circle. For wider
  valid column boxes, a radial presentation-only clearance places the eye
  outside the box's enclosing circle plus 32 units. Body root, attachment,
  animation and collision are not edited. The normal camera solve remains.
- Pole arms use the accepted ladder interaction clipping and actor-scoped
  near-cull bridge. Joint matrices and GTE projection remain native.
- Effective action is part of the existing replay snapshot. No new gameplay
  latch, generator hook, framework edit or submodule pin is needed.

## Verification

Accepted executable SHA-256:
`f8fcbc1703d1c2ff713c9df54c2eef10f794bb647284e002c0277814e6105506`.

- Player build passes; movie/math shard checks remain current.
- Native controls suite passes against owned EXE, clean LEVEL00 fixture,
  LEVEL01 and all 21 selectable overlays. New checks cover policy truth
  combinations, authenticated shared catches, lifecycle poses, exits/drop,
  both choices, global view, pause, save-load reacquisition, replay, arm
  matrices, scoped clipping, held-side latching and wide-column clearance
  without root/attachment changes. Existing no-floor stall drop, native
  floor-below/phantom-floor-above and Vanilla refusal checks still pass.
- Native input, aiming and near-clipping suites pass. All 95 profile tests
  pass, including schema-44 migration/backup, retained choices, menu-sidecar
  merge, validation, CLI and Vanilla's legacy-third environment.
- Private Xvfb/OpenGL route at the 120 FPS setting: chain type 842 at
  0x801dd3a4, top mount, save/load while attached, pause/resume, descent,
  lower-floor exit 191, top exit 196 and Ctrl drop. All 675 sampled states
  retained full first-person blend, including 549 Pole samples. This route
  preceded only the addition of pole-owned shared catches 147/148.
- A second probe descends below the top and circles both ways, with mouse
  look crossing sides while A/D stays held. Pad mappings remain constant
  during each hold; native circling and look work. Hands were inspected
  looking into the chain, up and down. At the initial top-rest position the
  first probe did not circle; lower-chain motion is the circling evidence.
- The historical D08J3 save contains another chain, type 841 at 0x801ddbe4.
  Its airborne catch exposed shared pose 148 before 154. The final build
  covers that ownership-qualified pose. Final 120-setting probe: jump without
  Grab, airborne catch, climbing, descent and lower-floor exit. All 286
  samples retain full first person, including 144 Pole samples. No-Grab
  jumping did not acquire an attachment. Earlier pre-fix circle/second-chain
  captures are retained as diagnostic history, not final acceptance evidence.

- Final-build Third Person action comparison at the 60 FPS setting: mount,
  save/load, pause, climb, both exits and return to global First Person.
  There are 671 samples, 635 resolved Pole states, none effectively first
  person. The final short Ctrl tap did not drop in this comparison; the
  first-person route is the explicit-drop evidence.
- Final-build live menu at the 60 FPS setting: global P toggle, cancel with
  zero writes, CONFIRM to Third Person and back to First Person, both applied
  live while attached. Menu layout and native arm captures inspected.

Private scripts, copied historical saves, settings, traces and captures are
under `recomp/analysis/d35f/`. Sources are private D08J5 and D08J3 copies;
the player's writable settings, saves and cards were not used or changed.
Original media was read-only. All diagnostic sessions are stopped. Rate settings are not cadence benchmarks;
dummy audio is not an audible-sound test.

## Acceptance and coverage limits

The user accepted the candidate and authorized closeout. Their acceptance
does not enumerate additional playtest routes. The tested
objects are two chain types; a separate solid pole and confirmed climbable
lamppost were not located. A nearby type-613 pole-family object was inventoried
but not traversed or visually identified. No whole-campaign/every-costume
claim is made. Natural no-floor bottom descent was not reached; its existing
native regression and the live explicit drop cover separate aspects.

A save loaded directly into an attachment-cleared 108/9/3 transient has no
previous host Pole provenance; normal fall/landing reacquires the view.


```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Use P for global First Person if needed. CHAIN/POLE VIEW defaults to First
Person. E mounts/catches, W/S climbs, A/D circles, mouse looks and Ctrl/Space
lets go as before. Try both menu choices and CONFIRM. Gameplay captures
automatically; F10 is not required.

## Closeout

Authored implementation, regression tests, manual and job records are included
in the authorized MusicMonsterMod commit/push. Runtime patch re-export was
unchanged; application to the clean pinned framework index reproduced all 51
changed/new files. No pins changed. No post-acceptance gameplay edits, builds
or launches; no player data touched. No next job started.
