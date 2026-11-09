# D10H9 - Press to fire on the energy weapon and flamethrower (measurement and proposals)

2026-11-08/09. Status: Done (user-accepted 2026-11-09: "this one is fully
accepted. great work"; follow-up D41J: the upgraded flamethrower's light). User report (D49B playtest): "there
feels to be a lag between mouse click down, and actual firing, for the energy
weapon, and the flamethrower". The user chose proposals 1, 2 and 3 ("we go
with your exact recommendation!"); see Implementation at the end. Diagnostic
trace `DNTTK_FEEL_TRACE=1` (never set by `run.py`): one `ttk-feel-upd` line
per player update with fire held, upper track, key, units left, `+0x23a`,
`+0x81c`, `+0x224` flags and the game clock.

## How the original fires them (read and traced)

Both weapons use the same upper-body tracks (the shotgun's rest and raise):

| Track | Keys | Units | Role |
| --- | --- | --- | --- |
| 20 | 1 x 150 | 150 | rest (gun lowered) |
| 24 | 4 x 20 | 80 | raise; played backwards it is the lowering |
| 23 | 1 x 150 | 150 | gun up, ready |

The upper track advances about 11 units per 33 ms update (game clock 10 per
two-field update, rate 1.1).

- **Rest to ready.** The press is seen on the next update, and the raise 24
  starts on the update after that. It runs 8 updates, about 265 ms.
- **Firing comes from the ready pose, not from a fire key.** The track 23
  handler (`0x8004f184`, upper callback `0x8004e840`) checks each update that
  fire is held (pad table `0x800d1b50` bit 1). For the flamethrower (9,
  upgraded 27) and the energy weapon (10) it then sets the stream flag
  `+0x224 |= 0x400` (`0x8003d724`). If fire is not held and the timer
  `+0x23a` is 0, it plays 24 backwards (lowering).
- **Per-update weapon tick** (`0x8003d6b4`, by weapon class from
  `0x8003df40`):
  - Energy weapon: `0x80058a7c` fires on every update the stream flag is set
    (gun call `0x8003c500` from `0x80058b78`, launch type 15 from
    `0x80073220`) and starts the beam sound.
  - Flamethrower: `0x8003d524`. While the flag is set and the charge
    `+0x81c` is below 100, `0x8003d2d0` ramps it by about 3.3 x the clock
    (+33 per update) and plays the ignition sounds `0x100c`/`0x100d`. At 100
    (and `+0x224 & 0x800`), `0x8003d48c` streams fireballs (type 14,
    `0x8003ca54` to the gun call from `0x8003cac0`). On release
    `0x8003d3bc` ramps it down.
- Neither weapon goes through D10H4's fire tracks (gun calls from
  `0x8004f7b4`/`0x8004fa88`), so the RETRO latch, raise boost and pacing never
  touched them, as D10H4 intended.

## Measurements

Lab `recomp/analysis/d10h9/` (local): `feel9.py timing` (D10H1 harness,
private D10B apartment card copies, level 1, `dnstuff`/`dnkroz`, Duke's
health topped up by the lab because that spot hurts him, offscreen, 180 Hz,
player settings: WEAPON FEEL RETRO), `keys.py` (track keys), `ramp_shot.py`
(screenshots through the ramp). Times run from the test input press to the
`ttk-launch` line. Player files were hash-checked unchanged after every run.

Press to the first round, held until it leaves, ms:

| Weapon | First person, rest | First person, 0.3 s after release | Third person, rest | Third person, 0.3 s after release |
| --- | --- | --- | --- | --- |
| Flamethrower | 430-457 | 594-599 | 356-488 | 572-597 |
| Energy weapon | 289-350 | 458-464 | 297-332 | 409-439 |
| EDuke32 freezer (stream analog, note 389) | 81 (72-99) | 81 | - | - |
| EDuke32 pistol (instant analog, note 389) | 58 | 58 | - | - |
| TTK RETRO guns after D10H4 (note 390) | 50-70 | 71-82 | - | - |

Where the time goes (first person, from rest, per-update trace):

| Part | Energy weapon | Flamethrower |
| --- | --- | --- |
| Press to the update that sees it | 17-50 | 17-50 |
| One more update before the raise starts | 33 | 33 |
| Raise 24 (80 units) | about 265 | about 265 |
| Ignition ramp (`+0x81c` 0 to 100, nothing visible) | - | about 100 (3 updates) |
| Shot to screen | about 18 first person (D10H8), about 60-70 third person (D10H5/D10H7) | same |

- **After a release it is slower than from rest.** Releasing fire in 23
  starts the lowering at once. A new press does not reverse it: the lowering
  plays to the end (8 updates), then the full raise (8 more). EDuke32
  weapons never lower.
- **Short clicks fire nothing.** 60 ms and 150 ms clicks fired 0 of 12 each
  (both weapons, rest and after release). Fire must still be held when the
  raise ends (and through the ramp on the flamethrower), about 300 / 430 ms.
  EDuke32 fires every 60 ms click (note 389). This is probably the largest
  part of what the user feels.
- **Ramp screenshots (first person):** the gun is up but no jet is drawn at
  charge 33 or 66. A faint glow shows at the target at 99, and the first
  fireball is the first fire on screen.
- **Held fire:** the energy weapon fires every update (30/s at two-field
  updates). While the flamethrower streams, the game drops to three-field
  updates (clock 15, 50 ms), so fireballs come at about 20/s and the whole
  game updates at 20 Hz. D10H6's fire CPU covers only the RETRO guns.

## Proposals (Modernized, WEAPON FEEL RETRO only; the user chose 1-3)

Ammo per round, damage, the beam's lock and reach, fireball flight and Vanilla
stay unchanged. MODERN keeps the original timing.

1. **Latch the press and run the raise out on the next update** (the D10H4
   rev 2 scheme, applied to track 24 for weapons 9, 27 and 10). The press
   reaches the game even when the click is short. The update after the press
   finishes the raise and carries into 23, where the original sets the
   stream. This gives the energy beam about 55-85 ms from rest (EDuke32
   freezer 81). A click fires at least one beam, and the flamethrower at least
   its ramp plus a short burst, because the latch holds fire until the stream
   starts.
2. **Stay ready after a release.** Keep track 23 (gun up) for a short time
   after fire is let go, then lower natively. A press inside that window fires
   on the next update. The original's own `+0x23a` timer does this in the 23
   handler (not yet traced where it is set). Alternatively, a press during
   the lowering reverses it and runs the raise out at once. Proposed window:
   about 1 s, or the time the D10H4 guns hold up after a shot. Removes the
   410-600 ms "after release" case.
3. **Shorter flamethrower ignition.** Bring the ramp from 3 updates to 1
   (start `+0x81c` at 66 on the first ramp update), so fireballs start about
   67 ms sooner. The ignition sounds still play, and no ammo is spent during
   the ramp (measured). With 1 and 2 the flamethrower would be about
   90-120 ms from rest, 35-70 ms more than the energy weapon.
4. **(Optional, changes damage per second; not recommended without the
   user's word.)** Run D10H6's fire CPU while the flamethrower streams, so
   the game stays at 30 Hz updates under the flame. That gives about 30
   fireballs/s instead of 20, so more damage per second.

Test plan after the choice: the same lab table (rest, after release, 16, 60
and 150 ms clicks, held), first and third person, 180 Hz, RETRO and MODERN;
`ttk-input-test`, `ttk-aim-test`; then the user's playtest clicking and
holding fire on both weapons.

## Not verified / limits

- The EDuke32 rows are D10H1's measured figures (the same lab and EDuke32
  build), not re-run. EDuke32 Atomic has no flamethrower; the freezer is the
  stream analog.
- Shot-to-screen figures come from D10H8/D10H5 measurements, not re-traced
  per present for these two weapons.
- Climbs, swimming, the jetpack and the upgraded variants (27, energy
  upgrade) were not run. The upgraded flamethrower takes the same 23 handler
  path (read).
- Where `+0x23a` is set, and whether a reversed lowering looks right, were
  not checked.

## Implementation (2026-11-09, proposals 1-3)

`recomp/src/ttk/weapon_feel.inc` (Modernized, WEAPON FEEL RETRO only; no
hook, codegen or framework change, existing savestates load):

- **Latch.** A press with the Flamethrower (9, 27) or Energy Weapon (10)
  drawn holds the fire pad (`weapon_feel_latched_fire`) until the stream has
  started (`+0x224 0x800`: the beam sound or the Flamethrower at full) and
  one update more, at most 30 updates.
- **Raise.** While that press waits, track 24 played forward (the raise) gets
  the rest of its units plus 1, so it carries into 23 in that update and
  the original's 23 handler starts the stream. Played backwards (the
  lowering), it runs out the same way, and the next update starts the raise.
  The direction comes from the key order, because `+0x78` is not the end key
  (it trails the active key).
- **Ready.** `+0x23a` is the original's hold timer for pose 23 (counted down
  by the clock at `0x80058284`). It is 0 while streaming, so a release
  lowered at once, and the original set it to 100 only afterwards. It is now
  kept at 300 (1 s) while fire is held in 23, and raised to 300 again after
  the original's write of 100 (`DNTTK_WEAPON_FEEL_READY`, 0-3000, 0 off).
- **Ignition.** `+0x81c` is precharged to 68 as the raise runs out or as a
  press arrives with the gun up, so the first ramp step (+33) reaches full
  and the first fireball comes on the next update. A ramp that started
  without that is raised to 68 after its first step.
  `DNTTK_WEAPON_FEEL_IGNITION=0` turns this off.
- Debug: `ttk_input` `controls.weapon_feel.stream` (presses, started,
  expired, raises, ready_holds, ramp_cuts, pending).

### Results (lab as above, 180 Hz, ms from the press to the first round)

| Weapon, view | Rest, before | Rest, now | 0.3-0.5 s after release, before | Now |
| --- | --- | --- | --- | --- |
| Energy weapon, first person | 289-350 | 51-105 (mostly about 82) | 458-464 | 48-50 |
| Energy weapon, third person | 297-332 | 48-104 | 409-439 | 13-48 |
| Flamethrower, first person | 430-457 | 83-130 | 594-599 | 48-84 |
| Flamethrower, third person | 356-488 | 80-142 | 572-597 | 62-81 |

- 60 ms clicks: 16 of 16 fired (both weapons, rest and gun up); 150 ms
  clicks too. Before: 0 of 12.
- 16 ms taps: some never reached the game in this lab. The Desert Eagle (D10H4,
  20 of 20 before) dropped 4 of 12 here too, so this is the lab's
  input-file timing, not the change. A real click is a queued event.
- Held fire unchanged: Energy Weapon 30/s, Flamethrower about 20/s (50 ms
  updates).
- MODERN, set through the private profile (`run.py` overrides the
  environment variable): Flamethrower 433-465 / 600-605, Energy Weapon
  300-324 / 465-467, as before.
- Weapon switch to the Desert Eagle 0.1, 0.5 and 1.5 s after a burst: about
  the same as MODERN (740-940 ms in both); the ready hold does not block it.
- `ttk-input-test` and `ttk-aim-test` pass. `ttk-controls-test` was not run
  (it needs its fixture arguments, and it has failed at its D14 assertion
  since before this job, per D08K15).

### Not verified

- A real mouse at the player's 180 Hz; whether the instant raise looks right
  (the gun snaps up as with D10H4).
- Climbs, swimming, the jetpack and the upgraded variants were not run. The
  raise run-out is limited to ground, airborne, jetpack and swim states, as
  D10H4's is.
- Proposal 4 (fire CPU under the flame) was not built.
