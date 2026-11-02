# D08A33 - EQUIP PICKUPS: IF NEW / NEVER

Status: **Done** (user-accepted 2026-11-02: "i accept the pickups job!! lock it in,
close us out"). Modernized only; Vanilla unchanged.

User request (2026-11-02, while playtesting D17AY): an EDuke32-style "equip
pickups" option. IF NEW (default): a weapon Duke does not own is drawn at
once; a gadget he picks up becomes the selected item, only selected, not used.
NEVER: nothing is equipped or selected. User: "having tested it out, this
functionality appears to exist perfectly for the items, just not the weapons."
Ammo never switches, as in EDuke32.

## Behaviour

- **IF NEW**: picking up an unowned weapon draws it through the original
  weapon request, so the stow/draw animation, ammo and HUD are native. The
  D08A9 gadget selection ([note 128](128-d08a9-pickup-selection.md)) is
  unchanged: new gadgets and top-ups become the `[ / ]` selection.
- A weapon Duke already owns (its pickup only adds ammo) and ammo pickups never
  switch. The `dnstuff` grant does not go through the pickup dispatcher and
  does not switch (checked live), as in EDuke32.
- If Duke is not ready for a weapon change at the pickup (fire held, climbing,
  pushing, a throw, an attack or draw animation, any state where the number
  keys are refused), the draw waits for the same gate the number keys use, for
  up to 60 player updates (about two seconds), then is dropped. A weapon key
  pressed meanwhile (taken or refused), a savestate load, death or the weapon
  already in hand drop it. In water only weapons the swim states can use are
  drawn (D08O1 rule); in jetpack flight the D08Q5 request applies.
- **NEVER**: no weapon draw and no D08A9 selection. The D08A6 fallback still
  selects a gadget when the current selection is unusable (for example the
  first gadget picked up with an empty inventory); that is not a pickup rule.
- The option is live: changing it in OPTIONS applies at once.

## The original weapon pickup

The pickup dispatcher `0x80081a48` (only caller `0x8007fe64`) gives weapons
through the helper `0x800818ec` (a0 Duke, a1 slot, a2 message):

- If `+0x2c4+4*slot` bit 0 is clear, it sets it and adds the table amount
  (`0x800c4590+44*slot`, +0x26) capped at the maximum (+4).
- If the weapon is owned and below its maximum, it only adds ammo; at maximum
  it refuses (message 0x107) and the pickup stays.
- Only in multiplayer (`0x800c27bc` >= 2) does it request the new weapon
  itself (`+0x3ba = slot`, flag 4) when the slot is above the current one, the
  weapon table entry lacks flag 8 and the lower animation lacks flag 0x20.

Ammo pickups go through `0x80081720`, which never touches bit 0.

## Weapon and ammo pickup types

From the dispatcher's type cases; checked live in level 0 (spawned on a fresh
slot-12 load and walked over) for every one that level loads: each weapon set
its slot's bit 0 with its ammo and, with IF NEW, was drawn; each ammo pickup
only added ammo (also to an unowned weapon) and never switched. Slots 12-14
are given by setting bit 0 with the ammo helper, not `0x800818ec`; the
ownership comparison covers both.

| Weapon (slot) | Weapon type | Ammo type |
| --- | --- | --- |
| Throwing knife (1) | 45 | - |
| Throwing axe (2) | 1 | - |
| Crossbow (3) | 21 | 609 (not loaded in level 0) |
| Desert Eagle (4) | 59 | 33 |
| Combat shotgun (5) | 66 | 65 |
| Buffalo rifle (6) | 10 | 124 (not loaded in level 0) |
| Gatling gun (7) | 41 | 562 |
| RPG (8) | 47 | 106 |
| Flamethrower (9) | 37 | 38 |
| Energy weapon (10) | 34 | 125 (not loaded in level 0) |
| Freezer (11) | 641 | 642 |
| Pipe bomb (12) | 58 | - |
| Holy hand grenade (13) | 608 (not loaded in level 0) | - |
| Dynamite (14) | 126 (not loaded in level 0) | - |

With an upgrade (`+0x85f` flags), the Gatling gun, RPG and flamethrower
pickups give the upgraded slot (28, 29, 27) and the shotgun and energy weapon
pickups mark the upgrade on their slot. The
console `spawn` (or `items`) list now names these in "Weapons here" and "Ammo
here" sections (`recomp/src/ttk/spawn.inc`, group field); `spawn <name>` works
for them too. The Desert Eagle was already owned in slot 12, so its pickup
added nothing there.

## Implementation

- `recomp/src/ttk/pickup_select.inc`: the existing dispatcher entry snapshot
  (D08A9) also saves weapons 0-14 ownership; at the existing return point
  (`0x8001ca4c`, ra `0x8007fe78`) a weapon 1-14 whose bit 0 went 0 -> 1 is
  queued. `equip_pickups()` reads `DNTTK_EQUIP_PICKUPS` through
  `ttk_live_env`; `never` stops both the weapon draw and the D08A9 selection.
- `recomp/src/ttk/shortcuts.inc` (`select_weapon`, every player update): the
  queued slot is requested with `+0x3ba` and flag 4, under the number keys'
  gate (movement-ready, jetpack or swim weapon state; equipment 0/2/remote;
  weapon idle loop; no fire, precision aim or interact held; no `0x2cd` flags).
- OPTIONS > ADVANCED > ITEMS > EQUIP PICKUPS (IF NEW / NEVER), live.
- Profile schema 61: `equip_pickups` (`if-new` default, `never`); migration
  adds `if-new`; `run.py --equip-pickups`; terminal `--settings` choice E;
  Vanilla always passes `never`.
- Debug: `ttk_input` -> `controls.pickup_select` adds `weapon_draws`,
  `weapon_drops`, `weapon_waiting`.
- No new hooks, no codegen change, no framework change; savestates load.

## Evidence (executable `2fd0b986485196af4eb17d7f6cd86315579e738f0db1c337198e0a0be426831b`)

Native: `ttk-controls-test` (owned EXE, LEVEL00 fixture, LEVEL01, all levels):
122 groups pass, new D08A33 group (new weapon drawn once; owned weapon ammo,
NEVER, Vanilla not drawn; fire held waits and draws after release; an attack
longer than the window drops it; a weapon key drops it; NEVER leaves a new
gadget unselected) and the D08A9 group unchanged. `ttk-input-test`,
`ttk-aim-test`, `ttk-near-test` pass. Python profile/input tests pass (new
EquipPickupsOptionTest: menu choice, invalid value, migration 60 -> 61, CLI,
in-game menu side file, Vanilla).

Live: private card copies (`recomp/analysis/d08a33-equip-pickups/cards`, from
`saves/local-play`), Xvfb, dummy audio, debug-port driver `probe.py`; player
save/card/profile hashes unchanged after every run. UI slot 12 (LEVEL00
alley, Gatling gun in front of Duke), 60 Hz, first person:

| Case | Result |
| --- | --- |
| IF NEW, walk over the Gatling gun | owned 0,4 -> 0,4,7; request 7 at the pickup; shotgun stowed, Gatling drawn (upper 29), HUD 150 |
| IF NEW, walking with fire held | Gatling owned, shotgun kept while held; drawn after release |
| `dnstuff`, then Gatling ammo 10 and walk over it | ammo 10 -> 160, weapon unchanged; the cheat did not switch either |
| NEVER, walk over the Gatling gun | owned, shotgun kept |
| IF NEW, medkit selected, spawned steroids | steroids selected (held, not used: flags 1) |
| NEVER, same | medkit stays selected; steroids held |
| OPTIONS > ADVANCED, real keys | EQUIP PICKUPS row under ITEMS, Right -> NEVER, Confirm: side file and profile `never` |

## Reported, not reproduced: holster while in the console

User (2026-11-02): "duke will holster his weapon when you are in the
terminal." Not reproduced offscreen on a private copy of slot 12 with a
private copy of the player's settings: opening the console, typing (letters,
digits, Backspace), Enter on `spawn`, `help` and `spawn 41`, left/middle/right
clicks and holds, and about 25 seconds open left equipment 2, the drawn weapon
and the ready loop unchanged, and the first-person weapon visible. The console
consumes key events (`debug_console_event`) and capture is withheld while it
is open. Waiting on the exact steps from the user; no fix made.

## Limits

- Weapons and ammo not loaded in level 0 (holy hand grenade, dynamite, and
  crossbow, Buffalo rifle and energy weapon ammo) were not picked up live. Climbing, pushing, swimming and jetpack
  pickups were not exercised live (the gate is the number keys').
- Vanilla is covered by construction (`input_modernized()`, launcher
  `never`) and the native test; the scripted input driver does not run in
  Vanilla.
- The "about two seconds" window counts player updates; it is not a host
  timer.
