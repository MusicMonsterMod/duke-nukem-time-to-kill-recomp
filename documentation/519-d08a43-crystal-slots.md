# D08A43 - A crystal put into a level device stays visible

Status: **Done, user-accepted and closed out 2026-11-20** ("fix accepted! great work king", "close it out big boy!"). Executable
`c489e33199f31006a1c5ccb2a928488401d247216441584169cb0cff10613a12`
(also carries D17BL).

## Report

User, 2026-11-14 (playthrough commentary): after putting the green energy
crystal into the conveyor-belt power slot in OBEY OR DIE (level 5), the
crystal is not visibly there, though it has to be fetched back later.

## How the game does it

Level 5's objects (level object table `0x800d2550`, from a private run):

| Slot | Type | What |
| --- | --- | --- |
| 1 | 728, kind 6 (LEVEL05 overlay handler `0x800cd878`, subtype 1) | the conveyor power slot, at (-31688, -10799, -330), a round cog on a red brick wall |
| 2, 3, 4 | 176 red, 178 green, 177 blue | the crystals |
| 12, 13, 14 | 180/182/181, kind 17 (generic holder `0x80092e98`) | the holders the crystals start in (+0x35 = 1, 2, 3) |
| 7, 10, 11 | 180/181/182, kind 17 | three empty holders together (the time machine) |

- **The generic holder** shows a crystal put back into it: mode 3 moves the
  crystal object to the holder (194 units up), finds its room
  (`0x8005d790`) and links it into the room's object list (room `+0x48`,
  `0x8001c6dc`; nodes hold the object at `+8`, the next node at `+4`).
- **The conveyor slot** (overlay subtype 1) accepts any crystal in Duke's
  hand, writes its state to `+0x35` (1 red, 2 green, 3 blue), switches its
  own model to its type's model 1 (`0x80092bdc`: models 1577 -> 1437, the
  ring turns from red to green), starts the conveyor (object 6) and clears
  the hand. The crystal object stays in no room list, so it is invisible.
- **Taking it back** is the game's crystal take `0x80091cec`: the device's
  `+0x35` names the crystal (`0x80092d78(level, state - 1)`: level 5 gives
  slots 2, 3, 4), the take unlinks the crystal from its room's list
  (`0x8001c9ac`, which does nothing when it is in none), puts it in Duke's
  hand, calls the device's handler in mode 3 (the slot powers off and turns
  red) and clears `+0x35`.

## Fix (Modernized, game-wide, the game's own data)

`recomp/src/ttk/crystal_slots.inc`, run each game frame beside the debug
spawn (game thread, Modernized only):

- For each crystal of the level (`0x80092d78`, in the crystal levels 0, 5
  and 9) that is neither found nor in Duke's hand, a level object whose
  `+0x35` names it, which is not a crystal and not a generic holder, and whose
  type has more than one model, holds it. A device whose model the game has
  switched from its first is preferred.
- The crystal object is linked into that device's room with the game's own
  list routine, at the device. For a wall plate (its thinnest axis
  horizontal) the crystal lies along the plate's normal, pushed into the
  plate's central hole from the face that looks into its room, 60 units of it
  showing: a quarter turn about z (+0x20 = 1024, the drawn copy at +0x26..
  too) for a plate facing along x, about x (+0x1e) for one facing along z,
  measured in the lab (+0x1c yaw, +0x1e about x, +0x20 about z). On a flat
  device it would stand on top. For the conveyor slot: (-31635, -10799, -330),
  angles 0, 0, 1024. Taken back, the crystal is set upright (0, 0, 0) for the
  holders. (Revision 2, user 2026-11-20: "the crystal should insert into the
  central hole horizontally, not sit vertically on the slot".)
- The game's take unlinks it as before. All state is guest RAM, so savestates
  carry it. A crystal this rule placed whose device let go of it without the
  take is unlinked again. A crystal the game itself has linked somewhere is
  never moved.

Developer: `DNTTK_CRYSTAL_SLOTS=0` turns it off. Log line
`[TTK crystal slot]` for each placement.

New developer console command for the lab, `warp x y z` and
`warp object N [distance] [degrees]` (in `spawn.inc`, Modernized): moves
Duke on the game thread in one step (position, room from the room boxes,
facing, view yaw). The debug port's byte writes could not move him (the game
ran between bytes) and its `pause` has been removed.

## Evidence

Lab `recomp/analysis/d08a43/` (local only; `probe.py` cases `cycle`, `red`,
`lv`; `objs.py` lists the level object table from a RAM dump; `mdis.py`
disassembles). Private card copies; the player's saves and profile
hash-checked unchanged by every run. Fresh `level 5` with `dnkroz`, the green
crystal taken from its holder with E, then:

| Step | Slot `+0x35` | Green flag | Crystal | Picture |
| --- | --- | --- | --- | --- |
| Before the fix, inserted | 2 | `0x4000` (used) | in no list | green ring, no crystal |
| Inserted | 2 | `0x4000` | linked in room 24 at the slot | crystal seated in the cog |
| Savestate saved, another loaded, this one loaded | 2 | `0x4000` | linked | crystal still in the cog |
| Taken back with E | 0 | `0x1` (found) | unlinked | red ring, empty |

Levels 0 and 9 start with no placement (their crystals use generic holders).
Pictures: `research/before-after/D08A43/`.

## Cheated crystals (user, 2026-11-20, revision 3)

User: "the red crystal in dnstuff does not appear in the slot ... cheated
crystals do not appear in the slot still" (natural red and green both show).
`dnstuff` sets the crystals' found flags but leaves the crystal objects in
the holders they start in (holders +0x35 still 1, 2, 3; each crystal still
linked in its room), and the rule left any crystal the game had linked
elsewhere alone. Now a crystal a device holds that is still in a generic
holder naming it (within 600 units) leaves that holder for the device: the
holder's +0x35 is cleared as the take clears it, then the crystal is moved as
usual. Lab: fresh level 5, `dnstuff`, E at the slot takes the red crystal
(the overlay checks red first); it shows in the hole, its holder is empty,
and taking it back restores the flag and empties the slot.
(`research/before-after/D08A43/after-dnstuff-red-crystal.png`.)

The user's first-press report turned out to be this: the first try used a
`dnstuff` crystal, the second a crystal found in play.

## Choosing the crystal (user, 2026-11-20, revision 4)

User: "it would be actually fine to just be able to use any of the 3 crystals
in that slot". The slot takes any crystal, but its handler's mode 2 asks for
the first one Duke has (red, green, blue), so with several it always took red.
Now Enter on a browsed crystal in the mission inventory (D08A18) at a device
that wants a crystal sets the other found crystals aside for that use (their
found bit moved to bit 0x0800 of their own flag word, so savestates keep
them), the device then asks for the browsed one and the normal use runs; the
others come back as soon as the chosen crystal is in Duke's hand, after 120
game frames if the use never starts, or after any load. E alone keeps the
game's order. Lab: `dnstuff`, browse to the green crystal, Enter at the slot:
green in the hole, its holder empty, red and blue found again
(`research/before-after/D08A43/after-chosen-green-crystal.png`). The lab's
test-input words gained `comma` and `period`.

## E needing a second press (user, 2026-11-20)

The user's first insert did not work, the second did. In the lab the game's
own action finder (`0x80077f28`) needs Duke in contact with the slot; a press
without contact puts the weapon away and draws it again and nothing else
happens (target `player+0x290` stays 0). With the lab's warp, a press right
after taking the crystal from its holder missed twice and worked the third
time; after 10 s of waiting the first press worked. The lab places Duke by
warp, so contact differs from walking up; whether Vanilla behaves the same
was not checked (the warp is Modernized only). Answered: see Cheated crystals above.

## Limits

- Tested with the green crystal; the red holder was not reached by the lab's
  warp spots (the rule does not depend on the colour).
- In the lab, E at the slot sometimes needs several presses to insert or
  take (before the fix too): warp-and-press timing, not the rule.
- Level 9's crystal devices (LEVEL09 overlay) were not exercised; at its
  start nothing is placed.
- Offscreen runs; the player's real play through OBEY OR DIE is the playtest.
