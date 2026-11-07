# D08J3A - Climb camera revisit with aiming

Status: **Accepted** (2026-11-07, user: "excellent work!!! well done! lock it in close it out. testing was solid. i love it"). Modernized third person only;
Vanilla, the Original camera and first person unchanged.

## What was measured

Private offscreen lab `recomp/analysis/d08j3a/` (local only: `lab.py`,
`sweep.py`; port 9617-9621; copies of the D35E ladder, D35F chain, D35I1
climbing wall, D35G ledge hang and the D08J6D slot-6 monkey-bar fixtures;
player saves, cards and settings hash-checked unchanged after every run).
Each sweep turns the view a full circle in 12 steps, sidearm out, saved
distance 1344, and records the clear lens (note 380) and a picture.

Executable `b4a6a545...` (the D08J6F baseline), before the change:

| Climb | Result |
| --- | --- |
| Ladder (alley) | lens clear in every view; side views pull in to 459-632 against the alley walls and the native fade covers Duke (narrow alley, as noted in D08J3) |
| Climbing wall | lens 1122-1537, Duke in frame in every view |
| Chain | 6 of 12 views (from 110 degrees round to the chain side) held at 450, the D10F1 floor: the platform wall is nearer, so the lens sat inside it and the frame filled with dark geometry close to the lens |
| Ledge hang | 5 of 12 views (100 degrees and more off his back) held at 450 inside the ledge wall; a faint ghost of Duke over the opposite alley wall |
| Monkey bars | Duke turns to the aim (D08J6E), so the view stays behind him; 4 of 12 held at 450 against the room wall |

The cause is the D10F1 floor (450, an accepted ground default). On a climb or
hang the climbed surface is within a body width of the pivot, so a lens held
at 450 from that side is behind the surface.

The other checks in the job:

- Centred shoulder (D08J3 follow-up) against aiming: unchanged. With the
  climb centring, the crosshair stays visible in every framed view; on the
  bars Duke is centred below it. Not changed; raise it in play if the body
  covers targets.
- Aim leaving the gun arc: D08J6E (bars, chains) and D08J6F (ladder, wall,
  ledge swing-out) already turn or swing Duke; the camera keeps the view,
  so nothing further was needed here.

## Change

`clear_lens.inc`: `clear_lens_attached()` (mode-3 climbs, ledge hang and
shimmy, monkey bars) uses the existing 96-unit minimum as the floor in
place of 450. The lens then stops 48 units in front of the surface it
would hit; the native fade covers Duke when it comes close. The ground keeps
450. Developer tuning `DNTTK_CLIMB_CAMERA_FLOOR` (96-1600); `DNTTK_CLEAR_LENS=0`
remains the full A/B. Debug: `clear_lens.attached`.

No hook address, guard, generated code, framework or save change.
Candidate executable `6ecfe38fb8b80476de41f6824381351da066b9dbf452093325c550008b3dd0c3`.

## Evidence after the change

| Sweep | Result |
| --- | --- |
| Chain, 60 and 180 Hz | the six wall-side views stop at 219-348 in front of the wall; the room beyond is clear, no dark geometry; Duke fades out at that range; 180 identical to 60 |
| Ledge | the five wall-side views at 128-184 or 422; same picture of the opposite alley wall, without the ghost |
| Ladder, climbing wall | unchanged (the floor was never reached) |
| Monkey bars | the four close views at 298-331; Duke's arm can come into frame, crosshair clear |

Before/after sheet: `recomp/analysis/d08j3a/before-after.png` (local).
Native: `ttk-controls-test` 153 PASS groups including the new D08J3A case
(a climb uses the minimum floor, the ground eases back to 450), with the
pre-existing D14 HUD block (lines 1751-1766) bypassed for the run only;
`ttk-aim-test` pass.

## Limits

- In views from the climbed surface's side Duke is not visible: there is no
  room for a camera behind him there. Ideas if this is not good enough in
  play: raise the lens while compressed, or blend to the first-person view
  when the boom collapses on a climb. Not built.
- Fixtures only (user UI slots not used: all were re-saved after the job
  was written and show first-person views, so their contents are unknown).
- No enemies framed; no first-person, Vanilla or Original camera runs
  (those paths do not reach the clear lens).
