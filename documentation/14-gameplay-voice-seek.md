# Gameplay voice stream continuing instead of background music

## User report and current result

After confirming smooth movies and gameplay, the user reported that Duke
appeared to play successive voice clips on entering gameplay. Investigation
found a reproducible CD seek bug. The patched build correctly changes from
the speech asset to the requested music asset. Listening confirmation remains
separate from the command-history checks below.

The user also confirmed the product direction: modern controls are a central
feature, with actual changes to movement, camera and aiming rather than button
remapping. A modern third-person implementation can precede first-person mode.
See [the updated controls design](06-first-person.md). The immediate milestone
is still faithful original game behavior.

## Evidence from the game

In the baseline first-level transition, the game first selected XA file 1,
channel 7, and read MSF `27:35:41`. It later changed the filter to channel 1,
issued Setloc `32:08:30`, SeekL, then Demute and ReadS. However, that ReadS
continued at `27:41:07`, 20048 sectors before the intended position.

The raw-disc ISO map resolves those positions as follows (absolute MSF includes
the usual 150-sector lead-in):

| Position | LBA | Asset |
|---|---:|---|
| 27:35:41 | 124016 | start of `/SOUND/DISM02.IDF;1` |
| 27:41:07 | 124432 | still inside `/SOUND/DISM02.IDF;1` |
| 32:08:30 | 144480 | start of `/SOUND/MUSIC1.IDF;1` |

This supports the reported progression through speech: the game requested
music, but the runtime changed XA channel while continuing through the previous
asset. The filter itself selected the requested channel correctly in the
captured gameplay sectors. An initially considered unfiltered-XA selection
issue was therefore left unchanged.

## Root cause and change

The pinned runtime's SeekL/SeekP command handler reset XA decoding and stopped
CDDA, but did not stop the active data/XA read stream. Seek completion then
cleared `setloc_pending`. The next ReadS reached
`read_continues_current_stream()`, saw an existing read with no pending Setloc,
and acknowledged it as a continuation without moving to the seek target.

The fix calls `stop_read_stream()` and clears the READ status bit when an
explicit SeekL/SeekP is accepted. Existing seek latency and interrupt handling
remain in use. The next read starts from the requested seek address. This is
a controller-state correction, not a voice blacklist, forced mute, or scripted
replacement of game audio. The existing seek-completion status compatibility
behavior is retained; broader CD-controller parity is not claimed.

The investigation also checked the explicit-seek state transition in
[DuckStation's CD controller](https://github.com/stenzek/duckstation/blob/master/src/core/cdrom.cpp).
That implementation separates seeking from active reading. It is supporting
reference evidence, not a hardware differential test of this build.

## Reproducible source change

The runtime submodule now has a local modification at
`recomp/psxrecomp/runtime/src/cdrom.c`. The exact patch is preserved in the
parent checkout at `recomp/patches/time-to-kill-cd-seek.patch`.

`tools/local/apply_runtime_patches.py` applies it to the pinned source, or
recognizes the already-applied patch. It fails on conflicting source instead
of resetting local work. The normal `tools/local/build.py` calls this step
before building. Existing generated game and movie code need no regeneration
for this controller-only fix.

For a previously built checkout, from `recomp/`:

```sh
python3 tools/local/apply_runtime_patches.py
cmake --build --preset local-dev --parallel 4
python3 tools/local/run.py
```

## Regression evidence

`tools/local/check_cd_seeks.py` evaluates recorded `cdrom_command_history`
responses. It checks the first ReadN/ReadS after each explicit SeekL/SeekP.
Another Setloc supersedes the old target. A 16-sector observation allowance
covers command-service latency; this is a location regression check, not an
assertion of exact mechanical timing. Histories with no tested seek fail.

- Baseline: 22 checked transitions, four wrong locations, including the
  speech-to-music transition and three movie seeks.
- Patched: 21 checked transitions, zero wrong locations. Every tested first
  read was at the exact requested MSF, including `32:08:30` for music.
- Windowed gameplay was reached after the patch.
- A fresh live FMV interval advanced MDEC decode count 261 to 429 at about
  60.05 guest frames/s, with the audio underrun counter staying at zero.
  See `reports/voice-seek-live-fmv.json`. An earlier 20-second benchmark had
  constant MDEC count and is not treated as movie-playback evidence.
- The saved runtime patch was applied to a temporary copy of the pinned
  original source, compared byte-for-byte with the working runtime, and
  reversed back to the original successfully.

Reports: `voice-command-history.json`, `voice-seeks-before.json`,
`voice-fixed-cdrom_command_history.json`, `voice-seeks-after.json` in `reports/`.
Build and playback logs use the `voice-*` prefix. Local screenshots and the
downloaded reference source are under ignored `recomp/analysis/voice-investigation/`.

To check another captured history:

```sh
python3 tools/local/check_cd_seeks.py ../documentation/reports/voice-fixed-cdrom_command_history.json
```

Full-campaign voice selection, music looping, interruption by combat lines,
and audible reference parity still need verification. Do not equate correct
seek positions with proof that every sound is now identical to original hardware.
