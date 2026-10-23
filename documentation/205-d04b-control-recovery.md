# D04B - Modern control recovery and explicit scheme selection

Status: Done (user-accepted 2026-10-23). Implementation and documentation
closeout explicitly authorized, including the bounded follow-up below.

Accepted playtested executable: `31b9ed43bc8738ad4eeb968af629d3be423ff2172a97cfa0d22305766bed950c`.
Final executable after the bounded follow-up:
`41bb0257acb5bd2e89677b5062fad2c74da0ed0e94d08cca546ae9847dd564c5`.

## Confirmed failure

On the unchanged accepted D17S executable, load a private copy of the paused
SAVE GAME fixture, resume and wait for automatic capture, then F9 followed
immediately by Escape and another Escape about 120 ms later. Gameplay can
remain running with the mouse released after a further three seconds. The
same failure reproduced with a gameplay/debug fixture and a 20 ms delay
between F9 and Escape. An initial shorter probe sampled too early; the later
`baseline-confirmed` run establishes the persistent failure.

The Escape hold waited for gameplay capture offers to stop for eight host
frames. Load/resume and closely spaced Escape pulses can leave gameplay
running without that gap. Diagnostic telemetry showed `capture_hold=true`,
`capture_allowed=true`, `capture_requested=true`, fresh offers, and no capture.
This is a host capture-recovery failure, not evidence that the saved profile
itself changed. It is separate from D19Q2's deliberately paused debug saves.

## Change

- The input layer accepts explicit menu ownership/entry as acknowledgement of
  Escape. It also ends the hold when an authenticated gameplay offer arrives
  after the Start pulse has expired and its freshness window has elapsed.
  A pulse consumed or merged around loading cannot disable recovery forever.
  Host overlay, focus and fresh-offer gates still apply.
- F10 has no action. No ordinary shortcut opts out of automatic recovery.
- Removed implicit WASD tank/strafe and mouse-to-D-pad fallback when the
  Modern control lease is absent. Released Modern gameplay no longer exposes
  the fixed original keyboard movement route. Identity guards are unchanged;
  unknown code is still refused, not treated as safe Modern gameplay.
- OPTIONS > CONTROLS > CONTROL SCHEME offers MODERN (default) and LEGACY.
  CONFIRM applies it live and persists it through the existing menu-settings
  file. Profile schema 40 adds `controls.scheme`, preserving older choices
  and adding Modern on migration. Guest saves cannot override this host value.
- Legacy uses original turning/aiming with bound actions and keeps the
  Modernized menu/save facilities. Modern gameplay snapshots and late mouse
  look are disabled for that explicit choice. Camera/weapon-aim policies honor
  it; choosing Modern restores their saved options. Vanilla stays separate.
- Debug input JSON exposes scheme, capture hold/request/permission and offer
  age. Manual/help text no longer recommends F10 recovery or implicit fallback.

No framework sources were edited. The pre-existing patched submodule worktree
was preserved; the accepted runtime patch remains unchanged.

## Verification

All runs used private profiles and copies of test saves/cards under
`recomp/analysis/d04b-controls/`. The player's settings and saves were not
modified. Xvfb supplied real SDL keyboard/mouse events, with OpenGL and dummy
audio; these are not physical monitor pacing or audible sound tests.

- Incremental local-dev build, movie/math shard checks: pass.
- `ttk-input-test`: pass, including no-gap Escape recovery, menu acknowledgement,
  host-overlay/focus recovery, inert F10, no implicit tank input, explicit
  Legacy selection and selection surviving the host load notification.
- `ttk-aim-test` with the owned EXE: pass.
- `ttk-controls-test` with the owned EXE, clean LEVEL00 fixture, LEVEL01 and all
  21 selectable level overlays: pass, including identity rejection, Vanilla,
  traversal/camera leases, aiming and movement contracts.
- Python profile tests: 89 pass. Binding tests: 11 pass. Includes schema-39
  migration, Legacy persistence and the in-game settings-file import path.
- 36 rapid load/Escape sequences across 30, 60, 120, 180, 240 and Unlimited,
  first/third-person views and private fixtures from levels 0, 1, 2, 3 and 6:
  after settling, every running-game case had capture and every paused case
  had a free cursor. Rapid presses can merge into one original Start pulse;
  the test does not assume that two presses necessarily leave the game running.
- Real menu actions: Modern -> Legacy, F10, load retaining Legacy, then
  Legacy -> Modern; F2 SAVE GAME write, F3 LOAD GAME, F6/F9, F7, console
  open/close, movement and mouse look afterward: pass.
- Vanilla gameplay save/load, original paused save/load and explicit Start
  resume: pass. Paused debug saves were not globally auto-resumed.
- Controls screen visually inspected. Its description was shortened to fit.

An older level-9 fixture was rejected for its incompatible codegen hash.
Its header was not changed; it is not counted as a passed live check. Later
valid fixtures supplied the broader level checks instead.

Evidence: `baseline-confirmed.json`, `timing.json` (the diagnostic candidate
before the final recovery fix), `fix2.json`, `rate-*.json`, `rates.txt`,
`later-levels.json`, `menus.json`/`menus.txt`, `vanilla.json`/`vanilla.txt`,
`menu-preview.json`, screenshots and copied build/test logs in the local lab.

## Acceptance and bounded follow-up

User accepted the behavior as "way better" and authorized a quick polish pass,
then commit/push and closeout. They still observe a split-second interruption
when Escape immediately follows a load, with automatic recovery. This report
is retained rather than claiming the interruption is entirely gone.

The bounded follow-up removes the generic 12-host-frame capture cooldown when
our menu explicitly hands control back. It discards previous capture offers
and waits for the first new authenticated gameplay offer. The native test
checks both the refusal of stale offers and capture on the first new offer.
The no-menu-gap path still retains its guard/pulse timing. This is a targeted
resume improvement, not a rewrite of pause/load handling.

The final incremental build and input suite pass. The existing broader checks
above were on the accepted candidate; focused real-input quickload/rapid
Escape checks at 120 FPS were repeated on the final executable. Physical
monitor feel of the final polish was not independently user-tested. A live
death/Continue and full traversal/campaign sweep remains a coverage limit,
not a full-campaign fidelity claim. No additional job was started.

The framework patch was re-exported unchanged, applied to the pinned clean
dependency, and matched all 51 modified/new files exactly. No framework source
or pin was changed by D04B. All authored implementation, tests and documentation
are included in the authorized MusicMonsterMod closeout.

D17Y now records the separate slot-8 platform/wall rendering report. Backlog
only; no rendering investigation or implementation was performed.

Launch with saved preferences:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

No F10 activation or recovery step is needed. The user explicitly authorized
the acceptance, bounded follow-up and commit/push closeout.
