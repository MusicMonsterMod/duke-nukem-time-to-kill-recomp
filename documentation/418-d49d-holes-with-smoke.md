# D49D - Pistol and shotgun wall smoke in step with the bullet holes

2026-11-08. Status: Done (user-accepted 2026-11-08: "it trails behind again
but its so much better"; second pass: bullet hits left at the real hit; the
first pass, below, was judged worse by the user). User (after D49A): "if you load up the
pistol, or shotgun, the trailing smoke seems to be out of sync slightly with
the bullets when you're firing against a wall face ... giving the illusion
that it \"moves towards\" the bullet hole in the wall". Follows D49A
([note 417](417-d49a-impacts-follow-beam.md)).

Before/after (local, not committed): `research/before-after/d49d-holes-with-smoke/`
(pistol, fast turn right, 180 Hz, first person, 24 presents; before: the D49A
look with smoke under the crosshair and the hole behind; after: second pass,
smoke and hole one world-fixed hit).

## Cause

The lead in the job was right. D49A draws each new wall puff (effects
`0x12`/`0x20`/`0x21`/`0x1c`) under the crosshair in late first-person redraws
for 3 game frames, then eases it back over 5 to the real hit. The bullet hole
was always drawn at the real hit, so on fast turns the puff appeared beside
its hole and then slid onto it.

## What the bullet hole is

The round wall impact `0x8006f1b8` (a0 the round, a1 the point, a2 the wall
normal) first calls `0x8006d848` when the round's `+0x58` (decal texture, 275
for bullets; `+0x5a` size, 24 pistol / 48 shotgun) is non-zero, then spawns
the puff. `0x8006d848` finds the room for the point and calls `0x800a291c`
(flags `0xc080be00`), which takes a 0x3c-byte record from the free list
`0x800e1ef0` or, when the pool is full, recycles the oldest, and puts it at
the head of the active list `0x800e1ef8` (`+0` next, `+4` prev). Pool base
`*0x800c5984`, count `*0x800c5988` (96; at most 256). Record: `+8` flags (bit
0 persistent), `+0xc` scale matrix (size, size, 4096), `+0x20..+0x28` the
position (exactly the puff's point), `+0x2c` flags, `+0x30` colour, `+0x34`
texture, `+0x38` room. Far walls got no hole in the lab (the room lookup).

## Outcome (second pass, current)

User playtest of the first pass: "unfortunately i think it looks worse". A
hole travelling across the wall with its smoke reads worse than smoke
travelling onto a fixed hole: anything that moves relative to the wall looks
wrong. The underlying cause is the high-refresh world delay (50-90 ms, D10H5),
not hitscan. EDuke32 (research/eduke32) turns the view every drawn frame
(`drawframe_do` -> `P_GetInput` -> `P_UpdateAngles`), blends the world between
its last two 30 Hz tics (`G_DrawRooms`, `smoothratio`), resolves a bullet with
one hitscan in the tic and spawns hole and smoke as world sprites at the hit:
hits trail by at most one tic of turning and never move. The user chose to
match that ("eduke32 feel (cleanroom) IS the goal"):

- Bullet wall hits are not moved at all: the hole tagging below and D49A's
  puff following are removed, together with D49A's round impact queue in
  `weapon_aim.cpp` (shot views, round binding, `aim_take_impacts`) and its
  `ttk-aim-test` group. Smoke and hole appear together at the real hit, as one
  world-fixed hit.
- The energy beam's glow and sparks still follow (D49/D49A): the beam is tied
  to the gun.
- Shrinking the world delay itself is the new job D10H8.
- Rejected: moving the decorative hit to where the crosshair is (a guessed
  wall, marks away from the damage and the impact lights, and everything else
  still late).

Lab (180 Hz, pistol, fast turn right, present sequence): smoke and hole move
across the screen together as one hit in every frame. Energy weapon: 64 glows
and 220 sparks tagged, 0 misses. `ttk-aim-test recomp/disc/SLUS_005.83` and
`ttk-input-test` pass.

## First pass (removed)

### Change

`impact_follow.inc` (no change to `weapon_aim.cpp` or the game's records):

- Scan (live game, before each gameplay frame goes to the redraw workers):
  the records added at the head of `0x800e1ef8` since the last scan (walk
  until the previous head; if it is gone, e.g. after a level load, nothing is
  taken as new) are new holes. Each new hole within 96 units of one of Duke's
  view-aimed round impacts queued this frame takes that impact's view, the
  same tag its puff takes. Tags for 256 hole slots follow the effect and spark
  tags in the guest block; a savestate load clears them.
- Worker (late first-person redraw, D49 flag 16): tagged holes move by the
  same view change and the same weights as the puffs (fully for 3 frames,
  then easing back over 5), so smoke and hole appear together under the
  crosshair and settle onto the real hit together.
- Drawing only: hits, damage, the game's hole records and their real
  placement, D41B lights, the beam (D49/D49A), 60 Hz, third person and Vanilla
  are unchanged. The Gatling's holes follow the same way (same impact path).
- A/B: `DNTTK_IMPACT_HOLES=0` keeps holes at the real hit (the D49A look);
  `DNTTK_IMPACT_FOLLOW=0` or console `beamfollow off` turns all impact
  following off. Debug port `render_replay` `plugin.impact.holes` counts
  tagged holes.

### Evidence

Lab `recomp/analysis/d49d/lab.py` (local), private copy of UI slot 8 (internal
7), offscreen, first person unless noted, fast turns and strafe while firing
at the near wall.

| Run | Impacts / puffs / holes tagged |
| --- | --- |
| Pistol 180 Hz | 16 / 16 / 16 |
| Shotgun 180 Hz | 60 / 60 / 60 |
| Pistol 120 Hz | 17 / 17 / 17 |
| Gatling 180 Hz | 27 / 27 / 27 |
| Third person 180 Hz | 11 / 11 / 11 tagged, none moved (no late first-person redraws) |

Present sequences (24 consecutive presents, pistol, fast turn right, 180 Hz):
before, the hole sits outside its puff (well to the side of it early, at its
edge later); after, the hole is inside its puff in every frame. The shotgun
sequence window held no impact (slower cadence); its tag counts show the same
path. `ttk-aim-test recomp/disc/SLUS_005.83` and `ttk-input-test` pass.

### Limits (first pass)

- The pair still settles from the crosshair back to the real hit over about
  0.15 s after the first 0.1 s (D49A timing); now the hole travels with its
  smoke instead of the smoke travelling onto the hole. If the moving hole
  reads worse in play, the alternatives are a shorter settle (developer
  `DNTTK_IMPACT_SPARK_HOLD` / `DNTTK_IMPACT_SPARK_FADE`) or leaving bullet
  puffs at the real hit.
- Not judged on the user's display yet.
