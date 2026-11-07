# D10H2 - Pistol chase: rounds that leave on target land

2026-11-07. Status: Done (accepted and closed 2026-11-07: "lock it in and close it out. insane work"). Follows the D10H1 lab
([note 389](389-d10h1-weapon-feel-lab.md)), D10G4
([note 387](387-d10g4-shoot-what-you-see.md)) and D10H3
([note 403](403-d10h3-eduke32-auto-aim.md)). Executable
`0a960573b50f1779f37930d11bf06c07b8c45f2ecea54d39ffc42c621ccf77ae`.

D10H1 measured that 65-75% of the Desert Eagle rounds that left with the
crosshair on a sliding pig cop landed. This job reproduced the chase without
the fixture's race, found what still takes a round off an enemy, and fixed it
for every view-aimed round.

## How a pistol round resolves (measured)

The pistol round is not hitscan. It is projectile type 0, launched at the gun
call `0x8003c500` and swept about 3300-3500 units per game update by
`0x8006dd2c`, which calls the segment query `0x8006d980`. The first sweep runs
4-13 ms after the launch, in the same game update. Enemies that update after
Duke run between the gun call and that sweep. In the type 26 runner chase they
moved 91-118 units in that gap, and a placed pig cop walking on its own moved
56-96.

## Causes found

1. **The D10H1 fixture.** The debug port's `write_ram` writes one byte per
   request, at interrupt safe points (`debug_server_poll` from
   `interrupts.c`). The slide fixture's 4-byte position words could therefore
   land between the shot's query and the round's sweep, and even torn
   (partly written). A game-thread fixture (below) moved the same pig cop at
   the same speed, and every round that left with the drawn crosshair on it
   landed. Most of D10H1's misses were the fixture.
2. **The crosshair asked a different question from the round.** The sweep
   passes two words to the actor test `0x80076330` that the aim query left at
   0: projectile `+0` (argument 5) and `+0x2e` (argument 6). For Duke's rounds
   the launcher `0x800718ac` sets `+0 = 3` when the projectile type's table
   word (`0x800C4BC0 + 64 * type`, word 0) has bit 4, and `+0x2e` from table
   `+0x1a`. Pistol 0, shotgun 1, Gatling 7, RPG 8 and Freezer 13 all have
   bit 4. In `0x80076330`, bit 2 of argument 5 selects the model-box test
   `0x80075f0c` for an actor with a `+0x40` model box and no `+0x44` sphere
   list. Word 0 takes `0x80075bfc`, a coarser shape. On the pig cop the coarse
   query met the body about 200-265 units from its position, while the round's
   test met it 80-245 units out. So the crosshair counted rays past the body's
   edge as "on target", and the round, aimed at that coarse edge point, missed.
3. **Update order.** A round aimed at an enemy that updates after Duke
   reaches the sweep one enemy step later (about 60-100 units at 1900-3000
   units/s). A round on the trailing edge then misses.

The muzzle offset was not a cause. The round's own line (muzzle along the
final direction), traced with the round's test at the gun call, met the
target on every round that then landed. Every miss on a race-free fixture was
a round whose target moved or animated after the gun call.

## Change (`recomp/src/ttk/weapon_aim.cpp`)

- `trace` takes the hit-test word for stack argument 5. Argument 6 stays 0,
  because the crosshair is a point. The D10G3/D10G4 screen-ray query, the
  D10H3 auto aim eye and muzzle traces, and the `original-lock` assist checks
  pass 3, the word the rounds use. The body-to-muzzle cover check and the
  camera queries keep 0. `DNTTK_ROUND_TEST=0` at launch keeps the coarse
  test (developer A/B).
- **Round follow.** When a view-aimed gun round was aimed at an actor (the
  crosshair's actor, the D10H3 auto aim target or the original lock), its
  first sweep shifts the round's end point by that actor's step since the gun
  call, scaled along the round's line. The round then goes through the point
  the crosshair met. Only rounds that start at this shot's origin, owned by
  Duke, on a living actor, with a step of 1-512 units, in that game update:
  Duke's next animation-runner entry (`0x8005a210`) disarms it. The end point
  stays at the same distance, so speed is unchanged. Spread round the line,
  damage, later sweeps and walls are the original's. A/B: backtick console
  `roundfollow [on|off|default]` (session only), `DNTTK_ROUND_FOLLOW=0` at
  launch. Aim debug JSON: `round_test`, `round_follow`, `follow_arms`,
  `follow_shifts`, `last_follow_shift`.
- Modernized view aiming only (the same gates as D10G3/D10G4; the sweep hook
  also requires `input_modernized()`). Vanilla and `original` aiming are
  unchanged.
- **Diagnostics** (`DNTTK_AIM_TRACE=1` only): `ttk-aim-check` (the shot's
  actor, its position, and what the round's own line and test meet now) and
  `ttk-flight ... tgt=<actor>,<position> test=<+0>,<+0x2e>`;
  `ttk-round-follow projectile actor step`.
- **Lab fixture:** backtick console `testmove <actor hex> <units/s> [span
  [late]] | off`. It moves an actor sideways across the view on a triangle
  wave, one step per game update, on the game thread. Its health is held at
  30000, and every drop is logged (`ttk-testmove-damage`). By default the
  step is taken at Duke's animation-runner entry, before his shot. With
  `late`, the step is taken at the first sweep of one of Duke's rounds, which
  is where an enemy that updates after Duke takes its step. With span 0 the
  game moves the actor, and only the health log runs. Session only, for
  testing.

## Evidence

Builds: the game binary and `ttk-aim-test`.

- `ttk-aim-test recomp/disc/SLUS_005.83`: new D10H2 group, and every earlier
  group still passes (D10G1, D10G3, D10G4, D08Q3, D07F, D08O1, D08J6A, D22D,
  D10H3). The screen ray carries test word 3 and the cover check 0. A round
  aimed at the D10G4 fixture actor (target 300,0,650) whose actor stepped 60
  goes through 360,0,650 (x/z 0.400), with speed kept. A still actor, a later
  sweep, another owner, a dead actor, a 600-unit jump, Vanilla and
  `roundfollow off` all leave the round unchanged (0.308). The console
  refuses a bad value.
- Runtime lab `recomp/analysis/d10h2/chase.py` (local). It uses the D10G
  latency harness, private apartment card copies, offscreen, level 1,
  `dnstuff` / `dnkroz`, third person, 180 Hz, Desert Eagle, AUTO AIM OFF
  unless noted, and fire pulsed 300/150 ms. A spawned pig cop about
  3000 units out slides at 1900 units/s, plus or minus 300 units, across a
  fixed crosshair, moved by `testmove`. "On drawn" means the D10G4 query met
  the pig cop where the shown image drew it. "Landed" means a health drop was
  logged within 80 ms of the launch. Player files were hash-checked unchanged
  after every run.

| Run (late = the pig cop steps after Duke's shot) | On drawn | Landed |
| --- | --- | --- |
| Step before the shot, final code | 51 | 51 |
| Late, the code before D10H2 (coarse test, no follow) | 99 | 98 |
| Late, coarse test with follow | 88 | 73 |
| Late, round test only (no follow) | 54 | 53 |
| Late, final code (round test and follow) | 97 | 95 |
| Late, 3000 units/s, round test only | 102 | 97 |
| Late, 3000 units/s, final code | 95 | 92 |
| Late, final code, first person | 47 | 45 |
| Late, final code, 60 Hz (no D10G4 rewind) | 44 | 43 |
| Late, AUTO AIM HITSCAN, final code (all rounds) | 119 | 117 |
| Still pig cop, final code | 48 | 48 |

- How the misses differ:
  - **Coarse test with follow.** The follow holds the round on the coarse
    edge point (about 250 units from the pig cop's position, outside its
    model box), so it misses. The code before D10H2 kept landing many of
    those rays only because the pig cop's step often carried its body into
    the line. That is why follow is correct only together with the round's
    own test.
  - **Round test only.** 4 of 156 drawn-on rounds missed the classic way:
    the line was 84-99 units inside the pig cop at the shot and 162-191 units
    off it after its step.
  - **Final code.** 1 of 192 missed that way.
  - **Thin parts.** The other misses in both groups were rays that met thin
    parts 540-620 units from the pig cop's position, which animate between
    the shot and the sweep.
- **Game-moved runners.** Type 26, level 1, flees at about 2700 units/s, game
  AI only, the view kept on it by yaw: 15 of 15 rounds that left on the drawn
  runner landed, including ones it moved 91-118 units after the shot. They
  run almost along the line of fire, so this is not a crossing test.
- **D10H3 auto aim** (`d10h3/ttk_autoaim.py`, HITSCAN, still pig cop):
  0 degrees 18/18 auto aimed and landed, 4 degrees 13/13 (5 more went to
  another enemy in the cone, as in note 403), 8 degrees 18/18, 12 degrees not
  assisted. This is the same as before the change.
- **D10G3** (`d10g-latency/rays.py`, turning about 72 degrees/s, fire held):
  median ray error -0.01 degrees, 21/21 shots from the screen view. This is
  unchanged.

## Not verified / limits

- The real mouse on the player's 180 Hz display, against real running
  enemies.
- A crossing enemy moved by the game's own AI: the woken pig cops mostly
  stand, and the type 26 runners flee straight away. The crossing case was
  measured with the game-thread fixture, which gives game-like update order
  but not game AI motion.
- Round follow shifts by the actor's position step (`+4`). A turn or an
  animation change between the shot and the sweep is not followed. That is
  the thin-part misses above.
- Enemies with a `+0x44` sphere list are tested by spheres whatever the
  word, so the test change does not affect them. Only the pig cop was
  measured.
- Shotgun, Gatling, RPG and Freezer share the change (all Duke projectile
  types with table bit 4) but were not run in this lab. Rockets and freezer
  bolts take several sweeps, and only the first sweep follows.
- Climbs, swimming and the jetpack share the hook but were not run.
