# D17AR4 - Superseded portal patches: Classic keeps them, Polymorph covers them

2026-11-01. Status: Done (accepted 2026-11-01: "ready to accept this, and then
you can close us out").

D17AR1 planned to retire the portal rules that host visibility supersedes:
D17P's portal margin, D17Y's open upper edge and D17AQ's open-sky room list
(all in `recomp/src/ttk/draw_distance.inc`). Since D17AR6, Classic is a
player choice and runs with host visibility off, so those rules are what
Classic draws with. User decision (2026-11-01): **keep them for Classic**;
Polymorph must not depend on them.

Evidence is local under `recomp/analysis/d17ar4/` (`ab.py`, `sheet.py`,
`slots.py`; logs, traces, shots). Every run used private copies of the D17AQ,
D17Y and D17P fixture cards and a private profile; the harness hash-checks
the player's saves and profile before and after.

## Before this job

- The three rules already ran only on frames the host pass did not draw
  (`draw_distance_apply`: `if(walk && !host_vis_walk(cpu))`), so Polymorph
  frames never used them; Classic did.
- The host pass reuses the D17AQ over-the-walls geometry (`open_sky_pieces`,
  `open_sky_in_view`, `open_sky_seen`), and it was gated by the Classic
  developer switch: `DNTTK_OPEN_SKY=0` also turned off Polymorph's sky rooms
  seen over walls.
- **Polymorph did not cover D17Y.** The host portal walk clips a room seen
  through an opening to that opening's projection. D17Y exists because a
  full-height opening from the camera room into a taller sky room ends at the
  camera room's ceiling while the walls behind it rise higher. Third person,
  slot-8 castle (D17Y fixture file slot 07, level 6, room 28 into room 22),
  wall view yaw 180 during a jump: room 22's rectangle top fell from -120 to
  -33 in Polymorph exactly as in Classic with `DNTTK_OPEN_PORTAL_TOP=0`, and
  the tower tops were replaced by sky (`castle-third-top.png`), while Classic
  drew them. First person at the same place did not show it (room 22 already
  spans the view).

## Changes

- `draw_distance.inc`: the D17Y geometry is one predicate,
  `upper_edge_open(source room, target room, portal)`; Classic's
  `open_upper_portal` (still behind `DNTTK_OPEN_PORTAL_TOP`) loops the camera
  room's portals and calls it. Comments mark the margin, open top and open-sky
  list as Classic's (they still run on a frame the host refuses).
- `host_visibility.inc`:
  - the host portal walk applies `upper_edge_open` itself: an opening from the
    camera room that qualifies gives the target room the camera room's
    (guard-banded) top, as Classic does for that one direct neighbour;
    downstream openings keep their apertures. Counter `open_tops`.
  - over-the-walls selection no longer reads `DNTTK_OPEN_SKY`; that switch is
    Classic-only.
  - developer audit `DNTTK_HOST_VIS_AUDIT=1`: per host frame, for every room
    the game's portal walk listed, whether the rectangle Classic would use
    (2-pixel margin, open top) reaches past the host's room rectangle. Debug
    JSON `audit`: [checked, past, open-top entries, open tops past the host,
    largest overshoot in pixels, last room].
- No framework changes; no generated code or hooks changed.

## Results

Fixture routes, 60 Hz, scale 2, 16:9, FOV 90 (counters over each route):

| Case (fixture) | Polymorph | Classic | Classic, rules off |
| --- | --- | --- | --- |
| D17AQ jetpack height, moat (D17AQ file slot 09, flown up) | castle whole; 0 refusals, 0 pieces handed back | whole (969 widened, 571 opened, 1795 added) | castle cut on a vertical line |
| D17P subway tunnel (D17P file slot 07) | clean | clean (647 widened) | thin dark outline round the far opening (`subway-far.png`) |
| D17Y castle jump, third person (D17Y file slot 07) | tower tops drawn (after the fix) | drawn | sky in place of tower tops |

- Audit (Polymorph, first and third person, all four routes): open tops past
  the host in 35 of 447 open-top entries before the fix (largest 87.9 px, room 22), 0
  after; `open_tops` fires on every castle frame and nowhere else. The only
  other overshoot is the 2-pixel margin itself (max 2.0 px): the host tests
  each face against exactly projected openings, which is what the integer
  margin compensated for; the subway shows no seam in Polymorph.
- `DNTTK_OPEN_SKY=0` and all three Classic switches off: Polymorph's
  over-the-walls rooms unchanged (1422 jetpack, 542 moat), pictures the same.
- 120 Hz third person castle (redraw workers): tower tops drawn.
- Level sweep (D17AR2 `sweep.py`, 21 levels x 4 views, host on/off): no
  refusals, no pieces handed back, no world piece data changes; 83 views within
  animation noise, the other a gameplay event (level 0: an enemy's muzzle
  flash lights the street in the host-on shot).
- `ttk-near-test`, `ttk-input-test` pass. `ttk-controls-test` stops at the
  existing `movement_ready()` assertion (line 394) recorded in D17AR5, before
  the portal groups.

## Limits

- The D17Y case was checked on the one fixture where it is known; other
  levels rely on the shared predicate and the level sweep.
- Subway third person: the game's walk listed a room the host walk did not
  reach in 186 frames (`game_missed`); such rooms are drawn through the
  game's own rectangle (the host never draws less than the game), so nothing
  is lost, but the host walk does not explain them yet.
