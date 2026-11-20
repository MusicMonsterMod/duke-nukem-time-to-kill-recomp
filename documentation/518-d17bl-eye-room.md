# D17BL - The sky through a ceiling hatch: the room the eye is in

Status: **Done, user-accepted and closed out 2026-11-20** ("fix accepted! great work king", "close it out big boy!"). Executable
`37cd47eea6062cd229ceba6dca333d0bfc2ec382367d5653a435857ce3d3a49b`.

## Report

User, 2026-11-18 (desktop, UI slot 12, level 8 HOLY TERROR): "when you climb
up this ladder, the skybox appears briefly." The save was copied privately
to `research/savestates/d17bl/` and restored to the player's UI slot 12 on
2026-11-20 at the user's request (the save it replaced, 2026-11-19, is kept in
`research/savestates/slot11-before-d17bl-restore-20261119/`).

## What happened

Reproduced on the first private run (first-person ladder view, the player's
own profile, offscreen): holding W from the save, 7 of 60 shots were the sky
fill with only the ladder over it - no walls, ceiling or floor. The same
frames showed it with the Classic world renderer, with DRAW DISTANCE ORIGINAL
and at FRAME RATE 60; third person and Vanilla never did. So neither the host
world pass nor the draw distance owned it; the first-person eye did.

The ladder climbs through a ceiling hatch: room 60 (below) joins room 38 (the
shaft) through one horizontal portal at y = 1088. The camera render
(`0x8002e48c`) passes the camera's room word (camera `+0x90`) to the portal
walk (`0x8006276c`: a0 room, a1 camera, a2 view rectangle, a3 list), and
every visible room is found from there. Logged per frame during the climb,
that word stayed 60 for about 20 game frames after the eye had passed above
the hatch (eye y 1058 down to 899), and even flipped 38 -> 60 -> 38 once. A
walk from a room the eye has left finds no opening facing it, so nothing of
the world was listed.

Room boxes are loose: room 60's box reaches up to y = 896, above its own
hatch, so "is the eye in the room's box" alone cannot tell; the portal's
plane can.

## Fix (game-wide, no per-level data)

`recomp/src/ttk/eye_room.inc`, Modernized only (inside the input-modernized
gate; Vanilla never reaches it):

1. **Through a portal.** When the eye is on the far side of one of the camera
   room's portal planes (the side away from the room's box centre, towards the
   target's; a portal whose two rooms' centres are not on opposite sides is
   skipped), within the opening's extent widened by 128 units across it (and
   along its normal by the distance past it), and the target's box holds the
   eye, the walk starts in the target. Up to three openings in one frame.
2. **Outside the box.** When the eye is still outside the room's box, a room
   joined through one or two portal steps whose box holds the eye is used, the
   nearest step first, then the smallest box. An eye in no joined room (a
   wall-clipped orbit camera) keeps the game's room.
3. Only the walk's start changes: the portal walk's a0 at its entry (ra
   `0x8002e650`, and only when a0 is the camera's own room word), the host
   world pass's source room, and the Classic open-sky and open-top helpers.
   The camera's room word and all game state are untouched, so play, sound,
   savestates and the camera update see the original value.
4. **Host pass, both rooms.** When the eye was moved but is still inside the
   game room's box (in the slab of the hatch both boxes hold it), the game's
   room is drawn through the whole view too, so the hatch's walls beside the
   eye show, not only what lies through the opening.

Developer: `DNTTK_EYE_ROOM=0` keeps the game's room, `DNTTK_EYE_ROOM_TRACE=1`
prints each walk's start (in, out, eye). Counters: `ttk_input` ->
`draw_distance.eye_room` (`checks`, `moves`, `unfound`, `last_from`,
`last_to`; counted per call, several calls a frame).

## Evidence

Lab `recomp/analysis/d17bl/` (local only): `probe.py` (cases `climb`,
`rooms`, `fine`, `portals`, `slots`), `sky.py` (share of the top band in the
sky fill's flat colour), `bot.py` (pale sky in the bottom band). Private card
copies; the player's saves and profile hash-checked unchanged by every run.

| Run | Sky shots (top) | Bottom sky |
| --- | --- | --- |
| Before, player profile 180 | 7 of 60 | 11-57% in 6 shots |
| Before, Classic / ORIGINAL draw distance / 60 | 6 of 25 each | - |
| Box rule only | 3-4 of 40 | 6-9% |
| Final, player profile 180 (two runs) | 0 of 30 | 0 |
| Final, Classic | 0 of 40 | 3-6% in two shots (see Limits) |
| Final, ORIGINAL draw distance, 60, 120, third person, Vanilla | 0 of 40 each | - |
| Final, climb up and back down | 0 of 40 | 0 |

- Trace at 180 (main game and redraw workers, 1659 walks): no walk started
  in room 60 with the eye above the hatch; 132 walks moved to room 38. The
  only walks left in room 60 near the hatch have the eye 2-4 units below it,
  still in room 60.
- Regression on the player's other 11 savestates (copies, FRAME RATE 60,
  load picture and three turns each): first person 0 moves in all; load
  pictures identical with the rule on and off (at most 0.1% of pixels, idle
  animation). Third person: 2 moves on UI slot 4 (room 2 -> 1, the orbit
  camera turned through a portal into room 1, which is the correct room) and
  208 `unfound` on UI slot 9 (a wall-clipped orbit camera, which keeps the
  game's room as before).
- Before/after pictures: `research/before-after/D17BL/`.

## The small ladder in the distance (user, 2026-11-20)

"i can see a kind of small ladder in the distance when i climb it." Near the
top of the climb the view runs down the corridor of the shaft room; at its
far end the brick panels between dark wooden posts (the level's repeated
frames, objects 148/486 in room 59, about 11000-15000 units on) read as a
small ladder (`research/before-after/D17BL/distant-corridor-zoom.png`). It
is the same with the D17BL rule off (`DNTTK_EYE_ROOM=0`), with DRAW DISTANCE
ORIGINAL and with Classic: the level's own content, seen now that the sky no
longer covers the view through the hatch.

## Revision 2: the eye level with the hatch (user video, 2026-11-20)

The user's video (`research/screencaps/Video_2026-11-20_13-34-39.mp4`, local)
shows, while climbing up and down at the hatch and looking up, the lower half
of the view black with the sky fill along it and only the ladder object far
below: the room under the hatch was not drawn. The trace shows walks that
start in the shaft room with the eye on the hatch plane or a few units above
it (eye y 1087-1088, plane 1088): an opening at zero distance is inside the
near plane, so the walk cannot list the room behind it. New rule
(`eye_room_near_openings`, host pass): every room behind an opening of the
start room that the eye is within 48 units of (either side, inside its
extent) is drawn through the whole view. Developer `DNTTK_NEAR_OPENINGS=0`
turns it off. The lab did not reproduce the black frame, with or without the
rule (40-shot bobbing runs at 180, 120 and 60, pitched up), so the fix is
confirmed at the user's spot in revision 3 below.

## Revision 3: the user's exact spot (2026-11-20)

The user's new UI slot 12 save at the spot (copied to
`research/savestates/d17bl-2/`) reproduced it at once: eye y 1001, the walk
starting in the shaft room (38) as the game and the rule agree, the lower
half black with the ladder far below. The hatch's opening (y 1088) lies under
the room's real ceiling: room 60 reaches up to y 896 around the hatch, so an
eye between them is inside room 60's air while past the opening's plane, and
the host pass listed only rooms 38, 37, 40 and 59. The near-opening rule now
also takes the room behind an opening when the eye is inside that room's box
and within 256 of the opening's plane. At the spot room 60 is drawn and the
band is gone (`research/before-after/D17BL/before-hatch-band.png` and
`after-hatch-band.png`); turning on the spot, the climb from both saves and
the player's other 11 saves (load pictures identical on and off) show no
regression.

## Revision 4: the ladder through the gap (user and real display, 2026-11-20)

User: pressing W and S at the saved spot still showed the distant ladder
"through the gaps". It reproduced only on the real display (timing at AUTO
180): a black triangle at the bottom centre with the ladder object of room 60
in it. A watcher during the user's own play (screenshots plus the developer
trace `DNTTK_HOST_VIS_TRACE=1`, each frame's eye, start room, selected rooms
with rectangles, and per room the pieces picked/drawn/left) caught it with the
eye at y 764-958: from the shaft above, room 60's top band around the hatch
(y 896-1088, above the opening's plane) is in plain view, but the opening
itself was off the bottom of the screen, so the walk never reached room 60.
The rule now also takes the room behind an opening when the eye is over the
opening (inside its extent, widened by 128) and within 768 of that room's box.
The same real-display W/S run then shows the hatch's stone in place of the
triangle, with no piece left undrawn in any frame.

Lesson for level design (and the renderer): a room whose box reaches past
its opening's plane (a hatch tube modelled as part of the room below) is seen
without going through the opening. Either the opening should sit at the real
geometric boundary (the top of the tube), or the visibility rule must treat
the overlap as belonging to both rooms, as it now does.

## Limits

- Classic (frozen since 2026-11-01) has no way to add a second room, so in
  the frames where the eye is inside the hatch slab it can still show a thin
  strip of sky beside the opening (3-6% of the bottom band in two shots).
  Polymorph, the default, is clean.
- Offscreen runs on the desktop GPU; the player's real display at 180 is the
  playtest.
