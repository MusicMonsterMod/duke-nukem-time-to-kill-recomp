# D17E/K/L - Subway visibility, blood and tabletop props

**Follow-up recorded 2026-10-04:** D17E and D17K are now Accepted by the
user. D17L remains open: the isolated cup still flickers, largely unchanged,
although bar cups/bowl are stable. The bounded L captures below are historical
evidence, not proof of resolution. D17C remains Done. Current UI 12 now holds
a separate wall report, D17N; UI 9 ladder ammo is separate D17M.
See [latest results and actual mesh inspection](89-d17-playtest-followup.md).

2026-10-03. The user selected D17E, D17K and D17L together and accepted
D17C's current camera stability and look. D17C is Done; no camera behavior
was changed for that acceptance. D17D/F remain accepted regression baselines.

## Reproductions and causes

Tests use a dated private copy of all current player card/state files,
`analysis/d17-high-refresh/cards-ekl-20261003`, private settings, debug port
9231 and NVIDIA EGL through SDL offscreen. The source hashes and mtimes are
in the private `ekl-save-manifest.json`. UI N means file N-1.

| UI slot | Job | Current state SHA-256 |
| --- | --- | --- |
| 3 | D17E | `21090f8cb992dc89b817cd08a86f5862a16ea8a6af56c26bd1bb8a9940d3f816` |
| 11 | D17L | `267c6a268992f8468179211291031a668908dfa081c70bcf9439280004e933ef` |
| 12 | D17K | `31271d6284c52c92b449c594241b877810fd77a6bced59adfb184a2237eedc21` |

D17K reproduces as triangular floor sections painting over blood. Turning off
host depth testing alone leaves the cuts; disabling the near-polygon path
restores complete patches. Diagnostic tint shows the blood remains native
while the floor is host-rendered. The traced blood packets are textured quads
(GP0 0x2c), page 0x1c, CLUT 0x57ac. These are diagnostic identities, not
runtime texture-selection rules.

The original world renderer's comparisons at 0x80011654/84/b4 skip smaller
vertex depths. Thus its ordering key is the source polygon's **maximum** SZ,
shifted by five at 0x80011708/0x800117d0. The old near-renderer comment called
this a minimum, and the enhanced path actually sorted each subdivision by
its own average depth. That lets a near floor piece paint over a native
blood quad originally sorted in front of the complete floor polygon. This
reproduction establishes a mixed-path ordering defect; it does not require
assuming genuinely ambiguous coplanar blood placement.

D17L reproduces on the club tables in the current UI 11. The cup is visibly
cut off at middle distance and becomes complete on approach. A table can
enter the enhanced path while a compact prop sitting on it remains outside
the 3072-unit takeover boundary. The original renderer's approximate painter
ordering can also cut the cup in a near-clip-disabled comparison.

A diagnostic trace resolves the cup mesh to 0x80129a3c (eight vertices,
60 x 86 x 60 local-unit extent) and bowl to 0x80129880 (17 vertices,
210 x 86 x 210). Sample view depths are 5603-5677 and 4895-5102 respectively.
Both come through the static-prop caller returning at 0x80032288. These
addresses are evidence, not object-specific runtime conditions.

## Bounded implementation

- Enhanced world subdivisions retain their source polygon's original
  farthest-corner ordering key. Host depth still resolves host-to-host
  occlusion, and native decals keep their original textures and blending.
- Compact static meshes, no more than 256 local units per axis, receive
  consistent host depth even beyond the near radius. This covers cups and
  bowls without pulling large distant scenery into the expensive path.
  Eligibility uses mesh extent and the verified static-prop caller, never
  level coordinates, mesh addresses, texture IDs or save-slot identities.
- The static caller's seven instructions at 0x8003226c are byte-authenticated
  alongside the existing renderer guards. Articulated dancers use a different
  caller (0x80033778); actor joints use 0x800351f4. They retain their previous
  takeover behavior. Original view/Vanilla still does not enter this path.
- Opaque faces of a compact prop share its average source depth as an ordering
  key. The depth buffer resolves the faces, while keeping them together avoids
  repeated host/native draw-state switches. Translucent faces retain individual
  depth ordering.
- Fully inside clipping polygons retain their existing vertices without five
  temporary vector copies. Fully outside polygons reject directly; mixed
  edges use the existing clipper. This reduces the cost of compact props.

The original source-plane correction and two-unit contact tolerance are
unchanged. No blanket visibility/culling change, blood offset, depth-test
removal, camera/input/simulation/audio change or retail mesh edit is used.
Existing arena limits and whole-mesh fallback remain in force.

A broad all-object depth experiment was rejected because it added expensive
articulated meshes and distant scenery, reducing the club's 120-target result
and causing output underruns. The compact, verified static-prop path replaces
that experiment; do not re-enable broad takeover as a quick fix.

Developer comparisons, not saved settings: `DNTTK_SOURCE_ORDER=0` restores
piece-average world ordering; `DNTTK_PROP_DEPTH=0` disables distant compact
prop coverage. Existing `DNTTK_PLANE_DEPTH` / `DNTTK_PROP_CONTACT` comparisons
are unchanged. This is title code only; the framework patch is unchanged.

## Verification and remaining acceptance

- D17E: full 2728 x 960 compositor captures, both wall approaches, forward/back
  travel, edge turns and the opposite stair route at 60/120/180. No missing
  peripheral wall section reproduced. A 120 FPS comparison with both new
  corrections disabled also does not reproduce it. This supports a player
  closure check, not a newly claimed E-specific fix.
- D17K/L: before/after captures identify the blood cuts and incomplete cups.
  The candidate keeps the blood complete and the cup/bowl intact along the
  approach/retreat and slow-look routes at 60/120/180. Four 24-present
  sequences cover moving K/L views at 120/180. Original PS1 motion remains.
- The accepted shelf and two closet snapshots from the older private
  `cards-user8` copy were checked separately at 120/180. Current UI 11/12
  have new save hashes and are not substitutes for those old closet states.
  The shelf and closet surfaces remain intact. Third-person smoke succeeds;
  Vanilla has zero host-depth triangles and zero replay workers.
- New owned-EXE native packet tests exercise the registered hook, not a
  duplicate helper: all floor subdivisions stay behind a native decal's
  ordering slot; distant compact static meshes get depth; actor/dancer
  callers and larger distant meshes do not. Boundary cases at 256/258 units,
  opaque grouping versus translucent ordering, original view, and a changed
  caller instruction all pass. Existing projection/plane math tests pass.
- Final GPU-surface restore oracle: 199 checks, zero mismatches, with private
  table and blood loads at 2x. An earlier shelf/closet pass also had zero
  mismatches in 186 checks. These readback-heavy checks are not speed tests.
- Python: 100 tests, 98 pass and two skip. The installed framework patch
  check passes; no framework files or generated game C changed in this job.
- Final 120-target seven-scene timing samples have zero output underruns,
  zero new mesh-budget fallbacks and zero worker failures. Distinct image
  submission rates range from 111.3 to 120.4/s, depending on the scene; this
  is not a promise of a locked 120 unique images/s everywhere.
- Final 180-target club samples: opening 177.9 distinct images/s with zero
  output underruns; table scene 178.6/s with 2,154 missing output sample
  frames (about 49 ms accumulated over eight seconds). Earlier baseline
  comparisons range around 177-180/s and sometimes have small underruns;
  the immediately preceding settled baseline was clean. Thus a small added
  rendering cost remains measurable at 180. Do not claim zero-cost 180 FPS
  or actual-device audio acceptance. Player listening/feel is still needed.
  Broad and 512-unit prop coverage had materially worse results and were
  rejected; the final rule is 256 units plus opaque grouping.
- All 28 original player file hashes and mtimes match the pre-test manifest.
  Tests used private preferences/cards and offscreen rendering; no player
  settings, cards, states or original media were written.

Final executable SHA-256:
`151ea6aab6957eeb2c3da5cf990700d566a227eb723eb829d13600146c098520`.
Codegen remains unchanged. This is bounded first-level evidence, not campaign
proof or a universal solution for arbitrary overlapping objects. At the candidate handoff, D17E/K/L
were **Needs playtest** (superseded by the follow-up above); D17C is **Done** on the user's earlier acceptance.

Private evidence uses `ekl-` prefixes under `analysis/d17-high-refresh`.
Final 256-unit capture runs are `ekl-release-*`; timing is
`ekl-release-regress-120-f1.json` and `ekl-small.json`. The initial accepted
executable is preserved in `ekl-baseline-20261003/`. Full-width wall routes
are in `ekl-subway-*`, including the disabled-correction baseline comparison.
No captures or original game data are distributed in Git.

Historical candidate player check (save slots have since changed): load F7 UI 12 and inspect blood while moving/looking; load UI 11
and approach/retreat from the table; load UI 3 and check walls/stairs near both
view edges. Start at 120 FPS, then try the usual refresh setting, including
club audio/feel at 180. F10 captures Modernized input if needed. Launch with
`python3 recomp/tools/local/run.py`; no special activation setting is needed.
