# D23A — Modernized frame budget: the code-identity guard cost

**Done - 2026-09-29 (user-accepted):** "the in game stutter fix works fine".

## Report

"I'm getting that stuttering audio / slowness issue" (2026-09-29, before
the D08Q jetpack playtest). The symptom has recurred across sessions
(D18A "club starvation": paced captures below normal frame rate, output
queue starved, crackle).

## Reproduction (isolated, this machine, i7-5960X)

`/tmp` harness: Xvfb, software renderer, isolated cards
(`recomp/analysis/d08q-jetpack/`), F7 state slot 1 (level 1 turret room),
20–30 s of WASD + mouse sweeps, sampling the runtime's own telemetry
(`frame`, `audio_stats`, `phase_profile`, `phase_hot`, `ttk_input`).

| Build / mode | fps | audio underruns | output fill (target 180 ms) |
| --- | --- | --- | --- |
| `481dd2ec…` Modernized (jetpack build) | **47.5–49.3** | 600–780 in 20–30 s, continuous | **17–34 ms** |
| `481dd2ec…` Vanilla | 60.0 | 0 | 264 ms |
| Sep-27 baseline `a21f9478…` Modernized | 57.5 | 158 in 20 s | 38 ms |
| Fixed `81a4a909…` Modernized | **59.94** | **0** | **267 ms** |
| Fixed, jetpack flight | 60.1 | 0 | 250 ms |

Vanilla was clean and Modernized was not, so the cost was in the host
control layer. The Sep‑27 build (fewer guards, fewer hooks) sat between,
consistent with a cost that grew with every guard added since.

## Root cause

`ttk::identity()` (`modern_controls.cpp`) authenticates the guarded
original code before any host write. Its live compare
(`code_identity.h`) walked **every word of every guard on every call**:
106 guards, **81,220 bytes = 20,305 words**, each through
`psx_mod_read_word` → `psx_read_word` (guest-bus decode per word).
Measured with counters added to `identity()`:

* **46 calls per frame** (`movement_ready`, `locomotion_input_ready`,
  `jetpack_input_ready`, traversal/swim predicates, `player_identity_ready`
  … several are evaluated per `input_pad()` poll and per hook);
* **108 µs per call**;
* **≈ 5.0 ms per frame = 24 % of wall time** — 30 % of the 16.7 ms frame
  budget, on top of the emulation itself.

`weapon_aim.cpp` has a second `identity()` over 17 aim guards (12.6 KB)
that was not in that figure. The apartment substitute
(`apartment_identity_word`) had already been narrowed once for the same
reason ("identity runs many times per frame … stalls gameplay/audio"),
but the walk itself stayed.

When the frame overruns, the audio bridge is not fed: fill fell to
17–34 ms against a 180 ms target and the output underran every callback
— that is the stutter/crackle; the fps drop is the slowness.

## Fix (`code_identity.h`, `modern_controls.cpp`, `weapon_aim.cpp`, `pc_input`)

1. **Fast compare.** One `memcmp` per guard straight against the
   runtime's RAM image (`g_psx_ram`, memory.c) instead of 20k bus reads.
   A mismatch falls back to the exact per-word path (which honours the
   Modernized apartment word substitution) before a range is judged
   foreign. A full verification is now **~28 µs**.
2. **Per-frame verdict memo** (`IdentityMemo`). The verdict is reused
   only while both the host frame (`input_host_frame()`, advanced by
   `input_frame()` in the main loop) and the runtime's RAM-code generation
   `g_dirty_ram_code_gen` are unchanged. That generation moves on CD/EXE
   loads marking executable ranges (overlay loads), on clean→dirty page
   transitions of any store, on save-state restore and boot — so an overlay
   load between two hooks in the same frame still invalidates at once.
   Only a rewrite inside an already-dirty page is deferred to the next
   frame's full compare. Negative verdicts are memoised too; `refusals`
   still counts per call.
3. Same treatment for the aim guards.
4. Counters in the debug JSON: `identity_calls`, `identity_checks`,
   `identity_us` (controls), `identity_calls` / `identity_checks` (aim).

After: 46 calls/frame, **1.00 check/frame, 28 µs, 0.17 % of wall time**.

Tests: `ttk-controls-test` gained a memo case (same frame + generation
reuses the verdict; a generation bump or a new frame re-verifies; the
patched apartment pair passes through the fallback). The test mocks now
define `g_psx_ram` / `g_dirty_ram_code_gen` and model every store — and
every direct code poke — as a generation bump, the conservative reading of
the runtime rule. `ttk-input-test`, `ttk-aim-test`, `ttk-controls-test`
PASS.

## What this does not cover

* Renderer cost: probes ran the software renderer under Xvfb; the player
  uses OpenGL. The CPU-side cause is renderer-independent.
* `phase_hot` static ranking puts ~35 % of samples on the OpenBIOS
  exception epilogue (`0x29CC`) in both modes; that is attribution of
  post-exception time to the last stamped kernel function, not a new
  hotspot, and it was identical in the clean Vanilla run.
* D18A's historical "club starvation" captures were not re-run; the
  mechanism found here (Modernized frame overrun starving the bridge) is
  consistent with them but the club scene was not measured.
* Host profiling (`perf`) is unavailable (`perf_event_paranoid=4`); the
  runtime telemetry plus in-plugin counters were sufficient here.

## How to check again

`ttk_input` debug JSON → `controls.identity_checks` should advance about
once per frame and `identity_us` by ~30 per frame; `audio_stats` →
`out.fill_ms` should sit near or above `target_ms` with `underruns` flat.
