# D61A - Cut-content hunt: what Time to Kill left on the disc

Status: **Done 2026-11-26** (selected by the user: "work autonomously on
all this"; closed: "lock it in and close it out"). Built on D61's atlas and goto ([note 559](559-d61-level-atlas.md)).

Everything here is derived from the owned disc and stays local: extracted
files, sounds, speech, photographs and RAM dumps are under `research/d61a/`
(untracked). This note records addresses, counts and hashes only.

## Evidence labels

| Label | Meaning |
| --- | --- |
| **disc** | read from the disc's files (static) |
| **code** | read from the executable or an overlay's disassembly (static) |
| **memory** | read from the game's RAM in a level (offscreen run of the recomp) |
| **seen** | photographed in the recomp |
| **original** | checked on the original game (DuckStation portable copy) |
| **online** | already described in a public source |

The recomp runs the original's code, so **memory** and **seen** results are the
original's behaviour unless a Modernized feature touches them (named where it
does). The key RAM facts were also read on the original game (section
"Checked on the original game").

## Headline finds

1. **132 unused Duke voice lines, about 324 s of speech** (code, disc).
2. **Commando Duke**: the hidden Modern 4 level's Duke wears the two-player
   COMMANDO DUKE costume (memory, code, seen).
3. **A fourth boss**: string 235 `BOSS 4` sits in the level-name slot of
   index 30, the nameless empty room, whose folder carries the boss levels'
   own overlay (code, disc).
4. **A quadruped creature brain** (`qped.c`) survives only in an overlay no
   level loads (disc).
5. **The hidden levels' sound tables are older**: Modern 4, the animation test,
   the empty room and the sound test have no gadget sound group at all, which
   is why the jetpack is silent there; True Grit has the group but every entry
   plays one placeholder blip (disc, code).
6. **Four movie-replay cheats and the full pad-cheat table** (code).
7. **Six sound samples no table references anywhere**, and 30 samples found
   only in hidden levels (disc).

Details follow.

## 1. Duke's voice: 132 lines no code plays (code, disc)

Found by a static pass over the executable and every overlay (no game launched).

- **Files.** `/SOUND/DISM01-30.IDF` and `/SOUND2/DISM31-50.IDF`: XA file 1,
  eight interleaved channels, coding `0x01` (stereo, 37.8 kHz, 4-bit; the two
  sides are bit-identical in all 400 streams). One channel of one file is one
  line.
- **Player.** `0x8006cad8(line)` (nothing while `0x800d73c4` is 5) calls
  `0x8003c000(line)`. It refuses while a line is playing (`0x800c26a8` 6) and
  skips a line already played unless the line's flag bit 1 (repeatable) is set
  (bit 0 = played, cleared by `0x8003c4cc`). Then: mute, Setfilter file 1 /
  channel, Setmode `0xc8`, SeekL, and the end position in `0x800d7174`.
- **Table.** `0x800c0dac`, 16 bytes a line: flags, channel 0-7, start, end.
  Lines 1-396 (entry 0 is -1). Positions are stored as LBA minus 23; the base
  `0x800d7178` is set to `0x17` at `0x8001b548`. That is why no file position
  appears in the executable. Every start is the first sector of a DISM file.
  The four slots not in the table (DISM03 channels 1-2, DISM05 7-8) are
  digital silence.
- **Callers.** 40 call sites in the executable, 34 in overlays. A second
  entry, `0x8003b9f0`, plays a queued line from `0x800c2704`, which nothing
  writes.

| Lines | Count |
| --- | --- |
| A constant at a call site | 231 |
| Only through a random or counted index (each range read in the disassembly) | 33 |
| **No reference found** | **132** (about 324 s) |

- **Whole files unused:** DISM08, DISM17 and DISM30 (8 of 8). DISM13 and
  DISM16 7 of 8, DISM45 7 of 8, DISM25-29 5-6 of 8.
- **Not new audio:** DISM03 channels 3-8 are bit-identical copies of DISM24
  channels 3-8 (lines 17-22 against 183-188; lines 17/183 and 20/186 are
  unused in both places). Line 29 is close to the used line 48.
- **Line 140** is played only by LEVEL04 (True Grit).
- **Table oddities:** line 60 ends 7 sectors after its file's start, before
  its channel's first sector, so even if called it would play almost nothing
  of its 2 s; lines 71-74 end about 0.2 s before their recorded audio does.
- **Limits.** "No reference found" is static: a missed jump table or an
  unknown code path could reach a line. "A constant at a call site" means a
  code path passes it, not that play reaches that path.

What the unused lines say (machine transcripts, to be confirmed by ear):

| Group | Examples |
| --- | --- |
| A difficulty menu that talks back (DISM08, whole file) | "Playing it safe, huh?", "This mode's for girls.", "What? You like suicide?", "It's time to get some!", "Prepare to die." |
| Refusals when a goal is not met (DISM45-46) | "Been there, done that.", "I'm not into repeating myself.", "Uh-uh, can't do that yet.", "Hmm, I still need something.", "Still got work to do." |
| The medieval chessboard (DISM22) | "Oh look, checkers!", "Checkmate", "Hail to the Queen, baby!", "Now Queen takes King!" |
| Rome (DISM28-29, DISM47) | "Caligula, here I come!", "Et tu, Brute?", "Great Caesar's ghost!", "Oh boy, ancient Roman shit." |
| Old West (DISM13, DISM16-17) | "What's this, fool's gold?", "Where the hell is all the gold going?", "Happy trails to me!", "Nobody screws with my past and lives." |
| Heavy pickups (DISM17) | "Whoa, this is heavy, even for me!", "And I thought ten guns were hard to carry.", "If I had a sidekick, he'd carry this crap." |
| The missing babes (DISM20, 36, 43) | "Where the hell are my chicks? Someone's gonna pay." (three takes) |
| Heat (DISM20) | "It's hotter than a Senate hearing on video game violence." |
| The pain quote (lines 17 and 183, the same audio) | "Show me yours, and I'll show you mine. Turn out the lights, and I'll show you the meaning of pain." The executable also holds it as on-screen text (strings 176-177). |

Files (local): `research/d61a/voice/` (`DISMnn-chK.wav`, `slots.csv`,
`voice_table.csv`, `transcripts.json`, `index.html`). Transcripts are
machine-made (Whisper small.en, offline) and need a listen.

## 2. Sound effects: older tables in the hidden levels (disc, code)

Each level folder's `SND.DAT` is the game's sound table followed by a
standard VAB bank.

- **Table** (pointer at `0x800c39c4`, read by `0x80068b20`): 29 groups of
  (count, pointer) from offset `0xC`, then 16-byte entries. A sound id is
  `group << 12 | index` (index `& 0x7ff`; `0x8006b270` and its siblings take
  the id). Entry byte 0 is the VAB program, byte 1 the tone, bytes 6-7 the
  playback rate in Hz (8000-16000), byte 4 bit `0x80` disables the entry.
  Every tone's centre note is 64.
- **Groups.** 0-7 are common to all levels (group 0 the menu sounds), 8-27
  level-specific, 28 (`0x1c`, 11 entries) the gadget sounds. The jetpack's
  open/close (`0x8003fffc`) plays `0x1c007`; the cluster at
  `0x80043780-0x80043910` plays `0x1c001`, `0x1c003` and `0x1c005`; Duke's
  damage handler plays `0x1c000` and `0x1c004`.

| Level | Gadget group `0x1c` | Level groups | Menu group 0 |
| --- | --- | --- | --- |
| Campaign, bosses, arenas, challenges | 11 entries, real sounds | 1-3 groups | shipped sounds |
| 4 True Grit | 11 entries, **all one 0.18 s sample** | group 16 (5) | older samples (sim 0.2-0.6) |
| 13 Modern 4 | **none** | none | a 0.07 s blip |
| 14 Animation test | **none** | none | a 0.07 s blip |
| 30 empty, 31 sound test | **none** | none | a 0.07 s blip (31) |

- **Why the jetpack is silent** in Modern 4, the animation test and the sound
  test: the lookup returns nothing for group `0x1c`, so the open, close and
  thrust sounds never start. In True Grit it plays the 0.18 s placeholder.
  This matches the user's report and Mortimer's prototype note of a
  "different jetpack sound" (online).
- **Placeholders.** In Modern 4, 14 ids play the same 0.07 s sample
  (`771af8df83`): 0000, 0002, 0003, 101f, 2006, 3009, 4008, 5008, 600c-600f,
  7008, 700a. True Grit plays one 0.18 s sample (`1e6556db1a`) for 14 ids.
- **Same recordings, re-encoded.** Many Modern 4 and animation-test ids
  (1007, 1013, 1014, 1019, 3004, 3008, 4004 ...) match the shipped sound
  (spectrum correlation 0.93-1.0, same length) with different ADPCM bytes.
- **Variants.** Different-sounding samples under a shipped id: True Grit 7002
  and 700a; Modern 4 100b, 200c, 200e, 2005, 6003; animation test 2004, 2005;
  sound test 200c, 200d, 5008.
- **30 samples exist only in hidden-level banks** (8 True Grit, about 20
  Modern 4 / animation test, 2 empty room / sound test).
- **Six samples no table references in any level** ("orphans"): True Grit 3
  and 4 (0.26 and 0.22 s), RESISTANCE IS FEUDAL 131 (0.57 s), Modern 4 43
  (0.35 s), Challenge Stage 6 109 (0.78 s), MOLOCH 75 (0.29 s). Their playback
  rate is unknown (written at 11025 Hz).

Files (local): `research/d61a/sound/` (`index.html` pairs each hidden sample
with the shipped one under the same id; `pairs.json`, `idmap.json`, WAVs).

## 3. Commando Duke (memory, code, seen, online)

- Duke's body is model record `player+0x40`: 19 joints, 0x28 bytes each from
  `+0x44`, the mesh pointer at joint `+0x24`. Meshes compared by vertex
  geometry (faces differ with texture placement).
- **Five costumes, five model sets:** modern 98-114 (levels 0, 5, 9, 14, 21-26,
  29-31), Old West 152-157 (1-4, 27), medieval 302-318 (6-8, 28), Roman
  682-698 (10-12), and **489-506 in Modern 4 only**.
- The two-player arenas (15, 16) load the same 489-506 meshes, geometry
  identical. CHOOSE CHARACTER's names (strings 195-200): RED DUKE, BLUE DUKE,
  WESTERN DUKE, MEDIEVAL DUKE, **COMMANDO DUKE**, TOGA DUKE. Modern 4's Duke
  is the two-player Commando Duke, used as the one-player Duke nowhere else.
- **Online:** Mortimer's prototype thread says Modern 4 "Duke uses his
  Commando outfit"; a 1998 GameSpot preview lists commando garb and a kilt
  among the planned outfits. The six-name list and the model ids were not
  found online.

## 4. Index 30 is the cut fourth boss (code, disc, online)

- Level names are string ids at `0x800c3d2c`: the three boss levels 27-29 use
  232-234 (THE REAPER, WING'ED DEATH, MOLOCH THE GATEKEEPER), index 31 uses 236
  (SOUND TEST). Index 30's entry is 0 (no name), and **string 235 is `BOSS 4`**:
  the slot between them.
- Index 30's folder `D3/DB30` carries the boss folders' own 32 KB
  `BASEOVR.OVR` (byte-identical in DB27-DB30). Its sound bank is the sound
  test's (same size and layout, no gadget group), and its level data is one
  room (note 559).
- **Online:** Mortimer documents a prototype "Boss 4 (level index 30)": in June
  a platform with "a particular metallic alien hound", by August pitch-black
  without sound, in September "a mere empty boss bar ... ultimately taken out".
  The retail empty room is what is left of it; no source said so.

## 5. A quadruped brain nothing loads (disc)

- `BASEOVR.OVR` in DB27-DB30 (32588 bytes, tag `0x13`) holds the creature
  brains `brain.c`, `hellwing.c`, `robot.c` and **`qped.c`** ("quadruped start
  pos in terrain", "unknown quadruped %d", "distance / magnitude / smooth").
- No level overlay and not the executable contains `qped.c`. The file is not
  in the executable's extent table and was not found in any level's RAM.
- Its internal pointers cluster at `0x800c4d00-0x800c9000`, below the retail
  overlay base `0x800ca968`: it was most likely linked for an earlier memory
  layout and cannot run as it is.
- With Boss 4's "metallic alien hound" (online), this is most likely that
  creature's brain. Probable, not proved.
- Also stale: the 360-byte `BASEOVR.OVR` in DB07-DB13 (identical; a stub that
  installs default handlers, linked for a base near `0x800c7218`), never loaded.
  `BONUS.OVR` (DB21-DB26, identical) was not seen in RAM at a challenge
  stage's start.

## 6. The pad-cheat table (code, online)

Sequences at `0x800c3d8c` (19 pointers), dispatched by `0x80083904` through
the jump table `0x80016e38`; each case shows a message by string id. Buttons:
U/D/L/R the d-pad, S Select, 1 L1, 2 L2, 3 R1, 4 R2 (TCRF's legend).

| Sequence | Effect |
| --- | --- |
| LRLRSLRLRS | unlimited ammo (toggle) |
| 12U12D3R4L | all weapons |
| 2314UDUDSS | invulnerability (toggle) |
| 3333322222 | all inventory |
| URULDURLRD | all keys |
| DDDDDDDDDU | level select |
| 34213421SS | super weapons |
| 333333333U / D | big / tiny head Duke (toggle back: normal) |
| 333333333L / 333111444R | big / tiny head enemy |
| 1313131313 | invisibility (6000 units) |
| 2424242424 | Double Duke (6000) |
| 3212313212 | invulnerability power-up (6000) |
| SUUUUUUUUUS1-4 | movie replay selector `0x800c3d80` = 1-4: INTRO, VICTRY, CREDIT, KILT (note 281), no message |
| LRULRDLR13 | 99 continues |

All 19 are on TCRF already, including the four movie cheats. KILT is known
online as the "Duke Wallace" clip.

## 7. Strings with no use found (code)

`SCUBA TABS` (105) sits in the inventory name block between the pipe-bomb
detonator (104) and the jetpack (106). `CANNONBALL` (162), `GUNPOWDER` (163),
`STONE KEY` (244), `SPECIAL 1-3` (169-171), `KEY 1-5` and `NO WATER IN TANKS`
(159) also exist. No pickup type or code use was found for SCUBA TABS.
Online, scuba tabs, the cannonball, gunpowder and the stone key are prototype
items (Mortimer: True Grit's stone altars held a STONE KEY in July).

## 8. The Holy Hand Grenade has the same three models everywhere (memory)

- Captured with `DNTTK_HHG_LOOK=0` (the D08A47 rewrite off), all 26 levels:
  the held mesh 945, thrown 944 and pickup 1129 have the same vertex geometry
  in every level, the hidden ones included (42, 42 and 35 vertices).
- So no level, hidden or shipped, has a held grenade shaped like the pickup.
  A gold grenade in Duke's hand in a hidden level is the D08A47 fix (Modernized
  converts the meshes in every level, hidden ones included).
- **A side effect of that fix (seen):** in Modern 4 the converted grenade in
  Duke's hand draws magenta. The pickup's CLUTs (`0xc7a`/`0xcba`) hold other
  colours in that level's VRAM. Filed under D61B.

## 9. The hidden creatures (memory, seen)

Only four creature types appear in no shipped level (spawner records of all 32
indices):

| Type | AI class | Skeleton (type `+0x10`) | Where | Result |
| --- | --- | --- | --- | --- |
| 538, 543 | 8, the non-hostile class (as the shipped club dancer 167) | 22, 23 (shipped dancer 167: 15) | Modern 4 club, room 12 (spawners 20, 19, live) | Seen dancing on the club's poles |
| 121 | **12**, a class no other creature uses | 11 | Modern 4 room 47 (spawner 17, flags 0, not live) | Never activates; the room is the Moloch-like chamber |
| 621 | 15 (as 622 in level 5 and 623 in level 9) | 30 | Sound test room 15 (spawner 5, live) | Not seen in the tour shots |

- The console's `spawn` makes a placed object of the type (the log shows the
  object in the right room), which draws nothing for a creature: creatures need
  the brain and skeleton the spawner system sets up. Seeing 121 and 621 needs
  their spawners started, left to D61B.
- Mortimer mentions "a missile ready to be launched" in Modern 4. Whether
  class 12 is that missile is not known.

## 10. Every level's unused models photographed (seen)

`atlas.py models` ran on levels 0-14 (3 was done in D61): 12, 19, 30, 36, 19,
8, 13, 47, 25, 34, 39, 55, 18, 23 and 1 types. Sheets:
`research/atlas/models/levelNN/sheet.html`. Most are props the level never
places (crates, shelves, chains, barrels, walls, a security panel with
mirrored text in Modern 4, True Grit's urns, crossed poles and dead branch),
some untextured (magenta in Modern 4). Debris and effects draw nothing.

## 11. Every level toured (seen)

`atlas.py tour` (god mode) on the 11 shipped levels D61 had not toured, in one
background pass (about 54 min): arrivals 78/84 (level 0), 73/78 (1), 49/52 (2),
70/77 (5), 24/29 (6), 33/34 (7), 49/64 (8), 70/76 (9), 44/45 (10), 57/58 (11),
61/65 (12). Rooms without an arrival have no standing point (vents, alcoves).
With D61's tours of 3, 4, 13, 14 and 31 every campaign level and hidden index
except the empty room has a contact sheet (`research/atlas/tour/levelNN/`).
The sheets were not reviewed shot by shot; they are there for the user and
for D61B.

## Checked on the original game (original)

DuckStation portable copy (`research/d61a/ds`, copied from the D14 one:
private settings, memory cards None, no resume state, GDB stub on, update
check off), Xvfb, `research/d61a/lab/ds_check.py`. ONE PLAYER, then each
hidden level forced as `level force` does (end code `0xFD`, the level index
written at the init call `0x8002b9f4` through a breakpoint), and RAM read over
GDB:

| Level | Gadget group `0x1c` entries | Duke's joints |
| --- | --- | --- |
| 0 TIME TO KILL | 11 | 96-114 (modern) |
| 13 Modern 4 | **0** | **489-506 (Commando)** |
| 4 True Grit | 11 | 149-166 (Old West) |
| 14 Animation test | **0** | 96-114 (modern) |

Screenshots in `research/d61a/original/`. The player's own DuckStation folder
was hashed before and after: only flatpak's linker cache (`.ld.so`) changed;
config, resume state and memory cards were untouched.

## In the recomp: the jetpack trace (memory)

`research/d61a/lab/lab2.py jet`, `DNTTK_SOUND_TRACE=1`, a jetpack spawned and
picked up, switched on (`player+0x358` 1 then 3), Space held, switched off:

| Level | Gadget ids requested (besides footsteps) |
| --- | --- |
| 0 | `1c002` (pickup), `1c007` (open/close) |
| 13 | `1c002` (pickup: no group, nothing plays) |
| 4 | `1c002` (plays the placeholder) |
| 14 | `1c002` (no group) |

`1c007` was requested only in level 0 in this run; why the hidden levels did
not request it was not traced. The thrust ids were not requested in any level
(Duke may not have left the ground in first person). The audio itself was not
heard: offscreen runs have no sound.

## Already known online

Report: `research/d61a/online.md` (local). Mortimer's Duke4.net thread "Duke
Nukem: Time To Kill Prototype Differences" (2021) compares four 1998
prototypes. It already describes True Grit, Modern 4 (with the Commando
outfit, the strippers and the alien facility), the animation and sound tests,
the prototype Boss 4 at index 30, prototype items (scuba tabs, cannonball,
gunpowder, stone key) and a different jetpack sound. TCRF lists all 19 pad
cheats. The RTOPEN demo (hold L1, L2, R1 and R2 at the GT logo) and the "Duke
Wallace" KILT clip are known.

Not found online: the 132 unused retail voice lines, the missing gadget sound
group and why the jetpack is silent, the placeholder samples, the six orphan
samples, the `BOSS 4` string in its name slot, `qped.c` in the retail disc's
boss overlay, creature 121, the model ids of the costumes and the grenade's
single model set.

## Limits

- Static reachability (voice lines, strings, overlays) can miss a computed
  path; each such claim says "no reference found".
- The voice transcripts are machine-made and need a listen.
- Not checked on the original: the voice-line table use, the placeholder
  samples by ear, the creatures.
- Creatures 121 and 621 were not seen.

## Files (all local, untracked)

| Path | What |
| --- | --- |
| `research/d61a/disc/` | the disc's logical files (no movies, no XA) |
| `research/d61a/raw/` | RAM dumps and start shots of 26 levels, `DNTTK_HHG_LOOK=0` |
| `research/d61a/sound/`, `research/d61a/voice/` | WAVs, tables, listening pages |
| `research/d61a/shots/`, `research/d61a/original/`, `research/atlas/models/` | photographs |
| `research/d61a/lab/` | the lab scripts (capture, meshes, lab, lab2, front, ds_check) |
| `research/d61a/online.md` | the online check |
