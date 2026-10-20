# D07F - Airborne shots follow the crosshair

Job: [D07F](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "confirmed working").
Found by the user while testing [D19G](159-d19g-options-stage-2.md).

## Problem

Fire while jumping, looking down, and the rocket or shotgun blast lands ahead
of Duke instead of at the crosshair.

The view-aim adapter (`0x8003c500`, [note 21](21-modern-weapon-aiming.md))
accepts a shot only while `shot_ready()` holds: the ground movement lease,
the jetpack lease or the swim lease. A jump holds none of those. From takeoff
to landing only the camera-only lease is live (mouse look, the D22B eye-air
and flight states), so every airborne shot was left to the original aim.

Measured before the fix (level 1 save slot 9, manual jump, pitch -35 degrees,
private profile, offscreen on the real GPU):

| Shot | Adapter | Lease miss | Rocket explosion |
| --- | --- | --- | --- |
| RPG, standing | view | - | `23081,-128,-12161` (crosshair `23082,0,-12160`) |
| RPG, mid-jump | original | `state a=97 9/9 f=00000100 camera=1` | `39808,-1397,-13212`, about 16700 units ahead |
| Shotgun, mid-jump | original | same | - |

Both shots came through the gun caller `0x8004fa88` with the normal stack
layout. Only the lease refused them.

## Fix

`recomp/src/ttk/modern_controls.cpp`: `airborne_shot_ready()` accepts the
camera-only lease (`lease_ready(true)`, so the same identity, epoch and D07E
frame-window checks apply) when Duke is airborne: mode 9 (owned or unowned
jump and fall, the airborne roll launch), or mode 0 on the takeoff (96/97)
and the first landing frame (previous mode 9). It refuses the jetpack
(it has its own lease), pushes, mantles and hangs, ladder mounts and
attached climbing.

`recomp/src/ttk/weapon_aim.cpp`: `shot_ready()` adds it, so the gun and beam
paths and the crosshair use it. The shot is the D07C one: physical muzzle,
camera ray, cover checks, original projectile. Nothing about movement,
facing or locomotion writes is widened.

## Verification

Executable `38076cd85f3e0b686cd04d25e8ad6db2c6c2d213d695c775e0c57b8ce0e120ef`.
Lab: `recomp/analysis/d07f-air/air.py` (private profile and card copies,
`dnweapons` / `dnammo`, port 9381; `DELAY` is frames from the jump press to
the shot).

| Case | Shots | Misses | Result |
| --- | --- | --- | --- |
| RPG, manual jump, standing and running, fire 2 / 10 / 20 frames in | 6 | 0 | explosions at the crosshair point (within 2 units) |
| Shotgun, same 6 cases | 6 | 0 | pellets land around the crosshair point (original spread, about 60 units) |
| RPG, first person | 2 | 0 | explosions at the crosshair point |
| RPG and shotgun, assisted jump | 4 | 0 | explosions at the crosshair point (RPG) |
| RPG aimed up (+20 degrees) | 2 | 0 | view-aimed (the target is far wall or sky) |
| Pistol | 2 | 0 | view-aimed |
| RPG standing (regression) | 1 | 0 | `23087,-128,-12160`, same spot as the baseline |

Tests: `ttk-aim-test` new D07F group (pistol, shotgun, RPG; pitch down and up
on the airborne lease alone; refused without it); `ttk-controls-test` (D22B
jump group: takeoff, flight and the landing frame are airborne shots; idle,
the plain landing pose, a stale gap and the ladder are not; all levels pass,
48 groups); `ttk-input-test`, `ttk-near-test`, `ttk-live-env-test`,
`ttk-font-test`, `ttk-inventory-test`; Python 153 OK (2 skipped);
`check_repo.py` OK. Vanilla is unchanged by construction: the adapter
requires Modernized input and `view` weapon aiming. No Vanilla run was made.

## Limits

- Only level 1, save slot 9, a flat floor. No moving targets, no thrown
  weapons (dynamite) or energy beam in the air.
- Unowned falls (walking off a ledge) are admitted by the same rule but were
  not fired from in the lab.
- Duke's arm pose in the air is unchanged: the shot follows the crosshair even
  where the jump animation does not point the gun there.
- Not verified on the user's display or by feel.
