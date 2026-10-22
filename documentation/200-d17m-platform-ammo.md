# D17M - Ammo occlusion beneath thin platforms

2026-10-22. Done, fully accepted by the user. Accepted ammo reproduction is the earlier UI slot
10, not the historical slot 9. The private state SHA-256 is
`2a2881e919bd72c2dee33d19c8950da06e74639719a132625af8a5a013760841`.
All diagnostic runs use offscreen rendering and private profile/card/state
copies under `recomp/analysis/d17m-platform/`.

## Cause and correction

The ammo is visible through the alley platform at 60 FPS as well as high
refresh. The baseline primitive trace identifies pickup instance `0x800de4e0`,
mesh `0x8012249c`, and the platform underside record `0x8018cbf8` in world mesh
`0x8018c8d8`. These are diagnostic identities, never repair conditions.

The object renderer takes nearby props into the host depth path out to 3072
view units; ordinary world takeover ends at 1536. The underside lies between
those boundaries and stays native, without depth writes. Pickup pieces are
ordered separately, with the existing foreground prop offset, so some arrive
after the intervening platform. A captured GP0 stream includes the platform
at sequence 386324 and pickup pieces both before and after it. This is a
mixed native/host occlusion gap, not a high-refresh-only visibility defect.

`near_clip.cpp` now gives the underside of a thin world platform depth in that
gap. Eligibility requires the existing authenticated world caller/context,
Modernized near rendering, precise host vertices and Corrected textures with
whole-polygon rendering enabled. The platform search extends to 8192 view units to include the third-person
camera setback, without extending ordinary world takeover. Every corner must
be safe; the face must be
horizontal and above the eye, with matching upper corners at a common 128 or
256 units above it. No mesh, pickup, room, overlay or slot ID selects a repair.

At third-person distance, the ammo mesh itself was also native: its local
extents (298x156x265) narrowly exceed the existing 256-unit compact-prop
limit. The compact-depth limit is now 320 only for walk-over pickups, using
the original type table's `0x20000000` flag at the authenticated static caller.
Instance/table pointers and the 1062-type range are checked. Ordinary scenery
keeps its 256-unit limit, and actor/viewmodel paths keep their existing rules.

Eligible faces retain their source maximum-SZ ordering slot, far rejection,
original corners, selected position/texture precision, lighting and native
triangle/quad diagonal. They gain depth testing without extra subdivision or
contact offsets. Whole-mesh budget fallback remains in place. Original texture
subdivision, Vanilla, walls, slopes, floors below the eye and ordinary room
ceilings without matching thin-slab corners keep their previous paths.

Developer comparison: `DNTTK_PLATFORM_DEPTH=0`. This is not a player option.
The same comparison switch also disables the pickup extent extension.
No framework or generated source changes, save-format changes or new hooks.

## Rejected alternatives

Giving all nearby world faces depth hid the ammo but reduced the measured
180-target table scene from about 177 to 119 distinct submissions/s. Limiting
that to overhead horizontal surfaces still cost too much. Neither version is
shipped. The final matching-corner slab restriction restores the measured rate.

Removing the global foreground prop bias also hid the ammo at one angle, but
it remained visible at another. That experiment is not shipped; existing prop
bias/contact behavior is preserved.

## Validation

The player builds. Owned-EXE `ttk-near-test` passes with Original textures,
Corrected textures, Corrected geometry plus textures, the correction disabled,
and original world subdivision forced. New packet tests cover triangles and
quads, source ordering, original corners/UVs, 128/256-unit thickness, mismatched
upper corners, slopes, thick slabs, below-eye faces, range/far rejection,
foreign callers, budget exhaustion and Vanilla exclusion. They also verify
the pickup flag/extent gates and that searching farther for platforms does
not expand ordinary takeover for unrelated wide world polygons. Existing renderer
contracts remain in the same suite. The primitive-trace observation/gating
suite passes with both Original and Corrected textures (two tests each).

Final offscreen captures cover below-platform camera sweeps and short forward/
backward movement at 60/120/180 in first person and at 120 in third person.
The ammo is occluded in the inspected views, including the third-person angle
that defeated the earlier candidate. The private elevated jetpack capture
shows the unobstructed ammo still visible on the platform. Six older scenes
(table, shelf, closet, apartment, blood and subway) were checked at 120;
inspected captures show no obvious new rendering defect. Their short samples
range from 118.9 to 120.1 distinct submissions/s. Vanilla reports zero host
depth packets and zero replay workers. These checks do not replace traversal
acceptance. All 40 recorded player save/card/profile files retain their intake
hashes and modification times.

The final 180-target offscreen comparison uses the same private profiles and
states, four seconds of warmup and six-second stationary samples:

| Scene | Correction off | Final thin-platform correction | Audio underruns off/on |
| --- | --- | --- | --- |
| Old table scene | 178.8 distinct submissions/s | 178.6/s | 0 / 0 |
| Current slot-10 platform | 179.8/s | 180.3/s | 0 / 0 |

These are bounded offscreen submissions and dummy-audio counters, not monitor
cadence or listening acceptance. They do not establish a locked rate everywhere.
The broader rejected candidates and their captures remain in the local lab.

The requested player acceptance covered looking beneath the platform, climbing up and
down, and checking that the ammo remains visible when unobstructed. The scripted
ladder attempt reached attachment but did not complete the ascent/descent;
do not count it as traversal acceptance. Diagnostic coordinate writes snapped
back to the floor and are not above-platform evidence. A private jetpack route
provides an actual elevated view of the pickup on the platform instead.

Accepted executable SHA-256:
`8a034f815fc3abf27c27f843dfcc27e01444b4a5e383fa177fe7370ed98412da`.

Launch from the workspace:

```sh
python3 recomp/tools/local/run.py
```

Load UI slot 10, use the saved Corrected texture setting, and press F10 if
mouse capture is needed. No new activation step. D19Q2 separately tracks the
user's recurring paused-save report; that job has not been implemented here.

## Acceptance and closeout

2026-10-22: user explicitly fully accepted the fix and authorized closeout,
commit and push. The automated traversal limitation above remains part of the
historical evidence; user acceptance closes D17M. No new build required.
Framework patch re-export is unchanged and applies to the clean pinned
upstream, matching all 50 patched files. D17G/H/I/J manual triage closeouts and
D19Q2 paused-save backlog are included in this authorized commit.

The subsequently updated slot 10 shows a separate upper-ladder/platform issue,
tracked as D17M1 Todo after brief private-copy inspection. It does not reopen
the accepted ammo correction. See the job board for its save identity and scope.
