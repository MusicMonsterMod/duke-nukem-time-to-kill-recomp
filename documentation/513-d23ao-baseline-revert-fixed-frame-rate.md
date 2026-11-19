# D23AO: back to the 18 November build, and a frame rate that never changes

Date: 2026-11-19. Jobs: D23AO (this note), D23AP (objects in the room they
stand in), D23AQ (the struggle points, next). D23AL and D23AN cancelled.

## 1. What happened

On 2026-11-19 D23AL (the hidden-draw rule) and D23AN (reprojected in-between
pictures) were built and tested on the field laptop (i7-5600U, 2 cores and 4
threads, HD 5500, 60 Hz panel). The warp raised the pictures a second there,
but each user test found a new visible problem: the level 6 sky flicker, the
smeared screen edges when turning, the edges again after the first fix, then
judder and edge artefacts on slot 12's run and jump to the ledge. After five
fix attempts the user asked for the work to be reverted: "maybe we revert and
just see how it's looking for now. And then carefully add features back in".

The test build was ef6c866 (D23AG, the overnight chain the user confirmed on
the morning of 18 November), built beside the main checkout on the laptop.
User: "yeah it plays well, not glitchy". Features then came back one at a
time, each played on the laptop:

1. The slot 12 ammo fix (D23AP), alone.
2. D23AK, the lighter redraw worker (a139fa3, accepted on 18 November).
3. The frame rate never changes (D23AO). User: "it should just never change
   framerate", then "we maintain frame rate. let the game struggle if it needs
   to. we should be focussed on optimizing those struggle points".

User, after the session: "honestly look at that. i didnt wanna stop playing.
how good was that session!!!", then "let's acually lock this work in ... can we
now make this main".

## 2. What main is now

- Code: a139fa3's `recomp/src`, `recomp/tools`, `recomp/tests` and framework
  patch (byte-identical to a139fa3's), plus D23AP's `object_room.inc` port and
  D23AO's two launch defaults. The code of 592a75f (D23AI's developer GPU pass
  timing `PSX_GPU_PHASES`, `DNTTK_SLEEP_VIEW_TEST`, `DNTTK_OBJ_RECT_STATS`) is
  not on main; D23AI's measurements stand (note 511).
- The D23AL/D23AN work (code and note 512) is kept on the local branch
  `shelved/d23al-d23an` (not pushed), for reference only.
- Documentation, the job board and the logs keep everything (nothing
  rewritten), with this revert recorded.

## 3. D23AO: the frame rate never changes

`run.py` (Modernized, overridable from the environment) adds to the D23AK
redraw bundle:

- `DNTTK_REPLAY_FLOOR=0`: the governor never falls back to the original
  presentation (about 30 pictures a second). Before, three slow seconds at the
  last step dropped to it, for 15 s (D23AK) or 120 s (the 18 November build),
  and the retry waited for clean seconds: in the user's first test session on
  the laptop it stayed at 30 for minutes.
- `DNTTK_REPLAY_SHED=0`: no step to every 2nd or 3rd refresh on 120 Hz and
  faster screens.

AUTO's launch-time choice by processor threads (D23P) is unchanged: it is a
fixed choice, not a change during play. Where the computer cannot keep up the
game itself runs a little slow for a moment instead. Vanilla is unchanged
(the original presentation always).

## 4. D23AP: objects in the room they stand in

The fix found during D23AL's first laptop test (note 512 section 4 on the
shelved branch): in the user's UI slot 12 (GOLD AND GUNS) the ammo box by the
mine door has room byte 1 (the room beyond the doorway) while it lies on
this side; the renderer culls it against room 1's rectangle (`0x80031584`),
which shrinks to the doorway above the box as Duke steps closer, so the box
vanished while in view, in every build. Fix (`recomp/src/ttk/object_room.inc`,
Modernized with EXTENDED draw distance, developer `DNTTK_OBJECT_ROOM=0` off):
at the object pass entries `0x80031d10` and `0x80032e78` (after
frame_replay's `actor_draw`), when the object's position lies inside the box
of other rooms the host shows, the renderer gets a copy of its room's list
entry whose rectangle also covers those rooms' rectangles (clamped to the
guard-banded view); host depth still hides what lies behind their walls. The
copy is written to 16 bytes of mod memory reserved in `render_state_prepare`
(before the redraw workers fork), so live frames and redraws use it alike.
`host_visibility.inc` keeps each shown room's rectangle and the camera record
for it. The D23AL port measured the box drawn in 3,245 of 3,289 frames (851
before); the user tested this port on the laptop.

## 5. The playtest (field laptop, real screen, BALANCED, FRAME RATE 60)

Session log `play-20261119-113030.log` (laptop, 18 November copy): about four
minutes of level 3 (the user's slot 0), first person.

- Every whole minute 60.0 presents a second (the first 56.3, with the load);
  no governor change, no first-person drop, no crash.
- The game fell behind real time 11 times, to 52.7-57.2 frames a second
  (88-95% speed) for a few seconds; presents stayed at 60 except twice (56,
  58), and one audio underrun (buffer at 22 ms).
- Process memory 558-602 MB (638 MB on the 18 November build alone).

The struggle points (Duke's position, game frames a second) are D23AQ's work:

| Game frames/s | Presents/s | Duke (x y z) | Notes |
| --- | --- | --- | --- |
| 52.7 | 56 | 6222 4613 2305 | one audio underrun |
| 56.2 | 60 | 6319 4615 2535 | same area |
| 53.6, 55.1 | 60 | 31372 9938 27158 | |
| 53.7 | 58 | 23275 6647 55386 | |
| 53.9 | 60 | -21221 7430 46762 | |
| 56.6 | 59 | -15764 9425 55857 | |
| 56.6 | 60 | 4874 3043 -1348 | |
| 56.8 | 60 | 27402 487 -41882 | |
| 57.2 | 60 | 9941 9286 66597 | |

## 6. Also seen (not changed)

- First person dropped to the original third-person camera a few times while
  swimming underwater in the level 6 moat with Double Duke active (first
  test session, `[TTK lease] inactive (unsupported-camera) anim=127/128
  state=5/5`): the lease refused swim poses there. Not reproduced in the next
  session; the swim lease code is the same in every build since before
  18 November. Double Duke only doubles Duke's damage (+0x89e timer).
- Lessons from D23AL/D23AN: judge presentation changes by the player's
  actions on the real screen (run, jump, fast turns) before measuring
  pictures a second; a screen recorder on the laptop drops frames, so motion
  needs every presented frame (the shelved branch's `PSX_GL_PRESENT_SEQ`).
