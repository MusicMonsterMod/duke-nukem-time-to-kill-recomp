# D17BM - The power switch panel always shows lit

Status: **Done, user-accepted and closed out 2026-11-20** ("i can confirm thats now fixed!", "lock us in and close us out"). Job board: D17BM. Owner code:
`recomp/src/ttk/security_monitor.cpp` (D53's keypad and power sign glow,
[note 432](432-d53-security-monitors.md) revision 5).

## Report

User, 2026-11-20 (UI slot 12, TIME TO KILL, level 0): "this panel sometimes
shows weirdly dimmed like this, rather than lit." Then, selecting the job:
"i cant always replicate this one unfortunately, but how about we just make
sure that interactive wall panel stays on. i wanted the player to enter that
room and be greeted with the lit electricity symbol ... it doesnt need to be
animated or anything, lets just focus on actually owning that panel so it
presents well".

## The panel

The wall power switch of LEVEL00 (Duke at (-7577, -6643, 51063) in the
user's save, saved 17:33, copy in `research/savestates/d17bm/`, local):

| Object | Type | Model | What |
| --- | --- | --- | --- |
| 0x801dd364 (table slot 38) | 477 | 0x80142980 off, 0x801429e0 on | the keypad, one 256x192 quad; E sets +0x35 to 1 and swaps the model |
| 0x801d9384 (room list only) | 481 | 0x801305ec | the lightning sign above it, one 256x260 quad, never animated by the game |

Since D53 revision 5 both are drawn self-lit in Modernized Polymorph (a copy
of the quad whose corners take one fixed colour), and a blue-white light
(0.38, 0.62, 1.0) in front of them lights the wall. Without the self-lit
copy the sign is lit as an ordinary prop by that light: the blue disc stays
bright and the yellow bolt turns dark olive, the keypad's buttons lavender.

## Cause (measured)

- The save's thumbnail (the presented image at save time,
  `gl_renderer_grab_presented_argb`) shows exactly that: bolt about
  (66, 64, 6), disc (38, 59, 151).
- D53 chose the glowing models in its prop scan (game thread, at a level
  change, a savestate load and every 300 frames) and kept them in fixed
  host-side slots. The scan skipped a mesh already listed from the last scan
  and then cleared the slots it had not filled: every second scan emptied
  them, so the panel was unlit for one scan period in two.
- Redraw workers (`render_worker.c`) are forked copies of the game process
  and keep the host state they were forked with. They draw most presented
  images at high frame rates, so what the player saw depended on when the
  workers were last forked (a level or overlay load, a mod memory allocation,
  a worker replaced after a deadline): lit for a long time, or unlit for a
  long time. That is why it came and went and could not be reproduced on
  demand.
- Reproduced deterministically with only live frames (`frame_rate original`,
  no redraws), the old executable, the user's save: bright bolt pixels in 60
  shots 20 frames apart: lit for 16 shots, **0 for 23 shots** (about 460
  frames), lit again. The unlit frames match the thumbnail
  (`research/before-after/D17BM/thumb-oldunlit-oldlit-new.png`: thumbnail,
  old unlit, old lit, new).

## Fix (Modernized Polymorph, game-wide)

- The keypad and the sign are recognised by their models when drawn
  (`keypad_mesh`, `sign_mesh`: vertex and face list data), in the game
  thread and in every redraw worker alike. No host list, no scan dependency.
  The keypad's switched-on model matches too, so the panel stays lit before
  and after it is used.
- The sign is steady at 255 (the most a vertex colour gives, twice the
  neutral 128), like the keypads; its breath and crackle (170..255) are gone.
  Its wall light is steady at the pulse's former peak (strength 1.8, radius
  1200). Developer: `DNTTK_POWER_SIGN_GLOW=64..255`, `DNTTK_POWER_LIGHT`.
- The glow block (two 0x80 slots in mod memory) is allocated on the first
  live Modernized frame, before redraw workers fork, not at the first draw
  of a keypad (a worker cannot allocate it).
- Signs are no longer required to stand within 600 of a usable keypad to
  glow; the wall light still is. A survey of the retail levels found two
  signs (LEVEL00, 226 from its keypad; LEVEL05, 277), both by keypads, so no
  retail sign changes; a custom level's lone power sign would glow as well.

Vanilla and Classic: unchanged (the swap and the allocation are Modernized
Polymorph only). Vanilla shots of the save are pixel-identical to the old
executable.

## Evidence

Lab `recomp/analysis/d17bm/` (local, trimmed to `probe.py` at closeout; from D57F, private card
copies, offscreen on the real GPU, the player's saves and profile
hash-checked unchanged by every run).

- Live frames only, the save, 60 shots 20 frames apart, bright bolt pixels:
  old executable lit/0/lit as above; new executable 1250 in all 60 (steady:
  the pulse is gone).
- 180 Hz (redraw workers), new executable: before use 1244 in 10 of 10
  shots; E at the keypad (+0x35 1, model 0x801429e0), then 30 of 30 shots
  lit (1339, the view from 1100).
- Vanilla, the same case on the old and new executables: 0 pixels differ.
- `python3 -m unittest discover -s recomp/tests/local`: 268 tests OK (2
  skipped).
- Shots: `research/before-after/D17BM/` (local): `before-unlit.png`,
  `after-lit.png`, `thumb-oldunlit-oldlit-new.png`.

Not done: a check on the real display (the change is colour only, the same
self-lit path as before; the pulse removal is visible there).
