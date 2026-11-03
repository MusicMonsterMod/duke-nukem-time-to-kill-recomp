# D08G7 - dnkroz feel: GOD MODE text, red flash and pain grunt on OFF

Job: [D08G7](../MODERNIZATION_JOBS.md). Status: Done (accepted as preliminary
2026-11-03). The flash moved to the D39 framework
([note 370](370-d39-screen-flash-framework.md)) in D39D: on Polymorph it is
`screen_flash(32, 32, 0, 0)`; the quads below remain Classic's only.
`dnkroz` and `dncornholio` stay the same cheat (`recomp/src/ttk/cheat_codes.h`).

## Reference (Duke3D / EDuke32)

Measured frame by frame from the user's EDuke32 recording
`research/screencaps/dnkroz-dncornholio.mp4` (60 fps, mean red
dominance R - (G+B)/2 per frame, three OFF toggles):

| Phase | Measured |
| --- | --- |
| Onset | instant: baseline 9.6 to 66.3 between two frames |
| Peak | about a 50% blend toward red: G and B x0.47, R x0.47 + 63 (video levels); the HUD and quote text are tinted too |
| Hold | about 2 game tics (67 ms) at the peak |
| Fade | linear, one step per 30 Hz tic (pairs of identical 60 fps frames), to nothing after about 32 tics; 1.07-1.13 s in all |
| Sound | a short pain grunt about 0.2 s after the flash on 2 of 3 toggles |
| ON | text only, no tint |

So: a sharp rise and a slower, straight-line fall, as the user guessed. The
grunt rule matches the Duke3D player script: a hit at full health plays the
pain sound on `ifrnd 128` (half the time). The cheat restores full health and
leaves a zero-damage "hit" pending, which the player actor handles as a hit:
hence the flash and the grunt. EDuke32 itself was not driven: the recording
answered the timing question.

## TTK has no hit tint

The original damage handler `0x800a40a8` (D08A13 table) gives no screen
tint. Per hit it rumbles the pad (`0x8001e840`, `0x8001e8a4`, `0x8001e97c`)
and, if Duke lives, plays a pain sound through `0x8006b270(id, pos, 0)`:

| Damage type | Sound |
| --- | --- |
| 11 (fall damage, from `0x80054da8`) | `0x200a` |
| 12 (toxic gas) | `0x1c004`, 10% |
| Duke in mode 5 (under water) | `0x1c000`, 15% |
| anything else (shots, fire, acid) | `0x2009 + rand % 2` |

The byte `[actor+0x40]+2 = 255` the handler writes is the hit-part index of
the collision record (readers at `0x80063dd8`, `0x800a3520`), not a tint.
Offscreen screenshots of Duke being shot in slot 3 show no colour change.

## Change

- `cheats.inc`: the toggle reads `GOD MODE: ON` / `GOD MODE: OFF` (D08G1
  style, all capitals like the other confirmations). OFF calls
  `god_mode_off_feel`: the flash starts and, half the time (host random, so
  the game's own random sequence is untouched), one of the ordinary-hit
  grunts `0x2009` / `0x200a` plays at Duke's position through `0x8006b270`
  (`cheat_call` gains a plain a1). No damage, no rumble, no health or armour
  change. ON shows neither.
- The flash level is in frame-delta units (300 a second, `0x800d21fc`):
  starts at 340, drawn at min(level, 320)/320 of the peak, so it holds two
  30 Hz ticks and then fades over 32. `screen_flash_tick` steps it once per
  player update by the frame delta (same duration at 20 fps; paused, it waits).
- The level lives in the spare word after the D20C region mirror (mod memory,
  tag `0x5d087000`): savestates and high-refresh redraw workers carry it, and
  an old state's word reads 0.
- `near_clip.cpp` `near_clip_screen_flash_draw`, from the D35J5 freeze-tint
  packet: two full-width halves (widescreen margin included) of a
  subtractive quad (semi-transparency 2: green and blue down, 44 at the peak)
  then an additive one (1: red up, 40), each preceded by a degenerate textured
  triangle that sets the blend, and the prior page restored after. The PS1 GPU
  has no variable blend, so this is the closest fade; the amounts were fitted
  to Duke3D's measured half-red mix on a TTK street frame (mean error about 11
  levels at the peak).
- Placement: appended to ordering-table slot 0 at `0x8001fc44(0)` (ra
  `0x800265c4`), the moment the view chain holding the world and the status
  bar is linked. At the chain close `0x8001fba0` (ra `0x80026904`) it was
  never visible; at the status bar entry `0x8008ba30` it went under the HUD.
- The packet has a fixed 192-byte slot at the end of each region's base part,
  excluded from every cursor allocation. Taken from the cursor it was
  allocated before the props and actors and shrank what the world pass leaves
  them: a prop that no longer fitted went back to the original renderer,
  drawn dark (black triangles across the top of the slot 3 street).
- Modernized only (`input_modernized()`); cheats are Modernized-only anyway.

## Evidence

Lab `recomp/analysis/d08g7-kroz-feel/` (ignored: `probe.py` from D08G8,
`tdis.py`, private card copies, Xvfb). Executable
`1cf2a7008a8533051fe814df22572356f9f9632b0c7cede67d3acfa4807bc225`.

- Third person, 60 Hz, slot 3: OFF at level 330 gives mean RGB (104, 22, 14)
  against (64, 55, 41) without; level steps 340, 300, 280, ... 0 in about
  1.1 s; world, Duke and status bar tinted; the god-mode text stays readable.
  ON: no change.
- 180 Hz (redraws): the flash shows in presented redraws and fades the same.
- 40 OFF attempts: 21 toggled off, 9 grunts (43%); `0x2009` and `0x200a` both heard in the log.
- Slot 4 (calm): health 10000 and armour 2428 unchanged across every OFF.
- Vanilla: the codes do nothing, no flash.
- Suites: `ttk-near-test` (new D08G7 group: slot 0 tail after the HUD, empty
  slot 0, fixed slot leaves the cursor, page restored, level scaling),
  `ttk-controls-test` 125 groups, `ttk-input-test`; Python 223, 1 failure
  (`test_primitive_trace`, failing before this job), 2 skipped.
  No framework change, no hook list or generated code change.

## Limits

- The fade is additive/subtractive, not Duke3D's proportional palette mix:
  very bright pixels keep more of their colour (blue jeans read purple at the
  peak), very dark ones go a little deeper red.
- The host-drawn Modernized inventory box (D08A5) and the OSD text are not
  tinted; Duke3D tints its quote text.
- First person was requested in one run but the private profile came up in
  third person; first person rests on the same chain (no first-person-specific
  path), not on a capture.
- Seen while testing, not caused by this job: at 180 Hz the slot 3 street
  shows black triangles across the top for the first seconds after a load,
  with no cheat typed (distant prop budget fallback).
- User's playtest (2026-11-03): "a great start, but the fade isnt smooth
  enough, it kind of staggers". Cause: the level steps once per 30 Hz (20 Hz
  on overrun) game tick and 180 Hz redraws repeat each step. Not refined
  here: the user chose to own flashes in the renderer instead (D39).
