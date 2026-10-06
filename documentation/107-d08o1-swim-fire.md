# D08O1 - fire weapons while swimming

2026-10-06. **Accepted** (user: "i completely accept that this works!
mechanically, it does exactly what it's meant to, but the only issue is the
animation"; the downward-pointing weapon while swimming is D08O2). User report (2026-10-05, clarified
2026-10-06): "its when swimming underwater", "you cannot be in motion while
swimming and shooting", "you can shoot while still in water".

## Cause

The original swim states are `player+0x22c` 4 (surface) and 5 (underwater),
dispatched from `8004b594` to `800455bc` with jump table `80013ed8` (indexed
by anim - 122). The underwater handlers test the Cross action word
`*(800d1b50[player+0x233])` (fire) before Square (thrust):

| Handler | Fire held, weapon out | Effect |
| --- | --- | --- |
| idle 127 (`8004653c`) | `equip == 2` branch at `80046748` sets `+0x2ba = 0x400` and stays | the Square thrust branch (`8007c788`, anim 128) is never reached |
| thrust 128-130 (`80046994`) | `equip == 2 && !(+0x224 & 4) && Cross & 1 && !(+0x224 & 0x241)` skips the Square test | `s0` stays set, anim 127 at `80046bb0` |

So the original stops Duke to shoot underwater; it keeps firing through the
upper-body track (upper anims 9/10 with the pistol) while he floats in 127.
On the surface (125/126/123/124) firing and swimming already ran together.
Baseline in the private level 6 lab: W 2892 units in 90 frames; W, A, S,
Space or Ctrl with fire held 0 units, anim 127. Modernized view aiming also
did not adapt swim shots (`shot_ready()` admitted only the ground and jetpack
leases), and weapon selection never ran in water (its hook, `80058120` from
`80041c3c`, is on the ground path only).

## Change (Modernized only)

- `swim.inc` / `modern_controls.cpp`: new entry hooks `0x800455bc` and
  `0x80055e80` (`game.local.toml`, regenerated). At the swim handler entry
  (`a0 == player`, `ra == 0x8004b5bc`), when the swim thrust lease is live,
  fire is held with a weapon out (`+0x3b8 == 2`), holster is not held and a
  swim direction (WASD, Space or Ctrl) is held, bit 0 of the Cross word is
  cleared for that handler only. It is put back at `0x80055e80` (callback
  table `800c2754[3]`, which `8005a210` always calls right after the state
  callback), `0x800493a4` (animation start) or the camera update
  `0x8003ade4`, whichever runs first, and only if the word still holds our
  write. Other hooks must not restore it: the handler calls `0x80076330`
  before its fire test. The weapon keeps seeing fire; the original thrust
  carries Duke where `swim_steer_underwater` points him.
- `swim_weapon_ready()` (surface or underwater state under the swim lease)
  joins `shot_ready()` (shot adaptation, beam completion, reticle) and
  `view_aim_input_ready()` (weapon presentation), as D08Q3 did for flight.
- Weapon selection runs from the swim handler entry. In water only weapon
  actions are taken (number groups, previous/next, last); items and the quick
  kick keep the ground lease. The original completes a switch in water only
  for weapons whose table record `800c4590 + 44 * id` has flag `0x04`
  (3 Crossbow, 4 Desert Eagle, 5 Combat Shotgun, 6 Buffalo Rifle, 7 / 28
  Gatling, 12 Pipe Bomb). Raw requests showed it remaps the others (1/2 -> 3,
  8-11 -> 7, 13 -> 12) or leaves 14 pending with `+0x224` bit 4 set, so
  water choices skip them.
- Guards: `800455bc` (7216 bytes), `80013ed8` (48), `800c2754` (16),
  `80055e80` (1364). The codegen hash is unchanged (`0x8bab543c`); existing
  savestates load.
- Vanilla: no change (all of the above is behind `input_modernized()`).

## Verification

Private lab: `recomp/analysis/d08o1-20261006/` (ignored), Xvfb display 96,
debug port 9351, private profile copy at 1x scale, private card/savestate
copies; test audio moved to a null PulseAudio sink per process. Player card
and savestate SHA-256 values match the intake manifest.

- Reproduction save (UI slot 2, file 01, SHA-256 verified) is in
  OBEY OR DIE where the moat near the start is not swimmable water (no
  `+0x834` surface). Deep water was reached in level 6 FAMILY JEWELS (console
  `level 6`, turn, run into the water) and saved in private slots 9 and 11
  (11 after `dnstuff` on land).
- Underwater, 90 frames with fire held: W 2788 units (thrust 128), A 2826,
  Ctrl 1271, Space 1135 then surfaces, fire alone 0 (original idle firing);
  adapted shots 9-10 per run; reticle eligible throughout. Shots converge on
  the crosshair surface (target constant while the muzzle moves with the
  stroke).
- Weapons underwater, W + fire: Desert Eagle, Combat Shotgun, Gatling Gun,
  Pipe Bomb, Buffalo Rifle, Crossbow all keep thrust 128 with their own upper
  firing animations, 1558-1683 units, adapted shots counted.
- Underwater keys 1-0, apostrophe, semicolon and the wheel select only the
  allowed weapons; others leave the weapon unchanged.
- Ctrl + fire on the surface dives (133, state 5); Space + fire surfaces
  (122, state 4). Surface W + fire 2638 units (already original).
- Holstering underwater works; holstered fire stays original (no draw).
- Regression: ground W + fire moves with adapted shots; Modern jetpack flight
  W + fire moves with adapted shots; Vanilla underwater Up + Cross still
  stops Duke (0 units, no hides, no adaptation).
- Suites: ttk-aim-test (new D08O1 swim case), ttk-controls-test (with the
  `pc-input/d08-camera-final` LEVEL00 fixture, LEVEL01 and all levels),
  ttk-input-test, ttk-near-test, ttk-inventory-test, ttk-font-test and the
  Python unittest suite (112, 2 skipped) pass. The older `d08p-turret`
  fixture fails an apartment-patch assertion unrelated to this change.

Build SHA-256:
`711a313d983f78b1929431f68a0443fcf0f5a70fb3e2a424444d9d5515e12eb7`.

## Limits

- Only level 6 was driven live; level 12 BLOOD BATHS (also deep water) was
  not reached by the scripted route. The acceptance asks for two levels and
  the user's report location.
- While swimming and firing Duke's body faces the swim direction (the
  original thrust only moves along body yaw and pitch); shots and arms follow
  the view. Strafing or backing while firing therefore looks sideways or
  backwards in third person.
- First person in water is D11E; the view still falls back to third person.
- Holstered fire does not draw the weapon in water (original).
- Functional Xvfb runs are not feel, frame-rate or audio measurements.
