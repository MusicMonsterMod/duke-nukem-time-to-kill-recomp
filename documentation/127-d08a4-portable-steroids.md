# D08A4 - Portable steroids (EDuke32 style)

Status: **Done** (user-accepted 2026-10-07: "mechanically, the steroids work
perfectly"). Follow-up D08A8: show the running countdown in the steroids box
instead of the original armor element. Modernized
only, optional (`steroids` = `portable`, the default, or `original`); Vanilla
unchanged.

User request (2026-10-06): "Eduke style steroids, where you actually pick up the
roids as an item, and it appears in our items list. it is invoked with the R key."

## Behaviour

- A steroids pickup shows the original STEROIDS message and sound, and is kept
  instead of starting the effect.
- Held steroids appear in the `[ / ]` switcher right after the medkit (EDuke32's
  order) and in the D08A6 HUD item box, with the user's own pill-bottle icon
  at 100%.
- **R**, or **Enter / U** with steroids selected, takes them: the original
  effect starts (full time), the pickup sound plays and `USED STEROIDS` shows
  (Duke 3D quote 12). While they run, the switcher shows them draining with the
  active mark; the original status bar shows the remaining percent in its armor
  element (with the armor icon), as it always has for running steroids.
- One at a time, as in Duke 3D (`GAME.CON`: `ifpinventory GET_STEROIDS`): while
  one is held, more steroids stay on the ground. A pickup while steroids run is
  left to the original, which refreshes them.
- R with none held, while they run, or while Duke is dead does nothing.
- `dnhyper` still starts the effect directly (EDuke32's `dnhyper` sets 399, an
  active dose); the original inventory grant (`dnstuff`, `dninventory`,
  `dnitems`) gives a held one, like EDuke32's inventory cheat.

## The original's own portable steroids

The original still has a held steroids item; only its pickup skips it.

| Piece | Address | Meaning |
| --- | --- | --- |
| Item 4 | player `+0x364` flags, `+0x366` amount/timer, full amount `0x800c2722` (9000) | Typed 3 (timed toggle) in the item table `0x800c2710`, like the Bio Mask and goggles |
| Pickup | dispatcher `0x80081a48` (only caller `0x8007fe64`), case `0x800827e8` for type 638 | Full amount, `+0x364 |= 2` (starts the effect), message 110 into `0x800dcd90[player 0x233]`; shared tail `0x800828d0` plays sound `0x100f` at Duke through `0x8006b73c` (ra `0x800828d8`) and returns 1 (caller removes the object) |
| Drain | `0x800414a0` | While bit 1: amount -= frame step `0x800d21fc`; at 0 clears bits 0-1 |
| Damage | `0x800a4154` | Takes 1500 from a running amount; can end it (bits 0-1 cleared) |
| Readers of bit 1 | kick `0x80049058` (x4 damage), status bar `0x8008bd94`, `0x800881e4` | The effect |
| Inventory grant | `0x8003d738` | Items 0-5: `|= 1`, amount = capacity. For steroids: owned, full, not running |
| Original Select menu | list `0x80088134`, use `0x80089494` | Uses a timed toggle with `flags ^= 2`; the use routine `0x800402d0` only clears steroids' pending bit. The list has no name for item 4 (`0x80087da8`), so held steroids are not listed |
| Level-end snapshot | `0x80083348` (from `0x80083404`), carry record `0x800dd030` | Items 0-6 as bit 0 plus the amount; the memory card save writes this record (items at save offset `0x158`) |
| Card load restore | `0x800831cc` (from `0x800258f0`, pending load `0x800be560`) | Item words copied back verbatim |
| Level-start reset | `0x8003fd98` loop `0x8003fe10` | Zeroes items 0-5 at level 0, while `0x800bdea8` is set, in two-player games, and when end code `0x800be55e` >= 5 outside levels 21-26 (the pause menu Restart, 0xFD) |

So a held steroids item is bit 0 with an amount, bit 1 off. The first design kept
a dose count in spare flag bits 8-11 (no original writer touches them), but the
level-end snapshot keeps only bit 0 of each item, so a card save dropped it
(verified: the card held `0000` for item 4 while RAM held the dose). The
original representation goes through every original path like the jetpack.

## Implementation

`recomp/src/ttk/steroids.inc` (in `modern_controls.cpp`):

- At the dispatcher entry (existing hook), a steroids touch by Duke from
  `0x8007fe6c` is noted when nothing is held or running. Held: the dispatcher
  gets a blank object (type 0xffff), so the pickup stays (the D08H concealed
  apartment pickup technique). Running: left to the original.
- New entry hook `0x8006b73c` (`game.local.toml`, regenerated: one generated
  line in `SLUS_005.83_full_35.c`; the codegen hash is unchanged, so existing
  savestates load). Only the pickup tail's call (ra `0x800828d8`, s1 Duke) goes
  further: bit 1 off, bit 0 on, full amount. This is still inside the pickup, so
  the effect never runs for a frame. A missed call would settle at the next
  item poll (`late_settles`, never seen).
- R and Enter/U on steroids (`shortcuts.inc`) are taken in every live gameplay
  state, like `[ / ]`: they set bit 1 (the original menu's toggle) and play the
  pickup sound through `0x8006b73c(0x100f, Duke+4, 0x800, -10)` on a private
  stack.
- Switcher (`inventory_hud.cpp`): order 5, 4, 1, 2, 3; item 4 reads the raw
  flags and amount. HUD box (`gadget_hud.inc`): the D08A7 own-art path is now a
  table; steroids use a 4bpp cell at VRAM (964,205) and palette (1008,207) in the
  band D08A7 surveyed as empty, uploaded with the box through the HUD
  ordering table.
- Code guards: drain `0x800414a0` (96 bytes) and damage cut `0x800a4154` (68);
  the dispatcher (incl. the tail) and the pickup case were already guarded.
- Profile schema 29: `steroids` (`portable` | `original`), `run.py --steroids`,
  `--settings` choice **S**, `DNTTK_STEROIDS` (Vanilla always `original`).
- Art: `recomp/assets/ui/items/hud-steroids.png` (user's art, D24A) in the
  switcher pack (`build_ttk_inv_icons.py --steroids`); its 15-colour reduction
  `hud-steroids-15col.png` from the new `tools/local/reduce_icon_15col.py`
  (k-means in PSX 15-bit colour, no dither, deterministic) for the HUD box.
- Debug: `ttk_input` -> `controls.steroids` (`portable`, `doses`, `flags`,
  `timer`, `stores`, `full_leaves`, `refreshes`, `uses`, `refusals`,
  `late_settles`).

## Evidence (executable `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5`)

Private Xvfb runs, private profile and a private copy of the user's cards and
savestates (`recomp/analysis/d08a4-steroids/`, local; the player's card was not
written). UI slot 4 (level 0) unless noted; pickups are spawned type 638
(`spawn steroids`) going through the original pickup code.

| Check | Result |
| --- | --- |
| Pickup | STEROIDS message; flags 0 -> 1, amount 9000, effect not running |
| Second pickup while held | stays on the ground (21-30 touch frames refused) |
| `[ / ]` | steroids after the medkit, icon at 100%; HUD box shows the icon and a lit 100 |
| R | flags 3, amount draining; `USED STEROIDS`; switcher shows 95% with the active mark; original armor element shows 95 |
| R running / none / Enter on steroids | nothing / nothing / same as R |
| Pickup while running | refreshed to 9000 (original) |
| Savestate save/reload | held item back after it had been used |
| Level completion (end code 4, stats, save offered) | carried into level 1 (`0x0001`, 9000) |
| Card save | item 4 `0100 2823` in the save record, same as the jetpack |
| Card load (pause, Load) after using it in level 1 | restored to held (`0x0001`, 9000) |
| Death, Continue | still held |
| Level 6 (via `level 6`) | pickup held, R runs it |
| `level N` travel | every gadget cleared by the original Restart reset (`0x8003fe10`, write trace), steroids included |
| `original` | pickup starts the effect (flags 2), R does nothing |
| Vanilla | `portable` false, nothing held or used |
| Original Select inventory | lists only the jetpack with steroids held |
| Suites | `ttk-controls-test` (fixture, LEVEL01, all levels; two new D08A4 groups), `ttk-inventory-test`, `ttk-input-test`, Python 131 (2 skipped), `level_overlay_guards.py --check`, `check_repo.py` |

## Limits

- Natural level placements were not surveyed
  (spawned pickups use the same type and code); enemy drops of steroids were not
  seen.
- Death while steroids run, the challenge and boss levels and two-player games
  were not exercised (only player one's pickups are taken; R is player one's).
- The original Select inventory cannot show or use held steroids (it has no name
  for them).
- Switching a profile to `original` with steroids held leaves them held; they can
  be used from Modernized later, and Vanilla ignores them.
