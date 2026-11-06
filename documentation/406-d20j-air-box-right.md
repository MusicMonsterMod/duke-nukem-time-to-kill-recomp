# D20J - The air box joins the countdowns on the right

Status: **Done** (user-accepted 2026-11-06: "lock it in and close it out.
amazing work"). Dependencies: D20I ([note 376](376-d20i-level-stats.md)).

## Problem and decision

Under water the original air box (status bar element 3) is drawn above the
health box, in the bottom-left band where D20I's LEVEL STATS sit, so the stats
covered it. The job first proposed moving LEVEL STATS out of the way (option
mockups). The user's decision: "jetpack fuel is displayed on the right. and in
fact, all countdowns are displayed on the right. ... why not just display the
oxygen on the right. problem solved". Revision 2 (user): no slide, it appears in
place like the other boxes.

## The original

Status bar `0x8008ba30`, layout `0x800dd778` (8 bytes per element, one (x,y)
pair per player slot, centre-relative), state `0x800dd7b8` (same stride; bit 0
visible, then the slide offset). Element 3, air: layout `+24` (-246,69), icon
record `0x800c44d4`. At `0x8008c4c0` the status bar turns it on when
`player+0x22c == 5` or air `player+0x236 < 12000` and sets its offset
(`+26`) to health y - air y; each draw subtracts the frame step / 4 until 0. It
turns it off when neither holds. The number is air * 100 / 12000.

## Implementation

`recomp/src/ttk/gadget_hud.inc`, `gadget_hud()` (status bar entry, Modernized
single player, inside the D14 / D08A6 saved layout window that
`widescreen_hud_restore` undoes at `0x8001fc44`):

- After the D08A6 / D08A8 / D08A17 stack is counted (selected gadget or
  jetpack in the slot above ammo, running steroids, running power-up coins), the
  air pair is written at the slot's x and one box row (box height + 4) above
  the top of that stack.
- On the original's own show condition the visible bit is set first and the
  offset kept 0, so the rise never starts. Hiding is the original's.

Saves, menus and Vanilla never see the change. LEVEL STATS keep their D20I
place. No framework edit.

## Evidence

`recomp/analysis/d20j/probe.py` (local; private D20I fixture copy, slot 10, air
set below full through the debug port, player files hash-checked unchanged).

| Check | Result |
| --- | --- |
| 16:9 borderless 60 Hz | air over the selected medkit; one row higher with an invincibility coin running |
| 4:3 full boxes 180 Hz | same, boxed; offset 0 from the first read, final place on the next shot; gone at 12000 |
| Vanilla 4:3 | original box above health, original rise (offset 14 at the same moment) |

Executable `a76810b8d0cdc1bfa53ce5ea238fa335ecff46a21538189b5567ec070795acba`.

## Limits

Accepted without a recorded test of a real dive, air with the underwater
jetpack (D08Q10), Bio Mask / goggles on, or STATUS BARS HIDDEN.
