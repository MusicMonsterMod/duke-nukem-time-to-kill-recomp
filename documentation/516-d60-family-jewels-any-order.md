# D60 - FAMILY JEWELS: the wheel takes the jewels in any order

Status: **Done**, user-accepted 2026-11-20 ("lock it in and close us out")
after a playtest that found the `dnstuff` case (fixed, below). Modernized
only; Vanilla keeps the original order rule.

## Request

User, 2026-11-19, field laptop (FAMILY JEWELS, level 6, UI slot 12): "i cant
put the third jewel in the device. that being said, i would prefer for this
device to just simply accept the gems in any order." Later the same evening on
ripspace (UI slot 12): "unfortunately i cannot put the green jewel in".

## Cause (established)

The order rule is the game's own, not a regression of ours.

The wheel is object type 857 (level object slot 22), kind 7, handler
`0x800cc338` in the LEVEL06 overlay (read from RAM, capstone). Its jewel case
keys everything on the wheel's count `object+0x35` (0, 1, 2):

| Mode | Original behaviour |
| --- | --- |
| 2 (which item) | 14, 15 or 16 by count: blue, then green, then red |
| 0 (can use now) | only that item's found bit (`player+0x38c/0x390/0x394` bit 0), or that item already in hand (`+0x3b8` = 1, `+0x3b9` = item); also refuses while `*(object+0x10)+2` is non-zero |
| 1 | nothing |
| 3 (complete) | clears the hand, sets the wheel model for count + 1 (table `*0x800ddb18`-based, `0x1980/0x1984/0x1988`), plays sound `0x7008`, counts up; from 2 to 3 it also sets `+0x15 = 0x20` on level objects 20 and 21 (the way out) |

Measured on private copies of the user's two saves:

| Save | Jewel flags (14, 15, 16) | Wheel count | Finding |
| --- | --- | --- | --- |
| ripspace (UI 12) | 0, 1, 0 | 0 | only green held; the wheel wants blue. Refused by the order rule |
| field laptop (UI 12) | used, used, **0** | 2 | the red jewel was never picked up (its pickup is still in the level at (13535, -397, 30725)); Duke also stood about 880 units out of reach |

With red given by a RAM write and Duke a step closer, the field-laptop save
completes in order (count 3, objects 20 and 21 `0x20`), in Modernized as in
the original protocol. So "the third jewel will not go in" there is the
missing jewel, not the device. Vanilla on the ripspace save: Cross with green
alone is refused twice; with blue given, blue and then green go in.

The original protocol, measured per frame at the wheel: the action press
(`0x80051890`) finds the wheel (`+0x174`, `+0x290`), sets `+0x27c` 3 and
stage `+0x27d` 1 after mode 0; stage 2 asks mode 2 into `+0x3bb`; animation
47's first frame (`0x8004e494`) clears the item's bit 0 and puts it in hand
(`+0x3b8` 1, `+0x3b9` item; D08A19 then adds the used marker `0x4000`); pose
266 pushes it in and mode 3 counts up.

## Design

The level overlay is compiled at runtime by the overlay cache, so it takes no
entry hooks, and patching its code would break its authentication. The fix
works around the handler's own protocol from main-executable hooks, so the
game still makes every use with the item it asks for:

- **Lend at the press** (`0x80051890` entry, fresh Cross, no use running):
  the host runs the game's own finder and asks the object's handler mode 2.
  If it wants a FAMILY JEWEL W (items 14-16; any lock that wants one, no level
  data) that Duke does not hold, and he holds another jewel J, the low
  halfwords of W's and J's flags trade places and J records the lend (bits
  `0x0c00` = W - 13). The press, mode 0, the line-up, animation 47, the push
  and mode 3 are the original's, with W.
- **Trade back** (once per player update, after D08A19's marker): once W's
  bit 0 is cleared (Duke took it in hand), the flags trade back. J is used, in
  the next place, and W keeps what it had (not found, or used earlier as
  another jewel). The hand (`+0x3b9`) and the use's item (`+0x3bb`) become J,
  so a use given back (`0x80091fc4`) returns J. A press the lock refuses, or a
  use that ends before animation 47, trades straight back.
- **Order**: each used jewel records the place it went in (bits `0x3000` =
  1..3). Used jewels without a place (older saves, Vanilla-made uses) take
  places in item order, as the game set them; a jewel found again loses its
  place.
- **Choice**: Enter on a browsed jewel in the mission inventory (D08A18)
  prefers that jewel; its usability check lends it for the lock's own mode 0
  and trades straight back, without a game frame in between. Otherwise E
  takes W when held, else the first found jewel.
- **Presentation** (D08A45): the hand shows the lent jewel J while the game
  holds W; the wheel's k-th socket shows the jewel with place k. The mission
  inventory shows the jewels as they are during a lend and never sees the
  host's bits.

All state is in Duke's own flag words, in guest RAM, so a savestate taken at
any moment (a lend included) continues. The upper halfword of a flag word is
an inventory amount (the medkit charge at `+0x356`), so it is not used. The
low bits used are free: the game tests bit 0 and the host's `0x4000`.

## Inventory cheats (user report, 2026-11-20)

User, after the first build: "i did insert the green gem ... then i did
dnstuff, and the gem in the slot on the wheel turned blue!" The original key
grant `0x8003d840` (run by `dnstuff`, `dnkeys`, `dnitems`) ORs bit 0 into
every mission item 6..16, used ones too. Green looked found again, so
D08A19 dropped its used marker, D60 its place, and the wheel's socket fell
back to the first colour not shown (blue); green could also have gone in a
second time. Now the host cheat (`cheats.inc`) restores every mission item
that was used up (no bit 0, `0x4000`) after the grant: the cheats give what
Duke does not have. Verified on the ripspace save: green in, `dnstuff` (blue
and red given, green still used in place 1, its socket green in third
person), then E puts blue in second and red third and the wheel completes.

## Implementation (title only; no framework change)

- `recomp/src/ttk/jewel_device_rules.h`: the flag rules (lend, trade back,
  undo, places, choice, mission view), pure.
- `recomp/src/ttk/jewel_device.inc` (in `modern_controls.cpp`, after
  `door_use.inc`): the press-time lend, the per-update step, the Enter
  check, `DNTTK_JEWEL_TRACE=1` (developer log).
- `modern_controls.cpp`: `0x80051890` entry calls `jewel_device_action`.
- `shortcuts.inc`: `select_weapon` runs `jewel_device_update` after
  `mission_used_track`; the mission array goes through `jewel_mission_view`;
  `mission_use` accepts a found jewel at a lock that wants another jewel.
- `jewel_colours.inc`: the hand's lent jewel; sockets in place order.
- `cheats.inc`: the inventory cheats keep used mission items used.
- Test: `ttk-controls-test` group D60 (lend, wait, undo, trade back with the
  hand rewritten, places 1-3 over three trades, Enter's choice, older saves'
  places in item order, a jewel found again losing its place, mission view).

## Evidence

Private offscreen Modernized runs on the NVIDIA GPU (SDL offscreen, 180 Hz),
private card copies with the user's two saves, debug port, player files
hash-checked unchanged (`recomp/analysis/d60/probe.py`, local). "Given" means
the found bit set by a RAM write, standing in for the pickup.

| Check | Result |
| --- | --- |
| ripspace save as saved (real green pickup), E at the wheel | green set in first: count 1, green used place 1, blue back to not found; green in hand and in the wheel |
| All six orders from that save (flags cleared, each jewel given before its press) | each completes: count 3, objects 20/21 `0x20` (the game's completion); places 1, 2, 3 in the order used; the hand item right at every step; lends only where needed (none for blue, green, red) |
| First-person shots after green, red, blue | sockets red, green and blue (none pink, none twice) |
| Enter with blue and red held, red browsed | red set in (preferred over blue, which the wheel wanted); blue kept |
| Savestate taken 3 frames into a lent use, loaded | starts mid-lend (green lent to blue), the game consumes, the trade back follows: count 1, green used place 1, blue not found |
| Savestate after one jewel, loaded, the other two | completes, places 2 and 3 |
| field-laptop save (blue and green used before D60, no places), red given | blue and green take places 1 and 2; red goes in third and completes; wheel blue, green, red |
| Lent press refused (wheel's busy halfword forced) | traded straight back (green found, blue not found, count 0); retry after release works |
| Vanilla, ripspace save, Cross with green only | refused twice; no host bits written |
| Vanilla, blue given | blue, then green go in (original order) |

Suites: `ttk-controls-test` (155 groups, D60 included), `ttk-near-test`,
`ttk-input-test`, `ttk-inventory-test` and the other native tests; Python 266
OK (2 skipped).

## Limits

- Played by the user on the real display (2026-11-20); the order sequences
  and savestate checks are from offscreen runs.
- Pickups were given by RAM writes except the user's own green; the game's
  pickup sets the same bit (D08A45 verified the pickup).
- Not established: the red jewel's reachability in the field-laptop save
  (its pickup lies about 8000 units higher than the wheel room's floor).
- The lend applies to locks that want items 14-16 (the FAMILY JEWELs). Other
  multi-item locks keep their own rules.

## Player check

FAMILY JEWELS (UI slot 12 on either machine): stand at the wheel with any
jewel and press **E**; or browse to a jewel with **,** / **.** and press
**Enter**.

```sh
python3 recomp/tools/local/run.py
```
