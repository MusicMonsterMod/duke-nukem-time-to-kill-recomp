# D22E - Campaign matrix and evidence baseline

Status: Needs review (documentation only, 2026-10-30). Parent: D22. This note
is the D22 campaign matrix. D22F-D22O fill it in; each fills only its own
columns and records failures as separate jobs. No game runs, builds or code
changes were made for this baseline. Every cell is taken from an existing job,
note or user report, cited below, or is marked unverified.

## Evidence labels

| Label | Meaning |
| --- | --- |
| **U** | User playtest report (date and job cited) |
| **A** | Automated or offscreen run by an agent (job/note cited) |
| **I** | Inferred from code/data research; no run of that behaviour |
| **-** | Unverified |
| n/a | Does not apply to this level |

A cell describes only what its source proves. A load-only arrival does not
prove a level can be completed. Scripted firing in a level does not prove
enemies take damage.

## Level identity and order

Level numbers are the console's internal indices (`level N`, `0x800be570`;
overlay file id `0x1ad + index`, [note 101](101-d26a-level-select.md)).
They are not the campaign's numbering. The authoritative campaign order,
numbering and boss positions are D26D's job and are not established here. The
rows below follow index order. Known facts about the flow (I, [note
179](179-d20b-statistics-screen.md)):

- The statistics screen's exit picks the next level with `0x80027fc0`. A found
  challenge item leads to its challenge stage. Level 29 leads to the ending.
- Levels 27-29 are boss levels: no secrets row, and the boss checks use the
  enemy count ([note 183](183-d20e-statistics-research.md)).
- CHALLENGE FOUND is shown only for levels 0, 2, 5, 7, 9 and 11.
- The campaign ending plays VICTRY, then KILT on difficulty 3, then CREDIT
  (`0x80028178..0x8002819c`, [note 281](281-d18-fmv-reliability.md)).

Out of campaign scope (D22N): challenge stages 21-26, two-player arenas
15-20, and unused or test levels 4, 13, 14, 30 and 31.

## Shared baseline (applies to every row unless a row says otherwise)

- **Load (A, D26A 2026-10-04):** `level N` reached normal play in every one of
  the 21 selectable levels. Each had its own overlay resident (byte-equal to
  the disc apart from the scratch tail), the matching tag and health 100.
  Arrival was checked for a few seconds only.
- **Modernized controls (A + U, D22B 2026-10-04/05):** the control lease
  authenticates all 21 levels against the disc-derived rule. There were zero
  identity refusals in 60 s of scripted walk, run, strafe, turn, jump and fire
  per level. The user accepted D22B after a game-wide `level N` playtest. D22C
  fixed control mode carrying over after the console (U, 2026-10-05).
- **Native code (I + A, D23F, [note 105](105-d23f-fast-timing.md)):** level
  overlay code runs as compiled overlay libraries. The interpreter took about
  5% of the emulation thread on the one measured route (fast timing, slot 9,
  150%). No per-level dispatch or interpreter counters exist yet (D22G).
- **Saves (A, D19Q/D20/D26A/D20B):** SAVE GAME / LOAD GAME savestates work as
  a system. D26A saved in level 1, travelled to level 5 and loaded back into
  level 1 with controls active. D20B saved the level-complete statistics
  screen for level 0, and its load continued to level 1. In Modernized, no
  game save is written to the memory card ([note 179](179-d20b-statistics-screen.md)).
  Per-level round-trips of inventory, health and objectives have not been
  audited (D22K).
- **Movies (I + A, D18, [note 281](281-d18-fmv-reliability.md)):** the disc
  has eight movies, all on the startup route or the campaign ending. No level
  has its own FMV, so per-level movie cells cover in-engine scripted sequences
  only. Startup movies play (A). VICTRY, CREDIT and KILT played fully from a
  private table fixture (A), but the natural ending route has not been reached.
  D18's research is accepted; note 281 leaves listening checks on a real
  audio device open.
- **Statistics handoff (A + U):** D20B (user-accepted) showed level 0, ended
  with end code 4, going through the statistics screen to level 1. D22A (U,
  2026-10-04) took level 0's portal by play into level 1. No other level's
  natural exit is recorded.

## Matrix

Column key: **Load** (arrival), **Handoff** (natural exit to the correct next
level), **Prog** (progression items and gates), **Enemies**, **Weapons**,
**Boss**, **Saves** (per-level round-trip), **Scripted** (in-engine sequences),
**Native** (per-level counters), **Played** (completed by play, start to exit).

| Idx | Name | Load | Handoff | Prog | Enemies | Weapons | Boss | Saves | Scripted | Native | Played |
| ---: | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | TIME TO KILL | A | U (portal, D22A); A (D20B) | I items; U partial (1) | A partial (2) | A partial (2) | n/a | A (D20B) | - | - | U partial (1) |
| 1 | DUKE HILL | A | - | I items | A partial (2) | A partial (2) | n/a | A (D26A) | - | - | U partial (3) |
| 2 | MINER 69ER | A | - | I items (4) | - | - | n/a | - | - | - | - |
| 3 | GOLD AND GUNS | A | - | I items | - | - | n/a | U load (5) | - | - | - |
| 5 | OBEY OR DIE | A | - | I items | - | - | n/a | - | - | - | - |
| 6 | FAMILY JEWELS | A | - | I items; A partial (6) | - | - | n/a | U load (5) | - | - | U reported (7) |
| 7 | RESISTANCE IS FEUDAL | A | - | I items | - | - | n/a | - | - | - | - |
| 8 | HOLY TERROR | A | - | I none | - | - | n/a | - | - | - | - |
| 9 | PIG FACTORY | A | - | I items | - | U partial (8) | n/a | U load (5) | - | - | - |
| 10 | HOG HEAVEN | A | - | I items | - | - | n/a | - | - | - | U partial (9) |
| 11 | LET THE GAMES BEGIN | A | - | I items | - | - | n/a | - | - | - | U partial (10) |
| 12 | BLOOD BATHS | A | - | I none | - | - | n/a | - | - | - | - |
| 27 | THE REAPER | A | - | n/a | - | - | - | - | - | - | - |
| 28 | WING'ED DEATH | A | - | n/a | - | - | - | - | - | - | - |
| 29 | MOLOCH THE GATEKEEPER | A | n/a (ending) | n/a | - | - | - | - | - | - | - |
| - | Ending (VICTRY, KILT, CREDIT) | n/a | - | n/a | n/a | n/a | n/a | n/a | A fixture only (11) | - | - |

In every row, Load stands on the shared D26A/D22B baseline. The **Prog** "I
items" cells use the mission items the game itself names for each level
([note 114](114-d08a5-mission-tracking.md)), listed below. Levels 8, 12 and the
boss levels name no mission items, so D22H still has to check their gates
(switches, doors, scripted triggers) another way.

### Row notes

1. Level 0: the user's first-map pass inserted the security card and entered
   the next area (early project status). D26F's level crystals were
   user-accepted in level 0 (2026-10-07, [note 117](117-d26f-crystals.md)).
   D22A's portal exit was played from a mid-level save, not from the level's
   start.
2. Levels 0 and 1: D22D's energy weapon locked onto and damaged enemy types 3
   and 4 (A, [note 152](152-d22d-energy-beam.md)). No other weapon or enemy
   type has a per-level record. D22B's scripted fire was in every level, but
   it does not show that enemies take damage.
3. Level 1: D22A's arrival had movement, aim, fire, pause and view checks (A,
   U accepted "completely playable", 2026-10-04). Its exit was not recorded.
4. Level 2: D29 records the bank-vault notes as a legibility problem, not a
   blocker.
5. The user loads private saves in these levels (renderer jobs D17W level 3,
   D17X/D17AN level 6, handoff slot 5 LEVEL09). That shows the saves load. It
   does not audit their state.
6. Level 6: one debug-spawned Family Jewel was picked up live, and a spawned
   skeleton key set item 6 (A, note 114).
7. Level 6: "Family Jewels played through" (U, 2026-10-29, paraphrased speech
   to text, D17AM3/AM4 playtest). Whether the level's exit was reached is not
   recorded. D22O should confirm it.
8. Level 9: armed dodge rolls good (U, D22B playtest 2026-10-05). D23D notes
   a minor slowdown in the strip-club area.
9. Level 10: first-person height and costume kick good in the Roman/HOG HEAVEN
   era (U, D22B playtest).
10. Level 11: played "for ages" with nothing wrong (U, 2026-10-29). D22C fixed
    the Legacy-controls start (U, 2026-10-05).
11. Ending movies: full playback from a private movie-table fixture (A, D18).
    The natural route through level 29's completion, and the difficulty-3 KILT
    condition, are unverified.

The user also reported medieval-era play (first-person height and kick good,
D22B; D23C moat/Necro slowdown). The level index for those reports was not
recorded, so they are not placed in a row.

### Mission items per level (I, note 114)

| Idx | Items the game names |
| ---: | --- |
| 0 | Subway Security Key, Transport Room ID, Red/Blue/Green Energy Crystal |
| 1 | Skeleton Key, Scrap of Paper, Old Note, Torn Paper |
| 2 | two Skeleton Keys |
| 3 | Skeleton Key |
| 5 | Warehouse Key, three crystals |
| 6 | two Skeleton Keys, three Family Jewels |
| 7 | Gantry Key, Valve Key |
| 9 | Lab Key, Valve Room Key, three crystals |
| 10 | two Skeleton Keys |
| 11 | Skeleton Key |
| 8, 12, 27-29 | none named |

## Extra modes (D22N scope)

| Mode | Status |
| --- | --- |
| Challenge stages 21-26 | Load A (D26A); statistics-screen variants drawn by rewriting the level index (A, D20B); no music (D18E Todo); completion unverified |
| Two-player split-screen | D34B/D34C closed for now; not part of campaign acceptance |
| Two-player arenas 15-20 | Not reachable by level select; untested |
| Character and difficulty pages | Title pages re-implemented (D19J, user-accepted); difficulty effects on the campaign unverified |

## Known open issues that touch campaign play

None is a known progression blocker. D17AO (slot-11 ceiling hole, level 6)
is Deferred as an authored level gap. D18C (crackle at construction signs and
the train ledge) and D23D (level 9 slowdown) are Todo and do not block
progress. Note 281 leaves D18 listening checks open.

## What the sub-jobs fill

| Job | Columns |
| --- | --- |
| D22F | Load from a new-game route, Handoff for every level |
| D22G | Native |
| D22H | Prog (gates, switches and the item-less levels too) |
| D22I | Enemies, Weapons |
| D22J | Boss (27-29 and any boss fights inside other levels) |
| D22K | Saves |
| D22L | Scripted |
| D22M | Level 29 Handoff and the Ending row |
| D22N | Extra modes table |
| D22O | Played, with a user checklist; closes D22 |

## Gaps this baseline found

- No level other than level 0 has a recorded natural exit.
- No boss fight has any evidence.
- Enemy and weapon evidence exists only for levels 0 and 1, and only for the
  energy weapon.
- No per-level native counters exist.
- Campaign order is still unknown until D26D. D22F and D22O need it to check
  "correct next level", so D26D should be done before or alongside them.
