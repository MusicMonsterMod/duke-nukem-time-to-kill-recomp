# D17M1 - Ladder/platform depth

2026-10-22. Done, user-accepted; closeout authorized.

## Diagnosis

The updated UI slot 10 private copy has SHA-256
`1b709040b1614b4abd6418021cf7bcea99801bf121a611154ff4ee004ce2c9e5`.
Offscreen primitive traces identify plain ladder instance `0x801d9fe4`, type
46, mesh `0x8011a4d8`, with ten vertices and four textured panels. Its local
bounds are 378 x 2046 x 0. One panel participates in host depth while three
remain native, including the upper panels that paint through the platform.
World mesh `0x8018c8d8` already has three D17M platform-depth faces. The
candidate trace moves the three native ladder panels into depth-only drawing;
the existing near panel retains its path. These addresses identify evidence,
not conditions in the correction.

## Correction

`near_clip.cpp` reuses the authenticated static-object caller, instance/type
bounds and the game's plain-ladder type flags (0x200 present, 0x100/0x400
absent), as used by the existing climbing code. In Modernized Corrected
textures with whole-polygon rendering, safe opaque textured ladder faces
within 8192 view units gain depth testing. Their original ordering slot,
quad diagonal, positions, UVs, lighting and far rejection are retained.
No compact-prop grouping, extra subdivision or contact offset is applied.
Existing whole-mesh packet-budget fallback remains in effect.

Original textures, Vanilla, unrelated scenery, other climb families and
translucent panels keep their existing paths. No movement/control, framework,
generated source, save-format or player-settings changes. Developer comparison:
`DNTTK_LADDER_DEPTH=0`; the D17M platform comparison also gates this extension.

## Verification

Player build succeeds. Owned-EXE `ttk-near-test` passes with Original textures,
Corrected textures, Corrected geometry/textures, ladder correction disabled,
and original world subdivision forced. Added contracts exercise plain ladder
classification, source ordering, unsplit vertices, the 8192-unit boundary,
transparency and foreign-caller exclusions, and Vanilla gating. Primitive-trace
observation/gating tests pass in Original and Corrected textures (two each).

Offscreen camera sweeps and short lateral/forward/back movement cover both
views at 60/120/180. Hidden ladder panels are occluded in inspected captures;
exposed rails/rungs remain visible. First-person 120/180 stationary samples
measure 118.5/180.0 distinct submissions/s, third-person 117.8/179.8, with no
dummy-audio underruns. The 60 mode has no replay distinct counter, so its
zero counter is not a frame-rate measurement. These are bounded offscreen
checks, not monitor cadence or listening acceptance.

Six older scenes (table, shelf, closet, apartment, blood, subway) show no
obvious new rendering defect in inspected captures; short samples range from
117.9 to 120.0 submissions/s. Vanilla reports zero host-depth packets and
zero replay workers. The 180-target comparison on the same ladder route is
180.2/s disabled versus 180.0/s enabled, both with zero dummy-audio underruns.
The earlier accepted D17M ammo state was also swept at 120; inspected
below-platform views retain ammo occlusion. This run does not re-establish
the earlier above-platform/traversal acceptance.
Scripted approaches reach the ladder contact but do not complete ascent or
descent; they are not automated traversal acceptance. Subsequent user gameplay
acceptance closes the job without expanding the automated evidence.

All diagnostic runs use private cards, saves and profiles under
`recomp/analysis/d17m1-ladder/` (and existing private D17M regression states).
Player saves/cards/preferences were not used as writable test locations.

Accepted executable SHA-256:
`1381185d45a6b0a68b5f8529fe6f4e812e5130c645a9a313319cb0d37186559f`.

## Player check

Load updated UI slot 10 with Corrected textures. Look slightly upward beneath
the platform, shift left/right and forward/back, then climb up and down.
Check that exposed rungs stay visible and the ammo remains hidden beneath the
platform but visible from above. Try first and third person. F10 captures the
mouse if needed. No new option or binding.

```sh
python3 recomp/tools/local/run.py
```

## Acceptance and closeout

2026-10-22: user confirmed "it's fixed!! accepted. lock it in, and close it out".
D17M1 is Done on user gameplay acceptance. No further implementation changes,
rebuild or additional automated gameplay tests were needed. The earlier
scripted traversal limitation remains recorded above. Framework patch export
is unchanged and applies cleanly to the pinned upstream, matching all 50
patched files. Implementation, tests and documentation are included in the
authorized commit/push as MusicMonsterMod. No next job started.
