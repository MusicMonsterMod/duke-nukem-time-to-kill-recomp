# D17AB - Slot-5 mirror rendering investigation

## Accepted closeout - 2026-10-27

D17AB is Done. User: "lock it in and close us out!". The delivered mirror
cloud/HUD/palette correction is accepted; implementation, regression tests and
documentation commit/push are explicitly authorized. Earlier In progress and
Needs playtest entries below are historical. Preserve their evidence limits;
acceptance does not imply a new full campaign or every-mirror test.

Accepted executable SHA-256:
`16126a9f6c4e8ac50839508f1bfcc2474863257e5f5ed3e10ef8e967abafbf7a`.
No new build, game launch, framework edit or next job during closeout.
Launch from the workspace root: `python3 recomp/tools/local/run.py`.


## Cloud follow-up - 2026-10-27

**Candidate status at this stage: Needs playtest.** The cloud symptom is now reproduced and
linked to the same packet-lifetime defect. The earlier open-cloud statements
below describe the first investigation pass and are superseded by this section.
No additional sky rendering change is needed; candidate SHA-256 is unchanged.

Rebuilt a private pre-fix executable by reverting only the pending-packet guard,
then restored and rebuilt the normal candidate. Both use private slot-5 copies,
current player preferences, 4x OpenGL and the same mirror approach. At 120 the
old build draws dark sky-colored triangles across the ceiling and loses HUD.
The fixed build's matching approach retains ceiling and HUD. Disabling only
DNTTK_SKY_BACKGROUND in the old build removes the overhead triangles, while
its other packet problems remain. This ablation is diagnostic only.

Native primitive tracing at 60 identifies 14 rapid packet-address reuses from
negative-depth deferred weapon polygons (caller 0x800351f4) to sky backdrop
polygons (caller 0x80038c94), within two input frames. The fixed comparison has
zero such matches and still emits the sky backdrop and both cloud-layer callers.
These are input-frame timestamps, not proof that each reuse is a separate scene;
the previous arena trace and six-mesh regression establish the unsafe lifetime.
Combined with the old/fixed/sky-off visual comparison, this identifies sky
packets appearing through the deferred foreground path, rather than establishing
a missing ceiling mesh or a need to suppress sky indoors.

The sky native suite passes, including widening, caller/identity gates and the
accepted background rule. Fixed 60 tracing and 120 approach captures show a
solid ceiling. Four outdoor 120 yaw samples using the historical D17R2 LEVEL06
fixture retain visible clouds; this is bounded outdoor coverage, not a new full
campaign sky survey. The normal player executable is byte-identical to the
first candidate. No framework, generated code or new sky-source edits.

Evidence: ignored `recomp/analysis/d17ab/cloud/`, including
`before-approach4-6.png`, `before-no-sky-approach4-6.png`, `fixed120-6.png`,
`old-sky-reuse.json`, `fixed-sky-reuse.json`, `fixed-trace-view.png`, outdoor
captures and `sky-test.log`. Private diagnostic sessions are stopped. Source
slot-5 save/sidecar/thumbnail still match their intake copies. No commit/push or
acceptance closeout. User should repeat the original cloud/HUD/freezer route;
other mirrors and every transition remain outside this bounded verification.


2026-10-27. In progress. User selected job registration, investigation and
implementation. This is a bounded corruption candidate, not acceptance closeout.

## Reports and observed evidence

User reports circling outdoor clouds overhead indoors at the mirror, brief HUD
loss when approaching/jumping with pistol key 2, and severe multicolored rendering
or lasting palette shifts after DNSTUFF, freezer key 0 and jump/fire into it.
Current UI slot 5 (file index 4) is the club mirror. Older notes describing slot
5 elsewhere are historical and were not used as the reproduction scene.

Private offscreen OpenGL captures reproduce severe multicolored rendering during
the freezer jump/fire route at 60. It also occurs with ordered transparency
batching disabled. Disabling host near clipping removes it in the short comparison;
that is diagnostic evidence, not the delivered fix. A software comparison was
exploratory and does not establish identical timing or weapon-state coverage.

## Confirmed packet-lifetime defect and candidate

Near clipping defers first-person arm/weapon packets until the draw boundary.
Those packets consume the host GPU arena but do not yet populate the native
ordering table bitmap. The mesh-entry fallback mistook that empty bitmap for a
new frame, rotating/resetting the arena between deferred meshes. Temporary live
tracing recorded rotation through all four regions within one input frame,
including successive 880/440/240/920-byte allocations while approaching the
mirror. Reuse can overwrite packets that are still pending submission.

Require both an empty native bitmap and no pending deferred packets before that
fallback reset. The authenticated composition boundary still resets normally;
load/replay restoration and ordinary world allocation are unchanged. Temporary
tracing was removed. No framework, generated C, media, settings or asset changes.
The pre-existing framework working changes remain untouched.

A six-mesh native regression preserves an empty native table while queuing
viewmodel packets, verifies stable region/increasing usage and intact first-mesh
bytes, then adds native HUD and flushes/traverses the packet chain. It fails on
the old implementation at the arena-lifetime assertion and passes with the fix.
The specific transient GPU command responsible for persistent palette changes
has not been reconstructed; do not claim a captured invalid CLUT write.

## Verification and remaining work

Player build passes. Full near tests against the owned executable pass with
Original and Corrected geometry/textures, including existing world/object,
Vanilla refusal, Bio Mask and frozen-tint packet contracts. Diff whitespace check
passes. Candidate executable SHA-256:
`16126a9f6c4e8ac50839508f1bfcc2474863257e5f5ed3e10ef8e967abafbf7a`.

Private 60/120 short approach/freezer jump-fire captures no longer show the
reproduced multicolored corruption; close pistol captures retain HUD. Early
routes can die from reflected freeze shots and do not represent prolonged live
fire coverage. A later 120 stress route uses private DNKROZ, verifies equipment
2 / weapon record 11 through 12 jump/fire cycles, and reloads slot 5 successfully.
An earlier stress attempt retained pistol record 4 because the diagnostic action
word had not changed; it was excluded from freezer evidence and repeated with
an asserted record-11 selection. Replay reports three workers and zero worker
or plugin/replay failures. These settings are not performance benchmarks.

Overhead pitch/approach captures at 60/120 and an exploratory sky-background-off
comparison have not independently established the circling-cloud report or its
cause. That symptom remains open. Sparse captures also do not certify every
brief HUD transition. Fresh third-person/Vanilla gameplay, outdoor-sky regression,
other mirrors and broad campaign coverage remain unverified. No sky-rule changes
were made. D17AB stays In progress, with this corruption candidate available for
user review, rather than treating all acceptance criteria as met.

Evidence and private scripts: `recomp/analysis/d17ab/` (ignored). Player slot-5
save, sidecar and thumbnail hashes match the intake copies. Private diagnostic
sessions stopped; player cards/settings were never writable test destinations.
No commit, push or acceptance closeout.

From the workspace root, launch the candidate and repeat slot 5 approaches and
jump/fire, watching HUD, ceiling/reflection and colors. F10 captures controls.

```sh
python3 recomp/tools/local/run.py
```
