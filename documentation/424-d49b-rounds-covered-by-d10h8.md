# D49B - Launched rounds leave the gun (covered by D10H8)

2026-11-08. Status: Done (user-accepted 2026-11-08: "it honestly feels pretty
damn good!", "the rpg felt just phenomenally good!", then "lock it in and close
us out"). No code change.

## Question

D49's lab showed rounds that fly on their own (flamethrower fireballs, RPG
rockets, freezer shots) appearing beside the gun on turns at 120/180 Hz,
because the world was drawn about 90 ms behind the view. D49B was scoped to
draw each new round from the drawn barrel along its shot view and blend it
into its real path.

Two later results changed the picture:

- D49D ([note 418](418-d49d-holes-with-smoke.md)): moving world hits toward
  the crosshair read as sliding. The user chose the EDuke32 feel: leave world
  objects at their real place and cut the delay instead.
- D10H8 ([note 419](419-d10h8-newest-world-draw.md)): first person now draws
  the newer game frame's list, with 16 ms extrapolation. New things appear
  about 18 ms after their game frame instead of about 70 (about 7 instead of
  24 degrees of trail at 360 deg/s).

## Measurement

Lab `recomp/analysis/d49b/lab.py` (local): private apartment card copies,
level 1, `dnstuff`, offscreen, 180 Hz, first person.

- The flamethrower launches one fireball (projectile type 14) per game frame.
- Still: the newest fireball is on the crosshair.
- Slow turn: slightly behind; the stream bends with the turn because each
  fireball keeps its own shot's path, as in EDuke32.
- A/B against the old draw (`DNTTK_WORLD_DRAW=pair`): the newest fireball sits
  nearer the crosshair now; only very fast turns (dx=20) leave it clearly
  behind.
- The lab's RPG run was spoiled (Duke killed by his own point-blank rocket);
  the freezer key did not select in the lab.

## Outcome

The user chose to playtest before any change. Playtest at 180 Hz: the
flamethrower and RPG feel right ("the rpg felt just phenomenally good"), so
no barrel blend was built. The user's other observation, a lag between mouse
button down and the first shot on the energy weapon and flamethrower, is
D10H9 (press to fire), not a draw problem.

Not built (kept as options if wanted later): the barrel blend (risks the D49D
sliding look) and a birth-point ease (new rounds start at the drawn barrel
tip without being turned).
