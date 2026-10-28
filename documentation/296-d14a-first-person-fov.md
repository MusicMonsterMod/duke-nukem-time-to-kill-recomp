# D14A - Configurable first-person FOV foundation

2026-10-28. User playtest passed; D14A accepted and Done. D19X is not selected.

## Result and scope

Modernized has a session-only `fov [60..90|default]` developer console command
and `DNTTK_FP_FOV` launch override. These change the actual guarded GTE
projection, not framebuffer stretching. Default 64 maps to the existing H=256
exactly. The original camera projection/eye-placement field is not rewritten.
The existing perspective resolver still owns first-person eligibility, scripted
camera fallback, global Third Person and action transitions. Vanilla is excluded
by the existing Modernized hook gate.

The console command and atomic runtime setter are in `first_person.inc`;
`level_select.cpp` routes the command. `first_person_fov.h` owns conversion and
validation. Replay snapshots carry the chosen integer projection, so workers
use their frame's value rather than a fork-time setting. No framework files,
retail assets, native animation events, game saves or save formats were edited.

## Measured convention

**S:** native view bounds are 512x240, centered at zero. The game camera matrix
scales the horizontal axis by approximately 1.6 for presentation at 4:3.
**L:** baseline GTE records in four private save scenes show H=256, zero OFX/OFY,
and horizontal matrix norms near 1.6 versus approximately 1.0 vertically.
Small departures reflect native fixed-point rotation/scale rounding. These
records support a nominal rectilinear convention; they are not an assertion
that each individual quantized pixel has an exact analytical angle.

Use **horizontal degrees at 4:3**. Half-extents in square-pixel camera units are
160 horizontally and 120 vertically:

- `H = round(160 / tan(degrees * pi / 360))`.
- Effective 4:3 horizontal angle: `2 * atan(160 / H)`.
- Vertical angle: `2 * atan(120 / H)`.
- Wide horizontal angle: `2 * atan((256 + native_margin) / (1.6 * H))`.

Angles in the last three expressions must be converted to degrees. Widescreen
adds real columns and retains the vertical angle; native margin rounding is
included in the reported wide angle. This replaces reliance on the historical
unmeasured "about 64 degrees" comment.

| Requested | H | Effective 4:3 horizontal | Vertical | 16:9 horizontal, margin 85 |
| --- | --- | --- | --- | --- |
| 60 | 277 | 60.023 | 46.846 | 75.150 |
| 64 / default | 256 | 64.011 | 50.230 | 79.556 |
| 75 | 209 | 74.872 | 59.726 | 91.120 |
| 90 | 160 | 90.000 | 73.740 | 106.206 |

At the observed 21:9 margin of 192, the default is nominally 95.128 degrees
horizontally and the 90 setting is 120.510 degrees. Integer H quantization is
intentional; a requested degree need not equal the effective angle exactly.

## Projection-dependent native-arm fade

**L:** at H=160, wheel and ladder contact limbs disappeared, while held items
and combat arms remained visible. Native mesh traces show the interaction
polygons were still emitted, with the original subtractive transparency flag.
**S:** the root joint's H cutoff precedes `0x80034e18`, which enables proximity
fade below depth 1024. Lowering H admits a root previously rejected at the
baseline projection, enabling that fade for subsequent native joints.

For an already eligible Interaction owner with actual GTE H in [160,256), the
existing per-joint proximity-fade correction is used. It recognizes only the
exact native command/tpage pair, uses Duke's environment brightness, preserves
intentional actor transparency flags 0x40/0x2000/0x4000 and restores state at
scope boundaries. Matrices, native animation, world clipping and occlusion
remain with the Interaction owner. Default H=256 keeps the accepted policy.
This is a D14A projection compatibility correction, not a redesign of D36 poses
or a move of interaction arms into the weapon overlay.

**L:** matched wheel and ladder captures regain connected limbs at 90 after
the correction. Card/crystal held phases, use transitions and vending also
retain their original gameplay path in the private fixtures.

## D19X configuration contract

- Proposed saved key: `controls.first_person_fov`, numeric degrees at 4:3.
  No key/schema migration or player-menu row is added by D14A.
- Default: 64. Range recommended from this bounded candidate: 60-90, step 1.
  Wider/narrower ranges are not validated by this work.
- `first_person_set_fov(double)` returns false on nonfinite/out-of-range values,
  retaining the previous setting. It is safe for the event-pump caller; the
  emulation thread reads an atomic integer projection.
- `first_person_fov_setting()` returns the effective quantized 4:3 angle.
  D19X should retain/serialize the requested slider value, not infer slider
  position by truncating this diagnostic effective angle.
- `fov default` explicitly selects H=256, including after a launch override.
  `fov` alone reports the effective value without changing it.
- `DNTTK_FP_FOV` is parsed once on first use. Missing/invalid numeric values
  resolve to the original default. If absent, the older developer-only
  `DNTTK_FP_PROJECTION=160..386` remains available for historical diagnostics;
  its wider interval is not a validated player range. An explicitly supplied
  degree override takes precedence over that legacy override.
- Runtime values survive view switches and save loads for the process lifetime.
  They do not persist to disk. D19X should load the validated saved value before
  the first gameplay frame; missing/invalid old-profile values should use 64.
- Projection changes appear on subsequent gameplay draws and their replay
  frames. In-flight/history frames can still show the previous value briefly.
  Each worker gets the projection with its snapshot; a worker refork is not
  required. D19X must preserve confirm/cancel and Defaults behavior.
- `ttk_input.controls.fp.fov` reports target H, effective nominal horizontal and
  vertical angles, wide horizontal angle and native margin. `fp.projection`
  is the last applied projection (or the original camera field outside FP).

Candidate executable SHA-256:
`45148c23fde7ead1a6084c26a61f7d121ff0d773f552833a6d94e6333de6ee6f`.

## Verification and evidence limits

**Automated:** player build; native controls, input, aiming and near-clip suites
pass. New controls cases cover exact default, conversion, malformed/nonfinite
values, scoped H changes, unchanged camera field, third person/Vanilla/other
caller exclusion, replay snapshot versus later live value, and interaction fade
correction/restoration with intentional-transparency exclusions. Controls were
rerun after the fade correction; the other tested modules were unchanged.

**L:** private copies of current saves and D36 audit saves were used, with
hash/mtime checks preserving player files. Local evidence lives under ignored
`recomp/analysis/d14a/`; scripts, raw JSON, native and composed PNGs remain local.

- 60 Hz, 16:9: four scenes at default/60/75/90. Wider values reveal additional
  scenery and shrink projected world features; HUD retains its scale.
- 120 setting, 21:9: ten saved scenes at 60/90; additional settled comparisons
  at default/60/90 in three scenes. Initial heavy-trace captures sometimes
  preceded the changed presented image. They are not all valid matched pairs.
  A longer 16:9 comparison captures both native and presented images at 60/90
  and confirms the change reaches the replay presentation.
- The first three replay passes recorded 1,560, 905 and 332 surface-restoration
  checks, respectively, with zero mismatches and no failed worker jobs. This
  is replay correctness evidence, not a claim of sustained 120 FPS; tracing,
  readback and debug requests substantially disturbed pacing.
- The corrected candidate's final 120-setting run adds 644 restoration checks,
  zero mismatches and no failed worker jobs. Ladder 186 is verified in state
  and presented captures with restored arms. That run's wheel attempt remained
  in ground locomotion, so it does not establish a 120-setting wheel interaction;
  the corrected wheel route is verified at 60.
- 4:3 weapon samples identify held records 4-12 (including dynamite), holstered
  state, pistol firing, crouch, active quick kick, look changes and jetpack.
  Some scripted inputs did not produce their intended action; the attempted
  push and initial scripted view-toggle labels are not acceptance evidence.
- A separate Xvfb run using real P key events verifies third-person H=386 and
  return to the selected first-person H. Pause/resume and repeated loads retain
  the selected target. Native tests separately verify non-game render callers.
- D36 private fixtures verify wheel 270, vending 277, ladder 186, held crystal
  11/upper 47 then insertion 267, and held card 6/upper 47 then use 266, followed
  by weapon return. Both 60 and 90 were captured. The failed push attempts did
  not establish push/pull coverage.

**S:** aim uses the unchanged camera-forward center ray and physical-muzzle
queries; FOV changes neither. Existing near clipping reads actual GTE H, and
widescreen portal bounds continue to describe the presented rectangle. Native
aim regression tests pass; the live firing samples are not exhaustive weapon
hit-accuracy or portal-transition certification. The Bio Mask keeps its existing
fixed screen-space projection, appropriate for face-mounted equipment.

**Remaining playtest:** subjective framing and comfort, all weapons/upgrades and
costumes through full animations, push/pull, free swimming and additional
traversal/portal routes, adaptive resize, and whole-campaign culling coverage.
Menus/FMVs retain the existing scoped projection path; no complete FMV replay or
Vanilla campaign was repeated. No user acceptance, commit or push is implied.

## Acceptance and closeout

The user confirms the new first-person FOV playtest passes and accepts the work
on 2026-10-28, with explicit commit/push authorization. The automated controls
(all-level fixtures), input, aim and near-clip suites pass again at closeout.
The evidence limits above describe the bounded automated/private coverage;
the user acceptance does not claim exhaustive campaign certification.
D14B owns the separately reported weapon/arms framing follow-up; D19X owns the
saved menu slider. The reported background continuation is not started or
modified by this closeout.
