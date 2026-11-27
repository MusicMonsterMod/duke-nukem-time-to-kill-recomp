# D61B - Stabilize the hidden levels

Status: **Done 2026-11-26** (closed: "lock in todays efforts, and close us out") (selected by the user: "work on D61B ...
I dont want to lose what they are, but instead in this iteration, just fix or
patch over clearly broken elements of them"; plan read and approved: "use your
order as described in the artifact", "proceed autonomously").

The hidden levels are True Grit (4), Modern 4 (13), the animation test (14),
the empty room / Boss 4 (30) and the sound test (31). They keep their layouts,
prototype props, green test grids, Commando Duke and their own sound variants.
This job patches over what is plainly broken. Modernized only; Vanilla, the
shipped levels and the disc are unchanged. Everything derived from the disc is
built into `build-local/` packs from the player's own disc and never committed.

Evidence labels as in [note 560](560-d61a-cut-content-hunt.md): **code**
(executable disassembly), **disc**, **memory** (RAM of an offscreen run),
**seen** (photographed in the recomp).

## The plan the user read

A private artifact (local copy `research/d61b/plan/`) sorted every finding as
Fix, Keep or Ask, by level. User decisions: True Grit room 27 water "the same
look and feel as normal water used elsewhere"; Modern 4 room 44 is normal (its
alien section looks right from eye level) and only its water needs stabilizing;
Modern 4 borrows a modern sky; the surfacing breath in Modern 4 that sounds like
Duke 3D's scuba is documented for later (D61C); order: sounds, the grenade,
textures and sky, then the rest.

## 1. Sounds (code, disc, memory)

How a level's sound bank loads (all **code**):

| Routine | What it does |
| --- | --- |
| `0x8006a508(level)` | loads `SND.DAT` (file id `0x16b + level`) once per level |
| `0x8006a2c0` | copies the table (header word 0 bytes, at most 0x2000) into the buffer `*0x800c39c4` and makes the 29 group offsets RAM addresses (entries from `+0xf4`) |
| `0x8006a3ac(vh)` | turns the VH's first 140 sample sizes (its last 0x200 bytes) into SPU addresses from `0x1010` (`0x800da730`; index k plays VAB sample k+1) |
| `0x8006a3fc(vb)` | uploads the VB to SPU `0x1010` |
| `0x80068b20(id)` | the lookup: no group, an index past the count, or entry byte 4 bit `0x80` returns no entry, so nothing plays |
| `0x8006b3ac` | the sample index is entry byte 0 * 16 + byte 1; the VH's program and tone blocks are never read |

Findings (**disc**, correcting note 560 where it said "placeholder"): Modern 4,
the animation test, the empty room and the sound test have no group `0x1c`;
True Grit has the group but all 11 entries carry the disabled bit, as do some
other ids (Modern 4: `0002`, `0003`, `101f`, `2006`, `3009`, `4008`, `5008`,
`600c-600f`, `7008`, `700a`; the animation test 7 of those). Those ids were
silent, not blips. The menu group 0 is a set of short prototype samples.

Fix (`recomp/src/ttk/hidden_level_sound.cpp`, pack `build_ttk_sound.py` ->
`build-local/ttk-sound.pack`): at the entry of `0x8006a3ac` (added to
`game.local.toml` `mod_function_entry_funcs`; one generated line, codegen hash
`0xebce51ea` unchanged, savestates still load), for those five levels only, the
pack's entries are written into the table (a missing gadget group is appended
after the level's entries in the 0x2000 buffer), the donor samples' sizes into
the VH and the samples into SPU RAM past the level's VB (below `0x7aa40`, the
end of the largest shipped bank), through the new framework call
`spu_ram_poke` (spu.c, in the accepted-source patch). The game then builds the
addresses, uploads its own VB and plays the sounds itself. Donors: TIME TO KILL
for the modern levels, DUKE HILL for the Old West True Grit (the common groups
differ by era). What is copied: the whole gadget group, the menu group, and
any id of groups 0-7 that the hidden level disables and the donor plays. Ids
the hidden level plays with its own different sample are kept. Guards: the
loader's code (three SHA-256 ranges), the table's bytes against the disc's, the
VH magic and sizes, the SPU end. `DNTTK_HIDDEN_SOUND=0` off,
`DNTTK_HIDDEN_SOUND_TRACE=1`.

| Level | Entries | Samples added | Bytes |
| --- | --- | --- | --- |
| 4 True Grit (donor 1) | 16 | 11 | 24752 |
| 13 Modern 4 (donor 0) | 27 | 22 | 58944 |
| 14 Animation test | 23 | 18 | 49392 |
| 30 Empty room, 31 Sound test | 16 each | 11 each | 23520 |

**The playtest beep.** The game's VB upload (`0x8006a3fc`, right after the
hook) moves whole DMA blocks and wrote past the VB's end into the first added
sample (True Grit: 48 bytes). That sample is the menu sound `0x0`, so the pause
menu started a voice looping on the damaged block, a steady tone (user,
2026-11-26; read live from the player's game: voice 0 at `0x653A0`, looping
`0x653C0`, the table patched). The samples are now written again at Duke's
first update after the bank load (`0x8005a210`). Verified: in 4, 13, 14 and 31
every added sample in SPU RAM equals the pack byte for byte, and the pause
menu leaves no voice playing from them (`research/d61b/lab/fix_check.py`).

Verified (**memory**, `research/d61b/lab/sound_lab.py`, offscreen,
`PSX_TRACE_HISTORY=1`): with a jetpack spawned, taken, opened, flown and
closed, `1c007` (open/close) is now requested in all four playable hidden
levels and SPU key-ons start inside the added samples (Modern 4 23 of 33,
animation test 6, sound test 2, True Grit 2; TIME TO KILL 0, unpatched).
Vanilla (`--vanilla`): Modern 4's gadget group stays empty, no key-on from added
samples. The sound itself was not heard (offscreen); WAVs of each run are in
`research/d61b/run-mod/jet-levelNN.wav` (local).

## 2. The magenta grenade (memory, seen)

Modern 4 and the animation test load the Holy Hand Grenade pickup (model 1129)
with unresolved faces: CLUT `0xffff`, page `0x20`. D08A47 copied those faces
onto the held and thrown grenade, so Duke held a magenta grenade.
`hhg_look.inc` now refuses the conversion when any pickup face names CLUT
`0xffff` (refusal 6) and the original held grenade stays. True Grit and the sound
test resolve the pickup and keep the gold grenade. Before/after:
`research/before-after/D61B/hhg-held-level13-*.png`.

## 3. Textures and skies

**The marker.** A model face whose texture the level's set lacks keeps CLUT
`0xffff` on page `0x20` (x 0, y 0: the frame buffer). That draws magenta in
Modern 4 and purple-white stripes in True Grit. Shipped levels have a few such
faces on models they never draw, so the marker alone does not prove a visible
fault; each case was looked at.

**Texture VRAM** (**disc**, **memory**): a level's texture VRAM is exactly its
two raw images, `DBnnA.RAW` at (512,0) 448x256 and `DBnnB.RAW` at (512,256)
512x256 (debug-port dumps equal them row for row; note: `vram_peek` prints
big-endian words).

### Skies (code, memory, seen)

| Level | Layers | Their textured faces |
| --- | --- | --- |
| 4 True Grit | Duke Hill's ids 0x611-0x614 | all CLUT `0xffff` (the striped band over the pink-purple backdrop) |
| 13 Modern 4 | TIME TO KILL's ids 0x619-0x61b | both cloud bands CLUT `0xffff` (the magenta band over the street) |

`sky_borrow.cpp` (D17AO1) now also has a restore mode: in True Grit and Modern 4
when the level's own slow band is wholly unresolved, the borrowed set is drawn
whenever the game draws its sky. True Grit gets Duke Hill's set as before; Modern
4 gets TIME TO KILL's (user: "borrow modern sky"). `build_ttk_sky.py` appends
TIME TO KILL's backdrop and bands to `ttk-sky.pack`: Duke Hill owns x 512-1023
of the second texture bank, so the bands' texels go to page 7's lower half
(x 464-511, y 256-383, same UVs) and their six CLUTs to x 448 (page `0x3b` ->
`0x37`, CLUT index -0x10). GOLD AND GUNS' painted skylight works as before. Every shipped sky is drawn
this way: the cloud bands ring the horizon and nothing turns overhead (user:
"they never have the clouds rotating in the centre of it, its always just the
edges"; checked on DUKE HILL's own sky). So a restored sky leaves out D17AO1's
cloud deck (the bands' second polygon group, built for GOLD AND GUNS'
skylights, which look straight up): the copy's list ends after the bands' own
group. True Grit's sky overhead is now the plain backdrop, as DUKE HILL's.
Shots: `research/d61b/sky/level{01,04,13}-up*.png` (local).
Before/after: `research/before-after/D61B/sky-*.png`.

### Untextured models (memory, seen)

`recomp/src/ttk/hidden_level_look.cpp`, pack `build_ttk_look.py` ->
`build-local/ttk-look.pack`. Once the level is in play (Duke's update) and a
listed face still names CLUT `0xffff`, its VRAM blocks go up with the GPU's own
CPU-to-VRAM copy (`gpu_write_gp0`, GP0 `0xa0`) and the faces get a CLUT, page
and UVs shifted onto the block. Faces already textured and models whose vertex
and face counts differ are left alone. `DNTTK_HIDDEN_LOOK=0` off.

| Level | Model (type, room) | Source |
| --- | --- | --- |
| 4 | 87 (81, room 25: the totem poles by the bridge, the user's report), 8 wing faces | **our painting** `recomp/assets/hidden-levels/true-grit-totem-wing.png` (`paint_totem_wing.py`, 96x32, drawn through the totem's CLUT `0x1438`) |
| 4 | 87, 20 head faces (y above -1380) | a Drak's face (user: "texture the head to be similar to that of one of the draks ... colored up nice to match the aesthetic of the totem pole"): the lizard guardian's own head profile (page `0x2c`, CLUT `0x2171`, u 64-95 v 160-175) and scales (`0x2271`, u 72-95 v 216-231) from True Grit's raw image, recoloured by brightness into the totem's gold ramp (`0x1438`), one 32x32 block at (912,72); UVs projected from the head's vertices in `DB04MOD.BIN` (sides the profile, snout forward; top, underside and back the scales). The teeth and crest keep their own textures. |
| 13 | 85 (79, the club's bar fronts) | TIME TO KILL's twin: page `0x239` u e0-ff v 20-3f, CLUT `0xe38` |
| 13 | 528 (232, room 13) | TIME TO KILL: pages `0x2a`, `0x2e` |
| 13 | 872 (472/594, rooms 6, 8) | TIME TO KILL: page `0x2b` |
| 13 | 1103, 1109 (596/602, room 15's crates, seen through room 17's window) | no twin: the level's own crate (type 20, page `0x38`, CLUT `0x16fb`) |
| 9, 13, 14, 30 | 1129 (608, the Holy Hand Grenade pickup; D08A51) | level 6's own: two 16x24 blocks at (748,48) / (758,48), CLUTs `0xc7a` / `0xcba`; faces 0-14 / 15-22. Places: 9 (628,136) + CLUTs (816,86); 13 (812,76) + (912,32); 14 (552,32) + (608,32); 30 (776,32) + (800,32). Pig Factory (9) is a shipped level; see note 520 |

The head first got the wing painting too (its 20 faces were among the 28
untextured). A Drak head match was checked and ruled out: no mesh in any
shipped level matches the totem's head (exact translation over every model,
and a normalised shape score against every skeleton joint; the lizard
skeletons' head joints are different shapes), and none of its texture bands
recurs, recoloured, in any level's raw image. Render:
`research/before-after/D61B/totem-head-drak-render.png`; in game the 20 faces
name CLUT `0x1438`, page `0x2e`.

**The totems' collision** (user: "duke cant walk under their wings", then
"not really able to get overly close to the base"): the poles' object `+0x44`
is model 87's mesh, at once the draw's mesh and the collision box list (header
`+4`'s low byte the box count, `+8` the boxes: min x, y, z, across, max x, y,
z, corner radius; shipped props use up to 3). The disc gives one box, the
whole bounds (x -339..229, z -853..847, wings and beak). The pole is a
hexagonal prism: corners (+-228, +-132) and (0, +-264) from the floor to y
-629, (+-156, +-90) and (0, +-180) to -1364; the wings (from -1285) and head
clear Duke's 1280. `hidden_level_look.cpp` gives it four boxes, a core and a
points box per section, in mod memory (the draw never reads `+8`; the mesh
header's radius stays), rewritten while in use. Two earlier versions were
replaced: a header copy in mod memory hid the totems (the host draw path takes
meshes only in main RAM), and one in-place box (x +-228, z +-264) overhung the
hexagon's corners.

Measured (memory, `research/d61b/lab/pole_test.py`, running at the pole from
four sides): Duke now stops 541-593 from the pole's centre (the disc's box:
699-899). That is the nearest face plus Duke's own collision radius, about 370
(ledge_reach.inc), the same gap as at any wall or crate: from the south 198 +
370 = 568 (measured 567). Changing the boxes' across radius had no effect, so
the game collides with the faces. Room 25's wall stands 416 north of one pole.
Limit: the wing tips slope to 800 above the floor, lower than Duke's head.

Twins were found by identical vertex bytes in the shipped levels' RAM. Each
untextured face's local UV box has the size of the twin face's box. Every
destination is zero in the level's raw image and clear of every texture its
world and models name (`research/d61b/lab/free_vram.py`); the builder checks
the zeros again. Before/after: `totems-*`, `club-bar*-*`, `crates-*`.

### Water (code, memory, seen)

The water renderer `0x8002ef90` textures its tiles from the material table
`*0x800c597c` (materials 0, 1 and 4; `+0x20` points to six halves: UV 0, CLUT,
UV 1, page, UV 2, UV 3), which the game rewrites every tick from the water's
frame list. A shipped level's frames are 32x32 caustic tiles (PIG FACTORY: 16 on
page `0x1f`). True Grit's and Modern 4's are an earlier set: 64x64 boxes whose
top-left 32x32 alone holds a caustic tile (the rest is flat light grey: a bright
sheet under additive blending), and some frames are all zero (the water then
sampled the frame buffer for a tick: stripes and flat slabs). At the
renderer's entry, in those two levels, a 64x64 frame is narrowed to its
caustic tile and a zero frame keeps the last good one. `DNTTK_HIDDEN_WATER=0`
off. Before/after: `tg-water*`, `m4-water44-*`.

A first attempt (filling the descriptors with a shipped tile at load) was
removed: the zeros seen in the start-of-level RAM dumps were only the frames
before the first tick.

## 4. Titles

User: "line up all the real level titles so they are actually presented to
the user when warping to one of the lost levels. currently it says XXX".
Level names are string ids at `0x800c3d2c` into the text table `0x800c60f4`
(**code**): True Grit and Modern 4 name strings 93 and 102, both "XXX"; index 30
names 0, while string 235, "BOSS 4", sits in its slot's place; the animation
test (103) and the sound test (236) have their own. No retail source names 4
or 13; the titles are "TRUE GRIT" (Mortimer's prototype notes: "Level 04? -
True Grit", called XXX already in the Jul 7 build) and "MODERN 4" (the only
name it has had), and "BOSS 4" (the game's own string) for 30.

Modernized draws its own level card, pause title, save names and console
replies, so `hidden_titles.h` gives those three titles there (`game_menu.cpp`,
`level_select.cpp`, `goto.inc`), while the disc's name ids are in place. No
guest data changes: a first version repointed the strings and index 30's id,
which broke the level select's and the game menu's code guards (both cover the
name table, the menu also the text table), so travel refused ("unrecognised game
code"); replaced. Vanilla keeps XXX. Seen: the arrival cards MODERN 4, BOSS 4,
TRUE GRIT and the pause title TRUE GRIT (`title-*.png`); `levels` lists them.

## Not changed

- The animation test and sound test grids, the empty room's void, True Grit's
  dim brick and prop set, Commando Duke, the dancers, the levels' own sounds.
- Creatures 121 (Modern 4 room 47) and 621 (sound test room 15), exits, Boss 4,
  the unused voice lines (phase 4, one at a time with the user).
- Props the levels load but never place.

## Limits

- Not heard: the sounds were verified by table, request and SPU key-on, offscreen.
- Not checked on the original (DuckStation): the sky, model and water faults.
  They follow from the disc data (unresolved faces and zero frames come from the
  loader and the level files), not from a recomp path, but that was not
  confirmed on the original.
- Modern 4's room 44 floor looks the same before and after; which of that
  section's surfaces are water was not established.
- The green slab under True Grit's bridge in the D61 tour did not reproduce in
  the current build.
- Offscreen runs use llvmpipe; judge the look on the real display.
- `ttk-sky-test` segfaults before main (its hook map is initialized after the
  prioritized registration constructors, D23AH); it predates this job and was
  not fixed here.

## Files

Tracked: `recomp/src/ttk/hidden_level_sound.cpp`, `hidden_level_look.cpp`,
`sky_borrow.cpp/.h`, `hhg_look.inc`, `ttk_register.h`, `recomp/CMakeLists.txt`,
`recomp/game.local.toml`, `recomp/tools/local/build_ttk_sound.py`,
`build_ttk_look.py`, `build_ttk_sky.py`, `paint_totem_wing.py`,
`recomp/assets/hidden-levels/true-grit-totem-wing.png`,
`recomp/tests/local/test_hidden_level_packs.py`, the accepted-source patch
(`spu_ram_poke`). Local: `research/d61b/` (plan, labs, runs, VRAM dumps),
`research/before-after/D61B/`.
