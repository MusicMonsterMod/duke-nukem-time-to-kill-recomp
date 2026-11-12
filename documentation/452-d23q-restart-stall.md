# 452 - D23Q renderer restart stall after an early savestate load

Date: 2026-11-12. Status: Done (user-accepted 2026-11-12 after real-display
play with early loads: "all the save/load stuff is fine"). Lab: `recomp/analysis/perf/lab.py` cases `boot` and
`idle` (local; offscreen, private card copies, the player's own profile copied
read-only, player files hashed before and after every run).

## 1. What D23K saw

In 4 of 7 D23K lab launches on the real display (2026-11-11, windowed and
borderless), the game ended right after a savestate load at boot with
`starvation_watchdog: 4042946 us without heartbeat ... aborting`; headless
runs never did. The log (`recomp/analysis/d23k/real-win-s7.log`, local)
shows the order: the load, the redraw workers forked, then the abort, and
only after it the render thread's `renderer = opengl` / `GL GPU pipeline
ready` / `render thread(s) ready` lines. The same hour had 72
`nvidia-modeset: Idling display engine timed out` messages from the D23K
EDuke32 fullscreen churn (see the memory rule on real-display tests).

## 2. Cause (CONFIRMED in code and by a lab freeze)

- The "renderer restart" is the render threads' start (D17AW, D23O): on the
  first gameplay redraw, the emulation thread creates one hidden window and
  one shared GL context per render thread, and each thread then builds its
  own renderer (shaders included). Nothing else restarts. A normal launch
  starts neither the redraw workers nor the render threads during the
  opening (12 s observed, `idle`): both start on the first gameplay frame,
  which follows the player's load.
- The starvation watchdog is checked on the emulation thread itself, every
  65,536 guest cycles (`psx_advance_cycles_slow` /
  `psx_devices_service_to_now`). Its heartbeat comes from
  `debug_server_poll`. It is meant to catch a guest that runs without
  polling, but it measured wall time: when the emulation thread was blocked
  in host code for more than 4 s (here, the context creation on a display
  driver in trouble), the first check after the block aborted the game,
  after it had already recovered. The dump of the D23K abort has no ring
  entry after the last heartbeat: the guest did not run in those 4.7 s.
- Not the redraw workers: they set `psx_in_device_service`, whose fast path
  returns before any watchdog check.

Not established: why GLX context creation took over 4 s on the real display
that hour. Headless (EGL) the whole start takes about 75 ms (below); the
driver's modeset timeouts of the same hour are the likely cause, not proven.

## 3. Changes

1. **Watchdog counts guest time only** (`starvation_ring.c`). A gap of more
   than 250 ms between two checks is time away from the guest (host work, a
   stopped process, a menu); the heartbeat moves forward by the gap (capped
   at now), and gaps of 1 s or more are logged:
   `starvation_watchdog: emulation thread away from the guest for N ms (host
   work; not counted)`. Guest-side starvation is detected as before.
   Developer `PSX_HOST_GAP_LOG_MS` logs shorter gaps.
2. **Render threads start in the opening** (`render_thread.c`
   `psx_rt_prewarm`, called after the 30th present in `gl_swap_with_osd`):
   when the title has asked for them (Polymorph, Modernized), the windows,
   contexts and the threads' renderers are made half a second into the
   opening instead of at the first gameplay redraw after the player's load.
   Classic and Vanilla never ask for them (unchanged).
3. **Timings in the log**: `render threads: windows and shared contexts N ms`,
   `render thread k: renderer init N ms`, and the workers' fork time on the
   `render replay worker(s) started` line.

## 4. Evidence (offscreen, 180 Hz)

| Run | Result |
| --- | --- |
| `boot`, load at boot (UI slot 8), before the prewarm | workers fork 6.8 ms; windows and contexts 5.9 ms; renderer init 68.7 / 60.0 ms; no host gap of 30 ms or more |
| `idle`, normal launch, with the prewarm | `render threads started early (opening)`; windows and contexts 2.5 ms; renderer init 65.2 / 56.5 ms |
| `boot` with the whole process stopped twice for 6 s (SIGSTOP) in play, first fix (credit only when no heartbeat came after the last check) | first freeze survived, second aborted (`6007835 us without heartbeat`): the old failure |
| the same with the final fix | both freezes logged (6004, 6006 ms, not counted); the game ran on to field 1596 and quit cleanly |
| Vanilla, `idle` + `boot` | no render threads or workers; ran to field 1144 |

The lab's debug-port load comes before the 30th present, so in that test the
threads still start at the first redraw, which is fine now that a long block
no longer aborts.

## 5. Remaining

- Accepted on the user's real-display play (early and repeated loads, no
  abort) in place of 20 unattended launches, which are not run (they hung
  nvidia-modeset in D23K).
- If the context creation is slow again on the real display, the log now
  says how long, and the game continues.
- The D23K `starvation_dump.jsonl` beside the executable was overwritten by
  the lab's freeze test; its analysis is above.
