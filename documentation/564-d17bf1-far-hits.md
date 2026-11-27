# D17BF1, D17BF2, D17BF3 - far hits: round range, more enemy slots, sleeping enemies wake

Modernized with Extended draw distance only. Vanilla and the `original` draw
distance are unchanged (checked: cap 12, no round stretched, no wake).
Code: `recomp/src/ttk/far_hits.inc` (new), `recomp/src/ttk/enemy_reach.inc`.
One new hook, `0x8009934C` (one generated line, codegen hash `0xebce51ea`
unchanged, savestates load).

User, 2026-11-27, after a far Necro in UI slot 2 "just doesnt quite hit":
"we need to do that ... i totally see that we do need to make the extended
enemy wake when hit stuff work ... there is no issue with doing something like
this in eduke ... Doom, where these slaughter maps with hundreds if not
thousands of enemies play out so smoothly ... it's foundational".

## 1. Why far enemies could not be hit

UI slot 2 (level 6, row 0) has a real D17AQ dormant Necro 34,900 units from
Duke. With aim within 0.3 degrees, every round hit at 31,974 units or nearer
and none at 32,986 or farther (private copies, offscreen).

Rounds move in `0x8006ed5c`. Each update adds the step (`+0x46` = speed `+0x44`
times the frame scale `0x800d21fc`) to the travel counter `+0x40`, and the
round ends once the counter passes its kind's range: word `+0x1c` of the
projectile type table `0x800c4bc0` (64-byte entries, kind byte `+0x30`).

| Range | Kinds |
| --- | --- |
| 32767 | bullets (most kinds) |
| 25000 | kinds 4, 11, 12, 15-23, 25 |
| 65536 | kind 30 |
| 3072 | kinds 14, 26 (flames) |
| 1 | kind 10 |

Nothing else uses the counter: every falloff distance (`+0x20`) is 0, so
damage is always the far value `+0x0c`. The rocket mover `0x80070568` and the
mover `0x8006ddb8` have no range check, only the life `+0x32`. The other
`+0x1c` reads near these routines are a round's own previous position.

In the original no live enemy existed past the spawn radius (fade start +
0x13d0, about 25,500 in level 6), so the limit never showed. D17AQ keeps real
enemies out to the extended view limit (0xffff), past it.

## 2. Round range (D17BF2)

The round sweep `0x8006dd2c` (hooked since D10H2) is called from
`0x8006ed5c` at `0x8006efb8` (ra `0x8006efc0`), after that update's range
check. For Duke's own round (`+0x10` = `0x800d7198`) of a kind whose range is
at least 25000, `far_stretch` takes back the part of this update's step that
the far range does not need: the counter grows by `step * range / far` per
update, so the kind's own range is used up over `far` = the extended view
limit times 1.16 (a target at the edge of a 60-degree half view is
1/cos 30 farther than the view depth): 76,021 in level 6. Speed, spread,
damage and the hit test are the original's; enemy rounds and flames keep
theirs. No game data is written. Code identity: the range check and its sweep
call (`far_guards`).

Result: the slot 2 Necro at 34,900 went 8000 -> 7063 on the first well-aimed
round from where the user stands.

## 3. More enemy slots (D17BF3)

The cap is the size of the enemy pool. The level load (`0x800242b0`, at
`0x800246d4`-`0x80024804`) allocates from its bump heap (`0x8002b8bc`,
current `0x800d2698`, end `0x800d269c`) `count * 0x2cc` bytes of actors
(`*0x800ddb04`), a pointer table and one 0x544-byte skeleton per slot
(`*0x800ddb08`); `count` is the halfword `0x800c56b4[level]` (12 for most
levels; 20, 10, 18, 16 for some; 1 for the arenas). The spawner setup
`0x8009934c` (single caller, `0x800246c4`, just before the allocation) sets
the cap `0x800c56a8` from the same entry, and the creation's free-slot search
(`0x80096614`) reads it again as its bound. Those are the table's only three
readers. The creation `0x80098f3c` does not check the cap itself.

New hook at `0x8009934c` (`reach_slots_setup`): for the level being loaded the
entry is written as the level's own count plus 8 (`DNTTK_ENEMY_SLOTS`, 0-16)
in Modernized extended, and as the own count otherwise, so a Vanilla or
`original` launch always loads the original pool, even from a Modernized
savestate. Pool, cap and search bound always agree for the whole level.
Arena levels (one slot) are left alone. Growth needs 100 KB of heap free
before the pool beyond the extra slots (the rest of the load takes 63-85 KB
after the pool in levels 1-12).

Heap free after load (8 more slots, 2,068 bytes each):

| Level | Slots | Free before (own pool) | Free after (grown) |
| --- | --- | --- | --- |
| 1 | 28 (own 20) | - | 23,388 |
| 3 | 20 | 49,184 | 32,640 |
| 6 | 20 | 36,200 | 19,656 |
| 8 | 20 | 44,464 | 27,920 |
| 10 | 20 | - | 27,588 |
| 12 | 20 | - | 94,576 |
| 21 | 18 (own 10) | - | 377,632 |
| 23 | 24 (own 16) | - | 377,736 |

Level 6 grown (the tightest) kept exactly 19,656 free through fighting, the
pause menu, the inventory and a quicksave (`heap_low` in the debug JSON, the
lowest free heap seen at a spawner tick): the level makes no allocation
during play in those paths. Not covered: death and continue, cutscenes,
every level.

**Combat count kept.** `reach_tick`: with more slots than the level's own
count, at most that many enemies the player has not hurt are active inside
the original radius; those already active stay so while inside it (sticky,
`reach_engaged`), free places go nearest first, and the rest wait dormant
(`waiting`). Level 6 fresh: 20 live, 9 engaged, 11 dormant.

## 4. Sleeping enemies wake when hit (D17BF1)

`far_wake`, at the same sweep entry, for any of Duke's projectiles (rounds
from `0x8006ed5c`, rockets from `0x80070568`, the mover `0x8006ddb8`): the
path this update (previous position `+0x14` to `+0x04`) is tested against each
stand-in in the sleeping table: a column from the stand-in's position up two
bounding radii (`+0x5a`, at least 256) and half a radius down, reach 1.25
radii. A path within reach creates the real enemy first, with the spawner's
own creation `0x80098f3c` on the copy's row; the original sweep that follows
meets it with its own model test, walls and damage. A near miss leaves an
ordinary dormant enemy.

- At the cap the farthest enemy that is dormant, not hurt, not dying (actor
  `+0` bits 1-2), not held by a script (row `+0x16`) gives way through the
  removal `0x80098d70`; if none can, the copy sleeps on (`full`).
- `reach_note_created` lists the new enemy at once with its health at
  creation (damage in the same update still wakes it) and 30 ticks of grace.
  The pool reuses the slot of the enemy that gave way, so the entry for that
  address is replaced, not kept (the first version kept it: the new enemy was
  seen as an unhurt far one, replaced at the next tick and created again).
- An enemy the player has hurt (or within its grace) is never removed: the
  range removal keeps it (`spared`), and when the original replacement would
  remove it, the farthest enemy that may give way goes instead if it is
  farther than the new spawner (`yielded`), else the spawner waits.

Test (UI slot 1, level 9, room 25; row 18, type 95, a copy 22,620 units away
in clear view; lab: the original radius squared `0x800dd9e0` set to 1000^2 so
live enemies are dormant and can give way): the first shot created it (one
wake, one gave way) and hit it (to 3126), the next two killed it; row flags
`0x4`, the HUD kill count 6/30 -> 7/30. Before the grace fix the same shot
created it twice.

## 5. Finding copies in view

`farhits probe` (developer) runs the game's segment query (`0x8006d980`,
`atlas_segment_query`) from Duke's eye to each copy and logs
`row:distance:clear`. From the standing spot of every room of levels 0, 2, 3,
6 and 9 (about 350 rooms, 12 slots), one spot saw a copy (level 9 room 25),
and one more turned up in level 3 in a second pass (room 48, rows at 37k and
50k, a state that depended on the route). With 20 slots, fresh loads of
levels 3 and 9 had no copies at all near the tested rooms: every placed enemy
within reach was a real enemy. Copies in direct view are rare; they matter on
the security monitors and in very crowded or very far areas.

## 6. Debug and developer

- Debug JSON (`ttk_input`, draw distance, enemies): `spared`, `yielded`,
  `waiting`, `engaged`, `slots {grown, restored, refused, heap_low}`,
  `far_hits {range, wake, far_range, rounds, stretched, tests, wakes, yield,
  full, fail, refusals, probe_clear}`.
- `DNTTK_ROUND_RANGE=original`, `DNTTK_FAR_WAKE=0`, `DNTTK_ENEMY_SLOTS=0..16`
  (default 8); console `farhits on|off|default|probe`.
- Log: `[TTK enemy slots]` per level load, `[TTK far hits] wake ...`,
  `[TTK far hits] created enemy ... gone within its grace` (should not appear).
- Lab scripts: `recomp/analysis/d17bf1/` (local).

## 7. Accepted, and not done

User, 2026-11-27: "its working. that's genuinely amazing. the test passed playing my slot 2 from far away. lock it in and close it out". Remaining items are D17BF4 and D17BF5.


- Blasts (`0x800a9bf8`) and flames (`0x800a9ee0`) that land near a copy
  without their projectile passing it; kick, beam lock and auto-aim on copies.
  A rocket whose flight passes a copy does wake it.
- Performance with 20 live enemies at 180 Hz not measured (the extra ones are
  dormant: their class handler is skipped).
- Heap headroom in levels not loaded here, death and continue, cutscenes.
- Far more enemies (the "slaughter map" direction) needs a pool outside the
  game heap; this job grows it inside the heap.
