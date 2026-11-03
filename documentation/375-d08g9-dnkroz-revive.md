# D08G9 - dnkroz after death: Duke gets back up where he fell

2026-11-03. Status: Done (user-accepted 2026-11-03, with D42).

## Request

Typing `dnkroz` (or `dncornholio`) while dead brings Duke straight back with
full health and god mode on, where he died, as in Duke 3D (D08G9; user,
2026-11-03: "typing dnkroz at the death scene doesnt resurect duke there on
the spot").

## Change (Modernized, one player)

- `cheats.inc` `death_cheat_take()`: on the D42 death scene Duke's update (and
  so `cheats_update`) does not run; the D42 death hook takes typed cheats.
  dnkroz / dncornholio turn god mode on (`0x800c3cc6`) and show GOD MODE: ON;
  any other cheat shows "Cheat unavailable - Duke is dead".
- `game_menu.cpp` `death_update_entry`: the revive is the game's own CONTINUE
  respawn (`0x80042de8`, waiting state `+0x15` = `0x19`, request `+0x35` = 2),
  which resets Duke (health, state, camera) and puts him in his room from a
  position. That position comes from the checkpoint's placement row (table
  `*0x800de720`, 48-byte rows, index from `0x800d82f8` by `+0x27a`; words
  `+8..+0x14` to the player's `+4..+0x10`). For that one tick the row holds
  Duke's own position, so he stands up where he fell; the row is restored
  as soon as he is alive (`game_menu_frame`; dropped on a load). The
  respawn's CONTINUE invincibility (`+0x8a0` = 1500) is cleared, god mode
  covers it. The continues count is kept above zero as in D42.
- First person (`modern_controls.cpp`): after a revive the death view's roll
  unwinds, half per tic, as Duke 3D's does once the player is alive
  (`rotscrnang -= rotscrnang >> 1`); CONTINUE and restarts reset it at once.

## Evidence

Private fixture (slot 10), offscreen. Death by `hurt=1,30000`, then
`cheat=dnkroz` on the scene: health 0 -> 10000, god on, state 2, x/z
unchanged (-18962, 47019), standing height again, `+0x8a0` 0, GOD MODE: ON
shown; Duke walks with W afterwards. First person: the rolled floor view
unwinds over about 0.1-0.4 s to the upright eye. Third person: Duke stands
up in place. `ttk-controls-test`, `ttk-input-test`, `ttk-flash-test`,
`ttk-inventory-test` pass.

## Limits

- Duke comes back with the respawn's own weapon state (the weapon was not
  in hand after the revive in the test); facing is the checkpoint's.
- Gibbed deaths (blast, state 4) and water deaths not run with dnkroz yet.
- Not yet the user's playtest. dnkroz while alive is unchanged (code path
  untouched).
