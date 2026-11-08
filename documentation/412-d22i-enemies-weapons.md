# D22I - Enemies and weapons per level

Status: Done (user-accepted; closed 2026-11-08). Parent: D22. Fills the **Enemies** and
**Weapons** columns of the [D22E campaign matrix](330-d22e-campaign-matrix.md).
All evidence is automated (label **A**): private offscreen runs, no user
playtest. Bosses (levels 27-29) are D22J; here they only show they take
damage.

Executable `d2d6c09472cbe455b0bb05e1f85d324a216606b9466f6c9b766f25abb0c88570`
(adds the spawn record fix below to the D08Z9 baseline).

## Method

Lab `recomp/analysis/d22i/` (local, not tracked): `sweep.py`, `pickups.py`,
`merge.py`, built on the D10G/D10H1 lab (`analysis/d10g-latency/lab.py`,
`analysis/d10h1/ttk_lab.py`). Each run is an offscreen SDL instance with
private copies of the lab cards and profile, dummy audio, its own debug
port, first person, Modernized, weapon aim view and AUTO AIM off. The
player's saves and profile are hash-checked before and after every run.

Per level (`level N`, `dnstuff`, god mode set only if off):

- **Enemy pass:** every hostile type the level's spawner records hold
  (`spawn enemy` list; class 8 non-hostile types left out). `spawn enemy
  <type>`, watch 2.5 s (reaction = it moved or closed in), then aim the view
  at its `+0xEC` point with real mouse counts and fire the reference weapon
  (Combat Shotgun) until its dead bit (`+0 & 2`) or its spawner record lets
  it go, up to 14 s.
- **Weapon pass:** every weapon slot after `dnstuff` (Mighty Boot left out:
  key 1 does not cycle to it while armed; kicks are D12/D36), drawn with its
  number key, fired at a living target of a type the enemy pass damaged. The
  target is reused with its health refilled between weapons; each weapon
  fires until the first damage (health `+0x32` falls) or a kill.
- **Line-of-fire witness:** `DNTTK_AIM_TRACE` logs, per shot, whether the
  round's own line meets the target (`ttk-aim-check check=2,<actor>`). A try
  with no damage is repeated from a new spawn in another direction (four
  directions), so walls are not counted against a weapon.
- **Kills:** player `+0x3b6` (D20I).
- Ammo is refilled with `dnstuff` before every weapon draw.

A cell is the best result across the saved runs (`merge.py`; runs a, b, c,
g1-g6, 2026-11-07/08). "Damaged" means health fell; "died" means the dead
bit or the record released it.

## Pickup census (all 15 campaign levels)

`pickups.py`: on a fresh `level N`, before any cheat, the placement table
`*0x800de720` (rows past the spawner count `0x800c56a0` up to `0x800de724`
are objects). Every weapon or ammo row (the D26E pickup types) has a live
object of the same type, and every one of those types has its model in the
level's model table (the original's rule, as `spawn_model_ready`). No
placed weapon or ammo pickup fails to appear in levels 0, 1, 2, 3, 5-12 or
27-29. Placed per level: 14, 24, 16, 20, 26, 28, 4, 31, 13, 16, 15, 23, 4,
25, 5. Weapon and ammo carried as contents by containers and enemies
(`+0x2c`) are listed in `pickups.json`; they appear only when the holder is
destroyed and were not followed.

## Results

Types are the spawner type numbers; names only where verified: 57 pig cop
(D10H1), 1044 vest pig (note 400), 622/623 robotic turrets that drive
around (user, D44C), 540 and 605 turrets (user, 2026-11-08). Class is the actor's `+0x15`.

| Idx | Enemies (type: result) | Weapons |
| ---: | --- | --- |
| 0 | 26 died; 57 damaged; 204 died | 14/14 hit, 8 killed |
| 1 | 26 died; 57 died; 540 (gatling turret) explosives only, original design (U) | 14/14 hit, 7 killed |
| 2 | 26 died; 57 died; 1044 closes in and dies by itself; 847 untested | 14/14 hit, 7 killed |
| 3 | 26 died; 57 died; 605 (turret) cannot be harmed, original design (U); 1044 dies by itself; 44 (flyer) see level 7 | 14/14 hit, 11 killed |
| 5 | 26 died; 622 (robotic turret) damaged | 14/14 hit, 7 killed |
| 6 | 26 died; 57 died | 14/14 hit, 7 killed |
| 7 | 57 died; 95 died; 44 (flyer) killed in play, Desert Eagle hitscan (U) | 14/14 hit, 8 killed |
| 8 | 26 died; 57 damaged | 14/14 hit, 8 killed |
| 9 | 44 (flyer) see level 7 (U); 57 damaged; 95 died; 204 died; 623 (robotic turret) died | 14/14 hit, 9 killed |
| 10 | 26 died; 57 died | 14/14 hit, 7 killed |
| 11 | 95 died; 44 (flyer) see level 7 | 14/14 hit, 7 killed |
| 12 | 57 died; 848 untested | 14/14 hit, 8 killed |
| 27 | 61 (boss) damaged | D22J |
| 28 | 901 (boss) damaged | D22J |
| 29 | 62 (boss, Moloch) damaged | D22J |

The 14 weapons: throwing knife, throwing axe, crossbow, Desert Eagle,
Combat Shotgun, buffalo rifle, gatling gun, RPG, flamethrower, energy
weapon, freezer, pipe bomb, holy hand grenade, dynamite. Every one damaged
an enemy in every non-boss level with view aiming and AUTO AIM off. "Killed"
counts the weapons that also finished the target within the trial; the
others stopped at first damage by design.

Reaction was measured only as movement in the 2.5 s after the spawn. Types
26, 57, 95, 622 and 1044 moved 960-3700 units in at least one level (walked
in, fled or charged). The stationary types (204, 540, 605) hold position by
nature, and 623 and the bosses moved under 300 units. Many single trials
moved under 20 units (turning or attacking in place is not read), so a
missing move is not a missing reaction.

### Findings

1. **Type 540, gatling turret (level 1): bullets do not hurt it.** Shotgun
   rounds met it on the line 50 of 56 times over four spawns without any
   health change. One RPG round or one pipe bomb destroyed it. **Original
   design (U, 2026-11-08):** "that's actually how the game plays. you have
   to destroy it with an explosive." The user noted it is not obvious,
   since level 0's turrets fall to the Desert Eagle, but chose to keep it
   unchanged. Not a defect; D22I2 no longer covers it.
2. **Type 605, turret (level 3): no damage from anything tested, by
   design.** Health reads 60000 (`-5536` signed). Shotgun (25 rounds on
   line), RPG (28 on line), pipe bomb, dynamite and Desert Eagle left it
   unchanged. **Original design (U, 2026-11-08):** "this enemy is quite
   simply a turret. the turret cannot be harmed, you have to deactivate
   it". The agent first misread the spawn screenshot (the turret drawn over
   a pig cop) as a walking robot; that reading is withdrawn.
3. **Type 44 is the flying enemy (U, 2026-11-08, level 7, user's UI slot
   8):** "they are your flying enemies. they are famous in this game for not being easy to kill, only really the energy weapon works on them. however with our hitscan logic, you can shoot them now with the desert eagle. this is amazing and a much welcomed change to the game. well done accepted". So the flyers die in play in Modernized, with the
   Desert Eagle through AUTO AIM HITSCAN (D10H3) and the energy weapon. The
   sweep could not test them: once spawned they fly off along fixed lines
   above Duke and no shot's line met one. Types 847 (level 2) and 848
   (level 12), classes 27 and 52, behave the same way in the sweep and are
   still untested. After such spawns, later spawns in the same level often
   fail ("Spawn could not complete"). Tool limitation, D22I1.
4. **Type 1044, vest pig (levels 2, 3)** closes in and dies on its own
   every time (no shot needed): consistent with a suicide bomber. Not a
   failure; its damage to Duke was not measured (god mode).
5. **Dormant scripted rows (fixed).** In levels 0-2 `spawn enemy 57` could
   pick a row a level script holds (`+0x16` half set; levels 0-2 wake them
   through `0x80091828`). Its actor is created dormant (actor `+0` bit
   `0x10000`), facing away, and neither moves nor takes damage (three runs,
   140 rounds on line). `spawn.inc` now ranks ordinary rows first, then rows
   not in play, then script-held rows. After the fix the level 0 pig cop is
   damaged and dies. Debug console only; no gameplay path uses it.

### Harness limits (not game results)

- One weapon draw per pass missed in level 9's key 9 group (buffalo rifle /
  crossbow), a different member each time; both drew and hit in other
  passes and levels. Recorded as harness timing, not a switching bug; no
  user report matches it.
- Enemy records are retired when their enemy dies, so a level allows only
  as many spawns of a type as it places; some cells needed a fresh level
  load (runs g1-g6).
- Damage to Duke, enemy attacks, gibs, death animations, drops and the
  upgraded weapons (27-29) were not measured. Freezer: a damaged or killed
  target counts; the frozen state itself was not read.
- Third person was not swept (first person only); the aim adapter is the
  same for both (D22D, D10H3).

## Tool change

`recomp/src/ttk/spawn.inc` (`spawn_enemy`): row preference by rank
(ordinary row 0, in play +1, script-held `+0x16` +2) instead of "first, then
one not in play". Contract note 111 updated.

## Follow-up jobs

- **D22I1** - spawn tool support for path, wall and ceiling creatures
  (classes 21, 27, 52: types 44, 847, 848), wall-mounted turrets (540) and
  the spawner failing after them; then sweep those types.
- **D22I2** - closed without work: the user settled both damage rules
  (540 explosives only, 605 must be deactivated) as the original design.
