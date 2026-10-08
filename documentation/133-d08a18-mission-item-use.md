# D08A18 - Using a mission item with Enter (research)

Status: **Done (user-accepted 2026-10-08: "accepted and confirmed working!!!").** The user picked mockup B: "it should be
option B, but in the console steel palette, just like our cheat messages ... in
fact the font should be exactly the same as our cheat messages". Built as
below.

## Question

The user picked option C: Enter on the browsed mission item (`,` / `.` mission
inventory, D08A5) makes Duke try to use it, and anywhere it does not apply a
short message such as "CAN'T USE THIS HERE" appears. How does the original use
mission items, and can Enter drive that same path?

## Findings (static reading of the executable, plus Vanilla checks)

**The original has no "use from the inventory" for mission items.** The Select
screen (`0x800884bc`) lists them, but its selectable check `0x80088134(player,
item)` returns 0 for every item >= 6. The Vanilla check agrees: on a private
copy of the user's UI slot 5 (level 0, Transport Room ID and red crystal held),
Select -> Right -> Down kept the cursor on the jetpack and never reached the
mission rows.

**Duke uses an item by pressing action at the lock.**
- `0x80051890(player)` runs on the action press. `0x80077f28(player)` finds
  the object in front of Duke, at `player+0x174` (it writes only `+0x174` and
  `+0x178`). The action press then classifies that object by its type flags
  (`*0x800d2660 + 28*type`, word 0). A type with `0x01000000` and none of
  `0x08000000` / `0x00400000` / `0x00800000` takes the item path: interaction
  mode `player+0x27c = 3`, state `+0x27d = 1`, target `+0x290`.
- Every object has a class handler `handler[object+0x19](player, object, mode)`.
  The table is at `*0x800dd84c`, in BSS: level 0 runtime `0x800ccd64`. Mode 0
  answers "can it be used now" (0 = yes). Mode 2 answers "which item it
  wants". Mode 1 performs the use and mode 3 completes it.
- State 1 of `0x800513e4` asks mode 2 for the item, writes it to
  `player+0x3bb` and sets `player+0x224 |= 0xc`. `0x8004d690` then starts
  animation 47. At its first frame (`0x8004e494`) the item's flag
  `player+0x354+4*item` bit 0 is cleared and Duke holds it: `+0x3b8 = 1`,
  `+0x3b9 = item`.
- Shared lock helper `0x80092a84(player, mode, object, item, target1, target2)`
  is called by the level overlays' handlers. Level 0's kind-1 handler at
  `0x800cc14c` calls it with items 6 and 7.
  - Mode 0: returns 1 if the lock is already open (`object+0x35`), else 0 if the
    item is owned or held, else 1.
  - Mode 2: returns the item.
  - Mode 1: if Duke holds the item, it marks the lock open.
  - Mode 3: clears the held item (`+0x3b8 = 0`) and fires the targets
    (`0x80091c88`).
- Crystal receptacles (types 180/181/182, handler `0x80092e98`) answer mode 2
  with items 11/12/13.
- **The original uses up a mission item.** Its flag is cleared at animation 47
  and is never restored. After a key is used, the original Select screen no
  longer lists it. The D08A5 mission inventory then shows that key as
  "NOT FOUND YET", the same as an item never found. This was found by code
  reading and has not been seen live.
- The disc has no refusal text. The D24A string table has nothing like
  "can't use", "locked" or "need". The new message is therefore host text.

Level 0 item locks in the level object table `0x800d2550` (UI slot 7 copy):
- crystal receptacles: types 180/181/182 at about (22537..25079, -7069,
  46634..49627), slots 5/7/8;
- crystal holders: also types 180/181 (slots 14/18/37);
- card readers: type 478 (flags `0x01001002`, kind 1) at slots 42/43.

In level 6 (UI slot 2), type 736 (slots 5/27) and type 857 (slot 22) carry the
item flag.

## Built

`mission_use` in `recomp/src/ttk/shortcuts.inc` runs on Enter or U while the
mission inventory is open, in Modernized only. It uses the `original_call`
gates.
The item comes from `mission_browsed_item()` in `inventory_hud.cpp`, and the E
request from `input_request_interaction()` in `pc_input.cpp`. The steps:
0. If the item is not found: "NOT FOUND YET".
1. Call `0x80077f28(player)`, as the action press does, and read `+0x174`.
2. Proceed if that object's type takes the item path described above. Then ask
   its handler for mode 2 through `original_call`, which is the call the game
   itself makes for such objects.
3. If the lock wants the browsed item, the item is found, and mode 0 says it can
   be used: close the mission inventory and start the normal E interaction (the
   same pending interaction E starts in `pc_input.cpp`). The original then
   performs the whole use.
4. Otherwise nothing in the level changes and "CAN'T USE THIS HERE" appears,
   through `input_notice`. That is the same call and style as the cheat
   results: TTK Big Italic, Console steel, navy shadow.

Defaults taken for the open questions:
- one message for every refusal, including a lock that wants another item, so
  no item is named;
- "NOT FOUND YET" for an item not found yet;
- no sound;
- used items are not tracked: the original clears their flag, so they show as
  missing.

## Verified (2026-10-08, build only, private Xvfb Modernized runs on copies of
the user's cards, `analysis/d08a18-use/t8.py`, `t9.py`)

- Level 0 (UI slot 5 copy), away from any lock:
  - Enter on the Subway Security Key (not found) shows NOT FOUND YET;
  - Enter on the Transport Room ID (found) shows CAN'T USE THIS HERE;
  - the jetpack and every flag are unchanged.
- Turned and walked to the empty red-crystal holder (object `0x801d6124`, type
  180), then `.` to the red crystal and Enter. Log: `item 11 at object
  801d6124 type 180 wants 11 blocked 0 -> use`. Then the original:
  - weapon stowed;
  - `+0x27c = 3`;
  - animation 47 with the crystal held (`+0x3b8 = 01 0b`, flag cleared);
  - animation 267, the holder's `+0x35` becomes 1, the held item clears;
  - weapon redrawn.
- Same place with the Transport Room ID browsed: `wants 11 -> refuse`. CAN'T
  USE THIS HERE shows at the top centre in the cheat style (full-window
  capture `t10-e-notice.png`), and nothing changed.
- Suites: `ttk-inventory-test`, `ttk-input-test`, `ttk-controls-test` (44
  groups; stubs added), Python 132 OK (10 skipped),
  `level_overlay_guards.py --check`, `check_repo.py`.

Mockups: section 12 of the local picker
(`recomp/analysis/d24a-fonts/ttk-font-picker.html`, built by `make_picker.py`).
- A: the message on the item card, with the inventory left open.
- B: a centred quote, as "USED STEROIDS" is shown, with the inventory closed.
- C: both.
- The success case adds no new text.

## Not verified

- Card readers and key doors were not used live; only the crystal holder was.
  (Placing Duke by memory writes put him on the wrong floor, `t7.py`; walking
  works.)
- The level overlay handlers for other levels: only level 0's kind-1 handler
  was read.
- What the original does when action is pressed at a lock without the item
  (`player+0x224 |= 0x20`). No message is known; any sound has not been
  checked.

Evidence: `recomp/analysis/d08a18-use/` (local): `t0` slot survey, `t3` Vanilla
Select screen, `t5`/`t6` RAM and object table, `t7` placement attempt.
