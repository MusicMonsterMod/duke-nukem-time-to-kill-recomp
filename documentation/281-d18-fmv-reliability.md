# D18 - FMV and audio reliability pass

Status: Needs playtest, reliability pass 2026-10-27. Findings distinguish original
source/disc evidence, runtime measurements, diagnostic fixtures and unverified
listening/campaign coverage. User authorized session lock-in, documentation,
commit and push on 2026-10-27 after the previous session clear. This closes the
investigation session; D18 remains Needs playtest with the checks below open.
D10E and D18F remain Todo. No new gameplay implementation or playback claim.
Closeout reran all four poll/DMA math tests and checked runner syntax/CLI help.
The framework export is unchanged and clean-pin patch verification passes.

## Scope and preservation

Reliability of movie playback, skips, transitions, cadence, audio continuity,
display changes and closing during playback. D18B concurrent voice/music,
D18C gameplay crackle and D18E challenge music remain separate jobs.
Modernized is the default; Vanilla remains a one-launch option. No gameplay,
movie-decoder, original media or framework source edits in this investigation.
All runs use new private settings and blank cards under
`recomp/analysis/d18-reliability/`. No player cards or saves are used.

Player executable SHA-256:
`16126a9f6c4e8ac50839508f1bfcc2474863257e5f5ed3e10ef8e967abafbf7a`.
Pinned framework `08ec704a974b1f3a16335b4afeb340b9eff19926`, UI
`be8ac1d03ee19d55394b5a5f2d9d1506edd56659`. Existing framework working
changes were present before this pass and are preserved.

## Movie inventory and native routes

Read-only ISO9660/STR inspection of the owned US disc found 16 STR files: eight
A/B name pairs. All eight B files are byte-identical logical payloads, SHA-256
`e5275c6de5d0e7c0bdcec93274aafb74b98260e13c66d6511cfcf756210ae3c6`.
They contain 100 frames at 640x192; they are not eight independently distinct
movies. Do not assume their runtime use from the filenames alone.

The resident movie table at `0x800c6764` contains eight count/sector-offset
records. `0x800aaea8` uses the ID supplied by the original wrappers, adds the
disc base at `0x800d7178`, seeks and streams through MOVIE.OVR. The native
termination counts differ from the number of frame headers on disc; this is
original behavior, not evidence of a recompiler truncation.

| ID | A clip | LBA | STR frames | Native count | Frame dimensions |
| --- | --- | ---: | ---: | ---: | --- |
| 0 | LOGO | 92103 | 150 | 150 | 320x240 |
| 1 | GTLOGO | 61348 | 222 | 120 | 320x192 |
| 2 | INTRO | 66907 | 2187 | 2160 | 320x240 |
| 3 | VICTRY | 115889 | 584 | 500 | 320x240 |
| 4 | RTOPEN | 94707 | 2007 | 1997 | 320x240 |
| 5 | CREDIT | 26755 | 3349 | 3300 | 320x240 |
| 6 | KILT | 89880 | 112 | 95 | 320x240 |
| 7 | 3DRLOGO | 24002 | 165 | 150 | 320x240 |

Source route evidence:

- Startup `0x8001b59c..0x8001b61c`: INTRO, GTLOGO, optional RTOPEN when
  the four tested shoulder-button states are held, then 3DRLOGO and LOGO.
- Campaign completion `0x80028178..0x8002819c`: VICTRY, conditional KILT
  when the difficulty value at `0x800c3d7c` is 3, then CREDIT.
- Native replay selector `0x800c3d80`, dispatched by `0x800283a0`: 1 INTRO,
  2 VICTRY, 3 CREDIT, 4 KILT. This belongs to mode 3's lifecycle, not the
  title menu. Merely changing the selector at the title is not a valid route.
- Each inspected movie wrapper passes the original player's skip-enable flag.
  The loop polls original pad edges and runs its own streaming teardown.

Raw disc data, captures, audio and generated game code remain local.
Inventory metadata: `recomp/analysis/d18-reliability/disc-movies.json`.

## Baseline and diagnostic limitations

`startup-modern120` ran the unmodified startup at Modernized 120, 4x internal
resolution, 960-pixel window, OpenGL on Xvfb and PulseAudio/PipeWire output.
The native movie shard logged 19 active functions. Clip-interior samples:

| Clip | Observed sample interval | Guest ticks/s | Added output underrun samples |
| --- | ---: | ---: | ---: |
| INTRO | 143.47 s | 59.944 | 0 |
| GTLOGO | 8.27 s | 59.884 | 0 |
| 3DRLOGO | 9.26 s | 60.050 | 0 |
| LOGO | 9.30 s | 60.006 | 0 |

INTRO decoded about 15 images/s; 120 Hz presentation did not double the movie
speed. Endpoint sampling is not a frame-exact comparison to original hardware.
Startup audio overflow counters were nonzero and must not be described as zero;
they stayed stable through the measured intro. Audible sync remains a separate
review requirement.

After the logos, the 4x Xvfb run slowed to about 2.1 guest ticks/s during menu
loading and accumulated output starvation. At investigation time there was no
active backup/update process found, CPU pressure was elevated, I/O and memory
pressure were zero, and approximately 23 GiB RAM was available. This snapshot
does not reconstruct every earlier instant or establish causation.
`glxinfo -B` confirms Xvfb uses unaccelerated llvmpipe; the normal desktop uses
an NVIDIA GTX 1080 Ti. Treat the offscreen load result as an observation requiring
a matched hardware-backed comparison, not a confirmed player regression.
A separate 1x skip-startup route reached the title with zero output underruns;
its different route prevents attributing the difference solely to resolution.

Excluded harness attempts:

- Initial title detection incorrectly accepted pre-boot zero-initialized RAM.
  Corrected to require initialized game state. No movie coverage claimed.
- Directly setting native mode 3/selector from an allocated title state did not
  start playback within 30 seconds. This bypasses the original resource cleanup;
  no product defect or valid ending-route coverage inferred.
- Frame 100 was earlier than resident table loading. Fixture now waits for an
  exact 64-byte match to the owned executable and requires zero prior MDEC decodes.
- A port reuse check encountered TIME_WAIT; corrected the harness's probe socket.

For remaining clip-decoder tests, a private startup fixture replaces only table
record 2 with another original record after identity verification. It changes
RAM data, not code, disc or saves. The original startup movie player, native
count, decoder, skip and teardown paths execute. This verifies clip playback;
it does not certify natural campaign completion, difficulty gates or the replay
menu lifecycle. Each run records the replacement and observed streamed LBA.

## Verification and remaining work

The existing `test_fmv_poll_math.py` suite passes all four tests, including the
20,000 randomized event/timeout boundary cases. This is poll arithmetic and DMA
exclusion evidence, not listening, visual fidelity or campaign acceptance.

Early audio exports used the debug API's default `start=-1`, which means the
oldest retained PCM rather than its latest tail. Those early host/SPU snippets
contain boot silence and are excluded from movie sound conclusions. The runner
now uses each tap's absolute frame count minus four seconds and waits for
presented screenshot completion. A corrected RTOPEN midpoint capture contains
nonzero CD, SPU and host audio; host peak 11287/32768, no full-scale samples in
that four-second slice. This does not establish absence of all audible artifacts.

The full CREDIT fixture recorded a 220.101-second clip interval at 59.941 guest
ticks/s, with zero added output underrun or overflow samples. VICTRY recorded
33.169 seconds at 59.935 ticks/s with the same zero-delta result. Both proceeded
through the following startup logos and exited cleanly. Each captured command
history passed 13 explicit seek-to-read checks, with no wrong-location result.

The expanded runtime matrix follows. Final listening/sync,
natural ending-route coverage and any remaining display/transition limits must
stay explicit. Launch the ordinary player from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

## Expanded matrix (2026-10-27)

All timings below are sampled guest progress, not display refresh or a claim of
bit-identical playback. Native count/sector selection was retained for every
clip. Offscreen runs use Xvfb/llvmpipe, OpenGL 1x and the PulseAudio backend unless
noted. Baseline above used 4x. Original B alternatives were inventoried but not
independently played, since the inspected native table selects the A assets.

| Private run | Coverage | Observation |
| --- | --- | --- |
| `ending-table-r3` | VICTRY full, subsequent logos; Modernized 120 | 33.169 s sampled VICTRY, 59.935 guest ticks/s; no added clip underruns/overflow; clean exit |
| `credits-table-modern120` | CREDIT full, subsequent logos; Modernized 120 | 220.101 s sampled CREDIT, 59.941 ticks/s; no added clip underruns/overflow; clean exit |
| `rtopen-full-120` | RTOPEN full, subsequent logos to title loading; Modernized 120 | 133.080 s sampled RTOPEN, 59.934 ticks/s; no added clip underruns/overflow; clean exit |
| `kilt-full-120` | KILT full, subsequent logos to title loading; Modernized 120 | 6.353 s sampled KILT, 59.971 ticks/s; no added clip underruns/overflow; clean exit |
| `intro-display-60` | INTRO, two resize/F11 requests, Start skip at about 25 s, logos/title loading; Modernized 60 | 25.751 s sampled INTRO, 59.958 ticks/s; no added clip underruns/overflow; 12 seek checks pass |
| `intro-quit-modern` | Quit while INTRO/XA active; Modernized 120 | Exit 0 without forced termination; no added measured clip underruns |
| `intro-quit-vanilla` | Quit while INTRO/XA active; Vanilla 60 | Exit 0 without forced termination; no added measured clip underruns |
| `modern-gameplay-r4` | Early INTRO skip, logos, custom menus and fresh gameplay; Modernized 120 | Music stream resumes, 21 seek checks pass, clean exit; gameplay output starvation recorded below |
| `vanilla-gameplay-r2` | Early INTRO skip, logos, native menus and fresh gameplay; Vanilla 60 | Music resumes; zero measured output underruns; clean exit |
| `intro-late-skip-vanilla` | INTRO late skip and following logos | Clip/skip observations valid; attempted gameplay automation timed out at main-menu readiness and does not prove gameplay recovery |

RTOPEN and KILT histories each passed 12 explicit seek/read checks. Quit runs
each passed 16 checks. Counts include boot/file reads, not just movie seeks.
One Vanilla quit run already had 1333 output missing samples at the first movie
sample; the count did not increase over the observed clip. No precise cause or
audible consequence was established. Do not describe startup as universally
underrun-free. Other zero-delta results refer to the measured clip intervals,
not all process startup/loading counters. Loading also adds overflow-drop
counts even in runs with no output starvation.

Presented PNGs were inspected for movie content and resize behavior. The Xvfb
F11 requests produced the fullscreen OSD, but Xvfb has no normal desktop window
manager; this is not proof of a real fullscreen mode change. Both display
captures retained the fullscreen label. Hardware/WM fullscreen acceptance
remains pending.

Two desktop attempts (`hardware-full-120`, `hardware-retry-display`) did not
reach a frame or OpenGL initialization before the diagnostic timeout. The
second explicitly selected X11 and disabled compositor bypass. Its own window
was present but `IsUnMapped`; forced private-process cleanup was necessary.
No movie ran, so these are excluded from playback evidence. Attaching a debugger
was disallowed by the host ptrace policy; no host setting was changed. The
hardware-backed 4x comparison and the cause of the window-init stall remain
unverified. Do not infer a movie regression, or normal-player reproducibility,
from this diagnostic startup failure.

A title state value of 5 with main-loop phase 2 can still show the loading
spinner. The runner now records loading separately and, for `--gameplay`, waits
for the interactive main menu using released Start presses before proceeding.
The earlier title captures do not certify readiness for input.

## Reproduction tool

`recomp/tools/local/fmv_reliability.py` is the only authored implementation
addition. It does not change the player executable. It refuses an existing
player process, occupied port, existing output directory or wrong owned EXE;
waits for the resident table identity; records every diagnostic table replacement
and pad input; creates private preferences/cards; waits for presented captures;
exports the latest four seconds of each audio tap; records failures separately
from exit codes; and closes the private process group on a diagnostic timeout.
Window operations are restricted to the verified private runtime PID.

Examples from the workspace root (choose new names for repeat runs):

```sh
xvfb-run -a python3 recomp/tools/local/fmv_reliability.py --name d18-intro-review --action skip --gameplay
xvfb-run -a python3 recomp/tools/local/fmv_reliability.py --name d18-credits-review --movie credits
xvfb-run -a python3 recomp/tools/local/fmv_reliability.py --name d18-vanilla-review --mode vanilla --rate 60 --action quit
```

Xvfb results are not hardware-backed fullscreen or listening certification.
Omit `xvfb-run -a` for a desktop test, subject to the unresolved initialization
limit above. Raw output is ignored under `recomp/analysis/d18-reliability/`.
Do not commit retail-derived movie/audio captures or private cards.

The default debug CD-history response contains only 128 records. After 30 seconds
of gameplay this can contain only GetlocP polls and zero testable seek/read
pairs; zero is not a pass. The runner now explicitly requests up to 4096 entries
from the 8192-entry ring. Earlier movie-run checks were nonempty as reported;
the first completed Vanilla gameplay report's final 128-entry history had zero
checkable pairs and is not counted as a passing seek regression.

## Gameplay handoff evidence

`vanilla-gameplay-r2` completed the corrected early-skip -> startup logos ->
interactive main menu -> difficulty -> fresh first-level route. Thirty gameplay
samples span about 30 seconds; output underruns remained zero. By the second
sampled phase, the read position was within `/SOUND/MUSIC1.IDF;1` (starts at
LBA 144480) and continued through that file, rather than advancing through the
speech bank. Gameplay captures and current-tail audio are retained locally.
This establishes the bounded route; it does not assert every voice trigger or
sample-identical output. The final history defaulted to 128 GetlocP-heavy
records, so this run has no claimed seek-pair pass count.

`modern-gameplay-120` reached the Modernized custom main menu (owned page 5,
frontend state 12), but the first driver incorrectly waited for Vanilla's state
7 and timed out. No gameplay failure inferred. A supplemental 4096-entry
history passed 19 boot/movie/file seek checks; it did not yet include the level
music handoff. The runner now checks the actual owned Modernized main page
before proceeding. A second attempt (`modern-gameplay-r2`) identified the same
distinction for difficulty: Modernized owns page 6 while its frontend state
remains 12, and only hands back to native state 8 on confirmation. Both checks
now use the source-verified owned pages. These corrections affect the diagnostic
tool only.

The third attempt (`modern-gameplay-r3`) still sent native Cross input to the
host-owned menu. The corrected runner uses SDL Return events for Modernized
confirmation, restricted to its verified private window; Vanilla retains native
Cross. No player menu defect was inferred from those driver timeouts.

`modern-gameplay-r4` then completed the route and exited normally (98.44 seconds,
exit 0, no forced cleanup). Thirty gameplay samples span 30.99 seconds. Presented
captures at approximately 5 and 25 seconds show the first level and active
simulation. The 4096-entry history passed 21 seek/read checks with zero failures.
After the initial speech sectors (124043-124313), reads moved into MUSIC1 at
LBA 144528 and progressed to 147922. No repeated voice-bank stream appeared on
this bounded route.

**Open finding: Modernized offscreen gameplay audio starvation.** Output underruns
were zero through the movie/menu route and first gameplay sample, then increased
to 121327 missing output samples across the level-load/gameplay interval. The
first two one-second intervals advanced only 13 and 12 guest ticks; the counter
also increased intermittently after music resumed. This is not a clean gameplay
audio pass. The successful Vanilla route had zero output underruns. These are
separate runs and different rendering/control profiles, not a controlled causal
comparison. The inspected PCM/counters do not establish perceived sound quality.

Immediately after the Modernized run, CPU pressure avg10/avg60 was 0.78/1.31,
I/O and memory pressure avg10/avg60 were zero. This post-run snapshot cannot
establish contention during every stall. Xvfb uses software rendering; no
hardware-backed reproduction or exact cause is established. Keep this evidence
alongside the earlier 4x menu-load starvation and D18C/D17 performance work;
do not infer decoder damage or broaden this pass into an unproven audio fix.

## Review limits and player checks

- Listening/lip-sync acceptance on the real audio device remains open. Backend
  counters and PCM samples do not prove that every clip sounds correct.
- Real desktop fullscreen/resize and closing with the window's close control
  remain open. Automated clean exits above used the normal debug quit request.
- The optional startup shoulder chord and campaign-completion/difficulty gates
  were inspected in source. RTOPEN/ending/credits/KILT playback used the recorded
  startup table fixture; their natural lifecycle entry is not certified by it.
- Linux is the measured platform. No Windows playback claim, full campaign
  fidelity claim, or replacement/scaling/frame-interpolation feature is added.
- The 4x software-rendered menu-load slowdown and the desktop unmapped-window
  startup attempts are retained as unresolved diagnostic observations. They do
  not justify changing the accepted movie decoder on this evidence.

For player review, launch normally, watch/listen through the intro and logos,
try Escape/Enter skipping, resize/F11 and close during playback, then confirm
that starting a game gives the expected speech and music. Repeat the core route
with the one-launch Vanilla command. Natural ending/credits coverage should be
recorded when reached in campaign testing. These commands use saved player
preferences and are not the isolated diagnostic runner:

```sh
python3 recomp/tools/local/run.py
python3 recomp/tools/local/run.py --vanilla
```
