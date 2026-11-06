# D10H3 - EDuke32 auto aim for the bullet weapons

2026-11-06. Status: Done, accepted and closed (user: "lock it in and close it out!!"; revision 3: EDUKE32 all / HITSCAN / OFF / ORIGINAL; revision 2: never NPCs). Follows D10H
([note 388](388-d10h-duke3d-weapon-feel.md)) and the D10H1 lab
([note 389](389-d10h1-weapon-feel-lab.md)).

User decisions (2026-11-05): auto aim "on by default, and a toggle to turn it
off", in EDuke32's hitscan-only style (the user plays with `cl_autoaim 2`);
"we go with eduke's code" for the cone.

## Option

OPTIONS > ADVANCED > COMBAT > **AUTO AIM**: **EDUKE32** (default) / **OFF** /
**ORIGINAL**. It is the existing profile key `aim_assist` (values `eduke32`,
`off`, `original-lock`), so `run.py --aim-assist eduke32|off|original-lock` and
terminal settings item 9 work as before. Profile schema 69: a profile whose
Modernized `aim_assist` was `off` (the old default) becomes `eduke32` once; a
chosen `original-lock` is kept, and a schema 69 `off` stays off. Confirming the
MODERN control scheme now presets EDUKE32 (LEGACY still presets ORIGINAL).
Vanilla always runs with `DNTTK_AIM_ASSIST=off` and the hook is gated on
Modernized view aiming. The row moved from ADVANCED > ORIGINAL GAME (where the
2026-10-20 layout review put the original-lock option) to COMBAT beside WEAPON
FEEL, since its default is no longer an original-game setting.

## Revision 3 (2026-11-06): EDUKE32 (all) and HITSCAN

User: "split this out into eduke32 (all auto aim), hitscan only (weapons as
described + buffalo rifle), off, and original".

- AUTO AIM: **EDUKE32** (`eduke32`, EDuke32 `cl_autoaim 1`) / **HITSCAN**
  (`hitscan`, the default, `cl_autoaim 2`) / **OFF** / **ORIGINAL**
  (`original-lock`); the MODERN scheme presets HITSCAN.
- HITSCAN: weapon ids 4 (Desert Eagle), 5 (Combat Shotgun), 6 (Buffalo
  Rifle), 7 / 28 (Gatling).
- EDUKE32: every view-aimed gun call (ids 3-8, 11, 28, 29: Crossbow, RPG,
  Freezer too) and the continuous Flamethrower (9, 27) and Energy Weapon (10),
  whose D22D beam lock becomes the auto aimed enemy. Throws (1, 2, 12-14)
  keep their lob; EDuke32 never aims a pipe bomb.
- Profile schema 70: a schema 69 `eduke32` (revision 2) and `duke3d`
  (revision 1) meant the hitscan behaviour and become `hitscan`; a schema 70
  `eduke32` is kept. A `duke3d` in a menu sidecar or settings is read as
  `hitscan`. Revision 2's quiet rename is replaced by this migration.
- Evidence: `ttk-aim-test` (EDUKE32 aims 3-8, 11, 28, 29 and never throws
  1, 12, 14; HITSCAN aims 4-7, 28 and never 3, 8, 29, 11; the NPC cases) and
  `test_player_profiles` 131 pass (schema 68 / 69 / 70 values). Lab, still
  pig cop: Buffalo Rifle on HITSCAN 4 and 8 degrees 5 of 5 auto aimed and
  landed; EDUKE32 Freezer 4 and 8 degrees 36 auto aimed, 34 landed; EDUKE32 RPG
  both rockets auto aimed at the pig cop (the 80 ms hit window does not cover
  rocket travel). The Flamethrower, Energy Weapon and Crossbow on EDUKE32
  were not run in game.

## Revision 2 (2026-11-06): never NPCs, named EDUKE32

User: "the auto aim wants to target the strippers. it should never, ever
target npc's. let's not actually call it "Duke 3D" but instead "EDuke32"."

- **Cause.** The rooms' targetable lists (the set the D22D beam lock uses) are
  not only enemies. A live dump of level 9 listed pig cops (57, 95; class 6),
  turrets (623; class 15), the two dancers (1013; class 8), a prop (204;
  class 9) and Duke's own type 32 (the player object, already excluded).
  Level 5 lists dancers 922 (class 8); level 1 lists a class 9 prop (540).
  Revision 1 accepted any of them, so a dancer in the cone took the round.
- **Rule.** Auto aim now takes only actors whose type has the hostile flag
  `0x10000000` (`*0x800D2660 + 28*type`, word 0) and whose class is not 8,
  both in the type table (byte 6) and on the actor (`+0x15`). Measured type
  table words: every civilian type with a damage reinforcement (122, 123,
  167, 274, 319, 343, 344, 538, 543, 893, 895-900, 1013, 1022, 1023) and the
  dancers 922 / 927 are class 8 with `0x40000000` and without `0x10000000`;
  enemies 26, 57, 95, 1044 (class 6), the flyer 50 (class 7), turrets 622 /
  623 (class 15) and 847 (class 27) all have `0x10000000`; props 204 / 540
  (class 9) and type 32 have neither. This matches EDuke32, which aims only
  at `A_CheckEnemySprite` actors. An NPC under the crosshair is no longer
  given the exact aim point either: the round goes where the crosshair
  points, so aiming at one yourself still works as before.
- **Name.** The choice is **EDUKE32**, value `eduke32`. A profile saved by
  revision 1 (`duke3d`) is renamed in place with a notice and keeps the choice;
  a menu sidecar or settings value `duke3d` is read as `eduke32`. The
  environment value `duke3d` is not a mode (the launcher never passes it).

Revision 2 evidence: `ttk-aim-test` D10H3 group extended (a dancer type 922
class 8, also under the crosshair; a class 9 prop 540; a hostile type on an
actor of class 8; the old `duke3d` value: never aimed; the pig cop type 57:
aimed) passes with every group; `test_player_profiles` 131 pass (the
`duke3d` rename and its notice). Runtime, level 1 pistol on a spawned pig cop:
0 degrees 17 of 18 landed, 4 degrees 14 of 14 auto aimed rounds landed (4 more
went to another listed enemy), 8 degrees 17 of 18, 12 degrees not assisted.
A dancer test in game was not possible offscreen: there is no teleport, and
a dancer moved by RAM writes into Duke's room kept its old aim point (its
room did not change), so the in-game check at the strippers is the user's.

## What EDuke32 does (source)

`source/duke3d/src/player.cpp`, local EDuke32 build:

- `P_PreFireHitscan` calls `GetAutoAimAng` for `SHOTSPARK1` (pistol),
  `SHOTGUN` and `CHAINGUN`. With `auto_aim == 2`, `A_FindTargetSprite` returns
  no target for the RPG, freezer, shrinker and other projectiles.
- `A_FindTargetSprite`: enemies (and players) whose position lies inside
  `projAng = AUTO_AIM_ANGLE` (48/2048 of a turn) either side of the player's
  angle, with a forward distance (along the facing) above 512, the nearest by
  that distance first; "on screen" vertically:
  `|scale(dz, 10, dist) - (horiz - 100)| < 100`, that is a slope within
  100/160 = 0.625 of the view's; and `cansee` from the enemy's centre to the
  player's eye.
- The pistol (`accurateAim`) then goes exactly at the target with no spread;
  the shotgun and chaingun aim at it and add their spread.

## Implementation (`recomp/src/ttk/weapon_aim.cpp`)

At the authenticated player gun call (`0x8003c500` from `0x8004f7b4` /
`0x8004fa88`), after the D10G3 screen view, the D10G4 crosshair query and the
existing body-to-muzzle cover check, for weapon ids 4 (Desert Eagle), 5
(Combat Shotgun), 7 and 28 (Gatling, upgraded Gatling) only:

- Candidates: the actors in every room's targetable list (room table
  `*0x800DA454`, count `*0x800DA47C`, list head `+0x4C`, node `+4` next / `+8`
  actor; the list D22D uses), alive (flag bit 2 clear), not Duke, at most 32.
- Each is tested at its aim point `+0xEC` (the original lock's point) where the
  shown image drew it (the D10G4 rewind shift), from the shown eye: inside the
  cone horizontally (default 48/2048 either side, about 8.4 degrees), slope
  within 0.625 of the view's, in front of the eye, and more than 256 units
  ahead of Duke's body along the view (EDuke32's 512 is under Duke 3D's eye
  height; TTK's spawned pig cop sits about 3000 units from the third-person
  eye), within 16384.
- Nearest by that depth first, at most four candidates: the eye-to-point and
  muzzle-to-point segment queries must both meet that actor. The round then
  aims exactly at its current aim point. The original's own spread still
  applies (TTK's pistol showed none in the D10G shot logs; the shotgun pellets
  spread round the line).
- Difference from EDuke32: an enemy already under the crosshair (the D10G4
  query met a targetable actor) is tried first, so the player can pick the
  farther of two. It too gets the exact current aim point instead of D10G4's
  rewound hit point. On a running enemy that point still missed about half
  the time (D10H2), and the lab shows the exact point lands.
- An assisted shot skips the D10G1 point-blank bend, as `original-lock` does.
- Diagnostics: `ttk-aim-shot ... autoaim=<actor>,<degrees>,<candidates>` with
  `DNTTK_AIM_TRACE=1`; `aim_debug_json` adds `auto_aim`, `duke_aim_cone`,
  `duke_aim_shots`, `duke_aim_searches`, `duke_aim_misses` and the last
  actor, angle and candidate count.
- Console: `autoaim` reports; `autoaim <1..256>` sets the cone in 2048ths of a
  turn either side for the session; `autoaim default` is 48.

## Evidence

Builds: `psx-runtime` / the game binary, `ttk-aim-test`, `ttk-input-test`.

- `ttk-aim-test recomp/disc/SLUS_005.83`: new D10H3 group passes with every
  earlier group. Weapons 4, 5, 7 and 28 aim exactly at a listed actor 2.3
  degrees off the view; 3, 6, 8, 29 and 11 never; OFF never; not seen from the
  eye, muzzle behind cover, dead, not in the targetable list, off screen
  vertically (slope 0.8), 11.3 degrees off with the default cone: free shot;
  `autoaim 70` (12.3 degrees) then aims; invalid cone values are refused;
  Vanilla writes nothing.
- Python: `test_player_profiles` 131 pass (new `AutoAimDuke3dTest`:
  default, terminal edit, invalid value, schema 68 `off` to `eduke32`,
  `original-lock` kept, schema 69 `off` kept, launcher env, Vanilla `off`,
  in-game menu file).
- Runtime lab `recomp/analysis/d10h3/ttk_autoaim.py` (local; D10G latency
  harness, private card copies, offscreen, level 1, `dnstuff` / `dnkroz`,
  third person, 180 Hz unless noted). A spawned pig cop (health kept topped
  up) about 3000 units away, the crosshair turned the given yaw off it, fire
  pulsed 300/150 ms. "Landed": its health fell within 80 ms of the launch.

| Run | 0 deg | 4 deg | 8 deg | 12 deg |
| --- | --- | --- | --- | --- |
| Desert Eagle, OFF | 18/18 | 0/18 | 0/18 | 0/7 |
| Desert Eagle, EDUKE32 | 32/32 auto | 32/32 auto | 18/18 auto | 0/6 (not assisted) |
| Combat Shotgun, EDUKE32 | 5/5 | 5/5 auto | 5/5 auto | 0/3 |
| Gatling, EDUKE32 | 25/27 | 27/27 auto | 26/27 auto | 0/27 |
| RPG, EDUKE32 | | 0 assisted of 2 | | |
| Desert Eagle, EDUKE32, 60 Hz | | 13/13 auto (5 at another enemy) | 18/18 auto | 0/18 |

  The shotgun and Gatling rows and the Desert Eagle's 8 and 12 degree
  cells ran on the first revision, in which an enemy under the crosshair kept
  D10G4's point instead of being auto aimed (so their 0 degree cells are
  unassisted). Only the 0 degree column depends on that change. The Desert
  Eagle's 0 and 4 degree cells, the 60 Hz row, first person and the
  sliding runs are the final code.

  Sliding pig cop (1900 u/s, plus or minus 350 u across the crosshair, two 6 s
  runs each): EDUKE32 43 of 48 rounds landed (auto aimed at it 40, of which
  39 landed; 7 went to another, nearer enemy); OFF 33 of 48. The one
  auto-aimed miss is within the fixture's known race (its host position writes
  can land between the query and the sweep, note 389). With plus or minus
  700 u the pig cop leaves the cone part of the time, and Duke is hurt by the
  woken pig cop, which blocks fire.

## Not verified / limits

- The real mouse on the player's 180 Hz display: whether EDUKE32 feels like
  EDuke32's autoaim, too strong or too weak.
- The user's "enemyshoot" EDuke32 save comparison and a game-moved pistol
  chase (D10H2's fixture) were not run.
- First person: the lab's turn did not reach the larger offsets at the close
  first-person distance (the crosshair stayed on the pig cop's body), so first
  person was shown only through the crosshair-enemy path (45 of 46 assisted
  rounds landed).
- Swimming, climbs (the climb sidearm is the Desert Eagle) and the jetpack
  share the same hook but were not run.
- Enemies that are not in a room's targetable list are never chosen (the same
  set the D22D beam lock uses).
