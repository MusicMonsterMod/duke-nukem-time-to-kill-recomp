# D08Z3 - Modern landing (no stand-still after a jump)

Status: **Done** (2026-10-08, user-accepted: "Completely accept this work as complete, working, and done."). Modernized option `landing`,
`modern` by default; `original` and Vanilla keep the original recovery.

## User report

At D08Z1 acceptance (2026-10-08): "there is the occasional jump where duke
lands and he's stuck for a second before being able to move again."

When selecting the job (2026-10-08): "it may not actually be the bump causing
it because it happens also when you jump without run on for example. if you
jump forward, when you land, there's that horrible pause when you land before
duke starts moving. it's all clunk. I actually want that entire mechanic to be
totally smoothed out and modernized so that the flow feels much more natural".

## What the original does (static analysis of the owned executable)

The airborne handlers land through the selector `0x80054c04` (callers
`0x80055a64`, `0x80055cf4`, `0x80055728`, `0x8004ad84`); its return value is
written to the lower animation `+0x60`:

| Condition at the landing update | Result |
| --- | --- |
| Water surface reached | 127 (swim entry) |
| Health 0 | 227 (death) |
| Fall speed above the damage threshold | fall damage `0x80054af8`, rumble, 106 (hard landing; 158/161 from 157/160 rolls) |
| Run input (`0x800d14f8` table) and flight 99/100/108/110/111 | 85; other flights 84 (`+0x228` bit 3) |
| Direction input (`0x800d1ce8` table) and the flight `+0x23e` was 98 (directional), 103/104 (running) or 108 (short fall) | 78, run continuation (`+0x228` bit 3) |
| Jump input (`0x800d1b88` table), no jetpack thrust | 96, the next jump |
| Anything else (straight-up jump 97, a bounce 107, nothing held) | **105**, recovery |

The ground dispatcher `0x80048410` (jump table `0x800140a8`) runs 105 and 106
through the landing-pose handler `0x80052fb8`: floor collision `0x800788e0`
and the animation's own small root step, no input. Duke leaves 105 only when
the animation ends (about 37 fields at 60 Hz, 0.6 s). So:

- a straight-up jump (Space alone) always lands in 105, even with W held all
  through the flight (manual style steers it in the air);
- a directional jump whose direction was let go before the touch-down, even
  for an instant, lands in 105;
- a direction pressed at or after the touch-down is ignored until 105 ends;
- a jump press during 105 is lost (the host's 130 ms jump buffer lapses).

This is the original game's behaviour (reproduced in Vanilla: Square, then Up
held from mid-flight, 105 for about 37 fields), not a D08Z1 side effect.
Landings with a direction held through a directional or running jump already
continue straight into the gait, in both modes.

## Change

`recomp/src/ttk/landing.inc` (included from `modern_controls.cpp`), at the
existing ground dispatcher hook `0x80048410` (ra `0x8004b634`), before its
handler for `+0x60` runs:

- `+0x60` is 105, mode 0, previous mode 0 (the update after the touch-down),
  Modernized, guards and identity ready, no crouch, no precision aim, not in
  wade water, `+0x224` clear of `0x202002c1`;
- a direction held: 72 (walk) or 76 (run, `input_running()`), frame 0
  (`+0x68`/`+0x6a` = 0), the same start the existing gait switch uses for
  70/71. The original walk/run handler, collision and stride run in that same
  update, and the host's camera-relative redirect applies from it;
- no direction but a jump pending: 63 (stance), so the host's buffered press
  reaches the original standing jump on the next update;
- nothing pressed: 105 plays as before (the landing pose stays).

A first prototype fast-forwarded 105 at the lower-body runner (as the manual
quick takeoff does for 96); it compressed 105's own root step into one update
(a visible 60-80 unit lurch) and was replaced by the direct start.

Unchanged: the landing selector and its sound, takeoffs, flights, fall damage
and the hard landing 106, water entries (127/128), slope-slide landings keep
the camera latch, and the continuation landings above.

Option: profile schema 31 `controls.landing` `modern` (default) or
`original`; `run.py --landing`, `--settings` choice L, `DNTTK_LANDING`
(always `original` for Vanilla). No new hook, no generated code change,
codegen hash unchanged (savestates load). Guard added for `0x80052fb8`
(128 bytes); the dispatcher (`0x80048410`) and its 105 table entry
(`0x80014148`) were already guarded.

Diagnostics: `ttk_controls.landing` (`modern`, `recoveries`);
`DNTTK_TRAVERSAL_TRACE` prints `ttk-landing recover` lines.

## Evidence

Private copies of the player's savestates and private cards only
(`recomp/analysis/d08z3-landing`, Xvfb, dummy audio), the player's
Modernized settings (manual jump, first person, walk by default) at internal
scale 1. `land.py`: fields from the touch-down (first update out of mode 9)
to the first field with horizontal motion; real keys through xdotool.

| Jump (UI slot 7 portal room / UI slot 10 western street) | `original` | `modern` |
| --- | --- | --- |
| Space alone, W pressed at the touch-down | 40, 44 / 36, 36 | 5, 8 / 3, 3 |
| W + Space, W let go in the air, pressed again at the touch-down | 40, 41 / 37, 38 | 6, 8 / 3, 6 |
| S + Space (slot 7: straight up, the back jump was refused), S held | 42, 42 / 2, 3 | 7, 7 / 3, 3 |
| Space alone, Space tapped at the touch-down | no second jump / no second jump | 96 at +6..7, airborne at +12 (4/4) |
| W + Space, W held | 3, 4 / 3, 3 | 2, 5 / 2, 3 (identical path 78 -> 74) |
| D + Space, D held | 4, 7 / 3, 3 | 3, 2 / 3, 3 |
| Shift + W running jump, held | 2, 3 / 3, 4 | 2, 3 / 3, 3 (78 -> 76/78) |
| Space then W during the takeoff, W held | 5, 2 / 4, 4 | 2, 2 / 2, 3 |

With `modern` 105 lasts the touch-down update and one more (2..3 fields),
then 72 and the normal walk (about 10 units per field); no lurch. Vanilla
check (`vanilla.py`): Square, Up held from mid-flight, 105 then 76/78 after
about 37 fields, as described above.

Hard landing 106: not reproduced in play. The slot-5 pit is water (127/128),
and a height fixture (writing `player+8`) is snapped back to the floor; 106 is
left original by design (it only follows a damaging fall).

Regression routes (`regress.sh`: D08X/D08Y/D08U/D08W/D08Z, `modern` against
`original` on the same binary, executable
`e5e416a7999de693f79236af5f897fc9f8fcf00ead208735b3741bc149ae7fb3`, jump walls `slide`):

- Slot-12 1024 wall: W+E+Space grabs and pulls up in both; W+Space lands at
  the foot of the wall in both.
- Slot-12 `cases2`: holstered E jumps catch in both; the tall wall never
  grabs. `pit_jump_e` missed 3 of 6 `modern` reps (two reruns): each miss
  grabbed a ladder with W+E before Space (156, then exit 190), the flake
  already noted in D08Z; no jump or landing took place. `original` 2/2.
- Crates, crate-to-crate, D08X angles, D08W wade jumps and ledge sweeps,
  D08U ladder top, furniture run-offs, slot-5 gap (both jump styles): same
  outcomes; the differences are the run-to-run timing already recorded for
  these routes (angle catches at -10/+15, crate B, one-frame press timing).
- The `original` furniture pass first stopped on a harness debug timeout
  ("emu busy or frozen"); its rerun matches `modern`.

The final build adds only the `0x80052fb8` guard (executable
`11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e`);
`land.py` on UI slot 10 again: Space alone and released-W landings move
after 3 fields, the landing hop takes off, W held unchanged.

Native: `ttk-controls-test` 46 groups; the new D08Z3 group covers walk, run
and strafe starts, the jump-to-stance case, and that idle, the touch-down
update, 106, another caller, crouch, precision aim, `original` and Vanilla
are untouched. `ttk-input-test`, `ttk-aim-test` pass. Python 139 OK
(10 skipped), including schema 31 migration, CLI, environment and menu.

## Limits

- The hard landing 106 after a damaging fall still holds Duke for its
  length.
- 105 still shows for the touch-down update (2..3 fields) before the walk
  starts; the selector runs inside the airborne handler and is left original.
