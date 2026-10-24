# D35C - Jetpack perspective option

2026-10-24. Done. User: "lock it in, close it out".

## Behavior

OPTIONS > ADVANCED > ACTION PERSPECTIVE offers JETPACK VIEW:
FIRST PERSON (default) or THIRD PERSON. It applies live on CONFIRM and persists
independently of the Modern / Classic / EDuke32 flight scheme. Modern controls,
Independent camera and global First Person are required for the eye view.
Global Third Person wins. Leaving flight follows the current global view.

The D35A resolver applies this preference only to the resolved Jetpack action.
The brief pack-cleared flight pose retains that action; subsequent fall/landing
uses the existing global-view bridges. Swimming, mantles, scripted cameras and
other actions retain their own policies. The resolved policy and blend already
travel in the render snapshot, so replay never reads a newer live preference.

No flight, fuel, root motion, collision, platform-hover, sound or mirror-flame
implementation changed. D11F's accepted eye and transition support are reused.
The existing presentation-only transition bridges remain gated by global first
person; this option does not grant movement, aim, fire or capture permissions.

Profile schema 42 adds controls.jetpack_view (first or legacy-third). Migration
adds first while retaining other preferences and the existing recovery backup.
The launcher supports --jetpack-view; it saves the preference and supplies
DNTTK_JETPACK_VIEW. Vanilla receives legacy-third and retains its original
behavior. Menu draft/cancel/confirm and atomic persistence use the existing
settings path. Camera side-file merges retain the independent action choice.

## Verification

- Incremental player build passed; movie and geometry shards current.
- Native controls suite passed with the clean owned executable/LEVEL00 fixture,
  LEVEL01 and all 21 selectable overlays. Added all eight flight poses across
  the three scheme settings, live choices, global-third precedence, world reset,
  replay restoration, pack-cleared flight, fall/landing return and unchanged
  position/velocity/fuel/pack fields. Existing D11E/D11F, platform sweep,
  mirror drawing, ownership and Vanilla checks pass.
- Native input, near-clipping and separate EDuke32 fuel/toggle suites pass.
- All 92 Python profile tests pass, including schema-41 migration, backup,
  enum rejection, CLI launch gates, menu persistence and camera side-file merge.
- Offscreen OpenGL menu check passed Cancel without a write, draft reset,
  Confirm, first/third switching during flight, flight save/load under the
  current choice and saved Third Person on relaunch with global First Person.
  Menu and third-person flight captures inspected.

The two-level third-person flight matrix passed in TIME TO KILL and DUKE HILL
for Modern and EDuke32 at configured 120 FPS and Classic at 60 FPS. It records
1,066 checkpoints, including 775 flight checkpoints with the third-person
target throughout. Routes include start, takeoff, movement, mouse look, firing,
holster/draw, descent and off/fall. Small nonzero blend values at transitions
are expected easing, not failures. The menu run exercises both perspectives
live; the native fixture covers both across all schemes and flight poses.

The Modern platform-to-street descent route passed 138 active flight samples,
including release below the stale cached platform height. The pack and native
sound loop stayed active until J-off. Mirror capture shows reflected flames
with Third Person selected. Dummy audio verifies the native sound handle only,
not audible quality. These are the known D11F regression routes, not exhaustive
moving-floor or collision coverage.

A final EDuke32 60 FPS route passed global-view toggles during flight,
pause/resume, private flight save/load, firing, and native fuel exhaustion
followed by fall/landing back in the selected global First Person view.
All diagnostic game processes are stopped. Final diff whitespace check passed.

Private diagnostics, settings and copies of earlier private save fixtures are
under recomp/analysis/d35c/. No player settings, saves or memory cards were
changed. No original media, generated C, framework source or submodule pins
were edited; pre-existing framework worktree changes are preserved.

Accepted executable SHA-256:
`9265a3f7b044df83eb47ba3de49e310993eabae3358892f8b24da772389f4053`.

The user accepted the result and explicitly authorized closeout. Their report
does not enumerate routes. Natural water/contact interruptions and exhaustive
campaign, weapon and costume coverage remain unverified in this job. Configured frame rates
are functional coverage, not monitor-cadence or performance certification.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Select global First Person with P if needed. Set JETPACK VIEW in OPTIONS >
ADVANCED and CONFIRM; try both choices while flying. Gameplay captures
automatically. J starts/stops Modern and EDuke32 flight; Classic retains its
J-arm/Space-lift behavior. The view choice does not change those controls.

## Acceptance and closeout

Authored implementation, regression tests, manual and supporting job records
are included in the authorized commit/push as MusicMonsterMod. Runtime patch
re-export was unchanged; applying it to the clean pinned framework index
reproduced all 51 modified/new files. No submodule pin changed. No further
gameplay edits, rebuilds or launches. No player data touched. No next job started.
