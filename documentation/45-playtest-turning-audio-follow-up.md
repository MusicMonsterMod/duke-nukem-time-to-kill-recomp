# Playtest follow-up and clear-session handoff — 2026-09-27

**Follow-up work:** see [implementation, comparisons and remaining limits](46-turning-audio-inventory-progress.md). The documentation-only stop point below is historical; contextual Enter has since been implemented.

This is a documentation-only checkpoint after speech-to-text player feedback.
No runtime edits, build, game launch/input, or saves/settings writes occurred in
this checkpoint. The player may still have a game running: detect and avoid it.
The latest explicit correction requests **Enter to activate inventory items**.
This supersedes the earlier suggestion to retain Enter as pause in all contexts.

## Accepted results

- **D08G2 Done:** silent cheat entry and horizontally centered results explicitly accepted.
- **D08J Done for the bounded tested route:** gun-drawn run/jump/E transfer from the
  first platform to the second exterior ladder succeeded twice; user says the job
  is done. Keep previous private pistol/shotgun/switch/redraw evidence and broader
  weapon/era limits; do not reopen accepted mechanics without a regression.
- **D08I Needs playtest/refinement:** run-start jumps and platform departures feel
  substantially more reliable (user estimates 95%); exact close-bed case below remains.
- **D08A1 Needs playtest:** inventory presentation strongly accepted. Activation
  needs the explicitly requested contextual Enter behavior below.
- Aiming and ordinary close/longer bed approaches continue to work well. Preserve
  D07C and furniture movement. Strip-club secret door/wardrobe worked; this does
  not establish apartment hidden-pipebomb/NPC/switch criteria still unreported in D08H.
- **D08K Blocked:** original animation/collision audit remains authoritative; no
  safe low gait/override pipeline. **D19B remains deferred** with modern menus.

Speech-to-text references to “pig cups/cocks” mean Pig Cops, and “silent sheet /
scented confirmations” mean silent cheat entry / centered confirmations.

## Selected next work, in order

### D10A — rapid mouse turning, especially running with Shift

Report: quick mouse movement felt wrong from the starting street. Inside the strip
club, running with Shift held made it difficult to turn/look toward Pig Cops to
shoot. Shift involvement is a hypothesis, not a demonstrated cause. Aiming itself
was otherwise praised; do not reopen accepted projectile alignment.

Reproduce slow/fast sweeps, standing/walking/running, Shift held/released, aiming
and moving, outdoors versus club interior. Trace input receipt/accumulation/clamps,
capture/focus/epochs, frame pacing, camera ownership/animation leases, yaw/body
facing and original collision limits. These are investigation leads, not new source
findings. Acceptance: record the failing comparison before claiming a cause; apply
only a bounded supported fix, verify responsive turning with/without Shift and
preserve legitimate camera collision, aiming, traversal, custom bindings and Vanilla.

### D18A — audible crackle during voice/music/gunfire

Report: crackle during voice clips and strip-club music, possibly overlapping
music, and while shooting the Pig Cop on the ledge overlooking the station. User
is unsure whether this predates this build. No regression or cause is established.

Reproduce and capture actual audible output privately where feasible. Inspect
callback deadlines, underruns, buffers, resampling/mixer clipping, CD-XA/voice/music
and SPU timing, plus host scheduling. Compare overlapping sources versus individual
sources and gunfire, both profiles and an available prior build without replacing
the player build. Earlier dummy-audio tests do not verify audible quality. Do not
conflate crackle with previously fixed repeated voice-bank playback. Acceptance:
record evidence and limits, distinguish clipping/underruns/source artifacts before
fixing, preserve sync/latency/music and verify the same route after a bounded fix.
Do not blindly enlarge buffers or mute sources to hide it.

### D08I — residual close-bed Shift+W+Space jump

Exact report: move as close to the bed as possible, hold Shift, then press W and
Space; Duke sometimes jumps vertically instead of toward/on the bed. Other close
and longer approaches worked. Reproduce proximity and input ordering; compare
Shift pre-held, no Shift, simultaneous W+Space, W→Space and Space→W. Trace direction
latch, input receipt/consume, gait, support, original clearance, animation and
velocity through landing. Distinguish legitimate feet-clear-obstacle delay from a
lost direction/gait handoff before changing anything. Acceptance: improve any
proven inconsistent valid jump while retaining original clearance, collision,
reach, accepted furniture/edge behavior and no double jump. Keep six-input-frame
small-edge grace and its large-fall/headroom guards bounded.

### D08A1 — contextual Enter activation and clear guidance

User explicitly corrected the plan: “i want enter to be able to select the
inventory items”. Bracket highlighting already selects the gadget, so implement
Enter as **use/activate the highlighted item while the gameplay inventory strip is
visible**, consuming that press so it does not also pause. Outside that context,
Enter retains pause. Keep U/item_use and existing custom bindings; no persistent
rebinding or player-setting migration. Preserve accepted tile design; show concise
contextual guidance reflecting actual usable bindings. This is selected work, not
a permission question. No runtime implementation has happened in this checkpoint.

Audit Enter/Start/pause ownership and capture/menu guards before implementing.
Acceptance: brackets→Enter uses/toggles exactly the highlighted item once and never
pauses; held/repeated Enter cannot double-use or fall through into pause when the
strip closes/times out. No-item, depletion, timeout, U, custom bindings, released
capture, existing menus and focus/scene transitions behave safely. Ordinary Enter
pause and menu confirmation remain intact outside the gameplay strip. Check both
renderers and Vanilla. Do not broaden into deferred D19B menu redesign.

## Preserved implementation and evidence

Last delivered binary SHA-256 (recorded, not rehashed during this docs-only turn):
`f89f1ca92d5a7e0445022d98e6d9925bf893249bd9fa5bd410c7eec99e3c3bcc`.
[Implementation/evidence](44-traversal-inventory-implementation.md),
[historical progress](42-traversal-feedback-progress.md),
[crouch blocker](43-crouch-walk-audit.md).
`recomp/analysis/iteration42/verification.json` and `preservation.json` record the
previous build, six native suites, 60 Python tests (2 skips), private traversal and
14-checkpoint Vanilla route. They are prior evidence, not new tests of these reports.
The existing source checkout is dirty/untracked; preserve all unrelated work.
Never hand-edit generated C or original assets, use player cards for tests, or
interfere with an active player game. Read build/tooling docs before implementation.

Update board/status/handoff and evidence after each meaningful finding so another
agent can resume without repeating work. Record exact modified files, build hash,
reproduction/result paths, outstanding failures and private process ownership.
The user is conserving account quota and plans to clear/switch agents; do not
claim visibility of remaining quota. Current stop point: documentation complete;
all four follow-ups selected, no reproduction/source diagnosis or fixes begun.

Launch current build with saved preferences:
`python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`
Modernized captures automatically; F10 toggles capture. Current build still uses
U for highlighted inventory activation; contextual Enter is pending implementation.
