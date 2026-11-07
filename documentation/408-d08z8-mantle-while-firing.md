# D08Z8 - Mantling while firing

Status: **Done** (user-accepted 2026-11-07 at revision 7: "i accept it"; earlier: walking (rev 2) and run-jump object mantles (rev 3) confirmed by the user). Modernized only; Vanilla unchanged.
Revision 1 executable `e13f554fda1fe6cd2437c38c859075754e9dc9d2ec7342af3fbbf29a7a945779`.

User (2026-11-07): mantling while actively shooting the Desert Eagle fails;
stopping, mantling, then shooting works. Repro: the post in UI slot 8.

## Cause

On land the mantle is the original's E sequence: the host stows the weapon
with one Circle tap (`interaction_holster_ready`: equipment 2, upper body
idle 5 / 20 / 29 / 35 / 39), then E's Cross starts the reach (lower 72, 74)
and, when Duke walks into the face (walk 72 / 74), the obstacle action
`0x80051cf0` starts the mantle (139, 140, mode 8). Logged on a private copy of slot 8:

| Run | Upper track | Equipment | Result |
| --- | --- | --- | --- |
| W + E | 6 (stow), 0 | 2 -> 0 | 72, 74, 139, 140, on top |
| fire held, then W + E | 16 (fire) throughout | 2 | walks 72, 74, stands 63 |

Two host rules stopped it (`recomp/src/ttk/pc_input.cpp`): held fire cleared
the pending E (`interaction_pending && action_down(fire)`), and the stow tap
waits for an idle upper body, which held fire never gives. Armed Cross is
fire, so the original never mantles armed.

## Fix

An E pressed while fire is already held (and the weapon out, in ground
movement, Modernized) sets `interaction_over_fire`:

- held fire no longer cancels that E;
- the fire button (Cross) is kept off the pad while the weapon is still out,
  so the upper body settles (Desert Eagle 16 / 10 -> 14 -> 15 -> 5) and the
  original stow runs;
- it lasts through E's Cross pulse after the stow, and `fire_draw_ready`'s
  draw is held off meanwhile (it would take the weapon back out);
- it ends when fire is released, the E expires, or the climb starts
  (`interaction_climb_started`: mode 8 or 6), where D08J8 mantle fire draws
  the sidearm and fires through the climb.

A fire press after E still cancels the E (only fire held before E counts).
In a ledge hang E is the D08J8 pull-up and is left alone.

## Evidence (private offscreen runs, copy of the player's saves, `recomp/analysis/d08z8/`, local)

- Slot 8 post, Desert Eagle, fire held 1 s then W + E: stow, reach, 139,
  sidearm fire during the climb (track 11, fire request on), on top, still
  firing. First and third person (three third-person runs; one earlier
  third-person run that had emptied a clip into a reload did not climb and
  did not repeat).
- Running at the post with fire held and an E tap: stow at 1.08 s, 139 at
  1.14 s, over the post firing (ammo 177 -> 163); before the Cross-pulse
  rule, held fire redrew the weapon after the stow and nothing climbed.
- Shotgun: mantles; D08J8 swaps to the sidearm for the climb as designed.
- E then a fresh fire press: no stow, fire goes through (unchanged).
- Open ground, fire held, E: fire pauses, stow, the weapon comes back (no
  ledge), firing resumes after about 1.1 s, as for a plain E.
- `ttk-input-test` passes, with a new D08Z8 group (fire leaves the pad, the
  Circle stow, E's Cross without a fire draw, the later-fire cancel).

## Limits

- The stow waits for the weapon's own fire track to settle (about 0.45 s for
  the Desert Eagle): a stow request inside tracks 14 / 16 is ignored by the
  original (tried, it wastes the one-shot stow), so this is not shortened.
- Not run: the user's own approach at 180 Hz, other weapons than the Desert
  Eagle and shotgun, jumping mantles (D08Y) with fire held, water climb-outs
  (D08O3) with fire held, Vanilla (gated by `input_modern_controls()`).

## Revision 2: the Desert Eagle climbs without a pause

User (2026-11-07): "its not smooth enough ... make this really unintrusive
when you mantle up. if its already the desert eagle, duke should just be
able to continuously shoot." Executable `5b370e4507a14a935640556e3a389f89e1dff89fd72c098e40a39c7dab5003ae`.

### The original obstacle action (`0x80051cf0`)

Called by the walk / run handlers (ra `0x80052544`, `0x8005349c`,
`0x8005384c`) while Duke walks into something. Gates in order: Cross this tick
(action word bit 0) or `+0x224 & 0x241`; the upper track without upper-table
bit 8 (every armed pose has it: 5, 6, 10, 11, 14-17, 20); then the contact
(`+0x174`) mounts (ladder-type flags 0x200 / 0x400 / 0x100, unarmed only, line-up
`0x8007ec4c`), else the ledge probe `0x8007d598` and the height `+0x1c4`:
134 (> -0x1bf) .. 138, 139 for a climbable object (flag 0x80) higher up.
With equipment 2 it already allows a weapon whose table entry
(`0x800c4590 + 44 * weapon`, halfword 0) has bit 1 (the Desert Eagle: 7) on
the low mantles (`+0x1c4 >= -0x2ff`) and refuses higher ones with
`+0x224 |= 0x20`; the bit 8 gate stops every armed Duke before that.

### Design (`recomp/src/ttk/sidearm_mantle.inc`)

- E with the climb sidearm out on the ground (D08J9's `door_owner`, weapon
  `climb_sidearm`, no use running) opens a question (`sidearm_mantle_ask`, from
  `pc_input.cpp` when a new E is pending); the E stow waits while it is open,
  and revision 1's fire mask is off.
- At the obstacle action's entry, with the question open, Duke is lent as the
  original sees a holstered Duke: `+0x3b8` = 0, the upper animation id = the
  lower one (a free upper body), Cross this tick. The original then decides
  and does everything, and its caller gets its own answer.
- The loan closes at this update's fire request entry (`0x80058120`, ra
  `0x80041c44`, with the D08J9 door window) or the next player update: the
  action word bit, the equipment and the upper id go back. No savestate is
  taken or loaded in between (`psx_mod_host_call_begin`).
- A new mantle (134..139) answers E (no stow). `trav_drawn` marks the climb's
  sidearm, so a 139 that ends in the ledge hang pulls up one-handed (D08J7C).
  Fire held: D08J8 drives the fire track through 134..139 (track 11) and the
  original fires in 140. Not firing: the upper track takes the climb
  (`mantle_upper_handoff`, now shared with the D08Z2 jump mantles) and the
  weapon pose returns when it ends.
- "No": a ladder-type contact, no movement input (the original climbs only
  what Duke walks into), Duke no longer the owner, or no mantle within 12
  player updates of a tap (60 with E held). E then stows as before.
- A door use (D08J9, answered first at the player update) withdraws the
  question; a fresh fire press or the holster key cancels it with the E.

### Evidence (private copy of UI slot 8 / 10, offscreen)

| Case | Result |
| --- | --- |
| Fire held, W+E (first person, 60 Hz) | equipment 2 throughout; walk-in 72 / 74 firing; 139 at 1.10 s (as unarmed); track 11; hang 149 for one update; 140; on top; a shot every ~165 ms (one 233 ms gap at 149 -> 140) |
| Same at 180 Hz, screenshots | the pistol fires through the climb, the free hand reaches the lip in the pull-up, on top armed |
| Third person | the same sequence |
| Running in, fire held, E tap | 139 at 0.57 s (revision 1: 1.14 s), firing over the post |
| No fire, W+E | armed climb, upper 139 / 140, gun pose 5 on top (one hand-off) |
| Shotgun, fire held | no question; revision 1 stow, D08J8 sidearm through the climb |
| E, then a fire press | the E is cancelled, Duke fires |
| E standing still | stow at once (0.13 s) |
| Slot 10 door, E with or without fire | the armed door kick (116) as D08J9; the question withdrawn |
| `ttk-input-test` | pass; revision 2 group: no stow or fire mask while asked, "mantled" consumes E, "no mantle" stows |

### Limits

- Low mantles 134..138 armed and jump mantles were not run with this path
  (jump mantles never meet it: they start from flight, D08X / D08Y).
- The walk-in before the original action first answers (about 1 s standing at
  the slot 8 post, 0.17 s from a run) is the original's own pace.
- Not run: ladders with the Desert Eagle (contact flags send them to the stow,
  by code), water, the user's own approach.

## Revision 3: run-jump mantles

User (2026-11-07): walking mantles confirmed; "run jump to mantle still
pauses and isnt smooth". Executable `db919fa00977881b97e2016df12c5a7527c7504c9875b27a1b737602b4aab5c0`.

Cause (slot 8 post, fire held, run, Space + E): E pressed with fire held set
revision 1's mask; the upper body settled (10, 8, 9, 5), the E stow ran in
flight (6, equipment 0 at 1.07 s), and only then the D08X jump object mantle
(`ledge_reach_mantle`, equipment 0 only) started 136; D08J8 drew the sidearm
again. No shot from 0.51 to 1.13 s.

Change:
- `pc_input.cpp`: revision 1's fire mask is off for an E that meets a jump
  with the climb sidearm out (`sidearm_in_flight`: takeoff 96 or mode 9, or the
  Space press window). With fire held the upper body never rests, so no stow.
- `ledge_reach.inc`: the jump mantle also starts with the climb sidearm drawn
  (`sidearm_drawn`), through `jump_mantle_start` (the shared D08Z2 hand-off);
  `trav_drawn` marks the climb's sidearm. If the mantle cut a mid-air E stow
  (upper 6), the kept weapon block becomes ready (5, frame 0), so the gun does
  not finish the stow on top.
- Without fire nothing changes in flight: the stow still starts and, with no
  object to mantle, the original reach grabs world ledges unarmed as before.

| Case (private copy, slot 8 post) | Result |
| --- | --- |
| Run-jump, fire held, E (60 Hz) | 104 at 0.35 s, 136 at 0.52 s, equipment 2 throughout; 15 shots, gaps 0.155-0.206 s |
| Same at 180 Hz | gaps 0.160-0.204 s; screenshots: the pistol fires up and over the post |
| Third person | gaps 0.159-0.204 s |
| Run-jump, no fire, E | 137 armed (upper 137), ready 5 on top, no stow |
| Walking, fire held / no fire | as revision 2 |

Limit (fixed in revision 4): a jump at a world ledge with fire held did not grab. Not run: other levels' objects, low jump mantles 134 / 135 armed.

## Revision 4: jump grabs of world ledges

User (2026-11-07, new UI slot 8 on the alley dumpster): run-jump to the
ledge ahead while shooting, "he cant mantle it while shooting". Executable
`3849d05174c4d6300e97f5d6b1d072fbb2021d1680c207310c1be0564914597c`.

The reach `0x800557f8` (first in the airborne handler `0x80055904`): with the
D08X reach armed (`+0x224` 0x800000, none of 0x241), Cross this tick and an
upper pose without table bit 8 it sets the lower to 109 (frames 0, clears
0x800000); with the lower on 109 it runs the ledge acquisition `0x80055208`
every update (grab 147 / 148, hang 149, pull-up 140). Held fire is Cross, so
the weapon pose was the only refusal. Table bits: 109 0x1, 147 / 148 0x61,
149 0x18042, 140 0x61 (no 0x2000 fire refusal).

- `sidearm_reach` (`sidearm_mantle.inc`), at the ballistic hook right after
  the reach (ra `0x80055934`, beside `ledge_reach_arm`): Modernized, fire and E
  held (`input_airborne_reach_held`), the climb sidearm drawn, a jump 98 / 103 /
  104, the reach armed and Cross this tick, an armed upper pose: the same
  transition the reach makes. The acquisition and everything after are the
  original's. Without fire the E stow runs in flight and the original reach
  grabs unarmed, as before.
- `sidearm_climb_live` (`ledge_reach.inc`): set by every armed sidearm climb
  (the walked mantle, the jump mantle, the jump grab), cleared on the ground;
  on reaching the ledge hang it sets D08J8's `trav_hang`, so W / E pull up
  one-handed (D08J7C). `trav_drawn` alone was cleared at once after a jump
  mantle (it starts in mode 0, where `trav_weapon_ready` refuses 139) or a
  reach (109 is no climb clip): the dumpster run-jump stayed in the hang.

| Case (private copy, new slot 8; clip set to 22) | Result |
| --- | --- |
| Run-jump to the ledge, fire + E | 109 at 0.39 s, 148 at 1.16 s, 149, 140 at 1.76 s, on top; gaps 0.16-0.24 s, one 0.33 s at the hang to pull-up hand-off |
| Same at 180 Hz / third person | the same rhythm (max 0.333 / 0.332 s) |
| 180 Hz screenshots (the save's own clip) | the grab, shots into the face from the hang, the one-handed pull-up, firing on top |
| Off the dumpster and run-jump back onto it (two timings) | 139 armed, 149, 140, on top; one 0.44 / 0.27 s gap at the hand-off |
| Ledge jump, no fire | stow in flight, unarmed grab 148 and pull-up, as before |

Not changed, measured: at the D08J7C hang to pull-up hand-off one shot is
late (0.27-0.44 s gap). The hand-off saves and restores the hang's fire track;
after a D08J8 host-driven climb (134..139) the restored track is late in its
loop and the original winds it down (9) and raises again (10) in 140 before
firing. This is the accepted D08J7C hand-off shared by every armed pull-up;
narrowing its Cross mask to the hand-off update did not change the gaps and
broke its input test, so it was put back. The save's own clip (2 rounds)
empties at takeoff, and the Desert Eagle's reload (track 12) takes about 0.9 s
of the jump; that is the game's reload.

## Revision 5: any other weapon on a jump grab

User (2026-11-07): the same ledge jump "while open firing with the shotgun.
duke should still mantle it and pull the sidearm out surely ... (any other
weapon)". Executable `240ccc9b776a14210b75c2fc9ac09aa869ee834b2f64d291d94123decb4b9263`.

Before (private copy of the new slot 8, shotgun, fire + E): revision 1's mask
hid fire, the shotgun's pump tracks (25, 26, 23, 24) never settled to idle in
the jump, so no E stow and no reach: Duke landed short with one shot.

Change (`sidearm_mantle.inc`, `sidearm_swap_reach` / `sidearm_swap_update`):
- `sidearm_in_flight` now covers any drawn weapon with sidearm ammo, so in a
  jump E keeps fire on the pad (and Cross this tick, which the reach needs).
- At the ballistic hook (after the reach, ra `0x80055934`), with fire and E
  held, the reach armed and a non-sidearm weapon: the whole player block
  (0x8a4 bytes) is saved, the lower set to 109, the acquisition
  `0x80055208` called through `original_call`, and every byte put back. Where
  it would have caught, the jump enters 109 as the reach itself does (lower 109,
  frames 0, the reach flag cleared), so the original's next update runs its
  own acquisition and catch (no state from inside the ballistic step survives;
  an earlier trial that kept the probe's catch showed a one-update bounce 107).
- At the grab (147 / 148 / 149) the sidearm takes the weapon's place as D08J8
  does on climbs: `climb_swap_from` keeps the weapon for SIDEARM RETURN,
  `+0x3ba` / `+0x3b9` = sidearm, upper 6 (the draw, 2x via
  `weapon_feel_draw_press`); the climb flag gives the one-handed pull-up.
  A reach that lands empty swaps nothing.

| Case | Result |
| --- | --- |
| Shotgun, fire + E, run-jump to the ledge (60 Hz) | 109 at 1.14 s, 148 at 1.19 s, the pistol drawn at the grab, first shot 1.65 s, hang, 140, on top firing |
| Same, third person / 180 Hz | the same timings; 180 Hz screenshots: shotgun in the jump, the pistol at the grab, shots into the face, one-handed pull-up |
| Shotgun, no fire | E stow in flight, unarmed grab and pull-up, as before |
| Shotgun, fire + E, jump with no ledge | keeps the shotgun (no swap) |
| Desert Eagle ledge jump | unchanged |

Limits: the shotgun fires once in the jump before the catch (its own pump
cycle in the air); the draw takes about 0.45 s before the first pistol shot.
Weapons other than the shotgun share the path (any weapon slot with the
sidearm usable) but were not run one by one.

## Revision 6: the Gatling's loop, and object jump mantles with other weapons

User (2026-11-07): "i tried with the gatling gun and the sound just
continued on!!" Executable `74fc48d7664b1019af65e238ea645939b6bfab97e72d52a50c0ea29fa5d563aa`.

The Gatling's fire (`0x80058b90..`): spin-up `0x100f` and the loop `0x1010`
(`0x8006b270(id, Duke + 4, 0x900)`) into `+0x260`, `+0x224 |= 0x800`. Its
end (`0x80058bd4`): `+0x224 &= ~0x800`, `+0x81c` = 0, `0x8006af44(+0x260)`,
`+0x260` = 0, the spin-down `0x1011` at Duke (`0x800`). A host swap writes the
sidearm into `+0x3ba` / `+0x3b9` and the draw into the upper track at once, so
that end never ran; traced: `+0x260` stayed `0x0f0e8b2c` after the swap.

- `weapon_loop_stop` (`sidearm_mantle.inc`, each player update with a CPU):
  after any host swap to the sidearm (`weapon_loop_stop_from`, set by D08J8's
  climb swap in `trav_fire_update` and by `sidearm_swap_in`), the original end
  above through `original_call`; the spin-down only when the weapon swapped
  out was a Gatling (7 / 28). Traced: `+0x260` = 0 from the swap update on.
- `sidearm_swap_in` (`ledge_reach.inc`): the one swap (weapon kept for SIDEARM
  RETURN, loop stop, sidearm, upper 6 and the 2x draw, the climb flag), used
  by the jump grab (revision 5) and now the object jump mantle: with fire held
  and another weapon drawn (sidearm ammo available) `ledge_reach_mantle`
  starts the mantle and swaps; the hand-off keeps no block of the old weapon.
  Revision 5 had left that case bouncing (no stow in the jump any more).

| Case (private copy, slot 8, E and fire held) | Result |
| --- | --- |
| Gatling, run-jump onto the dumpster | 139 armed at 0.25 s, pistol drawn, 149, 140, on top; loop handle 0 |
| Shotgun, the same | 139 at 0.24 s, pistol, on top; no gap over 0.25 s |
| Gatling, run-jump to the ledge | grab 148 at 1.18 s, swap at 1.21 s (handle 0), hang, pull-up, on top; gaps 0.49 s (the draw) and 0.43 s (the hang hand-off) |
| Desert Eagle, dumpster and ledge | unchanged (139 / 148, max gaps 0.43 / 0.33 s) |

Not heard directly (offscreen runs are silent): the stop is the original's own
call on the same handle the game would stop. Not run: the flamethrower (its
fire also keeps a handle in `+0x260`; the stop applies, no spin-down).

## Revision 7: the question must never block other E uses

User (2026-11-07): after a Gatling run-jump mantle while shooting, "i cant
change weapon ... and i cant mantle anymore". Executable `78a37fe292018b8eb293973cc684531dbdabef7c1e8fb20f4f6038c3d04da06b`.

Reproduction (private copy of the new slot 8; the user's session log showed
the Gatling ledge climb, then attempts from the dumpster top): after the
climb Duke holds the Desert Eagle (revision 5's swap, SIDEARM RETURN never).
From the dumpster top, W + E (with or without fire) toward the wall ahead:
asked (state 1), walking 72 / 74, no obstacle action call (loans 0), the
question ran its 2 s, no stow. The wall carries a ladder that hangs out of
reach; D08J1 leaps for it only with the weapon away (`ladder_leap_ready`
requires equipment 0). With the Gatling (no question) the same input stowed,
leapt, caught the ladder (148, mode 3) and climbed (190).

Changes:
- `ladder_top.inc`: `ladder_leap_geometry` (the reachable plain ladder ahead)
  split from `ladder_leap_ready`, which keeps its equipment test.
- `sidearm_mantle.inc`: the question answers "no mantle" at once with a
  ladder ahead or a ladder top below (`ladder_top_available`), after two
  obstacle actions in a row that did not mantle, or 0.67 s held without
  contact (was 2 s; a tap stays 0.4 s).
- `interaction_climb_started` (revision 1's fire hold) includes ladders
  (mode 3) and monkey bars (7).

| Case | Result |
| --- | --- |
| Desert Eagle, W + E at the ladder wall | "no" at 0.2 s, stow, leap 96 / 109, ladder 148, top 190, up to the top |
| Same with fire (clip 22) | the same; at the top D08J8 draws the sidearm and fires (ammo 51 -> 41) |
| Gatling, W + E + fire at the ladder wall | stow 0.35 s, leap, ladder, sidearm at the top, firing |
| After a Gatling ledge climb: release fire, keys 3 / 4, wheel | each changes weapon (draw tracks, ready) |
| Dumpster walk-in, Desert Eagle, fire + E | 139 armed, 149, 140, on top, max gap 0.20 s |
| Dumpster run-jump, Gatling / ledge jump, Desert Eagle / Gatling | unchanged from revisions 4-6 |

Not a regression, measured: with fire held, a number key waits for the
weapon's idle pose (Gatling 33 / 34, Desert Eagle 16 / 17 never idle), the
same on the ground with no climb. A reload-length gap also appears when a clip
runs out (the save's Desert Eagle clip is 2 rounds).
