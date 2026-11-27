# D08A52 - A steady hand with the Holy Hand Grenade

2026-11-27. Status: **Done, user-accepted 2026-11-27** ("lock it in and close it out"). User, 2026-11-26: "when the holy
handgrenade is picked, dukes hand/the grenade kinda shakes, we encountered
stuff like that before. i'd say fix that". Evidence labels: S source (SLUS-00583
read-only disassembly, recomp source), L private runtime (offscreen lab), U
user report. The first-person rules of the D36 audit
([note 257](257-d36-first-person-presentation-audit.md)) apply; the closest
earlier case is D48's Gatling support hand ([note 423](423-d48-gatling-steady-hand.md)).

## Two causes

### 1. The grenade was drawn twice (both views)

L: the three throwables (Pipe Bomb 12, Holy Hand Grenade 13, Dynamite 14)
share one draw clip (upper 40, keys 2400-2402; backwards it is their holster)
and one hold clip (39, a single key 2484). Since D08A54 the grenade is `6 6`.
From the Desert Eagle, `6 6` played clip 40 forward, backward, forward: up,
down, up, then the hold. Dynamite (`6 6 6`) went up and down twice. A single
`6` to the Pipe Bomb raised it once.

S/L: per game frame, with the request flag `+0x224 & 4`, the bytes
`+0x3b8..+0x3bb` and the D08A37 queue state:

1. The first `6` writes `+0x3ba = 12` and flag 4. The pistol holsters.
2. The original takes the request when the holster ends: it clears flag 4
   (a single `6`: cleared at field 18, the draw starts at field 30 from
   `+0x3ba`).
3. The second `6` arrived while the pistol was going away. The D08A37 queue
   took it over and served it as soon as the hands were empty, writing
   `+0x3ba = 13` and flag 4 again: a second request.
4. The original drew the grenade (`+0x3ba` read at the draw start). Its hold
   handler (`0x8004dea4`, grenade case `0x8004e180`) starts the holster (clip
   40 backwards) whenever flag 4 is set, so the freshly drawn grenade went
   away and the same grenade was drawn again.

From the Gatling the holster runs straight into the next draw (no empty-hands
phase), so the queue instead waited for the Pipe Bomb's hold and made a full
change from it to the grenade: the same up, down, up.

Change (`recomp/src/ttk/shortcuts.inc`, Modernized only): a new group /
previous / next / last choice that takes over our own request while the
original still holds it, either not yet taken (flag 4 still set: the old
weapon is going away) or taken with its draw still to come (hands empty),
only changes `+0x3ba`. The original reads that byte when the holster ends
and again when the draw starts, so it draws the newest choice once. Once a
draw has begun (equipment 2, the drawn weapon set), a new choice waits for
the ready loop and makes one normal change, as before. Counter
`weapon_switch.retargets` in the debug `ttk_input` reply. Vanilla has no
queue (number keys are Modernized only).

### 2. The held grenade flickered (first person)

L: the present trace (`DNTTK_PRESENT_TRACE=1`, `weapon_hand`) while holding
still: the Desert Eagle (seeded ready pose) shows 1 position over 720
redraws at 180 Hz; the grenade 6 positions within about 2 view units, with
about 16% of redraws stepping 2-7 px at 2560 wide. The throwables have no
ready-pose seed, so first person showed the live skeleton's hand relative to
the torso, and its integer and GTE rounding through the arm chain changes
from tick to tick (and between ticks in the redraws) although clip 39 is one
key.

Change (`recomp/src/ttk/first_person.inc`, Modernized only): slots 12-14 take
the hand from the right arm's animation channel standing too, the D08K10
rebuild already used for them when crouching or swimming (clip quaternions
and the model's joint offsets, slerped by the clip timer; redraw workers
blend the published per-tick hand, D17AY). The hold is then exact, and the
draw and throw keep their motion. The channel hand sits within 1.5 view
units of the live hand's mean, so the grip and placement are unchanged.
Developer A/B: `DNTTK_FP_THROW_HAND=live` keeps the live skeleton hand (not
saved).

Third person draws the native skeleton and is unchanged by this part; cause
1's fix applies there too.

## Evidence

Lab `recomp/analysis/d08a52/` (local, ignored): `probe.py` (from the D08A51
probe; cases `flags6`, `hold`, `look6`, `hhg`), `sweep.sh`, `idlepos.py`.
Private card copies of UI slot 2 (level 6), the grenade spawned with the
console, offscreen on the GTX 1080 Ti at 180 Hz; the player's saves and
profile hash-checked unchanged after every run.

Taps from the Desert Eagle, gap in 60 Hz fields between `6` presses ("dips"
= backward clip-40 runs, i.e. a holster of a drawn throwable):

| Taps | Before | After |
| --- | --- | --- |
| `6 6`, gap 4 / 8 / 12 / 20 | 1 dip each, hold at field 66-70 | 0 dips, hold at 43-47 |
| `6 6`, gap 30 (the Pipe Bomb's draw has begun) | 1 change | 1 change (unchanged) |
| `6 6 6`, gap 8 / 12 | 2 dips | 0 dips, Dynamite drawn once |
| `6 6` from the Gatling, gap 8 | up, down, up | 0 dips, hold at field 31 |
| `6 6`, third person, gap 8 | - | 0 dips |
| `1 1` (Mighty Boot to Throwing Knife) | - | one draw |
| `6` from the Pipe Bomb in hand | 1 change | 1 change (unchanged) |

Present trace, `6 6` window (view units per game frame): boundary mean 26.6
to 10.5, peak 164 to 115. Hold: 6 positions to 1, level, looking down and
looking up. Throw frames match the live hand's frame for frame. Pipe Bomb
and Dynamite holds: 1 position each.

Native: new `ttk-controls-test` group D08A52 (rest, holster, empty hands,
begun draw, ready loop); the D08K10 group's standing expectations for 12-14
now expect the channel hand. 168 groups pass; `ttk-near-test` and
`ttk-input-test` pass; Python 292 OK (2 skipped).

Pictures (local): `research/before-after/D08A52/` - `hand-trace.png` (hand
height through `6 6` and while holding, before and after), hold and throw
frames.

## Limits

- Expected, not a defect (user, 2026-11-27: "we are literally cycling
  through them! that is truly expected functionality"): a tap that lands after
  a draw has begun gets one normal change. From the Gatling (short holster,
  the next draw starts about 17 fields after the first `6`) a third tap of
  `6 6 6` at an ordinary pace lands there, so the grenade comes up and Dynamite
  follows.
- Between ticks, a redraw past the next tick holds the channel hand at that
  tick's pose (the D17AY channel blend, as when crouching), so a draw moves
  in small 2-3 unit steps at some tick boundaries instead of extrapolating.
- Offscreen runs; the real display at the player's rate is the playtest.
- Other weapons are D08A53's review; their quick double taps go through the
  same retarget.

Candidate executable (dev build) SHA256:
`af6390199c75c9a44d60e4c124099e307ee5de8da65b2b5818df7a76d194fc26`.
