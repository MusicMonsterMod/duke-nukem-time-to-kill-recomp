# D17AF - Water tile near-clip

2026-10-29. In progress. This note records investigation and implementation.

## Problem statement

Slot-10 water tiles disappear at close range. The native water renderer builds
flat-quad FT4 packets through the GTE independently of the main mesh pipeline,
so the ordinary near-clip path does not intercept them. At close range the GTE
saturates their corners to extreme screen positions; the rasterizer discards or
clips the degenerate quad and the tile vanishes.

## Native water renderer anatomy

Hook address `0x8002bc18` sits in the water draw loop (confirmed by calling-code
analysis at the start of D17AF investigation). Return address `0x8002fbb4` is the
unique post-render return seen in that loop.

Identity region: `0x8002ef90`, size `0xcc0`, digest
`52db23011c61c794e5d5f5923fdf405242612f7ff8251d5e1734d151fc0b1daa`.

Registers at the hook site carry a complete description of the tile:
- `gpr[5]` - FT4 packet pointer
- `gpr[21]` - region descriptor (tile world position)
- `gpr[29]` - stack pointer; `sp+0x90` is a sanity back-pointer to region
- `gpr[17]` / `gpr[18]` - previous/current Z rows in the water grid
- `gpr[20]` - column index within the row
- `gpr[4]` - native OT slot for painter ordering

The region descriptor encodes: `+0` X world, `+2` Y world. The Z coordinate
is a signed 16-bit value read from `sp+0x58`. Water Y-offset phase is stored at
`0x800c3cd8` and added during reconstruction. Column stride is 1024 units; row
stride (Z) is also 1024 units.

## Implementation

`recomp/src/ttk/water_clip.inc` is included inside `near_clip.cpp`'s anonymous
namespace after `object()`.

The hook:
1. Guards on RA == `0x8002fbb4`, `DNTTK_WATER_CLIP=1`, `identity()`,
   `water_render_identity()`, modern, enabled, precise, `live_eye`, budget.
2. Reconstructs the four tile corners from register state using `near_project()`.
3. Cross-checks each corner's SXY/SZ against the grid record registers; returns
   with `water_grid_reject` if any record disagrees (fail-closed).
4. If no corner is unsafe and the quad is not oversize, returns without taking
   over (pass-through for distant tiles).
5. Sets `f.sky=true` to use painter depth, preserving native OT slot.
6. Emits a clipped FT4 through `draw()` in the same native slot.
7. Zeroes the upper byte of the native packet link word so its original location
   leaves a zero-terminated chain rather than a raster command.

The hook is wired at line 1406 of `near_clip.cpp` via `observed_hook(&wc, 0x8002bc18)`.
`water_clip.inc` is included at line 1332.

Counters: `water_seen`, `water_taken`, `water_guard_reject`, `water_grid_reject`.

## Tests

`recomp/tests/local/near_clip_native.cpp` contains:

- `water_clip_contracts()` - guard-only tests (no game EXE required):
  - RA mismatch rejects
  - `DNTTK_WATER_CLIP` unset (default 0) rejects
  - `!modern` rejects
  - `!live_eye` rejects

- Positive-path tests inside `packet_contracts()` (requires game EXE):
  - Takeover test: tile with z_far=1200 (near z=176), y=100; corner at
    (1024,100,176) saturates; asserts `!host_corners.empty()`, all depths
    negative (painter), native packet upper byte zeroed.
  - Grid mismatch test: corrupts one grid record SXY; asserts
    `host_corners.empty()`.
  - Pass-through test: far tile z=3100 (near z=2076); asserts
    `host_corners.empty()`.

Both test groups pass: `./build-local/ttk-near-test disc/SLUS_005.83`.

Note: the positive-path block restores the code word at `0x80032280` that the
preceding identity-failure test XOR'd (that region overlaps `near_guards`); the
restore is required so `identity()` and `water_render_identity()` pass for the
water hook.

## Status

Implementation written and hooked. Contract tests passing. Gameplay verification
needed: `DNTTK_WATER_CLIP=1` must be set to enable the hook in the player build.
Once verified in slot-10 that tiles no longer disappear at close range and no
regressions appear in other water-bearing levels, the env-var guard can be
removed and the feature made default.

Remaining items:
- Gameplay verification in slot-10 (water tiles stay visible at close range).
- Regression check in other water levels.
- Consider promoting from opt-in (`DNTTK_WATER_CLIP`) to default after validation.
- `water_grid_reject` monitoring: a non-zero counter in gameplay would indicate
  the register/grid interpretation has drifted from the native builder.
