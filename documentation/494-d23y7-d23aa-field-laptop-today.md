# 494 - D23Y7 and D23AA re-measured on the field laptop with today's build

Autonomous three-machine session of 2026-11-16 (laptop lane). User: "C3.
D23Y7 / D23AA re-measured on the laptop with today's build: DUKE HILL town
and FAMILY JEWELS start at 120, ULTRA and SIMPLE; profile the worst seconds
(emulation thread, workers, GPU) and name the dominant cost of each dip."

Evidence labels: MEASURED, CODE (read in the source), INFERRED.

## 1. Setup

Field laptop (i7-5600U 2c/4t, 15 W, Intel HD 5500), checkout at `33e40e1`
(binary built at `081386c`, the same code), lab `recomp/analysis/d23ab/`
(local; the d23y6 lab with a `GRADE` switch that applies a grade's rows
exactly as `run.py` does, then sets FRAME RATE), the lab's private card
copy (DUKE HILL town private slot 4, FAMILY JEWELS start slot 11),
offscreen (SDL offscreen driver on the real GPU), case `cad` (30 s of
walking, turning and firing), three interleaved runs each. Host state was
checked before every run (`xm.sh`: CPU idle over 3 s, I/O pressure, no
package, backup, indexer or build process; all passed, no waits). Offscreen
numbers leave out the window present and the compositor: they are not the
real display.

## 2. The two scenes at 120 (MEASURED)

| Scene, grade | game median / lowest tenth | pictures a second | emulation thread | worker | GPU busy @ MHz | CPU MHz |
| --- | --- | --- | --- | --- | --- | --- |
| DUKE HILL town, SIMPLE (2x) | 59.9 / 58.8 | 120 (120 120 120) | 75% | 83% | 78% @ 557 | 2494 |
| FAMILY JEWELS start, SIMPLE | 59.9 / 58.8 | 120 (120 120 120) | 63% | 73% | 78% @ 559 | 2494 |
| DUKE HILL town, ULTRA (4x) | 50.1 / 41.9 | 45 (44 46 45) | 93% | 25% | 90% @ 795 | 1224 |
| FAMILY JEWELS start, ULTRA | 58.6 / 51.0 | 51 (51 49 52) | 82% | 42% | 75% @ 774 | 2594 |

(Pictures: swaps a second from the present ring. At ULTRA the redraws are
mostly off, "workers behind", and the original 60 presentation runs, below
60 when the game itself slows.) SIMPLE holds 120 in both scenes with the
game at 60; ULTRA does not, and in the town the game itself drops to 50.

## 3. The worst seconds (MEASURED)

One more run each with the emulation thread profiled (`PSX_PROF` with
stacks, each sample tagged with its game frame), `c3worst.py`: the fifth
of the seconds with the fewest game frames against the fifth with the
most.

- DUKE HILL town, ULTRA: worst seconds 29-46 game frames, CPU clock 1,098-
  1,197 MHz in four of seven and the emulation thread 98-99% there; the
  best seconds 60-63 frames at 1,377-3,134 MHz. CPU time a game frame
  barely differs (16.8 against 15.8 ms): the work is the same, the clock is
  not. Three worst seconds ran at 2.7-2.9 GHz with the emulation thread at
  only 35-54%: waits (presentation or GPU), not work.
- FAMILY JEWELS start, ULTRA: every worst second at 1,197-1,796 MHz with
  the emulation thread at 93-97%; the best at 2,477-3,118 MHz. A frame
  costs 16.9 ms there against 11.8 ms in the best seconds, spread over
  device service, timing edges, GP0 and our world pass in about the
  clock's proportion.
- At ULTRA the emulation thread also draws the game's own frames (redraws
  off): the GP0 path (`dma_gpu_ll_advance`, `gpu_write_gp0`) 11-15% of it
  and our host geometry (`draw`, `world`, `object`) 8-12%.
- SIMPLE, the town: no dip (game lowest tenth 59.0, 120 pictures every
  second); the emulation thread pays the capture and publish (about 5% each)
  instead.

## 4. The dominant cost

At ULTRA the dips' dominant cost is the CPU clock: with the 4x GPU 75-90%
busy, the 15 W package takes the CPU down to 1.1-1.8 GHz in those seconds
and the emulation thread (12-17 ms of CPU a frame) cannot finish 60 frames;
no single code path grows. This is note 474's finding again (the GPU's
share of the package lowers the CPU clock), now at 4x. At SIMPLE (2x) both
scenes hold 120. The user plays ULTRA at 120 on the real display and
accepted it (2026-11-16); these offscreen numbers do not contradict that
(no vsync or compositor here), and a real-display look in the town at
ULTRA and at PRETTY would show where the package limit starts.

D23Y7 and D23AA stay Blocked: what would help at ULTRA is less work per
frame on the emulation thread (D23Y5, D23Y8) or less GPU power per frame,
not a fix in either scene.

## 5. Reproduce

`recomp/analysis/d23ab/` on the field laptop (local): `xm.sh` (`PRE=c3
GRADES="ultra simple" RATES=120 SLOTS="4 11" REPS=3`), `xmsum.py c3`,
`c3prof.sh` (profiles), `c3worst.py TAG`.
