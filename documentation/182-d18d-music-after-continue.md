# D18D - Music silent after death and Continue

Job: [D18D](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "i can confirm the music started with no further issue").

## Symptom

After Duke dies and the player chooses CONTINUE, the level music stays
silent until Duke's next voice line (user report, 2026-10-04). Reproduced
in a private Xvfb run in both Modernized (our D19H death list) and Vanilla
(the original death screen). The cause is in the emulated CD drive, not in
the menus, so both profiles share it.

## The game's music player

Music and speech are XA streams on the one CD drive. A small state machine
in the executable plays them (`0x8003b780`-`0x8003c4bc`):

| Address | Meaning |
| --- | --- |
| `0x800c26a8` | drive state: 0 idle, 1 seek started, 3 seek done, 2 music playing, 6 voice playing, 9 paused |
| `0x800c26ac` | seek-complete flag, set by the seek callback |
| `0x800d7184` | saved music position (resume point) |
| `0x8003c240` | resume music at the saved position: Mute, Setfilter, Setmode, Setloc, `CdControl(SeekL)`, then `CdSyncCallback(0x8003b838)`, state 1 |
| `0x8003b838` | SeekL callback: on CdlComplete, state 3 and flag set |
| `0x8003b9d0` | per-frame update: state 1 or 3 with the flag set -> Demute, ReadS, state 2 |
| `0x8003c478` | pause music (CdlPause, state 0), called at death |

The callback is installed only after `CdControl(SeekL)` returns. libcd's
`CdControl` (`0x800b60f8`) clears the sync callback while a command runs and
restores the old one afterwards, so a seek that completes inside `CdControl`
never reaches `0x8003b838`.

## Cause

At death the game pauses the drive a few sectors past the music's saved
position (32:29:15 vs 32:29:08). On CONTINUE, `0x8003c240` seeks back. The
runtime completed a "near" seek (16 sectors or less) after 0x800 cycles
(about 60 microseconds). The completion interrupt arrived inside
`CdControl`, the callback never ran (function trace: `0x8003c240` and the
callback install at frame 744, no `0x8003b838` call), and the state stayed
at 1 with the CD muted. Music came back only when the next voice line
restarted the player from scratch.

Reference emulators time a near seek far longer:

- Beetle PSX `PS_CDC_CalcSeekTime`: travel `sectors * 33868800 / 324000`,
  at least 20000 cycles, plus 1237952 cycles (double speed; twice at
  single speed) when the drive is paused.
- DuckStation `CDROM::GetTicksForSeek`: at least about two sector periods
  for a short seek, plus 0.5-1 ms.

## Change (framework, `runtime/src/cdrom.c`)

`seek_complete_delay_cycles()` keeps the far-seek time (four 1x sector
periods, PCSX). A near seek now follows Beetle: travel time, minimum 20000
cycles, plus the restart delay when SeekL/SeekP is issued to a paused drive
(motor on, not reading, playing or seeking). Exported into
`recomp/patches/time-to-kill-accepted-source.patch`; the patch applies to a
clean pinned framework checkout (`08ec704`) and matches the working tree.
No savestate format change, no game hook, no generated code.

## Evidence

Private profile, cards and savestates (`recomp/analysis/d18d-music`,
copied; script `x.py`, Xvfb window, `SDL_AUDIODRIVER=dummy`). Death made by
setting health to 1 next to enemies. Executable
`23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a`.

| Check | Before | After |
| --- | --- | --- |
| Modernized death list, CONTINUE | state 1, CD muted, no read for 9 s until the next death | callback 2 frames later, state 2, ReadS streams MUSIC (32:31 -> 32:43 in 6 s) |
| Vanilla original death screen, CONTINUE | (same CD path) | state 2, ReadS streaming |
| Level start (`level 1`): speech then music | - | voice (state 6, channel 6) -> 3 -> music (state 2, channel 2) |
| Level load cost (`level 1`, `level 3`) | - | one near seek from pause per load: +~1.26M cycles (~37 ms); far seeks unchanged |
| Intro FMV | - | 297 MDEC decodes in 24 s (normal cadence), no underruns |

User playtest: the music resumed after Continue with no further issue.
Not verified by tooling: the DuckStation portable
copy or real hardware on the same route, other levels' death/Continue, and
the D18A/D18C crackle routes.
