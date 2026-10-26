# D08A29 - Weapon selection while running

2026-10-26. Done, user-accepted. User: "otherwise this is solid work. lock it
in and close it out". Implementation and documentation commit/push authorized.
No new build or gameplay test during closeout; the evidence and scripted limits
below are retained. D08A30 separately queues the requested inverse wheel mapping
(up = next, down = previous); it remains Todo with no implementation.

## Cause and bounded change

U: wheel weapon changes required releasing run.
L: the private baseline accepted pistol and RPG wheel requests while running,
but rejected shotgun, Gatling, Energy Weapon, Flamethrower, Buffalo Rifle and
Freezer requests. Their upper body entered 18/19 instead of the stationary
20/29 ready loops. The wheel command reached the input snapshot but was consumed
by the ready-state gate without a selection.

S: the owned executable's upper dispatcher table at 0x80014da8 maps 18 and 19
to 0x8004ee84. That handler explicitly accepts the existing flag-4 weapon
request and selects the native stow track. Its gait branch selects 18 for
lower 76/84 and 19 for 78/85. The completion table at 0x800153e0 maps their
completion to 0x800508d0/d8, alternating 18 and 19. These are ready loops,
not firing or reload animations.

`shortcuts.inc` now includes 18/19 in its ready predicate, both for recording
the equipped weapon and for accepting a selection. Native stow/draw, movement,
events and weapon request bytes remain the mechanism. A 468-byte identity guard
covers the verified handler. Existing ownership, movement, fire, interaction,
original-aim, action flags and unsafe upper-state checks remain.
No input buffering, forced animation change, synthesized fire, framework edit
or generated-C edit was introduced. Modernized only; Vanilla is unchanged.

## Verification

- Incremental player build passed. Executable SHA-256:
  `01dfa5416a79dc5858c5665786216010b6abc4b2824f15085c88d592636a2233`.
- Native controls suite passed with owned EXE, clean LEVEL00 fixture, LEVEL01
  and all selectable level overlays. New regressions cover both run-ready
  halves, exact previous/next selection across base and upgraded records,
  number-key selection, unchanged lower/upper pose, held-fire rejection and
  no deferred request after release. Draw, reload, attack and charged-throw
  states remain rejected. Existing remote/history, swimming, Vanilla and
  identity/actor/caller coverage also passed.
- Native SDL input and weapon-aim suites passed, including wheel accumulation,
  Alt-wheel separation, menu input, RUN MODE and aim safety.
- L, 60 setting: private offscreen wheel-down checks passed for all six
  affected families, with and without `dnupgrade`. Running lower 76/78
  continued through successful native changes where the route had room.
- L, 60 setting: private Xvfb wheel-up checks covered Shift and actual Caps
  Lock RUN MODE, first/third view, forward/strafe/diagonal inputs. Alt-wheel
  changed preferred radius 1536 -> 1344 without changing weapon. A wheel notch
  while Gatling fire was held was rejected and did not switch after release.
- L, 120 setting: private third-person running, jetpack (lower 165), and
  swimming (133 -> 128) Gatling-to-shotgun wheel changes passed.
- Player settings/cards were not used as writable targets. Runs used private
  profile/save copies under ignored `recomp/analysis/d08a29/`; all stopped.

## Limits and review

The slot-8 route obstructs some backward/right/diagonal movement. In lower
94/95 wall-stop recovery, the existing movement gate still rejects a notch;
this is not an eligible running loop. Do not widen that gate based on this job.
Those samples do not prove continuous movement in every direction on open ground.
The initial 120-setting Xvfb/old-save harness failed to establish its inventory
and was not counted as a gameplay pass. The working Xvfb checks used 60, 1x.
These are behavior checks, not performance measurements or visual acceptance.

The 120-setting remote attempt reached equipment 1 / selected 12 but upper 42,
so the existing unsafe-state gate rejected the notch. Successful live remote
switch-out was not established; the existing native ready-remote regression
passed. Preserve that distinction rather than claiming complete live coverage.

Retained regression route: run with shotgun, Gatling, Energy Weapon, Flamethrower, Buffalo
Rifle and Freezer; scroll both ways without releasing Shift, then repeat with
RUN MODE. Check number keys and last-weapon history, upgraded weapons, safe
remote transitions and both views on an open route. Rapid notches during an
active stow/draw or attack remain intentionally discarded, not deferred.

Launch from the workspace: `python3 recomp/tools/local/run.py`.
