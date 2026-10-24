# D35A - Shared action-perspective foundation

2026-10-24. Accepted after user regression playtest; D35A is Done.

## Implementation

`recomp/src/ttk/action_perspective.h` defines a pure presentation resolver with
an action category, selected view, capability, effective target and reason.
The verified normal-camera hook supplies existing action predicates. Hard
identity/owner/world/orbit denials remain ahead of eye acquisition; the resolver
records those denials without granting any action lease. `state()` and movement,
aiming and firing permissions retain their existing guards.

The existing push, traversal and early water-exit fallback formula is preserved.
Committed rolls/slides and unowned falls keep their exceptions; swimming and
jetpack remain supported. Categories intentionally reflect today's broad
presentation rules, not unverified door/mirror or climb-subtype recognition.
The blend remains separate from its effective target. Diagnostics expose
`fp.action` and `fp.effective` alongside requested/supported/blend/reason.

The resolved value contains no pointers and is saved/loaded in `RenderState`.
World reset clears transient policy and retains the current global preference.
The existing kick presentation capability check reads this same resolved value.
No new hooks, generated-code edits or framework changes are required.

## Settings contract

Future implemented action fields use flat `<action>_view` controls with `first`
or `legacy-third`, default `first`. Global third person always wins; a legacy
choice selects the Modernized orbit, not Legacy controls. Unsupported actions
remain unsupported regardless of choice. No action field or inert menu row is
registered in this foundation job.

Python provides shared string-enum validation and action-value validation.
The menu's `view_row` constructs both global and future action rows using the
existing draft, live-apply and atomic settings writer. Global VIEW now uses that
helper with unchanged choices/default/order. Schema 40 remains valid: there are
no new stored fields to migrate. Action jobs must extend defaults/migrations,
launcher environment plumbing and side-file coverage when they register keys.
The launcher description now correctly states swimming/jetpack FP support.

## Verification

- Incremental player build passed.
- Native controls suite passed with the owned executable, existing private
  LEVEL00 guard fixture and all 21 extracted level overlays. Added exhaustive
  comparison of 512 fact combinations against the previous support/reason rule,
  denial precedence and enum validation.
- Added replay policy restoration assertions and a water-exit fallback test
  where switching global view survives exit and world reset. Existing D26B
  ownership/pause/reset, D11E swim and D11F jetpack fixtures passed, alongside
  controls/identity/Vanilla regressions.
- Native input suite passed.
- Python player-profile suite passed, including migration, malformed settings,
  menu save/cancel, atomic persistence and side-file merges. Added action-enum
  validation and corrected launcher-description coverage.
- Private offscreen 120 FPS gameplay verified first/third switching and save/load
  recovery. The real OPTIONS menu verified cancel without a write, confirm
  with one write, third person after resume, and atomic profile merging. The
  paused replay retains its previously resolved frame until resume; diagnostics
  report that frame's policy, not an unrendered menu draft. Relaunch with the
  saved profile retained third person when loading the first-person test save.
  Diagnostic scripts/logs are local under `recomp/analysis/d35a/`;
  cards were copied from an earlier private test directory, not player cards.

Accepted build SHA-256:
`d0710eb9081889a13856dc12a203cccc27a4fd09b169ed1bb264a420e4db8fc3`.

These checks do not establish full campaign fidelity or new action support.
The user confirmed unchanged behavior and explicitly requested closeout.
The report is acceptance of this foundation, not exhaustive campaign coverage.
Authorized closeout includes implementation, tests and supporting documentation
as MusicMonsterMod. Framework patch re-export was unchanged; applying it to
the pinned clean dependency reproduced all 51 modified/new framework files.
No further gameplay changes, rebuilds or launches.

Launch with saved settings:
`python3 recomp/tools/local/run.py`.
