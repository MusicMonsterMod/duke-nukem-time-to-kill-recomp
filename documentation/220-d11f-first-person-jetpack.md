# D11F - First-person jetpack flight

2026-10-24. Accepted and closed out. User: "lock it in and accept! close it out!"

Accepted executable SHA-256:
`85d6969593b96e0dc0b8358e310daca0b83e31e0f12ee95780bcf191042a3fd9`.
The implementation and documentation are authorized for commit/push as
MusicMonsterMod. No post-acceptance gameplay changes or launches. The sections
below retain the development history and distinguish earlier candidate evidence
from the final slot-12 refinement. User acceptance supersedes their pending
playtest/authorization statements; recorded coverage limits still apply.
Runtime patch re-exported unchanged and verified against the pinned clean
framework during closeout.

## Behavior and implementation

Modernized with Independent camera retains the selected first-person view
through jetpack takeoff, flight, firing and landing. The shared camera path
supports Modern, Classic and EDuke32 flight. The initial camera change left
physics and controls unchanged; the accepted floor/platform and mirror
refinements are described in the follow-up sections below.

`first_person.inc` removes the explicit jetpack fallback. A private baseline
survey measured the flight neck about 198 units above the body root across
lift, hover and directional poses. The eye is centered over the root at 294
units (198 plus the existing 96-unit eye offset). It follows body travel
immediately, eases the posture offset, and does not inherit flight lean or
land walking bob/glide. Switching between the land pivot and flight root
rebases the relative offset, as the existing swimming camera does.

The normal camera solver still handles room/geometry and obstruction. Existing
head hiding, weapon display and near clipping remain in use. No new hook,
generated code edit, framework change or persistent setting is required.

Live testing exposed three native transition frames previously outside camera
presentation: mode 10 with the pack already off but the flight pose still
present, fall animation 108 in mode 9 with previous mode 10, and landing
animation 105 with previous mode 10. `state()` now admits
these for selected first-person presentation only. Gameplay/action eligibility,
third-person policy, identity, camera ownership, actor/death and script-inhibit
checks retain their existing guards.

Climbing, mantling and pushing retain their existing third-person policy.
Scripted/turret cameras and death retain the original camera.

## Verification

Player build and native controls, input, aiming and near-clipping suites pass.
The controls suite includes the clean owned EXE/LEVEL00 fixture, LEVEL01 and
all 21 selectable overlay identities. New cases exercise all eight flight
animations, two costume layouts, all three scheme settings, deliberately large
neck-pose changes, direct flight acquisition, suspended input, pack-off and
landing transients, death, Vanilla exclusion, third-person selection and the
ground handoff. Camera-only checks retain the original root, velocity and pack
fields. Existing swimming, ground weapon and traversal assertions pass.

All live testing uses offscreen OpenGL, dummy audio, private settings and a
copy of the earlier private D08S fixtures under `recomp/analysis/d11f/`, port
9421. Player saves/cards/settings and original media are not written. Configured
FPS is not a physical-monitor cadence or performance certification.

The first exploratory run found the landing/off transition gaps. Its second
level did not receive the repeated diagnostic inventory cheat, so that level
is not flight evidence. The corrected harness resets the cheat token between
levels and records actual mode, animation, equipment, aim and view state.
Classic checks hold native thrust during flight sections because releasing
thrust otherwise lands Duke before the firing checks.

### Two-level flight matrix

`final-{modern,classic,eduke32}-120.json` records TIME TO KILL (level 0)
and DUKE HILL (level 1), using their original level travel and inventory
cheat on private data. All 1,070 samples retain first-person blend 1,
including sampled ground/takeoff, flight, turning/pitched look, directional
travel, firing, bound-key holster/draw, descent, landing and pack-off/fall states.
There are 255 Modern, 194 Classic and 258 EDuke32 flight samples. Both levels
have confirmed airborne adapted shots with a visible crosshair. Holstering
reaches equipment 0 and an unobstructed eye view; drawing restores the weapon.
Selected presented hover, fire, holster and landing captures were inspected.

The matrix ran on `5ffb71e1...`, immediately before the final fuel-out
mode-9/previous-10 presentation addition. The final executable received the
full native controls suite and the transition runs below; the entire matrix
is not represented as a new run on the final hash.

These are bounded routes, not full weapon/damage/costume coverage. Classic's
held-thrust route climbs above the skyline; its capture includes a horizontal
sky/background gap. This run does not establish whether that high-altitude
background behavior differs from the existing camera/rendering baseline.

### Final transition runs

`transitions-modern-60.json` and `transitions-eduke32-60.json` are the final
executable's short flight-only transition routes. They assert mode 10 before
saving and after loading. Both check the bound view toggle to third and back,
pause/resume, a private flight save/load, firing and native fuel-out/fall/landing.
The only intended blend changes are the explicit view toggles. God mode holds
fuel during setup; it is disabled before setting a small private fuel quantity
and observing the original exhaustion path.

Earlier `edges-*` attempts are exploratory evidence: the first stayed paused
because the harness used the gameplay Escape path to resume; a corrected run
found the mode-9/previous-10 fuel transition. Later long runs landed before
some labeled flight checks and are not counted as airborne save/load evidence.
During those later runs the host reported substantial I/O pressure (about
48% avg60, with no CPU or memory pressure), and the runtime logged slow frames.
No Timeshift/rsync process was identified in the immediate process check; the
cause of the I/O pressure was not established. These runs are functional
checks, not timing/performance evidence.

The weapon selection labels in exploratory `edges-*` filenames are not an
inventory oracle: recorded held slots were 7 (Gatling) and 6 (rifle), including
the file labeled crossbow. Those captures are not crossbow coverage. Pistol
flight/fire is covered in both levels by the matrix. Animated thrown/melee
weapons, all upgrades, natural death and exhaustive ceiling/near-wall routes
remain wider playtest coverage. Native near-clipping and death/ownership
checks pass; no clipping implementation was changed.

## Candidate

Executable SHA-256:
`9912e67e4ba820de6b620bf6727c7990125c5c8578261e8b21e98050e76c2438`.

User camera-feel acceptance remains required. Broader campaign/costume and
weapon coverage is not established by a two-level route. No commit, push or
acceptance closeout is authorized by this implementation request.

Launch using saved preferences, then press P if needed:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Capture is automatic. J starts/stops Modern and EDuke32; Space ascends and
Ctrl descends. Classic retains J-arm/Space-lift and its original burst physics.


## Follow-up: keep the pack running at ground level; mirror flames

The user found the camera 99.8% right, but reported the original ground/armed
state requiring Space to restart and missing mirror flames. The earlier
candidate's unchanged-physics statement applies only to that initial build.

Modern/EDuke32 now bound Ctrl's downward step against native floor/foot
clearance, targeting a low 64-unit hover. This avoids the native landing
transition that leaves the pack armed without flight; release retains that
height instead of rising to the normal 512-unit approach height. Native
collision still handles walls, ceilings, water and unexpected contacts.
This is a low hover, not a grounded walking state. J stops the pack; fuel-out,
water and native interruptions remain valid reasons for flight to end.
Classic retains its original ground/Space behavior and Vanilla is untouched.

Native actor drawing requires both 0x82000000 bits at player+0x224 for flames.
The primary draw consumes 0x02000000 before the mirror draw (caller 0x8003816c).
The post-flight hook now supplies both bits, and each verified running-Duke
draw re-arms them. The draw path uses identity/ownership checks without calling
the stateful camera lease evaluator in render replay. Other actors and Vanilla
are excluded. No generated code or framework source was edited.

Evidence is local under `recomp/analysis/d11f-r2/`:

- `baseline-mirror.json`: Ctrl ended in mode 0, pack 3, sound handle zero.
- `fixed-mirror-hover.png` under `shots/`: visible reflected flames after
  restoring the per-draw flag. Final capture is `final-mirror-hover.png`.
- `floor-modern-60.json` and `floor-eduke32-120.json`: 123 and 125 samples.
  Held Ctrl for four seconds, release, strafe/return near the floor, Space
  climb, repeated descent and J-off. During checked flight: mode 10, pack 3,
  nonzero sound handle. J-off returned to mode 0, pack 1 and zero sound.
  Sound continuity is verified through the native loop handle with dummy
  audio, not a listening test. This route uses private copies of club saves.
- Player build and native controls, EDuke32 61-second fuel/toggle/safety,
  input, aiming and near-clipping suites pass. Added floor/hold/climb and
  mirror redraw/foreign-actor/Vanilla assertions. The hook-only EDuke32
  fixture now supplies airborne clearance before testing Ctrl cancellation;
  it does not integrate the native lift itself.

Final follow-up executable SHA-256:
`46a93ab9f8eef1a500275f4013a3bd59b98efba9484b25dc9bd8e0151a9fe769`.

Floor routes cover this flat club floor, not every slope, moving floor or
campaign collision. The initial flight matrix and transition runs above were
on the earlier candidates. User playtest of the ground feel remains required.
No acceptance closeout, commit or push performed.


## Slot 12 platform follow-up

The user reported that descending onto an alley platform still stopped the
sound and left Space able to restart, and that Ctrl could not descend below
the platform height after flying away. The earlier flat-floor workaround was
insufficient. Native +0x1c8 is cached collision state, not a fresh floor query:
the slot-12 route retained -13312 even while the actor flew over lower surfaces.

The cached-height descent clamp is replaced by an isolated call to the original
flight sweep at 0x8007a98c, whose body is already covered by control identity
guards. It queries the proposed horizontal position and downward step, restores
all actor scratch/position changes and uses only an actual floor hit. A
160-unit margin covers changing animated feet between this query and native
integration. A blocked descent uses a tiny -2 root so native horizontal motion
continues and the later foot update does not convert the hover into a landing.
Release drift is checked too. Manual Space/Ctrl input now retains the newly
chosen hover height, avoiding a pull toward the old cached platform on release.
Classic/Vanilla gates and native water, fuel-out and J-off behavior remain.

Private evidence: `recomp/analysis/d11f-r3/`, using copies of the player's slot
12 (file slot11) and private settings/cards. No player save was modified.
The first 64-unit sweep trial passed the platform but still landed on a lower
surface after crossing an edge; it is not a passing candidate. With the larger
margin, both Modern 60 FPS and EDuke32 120 FPS platform-to-street routes kept
mode 10, pack 3 and the same nonzero native sound handle until J-off. The old
floor cache remained -13312 while Duke descended to approximately -9990 root
Y over the street (actual floor -9216). Sound was checked through the loop
handle with dummy audio, not by listening.

The diagnostic resume setup temporarily sends uncaptured menu input and can
land the saved flight before steering is enabled. Setup restores flight before
the measured platform descent; those resume samples are not passing flight
coverage. Release-height checks and final mirror evidence are listed below.
Native regression tests now cover a newly encountered floor with stale distant
cache, a cleared sweep with stale high cache, release-height retention, and
restoration of collision query scratch. Both full controls and EDuke32
fuel/toggle/safety suites pass. No framework or generated-code edits.

Final candidate SHA-256:
`85d6969593b96e0dc0b8358e310daca0b83e31e0f12ee95780bcf191042a3fd9`.

This fixes the reproduced route; exhaustive campaign/moving-floor coverage
remains unverified. User playtest is still required. No commit, push or
acceptance closeout.

Final-binary evidence: `release-eduke32-120.json` passed 138 flight samples,
including a release below the old platform with only 2 units of root-height
variation, then continued descent and J-off. `final-mirror.json` and
`shots/final-mirror-hover.png` confirm reflected flames and running flight after
floor descent in Modern. The earlier `final-modern-60.json` and
`final-eduke32-120.json` (131 checked flight samples each) used the same sweep
with the release-height refinement not yet applied. Native regression tests
include that refinement. All diagnostic sessions have stopped.
