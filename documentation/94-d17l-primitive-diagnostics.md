# D17L - Per-instance primitive diagnostics

2026-10-04. D17L is In progress, selected for the bounded diagnosis proposed
by R01. This is diagnostic preparation, not a reproduced cause or a cup fix.
The previous user report in note 89 remains authoritative.

**Continuation:** runtime reproduction identified native tabletop overdraw;
the bounded fix and final validation are in [note 95](95-d17l-tabletop-depth.md).
D17L is now Needs playtest; the initial preparation below is historical.

## Scope and provenance

The prior trace deduplicated by mesh and could not distinguish a disappearing
cup from stable instances of the same prototype. The new title-owned trace
retains the static instance from s2 at the authenticated 0x80032288 caller,
source mesh, source face, per-process call serial, host frame, savestate-load
epoch and live/replay identity. Other callers have instance zero rather than
an assumed instance identity.

This independently implements the measurement lesson from
[R01](93-disruptor-reference-research.md), using TTK's existing guarded hook,
projection, packet and ordering contracts. No Disruptor source was copied,
translated or imported. No rendering repair or graphics setting is introduced.

## Opt-in capture

Set `DNTTK_PRIMITIVE_TRACE` to an absolute prefix in an ignored analysis
directory. Creating `PREFIX.enable` enables recording at subsequent mesh
entries; removing it stops new mesh records. Each process appends to
`PREFIX.PID.jsonl`, so forked workers have separate unbuffered files.
No trace path means no file access or trace formatting. Normal launches do
not enable this diagnostic.

- `DNTTK_PRIMITIVE_TRACE_LIMIT`: maximum ordinary records per process, default
  200000, range 1-2000000. A final `limit` record identifies truncated evidence.
  Restart with a new prefix to obtain a new capture budget.
- `DNTTK_PRIMITIVE_TRACE_MESH`: optional numeric mesh-address filter, parsed
  with base autodetection. This is a diagnostic filter, never a rendering rule.
  Leave unset when examining potential occluders alongside a cup.
- Join records by `(pid, call)`, not mesh alone. Pointer identities are local
  to their authenticated scene and load epoch, not permanent object IDs.

Events include mesh/GTE/context state, projected source vertices, per-face
native-retained/host-candidate decisions, whole-mesh budget fallback, backface
and clip rejection, clipped corner count, far/budget packet rejection, and
emitted host packet address/ordering slot/command and raster XY/depth.

The trace is an upstream producer diagnostic. Host coordinates precede GPU
draw offsets and backend transforms. `native_kept` and `native_mesh` mean
that native rendering retains ownership, not that the native routine actually
submitted a polygon. A host `emit` is not proof that the depth test passed or
that its pixels reached the presented frame. Pair these records with actual
GPU packets, presentation captures and depth/order evidence. A worker trace
is not proof its image was displayed. Do not classify a bug from counts alone.
Trace I/O perturbs timing; disable it for performance acceptance.

## Offline verification

- Owned-EXE near-renderer packet contracts and projection/plane tests pass.
- The observation oracle hashes fixture RAM, CPU general/GTE-control registers
  and host-vertex counts after each hook. Trace disabled and enabled match.
  Compiling the pre-change renderer against the same harness also matches
  the candidate: digest `1fe1d468fd2c28c7`. This is fixture equivalence, not
  whole-game visual or timing equivalence.
- Python trace checks verify opt-in gating, separate instance identities for
  a shared mesh, source-face/packet linkage, raster payloads and bounded output.
- The controls harness builds and its owned-EXE/LEVEL00 guard fixtures pass.
- The player executable builds; this is not gameplay or performance evidence.

Private preparation is under `recomp/analysis/d17l-20261004`: accepted source
and executable backups, player hash/mtime manifest and separate dated test
cards. UI 11's private state matches
`267c6a268992f8468179211291031a668908dfa081c70bcf9439280004e933ef`.
No player file or original media was written. No framework or generated game
source was edited. Existing accepted framework modifications remain intact.

## Remaining work

At the initial diagnostic checkpoint, explicit launch authorization was pending
and no game had been launched. The user subsequently authorized development
launches on 2026-10-04, preferring background/offscreen tests and foreground
windows only when visual verification requires them. AGENTS.md now records
that standing permission. The continuation is recorded below.

The investigation uses private slot 11 and approach/retreat captures to
identify the exact cup and supporting/foreground surfaces, and correlate the
producer records with submitted/drawn/presented primitives. Stable bar props
are controls. Determine whether the cause is missing submission, clipping,
depth rejection, ordering or a genuine coverage gap before changing behavior.
Preserve accepted E/K/D/F/O and the primary 120 FPS baseline. Full D17L closure
still requires a bounded fix, 60/120/180 checks and user acceptance.
