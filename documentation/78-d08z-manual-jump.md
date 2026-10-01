# D08Z - Optional manual modern jump

Status: **Done** (2026-10-01, user-accepted: "i love it. lock it in"). Modernized option, `assisted` stays the
default; Vanilla unchanged.

## User report

The D08Y jump reliably makes the slot-5 gap, but it feels "on the rails": "the
long jump thing really feels like it's just not me", "feels off in the hand".
The user knows it is the original behavior and wants a more modern,
player-controlled feel as an option.

## What makes the original jump feel on rails (measured)

Private copy of UI slot 5, Modernized, first person, 16:9, real keys on a
private Xvfb display (`recomp/analysis/d08z-manual-jump/measure.py`). Fields are
60 Hz; the game updates at 30 Hz.

| Jump | Press to airborne | Flight | Notes |
| --- | --- | --- | --- |
| Running (Shift + W), far from a gap | 2..4 fields | ~60 fields (1 s), ~3100 units, apex ~580 | horizontal velocity 10085, constant |
| Running, pressed ~800 before a gap | 14 fields | as above | queued by the original look-ahead, leaves from the lip |
| Standing directional (W + Space) | 15 fields | ~50 fields, ~1550 units | crouch preparation 96 first |
| Walking jump | 14 fields | as standing directional | crouch preparation 96 first |
| Standing vertical (Space) | 18 fields | ~55 fields, apex ~1000 | 96, then 97 |

Three things take control away from the player:

1. **The takeoff point near gaps.** The original run handler looks 1024 units
   ahead (`0x80078c0c`); before a drop it queues the jump (`+0x228` bit 4) and
   launches from the lip (D08Y restored this for drops over 768).
2. **The fixed arc.** Takeoff sets the horizontal velocity once
   (`+0x1f4/+0x1fc`); the ballistic update (`0x8003ebf4` from `0x80055934`)
   only adds gravity. There is no air control for about a second.
3. **The wind-up.** Standing and walking jumps play the crouch preparation 96
   for 14..18 fields (about 0.25..0.3 s) before Duke leaves the ground.

## Options considered

- **Air steering** (taken). Bend the horizontal velocity toward the held
  camera-relative WASD during an owned flight, never above the takeoff speed.
  Original gravity, swept collision, impacts and landings keep running, so no
  wall clipping and no extra height. This is the core of the modern feel.
- **Jump on the press near gaps** (taken). The D08I "jump now" override of the
  look-ahead, for gaps as well.
- **Edge grace (coyote time)** (taken). Jumps pressed just after running off an
  edge still launch, so a late press is forgiven. The existing small-drop grace
  (furniture, six input frames) extends to every run-off in manual.
- **Quicker standing takeoff** (taken). The preparation 96 plays three times
  faster. Its events still run in order (same idea as the D08E weapon
  transition boost).
- **Variable jump height** (not taken). Releasing Space early to cut the jump
  would shorten nearly every tapped jump (a tap releases within ~100 ms) and
  make gaps harder. Can be revisited.
- **Longer arc / extra air speed** (not taken). Not needed: the measured slot-5
  window is reasonable with the grace and the D08Y low-lip scramble.
- **Removing the D08Y safety nets in manual** (not taken). The low-lip scramble
  and E mantles only act when Duke would otherwise bounce off; they do not move
  the takeoff and do not feel like rails.

## Change

- Profile schema 20: `controls.jump` `assisted` (default) or `manual`;
  `run.py --jump`, `--settings` choice J; `DNTTK_JUMP` (always `assisted` for
  Vanilla). Migration keeps every other preference.
- `recomp/src/ttk/manual_jump.inc` (included from `modern_controls.cpp`):
  - `manual_air_control()` from the owned ballistic update (98/103/104, owned
    short falls 108, and the E reach 109 in manual only): with WASD held,
    `air_steer()` (`control_math.h`) moves the horizontal velocity at most
    `0.14 * cap * dt/11` per update toward `wish * cap`, where `cap` is the
    flight's own takeoff speed (at least 2048). The path stays inside the cap,
    so steering never gains speed. No input, or precision aim held: no write.
    `DNTTK_AIR_CONTROL` (0..3, default 1) scales the rate for playtest tuning.
  - `manual_air_seed()` sets the cap when a flight is owned (dry-land takeoff
    redirect, short-fall velocity). Wade jumps (D08W) and fall leases reset it
    to 0, so they are never steered.
  - `manual_quick_takeoff()` at the lower-body runner `0x80059db0` (ra
    `0x8005a3d4`, track `player+0x60`, a3 0, delta at the update frame
    `sp+0x10`): delta x3 for animation 96 on the ground, once per update.
- `0x800780b4` look-ahead hook: in manual the D08Y large-drop probe is skipped,
  so the D08I jump-now override applies before gaps too.
- `terrain.inc` edge grace: in manual it accepts large run-offs, needs no floor
  ahead, and lasts 14 input frames from the last grounded camera update.
- Diagnostics in `ttk_controls`: `jump_style`, `air_steers`, `coyote_jumps`,
  `quick_takeoffs`, `air_cap`.

## Evidence (binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`)

Same route, manual (`measure-manual.json`, `gap.py`):

| Jump | Assisted | Manual |
| --- | --- | --- |
| Standing vertical, press to airborne | 18 fields | 8 |
| Standing directional | 14..15 fields | 6..7 |
| Walking | 14 fields | 5 |
| Running, ~800 before the gap | queued, leaves from the lip | leaves on the press (falls short, as expected) |
| Running jump, D held after takeoff | straight on | velocity turns 90 degrees in ~20 fields, speed stays 10085; hit the side railing and bounced (original collision) |
| Running jump, S held after takeoff | straight on | stops in ~14 fields, reverses to -10085 in ~30 |
| Running jump, S held during the E reach 109 | - | steers the same way |

Slot-5 gap, manual, running, Space pressed when Duke's centre reaches Z (lip
at 87107, the assisted lip launch):

| Press | No E | E held |
| --- | --- | --- |
| 85800, 86100 (1000..1300 before the lip) | - | jump mantle 134, on the platform |
| 86300 | falls short | jump mantle 134 |
| 86500 .. 86800 | bounce, low-lip scramble 134, on the platform | 86600: jump mantle 134 |
| 86900 .. 87100 | lands across | 86900: lands across |
| 0..10 fields after running off the edge | grace jump, lands across | 4 fields: lands across |
| 12..16 fields after running off | falls (grace over) | - |

Assisted on the same binary: presses at 86300/86600/86900 are queued and land
across from the lip, a press 6 fields after running off falls (no large-drop
grace), standing 18 / directional 14 fields, no air steering.

Native: `ttk-controls-test` adds the D08Z groups (manual look-ahead jumps now
before a large drop; steering converges at the cap, no writes without input,
with precision aim or in assisted; 109 steered only in manual; quick takeoff
x3 once per update, only for 96, only manual, not in the air, not Vanilla).
`ttk-input-test`, `ttk-aim-test` pass; Python 95 OK (jump style migration,
CLI, environment, validation and menu).

Regressions, same binary, manual against assisted (`regress.sh`, outputs in
`regress-manual/` and `regress-assisted/`; earlier analysis files restored
afterwards):

- Slot-12 1024 wall: W + E + Space grabs 148 and pulls up 140; W + Space
  bounces off and lands. Same states as D08Y, Duke leaves the ground ~10
  fields sooner.
- Slot-12 `cases2`: holstered E jumps end on the ledge in both styles (manual
  mantles 139/140 straight away where assisted catches 148 and pulls up); tall
  wall falls in both. The pit jump with E missed once in the batch (1/2);
  repeated with `pit.py`: every manual jump that left the ground caught and
  pulled up (3/3; a fourth run grabbed a ladder with E before Space), assisted
  3/3.
- Crate A 4/4 identical; crate B varies run to run in both styles (as noted in
  D08Y); crate-to-crate 8/8 on top; angle route identical (-15..0 on the
  ledge); D08U ladder-top regress identical.
- Subway wade jumps (D08W `sidejump.py`, `ledgesweep.py`): identical apart from
  the standing D jump into the train (short in both styles).
- Apartment bed: jumps immediate in both. Fire-escape platform: assisted queues
  presses for the lip (8, 7 postponements); manual jumps on the press (0).

## Limits

- Only the slot-5 walkway and the regression routes were exercised; other
  gaps follow the same rules.
- The steering rate (0.14 of the takeoff speed per update) and the 14-frame
  grace are first guesses to tune by feel; `DNTTK_AIR_CONTROL` scales the rate
  without a rebuild.
- Duke's body keeps its takeoff facing while steered; in third person a hard
  sideways steer looks like a sideways drift.
- Shallow-water (wade) jumps keep the D08W behavior (no steering).
- The jump height is fixed (no early-release short hop).
