# D53C - no reach into a Duke 3D security TV

2026-11-21. User-accepted ("lock it in and close it out, perfect").
Labels: U user report/acceptance, S source/native tests, L isolated runtime observations.

## Report

U, UI slot 10: Duke physically puts his hand into the security screen when he
uses it; either no hand animation, or keep the touch in front of the screen.

## Cause

L/S, LEVEL00 monitors (type 858, subtypes 39-41):

- The Action press `0x80051890` lines Duke up with `0x8007ec4c(player, 0x113)`:
  a ray to the object's collision face (`0x80076330`), then Duke's x/z are set
  275 units from the hit point in one update (heading corrected up to 45 degrees).
- The selector `0x800513e4` picks the class-2 use 277 (the vending/NPC gesture).
  Its hand joint reaches 283 in front of Duke: onto the TTK panel on the wall.
- The monitor callback (`0x800cc14c`) only answers event 0 (available) and
  event 3 (alternate camera `0x8003b36c`); event 3 is keyed inside 277, about
  0.47 s in.
- D53's TV stands 140 out from the panel with its glass at 125 (note 432), so
  the reach went about 130 units through the glass in both views.

Keeping the touch in front of the glass was ruled out: at the lined-up spot the
shoulder joint is already 46 units from the glass, and a larger line-up distance
snaps Duke about 130 units backwards when use is pressed.

## Fix

`recomp/src/ttk/monitor_use.inc`, at the pose build `0x800411b8` (return
`0x80042634`, after every channel blend, like the D08J7C one-handed pull-up).
While 277 plays during a committed use (stage 1-8, `+0x224 & 0x40`) of an object
`ttk::security_tv_object` reports as the TV (Duke 3D look on, TTK monitor mesh;
`security_monitor.h`), joints 9, 16 and 10-15 (chest, neck, arms) take the
unarmed idle 63's first frame on their own channel, ramped in over 60 ms and out
over 150 ms of real time. Third person, the first-person contact rig,
reflections and redraws all read these joints.

Unchanged: legs, root, line-up, 277 timing, the camera event, weapon stow/return,
every other 277 use, the TTK look, Classic and Vanilla. Developer A/B
`DNTTK_MONITOR_USE=native`; debug JSON `monitor_use` (active, weight, uses, writes).

## Evidence

- L: private copy of UI slot 10, offscreen. Third and first person at 60 and
  third person at 180: no hand at the screen; camera bit at the same point; weapon
  returns. TTK look: native reach onto the panel, override unused. A/B native:
  unused. Player saves unchanged after every run.
- S: native group D53C in `modern_controls_native.cpp` (held pose, ramp-out, legs
  native, vending-style 277, no TV look, other caller, Vanilla, no use in
  progress); controls 161 groups, input, aim and near suites; Python suite.
- Pairs: `research/before-after/D53C/` (local). Lab: `recomp/analysis/d53c/`
  (local; `lab.py`, `mdis.py`). Executable
  `cdef94b467b718da4ac71bc19ba79210c802a3b7ceb4b1a4747bb9237b5bdd18`.

## Limits

Only LEVEL00's monitor mesh becomes the TV, so only it is affected. First person
at 180 Hz and the real display were not recorded; Vanilla was checked in source
and the native test. One frame at the switch draws the REC frame over Duke's own
view before the feed; this predates D53C and is unchanged.
