# D10F - Third-person camera near close surfaces (research report)

2026-11-04. Research only: no gameplay code, hook, build or executable change.
Executable measured: `c0402b1ac27410cf60976b39dd4c00bd3fc0eb6dff1dc6b0a93d917cca521b14`
(the accepted D08J6E build).

User (2026-11-04): "the free look around feels very kinda off ... you can get
caught behind objects and such ... usually when up against close surfaces".

## How the native solve works (static read of the retail executable)

The Modernized orbit (D06/D10) only requests a camera anchor; the original
constraint `0x8003AA48` still decides where the camera goes:

1. **Four corner rays.** `0x8003A938` runs once per corner of a window
   centred on the requested anchor. The window is 512 x 240 world units
   (half sizes from `0x800D22A4/6`, 256 x 120), lies in the anchor plane
   (local z = 0) and is turned by the view matrix. Each corner is traced
   from Duke's pivot (`0x8003A3AC` -> `0x80039F40`, the map-cell walk with
   `0x8005D790` and the camera room `camera+0x90`). The closest hit wins.
2. **Snap to the hit.** `0x8003A5E0` moves the anchor so the hitting
   corner sits on the hit point.
3. **768 minimum.** If the solved anchor is nearer than 768 units
   (`0x800C3C48`) to the pivot, `0x8003A85C` puts it back at 768 along
   **-Z of the target's own rotation** (`player+0x7CC`, Duke's heading,
   measured: its first halfword tracks `player+0x1C`), keeping the local
   height. It does not use the view direction.
4. **Lens behind the window.** The camera position (`0x80039C7C`) is the
   anchor plus `(0, 0, -camera[+0x42]/2)` in view space: the lens is 193
   units (`+0x42` = 386) **behind** the tested window, outside every ray.

So the solve guarantees a clear window in front of the lens, not a clear
lens. Pinned at the minimum, the camera is pushed back through the surface it
hit, and the push follows Duke's back rather than the orbit.

## Reproduction (private fixture, player-like mouse orbit)

Private copy of the D10B apartment fixture (runtime slot 5, LEVEL00 bedroom
near the wardrobe and bathroom). Private profile copy with the user's saved
camera distance 3456, centred shoulder, third person, offscreen, port 9616,
`recomp/analysis/d10f/` (local). Sweep: 42 mouse steps of 25 counts (about 9
degrees), each held 0.05 s then settled 0.12 s, one screenshot per step;
`dA` = anchor to pivot, `dE` = lens to pivot, `room` = `camera+0x90`. Player
files hash-checked unchanged after every run.

| Yaw (deg) | dA | Room | What the frame shows |
| --- | --- | --- | --- |
| 279-15 | 3440-3452 | 8 | Open room, Duke clear (full boom). |
| 33, 60 | 2058, 1312 | 8 | Lens inside the wall: about three quarters of the frame is a dark wall face. The minimum is not involved. |
| 45 (Duke backed toward it) | 998-1496 | 8 | The whole frame is the wardrobe's face; Duke hidden behind it. |
| 120-171 | 761-775 | 8, then 83 | Pinned at the 768 minimum; Duke ghosted by the native fade, a wall or wardrobe band along the right edge. At 171 the camera's room changes to 83 (pushed through into the wardrobe). |
| 171 -> 180 | 761 -> 2598 | 83 -> 7 | **Pop**: one 9-degree step jumps the camera about 1840 units outward into the next room. Duke is then framed through a door frame from outside his room. |
| 207 -> 216 | 1964 -> 1168 | 7 -> 83 | The reverse pop back through the doorway. |

Same fixture with the D10B comparison (`DNTTK_TIGHT_CAMERA=1`, minimum 256,
instant in, eased out): the pinned range compresses to 458-716 units, but the
lens is still in the wall at yaw 30-63, the room still changes to 83 and the
camera still pops out to room 7 at 168-177 (527 -> 1980). Duke also
disappears completely at 458-527 units (D10B's own fade). Lowering the
minimum alone therefore does not fix what the user describes.

Free look held (D10E, middle mouse; gameplay yaw fixed at 270, Duke facing
it): where the boom is pinned, the lens comes out **nearer** to Duke than the
anchor (dA 414 / dE 377, 743 / 607): the minimum pushed the anchor along
Duke's back while the view looks elsewhere. In all five pinned steps Duke is
out of the frame entirely. Grounded Modernized Duke normally faces the view
(D07A), so this shows up wherever he does not: free look, climbs (D08J3, his
pivot against the climb surface), and any facing lag.

180 Hz (`frame_rate` 180, same sweep): the same pins (761-775), the same room
change and the same pop (761 -> 2809, rooms 83 -> 7). The solve runs once per
game update, so the pop is a game-update step at every presentation rate.

Not reproduced here: actor occluders such as crates or breakables (the native
box fade the user kept in D10B1 handles them), enemies, ladders, chains and
hangs (D08J3A), other levels. One fixture only.

## Comparison with the industry pattern (board research, 2026-11-04)

| Pattern | TTK native | Gap |
| --- | --- | --- |
| Probe a sphere or box swept from the pivot to the lens | Four rays to a window 193 units in front of the lens | The lens itself is never tested |
| Pull in instantly | Yes (the snap to the hit) | - |
| Never push the camera back through what it hit | 768 minimum re-expands, through the surface | Caught behind walls; room changes |
| Keep the camera on the view axis | Minimum follows Duke's heading | Duke leaves the frame in free look and climbs |
| Ease back out (about 0.3-0.5 s) | Outward jumps arrive within one game update | Doorway pops of about 1800 units |
| Shoulder toward centre when compressed | Shoulder offset applied before the solve, unchanged | Compressed shots stay offset |
| Fade thin occluders, keep the scene readable | Native box and Duke fade (kept since D10B1) | Keep as is |
| No auto-yaw | None | - |

## Proposal: D10F1 (camera position only)

Modernized, independent camera, third person (`fp_blend == 0`) only. Vanilla
and the Original camera keep the native solve. No change to fades, opacity,
the saved distance, the saved shoulder, the look input or the aim ray (shots
already trace from the solved lens, `camera+0x14`).

1. **Clear lens.** After the native solve, trace the original segment query
   (`0x8006D980`, already wrapped query-only by `weapon_aim.cpp` `trace`) from
   the pivot to the real lens point and to a small ring around it (about 64
   units, the sphere probe). On a world hit, shorten the boom so the lens sits
   that margin in front of the hit. Actor hits are ignored, so crates and
   breakables keep the native fade instead of shoving the camera.
2. **Replace the push-out with an on-axis floor.** Below 768 keep the camera
   on the orbit's own axis at the cleared distance, never along Duke's
   heading. Floor about 450 lens units, where the native fade already ghosts
   Duke (screenshot at 768); never through the surface. Developer A/B switch.
3. **Ease outward only.** Inward corrections stay instant. Outward hold about
   0.1 s, then ease over about 0.35 s in host time (frame-rate independent),
   so a doorway clearing does not pop the camera 1800 units in one update.
   Optionally refuse an outward step that would change the camera's room away
   from Duke's while the boom is compressed (the doorway case).
4. **Shoulder toward centre while compressed**, from the solved distance over
   the preferred boom (the D10B relief, which was not the rollback reason);
   the saved side returns as the boom recovers.
5. **Later, not in D10F1:** a brief thin-occluder tolerance for poles and
   railings in level geometry, if playtests still show snaps there.

Climbs keep their own shoulder rule; D08J3A applies this result to ladders,
chains, walls, hangs and monkey bars.

Tests planned: native controls cases (lens probe hit, floor on axis, outward
hold and ease, Vanilla and Original untouched), this sweep at 60/120/180
(target: no lens-in-wall frames, no room change to a room Duke is not in, no
outward jump over about 300 units per update, Duke inside the frame in free
look), D10E free look, D08J6 climb fire and the near-clip suite.

## Open for the user

- Save states where it feels off (pillars, crates, doorways, corridors,
  ladder walls); private copies only.
- Closest allowed camera distance (proposed about 450) and the return speed
  (proposed 0.1 s hold, 0.35 s ease).
- Whether a compressed camera may still look through a doorway into the next
  room, or should stay in Duke's room.

## User review and save states (2026-11-04)

The user reviewed the report and saved states in awkward positions (UI
slots; runtime slot = UI - 1; private copies only):

- **UI 12 (runtime 11), the apartment closet:** "part of the closet is
  obscuring the camera". The same wardrobe case as the sweep above.
- **UI 11 (runtime 10), the crate room:** "not as bad as I thought because
  the code which makes objects translucent starts when the camera gets too
  close ... the box room is playable, very playable". The native box fade
  works; no change wanted for actor occluders (confirms the D10F1 rule that
  actor hits do not move the camera).
- **UI 6 (runtime 5), the monkey bars:** "the camera goes above the grill
  ... you can't really see what's going on there ... it's a bit silly that
  it doesn't just adjust so that more of the game is visible because you
  want to be able to look down and ... see if there are any enemies down
  below". The camera must stay below the climbable ceiling and keep the
  floor in view (D10F1 lens probe, with D08J3A for the climb framing).

User's direction: "go through a few of them and just start seeing where we
can tighten things up where the camera does adjust to it better when it's
against walls and not clipping into walls". Also: "we will never have it as
good as first person I think but we've done pretty well". The proposed
floor, return speed and the doorway rule were not answered; D10F1 starts at
the proposed defaults and checks them with the user.

D10F is Done as a research job (user closeout 2026-11-04: "close this out
for this session"). Implementation is D10F1.
