# D08A49 - Key cheats give only the mission items the level can give

2026-11-23 (overnight session). **Done, user-accepted** ("solid work!!! lock it in and close it out"). Job:
[D08A49](../jobs/items.md#d08a49---key-cheats-never-give-a-mission-item-the-level-cannot-give).

User, 2026-11-23: "when doing dnstuff etc on gold and guns, it gives you that
mystery key remember? that key we prevented showing in the mission items
because it doesnt really exist. prevent that."

Private runs only: lab `recomp/analysis/d08a49/` (local; its own card copy,
offscreen, debug port; the player's saves and settings hashed before and
after every run: unchanged).

## Cause (MEASURED)

The key-granting cheats (`dnstuff`, `dnkeys`, `dnitems`) call the original
key grant (`0x8003d840`), which sets bit 0 ("held") on every mission item
6..16. D60 already put back an item used at its lock. D08A48 lists an item
in the mission inventory only if the level can give it (the
`mission_givable_scan`: placed pickups, every object's contents, every
enemy's drop) or Duke holds or used it, so a held phantom item (GOLD AND
GUNS's Skeleton Key, item 6, that no pickup gives and no lock takes; note
517) shows again.

## Change

`cheats.inc`, `shortcuts.inc` (Modernized; Vanilla keeps the original
cheats: its cheat path is the game's own, not this one).

- Around the key grant, each mission item's flag and amount are read
  before. After the grant, in Modernized, an item the level cannot give (not
  in the D08A48 scan's set) and that Duke did not hold before goes back to
  exactly what it was: no new phantom item. D60 is kept: an item used at its
  lock stays used.
- The scan is run at once when the cheat is typed (a cheat in the first half
  second of a level or after a load). Only a scan that finished on this
  level since it began or since the load counts (`givable_scan_ready`); if
  it could not finish (a pointer failed its checks), the grant stays as the
  original.
- An item Duke already holds is never taken away, even if the scan does not
  list it: after a load the scan sees only the pickups still in the world, so
  a key picked up before the save would look "not givable".
- Developer trace `DNTTK_CHEAT_TRACE=1`: `[TTK cheat] level N key grant:
  givable MASK, withheld MASK (scan ready|not ready)`; the line is also
  printed whenever an item is withheld.

## Evidence

Mission flags 6..16 after each cheat (lab case `cheat`: the slot, a forced
"used" item, a cheat at once after a load, then every level's fresh start
through `level N`, each cheat in turn):

| Start | Givable (scan) | After `dnstuff` / `dnkeys` / `dnitems` |
| --- | --- | --- |
| Level 3 GOLD AND GUNS, fresh | none | all 0: no Skeleton Key; the panel stays MISSION / NONE |
| Level 0, fresh | items 6 7 11 12 13 (`038c0`) | those set; 8-10 and 14-16 withheld (`1c700`) |
| Level 1, fresh | `1c040` | items 6 14 15 16 |
| Level 6, fresh | | items 7 14 15 16 |
| Levels 2, 5, 7, 9-11 | | each gets its own level's items |
| Levels 8, 12, 21-29 | none | all 0 |
| Level 0, item 6 marked used, then each cheat | | item 6 stays used (`0x4000`), the others as above (D60) |
| A cheat at once after a load | | scan ready: same result |
| Vanilla, level 3 | | the Modernized path does not run (no trace line); flags untouched |

The D57B lab save (the user's GOLD AND GUNS save of 2026-11-23) already holds
every item 6..16 from an earlier cheat: the cheats leave it as it was (an
item held is never taken away). A fresh start of the level, or a save made
before the cheat, shows the fix.

## Limits

- Items already held from an earlier cheat stay held (see above). The user's
  GOLD AND GUNS saves made after a `dnstuff` keep the phantom key until the
  level is started again.
- `dninventory` does not call the key grant and gives no mission items (as
  before).
