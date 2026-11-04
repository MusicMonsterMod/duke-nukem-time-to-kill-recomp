# D10G - Aim and mouse feel in third person (research report)

2026-11-04. Status: Done (user closeout 2026-11-04). Research only: no gameplay code, hook, build or executable change.
Executable measured: `f57c694de9e655fa7d50f1a109b9c31e7bcf2fcae76590809499591cb419e070`
(the current player build, after D12E1 and D08A35).

User (2026-11-04): "any tightening we should do between that [aiming], and
even between third person mouse movement". Board questions: sensitivity,
smoothing and frame-rate independence at 60/120/180; whether the boom
compression changes the turn per mouse motion; crosshair against muzzle near
walls; whether a light aim camera fits TTK; D10E free look.

## Summary

The mouse path is already clean: raw, linear, no smoothing or acceleration,
the same angle per count at any frame rate and any boom length. What feels
"off" up close is the shot, not the turn:

1. **Point-blank parallax (new, measured).** With Duke against a surface and
   the boom compressed (lens 450-520 from the pivot), the crosshair point is
   a few hundred units from the muzzle and well above it. The round and the
   drawn gun arm then go up to 70 degrees off the view: in the wall sweep 18
   of 24 shots left more than 20 degrees off (median 44, open room median 7),
   and the pistol was drawn pointing steeply up at the wall (screenshot
   `look-0.png`). Several of those rounds recorded no impact and flew on over
   1000 units. This matches "shooting in impossible directions" on the
   ground, not only on climbs.
2. **Press-to-shot delay (native, measured).** The pistol leaves about
   200 ms after a click from rest and about 100 ms when the gun is already
   up (within about 0.4 s of a shot), identically in first and third person.
   The round takes the crosshair direction when it leaves, so motion during
   the delay goes into the shot (11 degrees at a steady 72 degrees/s).
   This is the original weapon timing, not a Modernized regression.
3. **Shot sample.** At the moment a round leaves, its direction is about one
   game update (about 33 ms) behind the newest mouse motion shown on screen
   (per-redraw late view), about 2.4 degrees at 72 degrees/s. Small; it
   only matters while tracking fast.

## What the code does today (source read)

- **Input.** SDL3 relative mouse mode on X11 (the player's session) uses
  XInput2 raw motion, so no desktop acceleration. `pc_input.cpp` sums
  `xrel`/`yrel` into totals; nothing scales, filters or accelerates them.
- **Turn.** `modern_controls.cpp`: `yaw += counts * sensitivity`, pitch the
  same with the invert option; sensitivity is degrees per count (player
  profile 0.12, the default), never multiplied by frame time. The
  accumulator is consumed per game update, so the total angle is the same
  at 60, 120 and 180 Hz.
- **Presentation.** `late_view` redraws the view from the mouse total up to
  each present (including motion still queued in SDL), placed by event time
  with at most an 8 ms spread, so turning is smooth at high refresh. This is
  the only "smoothing" and it adds no lag.
- **Boom.** The orbit turns about the pivot; the radius (Alt-wheel,
  D10F1 clear lens, the 450 floor) changes only the camera position, never
  the yaw/pitch step. The same mouse motion turns the view by the same angle
  compressed or not. Compressed, Duke fills more of the screen and the near
  scenery sweeps less, which can read as slower; the angle is identical.
- **Shots** (`weapon_aim.cpp`). The screen ray starts at Duke's depth and
  finds the crosshair point; the round leaves the physical muzzle toward
  that point; a body-to-muzzle check guards cover; if the point is less than
  64 units ahead of the muzzle along the view, the origin retracts to the
  body. There is no limit on the angle between the round and the view, and
  the crosshair never shows a blocked or off-axis shot. Shots read the game
  camera (`camera+0x0c`), not the late view. Free look (D10E) aims from its
  own eye; fire does not cancel it today (held free look is a look-around,
  and its decay restores steering on release).

## Measurements (private copies only)

Local: `recomp/analysis/d10g/` (`lab.py`, `lag.py`, `lead.py`, `delay.py`,
`agree.py`, `divergence.py`, `look.py`, result JSON; runs under
`recomp/analysis/d10f1/runs/`). Fixture: the D10B apartment card copy
(runtime slot 5), private profile copy, offscreen, `DNTTK_AIM_TRACE=1`,
pistol (`dnweapons`). Player files hash-checked unchanged after every run.

| Test | Result |
| --- | --- |
| Turn, stop, click (`lag.py`, 8 trials) | Shot at the final yaw (0.0-0.17 degrees from a settled reference). Turning that has stopped is never lost. |
| Click and turn together (`lead.py`, 7 trials) | The shot took in 9.2-9.5 fields of turning after the click (11.1-11.4 degrees at 1.2 degrees per field). |
| Press to shot (`delay.py`) | From rest: 12-13 frames (208-228 ms) in third and first person. Gun already up: 5-7 frames (91-126 ms). |
| Open room sweep (`agree.py`, 24 x 15 degrees) | Rounds stop on the crosshair point (0-5 units). Round-to-view angle median 7, max 46 degrees (natural parallax a few hundred units past Duke). |
| Against a wall (`PRE=w:2.5`, 24 steps) | Lens 450-520 for 18 steps. Round-to-view angle median 44, max 71 degrees, 18 of 24 over 20. 13 of 24 shots recorded no impact. |

The press-and-turn result (about 9.4 fields of turning) against the press
delay (about 11-12 frames polled) leaves about two fields: the shot uses the
camera of the previous game update. The polling adds its own field or so, so
this figure is approximate.

Not measured: enemies (no enemy spawn yet), other weapons' delays, climbs
(D08J3A, D08J6F), real 180 Hz display pacing (offscreen only), gamepad.

## How other games handle it (online, 2026-11-04)

- **Two traces.** Trace from the camera to find the aim point, then from the
  muzzle to that point; hitscan uses the muzzle trace for the hit and
  projectiles for their direction ("parallax aiming"). TTK already does this.
- **Close or blocked targets.** Compare the two traces: when the muzzle
  trace hits something the camera trace did not, the shot goes to the muzzle
  hit, and many shooters show a blocked reticle (a small X or a second dot
  at the muzzle hit). When the aim point is very close, fire along the view
  (parallel to the camera ray) rather than at the point, so the round never
  leaves at a steep angle.
- **Reticle.** A reticle pinned to the muzzle moves about and is hard to
  play; keep it at screen centre and add the blocked hint only.
- **Aim camera.** Shooters with an aim button pull the camera in and toward
  the gun shoulder while aiming to shrink the mismatch. Smoothing and hidden
  mouse acceleration are the most common PC complaints about third-person
  aim; raw, unsmoothed input is preferred.

Sources: Unreal Engine forums, "Third Person Weapons - Correct Aiming"
(forums.unrealengine.com/t/third-person-weapons-correct-aiming/309866) and
"Third Person Shooter - Aiming at Crosshair"
(forums.unrealengine.com/t/third-person-shooter-aiming-at-crosshair/129180);
"Handling third person perspective shooting, devlog 1" (moddb.com, MGU);
r/howdidtheycodeit "How does projectile over the shoulder aiming work"
(2024); Bugnet "how to fix crosshair not aligned with shots" and "how to fix
mouse sensitivity problems in your game"; Blur Busters raw-input thread;
PCGamingWiki smoothing tags. Mostly forum and devlog material, not official
documentation. Earlier board sources (Naughty Dog GDC 2024, Nesky GDC 2014)
still apply.

## Proposals (each a separate job for the user to select)

Modernized, view aiming, third person only unless stated. Vanilla and the
ORIGINAL aim option unchanged.

- **D10G1 - Point-blank aim (recommended first).** When the crosshair point
  is close to the muzzle (proposed: closer than about 512 units, or the round
  would leave more than about 25 degrees off the view), aim the round along
  the view from the muzzle instead of at the point, blended by distance so
  there is no visible switch; the drawn gun arm follows the same direction.
  Keep the body-to-muzzle cover guard and the 64-unit retraction. Test: the
  wall sweep above (target: no round over about 25 degrees off the view,
  every round meeting the surface it is fired at), the open-room sweep
  unchanged, `ttk-aim-test`, D08J6 climb fire.
- **D10G2 - Blocked-shot hint.** Trace muzzle to crosshair point (one query,
  already used for cover); when something nearer blocks it, show a small
  marker at the muzzle hit or dim the crosshair. Optional, user choice.
- **D10G3 - Shot sample at the newest look.** Take the shot direction from
  the late view at the moment the round leaves (the per-redraw look the
  player sees) instead of the previous game update's camera. About 33 ms
  tighter while tracking; small.
- **Press-to-shot delay: report only.** The 100-200 ms is the original
  weapon timing. Shortening it changes the rate of fire and balance, so it
  is not proposed; a "ready" stance that keeps the gun up while armed
  (so every first shot is the 100 ms one) could be scoped later if the user
  wants it.
- **Aim camera: not recommended now.** TTK has no aim button (right mouse is
  Grab, middle mouse free look) and firing is constant, so a fire-triggered
  zoom would move the camera on every shot. Revisit only with a dedicated
  aim binding.
- **Mouse turn: no change.** Raw, linear, frame-rate and boom independent.
  If the user feels the turn is slow when compressed, that is perception;
  a separate third-person sensitivity is possible but not recommended.
- **Free look (D10E):** unchanged. Fire aims from the free-look eye already;
  the board's "fire cancels it" is not current behaviour and is not needed.

## Open for the user

- Select D10G1 (and optionally D10G2, D10G3)?
- Save states where shots felt wrong on the ground (walls, corners,
  doorways, crouched behind cover); private copies only.
- Whether a ready stance for the first shot is worth scoping.

## Decision (2026-11-04)

User: "i wanna go with your recommendation for an easy smooth and
professional experience". Going with the recommendations: D10G1 point-blank
aim first, then D10G2 blocked-shot hint and D10G3 newest-look shot direction
(board jobs). Declined: mouse turn changes, a fire-triggered aim camera,
shortening native weapon timing; a ready stance was not requested.

New report in the same closeout: at the user's UI slot 11 save, opening the
door puts the camera behind something. Logged as D10F2 (clear lens follow-up);
not reproduced in this session.
