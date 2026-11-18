# 509 - Overnight chain 2026-11-18: D23AG, D23AF6, D23AH, D23AI, D23AJ

User, 2026-11-18: "work autonomously overnight on all three PCs, in order:
D23AG (FRAME RATE relabel: every value counts new pictures, the original look
named e.g. ORIGINAL (30); propose wording, leave for my playtest), D23AF6 (fix
the identical-picture validator), then new jobs D23AH (PGO/LTO build), D23AI
(BALANCED GPU passes on the laptop's HD 5500) and D23AJ (laptop campaign
census, SIMPLE and BALANCED at 30 and 60 new pictures a second). Goal:
BALANCED at 60 new pictures a second on the laptop (FRAME RATE 120 on its
60 Hz screen)". Later: "you can use the screen on all 3 machines btw".

Machines: the desktop (i7-5960X, GTX 1080 Ti, 180 Hz), the field laptop
(i7-5600U 2 cores / 4 threads at 15 W, Intel HD 5500, 60 Hz panel) and
citadel (Ryzen 3 3200GE, Vega 8, 60 Hz TV); see note 503 for the census
method. Evidence labels: MEASURED (a run on the named machine), CODE (read
from the source), PROPOSED (not built).

## 1. D23AG - FRAME RATE counts new pictures

### Before

CODE (`src/ttk/frame_replay.cpp`, `player_profiles.py`): FRAME RATE `60`
was the original presentation: no redraws, the game's own pictures (about
30 new ones a second, fewer in busy scenes) each shown twice at 60 Hz. Every
other number was a redraw target (D23Y6: capped at the display's refresh),
so on a 60 Hz screen `120` gave 60 new pictures a second. The runtime used
60 itself as the "original presentation" sentinel.

### Change

- Saved value and runtime token `original` for the original presentation,
  shown in OPTIONS > DISPLAY as **ORIGINAL 30**. `60` is now redraws at 60
  new pictures a second. Wording proposal: the user suggested `ORIGINAL
  (30)`; the menu font draws `(` and `)` as `<` and `>` (screenshot:
  `ORIGINAL <30>`), so the label has no brackets. The help line reads
  "NEW PICTURES A SECOND. ORIGINAL: AS ON THE PS1. SAME GAME SPEED".
- Runtime (`frame_replay.cpp`): `parse_rate` returns its own sentinel for
  `original` (and for an unreadable value, as `60` was before); 60 now runs
  the redraws with in-between pictures (`interp_replay` from 60 up). A fixed
  60 on a display within 5% of 60 Hz is timed on the display, exactly the
  path `120` takes there (D23Y6), so it never beats against a 59.94 or
  60.01 Hz panel; other fixed rates are unchanged.
- Launcher schema 79 (`player_profiles.py`): a saved `60` (both profiles)
  becomes `original` once, with a notice; nothing else changes, so every
  existing setting looks the same after the update. Vanilla launches with
  `original`. `--frame-rate`, `--settings` F, the profile summary and
  `fmv_reliability.py --rate` use the new names.
- GAME_MANUAL "Frame rate (Modernized)" rewritten in new pictures; the four
  D10G3/D10G4/D49/D49C paragraphs say "FRAME RATE 60 or higher (the extra
  pictures)".

### Evidence

MEASURED, desktop offscreen, BALANCED third person, census route
(`recomp/analysis/d23ag`, local): FRAME RATE 60, levels 0 and 27: game
59.9, new pictures median 59.8 / 59.9 (lowest tenth 57.8 / 59.0), redraws
own the cadence every second; `original`: no redraw workers, the original
presentation every second, game 60.

MEASURED, real 60 Hz screens, BALANCED third person, levels 0 and 27 (new
pictures a second, median / lowest tenth; game 60 everywhere):

| machine, level | FRAME RATE 60 | FRAME RATE 120 |
|---|---|---|
| field laptop, 0 TIME TO KILL | 59.9 / 58.6 | 60.0 / 59.5 |
| field laptop, 27 THE REAPER | 41.6 / 32.1 | 44.4 / 34.2 |
| citadel, 0 | 59.7 / 57.0 | 60.0 / 58.6 |
| citadel, 27 | 59.9 / 58.6 | 59.5 / 57.6 |

Both machines log "frame rate 60 on a 60.0x Hz display: presents at the
display's rate", the line 120 logs; the two are the same path within run
noise. The laptop's THE REAPER at BALANCED is short of 60 either way
(D23AI). Note: in the original presentation the census's `pictures` counts
presents (60), not new pictures.

MEASURED, desktop, OPTIONS menu (offscreen, private profile): the row shows
`ORIGINAL 30` and `60`; confirming 60 from an `original` launch logs
"frame rate 60" and starts the redraws at once; after quitting, the profile
holds `60`.

Tests: `test_frame_rate_migration_79` (six rates, both profiles, the env the
launcher passes), `test_menu_frame_rate_choices_match_the_launcher`, the
schema bump through the existing tests.

MEASURED, desktop real 180 Hz screen (`DISPLAYMODE=real`, ULTRA rows,
level 0, VSYNC on; presents / new pictures a second, medians):

| FRAME RATE | before | after |
|---|---|---|
| AUTO | 180 / 179.8 | 179.8 / 179.8 |
| 180 | 180 / 179.8 | - |
| 60 | 180 / 89.6 | 60.0 / 59.9 |
| 120 | 180 / 177 | 180 / 175.5 (unchanged, D23AG1) |

CODE (framework, `gl_renderer_rp_tick`): D17BH's vsync placement put the
next present in the refresh after this one (times the governor divisor),
ignoring the rate's own period, so every fixed rate below the panel
presented at every refresh. Now a rate whose period is a whole number of
refreshes (within 2%) presents every that many refreshes. Placing other
ratios at their own rate was tried and measured worse (120 on 180 Hz: 119.9
presents but 67-82 new pictures: the in-betweens are planned evenly at the
rate, presents one or two refreshes apart show some twice), so those keep
every refresh; follow-up D23AG1. FRAME RATE 60's lowest tenth on the
180 Hz screen was 42.8-54.3 over three runs (a missed slot costs three
refreshes); medians 59.8-59.9.

Remaining: the user's playtest of the wording and of 60 on the real
screens; D23AG1.

## 2. D23AF6 - the identical-picture validator

### Where the differences were

MEASURED, desktop offscreen, ULTRA rows (RELIT, smooth shadow edges, 4x),
FRAME RATE 180, three workers, third person, `PSX_RT_VALIDATE=1`; the lab
census now reads the validator's exact / bad / skipped counters before each
`level N` travel, on arrival and after the route:

| run | travel to 1 | play 1 | travel to 0 | play 0 | travel to 27 | play 27 |
|---|---|---|---|---|---|---|
| before, classes on | 8 / 158 bad | 671 / 0 | 85 / 0 | 612 / 0 | 149 / 1 | 749 / 0 |
| before, `PSX_SHADOW_CLASSIFY=0` | 173 / 1 | 735 / 0 | 90 / 1 | 505 / 0 | 10 / 119 | 736 / 0 |

(exact / bad). Every gameplay comparison was exact. The bad ones all came
from one loading screen per session (which one depended on timing: the
first travel, or a later one), always the spinning radiation logo: its
lit half up to 11 levels apart, the second draw about 1.2% darker. Shadow
classes and D41P (`DNTTK_D41P=live`, 154 bad) were not the cause. This is
D23AF's "158 of 158 / 166 of 166": the census's validations at that time
were nearly all from those loading screens.

### Cause

A developer state signature (`gl_rt_state_sig`, 16 words: bake generation
and on/off, lamp levels and their hash, bake eye, lights and ambient,
shadow mesh generation and slot stamps, shadow edges, scale) is now stored
with each render-thread image and compared by the validator after its own
draw; a differing image prints the fields that differed. MEASURED, 153 of
153 bad: `lamps bake_eye` only. CODE: the bake's lamp levels (channel lamp
colours, steady gain, eye; `psx_mod_gpu_set_lamp_levels`, kind 9 in the
redraw stream) were sent only by the title's host walk (`lights_upload`)
and only when a bake had lamps. A loading-screen image is never walked, so
its stream had none, and each renderer instance drew it with the levels
its own previous image had left: the render thread with one gameplay
image's, the emulation thread (validator) with another's. The logo's lit
triangles take the bake path (`u_bk_on` and a surface), so the flickering
lamps and RELIT's gain reached it.

### Fix

- `render_replay.c` (framework): every replayed image starts from no host
  light state, `replay_light_state_reset`: no lights (ambient 1) and the
  bake's inactive lamp levels (no channel lamps, gain 1, eye at the origin);
  what the stream sets applies on top. Both the render thread's session and
  the emulation thread's draw (present_job, the validator) enter through
  `render_replay_feed`, which resets first. Walked images set their own, so
  gameplay is unchanged (it was exact before and after).
- `dynamic_lights.inc` (title): a walked image sends lamp levels also when
  the camera is not known yet or no bake has lamps (the inactive state), so
  every walked stream is complete on its own.
- `PSX_RT_VALIDATE=1` prints the state signature of a differing image; the
  render thread computes it only then.
- `runtime/tests/test_replay_light_state.py` (registered as
  `ttk_replay_light_state`) pins the reset and its place.
- The validator counts as skipped (not bad) a comparison its two draws
  cannot share: a savestate load in between (the render thread's epoch
  moved on: the image is discarded unshown and the VRAM replaced), or the
  shared stores at a new generation (lightmap or shadow mesh, a level
  change between the two draws). Any other difference stays bad and prints
  the state that differed.

### After

MEASURED, desktop, same settings, travels 1, 27, 0, 1 with the route on
each, four runs (exact / bad, all phases): 2,962 / 0, 2,603 / 0, 2,962 / 0
and 2,593 / 0. Runs two and three each printed one difference before any
travel, right after the session's savestate load (state "the same"): the
load case above, skipped from the fourth run on (0 differences in the
fourth run's log).

MEASURED, citadel (Vega 8, real TV, PRETTY at 60, RELIT, the governor's
fallback off with `DNTTK_REPLAY_FLOOR=0` so the redraws run throughout the
validator's extra draws): 1,277 / 1 over the same travels and routes. The
one differed in `bake_gen bake_on shadow_gen shadow_stamps shadow_edges
class_mesh`: drawn while the level change replaced the lightmap and shadow
mesh, now skipped as a store change.

So the redraws are exact against the single-thread path in gameplay and on
loading screens at RELIT with three workers. Visible effect: the loading
screen's spinning logo is no longer lit by whatever lamp levels a renderer
last had (its lit half could differ by up to 11 levels between frames
drawn by different threads).

## 3. D23AI - BALANCED on the field laptop's HD 5500

### Baseline: where BALANCED 60 falls short

MEASURED, field laptop, real 60 Hz screen, GRAPHICS BALANCED (2x, LIGHTING
ENHANCED, simple shadow edges, draw distance extended), FRAME RATE 60 (the
D23AG name; 120 before), third person, census route on all 21 levels: game
60 everywhere. New pictures a second, median / lowest tenth, and the
single redraw worker's CPU:

| level | new pictures | worker % | note |
|---|---|---|---|
| 0, 3, 7, 8, 12, 23, 24, 25, 28 | 59.8-60.0 / 56-59 | 33-78 | holds 60 |
| 1, 2, 10 | 59.0-59.9 / 38-54 | 50-83 | dips |
| 5, 11, 21, 26 | 56.7-59.5 / 26-48 | 69-87 | dips |
| 22 | 48.5 / 33.0 | 95.6 | worker saturated |
| 27 THE REAPER | 41.9 / 33.9 | 96.1 | worker saturated |
| 29 | 40.8 / 33.1 | 95.2 | worker saturated |
| 6-9 | (fallback) | 2 | 21-31 s of the original presentation |

The census counts presents as pictures while the original presentation
runs, so the fallback levels' 60 are not new pictures. The governor's
fallback holds for 2-8 minutes once taken, so which levels show it depends
on the level before (a second run moved it from 6-9 to 1 and 5).

### What each part of BALANCED costs

MEASURED, same machine and route, levels 6, 8, 1, 5, one change at a time
(package 14.8-15.4 W in every configuration: the 15 W limit):

| configuration | GPU % (clock-weighted) | worker % (6 / 8) | CPU MHz (6 / 8) | fallback seconds (6, 8, 1, 5) |
|---|---|---|---|---|
| BALANCED | 54 | 92 / 78 | 2947 / 2195 | 0, 0, 27, 29 |
| LIGHTING ORIGINAL | 37-41 | 80 / 54 | 2494 / 3084 | 0, 0, 0, 0 |
| draw distance ORIGINAL | 41 | 69 / 59 | 2524 / 2693 | 0, 0, 18, 34 |
| LIVE SHADOWS OFF | 49-53 | 88 / 63 | 2387 / 2670 | 0, 1, 30, 34 |
| RESOLUTION 1x | 22-25 | 62 / 56 | 3093 / 3093 | 0, 0, 0, 0 |

On this 15 W package the GPU's work takes power from the CPU: at 1x the
cores run at full turbo (3.1 GHz) and the same redraw worker needs a third
less CPU time. So BALANCED's GPU cost shows up as CPU time, and the
saturated levels are limited by the one redraw worker.

MEASURED, two redraw workers (`DNTTK_REPLAY_WORKERS=2`) on the saturated
levels 22, 27 and 29: the governor fell back within seconds on all three
(24-31 s of the original presentation): the second worker takes the
emulation thread's core. D23Y6's one worker stays.

MEASURED, `PSX_SHADOW_PROF=1` (scene GPU time per present interval): about
15.5 ms with and without lighting, but only for the presenting instance
and as elapsed GPU time including idle gaps; it cannot split a redraw into
its passes.

### Conclusions and proposals (PROPOSED)

- The goal (BALANCED at 60 new pictures a second on this laptop) needs
  the redraw worker's CPU per image down by about a third on THE REAPER and
  levels 22 and 29 (95% busy at 38-48 pictures). D23AH's LTO measured
  4-11% on the desktop (section 4).
- GPU per-pass timing needs instrumentation: GL_TIMESTAMP pairs around the
  world draw, lit draws, lightmap, shadow maps and the resolve, read back
  four images late, per renderer instance (as frame_perf's mirror pairs).
- Leanest look trade-offs for the user to choose (none applied): the
  extended draw distance and LIGHTING ENHANCED each cost about 15 GPU points
  and 10-25 worker points here; 1x halves the GPU.

## 4. D23AH - link-time and profile-guided optimisation

### Where the laptop's CPU goes

MEASURED, field laptop, BALANCED 60, THE REAPER, real screen: 150 stack
samples a thread with `sudo eu-stack` (perf is not installed there). The
emulation thread: 57% runtime helpers (top: `psx_write_word_raw` about
10%, device service, interrupt checks, dispatch), 11% generated game code,
15% unnamed library/driver frames, 11% waiting. The redraw worker: busy
throughout in the title's walk hooks, the runtime and generated code. Small
runtime functions called across files from the generated shards: what LTO
(cross-file inlining) and PGO (hot/cold layout, inlining decisions) act on.

### Builds

Separate build directories beside `build-local` (the player build is
untouched): `build-lto` (`-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON`, the
same cache settings as the `local-dev` preset, the fetched SDL3 and
libchdr sources reused) and `build-pgo` (LTO plus `-fprofile-generate
-fprofile-update=atomic`, a training run, then `-fprofile-use
-fprofile-partial-training` in the same directory so the counts match the
objects). MEASURED, desktop (GCC 13, 16 threads): LTO build 9 min 9 s,
largest process 1.8 GB; instrumented build 9 min 42 s, 3.6 GB; profile-use
build 7 min 19 s, 1.8 GB.

### Desktop A/B

MEASURED, desktop offscreen, BALANCED 60, third person, levels 27, 22, 29,
each build swapped into `build-local` in turn (the original restored and
compared byte for byte afterwards); emulation thread / redraw worker
CPU %, two runs each:

| build | 27 | 22 | 29 |
|---|---|---|---|
| normal | 44.8-47.9 / 64-66 | 48.6-51.3 / 55-66 | 44.4-46.3 / 63-68 |
| LTO | 41.9-43.1 / 58-63 | 45.4-48.3 / 55-66 | 42.2-42.8 / 63-64 |
| LTO + PGO (first training) | 36.1-36.2 / 60-62 | 38.5-41.5 / 60-64 | 34.8-35.1 / 63-65 |

New pictures held 57.7-59.7 in the lowest tenth in every build. LTO cuts
the emulation thread 5-11%; PGO about 20%. The first training left the
redraw worker untrained: the instrumented game ran at 15-27 game frames a
second, the governor kept the original presentation, and workers end with
`_exit` or SIGKILL, which skips writing their counts.

Second training (the framework's retiring redraw workers now write their
counts first: `worker_exit` calls `__gcov_dump`, a weak symbol present only
in an instrumented build, so every other build is unchanged), with the
governor's fallback off (`DNTTK_REPLAY_FLOOR=0`) so the redraws ran
throughout (workers 84-136% of a core while the instrumented game ran at
15-33 game frames a second). MEASURED, same A/B, two more runs each:

| build | 27 | 22 | 29 |
|---|---|---|---|
| normal | 46.8-47.2 / 63-67 | 48.5-52.0 / 56-62 | 44.6-45.3 / 67-68 |
| LTO | 43.2-43.5 / 63-65 | 44.7-46.0 / 56-58 | 40.9-41.7 / 65-66 |
| LTO + PGO (redraws trained) | 34.2-35.0 / 60-63 | 39.0-40.5 / 60-62 | 34.3 / 63 |

PGO takes about a quarter off the emulation thread; the redraw workers'
CPU per image barely moves with either. Their time is not in branchy code
the profile can lay out better (copy-on-write faults after each fork,
D23AF3; stream recording). Lowest tenth of new pictures 54.0-59.4 in every
build (the desktop holds 60 either way).

## 5. D23AJ - field laptop census at ORIGINAL 30 and 60

MEASURED, field laptop (2026-11-18 03:47-05:10 on its clock), real 60 Hz
screen, the current build (D23AG, D23AF6), census route, third person,
every level-select level; new pictures a second per second of the route:
the redraws' distinct images while they own the cadence, the game's own
new images (its logic ticks) in the original presentation or a fallback.
Median / lowest tenth per level; "fb" = seconds without redraws:

| level | BALANCED 60 | BALANCED ORIGINAL 30 | SIMPLE 60 | SIMPLE ORIGINAL 30 |
|---|---|---|---|---|
| 0 | 59.7 / 37.5 | 29.9 / 29.1 | 60.0 / 59.3 | 30.1 / 29.5 |
| 1 | 30.0 / 29.3 (35s fb) | 29.9 / 29.1 | 59.6 / 46.5 | 29.9 / 29.5 |
| 2 | 30.0 / 29.0 (34s fb) | 29.9 / 29.3 | 59.9 / 58.1 | 29.9 / 28.6 |
| 3 | 30.0 / 29.1 (36s fb) | 30.0 / 29.2 | 60.0 / 59.3 | 30.0 / 29.2 |
| 5 | 30.9 / 29.0 (16s fb) | 29.9 / 29.0 | 59.1 / 55.9 | 30.0 / 29.2 |
| 6 | 54.3 / 30.9 | 30.0 / 29.0 | 59.8 / 50.0 | 30.1 / 28.9 |
| 7 | 58.9 / 50.9 | 30.0 / 29.2 | 60.0 / 59.2 | 30.0 / 29.2 |
| 8 | 59.6 / 52.5 | 30.0 / 29.0 | 60.0 / 58.0 | 29.9 / 29.1 |
| 9 | 30.0 / 29.1 (32s fb) | 30.0 / 29.2 | 59.2 / 43.1 | 30.0 / 29.3 |
| 10 | 30.0 / 29.1 (36s fb) | 30.0 / 29.2 | 60.0 / 59.2 | 30.0 / 29.5 |
| 11 | 30.0 / 29.1 (36s fb) | 30.1 / 29.1 | 60.0 / 55.6 | 30.0 / 29.3 |
| 12 | 30.1 / 29.1 (24s fb) | 30.0 / 28.9 | 59.8 / 57.9 | 29.9 / 28.0 |
| 21 | 56.8 / 32.5 | 30.0 / 29.1 | 59.9 / 58.5 | 30.0 / 29.2 |
| 22 | 39.5 / 30.0 | 28.7 / 19.2 | 59.4 / 48.2 | 29.4 / 22.6 |
| 23 | 60.0 / 54.4 | 30.0 / 27.4 | 60.0 / 59.4 | 30.0 / 29.3 |
| 24 | 59.7 / 38.6 | 30.1 / 29.3 | 60.0 / 59.3 | 30.0 / 29.4 |
| 25 | 60.0 / 59.5 | 30.0 / 29.3 | 60.0 / 59.2 | 30.0 / 29.5 |
| 26 | 59.7 / 45.0 | 29.9 / 23.8 | 60.0 / 59.6 | 30.0 / 26.7 |
| 27 | 39.5 / 31.1 | 30.0 / 29.4 | 59.4 / 46.4 | 29.7 / 27.6 |
| 28 | 59.9 / 58.1 | 30.0 / 29.1 | 60.0 / 57.5 | 30.0 / 29.2 |
| 29 | 37.0 / 30.5 | 30.0 / 29.1 | 57.0 / 49.8 | 29.4 / 21.3 |

All seconds (about 745 a configuration, game 60 median and 59.0-59.2
lowest tenth in all four):

| configuration | new pictures median / lowest tenth | seconds without redraws |
|---|---|---|
| BALANCED 60 | 47.5 / 29.5 | 249 of 744 |
| BALANCED ORIGINAL 30 | 30.0 / 29.1 | (all, by design) |
| SIMPLE 60 | 59.9 / 55.1 | 0 of 745 |
| SIMPLE ORIGINAL 30 | 30.0 / 29.1 | (all, by design) |

Reading: ORIGINAL 30 shows about 30 new pictures everywhere (22 and 29
dip to 21-23 in the lowest tenth: busy scenes), as its name says. SIMPLE 60
meets the goal rate on this laptop. BALANCED 60 does not: one governor
fallback (2-8 minutes of the original presentation, so it covered
levels 1-3, 5 and 9-12 here) and the single redraw worker saturated on
THE REAPER, 22 and 29 (37-40 new pictures). The gap to the goal, level by
level: 22, 27, 29 (worker), 6, 21, 24, 26 (lowest tenth 31-45), and the
fallback that follows them.

First person (MEASURED afterwards, same machine and build, 05:11-05:44 on
its clock), FRAME RATE 60:

| level | BALANCED 60 | SIMPLE 60 |
|---|---|---|
| 0 | 60.0 / 59.7 | 60.0 / 58.1 |
| 1 | 50.8 / 36.4 | 59.5 / 41.9 |
| 2 | 59.9 / 47.4 | 60.0 / 59.4 |
| 3 | 59.9 / 59.3 | 60.0 / 59.7 |
| 5 | 59.8 / 39.9 | 60.0 / 59.6 |
| 6 | 48.3 / 24.9 | 59.8 / 46.8 |
| 7 | 59.9 / 57.3 | 59.9 / 59.4 |
| 8 | 59.7 / 51.9 | 59.9 / 59.4 |
| 9 | 53.9 / 34.5 | 57.2 / 37.7 |
| 10 | 59.5 / 50.8 | 60.0 / 59.3 |
| 11 | 56.0 / 28.1 | 60.0 / 58.9 |
| 12 | 59.9 / 48.0 | 60.0 / 59.7 |
| 21 | 49.6 / 27.0 | 60.0 / 56.4 |
| 22 | 37.0 / 30.3 | 58.5 / 41.4 |
| 23 | 60.0 / 59.0 | 60.0 / 59.0 |
| 24 | 59.2 / 50.3 | 60.0 / 59.2 |
| 25 | 60.0 / 59.7 | 60.0 / 59.6 |
| 26 | 60.0 / 51.5 | 60.0 / 59.1 |
| 27 | 35.1 / 29.5 | 54.6 / 37.2 |
| 28 | 58.7 / 28.5 | 60.0 / 32.3 |
| 29 | 32.4 / 28.3 | 41.8 / 35.1 |

All seconds: BALANCED 59.6 / 32.1 and SIMPLE 59.9 / 45.4 new pictures, the
redraws on in every second of both (no fallback this time), game 60. The
same worker-bound levels (22, 27, 29: 32-37 at BALANCED) are short; level
29 is short even at SIMPLE (41.8).

## 6. D23AH on the field laptop, and what limits BALANCED there

The laptop builds the normal executable itself; the LTO executable came
from citadel (the same source, GCC 16.2.1 on both, the same paths; LTO
there: 34 min, 5.7 GB peak of 5.8 GB plus swap, too tight to make it the
default build on that machine). MEASURED, real 60 Hz screen, BALANCED 60,
levels 27, 22, 29, 6, alternating, two runs each (emulation thread % /
worker % / new pictures median):

| level | normal | LTO |
|---|---|---|
| 27 | 69.3-69.8 / 95 / 39.9-45.0 | 60.0-63.6 / 94-96 / 38.8-41.7 |
| 22 | 76.8-80.9 / 89-94 / 41.2-54.0 | 64.9-69.2 / 92-94 / 42.6-46.4 |
| 29 | 75.6 / 96 / 37.3 (and a fallback run) | 63.5-64.3 / 95 / 37.9-43.1 |
| 6 | 77.3 / 80 / 55.7 (and a fallback run) | 69.1-72.6 / 75-84 / 58.1-58.6 |

LTO takes 8-16% off the emulation thread and neither LTO run fell back
(one normal run did, from level 29 on), but the redraw worker stays
saturated and the saturated levels gain nothing. MEASURED, 149 stack
samples of the worker on THE REAPER: about 20% inclusive in the title's
host world (`host_vis_walk`, `draw_distance_apply`,
`near_clip_host_world_piece`: the same emission D23Y5 profiled on the
emulation thread, redone for every redraw), 17% self in the title's other
helpers, 4-8% recording host vertices, 13% asleep; the rest generated
code and memory helpers.

PGO on the laptop: citadel trained and built it (no LTO, which does not
fit citadel's memory with instrumentation; instrumented build 77 min,
training on its real TV at BALANCED 60 with the fallback off, 7 levels,
232 count files; profile-use build 46 min); the executable copied to the
laptop. MEASURED, same A/B (emulation thread % / worker % / new pictures
median, two runs each):

| level | normal | PGO |
|---|---|---|
| 27 | 62.2-66.6 / 85-95 / 38.7-50.9 | 52.1-55.5 / 90-95 / 47.2-59.1 |
| 22 | 75.5-76.0 / 92-94 / 42.2-51.7 | 60.4-64.8 / 82-88 / 54.3-55.8 |
| 29 | 71.3-74.5 / 95-96 / 40.5-40.6 | 56.3-62.5 / 95 / 41.2-42.6 |
| 6 | 65.3-75.2 / 80 / 53.8-59.9 (one run with 23 s of fallback) | 63.3-64.1 / 66-71 / 59.3-59.5 |

PGO takes 12-21% off the laptop's emulation thread, neither PGO run fell
back, and 22 and THE REAPER come nearer 60 (one run 59.1); level 29 stays
worker-bound. The nearest of the night to the goal, not the goal.

PROPOSED (D23AK): a lighter redraw worker. The worker re-runs the title's
host world walk for each redraw image; most of it depends only on the game
frame, not the in-between camera, so it could be computed once per game
frame and reused by that frame's redraws (exact, behind a developer switch,
validated with `PSX_RT_VALIDATE`). On the laptop each new picture costs
the one worker about 24 ms on THE REAPER at BALANCED (95% of a core at
about 40 a second); 60 needs at most 16.7 ms, about 30% less.

PROPOSED, D23AH: LTO (and PGO) as an opt-in build (`build.py --lto`,
`--pgo`), not the default: the emulation-thread gain is real (5-16% LTO,
about 25% PGO on the desktop) and keeps the governor from falling back,
but the build needs 1.8-5.7 GB per link and 10-35 minutes, PGO needs a
training run per compiler version, and neither moves the redraw worker.
Exactness of the LTO/PGO executables was not checked image by image
tonight (no fast-math, no code changes; the validator only compares within
one executable).

## 7. Closeout

User playtest on the field laptop, 2026-11-18 (BALANCED, first person):
FRAME RATE 60 first looked like 30 and 120 like true 60. The session log
showed 60 display-timed at 60 presents for about 30 s, then the governor's
fallback (emulation 46.6 frames a second) holding the original
presentation 120 s; choosing 120 had cleared the fallback. Compared at
SIMPLE: "ok yes confirmed working. lock it in and close it out". D23AG,
D23AF6 and D23AJ Done; D23AH (opt-in builds) and D23AI (on D23AK) Blocked;
D23AG1 and D23AK Todo. Suggested from the playtest, not built: an on-screen
note when the governor falls back, and a shorter first wait after a brief
slowdown.
