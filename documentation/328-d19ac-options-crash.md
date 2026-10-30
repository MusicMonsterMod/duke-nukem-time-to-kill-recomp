# D19AC - Crash on opening OPTIONS from the pause menu

2026-10-30. Reported during the D17AN batch playtest and fixed at once on the
user's instruction. No commit, push or acceptance implied.

## Report

User (speech to text, paraphrased): load UI slot 11 from the F7 menu, press
Escape, choose OPTIONS: the game crashes, every time. Also with UI slot 2.
Waiting about 30 seconds first did not help. Once it did not crash.

## Evidence

The session logs (`recomp/build-local/logs/`) end silently after
`Mouse released (Escape)`. `coredumpctl` lists five SIGSEGV dumps of the
player build between 07:22 and 07:25, all at the same address:

```
#0 view_slider (r=...)            game_menu.cpp:781
#1 ttk::game_menu_input (keys=...) game_menu.cpp:1492
#2 ttk::menu_owns_keys ()          pc_input.cpp:1335
#3 ttk::input_pad ()
... pad sampling on the vblank edge
```

The keys show `confirm` held: this is the first menu tick after OPTIONS was
chosen.

## Cause

`screen` is -1 while the OPTIONS list itself is shown (its entries open the
settings screens 0-4). The mouse-wheel check runs on every menu tick:

```
const bool camera_slider=page==Page::Options && view_slider(screens[screen].rows[cursor]);
```

Before D19AB (71409fa) it read `screen==2 && screens[screen]...`, which
short-circuited on the list. D19AB keyed the slider layout on the two rows
instead of the screen and dropped that guard, so the list read `screens[-1]`,
the heap memory just before the vector, and then a `Row` from a pointer found
there. Whether that crashed depended on what the heap held, which explains
why it crashed nearly always after a load but not always. Every other
`screens[screen]` use is already guarded by `screen<0` checks.

## Fix

`game_menu.cpp`: `page==Page::Options && screen>=0 && view_slider(...)`. The
wheel on the OPTIONS list moves the cursor as before; on DISPLAY it still
adjusts the two view sliders.

## Validation

- Repro `recomp/analysis/d19ac/repro.py` (offscreen, private copy of the D17AM3
  playtest fixture, `DNTTK_TEST_UNCAPTURED`, player files hash-checked
  unchanged): load the slot, Escape, down twice, Enter, then walk the list.
  The menu state is read from `render_replay` after each key.
- Previous player build (`build-local/*.an-cand2`): died on Enter in 5 of 6
  runs on UI slots 11 and 2, plus 2 of 2 logged runs. Same stack as the
  player's dumps.
- Fixed build: 10 of 10 runs (UI 11 and UI 2, five each) reached the OPTIONS
  list (`page 0`, `screen -1`) and survived eight more cursor moves.
- `recomp/tests/local/test_pc_input.py`: new source guard on the wheel check;
  18 tests pass.

## Limits

- Wheel adjustment of the DISPLAY sliders was not re-driven (the test input
  has no wheel); its code path is unchanged for `screen>=0`.
- The user's live check remains.
