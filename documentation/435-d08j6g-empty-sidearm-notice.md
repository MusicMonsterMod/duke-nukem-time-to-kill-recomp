# D08J6G - Cannot shoot on a ladder: empty sidearm notice, centered toasts

2026-11-09. Status: Done, user-accepted 2026-11-10: "lock it in and close it out". Executable
`2a5d4b2a5d4f71207cebacc14cfc3dc70767dff66dcaaa253243c95c744a16dd`.

## Report

User, UI slot 7: "i cant shoot while on this ladder. stand up a job first,
and then investigate it".

## Cause (private copy of UI slot 7 = runtime slot 6)

- On the ladder: lower and upper clip 187, mode 3/3, holstered (equipment 0),
  selected weapon 5 (Combat Shotgun, 49 rounds).
- Desert Eagle (slot 4): inventory 9 (owned), ammo `+0x2c6+16` = 0.
- Climb fire draws only the sidearm (`climb_sidearm` = 4) and requires
  `usable_weapon(4)` (owned and ammo > 0). With an empty sidearm it refused
  silently, as the manual described. No regression: held fire on the copy
  gave no draw, no Cross, no ladder step-off.

User, after the cause: "so it was user error :')"; chose "Feedback only"
over firing the selected weapon from a climb.

## Change

1. `recomp/src/ttk/climb_fire.inc` (`climb_fire_request`): a new left click
   (edge, not held) while attached and holstered with no usable sidearm, not
   with E, holster or W-in-hang held, shows `DESERT EAGLE: EMPTY` (owned) or
   `DESERT EAGLE: NOT FOUND` for 3 s through `host_osd_push_centered`, the
   cheat notices' style, logs `[TTK climb fire] ...` and counts
   `climb_fire.empty_notices`. Modernized only (`attached_weapon_owner`).
2. User request: save, load and the other toasts centered too.
   `psxrecomp/runtime/src/host_osd.c` `host_osd_push` now sets the centered
   flag, so every toast uses the centered notice style and font. The death
   prompt still waits for a visible notice to end. Runtime patch re-exported.
   `host_osd.h` (comment only) is not a codegen hash source; saves load.
3. `tests/local/modern_controls_native.cpp`: stubs for
   `host_osd_push_centered` and `gpu_vram_peek` (the latter missing before
   this job) so `ttk-controls-test` links.

## Verification (`recomp/analysis/ladder-fire/`, private cards)

| Case | Result |
| --- | --- |
| Slot 7 copy, two clicks on the ladder | 2 notices, log lines; no draw, no step-off |
| Same, Xvfb window capture | DESERT EAGLE: EMPTY and LOADED SLOT 7 centered at the top, cheat font |
| Desert Eagle given 20 rounds, held fire | draws, 20 shots, back to the shotgun; 0 notices |

- `ttk-input-test`, `ttk-aim-test` pass.
- `ttk-controls-test` builds, but every local LEVEL00 fixture copy tried
  fails the overlay identity assert at start (fixture drift, not this
  change); not run to completion.

## Limits

- Ledge hang, chain, wall and monkey bars notice cases not run (same request
  path as the ladder; ceiling and hang call the same function).
- A sidearm emptied mid-climb with fire still held shows no notice until the
  next click.
- Every toast is now centered, including developer ones (fast forward,
  rewind, runtime settings).
