# D08A12 - Modern dynamite: selecting it is safe

2026-10-08. Status: Done (user-accepted 2026-10-08: "perfectly done"). Selecting dynamite (6, the wheel) must not
commit Duke to a lit fuse. Dynamite Behaviour `modern` (Modernized default) /
`original`; Vanilla always original.

## Original state machine (SLUS-00583, read-only disassembly)

Player fields: equipment `+0x3b8` (2 = weapon drawn), drawn weapon `+0x3b9`
(14 = dynamite), requested weapon `+0x3ba`, flags `+0x224` (bit 2 = weapon
request), upper-body animation `+0x74` (39 = holding a throwable, 40 =
draw/holster, `+0x7e` = 1 plays it backwards), fuse `+0x250`, throw charge
`+0x248` (start time) / `+0x24c` (charge), looping-sound handle `+0x260`.

1. **Equip = ignition.** There is no unlit state. The draw completion (anim 40
   end, `0x8004e39c`, table `0x80014d20`, case `0x8004e5ac`) sets equipment 2,
   weapon 14 and the fuse `+0x250 = *(int16*)0x800c4ed8 = 4000`.
2. **Fuse.** The held-throwable handler `0x8004dea4` (called from upper-body
   animation 39 and two other states; table `0x80014ce8`, case `0x8004e018`
   for dynamite) subtracts about 3.33 x the frame delta (`0x800d21fc`) each
   update. Measured: about 18 per field at 60 Hz, so 4000 lasts about 220
   fields (3.7 s); live, the stick went off 226 fields after the draw.
3. **In-hand explosion.** At 0 or less: blast at Duke (`0x80070f7c` with the
   weapon's damage record), sizzle stopped (`0x8006af44`), sound `0x6001`,
   ammo -1 (unless `0x800c3cc4`), equipment 0.
4. **While burning:** sparks `0x8006d594(6, pos, 0xff)` (ra `0x8004e138`),
   the sizzle loop `0x8006b270(0x101d, pos, 0x900)` into `+0x260` when it is 0
   (ra `0x8004e154`), then the throw check `0x8004dc74`.
5. **Throw.** `0x8004dc74` reads Cross (`*(0x800d1b50[+0x233])`, a shift
   register: bit 0 this tick, bit 1 the previous one). Press with `0x241` clear
   starts the charge, holding raises `+0x24c` to at most 700, and release (or
   full charge) returns throw animation 41/42 (43 on the jetpack), unless
   `0x4000` is set. The throw event `0x8004e6a0` spends one stick and hands the
   remaining fuse to the thrown object (`0x8003c500`); the end of the throw
   (table `0x80014d70`) empties the hand (equipment 0), as in Original.
6. **Switching away.** The other throwables' cases (Holy Hand Grenade
   `0x8004e180`, pipe bomb `0x8004e1d4`, knife/axe `0x8004e260`) start the
   holster (anim 40 backwards) on a weapon request, or when the holster button
   (`*(0x800d1a90[+0x233])`, released = `&3 == 2`, `0x4241` clear) is let go.
   Case `0x8004e018` checks neither, so a request waits until the stick is
   thrown or explodes (the D08A / D08Q5 observation).

## Can equip and ignition be separated?

Yes, cleanly. Everything that makes the stick "lit" happens inside one handler
call per update, after the dispatch on weapon 14: fuse decrement, sparks and
sizzle. The draw only writes the fuse value. Nothing else reads the fuse while
Duke holds the stick.

## Modern implementation (`recomp/src/ttk/dynamite.inc`)

Three opt-in entry hooks (`game.local.toml`, regenerated; one generated line
each, codegen hash `0x8bab543c` unchanged, savestates load):

- `0x8004dea4` entry, Duke only, dynamite in hand, not yet lit, code identity
  intact:
  - **Cross down** where `0x8004dc74` would charge (lower-animation flag
    `0x20000` and `+0x224` bit 5 clear, `0x4241` clear): **light it**. The fuse is
    set full and from then on the original runs untouched (sparks, sizzle,
    charge, throw; holding past the fuse explodes in hand as before).
  - Otherwise **unlit**: the fuse is set full again (the original then takes one
    update's worth, so it shows about 3951), and this call's sparks and sizzle
    are kept quiet. A request (bit 2), or the holster button released with
    `0x4241` clear, starts the holster exactly as `0x8004e180` does
    (`+0x74 = 40`, `+0x7c = 0`, `+0x7e = 1`); the original completes the switch.
- `0x8006d594` entry, ra `0x8004e138`, unlit call: brightness `a2 = 0`
  (`0x80029ab4` makes a black, invisible spark).
- `0x8006b270` entry, ra `0x8004e154`, unlit call: sound id `a0 = -1`, which the
  routine refuses (`bgez` at `0x8006b2a0`), so `+0x260` stays 0 and no handle
  is left.

"Lit" is host state. It is cleared when the stick leaves the hand (any
controls hook sees `+0x3b8 != 0x0e02`) or a savestate is loaded, so a reloaded
or newly drawn stick is unlit. The D08A selector was not changed: it already
offered requests while Duke holds a throwable (idle animation 39) and refuses
them while fire is held.

New guards: `0x8004dc74` (560), tables `0x80014ce8` / `0x80014d20` (56 each),
`0x8006b270` (52), `0x8006d594` (100), `0x80029ab4` (24). `0x8004dea4` (2040)
was already guarded.

Profile schema 32: `dynamite` `modern|original` (`run.py --dynamite`,
`--settings` choice T); the launcher passes `DNTTK_DYNAMITE` (always `original`
for Vanilla). Debug: `ttk_input` -> `controls.dynamite` (`lit`, `holds`,
`lights`, `stows`, `quiet_sparks`, `quiet_sizzles`).

Unchanged: damage, blast radius, ammo, throw physics and charge, the thrown
object's fuse rules, enemies and the environment. One difference: the thrown
stick's fuse now counts from the fire press, not from the draw. In Original
the time spent holding is lost; in Modern a thrown stick always has nearly the
full fuse (live: 3363 left at the throw, against 3510 in a quick Original throw).

## Next stick after a throw (follow-up, 2026-10-08)

User: "the only thing i want is for the next stick of dynamite to be drawn
after throwing the first one". The original empties the hand at the end of a
throw (equipment 0, request byte still 14) and draws again only on the next
fire press. Modern now queues the next stick:
- **When it queues:** whenever a stick leaves the hand during throw animations
  41-46 (`dynamite_track`, any controls hook).
- **How it draws:** `select_weapon` issues the same request as the 6 key
  (`+0x3ba = 14`, `+0x224` bit 2). It waits until fire is let go, precision
  aim is off and `0x2c5` is clear, on the ground or in jetpack flight.
- **When it drops:**
  - Another weapon or request comes first.
  - Duke is out of sticks (the original then picks another weapon).
  - A savestate is loaded.
  - The option is `original`.

`+0x224` bit 8, set while a throw charge runs (observed), now also counts as
lit, so a press the previous update already started is never left unlit.

Evidence (executable `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d`):
- **Redraw:** four throws in a row, each followed within about 45 fields by
  the next unlit stick (fuse full) and a 2 s hold. The throws count down from
  20 to 16.
- **Holding fire:** keeps throwing. The original's own empty-hand fire press
  draws, so the redraw stands aside.
- **Last stick:** no redraw. The original switched to the Holy Hand Grenade.
- **Switching during a throw:** 3 pressed during the throw animation is
  refused (the D08A rule for attacks and throws), so the redraw wins. 3 works
  once the stick is in hand.
- **Suites:** `ttk-controls-test` 47 groups. New assertions cover throw vs
  stow/explosion, Original, and charge bit 8.

## Evidence (executable `c5b97f7956d80fa8bd3a3b6010e6466c5143f91682ce2c122b89840e523c7e59`)

Private Xvfb runs on card/savestate copies (`recomp/analysis/d08a12-dynamite/`,
local; the D08A19 private copies, savestate slot 5, `dnweapons`).

- `t1.py` Modern:
  - **Held:** dynamite held 6 s, fuse steady at 3951, health 10000, `+0x260` 0. The
    screenshot shows no spark at the fuse tip.
  - **Stow:** 3 stowed it to the shotgun (`stows` 1), ammo 20 unchanged.
  - **Wheel:** rapid wheel x3 down and up through the group passed over it. A
    slow wheel, one notch per 70 fields across every weapon, stopped on
    dynamite and moved on (`stows` 2). Health and ammo were unchanged.
  - **Savestate:** saved with the unlit stick in hand and loaded back: still
    unlit, fuse full after 3 s, then 3 stowed it.
  - **Throw:** LMB lit it (`lights` 1, sizzle handle set, the spark visible),
    released into throw 41. One stick was spent (19), the hand emptied, and the
    explosion showed in front of Duke.
- `t2.py` Original (`DYN=original`):
  - **Fuse:** runs from the draw (3787 at the first sample).
  - **3:** only queued the switch (flag 4, request 5) while the fuse ran down.
    The stick exploded in hand 226 fields after the draw (ammo 19, hand empty),
    then the queued shotgun was drawn.
  - **After a throw:** Duke is empty-handed, as in Modern.
  - **Counters:** the Modern counters stayed 0.
- `ttk-controls-test`: 47 groups, including the new D08A12 group:
  - **Unlit:** fuse full, sparks dark, sizzle refused, and only for the
    handler's own calls.
  - **Stow:** a request or a released holster button stows it. `0x4241` and a
    held holster button do not.
  - **Lit:** Cross lights it, after which nothing is rewritten. The hand
    emptying resets it.
  - **Not lit by Cross:** the charge's reset state and `0x241` keep it unlit.
  - **Untouched:** other actors, other weapons, `original` and Vanilla.
- `ttk-input-test` passes. The Python suites run 142 tests OK (10 skipped).
  `level_overlay_guards.py --check` passes.

## Limits

- **E with dynamite drawn:** stowing via the holster button that E pulses is
  covered natively, not live.
- **Jetpack and first person:** not run live. The hook does not depend on the
  movement mode.
- **Health:** the in-hand Original explosion did not lower health in this
  state, armour 100. Damage is the original's and was not changed.
- **Hold past the fuse:** a lit stick held past its fuse explodes in hand as
  in the original. That is the "once armed, normal fuse rules" choice. The
  original also throws on its own at full charge (`+0x24c` 700), so a long
  hold normally throws first. The relative timing was not measured.
