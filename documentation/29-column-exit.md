# Column-area exit: stale scene objects

## Report and evidence

The user reported excellent control feel, then an unexpected exit while crossing
the column beside the first map's fenced atomic-health area. The supplied log
ended at guest PC zero. The matching on-disk exit capture was preserved before
launching private tests (`analysis/d08-column-exit/`, local game-data artifacts).

At frame 15637, PC was zero, RA `0x80025C04`, SP `0x801FFF50`, and a0
`0x801D3054`. The resident update loop calls the handler selected by object byte
`+0x15`. In the capture that byte is 255 and the table slot is zero. The object
is still linked through node `0x800CE5F8` in the update list at `0x800C5694`.

Crucially, that address lies inside current enemy-animation slot 4:
`[0x801D2E00, 0x801D3344)`. The surrounding overwritten bytes are animation/model
state. Initial-load tracing establishes that `0x801D3054` was previously an
allocated 0x84-byte, state-22 world object. It is not a valid world object in
the current animation allocation. Duke's final animation is running (78), with
position (-1671, -9766, 18055); the exit snapshot is not itself a climbing frame.

This establishes a stale scene-object reference into reused memory. It does not
establish the exact earlier transition that retained or reintroduced the node.
Two instrumented fresh starts executed the original cleanup completely. No
claim is made that the full user's failure sequence was reproduced, nor that
this conclusively explains the older security-key report without RAM evidence.

## Bounded prevention at memory reset

`src/ttk/scene_lifetime.cpp` retires remaining update/deferred scene lists before
`0x8002B9F4` resets the level arena. The original unload path already releases
these lists through `0x8001C83C`; an ordinary cleaned transition is a no-op.
The safeguard implements those routines' no-callback list release semantics:
clear each node's data, push it onto the original free chain, update the free
count, and clear the two list heads. It runs in both profiles as a title-specific
lifetime correction, independent of input, camera, aiming, and rendering.

The hook requires authenticated live bytes for the reset/release/pool routines,
a verified resident caller, reset argument 1, and normal execution. Before any
write, it checks the complete free chain and both scene chains for pool bounds,
alignment, cycles, shared nodes, backlinks, tail ownership, and exact free-count
accounting. Unknown or malformed state is left untouched. It never substitutes
or skips an object handler, changes health, or tries to resume the corrupt exit
snapshot. A nonempty cleanup logs its node count and caller.

The hook is declared in `game.local.toml` and emitted through the normal generator.
No generated C, original disc, or engine submodule source was manually edited.

## Verification and limitations

The native fixture uses the user's actual 2 MiB exit RAM as list/allocator data.
It compares all RAM effects against execution of the owned original MIPS
no-callback release routines, excluding their temporary stack-save words. The
42-node release is identical; CPU and player state remain unchanged. Repeated
clean resets, wrong callers/arguments, changed live code, malformed/shared/cyclic
lists, incorrect free counts and an active call bailout are checked.

Input, control and aiming native suites also pass. Python tooling tests pass.
Private gameplay and Vanilla evidence are recorded in the companion report.
The extended fenced-area replay reached the column, performed standing jumps,
and killed two nearby pig cops through original weapon damage. Its final active
list contained 46 nodes with no null handler or animation-buffer overlap. The
player remained alive and the process exited 0. Several automated approaches
were blocked by geometry or combat; **this is not a completed column-overclimb
acceptance run**. No movement adjustment was made to improve the test route.
Automated routes use isolated cards; the traversal driver logs player-health
longevity writes and does not change movement, enemy health, or scene lists.

The exact transition that produced the user's stale reference remains open.
The reset safeguard prevents a nonempty old list from surviving an observed
arena reset, but cannot prove that another path never reintroduces an obsolete
reference afterward. D08 therefore remains Needs playtest. Retain any fresh
exit RAM/log and the `[TTK scene]` messages if the exit recurs. The decoder now
reports animation-buffer overlap and scene-list membership automatically.
