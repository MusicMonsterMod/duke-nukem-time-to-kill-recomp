# D20E - Statistics research: enemy total, shots, hits

Job: [D20E](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-22). Builds on
[note 181](181-d20d-richer-statistics.md) (D20D). Static reading of the owned
SLUS-00583 executable and level overlays, then offscreen runs with private
cards (`recomp/analysis/d20e-stats`).

User decision (2026-10-22): accuracy is not wanted on the statistics screen
and does not need a backlog job; the findings below are kept in case a later
job needs them. Damage taken is parked for the same reason (the original has
no counter; see the end).

## Kills: player `+0x3b6`

The kill credit routine `0x8003d8a4(killer, victim)` is called only when the
killer's type is Duke (`0x20`): from the character death `0x80063f0c`
(`0x80063f7c`), `0x80043630`, `0x80082dec`, and from the per-level death
handlers in 18 overlays. It switches on the victim's type (`+0x2c`), adds to
an unused per-type tally (`+0x39c..+0x3ac`; `+0x3ae` is the two-player frag
count, -1 for a self-kill), then adds 1 to `+0x3b6`. Not counted: types
`0x7a`, `0x7b`, `0xa7`, and in one-player a victim of type `0x20` (Duke's
own death). Kills survive death and Continue (checked: 1 before, 1 after,
continues 3 -> 2).

## Enemy total ("X OF Y")

The level load `0x800242b0` zeroes the enemies-left count `0x800d253c`
(signed half) and adds 1 for each placement record of kind 1 whose type is
not `0x20`, has bit 0 in its type flags (`*0x800d2660 + 28*type`), has a
valid map cell, and is not one of `0x7a 0x7b 0xa7 0xcc 0x112 0x13f 0x158
0x21a 0x21c 0x243 0x25d`. The rows are copied into the placement table
(`*0x800de720`, 48 bytes, type at `+0`, count `0x800de724`).

Deaths subtract 1 (`0x80063fd8`, `0x80097590`, and in the overlays right
after their kill credit). Its readers: the challenge-stage HUD (levels
21-26), the stage-complete check `0x80095034`, the bosses' checks (levels
27-29) and a level 11 event (-9 threshold). The only runtime increment is
challenge stage 25's spawned type `0x7f`.

**The live count cannot stand for "dead so far"**: the strip-club dancers
(types 922, 1013, class 8, 1 HP) are in the count, and killing one credits a
kill (`+0x3b6` +1), but nothing subtracts them. So Y is the load rule applied
to the placement table, which keeps its rows through deaths, Continue and the
statistics screen.

Counted types seen: 26, 57, 95, 1044 (hostile class 6), 50 (flying, class 7),
622 / 623 (gun turrets, class 15), 847 (class 27), 922 / 927 / 1013
(dancers, class 8).

| Check | Result |
| --- | --- |
| Rule vs the game's count at level start, levels 0, 1, 2, 5, 6, 9, 21 | equal: 60, 58, 47, 59, 44, 30, 18 |
| Difficulty 0 vs 3, levels 0, 1, 5, 9 | same totals (no difficulty filtering) |
| Real kills (Vanilla, original auto-aim): 57 and 26 on level 0, 57 on level 1, turret 622 on level 5 | each: kills +1, enemies-left -1 |
| Dancer 922 (level 5), 1013 (level 9) | kills +1, enemies-left unchanged |
| Death + Continue, level 0 | kills, enemies-left and the dead enemy kept; rule still 60 |
| Statistics screen (`endlevel`, level 2) | table intact, rule 47 |

Shown since D20D's screen as `ENEMIES KILLED: X OF Y` (`game_menu.cpp`,
`enemy_total`); the plain count if the table does not read sensibly.
Limits: a full clear (X = Y) was not played; dancers are part of Y, as the
game itself counts them; challenge stage 25 spawns extras (challenge stages
show no kills line).

## Shots and hits (not shown)

- `+0x3b2` shots: incremented in the weapon fire code (`0x8004f7b8`,
  `0x8004fae0`). Equal to ammo used for weapon slots 1-11 (pistol 4,
  shotgun 5 one per trigger).
- `+0x3b4` hits: incremented when Duke is the attacker in damage handlers
  1-3 of the per-class table `0x800c5ecc` (index: victim `+0x16`): class 1
  (many breakable props), 2 (Duke, so self-splash counts), 3 (ground enemies,
  turrets). Not class 6 (flyers, type 50). Each pellet and splash victim
  counts (shotgun: 1 shot, 9 hits). A hits / shots accuracy is therefore not
  meaningful as the game counts it.

## Damage taken (not shown)

The original keeps no counter. It would need our own (summing Duke's health
loss at his damage handler `0x800a40a8`, already hooked for D08A13) and a
savestate sidecar to survive loads.
