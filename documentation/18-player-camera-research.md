# D03 — Verified first-map movement and camera boundaries

D03 is complete as a research job. The first-map movement and normal-camera
entry points below have live call/argument evidence, original-code evidence,
and fail-closed diagnostic guards. This does **not** implement modern controls
or certify a replacement algorithm. D05 and D06 must implement and test those
algorithms; D07 owns weapon aiming. No generated C or guest state was patched.

## Reproduce and reject unsupported identities

```sh
python3 recomp/tools/local/vanilla_regression.py --name UNIQUE --mode vanilla --state-probe
python3 -m unittest discover -s recomp/tests/local -p 'test_ttk*.py'
```

The runner owns a fresh process, separate cards/settings and a bounded input
route. It refuses an active game. `ttk_state_probe.py` checks the exact supported
SLUS-00583 boot executable SHA-256:
`b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a`.
It compares twelve live code ranges against that executable, requires reciprocal
player/camera pointers, and requires the **9652-byte code/table body** of
LEVEL00.OVR at `0x800CA968` to hash to
`274d71ddd8aeb6e1ca12c0229eea5087c7750a39f442785b94f0efeec81a25b3` (the
file's final 16 bytes are a scratch hit-position vector the zone script
writes during play — D08P, documentation/56).
A mismatch raises an error; the runner cleans up its owned process. Identity
is checked before arming diagnostics and after the checkpoint in the final
acceptance route. The code ranges in the tool are byte guards, not declarations
that every seed interval is exactly one function.

`ttk_hook_contract.py` additionally rejects unexpected targets, callers,
arguments and malformed function-trace records. This prevents shared animation
code running for an enemy from being attributed to Duke. Ten tests exercise
identity/overlay/pointer refusal, read-only sampling, actor/caller rejection and
collision-mode rejection. These are observation guards, **not mutation permits**.
A future runtime hook must recheck its identity and state at the actual invocation;
pre/post debug snapshots cannot certify the intervening frames atomically.

The exact overlay match is independently reproduced by
`correlate_overlays.py` against the owned-disc overlays; see
[overlay correlation](reports/d03-overlay-correlation.json). LEVEL01's three
shared chunks fail whole-file equality. MOVIE and LEVEL00 reuse the load base
at different times. Other maps must fail closed until their identities and
ownership are verified. Existing loader-cache metadata alone is insufficient.
(D22A/D22B: the 21 selectable LEVELxx overlays are now verified and
authenticated by tag and body; any other overlay still fails closed. See
[note 102](102-d22b-every-level.md).)

## Verified state and units

Addresses are KSEG0; trace physical addresses compare after masking with
`0x1fffffff`. Position values are signed raw game units, not metres or feet.

| State | Address/offset | Evidence |
| --- | --- | --- |
| First player | `0x800D7198`, stride `0x8A4` | Initialization, reciprocal camera link, player-filtered live calls |
| Normal camera | `0x800D6EB0`, stride `0xB0` | Initializer `0x8003ACCC`, live updater `0x8003ADE4` |
| Reciprocal links | camera `+0xA4`, player `+0x7D4` | Both checked before interpretation |
| World position | player `+4,+8,+0xC`, signed words | Forward changes X/Z; jump decreases Y; original integration adds deltas |
| Actual/target heading | player `+0x1C/+0x24`, halfwords | Target store `0x800578A0`, smoothing store `0x80057ADC` |
| Angular units | 4096 per revolution | Normalization and shortest-difference ±2048 at `0x80057A38..0x80057AFC` |
| Movement delta | player `+0xFC,+0xFE,+0x100`, signed halfwords | Animation production, rotation, collision and position integration |
| Turn increment | player `+0x258`, signed halfword | Input helper store `0x800568E8`, consumed by target-heading update |
| Animation identifier | player `+0x60`, signed halfword | Animation structure passed as player+0x60; state-specific callbacks |
| State bytes | player `+0x22C/+0x22D` | Standing 0, captured jump 9; not a complete state enumeration |
| Camera rotation | camera `+0..+0x10`, nine signed halfwords | Q12 matrix, magnitudes around 4096; transpose and vector-rotation code |
| Camera position/anchor | camera `+0x14..+0x1C`, `+0x64..+0x6C` | Signed world-space vectors consumed by camera update and world queries |

`0x80097C04` is anchored by the UpdateDukeMatrices diagnostic and consumes actor
orientation/model state. `0x800137A0` is a camera error **string**, not a function.
The full axis handedness, camera Euler order and all traversal states are not
claimed. See [earlier state observations](reports/d03-state-research.json) and
[delta producers](reports/d03-input-producers.json) for independent runs.

## Usable movement boundary and collision contract

The bounded entry is **`0x80053500(a0=player)`**, with observed return addresses
`0x80048664` and `0x800486A8` (calls at `0x8004865C/0x800486A0`). This is the
normal walking handler, after animation-derived delta production and **before**
its collision/state checks and position integration. A D05 wrapper can supply
movement intent here while continuing the original handler. Do not hook the
final position stores to bypass collision.

The live `d03-hook-boundaries` forward trace establishes this order:

1. `0x800598F0(a0=player+0x60,a1=player)`, return `0x8005A3E8`, produces deltas.
2. `0x80053500`, then `0x8007926C(player,4)`, return `0x80053548`.
3. Heading updater `0x80057230(player)`, return `0x80041C3C`.
4. Normal camera `0x8003ADE4(camera,player)`, return `0x80025EE8`.

For example forward trace sequence 36–40 and 55 records this chain. Sequence
56–63 contains **two** animation/integration iterations before heading update.
Do not assume one movement integration per VBlank. The animation producer is
shared with many enemies; the argument guard is essential.

Delta stores `0x80059D18/2C/44` come from animation displacement with fixed-point
scaling. The call at `0x80059D84` rotates it through `0x80066D94` using actor
orientation. Type `0x20` may add heading offset `+0x2B4`. Position stores at
`0x800538F0`, `0x8005390C`, `0x80053904` add the signed XYZ deltas only after
the walking handler's state/collision branches.

**Critical D05 requirement:** mode 4 in `0x8007926C` constructs a directional
probe through `0x80073B7C`, scales from actor `+0xB0`, then rotates it using
facing (plus the type-specific offset). It does not simply test the proposed
movement delta. Mode 6 has a distinct delta-based path. The query calls
`0x800780B4`, floor/height routines `0x8007765C/0x80077748`, and segment routine
`0x800774CC`; it returns categories used by the walking handler's jump table.
Responses can change deltas, animation or traversal before integration.
Independent movement therefore requires **consistent collision probe direction
and movement direction**. Replacing XYZ deltas alone is invalid. Do not assume
mode 6 can replace mode 4 without testing its different semantics.

Input-to-heading is also established: `0x80056770` reads per-player inputs
through tables `0x800D1600`, `0x800D1A98`, `0x800D14E8`, applies state flags and
`0x800563D4`, and writes `+0x258` at `0x800568E8`. Live right-turn traces show
nonzero increments, followed by target and smoothed-heading changes. D04 should
produce action-level intent; these game tables are not a host binding API.

Jump uses collision return `0x80052FD0` and distinct position stores
`0x80052FF4/0x80053008/0x80053010`; it is deliberately rejected by the walking
context contract. Climbing, swimming, death, scripted control and other maps
need separate state adapters and tests before D05 enables them. Unknown states
must execute original behavior. This is a verified bounded entry, not a claim
that a single replacement covers every animation callback.

## Usable camera boundary and retained constraints

**`0x8003ADE4(a0=camera,a1=player)`, return `0x80025EE8`** is verified live in
standing, moving, turning, jumping and fire-input intervals. Its writes to the
camera vectors are independently traced. The caller tests player `+0x224 &
0x20000000`; that branch uses alternate camera `0x800D7010` and routine
`0x8003B4F4`. Do not apply the normal-camera hook to that branch or to another
player. Menu/pause, inventory and scripted ownership must also suppress modern
input; a valid pointer alone is insufficient.

A D06 wrapper belongs at this normal update boundary. Its desired orbit must
feed the existing constraint stage, **before** the final position commits:
`0x8003AA48(camera, delta)` (live return `0x8003AEB0`, delta at caller
`sp+0x18`) evaluates four candidates through `0x8003A938`,
selects the closest, calls `0x8003A5E0` when appropriate, applies a distance
constraint through `0x8003A85C`, and returns a corrected delta. The update then
commits anchor values at `0x8003B080/08C/09C`, computes the position via
`0x80039C7C`, commits it at `0x8003B0A8/0B4/0C0`, updates room through
`0x80039DD0`, and makes world-bound/height queries through `0x8005D358`.
Replacing the final camera coordinates would skip those dependencies.

The mathematical boundary is now explicit: `0x80010D60` **transposes only the
3×3 matrix**, leaving translation untouched; it is not a full affine inverse.
`0x80039C7C` uses the transposed rotation on `(0,0,-camera[+0x42]/2)` and adds
the anchor vector. The final `0x8002B3C4` call extracts angles from a matrix;
it does not build a new view matrix. These corrections prevent carrying the
earlier speculative inverse/matrix-update labels into D06. Renderer view
translation/FOV/visibility changes remain separate work; D06 must verify orbit
signs and obstruction behavior in gameplay before advertising mouse camera.

## Explicit aiming research boundary

No ballistic-direction or universal weapon hook is verified by D03. Actor
heading, camera rotation and UpdateDukeMatrices establish three different
interfaces; changing one is **not** proof of independent weapon aiming.
Fire-input intervals are included in the traces, but both new runs'
captures retain ammo 200: do not label that interval a successful shot or a
ballistics test. Earlier route evidence remains separate.

D07 must trace an actual ammo-decrement/shot event for each hitscan/projectile
family through its muzzle transform, direction construction, target selection,
auto-aim and hit/spawn call. Record caller, weapon/state, active overlay and
vector units. Then implement separate camera aim, actor facing and weapon ray,
including muzzle-to-target obstruction and original-mode fallback. Projectile
velocity is not presumed to use position-delta units. Until that chain is
verified, aiming mutation stays disabled. This explicit boundary is D03's
required output; D03 does not claim D07 acceptance.

## Evidence and limits

The final [acceptance report](reports/d03-acceptance.json) records guarded runs,
call-context counts and sequence evidence. Raw traces, images,
RAM and proprietary disc data remain under ignored `recomp/analysis`.
Trace `func=0x000029CC` is stale in this hybrid path; use decoded store `pc`
and separately recorded function-entry target/arguments. Function- and
write-trace sequence numbers are separate domains and must not be compared.
Snapshots and screenshots span frames. Trace counts are retained observations,
not a cycle-accurate performance measure or proof of full campaign fidelity.

D04 (action input, rebinding and mouse capture) is next. D05/D06 consume these
bounded hooks, retaining collision/state ownership and validating any new
runtime callback in both native and interpreted execution.
