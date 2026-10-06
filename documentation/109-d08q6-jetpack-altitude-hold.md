# D08Q6 - Modern jetpack holds its altitude

2026-10-06. **Accepted** (user: "i accept this as fixed! the test passes and
this feels great", revision 5b, binary `2a49b29acbd0de49...`).
User report: "duke keeps gaining height when using the jetpack in modern mode
... duke's elevation should just be fixed when jetpacking unless the player
uses ctrl or space". After revisions 1-4 the user found flight "really
broken" and asked for a from-scratch re-evaluation.

## Revision 5: what was actually wrong, and the new model

**Root cause of "cannot move with WASD unless ascending or descending".** The
original root apply `8004ac08` drops the horizontal root whenever the
vertical root `+0xfe` is 0. At `8004ac28` it branches with `beq` to
`8004ad1c`, which zeroes `+0xfc`/`+0x100` before the final probe (confirmed in
the generated C). Original flight always had a vertical component (bob,
gravity, thrust lift), so this never showed. Revision 2 onward held height
exactly (bob parked flat, precise hold), so steady flight often had a zero
vertical root and WASD did nothing. Space/Ctrl added vertical motion, so they
"fixed" it. Revisions 1-4 each held height more precisely and made it worse.
The "stuck band" at 500-600 clearance was the same thing: the floor cap
parked Duke at exactly the approach height with no vertical motion.
Lab: at fixed heights 250-1100, W moved 0 before; after the fix it moved
400-500 per 40 frames at every height.

**The layered design was also wrong.** Modern flight mixed the original's
ballistic momentum (about 1 s to reach speed, long coasting, reversals fight
old speed), its hover lock (a different, throttled movement mode with a
pinned, bobbing height) and host corrections. Every start and stop switched
between two physics modes. Revision 5 replaces it.

**Calibration** (`cal.py`, `DNTTK_JET_CAL`, removed after use). With the hover
lock off and no pads, the handler moves Duke by `bv * dt / 1024` per update
(`bv` = `+0x1f4/+0x1f8/+0x1fc`, dt = `800d21fc` in ms) on x/z, and about 1.1x
that on y. Gravity and drag do not show when bv is rewritten before each
update. Fed pads add their own thrust on top (W 13-19 units per update,
Space 13-19 lift, varying with timing).

**New Modern model** (`jetpack.inc`, revision 5b after the user's "now the
jetpack has no fire etc, and moves very slowly"): the host owns the flight
velocity, and the original still sees its normal flight inputs.
- **Speeds from the original.** A post-handler trace (`post.py`, Classic)
  shows original flight reaching a horizontal bv of about 16000 (still
  accelerating at walls) and Square's climb capped at -11832. Modern targets
  16000 horizontal, -11800 climb and +6000 descent (about the old Ctrl rate,
  490 per 12 frames). The first 5a cut used 1600/1500/1640, taken from an old
  "units per frame" note, and was about 10x too slow. Horizontal eases with a
  50 dt time constant; vertical uses 15 dt, so a released climb stops within
  about 5 updates.
- **Original pads are fed again** (WASD/arrows to Up/Down/strafe pads, Space to
  Square). They bring back the flame, the lean poses (165-170, 163 on
  climbs), the thrust state and the original's own fuel drain. Shift (L1 hover
  toggle) is withheld, and Space with Ctrl gives no Square. The thrust the pads
  add is cancelled. `jet_post_update` (the `0x80058120` hook, right after the
  handler) reads the handler's thrust `+0x1e4..+0x1ec`, and the next update
  writes `target - (v*dt + v/2)` (8003ea58). The thrust changes slowly (81 →
  14 as speed rises; -37/-83 lift), so the prediction holds.
- **Flame while hovering or descending.** The flame is drawn from
  `+0x224 & 0x02000000`, which the handler sets only while it thrusts.
  `jet_post_update` sets it after the handler. Screenshots: moving, hovering
  and climbing all match the original's flame.
- **The 0 vertical root.** While moving, a planned vertical step under 2 units
  becomes an alternating +-2 (net 0). The pads' thrust shifts the applied
  vertical root by a few units, so `jet_post_update` learns the offset (the
  root the handler applied in `+0xfe` minus the plan, integrated at 0.3,
  clamped +-12) and the next update corrects for it. A function-entry hook on
  `8004ac08` would be exact, but it needs a new codegen entry
  (`mod_function_entry_funcs`), meaning regenerated code and a savestate risk,
  so it was not used. Zero-root stalls dropped from 162 to 3 of about 420
  moving updates; the 3 match the trace-cadence artifact.
- **Height hold, floor cap, released-residue snap and host fuel drain** (only
  while no pads are fed) are as in 5a. The hover lock stays off.
- Classic (D08R) keeps the original physics and pad bridge (WASD/arrows, Space
  Square, either Shift the hover toggle). Vanilla is untouched.

**Verification** (binary `2a49b29acbd0de49...`, a copy of the user's profile:
120 fps, first person, right shoulder, fast CPU 150%):
- `feel.py`: W ramps to about 156 units per update within about 10 updates
  and eases out; moving updates stall only at walls.
- Screenshots `modern5-w/idle/space.png`: lean pose with flame, hover flame,
  climb flame (cf. `classic-*.png`, `classic-lock.png`).
- `circuit.py`: height within about +-20 while moving, ending within 9-14
  units. Space 20 frames climbs about 1670, then stops within about 8. Ctrl
  12 frames descends about 470, then settles within 3.
- `regress.py`: W + fire covers 3799 with 7 shots; Ctrl lands; J falls and
  lands; fuel without god drains 278/s (`fuel.py` 269/s vs Classic 268-288/s).
  Switching 4 to 5 works with owned weapons.
- Vanilla: `jet` false, 0 host updates. Classic W covers 3918.
- Native: `ttk-controls-test` D08Q6 group (lock off, eased camera-relative
  speed, +-2 step, thrust cancel, flame flag, stop without creep, hold and
  floor cap, Space/Ctrl, fuel) and `ttk-input-test` (Modern feeds the flight
  pads, never Shift; Space+Ctrl no Square; Classic bridge; Right Shift) PASS.
  Aim, near and 112 Python tests PASS.

**Limits:** Speeds and eases are first values, set
from the original. Rising terrain uses the floor cap (unit-tested). D08S
should build on this model.

## History (revisions 1-4, superseded by revision 5)

## Reproduction (before the fix)

Private lab `recomp/analysis/d08q6-20261006/` (port 9361, Xvfb :97, private
profiles and copies of the D08Q5 cards; `level 0` from the console, `dnstuff`,
god mode already on in slot 1). `repro.py` / `trace1.py` with
`DNTTK_JET_TRACE=1`.

| Leg (120 frames) | dy (negative = up) |
| --- | --- |
| W / S | -47 / -35 |
| A / D | -129 / -140 |
| W+D / W+A | -83 / -98 |
| Six legs total | about -460 |

Two causes, both in the D08Q host layer, not in the original handler:

1. **Level flight still climbed** about 2 units per update. The fixed
   `k_jet_level_trim` (+0x1e8 = 18) does not cancel the handler's directional
   lift for every direction.
2. **Every release re-anchored the hover at the current height.** The hover
   lock set its base `+0x860` to Duke's Y at that moment, so the flight climb
   was banked. The original hover bob (`+0x864` phase, about +/-128 units
   around the base) also meant a key press released the lock anywhere in the
   bob, so each stop could bank up to 128 units more.

Pitching the view does not move Duke. Thrust only uses the body heading
`+0x1c`. The apparent "looking up" effect is the bob: hovering alone ranged
-128..+127 around the base. (A mid-run "freeze" in the first repro was Duke
dying: typing `dnkroz` there toggled god mode off.)

## Change (`recomp/src/ttk/jetpack.inc`, Modern scheme only)

- One **held altitude** `jet_hold_y`, captured on the first update without
  Space or Ctrl. Space or Ctrl clears it. Leaving flight, losing the lease or
  the Classic scheme also clears it.
- **Level flight** (WASD): as before, release the lock and write the trim. In
  addition, `+0x1f8 = clamp((target - Y) * 32, +/-2400)` steers back to the
  hold every update. bv is about 60 per unit/update, so this closes about half
  the error each update.
- **Hover** (revision 2): engage the original lock as before. The pin is
  `phase += dt` (`800d21fc`), `bob = 128 sin(6 phase)` and `Y = base + bob`
  (`8004b474..8004b4c8`). The host parks the phase at the sine peak
  (`+0x864 = 171 - dt`, so the angle is about 1024 and the bob 128). It keeps
  the base 128 below the target and slews it by at most 3 units per update.
  The peak is flat, so a long frame or a second handler step changes Y by
  about 1 unit.
- **Floor cap**: target = min(hold, Y + clearance - 0x200), with clearance
  `+0x1c8 - +0x10`, which is the original floor approach height (`8003ef78`).
  Over higher ground Duke rides at the approach height instead of being
  steered into a roof, then returns to the hold. A first attempt that only
  blocked downward corrections inside a 0x400 band failed, because normal
  street flight is about 940 above the floor. With a 0x280 band, low flight
  crept up to the band edge.
- Debug JSON adds `jet_hold_captures`. The trace adds `hold=valid/y`.

There is no new hook, address or guest write type. The writes stay within D08Q's
set (hover bit, base, phase, +0x1e8/+0x1f8). Classic and Vanilla return
before this code. The codegen hash is unaffected.

## Revision 2: jerky flight (user report)

User: "the jetpack does not control smoothly now, it's very jerky". Per-update
traces (`smooth.py`, `ana.py`, the same route on each build) show the cause:

| Build | Hover dy per update | Stop | Level flight |
| --- | --- | --- | --- |
| Before D08Q6 | smooth bob, up to +/-12 | smooth | steady -2 (the climb) |
| Revision 1 (phase 0, error slew) | +17 / -25 about every 6 updates | +21 step | 0 |
| Revision 2 (peak phase, 3/update slew) | 0 | +3, +3, +2 | 0, occasional -5/+2 (the original's own bumps, also before) |

Holding the phase at 0 left one timestep of the steep part of the sine in the
pin: about 20 units at a normal frame and 39 on a long one. The handler
sometimes steps twice per player update. Revision 1 also slewed the base by
the measured error, which echoed each twitch. An exact `-dt` cancel fixed the
normal case, but a double step still gave a 17.6 unit blip. Parking at the
flat peak removes both.

## Verification (revision 1 binary `1a55cc30647cb4d2...`; revision 2 `6af3f631f9fe5a29...` re-ran the circuit, low flight, landing, J fall, firing/switching and suites)

- `circuit.py` (LEVEL00 street, about 2400 flight updates): W/S/A/D, diagonals,
  pitch up and down with and without W, W with mouse turning, 16 random
  turning legs, 8 taps. The net altitude change after the whole circuit is
  -9..0 units in the final runs; within a leg Duke moves at most about 22 units
  (a transient when movement starts). Before the fix this was about 460 units
  in six legs.
- Hover after release: revision 2 is flat (0 per update; within 2 units after a Ctrl stop).
- Space 20 frames: climbs about 350-470, then holds. Ctrl 12 frames: descends
  about 450-510, then holds within about 25 after a short settle.
- `low.py`: hovering at 512 above the street, 14 random low legs with turning:
  within +/-15, clearance 510..1071 underneath, no landing. Ctrl to the
  ground lands (anim 105, then 63). J in the air gives the original 108 fall
  and landing. W + LMB for 90 frames: dy 0, 7 shots. Key 3 switches in flight
  (D08Q5).
- Classic (`PROFILES=profiles-classic.json`): `jet_hold_captures` 0, no host
  hovers, original rise and fall physics.
- Vanilla: original flight, `jet` false, 0 host updates, no hover lock.
- Native: `ttk-controls-test` (new D08Q6 group: hover pin without bob, base
  slew, level-flight steer and cap, floor cap, Space re-capture;
  revision 2 checks the peak phase and the 3-unit slew), `ttk-input-test`,
  `ttk-aim-test`, `ttk-near-test` PASS; 112 Python tests OK.
- The player's saves and cards were not used.

## Limits

- Not yet confirmed by the user on their LEVEL01 apartment/alley route. The
  lab circuit ran in the LEVEL00 street area around the start.
- The original hover bob is gone in Modern. If the user wants a gentle bob
  back, add it as a visual offset around the fixed hold.
- Rising terrain was not deliberately probed beyond the street's own variation
  (the cap is unit-tested). Ceilings: a hold above a low ceiling just presses
  Duke against it, as before.
- D08S (EDuke32 scheme) does not exist yet. It should reuse this hold.

## Revision 3: Shift and the arrow keys (user report)

User: "something is causing the shift key to change height, and i cant use the
arrow keys to move around. it goes from jerky to uncontrollable".

- **Shift** is the Modernized walk binding, which drives the original L1. In
  flight L1 is the original hover toggle (`800d1cf0`), which re-anchors the
  base to Y with phase 0. Against the parked-peak hover that is a 128-unit
  drop, and it was already a small jolt before D08Q6. `input_pad()` now
  withholds walk/L1 during Modern flight. Classic keeps the original toggle
  (D08R). As a safety net, hover re-seats the base at `Y - 128` whenever
  `base + 128` is more than 7 units off Duke.
- **Arrow keys** are the fixed menu escape hatch and reach the raw D-pad. In
  flight, Up/Down sent original thrust that the host did not count as
  movement. It held the hover pin, which throttles thrust and fought the hold
  ("uncontrollable"). Left/Right sent D-pad turns that `face_view` overrides,
  so they did nothing. In flight (both schemes) the arrows now map exactly like
  WASD: Up/Down give forward/back, Left/Right give the layout strafe pads, with
  no D-pad turns. `InputFrame.arrow_x/arrow_y` lets `jetpack_update` count
  them as movement. Arrows on the ground are unchanged (still the fixed D-pad).

Live (binary `714bed0f9ffd21ce...`): Up/Down/Left/Right fly 380-1700 units per
60 frames, depending on walls, and Right in open space is 760-1040. Shift taps,
holds, Shift+W and Shift+Up all give 0 height change. Over 1297 updates of
mixed keys the largest step is 7 units, with a net of 0.
`ttk-input-test` covers arrows in flight and Shift withheld in Modern and
passed in Classic. `ttk-controls-test` covers the re-seat.

## Revision 4: Right Shift dropped capture; idle creep (user report)

User: "duke cannot seem to move when using jetpack, with wasd keys, unless you
are ascending or descending. also i believe that idly hovering on jetpack makes
duke creep to the right extremely slowly".

Not reproducible with the lab keys, including a private copy of the user's
profile (120 fps, first person, right shoulder, fast CPU at 150%), 4
directions, high and low flight, levels 1/2/3/5/6, and Caps Lock. The user's
session log (`build-local/logs/session-20261006-163952.log`, read only)
showed the cause. During flight it repeated `[TTK lease] inactive (released)
anim=164 state=10/10 flags=8e000000`, followed by `Gameplay captured
automatically`, with no "Mouse released" line.

- **Right Shift.** `input_pad()`'s fixed escape-hatch keys map Right Shift to
  the original **Select**, and unlike X/Z/C it was not excluded while
  captured. The framework passes the final pad to `input_pad_context()`, which
  releases capture whenever Select/Start is pressed. One Right Shift tap
  therefore stopped the whole host layer. The original hover lock stayed on
  (`0x08000000` in the logged flags), so WASD only crept by the throttled raw
  thrust until Space (original Square) cleared the lock and the auto-capture
  came back. Reproduced live: after a Right Shift tap, `captured=False
  jet=False lock=1`, W/S moved 0. Game-wide fix: while captured, Right Shift
  never sends Select and counts as Shift (`down()` treats LShift bindings as
  either Shift, as it already does for Alt). Outside capture it is still the
  Select escape hatch. Live: a tap keeps `captured=True jet=True`, then W/S
  move 2000-2700. On the ground, RShift+W/S runs like LShift (about 2100-2300
  vs 440-640 walking).
- **Idle creep.** After movement the original horizontal drag stalls short of
  zero (integer decay; `bv` stuck at 203/-21). Under the hover lock that
  residue moved Duke about 21 units per 100 frames for as long as he hovered.
  In hover the host now zeroes `+0x1f4`/`+0x1fc` below 256 (about 4 units per
  update). A real coast still runs: 306 units in the first 100 frames, then 0
  for 500 frames.

Native: `ttk-input-test` covers Right Shift captured (no Select, Shift; the
Classic toggle) and `ttk-controls-test` covers the stalled-coast stop and the
kept coast. All suites pass. Binary `469417e7b65ab5a6...`.
