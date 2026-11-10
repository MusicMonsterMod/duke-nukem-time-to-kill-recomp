# D56 - Necro blast and atomic health give light

Status: Done (user-accepted 2026-11-10). Job board: D56. Follow-up backlog: D57
(review of other objects and items that should give light).

User request, UI slot 8 (level 2, MINER 69ER, Necros firing up the mine
corridor): make the Necro's blast and the atomic health light emitters.

## Findings

| Item | How the game holds it | Light before D56 |
| --- | --- | --- |
| Necro blast | Necro (object type 50) launches projectile type 20 through the launcher `0x800718ac`; the pool record (`0x800da978`, 0x60 bytes) reads flag `1` in flight, not the `3` of rounds; hits on Duke or an enemy do not call the wall-hit routine `0x8006f1b8` | default warm launch flash at the Necro, nothing in flight, a warm spark only on a wall hit |
| Atomic health | pickup object (kind `+0x14` = 4) of type 581 (`+0x2c`), linked in the room lists (`0x800da454`, heads `+0x48`/`+0x4c`); its mesh pointer `+0x44` animates, its place stays put once at rest; taken, it is unlinked | none |

Both CONFIRMED in a private copy of UI slot 8 (file slot 7) and UI slot 6
(file slot 5), offscreen lab `recomp/analysis/d56/` (local).

## Implementation

`recomp/src/ttk/dynamic_lights.inc`:

- `ShotLight` has a `bolt` field: a bolt is lit in flight while pool flag
  bit 0 is set (rounds keep the exact value 3) and throbs a little
  (0.88..1.12 per tick).
- `lights_bolts_tick` (per game tick): a bolt whose record goes out bursts
  at its last place with the type's impact light, unless a wall-hit spark
  is already within 1200 units (no double burst). Forgotten on a level
  change, a savestate load or LIGHTING off.
- Type 20: launch flash 0.80 0.36 1.25 radius 2800 (enemy flashes x0.8),
  flight 0.75 0.34 1.20 radius 2600, burst 0.95 0.45 1.35 radius 3000 for 5
  ticks. Colour reference: the D39E purple Necro screen flash (Duke 3D's
  Octabrain COOLEXPLOSION1).
- Console `light bolts off|on` (session): bolts back to the old warm flash
  and no flight light, for comparison.

`recomp/src/ttk/prop_lights.cpp` (with `prop_lights.h`, `level_lights.inc`):

- `pickup_lights[]` by pickup type, so later items are one line each. Atomic
  health 581: 0.42 0.82 1.30, strength 0.85, radius 1700, 260 above its foot,
  a slow throb (0.8..1.0, 1.4 s). Tuning `DNTTK_PROP_LIGHT_ATOMIC="r g b k
  radius"`.
- Scanned from the room lists every 10 frames and on a level change or load
  (dropped and spawned ones appear within a sixth of a second); each image
  checks the pickup is still linked and of its type, and follows its live
  position (a dropped one falls). Joins the D41L list: room gate, the shared
  budget nearest first, shadows.
- Console `lamp pickups off|on` (session).

Modernized only, LIGHTING ENHANCED or RELIT on Polymorph; Vanilla and
Classic take nothing (the same gates as D41B and D41L).

## Evidence

- Slot 8, Necro blasts: trace `launch enemy ... type 20` and `impact type 20`
  at Duke's place on hits. Timed shots: the left wall's mean colour goes from
  24/21/18 with no blast near to 49/32/49 and 53/33/53 as a blast reaches
  Duke, then back.
- Matched shots (local, `research/before-after/D56/`): `necro-before.png`
  (`light bolts off`) and `necro-after.png`, the blast at the same place;
  `necro-approach.png` a blast closing in; `atomic-before.png` /
  `atomic-after.png` (`lamp pickups off` / on), three spawned atoms on the
  level 5 street with a cyan-blue pool below.
- An atomic health spawned in front of Duke and picked up went dark.
- Build OK; `ttk-input-test` and `ttk-near-test` pass. Framework untouched.

## Limits

- Not measured on the player's GPU at 180 Hz.
- One atom's strength not judged in play (the shot had three stacked).
- The blue hit cloud seen at a hit is the game's own sprite.
- Vanilla not launched; the code is gated as noted.
