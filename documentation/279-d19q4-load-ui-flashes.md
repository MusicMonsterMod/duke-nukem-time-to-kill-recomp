# D19Q4 - Load-time UI flashes

Date: 2026-10-27. Status: Done (user-accepted).

User: "lock it in and close it out". Acceptance and implementation/documentation
commit and push authorized. Verification limits below remain unchanged.

## Findings

Both reported symptoms reproduced with private save copies and an isolated Xvfb
window. They have separate causes:

- SAVE GAME can snapshot VRAM containing OVERWRITE THIS SAVE?. Loading restores
  that image before D19Q3 resumes and redraws gameplay. The baseline recording
  shows the old confirmation for about 0.22 seconds on repeated quickloads.
  This is saved display content, not a new overwrite operation.
- Host inventory selection and overlay lifetimes survived the world change.
  Synchronizing a different restored selection was treated as a new selection
  announcement, opening the bracket-operated strip without a new bracket press.

## Candidate

The successful-load callback now resets the title menu's transient state before
presentation. For authenticated, living Modernized gameplay, it holds the existing
window image until an unpaused composition reaches the other draw buffer. This
proves a fresh composition has passed the native flip, rather than relying on a
fixed delay. The main presenter and GL replay swaps both respect this gate.
The guard does not change guest simulation, VRAM, save contents or D19Q3's resume
finalizer. Existing saves require no conversion.

The wait has a one-second safety limit and cancels on leaving gameplay/death.
Vanilla, death screens and statistics do not arm the gameplay presentation wait.
`DNTTK_LOAD_UI_TRACE=1` reports arming/completion; timeout is always logged.

Inventory images reject an old successful-load generation immediately. The next
update clears gadget/mission lifetimes and queued mission browsing. Restored
selection synchronization is silent and drops the old command serial; a fresh
bracket command still selects and displays normally. The existing availability
rules continue to choose a usable item when the saved selection is unavailable.

Files: `recomp/src/ttk/game_menu.cpp`, `inventory_hud.cpp`, `shortcuts.inc`,
framework `runtime/src/main.cpp` and `gpu_gl_renderer.c`, and the inventory and
controls native regression tests. The framework delta is exported in
`recomp/patches/time-to-kill-accepted-source.patch`.

## Verification

Candidate executable SHA-256:
`4c6f5450d2a8b126f0fe36e5080845a63801d5498619b06de59e046d32056a33`.

- Incremental player build and native inventory/controls suites pass. Added
  tests cover immediate hiding before an inventory update, silent restored
  selections, stale command/browse rejection, fresh bracket/mission input and
  unchanged item contents. A simulated load in the controls fixture also needs
  the normal camera update to renew its gameplay lease before subsequent tests.
- D37A `test_animation_save_restore.py` passes (allocation recovery, GPU clearing,
  Bio Mask reservation and animation header validation).
- Private real-keyboard tests at 60 and 120 FPS cover an old overwrite-dialog
  save, running saves, newly overwritten saves, repeated F9, F3 LOAD GAME, F7,
  metadata-free paused saves, subsequent movement and normal Escape pause/resume.
  The two runs record 20 successful fresh-frame completions and zero timeouts.
- Recorded window review shows deliberate overwrite confirmation and bracket
  selection still visible; old overwrite text and inventory announcements do
  not reappear after loads in these routes. Recording is 60 frames/s, including
  the 120 FPS player run; this is not an exhaustive every-present oracle.
- Separate OpenGL/software checks preserve level-complete statistics on load.
  Software resumes the paused gameplay fixture; Vanilla retains its pause.
  Death exclusion is a source guard, not a new full live death-route test.
- Exported patch applies to the clean pinned framework and reproduces all 53
  modified/new framework files byte-for-byte, including the pre-existing delta.

Evidence stays local under `recomp/analysis/d19q4-ui/`: baseline/candidate
recordings, scripts, results, screenshots and redacted logs. Tests use private
profiles, blank private card directories and copied savestates; no player card
is used. The first heavy recording attempt hit the diagnostic starvation
watchdog after loading. Later bounded recording runs completed with a longer
diagnostic watchdog budget. Its exact cause was not established; no performance
claim is made from recording runs. Xvfb used dummy audio, so audible quality and
physical-display feel remain user playtest items.

## Player check

Load several saves through LOAD GAME, F9 and F7, including one saved after an
overwrite confirmation. No old overwrite text or unexpected inventory strip
should flash. Confirm that deliberate overwrite prompts and [ / ] still work,
and that loading resumes immediately while a later Escape pauses normally.

From the workspace root:

```sh
python3 recomp/tools/local/run.py
```

User accepted the candidate and authorized closeout, commit and push on
2026-10-27. No next job started.
