# D08T12 - Crawling out of a space lower than a crouch

Status: **Done** (user-accepted and closed 2026-10-30). Modernized crouch walking only
(CROUCH MODERN); Original crouch and Vanilla unchanged.

## Report

UI slot 9 (saved by the user while stuck, 2026-10-30 13:24): with the slot 8
Duke-symbol block pushed into its wall recess, Duke got onto its top under the
recess ceiling, kneeling, and could neither stand nor move.

## Cause

On a private copy Duke kneels in animation 178 on the block top (Y 19814,
block at Z 35331). The host adopts that stance after the load (D08L1 rule) and
correctly refuses the stand-up: `crouch_blocked` counts every release, since the
headroom is below the 0x370 standing clearance. Every crawl direction was then
refused by the crouch probe as `hull_low` (the `crouchwalk` console counters:
111 probes per direction, all low): the headroom where he already kneels is
below the 704 crouch hull, so every destination, including the way back,
measured "too low". Most likely a crouched landing (D08K9) put him there.

## Fix (game-wide)

`recomp/src/ttk/crouch_hull.inc`, `crouch_probe()` for crouch walking
(`allow_drop`): measure the headroom at Duke's own position first. If it is
already below the hull (and at least 0x100), a move may go anywhere with at
least that much headroom, and the head ray runs just under that ceiling. Duke
can therefore always crawl back out the way he came or toward more room, never
into tighter space. Every other probe (stand-up, the test mover) keeps the full
hull. The `crouchwalk` console report gains a `squeezed` move count.

## Evidence

Private copy in `recomp/analysis/d08t12/` (local), real keys on a private X
display. Before: W/S/A/D (with and without Ctrl), Ctrl, Space and C leave Duke
in place. After (`out1.py`): S crawls along the block top, W crawls off its edge
into the original fall (105), Duke lands, stands (63) and walks; 73 squeezed
moves, no `hull_low` refusals.

`ttk-controls-test`: the D08K12 group adds the squeezed case (moves at headroom
0x280, refuses at 0xf0) and keeps its vent-size, release-under-ceiling and
ledge-fall checks; all 115 groups pass, as do `ttk-input-test`, `ttk-aim-test`
and `ttk-near-test`. The player's save file hash is unchanged.

## Limits

- One live location. The rule is position-independent, but a squeezed space
  that only narrows in every direction still has no way out (none was found).
- How Duke first got into the gap was not reproduced from a standing start.
