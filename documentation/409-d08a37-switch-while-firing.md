# D08A37 - Weapon switching while firing or jumping

2026-11-07. Status: Done, user-accepted: "lock it in and close it out! its
working". Regression baseline executable
`9adb09035646ff7a2c2745e5ad07cf7f3394cd796b458f4a77f8e853d9c55d79`. No
framework edit; the runtime patch export is unchanged. No new hook or address.

## Report

U (2026-11-07, D08Z8 closeout): "weapon switching while firing", "and weapon
switching while jumping", "i cant seem to switch specifically to the desert
eagle". A number key, the wheel or ; / ' pressed while fire is held did
nothing until fire was released and the weapon settled; a key pressed while
the weapon was not ready (landing, draw, fire track) was dropped.

Decided with the user before the change: **finish the shot** (the current
round leaves, then the switch starts at once; no lost or doubled shot), and
**keep firing** (held fire continues with the new weapon once it is ready,
EDuke32 style).

## Cause

S: `select_weapon` (`recomp/src/ttk/shortcuts.inc`) refused every weapon
command with fire held, outside the stable rest poses (5, 18, 19, 20, 29, 35,
39) or with a transition pending, and consumed the command. Held fire keeps
every weapon on its fire tracks (Desert Eagle 16, Gatling 34, shotgun 25/26),
so the command was always lost. The same on the ground, in the air (D08A36)
and swimming.

## Change (Modernized only)

- **Queue.** Weapon commands (groups, previous/next, last) resolve to a
  target at once (the cursor is the waiting or drawing target, so a group key
  pressed twice cycles) and wait up to 1.5 s for the weapon to be ready. A
  new choice replaces the waiting one, also during a draw (the original then
  draws the new one). The key of the weapon already out drops the waiting
  choice and does not touch the burst. E (interact) or a holster key cancels;
  a savestate load or Vanilla clears it. Items, the kick and the gadget keys
  keep their old path and gates.
- **Fire release.** While a choice waits (with a weapons lease: ground,
  air, swim, flight) and until the new weapon reaches its ready loop,
  `switch_fire_masked()` releases the game's fire pad (`pc_input.cpp`, after
  the D10H4 latch), and WEAPON FEEL's held-fire pacing, the traversal fire
  drive and the D08E fire draw see fire as released. The original's own
  release then finishes the shot: the Gatling's loop handle `+0x260` is
  stopped when it leaves its fire track (34 -> 32), measured below.
- **Finish the shot, not the lowering.** Once the fire track has ended, the
  post-shot tracks are cut to the rest pose (`+0x74` = rest, `+0x7c/+0x7e` = 0,
  the original fire request bits `+0x224 & 0x110` cleared): Desert Eagle 14 ->
  5, Gatling / Freezer 32 -> 29, shotgun pump 26 and 23 -> 20. The request
  (`+0x3ba`, flag 4) is written from rest, so the original plays its own
  holster and draw. Other weapons' tracks are not cut; they reach rest
  through their original tracks (the crossbow group's 23/24 take about
  0.6 s). Reloads (DE 17) are never cut.
- **Served.** The request needs the lease, equipment 0/2 or the remote, no
  held original aim, no `+0x224 & 0x2cd`, a rest pose, and with fire held at
  least two masked updates (so the game has seen the release). Dynamite out
  is never masked (its charged throw is on release); the queue waits for its
  rest.
- **Done.** When flag 4 is spent and any weapon is in its ready loop (the
  choice, or the original's remap of it in water) the mask ends; 3 s at most,
  or when the lease is lost (a mantle, a climb).

## Verification

L, private offscreen runs (`recomp/analysis/d08a37/`, copies of the d36-audit
cards, slot 8, `dnstuff`, 60 setting, port 9497). Key at time K, fire held
throughout unless noted:

| Case | Holster starts | New weapon fires | Notes |
| --- | --- | --- | --- |
| Desert Eagle -> shotgun (3) | K+0.25 s (cut 14) | K+1.44 s | before the cut: K+0.67 s |
| Gatling -> Desert Eagle (2) | K+0.18 s | K+1.11 s | `+0x260` 0 at the switch, no spin flag |
| Shotgun (in its pump) -> DE | K+0.14 s (cut 26) | K+1.2 s | before: K+1.08 s |
| RPG, Energy Weapon, Buffalo Rifle, Freezer -> DE | K+0.12 to 0.38 s | yes | |
| Crossbow group (9) -> DE | K+0.77 s | yes | not cut (23/24) |
| Remote detonator (6) -> DE | K+0.5 s | yes | |
| Jump, shotgun -> DE (mode 9) | K+0.14 s | in the air (one-arm 9/10) | D08A36 2x |
| 3 then 4 during a draw, no fire | | Gatling drawn | the later choice wins |
| Fire released after the key | | no fire | switch completes |
| 2 with the DE firing | none | burst goes on | no mask |
| Wheel down with the DE firing | K+0.28 s | pistol fires | |

- Vanilla run (same cards): no queue, no mask (counters 0).
- Native `ttk-controls-test`: new D08A37 group (held fire queues and masks,
  served after the release, gun-up 14 cut to 5 with 0x110 cleared, the fire
  track 16 not cut, mask through the draw until the ready loop, same weapon no
  mask, savestate drop, Vanilla). The D08A29 "rejected notch is not deferred"
  and draw-pose expectations now read "served when ready" (this job). The
  suite stops at the D14 widescreen HUD assertion (line 1751) on the clean
  baseline too (pre-existing, not this job); with that block skipped for one
  local run every group passed, then the block was restored. `ttk-input-test`
  (stub for `switch_fire_masked`) and `ttk-aim-test` PASS.
- The player's settings, saves and cards were not used.

## Limits

- Swimming and jetpack flight use the same code path (their leases), but were
  not run live.
- The "can't switch to the Desert Eagle" report never reproduced on its own
  (D08Z8 closeout); every case above switched to it. If it remains, the
  user's exact situation is needed.
- The cut to rest is a one-frame pose change before the original holster; its
  look is for the playtest.
- The holster and draw keep their original ground speed (1.5x stow/draw);
  a whole switch is still about 0.9 to 1.2 s from the key to the first new
  round.
