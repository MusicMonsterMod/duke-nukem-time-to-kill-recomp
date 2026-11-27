# D08J11 - A one-armed full hang on a ladder while armed

2026-11-27. User-accepted: candidate 2, side, is the default (user: "hangpose 2 is the one, yeah i like it. lets lock it in"). Done (closed out: "close it out").

## The request

User, 2026-11-27, accepting D08J10: "review duke's pose when hanging from the
ladder - rather than hang, he immediately pulls up. this can look jarring if
you start shooting because he's doing half a pull up while shooting enemies!
so my proposal is that during the hang phase, duke's position reverts to a
full hang with one arm if the pistol is drawn. this will need some fine
tuning so we will progressively iterate until we find the one that works, so
maybe we build out 3 poses and i choose the best one of the 3."

## Measured

Duke's clips (`recomp/analysis/d08j11/hands.py`, positions from the pelvis,
up is negative y). Channel 1 (the gun arm) drives rot 13-15, channel 2 (the
off arm, Duke's left) rot 10-12.

| Pose | Off hand above the pelvis | Off shoulder | Gun hand |
| --- | --- | --- | --- |
| Climb rest 186 | 358, 131 forward (elbow bent) | 172 | 140 |
| Climb rest 187 | 58 (a rung at the hip) | 173 | 319 |
| Ledge hang 149 | 425, at the body (full stretch) | 176 | 425 |

After a catch below the rungs (D08J10) the legs hang (`ladder_dangle`) while
the upper body keeps the climb. An armed climb rests (D08J6A), and after the
catch's first step that is 187, with the off hand down at the hip: the "half
pull-up" while shooting. Offscreen at the user's spot (second alley ladder,
standing jump with E, then a fire tap): catch 147, 188, rest 187 armed, the
legs hanging throughout.

## Change

New `recomp/src/ttk/ladder_armed_hang.inc` (included before `climb_aim.inc`),
hooked at 0x800411b8 after the dangle pose, and in the D08J6B build at
0x800b42ec. Modernized, LADDER CLIMBING MODERN, third person, the D08J6B aim
lease (sidearm out, view aiming), while the dangle lasts:

- **Off arm onto a high grip.** The off arm takes 186's channel-2 frame 0 as
  its pose, so from 187 the free hand moves up onto the rung the gun hand let
  go of.
- **Body offset.** The whole model is offset in the build's root (as the
  D08J6F swing): down and toward the ladder along Duke's heading. The D08J6B
  off-hand IK takes the offset back out of the hand's contact, so the hand
  stays on its rung, the arm straightens and the body hangs below it. Duke's
  position, collision and camera never change.
- **Never stretched.** Each solve reports the drop at which the hand is at
  full reach (.98 of the arm) from that build's shoulder; the drop is capped
  there. Measured: the IK's stretch stays at 1.000-1.006 (it was 1.07-1.12
  with a fixed drop), drops of 30-49 depending on the chest's turn toward the
  view.
- **The grip is held.** The contact follows the posed hand while the hang
  eases in, is frozen in the world once fully in (idle and firing moved it up
  to 40 units otherwise), and eases back to the posed hand on the way out.
- **Ease.** In over 220 ms once the sidearm is out and settled; out through
  an equip or stow track (upper 6/7: the W stow plays 6, then the weapon state
  goes 0 and the lease with it). Past the lease the build keeps applying the
  fading offset without the IK, so it never snaps (measured: 1.0 to 0 over 13
  fields, finished before the next climb step 189). It ends at once off the
  climb clips, off the ladder, once the legs reach the rungs, or in first
  person.

## The three candidates

Switched live with the console (`hangpose 1|2|3|off`, also by name) or
`DNTTK_LADDER_ARMED_HANG=hang|side|pull|off`. The user chose 2, side, which
is the default; the others stay switchable for tuning.

| | Name | Drop | Toward the ladder | Turn toward the aim |
| --- | --- | --- | --- | --- |
| 1 | hang | up to 60 (full reach) | 70 | none |
| 2 | side | up to 60 (full reach) | 60 | 0.6 of the view's yaw, 75 degrees right to 30 left, through the D08J6F swing about the grip |
| 3 | pull | 25 (elbow bent) | 90 | none |

Candidate 2 combines with the D08J6F law (which swings only past 90 degrees):
the larger of the two. The chest turn and the gun arc take the rest of the
view as before. `hangpose tune <drop> <forward> <side>` overrides the numbers
for the session (developer tuning), `hangpose tune off` clears it.

## Measured (after)

| View right of heading | Candidate | Drop | Swing (rad) | IK stretch |
| --- | --- | --- | --- | --- |
| 0 | 1 hang | 47-49 | 0 | 1.002-1.004 |
| 0 | 2 side | 46-47 | 0.01 | 1.000-1.001 |
| 0 | 3 pull | 25 | 0 | 1.000 |
| -35 | 1 hang | 30 | 0 | 1.000 |
| -35 | 2 side | 37-40 | -0.36 | 1.000-1.006 |
| -35 | 3 pull | 25 | 0 | 1.000 |
| +40 (earlier run) | 2 side | 46-49 | 0.40 | 1.001-1.003 |

Offscreen frames (`recomp/analysis/d08j11/shots/`, local) show the free arm
straight up on a rung with the body lower (1, 2) or a bent-arm pull (3), idle
and firing. The +40 and +70 views could not be framed in the alley (the
camera ends inside the wall and Duke fades); the side-on candidate there is
the user's playtest.

## Tests

- `ttk-controls-test`: the D08J11 group, inside the D08J10 dangle (a real
  hang below a high ladder): holstered stays 0; another caller of
  0x800411b8 is ignored; armed eases in (0.45 after 100 ms, 1 after 300 ms);
  a stow track eases out; `hangpose 3`, `side`, `tune`, `tune off`, `off`;
  off the climb clips it ends at once. The whole suite passes (162 groups),
  including D08J6B/D08J6F, whose build code it shares.
- `python3 -m unittest discover -s recomp/tests/local`: 286 tests OK.

## Not changed

The unarmed hang (D08J10), the ledge-hang legs, the climb, D08J6/D08J6B
firing and aim on the rungs once the legs are on them, first person
(D08J6C), LADDER CLIMBING CLASSIC and Vanilla (the dangle and the lease are
Modernized and modern climbing only).

## Limits

- The user chose side on the real display. The side views to Duke's right
  were never framed offscreen (the alley is too narrow).
- During the 220 ms ease-out after the weapon state goes 0 the IK no longer
  holds the hand, so it moves with the body back into the climb pose.
- The offset (at most 60 down and 90 toward the ladder) moves only the drawn
  model; the shot code is unchanged. Whether a muzzle flash or tracer start
  sits visibly off the offset gun was not measured.

Private probes and captures: `recomp/analysis/d08j11/` (local).
