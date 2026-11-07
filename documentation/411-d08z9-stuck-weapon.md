# D08Z9 - Stuck weapon: Duke cannot shoot or change weapons

2026-11-07. Status: Done, user-accepted: "it's all working!! lock it in and close it out". Regression baseline below. Modernized only; Vanilla unchanged. No
framework edit; the runtime patch export is unchanged. Executable
`94bfe4480d9f4a466c848b8fefbb1abb6a6805cb09db49377400a7dc585d75f9`.

## Report

User (2026-11-07, UI slot 7): "i've just encountered that strange bug where
i cant shoot or change weapons again. can we please prevent this from
happening."

## The state (measured, private copy of UI slot 7 = runtime slot 6)

- Equipment `+0x3b8` = 2, weapon `+0x3b9` = 4 (Desert Eagle, 45 rounds), no
  flags, no use in progress, on the ground.
- The upper track `+0x74` held the legs' own clip: 63 (the idle), the whole
  block equal to the lower one at `+0x60` (frame counters included). 63 has no
  weapon bit (upper table `0x800c2824` bit 8: `0x18252`).
- That is a holstered Duke's free upper body under a drawn weapon. Walking,
  the upper followed the legs (72, 74, 76, 78, back to 63) with equipment 2.
  The weapon state handlers only run from weapon-owned poses, so held fire
  spent no ammo, key 3 queued (D08A37) and expired after 1.5 s, and nothing
  in the original ever takes the state back. A savestate keeps it.
- The first-person view still showed the Desert Eagle in the hand.
- The user's UI slot 8 (saved 2 minutes earlier, Desert Eagle 161 rounds) was
  healthy. The session log shows no climb or mantle between the two saves.

## Cause (reproduced)

The D08Z8 revision 2 sidearm loan (`recomp/src/ttk/sidearm_mantle.inc`).
With the Desert Eagle out on the ground, a new E asks whether the original
would mantle; at the obstacle action `0x80051cf0` (walking into something)
Duke is lent to the original as a holstered Duke: equipment 0, the upper id
set to the lower clip. The close (fire request entry, or the next update)
put equipment 2 back but the upper id only when it still equalled the lower
clip saved at the loan.

When the walk handler changed the lower clip inside the loan (walking into
the wall: 74 to the stop 63), the lent free upper body copied it. The upper
was then 63, not 74, so nothing was restored: equipment 2 over an upper on
the legs, the stuck state.

Reproduced on the private copy of UI slot 7 (pose first repaired by a debug
write): A (strafe) into the wall beside Duke, E while still pushing. Two
loans, both "no mantle"; after them equipment 2 and upper 63; held fire
spent no ammo and key 3 did nothing. Reproduced every time.

## Change

1. **The loan returns the whole weapon block.** At the loan the whole upper
   block (`+0x74`, 0x14 bytes) is kept. A close without a mantle writes it
   back, so whatever the original did to the lent upper is undone. A mantle
   close is unchanged (the upper id, then the D08Z2 hand-off).
2. **Stuck-weapon guard** (`recomp/src/ttk/weapon_pose_guard.inc`, once per
   player update after the D08Z8 update). On settled ground (modes 0/0), with
   equipment 2 and a weapon 1-14, the upper on the lower's own clip, a land
   gait clip (63, 72-79, 178-184) and no weapon bit, no use (`+0x27d`), none of
   `+0x224 & 0x2cd`, alive, and no host hand-off open (D08Z2 mantle, D08J7 hang
   drop, D08Z8 loan or climb, D08J9 door window): after 6 player updates in a
   row (about 0.2 s) Duke is set holstered (equipment 0, the upper already
   free) and the same weapon is requested through the original's own draw
   (`+0x3ba`, `+0x224 |= 4`), as a number key draws from holstered. Each repair
   is logged:
   `[TTK weapon] weapon pose repaired (N): weapon W drawn with the upper on the legs' clip C; redrawn (...)`
   with the loan, hand-off and door counters, so a future case names its
   source. `ttk_input` reports `controls.weapon_pose` and
   `sidearm_mantle.loan_repairs`.

The guard also repairs saves already in the state: loading the user's UI
slot 7 redraws the Desert Eagle within about 0.2 s.

## Verification (private offscreen runs, `recomp/analysis/stuck-fire/`)

| Case | Before | After |
| --- | --- | --- |
| UI slot 7 copy, as saved | stuck (no shot, no switch) | repaired at load (log line), Desert Eagle fires |
| Wall strafe + E, Desert Eagle (3 runs) | stuck every time | `loan_repairs` +1 each, guard not needed; fires; 3 switches to the shotgun, also with fire held (D08A37) |
| Every weapon 1-14 (healthy UI slot 8 copy, god mode): fire tap, held fire, crouch, crouch fire, run, run-jump landing, walk firing | - | 0 repairs, no armed legs-pose sample |
| D08J9 door lab (UI slot 10 copy, 180 Hz, first person): Desert Eagle and shotgun EF, FE, E, WALK, walk + E | - | armed kick each time, never stuck; Desert Eagle shots 16 / 19 / 18 as before |
| D08Z8 lab (dumpster top, Desert Eagle): W+E, W+E with fire | - | unchanged (ladder leap; fire), 0 repairs |

- Native: `ttk-input-test` and `ttk-aim-test` pass. `ttk-controls-test`
  stops at the known D14 widescreen HUD assertion (line 1751, failing on the
  clean baseline too); with that block skipped for one local run every group
  passed (151), then the block was restored.
- The player's saves, cards and settings were not used; the slot files were
  copied into private test cards.

## Limits

- Not re-run: a walked or run-in sidearm mantle at a post (no fixture of the
  post remains; that code path is unchanged), swimming, jetpack flight,
  ladders and hangs with the guard.
- The user's own approach in the building was not recorded; the wall + E
  route is one confirmed way in, and the guard covers any other.
- A guard repair is a short visible redraw (about 0.3 s).
