# D08J4 - Climbable ceilings: camera-relative travel, no mid-span drops

Status: **Accepted** (2026-10-07, user: "it's a complete winner for me. I totally accept"). Modernized only; Vanilla unchanged.

User report (2026-10-07): "duke is supposed to be able to jump and climb across
this ceiling like monkey bars but he just randomly falls." With the video
`research/screencaps/Video_2026-10-07_17-49-43.mp4`: "I firstly just intended
to jump to the ledge, but Duke grabs the ceiling. this is fine ... The ceiling
monkey bar stuff is all tank controls! you absolutely do not move the way you
want, you move the way duke is facing ... i couldnt actually make it to the
ledge as duke then just falls off."

Test location: the user's UI slot 3 (savestate file 02), re-saved at the top of
the medieval chain, SHA-256
`a519f8bc0c2f8f23c2c61283793a548f8d2ede687cdd810e0c98f512eff262a6`. Private copy
in `recomp/analysis/d08j4-20261007/cards` (the player's file was only read).

## The original ceiling hang

- A jump with E (Cross) held under the grate catches it: mode 7, animation 149,
  `+0x17c` = the grate object (type 759, flags `0x101092`). Several grate
  objects tile the ceiling; `+0x17c` moves to the next one as Duke travels
  (`0x801ddee4` -> `0x801dde24`, `0x801ddc44`, `0x801ddd64`).
- `0x80055208` enters mode 7 for any object whose type flags hold `0x80`
  (`0x800553e4`); crates and the dumpster carry `0xc0`.
- Controls are tank controls (raw D-pad through the debug port, Modernized with
  no keys held): Up 150 frames carried Duke 1,400 units along his facing with
  the hand-over-hand 150/151; Down travels backward; Left/Right only turn him
  on the spot (yaw 30 -> 3984 / 150, position unchanged). At an end he stops in
  149 and keeps hanging. Only Square lets go.

## Cause of the falls: ours (D08X)

`object_hang_update()` (D08X, for a crate-stack hang with no pull-up) let go of
any mode-7 hang when S was held, or when W had been held for 40 updates
without a climb. On a ceiling, W is how you travel, so Duke dropped after
about two seconds of travel (reproduced: W 120+ frames -> 108 at +126,
`hang_releases` +1; raw Up for the same time never let go). S dropped him at
once. The crate case had since gained an immediate release (flags `0xc0`), so
the stall and S releases only ever applied to ceilings.

## Change

- `ceiling_hang.inc` (new): `ceiling_hang_ready()` = Modernized traversal
  input, mode 7, animation 149..151, object flags `& 0xc0 == 0x80` (not a
  crate). In the player update (`0x8005a210`, before the mode-7 handler), the
  host turns Duke toward the camera-relative WASD direction at 320 yaw units
  per update (writes `+0x1c`/`+0x24`) and asks for the original Up once he is
  within 640 units (~56 degrees) of it. Ctrl asks for Square. The original
  still moves him, follows the grates and stops him at the ends.
- `pc_input.cpp`: on such a hang the D-pad carries only the host's Up (or
  Square for Ctrl); A/D never reach Left/Right, S never reaches Down. Space
  stays the original Square (let go).
- `ledge_reach.inc` `object_hang_update()`: releases crate-type hangs only; the
  stall and S releases are gone.
- Counters: `ttk_input` -> `controls.ceiling` (`ready`, `updates`, `turns`,
  `advances`, `drops`).

## Evidence (binary `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`)

Private Xvfb runs, real keys, Modernized third person (`ceil.py`, `sweep.py`,
`gap.py`, `diag.py`, `van.py`):

| Case (120 frames held) | Travel relative to the camera | Result |
| --- | --- | --- |
| W | -8 degrees, 1,377 units | still hanging (mode 7), no release |
| A | -98 | still hanging (stopped at the grate end) |
| D | +84 | still hanging |
| S | 176 | still hanging |
| W+D | +41 | still hanging |
| Camera turned 38 degrees with the mouse, then W | -8 | still hanging |
| Ctrl / Space | - | let go and land (mode 0) |

- 8-direction sweep, 240 frames each: every direction stops at the grate's end
  and keeps hanging; Ctrl then drops him. Over the chain-top platform he lands
  on it (y about -505); beyond its open side he drops to the lower level (y
  about 2650) or the pit floor (y about 5900).
- Vanilla: the host code never runs (`ceiling.updates` 0). A scripted Vanilla
  catch of the ceiling was not reproduced, so the original's behaviour there
  comes from the raw-pad Modernized runs above.
- Suites: `ttk-input-test` (new D08J4 case: no D-pad from WASD on a ceiling,
  host Up and Ctrl Square; ladders unchanged), `ttk-controls-test`
  (`d08-camera-final` LEVEL00 fixture, LEVEL01, all levels; 37 PASS groups),
  `ttk-aim-test`, `ttk-near-test` PASS; Python 121 OK (7 skipped: 2 owned-disc,
  5 UI-art tests that need Pillow, absent from `.venv`).

## Limits

- The red-carpet ledge the user aimed for was not located offscreen; whether a
  drop from the grate's end reaches it is for the playtest. If it does not, a
  follow-up (for example a forward swing or drop-to-ledge at a ceiling end)
  needs its own design.
- One ceiling (the slot-3 grates) was played. Other climbable ceilings share
  the state and flags test, not individually played.
- First person: the ceiling hang keeps the traversal camera as before.
- Not changed: crate hangs (D08X immediate release), poles and chains (mode 3).
