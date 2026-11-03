# D17BC - Classic: lightweight visibility

2026-11-03. Status: Done (accepted 2026-11-03: "great work. fixed. lock it in and close it out").

## Request

User: Classic shows more over the slot-10 castle wall than Polymorph does
after D17BB. Classic should be the lightweight, easy option: bring the
Polymorph reduction to Classic and suggest any other Classic reductions.

## Cause

Classic's over-the-walls list (D17AQ open sky, `open_sky_rooms` in
`recomp/src/ttk/draw_distance.inc`) still offered **every** sky room whose
piece tops clear the camera room's walls. D17BB restricted only Polymorph's
host selection to the camera room's own open-air region (sky rooms joined
through the level's portals, sky room to sky room). Over the slot-10 moat
(room 1), Classic therefore also drew the courtyards (rooms 8, 12, 16, 18)
through rectangles around their wall tops, which show above the outer wall.

## Change

`open_sky_rooms` now uses the same rule and switch as Polymorph:

- a portal-listed sky room outside the region is not widened (its own portal
  rectangle stays);
- a sky room outside the region is never appended;
- `DNTTK_OVER_WALLS=0` turns Classic's over-the-walls list off, `all` restores
  the pre-D17BC list; console `hostvis walls off|region|all|default` switches
  both renderers live (developer, session only);
- `draw_distance.open_sky.region_skipped` counts the skipped rooms.

Classic therefore never adds an over-the-walls room that Polymorph would not
select. The game's own portal walk is unchanged. Vanilla is unchanged (no
draw-distance extension, no open sky). Polymorph is unchanged.

## Evidence

Local under `recomp/analysis/d17bc/` (`look.py`, `sheet.py`, shots, logs).
Private copies of the player's card and savestates (UI slot 10 = file slot 09,
saved 2026-10-30; UI slot 12 = file slot 11), offscreen video, dummy audio and
private debug ports; the player's saves and profile were hash-checked
unchanged after each run.

- **Slot 10, A/B in one run** (`shots/ab`, `ab.png`): at each of 16 headings
  the same frame was shot with `hostvis walls all` (old behavior) and with
  the default. The old list draws courtyard wall tops and merlons above the
  outer wall; the new one does not. The moat, banks, outer walls and gate
  stay drawn.
- **Slot 12 courtyard in Classic** (`shots/court`, `court.png`): the tilted
  moat-side merlon over the battlements is gone, as in Polymorph after D17BB.
  One block above the wall at the first heading remains in both modes, the
  same as Polymorph.
- **Cost**: the over-the-walls rooms appended during a 10 s turn fell from
  877 over 223 open-sky frames (3.9 per frame) to 602 over 241 (2.5 per frame).
  The turn is not frame-identical between runs, so this is indicative only.
  Fields per second stayed at 59.9 in both runs (the offscreen harness is
  capped at 60), so no frame-rate gain is measured; the saving is fewer rooms
  drawn through the game's own renderer.
- **Tests**: `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels) 125
  groups pass; the D17AQ group now joins its two sky rooms through a portal
  and also checks that a sky room in another region is not listed.
  `ttk-input-test` and `ttk-near-test` pass. `check_repo.py` passes.

## Other Classic reductions (proposals, not built)

Classic is the lightweight option, so each of these trades view range for
cost. The user chooses; none is needed to close D17BC.

1. **Over the walls only when the eye is above them** (D17BB option A, for
   Classic). From a sky room on the ground, skip the over-the-walls test
   entirely unless the eye is above the camera room's wall tops (with
   hysteresis). Saves the per-frame room and piece test on every outdoor
   ground frame; jetpack, rampart and tower views keep the vista. Small,
   recommended next.
2. **Classic over-the-walls off** (`DNTTK_OVER_WALLS=0` made the Classic
   default). The PlayStation's own portal view: cheapest, but the jetpack
   moat vista returns to the D17AQ cut-offs (rooms popping and cut
   mid-screen). Not recommended unless Classic needs to be strictly the
   original look.
3. **Shorter Classic draw distance.** Classic uses the same 0xffff far limit as
   Polymorph. A lower Classic limit (for example D17P's doubled limit) would
   draw fewer distant rooms but brings back the black far boxes in long views
   (slot 8 subway, slot 10 moat). Not recommended.

## Playtest round 1 (2026-11-03): shaking after a live switch

User: over the walls, Polymorph shows mostly black (with the gate visible
when moving forward on slot 10) and that presentation is "totally workable";
Classic should be similar (which is what the region rule gives). Then:
"Start the game on polymorph, and simply switch to classic, and then the
screen starts basically vibrating when you try to turn the mouse."

Investigation (not reproduced so far; private copies, the player's saves
hash-checked unchanged):

- The player's session log (17:39) shows one switch to Classic and no failed
  redraws, misses or errors. The repeated "host world visibility off" lines
  are the redraw workers each logging the switch once.
- Offscreen at 60 and 180 Hz (console switch, first person, slot 10): no
  one-frame outliers in 216 shots during a turn, with and without the D17BC
  rule.
- Foreground on the 180 Hz display, recorded with ffmpeg x11grab while real
  X11 mouse motion turned the view (`fg.py`, `recshift.py`, `recres.py`).
  Polymorph launch, Classic launch, console switch and the OPTIONS menu
  switch (pause, DISPLAY, RENDERER, CONFIRM, resume), at 2x and at the
  player's 4x: 180 presents/s, no misses, the same step pattern in every case
  (0-4 isolated backward steps in about 150 image changes in all of them,
  including the Polymorph and Classic launches), no vertical shake.
- The D17BC change only filters which rooms Classic lists; it does not touch
  the camera, the late mouse view or presentation.

Next: the user's own recording of the shake, and whether it also shakes
on a Classic launch (the saved renderer is now Classic), to separate a
switch problem from Classic at 180 Hz.

## Playtest round 2 (2026-11-03): the shake found and fixed

User video `research/screencaps/Video_2026-11-03_18-03-45.mp4` (local):
Polymorph launch on slot 10, OPTIONS > DISPLAY > RENDERER Classic, then
turning. User: a Classic launch does not do it; slot 12 does not do it; it is
hit and miss ("now its hit and miss as i cant replicate it again").

**What the video shows** (frame analysis, `video/`): from about 1 s after the
switch, single presents show a view from earlier in the session (another
heading, more sky, the wall tilted) between normal frames, 36 times in 10 s,
in a fixed rhythm of 11 and 21 video frames (two per 0.53 s), with the same
old image repeating for several cycles. That is two entries of the 16-entry
redraw image cache presenting stale content as the cache cycles.

**Cause (confirmed state, framework).** On Polymorph the render thread (D17AW)
draws redraws, and the emulation thread adopts each finished image by
pointing its cache entry at the thread's texture (`gl_rt_adopt`). D40's switch
to Classic turns the render thread off but kept every adopted texture in the
cache, did not drop queued work and did not advance the epoch, so unfinished
redraws from before the switch could still be adopted later. On Classic the
emulation thread then drew into, resized and deleted textures that belong to
the render thread's renderer instance. `gl_rt_forget_cache()`, written for
this, was never called. A Classic launch never starts the thread, and whether
stale entries show depends on what the thread holds at the moment of the
switch, which fits "hit and miss". In the stress run below every switch found
all 16 cache entries holding render-thread textures.

**Fix** (framework, exported in the accepted patch):

- `psx_mod_set_render_thread()` turning the thread off (while it runs) drops
  its queued and unfinished redraws (`psx_rt_drop`: queue, epoch, slot
  snapshots) and releases the adopted cache images (`gl_rt_forget_cache`), so
  Classic draws every entry into its own texture. The log records it:
  `psxrecomp: render thread off: N queued redraw(s) dropped, M cache image(s)
  released`.
- The image cache records which textures it made (`s_rp_own`): it never draws
  into or deletes an adopted texture, and adopting replaces (deletes) its own
  texture, so switching back to Polymorph does not leak one image per entry.

**Evidence.**

- Stress, offscreen, 180 Hz, 4x, slot 10, Polymorph launch, 12 live switches
  while turning: every switch to Classic logs `0 queued redraw(s) dropped, 16
  cache image(s) released`; 179.3 presents/s, 177.6 distinct, 0 misses, 0
  failed redraws; the render thread resumes on each switch back.
- Foreground on the 180 Hz display, 4x, real X11 mouse motion, Polymorph
  launch then switch, three runs: no flashes of the video's size. The old
  build did not reproduce the flash in this harness either (six runs, console
  and OPTIONS menu), so the fix is not proven against a reproduction; it
  removes the confirmed stale-texture state.
- Suites: `ttk-controls-test` (LEVEL00, LEVEL01, all levels) 125 groups,
  `ttk-input-test`, `ttk-near-test`, `ttk-live-env-test`, `ttk-flash-test`,
  `ttk-aim-test` pass. Framework patch exported; a clean worktree of the pinned
  commit `08ec704` plus the patch applies and reproduces the changed files.

The user's playtest confirmed it fixed (2026-11-03).
