# D47 - The first shotgun shot smokes at the barrel

2026-11-08. Status: Done (user-accepted 2026-11-08: "lock it in, close it out! good work"). User report (D41B playtest): "the first
shoot from the shotgun seems to 'fire' (smoke etc) from duke's left shoulder
(when viewed from first, and third person)". It had been happening "for a
good hot minute", before D41B.

Before/after (local, not committed): `research/before-after/d47-first-shot-smoke/`
(third person, 180 Hz, 15 frames from 40 to 250 ms after a press from rest;
first person 140 and 200 ms; `before` with `DNTTK_MUZZLE_SETTLE=0`).

## Cause

The shot's puff (effect type `0x12`) and D41B's muzzle light are placed at
the player's muzzle point `+0xbc..+0xc4`, the same point the rounds launch
from (`0x8003c690` copies it into the gun call's origin). That point is not
computed in the player update. It is written by `0x80032364` at the start of
Duke's actor draw `0x800348d8`: the muzzle joint (`+0xc8`) of the pose just
evaluated, plus the weapon's offset from the table at `0x800c45b0` (indexed by
`+0x22b`), with Duke's position as the fallback. The draw then calls
`0x80032bd8` (a0 the actor) unconditionally.

A game frame runs the composition (`0x80026164`), the render (object loop
`0x800374cc`, then effects `0x8009ce44`) and then the update. The gun call
`0x8003c500` (ra `0x8004f7b4` / `0x8004fa88`) runs in the update, so it reads
the muzzle the previous draw wrote. When the press finds the shotgun at rest
(port arms), that is the rest pose's muzzle: about 250 units up and 250 to
Duke's left, at the left shoulder. The next draw already poses the gun aimed
(the muzzle jumps from Duke-relative (-203, -257, -246) to (-374, -120, +68)
in one frame), so held shots, fired with the gun already up, are right. Only a
shot from rest is wrong, which is why the user saw it on "the first" shot.

Not a regression. It is the original code order (the recomp does not touch
`+0xbc`): Modernized with the original weapon timing (`DNTTK_WEAPON_FEEL=modern`)
shows the same, so D10H4's earlier shot did not cause it. A true `--vanilla`
run was launched (the correction reports disabled there) but the lab's test
input could not cycle Vanilla to the shotgun, so the glitch itself was not
watched in Vanilla. Every shotgun press from rest showed it, not only the first
after loading.

## Fix (Modernized)

`recomp/src/ttk/muzzle_settle.inc` (included by `modern_controls.cpp`):

- Gun call entry (live game): note the muzzle the shot sees and the effects
  alive at that moment, in a guest-memory block marked pending with a magic
  word.
- `0x80032bd8` entry from Duke's draw (ra `0x8003492c`, a0 the player): the
  muzzle has just been written for this frame. Effects that the shot added
  within 48 units of the old muzzle move by the difference. If one moved, so
  does Duke's muzzle light from that launch (age 0, same place). Effects are
  drawn after the object loop in the same render, so the live frame never
  shows the puff at the shoulder.
- The 120/180 Hz redraw workers copy the game at the composition, before that
  draw corrects the puff. The pending block is guest memory, so it travels in
  their copies, and each replayed Duke draw makes the same move in its own
  copy. The magic word makes a copy taken before the block existed do
  nothing.
- Only shots that left a puff at the stale muzzle are moved. In practice
  that is the Combat Shotgun. The Desert Eagle, Gatling and RPG leave no puff
  there, and the RPG's muzzle point switches tubes from frame to frame, so
  moving its light would have pulled it away from the rocket. Those guns are
  unchanged.
- Rounds keep their origin and direction (hits unchanged). Records are moved,
  never added or removed. Vanilla is unchanged (`input_modernized()`).
- Guards: `0x80032364` (0x73c bytes), `0x8003c500`, `0x800348d8`.
  `0x80032BD8` was added to `game.local.toml` `mod_function_entry_funcs` and
  regenerated: one generated file changed, codegen hash `0xebce51ea`
  unchanged, so saves still load.
- Developer A/B: `DNTTK_MUZZLE_SETTLE=0`. Trace: `DNTTK_MUZZLE_TRACE=1`.
  Debug port `ttk_input` `controls.muzzle_settle` counts shots, moves,
  effects, lights, expired and worker replays.

## Evidence

Lab `recomp/analysis/d47/lab.py` (local), private copy of UI slot 8
(internal 7), offscreen.

| Run | Puff, Duke-relative |
| --- | --- |
| Before, 60 Hz first and third person, three presses from rest | (-203, -257, -246) every press (left shoulder) |
| Before, original weapon timing | same |
| After, 60 Hz first and third person | (-374, -121, +68) every press (aimed muzzle) |
| After, 180 Hz third person, 15 frames 40-250 ms, two presses | puff and smoke at the barrel in every frame, none at the shoulder |
| After, 180 Hz first person | puff at the gun; before, a large puff left of the screen |

Weapons (press from rest, then 1.5 s held): Desert Eagle, Gatling and RPG
0 moves. Combat Shotgun: 1 move per press and 1 at the start of held fire,
none for the held shots after it. Vanilla: disabled, 0 shots handled.
`ttk-input-test` and `ttk-aim-test disc/SLUS_005.83` pass. `ttk-controls-test`
(LEVEL00 fixture) stops at the known D14 HUD check (`hud_intact`), as in D51.

## Limits

- The muzzle light is host state published with each image. At 120/180 Hz
  the redraws of the shot's first game frame still light from the old point
  (about 400 units away, radius 4400). From the next game frame they use the
  corrected light.
- In first person the puff is at the game's aimed muzzle, as later shots
  always were, not at the presentation gun's drawn barrel (D12/D36 pose). See
  D49 part 2 for that gap.
- The pellets still start from the old point, so hits are unchanged. Whether
  they draw anything in flight was not checked.
