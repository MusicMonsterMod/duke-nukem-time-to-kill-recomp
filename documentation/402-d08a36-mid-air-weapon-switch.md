# D08A36 - Weapon switching in mid air

2026-11-06. Done, user-accepted at revision 2: "confirmed working!! lock it
in, and close it out". Executable
`c1e6f6239caaac7bdd8421eef3c5d3d404cdaa12457ec1606eee0be8a962772b` is the
regression baseline. No framework edit; the runtime patch export is
unchanged.

## Report and cause

U (2026-11-05): "weapons cannot be changed in mid air". Load any save, jump,
and the number keys, the mouse wheel and ; / ' do nothing.

L, private research run (slot 8, `dnstuff`, shotgun drawn): writing the
ordinary request (`+0x3ba` = weapon, `+0x224 |= 4`) during a standing jump
(anim 97, mode 9) makes the original play its normal upper-body sequence over
the jump pose: holster 6, empty 1, draw 21, ready 20, all before the landing
(105). So the original accepts the change in the air, as it does in jetpack
flight (D08Q5).

S: our shortcut layer was the only block. `select_weapon` (`shortcuts.inc`)
needs `movement_ready()` (the ground lease) or one of the weapons-only
states (swimming, flight). A jump or fall holds only the camera-only lease,
so every weapon command was dropped.

## Change

`select_weapon` adds a third weapons-only state, `air_select`: not flight or
swimming, no ground lease, and `airborne_shot_ready()` (the D07F mid-air shot
lease: mode 9, or its takeoff/landing frames; push, mantle, ladder mount,
climb and jetpack fall excluded), except dodge rolls (157..162, which keep
their own ownership). There weapon groups, previous/next (wheel,
semicolon/apostrophe) and X take the same native request as on the ground.
Items, the quick kick, held fire/original aim/interact, pending transitions
(`+0x224 & 0x2c5`), non-ready upper-body poses and Vanilla stay refused, as
in flight. No new hook, address, guest write or framework change; codegen
hash unaffected.

## Verification

- Incremental local-dev player build OK. Executable SHA-256:
  `83389a8849f2bd98b365fd5bc5c5d4e634ee5b10ca6fad9863d829ba9a1c4c60`.
- `ttk-controls-test` (owned EXE, LEVEL00 fixture, LEVEL01 and all level
  overlays) PASS, with a new D08A36 group: anims 97/98/104/107/108 in mode 9
  and the 105 landing take a group key; wheel previous/next; refusals for the
  medkit, the kick, held fire, a draw pose (22), rolls 157/160 and Vanilla;
  the ground lease returns on landing. `ttk-input-test` and `ttk-aim-test`
  PASS. `ttk-near-test` does not link (missing `cowboy_hat_mesh` symbols in
  its target, unrelated to this job); not run.
- L, 60 setting, private offscreen runs (`recomp/analysis/d08a36/`, port
  9493, copies of the d36-audit cards, slot 8): with the shotgun drawn, a
  number key (3 = RPG) and wheel down (to the pistol) during a standing jump
  and during a Shift running jump each completed holster, draw and ready in
  the air (mode 9), before the landing.
- Vanilla run: Space is not Vanilla's jump, so it did not test the air; the
  key and wheel still did nothing on the ground. The new path needs
  `input_modernized()`, asserted in the native group.
- The player's settings, saves and cards were not used.

## Revision 2: the switch is ready in the air (2026-11-06)

U: "switching works, but you cant start shooting until you land!"

L, private runs: firing itself works in the air. A jump with no switch fires
the shotgun in mode 9 (ammo 200 -> 195 before the landing), and after a
switch every weapon starts its attack in mode 9 once its ready loop is
reached. The switch was the problem: holster 6, the generic blend 1 and the
draw took about 1.1 to 1.25 s of a 1.33 s standing jump (switch started
about 0.3 s in), so the new weapon was ready 0.1 to 0.2 s before landing,
inside the attack windup.

S: the upper-track hook (`0x80059db0` from `0x8005a5a8`) runs stow/draw
tracks 1.5x only under the ground lease (`movement_ready()`), so in the air
they ran 1x. Giving the air the ground's 1.5x only saved about 0.17 s; the
blend (1, about 0.36 s) is never boosted there.

Change: an air switch (the same air lease, rolls excluded) runs 2x on the
upper track: the stow/draw tracks and the generic blend 0/1 while a weapon
is requested (`+0x3b8` 0, `+0x3ba` nonzero). This is the D08J7B mantle-draw
rule (2x with the blend). Once per player update, original events intact;
the E stow's 4x still wins. Ground switches are unchanged.

L, after (60 setting, standing jump, shotgun to each group, fire held from the
ready loop): ready 0.82 to 0.92 s, landing about 1.5 s, and every one fired
in the air: RPG 50 -> 49, Energy Weapon 400 -> 394, Flamethrower 8 -> 6,
Gatling 200 -> 196, Buffalo Rifle 200 -> 196, Crossbow 20 -> 19, Freezer
200 -> 195. Before (1x): only Energy Weapon, Flamethrower and Freezer got a
round out. Shift running jumps: Energy Weapon 400 -> 394, RPG 50 -> 49 in the
air.

Native: the D08A36 group checks 2x for 6/7/21/22/30/31/36/40 and the blend
with a request, 1x for the blend without one, armed blends, ready/attack
tracks, rolls and Vanilla. The airborne E-stow group's "no E" expectation
now reads 2x (this change). Controls (154 groups), input and aim PASS.
Executable SHA-256:
`c1e6f6239caaac7bdd8421eef3c5d3d404cdaa12457ec1606eee0be8a962772b`.

## Limits

- Falls (stepping off a ledge, 107/108) are covered by the native group only,
  not by a live run.
- As on the ground, presses during an attack, a throw or an active stow/draw
  are refused rather than queued.
- A switch started late in a jump may still land before the draw ends; the
  draw then finishes on the ground (at 2x).
- 2x is a first measured value; faster is possible (4x left upper tracks
  stuck on jump poses for the E stow, see the hook comment).
- Automated checks only; the user's in-game feel is not confirmed.
