# D12C - Kick impact sound on a real hit only

Status: **Done** (user-accepted 2026-10-08: "perfect!! fully accepted."). Final tuning: one octave
up, three times the table volume, a still-playing `0x2007` stopped first
(revisions below). Modernized only; Vanilla unchanged.

User request: when Duke's kick connects (a wall, a breakable object, an enemy)
there should be a physical impact sound, the non-vocal thud the game plays
when Duke runs into a wall, never his grunt. A kick that touches nothing stays
silent. "If Duke's boot actually hits something, I should hear the impact. If
his boot hits nothing, I should hear nothing."

## The sound (acceptance 1-3)

Running into a wall head-on switches Duke to the bump animation 94/95 and, in
the same frame, starts SPU sample `0x1BAA0` (private run, first street: the
anim 94 frame and the KEYON frame match; the other samples in the capture
are ambience that also plays with no input). The code is the bump case of the
ground movement result (`0x800537f4`..`0x800538bc`):

```
sh   94 or 95 -> anim            (facing test 0x80066b88)
jal  0x8001e840(pad, 3)          pad rumble
li   a0, 0x2007
addiu a1, Duke+4
jal  0x8006b270(0x2007, Duke+4, 0x800)
```

That call plays only `0x2007`; there is no vocal in it. `0x2007` is bank 2
entry 7 (`id = bank << 12 | index`), SPU sample `0x1BAA0` on the first street;
`sfx 0x2007` plays the same sample. The landing after a fall (`0x80054cd4`)
uses the same id. Duke's grunts are other ids, so nothing needs separating
beyond using this id alone.

## The hit (acceptance 4-5)

The kick's damage is the sphere `0x800a979c(point, 96, 10, damage, Duke, 0)`
(note 65). It returns no result. It walks the room's actors then objects and,
for the first box the sphere touches, stores its fifth argument (the attacker)
at `sp+0x10` of its own `0x80`-byte frame (`0x800a9b8c`) and calls the struck
object's class handler (`jalr` at `0x800a9bac`). Nothing else writes that
slot (callees use their own frames). So a host call on a private stack clears
the slot, calls the sphere, and reads it back: the attacker means the sphere
struck something and the handler ran.

The original sphere ignores level geometry (the foot passes through walls),
so the game has no wall result for a kick. Wall contact uses the original
segment query `0x8006d980` (the D07 aim query): kind 1 (world) where the boot
reaches.

- **Eye view (D12A quick kick):** each player update in the hit window
  (frames 8..20) the sphere runs as before, now with the struck check. Wall:
  a horizontal segment from Duke's root, 150 above it (boot height), out
  along the view's heading 480 (the D12A foot reach). The D12A crosshair
  trace starts at the camera point (about 260 behind Duke in the measured
  run) and reported no world hit along a wall Duke had stopped against, so
  walls use the segment from his body. Horizontal at boot height, so floors
  and ceilings never count.
- **Third person (original kick 112..115):** a new entry hook on
  `0x800a979c` acts only for the kick case's call (ra `0x80049098`; Duke as
  attacker, radius 96, type 10), Modernized with player identity. The host
  makes the same call with the same six arguments on a private stack and
  gets the result. The original call then gets radius `0xf0000000`: its box
  test needs the point within (box - radius), which no box passes, and the
  squared radius wraps to 0, which the distance gate rejects first. Damage is
  applied once, by the original routine with the original arguments. Wall:
  a segment from Duke's root to the foot point at the foot's height, when the
  foot is more than 48 above the root.

**Once per kick:** the thud plays at the first contact of a kick and not
again in that kick (quick kick: per `kick_request`; original: per kick
animation, reset when Duke leaves 112..115 or the variant changes). A kick that
strikes an enemy and then reaches a wall is one thud.

**Sound call:** `0x8006b270(0x2007, Duke+4, 0x800)` on a private stack, the
wall bump's own call. Targets keep their own reactions (pain vocals, breaking);
nothing they play is suppressed or duplicated (in the bag and pig cop runs the
thud sample starts once per kick).

## Code

- `recomp/src/ttk/kick.inc`: `kick_call(..., bool* struck)`, `kick_impact`,
  `kick_wall_reached` (eye view), `kick_sphere_entry` (third person), debug.
- `recomp/game.local.toml`: `0x800A979C` added to `mod_function_entry_funcs`
  (one regenerated line; codegen hash `8bab543c` unchanged, savestates load).
- `modern_controls.cpp`: the hook registration (ra `0x80049098` only).
- `control_guards.inc`: the bump sequence `0x800537f4` (208 bytes) is now
  guarded too; `0x800a979c`, the kick case `0x80049044` and `0x8006b270` already were.
- `DNTTK_KICK_IMPACT=off` (diagnostics) turns it off and leaves the original
  third-person call alone; `DNTTK_KICK_IMPACT=<id>` plays another sound.
- Debug: `ttk_input` -> `controls.fp.kick.impact` (`sound`, `played`,
  `objects`, `walls`, `original_calls`, `failed`).

## Evidence (executable `3873288acd438d2e367ca2c0fffa20f0953134de61af7715522fec0ffcc87c98`)

Private Xvfb runs, private profile, copies of the D12A test cards
(`recomp/analysis/d12c-kick-impact/`, local). "Thud" is a KEYON of sample
`0x1BAA0` within the kick.

| Case | Result |
| --- | --- |
| Eye view, empty air (facing away) | silent; 6 sphere calls |
| Eye view, looking 60 degrees down at open ground | silent |
| Eye view, pig cop in reach | 1 thud, same frame as the kick sound (frame 11-12); damage as before (e.g. 2250 -> 500) |
| Eye view, next kick, enemy out of the sphere (missed) | silent, health unchanged |
| Eye view, pig cop 800 away | silent, health unchanged |
| Eye view, head-on wall (game bump animation 94 seen first) | 1 thud per kick, twice; stepped back 1 s: silent |
| Eye view, alley garbage bag (type 219) | 1 thud; the bag breaks as before (word 0 `0x0` -> `0x4`); a kick at the empty spot after: silent |
| Third person, Boot selected, Q at the club door (object) | 1 thud, 32 host sphere calls in the kick |
| Third person, pig cop, 3 kicks (112, 112, 115) | each kills it (3750 -> 0), 1 thud each at first contact (frames 10-14); the original kick sound at 38-46 |
| Same, `DNTTK_KICK_IMPACT=off` (original call untouched) | 115 and 114 kill it (3750 -> 0), 113 missed: the same kind of outcome (the variant is random; 113 is a high kick) |
| Third person, kick away from the wall | silent |
| Vanilla route `vanilla-regression/d12c-vanilla` | exit 0; captures show the original camera |

- Native `ttk-controls-test` (PS-X EXE, LEVEL00 fixture, LEVEL01, all levels)
  with a new D12C case: silent in empty air and with an actor in view but
  not struck; one thud right after the first struck sphere call; a wall in
  reach; third-person host call with the same arguments, original radius
  neutralized, one thud per kick animation, other callers and Vanilla
  untouched. `ttk-input-test`, `ttk-inventory-test`, `ttk-aim-test`,
  `ttk-near-test`, `ttk-font-test` pass; Python 131 OK (9 skipped);
  `level_overlay_guards.py --check`; `check_repo.py`.

## Revision 2026-10-08: one octave up

User playtest: "i think i hear something but i cant tell if it's the right
noise ... it seems very quiet." Measured: 92% of the `0x2007` sample's energy
is below 150 Hz (a sub-bass thump; on the pad the bump also rumbles), and the
street ambience runs at about 2,000-2,500 RMS in the mix. The sample itself is
not weak (RMS about 10,800 against the kick swing's 4,900). Preview clips
(`recomp/analysis/d12c-kick-impact/preview/`, local); the user chose the same
thud one octave up.

The impact now goes through `0x8006b73c`, which is `0x8006b270` with the
voice's pitch byte (`+0x3e`) as its fourth argument. The voice setup reads it
signed at `0x80068740`: `0x40` (what `0x8006b270` always stores) is the table
pitch + 4 with a random 0..7 fine spread; any other value is the table pitch +
value with no spread. Measured for `0x2007` (SPU pitch per value): 0 `0x398`,
24 `0x5C0`, 28 `0x61C`, 32 `0x678`, i.e. `0x398` + 23 per step; `0x40` gives
`0x3F4`-`0x3F8`. Value 48 gives `0x7E8`, one octave above `0x3F6`. Same
sample, same position, same voice volume.

- Default pitch value 48; `DNTTK_KICK_IMPACT_PITCH=-64..63` (diagnostics,
  `0x40` = the original pitch) overrides it for one launch.
- Guards added: `0x8006b73c` (452 bytes) and the pitch read `0x80068738`
  (104 bytes).
- Evidence (executable `b62af2b911cfdcc7c300ac025f00f1218b6f8c7439bbd8eda460ce9243283e3d`): pig cop kick, thud KEYON at pitch `0x7E8`
  (kick swing `0x3F8`); wall kicks `0x7E8` twice; kick into air silent;
  `ttk-controls-test` (thud through `0x8006b73c` with pitch 48) passes.
  In-game recordings `preview/8-ingame-octave-kick-wall.wav` and
  `8-ingame-octave-kick-air.wav`.

## Revision 2026-10-08: twice the volume

User: "double it's volume". A second identical call does not layer: the
routine refuses a second instance of a sound already playing (one KEYON
measured). The volume comes from the sound table: `0x8006b73c` copies the
table entry into the sound object (`+0x64`..`+0x73`, `0x8006b850`), and
`+0x6a` is the volume the voice update ramps to (`0x80068ddc`) or sets
outright (`0x80069004`) before the positional mix. The call returns a handle
with the sound's address in its low 24 bits (`0x80067e74`). The host checks
`+0x5c` is `0x2007` and doubles that sound's `+0x6a` (capped at `0x3fff`);
the table and every other sound are untouched.

- `0x2007`'s table volume is 5192 (`0x1448`), now 10384 for the impact.
- Live voice volume while it plays (first street, pig cop kick): gain 1
  `0x464`/`0xC2A`, gain 2 `0x802`/`0x1855` (right exactly x2, left x1.83 from
  the positional pan rounding); final build `0x616`/`0x1855`.
- `DNTTK_KICK_IMPACT_GAIN=1..4` (diagnostics) overrides the multiplier.
- Guards for the two volume reads (`0x80068ddc`, 180 bytes; `0x80069004`, 16
  bytes). The native test checks the impact's sound is set to twice 5192.
- Executable `de112f13e75afef2e31543f48f09ec58c49ba4f6ada9a93b914d2e66cf0d9dbd`; in-game recordings `preview/9-ingame-loud-kick-wall.wav`
  and `9-ingame-loud-kick-wall-1.wav`.

## Revision 2026-10-08: three times the volume; the first kick at a wall

User: "let's set 3 to the new default multiplier ... sometimes, occasionally,
the first kick doesnt make a noise ... kicking against random walls ... when
holding kick down the first one is sometimes silent".

- Default multiplier 3 (`0x2007` table volume 5192 -> 15576, under `0x3fff`);
  live voice `0xC2A` -> `0x2480` (right, exactly x3).
- **Cause of the silent first kick:** `0x8006b73c` refuses a sound whose id is
  already playing (`0x8006b7b0`, table flag 4: it walks the playing list at
  `[0x800da5b8]`, linked through word 0, comparing `+0x5c`). Walking Duke into
  a wall plays the same `0x2007` (the bump, very quietly); a landing does too.
  A kick that lands while that sound still plays was counted as an impact but
  nothing sounded. Reproduced: `sfx 0x2007`, then a kick on the pig cop: only
  the console's `0x398` KEYON, no kick thud (twice); a clean kick plays it.
- **Fix:** before starting the impact, a `0x2007` still in the playing list
  (active, `+8` nonzero, sound state `[0x800c39b4]` == 7) is stopped with the
  game's stop `0x80068900` (what `0x8006af44` does for a handle; 53 callers).
  Same reproduction after the fix: `0x398` then the kick's `0x7E8` thud
  (frames 15-18), `replaced` 1; clean kicks replace nothing. Guard for
  `0x80068900` (52 bytes). Debug `impact.replaced`.
- Executable `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f`: empty air and floor silent, pig cop one thud per hit,
  missed kick silent; controls suite passes (three times the table volume).

## Limits

- **No multi-kick crate was found** in the private saves (no object in those
  scenes has health above 1). Every struck object goes through the same sphere
  result, so each connecting kick on one should thud, but this is not
  observed.
- **Third-person timing:** the original foot touches a close enemy early in
  the animation (frame 10-14), before the original swing sound (its event,
  frames 38-46). The thud follows the real first contact, which is also when
  the original's damage starts.
- Walls: the original segment query decides what is world. Thin props that
  are neither world nor kickable objects stay silent. Glancing walls beside
  Duke, outside the view heading, do not count in the eye view.
- Positional at Duke (as the bump), not at the contact point.
- Measured on the first street and the alley only. Other levels use the same
  bank-2 bump sound through the same code, but were not played.
