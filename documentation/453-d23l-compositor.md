# D23L - The desktop compositor: windowed against fullscreen

Status: measured on the real display with the user present (2026-11-12).
Job board: D23L. Evidence labels: CONFIRMED (our measurement), INFERRED.

## 1. Method

Same machine and display as [note 444](444-d23k-eduke32-parity.md) (i7-5960X,
GTX 1080 Ti, driver 580, one 1920x1080 monitor at 180 Hz, Linux Mint,
Cinnamon 6.4 with Muffin, X11). Whole-GPU use is clock-weighted
(`nvidia-smi` utilization x graphics clock / 1999 MHz, every 200 ms over
15 s), with `nvidia-smi pmon` per program.

- **Our game**: lab `recomp/analysis/d23k/lab.py` (local), `REAL=1`,
  `CASES=perf2`, the player's saved profile copied read-only (Relit, live
  shadows sharp, internal scale 3, AUTO = 180 Hz, window 1428 wide), private
  card copies, no console commands, turning all the time (`SPIN=dx=3`).
  Views: UI slot 2 and UI slot 8 (level 6), as note 444. Display set with
  `PRES={"display":"windowed"|"borderless"|"exclusive"}`.
- **EDuke32**: the player's build (r10669) in a scratch folder with copies of
  their settings and saves, `r_maxfps 180`, `r_vsync 0`, a window of the same
  size as ours (1428x802) or fullscreen 1920x1080, Polymost and Polymer, in
  game on E1L1 and E1L2 (`-v1 -lN -s2`), turning all the time (the Left key
  held through XTEST). Frames counted with an `SDL_GL_SwapWindow` shim: 179-180
  a second in every run. Pictures checked in a shot at the start and end of
  each run (in game, the view turns).

## 2. Results (CONFIRMED)

| Game | View | Windowed | Borderless | Exclusive | Windowed minus fullscreen |
| --- | --- | --- | --- | --- | --- |
| Ours | UI slot 2 | 31.4% | 26.6% | 26.8% | 4.8 |
| Ours | UI slot 8 | 28.7% | 23.7% | 24.4% | 5.0 |
| EDuke32 Polymost | E1L1 | 8.2% | 4.5% | - | 3.7 |
| EDuke32 Polymer | E1L1 | 8.3% | 3.7% | - | 4.6 |
| EDuke32 Polymost | E1L2 | 8.4% | 3.7% | - | 4.7 |
| EDuke32 Polymer | E1L2 | 12.5% | 6.2% | - | 6.3 |

Our game held 180 presents a second in every run. With our window, pmon shows
`cinnamon` at 9-12 and `Xorg` at 4-6; in borderless and exclusive fullscreen
`cinnamon` is absent (Muffin unredirects the window). The same holds for
EDuke32: `cinnamon` 2-7 windowed, absent fullscreen.

So the compositor costs our window 4.8-5.0 points of whole-GPU use, and
EDuke32's window of the same size 3.7-6.3 (mean 4.8). The cost is the
desktop's, set by the window's area and the refresh rate: a window costs
the same whatever draws into it (INFERRED from the equal figures). D23K's
5-6 against 3.6 compared our 1346-wide window with EDuke32 at 1280x720 and
in its attract demos; at the same size the difference is the same.

Fullscreen draws a 1920x1080 picture against 1428x802 windowed, so the
true compositor cost of the window is somewhat larger than the difference
(both games alike).

## 3. What the compositor needs to step aside (CONFIRMED)

Cinnamon's `unredirect-fullscreen-windows` setting is off on this desktop,
so Muffin bypasses only a window that asks (`_NET_WM_BYPASS_COMPOSITOR = 1`,
which SDL sets on every window, SDL2 and SDL3 alike) and covers the whole
monitor. A maximised window (1920x1020 under the panel,
`_NET_WM_STATE_MAXIMIZED_*`) is still composited. Our borderless and
exclusive windows are real fullscreen windows and are bypassed.

A lab pitfall, recorded so it is not repeated: an invalid `display` value
in a settings file (for example `fullscreen`) makes the launcher restore the
presentation defaults (window width 0, which maximises), so the run looks
like fullscreen but is a composited maximised window. The first round of
this measurement made that mistake; its figures are not used.

## 4. Conclusion

The windowed cost is the desktop compositor's and matches EDuke32's at the
same window size; there is nothing in our window to cut (no extra windows:
the two render-thread windows are 16x16 and never mapped; the game window has a depth-24 visual, no alpha). Borderless or exclusive fullscreen avoids it, as GAME_MANUAL
says; GAME_MANUAL now adds that a maximised window is still a window. The
output pass half of D23L was done with D23M (0.11 -> 0.04 ms a redraw,
[note 448](448-d23m-direct-redraws.md)).
