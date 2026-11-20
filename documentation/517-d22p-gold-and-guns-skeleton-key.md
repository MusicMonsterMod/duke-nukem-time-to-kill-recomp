# D22P - GOLD AND GUNS: the skeleton key nobody can find

Status: **research answered 2026-11-20**; fix D08A48 built the same day. Both
user-accepted and closed out 2026-11-20 ("lock it in and close it out"). The gate room is the user's own solution (the lava-pit drop and
the lever on the far side, 2026-11-20) and is not part of this answer.

## Question

User, 2026-11-14 and 2026-11-20 (UI slot 10, GOLD AND GUNS): the mission
inventory lists a SKELETON KEY for this level, but where is it, and what lock
is it for? "why there is a mystery key in the inventory that seemingly has no
need to collect? i dont know where the lock is for this key."

## Answer

**GOLD AND GUNS has no skeleton key and no lock for one.** The game names a
key for the level in its own item-name table, but nothing in the level gives
it and nothing takes it. It is a leftover: a name with no item behind it,
most likely from content cut or never built. There is no lost room or hidden
pickup behind it.

**Why the player sees it: our mission inventory (D08A5).** The original Select
screen lists a mission item only once Duke has it (flag bit 0), so in the
retail game this key never appears in level 3. D08A5 lists every item the
name table names, found or not, so it shows "SKELETON KEY - NOT FOUND YET" and
a MISSION count of 0/1 that can never complete. That listing is ours: a real
bug in the Modernized presentation, handed to the fix job **D08A48**.

## Evidence

Labels: **Disc** = read from the original files (identical in the retail
game); **Run** = observed on private offscreen recomp runs (Modernized,
SDL offscreen, debug port, private card copies, player files hash-checked
unchanged); **Inferred** = reasoned from the above, not observed.

1. **Which item (Disc).** Level 3's only named mission item is inventory item
   6, "Skeleton Key", from the name function `0x80087d4c` (note 114's table,
   re-derived by `test_mission_items.py`). The user's save (UI slot 10 copy,
   level 3) has item 6 at `0x0`: never found, never used (no D08A19 used
   marker).
2. **What gives item 6 (Disc).** The pickup dispatcher `0x80081a48` sets item
   6 only for pickup types 153, 542, 744 and 862. The full map read from it:
   153/542/744/862 give 6; 154/745/863 give 7; 155/746 give 8; 157 gives 10;
   547/761/854 give 14; 548/855 give 15; 549/856 give 16. Crystals 11-13 come
   from holders (types 176-178, `0x80091cec`).
3. **Nothing in the level gives it (Run).** A fresh start of GOLD AND GUNS
   (console `level 3`): 378 placed objects in 72 room lists, the contents
   (`+0x36`) of every object, and 52 enemy records (contents at `+0x2c`).
   No object of a key type and nothing carrying one. The user's mid-level
   save shows the same (324 objects).
4. **Nothing in the level takes it (Disc + Run).**
   - The disc's LEVEL03.OVR has no call to the shared lock helper
     `0x80092a84`, no CreateObject call, no key type constant, and never
     touches item 6's flag, the hand (`+0x3b8/+0x3b9`) or the use item
     (`+0x3bb`). Every other level overlay whose level has a working key
     calls the lock helper (00, 01, 02, 04, 06, 07, 09, 10, 11).
   - No level-3 object has the item-lock type flag `0x01000000`, which every
     lock needs before the action press asks it for an item (D08A18). Its
     handler table (`0x800cca08`) has one level handler (kind 4,
     `0x800cc388`).
   - LEVEL03.OVR's only other main-executable item call is the enemy-death
     drop (`0x80096b98`), which drops an enemy's recorded contents (ammo
     here).
5. **A trap avoided (Run).** RAM after LEVEL03.OVR does contain a lock call
   for item 6 (`0x800ccd6c`). It lies past LEVEL03's end (`0x800ccbd0`) and
   matches LEVEL02.OVR byte for byte: stale code from the previously loaded
   level, never run in level 3. LEVEL03 itself matches the disc file apart
   from its last 11 data bytes.
6. **Retail (Disc + Inferred).** The level's code (LEVEL03.OVR) and the name
   table, dispatcher and lock rules (main executable) are the original's
   bytes, so the absence of a lock is retail design, not a regression. The
   placed objects come from the disc's level data through the original
   loader, which the recomp runs unchanged; that no key is placed was
   observed in the recomp only. It was not checked in DuckStation (no debug
   port there; the static evidence does not depend on it).

## Other levels (same survey, fresh starts)

| Level | Named items | Given at the start | Lock objects (flag types) |
| --- | --- | --- | --- |
| 0 | 6, 7, 11-13 | 6, 7 by enemies; crystals | readers 478, receptacles 180-182 |
| 1 | 6, 14-16 | all placed | 545 |
| 2 | 6, 7 | 6 placed; **7 nothing** | 430 (overlay asks 6 only) |
| **3** | **6** | **nothing** | **none** |
| 5 | 6, 11-13 | all | 180-182, 728, 832 |
| 6 | 7, 8, 14-16 | 7, jewels; **8 nothing** | 736 (overlay asks 7), 857 wheel |
| 7 | 6, 7 | both by enemies | 827, 971 |
| 9 | 6, 7, 11-13 | all | 180-182, 1002, 1003 |
| 10 | 6, 7 | both placed | 864 |
| 11 | 6 | placed | 961 |

Level 3 is the only level with a named item and no lock at all. Level 2's
item 7 and level 6's item 8 are suspected leftovers of the same kind (no
giver at the start, and the level overlay's lock asks for another item), but
they were not proved: their levels have lock objects whose handlers were not
all read, and a giver could appear later in play. D08A48 must settle them.

## Fix (D08A48, Done 2026-11-20)

The suspects are settled: outside the pickup dispatcher, no main-executable
code writes items 6, 7, 8 or 10 at a fixed offset, so pickups and crystal
holders are the only givers, and level 2 item 7 and level 6 item 8 have none.

A game-wide rule, no per-level data (custom levels stay possible):

- `mission_givable_scan` (`shortcuts.inc`, emulation thread, before
  `mission_update`) walks the room lists, every object's contents and the
  enemy records, maps types through `mission_item_for_type` (the dispatcher
  map above, `inventory_hud.h`), and keeps the union for the level. It
  restarts on level change and savestate load, rescans every 30 updates, and
  reports only after a walk in which every pointer checked out.
- The mission inventory and the statistics screen's MISSION ITEMS list a
  named item when the scanned level can give it or its flag has bit 0 or the
  used marker. A level not scanned yet lists every named item, as before.
- User request (2026-11-20, HOLY TERROR: "no feedback"): in a level with no
  listable items, , / . open an empty panel: one empty grey slot, MISSION /
  NONE, and the card NO MISSION ITEMS (its heading alone, centred on the empty
  frame; user, 2026-11-20). It closes as the
  normal panel does; Enter only closes it.
- Vanilla is unchanged (the mission inventory is Modernized only).

| Level (fresh start, `.`) | Before | After |
| --- | --- | --- |
| 3 GOLD AND GUNS | SKELETON KEY, 0/1 | NO MISSION ITEMS, MISSION NONE |
| 8 HOLY TERROR | nothing | NO MISSION ITEMS, MISSION NONE |
| 0 TIME TO KILL | 0/5 | 0/5 |
| 2 MINER 69ER | 0/2 | 0/1 |
| 6 FAMILY JEWELS | 0/5 | 0/4 |

Evidence: `ttk-inventory-test` (empty panel and card, type map, level 3
filtered and listed again when found or used, levels 2 and 6 counts,
unscanned fallback), `ttk-controls-test`, Python 266 OK (2 skipped); the
table above from a private offscreen Modernized run (player files
unchanged). Not seen in game: the statistics screen line (unit test) and
Vanilla (same gate as before, by code).

## Tools (local, `recomp/analysis/d22p/`)

`probe.py` (cases `survey`, `fresh`, `levels`), `rooms.py`, `contents.py`,
`itemmap.py`, `levels.py`, `rdis.py`; level-3 RAM dumps (save and fresh start).
