# D10F2 - Third-person camera through door kicks and other use actions

2026-11-05. Status: Done (user-accepted 2026-11-05). Dependencies: D10F1
([note 380](380-d10f1-clear-lens.md)). Executable:
`a2fa88caa9f6d07815ee4bab2a93327c6fb4c5aea4b11d0c16771fc4f2326dbe`.

## Report

User (2026-11-04), UI slot 11 (runtime 10): "when you open the door, duke
exhibits a problem there where the camera just goes behind something and looks
dumb".

## Cause (measured)

Private copy of the user's 2026-11-04 slot (runtime 10, saved in front of a
large double door), offscreen at 60 Hz, E pressed once:

- E starts the native hinged-door use, pose 116 (mode 0/0, `+0x224` bit 0x40),
  for about 0.9 s.
- The camera-only presentation lease (`state(true,true)`) accepted the D35J/K
  transient actions (use 116/118/266/267/268/270/275/277/279, mirror 278, idle
  fidgets 64-69) only for the first-person eye. In third person it refused
  them, so for the whole kick the orbit, mouse look and the D10F1 clear lens
  stopped (the `orbits` and clear-lens `updates` counters froze at 48) and the
  native camera took over.
- The native camera for that action swung out from the clear lens's 1537 units
  to about 2415 from the pivot, inside the wall behind Duke (the frame showed
  flat untextured colour), then eased back after the kick ended.

It was not a door actor passed over by the crate-fade rule, nor the door
moving into the probed ray: the clear lens was not running at all.

## Change

`recomp/src/ttk/modern_controls.cpp`, `state()`: the transient presentation
lease no longer requires the first-person view. The gate is still
`presentation && camera_only` (camera presentation only): movement, aim and
fire leases, the native action, its alignment, effects and timing are
unchanged, and the ownership, script-inhibit (0x20000000/0x200) and death
guards still apply. Vanilla is gated out by `input_modern_controls()` as
before; the Original camera keeps the native solve (`independent_camera()`).
Game-wide: every use, mirror and fidget pose in the list, not this door only.

## Evidence (private copies only; player files hash-checked by every run)

Local tools: `recomp/analysis/d10f2/` (`door.py`, `orbit_use.py`, `idle.py`,
`fire_use.py`, `still.py`, `shots/`).

| Case | Before | After |
| --- | --- | --- |
| E at the slot 11 door, lens distance from pivot during 116 (60 Hz) | 1537 -> 2415, inside the wall, orbit frozen | 1518-1569, orbit and clear lens updating |
| Same at 120 Hz and 180 Hz | - | 1518-1569, darkest frame 18% (the door itself) |
| Mouse orbit during 116 | no camera turn | camera turns 265 -> 290 degrees; Duke heading fixed at 2936, native position track identical to the unmodified run |
| Armed kick, fire held after E (D08J8 door fire) | - | 1 kick, 10 rounds, lens 1517-1555 |
| Idle fidgets 65, 66, 67, 69 (Shift held, about 60 s) | - | orbit and clear lens update through each, lens 1535-1538 |

Native `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels): 153 PASS
default, 152 with `DNTTK_CLEAR_LENS=0` and with `DNTTK_TIGHT_CAMERA=1` (the
D10F1/D10B camera group is swapped, as before). New case in the D35J/M
transient group: every use, mirror and fidget pose in third person keeps the
camera update (`cameras` advances) while movement and locomotion stay
refused and the native pose and flags are untouched; it fails against the
old gate. `ttk-aim-test` passes. `ttk-near-test` does not link
(`cowboy_hat_mesh` undefined in `near_clip.cpp`); this change does not
touch that file. No framework edit; the runtime patch export is unchanged.

## Limits

- Only the slot 11 door was run in game; the other use poses (switches,
  wheel, statue, toilet, mirror) are covered by the native test, not by
  in-game runs.
- Scripted cameras (`camera+0xa4` not Duke) are still the original's, by
  design.
- Needs the user's playtest at the slot 11 door on the real 180 Hz display.

## Acceptance (2026-11-05)

User: "honestly incredible work. lock it in and close it out." Done as implemented; the limits above
stay recorded.
