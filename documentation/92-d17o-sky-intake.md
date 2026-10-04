# D17O - sky rendering and camera-relative replay repair

2026-10-04. **Done, user-accepted.** User: "amazing work!!! accepted. well done.
document. commit. push". Preserve implementation `25734d1`. This acceptance
closeout adds no new gameplay test, build or launch. The recorded technical
limits remain; no next job is selected. The historical intake and initial
investigation plan follow this report.

## Finding and bounded result

The original sky is camera-relative geometry. High-refresh object interpolation
mistakenly treated three sky matrix loads as extra transforms of the last world
object. It replaced the freshly computed camera-relative matrices with older
world-space matrices. The late presentation camera could then be hundreds of
units away from the substituted sky origin. The small cloud bands visibly
shifted, changed shape or left the view during camera motion.

The repair excludes exactly those three authenticated sky calls from world
object transform recording/substitution. The original sky routine still builds
its matrices on every guest composition and worker redraw. It keeps its own
geometry, textures, rotations, timer, clipping and ordering. No replacement
skybox, frozen animation, world-rendering bypass, save edit or jetpack change.

This establishes a concrete redraw defect and removes its measured displacement.
It does not establish that every part of the user's description was caused by
this one defect. The user subsequently accepted the reported sky appearance; broader coverage
limits below remain.

## Reproduction and identities

- Owned US executable SHA-256:
  `b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a`.
- UI slot 10, file `openbios/state_800AB6FC_slot09.pst`, SHA-256:
  `2307f0f6c5bfe7fd976f66e3cbef3fb72c3a25b05e6dfa9252d1414a5dadb73d`.
- Immutable intake: `recomp/analysis/d17o-sky-intake-20261004/cards-intake`.
  Work copy: `recomp/analysis/d17o-sky-work/cards`. The original 28-file intake
  manifest was checked before testing. The accepted executable is preserved
  locally as `recomp/analysis/d17o-sky-work/baseline`.
- This exact save resumes in jetpack flight. Ground tests descend with Ctrl
  until normal locomotion and the requested first-person view return. Merely
  selecting first-person at launch does not mean the flight view is first-person.
- Main visual captures: Modernized, independent camera, view aiming, 16:9,
  OpenGL 4x, CPU 100%, bob on, original accepted clipping/depth defaults,
  60/120/180 targets. Tests use private preferences and debug port 9317.
  Hardware offscreen logs identify NVIDIA OpenGL 580.178.04.
- Vanilla was loaded separately as an original-camera control. It does not
  accept the modern synthetic look route, so its screenshot is not a matched
  first-person camera-angle comparison. Modernized 60 provides the comparison
  with the same modern camera and no intermediate-frame redraws.

All code studied here is resident below the level-overlay region. The audit
compares the active RAM sky routine with the owned executable, not just a static
address label. Modernized controls also retained their existing first-map code
identity checks. No new level-overlay hook or generated C edit is involved.

## How the original sky works

### Composition and coordinate spaces

The main composition at `0x80026164` renders the world using `0x8002e48c`.
That routine computes the eye in camera `0x800d6eb0` at offsets `+0x14/+0x18/
+0x1c`. Later in composition, the ordinary path calls `0x8001fc44(0)` at
`0x800265bc`, followed by the sky routine `0x800388e4` at `0x800265c4`.
The routine ends at `0x80038e58`.

The model pointer table is at the pointer stored in `0x800ddb18`. The current
scene selects these layers:

| Selector | Model | Geometry and placement |
| --- | --- | --- |
| `0x800da464` | `0x61b` | Six-vertex, four-G3-triangle backdrop. Horizontal camera direction is normalized (`0x800b3d3c`) and converted to angles (`0x80066f88`). Its matrix follows camera yaw and translates to the current eye. |
| `0x800da344` | zero | Optional fixed-orientation eye-relative mesh path, inactive in this snapshot. Uses `0x80029660` directly. |
| `0x800da45c` | `0x619` | 108 vertices, 72 textured quads. Rotates using the sky timer shifted right by 5. Y scale is 2560/4096; X/Z scale is 4096/4096. Translates to the current eye. |
| `0x800da46c` | `0x61a` | Same 108-vertex geometry with a separate polygon/UV list. Rotates using the timer shifted right by 3, without the slow layer's vertical flattening. Translates to the current eye. |

The matrices are built on the guest stack using `0x8002afe4`. The slow layer
also calls `0x800b4b5c` for scaling. Each of the three active layers then loads
its view transform through `0x800292a0(camera, matrix)`:

| Call instruction | Return address | Layer |
| --- | --- | --- |
| `0x80038c60` | `0x80038c68` | Camera-facing backdrop |
| `0x80038d74` | `0x80038d7c` | Slow cloud band |
| `0x80038e10` | `0x80038e18` | Fast cloud band |

Their matrix translation is deliberately equal to the eye. Consequently there
is no walking-induced translation parallax for these sky meshes. Camera rotation
and the independent layer rotations produce their changing appearance.
Interpolating their eye translations as independent world positions violates
that contract. Interpolating the backdrop's rotation independently of the late
camera also violates its camera-facing construction.

### Animation, textures and asset provenance

`0x800c0c64` is the sky phase counter. The sky routine adds game delta
`0x800d21fc` when the pause/menu globals `0x800be568` and `0x800d2540` are zero.
The cloud angles are integer shifts of this counter; the fast angle progresses
four times as quickly as the slow angle. The routine clears the generic texture
animation pointer at render context `+0x60`. The observed motion is mesh rotation
with authored UVs, not proof of a scrolling-UV skybox. Original game-frame and
integer-angle quantization remain. Worker increments occur in the isolated
render snapshot and do not advance live game simulation.

The read-only tool `recomp/tools/local/audit_sky.py` authenticates the EXE and
active routine, reads scene selectors and verifies whole vertex-array matches
against `/D0/DB00/DB00MOD.BIN;1` from the owned disc. That asset has 187212 bytes
and SHA-256 `c74020b25cb16efd6991b07c8e583636c130abfe8a02b5790e13b26d4123fa93`.

- Both cloud vertex arrays match offsets `0x2afc0` and `0x2b7d0` in that file.
  They are identical geometry, so vertex bytes alone cannot distinguish these
  two source placements. Their SHA-256 is
  `f124f00ff94b4b85602b03a9ef0624804803e365553fc55dc0cc98fffdc797fb`.
  Bounds are X/Z -474..474 and Y -250..-83, before the slow layer's scaling.
- The backdrop matches offset `0x2bfe0`; SHA-256
  `5d48ababbd3b439cb880091dfad4a47578056b16686700f82e811882a70de7af`.
  Bounds are X -416..416, Y -448..-33, Z 383..622.
- The cloud polygon lists contain FT4 records with authored UV/CLUT/tpage words
  and semi-transparency. The backdrop uses vertex-colored G3 records.
  Texture-page names and all campaign sky assets have not been catalogued.

These are narrow curved bands and a backdrop, not a complete enclosing cube.
Black overhead regions also occur in the 60 FPS comparison. This repair does
not extend the artwork or claim to correct every original high-pitch limitation.

### Projection, clipping and ordering

All three active meshes use the original object renderer `0x80010000` (returns
`0x80038c94`, `0x80038da8`, `0x80038e44`), with the usual context `0x800d67a8`,
OT `0x800d27a0` and occupancy bitmap `0x800d26a0`. `0x80033dd8` selects vertex
scratch storage. The original renderer projects with RTPT, computes AVSZ3 depth,
applies polygon bias and inserts into the OT. The sky routine enables the
context's rectangle clipping bit `0x80` and uses the root bounds at `0x800d22a0`.

`0x8001fc44` closes the prior screen-mode chain through `0x8001f9c4` /
`0x8002bb80`, then switches to the sky's mode-0 chain. Thus these small local
mesh coordinates are not an instruction to put the sky in front of nearby world
geometry. The existing chain composition provides the background layering.
This change does not rewrite the OT, depth buffer, root rectangles, UVs or
near-clip takeover rules. Captured horizon/building occlusion is retained.

## Verified cause and before/after evidence

`frame_replay.cpp::actor_draw` sets `xf_obj` and resets the transform sequence
when a world object begins drawing. The old code did not retire that identity
when the object returned. `transform_load` therefore recorded the subsequent
sky stack matrices as more transforms of the last object. For the traced slot,
that object was `0x801d7c84`; the three sky calls acquired sequences 1/2/3.
`append_xf` blended them with that object's other transforms. A worker then
substituted these matrices even though the original sky routine had just built
correct matrices for its newer camera.

Example from the baseline trace:

```
return 80038c68, object 801d7c84, sequence 1
original current translation / current eye: 19542,-11959,10512
substituted old translation:                18894,-11373,10548
```

The same bad translation was substituted for both cloud layers. This is a
camera-centering error, not an animation-state or jetpack ownership error.

In the bounded yaw/pitch sweep, 360 logged displaced baseline calls reached a
1075.397-unit Euclidean offset (mean 523.644 in this deliberately displaced
sample). The cloud mesh radius is roughly 474. After the fix, all 360 logged
native sky calls had exactly `offset=0,0,0`. The phase advanced from 100055 to
100390 during the candidate trace. These are sampled diagnostics, not a
whole-campaign statistical estimate.

Local evidence under `recomp/analysis/d17-high-refresh`:

- `sky-sweep-base.log` / `sky-sweep-fixed.log`: transform evidence.
- `shots/sky-sweep-{base,fixed}-*-seq-*.png`: 24-present full-width sequences at
  yaw 0, 90, 180, -90 and pitch -15, -25, -35, including slow turning.
- `shots/sky-{base120,fixed120,fixed180}-*-seq-*.png`: flight, grounded first
  person and slow yaw sequences from the reported save.
- `shots/sky-{base60b,fixed60}-*-seq-*.png`: compositor captures at 60, where
  the replay provider is off. These are not high-refresh redraw captures.
- `sky-noclip-*`: original object-clipping comparison. Disabling near clipping
  did not remove the erroneous replay transform mechanism; no clipping change
  was justified by that comparison.
- `recomp/analysis/d17o-sky-work/{asset-audit,trace-summary}.json`: derived
  provenance and trace measurements. Raw RAM, original disassembly, captures
  and copied saves remain local and ignored by Git.

Presented replay dumps are stored bottom-up. Flip vertically for visual review;
the existing `sheet.py` does this. Ordinary compositor screenshots do not need
that flip. Dumping PNG sequences stalls the emulation thread, so these captures
are not frame-pacing or audio benchmarks.

## Implementation, boundaries and regression approach

`sky_render.h` recognizes only the primary camera and the three return addresses
above. `SkyRenderIdentity` authenticates all 1400 bytes of the resident routine
at `0x800388e4`, digest
`c78073edff91c270605ab870d7804078115604c2c49073eb79d359fd5322df13`.
Its existing-style identity memo invalidates on host frame or RAM-code generation
changes, including save restoration. Unknown callers/cameras or changed code
cannot acquire the new exemption.

The exemption applies before recording a live transform or substituting one in
a worker. It does not clear/skip world objects or globally turn interpolation
off. The existing high-refresh Modernized gate means Vanilla and the 60 FPS
path never enter it. `DNTTK_SKY_CAMERA=0` is a developer comparison restoring
the previous path. `DNTTK_SKY_TRACE=1` produces bounded diagnostic records;
neither is a saved player setting.

`ttk-near-test EXE` additionally checks the actual owned routine digest, all three
callers, wrong cameras/callers, altered code, generation invalidation and the
next-frame already-dirty-code case. Existing packet tests retain coverage of
world ordering, compact prop depth, translucent ordering and Vanilla exclusion.
The authored audit tool allows the geometry evidence to be reproduced without
committing any original data.

No framework source changed. The pinned submodules and accepted-source patch
remain at the accepted baseline; this title-source change needs no new runtime
patch export. No new generated hook, recompiler output or media file is committed.

## Final verification and remaining limits

| Check | Result and scope |
| --- | --- |
| Final local-dev build | Player linked; initialized geometry math and native movie shard checks current. |
| Native suites | Near/owned packet+sky identity, input, aiming, controls and inventory all exit 0. Final logs and return codes are in `analysis/d17o-sky-work/*-final.log` and `final-checks.json`. Controls include D08Q2 recovery; aiming/controls include D08Q3 flight eligibility. |
| Reported save at 60/120/180 | Ground first-person and flight fallback, stationary look-up, slow yaw and pitch changes captured. Separate third-person flight sweep covers four headings and multiple pitches. No worker failure in completed routes. |
| 120 FPS visual regression | Private file slots 9,0,2,3,4,8,11: turn left/right and walk forward/back. Full-width captures retain the sampled building, subway, club, ladder-platform and blood/prop contexts. 120.0-120.5 presents/s, 117.7-120.5 distinct/s; no sampled audio underruns or near-code guard refusals. These are bounded checks of D17C/D/E/F/K contexts, not renewed user acceptance of every old report. |
| 180 FPS support | Private file slots 9,2,3,4,11, same short routes: no worker failure. Most samples about 180 distinct/s. The busy club sample repeated frames and underrran audio; see comparison below. |
| Different sky location | Private file slot 3 at ground level, yaw/pitch view toward building roofs and traffic lights; full presented real/new/redraw sequence inspected for horizon/world occlusion. Same first map, not a second campaign sky asset. |
| Worker surface isolation | 1106 checks, zero mismatches, zero worker failures, at 1x with `PSX_REPLAY_COW_CHECK=1`, including slot 9 -> slot 3 restore. `sky-isolation.json`. |
| Modern/Classic flight regressions | Private Xvfb/SDL event runs at requested 120, 1x: loaded flight has aiming/reticle eligibility; moving pistol fire adds 5 Modern / 4 Classic adapted shots in the sampled segment; actual pack-off event, landing, ground firing, first-person return and re-takeoff pass. Flight restores third-person fallback. This pass does not repeat all six weapon damage routes. |
| Player data | All 28 player and immutable-intake hashes and mtimes still match the intake manifest. No player preferences were test destinations. |
| Existing framework delta | `apply_runtime_patches.py` reports the reviewed stack already applied. No framework edits made for D17O. |

The separate no-capture GPU timing route lasts eight seconds, using grounded
third person, slow yaw in both directions and walking. At 120, the previous
path and candidate each delivered 120.23 presents/s and 59.99 guest fields/s,
with zero audio-underrun increase. Worker averages were 4595 versus 4589 us.
Distinct images were 119.98 versus 117.36/s; short-run scheduler variance and
scene/input timing remain, so this is not evidence of a performance improvement
or a guarantee of every deadline. It does not show a material added worker cost.
Audio values are runtime telemetry with the dummy sink, not a listening test.

At 180, the multi-save busy club sample produced about 169.7 distinct/s and
12983 output-underrun samples. A fresh same-route comparison reproduced the
limitation with the old sky path: 172.7 distinct/s and 6748 underrun samples;
the candidate fresh run delivered 178.9 distinct/s and 1668 samples. Both had
zero failed workers. Do not call 180 FPS universally clean or attribute that
existing busy-scene throughput limit to this small sky exemption. D17 movement,
scheduling and audio follow-ups remain separate jobs.

Exploration failures retained as limitations of the measurement procedure:

- The first 60 FPS attempt used `replay_dump`, which cannot supply redraw
  sequences with the provider off. It timed out; final 60 captures use the
  full-width compositor screenshot command instead.
- One immediate-load 4x surface-isolation attempt timed out waiting for fields.
  The completed 1x check above is the surface-restoration evidence; it is not
  a 4x performance claim.
- Initial flight helper attempts used held synthetic E and wall-time waits.
  Those do not establish a completed pack toggle/landing or weapon recovery.
  Final transition runs use actual SDL key events and guest-field waits,
  including recovery after ground fire. Xvfb ran below real time and supplies
  functional evidence only. No GPU performance inference comes from Xvfb.

The candidate deliberately keeps original integer cloud-angle steps, original
textures and high-pitch geometry limits. There is no interpolation of a new
continuous sky phase and no attempt to fill all black overhead areas. Full
campaign sky selection, other disc revisions, split-screen, 240/Unlimited and
every combination of optional camera/render settings were not validated here.
The confirmed defect concerns high-refresh redraws; no new 60 FPS sky behavior
is claimed. D17L/M/N and broader campaign fidelity are not closed by this work.

Final player binary SHA-256:
`8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.

## Reproduce the audit and player check

After capturing a 2 MiB RAM dump from an isolated loaded state, run:

```sh
python3 recomp/tools/local/audit_sky.py \
  --ram recomp/analysis/d17o-sky-work/baseline-ram.bin \
  --output recomp/analysis/d17o-sky-work/asset-audit.json
recomp/build-local/ttk-near-test recomp/disc/SLUS_005.83
```

For developer A/B use the same private profile/cards/input route with
`DNTTK_SKY_CAMERA=0 DNTTK_SKY_TRACE=1`, then without the camera override.
Read `[sky-xf]` substituted origins versus `[sky-native] offset=0,0,0` and
inspect full presented sequences, not only one canonical guest screenshot.
The ignored `probe_sky.py`, `sweep.py`, `regress.py`, `timing.py`,
`flight_checks.py` and `isolation.py` in `analysis/d17o-sky-work` preserve the
local investigation routes; their imports reuse the existing D17 harness.

For the player, ordinary launch keeps saved preferences:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Load UI slot 10, look up and slowly turn in both directions, then vary pitch
and move. Compare grounded and flight views, keeping roof/horizon edges visible.
F10 captures Modernized input if needed. If the saved rate is 60, a deliberate
high-refresh comparison can use `--mode modernized --frame-rate 120`; these
flags update saved preferences. The user has accepted the reported sky appearance, so D17O is **Done**.
These steps remain the regression route for future changes. No next job is started.

# Historical intake and investigation plan

2026-10-04. Todo - next. User requested this backlog/handoff before clearing
context. No sky investigation, game launch, build or implementation in this
checkpoint. The statements below are the report and planned work, not findings.

## Report and reproduction

Load **UI save slot 10 (file 09)** and look up at the sky. The user reports
strange, glitchy movement rather than a neat parallax effect, and asks whether
its appearance can be stabilized. It is **not specific to the jetpack**; the
position in this save simply makes the issue easy to see. Do not scope this
as another jetpack fix or presume the game uses a conventional skybox.

A read-only intake copy of all current player cards/states is preserved under
`recomp/analysis/d17o-sky-intake-20261004/cards-intake`, with SHA-256, size and
mtime values in the adjacent `manifest.json`. UI 10 is:
`openbios/state_800AB6FC_slot09.pst`.

SHA-256: `2307f0f6c5bfe7fd976f66e3cbef3fb72c3a25b05e6dfa9252d1414a5dadb73d`.

Use another writable private copy for replays; keep this intake immutable.
Recheck save identity on resumption if the player has saved again. Earlier
D08Q2/D08Q3 slot-10 copies are historical regression fixtures, not an assumed
identity match. Player cards and settings are never test destinations.

Accepted implementation baseline: D08Q3 `f284dc1`, player binary
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.
D08Q3 is user-accepted: "i accept this! great work."

## Investigation to perform next

1. Record the active profile, view, refresh target, resolution, widescreen and
   save/overlay identities. Reproduce looking up while stationary, slowly
   turning/pitching and walking. Separate intended sky animation from motion
   induced by the camera. Start grounded; compare flight only as a control.
2. Trace how this game's sky is authored and drawn using the owned executable,
   active overlays and local retail assets. Establish whether it uses world
   geometry, a dome/box, textured background primitives, or another mechanism.
   Document authenticated routines/callers, coordinate spaces, camera coupling,
   texture/UV animation, depth/order, clipping and update cadence. Asset names
   or renderer options alone do not establish the draw path.
3. Capture full presented-frame sequences with the sky and horizon visible,
   preserving camera/input and frame timing. Compare Vanilla/Modernized,
   original/view cameras as relevant, 60 and 120 first, then 180. Determine
   whether the defect exists in original guest frames, replayed frames, or both.
   Isolate bob, widescreen, near clipping and precision only where evidence
   points to them. These are candidate factors, not established causes.
4. Distinguish intentional parallax/cloud scrolling and original PS1 precision
   from UV jumps, seams, jitter, clipping, ordering or stale/interpolated sky
   transforms. Relate visible good/bad frames to actual primitive/transform
   evidence before choosing a fix. Do not assume the D17N wall issue shares it.
5. Implement a bounded repair if supported by evidence. Preserve intended sky
   art/motion, horizon and legitimate occlusion, with Vanilla available. Avoid
   freezing the whole scene, broad rendering bypasses or a replacement sky
   asset unless separately justified and requested. Verify active overlays
   before hooks; never hand-edit generated C.

## Acceptance and documentation

A detailed engineering write-up must explain how the original sky works,
what causes the reported motion, what changed and why, verified addresses and
asset provenance, reproduction settings and save identity, before/after
presented-frame evidence, rejected hypotheses and remaining limits. Keep local
retail assets, captures and save files out of Git; commit authored source,
tests and documentation. Export and verify the accepted-source patch against
the pinned clean framework if framework source changes.

The result should look stable during repeated look-up, slow yaw/pitch and
walking routes while preserving deliberate sky motion. Check first/third
person, ground/flight, horizon edges and nearby world geometry; use other sky
views where available to avoid a slot-specific workaround. Retain accepted
D17C/D/E/F/K presentation and D08Q2/Q3 flight/recovery/aiming behavior. Record
60/120/180 behavior and any frame/audio cost without judging GPU performance
from Xvfb. User confirmation is required for Done; use Needs playtest if a
candidate is verified but visual acceptance remains outstanding.

Update this note with evidence as work proceeds, plus the canonical job board,
status/handoff and manual if player-facing behavior/settings change. Clearly
separate measured findings, user observations and hypotheses. Commit/push the
finished bounded result and stop without starting another backlog job.
