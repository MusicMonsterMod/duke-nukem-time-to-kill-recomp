# D08Q5 - weapon switching while flying the jetpack

2026-10-06. **Done, user-accepted:** "excellent!! i accept."

## Research

The original allows weapon changes in flight. With a private research write of
the normal request (`+0x3ba` = weapon, `+0x224 |= 4`) during mode 10 / anim
164, the original plays the ordinary upper-body sequence (holster 6, empty 1,
draw 21, idle 20 for the shotgun) while the flight pose, handler, altitude
and fuel continue. A request made during an attack (upper 26) is held by the
original until the attack ends. Holstering to the boot (weapon 0) gives the
unarmed flight pose (upper 164), from which a draw works again. The next shot
uses the D08Q3 flight aim and crosshair; no aim or reticle change was needed.

Our shortcut layer was the only block: `select_weapon` (`shortcuts.inc`)
returned after the J check whenever `jetpack_input_ready()` was true.

## Change

`select_weapon` still handles J / item-use(jetpack) first in flight. Then flight
joins swimming as a weapons-only lease: weapon groups, previous/next (wheel,
semicolon/apostrophe) and X pass through the existing D08A selectors and the
same ground request. Other gadgets, the quick kick, held fire/original
aim/interact, pending transitions (`+0x224 & 0x2c5`), non-idle upper-body
poses and Vanilla stay refused. Flight takes the ground weapon set (the D08O1
underwater filter applies only in water). No new hook, address, guest write
or framework change; the codegen hash is unaffected.

## Verification

Private lab `recomp/analysis/d08q5-20261006/` (port 9351, Xvfb :96, private
profiles and copies of the D08O1 cards, slot 8 = level 6 with `dnstuff`).

- Native: `ttk-controls-test` (new D08Q5 group: flight groups, wheel,
  refusals for medkit, kick, fire held, draw pose, Vanilla, ground handoff;
  36 groups PASS), `ttk-aim-test`, `ttk-near-test`, `ttk-input-test`,
  `ttk-inventory-test` PASS. Incremental local-dev player build OK.
- Modern flight (`r2.py`): 1 (boot, knife, axe), 3, 4, 5, 6 (pipe bomb,
  dynamite, Holy Hand Grenade) each drew in mode 10 and fired (adapted shots
  rose, reticle shown while armed). Later keys ran after the dynamite blast
  had landed Duke (see limits); a separate run covered the rest in the air.
- Classic flight (Space held, `PROFILES=profiles-classic.json`): every key
  1-0, X, semicolon, apostrophe and wheel up/down switched in mode 10 and the
  new weapon fired (20 adapted shots); landing and fall unchanged.
- Throws in flight: knife throw consumed one knife and the next switch worked;
  Holy Hand Grenade thrown in flight, flight continued.
- Swimming (`swim.py`, slot 11): shotgun and Gatling switch; knife, flame
  and RPG still skipped by the D08O1 rule; apostrophe and the wheel unchanged.
- Vanilla flight (`vanilla.py`): number keys, apostrophe, X and the wheel do
  not change the weapon.
- The player's `saves/local-play` files were not modified.

## Limits

- Dynamite thrown in flight explodes close below Duke and knocks him out of
  flight (anim 157, mode 9, then a landing). The same happens with dynamite
  selected on the ground before takeoff, so this is the original blast
  reaction, not caused by switching.
- Drawn dynamite runs its original fuse; a switch away waits for it, as on the
  ground (D08A).
- Like the ground, presses during an attack, throw or draw are refused rather
  than queued.
- The D08S EDuke32 scheme does not exist yet; it uses the same
  `jetpack_input_ready()` lease, so it should inherit this path.
- Automated Xvfb checks only; the user's in-game feel has not been confirmed.
