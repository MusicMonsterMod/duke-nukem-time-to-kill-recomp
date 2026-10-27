# D35B - Swimming perspective option

2026-10-24. Done. User fully accepted D35B and explicitly requested closeout.

## Scope

D35's source audit and roadmap are complete as a research deliverable. Its
Needs review status records pending review, not missing D35B prerequisites.
Unresolved event traces belong to D35J-N and campaign coverage to D22. The user
explicitly selected D35B if the research was complete. D35A and D11E are Done.

## Behavior and implementation

OPTIONS > ADVANCED > ACTION PERSPECTIVE now offers SWIMMING VIEW:
FIRST PERSON (default) or THIRD PERSON. The user requested the shorter
Third Person label; the stored legacy-third value remains compatible.
It is independent of SWIMMING
pace. The option applies with global first person, Modern controls and
Independent camera. Global third person wins. Leaving swimming follows the
current global view, including a view-key change made during swimming.

The existing D35A resolver supplies action ownership. Only its Swimming action
receives the new choice; a lingering water flag does not assign it to a mantle,
fall or another action. D11E's eye and weapon presentation are reused. Legacy
uses the accepted Modernized swim orbit, not Legacy controls. Water climb-outs
retain their existing third-person mantle policy; first-person mantles are D35G.

The D11E first-person-only equip-bit presentation exception is conditional on
the swimming choice too. Legacy keeps the existing third-person guard. No
movement, aiming, fire, water, oxygen or physics permission is widened.
The existing render snapshot already carries the resolved policy and blend;
workers do not re-read a live menu preference while replaying an older frame.

The menu uses the established draft/cancel/CONFIRM/live/atomic-write path.
Profile schema 41 adds controls.swim_view, validated as first or legacy-third.
Older profiles receive first while retaining their global view, swimming pace
and other preferences, with the existing recovery backup. The launcher's
--swim-view option persists the choice and passes DNTTK_SWIM_VIEW. Vanilla
receives legacy-third and retains its existing camera/control behavior.
Camera side-file merges preserve the independent action preference.

## Verification

- Final incremental player build passed; native geometry/movie shards current.
- Native controls suite passed with the owned executable, clean LEVEL00
  fixture, LEVEL01 and all 21 level overlays. New cases cover both swimming
  paces, live action preference, global-third precedence, land return after
  changing global view, world reset and replay snapshot restoration. The legacy
  equip-bit test confirms the FP-only presentation exception stays disabled.
- Existing D11E surface/underwater eye, costume, weapon, pause, water-exit and
  no-root/water/velocity-write checks passed. Existing Vanilla, ownership,
  traversal and first-person regression checks passed.
- Native input suite passed. All 91 Python profile tests passed, including
  schema-40 migration, backup, enum rejection, launch/profile gates, save and
  reload, menu side-file absorption and camera side-file merge independence.
- Private offscreen OpenGL menu run at configured 120 FPS verified Cancel
  without a write, draft reset, Confirm, live surface/underwater changes,
  load under the current preference, and saved Third Person on relaunch
  with global First Person. Presented menu and underwater captures inspected.
- Private two-level matrix passed 64 sampled checkpoints in levels 2 and 6:
  both swim views with both pace modes, configured 60/120 FPS, 4:3/16:9,
  surface/dive/resurface, holster (equip 0), redraw (equip 2), moving fire,
  loading underwater and global view changes. Presented captures inspected.
  This is option/regression coverage, not a new natural water-entry/exit route.
- Final diff whitespace check passed. All diagnostic game processes stopped.

Gameplay-tested executable SHA-256:
`96cd2fa12d72530c5722c7dffc5343bce066d188c1cca9b23ce9601026c8886c`.

Private scripts, captures, profiles and save copies are under
recomp/analysis/d35b/. Cards were copied from earlier private D11E diagnostics.
No player's cards, saves or settings were used or changed. No generated game C,
framework source, original media or submodule pins were edited. Existing
framework worktree modifications are preserved.

## Accepted result and coverage limits

The user fully accepted the option after the shorter Third Person label was
rebuilt. Their acceptance did not enumerate routes; the automated evidence
above remains bounded. Native fixtures establish policy boundaries, not a new
full-campaign or natural mantle playthrough. Wider costume/weapon/room and
renderer coverage remains bounded by D11E's accepted evidence.

Launch with the player's saved settings:

```sh
python3 recomp/tools/local/run.py
```

Select first person (P by default), then try both SWIMMING VIEW choices in
OPTIONS > ADVANCED. CONFIRM saves/applies; Escape cancels an unconfirmed draft.
Gameplay captures automatically; F10 has no action. No next job is started.

## Display label revision

User requested Third Person without Legacy. Menu label and current player
documentation updated; stored enum and behavior unchanged. Incremental player
build and diff whitespace check passed. Gameplay checks above precede this
text-only revision. Current executable SHA-256: `37b2f270bb73c026ac094963adf283e82083f565cba536d31d71130d1d7558b5`.


## Acceptance and closeout

User: "great work. fully accept, close it out". D35B is Done, including the
requested First Person / Third Person labels. Implementation, regression tests,
manual, board and supporting documentation are included in authorized closeout
as MusicMonsterMod. The accepted executable is the display-label revision above.
Runtime patch re-export was unchanged; applying it to the clean pinned framework
reproduced all 51 modified/new files. No submodule pin changed. No post-acceptance
gameplay edits, rebuilds or launches. No player data touched. No next job started.
