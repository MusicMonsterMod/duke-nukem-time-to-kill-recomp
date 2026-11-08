# D10H6 - Held fire on EDuke32's clock

2026-11-05. Status: Done (accepted and closed 2026-11-05). Part of WEAPON FEEL RETRO (D10H4,
[note 390](390-d10h4-shot-on-press.md)); lab of D10H1
([note 389](389-d10h1-weapon-feel-lab.md)).

User, after the D10H4 build: "holding down fire for example on the pistol, has
a kind of wonky feel to the shooting consistency if that makes sense, in
comparison to eduke32", then "let's do it now! this might actually get us to
the 1:1 i want for this!!"

## Why held fire wobbled (measured and read)

- The upper delta is `(word 0x800d21fc * half player+0xb0) >> 12`
  (`0x8005a2d0`). `0x800d21fc` is the game's frame step, 5 per video field
  since the last update (2-8 fields, `0x8001fde8`): 10 for a normal
  two-field update, 15 for a three-field one, 300 per second. `+0xb0` is
  Duke's animation rate (4800 at GAME SPEED 50). An EDuke32 tic is 10 units.
- The upper advancer `0x80059db0` counts the active key (`+6`) down in `+2`.
  When a key ends its flags are ORed into `player+0xb8`. A fire track's fire
  key has flag 8, and the fire handler tests `+0xb8 & 8` (an animation event,
  not the pad).
- Fire tracks (animation 0x800c2d64 variant 1, key ranges 0x800d8328, keys
  `*0x800d8324`, at 0x80faf380 above main RAM):

| Track | Keys (flags, duration) | Loop |
| --- | --- | --- |
| Desert Eagle 16 | 488 (8, 20), 489 (0, 20), 490 (1, 30) | 70 |
| Gatling / Freezer 34 | 1794 (8, 20), 1795 (1, 20) | 40 |
| Combat Shotgun 25, then pump 26 | 78 (8, 30), 79 (1, 30); 84-90 (7 x 30) | |
| RPG 37 | 1886 (8, 50), 1887 (0, 50), 1888 (1, 50) | 150 |

- Held Desert Eagle: 70 units at about 11 per update is 6.36 updates per
  shot, so 6 or 7 (200 / 233 ms) in an uneven pattern (measured gaps 167-237,
  average 209.8 ms, 4.7/s). EDuke32: every 5 tics, 167 ms, 6.0/s.
- The Freezer leaves its loop after every shot: 34, one update of the gun-up
  pose 32, then 34 again from the fire key.

## Change (Modernized, WEAPON FEEL RETRO only)

`recomp/src/ttk/weapon_feel.inc`, in the D10H4 upper-track hook (no new hook):

- While fire is held after a shot of the same weapon, on its fire track, the
  hook adds the frame step to a clock and replaces the update's delta. The
  track is placed at `loop * clock / period` units past its fire key. On the
  update where the clock reaches EDuke32's period it gets exactly the rest of
  the loop, so the fire key ends in that update. Desert Eagle 5 tics (50
  units, 6/s), Gatling 3 (30, 10/s), Freezer 4 (40, 7.5/s). The position
  past the fire key is read from the track (`+6`, `+2`) and the original's
  key tables, so a late or early start corrects itself.
- At the shot the clock keeps the overshoot of the update that crossed the
  period (the average rate holds). Lateness from a slow update is not owed, so
  there is no catch-up burst after a hitch.
- Freezer: the gun-up update (32) counts towards the clock, and the key after
  the shot is finished at once, so the exit and restart fit in 4 tics.
- The original runs every key, event and shot test: ammo, the clip reload,
  sounds, muzzle flash and damage are its own. Shotgun and RPG keep their
  rhythm (they already repeat faster than Duke 3D: 1.24 / 2.24 per s against
  1.0 / 1.5). The TTK-only weapons, Energy Weapon and throws are untouched.
  MODERN and Vanilla are unchanged.
- Trace (`DNTTK_AIM_TRACE=1`): `ttk-feel-shot weapon gap clock upper` per gun
  call. Debug `controls.weapon_feel.cadence_updates`, `last_gap`.

## Evidence

`recomp/analysis/d10h4/held.py` (fire held 3-5 s after a fresh level 1,
`dnstuff`, `dnkroz`, private card copies, offscreen; gaps from `ttk-launch`
host times, update and clock gaps from `ttk-feel-shot`), `keys.py` (key
tables), `clock.py` (frame step and `+0xb0`). Player files hash-checked
unchanged after every run.

| Weapon | Before (RETRO = MODERN) | After, RETRO | EDuke32 |
| --- | --- | --- | --- |
| Desert Eagle, 180 Hz | 167-237 ms, 6 or 7 updates, 4.7/s | 5 updates every shot; 166-167 ms, 150-184 around three-field updates; 5.99/s | 167 ms, 6.0/s |
| Desert Eagle, 60 Hz | - | 167 ms for 16 of 17 gaps (one 184) | |
| Gatling, 180 Hz | 8.5/s | 30 game-clock units per shot; 83-117 ms, mostly 100; 9.99/s | 100 ms, 10/s |
| Freezer, 180 Hz | 5.7-6.0/s | 133-134 ms (40 units) while updates are two fields; 150-167 when the lab game ran three-field updates | 133 ms, 7.5/s |
| Shotgun, RPG | 1.22-1.24, 2.23-2.24 /s | unchanged | 1.0, 1.5 |

- First shot of a press unchanged from D10H4 (Desert Eagle 71-110 ms from
  rest, Gatling 86-109, Freezer 87-104, RPG 112-142).
- Suites: `ttk-controls-test` (camera fixture, LEVEL01, all levels; 152 PASS
  lines), `ttk-input-test`, `ttk-aim-test`, Python `test_player_profiles`
  (125) pass; `check_repo.py` clean.

## Playtest 1 (2026-11-05): uneven rate after a reload is CPU SPEED

User: "It feels good, but the shoot rate for the desert eagle still feels
"inconsistent", especially after a reload. just holding down fire seems to
have a variable fire rate". Held 9 s through two reloads at the player's CPU
SPEED 150: every shot was paced (50 clock units, 55 when an update crossed
the boundary), but under fire the game drops to three-field (50 ms) updates,
more often after the first reload (about 3.7 s of firing). A shot can only
fall on an update, so gaps ran 150-184 ms. Before firing every update was two
fields. The same hold at CPU SPEED 200: 42 of 42 gaps exactly 5 updates and
50 units, 165-168 ms, before and after both reloads. The reload itself is
about 1.1 s every 22 rounds (original). Host CPU cost of 200 was not measured.

## Not verified / limits

- The real mouse on the player's 180 Hz display, and whether 6/s with the
  original pistol animation looks right (the loop now plays about 1.3x
  faster). The user judges.
- Game updates are the quantum: a three-field update makes one gap 183 ms
  (or 150 after it); the fire CPU below keeps updates at two fields on land. The Freezer's cycle needs at least 4 updates, so it
  falls to 6/s whenever the game runs three-field updates (seen in the
  offscreen lab while the Freezer fired).
- The Desert Eagle clip reload (about 1.1 s every 22 rounds) is the
  original's; EDuke32 reloads every 12.
- Shotgun and RPG were left faster than Duke 3D; slowing them to 1.0 / 1.5
  per s for a 1:1 match is a user choice.
- Swimming and climbs: no pacing (as for the D10H4 boost).

## Fire CPU (2026-11-05): steady at CPU SPEED 150

User: "Fix D10H6's uneven rate without changing CPU SPEED ... run the game's
updates faster only while a RETRO weapon is firing, so swimming and movement
are untouched."

- `weapon_feel_cpu` (`weapon_feel.inc`, from the player update): while fire is
  held on a RETRO weapon (Desert Eagle, Combat Shotgun, Gatling, RPG, Freezer)
  and for three updates after, the runtime overclock scale
  `g_psx_overclock_q16` (`psx_cycles.c`, already exported; no framework edit)
  is set to `DNTTK_WEAPON_FEEL_CPU` percent (200 by default, 100-400, never
  below the player's CPU SPEED), then back to the player's setting. The
  overclock lease, its safety net and its menu/movie/load exemptions are
  unchanged. MODERN and Vanilla never boost.
- Not in water (`in_water_early()`): Modernized swimming moves a fixed
  distance per player update (`pace_nominal_dt` 10.5 in
  `modern_controls.cpp`), so more updates per second swim faster. That is why
  CPU SPEED 200 changed the water strafe; land motion scales with the game's
  frame step. Firing while swimming keeps the CPU SPEED 150 rhythm.
- Debug `controls.weapon_feel.cpu_boost`, `cpu_boost_updates`.

Evidence at the player's CPU SPEED 150, 180 Hz, offscreen, private copies
(`held.py` with `CPU=150`; `moves.py`). Player files hash-checked unchanged.

| Hold | Before (playtest 1) | After |
| --- | --- | --- |
| Desert Eagle 9 s, one reload | 150-184 ms (148-199) | 42 of 43 gaps 165-172 ms, before and after the reload (the other is the 1.1 s reload) |
| Gatling 6 s | 83-117 ms | 97-103 ms, median 100 |
| Freezer 6 s | 133-167 ms | 132-135 ms, median 133 |

Land movement while firing, units per second, boost off (100) / on (200):
strafe left 629 / 636, strafe right 665 / 641, back 640 / 641, forward 623 /
636, run forward 2916 / 2923, run back 2898 / 2925; without fire 630-652 /
631-636. The same within measurement noise. Host process CPU during the holds:
55-61% of one core.

Suites: `ttk-controls-test` (camera fixture, 147 PASS, exit 0; its fake
runtime gains `g_psx_overclock_q16`), `ttk-input-test`, `ttk-aim-test`,
`test_player_profiles` (126) pass; `check_repo.py` clean.

Water (`water.py`, a private copy of the user's UI save slot 2, runtime 1,
walk forward into the water; swim mode 5, submerged; Desert Eagle): 6-9 shots
per 2 s strafe and 0 boosted updates; swim strafe left / right 616-637 /
653-675 units per second with or without fire, the same with the boost forced
off (100). User playtest 2 (2026-11-05): "it's playable still, the
difference feels subtle".

Not verified: the real mouse on the player's display, climbs. The swim pace's dependence on CPU SPEED itself
(`pace_nominal_dt`) is an open finding, not changed here.

## Playtest 3 (2026-11-05): the rate changed while running

User: "the weapon rate of fire changes when duke moves". `moving.py` (CPU
SPEED 150, 180 Hz, fire held 2.5 s per move): standing, walking, strafing,
backing up and diagonal runs fired every 167 ms on track 16, but running
forward, back or sideways the Desert Eagle fires from upper track 10 (keys 57
(0, 20), 58 (8, 20), 59 (1, 30): the same 70-unit loop, fire event on the
second key), which was not paced: 199-235 ms, the original 6 / 7 update
rhythm, about 4.6 per second. Gatling, Freezer and Combat Shotgun keep their
standing tracks while running.

Fix: `weapon_feel_fire_on` treats track 10 as a Desert Eagle fire track (the
D10H4 first-shot boost and the D10H6 pacing). After: running in every
direction 165-168 ms; holds that switch between running and walking two or
three times keep 166-168 ms (only reloads, 1.03-1.10 s, break the rhythm);
crouching, crouch-walking, crouch-strafing and running jumps 160-175 ms.

Original behaviour left alone: a run started against a wall plays Duke's
wall-bump stumble (body 95), during which the gun drops through upper
tracks 9 and 5 before raising again (15, 16); no shot can leave for about
500 ms. Pacing resumes at 167 ms once the gun is up. EDuke32 has no such
stumble; removing it is a separate choice.

Suites after the fix: `ttk-controls-test` (camera fixture, 147 PASS),
`ttk-input-test`, `ttk-aim-test`, `test_player_profiles` pass.

## Revision 4 (2026-11-05): water, a launcher switch, no wall stumble

User: "i can confirm that duke's firing rate is actually different in the
water. I'd say add a flag in", and "that stumble has gotta go".

- Water (`water_rate.py`, a private copy of UI save slot 2): the Desert Eagle
  fires from track 10 there too, but pacing skipped water (its readiness test
  had no swim case): 185-251 ms, about 5 per second. `swim_weapon_ready()` now
  joins the test. The boost now runs in water too: swim strafe speed and
  acceleration measured the same at CPU SPEED 150 and 200 (`water.py`
  `RAMP=1`: 0.15 / 0.3 / 0.5 / 1.0 s distances alike; steady speed 608-677),
  so the earlier "fixed distance per update" explanation was wrong and the
  water exclusion is gone. After: standing in water 164-191 ms; moving, the
  average is 6 per second but gaps alternate 145-207 ms, because firing in
  water drops the game to three-field updates about a second in, at fire CPU
  200, 300 and 400 alike (without fire it stays two-field; no safety-net
  events). What the game waits for there is not established.
- `run.py --fire-cpu off` (this launch only; not saved) passes
  `DNTTK_WEAPON_FEEL_CPU=100`. Test `test_fire_cpu_is_one_launch_only`.
- Wall bump: a run into a wall stores the stumble 94/95 at `0x800538b4`,
  plays the thud `0x8006b270(0x2007)` from `0x800538c4`, then an alert and
  returns; the original's plain stop (`0x800537ec`..`0x800539dc`) stores idle
  63 and zeroes `+0x68`/`+0x6a`. With a RETRO gun up, the thud call's entry now
  rewrites 94/95 to that stop (`weapon_feel_bump`), so the gun stays up; thud,
  rumble and alert stay, landings keep 94/95. Measured: runs into the same
  wall keep 164-172 ms with no 94/95 (before, one 500 ms gap). A later hold
  against that obstacle starts a full-body move (216; MODERN shows 218 there
  too). Debug `controls.weapon_feel.bumps_stopped`.


## D51 (2026-11-08): every drawn weapon

The wall-bump swap above applied only to the paced guns, so the Energy
Weapon and Flamethrower still stumbled: in a private copy of UI slot 8 the
held beam stopped for 633 ms at the wall. `weapon_feel_bump` now applies
with any drawn weapon (+0x3b8 2) under Modernized RETRO weapon feel. After:
both weapons keep 33-37 ms between rounds running back, right and forward
into walls, first and third person; no 94/95. Lab `recomp/analysis/d51/`.
