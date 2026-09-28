# D08 control polish — interaction, capture and movement

Current close-range/temporary-holster follow-up: [D07B / D08 close combat](28-d08-close-combat.md). Earlier behavior below is historical.

The user confirms that climbing, ladder-to-ladder jumps, and continuous running
landings work much better in the previous build. This follow-up addresses the
reported close respawn camera, occasional landing hesitation, heavy movement,
C-only default holstering, automatic Modernized entry, and interaction-only E.

**E must never fire.** The user's explicit correction supersedes the earlier
left-click parity proposal. E is not an alias for armed Cross. A tap requests the
original Circle holster sequence, waits for verified holstered ownership, then
supplies a short unarmed interaction pulse. Holding E keeps the original action
held for supported climbing. It does not directly change equipment or inventory,
and it does not automatically redraw on a ladder. C draws/holsters. Pending
interaction expires after 120 input frames and is cleared by input release/focus,
manual holster, or firing; live ownership is rechecked before every action pulse.

Profile schema 6 defaults new installations to Modernized and moves the old H
holster default to C. C remains reserved against rebinding unrelated actions,
because it also supplies the fixed original Circle input. Existing explicit mode
choices, custom holster keys other than H, other bindings, rendering and aim
preferences survive migration; the previous file is backed up. Vanilla remains
selectable through the current launcher settings.

Automatic capture is offered only by the authenticated first-map normal-camera
callback. SDL capture runs on the event-pump path. Menu/focus releases clear held
input and enforce a short recapture delay; a fresh gameplay callback is required.
Explicit Escape/F10 release disables automatic recapture for that session, keeping
the original-controls escape hatch. Manual F10 capture is still available.
Unsupported map/state ownership does not grant new movement permissions.

The independent camera now takes its initial requested boom from the original
unconstrained offset (+94/+98/+9c relative to target offset +88), which the guarded
original 28e1c path uses before collision. An established requested radius survives
temporary original-camera ownership. Wall-shortened or death-transition camera
positions therefore cannot redefine the requested follow distance. Original
camera constraints and world queries still determine the final position.

Trace research also found that the first running-jump ballistic update can occur
in state 9/0, before 9/9. Rejecting that first update could exhaust the short
intent lease before the next update, leaving a short/sideways jump unowned. The
exact authenticated takeoff caller now accepts 9/0 as well as 9/9; it still
redirects only once and preserves collision response. Normal-camera input intent
is refreshed independently of gait callbacks, covering short run-ups.

During the original running landing transition, state becomes 0/9 before the
animation changes from 103/104 to locomotion. The owned flight input lease now
covers that narrow seam; it does not grant normal gait writes there. Obstacle
impact animations still retain original recovery and standing jumps retain their
original rules.

Normal walking/running displacement is filtered before the original collision
query to reduce animation-driven speed pulses. The filter averages consecutive
root-motion magnitudes, preserves direction immediately, seeds modest initial
movement, and resets across epoch/gait changes or a callback gap. Released-input
braking uses 35% of the original animation tail. This changes horizontal feel;
vertical displacement, collision integration, jump ballistics, damage, climbing
and scripted motion remain original. Foot animation is still the original
animation and requires subjective playtesting against the smoother displacement.

## Verification

[Delivery evidence](reports/d08-control-polish.json) records the exact binary,
native/Python checks, original input replays, E ammunition/shot counters,
pause/capture transitions and respawn camera distances. The accepted automatic
route keeps E-only ammo at 200 and shot count at zero, observes the unarmed action
pulse after holstering, and verifies C redraw and pause/resume. A later deliberate
Mouse1 shot engages the enemies and is separate from the no-fire E segment.
After original enemy death/restart, camera distance settles near 3045 without
F10; after an explicit release/recapture it is 3044. A second manual-entry replay
also returns to approximately 3045 after respawn. Native coverage reproduces
temporary camera shortening and verifies retention of the requested boom. Enemy-death research uses a logged private health-only fixture;
no player position, camera, animation or traversal-state writes. Player cards are
never used. Earlier incomplete harness runs are retained and excluded from
acceptance. Broader D08 progression/security-card, swimming and campaign coverage
remain open; audio choppiness still needs listening evidence.
