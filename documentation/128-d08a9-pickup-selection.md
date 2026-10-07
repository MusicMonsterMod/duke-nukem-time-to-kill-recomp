# D08A9 - Picked-up gadget becomes the switcher selection

Status: **Done** (user-accepted 2026-10-08: "fully accepted, working
beautifully"). Modernized only; Vanilla
unchanged.

User request (2026-10-07): "when an inventory item is picked up, that item
should be the one selected in the switcher. i.e. you pick up biomask, then that
should be the selected item. you pick up jetpack, that should be the selected
item etc. that's how it worked in duke3d and i want that feel to exist here."

## Behaviour

- Picking up a jetpack, Bio Mask, goggles, medkit or (portable) steroids makes
  it the `[ / ]` selection at once: the D08A6 HUD box shows it and Enter / U
  use it.
- Topping up a gadget that was not full counts too (EDuke32's `P_AddInventory`
  sets `inven_icon` on every inventory pickup, refills included). Steroids
  picked up while they run are refreshed by the original and selected.
- A full gadget stays on the ground (the original rule), so the selection does
  not change. Keys, mission items, weapons, ammo, health and armour leave it
  alone.
- The original inventory grant (`dninventory`, `dnstuff`, `dnitems`) keeps the
  current selection.
- The strip does not pop up; the pickup message and the HUD box are the
  feedback, as in Duke 3D.

## The original pickup

Dispatcher `0x80081a48` (only caller `0x8007fe64`, ra `0x8007fe6c`), a0 Duke,
a1 the touched object:

| Item | Case | Rule |
| --- | --- | --- |
| Jetpack (1) | `0x80082580` | Skipped when `+0x35a` == capacity (`0x800c2716`); else bit 0, full amount, message 0x6a |
| Bio Mask (2) | `0x80082624` | Same with `+0x35e`, capacity `0x800c271a`, message 0x6b |
| Goggles (3) | `0x800826c4` | Same with `+0x362`, capacity `0x800c271e`, message 0x6d |
| Medkit (5) | `0x80082768` | Same with `+0x36a`, capacity `0x800c2726`, message 0xae, then a random voice |
| Steroids (4) | `0x800827e8` | Full amount and bit 1; D08A4 holds it at the sound call `0x8006b73c` (ra `0x800828d8`) |

With two or more players (`0x800c27bc` >= 2) the jetpack, Bio Mask and goggles
cases also start their activation (`flags |= 0x8000` (+2 for the jetpack) and
`0x800402d0`). After the dispatcher returns, its caller calls `0x8001ca4c`
(ra `0x8007fe78`) before anything else.

## Implementation

`recomp/src/ttk/pickup_select.inc` (in `modern_controls.cpp`):

- At the dispatcher entry (existing hook; Modernized, player one, caller
  `0x8007fe6c`) the flags and amounts of items 1-5 are saved, with the
  savestate load counter.
- At `0x8001ca4c` with ra `0x8007fe78` (existing hook) they are compared: the
  first gadget that became owned or gained amount is the pickup. A savestate
  load in between cancels it.
- It is selected through `remember_item` (host `selected_item` and the
  original menu ID `0x800c3f94`), so the HUD box and the D17 replay workers,
  which read guest memory, follow. Only a usable item is selected (so
  `steroids` `original`, which starts the effect, does not select).
- While any gadget is mid-activation (`+0x8000`), the selection waits for the
  next item poll in `select_weapon` that finds none (the D08A6 rule that never
  moves the menu ID off an activating item).
- Debug: `ttk_input` -> `controls.pickup_select` (`selects`, `refills`,
  `deferred`, `waiting`).
- No new hooks, no generated code change, no profile change; savestates load.

## Evidence (executable `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`)

Private Xvfb runs with a private copy of the test cards
(`recomp/analysis/d08a9-pickup-select/`, local: `t1.py` to `t6.py`,
screenshots). Pickups are spawned (`spawn <item>`), which go through the
original pickup code.

| Check | Result |
| --- | --- |
| Level 0, 60 fps: new Bio Mask, goggles, medkit, steroids | each selected (2, 3, 5, 4), HUD box shows it |
| Jetpack already full | stays on the ground, selection unchanged |
| Key 1 / key 2 | selection unchanged |
| `dninventory` | selection unchanged |
| Refill Bio Mask, medkit, goggles, jetpack (amount 100) | selected (`refills` counted) |
| Full goggles / medkit | stay on the ground, selection unchanged |
| Enter after goggles / jetpack pickup | switches it on |
| Steroids picked up while running (jetpack selected) | refreshed and selected |
| Level 6, 120 fps: medkit, steroids, Bio Mask | selected; the steroids box is in every presented image (replay workers) |
| Pickup while the Bio Mask is mid-activation (test-forced `0x8001`) | waits (`waiting` 3), selected when the bit clears |
| Savestate save, `]`, reload | selection back to the saved one |
| Vanilla | the debug `spawn` is Modernized-only; the code path is behind the Modernized gate and its counters stayed 0 |
| Suites | `ttk-controls-test` (new D08A9 group: grant, refill, key, full, other return, other caller, activation wait, Vanilla), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK, `level_overlay_guards.py --check`, `check_repo.py` |

## Limits

- Natural level placements were not walked to; spawned pickups use the same
  types and original code.
- Death and Continue were not reached in the scripted runs (writing health 0
  left Duke standing). Continue keeps the D08A1 selection rules; a pending
  pickup selection is dropped when Duke is not alive.
- A real pickup during a jetpack transition was simulated with the pending
  bit, not timed in play.
- Two-player games are not handled (only player one's pickups, as D08A4).
