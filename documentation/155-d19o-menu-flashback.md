# D19O - Menus no longer step back an image after a stall or a demo/logo switch

Job: [D19O](../MODERNIZATION_JOBS.md), round 2 also [D19L](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "accepted! lets bring this one to a solid close"). D19L accepted with it.
Found in [D19J](150-d19j-title-menu.md#limits). Builds on
[D19C](142-d19c-menu-response.md) (redraws from the newer image) and
[D19E](143-d19e-menu-closeout.md) (the same over the loading logo).

## What was really happening

D19J measured PRESS START -> main menu as 6-7 presents of the menu, then 13
of PRESS START, then the menu, in our menus and the original ones. That was
measured with 24-present `replay_dump ...-seq` dumps taken back to back.
Each one writes its PNGs after it is collected, which stops the game for
about 0.8 s, and the flash always came right after that stop.

With one stall-free 96-present dump (`-seq96`, below) there is no flash in
steady play: PRESS START -> menu and cursor moves on the original main menu
and on D19J's menu, at 120 and 180, are clean.

The flash is real, though, whenever the replay timeline resets in a menu:

1. A reset happens after a host stall (the guest clock drifts more than
   100 ms from the host), and at every switch between demo images and
   loading-logo images (demo -> blocking load, gaps in the logo, logo ->
   demo; four times in one demo scene load). The reset drops every captured
   image and the image last shown.
2. No pair of images exists until the next guest image, about 100 ms (6
   fields) later. The presenter then shows the guest's real image.
3. The real image is behind the redraws. D19C/D19E draw the redraws from the
   newest captured image. The real image shows the same change one to two
   guest images later: with the redraws withheld (`DNTTK_REPLAY_NOFEED=1`),
   a cursor move reached the real image 22 presents after it reached the
   redraws (120 Hz).

So for about 12 presents the screen stepped back to an older image. After
PRESS START that was PRESS START again. After a cursor move it was the old
highlight. Over a demo scene load it was the older logo image and one black
present.

A plain `SIGSTOP` of 0.8 s around the press reproduces it without the dump
tool. The fix is not an artifact of the measurement.

## What changed

`recomp/src/ttk/frame_replay.cpp` (Modernized, frontend images in game mode 0
only):

- `menu_alone`: while the timeline has no pair, the newest captured menu image
  is redrawn on its own (both ends of the pair are that image, so the
  camera, objects and logo angle are its own). When the next image arrives,
  normal pairs take over at the same pose.
- `reset_timeline(keep_shown)`: across a reset between menu images (no
  savestate load, no switch to or from gameplay) the last redraw stays
  available to repeat until the first single-image redraw is ready. Its cache
  entry is never reused while it is the last shown.
- `DNTTK_MENU_ALONE=off` (developer) restores the real-image fallback.
  `render_replay` -> `plugin.frontend` gains `kept` (resets that kept the
  last redraw) and `alone` (single-image redraws submitted).

Gameplay, gameplay level loads, savestate loads, the 60 Hz presentation and
Vanilla are unchanged. The guest, its pace, input and the attract demo are
untouched.

Framework (exported to the tracked patch): `replay_dump path=<base>-seq96`
collects 96 presents at a quarter size in one sequence and writes them
afterwards. Each sequence texture now remembers its own size. A sequence
still stalls the run while it writes, so back-to-back sequences straddle a
stall. No hashed header changed, so savestates still load.

## Evidence

Executable `226719899432b4b43694269a108943d1823e25fbe87c13089489278d64af5161`.
Offscreen on the real GPU, 16:9, 4x, private profile, private cards and the
D19J savestates (`recomp/analysis/d19o-redraw`, local: `t.py`, `matrix.py`,
`hl.py` reads the highlighted main-menu line of each present, `sheet.py`).
"Real" means the presenter showed the guest's real image instead of a redraw.

**Stall at the menu change** (`SIGSTOP` 0.8 s at 0.2 / 0.25 / 0.3 s after
Enter on PRESS START, 96 presents across it):

| Build | Runs | Flash back | Real presents |
| --- | --- | --- | --- |
| D19O, original and D19J menus, 120 | 6 | 0 | 0 |
| D19O, original and D19J menus, 180 | 2 | 0 | 0 |
| `DNTTK_MENU_ALONE=off`, both menus, 120 | 6 | 6 (1-14 presents of PRESS START) | 11-13 |
| Dump stall (D19J's method), D19O / off | 1 / 1 | 0 / 1 (13 presents) | 0 / 12 |

**Demo scene load behind the main menu** (slot 3; a Down tap just before each
switch; two tap phases):

| Switch | D19O real presents | Off real presents | Off: the newer image reaches the screen |
| --- | --- | --- | --- |
| Demo -> blocking load | 0 | 9-10 | - (the menu is hidden there, as in the original) |
| Blocking load -> logo with menu | 14, all while the menu is hidden | 31-34 | 14 presents later |
| Gap within the logo | 3, showing the current highlight | 14-15 | up to 12 presents later |
| Logo -> demo | 0 | 12-14 (with one black present) | 13 presents later |

No run returned to an earlier highlight with D19O.

**Steady play** (no stall): PRESS START -> menu and one cursor move on both
menus at 120 and 180 change once, every present is a redraw. At 60 there
are no redraws, so the flash cannot occur.

| Check | Result |
| --- | --- |
| Main menu latency, 16 Down taps at 120 | press to guest 68 ms, to screen 106 ms mean (173 max); off: 73 / 112; D19E: 75 / 110 |
| Smoothness at 120 / 180 (demo and menu) | 118.3-120.0 / 178.3-179.0 distinct presents a second, guest 9.9 images |
| Gameplay (slot 5) | 117.3 distinct presents a second at 120, guest 29.5; `kept` and `alone` 0 |
| Vanilla | original menu with pad presses (PRESS START, Down, TWO PLAYER), no replay presentation |
| Tests | Python 146 OK with the disc image, `ttk-input-test`, `ttk-live-env-test`, `check_repo.py`; the patch applies to the pinned framework and reproduces all 47 modified/new framework files |

## Limits and findings

- Accepted by the user on their display (2026-10-19).
- While a demo scene's load blocks (sub-state 0), the logo images there are
  never paired and are shown as real images, as before. The menu is hidden
  there in the original, so nothing steps back.
- At a long gap between logo images, 3 real presents remain. They showed the
  current highlight. Likely cause, not confirmed: the presenter stops
  redrawing once the last logo image is more than 15 fields old (D17U), as
  before.
- The original menus draw a cursor move one guest image after the cursor
  variable changes (the redraw of the image that first carries the new
  cursor still shows the old highlight). D19C/D19E `screen_ms` counts from
  the variable, so the real press-to-highlight time is about one image
  longer than those tables say. That is the game's own behaviour and is
  unchanged.
- A stall of the host itself still freezes the picture while it lasts.

## Round 2: the user's playtest (2026-10-19)

User: "when the scene changes or you reach a loading screen, there is
definitely still that judder ... you lose your hook into the menu
momentarily", and holding Down "when the background demo scene changes, it
stops the menu item changing". Measured on D19J's menu (the player's),
slot 3, Down held or tapped across a demo scene load:

| Found | Cause | Change |
| --- | --- | --- |
| The menu vanished for about 100 ms (12-13 presents) where the logo comes back after the blocking load, and for about 200 ms (24 presents) as the demo fades back in | At both hand-overs the guest builds the same image twice and shows the second pass. Our menu drew once per display buffer (`0x800bdbd0`), so the second pass, which covered it, was left without it (one skip per hand-over in the new `skips` counter) | `game_menu.cpp` `draw_page`: a new call site in the same image draws again |
| The logo stepped back and the menu came late at the same hand-over | The logo images pause more than 15 fields there; the presenter left the replay and showed the real image (about 15 presents) | `frame_replay.cpp` `menu_hold`: within such a gap (game mode 0, the demo scene still loading, up to 5 s) the last redraw is held. `plugin.frontend.held` counts it |
| Held Down stopped moving the cursor for the blocking load, about 1.9 s | The game reads no pad there, and our menu only read keys at the game's pad reads; title-page moves also waited for a normal tick | `pc_input.cpp` `input_menu_poll` reads our menu's keys at the load loop's backdrop (only while owned and the scene's blocking load runs); `title_tick` runs Up/Down/Left/Right there with their sound. Confirm and Back still wait for a normal tick |
| After that pause the cursor ran on several items at once (1 -> 2 -> 4 within 0.2 s) | The 80 ms key repeat replayed the steps it had missed | The repeat goes on from now after a gap |

A permanent stop of the held key did not reproduce: with the key held through
the X server's own auto-repeat (`xdotool keydown` on a private Xvfb display)
and with key edges only (`DNTTK_TEST_EDGES`, new, diagnostic), the cursor
kept cycling before and after four demo scene loads (5 minutes from a cold
boot). What did reproduce is the 1.9 s pause above.

Evidence, executable
`114a77530e92839c095aeb833078a84823b312417e7256c5c476460c1d7f38b2`
(16:9, 4x, 120 unless noted):

| Check | Result |
| --- | --- |
| D19J menu, the four switches of a demo scene load, a Down tap at each (120, and 180 at logo -> demo) | menu on every present, highlight moves once, all presents redraws |
| Same, before round 2 | menu missing 12-13 presents (logo back) and 24 (demo back) |
| Draw skips through a load (`plugin.frontend.game_menu.skips`) | 0 (before: 1 per hand-over) |
| Held Down through a load (cursor changes) | 17 during the blocking load, 16 over the logo (before: 0 for about 1.9 s) |
| Three taps during the blocking load | ONE -> TWO -> LOAD GAME -> OPTIONS on screen |
| Load timing with Down held (blocking load end / demo back) | 2.32 / 8.21-8.23 s, unchanged |
| Original menus across the load | as the original (hidden during the blocking load), no step back |
| Stall at the menu change, steady menus, latency (111 ms mean), smoothness (119.5-120), gameplay, Vanilla, D19J page walk, pause OPTIONS | unchanged |
| Tests | Python 146 OK, `ttk-input-test`, `ttk-live-env-test`, `check_repo.py`; framework unchanged since round 1 |

Accepted by the user on their display (2026-10-19). D19J's own menu only: the original menus
(`DNTTK_GAME_MENU=off`) still hide during the blocking load, as the original.
