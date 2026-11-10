# Time to Kill jobs - Controls

Job descriptions and work logs for D03-D07, D09, D10, D27 and D28: input,
camera, aiming and controller. Status and dependencies are on [the job
board](../MODERNIZATION_JOBS.md); the dated session log is in the
`log-YYYY-MM.md` files beside this one. Moved verbatim from the board on
2026-11-10 (D58), one section per job in ID order.

## Jobs

### D03 — Player, camera and aiming state research

Trace this game's player transform, facing, movement inputs, camera matrices, collision, weapon direction, animation states and overlay ownership. Document verified addresses, units, call sites and evidence separately from candidates. Build only the diagnostics needed to verify those relationships.

**Acceptance:** reproducible observations identify usable hooks for movement and camera, plus an explicit aiming research boundary. Hooks check the supported disc/code/overlay identity and fail safely. Do not patch generated C or assume a candidate camera function is proven.

### D04 — PC action input, rebinding and mouse capture

Add an action layer that supports independent movement and look, keyboard/mouse bindings, input devices and context-specific actions. Preserve the original controller translation in Vanilla. Handle focus loss, mouse capture/release and menu navigation; resolve binding conflicts visibly.

**Acceptance:** bindings persist; held inputs release on focus loss; mouse capture never traps the player; menus remain operable. Modern movement actions exist independently of the original turn-left/turn-right buttons. Update the manual with implemented controls only.

### D04A - Escape reliably frees the mouse for the pause menu

User report (2026-09-30): pressing Escape to open the pause menu should release the mouse so the cursor is visible, especially in windowed mode; it was hit and miss, sometimes the mouse stayed captured.

**Acceptance:** in Modernized, every Escape press during gameplay leaves the mouse free with a visible cursor while the pause menu is open; resuming recaptures automatically; F10, F7 and focus-loss behavior unchanged; Vanilla unaffected.

**Work log (2026-09-30, Needs playtest):** cause from code: Escape released capture and set the auto-recapture request, but recapture only waited 12 host frames for a fresh gameplay offer. Escape's Start reached the game only while the key was physically held, so a quick tap could miss the original pad poll; gameplay kept running, offers kept arriving and the mouse was grabbed again about 12 frames later. Change (local `recomp/src/ttk/pc_input.cpp`): Escape holds Start for a six-frame pulse so a tap reaches the pad poll, and a new Escape hold blocks automatic recapture until gameplay offers have stopped for the freshness window (the pause took hold), counted from the Escape press; host overlays clear it. If the pause somehow still does not open, the mouse stays free (F10 or Escape again). Escape releases are now logged as `Mouse released (Escape)`. Evidence: build OK, binary `10d36eb83fbca9892dd954359c14bfcf25ddcfb1b06a0287cecbec59f2f60168`; `ttk-input-test` PASS with the old Escape pad expectation updated to the pulse and a new case (Escape while offers continue stays released; offers stop, then resume recaptures). Not verified: live gameplay pause timing and cursor visibility in windowed mode (no game launched).

**Work log (2026-09-30, playtest fix, Needs playtest):** user: "when hitting esc again, it seems to leave modern controls and my mouse is not recaptured". Session log confirmed `Mouse released (Escape)` with no automatic recapture afterwards (F10 used). Cause: the resume Escape in the pause menu also set the hold, counted from that press; the game resumed within the freshness window, offers never gapped and the hold never cleared. Fix: only an Escape from running gameplay (captured, or offers still fresh) sets the hold; Escape in the pause menu only pulses Start. `ttk-input-test` PASS with a new pause-menu resume case. Game rebuilt after the session closed: binary `198673f5f5643aed5f25c543b79ec69ed0216d69e2f211a075394f3b86ded68e`.

**Work log (2026-09-30, Done):** user playtest on binary `198673f5...ded68e`: "perfect, accept". Escape opens the pause menu with the cursor free and Escape again resumes with the mouse recaptured.

### D04B - Robust Modern controls and settings-only scheme selection

**Done (user-accepted 2026-10-23).** After loading a game and
pressing Escape shortly afterward, Duke can leave the Modern control scheme.
This is a new user-reported control-recovery problem, separate from D19Q2's
accepted paused-save investigation. The candidate reproduces a persistent
Escape capture hold and fixes its recovery; see note 205 below. D19Q2's
successful ordinary load/resume probes did not disprove this timing failure.

**Required behavior:** Modern is the default control scheme. Within a
Modernized session, Legacy controls may be selected only by an explicit,
persistent setting in the settings panel. F10 must not switch Modern/Legacy
or be required to recover Modern controls. Mouse capture/release is a
menu/focus concern, not permission to change the selected control scheme.
The user explicitly rejects silent fallback to Legacy controls.

**Acceptance:**

- Reproduce and fix load followed quickly by Escape, including repeated
  Escape presses around load completion and pause/resume. Test SAVE GAME,
  F2/F3, F6/F9, F7 and relevant legacy/debug saves using private copies.
- With Modern selected, movement, independent camera and weapon aiming remain
  Modern across loads, pause/resume, menus, console, focus loss/return, death,
  level/overlay changes and traversal interruptions. No implicit tank/Legacy
  fallback when capture or a control lease is lost. Preserve verified hook
  identity checks; robustness must not mean running hooks on unknown code.
- Settings are the sole in-game authority for Modern/Legacy selection;
  Modern is the default. An explicit Legacy setting persists and can be
  changed back there. Savestates and transient input state cannot silently
  override that preference.
- Remove F10's Modern/Legacy switching behavior. Normal play and returning
  from menus recover automatically without F10 or another recovery shortcut.
  Audit all other shortcuts and fallback paths for implicit scheme changes.
- Escape reliably pauses with a free cursor; resume restores Modern controls
  and mouse capture. Focus loss safely releases held inputs without changing
  the scheme. Menus remain usable and the pointer is never trapped.
- Test rapid transition sequences at supported frame rates, both camera
  views, and a representative set of levels/traversal states. Include
  regression tests for the confirmed timing failure and manual playtest.
- Preserve explicit `run.py --vanilla` behavior. Update the manual and
  settings/help text to describe the implemented scheme and capture behavior.

Candidate and evidence: [note 205](documentation/205-d04b-control-recovery.md). The older D04A acceptance that left F10
unchanged is superseded by this job's new requested behavior when implemented.

### D05 — Camera-relative WASD movement

Implement forward/backward and strafing relative to camera yaw, preserving the game's collision, movement speed and relevant animation rules. Separate actor facing from movement direction where aiming requires it. Define diagonal normalization and keyboard walk/run behavior.

**Acceptance:** W/S/A/D move as expected while the camera rotates, diagonal movement gives no accidental speed advantage, aiming does not force tank turning, and walls, slopes and steps remain reliable. Vanilla follows its original path.

### D06 — Independent third-person mouse camera

Add continuous yaw/pitch, sensitivity and inversion with a usable pitch range. Keep camera orientation independent of animation-driven facing. Decide how camera recentering and the game's original camera transitions cooperate.

**Acceptance:** looking while standing, moving and firing works at different host frame rates; pause/focus transitions do not produce a camera jump; input does not alter simulation speed. Original camera behavior remains selectable.

### D07 — Modern weapon aiming and crosshair

Connect view direction to actual shots and projectiles. Account for muzzle offset, close walls, third-person parallax and the original auto-aim. Make auto-aim behavior explicit and configurable where appropriate. Cover the available weapon families rather than assuming every weapon uses the same path.

**Acceptance:** the reticle describes the intended hit direction, actual impacts agree within documented weapon behavior, and shots do not pass through near cover because the camera sees around it. Log which weapons and aiming states were tested.

### D07A — View-aligned Duke facing and weapon presentation

**Done — user accepted remaining playtest, 2026-09-28.** Ongoing campaign play
covers the previously open pose/interior checks. Keep documented weapon-family
and cover limits; do not reopen without a new report.

**Implemented 2026-09-27.** In supported Modernized
view-aim states, Duke follows camera yaw while camera-relative WASD remains
independent. Supported armed poses also receive view-aligned upper-body aiming;
original recoil, draw/holster and unsupported poses retain their original paths.
Original camera/weapon aiming and right-click precision aim remain available.
The entrance wall-facing movement lock and wall-bump orbit were corrected.

Evidence: [facing and presentation contracts](documentation/22-view-facing.md)
and [follow-up checks](documentation/24-d07-controls-and-aim-options.md).
Native guard/fallback checks and bounded gameplay routes support implementation;
the user's improved-aiming and later controls feedback supports feel. Remaining
pose/interior coverage was accepted in ongoing play 2026-09-28. D08/D08C
acceptance remains valid independently.

**Acceptance:** standing, moving and firing respond coherently to mouse yaw;
Duke/body/gun presentation agrees with supported aiming within documented animation
limits; no tank-turn coupling or regression to accepted movement/camera behavior.
Verify capture/focus, draw/holster and original precision aim. Retain D07's explicit
weapon, actor-hit, reticle and cover coverage gaps; do not claim these solved without
evidence. Use isolated cards and distinguish automated checks from user playtesting.

**Feedback-pass acceptance:** reproduce and correct wall-facing movement lock;
retain mouse orbit through the verified wall-bump states; test turning and moving
away near the strip-club entrance, with interior/corner user playtesting recorded
separately. Match the chosen EDuke32-style speed policy: walk on launch, Shift
reverses speed, Caps Lock toggles autorun. Preserve saved speed bindings and
original input paths. Clearly distinguish original autoaim from the view reticle.

### D07B — Optional assisted view aiming and display controls

**Done — user accepted remaining playtest, 2026-09-28.** Assistance and display
choices were accepted in ongoing play. Half-size EDuke crosshair and **I**
toggle were already accepted 2026-09-28. D19 still owns in-game menu exposure.

**Implemented 2026-09-27.** Saved launcher preferences
independently control aim assistance (`off` or experimental `original-lock`), the
modern crosshair and the original red autoaim dot. Marker visibility does not
change assistance. D19 still owns in-game menu exposure.

For supported shot paths, optional assistance requires an original acquired live
target within six degrees of the view, visible as the same actor from both camera
and muzzle. Missing acquisition or failed cover checks retain free view aiming.
Original aiming remains selectable; the red dot is an autoaim indicator, not a
laser sight. See [current contracts](documentation/24-d07-controls-and-aim-options.md).

Evidence includes native target/cover/marker checks, bounded original-actor contact
and software/OpenGL marker routes. Later assistance-off close-combat work verified
two original enemy deaths and preserved E interaction-only behavior; see
[close-combat evidence](documentation/28-d08-close-combat.md). This is bounded
implementation evidence, not all-weapon or campaign acceptance.

**Remaining acceptance:** human assessment of assistance and independent display
choices, moving targets, near cover and supported weapon families. Record tested
option combinations and unsupported paths explicitly. Preserve free view aiming,
original options and per-profile preferences. Remaining assistance/display
playtest was accepted 2026-09-28; the half-size EDuke crosshair and **I** toggle
were already accepted that morning.

### D07C — Unified Duke3D-style aiming, facing and projectile coverage

**Done — bounded user acceptance (2026-09-27).** The user confirms aiming is
massively improved, RPG shots follow the modern crosshair at the intended pitch,
and their tested weapons align correctly through firing/aim modes. Preserve prior
automated evidence and campaign/platform limits; this is not every-weapon/era
human verification. See [playtest follow-up](documentation/41-playtest-follow-up-plan.md).

Original scope: See [reproduction and verification](documentation/40-feedback-implementation.md). Duke3D is the interaction baseline for
Modernized: mouse yaw/pitch must determine intended aim, actual weapon travel and
coherent Duke/body/gun presentation. Preserve third-person camera parallax, weapon
spread/arcs and physical muzzle obstruction. The user reports that holding aim
and firing around 360 degrees lets shots go behind a forward-facing Duke, and RPG
shots without held aim travel dead horizontal despite intended vertical aim.
They question whether the modern crosshair or original red dot actually owns aim.
The report initially says left-click, then right-click; reproduce both fire-only
and held right-click/original-aim paths with recorded bindings/settings.

Audit shot/facing/reticle ownership before choosing whether to merge, suppress or
replace the original marker. A cosmetic reticle change alone does not satisfy this
job. Modernized should have one coherent aiming policy in ordinary and held-aim
states, while Vanilla and an explicit original-aim option retain original behavior.
Treat original autoaim as an explicit assistance option, not a hidden override of
free aim. Do not extend a weapon whitelist without proving each projectile path.

**Acceptance:** record weapon IDs, settings, body/gun yaw, camera yaw/pitch,
projectile launch vectors and actual impact positions. Test stationary/moving,
360-degree yaw, high/low pitch, held/released aim, fire, holster/draw, capture/focus,
near walls/actors and camera/muzzle parallax. At minimum pistol, shotgun/rapid-fire
and RPG, plus each materially different available projectile family (including
throwables with documented arcs). RPG must visibly follow pitched aim; shots
behind Duke must have coherent turn/weapon presentation. Compare assistance off/on
and original dot visibility independently. Preserve movement/traversal. Reconcile
D07A/D07B evidence without erasing their earlier bounded acceptance. See
[next-session brief](documentation/39-next-iteration-brief.md).

### D07D - Hide the original red autoaim dot by default in Modernized

**Done (user-accepted, 2026-09-30).** Previously Needs playtest: profile schema 16 defaults `red_dot` off and
migrates older Modernized profiles off once; the marker hook now owns the dot
at its enqueue alone, so it hides on every map and state that draws it. See
the 2026-09-30 D07D work log.

**User request (2026-09-30):** with modern controls the original red autoaim
dot is not needed; the player relies on the modern crosshair. Showing both is
confusing, "like double crosshairs".

Context: D07B already has an independent `red_dot` preference
(`--red-dot off`, settings item 9), but `DEFAULT_CONTROLS` in
`recomp/tools/local/player_profiles.py` sets `red_dot: True`, so Modernized
shows it by default. Suppression lives in `marker_hook` in
`recomp/src/ttk/weapon_aim.cpp` (`DNTTK_RED_DOT=0`), and it steps aside when
`g_precise_mode`, `g_ls_mode` or `g_psx_call_bail` is set, so the dot can
still appear during right-mouse precision aim and in some other states.

**Acceptance (user, 2026-09-30: "in modernized, it should be off by
default"):** the red dot is off by default in Modernized, regardless of the
weapon aiming or camera option, and only the modern crosshair shows, in third
and first person, for every supported weapon and during held aim, jetpack
flight, swimming and scripted cameras. A profile migration turns it off for
existing Modernized profiles that still hold the old default. Record how an
explicit user choice is told apart from that default. The setting stays
available for anyone who wants the dot back (note in the manual that
`original` weapon aiming hides the modern crosshair, so turning the dot back
on is the way to get a marker there). Hiding the dot must not change aim
assistance, autoaim target acquisition or damage. Vanilla is unchanged and
keeps the dot. Update the manual examples that pass `--red-dot off`.

### D07E - Stale-lease shots fall back to the original auto-aim

**Done (user-accepted 2026-10-19: "plays solidly ... both work as intended beautifully"; [note 154](documentation/154-d07e-frame-lease.md)).** Found in D22D (2026-10-19, offscreen lab). The shot adapter only
adapts a shot while `shot_ready()` holds. `lease_ready` needs the camera hook
within 4 host input frames (`f.sequence - camera_frame <= 4`); when that
lapses (seen with Duke idle, animation 63, mode 0, under the load of an
upgraded energy stream: 11 of 28 shots), the shot fires through the original
path with the original auto-aim and, for the energy weapon, the original lock.
This applies to every weapon. Also seen: Modernized with `original` weapon
aiming never made an original energy lock, because Duke's aim vector
`player+0x144` still pointed the old way after modern movement turned him;
check whether guns in that mode fire along it too. See
[note 152](documentation/152-d22d-energy-beam.md#limitations-and-findings).

**Confirmed 2026-10-19 (real GPU offscreen, the player's settings copied:
120 Hz, fast timing, 150% CPU, 4x; lab `recomp/analysis/d07e-lease/`).**
Host input frames advance once per emulated vblank (60 Hz). The game
normally updates the camera every 3 (20 fps): idle, walking and the pistol
missed nothing (87 shots; the one miss was a run-collision bump, animation
95, where the movement state is camera-only). With the upgraded energy weapon
the game slows to 4.6-4.9 vblanks per camera update (about 12 fps) and 34 of
107 shots missed the adapter, all `gap=5` (21 of 33 while standing). So any
slowdown below 15 game fps drops view aiming to the original aim, and the
same fixed 4-vblank window gates movement (`lease_ready`, the jump camera
and fresh-movement checks, `ground_frame`). New diagnostics: aim JSON
`lease_misses` / `last_lease_miss`, `DNTTK_AIM_TRACE=1` prints
`ttk-aim-lease-miss`, and `DNTTK_TEST_INPUT` accepts `cheat=<word>` and
`act=<action>`.

**Fixed 2026-10-19.** The lease window is one game frame: the slowest of the
last four camera intervals plus one, 4 to 10 vblanks (4 at the normal 20 fps);
energy weapon 0 of 105 misses, pistol 0 of 88. `original` weapon aiming: modern
movement never turned Duke, so the original shot followed the heading the save
gave him; now his heading follows the camera as with `view` (user choice), and
the original shot, auto-aim and energy lock follow it (5 original locks).

**Acceptance:** cause of the stale lease measured; Modernized view-aimed
shots never silently use the original auto-aim in a supported state; the
`original` weapon-aim profile aims and locks like the original; Vanilla
unchanged; the user confirms.

### D07F - Airborne shots ignore the crosshair

**Done (user-accepted 2026-10-20, [note 160](documentation/160-d07f-airborne-shots.md)):
airborne shots use the view aim on the camera-only lease.** Opened
2026-10-20, found by the user while testing D19G. Firing
while jumping does not follow the crosshair: the shot goes straight ahead of
Duke. Repro (user): switch to the rocket launcher, point at the ground, jump
and fire looking down, jump and fire again looking down, then the same with
the shotgun: the blast marks land ahead of Duke, not where the crosshair
was. Likely the view-aim path (D07C) does not own shots in the airborne
states the manual jump (D08Z) uses; verify before fixing.

**Acceptance:** jumping (assisted and manual jump, first and third person)
and firing at the ground or up hits where the crosshair points, for the RPG,
shotgun and the other view-aimed weapons; grounded aim unchanged; Vanilla
unchanged.

### D07G - Readable crosshair scaling in windowed mode

**Status: Done (user-accepted 2026-10-26).**

User report, 2026-10-26: the crosshair looks "really, really tiny" in windowed
mode. Improve its scaling/readability across window sizes. Exact window size,
renderer, aspect ratio and display scale were not supplied; no cause established.

Reference completed **D08A22**, [switcher/card scaling](#d08a22---switcher-strips-and-mission-card-scale-with-the-window-like-the-native-hud):
that repair scaled host UI from the fitted game picture instead of integer-floored
window height, with fractional scale, nearest sampling and a minimum readable size.
Its picture-height / 480 formula was specific to strip art authored at 2x; do not
assume that exact factor suits the crosshair. Its OpenGL/Software and 4:3/16:9
window-size matrix is useful prior evidence. Also consult D07B's crosshair display
contract and D24A's later original TTK art work before changing the visual asset.

When explicitly selected, inspect the current crosshair drawing/scaling path and
compare it with D08A22. Define a readable size relative to the fitted game picture,
including small windows, resizing, fullscreen and letterbox/pillarbox layouts.
Preserve crisp artwork, accurate screen/aim centering, crosshair visibility settings,
weapon aiming and Vanilla behavior. Do not merely enlarge every fullscreen size.

Acceptance: the crosshair remains readily visible in small/windowed play, scales
consistently without abrupt size jumps, stays centered within the game picture,
and retains acceptable fullscreen proportions. Verify representative sizes and
aspect ratios on supported renderer paths; user playtest decides readability.

User accepted: "perfect, lock it in and close it out". Implementation and documentation
commit/push authorized. [Note 276](documentation/276-d07g-crosshair-scaling.md) records
the completed verification and remaining platform coverage limits.

### D07H - Red dot off but drawn on the 180 Hz redraws

**Status: Done (user-accepted 2026-11-10: "it works"; cause CONFIRMED live and fixed, see the work log below).** Earlier: Monitoring (diagnostic build; cause not reproduced). Kept open but not
to be actively worked on until the user reports the dot again (user, 2026-11-10).
The dot had not been seen since early development before this report. When it
returns, read the `[TTK red dot]` line in that session's log and fix by its reason;
if it stays away through normal play, close as not reproduced and keep the silent
report.

User report, 2026-11-10 (live, video `research/screencaps/Video_2026-11-10_00-38-20.mp4`,
local): with RED DOT off the original aim dot reappears above and left of the
modern crosshair. Live session (profile Modernized, AUTO AIM EDUKE32, WEAPON AIM
view, GEOMETRY PRECISION corrected, 180 Hz AUTO): weapon 8 (key 5), on a level-1
rooftop near (40727, -3580, -8171) after jetpack, push grabs and four pause/resume
cycles. Debug port: the marker hook hid one dot per game frame (63 in 64 frames)
while 380 of 380 presents in the same window were redraw-worker images, and the
dot was on screen in every desktop grab. Worker spawns 15 (3 + four re-forks).

Not established: why the redraw workers do not hide it in that session. Ruled out
offscreen at 180 Hz (private copy of the session's slot 6, the player's controls
and precision settings, weapon 8 idle and firing): workers hide every dot and
re-run the code identity check per redraw (a frozen fork-time verdict was the first
hypothesis and is refuted). Scripted pause, cheats, view toggles and walking did
not re-fork workers there.

Acceptance: with RED DOT off in Modernized, the original dot never shows in game
or redraw images at any frame rate, for every weapon and state; RED DOT on and
Vanilla unchanged; hiding must not change aim assistance or damage.

**Work log (2026-11-10, recurrence, Needs playtest).** The user saw the dot
again and saved the session to UI slot 6. Session log: `[TTK red dot] off but
drawn in redraw <pid>: ... state 1 ...` from each of the three redraw
workers, none from the game process; the game had hidden 58 dots. `state`
means one of `g_ls_mode`, `g_psx_call_bail` or `busy` was set. sudo gdb
(batch, read only) on the live worker: `g_ls_mode` 0, `g_psx_call_bail` 0,
`ttk::busy` 1; the game process: all 0. Cause (CONFIRMED): `busy` is set
across the aim code's calls into game routines; a worker forked inside that
window (`render_worker.c` `spawn`, from the worker maintenance that runs in
replay publish and submit) inherits `busy` 1 and never clears it, so its
marker hook reports `state` and draws every dot. That fits the first report
(the game hid each dot, every redraw image showed it, worker re-forks
before it). Fix: `weapon_aim.cpp` registers a `pthread_atfork` child handler
that clears `busy` (a forked worker never resumes the interrupted aim work);
not on Windows (no fork). Verified on a private copy of UI slot 6 at 180 Hz:
gdb set `busy` 1 in the game, called `psx_mod_replay_workers_refresh`, set it
back to 0; every new worker read `busy` 0 through `/proc/<pid>/mem` (no stop,
so no deadline kill), no `[TTK red dot]` report, no dot in the shots; build
OK, `ttk-input-test` and `ttk-near-test` pass. The diagnostic stays. Not
verified: the user's own play (needs a relaunch: the running game uses the
old build).

### D09 — Modern controller support

**Status: Done (user-accepted 2026-10-27).** User reports "controller is working solidly" and explicitly authorizes lock-in and closeout. Implementation, tests and documentation commit/push authorized. [Note 289](documentation/289-d09-modern-controller.md) retains bounded test evidence; this acceptance does not establish a diagnosis of the historical two-pad selection report.

Provide left-stick movement, right-stick look, configurable sensitivity/inversion, dead zones and sensible action bindings using the modern action layer. Support switching input devices and disconnect/reconnect.

**D34B carryover (2026-10-27):** user reports being stuck at Red Duke /
Blue Duke character selection. D34B was closed for now at their request with
no further code changes, not accepted as a working physical-controller route.
Diagnose two physical pads and per-seat menu/character confirmation, then repeat
the complete split-screen match and real-display checks from D34B. Existing
virtual SDL results do not establish hardware acceptance. See
[note 288](documentation/288-d34b-classic-split-screen.md). D09 remained Todo at that closeout; the user subsequently selected it.

**Acceptance:** stick movement is predictable, no drift at rest on the tested controller, menus and gameplay agree on bindings, and disconnects do not leave held actions. Record tested hardware and limitations.

### D09A - Separate keyboard and gamepad seats for local two-player

**Status: Done (user-accepted 2026-10-27). Dependencies: D09
(Done), D34B implementation (retained; physical two-pad acceptance remains bounded).**
Register keyboard and gamepad as independent player controllers for local
split-screen: one player uses the keyboard, the other a physical gamepad.
Investigate existing seat assignment and keyboard merge before implementation.
Provide clear assignment/confirmation so keyboard input can select and confirm
one character while the gamepad independently controls the other.

Acceptance: both seats can select, confirm and start a match; keyboard movement,
fire and actions affect only its assigned player, and gamepad input only the
other. Verify pause/resume, results/exit, save/load and pad disconnect/reconnect
without cross-talk or stuck actions. Preserve accepted single-player mixed
keyboard/mouse/gamepad use, existing two-gamepad play and Vanilla. Keep original
split-screen gameplay rules; modern per-player cameras and networking are not
part of this job. Record hardware, routing and real-display test evidence.

Implementation and bounded evidence: [note 290](documentation/290-d09a-keyboard-gamepad-seats.md).
OPTIONS > CONTROLS > TWO PLAYER INPUT selects KEYS AND PAD. User confirms the
controller/keyboard arrangement "100% works" and two-player works as a
foundational first step. D09A is accepted; modern gameplay/presentation is D34C.
This is user-reported acceptance, not newly observed exhaustive hardware tests.
User explicitly authorizes implementation/documentation commit and push closeout.

### D09B - Restore classic gamepad auto-aim and red dot

**Status: Done (2026-10-28). Related: D09/D09A, D07D/E.**
User reports Legacy controls with AUTO AIM ON and RED DOT ON do not restore the
original behavior. Investigate auto-aim and the classic red dot separately:
trace saved settings, live values and original gameplay/marker systems, and
identify any input, aim or rendering overrides. Establish original behavior
from evidence; neither a modern aim-assist substitute nor the modern crosshair
is an assumed equivalent.
First report each feature's blocker, whether direct restoration is feasible,
and limitations/alternatives. Then implement supported restoration where
practical, scoped to relevant control/aim modes. Preserve accepted modern mouse
aiming, first-person behavior and Vanilla. Prefer original systems.
Acceptance requires an actual gamepad in Legacy controls: test auto-aim and
red dot independently and together, including acquiring/changing targets and
firing; demonstrate observable settings effects and correct disabling. Record
physical-device evidence separately from injected input or source inspection.
Accepted result: scheme transitions apply Legacy auto-aim/red-dot ON and
crosshair OFF, or the reverse for Modern; manual overrides remain available.
RED DOT ON suppresses the modern crosshair. Render workers refresh on confirmed
scheme/marker changes, fixing the stale red dot after switching to Modern.
The user confirms "all working" and authorizes closeout. This is user-reported
playtest acceptance; automated evidence remains separately labeled in
[note 294](documentation/294-d09b-classic-aim.md).

Candidate findings: private single-player Legacy tests reproduce native auto-aim
and the original red marker. All four preference combinations act independently.
The menu incorrectly claimed RED DOT needed AUTO AIM ON; that dependency and
its description are corrected. User now reports auto-aim works and identifies
UI slot 4 for the missing dot. RED DOT ON now suppresses the modern crosshair
without changing aim assistance; RED DOT OFF restores its prior preference. No gameplay restoration is justified by these
results. The reported failing context and physical-controller acceptance remain
open; this is not a diagnosis of the user's report. [Note 294](documentation/294-d09b-classic-aim.md).

### D09C - Controller rumble

**Status: Todo. Dependencies: D09, D09A.** User (2026-11-04, cancelling
D12E2): "stand rumble up in the backlog as a standalone for later on".

Known from earlier notes (not yet verified on hardware): the game drives
DualShock vibration through `0x8001e840` (wall bump, note 131; jump-wall
bounce, note 134; damage types, note 370), and the framework already relays
the guest pad's small/large motor values to `SDL_GameControllerRumble`
(`update_controller_rumble`, seat routing per D09A note 290). Note 381's
"no rumble in the host" meant no host-originated rumble. Physical rumble has
never been tested on this build.

Scope: verify the original vibration reaches a real pad (wall bump, damage,
explosions) with the game's vibration setting; fix the relay if it does not;
then propose Modernized rumble for events the original leaves silent (kick
hits in first and third person, firing, landings), with an on/off and
strength setting and nothing on a whiff. Two-player routing per seat. Vanilla
keeps the original vibration only.

Acceptance: the user feels the original events on their pad; Modernized
additions judged with the user; keyboard-only play and Vanilla unchanged.

### D10 — Third-person camera polish

**Done - user accepted 2026-09-29** ("it's done, fully accepted"). Distance
portion accepted 2026-09-28.
See [bounded implementation and evidence](documentation/33-controls-shortcuts.md).
**2026-09-29 options pass (accepted):** V recenter, H shoulder cycle
(off by default), Alt+wheel distance and shoulder side saved in the profile.
See [D10 camera polish](documentation/60-d10-camera-polish.md).

Tune follow distance, shoulder offset, recenter behavior and camera collision for Time to Kill's rooms and corridors. Offer practical options without changing the core game rules.

User-selected input: **Alt + wheel**, up closer/down farther, adjusts third-person
distance without changing FOV. Wheel alone cycles weapons; Ctrl belongs to held
crouch and right-click keeps precision aim. Clamp/smooth distance, preserve camera
collision, and keep preferred distance separate from temporary wall compression.
See [the control plan](documentation/32-eduke32-weapon-item-plan.md).

**Acceptance:** doors, corners, tight rooms and vertical traversal do not leave the camera inside geometry or hide necessary action; smoothing does not introduce excessive aiming delay. Record representative playtest locations.

### D10A — Rapid mouse turning and Shift-running investigation

**Done — bounded user acceptance (2026-09-27).** Player no longer encounters the
reported turning difficulty on the street/club route; aiming remains praised.
Sixty-four isolated sweeps never reproduced lost yaw; no speculative camera change
was shipped. Physical high-rate compositor edge cases remain uninstrumented but
are not current blockers. See [iteration 47](documentation/47-playtest-iteration.md).

**Needs playtest; 64 isolated street/club sweeps did not reproduce lost yaw.** See [current evidence](documentation/46-turning-audio-inventory-progress.md). Quick mouse sweeps felt unresponsive in the starting
street and especially running with Shift inside the strip club. Shift involvement
is unproven. Reproduce contrasting motion/state/location cases and trace input,
camera ownership/collision and timing before claiming a cause. Preserve accepted
aiming and Vanilla. **Acceptance:** documented failure/comparison and a bounded
verified repair where supported, with responsive turning and original camera
constraints intact. Full matrix: [current brief](documentation/45-playtest-turning-audio-follow-up.md).

### D10B - Tight-space third-person camera: translucent Duke and see-through doors

**2026-10-24 follow-up - Superseded by D10B1 (not accepted):** user requested later, gentler proximity
fade while retaining the accepted spatial effect. Tune only body coverage;
camera collision and shoulder behavior remain as accepted.

**Done - user accepted 2026-10-24:** "fully accept!! lock it in and close it out".
Accepted compression, shoulder centering/recovery, Duke-only proximity fade and
solid near scenery under the agreed policy below. Accepted build `4396ea72a48d95b6de13999501ff8b4f53f46515533b8aa7d7a6ca42d54429eb`.
Implementation, tests, framework patch and documentation included in authorized
closeout. See [note 221](documentation/221-d10b-tight-camera.md) for evidence and
scope limits. No further gameplay changes or next job started.

**Work log (2026-10-24, Needs playtest):** implemented collision compression,
shoulder centering, held/eased recovery and Duke-only proximity coverage on
OpenGL, with solid near scenery. Build, controls/input/aim/near-clipping and
replay codec tests pass. Private apartment routes pass at 60/120 FPS, including
view and shoulder switching, firing and save/load. Framework patch exported
and verified against the clean pinned dependency. See [note 221](documentation/221-d10b-tight-camera.md)
for captures, candidate hash and limits. User camera-feel and campaign playtest
remain; no commit, push or acceptance closeout.

**2026-10-24 - selected implementation:** collision-aware third-person compression,
shoulder centering in confined spaces, smooth return to the saved framing,
and a deliberate Duke proximity fade only when needed. Doors/walls remain
solid; no automatic perspective switch or aiming delay. User delegated design
and implementation. Candidate requires private route verification and user playtest.

The agreed acceptance supersedes the absolute head/body visibility requirement
below: preserve a clear view of the action, keeping Duke opaque where practical
and fading him smoothly only when proximity obstructs the view. Show before/after
sewer, door and tight-corner approaches, backing in/out, camera/aim regressions
and unchanged Vanilla.

**Correction (2026-09-30):** the missing head is not a camera effect. It was a
first-person hide flag stuck in the slot-12 savestate; see D11C. This job
keeps the close-camera transparency and see-through doors.

**User report (2026-09-30, D13 playtest):** in the narrow sewer at the slot-12
save, Duke is drawn semi-transparent with no head. Separately, the camera can
see through doors that vanish up close, like the apartment closet door did in
first person (D11B). Checked on a private copy of slot 12: identical at 1x and
4x, and also with the original camera, so it is the game's own close-camera
behavior. The camera is pinned right behind Duke, his head falls inside the
near cutoff, and the original close-camera transparency applies. Polygon
culling happens on original coordinates before the renderer scales, so
internal resolution cannot change which door polygons are drawn.

**Acceptance:** in tight spaces the Modernized third-person camera keeps
Duke's head and body visible (for example a raised or shoulder-offset
framing, or a smooth approach to first-person framing) with no pops. Doors
and walls near the camera stay solid or fade deliberately, never drop out.
Captures of the slot-12 sewer and a door approach, before and after.
Vanilla is unchanged.

### D10B1 - Roll back D10B behavior

**Done - accepted 2026-10-24:** "lock it in and close it out!"
Implementation, tests and documentation included in authorized closeout.
Build/controls/clipping checks passed; broad campaign coverage is not claimed.

User compared slot 6 in both modes:
native box transparency preserves third-person combat visibility; D10B's
opaque boxes make the room less pleasant to play. User requested a separately
logged rollback job before implementation.

**Scope:** disable D10B by default using its existing complete behavior gate.
Retain explicit developer opt-in for comparison, not a saved player preference.
Restore native third-person Duke/scenery fading and camera obstruction response.
Preserve pre-D10B work and subsequent jobs. Retain inert shared renderer/replay
support rather than reversing unrelated framework changes.

**Acceptance:** ordinary launch follows the existing `DNTTK_TIGHT_CAMERA=0`
behavior without requiring that environment variable. Native fade state is not
cleared; D10B camera minimum/follow overrides are inactive. First-person and
Vanilla guards continue passing. Build and regression tests pass; user verifies
slot 6 and cramped routes. No commit/push or acceptance closeout without request.
The earlier unaccepted fade-curve experiment is superseded by this rollback.

### D10C - Duke 3D look keys

**Done (user-accepted 2026-10-20: "awesome!! ... lock this current one in, and close it out"; [note 161](documentation/161-d10c-look-keys.md)).** Executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19` is the regression baseline. Follow-up D10D (arrow-key turning). Opened during D19G. As in Duke 3D:
**Page Up / Page Down** look up / down while held and recenter the camera
when released; **Home / End** lock the look up / down. New bindable actions
in KEY BINDINGS (CAMERA group) and `run.py --bind`; decide with the user
how they relate to RECENTER CAMERA (V) and mouse look.

**Acceptance:** the keys behave as described in third and first person,
show in KEY BINDINGS and persist; mouse look and V unchanged; Vanilla
unchanged.

**Work log (2026-10-20, Needs playtest; [note 161](documentation/161-d10c-look-keys.md)).**
Reference: the user's EDuke32 capture (Page Up / Down look while held and ease
back in about 0.3 s; Home / End tilt at about half speed and stay; a Page Up /
Down tap recenters). Four held host actions `look_up` PageUp, `look_down`
PageDown, `lock_look_up` Home, `lock_look_down` End (appended to the action
list; no pad bit, never commands; Look presses counted so a tap between
camera updates counts). The camera (`key_look` in `orbit_begin`) moves the
pitch at 150 / 80 degrees per second, eases Look back to the rest pitch (V's
angle and rate), keeps Lock; mouse look stays on top and stops the ease back.
Timed on the presenter clock, so the late camera tilts smoothly at high
refresh. KEY BINDINGS CAMERA rows LOOK UP, LOOK DOWN, LOCK LOOK UP, LOCK LOOK
DOWN; launcher schema 34 adds the keys to older profiles where free.
Executable `ee9d3adee53e952bce7acb0c9bc36840cf44fe0aba37b3e11a56d96038efaa19`. Lab (offscreen, level 1
slot 9, third and first person): hold, ease back, lock while walking, 0- and
2-frame taps, mouse after a lock, V all as described; screenshots confirm
up = sky, down = feet; KEY BINDINGS rows, swap and saved tokens checked.
Native, Python 155 OK. Not verified: feel on the real display. Open question:
center is the rest pitch (7.7 degrees down here, as V), not level.

### D10D - Keyboard turning (arrow keys)

**Done (user-accepted 2026-10-20: "perfect, lock it in, close it out"; [note 162](documentation/162-d10d-keyboard-turning.md)).** Executable `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892` is the regression baseline. Opened 2026-10-20, user request after D10C. As in Duke 3D, the
Left / Right arrows turn Duke and the view left / right in Modernized (the
classic tank-control turn, added to modern controls; WASD still moves and
strafes, the mouse still looks). Turning eases in like Duke 3D keyboard
turning and is smooth at high refresh (late camera, as D10C's pitch). New
bindable actions (TURN LEFT / TURN RIGHT, CAMERA or MOVEMENT group, `--bind`).
Today the arrows are fixed keys that reach the original pad (the D-pad,
menus, D08Q6 jetpack flight movement); decide with the user where turning
replaces that and keep menus and the jetpack scheme working.

**Acceptance:** the arrows turn in third and first person, on foot and in
the other supported Modernized states, without changing movement or mouse
look; rebindable and saved; menus and jetpack flight still work; Vanilla
unchanged.

**Work log (2026-10-20, Needs playtest; [note 162](documentation/162-d10d-keyboard-turning.md)).**
User decision: "a simple turn left, turn right, for the arrow keys ... global
... it doesnt affect the wasd keys"; Duke 3D rates. Two held host actions
`turn_left` Left, `turn_right` Right (appended; no pad bit, never commands).
`key_turn` in `orbit_begin` turns the camera yaw on the presenter clock with
Duke 3D's `getinput` rates (5 build units per 30 Hz tic for 1/8 s, then 15,
doubled running: about 26, 79 and 158 degrees/s); `late_view` uses the same
model, so turning is smooth at high refresh. Mouse adds on top, a turn key
stops a V swing, pitch untouched; Duke follows the view as with the mouse.
Captured, Left / Right never reach the original D-pad (no tank-turn anims
fighting the camera) and jetpack flight no longer strafes on them (Up / Down
still fly forward / back); without the modern camera (tank fallback) the turn
keys feed the original D-pad turn. Menus unchanged. KEY BINDINGS CAMERA rows
TURN LEFT / TURN RIGHT; launcher schema 35 adds the keys where free.
Executable `02150d53b5bdaf636ce7fb5abfe8b311d7cb61f75e75e347bac2a2e511da2892`. Lab (offscreen, level 1 slot 9): walk and run rates,
turning while walking / strafing, with the mouse, with Page Up, after V, in
third and first person and in jetpack flight (no strafe drift); KEY BINDINGS
rows and a swap with QUICK KICK saved; the player's profile (a copy)
migrates to 35. Native, Python 157 OK. Not verified: feel on the real
display, scripted-camera fallback states in play.

### D10E - Optional held middle-mouse third-person free look

**Status: Done (user-accepted; closeout, commit and push authorized 2026-10-27). Dependencies: D04, D06, D07, D10 (accepted/Done).**
Candidate and bounded verification: [note 283](documentation/283-d10e-free-look.md).
Requested 2026-10-27. Inspired by @vakho10's
[GTA San Andreas-style camera suggestion, issue #1](https://github.com/MusicMonsterMod/duke-nukem-time-to-kill-recomp/issues/1).
Credit that input while keeping this narrower request distinct: hold middle
mouse to inspect Duke/environment in third person; orbit must not turn Duke
with the camera. Release must smoothly restore existing third-person control
without a camera snap or an unwanted sudden body turn. Preserve ordinary
unheld behavior, first person and Vanilla. This does not select the issue's
separate head tracking, right-click aim or replacement control scheme.

Initial source inspection: `recomp/src/ttk/pc_input.cpp` stores mouse buttons
and accumulated look deltas; `input_bindings.def` has no default middle-button
action (Mouse3 maps to SDL button 2). `modern_controls.cpp` owns orbit/recenter,
uses camera yaw for movement, and calls `face_view()` through multiple state
paths. `weapon_aim.cpp` reads the camera forward vector for shots. Simply
suppressing one facing call would leave movement/aim coupled to the inspection
camera. Investigate a separate inspection orbit and retained gameplay heading,
with one explicit entry/exit owner; this is a proposal, not implemented behavior.

When selected, inspect current code again and decide held movement and firing
policy together: retain pre-look movement/aim heading, cancel free look on fire,
or another coherent policy. Report the choice and any limitation; do not silently
fire behind Duke or make release rotate him. Check custom middle-button binding
conflicts, camera collision, shoulder/recenter, view switches, capture loss,
pause/load and special-action camera ownership.

Acceptance: optional hold-to-orbit works at idle and through walk/run/strafe;
no camera-induced body turn while held; smooth enter/release including large
orbits; movement, aim, firing and their combination remain coherent. Test
repeated presses, 60/120 presentation, first-person/unheld/Vanilla regression,
and explicitly document unsupported states or design decisions. The original intake was backlog-only; the selected candidate and current test
evidence are in note 283.

### D10F - Third-person camera near close surfaces (research report first)

**Status: Done (research report 2026-11-04, [note 379](documentation/379-d10f-close-surface-camera.md); user review and save states recorded, closed for this session; implementation is D10F1). Dependencies: D10, D10B1, D08J3.** User: "you can get caught
behind objects and such ... usually when up against close surfaces". Report
first: reproduce with the user's real input (player-like orbits against
pillars, crates, doorways, ladder walls) on private save copies, record where
the native solve and its 768 minimum leave the camera behind an occluder,
snap, or pop, and compare with the industry pattern above (sphere probe from
the pivot, instant pull-in, eased return, brief thin-occluder tolerance, no
auto-yaw, shoulder toward centre when compressed). Propose a camera-only
change that keeps D10B1's native Duke and box fading (the user's rollback
reason) and keeps the saved distance and shoulder. Ask the user for save
states where it feels off. Vanilla and the Original camera unchanged; check
180 Hz.

**2026-11-04 research report (Needs review).** [Note 379](documentation/379-d10f-close-surface-camera.md).
Native solve (static read): four rays from the pivot to the corners of a
512 x 240 window at the anchor, snap to the closest hit, then the 768
minimum puts the anchor back at 768 along Duke's heading frame
(`player+0x7CC`), not the view; the lens sits 193 units behind the tested
window (`camera+0x42`/2). Reproduced on a private D10B apartment copy with
the user's saved distance 3456, mouse sweeps of about 9 degrees per step:
lens inside walls at 1312-2058 units (minimum not involved), the wardrobe
face filling the frame, pinned at 761-775 with the camera's room changing,
and a pop of about 1840 units through a doorway in one step (761 -> 2598,
rooms 83 -> 7). D10B's `DNTTK_TIGHT_CAMERA=1` still shows the lens-in-wall
frames and the doorway pop. Free look held: Duke out of the frame in all
five pinned steps (the push follows his back). 180 Hz identical. Proposal
D10F1 (camera position only, native fades kept): probe the real lens with
the original segment query (`0x8006D980`) and a small ring, an on-axis floor
in place of the push-out (about 450), instant in and eased out (about 0.1 s
hold, 0.35 s), shoulder toward centre while compressed. Open: user save
states, the floor, the return speed, and whether a compressed camera may look
into the next room. Limits: one fixture; crates/actors, enemies, climbs and
other levels not probed. No code, build or executable change.

**2026-11-04 user review and closeout.** User saved UI 12 (closet: "part of
the closet is obscuring the camera"), UI 11 (crate room: "very playable",
the native box fade "starts when the camera gets too close") and UI 6
(monkey bars: "the camera goes above the grill ... you want to be able to
look down and ... see if there are any enemies down below"). Direction: "go
through a few of them and just start seeing where we can tighten things up
where the camera does adjust to it better when it's against walls and not
clipping into walls". Recorded in note 379; D10F Done; D10F1 added.

### D10F1 - Third-person camera: clear lens against walls and grills

**Status: Done (user-accepted 2026-11-04, [note 380](documentation/380-d10f1-clear-lens.md)). Dependencies: D10F.** Implement the D10F proposal
([note 379](documentation/379-d10f-close-surface-camera.md)): probe the real
lens with the original segment query and a small ring (world hits only; crates
and breakables keep the native fade the user likes), an on-axis floor in
place of the 768 push-out (start about 450), instant in and eased out (start
0.1 s hold, 0.35 s), shoulder toward centre while compressed. Camera position
only: native fades, saved distance and shoulder, look input and aim
unchanged; Vanilla and the Original camera unchanged; developer A/B switch.
Test on private copies of the user's UI 12 (closet), UI 11 (crate room,
regression: must stay as playable) and UI 6 (monkey bars: camera stays below
the grill with the floor in view; coordinate with D08J3A) plus the D10F
apartment sweep at 60/120/180 (no lens-in-wall frames, no doorway pop, Duke
in frame with free look). Confirm the floor, the return speed and the
doorway rule with the user.

**2026-11-04 implementation (Needs playtest).** [Note 380](documentation/380-d10f1-clear-lens.md).
Modernized third-person orbit only: after the native solve, the follow easing
(`0x8002a038` from `0x8003ade4`) takes an anchor on the orbit's own axis whose
lens is clear. The original segment query traces the pivot to the lens and
three ring points (48 units); the nearest hit less 48 shortens the boom and
shoulder together; floor 450. Actors stop the lens unless the native fade
covers them from it and the lens is outside them (back query): crates pass,
the apartment wardrobe (type 194, whose inside drew black when passed) stops
it. Instant in; out after 0.1 s over about 0.35 s in host time. Native follow
lag kept and its lens re-probed. Monkey bars: the lens stays 96 below the
hanging grate (the query does not report grates). Native fades, saved
distance and shoulder, look input, aim and the 768 word unchanged; Vanilla and
the Original camera unchanged; `DNTTK_CLEAR_LENS=0` A/B. Evidence (private
copies): apartment sweep at 60/120/180 and free look with no black or wall
frames and Duke in frame; UI 12 at distance 3456 the wardrobe no longer fills
the frame; UI 6 the camera below the grill with the floor in view (very close
at a steep pitch, Duke not visible); UI 11 corridor clear but about 200 units
closer than native, so Duke's native fade starts sooner. Cost about 1.35 ms per
camera update orbiting beside the wardrobe (game still 30 updates/s).
`ttk-controls-test` (default, `DNTTK_CLEAR_LENS=0`, `DNTTK_TIGHT_CAMERA=1`),
`ttk-aim-test`, `ttk-near-test` PASS; no framework edits. Candidate executable
`ac4634201d59a1b6133043becb570c55aec461db5631df071c31a9667a5136d4`. Open for the user: the floor, return
speed, the doorway rule (a long boom can still ease into the next room), the
crate room in play and 180 Hz feel.

**2026-11-04 accepted and closed.** User: "I'm very happy with the gameplay and performance in this iteration, and the camera angles have made the game feel solidly playable." Done at the candidate
defaults (floor 450, hold 0.1 s, return 0.35 s, doorways allowed). The other
open questions (doorway rule, corridor fade, steep monkey-bar framing) were
not raised and stay with D08J3A/D10G if they come up. Runtime patch re-export
unchanged (no framework edits). No gameplay edits, builds or launches after
acceptance. Executable `ac4634201d59a1b6133043becb570c55aec461db5631df071c31a9667a5136d4`.

### D10F2 - Camera behind an object when opening a door (UI slot 11)

**Status: Done (2026-11-05, accepted and closed; user: "honestly incredible work"; [note 395](documentation/395-d10f2-use-action-camera.md); the third-person camera lease now covers door kicks, other use actions, the mirror and idle fidgets, so the orbit and clear lens no longer hand over to the native camera, which swung about 900 units out into the wall). Dependencies: D10F1.** User (2026-11-04): "i just saved slot
11 too, when you open the door, duke exhibits a problem there where the
camera just goes behind something and looks dumb". Reproduce on a private
copy of UI slot 11 (runtime 10): open the door and record the lens, the
D10F1 probe hits and actors (a door actor may be passed over by the
crate-fade rule, or the opening door may move into the probed ray between
updates). Fix in the D10F1 clear lens, game-wide, not for this door only.

### D10G - Aim and mouse feel in third person (research report first)

**Status: Done (research report 2026-11-04, [note 384](documentation/384-d10g-aim-mouse-feel.md); user closeout 2026-11-04: "i wanna go with your recommendation for an easy smooth and professional experience"; implementation is D10G1, then D10G2 and D10G3). Dependencies: D07, D10, D10E, D10F.** User: "any tightening we
should do between that [aiming], and even between third person mouse
movement". Report first: mouse sensitivity, smoothing and frame-rate
independence of the orbit at 60/120/180, whether the same mouse motion turns
the view by the same angle when the boom is compressed, crosshair against
muzzle agreement near walls (close retractions, muzzle cover), and whether a
light aim camera while firing (tighter shoulder, a little closer, same look
input) fits TTK. Coordinate with D10E free look (fire cancels it). No change
to Vanilla or the original aim option.

### D10G1 - Point-blank aim: fire along the view when the crosshair point is close

**Status: Done (2026-11-04, [note 385](documentation/385-d10g1-point-blank-aim.md); user: "it's gotta be pointblank 0.35 35", then "lock it in and close it out"; default lift 0.35, cap 35 degrees). Dependencies: D10G.** Measured in
[note 384](documentation/384-d10g-aim-mouse-feel.md): against a wall with the
boom compressed, rounds and the drawn gun arm leave up to 71 degrees off the
view. Modernized, view aiming, third person: when the crosshair point is close
to the muzzle (proposed under about 512 units, or the round would leave more
than about 25 degrees off the view), aim along the view from the muzzle,
blended by distance so nothing visibly switches; the drawn gun arm follows the
same direction. Keep the body-to-muzzle cover guard and the 64-unit
retraction. Test: the D10G wall sweep (no round over about 25 degrees off the
view, every round meets the surface it is fired at), the open-room sweep
unchanged, `ttk-aim-test`, D08J6 climb fire. Vanilla, ORIGINAL aim and first
person unchanged.

### D10G2 - Blocked-shot crosshair hint

**Status: Cancelled (2026-11-04; user: "nah cancel this off ... i dont want that, cool idea tho but not in this game"). Dependencies: D10G1.** When something nearer than the
crosshair point blocks the muzzle-to-point line (one query, already used for
cover), show a small marker at the muzzle hit or dim the crosshair, as in
modern third-person shooters. Crosshair stays at screen centre. Judged with
the user in play; off in Vanilla and with the crosshair off.

### D10G3 - Shot direction from the newest look

**Status: Done (2026-11-05, [note 386](documentation/386-d10g3-shot-view.md); user: "ok yes, it feels great, it feels like an improvement", then "close us out"). Dependencies: D10G1.** Shots read the previous game update's
camera, about 33 ms behind the per-redraw late view the player sees
([note 384](documentation/384-d10g-aim-mouse-feel.md)). Take the direction
from the late view when the round leaves. Native weapon timing (100-200 ms
press to shot) stays unchanged. Widened 2026-11-05 (latency investigation): also take
the eye from the camera on screen, not only the rotation (the shot eye runs
about 64 ms of Duke's sideways motion ahead, about 186 u at run). Test with
`recomp/analysis/d10g-latency/` (`rays.py` turn and strafe) at 60/120/180.

### D10G4 - Shoot what you see: crosshair tested against enemies where drawn

**Status: Done (2026-11-05, closed; [note 387](documentation/387-d10g4-shoot-what-you-see.md); user: "it's aceeptable as it is ... if we can tighten it more, i'd say let's do it"; further tightening is D10H). Dependencies: D10G3.** Proposed by the 2026-11-05 latency
investigation (local `recomp/analysis/d10g-latency/REPORT.md`, candidate 1).
At 120/180 Hz actors are drawn 90 ms in the past (`timeline_pair`) while
rounds are hit-tested against their current positions, so a running enemy
under the crosshair is missed behind (about 171 u at pig cop speed). Test the
D10G3 screen ray against actors where the shown present drew them (the replay
keeps eight snapshots); on an actor hit, aim the round at that actor's current
position after the existing muzzle and cover checks (lag compensation as
network shooters do it). Also fixes the knock-on where the D10G1 targetable
exemption and the D22D beam lock see a wall instead of the moving enemy.
Modernized view aiming only; Vanilla, `original` aiming, native projectile,
spread, damage and weapon timing unchanged; A/B switch like `shotview`. Needs
a moving-enemy test fixture (spawned pig cops stand idle until shot). The user
judges whether it feels fair at 180 Hz on the real display.

### D10H - Weapon responsiveness against Duke Nukem 3D (research report)

**Status: Report ready (2026-11-05, [note 388](documentation/388-d10h-duke3d-weapon-feel.md)); awaiting discussion with the user. Dependencies: D10G4.** User:
"a great test is chasing a pig cop with the pistol ... you can easily fire a
trail of bullets at it and not a single one will land", and "I want the
benchmark to actually be Duke 3D ... each analogue weapon ... should
essentially respond in the same way ... in terms of their responsiveness".
From the local EDuke32 source: the pistol fires on the tic after the press
(33-67 ms), repeats at 6/s, is hitscan, and with autoaim on (the user's
`cl_autoaim "2"`) goes exactly at the nearest visible enemy within about 8.4
degrees; the world is drawn at most one tic behind. TTK: 91-228 ms press to
shot, world drawn 90 ms behind, Modernized auto aim off. Jobs D10H1-D10H6
follow, iterating with the D10H1 lab.

### D10H1 - Weapon feel lab: TTK against Duke 3D analogs

**Status: Done (2026-11-05, accepted and closed; [note 389](documentation/389-d10h1-weapon-feel-lab.md); the pistol chase tracker is not reliable yet and moves to D10H2). Dependencies: D10H; the user's
"enemyshoot" save (an EDuke32 save, `~/.config/eduke32/save0004.esv`, E3L1).** Offscreen, private copies: per analog weapon (note 388 table)
measure press to shot, repeat rate while held, press to the shot visible on
screen, and per-round hit or miss against moving enemies (add a diagnostic
log of rounds that damage an actor). Scenarios: "enemyshoot", a pig cop chase
with the pistol, the D10G4 slide fixture (`seen_slide.py`). Output: a TTK vs
Duke 3D table re-run after each later job. No gameplay change.

### D10H2 - Pistol chase: rounds that leave on target land

**Status: Done (2026-11-07, accepted and closed; user: "lock it in and close it out. insane work"; [note 410](documentation/410-d10h2-pistol-chase.md); aim queries use the rounds' own actor test, round follow `roundfollow`). Dependencies: D10H1.** D10H1 measured it on the slide
fixture: about a third of the rounds that leave with the crosshair on a
moving pig cop miss (still targets 12 of 12). First reproduce that with an
enemy the game moves itself, because the fixture's host writes can land
between the query and the sweep. Using the lab's round hit log, find any
reason a round that leaves on the drawn enemy still misses (the game update
order, the muzzle start beside the screen ray, the running hit box) and fix
it game-wide. Test: the pistol chase; still targets, D10G1, D10G3, D10G4 and
`ttk-aim-test` unchanged.

### D10H3 - Duke 3D style auto aim for bullet weapons (option)

**Status: Done (2026-11-06, accepted and closed; user: "lock it in and close it out!!"; [note 403](documentation/403-d10h3-eduke32-auto-aim.md); AUTO AIM EDUKE32 / HITSCAN (default) / OFF / ORIGINAL in ADVANCED > COMBAT; never NPCs). Dependencies: D10H1.** User decision (2026-11-05): "on by
default, and a toggle to turn it off"; they play EDuke32 with autoaim on
hitscan weapons only ("Available settings are full, hitscan, and off ... If
set to hitscan, this automatic aiming will only apply to hitscan-based
weapons and not missile type projectiles", wiki.eduke32.com/wiki/Player_setup).
So: Modernized AUTO AIM defaults to the Duke 3D hitscan behaviour, with an
OFF toggle (keep ORIGINAL available). The Duke 3D analog bullet weapons
(Desert Eagle / starting pistol, Combat Shotgun, Gatling) take the nearest
visible targetable enemy inside the aim cone, roughly on screen vertically,
and aim the round exactly at it after the muzzle and cover checks. RPG,
freezer, throws and the TTK-only weapons stay unassisted. Cone: the source
gives 48/2048 of a turn either side (about 8.4 each side, `AUTO_AIM_ANGLE 48`)
while the wiki says "approximately 9 degree wide". User (2026-11-05): "we go
with eduke's code": the default is the source's 48/2048 either side
(about 8.4 degrees each side, about 17 across); console tunable. Test: the lab's chase and "enemyshoot" hit
rates against off; Vanilla unchanged.

### D10H4 - Shot on press: ready stance, then earlier fire frame

**Status: Done (2026-11-05, accepted and closed; user: "i accept the work we've done here. It's solid", "lock it in and close it out"; [note 390](documentation/390-d10h4-shot-on-press.md); WEAPON FEEL RETRO / MODERN). Dependencies: D10H1.** D10H1 found that TTK fires only if
fire is still held at the weapon's fire frame: presses under about 150 ms
fire nothing (rest and gun up), while every Duke 3D click fires. Latch the
press. User (2026-11-05): changing native
timing in Modernized is fine, "and make it togglable"; TTK-only weapons
(crossbow, buffalo rifle, flamethrower) keep their timing ("the dynamic on
those is actually pretty good"). Keep the gun up while armed in play so
every first shot takes the gun-up path (about 100 ms instead of about 210
from rest), then investigate firing on an earlier animation frame towards the
Duke 3D analog (pistol 33-67 ms). Modernized only; ammo, damage, spread and
Vanilla unchanged. Test: the lab's press-to-shot table, first and third
person, climbs and swimming.

### D10H5 - Shorter world delay at high refresh

**Status: Done (2026-11-05, accepted and closed; user: "adaptive it is. i've just played all of time to kill, challenge 1, and a bit of duke hill, and didnt experience a single issue"; [note 392](documentation/392-d10h5-adaptive-world-delay.md); adaptive delay about 63-72 ms at 180 Hz instead of 90, `DNTTK_WORLD_DELAY_MS` fixes it). Dependencies: D10H1.** Bring the 90 ms present timeline delay
(`DNTTK_WORLD_DELAY_MS`) towards Duke 3D's one tic, adaptive and about
55-65 ms first, so the world, muzzle flash and impacts show sooner. Judge at
180 Hz on the real display for judder (130 ms and faster phase correction
were rejected before). D10G3/D10G4 follow automatically.

### D10H6 - Repeat rate parity with Duke 3D analogs

**Status: Done (2026-11-05, accepted and closed; user: fire rate "accepted for how it feels in the water and on land"; [note 391](documentation/391-d10h6-held-fire-clock.md); fire CPU 200 while held, not in water, steadies the rate at CPU SPEED 150; user after the D10H4 build: holding fire on the pistol "has a kind of wonky feel to the shooting consistency ... in comparison to eduke32", then "let's do it now! this might actually get us to the 1:1 i want"). Dependencies: D10H1, D10H4.** Where TTK repeats slower while
fire is held, match the Duke 3D analog (pistol 6/s, chaingun about 10/s,
shotgun 1/s, RPG 1.5/s), Modernized only, with a toggle (user, 2026-11-05).
TTK-only weapons (crossbow, buffalo rifle, flamethrower) unchanged (user:
"the dynamic on those is actually pretty good"). Test: the lab's repeat table.

### D10H7 - Queue redraws at their exact lead

**Status: Done (2026-11-05, accepted and closed; user: "it works absolutely fine, it's so snappy now"; [note 393](documentation/393-d10h7-submit-lead-margin.md): submits are never early, so the present term was tail margin; halved, about 3 ms less world delay at 180 Hz and 4 at 120, starved 0.044%; `DNTTK_WORLD_DELAY_MARGIN=1` is the D10H5 build). Dependencies: D10H5.** D10H5's adaptive world delay is
p99 pair age + lead + one present period; the period term (5.6 ms at 180 Hz,
8.3 at 120) covers redraws being submitted up to a present early, because
`timeline_provider` submits every deadline within the lead at each tick. If
each redraw is submitted at (or measured against) its exact t - lead, the
period term can go: about 3-6 ms less world delay. Measure the actual
submit-to-deadline spread first (`target - submit_ms`); `starved` and pace
repeats must stay at the D10H5 level (`world_delay.py`, 120 and 180 Hz,
levels 1 and 8). Modernized only; Vanilla unchanged. Not in scope (D10H5
note 392): a lower percentile, early partial frames, extrapolation.

### D10H8 - EDuke32-close world delay in first person

**Status: Done (user-accepted 2026-11-08: "it's a whole tonne better!", 16 ms chosen, then "lock it in, close it out"; scoped from D49D; user: "eduke32 feel (cleanroom) IS the goal"; [note 419](documentation/419-d10h8-newest-world-draw.md)). Dependencies: D10H5, D10H7.**
At 120/180 Hz the view turns at once but the world (hits, smoke, bullet
holes, enemies, pickups, the beam) is drawn about 50-90 ms in the past,
because each present blends two finished game frames (`timeline_pair`; D10H5
note 392: the floor is one game interval plus composition plus lead). EDuke32
(research/eduke32: `drawframe_do` -> `P_GetInput` turns the view every
frame; `G_DrawRooms` blends the last two 30 Hz tics) draws its world 0-33 ms
behind, so a wall hit trails the crosshair by at most one tic of turning and
nothing needs moving. D49D showed that moving individual world objects to
hide the delay reads as sliding; this job attacks the delay itself. Measure
candidates with A/B switches, first person, 120 and 180 Hz, levels 1 and 8:
drawing from the newest finished frame (no blend; moving things step at the
game rate), a short extrapolation past it, and blends in between; record world
delay, `starved`, judder of enemies and Duke's motion, and how far wall hits
trail the crosshair on fast turns. Then decide with the user which D49/D49A
follow tricks are still needed. Modernized only; Vanilla, 60 Hz and gameplay
unchanged. Acceptance: user playtest against EDuke32 for turning and firing
feel.

**Work log (2026-11-08, Needs playtest).** Cause found: the redraw replayed
the older frame's display list, so anything new (hole, smoke, flash, HUD,
weapon frame) appeared a whole world delay (about 65-75 ms) after the game
made it, about 25 degrees of trail at a 360 deg/s turn. First person now
replays the newer frame's list with the same blended positions (as EDuke32
draws its newest sprites at interpolated positions), plus at most 8 ms of
extrapolation past the newest pose, limited per axis to the speed both of the
last two game steps agree on (stairs, stops and reversals hold, no overshoot);
the adaptive delay target is 8 ms lower. `frame_replay.cpp`
(`world_draw`, `lerp_x`, `submit_job`, `shown_poses`) and
`render_state_blend_newer` in `modern_controls.cpp`. Lab (`wd8.py`, private
apartment copies, first person, 180 Hz): new hits appear 28 ms after their
game frame instead of 71 (level 1: 26 vs 66; level 8: 17 vs 57), trail 10
degrees instead of 25 at 360 deg/s, world 6-9 ms fresher, no eye-path
deviation over 8 units, starved and pace unchanged. Rejected: step (no blend,
world jitter p95 28 ms) and 16 ms (no fresher hits in fast scenes, about 3
eye-step deviations per second on level 8). Third person, Vanilla, 60 Hz and
gameplay unchanged. `DNTTK_WORLD_DRAW=pair` / `DNTTK_WORLD_EXTRAP_MS=0|16`
compare (16 became the default after the user's A/B, below). `ttk-aim-test` and `ttk-input-test` pass. Executable
`2fdd2e6b8e4543890ab0154a717359528f2b05a7777ff31f78a7b49d1b2d172b`. Needs
the user's playtest against EDuke32: turning and firing (pistol, shotgun,
Gatling at a wall), strafing, stairs and enemies for smoothness; then decide
which D49/D49A follow tricks are still wanted.

**User playtest (2026-11-08): "it's a whole tonne better!"** The user found
`DNTTK_WORLD_EXTRAP_MS=16` a little more in sync and saw no wobble at all
("so let's go for 16 as default then"). Default now 16 ms: level 1 run-strafe
new hits 18.5 ms after their game frame (6.7 degrees at 360 deg/s), delay
47 ms, 0 starved. Executable
`289bc12019668f18782a8be599e9934e0baa4a4ef89ad4c40b06ab8599e666f9`.

**Accepted (2026-11-08).** User: "lock it in, close it out". The energy
beam's D49/D49A follow (glow and sparks) stays as accepted; revisit only if
the user asks.

### D10H9 - Press to fire on the energy weapon and flamethrower

**Status: Done (user-accepted 2026-11-09: "this one is fully accepted. great work", "lock it in and close it out!!"; user chose proposals 1-3: "we go with your exact recommendation!"; [note 425](documentation/425-d10h9-stream-weapon-press.md); user report 2026-11-08, D49B playtest: "there feels to be a lag between mouse click down, and actual firing, for the energy weapon, and the flamethrower"). Dependencies: D10H4.**
D10H4 latched the press and kept the gun up for the Duke 3D analogs, but
left TTK-only weapons, including the flamethrower, on their native timing
(user, 2026-11-05: "the dynamic on those is actually pretty good"). The
energy weapon's press to fire was not in its table. Measure press to the
first launch and to the first shot on screen (D10H1 lab, `ttk-launch`
types 14 flamethrower and 15 energy beam), from rest and gun up, 180 Hz,
first and third person, and compare with Duke 3D analogs (EDuke32
flamethrower/freezer stream and an instant-fire weapon). Find which part is
the weapon's raise or wind-up, the fire frame and the world draw, then
propose cuts to the user before changing native timing. Modernized only,
behind the D10H4 WEAPON FEEL switch where it changes timing; ammo, damage
and Vanilla unchanged. Acceptance: user playtest clicking and holding fire
with both weapons.

**Work log (2026-11-08, measured, proposals to the user).** No gameplay
change; diagnostic trace `DNTTK_FEEL_TRACE=1` (`ttk-feel-upd` per player
update). Lab `recomp/analysis/d10h9/` (private apartment card copies, 180 Hz,
RETRO). Press to the first round, first person: flamethrower 430-457 ms from
rest and 594-599 ms 0.3 s after a release; energy weapon 289-350 and 458-464
(third person similar). EDuke32 freezer 81, pistol 58; the D10H4 guns 50-82.
Both weapons fire from the ready pose 23 while fire is held (stream flag
`+0x224 0x400`). Before that, the raise 24 takes about 265 ms, after one
update to see the press and one more before the raise starts. The
flamethrower then ramps `+0x81c` to 100 over 3 updates (about 100 ms, nothing
drawn) before the first fireball. A release starts the lowering at once, and
a new press waits for the full lowering and raise. 60 ms and 150 ms clicks
fire nothing (0 of 12 each). The flamethrower stream drops the game to 50 ms
updates (about 20 fireballs/s). Proposals in note 425: (1) latch the press and
run the raise out on the next update, (2) stay ready about 1 s after a
release, (3) cut the flame ramp to 1 update, (4) optional fire CPU under the
flame (changes damage per second). Awaiting the user's choice.

**Work log (2026-11-09, Needs playtest).** User: "we go with your exact
recommendation!" (1-3). `weapon_feel.inc`, RETRO only. A press with the
Flamethrower or Energy Weapon drawn latches fire until the stream starts. The
raise (track 24 forward) runs out and carries into the ready pose 23, and a
lowering in progress runs out first. The original's ready timer `+0x23a` is
kept at 1 s while firing and after its own 333 ms write, so the gun stays up
after a release. The flame charge `+0x81c` is precharged to 68, so ignition
takes one update. Lab, first person, 180 Hz: Energy Weapon about 82 ms from
rest (was 289-350) and 48-50 within a second of a release (was about 460);
Flamethrower 83-130 (was 430-457) and 48-84 (was about 595). Third person is
similar. 60 ms clicks fire every time (were 0 of 12). Held streams, MODERN
(430-605 ms as before) and weapon switching are unchanged. `ttk-input-test`
and `ttk-aim-test` pass. Executable
`381b62070b0bc842475199f487339cafeb25d1cfbd3550bc130040f64355de28`. Needs the
user's playtest clicking and holding fire with both weapons.

**Accepted (2026-11-09).** User playtest: "this one is fully accepted. great
work". The user's lighting observation from the same playtest (the upgraded
flamethrower glows blue, its light stays warm) is D41J.

### D27 — Caps Lock RUN MODE quotes and Shift-run clunk silence

**Done for Caps Lock quotes (user accepted). Shift-run clunk silence deferred
low-priority (2026-09-28).** Plant-aligned gait delay was rejected: longer Shift→run
lag and clunk still audible. Restored immediate frame-0 walk↔run restart (responsive
gait with known plant SFX clunk). Soft mid-stride phase reuse remains forbidden
(prior freeze). Future work: identify plant SFX path and suppress only on Shift
gait restart without delaying the switch — not blocking.

**Acceptance (quotes):** met. **Acceptance (silent Shift-run):** deferred.

### D27A - Modern Shift/run is silent

**Done - user-accepted (2026-10-08):** "i fully accept this. great work. i
have wanted this one for a long time so this minor change has a huge impact on
me as a player." Modern Shift no longer clicks; the heartbeat is unchanged. See
[note 130](documentation/130-d27a-silent-shift.md). User request, 2026-10-08.
Follow-up to D27's deferred "silent Shift-run".

**Problem.** In Modern controls, pressing (and releasing) Shift to run plays a
sound. The user wants Shift silent: hold Shift -> run, release -> stop, with
no click, pulse, activation, looping or replacement sound caused by either
action.

**The sound is already identified (D08A10, do not rediscover):** see
[note 129](documentation/129-d08a10-steroids-heartbeat.md), "TTK sounds".
- Sound ID `0x0001` (bank 0; `id = bank << 12 | index`). SPU sample address
  `0x012F0`, pitch `0x228`-`0x22F`.
- What the game itself uses it for is not established; the D08A10 survey
  only showed that the player update plays it when `player+0x224` bit 1 (the
  walk/run state) flips. Do not assume any original link between this sound,
  steroids and running.
- Call path: the non-positional sound call `0x8006bbd8(1)` (not the
  positional `0x8006b73c`, so a log of that call alone misses it), from the
  player update at `0x800418cc`, `0x80041934`, `0x80041994` and `0x800419c8`.
  Find which of these the Modern Shift path reaches (by return address).
- Our own reuse: the D08A10 steroids heartbeat (`recomp/src/ttk/steroids_beat.inc`,
  `duke_sound(cpu, beat_sound(), false)`) calls `0x8006bbd8(0x0001)` itself on
  a private stack with return address `0x800000fc`, so it can be told apart
  from the player-update calls.
- Separate from the gait plant footsteps `0x2000` / `0x2001` (`0x80048378`,
  ra `0x80048380`), which are D27's old "clunk" topic and stay as they are.

**Scope.** Modernized only. Suppress `0x0001` only where the Modern Shift
walk/run toggle causes it; do not remove the sound resource or touch the
heartbeat. Find out whether Vanilla / original controls play the same click
on their own run toggle, and keep them unchanged.

**Acceptance.**
1. Shift in Modern makes Duke run normally.
2. Pressing Shift makes no sound.
3. Holding Shift makes no sound.
4. Releasing Shift makes no sound.
5. Repeated presses never trigger it.
6. Running behaviour and speed unchanged.
7. The D08A10 heartbeat still plays `0x0001` correctly (same sample, pitch).
8. The heartbeat still plays for the whole steroids cycle.
9. Running while steroids are active does not disturb the heartbeat.
10. Original / Vanilla controls unchanged.
Evidence: SPU KEYON capture (sample `0x012F0`) around Shift press, hold and
release with and without steroids, as in D08A10.

### D28 — Scroll Lock holster and WEAPON LOWERED/RAISED quotes

**Done — explicit user acceptance (2026-09-28 morning).** Scroll Lock holster and
centered quotes confirmed in playtest.

**Acceptance:** default binding + quotes; custom rebinds; E auto-stow/ladders;
manual/game manual updated.
