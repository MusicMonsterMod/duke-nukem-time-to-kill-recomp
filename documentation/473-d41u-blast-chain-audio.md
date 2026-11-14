# 473 - D41U: an explosion chain sounds as one blast

Session of 2026-11-14 (evening), on ripspace. User: "consolidate explosion
audio the same way D41T consolidated the lights. One voice per explosion
chain, game-wide ... Decide live while the game runs ... The first bang plays
at once with no added delay; a barrel far enough away to explode on its own
keeps its own sound. Tune it so a chain sounds like one big blast rolling
through the room, not a stack." Built; accepted and closed the same evening
(user: "working beautifully. i'd give it a 10/10 in fact. lock it in and commit").

Lab: the D41T lab `recomp/analysis/d50e/lab.py` (local, its own cards and
savestates: slot 0 level 7 RESISTANCE IS FEUDAL, slot 1 level 2 MINER 69ER).
New cases: `snd` (spawned chain, sound calls, KEYONs, peak active SPU voices,
the mix and voice-sum recordings), `sndpoll` (the game's sound lists every
poll), `ramdump` (RAM for disassembly). Offscreen, `SDL_AUDIODRIVER=dummy`,
`PSX_TRACE_HISTORY=1` for the audio rings. No copy of the player's saves or
cards; the lab checks the player's files are unchanged after every run.
Evidence labels: MEASURED, CODE, INFERRED.

## 1. Where the barrel's bang comes from (MEASURED, CODE)

`DNTTK_SOUND_TRACE` now also logs `0x8006b73c` and each call's position and
tick. One spawned type 944 barrel shot in level 7: the blast
(`0x80097090`, light hook) and in the same tick
`0x8006b270(0x6002, barrel+4, 0x800)` returning to `0x80098d5c`.

`0x80098a88` is the game's object death-sound dispatcher: a switch on the
object type `+0x2c` (types 9..0x40f, table `0x80017658`). Most cases set an id
and jump to `0x80098d4c` (`0x8006b270(id, object+4, 0x800)`); some use the
pitched `0x8006b73c` with a fixed or random pitch (-10, -15, -23 and random
for some types; TNT 299 plays `0x6001`). Every sound it plays returns inside
the routine (`0x80098a8c`..`0x80098d6c`). So a barrel's bang is "a sound from
this routine whose object's type has flag bit `0x10000`" (the D41T barrel
flag), with no per-type or per-level list. Guarded (744 bytes from
`0x80098a88`).

The sound system (CODE, from the routines):

| What | Where |
| --- | --- |
| Sound objects | pool of 64; free list `0x800da5b0`, playing list `0x800da5b8` (word 0 links) |
| Object fields | `+8` state, `+0x18` position pointer (own copy `+0x1c` unless a2 has `0x100`), `+0x3c/+0x3d` SPU voices (-1 none), `+0x3e` pitch, `+0x5c` id, `+0x64..+0x73` table entry, `+0x6a` volume |
| SPU voices | 24; free list `0x800da5c0`, taken by `0x80068658` in `0x80068690`. None free: the sound gets no voice and waits silent. Nothing is stolen |
| Refusal | id < 0 (`0x8006b270`) or bank `0xff` (table lookup `0x80068b20`, both routines): the call returns 0 and plays nothing |

## 2. Before (MEASURED; build of the D41T close, offscreen)

| Chain | Barrel voices started | Peak active SPU voices | Mix |
| --- | --- | --- | --- |
| 1 barrel (944) | 1 | 5 | peak 24934, no clipping, loudest 50 ms -11.0 dBFS |
| 8 barrels, level 7 | 8 (5 KEYON in one tick, then stages 6-7 tics apart) | 13 | 3995 clipped samples, -2.1 dBFS |
| 20 barrels bunched | 20 | **24 of 24** | 16786 clipped samples, -0.5 dBFS |
| MINER 69ER, 5 TNT | 5 | 9 | 95 clipped samples |
| RPG into 8 barrels | 8 | 11 | 5337 clipped samples |

With all 24 voices taken, any sound started during the chain (a shot, an
enemy, a pickup) gets no voice until one frees (CODE: no stealing). Each
`0x6002` is table volume 13000; a close barrel's voice already sits near the
SPU's per-voice ceiling (31148 of 32766, left + right).

## 3. What changed (CODE)

`recomp/src/ttk/blast_audio.inc` (new), hooks in `modern_controls.cpp`
(entries of `0x8006b73c` and `0x8006b270`, Duke's update for the tick),
console in `level_select.cpp`, guard in `control_guards.inc`. Modernized only
and not with the Classic renderer (`input_modernized() && !ttk_world_classic()`);
Vanilla never reaches it.

The same live rule as the D41T light, in game tics (30 a second):

1. **Head**: a blast with no live chain within 4500 units plays at once,
   untouched, and starts a chain.
2. **Merge**: a barrel within 4500 units of a chain whose bang began under 15
   tics (0.5 s) ago joins it: its call is refused (a0 = -1), the chain's
   voice swells (+12% table volume per blast, at most `0x3fff`) and its
   position moves 65% of the way toward the blast.
3. **Rumble**: the chain's next stage (a merge 3 or more tics after the head,
   or its third blast) adds the chain's one second voice: the merged blast's
   own sound through the pitched `0x8006b73c` at pitch value -2 (SPU `0x2C9`
   against the head's `0x355`-`0x3E0`, about 3 semitones down), table volume.
4. **Rolling**: a chain still going after 15 tics (each blast within 12 tics
   of the last) bangs again: that barrel plays as the new head and the old
   head and its rumble are ducked to 40% (a tail).
5. **Other explosives**: rockets and grenades (`0x80070418`), pipe bombs
   (`0x800708e4`), dynamite (`0x80071044`) and their under-water calls start
   chains like barrels but join one only when they go off together (within 2
   tics: pipe bombs detonated at once, a rocket into a row of barrels).
   Measured first with the barrel window: RPG blasts land 13-14 tics apart, so
   every other rocket was swallowed. Gib bursts (`0x8006413c`) are a splat and
   are left alone.

A savestate load clears the chains. Console `chainsound on|off|default`
(session A/B); developer `DNTTK_BLAST_AUDIO=0`, `DNTTK_BLAST_AUDIO_TRACE=1`.

Swelling alone does not make a chain bigger (one voice is already at its
ceiling, MEASURED), and barrels that join after the head's attack would be
lost in its decay: hence the one rumble. A chain holds at most two voices at a
time; more would be the stack again.

## 4. After (MEASURED; final build, same cases)

| Chain | Barrel voices started | Peak active SPU voices | Mix |
| --- | --- | --- | --- |
| 1 barrel | 1 | 5 | identical to before (peak 24934, -11.0 dBFS) |
| 8 barrels | 3 over the chain (head, rumble, one later bang) | 8 | no clipping, peak 23695, -11.1 dBFS, loud (within 6 dB of peak) 750 ms against 250 |
| 20 barrels bunched | 2 (head, rumble) | 6 | no clipping, peak 24644, -11.2 dBFS |
| MINER 69ER, 5 TNT | 2 | 6 | no clipping, peak 19520 |
| RPG into 8 barrels | rocket + 3 barrel voices over the chain | 6 | no clipping, loud for 1000 ms |
| RPG held, 5 rockets at the floor | 5 rocket blasts | 9 | identical KEYONs to before |
| `chainsound off`, 8 barrels | 8 | 13 | as before (4157 clipped samples) |

So no other sound is kept from a voice during a chain any more (6-8 of 24 in
use at the peak, against all 24). The trade-off, for the playtest: the chain
is now about as loud at its peak as one close barrel (it was +9 to +10 dB and
clipped), but lasts longer and has its second, deeper boom.

Native: `ttk-controls-test` group D41U (merge, swell and move, far barrel,
non-barrel type, other caller, Vanilla, `chainsound off`, rolling bang with
tails, rocket together and apart, one rumble). The suite itself stops later
at an older, unrelated group (section 6).

Recordings and waveform pictures (before on top, after below):
`research/before-after/D41U/` (local): one barrel, 8-barrel chain, 20 bunched,
MINER 69ER TNT, RPG into 8 barrels. Recorded from the SPU output (tap 0, the
mix the player hears before the host's volume), offscreen.

## 5. Tuning knobs (for the playtest)

`blast_audio.inc` constants: reach 4500, bang window 15 tics, together window
2, rumble pitch -2 and gain 1.0, tail 40%. If the chain should hit harder,
the measured options are a deeper or louder rumble, or a third voice for the
biggest chains (each extra voice about +2-3 dB, the stack's clipping returns
past a few).

## 6. Remaining

- The user's playtest (RESISTANCE IS FEUDAL barrel room, MINER 69ER), in
  particular whether the chain is big enough (section 5).
- Not exercised in game: dynamite and pipe bomb blasts, under-water blasts
  (same rule, by caller; covered by code reading and, for the rule, the
  native group).
- `ttk-controls-test` was stale before this job: it did not link (D23Z
  symbols `s_frame_count`, `g_psx_mod_hook_observer`, `gpu_vram_copy_async`;
  stubs added) and, with the fixtures on this machine, stops at the D-HUD
  layout group (`hud_intact`, line 1839) or an earlier code-write check. The
  D41U group was placed before them and passes. The stale groups were not
  investigated.
