# D08Q11 - Jetpack lifts off directly from a crouch

Status: Done (user-accepted 2026-11-02: "this job is now complete and I'm very
happy with the results so it's fully accepted"). Board: `MODERNIZATION_JOBS.md` D08Q11.

## Cause (confirmed, two parts)

Reproduced offscreen on the pre-fix build
(`e0714bcbd5b17717dca8c78688ef74e40f0dba42fdb9040bdfeef2c109d625e7`): from
a C toggle-crouch (animation 178, mode 0) J opened the pack (mode 10,
animation 163/164), but on the next updates Duke landed (105, then standing
63, mode 0) with the pack still open on the ground (`+0x358` 3). Only a later
Space lifted him. A held Ctrl crouch behaved the same. From standing, J lifted
off as expected.

1. **Crouch read as Descend.** The flight owner takes `held[crouch]` as
   Descend, and a descent ends the start lift (`jet_lift_active`). The C latch
   (and a held Ctrl) were still in the frame published for that update. The
   latch was dropped only on the next `input_frame()`, once
   `jetpack_input_ready()` was true.
2. **Kneeling root.** Duke's root `+8` is his pelvis: feet `+0x1c8` minus
   the pose's pelvis height. Kneeling (178) it is 282 above the feet,
   standing (63) about 515 (506-525 with the idle bob). The flight envelope's
   feet sit at root + 624, so a kneeling start put them 233 units into the
   floor and the native collision landed Duke on the first update.

Fixing (1) alone did not lift Duke (same landing). Both are needed.

## Change

- `recomp/src/ttk/jetpack.inc`, `jet_instant_toggle()`: a ground start
  (mode 0) from a crouch pose (176, 178-184) first checks D08K7's standing
  clearance (`crouch_stand_room`, 0x370 over the floor or a walkable
  object's top). Without standing room J is refused (pack closed, Duke keeps
  crouching, no notice). With room, after the pack opens the root rises to
  feet - 515 (`k_jet_stand_pelvis`) and that height is the lift's start, so
  the start lift and hover height match a standing start. Ceilings during the
  lift are handled by the existing D08Q8 flight clearance, as for a standing
  start.
- `recomp/src/ttk/pc_input.cpp`, `input_jetpack_started(bool ground)`: called
  by every successful start. It ends the C stance (also in the frame already
  published for this update). A ground start also ignores a crouch key held
  from before (Ctrl, or a pad crouch) until it is released, as a crouch jump
  does (`crouch_cancel_held`); the next press descends. Airborne starts keep a
  held Ctrl as Descend (unchanged).
- Classic (native jetpack) and Vanilla do not use this path
  (`jet_instant_ready` requires Modernized and not Classic).
- No framework change.

## Evidence (executable `8c9b31d2a33c8e613661625683e098d43be95e7ee4190a0b725ff75ded84612a`)

Probe `recomp/analysis/d08q11-crouch-jetpack/probe.py` (local, derived from
D26J's): private copy of `saves/local-play`, savestate slot 11 (level 0
alley), Xvfb, debug port, scripted keys (`DNTTK_TEST_INPUT`). Each case:
`dninventory`, then J, 90 sampled frames, Space climb, Ctrl descent, Ctrl to
the ground, J off, W walk. Player saves, cards and profile unchanged after
every run.

| Case (third person, 60 Hz) | Before | After |
| --- | --- | --- |
| Standing, J | lift, hover y -10247/-10279 | same (run-to-run spread about 40) |
| C crouch (178, y -9498), J | mode 10, then lands (105, 63), pack open on the ground | root to -9731, lift, hover -10291/-10323 |
| Ctrl held, J, Ctrl kept 90 frames | lands as above | lift, hover -10332/-10364 while Ctrl is held; Ctrl released holds height |

After the fix every case then climbed with Space, descended with Ctrl, flew
down to the low hover, switched off to standing 63 (the latch stays off) and
walked (72) normally.

First person at 180 Hz (`VIEW=first RATE=180`): C crouch and held Ctrl
lifted directly (hover about -10371), then climb, descent, landing to 63 and
walking as above. In that run the standing case's first J came on frame 45
right after the savestate load while the host was loaded (my build running,
emulation 14-17 fps) and did not register (Space then jumped); the standing
path is unchanged by this job, and a re-run on a quiet host (`first2`)
lifted from standing normally (hover about -10358).

The no-room branch (`first2`, first person): `crouchstand block` (debug, makes
the standing clearance refuse) then J kept Duke at 178, root -9498, pack
closed; `crouchstand free` and J lifted directly to about -10371.

Native: `ttk-input-test` new D08Q11 group (C stance cleared in the published
frame and after; held Ctrl ignored through key repeat until release, a fresh
press counts; airborne start keeps a held Ctrl). `ttk-controls-test`
(EDuke32 jetpack run) new D08Q11 group (kneeling root -1282 over feet -1000
rises to -1515 and lifts with no Space; refused with `crouchstand block`;
standing start root untouched). `ttk-controls-test` default run exit 0.

## Limits

- The base game has no crouch-only ceilings (D08K2's `crouchscan`), so a real
  low ceiling start was not played; the refusal was tested with the debug
  block and the native fixture. Under a ceiling that allows standing but is
  lower than the hover height, the lift stops at the ceiling through D08Q8
  (not separately re-tested here).
- The standing pelvis height is a measured constant (515), not read from the
  animation.
