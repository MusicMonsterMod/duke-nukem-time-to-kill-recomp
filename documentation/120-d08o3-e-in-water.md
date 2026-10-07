# D08O3 - E in water keeps the weapon

2026-10-07. **Accepted** (user: "fully passed my playtest. feels great to play"). User report after accepting D08O2A: "pressing
E while swimming holstered my weapon and i had to press one of the numbers to
draw it. I dont want that to happen." Agreed behaviour (user's choice of three
options): E keeps stowing first, so it can still climb out at a ledge; if no
climb-out starts, the weapon is drawn again automatically.

## Original facts

- E (Modernized) taps the original Circle to stow the weapon, then sends an
  unarmed Cross (`pc_input.cpp`), and later redraws through Circle once
  `interaction_restore_ready()` holds. That needs the ground movement lease,
  so in water the redraw never came.
- Climbing out of water is the surface handler's unarmed Cross plus the ledge
  probe `0x8007be9c(player, 4)` (climb-out anims 134-142, mode 8). Armed
  Cross at the same ledge fires and does not climb (lab, level 6), and W
  alone never climbs. So E must stow before Cross.
- The ledge probe writes Duke's contact fields (`+0x174/+0x178/+0x198`) and
  walks object lists, so it was not used as a host-side query.
- Underwater idle 127 starts thrust while Square is held, `+0x224 & 0x241`
  is clear and `0x8007c788` finds about 425 units clear along the body
  direction; a running stroke does not recheck. A Circle draw during a
  stroke could drop Duke to idle near geometry, and he then stayed there
  with W held (3 of 4 lab runs at one spot). A number-key switch, which is
  the original weapon request (`+0x3ba`, `+0x224 |= 4`), does not interrupt
  the stroke.

## Change (Modernized only)

- `swim.inc` `interaction_swim_restore_ready()`: the native swim states
  (surface 4, underwater 5) under the swim lease, weapon stowed (`+0x3b8 ==
  0`), no weapon request pending, upper animation settled (equal to the
  body animation), the same settled test as on land.
- `pc_input.cpp`: after an E stow, when that holds for 6 frames (the land
  settle), the redraw is queued as a one-shot request (30 frames) instead of
  a Circle tap; land keeps the Circle redraw.
- `shortcuts.inc` `select_weapon` (swim handler entry `0x800455bc`): takes
  the request and writes the weapon E stowed (`equipped`, only if usable
  and allowed underwater) to `+0x3ba` with `+0x224 |= 4`, as the number
  keys do in water. Counter `swim_redraws`.
- A climb-out leaves the swim states at once, so the existing land redraw
  still draws after it.
- Native test harnesses gained stubs for the two new functions.

## Verification

Private lab `recomp/analysis/d08o2a-20261007/` (level 6, slot 11, 120 fps):

- Surface, three walls without a ledge: stow, then redrawn about half a
  second later (upper 6 -> 5, equipment 2).
- Surface ledge (direction 3): stow, climb-out 140 (mode 8), land, redraw.
- Underwater idle: stow, redraw.
- E while swimming (W x4, A x3, D, and shotgun / Gatling / crossbow with W):
  anim 128 continues through the stow and the redraw in all 11 runs; stops
  only where swimming without E also stops (walls; one A strafe path stops
  near 915-940 with or without E).
- E on land: unchanged (Circle stow and redraw, no `swim_redraws`).
- Vanilla underwater fire, ground and jetpack regression, D08O2A swim-fire
  measurement unchanged. ttk-controls-test, ttk-aim-test, ttk-input-test,
  ttk-near-test, Python unittest (121, 2 skipped) pass.

Build SHA-256:
`e75b50f156137fa2377e4643407b2efbd16f2e976fa4d697afcbceb228e2e554`.

## Limits

- Pressing E away from a ledge still shows a quick stow and draw (by design,
  the agreed option).
- A weapon not allowed underwater is not redrawn there (it could not have
  been stowed in water by E anyway).
- Only level 6 was driven; the user's medieval canal (UI slot 2) is the
  playtest.
