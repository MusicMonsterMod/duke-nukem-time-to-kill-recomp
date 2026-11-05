# D10H1 - Weapon feel lab: TTK against Duke 3D analogs

2026-11-05. Status: Done (accepted and closed 2026-11-05). Follows the D10H research report
([note 388](388-d10h-duke3d-weapon-feel.md)). No gameplay change. One
diagnostic-only code change (test input weapon keys). Executable
`74a772eaa52977526c39777b2c50d48710c8d7817a435ead55303e219b3e42e8`.

Both games were measured at runtime, offscreen, on private copies. The Duke 3D
side ran on the user's "enemyshoot" EDuke32 save. The figures replace note
388's source-only estimates.

## The lab (local, `recomp/analysis/d10h1/`, ignored)

- **Duke 3D:** an instrumented private build of the user's local EDuke32
  source (`eduke32_20260807-10669-ec5824db8`, patch
  `eduke32-d10h1.patch`). It logs the fire press, every player shot, each
  hitscan result (the sprite hit, whether it is an enemy, the autoaim target)
  and per-tic positions, with `CLOCK_MONOTONIC` times. It also reads a lab
  input file: fire, weapon key, absolute view angle and pitch, autoaim. It
  runs on Xvfb (`d3run.sh`) with a private home. That home holds copies of
  `eduke32.cfg`, `settings.cfg` and `save0004.esv` ("enemyshoot", E3L1). It
  loads the save, then sets god mode and gives weapons and ammo through
  EDuke32's console, the same as dnkroz and dnstuff. The private
  `settings.cfg` adds `r_maxfps "180"`: Xvfb reports no refresh rate, and
  EDuke32's frame limiter then spins forever. The user's EDuke32 files are
  hash-checked unchanged after every run. One early smoke run wrote
  `eduke32.log` and `grpfiles.cache` into the EDuke32 game folder. Both were
  new files and were removed.
  - `d3lab.py timing`: per analog weapon, 6 short clicks (60 ms) from rest,
    then fire held for 4 s.
  - `d3lab.py hits`: pistol, held fire, the view kept on the nearest living
    enemy (plus an offset), autoaim on (`cl_autoaim 2`, the user's setting)
    and off. The save is reloaded for every row. `d3shots.py` classifies each
    round.
- **TTK:** `ttk_lab.py` on the D10G latency harness. It uses private D10B
  apartment card copies, level 1, `dnstuff` and `dnkroz`, Modernized, third
  person, `DNTTK_AIM_TRACE=1`, and log lines timestamped on the host clock.
  - `timing`: per analog weapon, the weapon is checked by its slot (note 361).
    Press and hold until the round launches, 5 times from rest (1.2 s idle)
    and 5 with the gun up (0.25 s after a shot), then 4 s held. Shots are
    `ttk-launch` lines of that weapon's projectile type (pistol 0, shotgun
    1, Gatling 7, RPG 8, Freezer 13).
  - `slide`: the D10G4 fixture. A spawned pig cop, its health raised to
    30000 so it survives the run, is moved across a fixed crosshair by
    debug writes. Fire pulses 300 ms on and 150 ms off. **Per-round hit
    log:** a round landed when the pig cop's health (`+0x32`, host-polled
    every few ms) falls within 80 ms of the round's launch. Rounds resolve in
    the shot's own update.
  - `ttk_click.py`: the shortest fire press that still fires.
- Code: `pc_input.cpp` test input (`DNTTK_TEST_INPUT`, never set by `run.py`)
  now also knows the keys `2`-`9`, so the lab can select every weapon.
  A first attempt logged hits at the enemy class damage handlers (table
  `0x800c5ecc`). It was removed: function-entry plugins fire only at
  addresses listed in `game.local.toml`, so the hooks never ran, and adding
  them would regenerate code and reject savestates.

## Results (180 Hz)

Press to shot is from the press to the round leaving. "Visible" is the press
to the first image that can show the shot.

| Weapon (TTK key / Duke 3D) | Duke 3D press to shot | TTK press to shot, rest / gun up | Duke 3D repeat held | TTK repeat held | Duke 3D shot to frame | TTK shot to image |
| --- | --- | --- | --- | --- | --- | --- |
| Desert Eagle (2) / Pistol | 58 ms (53-60) | 197 (177-205) / 82 (77-112) | 6.0/s (5 tics), 3.7/s with the 12-round reload | 4.6-5.0/s, 3.5/s with reload | 4-6 ms | about 90 ms (world delay) |
| Combat Shotgun (3) / Shotgun | 123 (104-132) | 425 (355-445) / 555 (541-562, still recovering) | 1.0/s | 1.22/s | 6 | about 90 |
| Gatling (4) / Chaingun | 88 (75-103) | 286 (279-311) / 106 (86-297) | 10/s (3 tics) | 8.4/s | 6 | about 90 |
| RPG (5) / RPG | 108 (106-137) | 230 (215-245) / 236 (193-279) | 1.5/s (20 tics) | about 1.9/s (one gap; only 6 rockets) | 4 | about 90 |
| Freezer (0) / Freezethrower | 81 (72-99) | 294 (257-330) / 112 (98-130) | 7.5/s (4 tics) | 5.0/s | 6 | about 90 |
| Mighty Boot (1) / Mighty Foot | 214 (204-229) | not measured (no projectile) | 2.5/s | - | 5 | - |
| Pipe bomb (6) / Pipe bomb | not measured (throw, no shot call) | not measured (no launch line) | - | - | - | - |

- Duke 3D figures are tic-quantised (30 tics/s). The pistol fires on the tic
  after the press, as the source said. Duke 3D's shotgun, chaingun, RPG and
  freezer are 1-2 tics slower than note 388's source reading because those
  weapons fire after their animation's first frames.
- TTK's "visible" figure is the measured 90 ms world delay at 120/180 Hz
  (D10G latency report). The muzzle flash itself was not traced per present.

## TTK drops short fire presses (new, measured)

**Corrected by D10H4 ([note 390](390-d10h4-shot-on-press.md)):** at this
lab spot Duke is hurt and dies about 13 s after arriving, and hit animations
block fire; most of the drops below were that. On a healthy Duke 30-60 ms
clicks fire; only clicks shorter than a game update (16 ms) drop.

TTK (Modernized, Desert Eagle) fires only when fire is still held as the
weapon reaches its fire frame:

| Press length | From rest (3 each) | Gun up, 0.25 s after a shot (4 each) |
| --- | --- | --- |
| 30 / 60 / 100 ms | no shot | no shot (100 ms: 1 of 4) |
| 150 ms | 1 of 3 | 0 of 4 |
| 200 / 300 ms | 3 of 3 (176-207 ms) | - |
| 500 ms | 3 of 3, plus a second shot | - |

A quick click (about 60-120 ms) fires nothing, while in Duke 3D every
60 ms click fired (6 of 6 for every weapon). This was found through the test
input path. Vanilla could not be compared because the test input runs only
in Modernized. A real mouse click was not measured. It belongs to D10H4
(shot on press: latch the press).

## Per-round hits on enemies

Duke 3D, "enemyshoot", pistol, view on the nearest living enemy (the walking
lizman sprite 683 at 700-1300 u, then the trooper 113 at about 4100):

| Autoaim / offset | Rounds | At a visible enemy | Landed |
| --- | --- | --- | --- |
| on, 0 deg | 28 | 18 | 18 (all autoaim-targeted) |
| off, 0 deg | 28 | 18 | 18 |
| on, 4 / 8 deg | 28 / 28 | - | 18 / 18 enemy hits (autoaim corrects) |
| on, 12 deg (outside the cone) | 28 | - | 14 (a second enemy inside the cone) |
| off, 4 / 8 / 12 deg | 28 each | - | 14 / 13 / 14 |

Every round that left on a visible enemy landed. The other 10 rounds per row
went at the drone (111) behind a wall. Autoaim holds the hit rate unchanged
out to 8 degrees off; the source's cone is 8.4 degrees.

TTK, Desert Eagle, slide fixture. Each pig cop moves at about 1900 u/s
across a fixed crosshair (triangle wave, plus or minus 700 u):

| Run | Rounds | Landed | Crosshair query on the pig cop | Of those, landed |
| --- | --- | --- | --- | --- |
| Still pig cop (held in place), 180 Hz | 12 | 12 | 12 | 12 |
| 180 Hz, D10G4 on | 49 | 18 | 17 | 11 |
| 180 Hz, `DNTTK_SHOOT_SEEN=0` | 45 | 12 | 10 | 7 |
| 60 Hz | 37 | 12 | 12 | 9 |

A still target is exact. With the target moving, about a third of the rounds
that left with the crosshair on the enemy still missed (65-75% landed). Some
missed even when the query against current positions also met the pig cop.
That answers D10H2's first question with a yes. Duke 3D lands all of these.
The fixture's position writes come from a host thread and can fall between
the shot query and the round's sweep, so part of this miss rate may be the
fixture. D10H2 must reproduce it with a game-moved enemy.

## Not verified / limits

- **Pistol chase (`ttk_lab.py track`):** spawned pig cops wake and mostly
  stand or walk off. The host tracker (yaw-only `aim_at`, or the D10G4 mouse
  tracker) kept the crosshair on them for only a few rounds. Its runs are not
  evidence. A pitch-aware tracker or a scripted enemy path is needed (D10H2).
- Enemy spawning in EDuke32 was not used: "enemyshoot" gives slow walkers at
  close range plus one occluded drone. Their angular speed is far below a
  running pig cop's, so the Duke 3D hit rate is a perfect-tracking baseline,
  not a matched chase.
- The Boot and pipe bomb have no TTK row. The RPG repeat comes from one gap.
- Third person only; first person, climbs and swimming were not run here.
- The EDuke32 runs used the lab input file, not a real mouse. The TTK runs
  used the test input file. Both sample input once per rendered frame.

## Re-running after each later job

From `recomp/analysis/d10h1/`: `python3 d3lab.py timing`,
`OFFSETS=0,4,8,12 python3 d3lab.py hits`, `RATE=180 python3 ttk_lab.py
timing`, `RATE=180 SPEEDS=0,1900 python3 ttk_lab.py slide`, and
`python3 ttk_click.py` (with `REST=0.25` for the gun-up case). Results go to
JSON files beside the scripts.
