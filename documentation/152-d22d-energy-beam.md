# D22D - Energy weapon damages what the crosshair is on

**Done (user-accepted 2026-10-19).**

User report, 2026-10-08: "The energy weapon in slot 7 does not work. It fires
but it has no effect on enemies." (Key 7, weapon slot 10, projectile type 15;
the upgrade, SUPER ZAPPER, upgrades in place.)

## Cause

A Modernized regression from D07C, not the original's behaviour.

The original beam is a lock-on weapon:

| Address | Role |
| --- | --- |
| `0x80073220` | Beam constructor. Calls the acquisition, then makes one beam per target (`s4` beams, `s1` index) and stores the target in projectile `+0x14`, origin pointer `player+0x80C` in `+0x18`. After each projectile it calls the list advance `0x8001CA4C` (ra `0x80073380`). |
| `0x800587C8` | Acquisition. Actors in a room's targetable list (room table `*0x800DA454`, 0x58 bytes per room, count `*0x800DA47C`, list head `+0x4C`, node `+4` next / `+8` actor) in Duke's room and its neighbours; distance squared from `player+4` below `*0x800C3B58` (36000000, a reach of 6000); clear world line from `player+0xDC` to the actor's `+0xEC`; angle to Duke's aim vector `player+0x144` at most 0x80 (11.25 degrees), or 0x200 (45 degrees) with the upgrade (bit 8 of the weapon's inventory word). Up to 3 targets, nearest first, kept in `player+0x858`. Without the upgrade only one beam is made. |
| `0x800733C8` | Beam updater. With a target in `+0x14` the beam ends on it and, when no wall (`0x80060980`) or other actor (`0x80076984`) is in between, calls the target type's handler from the table `0x800C5ECC` (indexed by the actor's `+0x16` type) with the weapon table damage. **Without a target it travels 6000 straight ahead and never calls a damage handler.** |

D07C's beam completion (`beam_finish` at the list advance) wrote 0 to `+0x14`
on every Modernized view-aimed beam to "detach the implicit lock", so every
beam was a no-target beam: visible, never damaging.

Evidence (slot 9 copy, level 1, the same pig cop `0x801CCB90`, about 2400 away):

| Run | Locks in `+0x14` | Enemy |
| --- | --- | --- |
| Vanilla, original controls and selector | 5 of 6 beams on the enemy | flags `0x1` -> `0x803` (dead bit 2), hit sparks |
| Modernized view aiming, baseline `a1fea19a...` | 16 of 18 beams with none; the crosshair ray hit the enemy (`hit_kind 2`) | flags stay `0x1` |

## Fix (`recomp/src/ttk/weapon_aim.cpp`)

- The shot adapter (`0x8003C500`, continuous weapon 10) now keeps the actor
  its crosshair ray hit (`trace(..., &view_actor)`), or the assisted original
  lock when `original-lock` assistance passed its two cover checks.
- `beam_finish`, first beam (`s1` 0): `+0x14` = that actor when it is
  targetable (in its room's `+0x4C` list, not dead) and within the original
  reach (`*0x800C3B58`); otherwise 0, a miss. No auto-aim: the original's own
  acquisition no longer chooses the first beam.
- Upgrade (`s4` > 1 acquired targets): the extra beams keep the original's
  other locks when they are targetable, in reach and within the upgrade's own
  45 degrees, measured round the view instead of Duke's aim vector. An extra
  lock on the crosshair actor takes the displaced original first lock, so one
  shot never hits an actor twice. Without the upgrade only the one beam is made
  (as before).
- The original updater still owns damage, cover, lifetime and visuals.
- The completion guard no longer compares the host input `sequence` with the
  shot's. It advances once per presented host frame on the main thread, and
  the beam completes on the emulation thread inside the same guest call as
  the shot, so the comparison raced: up to 8 of 13 beams kept the original's
  auto-lock. Stack frame, return address, owner, type, origin pointer and the
  capture `epoch` still prove the constructor call; every shot resets the
  pending state.
- Debug JSON `ttk_input` -> `controls` -> `aim`: `beam_locks`,
  `beam_extra_locks`. `DNTTK_AIM_TRACE=1` logs `ttk-view-beam ... index count
  lock original`.

No new hooks or guards; Vanilla and `original` weapon aiming are untouched.

## Verification

Executable `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98`.
Isolated lab `recomp/analysis/d22d-energy/` (private profiles and cards,
savestate copies, Xvfb, muted sink, `dnstuff` + `dnkroz`, key 7, closed-loop
mouse aim at the enemy's `+0xEC`, hold left mouse):

| Case | Result |
| --- | --- |
| Level 1 (slot 9), pig cop, first person | 5 crosshair locks, dead at fire frame 11-13; 21 of 21 beams completed |
| Same, crosshair 8 degrees off | no locks, enemy unharmed (the original's 11-degree lock would have hit) |
| Level 1, second pig cop `0x801CE788` | 5 locks, dead |
| Level 0 (slot 10), type 4 enemy, first person | 1 lock, dead |
| Third person, level 1 and level 0 | 5 locks dead at frame 13; 1 lock dead |
| Enemy behind the floor (slot 4) | crosshair ray hits the world, no lock (correct) |
| Upgraded, crosshair 12 degrees off two enemies | first beam no lock; extra beam kept the second enemy (6 extra locks) |
| Vanilla on the new build | 5 original locks, dead at frame 23 |

Suites: `ttk-aim-test` (links again: the missing D19J host-call stubs added;
new D22D case: crosshair lock, foreign list advance, consumed lease, miss,
dead, out of reach, not targetable, upgrade extras, cone, no double hit, no
fourth beam), `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels),
`ttk-input-test`, Python 146 OK (12 skipped).

## Limitations and findings

- The damage handler call itself was not traced (the debug `fn_*` trace
  records no entries for it); damage is shown by the actor's dead bit and the
  captures.
- Only levels 0 and 1, two enemy types (3 and 4). Bosses and other types
  rely on the same original handler table.
- Shots taken while the movement lease is stale (`lease_ready` needs the
  camera hook within 4 host input frames) skip the adapter and fire the
  original beam with its auto-lock. Seen in the offscreen lab (11 of 28 shots
  in one upgraded run, Duke idle, animation 63). It applies to every weapon;
  new job D07E.
- Modernized with `original` weapon aiming: no original lock was ever made
  in the lab. Duke's aim vector `player+0x144` still pointed the old way after
  modern movement turned him (152 degrees from the enemy), and the original
  acquisition measures its cone round it. Recorded under D07E.
