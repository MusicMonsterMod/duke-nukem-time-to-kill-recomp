# D35N - Mirror interaction and reflection opacity

## Accepted closeout - 2026-10-25

The user accepted the delivered combined result and explicitly authorized
closeout ("lock it all in and close us out"). D35J bounded research and
D35J1-J3/D35M/D35N are Done. The evidence below records the earlier candidate
and its coverage limits; pending-acceptance wording describes that delivery,
not the current status. No broader damage/hazard or campaign acceptance is
implied. The final combined executable is
`78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.


2026-10-25. Candidate extension to the D35J/M work, requested by the user.
Current UI slot 12 supplies the mirror interaction; slot 7 supplies the
close-range reflection fade. Prior idle/use changes were reported solid so far.
No acceptance closeout, commit or push was requested.

## Cause and implementation

Slot 12's E interaction runs lower/upper pose 278 with mode/history 0/0 and
player +0x224 bit 0x40. Normal camera ownership remains intact. The old
first-person eligibility rejects this full-body action, causing the switch.
The candidate admits precisely this presentation state without granting
movement or action input. It uses the upright root-relative eye and native
interaction arms; the original action, quote, timing and weapon return run.
Script/death/identity/camera-owner guards remain in force.

MIRROR VIEW independently chooses First Person or Third Person in the action
perspective menu. It defaults to First Person, with global Third Person taking
precedence. Schema 50 adds `mirror_view`; `--mirror-view first|legacy-third`
saves it. Vanilla forces the legacy setting and bypasses Modernized hooks.

The native actor renderer applies proximity translucency below camera depth
0x400 (branch at 0x80034e18). Slot 7's reflected Duke reaches that path.
Function tracing confirms actor draw 0x800348d8 with player a1 and return
0x8003816c, using the reflected stack camera rather than the primary camera.
The draw-local mirror flag now permits the existing fade-clear helper to
remove that specific distance-fade command and restore normal actor brightness.
Each joint, actor boundary and mirror list return restores the saved render
context/GTE colors; world/load and replay installation reset draw-local state.
The primary third-person body policy is unchanged. Other actors, intentional
translucency flags and power-up timers are excluded. Existing full-head
reflection and jetpack flame handling remain intact.

No new hooks, generated-C edits or framework edits were required. The existing
guarded actor draw and mesh entry points supply the scope.

## Evidence

Private evidence is under `recomp/analysis/d35n/` (local, not tracked):

- Baseline slot 12 repeatedly reaches pose 278 with first-person blend 0;
  candidate first60/first120 runs hold blend 1 throughout repeated uses,
  recover to idle and redraw the weapon.
- Slot 7 baseline and candidate screenshots show transparent versus opaque
  reflected Duke. Candidate walks backward and forward without restoring the
  unwanted fade. Reflected head and arms are visible during slot 12 use.
- Native controls tests cover mirror preference independent of use preference,
  global view, pause/load/replay, action locks and rejection guards. Opacity
  tests cover render/GTE restoration, nested list calls, next-actor cleanup,
  caller/actor isolation, intentional effects and Vanilla.
- Player build and controls/input/aim/near suites pass; all 100 profile tests
  pass, including migration, validation, persistence and launcher behavior.
- Live MIRROR VIEW menu cancellation/confirmation, switching back to First
  Person, pausing mid-action and reloading a private mid-action save pass.
- Global Third Person retains blend 0 throughout the same mirror routes.
  Vanilla completes a private saved pose-278 action and returns to idle with
  blend 0. This is an action-load check, not a fresh Vanilla interaction test.

Diagnostic saves/cards/settings are private copies. The player's corrected
geometry/texture settings were copied for representative visual checks;
an earlier diagnostic profile with original precision produced misleading
slot 7 texture artifacts and was replaced before the comparison.
Source slot 7/12 hashes and modification times are unchanged; private test
sessions are stopped.

## Review boundary

Needs playtest. Check slot 12 with E, repeat after weapon return, and check
slot 7 at the mirror and after backing away. User comfort/framing and other
mirror locations remain player acceptance work. This is not campaign-wide
acceptance or a new live jetpack/power-up survey.

Candidate SHA-256:
`78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.

Launch with the player's saved preferences:
`python3 recomp/tools/local/run.py`.
