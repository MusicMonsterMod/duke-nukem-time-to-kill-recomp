# D09B - Classic gamepad auto-aim and red dot

Status: Done, 2026-10-28. The user confirms "all working" and explicitly
authorizes closeout after the live-switch marker fix. User-reported playtest
acceptance is separate from the private injected-input evidence below.
Earlier candidate statuses and unresolved observations are retained as history.

## Findings before implementation

AUTO AIM maps `aim_assist` to `DNTTK_AIM_ASSIST`; the Modernized menu enforces
bit 0 of the original settings word at `0x800c3f9c`, including Legacy controls.
Legacy makes `weapon_aim.cpp`'s view adapter ineligible. It does not substitute
the modern six-degree assistance for the original acquisition/shot path.

Native acquisition at `0x80058640` checks the option, target eligibility,
angles and visibility. The yaw acquisition threshold is 0x60 angle units
(about 8.4 degrees); continued lock and aiming use the original state at
player offsets +0x280/+0x284/+0x288/+0x28c/+0x2b0. Not every enemy visible
on screen is a valid lock. Private firing samples confirm native locks with
the option on and no locks with it off. No auto-aim blocker was established.

RED DOT maps independently to `DNTTK_RED_DOT`. D07D collapses its original
quad only when that preference is off. The native marker routine at
`0x80033af8` checks the upper-animation table's 0x800 bit, rather than the
auto-aim option. `0x800582dc` supplies either a locked target or the ordinary
forward aim point. It is pose-dependent, not an always-visible crosshair.
The marker was visibly present during firing even with AUTO AIM OFF.

Direct native operation already works in the tested context. Adding another
lock system or replacing the marker would not address an established cause.
The specific reported mode/save/device circumstances still need reproduction.

## Bounded correction

Removed the menu's false `NEEDS AUTO AIM ON` dependency for RED DOT and changed
its description to explain independent operation. Corrected the manual too.
The first candidate changed no gameplay. The subsequent marker-priority
revision below changes only crosshair visibility. Native acquisition, projectiles,
controller routing and Vanilla remain unchanged. No framework edits or pin changes.

## Verification

Private fixture: copies of existing diagnostic cards in
`recomp/analysis/d09b/cards`, internal slot 5 (UI slot 6), original weapon ID 4,
Legacy controls, OpenGL, 60 presentation. SDL virtual-pad input draws the weapon
with Circle/B and fires with Cross/A. Diagnostic RAM writes enable god mode,
replenish ammunition and set a heading near a live target; they do not replace
acquisition, collision checks, target selection or shot construction. This is
injected fixture evidence, not physical-controller gameplay acceptance.

| AUTO AIM | RED DOT | Native lock samples | Suppressed marker count |
| --- | --- | --- | --- |
| On | On | 12/12 | 0 |
| Off | On | 0/12 | 0 |
| On | Off | 12/12 | 29 |
| Off | Off | 0/12 | 30 |

All cases recorded zero modern shot rewrites and zero modern assisted shots.
Native framebuffer captures inspected for on/on, off/on and on/off distinguish
the original red dot from the muzzle flash and the modern crosshair. Turning
through nearby enemies also produced different native candidate pointers, but
this is not full acquiring/changing-target physical-device acceptance.

A second matrix used a private copy of the current player profile, changing
only Legacy and the two tested switches. Native locks appeared in 4/12 samples
for each AUTO AIM ON case and 0/12 for both OFF cases. Marker suppression was
0 with RED DOT ON, 6 with auto-aim on/dot off and 4 with both off. Modern shot
rewrites remained zero. Its saved 4x scaling and Auto presentation were retained;
Xvfb resolved Auto to 60 Hz and reported emulation shedding. These wall-time
samples establish feature operation, not equivalent timing or performance.
This is not a 120 Hz test. Results: `player-matrix-results.json`.
The user's report is confirmed as single-player (later narrowed to UI slot 4). Their saved profile currently
selects Modern with both switches off; this does not establish their settings
at the time of the reported failure. Player preferences were not changed.

After the UI correction, the player build and all 106 profile tests pass.
The revised menu was visually inspected with AUTO AIM OFF / RED DOT ON:
the row is selectable and bright, the description fits, and the false dependency
is absent. Private artifacts: `matrix.py`, `matrix-results.json`, captures and
redacted runtime logs under `recomp/analysis/d09b/`. Diagnostic games are closed.
No player settings, saves or cards were used for writes. No commit or push.

## Remaining acceptance

The user confirms the report is single-player. Reproduce their specific
save/weapon/action context; those details remain outstanding. Test a physical gamepad in Legacy with AUTO AIM and RED DOT
independently and together, including drawing, acquiring/changing targets,
firing, disabling and saved-setting reload. Broader weapons, multiplayer,
120 Hz and a fresh Vanilla gameplay comparison were not tested in this pass.
The UI-only correction does not establish that the reported failure is fixed.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Use OPTIONS > CONTROLS > CONTROL SCHEME: LEGACY, then OPTIONS > ADVANCED >
ORIGINAL GAME for AUTO AIM and RED DOT. Confirm settings to persist them.
Draw with Circle/B before firing with Cross/A; a holstered Cross is an action.

## Slot-4 follow-up and marker priority

User reports auto-aim appears to work, identifies UI slot 4 for the missing dot,
and requests that RED DOT ON disable the modern crosshair. This is user evidence
for auto-aim, not agent-observed physical-controller testing.

Copied the current `state_800AB6FC_slot03` files to private diagnostic cards.
Recorded source SHA-256/mtime values and verified they remain unchanged.
Native idle and firing captures plus composed renderer captures show the red dot
on the club doorway while firing; idle has no marker. Composed captures at both
1x and 4x internal resolution show it. This does not reproduce or diagnose the
reported disappearance. Screenshots: `slot4-present-fire.png` and
`slot4-4x-fire.png` in the private evidence directory. Diagnostic god mode and
ammunition writes were confined to the private runtime, with SDL virtual input.

RED DOT ON now makes the effective modern crosshair disabled. Its hotkey cannot
turn it back on or silently change the retained preference while the red dot is
selected. RED DOT OFF restores the prior crosshair preference. Both settings
remain stored independently; existing profiles need no migration. This is a
visibility policy, not a change to aim assistance, camera or native marker poses.
The CROSSHAIR row explains why it is inactive; the RED DOT description explains
that it hides the modern crosshair. AUTO AIM remains independent.

Verification: player build and the native aiming suite pass, including new
coverage for suppression, hotkey behavior and restoration of both on/off
crosshair preferences. Existing shot-vector, visibility, marker suppression,
airborne, swim, flight and energy-beam cases pass. A private Modern session
loaded slot 4 with both preferences on and reported no modern reticle. Switching
RED DOT off through the real menu restored the visible modern crosshair live
(`crosshair-live.json`, `modern-red-off.png`). The revised RED DOT description
was visually inspected and fits. The previous 106 profile tests still describe
the unchanged persistence path; they were not rerun for this visibility-only
revision. No commit/push. Physical-controller visual acceptance remains open.

## Scheme presets follow-up

The user confirms the marker behavior works and requests scheme-driven starting
settings, with manual adjustments afterward. Confirming a changed CONTROL
SCHEME now applies all three settings together and persists them:

| Selected scheme | AUTO AIM | RED DOT | CROSSHAIR |
| --- | --- | --- | --- |
| Legacy | On | On | Off |
| Modern | Off | Off | On |

This applies immediately on CONFIRM, without restarting. It is a preset on an
actual scheme transition, not a continuous restriction: manual adjustments stay
available and survive unchanged confirmation, game loads and relaunches. Cancel
leaves all values intact. No migration rewrites existing profiles at launch.
The crosshair hotkey override resets when its setting is explicitly applied,
so it cannot defeat the new preset or a confirmed menu choice. The RED DOT
priority from the earlier candidate remains in force for manual combinations.

Verification: player build and native aiming suite pass, including hotkey
suppression/restoration and explicit preference-reset coverage. Private actual
menu traversal checks cancel, Modern-to-Legacy, Legacy-to-Modern, a repeated
Legacy switch, unchanged confirmation and a manual AUTO AIM OFF override.
Saved values and live original option bits agree. `scheme-results.json` records
these checks under the private evidence directory. Candidate awaits user review;
no documentation closeout, commit or push is authorized by acceptance alone.

Restart verification: the private launcher profile retains Legacy / AUTO AIM
OFF / RED DOT ON / CROSSHAIR OFF after shutdown. Relaunch and slot-4 load retain
Legacy and the original auto-aim bit stays off (`scheme-restart-results.json`).
The new control-scheme help text was visually inspected and fits. Diagnostic
games are closed; source player save hashes/mtimes remain unchanged.

## Live-switch red-dot leak follow-up

The user reports RED DOT and AUTO AIM both OFF after returning to Modern,
but the red dot appears while firing in slot 4. Reproduced in composed output
at 120 Hz using a private slot-4 copy: Legacy-to-Modern leaves the native dot
and modern crosshair visible together. Fresh Modern launches hide the dot.

The replay workers are forked processes. Their process-local live settings
retain RED DOT ON after the parent confirms OFF. The menu now refreshes the
workers once after a confirmed scheme or red-dot change, after all preset
values have applied. Cancel and unchanged confirmation do not refresh them.
This uses the existing runtime API; no framework changes are required.

Build and native aiming tests pass. Identical private Legacy-to-Modern menu
traversal and firing at 120 Hz now show only the modern crosshair in composed
captures (worker-before-6.png versus worker-after-2/6/10.png under the private
D09B evidence directory). Manual RED DOT OFF in Legacy also refreshes workers
and records guarded native marker suppression. A separate repeated-load menu
input harness attempt did not complete; it is not counted as toggle evidence.
Diagnostic invulnerability was enabled in private RAM. Player slot-4 hashes
and mtimes are unchanged. Needs user playtest; no commit/push or closeout.

## Acceptance and closeout

User confirmation: "lock it in and close it out!! all working" (2026-10-28).
The final implementation and documentation are accepted for commit and push.
No remaining D09B work. Vanilla and player save/card contents are unchanged.
