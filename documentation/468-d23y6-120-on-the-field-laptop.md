# 468 - D23Y6: FRAME RATE 120 on the field laptop, SIMPLE at 2x

User, 2026-11-13: "our goal needs to be to make 120fps playable on this
laptop. even at simple settings. the simple profile needs to ideally be 2x,
1x is really bad btw ... 120fps does sort of work, but i feel its jerkier now
than when i playtested it on the gold and guns level a few hours ago."

Machine: the field laptop of note 461 (i7-5600U, 2 cores and 4 threads, HD
Graphics 5500, one 15 W package, a 1920x1080 panel at 60.01 Hz under the XFCE
compositor). Evidence labels: MEASURED (this session), CODE, INFERRED.

## 1. Method (MEASURED)

Lab `recomp/analysis/d23y6/` (local, a copy of the D23Z lab): private settings
built from the player's profile (read only, never written) and a private copy
of the cards; the debug port; the real display (the cadence depends on the
compositor, and offscreen ran far lighter: 120 presents with 132% of a core at
3.1 GHz where the real window could not hold 60); runs waited while the
user's keyboard or mouse had been used since the previous run. Case `cad`:
the D23W walk/turn/fire pattern for 30 s after loading the savestate; each
second game frames, presents, the CPU clock, all threads and the emulation
thread; every swap's time (`gl_present_ring`) over the walk. Scenes: Gold and
Guns (level 3, savestate slot 0, the 13:16 playtest's level) and UI slots 2,
8 and 9 (savestate slots 1, 7 and 8). Configurations interleaved and rotated
per repetition (`abc.sh`), summarised by `comb2.py`: medians and the lowest
tenth over the seconds of all runs (the first two seconds of each walk
skipped), the CPU clock per run.

"Screen fresh" models what the 60.01 Hz panel shows: for 33 phases of its
refresh against the swap times, the share of refreshes that show a new
picture, and "held 2+" the share of pictures held two refreshes or more (a
visible stutter in a 60 a second stream). "25 ms gaps" is the share of present
intervals of 21-29 ms: every 3rd refresh of the 120 timeline, 40 a second,
which a 60 Hz panel shows as an uneven 17/33 ms.

## 2. The regression (MEASURED)

The session logs (`recomp/build-local/logs`): 13:16 played Gold and Guns
(level 3) at 120 for about 25 s (game frames 13-58 a second, presents settling
at every 3rd refresh, 40, after 2 rate changes; the D23Y1 fallback was not yet
built). 20:10 and 20:44 played UI slot 2 (level 6) at 120: game 51-60 in most
seconds, but the governor swung between every 2nd and 3rd refresh (60 and 40
a second) 4-6 times a minute ("trying faster" then "faster rate failed").
So the two sessions were different levels, and tonight's had more cadence
changes, not slower game frames.

A/B at 120, the user's settings (2x, LIGHTING on, SIMPLE shadow edges,
extended draw distance), 3 interleaved runs per configuration and scene:

| 120, user's settings | Gold and Guns | UI slot 2 | UI slot 8 | UI slot 9 |
| --- | --- | --- | --- | --- |
| default (D23Z sleep, async snapshot) | game p10 54.3, fresh 95.6%, 4.3 changes | 55.3, 81.4%, 5.0 | 54.9, 73.1%, 4.7 | 54.7, 83.8%, 5.3 |
| `PSX_PACER_SPIN=1` | 56.1, 96.3%, 3.7 | 55.0, 89.1%, 4.0 | 56.9, 73.5%, 4.3 | 54.1, 81.8%, 4.0 |
| `PSX_GL_ASYNC_SNAPSHOT=0` | 55.8, 95.9%, 4.3 | 55.2, 78.1%, 4.7 | 54.0, 73.3%, 5.0 | 53.6, 82.0%, 5.3 |

Neither D23Z change is the cause: the differences are within the run-to-run
spread (CPU clock 2.3-3.0 GHz between runs) and point both ways. What the
user saw is the scene: Gold and Guns holds every 2nd refresh (60) about 95% of
the time, UI slot 2's level 6 spends 40-50% of a walk at every 3rd (25 ms
gaps 39%) and swings between them 4-5 times in 30 s. The D23Z changes stay.

## 3. Why 120 jerks on this laptop (CODE, MEASURED)

FRAME RATE 120 runs a 120 Hz present timeline without the driver's vsync
(`present_replay_vsync`); on a 60 Hz panel the compositor shows the newest
picture at each refresh, so at most 60 of the 120 reach the screen. Each
present is a redraw: worker CPU, the render thread and GPU, and the emulation
thread's share of the presents. On the laptop that was the cost that made the
game fall behind; the governor (D23H) then stepped to every 2nd refresh (60,
even), probed back to 120 after 5-80 s, failed (a slow second each time) and
stepped to every 3rd (40, uneven on 60 Hz).

## 4. Built (CODE)

1. **Presents at the display's rate** (`frame_replay.cpp` `display_cap_check`,
   framework `psx_mod_display_refresh_hz`): a fixed FRAME RATE above the
   display's refresh (more than 5% above) is timed on the display (target 0,
   the panel's own period) while the setting stays 120; a faster display gets
   the chosen rate back. Log: `[TTK replay] frame rate 120 on a 60.01 Hz
   display: presents at the display's rate`. Developer `DNTTK_DISPLAY_CAP=0`
   presents at the chosen rate as before.
2. **The governor's last step keeps at least 30 even presents**
   (`last_div`): the largest divisor that keeps 30 a second, at most 3 (120 or
   144 Hz timelines: every 3rd as before; a 60 Hz display: every 2nd, 30,
   never 20 or an uneven 40). Behind there, the D23Y1 fallback to the
   original presentation follows; its retry state no longer loses the
   display rate (`floor_active`; a fallback from DISPLAY did not retry).
3. **Slow seconds at the last step accumulate** (`floor_good`, `floor_clear`
   10): they clear after 10 clean seconds in a row instead of one, so slow
   seconds that come one or two at a time reach the fallback.
4. **Placement and one worker by default** (framework `render_thread.c`,
   `placement_default`): D23Y4's placement (the emulation thread keeps one
   physical core, redraw workers and render threads the other) on 2-core SMT
   CPUs once redraw workers start, and then one redraw worker instead of
   three. `PSX_PLACEMENT=0` turns placement off (and three workers return).
5. **SIMPLE renders at 2x** (`player_profiles.GRAPHICS_PRESETS`, the
   OPTIONS table `grade_rows` in `game_menu.cpp`,
   `test_menu_table_matches_the_presets`, the manual); its other rows stay
   the lightest (ORIGINAL lighting, SIMPLE edges, original draw distance,
   FRAME RATE 60). GRAPHICS AUTO here: the cached probe is 6.3 ms for the 4x
   frame, so AUTO picks BALANCED (unchanged; SIMPLE is only below 14 ms).
6. **The late camera's latency query sorts once per finished redraw**
   (`psx_rt_latency_ms`; it sorted 64 values at every present, 0.5% of the
   emulation thread). Same value.

## 5. Results at FRAME RATE 120 (MEASURED)

SIMPLE (2x, lighting ORIGINAL, SIMPLE edges, original distance), 3
interleaved runs per scene; "before" is `DNTTK_DISPLAY_CAP=0 PSX_PLACEMENT=0`
on the same build:

| SIMPLE at 120 | game median / lowest tenth | presents median / lowest tenth | screen fresh | 25 ms gaps | rate changes a walk | all threads | emulation thread | CPU clock |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Gold and Guns, before | 59.8 / 55.9 | 119 / 60 | 99.1% | 0% | 2.3 | 154% | 65% | 2.5 GHz |
| Gold and Guns, after | 60.0 / 59.2 | 60 / 59 | 98.1% | 0% | 0 | 85% | 44% | 3.1 GHz |
| UI slot 2, before | 59.9 / 54.0 | 60 / 40 | 88.2% | 21% | 5.0 | 135% | 64% | 2.8 GHz |
| UI slot 2, after | 59.9 / 59.0 | 60 / 59 | 97.9% | 1% | 0 | 134% | 57% | 2.5 GHz |
| UI slot 8, before | 59.4 / 51.8 | 60 / 40 | 88.0% | 20% | 4.3 | 205% | 90% | 2.5 GHz |
| UI slot 8, after | 59.8 / 57.9 | 60 / 59 | 97.3% | 1% | 0 | 234% | 84% | 2.4 GHz |
| UI slot 9, before | 59.8 / 58.3 | 60 / 59 | 97.8% | 0% | 4.0 | 136% | 60% | 2.8 GHz |
| UI slot 9, after | 60.0 / 59.2 | 60 / 59 | 98.1% | 0% | 0 | 119% | 49% | 3.1 GHz |

The about 2% of refreshes without a new picture after the change is the
model's: the ms swap times against a fixed refresh phase (a steady 16.7 ms
stream drifting against the 60.01 Hz panel), not measured stutter.

Steps in between (MEASURED, SIMPLE): the cap alone held UI slots 2 and 9 and
Gold and Guns (lowest tenth 58.6-59.2, 0-0.3 changes) but not UI slot 8
(emulation thread 88%, lowest tenth 55.8, spells at 30). A profile of UI
slot 8's emulation thread (`PSX_PROF`, 3.1 GHz both) took 22,397 samples at
the original 60 and 31,999 at capped 120: +43%, spread over the whole
profile (dispatch lookups, memory, GTE, SPU alike), with the redraw path's
own work about 8 points (the per-frame RAM image for the workers 2.1%, the
render thread's VRAM capture 1.8%, swaps 1.7%, the latency sort 0.5%): the
rest is the emulation thread sharing its core with redraw workers.
Placement there: 81% instead of 87%, presents lowest tenth 56 instead of 30,
0.7 changes instead of 3 (UI slot 2: 56% instead of 66%). One redraw worker
instead of three (`DNTTK_REPLAY_WORKERS=1`) was as good or slightly better
at SIMPLE (UI slot 8: lowest tenth 59.0, all threads 207%) and clearly
better at BALANCED in UI slot 8 (game 59.2 and presents 59 against 56.7-56.9
and 30-52; D23Y4 at 3x: 56.2 against 55.3), so with placement the default is
one worker (`start_workers_once`; `DNTTK_REPLAY_WORKERS` overrides). The
section 5 table was measured before that (three workers); section 6's final
batch has one.

## 6. BALANCED and PRETTY (MEASURED)

Final build against the old behaviour (`DNTTK_DISPLAY_CAP=0
PSX_PLACEMENT=0 DNTTK_REPLAY_WORKERS=3`, the same build), FRAME RATE 120, 2
interleaved runs per scene and grade (SIMPLE repeated in the same batch):

| 120 | Gold and Guns | UI slot 2 | UI slot 8 | UI slot 9 |
| --- | --- | --- | --- | --- |
| SIMPLE, before | game p10 53.9, presents p10 60, 2.5 changes | 55.8, 40, 4.5 | 54.5, 40, 3.5 | 56.8, 59, 4.0 |
| SIMPLE, after | 59.2, 59, 0 | 59.1, 59, 0 | 59.0, 59, 0 | 59.0, 59, 0 |
| BALANCED, before | 53.9, 59, 4.0 | 52.9, 39 (25 ms gaps 40%), 4.0 | 56.0, 39 (69%), 4.0 | 57.3, 39 (50%), 3.5 |
| BALANCED, after | 59.0, 59, 0 | 58.9, 60, 0 | 59.2, 59, 0 | 59.3, 30, 1.0 |
| PRETTY, before | 52.9, 38 (40%), 2.5 | 32.4, fallback, 2.0 | 48.1, fallback, 2.0 | 31.0, fallback, 2.0 |
| PRETTY, after | 57.0, 30, 2.0 | 54.4, 29, 4.5 | 58.1, fallback, 3.0 | 59.0, 30, 0 |

(game p10: the lowest tenth of game frames a second; presents p10 the lowest
tenth of presents a second; changes: present-rate changes in a 30 s walk.)
Screen fresh after: 97.1-98.2% at SIMPLE and BALANCED except UI slot 9 at
BALANCED (86.6%: one spell at an even 30 a walk); before 73-99%. All threads
after 111-221%, CPU clock 2.2-3.1 GHz.

BALANCED: 60 even presents in three scenes of four; UI slot 9 still steps to
an even 30 for a spell in each walk (game frames unaffected, 59.3).

PRETTY at 3x Relit: the HD 5500 is the wall (CPU clock 1.9-2.9 GHz with
the GPU busy; note 464: the original 60 alone uses 64-69% of it), so 60
redraws a second do not fit. After: an even 30 presents a second (the panel
shows each twice) or the original presentation, game frames lowest tenth
54.4-59.0 (before 31.0-52.9 with uneven 40s and late fallbacks). A grade
whose redraws need more of the GPU than it has cannot be made smooth at 120
here without drawing less per picture (D23N2B-N4 cut CPU, not GPU).

## 7. Pictures and Vanilla

Vanilla: paused shots (the in-game pause menu freezes the game) at UI slots
2 and 8, the new build's default against `PSX_PLACEMENT=0`: 0 differing
pixels in all six (Vanilla never starts redraws; none of the changes runs
there).

Modernized at 120: the changes choose which present deadlines are used, the
CPUs threads run on and the number of worker processes, never what a redraw
draws. Exact frame checks were not possible here: the scenes animate (water,
the scrolling sky, flames, the LEVEL STATS clock) and two runs pause at
different moments. Paused shots, new against old: 61,103-66,775 differing
pixels in UI slot 8 at SIMPLE, all in the sky, the menu title's glow and a
flame; two runs of the same new build differ by 48,751 there and two of the
old by 68,003, in the same places. World geometry, HUD and menu text match.

Behaviour: game speed is unchanged by construction (presentation only);
in every measured walk the game ran at a median of 59.6-60.1 frames a second.

## 8. Remaining

- The user's judgement that 120 feels smooth (Needs playtest), at SIMPLE
  and at their own BALANCED-like settings.
- On a 60 Hz panel "120" now presents 60: half the pictures were never shown,
  but a present that lands just after the compositor's refresh waits up to
  one more refresh (INFERRED, about 4 ms on average against 120 presents).
  The presents are not locked to the panel's refresh (no vsync above 60):
  the model's 2% of refreshes without a new picture is the phase drifting.
- BALANCED UI slot 9: one spell at 30 a walk; PRETTY: GPU-bound (section 6).
- The emulation thread at 82-87% in UI slot 8 with redraws (D23Y5's
  hotspots: the per-frame 2 MB RAM image for the workers, the render
  thread's VRAM capture, dispatch lookups slowed by the shared cache).
- Offscreen measurements do not represent the real window here (section 1).

## 9. Reproduce

`recomp/analysis/d23y6/` (local): `abc.sh REPS "slots" tag "env" ...`
(case `cad`, the real display, waits while the user is active; `CTRL`,
`PRES` and `RATE` select the settings), `comb2.py tags [slots]`,
`final.sh` (section 6), `still.sh` (the frame checks), `symdiff.py`,
`leaf.py` and `callers.py` for `PSX_PROF` profiles.
