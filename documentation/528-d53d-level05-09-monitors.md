# D53D - Security monitors in LEVEL05 and LEVEL09

2026-11-21. Done (user-accepted: "they look aweseom! and work"). Follow-ups: D53E (the option does
not switch the look live), D61 (level atlas and teleport).
Labels: U user request, S source reading, L isolated runtime observations.

## Request

U: "work on D53 TVs in LEVEL05 and LEVEL09. for testing you can place a
statesave in slots 9 and 10 so i can easily get to those spots too". Note 432
listed these levels' monitors as unverified (D35K: their camera callbacks are
LEVEL05 subtypes 17-19 and LEVEL09 subtypes 11-13).

## Findings

- L/S: both levels use the LEVEL00 monitor model exactly: 16 vertices, nine
  0x2c quads, first corner (-200, 250, 0), radius 320 (`monitor_mesh`,
  `security_monitor.cpp`). Only the prop type differs: 858 in LEVEL00, 818 in
  LEVEL05 (mesh `0x8013f688`), 1000 in LEVEL09 (mesh `0x801437d0`). Every D53
  part is found by model, not type, so all of it already applies there.
- LEVEL05 OBEY OR DIE: three monitors at x -17395, y -9856, z -15360 / -14592
  / -13824 (room 18), facing +x, screen centre at Duke's eye height.
- LEVEL09 PIG FACTORY: three monitors at y -10178, z 55260, x 50619 / 52620 /
  54745 (room 25), facing -z, on a raised platform (top y about -10120, about
  780 above the hall floor at -9340, about 2300 deep), beside the game's
  RE-ENTRY POINT. On the platform the screen sits just below eye height.
- Lab gotcha: the developer `warp` keeps the height it is given, and Duke
  floats there until he moves. Warped at monitor height in LEVEL09 he landed
  inside the platform block (y -9600) and could not walk. One jump puts him on
  top. (The `warp` y is used as given; no change made, it is a lab tool.)

## Checked (offscreen, real GPU, private card copies)

Duke 3D look (the Modernized default), first person at 60 Hz, both levels:

- The three monitors draw as the D53 TV (swaps counted, template is the
  level's mesh); TV lights published (3 in LEVEL09; 4 emitters in LEVEL05
  with a power sign elsewhere in the level).
- Use (80 ms click from about 310 units): Duke lines up 275 from the panel,
  the camera view opens with the REC frame, the second click returns.
- D53A: after the view the TV shows the camera's low-res live picture (about
  15 pictures a second, 41-52 stored in the runs) until out of range.
- D53C, third person: no hand through the glass; the camera bit comes at the
  same point.
- TTK look (`--security-monitors ttk` / profile `security_monitors: ttk`):
  original monitors, bare camera view.

Pairs and camera/feed frames: `research/before-after/D53D/` (local). Lab:
`recomp/analysis/d53d/` (local; `d53d.py survey|visit`, `seq.py`, `ver.py`;
`red.py` and `tr.py` for the two findings below).
No code changed.

## Savestates (U request)

UI slot 9 (file `slot08`): LEVEL05, Duke 900 units in front of the middle
monitor, facing it. UI slot 10 (file `slot09`): LEVEL09, on the platform 1100
in front of the middle monitor, facing it. Each was loaded again on a private
copy and the walk up, use, return and feed checked. The player's previous slot
9 and 10 states are kept in `research/savestate-backups/2026-11-21-d53d-before/`
(local). Made with the current player build (2026-11-21 01:22, the D53A
build); a later build that changes the codegen hash will refuse to load them.

## Found, not changed

- **Red room in the TTK look (LEVEL05, LEVEL09).** Each monitor has an
  authored D41D1 sign lamp, pure red, strength 1.0, radius 1576, 300 in front
  of the screen (`assets/lights/level05.lights` lines 42-44,
  `level09.lights` 28-30; LEVEL00 has the same three at 57-59). The lamps are
  baked (D41P2), so `lamp off` does not hide them. In LEVEL05's pale room the
  TTK look turns the wall and floor deep red (frame mean 202,47,48; without the
  three lamps 42,47,48; LIGHTING off 59,67,69). In the Duke 3D look the wall
  stays its own colour (mean 88,79,67); why the bake shows them weaker there
  was not traced. LEVEL00's brick room shows only a pink cast. A fix is a
  lighting choice (weaker or no monitor lamps, or tie them to the TTK glass);
  left for the user.
- **A held click re-enters the camera.** Leaving the view with a press held
  about 150 ms or more starts a new use of the same TV (Duke +0x224 0x44 at
  0.17 s, camera again about 1 s later). Taps of 40-100 ms exit cleanly. Same
  use path in every level (LEVEL00 too); not examined in Vanilla.

## Playtest

U, UI slots 9 and 10: the TVs look right and work. The states load at the
right places but Duke has no visible weapons; suspected cause: they were made
after a developer `level N` load (not verified). Switching SECURITY MONITORS
to TTK in play leaves the TV (the pause menu shows the originals): D53E.
