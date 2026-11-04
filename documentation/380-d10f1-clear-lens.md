# D10F1 - Third-person camera: clear lens against walls and grills

2026-11-04. Implementation of the D10F proposal
([note 379](379-d10f-close-surface-camera.md)). Status: Done (user-accepted
2026-11-04). Executable: `ac4634201d59a1b6133043becb570c55aec461db5631df071c31a9667a5136d4`.

## Behaviour

Modernized, independent camera, third person only (`fp_blend == 0`, the
orbit lease). Vanilla, the Original camera, first person and its blends keep
the native solve. Developer A/B: `DNTTK_CLEAR_LENS=0` (native solve);
`DNTTK_TIGHT_CAMERA=1` still selects the D10B comparison instead.

The native four-ray solve (`0x8003aa48`) still runs, but its result is
replaced at the follow easing (`0x8002a038` from `0x8003ade4`, the three
calls at `0x8003af40/60/80`, view-space delta at `sp+0x18`), once per camera
update, by an anchor whose lens is clear:

1. **Lens probe.** The original segment query (`0x8006d980`, the query-only
   wrapper in `weapon_aim.cpp`, now `camera_segment_query` with an ignored
   actor and the actor it stopped at) runs from the pivot to the preferred
   lens (the D10 boom and shoulder behind the pivot, plus the native
   `camera+0x42`/2 = 193 lens setback) and to three ring points 48 units
   beside it (both sides, and the vertical side away from the pivot). The
   nearest hit less a 48-unit margin shortens the whole boom along that ray,
   so the shoulder shrinks with it. Floor: 450 lens units
   (`DNTTK_CAMERA_FLOOR`, developer only).
2. **Actors.** An actor stops the lens unless the native draw fades it from
   that lens (root nearer than 1024 in depth, `0x80034e18`; 896 is used for
   the root joint's offset) **and** the lens is outside it (a query back from
   the lens meets the same actor at least 48 units away). Crates and other
   small objects are therefore passed over and keep the native fade. The
   apartment wardrobe (actor type 194) stops it: with every actor passed over,
   the lens went inside the wardrobe model and the frame was black.
3. **Timing.** Inward changes are immediate. Outward ones hold 100 ms, then
   ease exponentially (time constant 350/3 ms, host time;
   `DNTTK_CAMERA_RETURN_HOLD`, `DNTTK_CAMERA_RETURN`, developer only), so a
   doorway clearing no longer pops the camera across a room in one update.
4. **Native follow lag kept.** The state's per-axis rates (`0x800c0d4c`) are
   applied to the new anchor as the original would; the lens this leaves is
   probed again (only when it strays from the probed ray) and pulled in
   front of a hit. The result is passed to all three easing calls at rate 1.
5. **Monkey bars.** While hanging from climbable grates (D08J4 state), the
   query does not report the grates (objects): measured pivot 237 units below
   the hanging grate's origin, lens 69 above it. The lens now stays 96 below
   that grate's plane, closer than the floor if it must (minimum 96), so
   looking down shows the floor under the grill.

Unchanged: native Duke and box fades (the D10B1 rollback reason), room and
height queries, yaw/pitch and look input, the saved distance and shoulder
(never written), aim (shots already use the solved lens), the native minimum
word (`0x800c3c48` stays 768).

Cost: each original query costs about 0.2 ms in this build (measured: save
17 us, query 198 us, restore 67 us before the change to word copies; rare
query peaks of 10 ms). The probe set is four queries; a still camera reuses
its result for one update (a closing door is seen one update later); actors
found to stop the lens skip the back query for the rest of the update.
Continuous mouse orbit beside the wardrobe: 1.35 ms per camera update
(5.7 queries, peak 3.5 ms) at 30 camera updates/s; the game kept 30
updates/s and 60 fields/s in both modes. Apartment sweep average 0.75 ms.

## Evidence (private copies only)

Local: `recomp/analysis/d10f1/` (`sweep.py`, `perf.py`, `still.py`,
`hang.py`, `camlab.py`, `shots/`, `runs/`). Fixtures: the D10B apartment
copy (runtime slot 5) and copies of the user's UI 12, UI 11 and UI 6 saves
(runtime 11, 10, 5) in `user-cards/` with the D10B private cards; source
state hashes checked unchanged; player files hash-checked by every run.
Offscreen, mouse steps of 25 counts (about 9 degrees), shots every few steps.

- **Apartment sweep (D10F fixture, distance 1536).** Native: lens inside
  walls, wardrobe face, black frames. Clear lens at 60/120/180: no frame over
  30% black (0 of 14 each), lens 464-1729, no step outward over 570 units per
  0.17 s sweep step (several updates; D10F native: 1840 in one step), no
  declines. The doorway now eases (979 -> 1557 -> 1716 lens over three steps).
- **Free look held (middle mouse).** Native: wardrobe face, black frame and
  Duke out of the frame in the pinned steps. Clear lens: Duke in the frame in
  every sampled step (ghosted by the native fade when close).
- **UI 12 (closet), distance 3456.** Native at yaw 90: the wardrobe fills the
  frame; clear lens: open view with Duke. At distance 1536 both modes are
  clear from the saved spot.
- **UI 11 (corridor near the crate room).** No actor on the lens ray from the
  saved spot; the clear lens comes about 200 units closer than native (lens
  in front of the wall instead of 193 behind it), so the native Duke fade
  starts sooner here (seen at lens 954). Crates themselves were not on the
  sampled rays: regression check still needs the user's crate playtest.
- **UI 6 (monkey bars), pitch sweep.** Native and the first candidate: the
  camera above the grill looking down onto it. With the grate cap: below the
  grill, the floor and stairs below in view; at the saved 36-degree pitch the
  lens is 221 from the pivot, inside Duke's head, so Duke is not visible.
- **Native tests.** `ttk-controls-test` (LEVEL00, LEVEL01, all levels) passes
  in three runs: default (new D10F1 group: rate-1 delta, instant pull-in in
  front of a wall, the floor, hold and eased return, no saved-distance
  write), `DNTTK_CLEAR_LENS=0` (D10B1 native follow unchanged) and
  `DNTTK_TIGHT_CAMERA=1` (D10B). `ttk-aim-test` and `ttk-near-test` pass.
  No framework edits; the runtime patch re-export is unchanged.

## Limits and open questions for the user

- Floor 450, hold 0.1 s, return 0.35 s: proposal defaults, to confirm.
- Doorways: a long boom can still ease out through a doorway into the next
  room and frame Duke past the door jamb (UI 12 at 3456, yaw 147-156). Keep,
  or keep a compressed camera in Duke's room?
- Duke fades more often in tight corridors (the lens no longer hides behind
  walls).
- Monkey bars: the camera is very close when looking steeply down; a lower
  climb pivot belongs to D08J3A.
- Cost about 1.3 ms per camera update while orbiting beside large actors;
  frame pacing on the real 180 Hz display is not measured offscreen.
- Not probed: enemies, ladders, chains, poles, ledges and other levels
  (D08J3A), dynamic doors closing on the camera.

## Acceptance (2026-11-04)

User: "I'm very happy with the gameplay and performance in this iteration, and the camera angles have made the game feel solidly playable." Done at the defaults above (floor 450, hold 0.1 s, return
0.35 s, doorways allowed). The open questions were not raised; follow-ups
belong to D08J3A (climb framing) and D10G (aim and mouse feel). New job D12E
(third-person quick kick) was logged in the same closeout.
