# D44A - Earthquakes (Duke Nukem 3D style)

2026-11-05. Status: Done (user-accepted 2026-11-05, revision 2). Research:
[note 396](396-d44-earthquake-research.md). Target (user): "make that camera
shake really look 1:1 like the one in eduke".

Evidence labels: **S** source read, **M** measured in a private run, **D**
static disassembly of SLUS_005.83.

## What the player gets

Modernized with OPTIONS > GAMEPLAY > EARTHQUAKES on (the default): a TNT barrel
or an explosive-vest bomb starts Duke 3D's earthquake. One global countdown of
16 tics (0.53 s); a new source sets it, never adds, so a barrel chain is one
long earthquake. While it runs, every image moves the
eye 21 units up or down, flipping each tic, and turns it 1.41 degrees right for
two tics of four. Full strength to the last tic, no fade, no easing, the same
everywhere on the map. Rockets, grenades, pipe bombs, the Holy Hand Grenade and
dynamite keep TTK's native shake and cause no earthquake (as in Duke 3D).
Console `earthquake <seconds>`, `earthquake` (16 tics), `earthquake 0` (stop).
EARTHQUAKES off restores the native jitter for barrels and vests. Vanilla is
unchanged.

## The pattern (S, M)

EDuke32 `G_DrawRooms` (`game.cpp:1058`), per drawn frame while
`g_earthquakeTime` t > 0 and `on_ground`:

```
camera z   += 256 - ((t & 1) << 9)     // even t: down, odd t: up
camera ang += (2 - (t & 2)) << 2       // +8 of 2048 when (t & 2) == 0
```

TTK uses the same formula on the same t (`earthquake.inc`, `earthquake_compose`):

- **Tic**: t is the remaining countdown in Duke 3D tics. A start composes
  t = 16 at the next live image; each later game frame lowers it by the frame
  step `0x800d21fc` / 10 (1 at 30 fps, 1.5 at 20 fps), so it lasts 0.53 s of
  game time at any frame rate. It does not run while paused.
- **Bob**: Duke 3D moves 256 of its 38<<8 eye height (1/38). TTK's standing
  eye is 797 units above the floor (M: slot 9, eye y 1251, floor `+0x1c8`
  2048), so the bob is 21 units. Game Y grows downward, as Build z does.
- **Yaw**: 8 of 2048, 1.41 degrees, about the world's vertical; positive
  turns right (TTK yaw: +Z 0, +X 1024).
- **Everywhere** (revision 2, user, 2026-11-05: "the only problem however, is
  that the effect stops if you jump"): Duke 3D shakes only `on_ground`; the
  first build matched that (mode `+0x22c` 0 or 1, no jetpack, mantle, ladder
  mount or climb). Now jumps, falls, swimming, flight and climbs shake too.
- **Pad**: EDuke32 rumbles both motors at t << 8 of 65535 each frame
  (`I_AddForceFeedback`, `input.h:84`). TTK raises the pad's motor byte to t
  of 255 through the same rules as the native latch `0x8001e97c` (port
  `player+0x233`, vibration enabled `0x800c3f9c & 0x30`, actuator
  `pad+0x216`, byte `pad+0x228` at `0x800d1610 + 0x22c * port`). Unverified:
  no pad in the test runs.

## Sources (D, M)

The native blast shake request `0x80028a2c(camera, position, strength, time)`
is hooked (new entry hook; one generated line, codegen hash `0xebce51ea`
unchanged). Its five callers, by return address:

| Return | Routine | Earthquake |
| --- | --- | --- |
| `0x800973e0` | TNT barrel blast `0x80097090` | 16 tics |
| `0x80064170` | explosive vest `0x80063f0c` | 16 tics |
| `0x80070474` | rockets and grenades `0x800702bc` | no, native shake |
| `0x80070930` | pipe bombs, Holy Hand Grenade `0x800707cc` | no, native shake |
| `0x80071090` | dynamite `0x80070f7c` | no, native shake |

`0x80063f0c` has one caller, `0x80064fa8`: a fuse that beeps (sound `0x9004`)
while the character's `+0x1f0` counts down and detonates at 0x800 or less. So
only vest bombers reach it (type 1044 in slots 9 and 11), not ordinary
enemies killed by blasts. Both call sites are checked to still be
`jal 0x80028a2c` (`0x0c00a28b`) before the hook acts.

For the two earthquake sources the native jitter is replaced: the request's
camera argument is pointed at a scratch copy of the camera (Expansion 1), so
its distance falloff and timers run on the copy and the real camera's `+0x48`
/ `+0x4a` stay 0 (M: no non-zero sample in any barrel or vest run). With the
option off they are untouched.

## Presentation only (frame_replay.cpp)

The camera `0x800d6eb0`: rotation `+0` (Q12 rows: right, up, forward in world
coordinates), eye `+0x14`, view matrix `+0x20` (the rows scaled), view
translation `+0x34` (M: equal to the eye, a world position), anchor `+0x64`
(eye plus forward times half the distance `+0x42`).

- **Live image**: at the composition entry (`0x80026164` from `0x800268f4`)
  the offset is fixed first (`earthquake_compose`), so the render state saved
  with the capture carries it; after the capture the shaken camera is written
  (`quake_begin`): rows turned about world Y, the anchor turned about the eye,
  eye, view translation and anchor moved by dy.
- **Back to steady**: at the composition's chain close (`0x8001fba0` from
  `0x80026904`), each field group that is still ours gets the steady value.
  The composition re-derives the eye and view translation from the anchor
  (`0x80039c7c`, +-1 rounding), so values within 2 units of the shaken eye
  count as ours (M: 62 of 62 images restored; before that rule the view
  translation kept the previous image's shaken value). A fallback at the
  camera update entry `0x8003ade4` runs the same restore. After an
  earthquake the camera fields equal the ones before it (M, Duke idle).
- **Redraws** (120/144/165/180/240 Hz): the worker applies its base frame's
  offset after the interpolated and late camera are written, just before the
  composition. The shot's view (D10G3) is recorded before that, so aim never
  sees the shake. The offset is held, never blended between game frames: at
  60, 120 and 180 Hz every presented image showed one of the four exact
  states (M).
- **First person**: Duke's own draw (the arms, the weapon, the legs) gets the
  steady camera back at its entry and the shaken one at the list step after
  it (`replay_quake_camera`, beside D14H's body camera; in a late redraw the
  body camera keeps owning the rotation, the world rotation is turned).

## Evidence (M)

Private runs (`recomp/analysis/d44a-earthquake/probe.py`, local): copied cards,
offscreen, debug port, `DNTTK_QUAKE_TRACE=1`. Player saves and profile were
hash-checked unchanged after every run.

| Case | Result |
| --- | --- |
| Console `earthquake`, slot 9 | 16 images, t 16..1, dy +21 / -21 flipping, yaw 1.41, 0, 0, 1.41 ... (EDuke32's pattern) |
| `earthquake 1` / `earthquake 5` / `earthquake 0` | 30 images; 150 tics running; stopped at once |
| Slot 9 (UI 10) TNT barrels, third and first person | 8 barrels, one 49-tic earthquake, t restarting at 16 at each link (as EDuke32's `bigrumble`); native shake 0 |
| Slot 11 (UI 12) vest pig | vest type 1044, 16 tics; native shake 0 |
| RPG at a wall | native shake (`0x80070474`), no earthquake; barrels it set off quake |
| Jump during `earthquake 3` | first build: 13 airborne images steady; revision 2: all 32 images shaken, tics consecutive through the jump |
| Savestate load during an earthquake | stopped (140 tics left before the load, 0 after) |
| EARTHQUAKES off | native barrel shake (13 non-zero samples), no earthquake |
| Vanilla | console refuses; no earthquake |
| Image shift, third person, 180 Hz | yaw twitch 23 px (1024 wide), far wall dy +-2 px, after: identical image |
| Image shift, first person, 60/120/180 Hz | yaw 10-11 px, near floor dy 5-8 px; weapon region unchanged (mean diff 1.3-1.5 of 255) |

The third-person view's narrower field of view makes the same 1.41 degree
twitch a larger share of the screen than in first person; the lens is far
from the walls there, so the bob reads smaller. Both are the same physical
motion as first person.

Native `ttk-controls-test` 153 PASS (stubs for the two new replay functions),
`ttk-aim-test` passes, profile tests 129 OK. Executable
`023672fbdf83dfe4f364f7e184673be56bed0bdcce368f8149d237d32c262d64` (first build); revision 2 `0badb77b72aca5dc355c0e66f893b650edc971e66e49f5fb156022cbe2d17a72`.

## Option

Profile schema 68 adds `earthquakes` (`on` default, `off`). `run.py
--earthquakes VALUE` saves it, terminal `--settings` choice Z, in game
OPTIONS > GAMEPLAY > EARTHQUAKES (live). The launcher passes
`DNTTK_EARTHQUAKES` (always `off` for Vanilla).

## Limits and open items

- Accepted by the user's playtest (revision 2: "ok, i absolutely accept
  this"), after asking for the shake to continue in the air.
- The vest uses one detonation (16 tics); longer is a one-line change if
  wanted (note 396 left it to the user).
- Pad rumble is EDuke32's strength (t of 255, at most 16), which is faint;
  not felt in testing (no pad).
- Scripted level events (collapses, crashes, Duke 3D's SE 2 analogue) are not
  sources yet: D44B.
- In first person, an effect drawn as its own object at the weapon (not part
  of Duke's draw) shakes with the world; none was seen in the test scenes.
