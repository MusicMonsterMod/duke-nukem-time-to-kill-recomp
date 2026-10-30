# D35N1/D35N2 - Mirrors behind furniture

2026-10-30. Status: Done. User playtest: "working perfectly"; closeout
authorized the same day.

## Report

E at two mirrors did nothing, so Duke never said his mirror line:

- D35N1: the user's slot 6 (2026-10-25 card), a bathroom mirror behind a wooden
  counter.
- D35N2: the Old West (user's UI slot 8, 2026-10-29), a mirror behind a dresser.

The working mirror (slot 7 on the 2026-10-24 card) is in open floor.

## Cause

S: the mirror action and its line are entirely native, and both run whenever E
selects a mirror:

- The idle Action `0x80051890` calls the use finder `0x80077f28`. It walks
  Duke's cell list and accepts the first object whose type flags have
  `0x09c00000` and that passes `0x80077ccc`.
- `0x80077ccc` casts three horizontal rays from Duke, at the object's centre
  height: centre, plus and minus 128 units to the side, each 512 units long
  (`0x800c3cf4` and `0x800c3cf8`). Each ray is tested against that object
  alone (`0x80076330`), and a hit sets the contact at player `+0x174`.
- Mirrors are interaction classes 11 and 15 (type-record byte 4), with the use
  flag `0x00400000` and the sign bit set. Their line-up `0x8007ec4c` therefore
  needs only a ray hit and never walks Duke. The selector `0x800513e4` installs
  pose 278 and, at `0x80051644..0x800516c4`, requests a random line 50-52
  through `0x8006cad8`.
- The quote table at `0x800c0dac` (16 bytes per line) has flags `0x12`: bit 1
  marks the line repeatable and bit 0 is set after it plays. Lines with `0x10`
  are skipped only while six or more sound channels are busy (`0x8006ca4c`).

L: in both reports Duke stands at the furniture and the mirror is out of ray
reach. Slot 6: player `(24948,-10741,-9037)`, type-637 mirror at 732 units
from its model centre. Old West: type 750 at 541 units. The finder never
reaches them, so E does nothing; nothing in the mirror or quote code is missing.
Raising `0x800c3cf4` through the debug port reproduced the fix without code
changes. 704 was the smallest tested reach that worked for both (slot 6 failed
at 640, slot 8 worked at 576), and both worked at 768 and 1024.

## Change

`recomp/src/ttk/apartment_interaction.inc` already aims the first finder ray
for the LEVEL00 light switch. Its call-chain check is now shared
(`use_finder_first_ray`):

- the first ray is `0x80076330` from `0x80077e18`;
- its stack frames return to `0x80077fa4` and `0x800518c0`;
- the game is one-player Modernized with identity;
- the object is in Duke's room (`+0x2e`).

The switch keeps its level-0 and object checks. Then:

- `mirror_use_query` applies only to class 11/15 objects with the use flag. It
  aims that one ray through the model centre, up to 896 units and within a
  35-degree view cone.
- The original model intersection, finder order, callback, line-up, pose 278,
  line choice and repeat rules run unchanged.

The change adds no hook addresses (`0x80076330` was already hooked, so the
codegen hash and savestates are unaffected). It needs no framework,
generated-C, save, settings or menu changes. The Vanilla hook returns before
this branch.

## Evidence

Private harness and captures: `recomp/analysis/d35n1/` (card copies of the
2026-10-24 slot-6 fixture and the 2026-10-29 slot-8 fixture; private port,
offscreen; player cards untouched).

- Before: E at either mirror leaves anim 63, contact `0` and no action.
- After, Modernized first person: E twice at each mirror plays pose 278 both
  times with first-person blend 1.0 and returns to idle with the weapon redrawn.
  `ttk-mirror-query` logs object `801d7da4` type 637 at 732.1 (slot 6) and
  `801dec04` type 750 at 540.7 (slot 8).
- Slot 8 quote table: line 52 then line 51 change from `0x12` to `0x13` on the
  first and second use, the native line request. All three slot-6 lines were
  already `0x13` in that save (repeatable, so they still play). Function
  tracing is not compiled into this build, and these runs use the dummy audio
  driver, so hearing the line remains player acceptance.
- Global third person: pose 278 at both mirrors with blend 0.
- The existing slot-7 mirror still plays 278 on both uses.
- `ttk-controls-test` (clean `d08-camera-final` LEVEL00 fixture) passes. New
  cases cover:
  - 700 units on level 1 aims the ray;
  - no aim beyond 896, behind Duke, in another room or for a non-mirror class;
  - no aim without the use flag, in Vanilla, with the wrong caller or in two
    players;
  - the light switch still refuses outside level 0.
- Vanilla live check is not evidence: the offscreen harness's pad presses did
  not register on any mirror, including the working one (as in D35K). Vanilla
  safety rests on the hook's Modernized return and the unit case.

## Limits

Only these two mirrors and the slot-7 regression mirror were driven. Other
mirror placements are covered by rule, not by play. The 896-unit reach is
deliberately generous (slot 6 needs more than 640). Within the cone, an E
press in the same room can now reach a mirror up to that distance across open
floor.
