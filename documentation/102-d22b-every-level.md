# D22B - Modernized controls and first person in every level

**Accepted (2026-10-05, user: "Accepted.").** A game-wide playtest followed:
height and costume kick good in the medieval and Roman/HOG HEAVEN eras, armed
rolls good in Level 9; follow-ups D08T2, D08U1, D22C, D23C, D23D, D26B,
D26C, D26D and a D17R update are on the board. Earlier status: needs playtest
(2026-10-04). Modernized controls and the selected first- or
third-person view now authenticate in all 21 levels the original level select
offers, and carry over through the natural transitions. Two original moves the
lease never covered, dodge rolls and steep-slope slides, now keep the mouse
camera and the view, and the original's own back-steps and strafes are taken
over by the lease, and unowned jumps keep the mouse camera in third person
too, instead of handing Duke back to the classic controls or camera.
Candidate executable SHA-256
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017`.

## Report

User, accepting D26A: "the modern controls dont carry over, and first person
etc." Earlier direction: "There should basically be no situation where the
classic controls override modern controls ideally."

Baseline survey (accepted D26A build, private card copy, `level N` for every
level, 25 s of play each): LEVEL00 and LEVEL01 keep the lease. In the other 19
levels the lease refuses on arrival and stays refused
(`[TTK identity] level overlay tag 0x00000005 at 0x800ca968 is not an
authenticated level`), first person reports `lease`, and only the original pad
moves Duke.

## Level coverage: one rule from the disc

Every LEVELxx.OVR on the owned disc loads at `0x800ca968` and has the same
layout: its tag word, strings and jump tables, the code (ending at its last
`jr $ra`), then one data block that the code addresses with `lui` pairs. In all
25 level overlays the first data reference is exactly the code end. The level
writes only a few bytes at the very end of that data block:

| Level | Written tail | What writes it |
| --- | --- | --- |
| LEVEL00, 01, 03 | 16 bytes | hit-position vector passed to `0x8007177c` |
| LEVEL09 | 20 bytes | hit vector (`0x800cdb74`) plus a word it stores |
| LEVEL05, 27 | last word | the level's own `sw` |
| LEVEL10 | last halfword | the level's own `sh` |
| all others | none | - |

The baseline survey's RAM diffs agree: the only changed overlay bytes were
LEVEL01's vector, LEVEL09's and LEVEL27's last word, LEVEL10's last halfword
and LEVEL00's existing apartment pair. Static stores also explain the LEVEL05
and LEVEL09 tails the short survey did not reach.

`recomp/tools/local/level_overlay_guards.py` derives the guard for each
selectable level from the owned disc: it checks that layout, fails if a level
writes inside its code, references data inside its code, or leaves unwritten
bytes inside its written tail, and authenticates everything before the tail
(tag, strings, tables, code and read-only data). It reproduces the accepted
LEVEL00 and LEVEL01 digests exactly. `--check` compares
`src/ttk/control_guards.inc` with the disc.

`control_guards.inc` `level_overlays[]` now lists the 21 selectable levels
(tags 3-0xf, 0x13-0x1b). Nothing else in the lease changed: the tag picks the
one body that must match, a known tag over another level's bytes fails
closed, and LEVEL04/13/14/30 and the two-player arenas stay unauthenticated
(they are not reachable in normal one-player play). The apartment pair and
conveniences remain LEVEL00 only.

## State coverage

The candidate survey (all 21 levels, 60 s of scripted Modernized play each:
walk, run, strafe, turn, jump, fire) recorded every sample where the ground
lease was not ready. Nearly all were the existing camera-only states (jump
96/97/98/103/104, landings 94/95/105, unowned falls 107/108), where the mouse
camera and first person stay live. Two original moves and one original gait family were missing:

**Dodge rolls, 157-162.** Seen armed in PIG FACTORY and THE REAPER: 157 and
160 launch in the air (mode 9), 158/159 and 161/162 tumble and recover on the
ground (mode 0). Each roll dropped the camera lease and first person for about
a second. Worse, with A/D or S still held the generic bridge fed the original
strafe/Down pads as the roll ended, so the original started its own strafe
(90/91) or back-step (84) outside the lease.

**Steep-slope slide, mode 2.** The airborne/directional handler `0x80055904`
sets mode 2 (anim 143 or 145) when `0x800544e8` reports a slope too steep to
stand on (`0x80055fb4` is the only store of mode 2 in the executable). Duke
leaves the slope as 99 or 108 (mode 9) and lands as 105. FAMILY JEWELS trace:
jump 96, 145 (2/9), 99 (9/9), 105 (0/0), idle, with the lease lost from 145 to
idle.

**The original's own directional gait, 82-85 and 88-93.** The idle dispatcher
(`0x8004c1dc..0x8004c2d4`) picks 82/83 (walk back), 84/85 (run back) or
86/87 (mid water) for Down, and L2/R2 pick the strafes 88-91 (92/93 in mid
water); a Vanilla pad probe confirmed L2 = 91 and R2 = 90. Modernized never asks
for them, but the D08V fall bridge and the first roll fix fed A/D/S as
original pads, so a key held as a fall, roll or slide ended started them
(84/85 after falls in HOG HEAVEN and FAMILY JEWELS, 90/91 after rolls), and
the classic controls owned Duke until the key was released; a jump from there
was not owned either (96/97/105 refusals).

Both are now "committed" original moves (`modern_controls.cpp`
`roll_state_early`, `slide_state_early`, `committed_move_early`): the
camera-only lease stays live (mouse camera, independent aiming view), first
person stays active (`first_person.inc`), and `pc_input.cpp` sends no
directional pads while one runs (`committed_camera_ready`). The original owns
the motion to its end; Duke then returns to idle or gait and the full lease
resumes WASD. A latch carries the slide through its 105 landing only.
Before D22B, WASD sent nothing during a slide either (move actions have no
original pad); only the camera and view were lost.

The directional gait is now part of the lease (`original_step_anim`, a set
separate from `land_gait_anim`, so the ladder-top, ledge-reach and push
features are unchanged): in mode 0/0 the full lease takes over the frame it
appears (it sends Up and the camera-relative facing; the original dispatcher
switches to forward gait), its first frames after a landing or slide keep the
camera-only lease, and the mid-water forms count only in shallow water.

**Unowned jumps in third person.** A jump the host did not start (Space-only,
or taken in the first frames after a landing, and the 105 landing after an
unowned fall) has no flight lease. D11 kept the camera-only lease through such
jumps only in first person ("Third person is unchanged"), so in third person
the orbit dropped to the original camera for the jump: 16-18 of 156 samples
per level in the third-person survey, and in first person the same gap
appeared when the landing before the jump was itself unleased (LET THE GAMES
BEGIN). The continuity (`jump_camera`, formerly `eye_jump`) now applies in both
views: a jump animation that follows a live camera lease keeps the camera-only
lease through takeoff, flight and landing. Motion, takeoff and landing stay
original; the ground movement lease still ends at 96 (the D08C takeoff test now
checks exactly that). The landing poses 94/95 in their first frame after a fall
(0/9) were also unleased, so a jump taken during them had no camera to
continue (LEVEL00 trace: 96, 94 0/9 refused, 94, 96), and the heavier landing
106 (BLOOD BATHS) was in no list at all. The landing poses 94/95/106
(`landing_pose_anim`) now keep the camera-only lease, including their first
frame after a fall or slide.

## Transitions

The lease re-authenticates every frame and is keyed on the resident tag, so
any path that loads an authenticated level returns Modernized control by
itself:

| Path | Result |
| --- | --- |
| `level N` travel (D26A), all 21 levels in one session | lease ready and first person on arrival in every level, zero identity refusals |
| Level complete (end code 4) through the statistics screen into the next level: 1-2, 2-3, 9-10, 12-29, 21-1, 27-5 | ready, first person, movement and mouse look in the new level |
| Savestate saved in PIG FACTORY, travel to WING'ED DEATH, load | back in PIG FACTORY, moving with mouse look |
| Pause menu (Start) and resume | lease off while paused, back on after |
| Portal LEVEL00 to LEVEL01 | unchanged from D22A (accepted) |

## Remaining original-control or original-view states

- **Swimming, modes 4/5 (BLOOD BATHS):** the Modernized swim controls and
  mouse camera own it (D08M/D08O), but first person deliberately shows the
  orbit underwater (`first_person.inc`: swim modes are unsupported). Follow-up
  D11E.
- **Mantles, hangs, pull-ups and unowned falls:** by D08V design, the orbit
  view with the mouse camera; the original owns the motion. Unchanged.
- **Death and Continue:** not exercised in a level other than LEVEL00 (no
  reliable offscreen death source: writing health 0 does not kill Duke, and
  enemies did not reach him in two minutes). Continue reloads through the same
  mode 1 init as `level N` travel and the statistics screen, which are
  verified. User playtest.
- **Natural level exits:** the statistics-screen path was entered by writing
  the level-complete end code, not by finishing each level's real exit.
- **Two-player arenas and the unselectable LEVEL04/13/14/30:** not
  authenticated; they fail closed with a `[TTK identity]` line.
- **Occasional jump camera gaps:** on the final build, one first-person
  CHALLENGE STAGE 2 jump (3 of 232 samples) and, in one third-person run,
  jump chains in PIG FACTORY and THE REAPER (15 and 10 of 156 samples) began
  without camera continuity (`[TTK lease] inactive (state) anim=96/97/105`).
  A rerun of the same third-person survey and targeted traces showed none.
  The cause is not established; CHALLENGE STAGE 2 also shows stale-camera
  idle frames (D17I scheduling).
- A lease refusal in normal play is still logged with its reason
  (`[TTK lease]` / `[TTK identity]`).

## Evidence

Native: `ttk-controls-test EXE LEVEL00_FIXTURE LEVEL01.OVR LEVELS_DIR` (34
groups). The new fifth argument is a directory of the owned LEVELxx.OVR files
(extracted locally, never committed): each of the 21 levels authenticates by
its own tag and body, tolerates arbitrary writes to its tail, refuses a change
to its last authenticated word and the previous level's tag over its body,
gets the apartment pair only on LEVEL00; LEVEL04/13/14/30 refuse. A D22B
roll/slide group checks every observed animation/mode pair keeps the
camera-only lease with no locomotion lease, the slide latch ends at idle, the
back-steps and strafes take the full lease (camera-only on landing frames, mid
water forms only in water), and swim/other/Vanilla stay out. `ttk-input-test` checks a committed move sends no
directions. `ttk-aim-test`, `ttk-near-test` and the Python suite pass
(`test_level_overlay_guards.py` includes the owned-disc `--check`).

Offscreen, final candidate (60 s of scripted play per level; samples every
0.25 s):

| Run | Result |
| --- | --- |
| First person, all 21 levels | zero `[TTK identity]` lines; first person `active` on 4,751 of 4,800 samples, `unsupported` (swim, D08V falls/mantles: the orbit by design) 46, `lease` 3 (one jump, below) |
| Third person, levels 2/6/9/12/27 (rerun) | camera lease live on every sample; zero refusals |
| Transitions | statistics screen into the next level for 1-2, 2-3, 9-10, 12-29, 21-1, 27-5; savestate across levels; pause/resume: Modernized and first person back by themselves, zero refusals |
| LEVEL00 slots 0-11 | unchanged (slots 8/9 are the existing ladder/jetpack orbit states) |
| Vanilla, levels 2/9/12/27/0 | travel works; no Modernized hook activity (0 moves, cameras, looks) |

Offscreen evidence lives in `recomp/analysis/d22b-20261004/` (private card
copy, port 9247; ignored local data).

No hook, generated C, framework file or hashed header changed; the framework
patch is unchanged and existing savestates still load.
