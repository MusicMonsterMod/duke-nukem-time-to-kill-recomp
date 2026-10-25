# D35J1-J3 and D35M - First-person idle, inventory and use actions

## Accepted closeout - 2026-10-25

The user accepted the delivered combined result and explicitly authorized
closeout ("lock it all in and close us out"). D35J bounded research and
D35J1-J3/D35M/D35N are Done. The evidence below records the earlier candidate
and its coverage limits; pending-acceptance wording describes that delivery,
not the current status. No broader damage/hazard or campaign acceptance is
implied. The final combined executable is
`78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.


2026-10-25. Needs playtest. User authorized implementing all reproduced cases
from [the D35J investigation](233-d35j-transient-perspective-research.md).
No acceptance closeout, commit or push is authorized by this candidate.

## Result

First person stays active through native idle gestures and held native
inventory. The reproduced NPC, lever, key/door and crystal actions retain
first person by default. USE INTERACTION VIEW under Advanced Action Perspective
also offers the existing Third Person action presentation. Global Third Person
wins. Ordinary use timing, alignment, callbacks, item consumption and weapon
return remain native; no movement, firing or interaction lease is added.

The shared setting covers the verified ordinary-camera selector family, not
all possible scripted events. The reproduced NPC pose belongs to this family,
so it shares the setting with the verified door/lever/item actions. Mirror
interactions and unrelated scripted cameras retain their separate scope.

Candidate executable:
`7c490764e5fd86316d1ef833e59467243a54489e31d72f6e3e0530cddea7d148`.

## Implementation

- `modern_controls.cpp:state` admits native idle lower poses 64-69 and the
  verified use poses 118/266/277 only for first-person presentation in mode
  0/history 0. Use poses also require native committed-action flag 0x40 and
  the First Person setting. Movement/action checks keep their previous gates.
- Held native inventory gets a presentation-only 0x200 exception for lower idle
  63 in mode/history 0/0. Death, script, identity and reciprocal camera-owner
  checks remain. The existing water exception is unchanged.
- `action_perspective.h` identifies Use as a presentation owner, carried by the
  existing render snapshot. `first_person.inc` keeps an upright eye over the
  actor for gestures/use and preserves native interaction arm transforms and
  the existing near-plane handling. Native root travel still moves the eye.
- Profile schema 49 adds `use_view`, first by default, migrating older settings
  without resetting other preferences. The live menu and launcher
  `--use-view first|legacy-third` persist it. Vanilla launches force the dormant
  legacy value; its gameplay does not acquire these first-person permissions.

No new hook, generated-C edit, native script/animation rewrite or framework
change was needed. Pinned dependencies and the existing dirty framework tree
are preserved. The stationary Walk-button idle-timer behavior discovered in
research is unchanged; use Caps Lock run mode to reproduce natural fidgets.

## Verification

Player build passes. Native controls, input, aim and near-clip suites pass.
All 99 profile tests pass, including new migration, validation, CLI, live-menu
sidecar persistence and Vanilla-isolation coverage. Native controls tests cover
all six fidget poses, the three use poses and held inventory, plus unrelated
pose rejection, movement isolation, selected view, action choice, pause, load,
render-state restoration, death/script denial and foreign camera ownership.

Local diagnostic scripts, private saves, JSON and screenshots are under
`recomp/analysis/d35j-fix/`. The test inputs are copies of the D35J research
fixtures, not the player's writable cards. A fresh manifest records current
player slots 8-11 for the non-modification check.

| Route | Observed result |
| --- | --- |
| Slot 10 NPC, slot 8 key/door, slot 11 lever, slot 9 crystal | At 60 and 120 presentation settings, every sampled action frame retains blend 1. Native use poses are reached, actions finish, and weapons return. |
| Key/crystal consumption | Native item flags change from found (1) to used (0x4000) in the private inventories. NPC and lever routes leave mission inventories unchanged. |
| Holstered idle | Natural poses 64/65/66 observed at both rates; all retain blend 1. Movement cancels/recoveries return to normal locomotion. |
| Armed idle | Native sequence holsters the weapon and reaches fidgets 68/65; first person remains active throughout. |
| Held Scroll Lock inventory | Native selection UI stays visible over first person; hold and release retain blend 1. W during the hold does not move Duke in X/Z. |
| Action Third Person option | All four use routes return to third person for their committed actions and recover the selected global first-person view afterward. |
| Global Third Person | All four use routes remain third person. |
| Live menu | Cancel produces no settings write. Confirm applies Third Person, and a subsequent NPC action uses it. Confirming First Person restores the eye path immediately. |
| Pause and load during NPC use | Pause/resume retains the eye. A private mid-action save loaded with immediate sampling shows lower 277 at blend 1 before input capture, then capture and native completion without losing first person. |
| Vanilla | Private mid-action save runs lower 277 through native completion to 63 at blend 0. Original third-person Duke/head are visible; no eye takeover. |

The first Vanilla fresh-activation attempt used short injected native pad
pulses and did not activate the NPC; it is not counted as a passing interaction
entry test. The verified Vanilla route loads the genuine native action save.
The initial candidate 60 FPS stdout and runtime log shared a filename; its
JSON and screenshots are intact, and subsequent runs use distinct summary
logs. Live reads are sequential rather than atomic frame snapshots, as in the
research; isolated boundary rows can pair a new pose with the prior camera
result. No performance benchmark is claimed. Timeshift was observed running
during the validation session; background activity alone does not establish
its impact on these tests.

## Limits and player acceptance

User playtest is still required for eye height, hand framing, comfort and the
complete familiar routes. Natural live fidgets 67 and 69 were not observed;
those selector variants have native/unit coverage only. Additional NPC/door
callback variants, full-campaign use interactions, hurt/knockdown/water hazards
and mirror interactions are not claimed fixed. This is the bounded set
reproduced in note 233, plus the known native idle selector family.

Play with First Person selected (P toggles the global view), verify idle and
slots 8-11, and hold/release Scroll Lock for native inventory. Gameplay captures
automatically. USE INTERACTION VIEW defaults to First Person after migration;
its Third Person choice should restore the familiar action presentation.

Launch from the workspace:

```sh
python3 recomp/tools/local/run.py
```

All private diagnostics are stopped at delivery. Source player save hashes and
mtimes are unchanged. No player settings/cards were used by diagnostic runs.
No acceptance, commit, push or next job is implied by this candidate.
