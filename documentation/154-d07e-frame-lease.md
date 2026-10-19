# D07E - Shots and movement keep the lease in slow scenes; original aiming faces the camera

Job: [D07E](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-19: "plays solidly ... both work as intended beautifully").
Follows [D22D](152-d22d-energy-beam.md#limitations-and-findings), where both problems were first seen.

## Problem 1: the lease lapsed whenever the game slowed down

Modern controls trust the player state only while a "lease" is fresh: the
identity-checked camera hook (`0x8003ade4`) must have run recently. Fresh used
to mean within 4 host input frames. Host input frames advance once per
emulated vblank (60 Hz). The game normally updates the camera every 3 (20
game fps), so 4 left one frame of slack.

Measured on the real GPU offscreen with a copy of the player's settings
(120 Hz, fast timing, 150% CPU, 4x), level 1 save slot 9:

| Scene | Vblanks per camera update |
| --- | --- |
| Idle, walking, pistol fire | 2.98-3.06 |
| Upgraded energy weapon firing, idle / walking | 4.55 / 4.92 |

Under the energy stream the camera ran 5 vblanks before the shot, and 34 of 107
shots (21 of 33 standing) skipped the view-aim adapter: they fired the
original path with the original auto-aim and energy lock. The pistol missed 1
of 88, a run-collision bump (animation 95, which only the camera-only lease
admits). Any slowdown below 15 game fps does the same, and the same fixed
window gated movement: `lease_ready` (movement, locomotion input, flight,
first-person draw), the jump-camera continuity, fresh movement intent for
takeoffs and `ground_frame` for the standing takeoff.

### Fix

`recomp/src/ttk/modern_controls.cpp`: the window is one game frame measured
in vblanks. Each camera update records its interval; the window is the
slowest of the last four intervals plus one, at least 4 and at most 10
(`frame_window_min`, `frame_window_cap`). At 20 fps it is 4, as before. An
interval over the cap (pause, menu, a scene without the camera) is not
remembered and resets the window to 4, so a stopped camera still lapses the
lease within 10 vblanks. Every former `<= 4` check above uses
`within_frame()`. Debug JSON: `controls.frame_window`.

After the fix: energy weapon 0 of 105 shots missed (window 7 while firing),
pistol 0 of 88; idle window 4, walking 4-5.

## Problem 2: `original` weapon aiming fired along a heading nobody updated

Modern movement redirects the original walk displacement but by design never
writes Duke's heading (D05), and no tank-turn buttons are generated. With
`view` aiming the D07A facing writes the heading from the camera each update.
With `original` aiming nothing did, so Duke kept the heading the level or save
gave him: he walked camera-relative while the original shot, auto-aim and
energy lock all followed that fixed heading (`+0x144` is computed from it). In
the lab Duke faced 93 degrees, walked toward a target at 2 degrees, and every
pistol shot went out at 93.

### Fix (user choice 2026-10-19: face the camera)

`facing_ready()`: with `original` weapon aiming the heading boundary
(`0x80058120`) writes the camera yaw too, under the same conditions as `view`
(independent camera, capture, lease, holstered or a supported weapon, not
while precision aim is held or `+0x224` bit 7 is set). The arm retargeting,
view shots and crosshair stay `view` only, so the shot itself is still the
original one: along Duke's (now camera) heading with the original auto-aim.

After the fix: the heading follows the camera, `+0x144` follows the heading,
and pistol shots leave at 3 and 19 degrees toward targets at 2 and 20. The
energy weapon formed the original lock (5 locked beams in 60 frames); D22D never
saw one in this mode.

## Diagnostics added

- Aim JSON `lease_misses` and `last_lease_miss` (reason: `state ...`, `gap=N
  window=W`, `released`, ...); `DNTTK_AIM_TRACE=1` prints
  `ttk-aim-lease-miss` for each supported player shot left to the original aim.
- `DNTTK_TEST_INPUT` also accepts `cheat=<word>` and `act=<action>` (a binding
  name such as `weapon_group_7`), applied once per change, because offscreen
  key events do not reach the cheat and weapon shortcuts.
- Lab: `recomp/analysis/d07e-lease/` (`lease.py`, `rate.py`, `origaim.py`,
  `origbeam.py`; private profile, card and savestate copies, port 9371).

## Verification

Executable `1b3f3283586656df529f440547327307144d17523fe12382bcd3d8b40f2502f5`.
`ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels; new D07E window
case; the `original` facing case replaces its old refusal), `ttk-aim-test`,
`ttk-input-test`, Python 146 OK (12 skipped), `check_repo.py` OK. Vanilla is
untouched by construction (every changed path requires Modernized input); no
Vanilla run was made.

## Limits

- Only level 1 (slot 9), pistol and energy weapon, mostly first person.
- Below about 6 game fps (10 vblanks) the lease still lapses by design.
- A one-frame run-collision bump still fires the original aim (the movement
  state excludes animations 94/95).
- Not verified: feel on the user's display, and whether the wider window
  changes the D23E / D17G stutter reports.
