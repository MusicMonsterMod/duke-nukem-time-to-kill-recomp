# D17AR5 - Polymorph: cheaper redraw feed for 180 Hz in the heaviest views

2026-10-31. Goal (job board): slot 10's jetpack vista fell to half the refresh
at 180 Hz because each finished worker redraw cost the main thread about 3.5 ms
to draw into the GL renderer (note 341). Make the feed about a third cheaper,
or move it off the emulation thread, so the vista holds 180 with the game at
60. Status: Done (user-accepted 2026-10-31: "fully accepted/ lock it in,
close it out").

Evidence is local under `recomp/analysis/d17ar5/`: `measure.py` (vista still
and turning; feed and session ms per redraw, game fields/s, presents/s;
optional perf record or perf stat), `hold.py` (stand in the vista with the
default governor, per-interval rate and pace divider), `batches.py`,
`still_shot.py`, `routes.py` (D17AR3 sweep copy), perf data and logs. Every
run used private copies of the D17AQ fixture card and savestates, offscreen
video, dummy audio, internal scale 4, and hash-checked the player's saves and
profile before and after. Builds compared side by side as copies in
`build-local/` (`.ar5-base` = D17AR3 part 4 head, `.ar5-h` = final).

## Where the time went

Main-thread perf (LBR call stacks, slot 10's vista standing at 180, shedding
off). The feed was not one hot spot; it was per-word and per-primitive work
repeated about 46,000 GP0 words and 7,600 triangles per redraw:

- GP0 plumbing, about 20% of the feed: each word went through
  `gpu_set_gp0_source`, `gpu_write_gp0` and `gpu_write_gp0_body` (stack
  protector, phase bookkeeping) one at a time.
- Repeated per-primitive questions with the same answer: the display info key
  compare (48 bytes, several times per primitive, 6%), the sprite-tag table
  probe (up to 16 probes on every call, several calls per primitive), and the
  native-4:3 classifier (`gpu_ws_present_native_43`, RAM reads and stamps).
- The widescreen census ring (a debug-server diagnostic) recorded every redraw
  primitive again under the live frame number, with two scratchpad reads each.
- The GL upload: 88-byte textured vertices (about 2 MB per redraw) and the
  NVIDIA driver at 12% of the thread.
- Per session, outside the feed: two full 1 MB copies of the CPU VRAM array
  (save at begin, restore at end), about 0.2 ms of the 0.34 ms session cost.

## Changes (framework patch; the game sees the same pictures)

All are exact: each gives the same result as before, only cheaper.

- **Bulk GP0 feed** (`gpu.c` `gpu_write_gp0_words`, `render_replay.c`): a
  recorded run of words goes in as one call; a fixed-length command that
  starts idle and lies wholly in the run is copied into the command buffer and
  executed at once (source address, write count and collection state as the
  per-word path leaves them). Polylines, VRAM transfers and commands split
  across runs keep the per-word path.
- **Held display info during a feed** (`gpu_display_info_hold`): a feed writes
  GP0 only, so the display registers cannot change; the key compare is skipped
  while held. Two per-primitive helpers read the cached info in place instead
  of copying it.
- **Native-4:3 classifier memo during a feed**: no guest code runs inside a
  feed (RAM, tag and GTE stamps, frame and cycle counts are fixed); of its
  inputs only the overhang stamp can move (GP0), so the answer is reused while
  that stamp and the mode are unchanged.
- **Sprite-tag memo** (`psx_ws_prim_is_tagged`): the answer is reused while the
  command source, frame and tag table (new write counter) are the same.
- **Census ring records live primitives only**: replays redraw the same frame;
  the ring remains on for live frames (`ws_census` debug command unchanged).
- **Copy-on-write shortcut**: each primitive asks to save its draw area; the
  newest saved entry is checked first.
- **56-byte textured vertices** (was 22 floats, 88 bytes): texture page, CLUT,
  limits, depth, raw and semi travel as non-normalized integers, which GL
  converts to the same float values the shader already rounded; colour alpha
  uses the attribute default 1.0; depth, raw and semi share one attribute so
  every offset stays 4-byte aligned (no driver-dependent slow paths). Shader
  arithmetic is unchanged.
- **CPU VRAM saved only when written** (`gl_replay_cpu_vram_touch`): a replay
  session saves the CPU VRAM array just before the first thing that writes it
  (CPU-to-VRAM transfer, point write, pixel raster, GPU readback) and restores
  only then. Most redraws write none.

Tried and dropped: caching a quad's shared host-vertex lookups between its two
triangles (within noise).

## Results

Side by side, slot 10's vista standing at 180 Hz, shedding off (three
alternating pairs, desktop in use, load average 3-4):

| Build | Main thread per redraw | Feed alone | Game fields/s | Presents/s |
| --- | --- | --- | --- | --- |
| D17AR3 part 4 | 3.94-4.04 ms | 3.60-3.71 ms | 42.6-44.4 | 157-159 |
| D17AR5 | 3.06-3.08 ms | 2.93 ms | 59.1 | 178-180 |

The main thread's cost per redraw is 24% lower (session overhead 0.34 to 0.14
ms; feed 20% lower). Turning: 1.4 ms a redraw (was 1.7-1.9).

Default governor (player settings, AUTO-equivalent 180):

- Two long holds in the vista (75 s and 45 s): 180 presents every interval,
  game 59.6-61.6 (before: steps to 90 at once and stays; the 30 s retry failed).
- The governed route (load, 8 s, 20 s standing, 6 s turning): 4 of 5 runs held
  180 standing and turning with no step-down. One run stepped to 90 at
  exactly 57.0 frames/s a few seconds after the savestate load, then retried
  after 30 s. Unshed, the first seconds after a load run at 51-59 and settle at
  57.7 or more from the 3rd second, so the vista has a margin of about 1-3
  game frames/s over the governor's 57 line on this machine with the desktop
  busy.

Fixture route sweep (`routes.py`, all 10 slots, still / turn / walk, 6 s each,
one process with back-to-back loads and debug polling, so harsher than play):

- 120 Hz: 120 of 120 route seconds at full rate; 0 failed redraws, no host
  refusals or pieces handed back, redraw-only engaged on every route.
- 180 Hz: 111 of 120 (the D17AR3 part 4 build in the same conditions the same
  evening: 92 of 120, with fixtures 1, 2 and 9 stepping down). The misses are
  fixtures 1 and 9, each a still route straight after its load followed by
  the 30 s wait before a retry; 0 failed redraws.

Visual: the vista and the in-game menu over it look the same as before
(captures in `shots/`; the paused world advances a few frames between runs, so
an exact pixel diff is not possible: two baseline runs differ the same way).

Tests: `ttk-near-test`, `ttk-input-test`, `test_auto_frame_rate.py` pass.
`ttk-controls-test` did not link since D17AR3 part 2 (`live_frame_redraw_only`
had no test stub); the stub is added, and the test then stops at its existing
`movement_ready()` assertion (line 394) with the D17AR3 part 4 executable as
well, so that failure predates this job and is not renderer-related. Patch
re-exported and verified against the pinned clean framework.

## Limits

- Not tested on displays other than this machine's 180 Hz one; rates forced
  offscreen.
- The vista's margin is small (see above); a busier machine can still step it
  to 90 for 30 s after a slow second. The feed was not moved off the emulation
  thread (it shares the GPU state and GL context with emulation).
- Vanilla uses the same textured vertex format; the change is exact by
  construction and was not checked by a Vanilla pixel diff.
