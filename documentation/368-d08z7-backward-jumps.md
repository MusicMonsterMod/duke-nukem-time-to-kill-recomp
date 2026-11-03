# D08Z7 - Backward jumps use the forward jump's technique

2026-11-02, revision 2 2026-11-03. Status: **Done**, accepted by the user
2026-11-03 ("it's a winner!!! lock and load") on revision 2. Modernized only (manual and assisted
jump styles, first and third person); Vanilla is unchanged.

Evidence is local under `recomp/analysis/d08z7-backward-jump/` (`probe.py`,
`analyze.py`, per-run `*.out`, `*.log` and `*-results.json`). Every run used a
private copy of the player's cards and savestates (UI slot 7 = file slot 6),
the player's profile with only view/jump style overridden, Xvfb, dummy audio
and private debug ports; each run hash-checks the player's saves and profile
before and after. File slot 9 of the private copy is a lab save at the bottom
of the drop behind the slot 7 start, made by the probe (`CASES=mksave`).

## User report

At UI slot 7, holding Shift and pressing S then Space (or holding all three),
"Duke ends up doing a forward jump". The backward jump should take off like
the modern forward jump: a natural FPS hop in the pressed direction.

## What happens (measured on the D08G8 executable `135a186d...`)

Behind Duke at slot 7 is a railing and a long drop. With the keys held the
game chains jumps, landings and bumps:

1. Shift+S runs backward (76/78) into the railing: wall bump 94, idle.
2. Space: preparation 96 (quick in manual), directional 98 backward over the
   railing, long fall, hard landing 106.
3. Space still held: the original takes the next jump straight out of 106
   (or out of a bump 94/95, or on the very update Duke lands from a previous
   jump). That 98 flew **forward**, along Duke's facing, about 3600 units
   while S was held (`base` run, `shift_s_together`: flight 2 `fwd+3636`).

Running backward then Space (no railing in the way) already gave the same
running jump (104) as running forward, redirected backward; the board's
"forward-jump" candidates 96 (the quick preparation) and 103 (running jump
with held keys) are the same technique the forward jump uses.

## Cause (confirmed)

The D08C standing-takeoff owner (initializer `0x800493a4`, return `0x8005a3a0`
or `0x8005a4d0`) only takes ownership with a recent grounded camera sample,
and that sample was recorded only in idle/walk/run (63, 72-79). A jump taken
straight out of a landing (105/106), a wall bump (94/95), or on the landing
update itself (state 0/9) had no sample, so the takeoff was unowned: Modernized
movement still supplied the original Forward, the original selected 98, and
nothing redirected its velocity. Diagnostic trace (`z7a`): `ground=83/333` and
`ground=428/511` (sample hundreds of input frames old) at the two forward
takeoffs, `ground=81/83` at the owned first one.

A second, smaller leak: when original clearance refuses an owned directional
takeoff (a wall within 375 units behind), the fallback is the vertical jump
97, whose small horizontal drift (about 2730, along Duke's facing) moves him
forward during the descent, about 400-950 units.

## Change

`recomp/src/ttk/modern_controls.cpp`:

- **Ground sample** (camera update `0x8002a7fc`): also recorded in the settled
  ground poses 94, 95, 105 and 106 when the camera-only lease holds and the
  state bytes are 0/0.
- **Landing update** (initializer): a takeoff whose last camera sample was the
  live jump camera (`jump_camera`, same epoch, within the frame window) is
  accepted, including state 0/9 (landing and taking off again in one update).
  Every other check (actor, caller, animation 96 uninitialized, independent
  camera, identity, flags, crouch, precision aim, epoch) is unchanged.
- **Blocked fallback**: in the 97 ballistic update (`0x8003ebf4` from
  `0x80055bc4`), at the 9/0 takeoff of an owned directional jump, the
  horizontal drift is pointed at the pressed direction with the same speed.
  Original gravity, swept collision and height are kept. Space alone, neutral
  preparations, precision aim, airborne (9/9) updates and Vanilla keep the
  original drift. Counter `blocked_drifts` in `ttk_controls`.

No body animation, height, speed, timing or clearance rule changed. Forward
jumps take the same path as before. The new owner paths use the existing
redirect and manual air steering.

## Evidence (executable `48d9f484a35d7b8648eb89f2dcc927895ac284680005e82de42a4b30a30c981d`)

Flights per case, travel along Duke's facing (`fwd`, negative = backward),
sideways (`side`, positive = left) and rise, from `analyze.py`:

| Run | Case | Before (`base`) | After |
| --- | --- | --- | --- |
| slot 7, manual, first | Shift+S+Space held | 98 `fwd-1329`, 98 **`fwd+3636`**, 103 `fwd-462` | 98 `fwd-1356`, then fall (`m-first`); third person 98 `fwd-1329`, 98 `fwd-2955`, 103 `fwd-182` (`m-third`) |
| lab slot 9 (bottom, wall behind), manual, first | S+Space held | 97 then 98 forward (`bottom`, earlier build) | 97 `fwd+0`, then five 98 hops `fwd -11..+10` in place against the wall (`mb-first`) |
| lab slot 9, manual, first | Shift+S+Space held | - | 97 `fwd+0`, four 98 hops `fwd -11..+6` (`mb-first`) |

All slot 7 cases (W, S, Shift+W, Shift+S, Shift+S quick, all three held,
Shift+A, A, Shift+S+A, Space) in manual and assisted, first and third person
(`m-first`, `m-third`, `a-first`, `a-first2`, `a-third`): every S flight has
negative `fwd`, every A flight positive `side`; forward, sideways and Space
alone match the baseline (98 rise about 320 manual / 425 assisted, running
jump 104 rise about 500-580, Space alone 97 rise about 800-1230). Running
backward then Space gives the running jump 104 backward (`fwd` about -5000
manual, -3400 assisted), as running forward gives 104 forward.

Native: `ttk-controls-test` passes all groups, including the new D08Z7 group
(owned takeoff from 94/95/105/106 and the landing update, refused for 0/9
without a jump camera, a fall pose, a changed epoch or a stale sample; the
landing-update 98 redirects backward once; the blocked 97 drifts backward at
the same speed; neutral, 9/9, precision aim, Vanilla and the 98 caller keep
the original drift). The group fails on the previous code
(`Assertion 'landed(pose,0,0)' failed`).
`ttk-input-test` and `ttk-aim-test` pass. Python: 223 tests, 2 skipped, one
failure in `test_primitive_trace` (the near-renderer harness `ttk-near-test`,
built from `near_clip.cpp` only during D17BB; it shares no code with this
change). `scripts/ci/check_repo.py` passes.

Two lab runs stopped on `[TTK load UI] fresh-frame wait timed out` after a
savestate load with two games running at once; the same message appears in
earlier labs (D08G8, D08Q11, D14, D17AH) before this change. The interrupted
cases were rerun alone.

## Revision 2 (2026-11-03): UI slot 11, slope jumps and jump-then-S

User, after revision 1: "for me it's slot 11, and i tried to test using the
backward jumping but the original problem still exists. interestingly, you
could jump, and hold S, and it will cause duke to move forward rather than
backward as you'd expect". The session log shows file slot 10 (UI 11).

Measured at UI 11 on revision 1 (`s11`; Duke's heading 3136 there, so
`analyze.py` now uses each route's own heading):

- **Jump, then hold S** (`space_then_s`): Space alone gives the vertical
  jump 97. It had no air control in either style, and its original drift
  along Duke's facing carried him **forward +717** while S was held.
- **Backward jump onto the slope behind Duke** (`shift_s_together`): the
  backward hop lands on a face too steep to stand on, Duke slides (mode 2,
  143 = facing down the slope), and the held Space takes the original slide
  jump. When Duke faces down the slope that jump is 98 along his facing:
  **forward +2785**. D08Z5 only rewrote the facing-up case (99/100) into the
  climbing slope jump.

Changes:

- `modern_controls.cpp`, 97 update (`0x8003ebf4` from `0x80055bc4`): any
  owned takeoff (also Space alone) seeds a cap from the drift (at least the
  2048 walking-fall floor); in the manual style the held camera-relative
  direction steers it each later update like every other jump
  (`manual_air_steer`, split out of `manual_air_control`). No key keeps the
  original drift; assisted keeps the fixed arc.
- `wall_backflip.inc`, slide jump (initializer, pending from the slide
  handler's jump branch): with Modernized input and a direction held, the
  direction decides instead of Duke's facing: toward the face (uphill,
  `+0x2bc` + 2048) it becomes the D08Z5 climbing slope jump; otherwise a 98
  whose first update points its velocity the pressed way at its own speed
  (with the slope jump's rise if the fall had already started). No
  direction, precision aim, `--wall-backflip original` and Vanilla keep the
  facing rule. Counter `wall_backflip.redirects`.

Evidence (executable `d214c47d11cd04abe4d9098c2a76d36ff78ec0120897384ab833f992f3739481`):

| Route | Revision 1 | Revision 2 |
| --- | --- | --- |
| UI 11, manual, first: Space then S | 97 `fwd+717` | 97 `fwd-536` (`s11m`) |
| UI 11: Space then W / then D | - | 97 `fwd+737` / `side-574` (right) |
| UI 11: Shift+S+Space held | 98 `-1208`, 97 `-1025`, 98 **`+2785`**, 103 `-1119` | 98 `-825`, 97 `-993`, then three slope climbs `fwd+0` (`s11m`) |
| UI 11: S+Space held | - | 98 `-825`, 97 `-1115` |
| UI 11, assisted, third: Space then S / all held | - | `-1037` / 98 `-1095`, 97 `-711`, 98 `-556` (`s11a`) |
| UI 11: W+Space, Space alone | 98 `+184`/97 `+0` | 98 `+2475` / 97 `+0` |
| UI 7 matrix and drop bottom, manual, first | as revision 1 | unchanged: every S flight backward, held S hops in place (`s7m`, `s9m`) |

`ttk-controls-test` passes all groups. D08Z5's checks now run with no key
held (the facing rule) and add: a facing-down slide jump with the uphill
direction held climbs; a facing-up one with downhill held leaves downhill at
its own speed with the slope rise; aim keeps the facing rule. The D08Z7
group adds the vertical steer (manual reverses the drift to the 2048 floor;
assisted, aim and a stale epoch keep the drift).

## Limits

- Duke's body keeps its takeoff facing, so a backward hop still plays the
  directional jump pose 98 (and running 103/104) in third person; in first
  person only the legs show it. A dedicated backward or neutral airborne pose
  was not added (no such original clip; would need new animation work).
- The blocked fallback keeps the original vertical 97 height (about 900-1200),
  the same as a forward jump refused by clearance. The vertical jump only
  moves horizontally on its way down (original handler), so steering it shows
  after the apex; its cap is small (about 2700), like the original drift.
- Assisted style keeps the original fixed arc for the vertical jump, so a
  direction pressed after takeoff does not steer it there (it does in manual).
- The player accepted it at UI slot 11; the lab routes cover UI 7 and 11 and
  the drop below UI 7. Other maps use the same rules.
