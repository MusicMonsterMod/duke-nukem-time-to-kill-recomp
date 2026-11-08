# D49 - The energy beam leaves the gun where it points

2026-11-08. Status: Done (user-accepted 2026-11-08 after part 2: "it's
really working!!", then "close this out then"). User report (D41B playtest):
"when you move around, the projectile beam lags behind where duke is
pointing." Follow-ups: D49A (impact glow at the drawn beam's end, then every
gun's wall hits) and D49B (rounds in flight leave the gun).

Before/after (local, not committed): `research/before-after/d49-beam-follows-gun/`
(`before-*.png` with `DNTTK_BEAM_FOLLOW=0`, `after-*.png` default; fast turn,
slow turn, running strafe; same lab run settings).

## Cause

At 120/180 Hz every presented image is a redraw (D17B late camera): the view
turns to the newest mouse look, while the world is drawn at the presented
world time, about 90 ms back (D10G4's `timeline_pair`). In first person Duke's
arms and gun are drawn as if turned to that late view (D14H body camera), so
the gun follows the mouse at once.

The energy beam is a world object. The constructor `0x80073220` makes one
projectile (pool `0x800da978`, 100 records of 0x60, type 15 at `+0x30`) per
shot; D22D's `beam_finish` gives each Modernized view-aimed beam a fixed
origin record (`+0x18`, the muzzle at the shot). The updater `0x800733C8`
rebuilds the end `+0x34` every tick as origin + 6000 along the angles `+0x24`
(yaw) / `+0x26` (pitch) (matrix `0x8002b1b4`: direction
`(sin y cos p, -sin p, cos y cos p)`, 4096 a turn), trimmed at walls. The
projectile draw loop `0x8006d08c` sends type 15 to `0x80072868`, which draws
the streak from the origin, the angles and the end. A beam lives about three
game ticks.

So a beam appears on screen about 90 ms after it was aimed and stays up for
up to another 100 ms, while the view, and the gun with it, has turned on. The
lab's dx=20 turn left up to 29 degrees between a drawn beam's aim and the
view it was drawn under; running, up to about 600 units of Duke's motion.
Not the game tick or the beam's own update: the redraw's world delay.

## Change

- `weapon_aim.cpp`: each beam record (now 100 x 48 bytes, one per pool slot;
  the old 96 x 16 table was four slots short of the pool) also holds a tag and
  the view the shot was aimed from (D10G3 eye and forward). The records live
  in guest memory reserved before the workers fork, so redraws read them.
  `aim_beam_view`, `aim_beam_follow_enabled`, console `beamfollow`.
- `frame_replay.cpp`: a late first-person redraw (body camera, flag 8) also
  carries flag 16. The worker moves each of Duke's live view-aimed beams in
  its own memory copy by the change from the shot's view to the drawn view:
  `q -> eye + Rl^T Rs (q - shot eye)` for the origin record and `+4`; the end
  too unless the beam is locked on an actor (the end stays on the actor);
  then the angles point from the moved origin to the end. The beam leaves the
  gun as it did on screen at its shot, with Duke's motion since.
- Drawing only: the game's beam, its updater, hits, D22D locks and damage
  are untouched. 60 Hz (no redraws), third person, game-owned cameras and
  Vanilla are unchanged.
- A/B: backtick console `beamfollow [on|off|default]` (session only),
  `DNTTK_BEAM_FOLLOW=0` at launch. `DNTTK_BEAM_FOLLOW_TRACE=1` (developer)
  logs each moved beam: lock, shot and drawn eye and forward, yaw before and
  after.

## Evidence

Lab `recomp/analysis/d49/lab.py` (local), private copy of UI slot 8 (internal
7, energy weapon drawn), offscreen, 180 Hz unless noted, first person.

| Check | Before (`DNTTK_BEAM_FOLLOW=0`) | After |
| --- | --- | --- |
| Fast turn right (dx=20) | streak swung far left of the gun and crosshair | streak from the muzzle toward the crosshair |
| Fast turn left | streak to the right | toward the crosshair |
| Slow turn (dx=6) down the corridor | streak bent toward the side wall | streak down the corridor to the crosshair |
| Still | streak to the crosshair | identical |
| Beams moved per redraw (180 Hz / 120 Hz run) | - | 1340 / 964 beams; view change since the shot median 2.1, p90 19.3 / 21.5, max 28.8 degrees; eye moved p90 442 / 472 units (running strafe) |

A fixed late-view offset (`DNTTK_LATE_TEST_YAW_FILE`, plus/minus 10 degrees)
does not show the problem: the shot also aims from that view, so the beam
already met the crosshair. Only real turning or moving does.

`ttk-aim-test` (D22D group, new D49 checks: tag and unit forward in the
record, foreign slot and a slot whose origin is the live muzzle refused) and
`ttk-input-test` pass.

## Not changed / limits

- **Impact glow.** The bright green glow and sparks where the beam meets a
  wall are game particles (heap records holding the spawn point, the beam's
  real end), not the beam. They stay where the beam really hit, about 90 ms
  of turning behind the crosshair, like every gun's wall-hit effect under the
  D10G4 world delay. Close walls make it large. While turning fast, the beam
  end and this glow separate. Moving them as well needs their list and owner
  (not found in this pass: no direct reference in the executable).
- **Other guns.** Flamethrower fireballs and Gatling wall puffs (lab
  screenshots) trail the crosshair the same way: flying rounds and impacts
  drawn 90 ms back. Rounds fly on their own after leaving the gun, so they
  cannot simply be glued to the view; a launch-time version (the newest rounds
  drawn from the gun, fading to their world path with age) would be a
  follow-up.
- **Third person** is unchanged: Duke himself is drawn at the same delayed
  world time, so the beam stays on his gun; both trail the crosshair by the
  world delay.
- Offscreen lab only; the user's real mouse at 180 Hz decides.

## Part 2 - the root on the drawn barrel (2026-11-08)

User, after the first pass: "the root of the beam still disconnects from the
barrel of the gun and this is the part of the illusion that i feel is the
most damaging".

Second cause: the first-person gun is not drawn where the game's muzzle is.
D12/D36 show it in a presentation pose (`prepare_display_matrix`,
`first_person.inc`: the recorded firing pose at a fixed anchor, pushed past
the near cull, per-weapon offsets and tilt, recoil kick) under the body
camera, while the beam's origin is the game muzzle (`player+0x80c`). In the
lab the drawn barrel tip sits at (81, 177, 746) in the view frame; the
moved game muzzle was about 330 units from it even standing still.

Change:

- `first_person.inc`: every first-person Duke draw records the barrel tip as
  drawn, exactly as the muzzle flash `0x800341e4` places it: the display
  hand matrix's translation plus its transpose times the slot's muzzle
  vector (shorts at `0x800c45b0 + slot*0x2c`, slot `player+0x3b9`), in the
  view frame. `first_person_drawn_muzzle`.
- `frame_replay.cpp`: at the list step after Duke's draw (the existing
  `0x8001ca4c` hook, where the body camera hands back to the world camera;
  projectiles are drawn after the actors), each of Duke's view-aimed beams
  gets its origin record and `+4` on that tip under the world camera, and its
  angles pointed from there to the end. A new entry hook on the beam draw
  `0x80072868` was not used: entry hooks must be listed in
  `mod_function_entry_funcs` and regenerated.
- Same scope as part 1: late first-person redraws only, worker copy only;
  `beamfollow off` disables both parts.

Evidence (same lab): 1447 beam draws at 180 Hz and 529 at 120 Hz put on the
barrel, none without a recorded barrel; screenshots still, fast turn right
and left, running strafe: the streak starts at the barrel tip. The root
follows the presentation pose, per-weapon offsets and the recoil kick, since
it is computed from the same matrix the gun is drawn with. `ttk-aim-test`
and `ttk-input-test` pass.

Still not covered: 60 Hz (no redraws; the root stays at the game muzzle,
which sits behind the drawn barrel along much the same screen line), third
person, the impact glow and the other guns (above).
