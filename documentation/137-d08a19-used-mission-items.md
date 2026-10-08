# D08A19 - Used mission items: USED and the green tick

Status: **Done (user-accepted 2026-10-08):** "well thats absolutely superb. I fully accept, this is the sort of design extension i wanted, introducing something the original game never had, and it looks completely at home. I'd call this feature finished." Executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8` is the regression baseline.

User request: a mission item that has
been used reads USED in the mission inventory and its icon carries a green
tick (mockup `research/inv/Subway Security Key Used.png`). On delivering the
art: "used, i want full color and green tick".

## Art

- `recomp/assets/ui/items/mark-used.png`: the user's 8x8 RGBA tick (green with
  a dark outline), copied from `research/inv/mark-used.png`.
- Drawn at 2x over the 16x16 icon's bottom-right corner (icon pixels 8..15),
  anchored bottom-right for any size up to 16x16.
- `build_ttk_mission_items.py` packs it as kind 5 of `ttk-mission-items.pack`.
- Local font picker section 14 (`recomp/analysis/d24a-fonts/`) shows it on
  every mission design.

## How "used" is known

For mission items 6..16 the original reads and writes only bit 0 of
`player+0x354+4*i`:
- pickups (`80081a48` cases) `|= 1`;
- the use, at animation 47's first frame (`8004e494`..`8004e4bc`), `&= 0xfffe`
  while Duke takes the item in hand: `+0x3b8 = 1`, `+0x3b9 = item`. The lock's
  mode 3 then empties the hand;
- `80091fc4` gives a held item back (`|= 1`, `+0x3b8 = 0`);
- level reset (`8003ff40`) zeroes all 17 halfwords;
- the Select list (`800885ac`), its selectable check (`80088174`), the lock
  helper (`80092abc`) and the crystal checks test `& 1` only.

Bits 1 and 15 belong to the gadgets (items 1..5) only.

The original keeps no record of a use, so a used item looked like one never
found.

## Built

- `mission_used_track()` in `recomp/src/ttk/shortcuts.inc` runs from
  `select_weapon` on every update while Modernized interaction is alive
  (captured or not; primary player only). For each item 6..16:
  - bit 0 clear, no marker, Duke holding that item -> set
    `ttk::mission_used_bit` (`0x4000`);
  - bit 0 set with the marker -> drop the marker (given back, or picked up again).
- The marker lives in the guest flag halfword. Savestates and card saves
  carry it, a level reset clears it, and no original code reads it.
- `inventory_hud.cpp`: used = marker set and bit 0 clear.
  - Icon: full colour plus the tick, in the strip and on the card.
  - Card status: USED, in font set 12 `mission_used` (system font, `#ffdc30`,
    the mockup's gold).
  - It counts toward found/total.
  - The name keeps the FOUND colour.
- `ttk_font.cpp` accepts up to 16 sets (was 12).
- D08A18 `mission_use`: Enter on a used item says "ALREADY USED" (it said
  "NOT FOUND YET").
- Vanilla is untouched: the tracker runs only behind `interaction_alive()`
  (Modernized).

## Verified (2026-10-08, build, private Xvfb runs on card copies,
`recomp/analysis/d08a19-used/` local)

- `t1.py`, level 0 (UI slot 5 copy), live:
  - Duke walks to the empty red-crystal holder. `.` x3 browses the red
    crystal, then Enter.
  - The original performs the use: holder `+0x35 = 1`, flag
    `0x0001 -> 0x4000`.
  - Reopened: tick on the red crystal in the strip and on the card, USED in
    gold, 2/5.
  - Enter on it: "ALREADY USED".
  - Savestate saved (slot 9), slot 5 loaded (`0x1`), slot 9 loaded back
    (`0x4000`).
- `t2.py`, level 6 (UI slot 2 copy):
  - Writing the original's use (bit 0 cleared, hand = skeleton key 7, then
    hand emptied) marks it `0x4000`. Strip and card show the tick and USED.
  - Setting bit 0 again drops the marker.
  - The same staging in Vanilla is never marked.
- Native renders (`ttk-inventory-test`): used card/strip, found again (no
  tick), never found (no tick).
- Suites: `ttk-inventory-test`, `ttk-controls-test`, `ttk-input-test`,
  `ttk-font-test`, Python 139 OK (2 skipped; font-set count and UI art
  inventory updated), `level_overlay_guards.py --check`, `check_repo.py`.
- Executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8`.

## Not verified

- A real key door or card reader use; only the crystal holder was used live.
- Memory-card save and reload after a use. By design the marker rides along in
  the player struct, but this was not observed.
- An item used before this build reads NOT FOUND YET: its use left no marker.
