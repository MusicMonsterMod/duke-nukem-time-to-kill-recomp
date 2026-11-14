# 472 - D50E and D41T: the barrel crash and one light per explosion chain

Session of 2026-11-14 (evening), on ripspace (the desktop: RTX GPU, 16
threads). User: "Work on D50E, then D41T, autonomously and completely ... The
goal is that I can blow up the barrels in RESISTANCE IS FEUDAL and keep
playing." Both built; accepted and closed the same evening (user: "lock it in and close it out! it 100% works").

Lab: `recomp/analysis/d50e/lab.py` (local), from the perf lab, with its own
empty card directory: a new game from the title, the console's `level 7` and
`level 2`, then lab savestates of its own. No copy of the player's saves or
cards was used; the lab checks the player's files are unchanged after each
run. The player's profile is copied read only (GRAPHICS ULTRA: Relit, sharp
live shadows, 4x). Evidence labels: MEASURED, CODE, INFERRED.

| Job | Result |
| --- | --- |
| D50E | Shadow classifier and `lz_ensure` shifts bounded; crash reproduced on the old loop, gone on the fix; framework patch exported and verified; Needs playtest |
| D41T | One merged light per explosion chain, no blast shadows, dark lights leave the list, importance cut to 32, one rebuild a tick; Needs playtest |

## 1. D50E: the shadow classifier loop (MEASURED)

**Fix** (`gpu_gl_renderer.c`, exported to
`recomp/patches/time-to-kill-accepted-source.patch`):

- `sh_classify_tri` walks the set bits of the light mask
  (`bits &= bits - 1`, `__builtin_ctz`) instead of `for (i = 0; m >> i; i++)`.
  For every mask without bit 31 the lights are visited in the same order as
  before, so no picture can change (CODE); with bit 31 the old loop never
  ended.
- `lz_ensure`: a tile row is now one 64-bit mask (surfaces up to 2048 px), and
  a wide surface wider than that is never loaded lazily (`gl_rt_load`), so no
  shift reaches 64. Before, a row was 32 bits and a surface wider than 1024 px
  would have shifted by 32 or more (not seen in play).
- No other `m >> i` light-mask loop in `runtime/src`; the shader loops shift
  the mask itself and end.

**Proof** (lab case `fill32`, level 7 start, `light bake off` so the 8 lamps
stay listed, then 24 test lights round Duke, the last one nearest and
brightest):

| Build | Result |
| --- | --- |
| old loop restored (only that hunk) | crashed when the 24th light went in: SIGSEGV in `sh_classify_tri` (`gpu_gl_renderer.c:4701`) from `gpu_textured_triangle`, a render thread, `i` = 29,109,864 (the user's core: 29,109,918) |
| fix | 2 minutes at 31-32 lights, a load, the lights cleared and refilled to 32: no crash, clean quit |
| `PSX_RT_VALIDATE`, classes on, 20 lights | old 11 of 379 redraws differ, fix 12 of 377: the asynchronous class cache's usual noise (note 446) |
| the same, 32 lights | fix 9 of 376 |
| classes and reuse off | fix 384 of 384 exact (the classifier does not run then) |

Patch check: a pristine checkout of the pinned framework plus the exported
patch matched all 90 modified and new framework files.

## 2. D41T: the light table during a chain (MEASURED before)

Barrels can be spawned: the game's death routine `0x80096be4` sends an object
to the barrel blast `0x80097090` when its type's flag word has bit `0x10000`.
Level 7 (RESISTANCE IS FEUDAL) places 86 of type 944 (black, radiation
symbol); level 2 (MINER 69ER) 16 of type 299 (TNT). `spawn 944` / `spawn 299`
makes one that takes damage and blows up through the game's own code.

Before (build of 2026-11-14, ULTRA), developer `DNTTK_SHADOW_COUNT=1`:

| Chain | Lights | Shadow casters | Maps | Rebuilds |
| --- | --- | --- | --- | --- |
| 11 barrels bunched (one tick) | 11 | 11 | 10 (1.9 ms) in that tick | 11 in that tick |
| 8 barrels, 420 units apart (10 ticks) | up to 9 | all | one per barrel | 1-2 a tick |
| MINER 69ER, 5 TNT barrels | 5 | 5 | 4 in one tick | up to 5 |

Also found: the 8 slots kept for blasts were counted before D41P2 took the
baked lamps out, so blasts could be refused while the final table had room;
casters past the 16 maps stayed listed at colour 0 (in the 32-light lab, 23
test lights showed 16 lit and the rest dark but listed); `light_add`, when
full of test lights, overwrote test light 0.

## 3. D41T: what changed (CODE)

`src/ttk/dynamic_lights.inc`, `light_shadows.inc`, `lamp_bake.inc`,
`modern_controls.cpp`. Modernized, Polymorph, LIGHTING ENHANCED or RELIT
only (`lights_enabled()`); Vanilla, Classic and LIGHTING ORIGINAL (SIMPLE)
never reach this code.

1. **One light per chain** (`light_chain_blast`): every blast (barrels,
   dynamite, pipe bombs and grenades, rockets and rounds, gib bursts) within
   4500 units of a blast light in its first 12 tics feeds that light instead
   of adding one. The light moves 65% of the way to the newer blast, takes the
   bigger blast's strength and reach, grows 12% brighter per blast (at most
   1.6x) and 5% wider (at most 1.35x), its envelope goes back to its peak and
   its life grows 2 tics (at most 32).
2. **No blast shadows**: blasts are no longer shadow candidates (as Polymer's
   point lights and Quake's dlights). Test lights still are (lab tool).
3. **Dark lights leave**: a source below 0.01 is not listed, and after the
   shadow slots and the bake any light below 0.01 (a caster left without a
   map, one faded out) is removed, so it sets no mask bits.
4. **Importance**: the tick's candidates are staged in a 64-entry list (every
   source fits); over 32, the most important stay (brightness times reach
   over distance from Duke, the D41F score), in list order. The published
   view stays 32 lights, so the draw-time state does not grow. Rounds in
   flight: the 8 nearest Duke (was the first 8 in pool order). A full source
   list replaces its dimmest non-test source (was the oldest).
5. **One rebuild a tick**: blasts, launches and sparks mark the view dirty;
   it is rebuilt in `lights_tick`, or at the image's capture
   (`render_state_save`) when an event came after the tick's rebuild, so a
   blast still reaches the image it happened in.

Developer only: the `DNTTK_SHADOW_COUNT` line now takes a period
(`=1` every tick) and adds the sources by kind, rebuilds, dark lights and
cut counts; `DNTTK_LIGHT_TRACE` names the blasting object's type.

## 4. D41T: after (MEASURED)

Same chains, new build:

| Chain | Lights | Blast casters / maps | Rebuilds a tick |
| --- | --- | --- | --- |
| 8 barrels rolling | 1 | 0 / 0 | 1 (2 when the shot's flash lands after the tick) |
| MINER 69ER, 5 TNT | 1 blast light | 0 / 0 | 1-2 |

**180 Hz**, offscreen on the real GPU (FRAME RATE 180, CUSTOM with ULTRA's
rows), 12 barrels bunched in front of Duke (11 go off in one tick), 3
interleaved runs per build, GPU utilisation x clock sampled every 50 ms. The
user was watching YouTube during these runs, so the absolute GPU figures
include that load; the before/after runs alternate.

| | Before | After |
| --- | --- | --- |
| lights at the peak | 12 | 2 (chain light, muzzle flash) |
| blast maps drawn | 12 (2.7-3.3 ms GPU) | 0 |
| rebuilds in one tick | 10-11 | at most 2 |
| GPU work, chain window over the second before it | +10.2 / +8.6 / +10.6 points | +1.7 / +2.2 / +3.4 points |
| GPU work peak | 92.2 / 88.3 / 93.2% | 76.5 / 68.7 / 61.8% |
| presents a second, misses | 174.7-176.7, 0 | 174.3-176.1, 0 |

Not measured: the field laptop (not here).

**Look**, tuned on ULTRA (`research/before-after/D41T/`, local): a single
barrel is unchanged (no merge; the corridor has nothing for its shadow to
fall on). In a chain the old lights stacked and flattened the whole corridor
to white-orange at once; now one fireball brightens and travels down the
corridor with the chain, then fades. BALANCED (Enhanced, simple edges, 2x)
reads the same. SIMPLE is LIGHTING ORIGINAL: no dynamic lights, unchanged.
MINER 69ER at point-blank range looks the same before and after (one strong
light instead of five).

Trade-off (by design, the user's direction): blasts now light through walls
(no blast shadows), as in Polymer and Quake.

## 5. Remaining

- The user's playtest: the RESISTANCE IS FEUDAL barrel room blown up
  repeatedly, including after a load (D50E), and the look of chains (D41T).
- Field laptop measurement during a chain (D41T acceptance).
- Local test `test_primitive_trace` fails (skipped in CI): its harness
  compiles only `near_clip.cpp`, `live_env.cpp` and `lamp_bake_map.cpp`,
  none changed here; its binary predated the D41P2/D41S `near_clip.cpp`
  changes. Not investigated in this session.
