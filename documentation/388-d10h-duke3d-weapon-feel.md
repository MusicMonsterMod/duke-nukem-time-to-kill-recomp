# D10H - Weapon responsiveness against Duke Nukem 3D (research report)

2026-11-05. Status: report for discussion with the user. No code, build or
executable change. Follows D10G4
([note 387](387-d10g4-shoot-what-you-see.md)).

User (2026-11-05), after D10G4: "a great test is chasing a pig cop with the
pistol for this, and you can easily fire a trail of bullets at it and not a
single one will land", and: "I want the benchmark to actually be Duke 3D ...
each analogue weapon ... should essentially respond in the same way ... in
terms of their responsiveness". Research first, then small jobs that can each
be tested, iterating towards Duke 3D.

## Sources

- EDuke32 source, local build `eduke32_20260807-10669-ec5824db8`
  (`~/Games/[duke3d]-eduke32-linux/build-eduke32/`):
  `source/duke3d/src/gamevars.cpp` `weapondefaults[]` (weapon timing table),
  `player.cpp` `P_ProcessWeapon` / `P_FireWeapon` (fire state machine),
  `P_PreFireHitscan`, `GetAutoAimAng`, `A_FindTargetSprite` (autoaim),
  `Proj_MaybeAddSpread`, `Proj_DoHitscan` (hitscan), `duke3d.h`
  (`REALGAMETICSPERSEC 30`), `player.h` (`AUTO_AIM_ANGLE 48`), `game.cpp`
  (`smoothratio` sprite interpolation).
- The user's EDuke32 settings: `~/.config/eduke32/settings.cfg` has
  `cl_autoaim "2"` (autoaim on for hitscan weapons, off for the listed
  projectiles).
- TTK measurements already on record: [note 384](384-d10g-aim-mouse-feel.md)
  (press to shot), the local latency report
  `recomp/analysis/d10g-latency/REPORT.md` (90 ms world delay, hit widths),
  [note 21](21-modern-weapon-aiming.md) (pistol rounds are fast swept
  projectiles), D10G3/D10G4 notes 386-387.
- Not yet available: the user's "enemyshoot" save (not on disk as of this
  report). Nothing here was measured in EDuke32 at runtime; its figures are
  read from source.

## How Duke 3D fires (from source)

- The game runs 30 tics per second (33 ms). Each weapon has a frame counter
  (`kickback_pic`). The tic in which fire is pressed sets it to 1; it rises by
  one per tic and the shot happens when it equals the weapon's `FireDelay`.
- Pistol: `FireDelay 2`, `TotalTime 5`. The shot fires on the tic after the
  press: 33 ms after the press tic, 33-67 ms after the click. Holding fire
  repeats every 5 tics, 6 shots per second. No "from rest" penalty: the
  weapon is always up unless switching or holstered.
- The pistol is hitscan (`SHOTSPARK1`): the hit is resolved instantly in that
  tic against current positions, no travel.
- **Autoaim (on in the user's config for hitscan):** `A_FindTargetSprite`
  takes the nearest visible enemy (line-of-sight `cansee`, more than 512
  units away, roughly on screen vertically) inside a horizontal cone of
  48/2048 of a turn either side of the view, about 8.4 degrees. For the
  pistol (`accurateAim`), with such a target the round goes exactly at the
  enemy's centre with **no spread**; without one it gets a small random
  spread (32/2048 angle, 256 height range).
- Display: EDuke32 draws actors interpolated between the previous and the
  current tic (`smoothratio`), so what you see is at most one tic (0-33 ms)
  behind where hits are tested.

Duke 3D analog timings (tics of 33 ms; shot delay = `FireDelay` - 1 tics
after the press tic):

| Duke 3D weapon | Shot after press | Repeat while held | Kind |
| --- | --- | --- | --- |
| Pistol | 1 tic (33-67 ms) | 5 tics (6/s) | hitscan, autoaim exact |
| Shotgun | 3 tics (100 ms) | 30 tics (1/s), 7 pellets | hitscan, autoaim + spread |
| Chaingun | 2 tics (67 ms) | about every 3 tics while held | hitscan, autoaim + spread |
| RPG | 3 tics (100 ms) | 20 tics (1.5/s) | projectile |
| Pipe bomb | throw at 5 tics, hold extends | 19 tics | thrown |
| Freezer | 2 tics | 5 tics, resets while held | projectile |
| Kick (knee) | 6 tics | 14 tics | melee |

## How TTK fires today (Modernized, 180 Hz)

- Press to shot: 91-126 ms with the gun already up, 208-228 ms from rest
  (note 384). Duke 3D pistol: 33-67 ms.
- Pistol rounds are swept projectiles moving about 3500 u per game update;
  within 3500 u they hit in the shot's own update, so travel adds nothing at
  normal ranges (latency report).
- What you see: the world is drawn 90 ms in the past at 120/180 Hz
  (D17 present timeline). D10G3 aims from the view on screen and D10G4 tests
  the crosshair against enemies where drawn, so a crosshair on the drawn
  enemy now picks the enemy. Duke 3D: 0-33 ms.
- Modernized auto aim is **off** by default; the optional `original-lock`
  only follows an existing native lock within 6 degrees. No pistol spread was
  seen in the D10G shot logs (the launch direction equals the aim
  direction).
- Pig cop hit half-width about 150-230 u at about 2900 u.

Press to seeing your shot (flash, impact), estimated: TTK about 100-230 ms to
the shot plus 90 ms before the image shows it, 190-320 ms; Duke 3D 33-67 ms
plus 0-33 ms, about 33-100 ms. This is the "twitchier" feel the user
describes.

## Why a trail of pistol rounds misses a running pig cop

Measured or established:

1. The press delay (100-230 ms). Each click fires after the crosshair has
   moved on; holding the crosshair on the enemy helps only once the gun is up.
2. Duke 3D's autoaim makes most of these shots hit regardless: any visible
   enemy within about 8.4 degrees gets the round exactly. TTK has no
   equivalent on by default, so every round depends on the crosshair being
   on the hit box at the moment the round leaves.

Not established (needs a round-to-enemy hit log on the chase): whether rounds
that leave on target still miss, for example from the order of the game
update (Duke's shot runs before the actors move in the same update, then the
projectile sweeps), the running pig cop's hit box, or the round starting at
the muzzle beside the screen ray. D10G4's slide fixture showed the crosshair
query picks the drawn enemy, but it did not log whether the rounds then
damaged it. That is the first measurement to make.

## Weapon analogs

| TTK (key) | Duke 3D analog | Notes |
| --- | --- | --- |
| Desert Eagle, the starting pistol (2) | Pistol | the user's chase test |
| Combat Shotgun (3) | Shotgun | |
| Gatling Gun (4) | Chaingun | |
| RPG (5) | RPG | |
| Pipe Bomb / Dynamite / Grenade (6) | Pipe bomb | thrown, keep TTK charge |
| Freezer (0) | Freezethrower | |
| Mighty Boot (1) / quick kick | Mighty Foot | |
| Energy Weapon (7) | none (user: "feels great") | keep |
| Flamethrower (8), Buffalo Rifle / Crossbow (9), knife, axe | none | match the nearest Duke 3D class only if the user wants |

## Proposed jobs (iterative, each with its own test)

1. **D10H1 - Weapon feel lab.** A repeatable offscreen measurement per analog
   weapon: press to shot, repeat rate while held, press to the shot visible on
   screen, and per-round hit or miss against a moving enemy (log rounds that
   damage an actor). Scenarios: the user's "enemyshoot" save, a pig cop chase
   with the pistol, the D10G4 slide fixture. Output: a TTK vs Duke 3D table
   that every later job re-runs. No gameplay change.
2. **D10H2 - Pistol chase: rounds that leave on target land.** With the lab's
   hit log, find and fix any reason a round that leaves on the drawn enemy
   misses it (update order, muzzle start, hit box while running). Game-wide.
3. **D10H3 - Duke 3D style auto aim (on by default, toggle off).** Modernized AUTO AIM choice:
   DUKE 3D (hitscan analogs) / OFF / ORIGINAL. DUKE 3D: bullet weapons pick the nearest visible
   targetable enemy within about 8.4 degrees horizontally (tunable in the
   console), roughly on screen vertically, and aim the round exactly at it;
   projectiles, throws and TTK-only weapons unchanged. On by default
   (user decision below).
4. **D10H4 - Shot on press: ready stance.** Keep the gun up while armed in
   play so every first shot takes the gun-up path (about 100 ms instead of
   about 210), then investigate firing on an earlier animation frame towards
   the Duke 3D analog's delay (pistol 33-67 ms). Modernized only; Vanilla and
   ammo, damage and spread unchanged.
5. **D10H5 - Shorter world delay at high refresh.** Bring the 90 ms present
   timeline delay towards Duke 3D's one tic (adaptive, about 55-65 ms first),
   so the world, muzzle flash and impacts show sooner. Judged at 180 Hz on the
   real display for judder; D10G3/D10G4 follow automatically.
6. **D10H6 - Repeat rate parity.** After D10H4, match each analog weapon's
   repeat rate while held to Duke 3D (pistol 6/s, chaingun about 10/s,
   shotgun 1/s, RPG 1.5/s) where TTK is slower, Modernized only.

Suggested order: D10H1, then D10H2 and D10H3 (the biggest felt difference),
then D10H4, D10H5, D10H6, re-running the lab after each.

## User decisions (2026-11-05)

- Auto aim: "on by default, and a toggle to turn it off". The user plays
  EDuke32 with autoaim on hitscan weapons only (wiki.eduke32.com/wiki/Player_setup:
  "If set to hitscan, this automatic aiming will only apply to hitscan-based
  weapons and not missile type projectiles such as rockets or shrink rays").
  D10H3 therefore covers the bullet analogs (Desert Eagle, Combat Shotgun,
  Gatling) only. The wiki calls the cone "approximately 9 degree wide"; the
  source tests 48/2048 of a turn either side of the view (about 8.4 degrees
  each side). User: "we go with eduke's code": the default is the source's
  48/2048 either side (about 17 degrees across), console tunable.
- Native weapon timing may change in Modernized (D10H4, D10H6): "Yes - and
  make it togglable". Vanilla unchanged.
- TTK-only weapons (crossbow, buffalo rifle, flamethrower) keep their timing:
  "No, the dynamic on those is actually pretty good."
