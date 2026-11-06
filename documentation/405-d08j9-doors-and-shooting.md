# D08J9 - Doors and shooting, rewritten

2026-11-06. Status: Done (user-accepted and closed 2026-11-06: "WORKING!!!!! well done"). Modernized only; Vanilla unchanged.

User, UI slot 10 (a door on TIME TO KILL): "open the door but try shooting at
the same time ... rapid fire happening from the shotgun. glitched out shots,
no shots, or we cant fire and open the door at the same time. kinda
unpredictable behaviour"; "we're supposed to be able to shoot completely
normally when opening doors"; test different weapons and the mouse held down;
"i want this to be really polished". Then: "instead of being hacky around
this, im totally opening to rewriting open-door code".

The D08J8C door code (`door_fire.inc`, [note 394](394-d08j8-shoot-through-traversal.md))
is replaced by `recomp/src/ttk/door_use.inc`.

## What was wrong (measured, private copy of UI slot 10, 180 Hz)

Private offscreen lab `recomp/analysis/d08j9/` (local only): `door.py` loads a
byte-identical copy of the player's UI slot 10 (runtime slot 9), selects a
weapon through the selection word, plays an input script and records Duke's
tracks, the use stage, shots (`ttk-launch`), health, and whether the door
opened (Duke walks through it afterwards). The player's saves, cards and
settings are hash-checked unchanged after every run.

| Case, D08J8 build | Result |
| --- | --- |
| Shotgun, E then fire | no round for about 1.6 s (the whole kick) |
| Shotgun, fire then E | one round, then 1.7 s without one |
| Pistol or shotgun, E alone | nothing happens: no kick |
| Pistol, E then fire | no kick, just fire |
| Pistol, quick taps after E | the door opens, no round for 1.7 s |

Causes:
- **E did nothing at this door.** The D08J8C probe ran only when the contact
  object `+0x174` was a door, but the finder `0x80077f28` writes `+0x174`
  only when the Action press runs. At slot 10 it was 0 until then.
- **Shotgun gaps.** The host drove each weapon's fire track through the kick
  and skipped "transition" upper tracks 21-23. Track 23 is not a stow. It is
  the shotgun's gun-up hold after the pump 26 (table `0x80c`, like 8, 14 and
  32), so the drive never ran.
- **Host-owned fire.** The kick's fire came from a host loop of the fire
  track, not the weapon's own cycle (pump, rate, reload). The E probe sent
  armed Cross through the pad, put back fire requests the probe raised, and
  cut Cross for an update while firing. Fire and E shared one button.

## The original (read from the executable)

- **Action press** `0x80051890`: the finder `0x80077f28`, then, on a fresh
  Cross (the action word `*(0x800d1b50[+0x233])` is a shift register: bit 0
  this tick, bit 1 the last) and none of `+0x224 & 0x241`, it starts a use:
  `+0x27c` mode, `+0x27d` stage 1, `+0x290` the object, `+0x224 |= 0x40`.
  With a weapon out, the lower idle handler still reaches it (`0x80052c14`).
  So an armed Cross at a door kicks it instead of firing.
- **The kick**: the selector `0x800513e4` installs 116 at stage 4 -> 5. The
  door opens on 116's strike key and stage 6/8 completes. The weapon stays
  drawn and its upper tracks keep running.
- **The weapon's gates** during the kick:
  - committed bit `0x40`, in every weapon state handler (`0x241` masks);
  - lower-clip table bit `0x20000` (`0x800c2824 + 4 * anim`), read only by
    the weapon state handlers `0x8004dc90..0x80051344`, for example the
    shotgun idle `0x8004f058` raising 24 on Cross, and by the target lock
    `0x80057f84`/`0x8005804c`;
  - lower-clip bit `0x2000`, read only by the fire request (`0x80041b58`,
    `0x80041bcc`, `0x80058134`).
  - The kick 116's entry is `0x22441`, so it carries both bits.
- **Update order** (`0x80041b2c..0x80041c44`): the animation runner
  `0x8005a210` (lower and upper handlers), the heading handler `0x80057230`,
  then the fire request `0x80058120` and the target update `0x80058404`.
  The runner and the fire request are hooked; nothing between them reads the
  use state.

## Change

- **E opens the door, weapon out** (`door_use_ready`/`door_use_request`, input
  thread; `door_use_answer`, the next runner entry). The host calls the
  game's own finder. At a door (type class 5, branch `0x800515e0`) it makes
  the original Action press, with a fresh Cross presented in the action word
  for that one call only. Nothing goes through the pad, so E and fire never
  share Cross. If there is no door, or the door refuses (line-up, its own
  answer), E goes on as before (stow, then the use). This works with fire
  already held, and it no longer depends on `+0x174`.
- **The weapon is the original's through the kick.** For the animation runner
  of each update in an armed door use, from the runner's entry to the fire
  request's entry, `0x40` is clear and 116's entry lacks `0x20000`. Both are
  put back at the fire request's entry, or at the next runner entry if that
  was missed. A host call is held meanwhile, so no savestate is taken or
  loaded inside the window. The fire request's `0x2000` refusal is answered
  at the next runner entry: the host makes the request's own decision
  without the kick. It sets `+0x224 0x100` when Cross is held, the upper
  track has table bit 8 and no `0x2000`, and none of `0x201` is set. The use
  stage machine, the door's own callback and every other reader see the
  original state. Raise, fire, pump, reload, ammo and rate are the weapon's
  own. D10H6 pacing, the D10H4 first-shot boost and view-aimed rounds apply
  as on the ground (`door_fire_owned` is now "armed door use").
- **Left click never opens a door.** At a door, an armed Action press from
  the idle handlers sees Cross as held since the last tick (bit 1), so it
  starts no use and the round leaves.
- **Guard**: the animation flag table's state guard is split so entry 116
  (`0x800c29f4`) also exempts `0x20000` (`control_guards.inc`). The digests
  are from the owned executable and `guard_writer_audit.py` passes.
- Removed: the D08J8C probe, the fire-request revert, the Cross gap, the
  left-click cancel, and the host-driven door fire track (`trav_fire_track`
  is 11 again, for mantles only).

## Results (180 Hz, third person unless noted; UI slot 10 copy)

Cases: EF (E, fire 120 ms later, held 2.6 s), FE (fire held, E at 0.6 s),
SIM (both together), HOLD (fire held 1 s, E, held 3 s), FEE (E twice while
firing), TAP (E, then 12 taps 70/130 ms), E (alone), L (left click only).
In every armed E case the kick runs and the door opens, with no holster.
With left click alone the door stays shut and the weapon fires.

| Weapon (id) | First round after E (EF / FE) | Held gaps through the kick | Ground (L) |
| --- | --- | --- | --- |
| Desert Eagle (4) | 194 / 58 ms | 163-171 ms | 164-170 |
| Combat Shotgun (5) | 204 / 55 ms | 798-838 ms, pump each time | 801-834 |
| Gatling (7), Laser Gatling (28) | 192 / 61 ms | 96-105 ms (28 FE: one 55/145 pair) | 98-102 (28: 86-113) |
| Crossbow (3) | 495 / 366 ms | 900-902 ms | 907 |
| Buffalo Rifle (6) | 505 / 370 ms | 1034-1069 ms | 1068 |
| Flamethrower (9), (27) | 561 / 422 ms | 45-88 ms | 64-69 |
| Energy Weapon (10) | 421 / 294 ms | 42-100 ms | 45-84 |

- Before the rewrite: shotgun EF first round 1590 ms, FE gap 1736 ms, and E
  alone did not kick.
- 60 and 120 Hz (pistol, shotgun, Gatling), and first person at 180 Hz:
  the same.
- First person: the kick leg and the weapon firing together, the door
  swinging open.
- Walking into the door (W held, fire held, E): kicks, opens, 167 ms /
  816-834 ms throughout.
- Backed away from the door, E stows the weapon as before (no kick).

Same as the original, not changed:
- The RPG at a closed door kills Duke with or without the kick: point-blank
  blast.
- Firing the Freezer point-blank at the door freezes Duke himself (player bit
  `0x20000`, the game's frozen state). E pressed while frozen waits, as for
  any frozen Duke.
- The crossbow, Buffalo Rifle and flamethrower do not fire on 70 ms taps,
  at the door or on open ground (their raise is about 360 ms).
- Every kick of this door costs Duke about 5.6 health, held fire or not. The
  D08J8 build does the same. Rounds that hit the door while it swings can
  change that by 1.9 either way. Cause not established.

Limits:
- A reload that falls inside a kick was not reached in these runs. The
  weapon's own state machine runs it, as on the ground, but it was not observed.
- One Gatling tap of the twelve, landing on the kick's final update, did not
  fire. Held fire, the other weapons and the remaining taps were unaffected.
- Thrown weapons (knife, pipe bomb) leave no `ttk-launch` line, so they
  were checked only for a normal kick and an opened door.
- Only this door (kind 20 hinged door, LEVEL00) was played. The rule
  follows the type class, so any class-5 door takes the same path.

Native: `ttk-controls-test` passes (the D08J8 door group is rewritten for
D08J9: E answer and refusals, the pad word put back, the window and its
fallback close, the fire request without the kick, holstered/non-door/Vanilla,
left click). `ttk-input-test`, `ttk-aim-test`, `test_player_profiles` and
`guard_writer_audit.py` pass. No framework edit, no new hook, no generated
code change. Executable
`25253bf04fd558d130ba1b34867002a45c715926b48ac816676337ec5daa8e6f`.
