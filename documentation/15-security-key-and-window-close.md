# First-map security-key freeze and unresponsive window close

## Report and status

The user played into the first map, inserted the security key, and the game
froze. Clicking the window's close button did not close it.

**Latest user verification: security-card insertion succeeded, the player
proceeded to the next area, and the window × closed the game cleanly.** The
diagnostic run report records `reason=atexit`, `exit_origin=sdl_window_close`,
at frame 12144; no game process remained when checked.

Window shutdown is fixed and tested. The earlier gameplay freeze was not
reproduced in this run; its exact cause remains unknown. This successful
playthrough is progression evidence, not proof of a specific gameplay repair.

## Evidence preserved from the failed run

The process was no longer alive when investigation began. Its log ended with
top-level execution returning PC `0x00000000`. The preserved exit report is
`reports/security-key-original-exit.json`:

- Final return address: `0x80025C04`.
- Final stack pointer: `0x801FFF50`.
- Function-trace sequence: zero; tracing was not enabled.
- Stack words were present, but general registers and full RAM were absent.

Disassembly places that return address after the indirect call at `0x80025BFC`
inside `0x80025AC8`, the resident object-update path. It reads the object's byte
at offset `0x15`, indexes a callback table through the pointer at `0x800DD850`,
and calls the resulting handler. This narrows the investigation to object
dispatch. It does **not** establish whether the selected pointer was null or
a called function incorrectly returned without publishing the next guest PC.
Both are consistent with the limited old report.

No callback is skipped or replaced with a no-op. Such a workaround could hide
the stop while preventing the door/key script from executing correctly.

## Why closing could hang

There were two independent host-side problems:

1. Fatal halt kept the TCP debugger alive but never pumped SDL events, so the
   window-manager close request could not be handled once emulation stopped.
2. Shutdown closed the debug listener and joined its I/O thread. On this Linux
   run, closing the descriptor alone did not wake the worker blocked in
   `accept()`. A connected client without a complete request could also block
   the worker in `recv()` indefinitely.

The runtime now registers a host-only event callback for fatal halt. That
callback handles window close without advancing or resuming guest execution.
Shutdown hides the window and stops host audio before potentially slow capture
I/O. It calls socket `shutdown()` to wake the listener before joining and closing
the descriptor. Accepted sockets have bounded receive/send waits; partial
requests are retained across receive timeouts, with shutdown checked between
waits. Memory-card flushing and capture cleanup remain part of shutdown.

## Verified close behavior

`tools/local/test_window_close.py` launches isolated test instances and sends
the window manager's close request using `wmctrl`. It verifies termination of
the process, not just removal of the window. The fatal case uses GDB to invoke
the real `psx_fatal_halt` after host initialization; it does not simulate a halt
by stopping the process externally.

| Test | Time to exit | Result |
|---|---:|---|
| Healthy running guest | 0.116 s | Exit 0 |
| Fatal-halted guest | 0.116 s | Intentional exit 1 |
| Connected debugger with incomplete JSON | 0.266 s | Exit 0 |

Evidence: `reports/window-close-regression.json`; detailed local test logs are
in ignored `recomp/analysis/security-key/close-*/`.

An initial synthetic-recursion probe did not reach a controlled halt on Linux;
the framework's native stack guard for that probe is Windows-specific. It is
not counted as a passing test. An earlier controlled halt test reproduced the
blocked listener join before the socket-shutdown correction.

These tests cover normal shutdown and a serviced fatal halt. An arbitrary
host deadlock that cannot reach the event loop is not proven closeable.

## Capturing the security-key failure

From the workspace root:

```sh
python3 recomp/tools/local/run.py --diagnostics
```

This enables the dispatch trace and keeps an abnormal top-level exit available
to the debugger. The runtime now writes all 32 general registers into
`recomp/psx_cps_exit_trace.json` and saves 2 MiB of guest RAM to
`recomp/psx_cps_exit_ram.bin` on such an exit. Those files are local game-data
artifacts; retain them locally. Reproduce key insertion, then leave the stopped
instance available for inspection before starting another run.

`tools/local/inspect_object_exit.py` decodes the object/state/table slot from
that snapshot when the return address matches this failure path. It fails on
the older incomplete register dump rather than inventing object state. Its
report is evidence for the next investigation, not a gameplay repair.

If the freeze recurs, determine the selected object state and expected handler,
inspect recent dispatches and overlay replacement, and establish the first
divergence. The latest successful replay does not require another reproduction
now; keep the diagnostic tools available while testing further progression.

## Reproducibility

The host fixes and additional capture are preserved in
`recomp/patches/time-to-kill-stopped-window.patch`. The normal build wrapper
applies every reviewed `time-to-kill-*.patch`, recognizing already-applied
patches and rejecting conflicts. The earlier CD voice-seek correction remains
in its separate patch. No generated game instructions were manually edited.

## Later column-area report

The newer column-area exit includes full RAM and establishes a stale scene object
inside reused animation memory. A guarded scene-reset cleanup and its limits are
documented in [the column investigation](29-column-exit.md). The older security-key
capture lacks RAM, so it still does not establish the same exact cause.
