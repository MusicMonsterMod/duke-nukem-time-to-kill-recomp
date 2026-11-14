# 469 - D23Y7, D23Y5, D23N2B: DUKE HILL at 120, the emulation thread, joint matrices

User, 2026-11-14: "work on D23Y7, D23Y5 and D23N2B autonomously and completely
on this field laptop, in that order ... Goal: squeeze more out of FRAME RATE
120 here, without changing pictures or behaviour; Vanilla unchanged. The late
warp stays cancelled ... Keep D23Y6's display cap, placement and one worker
unless a measured A/B beats them."

Machine: the field laptop of notes 461 and 468 (i7-5600U, 2 cores and 4
threads, HD Graphics 5500, one 15 W package, 1920x1080 at 60.01 Hz). Host
checked first: load 0.7, no backup, update or other heavy process, on AC
power, no thermal throttling (package 46-50 C, throttle counts 0); the RAPL
limits are 15 W sustained and 25 W short. Evidence labels: MEASURED (this
session), CODE, INFERRED.

Result in one line: no player-facing change. DUKE HILL's heavy view is
bounded by the 15 W package at the user's settings (measured below); every
exact cut tried was within the noise and was taken out again; D23Y6's cap,
placement and one worker stay (A/B). D23N2B gained a fourth check (joint
matrices) that locates the next piece.

## 1. Method (MEASURED)

The D23Y6 lab (`recomp/analysis/d23y6/`, local): case `cad` (30 s walk, turn
and fire after a savestate load, every second's game frames, presents, CPU
clock, all threads and the emulation thread, every swap's time), `abc.sh`
(configurations interleaved and rotated, the real display, waiting while the
user is active) and `comb2.py` (medians and the lowest tenth over all runs'
seconds, the first two seconds skipped). Added this session:

- **DUKE HILL savestates** in the lab's private card copy only (never the
  player's cards): private slot 10, a new `level 1` start made by case
  `mkhill` (console `level 1` from the TIME TO KILL state, then a savestate;
  KILLS 0/50, enemies alive); private slot 4 is the copy of the player's UI
  slot 5 (DUKE HILL, cleared). `comb2.py` names them DH and DH5.
- **Per-thread CPU over the walk** in `cad`, worker processes included (a
  worker forked during the walk counts from its start), and the worker
  statistics (spawns, jobs, mean job time).
- `incl.py`: inclusive shares from `PSX_PROF_STACKS` call stacks.

Settings: the player's profile copied read only (2x, LIGHTING on, SIMPLE
shadow edges, extended draw distance, FRAME RATE 120). For costs at a steady
60 presents the governor was held off (`DNTTK_REPLAY_SHED=0
DNTTK_REPLAY_FLOOR=0`); otherwise the player's governor ran.

## 2. D23Y7: where DUKE HILL drops (MEASURED)

The player's sessions (00:30-00:56): DUKE HILL from its normal start (after
TIME TO KILL) and from UI slot 5 stepped to 30 and twice to the original
presentation; every step followed seconds of 45-57 game frames at 60 presents.

| 120, the player's settings | game median / lowest tenth | presents median / lowest tenth | screen fresh | rate changes a walk | all threads | emulation | CPU clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| DUKE HILL start (5 runs) | 60.0 / 58.8 | 60 / 59 | 98.0-98.4% | 0 | 124-133% | 67-72% | 2.5-3.1 GHz |
| UI slot 5 copy (7 runs) | 59.9-60.1 / 56.3-58.4 | 31-39 / 30 | 68-73% | 4.3-5.0 | 124-136% | 67-72% | 2.6-2.8 GHz |

The start holds 60 even pictures; the walk from UI slot 5 (the open town,
about 18 dynamic lights in view) does not. There, per second: at 60 presents
the CPU clock falls to 1.5-2.2 GHz within a few seconds and the game to 52-57
frames, the governor steps to 30, the clock returns to 3.0 GHz.

Costs in that view (steady 60 presents, MEASURED):

| UI slot 5 copy | game median / lowest tenth | CPU clock | all threads | emulation | redraw worker | render thread | GPU busy x clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FRAME RATE 60 (original presentation) | 60.0 / 59.7 | 3.1 GHz | 67% | 64% | - | - | 195-230 |
| 120 capped at 60, player's settings (13 runs) | 57.6-59.5 / 50.9-53.5 | 2.0 GHz | 223-226% | 91-93% | about 100% | 30% | 524-549 |
| the same, dynamic lights off (diagnostic) | 60.0 / 57.1 | 2.3 GHz | 217% | 88% | | 26% | 419-498 |
| the same, 1x (diagnostic) | 60.1 / 59.1 | 3.1 GHz | 144% | 62% | | 24% | 281-303 |

The CPU work is about the same at 2x and 1x (clock times threads); at 2x the
GPU's share of the 15 W package takes the CPU from 3.1 to 2.0 GHz and the
emulation thread then needs 91-93% of a core and falls behind. GPU per
redraw (`PSX_GPU_PHASES`, offscreen): 7.6-10 ms in this view (batch draws
6-9 ms, the copies at the end 0.6 ms), 3.0 samples a pixel (3.5 fragment
shader invocations); UI slot 2 5.9 ms, 2.6 a pixel. The worker (`PSX_PROF_WORKERS`):
host world and model emission about 20%, the game's code 16%, memory
access 12%, the image load about 7% and the code-guard resync after it about
4%. Workers re-fork when the title's mod memory grows (lazy first-use
allocations: aim vectors, matrices): 1-2 re-forks in a walk's first seconds,
not later (CODE: `render_worker.c` `maintain`).

## 3. D23Y7: what was tried (MEASURED)

All A/Bs interleaved on the real display, the UI slot 5 copy:

| Change | Result | Kept |
| --- | --- | --- |
| Depth prepass (D23U, `PSX_GL_ZPREPASS=1`), 3 runs each | game lowest tenth 55.7 against 57.7, GPU 71-76% against 72-74% | no |
| Snapshot capture without its hi-res and wide copies (developer bound, wrong pictures), steady 60 | lowest tenth +2 to +4.5 points (5 pairs: 51.8 -> 55.9; a second batch 53.5 -> 56.2 hi-res only, 52.9 wide only, 55.0 both) | bound only |
| Incremental capture: only the 32x32 tiles written since that snapshot slot was last filled | exact (developer readback check: 0 differing tiles in about 1,080 compared captures, 2 runs); copied 245 of 512 hi-res tiles and 15 of 16 wide tile rows a capture (the game's two 512x240 frame bands are rewritten every frame); lowest tenth 53.5 against 53.5 | no (no measured gain) |
| Redraw fence before the swap (one batch submit less a present) and an overlay image cache (no re-upload of an unchanged crosshair or LEVEL STATS) | emulation thread 62.3% against 62.3% (UI slot 5 copy), 49.4% against 49.9% (UI slot 2), clock-normalised, offscreen | no |
| D23Y6 defaults against `PSX_PLACEMENT=0` and against two workers, the player's governor, 2 runs each | defaults: screen fresh 72.5%, presents median 39; without placement 56.0%, 30; two workers 63.4%, 30 (their higher game lowest tenth 59 comes from the time at 30) | defaults kept |

The incremental capture and the fence/overlay code are kept as local lab
patches (`recomp/analysis/d23y6/d23y7-*`), not in the tree. The framework is
unchanged (the exported patch is identical).

Status: Blocked (measured). At the player's settings the heavy DUKE HILL
view needs more than the 15 W package can give CPU and GPU together; the
exact cuts available are each below what this machine's run-to-run spread
can show (about 2 points of the lowest tenth). What would move it: less
worker CPU per redraw (D23N2B-N4), a lighter emulation thread (D23Y5's
structural step, below), or a lighter picture chosen by the player (dynamic
lights off reached a lowest tenth of 57.1 at a steady 60; 1x 59.1).

## 4. D23Y5: the emulation thread (MEASURED)

The original 60 presentation, UI slot 2, offscreen at a steady 3.1 GHz: the
emulation thread 57.4-58.6% (median; means 59.5-60.2, 4 runs). The target is
under 50%. Inclusive shares (call stacks, 21,495 samples):

| Where | Inclusive |
| --- | --- |
| emulated GPU command processing (`dma_gpu_ll_advance` -> `gpu_write_gp0` -> GL batches) | 13.7% |
| host world and model emission (`ttk::draw` 10.9%, host room walk 7.9%) | about 11% |
| presentation (`sdl_vblank_present`) | 6.4% |
| SPU (`spu_render`, reverb) | 3.3% |
| dispatch lookups (`psx_game_find_entry`) | 2.1% |
| guest memory access (exclusive: word writes 7.8%, word reads 2.3%) | about 11% |

In redraw mode (UI slot 5 copy, 120 offscreen) the thread also spends 10.6%
presenting redraws, 4.7% publishing the worker image and 2.0% capturing the
render threads' snapshot, and 8.5% in the title's model pass (whose
emission redraw-only frames do not show; CODE: unlike the world pass it is
not skipped, and skipping it changes the ordering-table links the DMA walker
reads, so it needs its own exactness proof).

Tried (exact): the per-store debug-tool write hooks bypassed (developer
experiment): 57.4% against 56.4% (3 runs each, no gain); the fence and
overlay changes above. The shares are spread: no single exact change is
large enough to measure here, and 8 points cannot come from them.

Status: Blocked (measured). The target needs a structural change: the
emulated GPU's command processing and presentation on their own thread (13.7%
plus 6.4% of the thread at the original 60, INFERRED to bring UI slot 2 to
about 46-48%), proposed as D23Y8.

## 5. D23N2B: joint matrices built in a later job (MEASURED)

Note 456 left "matrices built during the job" as the largest unpredicted
input of a later job. By caller, offscreen at 180 Hz (`DNTTK_N2_CHECK=2`),
the actor joint loop (`0x800348d8`, the transform call returning to
`0x80034c6c`) is most of them: UI slot 2 20,247 of 25,080; UI slot 8 12,636
of 18,829 (then `0x80033584` 2,899); UI slot 9 26,238 of 28,063.

New check, piece 4a (`DNTTK_N2_CHECK` bit 8, `redraw_n2.inc`): at that
transform entry, before and after our own callbacks.

| View | Joint calls | Entry passed directly | Entry as at the job start | Replaced by our callbacks | in mod memory | the camera block `0x800d6eb0` | replacement keeps the entry's rotation |
| --- | --- | --- | --- | --- | --- | --- | --- |
| UI slot 2 | 36,992 | 36,992 | 36,992 | 36,992 | 32,640 | 4,352 | 19,584 |
| UI slot 8 | 29,952 | 29,952 | 29,952 | 29,952 | 24,960 | 4,992 | 15,048 |
| UI slot 9 | 48,384 | 48,384 | 48,384 | 48,384 | 37,383 | 11,001 | 22,743 |

So the game contributes nothing here that a later job must recompute: in
first person every joint table entry is as it was at the job's start and
the loop passes it directly (the game's scaled-copy path, `ScaleMatrix`
`0x800b4b5c`, is not taken; a host port of it is in the check, CONFIRMED from
its disassembly, unexercised so far). Every matrix the transform then reads
is ours: the D36 first-person presentation's copies in mod memory (the hand,
the weapon, the limbs), or the camera block for a joint kept out of the view
(its translation is the eye, depth 0, so the loop skips it). A rebuilt later
job therefore gets these matrices by running our own D36 joint code with the
job-start inputs, not by recomputing game code.

Next (resumable): (1) list the inputs our joint callbacks read
(`first_person.inc` around the `0x80034c6c` hooks: pose, camera, fp_blend,
orbit, policy) and check each is as at the job start or predicted (the
camera is, note 456); (2) then call them from the later job's start and
compare every matrix word for word (bit 8 extended); (3) the other callers
(`0x80033584`, `0x8003133c`, `0x80038c68`/`0x80038d7c`/`0x80038e18`: about
700-2,900 a run each); (4) the rest of note 456 section 6 (object culling, the
camera-dependent hooks, sprites and particles, a whole later job word for
word).

## 6. Pictures, behaviour, Vanilla

No player-facing code changed: the only tracked code change is the
developer check (bit 8 of `DNTTK_N2_CHECK`, nothing runs without it). The
framework matches the accepted patch. Vanilla is therefore unchanged.

## 7. Reproduce

`recomp/analysis/d23y6/` (local): `SLOT=3 OFFSCREEN=1 CASES=mkhill python3
lab.py` (a DUKE HILL start into private slot 10); `abc.sh 3 "10 4" tag ""`
then `comb2.py tag "10 4"`; steady 60: `"LAB_DNTTK_REPLAY_SHED=0
LAB_DNTTK_REPLAY_FLOOR=0"`; GPU per redraw `LAB_PSX_GPU_PHASES=2`; profiles
`LAB_PSX_PROF`, `LAB_PSX_PROF_STACKS`, `LAB_PSX_PROF_WORKERS` with
`sym.py` and `incl.py`; joints `RATE=180 LAB_DNTTK_N2_CHECK=10`.
