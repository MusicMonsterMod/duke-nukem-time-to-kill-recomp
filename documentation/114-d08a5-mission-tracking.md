# D08A5 - Mission item tracking (mission inventory)

Status: **Accepted** (2026-10-07, "i fully accept!"). Modernized only; Vanilla and the
original Select inventory are unchanged. Built to the approved design E (board
entry D08A5; mockup `drawE()` in `recomp/analysis/d24a-fonts/picker.template.html`,
local).

## Player use

Current design (user revision after the first playtest, 2026-10-07; see
"Redesign" below):

- `,` / `.` (Comma / Period) open the **mission inventory** in a level with
  mission items. It takes the gadget switcher's place: one framed slot per
  item, missing items as dim grey silhouettes, "MISSION" and found/total at the
  bottom (green when all are found). The selected slot's frame is steel blue,
  and an **item card** at the top shows the icon, name (gold when found, blue
  when missing), type, and FOUND / NOT FOUND YET.
- The first press opens on the item last shown (the first item after level
  travel); `.` then steps to the next and `,` to the previous (wrapping). It
  closes 2.5 s after the last press.
- Enter or U while it is open only close it: no gadget is used. `[` / `]` close
  it and open the gadget switcher; `,` / `.` close the gadget switcher.
- `[` / `]` show the gadgets only, as before D08A5, with the gadget selection
  frame in the old tile0020 orange.
- Original strafe (PlayStation L1/R1), which held Comma/Period, is unbound by
  default; A/D strafe in every Modernized state.

## Where the data comes from

The original Select inventory builds its list in `0x800884bc`: for inventory
items 1..16 it asks the name function `0x80087d4c` for the current level
(`0x800be570`) and lists the item when the name is not the "none" string and
the flag halfword `player+0x354+4*i` has bit 0. Running that function for
levels 0-31 gives the mission items (items 6..16) the game itself names:

| Level | Items (inventory index: name) |
| --- | --- |
| 0 | 6 Subway Security Key, 7 Transport Room ID, 11/12/13 Red/Blue/Green Energy Crystal |
| 1 | 6 Skeleton Key, 14 Scrap of Paper, 15 Old Note, 16 Torn Paper |
| 2 | 6, 7 Skeleton Key |
| 3 | 6 Skeleton Key |
| 5 | 6 Warehouse Key, 11/12/13 crystals |
| 6 | 7, 8 Skeleton Key, 14/15/16 Family Jewel |
| 7 | 6 Gantry Key, 7 Valve Key |
| 9 | 6 Lab Key, 7 Valve Room Key, 11/12/13 crystals |
| 10 | 6, 7 Skeleton Key |
| 11 | 6 Skeleton Key |

All other levels (8, 12, Challenge Stages, bosses) have none. This corrects two
guesses in the ticket: the crystals are items 11-13 (`+896/900/904`), and level
6's skeleton keys are items 7-8 (`+880/884`). The table is compiled into
`inventory_hud.cpp`; `tests/local/test_mission_items.py` re-derives it from the
prepared executable by running `0x80087d4c` and fails if they differ.

Pickup dispatcher `0x80081a48` cases seen while checking: 153 sets item 6, 155
item 8, 157 item 10; skeleton keys 542/744/862 item 6, 745/863 item 7, 746
item 8; papers 547/548/549 items 14/15/16; jewels 854/856 items 14/16; 761 sets
item 14 with message 115. So the debug `spawn green energy crystal` (761) does
not set the crystal slot 13, which is why D26E saw it missing from the Select
inventory; real crystal pickups were not traced.

## Implementation

- `src/ttk/inventory_hud.cpp`: mission table and state, the mission inventory
  (`mission_visible`, Comma/Period presses, 2.5 s expiry) drawn in the
  switcher surface's place (72 lines: the panel and 2 clear lines under it),
  and the 592x52 item card (`ttk_mission_card_image`). Gadget switcher and
  mission inventory close each other. Non-premultiplied "over" compositing
  keeps the translucent panel translucent under icons and frames.
- `src/ttk/shortcuts.inc`: `inventory_feedback` also passes the level and the
  flags of items 6..16 every gameplay update, so found state follows pickups,
  savestate loads and level travel without any hook of its own. An Enter / U
  command while the mission inventory is open closes it and is not used
  (including the jetpack-in-flight path).
- `src/ttk/input_bindings.def`: `original_strafe_left/right` default to
  Unbound; new `mission_previous` (Comma, 54) and `mission_next` (Period, 55)
  appended. `pc_input.cpp` sends them to the host (`mission_browse_press`); they
  are never guest commands. `tools/local/pc_input.py`: `add_missing_actions`
  drops schema 27's `mission_browse` and moves Comma/Period from original
  strafe to the new actions when strafe still holds them (a customized key
  stays and the new action is unbound). Player profiles are schema 28; a
  scratch copy of the player's schema 27 profile migrated with Comma/Period on
  the mission actions and strafe unbound. Older launchers refuse schema 28.
- `tools/local/build_ttk_mission_items.py` (every build, no disc): pack
  `ttk-mission-items.pack` (TTKMIS1) with each item icon, its silhouette
  (greyscale, brightness 0.45, contrast 0.8, alpha 0.85) and the project
  frame in grey, tile0020 orange and Console steel (brightness-normalised
  palette swap, as the mockup).
- `tools/local/build_ttk_fonts.py`: sets 8-11: FOUND (system 8x8 `#5fd35f`) and
  the 2x Microfont "MISSION" (`#848484`), count (`#989c58`) and complete count
  (`#5fd35f`). `ttk_font.cpp` takes up to 12 sets and gained
  `ttk_font_text_width`.
- Framework (exported to the accepted patch): `host_osd_card_image` in
  `host_osd.h/.c`, drawn top centre at y 14 (480-line units) by the GL
  presenter and both SDL presenters, after the switcher. `host_osd.h` is not
  in the codegen hash; the baked hash is unchanged, so savestates still load.

## Evidence (first design: row under the gadgets, `\` browsing)

- Native: `ttk-inventory-test` (new mission checks: row height, translucent
  panel, card only while browsing and only at full width, wrap, level without
  items, all-found, row alone without gadgets, 2.5 s timeout), `ttk-input-test`
  (`\` reaches the host and queues nothing), `ttk-font-test`, Python suite
  (120 tests: new `test_mission_items.py` including the original-table check,
  and the schema 27 binding migration).
- `ttk-controls-test` fails at its first `movement_ready()` assertion with the
  available overlay fixtures, identically with this job's changes stashed:
  a stale fixture, not this job.
- Private Xvfb Modernized run on private card copies
  (`recomp/analysis/d08a5-mission/mission_test.py`, local):
  - level 6: `]` shows the gadget strip (orange frame) and the mission row 0/5;
    `\` shows the card "SKELETON KEY / KEY / NOT FOUND YET" and the steel
    frame moves with each press; after 3 s everything is closed; `\` with the
    switcher closed opens it with the card;
  - a real pickup (`spawn family jewel`, walked over) set item 14 and the row
    showed the jewel in colour and 1/5;
  - level 0 shows 0/5 (keycards and crystals); level 8 shows the gadget strip
    only and `\` shows no card.
- Vanilla private run: `]` and `\` show nothing.
- The accepted patch applies cleanly to the pinned framework; `check_repo.py` OK.
- Build SHA-256 `d067c37f80f4ab0b2615bfbacb85a90fe7bcbe84c1d5493db181126611d41f63` (after the redesign).

## Playtest fixes (first design, superseded where noted)

- **Fixes (2026-10-07, user report; the backslash handling was removed again
  by the redesign).** "\\ did not seem to allow me to
  cycle": the user's keyboard is UK (`gb`, pc105). There the key that types `\`
  is left of Z (SDL `NONUSBACKSLASH`, 100); scancode 49 types `#`. The default
  binding now also answers that key and any key whose layout keycode is `\`.
  Verified on a private Xvfb run with the `gb` layout set before launch,
  sending the raw key (keycode 94): `]` opened the row and `\` stepped the steel
  frame 1 -> 2 -> 3. "Bottom border ... looks thinner than the top": the 25x23
  frame was stretched to 42x39 by plain nearest-neighbour, giving the mission
  slot frames a 2 px top and 1 px bottom (right side thinner too), and the
  orange gadget frame 3 px / 2 px. Both stretchers now sample pixel centres
  (2/2 and 3/3). The mission panel's outline was also the surface's last line;
  the surface now has 2 transparent lines under it (132 lines).

## Redesign (2026-10-07, user)

"it would be nicer if we separated out the inventory with the mission items.
simply put, revert the inventory back to [/] and thats it, just inventory.
then the mission inventory replaces whatever ,/. are bound to (visually looks
like </> so thats cool). The reason for this is because i wanted to
instinctively press enter when cycling through the mission inventory, and when
i did, obviously it just puts the jetpack on. ... then you can remove the usage
of \ and #."

Done as described above; the `\` action, its UK `NONUSBACKSLASH` alias and the
schema 27 binding are gone. Evidence: `ttk-inventory-test` (gadgets only on
`[`/`]`, Comma/Period open the mission inventory and close the gadgets,
reopening on the last item, wrap backwards, close, no-items level, timeout),
`ttk-input-test` (Comma -1, Period +1, never queued; Backslash does nothing),
Python 120 OK. Private Xvfb run in level 6: `]` gadgets only; `.` mission
inventory and card; `.` steps; `,` `,` wraps to the last jewel; Enter closed it
with the jetpack flag unchanged (1), the next Enter switched the jetpack on
(3); `.` then `[` shows the gadgets.

## Limits

- **Controller is not implemented.** Modernized controls are keyboard and
  mouse; a physical controller drives the original PlayStation pad and has no
  input for the switcher or the mission inventory yet.
- The card is centred; in 4:3 that is the mockup's x 24, in widescreen it stays
  centred.
- Found state is bit 0 of the original flag only. Real pickups of every
  mission item were not walked in every level; only the spawned family jewel
  was picked up live. A debug-spawned skeleton key in level 6 set item 6,
  which the game's own Select list does not name there, so it did not count.
