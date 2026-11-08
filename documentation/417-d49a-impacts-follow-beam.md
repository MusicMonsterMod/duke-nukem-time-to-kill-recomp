# D49A - Impact glow at the drawn beam's end, then every gun's wall hits

2026-11-08. Status: Done (user-accepted 2026-11-08: "its absolutely rock
solid!"). User (D49 playtest): "it's really
working!! ... the only thing that doesnt line up as such as the impact point
lags behind". Follows D49 ([note 416](416-d49-beam-follows-gun.md)).

Before/after (local, not committed): `research/before-after/d49a-impact-follows-beam/`
(`*-before-*.png` with `DNTTK_IMPACT_FOLLOW=0`, `*-after-*.png` default; energy
weapon fast turn right and left, running strafe, still; Gatling fast turn
right and left; same lab run settings, 180 Hz, first person).

## What the glow, sparks and puffs are

D49's lead (records near `0x801c0000+` holding the end, word `0x800606f4`)
was most likely stale stack: `0x800606f4` is a code address inside the
routine that returns at `0x8006076c`, not a record field. The real owners:

- **Effects system.** The beam updater `0x800733C8`, when its line meets
  something, calls `0x8009cd74(0x307, 0x800c5814, beam+0x34, ...)` once every
  four ticks (beam `+0x4c`). `0x8009cd74` -> `0x8009c3fc` takes a 0x54-byte
  record from the pool `*0x800c5894` (count `*0x800c5898`, 64), links it on the
  active list `0x800deb50` (`+0` next, `+4` prev; free list `0x800deb48`) and
  stores type `+8`, angles/timing `+0xc/+0xe/+0x10` (from the table entry),
  position `+0x1c..+0x24` and room `+0x30`. Init switches on `type & 0x3f`
  (table `0x80018810`), draw on the same (table `0x800188a0`, in `0x8009ca2c`,
  called by the camera render at `0x8002e7e4` right after the projectiles
  `0x8006d08c`): most types queue a billboard through the decal queue
  `0x8009b7fc` at `+0x1c`. Update `0x8009c76c` ages `+0x2c` towards `+0xe`,
  then fades the colour alpha `+0x3f` by `+0x38`.
- **Glow:** type 7 (`0x307`), timing entry `0x800c5814` = (12, 36, 500).
  Measured: spawned exactly at the beam's end, lives about four game frames
  (`+0x2c` 128, 394, 660, 926).
- **Sparks:** type 7's init `0x8009c0fc` calls `0x8009bcfc(6, pos, colour,
  0x15e, 10, 10)`: ten bounce-engine particles (`0x8008f150`; pool
  `*0x800c4570`, 0x180 records of 0x80, active list `0x800dd7f0`, free
  `0x800dd7f8`; kind `+8` = `0x04002706`, position `+0x10..+0x18`). They fly
  out and fall for a second or more (hundreds alive while firing).
- **Bullet puffs:** the round wall impact `0x8006f1b8` (a0 the round, a1 the
  point) spawns by round type `0x12` (bullets, entry `0x800c57a4` = (24, 60,
  36), about a second), `0x20`/`0x21` (`0x800c587c`) or `0x1c` (`0x800c586c`)
  at the point. The dark bullet-hole marks are separate wall decals and are
  not moved. The Gatling's `0x05010705` bounce particles are spent casings
  (rounds in flight and casings belong to D49B).

## Change

- `impact_follow.inc` (included by `frame_replay.cpp`): guest memory (reserved
  before the workers fork) holds a scan counter and one 32-byte tag per effect
  slot and per spark slot: tag, scan, shot eye, shot forward x4096. On the
  live game, just before each gameplay frame is published to the redraw
  workers (`capture`), `impact_tags_scan` walks both active lists and finds
  the records that are new since the last scan:
  - a new `0x307` glow within 64 units of the real end of one of Duke's
    unlocked view-aimed beams takes that beam's shot view (D49 record);
  - a new `0x04002706` spark within 800 units of a glow tagged in the same
    scan takes the glow's view;
  - a new `0x12`/`0x20`/`0x21`/`0x1c` puff within 96 units of one of Duke's
    round wall impacts queued this frame takes that round's shot view.
  Every other new record's tag is cleared; a savestate load clears them all.
- `weapon_aim.cpp`: each view-aimed gun shot keeps its muzzle and view (ring
  of 16). A round's first flight sweep (`0x8006dd2c`) starts at its muzzle
  (`+0x14`), which binds the pool slot to the shot; its wall impact
  (`0x8006f1b8`) queues the point with that view (`aim_take_impacts`), once.
  Other owners, other origins and Vanilla queue nothing.
- Worker (`worker_draw`, the D49 flag 16: late first-person redraw with
  `beamfollow` on): `impacts_follow_view` moves each tagged record in the
  worker's copy by the same view change as D49's beams, `q -> eye + Rl^T Rs
  (q - shot eye)`, blended by age in game frames since the tag: glows always
  (they live four frames); sparks and puffs fully for 3 frames, then fading
  over 5 back onto their real path, so a lingering spark or puff settles
  where it really is.
- Drawing only: hits, damage, decals, the game's records, D41B's lights and
  D22D locks are unchanged. Locked beams (end on an actor) are not tagged, as
  D49 keeps their end on the actor. 60 Hz, third person and Vanilla draw no
  such redraws.
- A/B: `DNTTK_IMPACT_FOLLOW=0` at launch (developer); console `beamfollow
  off` turns beams and impacts off together. Tuning (developer):
  `DNTTK_IMPACT_SPARK_HOLD` (frames, default 3), `DNTTK_IMPACT_SPARK_FADE`
  (default 5). `DNTTK_BEAM_FOLLOW_TRACE=1` logs each moved record and its
  weight. Debug port `render_replay`: `plugin.impact` counters (glows, sparks,
  glow_misses, round_hits, puffs).

## Evidence

Lab `recomp/analysis/d49a/lab.py` (local), private copy of UI slot 8 (internal
7, energy weapon drawn), offscreen, first person.

| Check | Result |
| --- | --- |
| Glow tagging, 180 Hz, near wall, still/turns/strafe | 226 glows tagged, 0 misses; about 1000 sparks |
| Same, corridor wall, 180 Hz / 120 Hz | 180 / 92 glows, 0 misses |
| Energy fast turn right / left (screenshots) | before: glow off the beam's end, behind the turn; after: glow on the drawn end at the crosshair |
| Slow turn, strafe, still | after matches the drawn beam; still unchanged |
| Gatling fast turns | 21 of 21 wall impacts (aim trace) queued and tagged; newest puff on the crosshair, older ones settle where they hit |
| Pistol / shotgun fast turns | 12 of 12 / 30 of 30 wall impacts tagged |

`ttk-aim-test` (new D49A check: impact carries the shot's view once; foreign
rounds, other origins and Vanilla refused) and `ttk-input-test` pass.

## Not changed / limits

- **Superseded for bullet guns (D49D, 2026-11-08).** Puff following for the
  Gatling, pistol and shotgun (and the round impact queue in `weapon_aim.cpp`)
  was removed: bullet smoke and holes stay together at the real hit, like
  EDuke32. Glow and sparks of the energy beam still follow.
  [Note 418](418-d49d-holes-with-smoke.md).

- **Lights.** D41B's impact lights (`0x8006f1b8` sparks and beam light) stay
  at the real hit point: they light the wall where it was hit. The user
  accepted the result without asking for them to follow.
- **Bullet holes** are wall marks at the real hit point and stay there.
- **Lingering sparks and puffs** slide back to their real place over about
  0.15 s after the first 0.1 s; tuning is by the two developer variables.
- Other effects at a hit (blood on actors, explosions) are not covered;
  rounds in flight and casings are D49B.
- Accepted on the user's display (AUTO rate, first person). The user's next
  observation, that the beam itself may look animated at a lower rate than
  the game, is D49C.
