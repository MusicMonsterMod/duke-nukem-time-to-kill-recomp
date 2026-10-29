# D14F - 90 default and 120 maximum first-person world FOV

2026-10-29. Implementation candidate; live visual validation belongs to the
coordinated [renderer batch](316-fov-renderer-quality.md). No user acceptance,
commit or push implied.

## Convention and defaults

The prior default was 64, range 60-90. The existing convention is retained:
horizontal degrees at 4:3, using H = round(160 / tan(degrees * pi / 360)).
The native 512x240 view uses approximately 1.6 horizontal matrix scaling;
its square-pixel half-extents are 160x120. Widescreen adds horizontal columns
and preserves the vertical angle, rather than treating the requested number
as a vertical or actual widescreen horizontal angle. See accepted D14A note 296.

| Requested | Integer H | Effective 4:3 horizontal | Vertical | 16:9 horizontal (margin 85) |
| --- | --- | --- | --- | --- |
| 64 (previous default) | 256 | 64.011 | 50.230 | 79.556 |
| 90 (new default) | 160 | 90.000 | 73.740 | 106.206 |
| 120 (new maximum) | 92 | 120.202 | 105.048 | 133.303 |

Integer quantization is existing behavior. There is no cross-game equivalence
claim. Third-person projection and Vanilla remain separately owned.

## Changes

- C++ conversion/validation, direct launch fallback and console default use 90;
  range is 60-120. The older raw-H diagnostic interval remains unchanged.
- Gameplay slider defaults to 90, range 60-120 in one-degree steps. Confirm,
  cancel, live application and sidecar persistence retain existing behavior.
- Python profiles use 90 for fresh, absent or invalid FOV values. Every explicit
  valid saved value, including 64, is preserved. Existing version migrations
  already fill additive missing fields from defaults; no schema bump or rewrite
  of player preferences is needed. Sidecar validation accepts the expanded range.
- Manual and console/header descriptions agree. `fov default` now selects 90;
  `fov 64` remains available for the historical projection. Launcher Vanilla
  exclusion remains at its existing value and scoped hooks remain excluded.
- The existing Interaction-owner proximity-fade correction now derives its
  lower H bound from the supported maximum (92), retaining the upper H<256
  condition. This is the same authenticated root-cutoff fade path documented
  by D14A, not a new arm pose or visibility bypass. Exact native subtractive
  command/tpage matching, intentional-transparency exclusions and draw-scope
  restoration remain intact. Contact arms keep world depth.
- Weapon/item, connected combat arm, leg and kick fixed display projection stays
  H=256. Native matrices, aim, collision and save format are unchanged.

## Validation

Python profile suite: 111 tests pass. Expanded cases cover missing/invalid
values, explicit 60/64/75/90/91/120 migration, saved preferences and 120 sidecar
round trips; existing Vanilla launch exclusion passes.

Native controls tests extended for a fresh unset 90 launch in an isolated
child; previous fixture tests explicitly retain 64. Real guarded projection
cases include 60/64/75/90/105/120, malformed/nonfinite/out-of-range rejection,
unchanged native camera field and replay snapshot ownership. Quick-kick
pose invariance adds 105/120 across both costume maps. Interaction fade cases
add H=123/92 and below-supported H=91 with all existing effect exclusions.
The native controls executable builds and passes with owned executable, clean
LEVEL00 fixture, LEVEL01 and all-level overlay fixtures (exit 0).

## Limits

These are settings and hook-contract checks, not evidence that every scene
renders correctly at 120. The batch separately owns near clipping, sky and
save restoration investigations. Actual menu interaction, weapon motion and
64/90/120 scene comparisons at multiple aspects must be recorded by that
coordinated validation before claiming visual acceptance. Player saves and
settings were not modified by this subtask; no game was launched here.
