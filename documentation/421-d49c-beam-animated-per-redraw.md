# D49C - The energy beam animates at the display rate

2026-11-08. Status: Done (user-accepted 2026-11-08: "it's amazing work! lock it in and close it out"). User report (D49A acceptance): "the beam
itself looks like it's a lower framerate than the game ... it looks like it's
animated at a lower framerate than how we've got the rest of the game to run
at our high auto value". Dependencies: D49.

Before/after (local, not committed):
`research/before-after/d49c-beam-animated-per-redraw/` (`before-*.png` with
`DNTTK_BEAM_ANIM=0`, `after-*.png` default; still frames, the motion itself
only shows live).

## Cause (real, not imagined)

The beam draw `0x80072868` animates the beam itself, each time it runs:

- The streak is `+0x1c` strands (3 for Duke's energy weapon), each a wave
  over N segments (short `0x800c3b3c`, 10). A strand's phase starts at the
  record's `+0x40` (low half); every segment adds `4096/N`, and each strand
  then adds `2*(4096/N)` to `+0x40`. One draw advances the phase by
  `3 * 2 * 409 = 2454` of a 4096 turn (the same as -1642: seen at the game's
  rate the wave flows from the gun toward the end).
- The stripe colours (green/white gradients, primitive `0x32`/`0x3a`) swap by
  the parity of the draw count `+0x58`, incremented each draw.
- `+0x5a` (strand ring angle) gains `3 * 1365 = 4095` per draw, -1 of a turn:
  no visible change.

The constructor `0x80073220` seeds `+0x40`/`+0x5a` (128 apart for successive
beams) and clears `+0x58`; the updater `0x800733C8` never touches them.

The game draws once per game frame (about 30 Hz in play). At 120/180 Hz every
present is a redraw, and each redraw job reloads the published frame's memory
(`render_worker.c`: `render_replay_image_load` per job), so all redraws of
one game frame drew the same wave and stripes. Measured on a private slot 8
copy at 180 Hz: `+0x58` and `+0x40` change once per game frame (about every
33 ms, +2454 each); the beam's position follows every present (D49) but its
shape changed in steps of six presents.

## Change

- `frame_replay.cpp`: a gameplay redraw job carries `beam_t`, the redraw's
  time in game frames past the drawn list's frame (`alpha` for the older
  list, `alpha - 1` for D10H8's newer list; past 1 under extrapolation), with
  flag 32. Before the composition runs, `beams_animate` moves every live
  beam (pool `0x800da978`, type 15, any owner) in the worker's copy only:
  `+0x40` by `beam_t` times the per-draw advance taken the short way round
  the turn, `+0x5a` likewise, and the draw count `+0x58` by `floor(beam_t)`
  so the stripes change on the same clock as the wave. Continuous across
  game frames (checked in the trace: equal phase mod 4096 at the boundary).
- Drawing only. The game's beam record, updater, hits, damage, D22D locks and
  D49/D49A placement are untouched. 60 Hz and Vanilla have no redraws and
  are unchanged. Third person gets the same smoothing (its redraws repeated
  the same look too).
- A/B: backtick console `beamanim [on|off|default]` (session only),
  `DNTTK_BEAM_ANIM=0` at launch. `DNTTK_BEAM_ANIM_TRACE=1` (developer) logs
  each beam's phase and count per redraw.

The stripe swap stays a swap at the game's frame rate (about 15 flashes a
second, as on the console); only the wave is continuous. Swapping per present
would turn the stripes into an even blend at 180 Hz. If the stripes still
read as low frame rate, that is the next thing to try with the user.

## Evidence

Lab `recomp/analysis/d49c/lab.py` (local), private copy of UI slot 8
(internal 7, energy weapon drawn), offscreen.

| Run | Redraw times seen (`beam_t`) | Result |
| --- | --- | --- |
| 180 Hz first person, still | steps of 0.167 from -0.95 to +0.05 | phase moves about 274 per present instead of 2454 every sixth |
| 180 Hz first person, turning dx=20 | -0.98 .. +0.47 | every redraw its own phase |
| 120 Hz first person | -0.85 .. +0.35 in about 0.25 steps | same |
| 180 Hz third person (older list) | 0.006 .. 0.965 | same |
| `DNTTK_BEAM_ANIM=0` | none | old behaviour |

Captures (`recomp/analysis/d49c/shots/`, local): beam shape and colours as
before in still frames. `ttk-aim-test` and `ttk-input-test` pass.

## Limits

- Accepted on the user's playtest; the stripe swap stays at the game rate.
- Other effects drawn by game code that advances per draw (if any) are not
  covered; only the energy beam was reported.
