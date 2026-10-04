# Time to Kill — modernization jobs

This is the canonical job list for our **Duke Nukem: Time to Kill** PC project, targeting the owned US SLUS-00583 disc. The ambition is a polished, game-specific PC edition: faithful original play plus an optional modern experience. This is a plan, not a list of features already available.

Invoke **`$continue-duke-recomp`** (Codex) or **`/continue-duke-recomp`** (Claude Code) to see the current jobs and choose one. You can also request a job directly: **`$continue-duke-recomp work on D01`** or **`/continue-duke-recomp work on D01`**. The skill reads this file rather than keeping a second backlog. It must not automatically start the next job.

**Latest accepted job: D15 - optional geometry and texture precision.** [Acceptance and evidence](documentation/96-d15-accepted-precision.md). **Immediate follow-up: D17P**, the new subway slot 8 distant horizontal bands; separate from the accepted previous slot 8 opacity case.

## The experience we are building

**Vanilla** preserves the original movement, camera, aiming, game rules and presentation by default. Correctness fixes, reliable audio/video, clean shutdown and necessary PC integration belong in both modes. Optional display enhancements should remain independently selectable.

**Modernized** is an opt-in preset: WASD movement relative to the camera, independent mouse aiming, modern third-person camera, controller equivalents, higher-resolution rendering and appropriate quality-of-life features. This means changing how movement and aiming work, not just assigning keyboard keys to PlayStation buttons. First-person play follows a solid third-person foundation.

Players should be able to choose a mode, inspect its settings, customize it and restore its defaults. Start with selection at launch or another proven safe boundary; switching cameras and movement systems mid-animation is not an initial requirement. Preserve separate user preferences when switching presets. Explain settings that change gameplay or save compatibility.

Our reference for scope is the idea of a lovingly enhanced edition of one game. There is no requirement to make a generic recomp framework, engine or mod system.

## Where we are starting

- The local native/hybrid build runs; some overlays still use interpretation. A working first level is not proof of a complete 1:1 campaign.
- The user reports smooth gameplay and working FMV after the playback work. The repeated voice issue appears resolved in their testing.
- The user successfully inserted the first map's security card, entered the next area and closed the window cleanly. The earlier gameplay freeze did not reproduce on that pass; its exact cause remains unproven. Keep the diagnostic support.
- Persistent profiles and PC action bindings/capture are implemented (D02/D04). D05/D06 are Done following successful user playtesting and explicit sign-off of the guarded first-map movement/camera implementation; see their work logs for remaining coverage limits. D07 now has a user-accepted bounded weapon-aiming preview; first-person play remains upcoming work. Existing framework options are candidates for reuse, not evidence that a feature works in this build.
- See [current status](documentation/00-status.md), [manual](GAME_MANUAL.md), [validation](documentation/05-validation.md) and [camera research](documentation/06-first-person.md) for evidence and constraints.

## Job board

All jobs start **Todo**. Dependencies are prerequisites for completion; small investigations can happen earlier. A Todo job is ready to implement when its listed dependencies are Done. Use **In progress**, **Needs playtest**, **Blocked** or **Done** as work proceeds. Record the reason for a blocker and evidence for completion in the job's work log below. Do not confuse a successful build with a successful playtest.

| ID | Job | Status | Depends on |
| --- | --- | --- | --- |
| D01 | Establish the Vanilla regression route | Done | — |
| D02 | Persistent Vanilla / Modernized profiles | Done | D01 |
| D03 | Player, camera and aiming state research | Done | D01 |
| D04 | PC action input, rebinding and mouse capture | Done | D02 |
| D04A | Escape reliably frees the mouse for the pause menu | Done | D04 |
| D05 | Camera-relative WASD movement | Done | D03, D04 |
| D06 | Independent third-person mouse camera | Done | D03, D04 |
| D07 | Modern weapon aiming and crosshair | Done | D05, D06 |
| D07A | View-aligned Duke facing and weapon presentation | Done | D07 |
| D07B | Optional assisted view aiming and display controls | Done | D07 |
| D07C | Unified Duke3D-style aiming, facing and projectile coverage | Done | D07, D04 |
| D07D | Red autoaim dot off by default in Modernized (crosshair only) | Done | D07B, D07C |
| D08 | Modern traversal controls — accepted iteration | Done | D05, D06, D07 |
| D08A | EDuke32-style weapon and item shortcuts | Done | D04, D08 |
| D08A1 | Visible EDuke32-style inventory cycling | Done | D04, D19A |
| D08A2 | EDuke32 bottom-left inventory icon and green % | Done (revised: strip + green %) | D08A1, D19A |
| D08A3 | Original TTK inventory icons for the switcher (replace Duke3D art) | Done | D08A2 |
| D08B | Broader traversal and scripted-camera coverage | Done | D08 |
| D08C | Directional jumps from standstill — accepted both input orders | Done | D08 |
| D08D | Apartment light-switch secret convenience | Done | D08 |
| D08E | Responsive holstered fire and weapon transitions | Done | D08 |
| D08G | Typed Duke-style debugging cheats — user accepted | Done | D04 |
| D08G1 | Original Duke3D cheat confirmation wording | Done | D08G |
| D08G2 | Silent cheat entry and centered confirmations | Done | D08G1, D19A |
| D08H | Apartment furniture, hidden pickup and switch targeting | Done | D08 |
| D08I | Responsive run-start and edge jumps | Done | D08 |
| D08J | Armed airborne ladder grabs and automatic weapon transitions | Done | D08, D08E |
| D08K | True crouch walking and animation feasibility | Blocked | D08 |
| D08L | Inertial platform edge run-off | Done | D08 |
| D08M | Modern underwater swimming controls (foundation) | Done | D08 |
| D08O | Deep free-swim polish (strafe, Ctrl dive, mantle-only exit) | Done | D08M |
| D08N | Duke3D-style scuba gear item | Cancelled (out of scope) | — |
| D08P | Crystal-2 turret / scripted-camera control recovery | Done | D08 |
| D08Q | Modern jetpack flight controls | Done | D08 |
| D08R | Selectable jetpack scheme: Modern / Classic (WASD), CLI quick ship | Done | D08Q |
| D08Q1 | Faster Modern jetpack Ctrl descent (underwater dive speed) | Done | D08Q |
| D08Q2 | Jetpack unavailable after death, Continue and dnstuff (slot 10) | Done | D08Q, D08A1, D08G |
| D08Q3 | Jetpack weapon aiming and missing crosshair | Done | D07C, D08Q, D08Q2, D08R |
| D08S | Duke3D-style jetpack scheme (instant J on/off, midair) | Cancelled (may revisit) | D08R |
| D08T | Pushable objects: modern grab/push/pull and climb (alley dumpster) | Done | D08 |
| D08T1 | Separate push/pull from mantling: E always mantles, hold RMB to grab | Done | D08T, D04 |
| D08U | Top-of-ladder mount: grab a ladder from a platform and climb down | Done | D08, D08J |
| D08V | Sewer mantle/hang modern-control coverage (slot 12 area) | Done | D08, D08B |
| D08W | Subway shallow-water sideways jumps (A/D + Space jumps forward) | Done | D08, D08C |
| D08X | Hold-E airborne ledge grab and mantle (ladder-grab feel for ledges) | Done | D08, D08J, D08V |
| D08Y | Gap jump dead band: jump-mantle level-geometry ledges (slot 5 gap) | Done | D08X |
| D08Z | Optional manual modern jump (player-timed takeoff, air control) | Done | D08Y |
| D08J1 | Hold-E run-up grab for overhead ladders (slot-6 ladder) | Done | D08J, D08X, D08U |
| D09 | Modern controller support | Todo | D05, D06, D07 |
| D10 | Third-person camera polish | Done | D08 |
| D10A | Rapid mouse turning and Shift-running investigation | Done | D06, D07C, D08 |
| D10B | Tight-space third-person camera: translucent Duke and see-through doors | Todo | D10, D11B |
| D11 | First-person playable prototype | Done | D08, D10 |
| D11B | First-person near-wall polygon clipping | Done | D11 |
| D11A | Scroll-wheel zoom lock into first-person | Cancelled (P toggle suffices) | - |
| D12 | First-person weapons and state polish | Done | D11 |
| D12A | First-person quick kick without leaving the eye view | Done | D12 |
| D11C | Savestates can keep Duke's first-person head hidden (slot 12) | Done | D11 |
| D13 | Higher internal resolution and display scaling | Done | D02 |
| D14 | Widescreen, FOV and visibility | Done | D03, D13 |
| D15 | Optional geometry and texture precision | Accepted | D13 |
| D16 | Texture filtering and game-specific HD assets | Todo | D13 |
| D16A | HRP assets and first-person weapon research (later) | Cancelled (original assets preferred) | - |
| D17 | High refresh rate rendering without faster simulation (Match Display, 30-240, Unlimited) | Done (user-accepted) | D01, D13 |
| D17A | High-refresh texture/geometry instability: popping, flicker, black areas | Accepted | D17, D11B, D14 |
| D17B | Mouse responsiveness and input latency at high refresh rates | Accepted | D17, D08 |
| D17D | Club-exit furniture: lower wooden board popping | Accepted | D17A |
| D17E | Subway peripheral wall visibility near the camera | Accepted | D17A |
| D17F | Apartment/club closet edge and lower-geometry artifacts | Accepted | D17A |
| D17G | Strip-club translation jerk during Shift+W / Shift+S | Todo | D17B |
| D17H | Train-control and platform/ledge movement stutters | Todo | D17B |
| D17I | Unlimited stale-camera scheduling episodes | Todo | D17B |
| D17J | Verify older isolated visual reports against accepted baseline | Todo | D17A |
| D17K | Coplanar ground blood in save slot 12 | Accepted | D17D |
| D17L | Tabletop props cut off on approach/retreat in save slot 11 | Done | D17D |
| D17M | Shotgun ammo visible through ladder platform in save slot 9 | Todo | D17D |
| D17N | Diagonal wall artifacts during movement in save slot 12 | Todo | D17A |
| D17O | Unstable sky appearance when looking up (slot 10) | Done (user-accepted) | D17A, D17B |
| D17P | Distant horizontal black bands in new subway slot 8 | Done (user-accepted) | D15 |
| D17C | View bob: disable experiment, then a stable modern camera | Done (user-accepted) | D11 |
| D18 | FMV and audio presentation safeguards | Todo | D01, D02 |
| D18A | Voice/music/gunfire crackle investigation | Done | D01, D02 |
| D18B | Concurrent voice with music (no music mute) | Todo | D18, D21 |
| D18C | Load-sensitive crackle at construction signs and train ledge | Todo | D17B, D18A |
| D19 | Modern in-game menus, settings and input prompts (Sonic 3 A.I.R.-style customization; plan mode + artifact first) | Todo | D02, D04, D13 |
| D19A | Duke font assets for host messages and modern UI | Done | D04 |
| D19B | Responsive modern menu navigation and transitions | Todo | D02, D04 |
| D20 | Save management and optional quick saves | Todo | D01, D02 |
| D21 | Accessibility and sound controls | Todo | D04, D19 |
| D22 | Campaign fidelity and overlay coverage | Todo | D01 |
| D22A | Portal transition loses Modernized controls and first person (slot 7) | Todo | D05, D06, D11 |
| D23 | Performance budgets and long-session stability | Todo | D01 |
| D23A | Modernized frame-budget regression (guard identity cost) | Done | D08 |
| D23B | Intro FMV stutter: stranded native movie shard | Done | D23 |
| D24 | Linux / Windows player build and disc import | Todo | D19, D22, D23 |
| D25 | Modernized edition release acceptance | Todo | D08, D08A, D08B, D09, D10, D14, D17, D18, D20, D21, D24 |
| D26 | Backtick debug console (fps and helpers) | Done | D04 |
| D27 | Caps Lock RUN MODE quotes; Shift-run clunk silence deferred | Done (quotes); clunk deferred low-priority | D04, D19A |
| D28 | Scroll Lock holster and WEAPON LOWERED/RAISED quotes | Done | D04, D19A |
| R01 | DisruptorRecomp architecture and modernization reference research | Done | - |

Recommended opening sequence: **D01 → D02 → D03 → D04**, then **D05 / D06 → D07 → D07A → D08**. D13 is an early graphics option after profiles exist. D22 and D23 should accumulate evidence throughout development. First-person, HD asset packs and precision rendering are optional follow-up milestones, not blockers for a good modern third-person release.

## Foundation and controls

### D01 — Establish the Vanilla regression route

Create a repeatable route from boot and FMV through first-level combat, inventory, security-card insertion and the newly accessible area, then save/load and window close. Record build identity, settings, input steps, expected behavior and timings. Preserve test memory cards separately from the player's cards. Include the voice/music transition and diagnostics from the previous bugs.

**Acceptance:** another session can repeat the route from documented starting conditions; current results and known gaps are recorded. Include both observed user feedback and reproducible checks without treating either as full-campaign proof.

### D02 — Persistent Vanilla / Modernized profiles

Introduce versioned, persistent settings with explicit defaults and a minimal usable mode selector. Separate original gameplay behavior from optional presentation settings. Keep Vanilla as the initial default until the modern preset is playable; label incomplete Modernized features honestly. Document when changes take effect.

**Acceptance:** selection survives restart; restoring each preset works; malformed or older settings recover sensibly; switching back restores Vanilla behavior and preserves user customizations predictably. No advertised toggle silently does nothing.

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

### D08 — Modern traversal controls — accepted iteration

**Done on explicit user acceptance.** The user confirms every tested ladder jump
behaved as expected, E reliably caught the next ladder, and the controls achieved
the requested fluidity. This closes the current movement/interaction/ladder
iteration, including Escape/Start, running continuity, automatic interaction
holstering/redraw, and the second-ladder transfer fix.

Evidence: [state matrix](documentation/25-traversal-state-matrix.md),
[transfer reproduction and delivery](documentation/31-ladder-transfer.md), and
[recorded checks](documentation/reports/d08-ladder-transfer.json).
The original broad D08 coverage is retained explicitly under **D08B**, rather
than claiming swimming, every script/map, or full campaign validation is complete.

The later **D08C** standing/walking jump follow-up is also **Done** on explicit
2026-09-27 user acceptance: W→Space and Space→W during takeoff preparation both
work. See [accepted jump contract](documentation/34-standing-directional-jumps.md).
This adds accepted input-order flexibility without reopening D08 or claiming
full airborne steering or broader D08B terrain/campaign coverage.

### D08A — EDuke32-style weapon and item shortcuts

**Done — user accepted remaining playtest, 2026-09-28.** Wheel switching was
already accepted; remaining pickup/era/medkit/detonator checks were accepted
in ongoing campaign play. Keep documented TTK-only substitutions.

**Implemented 2026-09-27.** The delivered mapping uses
verified TTK executable identities and original use paths, with EDuke32 semantic
groups and the authorized substitutions on 7–9. Explicit user
requirements: **M uses the portable medkit; 6 selects pipe bombs**. Number keys
must represent familiar weapon roles, not simply the existing TTK slot offset.
Cover the complete weapon/item inventory and era variants, documenting sensible
choices for TTK-only items and absent Duke 3D equivalents.

Implementation and evidence: [controls engineering note](documentation/33-controls-shortcuts.md).
The [research proposal](documentation/32-eduke32-weapon-item-plan.md) is historical;
the user authorized its number groups, responsive held Ctrl crouch, and bounded
Alt-wheel camera distance. Q uses original standing Boot melee only; armed quick kick remains unproven;
R has no stored steroid dose in TTK. Their concrete limitations are documented.
Custom bindings are preserved with explicit migration of old defaults.

The local EDuke32 research and copied reference configs informed the layout;
owned-executable weapon records and original inventory/equipment routines establish
TTK identities independently of manual order. The complete mapping is in the
engineering note and [player manual](GAME_MANUAL.md). Repeated number-group presses
cycle usable alternatives; wheel up/down selects previous/next usable weapon;
X restores observed successful equipment. Zero-spare-ammo pipe-bomb detonator
access is retained when a live bomb remains. M/J/B/N, brackets and U use verified
original inventory paths. Q is restricted to safe original standing Boot behavior;
R cannot use a stored steroid dose because TTK has none.

**Human evidence:** the user explicitly confirms wheel weapon switching works and
praises the controls. Their confirmed Alt-wheel distance behavior belongs to the
bounded D10 feature. Neither report establishes every weapon/item shortcut.
Automated delivery evidence includes 57 Python tests, four native suites, a private
SDL inventory-fixture route and a 14-checkpoint Vanilla route; see
[recorded checks](documentation/reports/d08a-controls.json). Fixture ownership is
not evidence of natural item acquisition across eras.

**Remaining playtest:** real pickups/era variants, medkit use including a stable
full-health case, depletion and live detonator access, repeated/rapid commands,
interrupted actions, custom bindings and menu/focus transitions. Record specific human results as they arrive. Remaining playtest was accepted
2026-09-28.

Preserve the accepted movement and aiming, **E interaction-only (never fire)**,
**C holster**, Escape/Enter Start, and F10 capture semantics. This is a weapon/item
mapping pass, not permission to copy conflicting EDuke32 movement/menu bindings.
Keep Vanilla unchanged, preserve custom bindings and saved settings with an
explicit default-binding migration, and document new player-facing shortcuts.

**Acceptance:** all available weapons/items have a documented binding or explicit
unsupported/no-equivalent explanation; M consumes only an owned usable medkit
through the original heal/use path; 6 selects an owned pipe bomb without throwing
or detonating it; number keys select the intended weapon/era counterpart. Verify
empty/unowned inventory, full health, depletion, repeat presses, rebinding conflicts,
menu/focus isolation, and Vanilla regression. No inventory grants or damage/health
rule changes. User playtesting must confirm the familiar layout and preserved feel.

### D08A1 — Visible EDuke32-style inventory cycling

**Done — explicit user acceptance (2026-09-28).** Silent empty start and tiles-only
strip confirmed. Enter use remains. See [iteration 48](documentation/48-armed-ladder-reliability.md).
EDuke32 bottom-left single-icon HUD is tracked as **D08A2**.

**Follow-up implemented; Needs playtest.** Enter uses the highlighted gadget while the strip is visible, with one press owned through timeout/focus until release. Ordinary Enter pause elsewhere and U/custom bindings are retained. Default GL and custom O/P/L Software checks pass. See [current evidence and limits](documentation/46-turning-audio-inventory-progress.md).

**Needs playtest; implemented and privately verified.** Shared original-menu
selection, temporary named tiles/arrow/charge, custom bindings and both renderers
verified. See [evidence](documentation/44-traversal-inventory-implementation.md).
Follow-up to D08A: Brackets already select eligible TTK gadget
IDs 5,1,2,3 (medkit, jetpack, biomask, goggles) without using them; U activates the
selection. The existing host selection has no visible feedback in that branch.
Audit original TTK inventory selection/HUD paths before adding an overlay.
Use EDuke32's temporary owned-item strip, selection arrow and item-name feedback
as the interaction/presentation reference. Keep TTK item identity, quantities and
activation rules; no invented Duke3D-only inventory or borrowed mismatched icons.

**Acceptance:** both directions wrap and skip unavailable entries; zero/one/many
items, depletion, active toggles, original-menu selection, direct shortcuts and U
agree on the visible selected item. Cycling never consumes/activates an item.
Show a temporary readable selection strip/marker and name with the existing Duke
font, using inspected original TTK art where suitable and documented fallback.
Test rapid/repeated input, custom bindings, timeout, capture/menu/focus, HUD overlap
and resize in both renderers; preserve Vanilla. See the pinned reference in
[follow-up brief](documentation/41-playtest-follow-up-plan.md).

### D08A2 — EDuke32 bottom-left inventory icon and green %

**Playtest accepted (revised scope) — 2026-09-28 evening.** Always-on bottom-left
host gadget rejected. Delivered instead: centered **[ / ]** strip with
`research/inv` icons, green palette-22 THREEBYFIVE % (`03. Green (Palette 22)`),
EDuke ARROW `tile0020` frame locked at **50×60** / `frame_dy=-6`, Enter uses
selected item (Escape pauses), direct keys skip the switcher, **M** quote-only.
EDuke CROSSHAIR `tile2523` also landed (aim display). Evidence:
[51-session-inventory-crosshair.md](documentation/51-session-inventory-crosshair.md).

**Acceptance (revised):** brackets show the picker; Enter/U use always while
captured; M is message-only; no persistent bottom-left host gadget; Vanilla
preserved.

### D08A3 - Original TTK inventory icons for the switcher

**Done (2026-10-01, user-accepted).** The switcher now uses the game's own HUD item
icons from `/DATA/FONTS.RAW`, rebuilt by `build_ttk_inv_icons.py`. See
[source, builder and evidence](documentation/75-d08a3-ttk-inventory-icons.md).

The D08A2 switcher draws its gadget icons from Duke3D/EDuke32 tiles
(`research/inv`, packed into `recomp/assets/ttk-inv-icons.pack`, format
`TTKICO2`, loaded by `recomp/src/ttk/inventory_hud.cpp`). Replace them with
Time to Kill's own item art: extract the original TTK medkit, biomask, jetpack
and steroids images (and night-vision goggles, which the switcher also shows)
from the player's disc data, render them to PNG for review, then rebuild the
icon pack from those PNGs.

Locate the art from the original HUD/menu inventory display or the pickup
sprites/models, whichever is the game's own 2D icon; record source file, offset,
CLUT/palette and any transparency handling. Steroids activate on pickup and are
not a switcher entry today; extract their icon anyway and use it only where a
steroids icon is actually displayed, without adding a new switcher item. Keep
the extracted PNGs local only (retail-derived, like `research/`); the extractor
script and provenance notes may be documented.

**Acceptance:** PNGs for each item match the in-game original art (checked
against the vanilla inventory display or pickups); the switcher shows TTK icons
at readable size in the locked 50x60 cell with the green % and ARROW frame
unchanged; the pack builder is reproducible from the disc; a missing pack still
falls back cleanly; Vanilla unchanged. User confirms the look in play.

### D08B — Broader traversal and scripted-camera coverage

**Done on user acceptance (2026-09-29).** User: "d08b can be marked as
completely done man, its fine. accept it all" and "i have tested all of
them!". The user reports playtesting every limit recorded below; they are
kept as history only.

**Latest user feedback supersedes the earlier replay:** contact jumps remain
inconsistent; running off the bed still has residual slowdown; early pipe-bomb
pickup from the unopened bed is reproducible by the user again. Switch targeting
is awkward. Focused follow-up is D08H; none of these are declared fixed.

**Earlier furniture iteration — bounded automated evidence.** Close-contact bed jumps
recheck clearance while rising; normal walking can step off small furniture drops;
held movement survives short-drop landing. Final replay verifies bed contact entry,
bed/couch walk-off and continuous Shift-run landing. Central couch entry works;
outer-end/oblique approaches and the reported unopened-bed pipe-bomb pickup remain
open. See [furniture handoff](documentation/37-furniture-traversal.md). D08B remains
In progress for these limits and its broader campaign coverage.

**In progress — bounded fixes delivered 2026-09-27.** Attached input now checks
live gameplay context; E-owned redraw survives pause/inventory; verified attached
states can automatically recapture after pause. Final isolated replay verifies
recapture without F10, descent and bottom-exit redraw. A later ascent with a
pig cop on the landing stalls despite correct Up input; keep that obstruction
case open. See [implementation and evidence](documentation/35-traversal-and-apartment-followup.md).

The bounded standing/walking directional-takeoff follow-up is tracked as **D08C**;
its evidence does not close this broader job.

Carry forward the unverified portion of the original D08 scope: other ladder and
ledge types, oblique/diagonal approaches, standing/walking jumps, crouch/roll,
swimming/underwater, scripted cameras, other maps, and transitions through death,
respawn, inventory and level changes. Preserve the accepted D08 implementation.

**Acceptance:** expand the [state/ownership matrix](documentation/25-traversal-state-matrix.md)
with actual scenario evidence; required progression remains possible and modern
camera/input returns correctly after original-owned states. Repeat the first
security-card route with the delivered controls and retain unresolved scene-exit
and campaign/stability follow-ups. Prior successful user progression remains
valid evidence; it does not establish the earlier freeze's exact cause. Coordinate
full campaign/stability/audio coverage with D22/D23/D18.

### D08C — Directional jumps from standstill — accepted

**Done on explicit user acceptance (2026-09-27).** The user first confirmed
W→Space worked, then accepted the Space→W follow-up: “its working!! beautiful.”
This closes the bounded standstill/walking directional-jump and input-order iteration.

W/A/S/D (including diagonals) may precede Space or arrive during original takeoff
preparation after Space. No run-up is required. The first chosen direction persists
through early release; Space alone remains vertical. Original preparation is not
restarted, and original clearance, directional-jump arc, gravity and collision
remain. Running-jump selection, E interaction-only, C holster, precision aim,
wheel/Alt-wheel, Vanilla and customized bindings remain regression requirements.

Accepted delivery: `92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.
[Current engineering contract](documentation/34-standing-directional-jumps.md),
[final automated and human evidence](documentation/reports/d08c-input-order.json),
and [earlier implementation evidence](documentation/reports/d08c-standing-jumps.json).

Acceptance does not claim every diagonal, ceiling, moving support, map or ladder
was individually human-tested. Broader terrain/traversal/campaign coverage stays
under D08B. This feature accepts direction during preparation; it does not add
midair steering. Do not reopen the accepted feel/input-order work without a new
report or user-selected change. No other job is authorized by this sign-off.

### D08D — Apartment light-switch secret convenience

**Done — user confirms natural switch-first access, 2026-09-27.** Modernized removes only the
conversation prerequisite from the exact authenticated LEVEL00 lights-off
condition. Vanilla keeps the original condition. Native checks and the original
switch/bed replay verify opening with the prerequisite unset, one-time counting,
repeat toggles, subsequent dialogue and natural pipe-bomb pickup/6 selection.
The prerequisite-negative replay uses an explicit isolated conversation-bit reset;
the user has now verified untouched natural switch-first access. See
[the fixture boundary and delivery](documentation/35-traversal-and-apartment-followup.md).

User-requested Modernized convenience: switching off the apartment lights should
open the bed secret without first talking to the woman. User reports lights-off
alone did not move the bed; after turning lights on, talking, then switching off,
the bed moved and pipe bombs became accessible. Original intent/cause is unverified.

**Acceptance:** verify the exact LEVEL00 script/trigger and original prerequisites;
in Modernized, lights-off opens the secret with or without prior conversation,
including repeated toggles and already-open state. Preserve dialogue, reward,
collision and progression; Vanilla retains original behavior. Verify live script
identity before changing behavior; no guessed global flags or inventory grants.

### D08E — Responsive holstered fire and weapon transitions

**Done — user accepts held fire, release cancellation and faster transitions.**
Bound fire draws through the original action,
then fires while held. Equip/stow upper tracks advance at 1.5x with original events;
Vanilla and firing speed remain unchanged. Pistol live checks, four native suites,
57 Python tests and a 14-checkpoint Vanilla route pass. Broader weapon/era feel
remains open. See [delivery and evidence](documentation/36-weapon-response-and-furniture.md).

User requests fire to draw a manually holstered weapon, then
shoot while fire remains held, plus modestly faster draw/holster animations.
Use the bound fire action, preserve C and E ownership, original animation events,
weapon/ammo rules, attached traversal and Vanilla. Do not queue surprise shots
after release, focus loss or menus. Verify actual draw latency and no fire-rate change.

### D08G — Typed Duke-style debugging cheats

**Done for the delivered bounded scope:** user says the cheats work fine on
2026-09-27. Exact confirmation wording is now D08G1; font presentation is D19A.
Existing hidden-enemy save/load and broader-campaign limits remain documented.

User-selected 2026-09-27. Add familiar `dn...` cheats for private gameplay testing:
prioritize enemy-only hide/show (`dnmonsters`), invulnerability (`dnkroz`), keys
(`dnkeys`) and weapons/items (`dnstuff`), using verified original TTK paths where
available. Document supported equivalents and omissions; preserve Vanilla and
context/input isolation. Verify toggles, repeats, non-enemy preservation and reset.

### D08G1 — Original Duke3D cheat confirmation wording

**Done — 2026-09-27.** Private OpenGL captures verify every required label and both
toggle directions. Parsing, effects and guards are unchanged. User accepts cheat functionality and requests the original confirmation
labels, using their [InfoSuite reference](https://infosuite.duke4.net/index.php?page=references_cheats).
Change supported-code success messages, preserving capitalization and punctuation:

| Codes | Required success text |
| --- | --- |
| `dnkroz`, `dncornholio` | `God Mode: On` / `God Mode: Off` |
| `dnstuff`, `dnitems` | `Giving Everything!` |
| `dninventory` | `Got All Inventory` |
| `dnweapons` | `Got All Weapons/Ammo` |
| `dnkeys` | `Got All Keys` |
| `dnhyper` | `Steroids` |
| `dnmonsters` | `Monsters: Off` when hidden / `Monsters: On` when shown |

**Acceptance:** verify each supported result and both toggle directions on screen;
no change to code parsing, gameplay effects or context guards. Keep useful rejected
request feedback and distinct labels for the three added debug codes. Wording must
not imply that TTK gained Duke3D armor/jetpack semantics. The supplied larger list
is a reference, not authorization to advertise unsupported cheats as implemented.

### D08G2 — Silent cheat entry and centered confirmations

**Done by explicit user acceptance, 2026-09-27:** entry is silent and results centered as requested. The implementation-stage evidence below remains valid.

**Implemented and privately verified, then user-accepted.** Silent parser entry and
centered/wrapped results verified in both renderers at three sizes. See
[evidence](documentation/44-traversal-inventory-implementation.md). User wants no prefix/partial-code echo,
no cancellation chatter, and a result only after a supported complete cheat.
Retain the accepted exact labels, Duke glyphs, cheat effects and context guards.
Center confirmations horizontally in the Duke3D-style upper message area instead
of top-left. The user said “middle of the screen” and “like Duke3D”; the reference
centers horizontally, not over the vertical-center aiming reticle. Record this
placement interpretation and verify the actual captures.

**Acceptance:** valid complete codes produce one correct result; partial, invalid,
cancelled and timed-out typing stays silent. A recognized but refused request may
retain a concise truthful refusal (never a false success); no unsolicited typing
status. Preserve input isolation/reset and added debug-code result labels. Verify
both toggles/aliases, resize/wrapping/transparency/expiry, repeated messages, both
renderers and Vanilla. Do not move unrelated OSD panels or slow normal controls.

### D08H — Apartment furniture, hidden pickup and switch targeting

**Done — user accepted remaining playtest, 2026-09-28.** Movement was already
accepted 2026-09-27; remaining pickup/switch checks were accepted in ongoing
play. Broader campaign furniture stays under D08B.

**Implemented; movement portion explicitly accepted (2026-09-27).** User reports perfect close-contact
bed entry, smooth walk/run departures without the pause, and easier couch entry.
They did not explicitly retest concealed/exposed pipe bombs or switch/NPC order
in this report; do not invent those results. New run-start/edge jump misses and
armed ladder grabs are D08I/D08J, not a rejection of accepted furniture clearance.

Original scope: See [reproduction and verification](documentation/40-feedback-implementation.md). Focused child of D08B. Original request: revisit the whole apartment route: touching the bed
and W+Space from rest; tiny-step-back and running approaches; bed/couch walking and
running off edges; the remaining drop/landing slowdown; E targeting the light
switch from reasonable nearby angles; pipe bombs collected through the end/pillow
of the unopened bed. The latest user report confirms early pickup still occurs;
the earlier non-reproduction was not a fix.

**Acceptance:** close-contact jump gains forward motion as soon as physical
clearance permits, without clipping; walk/run edge departure and landing maintain
appropriate held movement smoothly. Release still stops normally; intentional
large falls/obstructions remain physical. Prevent premature hidden pipe-bomb
collection while preserving normal collection after moving the bed. Verify light
switch targeting and lights-before-conversation secret behavior together. Use fresh
private level state plus repeated open/close and varied approach tests; preserve
NPC interaction, other pickups, ladders and Vanilla. Avoid a broad pickup-radius
change or apartment coordinate patch without establishing the actual cause.

### D08I — Responsive run-start and edge jumps

**Done — explicit user acceptance (2026-09-27).** Close bed on/off jumps accepted
after repeated physical attempts; earlier automated close-bed chord comparisons
are retained as supporting evidence. Preserve six-frame edge grace/furniture
behavior. Broader campaign edge coverage remains under D08B. See
[iteration 47](documentation/47-playtest-iteration.md).

**Follow-up: Needs playtest.** Nineteen valid close-bed comparisons retained directional motion after original feet clearance; the reported intermittent lost direction did not reproduce. No speculative traversal change. Original report and hardware/input/geometry limits remain open; see [current evidence](documentation/46-turning-audio-inventory-progress.md).

**Needs playtest; reproduced, fixed and privately verified.** Run-start stride
lockout and separate six-input-frame small-edge grace are documented in
[evidence](documentation/44-traversal-inventory-implementation.md). User reports Shift+W followed a split second later by
Space sometimes fails to jump. Near a bed/platform edge, a late jump also sometimes
fails. Reproduce these separately, including outside apartment platform transfers,
and distinguish input loss, gait transitions, grounded support and airborne state.
Existing jump buffering and guarded animation handling are leads, not proven causes.
Duke3D is the responsiveness baseline; preserve accepted W→Space and Space→W,
furniture contact clearance and smooth departures.

**Acceptance:** sweep measured press offsets around standing/walking/run start,
Shift order and edge departure; record input receipt, consume/expiry, gait, support,
takeoff and velocity. A valid grounded press must survive a brief gait transition
and execute once at the earliest legal update. If a short edge-grace/input-buffer
window is needed, bound and document it separately; no double jumps, indefinite
queued jumps, wall clipping or increased jump reach by accident. Test bed/couch,
exterior platforms, obstructions, large falls, release, FPS/timing variation,
focus/menu/scene invalidation and Vanilla. Give latency evidence plus human test.

### D08J — Armed airborne ladder grabs and automatic weapon transitions

**Done — explicit user acceptance (2026-09-28).** Armed run-jump+E to the second
exterior ladder succeeded repeatedly; player cleared the job. Preserve E-owned
4× stow (ground+air) and aim-held flight lease. See
[iteration 48](documentation/48-armed-ladder-reliability.md) and
[plan](documentation/49-controls-hud-plan.md).

**Needs playtest — armed reliability pass (2026-09-28).** Player can grab while
armed but misses more often than holstered. E-owned stow now boosts 4x on ground
and in flight; jump+E refreshes the lease; shorter airborne holster pulse.
Private early-E and nominal armed attaches passed. See
[iteration 48](documentation/48-armed-ladder-reliability.md).

**Done for bounded user-tested scope, 2026-09-27:** armed run/jump/E exterior ladder transfer succeeded twice and was explicitly accepted. Broader weapon/era coverage remains limited; preserve this behavior.

**Reproduced, fixed, privately verified and user-accepted.** E-owned airborne
stow including its final blend and pending intent through weapon switches now
pass pistol/shotgun ladder, exit and ammo checks. See
[evidence](documentation/44-traversal-inventory-implementation.md). User could not run-jump with pistol drawn and press E to
catch the second ladder outside the apartment. They want climbing and transfers
to manage required stow/redraw automatically without knowing C. This extends the
accepted holstered transfer route; reproduce armed and holstered cases separately.

**Acceptance:** walk-up to first ladder and run-jump/E onto second ladder succeed
in valid contact windows with pistol drawn, holstered and during weapon transition.
Trace original ladder eligibility, interaction intent lifetime and upper-body
stow; prove the failing path before choosing a fix. Use original attachment and
weapon events, remember intended weapon and restore safely on exit. No remote
snapping, through-wall grabs, forced fire or stale E after focus/menu/scene change.
Repeat from multiple approach timings; preserve ordinary E targets, accepted
ladder transitions, movement, aiming, ammo and Vanilla. Broader weapons are tested
by materially different stow tracks rather than assumed from the pistol.

### D08L — Inertial platform edge run-off

**Done - user accepted (2026-09-30): "im happy with that!"** Measured brake removed on large ledges: the
fire-escape platform run-off dropped from ~47 to 16.5 units per frame (original
fall velocity 3058) and lost mouse look; it now keeps stride speed (9172) and
the camera through the fall. Stride spikes (14000) on small drops are capped
below the running jump. See [D08L note](documentation/66-d08l-edge-run-off.md).

**Original report, 2026-09-27 playtest.** Running off platform edges still feels like it
brakes versus Duke3D’s continuous inertial departure. Investigate original gait /
support / edge handlers versus Modernized stride bridging; deliver a bounded
Modernized change only after a failing comparison is recorded. Preserve collision,
accepted furniture drops, jump grace and Vanilla.

**Acceptance:** documented before/after velocity or feel comparison on representative
exterior platforms and apartment furniture edges; no added reach, no double jumps,
no wall clipping; Vanilla unchanged.

### D08M — Modern underwater swimming controls (foundation)

**Done — shallow/ledge + Space ascend foundation accepted, 2026-09-28.**

Case study: [55-swim-controls-research.md](documentation/55-swim-controls-research.md).
Shallow Space jump + Space-facing ledge hop closed (revisit only if needed).
Deep: Space↑ and W/S work; E mantle works. Root-bridge baseline binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`. Remaining
deep polish (A/D, Ctrl↓, mantle-only exit) → **D08O**. See
[54-swim-redesign.md](documentation/54-swim-redesign.md).

### D08O — Deep free-swim polish (strafe, Ctrl dive, mantle-only exit)

**Done — accepted by playtest 2026-09-28** (three rounds; final binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`). The user
confirmed A/D strafe, Ctrl dive + underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the shallow → subway-platform ledge jump (round-3 plain 98 arc) and the
Bio Mask item-switcher icon (`biomask-small.png`) all working. Reopen only on a
reported regression (55 §6 hard rules 7–9).

Playtest gaps on the D08M binary: A/D strafe dead; Ctrl does not descend; Space
can still exit at a ledge. Root causes and fixes (`swim.inc`, `pc_input.cpp`):

1. **A/D strafe** — A/D were injected as D-pad Left/Right, which `8005201c`
   treats as *turn* buttons (anims 71/70); the stick-X word is also turn. The
   strafe branches read the layout's **L2/R2** words (`800d1a68` → 88/91/92,
   `800d1b80` → 89/90/93). Free swim now resolves those pad bits from the
   layout tables (`bit = (ptr−800d1440)/4 − 21·pad − 1`, verified live: 8/9)
   and injects them; stick X is no longer written. Verified in-engine by pad
   injection: L2 → anim 91 sideways-left, R2 → anim 90 sideways-right.
2. **Ctrl descend (round 2, D08O.2)** — the first fix (floor-margin gate on the
   soft-Y dip) still felt dead in playtest. Root cause: once the original hands
   over to its **native swim state machine** (`800455bc`, `player+0x22c` = 4
   surface / 5 underwater, anims 122–142), `80044c18` pins body Y to the
   surface every frame, so no host Y write can dive. Ctrl now calls the
   original dive `80045564(player, player+0x60)` (→ state 5, anim 133, sound
   0x3003). Underwater the host sets body yaw/pitch (`+0x1c/+0x24`,
   `+0x1e/+0x26`, +900 up … −900 down, camera-relative) from WASD / Space /
   Ctrl and injects Square so the original thrust integrator (`8007c788` /
   `800451a0`) moves Duke; it hands back to state 4 by itself when he breaks
   the surface. Free-swim soft-Y remains only for the land-side water anims.
3. **Mantle-only exit** — free-swim `swim_try_mantle` (Space → `8003e2d0(98)`
   at a lip) removed; Space is ascend only.
4. **Wade ledge assist (D08O.3/.4, subway platforms)** — user reported the
   shallow → platform jump still fails often. Lab measurement on the west
   platform (room 21 → 57, ledge 635 above the water floor): the directed jump
   98 rises only ~560, so it landed only from a 700–1250-unit run-up; touching
   the wall it was cancelled on frame one (splash), from 300–600 units it hit
   the wall face (107) and fell back (105) — 4/5 far, 0/6 near. The standing
   probe `800797f4` reaches ~350 units, so `swim_climb_ahead` never saw the
   ledge either. Fix: `swim_ledge_ahead` repeats the standing probe from the
   current position and from 320/640/960/1280 units along the view (world XZ
   written and restored inside the query); a refused step (result 1) with floor
   256–1792 above the feet is a ledge. Space then launches the ordinary
   directed **98** toward it (D08O.4 — the first cut launched vertical 97 and
   switched to 98 at the lip; the user found that hop far too high and asked
   for the plain jump back). The 98 keeps its own arc shape but takes off with
   VY −9000 instead of ≈−6900 (`delayed_vertical` applied at the `8003ebf4`
   ballistic hook, as the apartment-bed fix does; 98 integrates ≈VY/96 per
   tick, so the rise is ≈840 instead of ≈560 — about a head taller, enough for
   the 635 ledge with margin). `swim_ledge_ballistic` then owns XZ each tick:
   within one probe step of the wall (offset 0/320) XZ is held at zero until
   the animated foot (`+0x10`) is 32 above the ledge top, then 3400/4000,
   because 98's first stride into the wall face is exactly what bumped (107) or
   cancelled; further back XZ is 1900 + 3.5/unit (max 6400 ≈ 98's own 6007) from
   takeoff so Duke lands just past the lip. Result on the final code:
   **15/15** lab jumps landed on the platform (`ledge18`; plus 13/15 on the
   previous cut, both misses the near-oblique bump this hold fixes) — at the
   wall, 350–1350 units back, standing or running, Space alone or with W,
   headings up to ±40° off perpendicular. Beyond ~1650 the ledge is out of probe
   reach and the plain 98 falls short (too far to see). At a plain wall (no
   floor ahead) W+Space gives the vertical hop instead of the cancelled splash.
   A stale-assist bug (pending flag surviving a landing) was fixed at the same
   time: the flag now drops whenever anim ≠ 98 or state ≠ 9. Debug counter
   `ledge_assists`; `DNTTK_SWIM_TRACE=1` prints `ttk-ledge probe/launch/clear/
   abort/ballistic` and `ttk-swim` per-tick lines.

Hard rules unchanged (never invent `+0x20c`, never clear entry `+0x1f8`, never
primary world XYZ — the probe offset is a restored query, not a move). Full
contract + checklist in
[55-swim-controls-research.md](documentation/55-swim-controls-research.md) §1.2–§9.

Verified on the final binary: ttk-controls-test + ttk-input-test PASS; shallow
subway water — W/S/A/D camera-relative wading, Space → 98 in open water,
Ctrl → 105; deep zone 35 (teleport) — Ctrl dives (133 → state 5), W/A/S/D
thrust camera-relative (128), look-up + W rises, Ctrl descends, Space thrusts up
and auto-surfaces to 122/state 4, surface W = 125, A = 123; west platform ledge
15/15 as above. Playtest accepted the swimming (A/D strafe, Ctrl dive, underwater
WASD/Space/Ctrl, mantle exit) and the Bio Mask icon; playtest 3 accepted the round-3 ledge jump. Binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.

**Acceptance:** A/D strafe; Ctrl dives and Duke swims underwater where he looks
(W/A/S/D), Space ascends/surfaces; free-swim Space = ascend only; exit via
mantle/E; shallow → subway platform jump lands reliably from the wall or a
run-up without a distance puzzle; no regression on D08M shallow/oxygen; Vanilla
unchanged. **All accepted 2026-09-28.**

### D08N — Duke3D-style scuba gear item

**Cancelled — out of scope, 2026-09-28.** The user directed that a scuba device
must not be added to this game. Do not reopen, prototype, or treat this as a
follow-on to swimming. TTK has automatic underwater air and no scuba gadget;
that remains the product. Bio Mask stays TTK's gas mask. No auto-equip scuba,
breath extension, `SCUBA GEAR ON` toast, inventory slot, or HRP scuba art.

**Acceptance:** this ID stays cancelled. Underwater air is original and
automatic in both modes. Do not add a scuba item.

### D08P — Crystal-2 turret / scripted-camera control recovery

**Done — 2026-09-29 (iteration 6, user playtest: "it really works. perfectly").** The user showed it is the
whole flooded-corridor ledge area (**F7 UI slot 3**), not depth: run, mouse
and jump die and never return. Their session log had `ident=0` from the
ledge onward; a live per-guard diff found the cause — the LEVEL00 zone
script writes a hit position into the overlay's trailing 16-byte scratch
vector (`0x800ccf1c`), which the 9668-byte overlay identity guard covered.
The guard is now the 9652-byte code/table body. In the same water (depth
512) the original swaps all gaits for wade clips 80/81 at 8–14 units/frame,
forward only; their handler `0x800539f8` is now hooked (new generated
entry, three new guards) and its root retargeted to camera-relative WASD in
the land run band: 45–58 units/frame, mouse steering, strafe, Space jump,
0 identity refusals across and past the ledge; slot 2 regression clean.
Wading straight into a wall idles as in the original (turn to resume).
Iterations 4–5 (waist-deep land lease, held keys across recapture, fresh
capture offers) stand.
See [56-scripted-camera-controls.md](documentation/56-scripted-camera-controls.md).

**Acceptance:** F7 slot 3 ledge: run forward along and past it with run,
mouse and jump intact, mid-depth water at run pace with mouse steering;
F7 slot 2 wade uses dry-ground mouse look, camera-relative
WASD and run speed; leaving the water keeps mouse and Shift-run even when
Shift was held through the load/pause; the pause menu is not captured;
Escape / F7 recapture still work; true deep swim and Vanilla unchanged. If
it still degrades, report the on-screen reason and the session log.

### D08Q — Modern jetpack flight controls

**Done - 2026-09-29 (user-accepted):** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."

User report: "when using the jetpack i
cannot control it at all aside from spacebar to ascend. wasd are blocked,
modern controls fail here completely." Confirmed on `79a3fd5b…` in an
isolated probe: original mode 10 (anims 163–170) was outside every lease,
so the mouse was dead, WASD only reached the original through the tank
fallback (Up/Down/L2/R2 relative to a body heading the player could not
turn) and the original's idle gravity sank Duke to a landing within a
second or two. Ctrl meant nothing.

Implemented as a host layer over the unchanged original handler
(`recomp/src/ttk/jetpack.inc`, camera-only lease in `state()`, pad bridge in
`input_pad()`, J gate in `select_weapon`): mouse orbit with the body facing
the view (`face_view`), so the original's body-relative thrust is
camera-relative — W/S → original Up/Down, A/D → the layout's strafe pads
(L2/R2); Space stays the original Square climb; **idle hovers** (host sets
the original hover lock `+0x224 & 0x08000000` at the current height; the
original drains fuel while hovering too); **Ctrl descends** at a steady
~48 units/update (hover lock released, ballistic `+0x1f8 = 2800`); WASD
flight releases the lock (under it the original only moves Duke by the raw
thrust vector, ~10 units/update) and pre-loads `+0x1e8 = 18` against the
handler's directional lift so flight stays level; J in flight is the
original inventory toggle → 108 fall, and the mouse camera survives that
fall until the landing. Three new code guards (`8004aaf8`, `8004ac08`,
`8004ade0`). No new generated hook. Vanilla untouched (all writes gated on
Modernized + capture + lease + identity).

Isolated Xvfb probe (level 1 turret room and ledge states, `dninventory`):
mouse turns body and camera together (`looks` 0→1); W/S/A/D headings
−33/154/−120/61° against camera −30° (expected −30/150/−120/60); level
flight ±25 units over 55 frames; W after a 100° mouse turn follows the new
view; speeds 25/29/24/14 units per frame (R2 strafe is the slower original
side); hover drift ≤ 32 over 60 frames; Ctrl 489 units in 30 frames, then
landing anim 105; Space climb 540–850 in 30 frames; J off → 108 fall with
working mouse → 105 → ground run at 45.7 with the land lease.
ttk-input-test (new jetpack pad case), ttk-aim-test, ttk-controls-test
PASS. Binary `481dd2ec…`; current player build (with the D23A frame-budget
fix) `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

Known/unverified: releasing WASD stops almost at once (hover engages) —
may feel abrupt; after Ctrl release Duke settles another ~130–300 units
before holding (the original's floor-approach in `8004ac08` pulls toward
~0x200 above a nearby floor); strafe is the original's half gain; fuel
drains while hovering (original rule); no probe of low ceilings, water
below, damage or death in flight. See
[57-jetpack-controls.md](documentation/57-jetpack-controls.md).

**Acceptance:** with the jetpack on (J), Space lifts off; in the air the
mouse turns the camera and Duke, WASD flies relative to the camera at a
useful speed and level height, releasing everything hovers, Ctrl descends
until a soft landing, Space climbs, J switches off in the air with a
controlled fall and the mouse still live, and the ground controls resume on
landing; fuel-out behaves the same as J; Vanilla flight unchanged.

### D08Q2 - Jetpack unavailable after death, Continue and dnstuff

**Done (2026-10-04, user-accepted):** "confirmed fixed! accepted."

The user died, chose
Continue, returned to gameplay, entered `dnstuff`, and then could not use the
jetpack. The inventory picker also skips the jetpack, preventing selection.
The user saved this failing state in **UI slot 10 (file 09)**. Investigation and
implementation were subsequently selected and authorized on 2026-10-04.

**Cause and repair:** death during deployment can strand the original jetpack
pending bit across Continue; the original inventory grant preserves it. The
reported save has full fuel and a closed/off pack with that bit still set.
The bounded Modernized repair clears only pending at that inactive endpoint.
Slot 10, a controlled interrupted-deployment/death/Continue route, both flight
schemes, fuel restrictions and save/load pass. The user confirms the repair
and accepts this job. A natural map-pickup replay was not performed in the
automated work; acceptance does not add that evidence. See [evidence and limits](documentation/90-d08q2-jetpack-continue.md).

**Reproduction/evidence:** private snapshot
`recomp/analysis/d08q2-jetpack-20261004/cards-reported`, with a SHA-256/mtime
manifest alongside it. Slot 10 SHA-256: `161253aa41f4f2c407f57ef7cff1371ba7889c4bdeba793183f9045b6184e18e`.
The snapshot preserves the resulting state, not necessarily the preceding
death/Continue transition. Reproduce that sequence separately on private
cards/settings, comparing a working jetpack state, before/after Continue,
and before/after `dnstuff`. Never write the player's cards or saved state.

**Investigation:** trace inventory ownership/quantity/fuel, selected item,
picker eligibility and activation gates; examine Continue/reset and cheat
grant paths. Check whether the cheat is acknowledged, whether its changes
persist, and whether direct activation and cycling disagree. Distinguish
retail restrictions from Modernized bridge state and save/load effects.
Keep modern/classic flight controls and the accepted renderer unchanged.

**Acceptance:** identify the responsible state/path and make a bounded repair
if justified. Verify slot 10 plus a fresh death/Continue/`dnstuff` sequence,
normal acquisition, picker selection, activation/deactivation, fuel handling
and save/load. Preserve legitimate empty/unavailable-item restrictions and
Vanilla behavior; confirm working Modern and Classic jetpack controls. User
confirms restored use.

### D08Q3 - Jetpack weapon aiming and missing crosshair

**Done (user-accepted 2026-10-04):** "i accept this! great work."
Guarded flight now admits view aiming, weapon presentation, beam completion
and the enabled crosshair. Both flight schemes pass private six-weapon firing
and transition checks; native suites and build pass. Accepted implementation:
`f284dc1`. Recorded live coverage limits remain documented. [Implementation and evidence](documentation/91-d08q3-jetpack-aim.md).

Original report after accepting D08Q2:
"i cant aim to shoot when using the jetpack / crosshair is also gone."
The private slot-10 intake now reproduces the baseline failure and verifies the candidate.

**Scope:** restore usable Modernized weapon aiming during jetpack flight and
show the enabled crosshair. Inspect flight-state eligibility in the weapon-aim
and reticle paths, original airborne weapon restrictions, and transitions into
and out of flight. Diagnose aiming and crosshair visibility separately; do not
assume that showing a reticle alone restores shots aimed at the view.

**Intake evidence (superseded by the verification above):** user report only;
cause and exact reproduction were initially unknown. D08Q2's slot-10 snapshot
was initially available for regression testing only.
Use private cards/settings; verify active overlays before changing hooks.

**Acceptance:** while using either Modern or Classic jetpack controls, the
player can aim and fire supported weapons with shots following the established
Modernized view-aim behavior. The enabled crosshair remains visible and meaningful
in flight; the disabled setting stays respected. Verify hovering, moving,
ascending/descending, mouse yaw/pitch, on/off, landing and fuel-out, with
representative hitscan and projectile weapons. Distinguish intentional original
weapon restrictions from defects and document any remaining unsupported cases.
Preserve flight controls, existing first-person/third-person flight transitions,
ground aiming, D08Q2's accepted recovery and Vanilla behavior. User confirms
restored aiming and crosshair. Do not reopen D08Q2 for this separate report.

### D08R - Selectable jetpack scheme: Modern / Classic (WASD)

**Done on user acceptance (2026-09-29):** "it's absolutely rock solid."

**Revision 2 (2026-09-29).** `run.py --jetpack classic|modern`
persists a Modernized `jetpack` control (schema 11, default `modern`). Per user
direction Classic keeps every modern control (mouse facing, camera-relative
WASD, J) and uses the original burst physics: Space boost, gravity, no host
hover/Ctrl descent. Revision 1 (original camera + D-pad turning) was rejected
in playtest: mouse turning failed and controls "slipped" out of modern. See
[D08R section](documentation/57-jetpack-controls.md#d08r---selectable-jetpack-scheme-modern--classic).

**Todo.** User (2026-09-29): Modernized players should be able to choose
classic jetpack controls instead of the D08Q modern flight, and classic must
still work with WASD.

**Scope revised 2026-09-29:** ship this as a CLI switch only. The scheme
choice is one of many customizations that will later live in the D19
in-game customization menu (Sonic 3 A.I.R.-style), not a jetpack-specific
screen. Do not build menu UI for it here.

Scope: add a persisted Modernized profile setting (for example `jetpack` =
`modern` | `classic`) in `player_profiles.py`, a `run.py` flag and the
environment passed to the runtime. The terminal settings menu may list it
only if that is trivial; in-game menu exposure belongs to D19. **Modern** is the
current D08Q behaviour and stays the default. **Classic** runs the original
TTK mode-10 flight as the game intended: body-relative thrust, original
hover toggle, original lift and gravity, no host hover-on-release, no Ctrl
descent and no `face_view` camera coupling. The only change is WASD mapped
to the original pads (W/S to Up/Down, A/D to turn or strafe per the layout)
so it is playable without the tank fallback surprises D08Q found (body
heading not turnable, idle sink). Decide and document whether the mouse
camera stays live in Classic (camera-only lease) or the original camera is
used. Vanilla is untouched in either case.

**Acceptance:** the setting persists and can be switched from the CLI;
Classic flight is controllable with WASD + Space and matches the
original's thrust, hover toggle, lift, fuel and cut-out rules; Modern is
unchanged from D08Q; switching schemes needs no rebuild; Vanilla flight
unchanged. Tests cover the profile migration and the pad mapping.

### D08Q1 - Faster Modern jetpack Ctrl descent (underwater dive speed)

**Done on user acceptance (2026-09-29):** "verified working!!!"

`k_jet_descend_velocity` 2800 -> 5500: steady
Modern Ctrl descent 29.5 units/frame (was ~16), matching the underwater dive;
soft landings, no health loss. See
[D08Q1 section](documentation/57-jetpack-controls.md#d08q1---faster-modern-ctrl-descent).

User (2026-09-29): "for jetpack modern, i'd like ctrl to
descend be faster. basically the speed of ctrl in water to descend".

Scope: Modern scheme only; raise the host Ctrl descent velocity
(`k_jet_descend_velocity`, `jetpack.inc`) so steady descent matches the
underwater Ctrl dive (~29 units/frame measured in the D08O deep-water lab,
`/tmp/swimlab/deep4.log`: y -1438 -> -888 over 19 frames). Classic, Vanilla,
hover, WASD flight and Space unchanged.

**Acceptance:** Modern Ctrl descent measured near the underwater dive rate;
landing still ends in the original soft landing without extra damage; Classic
and the other Modern inputs unchanged; user confirms the feel.

### D08S - Duke3D-style jetpack scheme (instant J on/off, midair)

**Cancelled 2026-09-29 (may revisit).** User: happy with the current Modern and
Classic jetpack schemes (D08Q, D08Q1, D08R); no Duke3D scheme for now. Nothing was
implemented. The scope below is kept in case it is reopened.

**Original scope (2026-09-29).** User (2026-09-29): a third jetpack scheme that replicates
Duke Nukem 3D. **J** starts flying immediately and pressing **J** again stops
flying immediately - no graceful lift-off, landing or cut-out animation.
It must also work **in midair** (while jumping or falling), which TTK does
not allow: the original entry `8004aaf8` requires Square on the ground and
rejects the airborne/landing states (anim 105/106 and the `+0x224 & 0x241`
flags). Controls while flying: **Space** ascends, **Ctrl** descends,
**WASD** moves relative to the view, mouse turns camera and Duke together.
No input holds position (Duke3D hover).

Research first: whether mode 10 can be entered directly from airborne
modes with the original handler kept intact, or whether this scheme needs a
host flight model that drives the root position and state flags itself;
what the instant J-off should do (Duke3D drops straight into a fall);
fuel drain rules (Duke3D drains while the pack is on); animation choice
for instant take-off; collision with ceilings and water. Keep all writes
gated on Modernized + the selected scheme; Vanilla and the other two
schemes unchanged.

**Acceptance:** with the Duke3D scheme selected, J toggles flight instantly
on the ground and in midair; Space/Ctrl climb and descend at steady rates;
WASD flies relative to the view; releasing input holds position; J off drops
Duke into a normal fall with controls live; fuel-out behaves like J off;
Modern, Classic and Vanilla unchanged.

### D08K — True crouch walking and animation feasibility

**Blocked after original-asset/collision audit:** no validated low-walk gait/override
pipeline and movement collision still uses standing clearance. See
[concrete findings and required next work](documentation/43-crouch-walk-audit.md).
No substitute slow-roll/sliding prototype was shipped. User wants held
crouch plus direction to walk low, not automatically roll. Treat this as a new
locomotion feature, not proof the present fast crouch transition is wrong. Inspect
original TTK lower-body animation clips, skeleton/event tables, collision height,
root motion and roll handlers; IDs labelled movement in source are not evidence
of an existing usable crouch-walk animation.

**Acceptance:** document available original animation assets and whether a viable
loop can be reused/blended or a new non-destructive local animation override is
needed. Deliver a bounded Modernized prototype if feasible: hold crouch to remain
low and move slowly in all directions with coherent body/feet, collision/headroom,
aiming/firing and safe stand-up. No automatic roll for ordinary crouch+direction;
retain original roll in Vanilla and document any separate Modernized roll binding
without replacing custom bindings. Test low ceilings, edges/stairs, release,
weapon/traversal interruption and camera. Do not promise a new skeletal animation
before verifying the asset pipeline; record concrete blockers if no safe prototype.

### D08T - Pushable objects: modern grab, push/pull and climb

**Opened 2026-09-30 from a user bug report.** In the first map's alley the
green dumpster is meant to be pushable, but in Modernized (third or first
person) Duke never grabs it: wedging into it and holding **E** does nothing
or climbs on top. On DuckStation the user could almost start the push
animation, then Duke climbed instead. The user wants both actions - push
and mount - with a modern, easy control that works for every pushable
object, not only the dumpster. Evidence: a private copy of the user's
slot-5 savestate (UI slot 5, in front of the dumpster).

**Acceptance:** in Modernized, Duke can grab any original pushable object
(object type flag `0x08000000`), push and pull it with camera-relative
movement and let go, without accidentally climbing; he can still climb
onto a climbable pushable object with a separate, obvious input; mouse
look stays live while grabbing; Vanilla keeps the original Action rules.

**2026-09-30 - Needs playtest.** Original rules traced and replayed from a
private copy of the slot-5 state: holstered, still, a fresh Action against an
object flagged `0x08000000` grabs it (anim 121); Action held + Up pushes (120),
+ Down pulls (119); any direction held with Action climbs a climbable one
(`0x80051cf0`, 139/140). The dumpster is both. Modernized failed because E's
Action dropped as soon as the grab began (no lease for 119..121) and W + E
took the climb branch. Implemented (`recomp/src/ttk/push.inc`, `pc_input.cpp`,
new generated hook `0x80051cf0`, three new SHA guards): **E** grabs any
pushable object even with W held; the grab latches; **W/S** push/pull relative
to the camera; **E** lets go; **Space** climbs a climbable one; mouse look
stays live; first person blends to the orbit while grabbing. Real-key Xvfb
checks in third and first person: grab without climbing, push 2582 -> 3713,
pull 2582 -> 1668, camera-facing W pulls, Space climbs to the top with
automatic holster and redraw, 59.5 fps. Vanilla push unchanged; Vanilla route
exit 0 with reviewed captures; native suites PASS. Limits: only the dumpster
checked; original line-up angle and push speed unchanged; Space next to a
climbable pushable object climbs instead of jumping. Details:
[documentation/64-d08t-pushable-objects.md](documentation/64-d08t-pushable-objects.md).
Binary `941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

**2026-09-30 - Done (user-accepted).** User playtest: "it works so much better than the original now. this is it rock solid. confidence level is very high." Remaining limits above stay recorded (only the dumpster checked).

### D08T1 - Separate push/pull manipulation from mantling

**Done (user-accepted, 2026-09-30).** E mantles pushable objects like any other;
holding Grab (RMB, or Alt; Alt only with legacy aiming, where RMB stays
precision aim) grabs, W/S push/pull, release lets go. Schema 17. Details and
evidence: [documentation/70-d08t1-grab-manipulate.md](documentation/70-d08t1-grab-manipulate.md)
and the 2026-09-30 D08T1 work log.

**User brief (2026-09-30), backlog only:** the accepted D08T controls make an
invisible object property change the traversal controls. On a pushable object
**E** grabs and **Space** climbs, while everywhere else **E** mantles. The
user wants pushability to add a verb, not remap one: **E** always mantles (a
pushable object mantles exactly like an equivalent non-pushable one), and a
new rebindable **Grab / Manipulate** action, **held**, grabs the object:
**W/S** push/pull while held, and releasing it lets go. There is no toggle and
no E LET GO. The default is **right mouse** in Modernized. The user decided
Modernized does not need right-mouse precision aim, since view aiming already
covers it. Vanilla keeps precision aim. Proposed: Alt is a second Grab
binding everywhere in Modernized, and the only one when legacy `original`
weapon aiming keeps RMB as precision aim (confirm at job start). Full
brief, including the other-input and interruption cases, the HUD text
(`W/S PUSH/PULL - RELEASE RMB TO LET GO`, using the live binding label if
possible) and the test list:
[documentation/69-d08t1-grab-manipulate-brief.md](documentation/69-d08t1-grab-manipulate-brief.md).

**Before implementing:** document how the current E/Space exception is built
(`recomp/src/ttk/push.inc`, `pc_input.cpp`, hook `0x80051cf0`). Audit every
Mouse2 / `original_aim` path in Modernized, including the D08Q aim-held
flight lease. Mantleable and manipulable stay independent capabilities, with
no dumpster-specific special case. Also check whether E while grabbing can
release and mantle cleanly. Release RMB then E must mantle at minimum.

**Acceptance:** normal and pushable objects both mantle with E; hold RMB
grabs, W/S push/pull with the existing D08T mechanics, and release disengages;
grab -> push -> release -> mantle and mantle -> leave -> grab -> pull ->
release repeat without stale state; RMB against ordinary geometry does
nothing odd; obstruction, damage, falling/separation, object loss and pause
end the grab cleanly. Third and first person. Vanilla unchanged.

### D08U - Top-of-ladder mount: grab a ladder from a platform and climb down

**Done (2026-10-01, user-accepted).** E at a ladder top lowers Duke onto it through
the original hang-to-ladder transfer; S climbs all the way down and steps off.
The original has no top mount for plain ladders. See
[research, change and evidence](documentation/72-d08u-ladder-top.md).

**User report (2026-09-30, during the D13 playtest):** standing on a platform
beside the top of a ladder that must be descended, there is no control or
mechanism that makes Duke latch onto the ladder and climb down. Reproduction:
the user's savestate **slot 12**, saved right next to that ladder. Test only on
a private copy of that state; never load or write the player's own cards or
slots in place.

The user wants it to feel "very fluid, natural, modern", to the standard of the
accepted traversal work (D08, D08B, D08J, D08T). Investigate first: whether the
original game has a top-of-ladder mount (a routine, animation or trigger volume)
that Modernized input fails to reach, or whether one has to be built from the
original climb state and animations. Record the ladder's location and level
from slot 12.

**Acceptance:** from the slot-12 platform, walking or backing toward the ladder
top with a simple, discoverable input (for example move toward the edge at
the ladder and/or the interact key) turns Duke to face the ladder and puts him
on it smoothly: no pop, fall, damage or camera snap. He then climbs down with
the existing ladder controls and dismounts at the bottom, redrawing a weapon as
D08J does. The input must not grab accidentally when walking past, and it must
not interfere with ordinary edge run-off (D08L) or jumps. Cover third and
first person. Vanilla keeps its original behavior. Check the other ladders
already known from D08B/D08J to see whether the same mechanism applies there,
and record any ladder that it does not cover.

### D08V - Sewer mantle/hang modern-control coverage (slot 12 area)

**Done (user-accepted, 2026-09-30).** Mantles, hangs, pull-ups, the frames after
them and unowned falls keep the mouse camera and modern buttons; the original
routines are unchanged. Reproduced sweep: tank fallbacks 12 -> 0, mantles
identical. Details: [documentation/71-d08v-sewer-mantle-lease.md](documentation/71-d08v-sewer-mantle-lease.md).

**User report (2026-09-30, D13 playtest, 4x exclusive):** mantling around the
sewers near the slot-12 save is glitchy: controls keep switching in and out of
modern mode, and the user wants them to stay modern. Mantling also felt
slightly harder, but the user was unsure. Session log
`recomp/build-local/logs/session-20260930-112050.log` has 54 "WASD tank
fallback / Modern movement lease resumed" pairs. The lease was dropped for
`state` in anim 139/140/142 (mode 8/8, 8/0, 8/6), 107/108 (mode 9), and
63/76/78 (0/8, 0/9). Resolution cannot cause this: the lease and the
simulation do not depend on internal scale, and the user measured a steady
60 fps.

**Acceptance:** identify these states on a private copy of slot 12 (never the
player's own slot). Extend the guarded lease so that modern movement, camera
and Shift stay in control through the sewer mantles, hangs and pull-ups
without flicker, while the original traversal routines stay intact. No
`ORIGINAL MOVEMENT` banner during ordinary sewer traversal. Compare mantle
success before and after. Vanilla is unchanged.

### D08W - Subway shallow-water sideways jumps (A/D + Space jumps forward)

**Done (2026-10-01, user-accepted).** Wade jumps now go in the held
camera-relative direction; the old boost kept the original launch's facing.
See [cause and evidence](documentation/74-d08w-wade-jump-direction.md).

**User report (2026-09-30):** in the shallow water of the first subway area,
running forward and then strafing (A or D, with Shift to run) and pressing
Space always makes Duke jump **forward**. A sideways jump is not possible
there at all. The user noted this to fix later.

Context to check first: D08C made directional jumps work from standstill and
while moving on dry ground. The crystal-2 flooded turret corridor wade (see the
Modernized movement notes in the manual) uses its own wade handling. The
subway's shallow water probably takes another wade/shallow-water path whose
jump uses Duke's facing instead of the held move direction, or where the
strafe input is not carried into the jump. Reproduce on a private savestate at
the start of the subway water. Never use the player's own slots.

**Acceptance:** in the subway's shallow water, W/A/S/D (with and without
Shift) plus Space jumps in the held camera-relative direction, with the same
feel as a dry-ground directional jump (D08C, D08I), including a pure
sideways jump while running forward and then strafing. Forward jumps and
original water behavior (slowdown, splashes, sounds) stay intact. Check the
crystal-2 wade for regressions. Vanilla is unchanged.

### D08X - Hold-E airborne ledge grab and mantle

**Done (2026-10-01, user-accepted).** Crate mantle mid-jump added for the boxes room. Ledge grabs accepted by the user ("the confidence level ... is 100%" from below a ledge). Follow-up requested: a Shift-run jump with E held into a box in the boxes room (user UI slot 11) bounces off instead of mantling onto it mid-jump. Original note: E held through a jump opens the original
reach at takeoff instead of at its late animation event, so a close ledge in
reach is caught instead of bounced off. See
[rules, change and evidence](documentation/73-d08x-ledge-grab.md).

**User request (2026-09-30):** in Modernized, holding **E** while jumping
toward a mantle-able ledge at grabbing height should make Duke catch the
ledge and pull himself up, the same way run-jump + E already catches a ladder
(D08J). The user wants it to feel fluid and modern, and to work with a high
degree of confidence: if the ledge is in reach and E is held, Duke grabs it.

Context to check first: the original game already has ledge hang and mantle
animations (134-142) and the 6/7 hanging dispatches, guarded separately from
camera-relative movement (see `documentation/26-d08-movement-followup.md`).
D08O reached a mantle from deep water with E, and D08V covers the sewer
mantle lease flicker. Trace the original ledge eligibility first: which
heights, distances, facing and velocity windows the original grab accepts,
whether it is automatic or button-driven, and why a Modernized jump misses
it (camera-relative facing, held move direction, speed, or the grab window
timing). Prove the failing path before choosing a fix; use the original
hang/mantle routines and animations rather than inventing a new climb.

**Acceptance:** in Modernized, a standing, walking or running jump toward a
ledge at grabbing height with E held (pressed before or during the jump)
reliably catches the ledge and mantles up with no pop, clip, damage or camera
snap, in third and first person. Grabbing works while armed, with the
automatic stow/redraw of D08J. E held into a ledge that is out of reach, or
into a wall with no ledge, does nothing odd; jumping without E keeps the
current behavior. It must not grab through walls or from remote distances,
and it must not break ladder grabs (D08J, D08U), pushable-object grabs
(D08T), ordinary E targets, edge run-off (D08L) or jumps (D08C, D08I).
Record grab success over repeated attempts on several representative ledges
(apartment exterior, sewer slot-12 area, crystal-2) and list any ledge type
that is not covered. Test only on private savestate copies. Vanilla is
unchanged.

### D08Y - Gap jump dead band: jump-mantle level-geometry ledges (slot 5 gap)

**Done (2026-10-01, user-accepted):** the slot-5 jump is made consistently, and
"the stuttering and audio issues, and freezeframes are now fixed" (round 5
build). Round 4: The remaining random freezes
(also without jumping) were the emulated PlayStation CPU running out of budget in
views the Modernized camera and widescreen reveal; Modernized now runs the
emulated CPU at 150% (setting, Vanilla stock): 144/59 slow frames to 0 on the
player's GPU. Round 3: Freeze frames during E
reaches (every reach frame ~50 ms) fixed with a per-update retry budget and
cheaper isolated calls (now 33 ms like any frame); the jump-mantle pop (a
~400-unit snap) is now a short glide. Round 2: root cause found: Modernized (D08I)
had switched off the original's run-jump postponement, which launches a jump
pressed near a gap from the lip; the levels are built around it. Before a large
drop (over 768) the original now decides again, and a tapped Space is held for
the queued jump; furniture and steps keep the immediate jump. Slot 5 without E
8/10 clean landings (was 0/10), E 14/14 across. Playtest 1 hitch (up to 27 ms
per reach update) fixed by cheaper isolated calls and fewer D08Y retries (~6.5
ms). D08Y jump mantles stay as the safety net. See
[research, change and evidence](documentation/77-d08y-gap-jump-research.md).

**User report (2026-10-01):** in UI save slot 5 Duke faces a gap to a second
platform at the same height. A running jump from the end of the walkway
rarely makes it: Duke "comically hits his head on the other platform and just
falls down". Starting the jump 3-4 feet earlier with E held catches the edge
and mantles. The user sees this as a level-design oversight and wants high
confidence on this jump.

Research (private copy of slot 5, real keys): without E the jump never crosses
(0/10; from the edge his feet are only 34..57 below the far top when he hits
its face, and the original bounces). With E, takeoff 0..200 units before the
edge lands across and 700..900 catches the ledge, but 300..600 is a dead band:
feet 270..540 below the top, too low to land and too high for the hang
acquisition, so he bounces whatever he holds. D08X's mid-air height mantle
covers only climbable objects (crates), not level-geometry ledges like this
one.

Planned approach: extend the D08X jump mantle to geometry ledges (the original
ground mantle heights 134..137/139, after the original line-up), and consider a
Modernized low-lip step-up landing (top at most about 0x100 above the feet) so
the from-the-edge jump without E also lands. Lowered hang retries are the
fallback.

**Acceptance:** on a private copy of slot 5, running jumps with E held from
any takeoff point 0..900 units before the edge end on the far platform (land,
mantle, or catch and pull-up), holstered and armed. Without E, at least the
from-the-edge jump no longer bounces off a lip of under about 0x100 if the
step-up is adopted. No false mantles onto tall walls. Slot-12 wall grab,
crate mantles, ladder transfers, ladder-top mount and D08L edge run-off are
unchanged. Vanilla is unchanged. Generic rule, no slot-5-specific data.

### D08Z - Optional manual modern jump (player-timed takeoff, air control)

**Done (2026-10-01, user-accepted: "i love it. lock it in").** `run.py --jump manual` (Modernized; assisted
stays the default): Duke leaves on the press everywhere, a jump pressed up to
~0.15 s after running off an edge still launches, WASD steer in the air
(camera-relative, never above the takeoff speed, also during the E reach), and
standing/walking jumps leave the ground ~3x sooner (5..8 fields instead of
14..18). Slot-5 gap without E: lands across when pressed in the last ~200
units or up to 10 fields after the edge, low-lip scramble when 200..600 early;
with E a mantle from 1000 back. Regression routes match assisted. See
[measurements, change and evidence](documentation/78-d08z-manual-jump.md).

User (2026-10-01, at selection): the long jump feels "on the rails", "not me",
"off in the hand"; wants to explore options for a much more modern feel.

**Original backlog note.** User: the D08Y jump makes the slot-5 gap
reliably, but it feels "on the rails", which is how the original intended it.
The user wants, later, a more manual, modern jump as an option.

Context: the original postpones a run jump pressed near a gap and launches it
from the lip (`0x80078c0c` look-ahead, `+0x228` bit 4; restored for large drops
in D08Y), its flight follows a fixed ballistic arc, and D08X/D08Y add mid-air
ledge help. A manual jump would take off on the press everywhere, and probably
allow some air steering, while the levels' gaps are tuned for the lip launch,
so a player-timed takeoff must still be able to clear them (for example a short
coyote window at the lip, or a slightly longer arc only in this mode).

**Acceptance:** a selectable jump style in Modernized (the D08Y lip launch stays
the default unless the user decides otherwise). In the manual style the jump
leaves on the press, has bounded air control, and the slot-5 gap and other
measured gaps remain clearable with reasonable timing. No double jumps,
no wall clipping. Vanilla is unchanged.

### D08J1 - Hold-E run-up grab for overhead ladders (slot 6)

**Done (2026-10-01, user-accepted: "genuinely working solidly").** User: the ladder at save slot 6 is awkward to
get to; "I want to just be able to run up to it while holding E and i grab it,
but duke seems to bounce off of it."

Finding: that ladder's bottom rung hangs about 1040 units (about Duke's own height)
above the floor, so the original ground mount
(`0x80051cf0`, Up + Cross, 185) never sees it: walking or running into it with
E stops at the wall (94), and only an E jump catches it in the air. Jumps with
E caught it from good distances but met the wall low and bounced (107) from
others (walking at 400 and 1300 units). See
[measurements, change and evidence](documentation/79-d08j1-ladder-leap.md).

Change (Modernized only): with E held on the ground, holstered (E stows
first) and heading at a plain ladder whose bottom is 300..1400 above the
floor, the host presses the original jump once per E hold when Duke is inside
the catching window for his gait (running 750, walking 800, standing 450
units from the panel). The reach stays held through the leap (an E tap at the
wall works too), and in the air Duke eases along the panel into its width
when he started up to 180 units beside it. The original reach makes the
catch and climbs. Vanilla, jumps without E and ladders mountable from the
floor are unchanged.

**Acceptance:** at the slot-6 ladder, running or walking at it with E held,
or standing below it and pressing E, grabs it without Space, armed or
holstered, first and third person, both jump styles; no new grabs without E;
D08J/D08U/D08X ladder and ledge routes unchanged; user playtest confirms the
feel.

### D09 — Modern controller support

Provide left-stick movement, right-stick look, configurable sensitivity/inversion, dead zones and sensible action bindings using the modern action layer. Support switching input devices and disconnect/reconnect.

**Acceptance:** stick movement is predictable, no drift at rest on the tested controller, menus and gameplay agree on bindings, and disconnects do not leave held actions. Record tested hardware and limitations.

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

### D11 — First-person playable prototype

Build an optional eye-level camera using the verified movement and aiming foundation. Investigate head/body visibility, near-plane clipping and room/portal culling. Keep third-person available as a fallback for unsupported states.

**Acceptance:** a documented first-level route is playable with correct collision and shot direction, no obstructing head geometry and no missing rooms caused by the camera offset. Mark unsupported states explicitly; moving the camera alone does not complete the job.

**Done - user accepted 2026-09-29** after an extended play session: "i have been playing it for a while, and im insanely happy with it. mark all as accepted. this is phenomenal, and it's like a dream come true."
**P** toggles an eye-level view in Modernized
(independent camera); saved as profile schema 13 `view`, `--view first|third`.
The eye follows Duke's neck joint through the original camera solve, the head
joint is skipped only during Duke's draw, and the render gets a shorter
projection distance (about 64 degrees). An isolated first-level route (street,
club door fight, doorway threshold, entry hall, main room, jump, crouch, P
toggle) passed its 14 checks. Swimming and jetpack flight blend back to third
person. Known limit: steep-angle wall contact can drop wall polygons (D11B).
See [D11 first person](documentation/61-d11-first-person.md).

### D11B - First-person near-wall polygon clipping

**Done (2026-09-29, user accepted).** Host near clipping for the two level mesh
renderers (`0x80011020` world, `0x80010000` object) while the eye view is live;
see [the engineering note](documentation/62-d11b-near-clip.md) and the work log.
Original report from D11: pressed against a wall and looking along it at
a steep angle, the first-person view loses or tears the wall polygons that
reach beside or behind the eye (club side wall, about 60 degrees off its
normal). The D11 attribution to `0x8002f1e0` was wrong; that routine draws a
horizontal grid surface. A shorter projection distance (192-256) and moving
the eye back 64-128 did not fix it.

**Acceptance:** the documented D11 wall-contact cases (head-on and 30-90 degree
oblique in the club and a first-map corridor) render without black or torn
wall polygons; third person, Vanilla and the frame budget are unchanged.

### D11A — Scroll-wheel zoom lock into first-person

**Cancelled 2026-09-29 by the user:** the D11 `P` toggle works well enough for
entering first person, so a scroll-wheel lock is not wanted. Kept for reference.

**Todo from 2026-09-27 playtest.** Player wants Fallout/Skyrim-style entry: scroll
the existing third-person distance inward until the view locks into first-person,
then scroll out to return. Coordinate with D10 distance and D11 eye-level camera;
do not break Alt-wheel distance, weapon wheel, or Vanilla.

**Acceptance:** continuous distance scroll reaches a documented first-person lock
without FOV hacks alone; unlock returns to the preferred third-person distance;
unsupported states fall back cleanly; aiming/movement remain coherent.

### D11C - Savestates can keep Duke's first-person head hidden (slot 12)

**Done - user accepted 2026-09-30:** "duke's head is back, mark as complete! well done". A stale head-hide bit is reclaimed at Duke's
next draw in either view, so loading slot 12 (or any save taken in first
person) shows the head again. Verified that the original never sets bit 0 on
a joint record. The save file itself is not rewritten. See the work log.

**Found 2026-09-30 (user noticed it was specific to slot 12):** in the eye
view, `first_person_draw` (`recomp/src/ttk/first_person.inc`) sets bit 0 of the
flag byte on Duke's head-joint record (joint 9, `[player+0x40]+0x44+0x28*9`)
from Duke's draw entry to the next object-list step, and `head_restore`
clears it. The user's slot-12 savestate was captured inside that window: on a
private copy, the head record flag reads `01` right after loading (all other
joints `00`; slot 1 reads `00`). After a load the host `head_flag` is 0 and
`first_person_draw` deliberately refuses to take ownership of an already-set
bit, so nothing ever clears it and Duke is headless in every view.

**Acceptance:** a save taken in first person can never carry the hidden head.
Two options: clear the bit before a savestate is written, or safely reclaim a
stale bit after a load. First verify whether the original game ever sets bit 0
on Duke's joint 9 itself, so a real hide is never undone. Loading the existing
slot-12 copy shows Duke's head in third person, while first person still hides
it. Add a native test case. Vanilla is unchanged; slot 12 itself is repaired
only if the user asks.

### D12 — First-person weapons and state polish

**Done - user accepted 2026-09-30:** "finally, we can mark this as accepted!!"
after six framing passes. Duke's own right hand and the
original weapon meshes are drawn in front of the eye in first person, held in
each weapon's own firing pose, drawn over walls, with the original muzzle flash
and a short kick, each weapon framed per the user's playtests (Devastator-style
twin cannons, HUD on top). No new assets. See
[D12 first-person weapons](documentation/63-d12-first-person-weapons.md) and
the work log. Known limits: instant holster/draw, no left hand, reload motions
not shown, projectile origin still at the real hand. The kick leaving first
person is D12A (quick kick, Done).

Resolve held-weapon presentation, body visibility, eye height, recoil, traversal, death and scripted sequences. Decide whether existing geometry is sufficient before proposing new assets.

**Acceptance:** tested weapon/state combinations are usable without clipping or misleading muzzle placement; camera transitions remain coherent; third-person and Vanilla still pass their routes. Document any optional new assets and their provenance.

### D12A - First-person kick without leaving the eye view

**Done - user accepted 2026-09-30:** "its done! accepted. ... this has been yet another amazing feat of engineering." Queued by the user on 2026-09-29: in first
person, kicking (Q, the original Boot melee) dropped out of the eye view to
the original third-person camera. On 2026-09-30 the user asked for no "duke-fu
spin around" in first person, but a Duke Nukem 3D style kick with Duke's leg,
"more accurate to aim". Delivered: in the eye view Q is a short quick kick with
any weapon, moving or standing, drawn with Duke's own right-leg meshes posed
from the original front kick (animation 115), hitting along the crosshair with
the original kick's damage sphere, radius and damage formula. Boot-selected
original kick requests (Q, E, held attack) are converted to it. See
[D12A first-person kick](documentation/65-d12a-first-person-kick.md).

**Acceptance (revised with the user's direction):** in first person a
standing and a moving kick, holstered and armed, stay in first person with the
view aim intact and no full-body spin; the leg reads as a kick; the kick hits
what the crosshair is on through the original damage path; third person and
Vanilla keep the original kick. The user accepted no knockback and the lower
damage per kick after the first playtest; the kick must hit low props under
the crosshair without crouching, kick on a held left-click with the Boot while
moving, and show a tuned thigh (second pass, Needs playtest).

## Graphics and playback

### D13 — Higher internal resolution and display scaling

Explicit user priority: selectable high-resolution settings. Inspect and reuse applicable renderer capabilities, then expose tested internal-resolution choices, fullscreen/window modes and output scaling. Distinguish rendering more scene detail from enlarging a low-resolution image. Preserve original-resolution presentation.

**Acceptance:** comparison captures demonstrate the effect, settings survive restart, UI/FMV sizing remains correct and tested performance is recorded. Verify the active build actually supports each offered option.

**Done (2026-09-30, user-accepted):** "correct correct correct, D13 is good.
I'd say let's approve it." Per-profile internal scale 1-4x (OpenGL; the
software renderer stays 1x), windowed/borderless/exclusive display, window
width and linear/nearest output filter, via `run.py --settings` choice R or
flags. Vanilla defaults to 1x, Modernized to 4x (user choice). Alt+Enter and
exclusive fullscreen were fixed after the first playtest. Evidence and limits:
[D13 note](documentation/67-d13-resolution-display.md).

### D14 — Widescreen, FOV and visibility

**Done (user-accepted 2026-10-01):** "im very happy with it! i accept!" Modernized opens in true widescreen (default
16:9; `run.py --widescreen off|16:9|16:10|21:9|auto`), rendered natively wide
with the original projection, so nothing is stretched. TTK's portal root
rectangle is widened so the revealed columns are drawn, the D11B near clip runs
in third person for near walls, and the status bar sits in the wide corners
(restored after it draws, so saves and 4:3 never see it). Movies and 2D menus
stay 4:3. The broken preview came from `nw_hud_corners`, which shifted world
polygons. Vanilla and `off` are unchanged. See
[the D14 notes](documentation/76-d14-widescreen-first-pass.md).

Explicit user priority: widescreen support. Render a wider view without stretching actors. Correct aspect, FOV, HUD anchoring, menus and room/portal visibility; define how original movies and fixed compositions are framed.

**Acceptance:** 4:3 and 16:9 pass representative indoor/outdoor tests without geometry popping at the added edges, misplaced aiming or stretched UI. Additional aspect ratios may remain explicitly unsupported.

### D15 — Optional geometry and texture precision

**Accepted (user playtest, 2026-10-04).** Geometry stability, slot 4 floor seams,
slot 1 idle polish, slot 5 black/missing left surfaces, slot 6 closet/furniture
and the previous slot 8 opacity case are accepted. [Implementation, regression
causes, tests and limits](documentation/96-d15-accepted-precision.md). The newly
replaced slot 8 subway bands are D17P and do not block D15.

Investigate renderer support for reducing vertex jitter and perspective distortion, preserving authentic behavior as an option. Compare effects on animated geometry, effects and seams before enabling enhancements by default.

**Acceptance:** captures show actual improvements and any remaining artifacts; toggles restore original presentation; collision, visibility and game timing remain unaffected. Unsupported renderer features are documented rather than simulated by ineffective settings.

### D16 — Texture filtering and game-specific HD assets

Offer tested nearest/filtered presentation and investigate a narrowly scoped replacement-texture path. Preserve palette changes, animation, transparency and texture-page reuse. Establish a small verified asset sample before attempting a large upscale pass.

**Acceptance:** the sample replaces the intended textures only, handles their variants, has a fallback to disc assets and records asset provenance. Filtering, higher resolution and replacement textures are separate settings. A full HD pack is a later asset-production job scoped from this investigation.

### D16A — HRP assets and first-person weapon research (later)

**Cancelled (2026-09-30, user decision).** The first-person weapons and other
presentation work built from the original TTK meshes and assets is the chosen
direction; HRP import or adaptation is unnecessary. The only follow-up kept is
replacing the Duke3D-sourced switcher icons with original TTK art, tracked as
**D08A3**. The text below is kept for history.

Investigate the [Duke Nukem 3D High Resolution Pack](https://hrp.duke4.net/)
as a future source/reference for weapons, interface, HUD/overlays and optional
3D model replacement. Prioritize first-person weapon presentation for D12.
This is later research, not authorization to import assets during controls work.
Inventory candidate weapon models, skins, animations and definitions; map them to
Time to Kill weapons and identify mismatches, missing states and conversion work.
Assess renderer integration, muzzle alignment, clipping, widescreen placement,
performance and disc-asset fallback. Review per-asset provenance, attribution and
applicable art terms before proposing redistribution or adaptation.

**Acceptance:** a sourced compatibility/rights matrix and a bounded prototype
proposal distinguish reusable assets, visual references and new work. Record
unresolved permission and technical questions; no assumption that an EDuke32 pack
loads directly into this recomp. Do not import or adapt HRP scuba gear as a TTK
item (D08N cancelled). Implementation requires a later selected job.
Keep additional user-supplied research topics as separate scoped backlog entries.
See [external research register](documentation/23-external-research.md).

### D17 — Presentation smoothness without faster simulation

**Done (user-accepted 2026-10-02):** on the 180 Hz monitor at Match Display
the user saw about 180 FPS with very good stability, correct FMVs and an
accurate fps readout, and accepted the job. Remaining refinements are separate
jobs: geometry/texture instability (D17A) and mouse responsiveness and input
latency (D17B; the acceptance's input-latency measurement moves there).

**Needs playtest (2026-10-02, second pass):** frame-rate option with Match
Display, even display-rate presents and in-between frames redrawn by TTK's own
renderer in worker processes (camera, Duke and object-loop transforms
interpolated; guest timing untouched). After the first playtest: emulation-thread
costs cut (forensic rings off in player sessions, GL binding cache, cheaper
presentation tick and SPU query), one in-between per display refresh (120 Hz: 3
per game frame) reaching nearly every present in all ten test slots, doorway
rooms from the game's own portal walk, FMV and quick-kick fixes. Design,
evidence and limits: [D17 notes](documentation/80-d17-high-refresh-audit.md)
(section 13 for the second pass, section 14 for the third: no repeated images,
copy-on-write redraw saves, exact rotation blends, near-clip seams, fps readout).

**Scope expanded 2026-09-30 (user brief):** arbitrary / high refresh rate
rendering. Full brief: [D17 high refresh brief](documentation/68-d17-high-refresh-brief.md).
Render frequency must be separated from gameplay timing: rendering at 180 FPS
must not run game logic 3x faster, and 30 FPS must not slow it down. Frame-rate
option: **Match Display** (preferred default; the refresh rate of the display
actually presenting the window, handling moves between monitors, never a
hardcoded 180 Hz), **30 / 60 / 120 / 144 / 165 / 180 / 240 FPS**, and
**Unlimited**. Use interpolation (player/camera, actors, projectiles, moving
sectors) where the data permits and it cannot change gameplay; mouse look must
not feel quantized to a lower rate; frame delivery must be evenly paced, not just
a good average. Audit every system listed in the brief rather than assuming
frame independence. Current 60 FPS is the known-good baseline and must not be
destabilized; 30 FPS stays as a compatibility and regression mode.

**First step when selected:** audit and a short technical plan (what sets
simulation frequency, render frequency, remaining frame-dependent systems,
current display refresh detection, where interpolation or delta-time
conversion is needed, and a safe staged path) before any timing-model change.
Until D19 menus exist, the option follows the D13 pattern (profile setting,
launcher/runtime flag). Vanilla keeps its original timing.

**Acceptance (expanded):** a repeatable numeric comparison at 30 / 60 / 120 /
180 / 240 FPS (known-distance walk/run, jump, fall, jetpack, camera rotation,
auto and semi-auto fire, projectile travel, enemy movement, doors/lifts, animations,
timers/scripts) shows equivalent gameplay; measured frame times match the target
cadence; 60 FPS behaviour is unchanged; plus the original criteria below.

Measure unique rendered frames, guest timing, host presentation and input latency independently. Investigate interpolation only where the game's data permits it; reset interpolation across teleports, room loads and camera cuts.

**Acceptance:** measured pacing improves without speeding up movement, scripts, audio or cutscenes; discontinuities do not smear or blend incorrectly. Report achieved unique-frame cadence honestly: a 60 Hz guest clock is not proof of 60 unique game images per second.

### D17A - High-refresh texture/geometry instability: popping, flicker, black areas

**Accepted by the user, 2026-10-03**, after natural first-level play at 120 and
180 FPS. Broad systemic work is closed. Remaining focused issues are tracked
in D17D-J and D18C; they do not reopen this parent. The following original
brief and criteria are historical. See [acceptance and baseline](documentation/86-d17-acceptance-and-regression-baseline.md).

User report (2026-10-02, third playthrough at 180 Hz, after D17 was accepted):
parts of the scene vanish for a moment and show black underneath. Places:
the apartment right after using the light switch (looking right and walking
toward where the bed moves from, black patches on the wall to the left); the
wardrobe ("closet") still flickers; on the highest alley platform looking
toward the burnt-out car, strong flicker of the brick wall and the platform for
a couple of seconds before it settles; occasional popping on the street
pavement; the subway, where vanished parts show as black areas or squares near
the edges of the image; the strip club stairs to the balcony, and bar stools
that disappear and return. Also a slight overall texture "vibration" or
shimmer. The user notes it often appears around polygon edges and vertices
(descriptive, not a diagnosis).

Known from D17 (documentation/80 sections 13-14), to verify rather than assume:
- Redraws match the real image exactly at alpha 0, and the D17 tools
  (`popsweep.py`, `seqcheck.sh`, `shift2.py`, scripted input) find mostly
  hairline seams and texel sparkle, not whole surfaces vanishing. The reported
  black areas have not been reproduced yet: reproduce first, at the reported
  places, at 60 Hz as well as above (is it high-refresh specific?).
- Native-pixel vertex snapping and affine textures are more visible with six
  distinct images per game frame; geometry/texture correction (D15) cannot
  resolve TTK's CPU-built packets as implemented.
- Near clip (D11B) takes over polygons beside the eye; its pieces meet the
  original polygons at seams, and its packet budget can refuse polygons.
- Black areas near the image edges may be widescreen margins (D14) where the
  game culls to its 4:3 view, or the in-between camera seeing past the real
  camera's cull; the strong flicker that settles after a couple of seconds may
  be a streaming or culling transition.

**Acceptance:** each reported place reproduced (or shown not to reproduce) with
a recorded test; causes identified and fixed or explained with evidence;
no surfaces vanish to black at 60 Hz or above in those places; any remaining
PS1-inherent artifact is documented, with an option (D15) where one is
feasible. Vanilla unchanged.

### D17B - Mouse responsiveness and input latency at high refresh rates

**Accepted by the user, 2026-10-03**, after natural first-level play at 120 and
180 FPS. Broad systemic work is closed. Remaining focused issues are tracked
in D17D-J and D18C; they do not reopen this parent. The following original
brief and criteria are historical. See [acceptance and baseline](documentation/86-d17-acceptance-and-regression-baseline.md).

User report (2026-10-02): frame rate is very stable, but turning with the mouse
(entering the strip club and turning to shoot, and later just walking and
turning) feels slightly delayed or floaty. Mouse input must feel immediate:
no perceptible latency or smoothing between the mouse and the camera.

To measure, not assume: mouse event to camera change to photons, at 60 Hz
(D17 off) and at 120/180 Hz, in first and third person. Candidates:
- The camera turns once per game update (30 per second); in-between images
  interpolate between two game frames, so the newest mouse motion reaches the
  screen only with the next game frame and arrives smoothed.
- Replay presentation draws in-betweens ahead into a cache; a prepared image
  can be up to one present old when shown.
- Swap interval, driver frame queueing and exclusive fullscreen behaviour.
- Input sampling once per field and any smoothing in the modern controls.
The runtime has a latency ring (`latency_ring_mark`) and the plugin a present
trace; extend them to a mouse-to-present measurement. A likely direction, if
the camera update is the cause: apply the newest mouse yaw and pitch to every
presented image (late camera update at present time) while gameplay keeps its
original 30 Hz aim.

**Acceptance:** measured mouse-to-present latency at high refresh is no worse
than at 60 Hz and as low as the pipeline allows; the user confirms aiming feels
immediate in first and third person; gameplay timing unchanged.

### D17C - View bob: disable experiment, then a stable modern camera

**Done (user-accepted 2026-10-03).** The user confirms the camera is much more stable and is happy with the current look. This closes the remaining camera-look playtest; the implemented camera and bob options are unchanged.

User request (2026-10-02): TTK's walking view bob looks wrong in the recomp,
especially at high refresh: rather than Duke's head moving, the floor and walls
seem to breathe, swell and melt (with the PS1 geometry wobble). First step: a
clean test with no view bob at all, to judge whether the world feels more
stable, whether the breathing goes away, whether it interacts with the D17A
instability, and whether any rendering work is tied to the bob. Longer term the
recomp should present a stable modern 3D view where appropriate.

**Acceptance (experiment):** a switch that removes the first-person view bob
entirely, verified by measurement; the user playtests it and decides the next
step (keep off, make it an option, or redesign).

### Focused D17 follow-ups: shared constraints

Current statuses are in the board above; accepted fixes remain regression
baselines for unfinished follow-ups. Primary quality/regression target is
**120 FPS**; also compare 180 FPS. 240 FPS+ is
robustness/compatibility, Unlimited stress/debug. No 120 FPS technical ceiling.
Preserve accepted mouse feel, FMVs/audio, idle stability, gameplay speed, FPS
reporting, culling improvements, PS1 character and optional Vanilla. Profile
before changing architecture. Do not trade away 120 FPS quality to perfect an
extreme rate. Fix a demonstrated systemic defect at its appropriate level.

Current reproduction copies: local `cards-user8`, UI slot N = file N-1.
Never write to original player saves/cards. Save numbers are not permanent
identifiers: preserve the dated snapshot/manifest and location in each report.
Older generations are explicitly named below. Evidence and baseline identity:
[86](documentation/86-d17-acceptance-and-regression-baseline.md).

### D17D - Club-exit furniture: lower wooden board popping

**Accepted (2026-10-03).** User confirms the UI 9 prop is fixed, along with
the UI 11 closet and the closet inside the strip club. This accepts the
source-plane depth correction and bounded static-prop contact tolerance.
The related closet report D17F is also closed on this explicit playtest.
New blood/tabletop-prop reports are separate D17K/L follow-ups.
[Cause, implementation and evidence](documentation/87-d17d-contact-depth.md).

Original report: UI 9 (`cards-user8`, file 08); inspect the lower wooden
board of furniture near the strip-club exit, stationary and during slow turns.
Also reported in `cards-user6` UI 9 and `cards-user5` UI 1. Triangular fragments
occur in both original and intermediate captures, and at 60 Hz. No clipping
budget, packet or copy exhaustion in the inspected sample. The investigation
confirmed near-coplanar placement plus snapped-depth surface intersections;
see the current evidence above.

**Acceptance:** identify the offending primitives and cause, fix this object
without coordinate hacks or blanket culling changes; capture before/after at
60/120/180, then preserve the opening/club regression baseline. No speculative
mesh change or general renderer rewrite. User confirms the lower board stable.

### D17E - Subway peripheral wall visibility near the camera

**Accepted (user playtest, recorded 2026-10-04).** Current UI 3 (`cards-user8`, file 02; earlier `cards-user7` UI 3).
Walk along the corridor close to either wall, turn near view edges, and walk
up/down stairs. Original report: blank/missing peripheral wall sections,
not conventional tearing. User now reports substantial improvement; bounded
60/180 full-width routes did not reproduce the missing section.

**Acceptance:** reproduce on the accepted build at 120 first, or document
repeatable non-reproduction and seek closure; distinguish widescreen margin,
near clipping and visibility state using full-width compositor captures.
Verify 60/120/180 and wall approach/retreat without regressing fixed walls.
No claimed fix solely from a canonical 512-pixel image that omits the margins.

**Candidate/evidence:** Full-width wall approach/retreat, edge turns and stair routes at 60/120/180 did not reproduce the old missing-wall report. User repeatedly walked the subway corridor and watched peripheral edges without reproducing the old gaps; the result is accepted. No E-specific visibility workaround shipped. Future similar reports get separate targeted jobs. See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17F - Apartment/club closet edge and lower-geometry artifacts

**Accepted (2026-10-03, user playtest of the D17D implementation).** The user
explicitly confirms the UI 11 closet and the closet inside the strip club
are fixed. Preserve these accepted surfaces. Tabletop props seen from UI 11
are a distinct new report, D17L; do not reopen the closet job for them.

Historical reproduction: UI 6 and 11 (`cards-user8`, files 05/10): apartment closet,
lower geometry/textures and intersections. Current apartment/secret reveal
is user-reported dramatically improved, stable and pleasant. Preserve it.
Older `cards-user6` UI 8: club closet disappearing walls; UI 11: apartment
lower closet popping; UI 10: good apartment comparison. `cards-user5` UI 6
is an earlier closet reproduction. These old snapshots are not current slot 8.

**Acceptance:** separate currently reproducible minor edge/polygon defects
from fixed see-through doors/walls; classify clipping/order/depth versus actual
texture changes; bounded fix with closet and secret-reveal regression at 120
and 180. Do not smooth away normal moving-camera PS1 character.

### D17G - Strip-club translation jerk during Shift+W / Shift+S

**Todo. Primary remaining movement polish.** Current UI 12 (`cards-user8`,
file 11): repeatedly Shift+W, Shift+S. User describes a minor polish issue;
club mouse rotation and general playability are accepted. Compare current
UI 4 opening, UI 5 dancers, UI 8 street, apartment/alley and a fresh entry.

Trace evidence at 180: regular presents can coincide with exhausted world
interpolation history. Roughly 67 ms pose intervals and 38 ms composition
completion delay consume the 90 ms history plus advance-preparation margin.
Alpha 1 alone is not proof of a hitch. Higher world delay (130 ms) and stronger
clock correction were tested and rejected; neither is shipped. No worker
replacement/camera-cut diagnosis was established for this case.

**Acceptance:** reproduce and measure at 120 before assuming the 180 stress
result applies; correlate translation holds, input timestamps, complete pose
availability, CPU/worker/GPU timing and audio. Improve the bounded route without
adding unreviewed movement latency or speculative incoherent geometry. Preserve
late mouse response, original gameplay timing, 180 support and PS1 identity.

### D17H - Train-control and platform/ledge movement stutters

**Todo.** Current UI 2 (`cards-user8`, file 01): activate train switch, leave
the door while the train moves, approach the pig cops at the far end. Current
UI 10 (file 09): turn around, walk forward, jump onto the ledge and move there.
UI 10 now means the train-platform area, not the previous late club state.
User reports subtle stutters, not an unplayable scene. The full moving-train
sequence has not been established by the short scripted interaction sample.

Slot 10's scripted jump/fire stress can reduce source throughput and starve
output audio despite high presentation FPS. Coordinate with D18C for audio;
keep this job focused on frame times and movement. Preserve original slowdown
UI 1/2 from `cards-user7` as historical stress comparisons, not proof of a
current severe regression.

**Acceptance:** perform the actual switch/door/train/pig-cop and ledge routes
at 120; capture spikes and source workload, then verify any local fix at 120
and 180 with unchanged train/AI/gameplay timing and opening/club baseline.

### D17I - Unlimited stale-camera scheduling episodes

**Todo. Lower priority stress/debug.** Use `cards-user7` UI 5 early dancers,
UI 10 later club, and current `cards-user8` UI 12. Earlier Unlimited capture:
about 161 distinct images/s, 114 ms stale camera sample and catch-up turns,
while emulation/audio could remain realtime. This is not a requirement to
produce unlimited perfect images, nor evidence that 120 is unacceptable.

**Acceptance:** investigate adaptive cadence/deadline pressure and bounded
queues with frame-age traces, improve correctness/recovery under overload,
retain frame-rate independence at 240+, and prove no 120/180 regression.
Do not impose a 120 ceiling or retune normal play solely for this stress mode.

### D17J - Verify older isolated visual reports against accepted baseline

**Todo. Bounded reproduction triage only.** Preserve reports without reliable
current slot identity: apartment after light switch/bed-secret reveal (black
wall patches); highest alley platform toward burnt-out car (brick/platform
flicker); street pavement/edge/intersection sparkle; club stairs to balcony
and bar stools disappearing. Historical source: note 81 / original D17A brief.
The apartment secret reveal is now user-reported stable: do not reopen it as
a confirmed current failure without reproduction.

Retain older `cards-user7` UI 8 initially slow area and UI 5 early/UI 10 late
club comparisons for first-visit versus revisit observations. No minute-long
shader-cache explanation was established. First-use scratch reforks and the
five-field camera reset were fixed, so do not treat those causes as still open.
Three earlier unmatched moving endpoint captures in note 82 lack generation
labels and are unclassified, not proof of a current visible defect.

**Acceptance:** bounded 120-first capture routes classify each as reproduced,
not reproduced, fixed/user-confirmed, or characteristic PS1 motion. Record
current state identity and primitive provenance for any reproduced defect,
then create a single scoped fix job or assign D17D-F; no broad implementation
under this triage job. Preserve historical evidence rather than guessing slots.

### D17K - Coplanar ground blood in save slot 12

**Accepted (user playtest, recorded 2026-10-04).** User report on 2026-10-03 after
accepting D17D: load UI save slot 12 (file 11) and look at the blood on the
ground directly in front of Duke. The user describes it as coplanar blood.
The initial report did not establish the cause; the candidate addresses a
traced mixed-path ordering defect.

Before testing, verify the current save identity and make a dated private copy;
do not assume its bytes still match the earlier `cards-user8` snapshot. Use
background/offscreen tests and private cards/profiles. Inspect the blood
primitive/decal path, transparency, depth and ordering relative to the floor.
Do not extend the static-prop contact bias blindly to blood or all sprites.

**Acceptance:** identify the offending primitives and establish whether this
is renderer depth/order handling or genuinely ambiguous coplanar placement;
implement a bounded fix if justified, or document an unavoidable placement
limit. Verify 60/120/180, transparent blending and nearby geometry while
preserving the accepted UI 9 prop, UI 11 closet and strip-club closet. User
confirms the result. D17D/F remain accepted.

**Candidate/evidence:** The candidate restores the source floor polygon ordering key so enhanced floor pieces do not paint over native blood. User confirms stable ground splatters, clearly visible varied patterns and excellent appearance. Preserve the current behavior. The original blood state is preserved privately; current UI 12 now holds the separate D17N wall case. See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17L - Tabletop props cut off on approach/retreat in save slot 11

**Done - user-accepted 2026-10-04, implementation `1e6fb0a`.** Runtime capture identifies a later native table-top quad overwriting the depth-tested cup. The new bounded correction and 60/120/180 evidence are in [note 95](documentation/95-d17l-tabletop-depth.md). The earlier unresolved candidate and retest below are historical. User report on 2026-10-03: load UI
save slot 11 (file 10), look at the props sitting on the table directly ahead,
and walk forwards and backwards. The props get cut off and reappear as Duke
moves closer. This concerns the objects on the table, not the now-accepted
closet in the same save. The earlier pre-repair retest showed the isolated cup disappearing/flickering, largely unchanged; other cups and the bowl on the bar are stable.

Verify the current save identity and use a dated private copy in background
tests. Trace the affected props through approach and retreat, including
visibility/fade, near clipping, host takeover and depth/order transitions.
Distinguish an entire object disappearing from individual polygons being cut.

**Acceptance:** reproduce and identify the responsible path, make a bounded
fix without coordinate hacks or blanket culling changes, and capture the
approach/retreat at 60/120/180. Preserve legitimate occlusion, the accepted
UI 9 prop and both accepted closets, weapons and original PS1 movement
character. User confirms stable tabletop props. D17D/F remain accepted.

**Historical candidate/evidence (superseded by note 95):** The candidate gives compact static props consistent depth beyond the near radius and groups opaque faces to control draw cost. The user does not accept this candidate as resolving the isolated cup. Read-only mesh inspection establishes polygonal table/cup geometry and shared cup meshes across table/bar instances, but does not yet establish the residual flicker cause. Trace the specific failing instance and supporting surface across a bad frame before changing behavior. See [follow-up inspection](documentation/89-d17-playtest-followup.md). See [implementation and verification](documentation/88-d17e-k-l-visuals.md). D17D/F remain accepted.

### D17M - Shotgun ammo visible through ladder platform in save slot 9

**Todo - separate backlog bug (2026-10-04).** Load UI save slot 9 (file 08)
and begin climbing the ladder. The shotgun shell/ammo pickup is visible
through the platform it sits on; the platform should occlude it from below.
This is a new report, not the accepted old UI 9 shelf/prop case.

**Investigation:** verify save identity, use a private copy, and trace pickup
and platform primitives. Compare culling, depth/occlusion, sprite handling,
clipping and render order; the symptom does not establish the cause.

**Acceptance:** legitimate platform occlusion throughout ladder ascent and
descent, with the pickup visible when unobstructed. Check nearby geometry,
60/120/180 and Vanilla/Modernized; preserve accepted D17C/D/E/F/K behavior.
No blanket sprite-depth or visibility bypass. User confirms the result.
See [save identity and regression baseline](documentation/89-d17-playtest-followup.md).

### D17N - Diagonal wall artifacts during movement in save slot 12

**Todo - separate backlog bug (2026-10-04).** Load UI save slot 12 (file 11)
and repeatedly walk forwards/backwards. Watch the wall directly ahead:
jagged diagonal lines appear during movement and the surface looks as if it
vibrates. Other nearby walls show similar behavior. This new state is not
the accepted D17K blood reproduction.

**Investigation:** trace affected source faces and output primitives on a
private save copy. Distinguish polygon/triangle boundaries, clipping,
interpolation, precision, native PS1 geometry behavior and other render-path
causes. Do not assign a technical cause from the visual description alone.

**Acceptance:** substantially steadier wall surfaces on repeated W/S routes
at 60/120/180, preserving the deliberately retained PS1 visual character,
textures, legitimate occlusion and accepted D17C/D/E/F/K behavior. Compare
Vanilla/Modernized and nearby walls; user confirms the improvement.
See [save identity and regression baseline](documentation/89-d17-playtest-followup.md).

### D17O - Unstable sky appearance when looking up (slot 10)

**Done, user-accepted (2026-10-04).** Autonomous investigation verified that the
original sky uses camera-relative meshes. High-refresh object interpolation
incorrectly recorded their matrices under the last world object, replacing the
current eye translation with an older one. A guarded exemption for the three
resident sky calls now retains the original camera-relative draw path and
animated layers. Sampled displacement fell from up to 1075.397 units to zero.
Private ground/flight captures cover 60/120/180 and multiple yaw/pitch views;
the user explicitly accepted the result: "amazing work!!! accepted. well done."
Preserve implementation `25734d1` and its recorded coverage limits. See [architecture, evidence and limits](documentation/92-d17o-sky-intake.md).
The report concerns UI slot 10 (file 09) and is not jetpack-specific.

**Scope:** reproduce the look/feel and establish how TTK draws and animates
its sky from owned game code/assets and authenticated active overlays. Trace
camera-relative/world coordinates, primitives, UV/texture motion, clipping,
ordering and update cadence. Compare original guest frames and presented
high-refresh frames before attributing the effect to a skybox, interpolation,
precision or camera motion. Implement a bounded stabilization if evidence
supports it, retaining intended art/parallax/animation and Vanilla behavior.

**Reproduction:** preserve the current UI 10 identity, then use writable private
cards/profiles for stationary look-up, slow yaw/pitch, walking and ground/flight
comparisons. Start at 60/120, then 180; inspect the full presented sky/horizon,
first/third person and another sky view if available. Never use the player's
cards for tests or infer identity from an older slot-10 fixture.

**Acceptance:** documented original sky architecture and verified cause;
before/after evidence of steadier sky motion without broken horizon seams,
world occlusion or lost intended animation; bounded tests/regressions covering
accepted D17C/D/E/F/K and D08Q2/Q3 behavior. Record timing cost and remaining
coverage limits. User confirms the appearance; otherwise Needs playtest.
Heavily document source/asset findings, verified addresses, reproduction data,
rejected hypotheses, implementation rationale and measured versus observed
results. Update status/handoff/manual as appropriate; commit and push source
and documentation, excluding all retail assets and local captures/saves.

[Detailed intake, private save identity and investigation plan](documentation/92-d17o-sky-intake.md).

### D17P - Distant horizontal black bands in the new subway slot 8

**Done (user-accepted 2026-10-04):** "i fully accept this fix." Implemented after accepted baseline `8cfd16f`. Separate follow-up
after D15 acceptance and its authorized commit/push. Do not reopen D15 or
confuse the replacement slot with the accepted old slot 8 door opacity case.

**Primary reproduction:** load the new UI save slot 8 in the subway corridor.
Walk forwards while watching distant geometry/textures. Multiple horizontal
black lines/bands appear farther down the corridor and seem to follow the
distant scene, as if sections disappear along horizontal boundaries.

**Investigation:** "possible culling" is a visual report, not an established
cause. Compare visibility/culling, clipping, polygon gaps, depth behavior,
geometry correction, texture rendering, precision and other rendering paths.
Use Original/Corrected geometry and textures independently, primary 120 Hz,
and lower-rate comparisons where useful. Trace the implicated primitives;
review prior fixes and R01 concepts without transplanting unrelated code.

**Acceptance:** eliminate the distant bands during approach and nearby view
changes, explain the responsible rendering path with evidence, and preserve
accepted D15 stability, floor seams, idle behavior, closet/furniture opacity,
Vanilla and optional Modernized modes. Recheck replay consistency and 120 Hz
cost; obtain a focused user playtest before acceptance.

New slot SHA256: `7bf643dcfff92379ce5b9f06789eedf0a1c8c9cd4a46c1cfcde8670d9dba5743`.
UI slot 8 = debug slot 7 / `slot07.pst`. Private intake and player-file manifest
are local at `recomp/analysis/d17p-subway-bands/`. Retail state stays untracked.
[Accepted baseline and distinct old/new state identities](documentation/96-d15-accepted-precision.md).

**Initial investigation:** private approach captures compare all four geometry/texture
precision combinations at 120 Hz and Corrected/Corrected at 60 Hz. A distant
horizontal ceiling discontinuity is visible with both Original and Corrected;
this does not yet establish the cause or identify every reported band. Local
evidence: `recomp/analysis/d17p-subway-bands/intake.json` and `probe.json`.
No renderer changes; player files unchanged. Next: longer approach and matched
primitive tracing before choosing a fix.

**Cause and accepted fix:** Vanilla shows the same bands; they are
three original limits, not a D15 regression. (1) The far limit: rooms past the
render context's `+0x74` (28072/33280 in the subway) are never drawn, and the
fog fade before it is squeezed into a pixel or two, so the corridor ends in a
hard black box. (2) Integer portal rectangles a pixel or two short of distant
openings drop whole strips of the next section's ceiling (rectangle outcodes in
`0x8001160c`), leaving full-width black lines. (3) One-pixel-tall distant faces
whose integer NCLIP is zero or wrong-signed are skipped, leaving jagged partial
lines. Proven with packet rasters, live limit reads, a far-limit experiment and
per-mesh face traces with context limits and rectangles.

New Modernized **Draw distance** option (`extended` default, `original`;
schema 24, `run.py --draw-distance`, settings **D**; Vanilla always original):
doubles the render-only limits (level globals and gameplay checks untouched),
widens portal rectangles by 2 native pixels, and enables the runtime's precise
NCLIP culling (exact PGXP sign, only with Corrected geometry or textures).
Hooks `0x8006276C`/`0x80062B48`/`0x8002FFEC`, guarded code ranges, framework
patch exported and verified on the clean pin. Matched 4x views show a continuous
ceiling and the real corridor end. All 12 private slots: unchanged game rate,
ring peak at most 50%, no budget hits or replay misses; 120 Hz cost unchanged.
Python 106 (2 skips) and native controls/near/input/PGXP/GTE tests pass.
The user's playtest accepted the fix.
[Cause, implementation, evidence and limits](documentation/97-d17p-distant-bands.md).

### D18 — FMV and audio presentation safeguards

Expand coverage beyond the opening movie: playback, skipping, transitions and return to gameplay. Preserve native movie cadence and audio synchronization through display/profile changes. Optional scaling must not invent a higher source frame rate or regress the fixed streaming path.

**Acceptance:** a movie/transition matrix records tested clips, sync and underrun observations; repeated voice-bank playback does not return on the regression route. Both modes close cleanly during movie playback.

### D18A — Voice/music/gunfire crackle investigation

**Done — player closed (2026-09-28).** No crackle/artefacts in club play; reopen if
it returns. Lab club starvation evidence remains historical in notes 46–47 and is
not treated as an open player defect.

**In progress.** Club no-poll starvation remains the positive lab reproduction; no
audio repair has passed that capture. The 2026-09-27 player session reported no
crackle in the club, which does **not** close the job. Keep producer/frame-time
attribution and matched comparisons. See notes
[46](documentation/46-turning-audio-inventory-progress.md) and
[47](documentation/47-playtest-iteration.md).

**In progress; output starvation reproduced in private club captures, no audio repair yet.** See [current comparisons and limitations](documentation/46-turning-audio-inventory-progress.md). User reports crackle during strip-club voice/music
and gunfire at the station-facing ledge Pig Cop; regression versus pre-existing is
unknown. **Acceptance:** reproduce/capture audible evidence, investigate mixing,
streaming and callback timing, verify any supported fix on the same scenes without
sync/latency regression. Dummy audio is insufficient. See [current brief](documentation/45-playtest-turning-audio-follow-up.md).

### D18B — Concurrent voice with music (no music mute)

**Todo from 2026-09-27 playtest.** Voice clips currently cut out music. Allow voice
and music to play together at sensible relative levels, as a precursor to D21
channel volume controls. Investigate original XA/SPU ducking versus host mixer
policy before changing behavior. Do not hide D18A starvation by muting sources.

**Acceptance:** documented overlapping voice+music route with music continuing;
no new underruns/clipping on the tested scenes; Vanilla/default behavior policy
recorded; volume-channel hooks prepared for D21 without requiring the full menu.

## Player features and release work

### D18C - Load-sensitive crackle at construction signs and train ledge

**Todo.** Current UI 1 (`cards-user8`, file 00): turn around, approach the
construction signs, move/fire, then jump onto the ledge; user hears brief,
self-resolving crackle. Current UI 10 (file 09): turn/move/jump around train
platform ledge. Include UI 12 club movement as an audio stress comparison.
D18A's older acceptance remains historical; this is a new, bounded follow-up.

Slot 1's tested route had zero output underruns, which does not establish
absence of an audible source/mixer/device defect. Slot 10 and some slot 12
stress runs show production starvation. Missing-sample counters are not a
count of audible crackles. Dummy-sink runs are diagnostics, not listening tests.

**Acceptance:** reproduce/capture on the actual audio device at 120, correlate
source/host PCM, producer/callback timing, queue fill, missing/dropped samples,
loads and firing/jumps. Distinguish starvation from clipping/streaming/device
issues. Verify any fix without increased audio latency, lost voice/music,
FMV desynchronization or gameplay/frame-pacing regressions. Check 180 too;
coordinate D17G/H without duplicating those movement fixes.

### D19 — Modern in-game menus, settings and input prompts

**Direction set by the user 2026-09-29 (backlog; not started).** The target
is a customization menu in the spirit of Sonic 3 A.I.R.: one aesthetically
pleasing place to pick Modernized options (control schemes such as the D08R
jetpack scheme, camera, aim, crosshair, display, audio and later
customizations), with clear per-option descriptions and good defaults.
Preferred approach: hack the original TTK in-game menu rather than bolt on a
separate host screen, and give it a responsiveness overhaul (see D19B) as part
of the same work. Until then, new options ship as persisted profile settings
with `run.py` CLI switches. **Menu toggles already requested (user
2026-09-29):** first-person near clipping (D11B: `conservative` default,
`off` = Vanilla rendering via `DNTTK_NEAR_CLIP=0`; the subdividing `full`
mode stays a developer/research option, not a player choice) and the
first-person occluder fade (D11B: off by default in the eye view;
`DNTTK_FP_OCCLUDER_FADE=1` restores the original see-through props).
**Before any implementation:** design in plan
mode and publish a design artifact (menu structure, option list, visual
style, navigation and how it hooks the original menu) for user review. There
is a lot to consider; this is a dedicated future session, not a side task.

Build a modern in-game menu system, using EDuke32 as a design reference for navigation, option organization and customization. Investigate reuse of suitable open-source menu code, recording its exact revision/license and compatibility with this runtime before adopting it; source availability alone does not establish integration suitability. Preserve the original menus as an option and support safe pause/resume, keyboard/mouse and controller navigation. Build an accessible Time to Kill settings surface for mode, implemented controls, display and audio options. Show actual bindings in help and prompts; provide reset and safe handling of display changes. Avoid exposing developer terminology in normal player flows.

**Acceptance:** a player can select a mode, change bindings/display settings and recover defaults without editing files. Keyboard/controller navigation works. The root [game manual](GAME_MANUAL.md) matches the shipped build and labels planned features separately.

### D19A — Duke font assets for host messages and modern UI

**Done (2026-09-27), bounded existing-renderer message pass.** Both font styles,
asset fallbacks and OpenGL/Software captures verified; see
[implementation evidence](documentation/40-feedback-implementation.md). Replace the generic host pixel font with the supplied Duke-style assets,
starting with cheat confirmations and extending shared modern message/prompt
rendering consistently. Small/message font for ordinary text; Atomic menu font
for suitable headings. Do not silently replace original TTK/Vanilla presentation.
No D13 dependency for the bounded existing-renderer message pass; coordinate later
menu work with D19. Inspect actual glyph coverage, spacing, transparency and palette
before selecting an asset path. Preserve provenance/credits and original archives.

Sources exist locally: `research/fonts/DukeNukemSmallFont.pk3`,
`DukeNukemAtomicFont.pk3`, the PNG sprite ZIP in that directory, and `DUKE3D.GRP`.
The PK3 metadata describes Doom-paletted glyphs, not a drop-in TTF; PNG sprites or
an atlas extracted from owned GRP tiles may be easier. EDuke32 names identify font
tiles; its engine source alone is not proof it bundles those art assets. See the
[inspected asset inventory](documentation/39-next-iteration-brief.md).

**Acceptance:** deterministic glyph mapping/atlas generation, intentional case and
missing-character fallback, correct colors/transparency, readable spacing/scaling,
and original cheat wording (D08G1) shown in the selected Duke font. Review gameplay
captures in supported OpenGL/software paths, representative punctuation/digits and
window sizes, no clipping/blur regressions, packaged asset loading and clean fallback.
Do not silently download substitute assets or remove original credits.

### D19B — Responsive modern menu navigation and transitions

**Todo; deferred with D19 menu design.** The user wants the responsiveness
overhaul done on the hacked original in-game menu as part of the D19 design
(2026-09-29). User reports the menu system is clunky,
laggy and has terrible responsiveness. Profile navigation, selection, back,
opening/closing and screen transitions separately from gameplay performance.
Measure event receipt, guest processing, presentation delay, repeat/debounce and
animation pacing before assigning a cause. Investigation can precede D13; final
modern-menu integration belongs with D19.

**Acceptance:** baseline and improved input-to-visible-response measurements plus
repeatable keyboard/mouse/controller routes; taps register once, held repeat is
predictable, rapid navigation/back works, and modern transitions do not impose
avoidable dead time. Define budgets from measurements, not an unsupported FPS
promise. Preserve safe pause/resume, focus handling, settings, audio/FMV and the
original-menu option. No global simulation-speed change to make menus feel fast.

### D20 — Save management and optional quick saves

First make original memory-card saves understandable and reliable. Investigate quick saves/checkpoints as optional Modernized features, accounting for CPU, device, streaming and overlay state. Define compatibility and restoration boundaries before implementing snapshots.

**Acceptance:** original save/load round trips pass; player cards are never replaced by test cards. Any quick-save feature actually shipped restores a documented gameplay scenario reliably and detects incompatible versions. If full snapshots prove unsuitable, document the result and split their implementation into a new job; original save management alone does not prove quick saves work.

### D21 — Accessibility and sound controls

Implement feasible music/effects/voice controls after verifying the game's mixing paths; provide readable UI sizing, reduced camera motion and toggle/hold options where useful. Investigate subtitles against available dialogue assets rather than assuming a complete transcript exists.

**2026-09-27 backlog emphasis:** mirror EDuke32-style **master / music / sound-effect**
channel volumes in the future menu system once D18B concurrent voice+music policy
exists. Coordinate with D19; do not invent a second mixer without measured paths.

**Acceptance:** exposed controls affect the intended channels/actions and persist; text remains legible at supported sizes. List subtitle coverage and any missing source material explicitly. Additional transcription can become a separately scoped job.

### D22 — Campaign fidelity and overlay coverage

Maintain a level-by-level matrix for loading, progression items, enemies, weapons, bosses, saves, movies and end-game flow. Investigate missing native coverage according to observed correctness/performance needs. Treat extra game modes as explicit test scope, not implicitly supported features.

**Acceptance:** the intended single-player campaign has recorded completion evidence and no known progression blocker; failures get reproduction steps and dedicated jobs. Record native/interpreted boundaries honestly. This long-running job must not prevent earlier prototypes.

### D22A - Portal transition loses Modernized controls and first person (slot 7)

**Todo. Bug, next-round backlog; not started.** User report on 2026-10-03:
load UI save slot 7 (file 06), walk into the portal and start the next level.
Duke then loses all Modernized controls and first-person view. The user reports
this as an easy reproduction; no independent reproduction or root-cause claim
has been made in this documentation-only session.

**Expected:** when playable control resumes in the next level, the selected
Modernized movement, independent camera/aiming and first-person preference
remain available. A temporary scripted camera during the transition must not
permanently disable them or reset the player's saved preferences.

Verify the current save identity and create a dated private copy before testing;
do not assume an older slot-7 snapshot still represents this portal. Use
background/offscreen tests with private cards/profiles. Trace the level/overlay
transition, supported-code identity checks, modern-control lease, input capture
and first-person activation/state restoration. Determine whether this is missing
next-level coverage, transition state invalidation, or another cause before
changing hooks. Do not bypass code/overlay guards to force controls on an
unverified level. This bounded bug can be investigated without completing the
whole D22 campaign audit; record any necessary verified coverage explicitly.

**Acceptance:** reproduce the portal route from the private slot-7 copy,
identify the cause and implement a guarded fix so Modernized controls and the
selected first-person view work when gameplay resumes. Verify movement, mouse
look, aiming/fire, capture/pause/resume and first-/third-person selection in the
new level; repeat the transition and check the accepted first-level baseline.
Vanilla stays original, preferences and player saves remain intact. User confirms
control/view continuity. Do not infer full-campaign support from this one route.

### D23 — Performance budgets and long-session stability

Define representative hardware and scenes, then measure frame pacing, audio underruns, memory growth, loading and shutdown. Exercise repeated level transitions and a sustained session in each available mode.

**Acceptance:** documented measurements satisfy the chosen budgets or identify explicit supported-setting limits; no unresolved leak, hang or audio regression in the tested route. Use profiling evidence before optimizing.

### D23A — Modernized frame-budget regression (guard identity cost)

**Done - 2026-09-29 (user-accepted):** "the in game stutter fix works fine".

User: "that stuttering audio/slowness
issue" before the D08Q playtest. Reproduced in isolation on this machine:
Modernized **47.5–49.3 fps** with continuous audio underruns (output fill
17–34 ms vs 180 ms target) while Vanilla ran 60.0 fps clean; the Sep-27
build sat at 57.5. Root cause measured: `ttk::identity()` compared all
106 guards (81 KB, 20,305 words) word-by-word through the guest bus on
**every** call — 46 calls per frame × 108 µs = **5 ms/frame, 24 % of wall
time**; the aim guards added a second copy. Fix: `memcmp` against the
runtime RAM image with the exact per-word path as fallback (~28 µs), and a
per-frame verdict memo keyed on the host frame and `g_dirty_ram_code_gen`
(overlay loads/restores still invalidate at once). Result: **59.94 fps, 0
underruns, fill 267 ms**, 1 check/frame, 0.17 % wall; jetpack flight 60.1.
Debug JSON: `identity_calls/checks/us`. ttk-input/aim/controls tests PASS
(new memo case). Likely the mechanism behind D18A's historical club
starvation, not re-measured there. See
[58-modernized-frame-budget.md](documentation/58-modernized-frame-budget.md).

**Acceptance:** the user's session no longer stutters or slows in
Modernized play (turret room, ledge, jetpack); `audio_stats.out.fill_ms`
stays near/above target with flat underruns; Vanilla unchanged.

### D23B - Intro FMV stutter: stranded native movie shard

**Done - 2026-09-29.** User: "im still getting the stuttering, but only on
the fmv at the beginning?" after D23A, then "make the playback more robust".
Cause measured: the native MOVIE.OVR shard's cache folder is keyed by the
`game.local.toml` overlay config hash, which includes the host hook list.
The 28 Sep hook additions moved it from `gc01bd77ae` (last shard, 27 Sep) to
`gc15256f69` (empty), so the intro's VLC decoder ran interpreted: ~52 fps,
continuous underruns, 30 % interpreter share. The loader reported this only
over the debug port. Shard rebuilt: 59.92 fps, 0 underruns, fill 267 ms,
4 % interpreter; the user confirmed smooth playback. Hardening: always-run
`ttk-movie-shard` CMake target in the `local-dev` preset, `run.py` pre-launch
check (`build_movie_overlay.py --if-ready --quiet`, ~0.2 s when current),
and a `ttk-fmv:` session-log line from `fmv_poll.c` that reports the native
decoder or a WARNING with the loader's mismatch message. See
[59-fmv-shard-namespace.md](documentation/59-fmv-shard-namespace.md).

**Acceptance:** the intro FMV plays at full speed without audio underruns
after hook changes to `game.local.toml`, and a missing shard is repaired on
build/launch or reported in the session log.

### D24 — Linux / Windows player build and disc import

Create reproducible player builds with a simple launch flow, settings/save locations and clear owned-disc import errors. Verify Windows independently rather than extrapolating from Linux. Package permitted runtime components; keep original disc assets and personal saves out of redistributable artifacts.

**Acceptance:** a clean installation on each claimed platform imports the supported disc, launches, saves/loads and closes successfully. Document dependencies, licenses, build identity and known limitations in the player instructions.

### D25 — Modernized edition release acceptance

Run the documented campaign and regression checks against both presets in a frozen candidate build. Reconcile manual, settings, supported platforms and feature claims. Choose release scope explicitly; optional first-person or HD packs need their own completed acceptance work to be advertised.

**Acceptance:** no known release-blocking progression, input, save or playback issue in the claimed scope; comparison evidence shows Vanilla preserved and Modernized usable; reproducible build and player instructions are complete. Record remaining issues visibly.

### D26 — Backtick debug console (fps and helpers)

**Done — explicit user acceptance (2026-09-28).** Console look and behaviour signed
off: scrollback echo, multi-line `help`, unknown/errors in console only, `fps`
persistent top-left debug block, `clear` / `quit`. F unbound; Scroll Lock is holster.

**Acceptance met:** backtick open/close; scrollback; persistent fps overlay;
gameplay/capture/menus intact.

### D27 — Caps Lock RUN MODE quotes and Shift-run clunk silence

**Done for Caps Lock quotes (user accepted). Shift-run clunk silence deferred
low-priority (2026-09-28).** Plant-aligned gait delay was rejected: longer Shift→run
lag and clunk still audible. Restored immediate frame-0 walk↔run restart (responsive
gait with known plant SFX clunk). Soft mid-stride phase reuse remains forbidden
(prior freeze). Future work: identify plant SFX path and suppress only on Shift
gait restart without delaying the switch — not blocking.

**Acceptance (quotes):** met. **Acceptance (silent Shift-run):** deferred.

### D28 — Scroll Lock holster and WEAPON LOWERED/RAISED quotes

**Done — explicit user acceptance (2026-09-28 morning).** Scroll Lock holster and
centered quotes confirmed in playtest.

**Acceptance:** default binding + quotes; custom rebinds; E auto-stow/ladders;
manual/game manual updated.

## Working rules and evidence

- Work on the selected job; split unexpectedly large discoveries into linked follow-ups with new stable IDs. Never renumber existing jobs.
- Inspect the actual workspace before changing it. The Git checkout is inside `recomp/`; framework submodules and local patches may already be modified. Preserve unrelated work.
- Keep original media untouched. Use the project's import/build tools, maintain runtime changes through the existing patch workflow and do not hand-edit generated recompilation output.
- Test shared fixes in Vanilla and test feature behavior in the selected modern configuration. Do not launch over an active player session or use their saves for automated experiments.
- A job may finish with documented research when its stated deliverable is research. Implementation jobs need working behavior. Use Needs playtest when user-visible acceptance is still unverified.
- Update this board, [status](documentation/00-status.md), [handoff](documentation/09-handoff.md) and relevant engineering notes as appropriate. Update the manual whenever player controls or behavior change.

### Work log format

Append an entry when a job starts or its status changes:

```text
Date / job ID / status:
Scope and decisions:
Files or build identity:
Verification and evidence:
Remaining limitations / next action:
```

### 2026-09-25 — Backlog created

Planning only. All modernization jobs are Todo. No gameplay, renderer, input or save behavior changed while creating this list. The first recommended job is D01; the user chooses what to work on next.

### 2026-09-26 / D01 / In progress

Scope and decisions: User authorized proceeding autonomously in the prescribed order. Establish isolated, recorded Vanilla checks before dependent implementation. Preserve all existing checkout/submodule edits.
Files or build identity: Existing Linux candidate; identity will be captured per run.
Verification and evidence: No active game process found before testing. Existing user security-card success remains separate from automated evidence.
Remaining limitations / next action: Record repeatable boot/input checkpoints and identify unverified progression/save steps.

### 2026-09-26 / D01 / Needs playtest

Scope and decisions: Added a fresh-card, recorded Vanilla input runner and a documented extended progression/save/audio/window route. User authorization to continue autonomously in prescribed order remains in effect; do not ask them to select D02 again once D01 acceptance is met. D01 is not Done solely because the bounded driver exits successfully.
Files or build identity: `recomp/tools/local/vanilla_regression.py`, `documentation/16-vanilla-regression-route.md`, `documentation/reports/d01-regression.json`. Existing executable SHA-256 `1d280e754974854393048322b59d65535214eee97cddbcdadc57a58f58d4f038`; no runtime or generated game code changed.
Verification and evidence: Fresh-card headless runs and visual checkpoint review. Two startup-timing routes failed semantically; two subsequent runs reached gameplay but exposed the test incorrectly holstering the initially drawn pistol. Corrected route verifies firing/ammo reduction, draw/holster, jump, inventory, movement/turning, damage and debug-command shutdown. Active-game refusal was exercised without creating a test directory. Exact attempts, timings and review results are in the report; local captures/logs stay in ignored analysis directories.
Remaining limitations / next action: Perform the extended route with recorded security-card input/timing, an original save/load round trip, death/restart and audible/window checks. Earlier user card success and existing window-close tests remain separate evidence. D02 and other D01-dependent jobs remain Todo until this gate is met; inspection found runtime settings beside the executable override game TOML, which future profile plumbing must account for. No changes to personal memory cards or original media.

### 2026-09-26 / D01 / Done; D02 / In progress

Scope and decisions: User confirms after playtesting that the game works to a very high Vanilla standard and authorizes continuing. Accept that overall playtest sign-off alongside the documented route and recorded bounded checks to close D01. This is user-reported acceptance, not newly instrumented proof of every individual save/campaign check. Existing detailed evidence gaps remain labelled in the route report.
Files or build identity: Existing candidate unchanged at D01 sign-off. Begin persistent launch-time profiles in the local launcher for D02.
Verification and evidence: User's latest confirmation; two corrected isolated D01 replays documented previously. No active game found at D02 start.
Remaining limitations / next action: Implement and verify independent profile preferences, defaults, persistence and malformed/versioned settings recovery. Modern controls remain unavailable until their implementation jobs; label the Modernized preview accordingly.

### 2026-09-26 / D02 / Done; D03 / In progress

Scope and decisions: Versioned persistent launch profiles with independent renderer preferences, terminal selection/reset, atomic writes, preserved recovery backups and future-version refusal. Vanilla remains default; Modernized is visibly a preferences preview using original gameplay. Begin guarded read-only player/camera research next.
Files or build identity: `tools/local/player_profiles.py`, `tools/local/run.py`, `tests/local/test_player_profiles.py`, `documentation/17-player-profiles.md`, manual and extended regression runner. Native binary unchanged.
Verification and evidence: Nine profile tests pass, including separate CLI process restarts, switching/reset independence, malformed/legacy/future settings, atomic-write failure and terminal cancellation. Actual launcher fresh-card replays in Modernized/software and Vanilla/OpenGL reach gameplay, visually verify shooting, jump, inventory, movement and damage, then exit 0. Reports/captures: `analysis/vanilla-regression/d02-modernized-software` and `d02-vanilla-opengl`. No player saves used.
Remaining limitations / next action: Profile preferences currently control renderer only; other display settings and keyboard/controller bindings remain shared. No modern movement, camera or aim behavior is advertised as implemented. D03 must establish safe game-specific hooks; initial state samples are evidence candidates, not authorization to patch state blindly.

### 2026-09-26 / D03 / In progress — guarded state and write attribution

Scope and decisions: Added strictly read-only executable/live-code/back-reference validation and optional candidate sampling/write tracing on isolated regression runs. Located input-correlated player position/heading, reciprocal camera structures, state bytes and candidate camera matrix/anchor fields. No mutation hooks applied.
Files or build identity: `tools/local/ttk_state_probe.py`, extended `vanilla_regression.py`, four synthetic guard tests, `documentation/18-player-camera-research.md`, `reports/d03-state-research.json`; existing native executable unchanged.
Verification and evidence: Live resident byte guards and object links matched throughout gameplay checkpoints. Corrected write tracing corroborates three position integration stores (23 changed writes per coordinate in forward test) and ten changed heading stores during turning; jumping uses a distinct movement path. All test processes exit 0. First mixed-filter trace is retained as partial evidence. The hybrid trace's `func` field is not reliable attribution; store PCs were cross-checked against original instructions.
Remaining limitations / next action: Continue D03 by tracing delta/target-heading producers, input/collision/animation ownership, camera conventions and active gameplay overlay identities. Establish a bounded weapon-aiming research interface. Do not skip collision with direct coordinate writes or declare a safe hook from these correlations. D04 follows D03 in the user's requested order; no renewed user approval is required.

### 2026-09-26 / D02 user verification; D03 continued

User confirmed the CLI settings launcher works and selected Modernized. Preserve their selection; it remains a preview with original controls. Clarified that no background work was running after the prior final reply and resumed D03 under the existing autonomous authorization. Expanded isolated tracing to target heading and movement-delta producers, with additional live code guards.

### 2026-09-26 / D03 / In progress — producer trace and first-map overlay

Scope and decisions: Continued after user asked for visible progress. Captured target-heading/delta producers and compared gameplay RAM with all 30 unique owned-disc overlays. Preserve the user's Modernized selection; tests use isolated Vanilla settings.
Verification and evidence: Fresh-card trace exits 0. LEVEL00.OVR matches all 9668 bytes at 0x800CA968; LEVEL01 partial chunks are explicitly rejected as residency proof. Five guard tests plus captured-RAM replay pass after adding an exact first-map overlay guard. Producer stores and source cross-checks documented in note 18 and reports d03-input-producers/d03-overlay-correlation.
Remaining limitations / next action: Continue input increment, collision and callback ordering research. Do not treat the read-only probe as a finished mutation hook. No further user acceptance is needed to continue D03.

### 2026-09-26 / D03 / Done — verified bounded hook contracts

Completed live function-entry and write attribution for first-map walking, input-to-heading and normal camera ownership. Walking entry 0x80053500 precedes collision; mode-4 collision probes follow facing, so D05 must coordinate movement and probe direction. Verified normal-camera entry 0x8003ADE4 and inner constraint stage 0x8003AA48, including arguments/callers. Corrected matrix transpose and angle-extraction semantics. Added fail-closed call-context checks and expanded live identity guards.

Verification: fresh Modernized/OpenGL and Vanilla/OpenGL research routes exit 0; final route validates twelve live code ranges and exact LEVEL00 identity before/after traced intervals. Ten diagnostic tests pass. Reviewed fire, jump, movement and turn captures; fire-input captures retain ammo 200 and are not successful-shot evidence. See documentation/18-player-camera-research.md and reports/d03-acceptance.json.

Remaining scope: D04 input abstraction is next; D05/D06 must implement and test replacements using the documented collision/state contracts. Other maps and traversal states retain original behavior until verified. D07 must trace actual muzzle/ballistic/auto-aim interfaces; no aiming mutation is enabled. Research completion does not mean modern controls are implemented.


### 2026-09-26 / D04 / Done — PC actions, rebinding and safe capture

User authorized D04 autonomously and reports continued high-standard verification
playtests of the preceding candidate. That feedback is retained separately from
new D04 automated evidence.

Implemented a native game-specific keyboard/mouse action layer, independent
normalized movement axes and per-frame relative look data; no movement/camera
mutation hooks were introduced. Modernized has persistent action bindings,
atomic conflict-rejecting edits, fixed menu navigation, explicit F10 capture and
Escape/pause/inventory/focus/host-menu release. Vanilla retains its original
keyboard/controller translation; physical pad state is neutral while unfocused.
Profile schema 2 preserves existing selections/renderers through backed-up
migration. The manual documents implemented actions and the unimplemented
WASD/mouse-camera boundary.

Verification: native player and SDL tests build; 42 Python tests pass (two
owned-disc integration cases skipped), native SDL state-transition checks pass,
and a real terminal binding edit rejects a conflict and survives restart.
Thirteen Modernized and five Vanilla window input assertions pass on isolated
Xvfb displays. Reviewed captures verify menu back in both profiles and final
Modernized menu-to-first-level navigation. The established Vanilla regression
route exits 0 with reviewed spawn, fire/ammo decrease, jump, inventory and
movement/turn captures. Ordered patches reproduce the live runtime from pristine
pinned files and pass idempotence checks. All cards/preferences are isolated.
See [D04 contracts](documentation/19-pc-action-input.md) and
[evidence](documentation/reports/d04-input.json), including earlier probe failures
and their narrower evidence boundaries.

Remaining scope: D05/D06 must apply the action data through guarded movement and
camera hooks. Hardware controllers/Windows/desktop-compositor testing and
full-campaign fidelity remain unverified. Custom runtime host shortcuts must stay
separate from PC action bindings. No next job was started.

### 2026-09-26 / D05 / Needs playtest — guarded camera-relative locomotion

Implemented horizontal intent before the original run and walk collision paths,
coordinating displacement and directional probe without writing actor facing.
Original animation controls speed; diagonals normalize, Shift selects original
walking, and deceleration preserves the last direction. Vanilla exits untouched.
Full code/LEVEL00 hashes and actor/caller/state/stack checks fail closed.

Verification: normal build/import/generation pipeline passes; geometry and owned-data
native guard tests pass. Isolated SDL route `d05-movement-02` exits 0 and checks
W/S/A/D, walk, diagonal, rotated-camera movement and capture release. Its walk
capture was visually reviewed. The first route's walk failure and deceleration
issue are retained, followed by the correction. See [contracts](documentation/20-modern-movement-camera.md).

Remaining acceptance: walls, slopes, steps, long diagonal speed comparison and
combat/aim interaction need playtesting. Other states/maps fall back to original
controls. D06 follows under the user's explicit two-job authorization.


### 2026-09-26 / D06 / Needs playtest — independent constrained mouse orbit

Implemented yaw/pitch mouse orbit before the verified original camera constraint
stage, with cumulative count consumption, capture-epoch resets, ±60° requested
pitch, sensitivity/inversion and selectable original camera. Profile schema 3
backs up/migrates earlier preferences while preserving bindings and rendering.
Actor facing and weapon rays remain original. Updated the manual, status, handoff,
[engineering contracts](documentation/20-modern-movement-camera.md) and
[D06 evidence](documentation/reports/d06-camera.json).

The first combined camera probe reached death despite numeric success; reviewed
screenshots rejected it. Tightened shared guards to the verified animation set,
death/heading-inhibition flags, exact code/overlay and caller/stack ownership.
Also finished D05's separate stop-animation hook so its deceleration follows
movement intent. Failed/superseded probes remain in the evidence trail.

Verification: full normal import/generate/build and final incremental build pass;
46 Python tests pass, two are skipped; native SDL and owned-data callback/guard
checks pass. Final `d05-final`, `d06-acceptance` and `d05-d06-final-vanilla` routes
all exit 0 with the exact same final binary hash and isolated cards/preferences.
Reviewed Modernized captures show camera changes while actor yaw stays 1537,
movement and real firing (ammo 200→198→197→196). Look-consumption count stays 27
across release, focus loss and pause/resume despite uncaptured motion. Reviewed
Vanilla captures show original spawn, fire/ammo, jump, inventory, move and turn.
The pre-existing runtime submodule diff is byte-preserved.

Remaining acceptance: D05 walls/slopes/steps, sustained speed/animation and
precision-aim combinations; D06 real desktop refresh rates, full yaw/pitch and
obstruction tests, recentering and state/script/progression transitions. Batching
math is not hardware refresh-rate evidence. Other maps/traversal retain original
controls; forced interpreter, Windows/controllers and full campaign remain
unverified. Both jobs stay Needs playtest. Stopped after D06; D07 was not started.


### 2026-09-26 / D05 and D06 / Done — user playtest sign-off

The user tested the Modernized build and explicitly confirmed that WASD movement
and the independent third-person mouse camera both worked, then requested that
both jobs be marked Done. Recorded as user acceptance alongside the existing
build, automated and visually reviewed evidence. D01–D06 are now Done; D07's
dependencies are satisfied.

This report does not newly establish every terrain, refresh-rate, obstruction,
script, platform or campaign case. Those remain coverage limitations and future
regression work, not a request to repeat the accepted D05/D06 milestone. The user
expects modern weapon aiming/crosshair next and will invoke the skill for D07
after clearing the conversation. D07 remains Todo and was not started here.


### 2026-09-26 — D07 bounded aiming implementation; Needs playtest

Implemented guarded Modernized view-to-muzzle aiming for selected first-map
projectile paths, with camera target queries, muzzle parallax and body-to-muzzle
cover checks. Original projectile integration/collision/spread remain active.
Added a Software/OpenGL crosshair and persistent view/original aiming choice
(schema 4), preserving old preferences with backups. Vanilla is unchanged.

The real starting pistol is a fast swept projectile, not instantaneous hitscan.
Launch, flight and wall-impact observations agree on static walls within the
recorded rounding error; a close-wall replay is reviewed. Native collision
fixtures test protruding-muzzle retraction separately from retail geometry.
Synthetic weapon grants/selection are explicitly separated from normal gameplay.
Original key-based selection failed its assertion, and a long synthetic sweep
was rejected on death. These failures remain in the evidence.

Acceptance is incomplete: damaging beam/hitscan paths retain original aiming;
normal weapon acquisition/selection and special states are unverified. Actor-hit
accuracy, around-corner cover, pitch/movement combinations, precision aim and
broader playtesting remain. This status is not an all-weapon completion claim.
See [contracts and coverage](documentation/21-modern-weapon-aiming.md) and
[D07 evidence](documentation/reports/d07-aiming.json) for final candidate checks,
failed runs and remaining gaps. D01–D06 remain Done. No D08 work was started.

### 2026-09-26 — D07 Done: user acceptance of this iteration

The user explicitly accepted the current iteration after confirming that mouse
look changes view/shot direction without turning Duke's body. Marked D07 Done on
that bounded user sign-off, not on additional automated or all-weapon evidence.
All documented weapon/actor-hit/reticle/cover gaps remain. D07A records the agreed
next focused iteration for body/weapon alignment before D08. It is Todo; no
implementation was started during this acceptance/documentation update.


### 2026-09-26 — D07A implemented; Needs playtest

Modernized independent-camera/view-aim mode now steers Duke's horizontal facing
from the actual view, both holstered and with supported drawn weapons. Existing
upper-body aim animations receive the view direction, including pitch, through
a guarded argument hook. Camera-relative movement/probes, original animation
speed and body pitch remain separate. Original camera/weapon options retain their
paths; Mouse2 precision aim suspends presentation and view-shot overrides.

Draw/holster research corrected D07's run-flag interpretation: equipment state,
not player+0x224 bit 2, now gates drawn-weapon presentation/reticle eligibility.
This is not a resolution of the earlier pellet reticle discrepancy or general
firing/animation continuity. Idle/recoil/transition poses and forward locomotion
footwork remain original; rapid view motion can have a simulation-update phase lag.

The normal generation/build, 52 Python tests (50 passed, two skipped), native
input/control/weapon fixtures, isolated facing/camera/original-camera movement,
original-aim, pitch/walking presentation and Vanilla routes pass. Reviewed captures include real draw/holster,
stationary/moving pistol fire, and Mouse2 plus arrows/fire. Failed early routes
and the corrected research interpretation remain in the evidence. See
[D07A contracts](documentation/22-view-facing.md) and
[evidence](documentation/reports/d07a-facing.json) for exact identities and limits.

Needs playtest: responsiveness and body/gun appearance across poses, yaw/pitch
extremes, sustained strafing/backpedaling, terrain and naturally acquired weapons.
All D07 weapon/actor-hit/reticle/cover gaps remain. D01–D07 remain accepted Done;
D08 is untouched. Settings/cards and pre-existing runtime changes are preserved.


### 2026-09-26 — D07A user feedback / follow-up In progress

User reports strong improvement in controls, but camera/movement can stick near
walls and in the strip club entrance/interior, requiring wiggling to escape.
Selected follow-up: reproduce and correct bounded wall recovery; Modernized
walk by default, hold Shift to run, Caps Lock toggles autorun (initially off).
Preserve existing bindings/preferences and original options. Clarify the original
auto-aim targeting dot and current view/original aiming choice; modern aim-assist
polish remains future work. This is not D07A sign-off or authorization for all D08.

### 2026-09-26 — Future graphics research requests

Confirmed D13 for high-resolution settings and D14 for widescreen; both remain
Todo. Added D16A for later HRP investigation, with first-person weapons the first
asset priority and interface/overlays/model replacement also in scope. No graphics
implementation or D08 work started. More research components may be added later.

### 2026-09-26 — Future menu request

Expanded D19 to modern in-game menus with EDuke32 styling/interaction reference
and a code-reuse feasibility investigation. Remains Todo; no menu code imported.

### 2026-09-26 — D07A feedback implementation

User reported wall/tight-room sticking and requested EDuke32-style controls,
walk by default, Shift run and Caps Lock autorun. Corrected the idle clearance
probe to follow WASD before movement starts; retained independent orbit during
verified wall-bump animations 94/95 without enabling locomotion/body hooks there.
Collision results and position integration remain original. Existing saved speed
bindings are retained, with no settings migration; autorun resets off each launch.

An isolated before/after approach outside the strip-club entrance recorded zero
horizontal backpedal before the fix and roughly 580 units after. The final live
route verifies walk/run/Caps combinations and approximately full camera turns in
both directions, with no death. The earlier longer route reached death and is
explicitly excluded from full acceptance. Native guards/input/aim tests and 50
Python tests pass (two additional owned-disc cases skipped). Final replay details
are in [feedback evidence](documentation/reports/d07a-feedback.json).

Original autoaim/red-dot meaning is documented; current View aiming still omits
that correction for supported shots. D07B records later assisted-view-aim research.
D07's weapon/acquisition/actor-hit/reticle/cover gaps remain. User must retest the
reported strip-club interior/corners, sustained movement and presentation. D01–D07
remain accepted; D08 and the newly recorded graphics/menu/HRP work are not started.

The subsequent tight-wall sweep reproduced a second issue missed by the earlier
away-from-wall sweep: final look-at orientation pinned yaw to the constrained
boom. Added a guarded requested-view matrix at the original final orientation
conversion, retaining position collision, smoothing and room calculations.
Recapture now seeds visible yaw/pitch rather than the displaced boom. The final
wall replay turns 360° each way at the wall (sampled yaw error below 0.19°) and
backs away about 550 game units. Original near-camera body fading is visible.
The first attempted hook used an incorrect return guard and remained inactive;
its failed replay is retained separately, not accepted as proof.

Final verification completed on binary `ac5f8b1d7df6eccf866df8873aa720ff64014bf286e2b30d3295fbb748de4203`:
seven isolated final routes pass (wall, facing, original camera, independent camera,
pitch/presentation, original weapon aiming, Vanilla). D07A is Needs playtest.
Current original-aim replay fires with modern shot/facing/arm counters at zero
and no modern reticle. User cards/settings and existing runtime diff are preserved.

### 2026-09-26 — D07 continuation authorized

User reports much improved aiming with slight audio choppiness and authorizes
D07 follow-up fixes and D07B research/implementation autonomously, then possible
D08 scaffolding. Investigating E interact, sidestep bindings, speed-transition
latency, backward running jumps, numbered weapons, independent assist/crosshair/
red-dot options and runtime overhead. Existing user settings/cards preserved;
all gameplay probes use isolated sessions. Earlier stop-after-D07A restrictions
are superseded by this request.

### 2026-09-26 — D07A/D07B continuation implemented; Needs playtest

Implemented E interaction while holstered, comma/period legacy sidesteps,
immediate guarded normal walk/run gait changes, one-time backward-running-jump
velocity alignment with leased airborne camera ownership, and number-key weapon
requests completed by the original equipment sequence. Custom bindings survive
migration and take precedence over numbered weapon shortcuts.

D07B now exposes three independent saved preferences: optional `original-lock`
assistance (default off), modern crosshair visibility, and original red-dot
visibility. Assistance uses an existing game lock within six degrees and requires
both camera and physical muzzle traces to identify that same actor. Marker hiding
changes only the verified player-marker primitive. Original aiming/Vanilla remain
available. Local EDuke32 settings were read for input conventions; no executable,
code or assets were imported into this project.

Hot guards now authenticate expected bytes once and compare all live words on
every invocation, including the complete guarded LEVEL00 range. Same-frame writes
still refuse the hook. Short dummy-audio timing improves toward 60 guest fps;
zero added output underruns do not establish that audible choppiness is fixed.
The player must still listen during ordinary play, dialogue and FMV.

Build/import/generation completed; three native suites passed and Python discovery
ran 55 tests (53 passed, two owned-disc skips). Final replay identities, visual
review and bounded pistol-to-original-actor contact evidence are recorded in
[the continuation report](documentation/reports/d07-continuation.json).
One earlier control route was rejected during visual QA because enemy fire killed
Duke before landing; its output is retained rather than counted as acceptance.

D01–D07 stay Done. D07A/D07B are Needs playtest for feel, interior/corner behavior,
natural weapon acquisition, moving targets/cover, progression and sustained audio.
D08 remains Todo with [its state matrix scaffold](documentation/25-traversal-state-matrix.md);
no broad traversal whitelist or complete campaign compatibility is claimed.
See [implementation contracts](documentation/24-d07-controls-and-aim-options.md).

Final verification accepts seven bounded delivery-binary gameplay routes, including
Software/OpenGL marker configurations, original aiming, wall/camera regressions
and Vanilla. Vanilla quit RPC is inconclusive (`emu busy or frozen`, process exit
0); no new shutdown fix is claimed. Final preservation checks pass for player
settings/cards and the pre-existing runtime diff. Delivery SHA-256:
`0758fa99986cac9c54bed12f25f5b760f7cf4e781bc9738f17979b39c57e7558`.

### 2026-09-26 — D08 movement/traversal feedback authorized

User confirms backward jumping and improved aiming, but reports intermittent
sideways jumps going forward, ladders stopping after one or two steps, inability
to climb brick walls in captured mode, and intermittent F10 recovery. Requests
continuous running after landing rather than the original hard stop. D08 is now
In progress for these fixes; retain D07A/B coverage limits and Vanilla behavior.

### 2026-09-26 — D08 movement feedback implemented; Needs playtest

Fixed the unhandled running-jump variant 103 alongside 104, explaining intermittent
forward launches during sideways runs. Held locomotion now reaches the original
landing selector during owned jumps, so clear landings return to running rather
than stationary recovery. Original obstacle impact and collision remain intact.

Authenticated holstered attached-state input handoff fixes the reproduced ledge
hang and ladder stall. W/E now completes the fenced car-park pillar pull-up and
subway-alley ladder ascent/exit without uncapturing. Attached motion/facing/camera
remain original; no broad movement-hook state whitelist was added.

Build, three native suites and 55 Python cases (53 passed, two skips) pass.
Direction and landing assertions, reviewed traversal segments and final regression
results are linked in [the delivery report](documentation/reports/d08-movement-followup.json).
Test-only health fixtures and rejected harness experiments are explicitly separated
from acceptance. Player files and existing runtime changes are preserved.

D08 is Needs playtest for feel and the broader state/progression matrix. D07A/B
remain Needs playtest; the audio report is not resolved by listening evidence.
F10 recapture checks pass, but no distinct timing defect was reproduced or fixed.

Final delivery binary: `21e062dd4eb6b17d1ee8b9679a69a79d2e2a07e8c6130ac382823407dbb0a64a`.
Wall/camera/focus/pause and bounded Vanilla regressions pass on this binary; all
private test processes have exited. Final preservation checks pass.

### 2026-09-26 — D08 polish follow-up authorized

User confirms ladder/ledge use and continuous running after jumps in all directions.
Investigating occasional stops, respawn camera zoom, E parity with left-click, C
holster preference, automatic Modernized gameplay capture and lighter movement.
Preserve original collision/traversal and optional Vanilla.

### 2026-09-26 — D08 control polish delivered; Needs playtest

User's correction is authoritative: **E only interacts, never fires.** A tap now
pulses the original holster request once, waits for holstered ownership, and emits
an unarmed interaction. Holding E continues supported actions. C is the default
holster/draw key; old H defaults migrate to C with a backup. Other custom bindings
and explicit profile choices survive. New settings default to Modernized and
verified gameplay captures automatically, including pause resume. Explicit
Escape/F10 release opts out for the session.

Respawn camera recovery retains its requested boom, including after collision or
original-camera ownership. Enemy death/restart replays recover to normal distance
without F10; toggling afterward no longer materially changes distance. E-only
segments consume no ammo and emit no shots. Separate Mouse1 engagement is labeled.

Both takeoff 9/0 and landing 0/9 transitions are now covered by the existing owned
jump contract. Trace research confirmed a missed 9/0 update could expire intent
before 9/9; short run-ups also retain verified normal-camera input direction.
The final six-jump route records six once-only corrections, both variants and all
four directions; clear landings continue running, original impacts keep recovery.
Normal stride displacement is filtered before original collision integration and
released-input braking shortened. Original vertical/traversal motion remains intact.

Three native suites and 54 Python tests pass (two owned-disc skips). Delivery
jump/Vanilla replays and unchanged-path supporting E/respawn/wall replays have
separate binary identities in [the evidence report](documentation/reports/d08-control-polish.json).
[Implementation notes](documentation/27-d08-control-polish.md) and the manual are
updated. Settings/cards and the pre-existing runtime diff are preserved. D08 is
Needs playtest for feel and broader state/progression coverage; D07A/B stay Needs
playtest. Audio still lacks listening acceptance.

Delivery SHA-256: `1acd362bfdfae52853d4199cf082ea47af0243c05be1c3a8420d775f1e17c95c`.

### 2026-09-26 — D07B / D08 close combat and temporary holstering delivered

Automatic E holstering now restores the weapon after the original action/ladder
exit completes. Manual C holstering is preserved; E remains interaction-only.
A guarded close-range muzzle retraction corrects a reproduced backwards-parallax
case; camera-to-player obstructions cannot select a target behind Duke. Original
projectile collision/damage and Vanilla are preserved.

The delivery replay mounts and climbs the apartment ladder, automatically redraws,
and kills both apartment pig cops with observed original contacts and health loss.
One encounter is at contact distance and exercises the retraction safeguard.
Armed E-only consumes no ammunition or shots; manually holstered E stays holstered.
Native suites, 54 Python tests (two skips), and the delivery binary’s bounded
14-checkpoint Vanilla route pass. Private cards and logged
player-health fixtures are used; no enemy-health or movement fixtures.

See [notes](documentation/28-d08-close-combat.md) and
[evidence](documentation/reports/d08-close-combat.json). D07A/B and D08 remain
Needs playtest for feel and broader progression/weapon/state/campaign coverage.
The specific adapter defect is proven, not every cause of the user's earlier
misses. Audio listening acceptance remains open.

Delivery SHA-256: `c6e6b5a0769ba74a9f49b26a4d1f6c839e7dac052c388b73e243c677ae1d80c4`.

### 2026-09-26 — Column-area PC-zero exit investigated; reset safeguard delivered

The user strongly accepted the current controls, then reported an unexpected exit
beside the atomic-health fence. Preserved full RAM establishes a stale world-object
node inside a current enemy-animation buffer, selecting a null handler. The exact
preceding transition was not reproduced; fresh traced starts clear their old lists.

Added a byte/caller/list-guarded no-callback cleanup before scene-arena reset. It
matches the original MIPS release routines on the actual 42-node crash-RAM fixture,
including complete RAM equivalence apart from temporary original stack-save words.
It does not skip live handlers or alter the accepted controls/aiming/audio sources.
The decoder now identifies scene-list membership and animation-slot overlap.

Native scene/input/control/aim suites and 54 Python tests (two skips) pass. The
extended fenced-area/combat replay exits 0 with a valid 46-node list; automation did
not complete the column overclimb. Original enemy death/restart restores the normal
camera without F10 repair, and the delivery binary passes the 14-checkpoint Vanilla
route. All tests use private cards and all owned processes have exited. Player
settings/cards and the pre-existing runtime diff are unchanged.

D08 remains Needs playtest: the safeguard prevents stale-list survival at reset;
it is not proof of the original transition or all subsequent lifetime paths. Broader
progression/security-card/campaign and audio listening acceptance remain open.
See [investigation](documentation/29-column-exit.md) and
[evidence](documentation/reports/column-exit.json).

Delivery SHA-256: `a607e052a401a04a6184af31f557c5a3719bc85ad353117aa95958160e7b78dd`.

### 2026-09-26 — D08 Escape / Start and responsive taps delivered

Implemented the user's selected menu/input follow-up. Escape aliases Start and
retains automatic capture after pause. F10 remains the explicit toggle. Added
an eight-frame grounded jump buffer and eighteen-frame airborne unarmed E window;
E remains interaction-only. Original collision, jump ballistics and aiming remain.
Native suites pass, Python 54 passed/two skipped. Private OpenGL replay verifies
Escape intro skip/pause/resume and six brief running jumps with correct direction;
four clear landings continue running, two obstacle hits keep original recovery.
Exact second-ladder edge timing/midair catch still needs playtest. D08 remains
Needs playtest; broader D07A/B and campaign/audio limits remain. See
[notes](documentation/30-responsive-input.md) and
[evidence](documentation/reports/d08-responsive-input.json).

### 2026-09-26 — D08 ladder transfer mechanic follow-up delivered

The user authorized revisiting the mechanic, beyond input tap buffering.
Reproduced held E losing Cross when the original running jump enters reach 109;
the old build misses the second ladder. Preserve owned reach input/camera, retain
one E reach intent through flight, and replace the legacy edge-delay forecast
with current support for guarded running takeoff. Original collision and jump
range/gravity remain; no position or velocity assistance was added.

Candidate and delivery both catch the actual second alley ladder and climb to the
upper platform. Delivery starts armed, auto-holsters, releases E before contact,
and redraws after climbing; ammo/shot counters unchanged and no C input.
Six directional jump regression checks and native suites pass; Python 54 passed,
two skipped. Human fluidity and broader traversal/campaign remain open: D08 is
Needs playtest. [Contracts/evidence](documentation/31-ladder-transfer.md).

Final ladder-transfer verification: delivery 14-checkpoint Vanilla replay exits 0; firing, jump, inventory and turn screenshots reviewed. Private sessions stopped; saved player settings/cards unchanged.

### 2026-09-26 — D08 Done: explicit user controls acceptance; D08A queued

The user explicitly signed off the current controls: every tested ladder jump
was exactly as expected, E caught the next ladder reliably, and movement delivered
the requested fluidity. Marked D08 Done for this accepted iteration. Preserved the
original wider traversal/script/campaign requirements as D08B Todo; this is not
an unsupported claim of complete campaign or swimming coverage. D07A/B retain
any separately documented aiming/option coverage gaps.

Added D08A Todo for the requested comprehensive EDuke32-style weapon/item mapping,
including M portable medkit and 6 pipe bombs. Read-only local reference checked;
no implementation or new gameplay test started. User will clear context and invoke
the skill again to select/continue work. Current build, saves and settings untouched.

### 2026-09-26 — D08A research and controls planning (Todo)

Added a pinned official EDuke32 checkout under `research/eduke32/`, separate
reference copies of the installed config, and a local US TTK manual scan. Compared
source weapon/input definitions and visually reviewed the manual's weapon/gadget
spreads. [The proposal](documentation/32-eduke32-weapon-item-plan.md) maps all base
weapons/items, records missing equivalents, Q kick feasibility, wheel/remote edge
cases and exact source pointers. User-selected Ctrl held crouch and Alt-wheel
camera distance are recorded; surrogate keys remain recommendations.

Evidence is read-only source/config/manual research, not gameplay verification.
No game code, build, player settings/cards or original media changed; no game
launched. D08A stays Todo pending implementation, D10 owns distance adjustment,
and D07A/B retain Needs playtest. No next job started.

### 2026-09-27 — Authorized D08A, held crouch and bounded D10 controls pass

Implemented the authorized number groups from the EDuke32 research proposal,
verified against TTK's executable name/record tables, with owned/ammo eligibility,
upgrade resolution, pipe-bomb remote access, wheel cycling and successful-equipment
history. M/J/N/B and brackets/U invoke original item paths. R has no stored-dose
operation because steroids activate on pickup. Q invokes original standing Boot
melee only when already selected; independent armed quick kick remains unsupported.
Original lit dynamite retains its fuse and pending-switch behavior.

Ctrl now requests the original crouch transition immediately, accelerates only its
body-track transition budget, and stands on release after original room clearance.
Alt-wheel adjusts a clamped/smoothed distance preference, separately from original
wall compression and without weapon input or FOV changes. This is only the bounded
D10 distance portion; shoulder/recenter and broader camera acceptance remain open.

Schema 7 explicitly migrates old defaults, preserving customized bindings. SDL
replay exposed and corrected frontend/demo capture, settled weapon-pose/history
recognition and host-poll/guest-update command loss. Native fixtures cover code,
state, clearance, interrupted actions, distance and existing controls contracts.
[Focused notes](documentation/33-controls-shortcuts.md) and the
[verification report](documentation/reports/d08a-controls.json) distinguish
native fixtures, original-game SDL replay and outstanding human acceptance.
Tests use private settings/cards and log health/inventory fixtures. Original media,
player cards and pre-existing local changes were preserved. Generated C came only
from the established build/import/patch workflow. No unrelated job was started.

D08A and the crouch/distance additions require human playtest. D08 remains Done
on its prior explicit acceptance; this does not close D07A/B or D08B coverage gaps.

Final verification: build/import/patch workflow, all 57 Python tests (including
owned-disc integration), and input/controls/aim/scene native checks passed. The
complete original-game SDL replay and 14-checkpoint Vanilla route exited 0 on
`fdc8d4ece50a4a98248933684e5efa7f51b182b965b3cf7f347c4971e5065c28`.
Crouched/standing and Vanilla spawn/inventory/turn captures were reviewed; this
is not human acceptance. **D08A: Needs playtest.** Crouch and bounded distance:
Needs playtest; broader **D10 remains In progress**, not Done. See the focused
note's acceptance checklist and explicit Q/R/state/dynamite limitations.

### 2026-09-27 — D08C authorized

User praises the controls and confirms wheel weapon selection and Alt-wheel camera
distance work. This is specific human evidence, not blanket item/terrain acceptance.
User selected the proposed standstill directional-jump follow-up. Preserve accepted
controls and original collision; implement and test only this scope.

### 2026-09-27 — D08C directional standing/walking jumps delivered

Reproduced simultaneous W+jump entering original vertical flight. Preserved
direction through original preparation 96, rotated its original clearance query,
and redirected original directional jump 98 once before collision/integration.
A fresh buffered press now starts original preparation from standing/walking;
original walking had no jump-selection branch. Space alone stays vertical,
running selection and active jetpack thrust stay original, and early release
retains the accepted direction. No air steering or position/collision bypass.

Build/import/patch workflow, all 57 Python tests and four native suites pass.
The delivery SDL route passes seven directional cases plus vertical control,
and six running jumps retain direction and expected landing/impact behavior.
Native cases cover interruption, focus/menu release, clearance rotation and
existing E/aim/crouch/wheel/distance contracts. Failed research iterations are
retained and explained in [the report](documentation/reports/d08c-standing-jumps.json).
Private profiles/cards and logged health-only fixtures were used; no player
settings/cards or original media were used for testing. Generated C came from
the established generator. Existing local changes were preserved.

**D08C: Needs playtest** for feel, edges, walls/ceilings and combined E ladder use.
D08 stays accepted; D08B and broader D10 remain unfinished. The user explicitly
confirmed prior wheel/Alt-wheel functionality; this does not close every D08A
inventory or D10 terrain criterion. No other job selected. Delivery binary:
`808878cfc35a02451f720f473b0dd141b74326b659833c4bd75d32a9ad69fecf`.

Final D08C verification: the same delivery binary completed the 14-checkpoint
isolated Vanilla route, exit 0; original jump and inventory captures reviewed.
All private test sessions exited. Source/report/binary identities agree.
Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.

### 2026-09-27 — D08C input-order follow-up

User confirms W→Space works, but Space→W remains vertical. Authorized follow-up
within D08C: retain a neutral takeoff preparation long enough to accept a direction
pressed after jump; preserve the working order, Space alone and original collision.
This follow-up accepts direction during preparation; full airborne steering remains
outside the delivered scope unless separately selected.

### 2026-09-27 — D08C Space-first follow-up delivered

Separated neutral preparation ownership from a latched direction. A Space-only
press now keeps the authenticated 96 camera/input handoff so W/A/S/D arriving
during preparation can choose the original directional jump, without replaying or
restarting the animation. No phantom Forward while neutral and no midair reinjection.
Original clearance and once-only 98 horizontal correction remain in charge.

The isolated SDL route passes six Space→direction cases (all cardinal directions
and two diagonals), Space alone and W→Space; every late-direction start was sampled
in 96 requesting vertical 97 before it became directional 98. All 57 Python and
four native suites pass. Native checks include neutral no-Forward, unchanged
animation progress, late direction, release, focus/epoch and airborne rejection.
See [evidence](documentation/reports/d08c-input-order.json). Human reverse-order
feel and terrain/traversal remain **Needs playtest**. Player settings/cards and
existing local changes were preserved. No unrelated job started. Build:
`92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.

Final input-order delivery also passes the 14-checkpoint isolated Vanilla route,
exit 0; original vertical-jump capture reviewed. All private test sessions exited.

### 2026-09-27 — D08C Done: both input orders accepted by the user

After explicitly confirming W→Space, the user tested the Space→W follow-up and
reported “its working!! beautiful,” then requested a substantial documentation
update. D08C is Done for the accepted directional standing/walking jump iteration.
The job board, current status/handoff, player manual, engineering contract, state
matrix and evidence reports now record that acceptance and the correct current
binary. Historical failed probes and earlier Needs playtest entries remain as
history; they do not override this acceptance.

Human sign-off is distinct from the final delivery's 57 Python tests, four native
suites, eight-case SDL input-order replay and 14-checkpoint Vanilla route. The
accepted behavior is key-order flexibility during takeoff, not midair steering.
Broader D08B terrain/campaign and D08A/D10 outstanding criteria remain open.
This turn changed documentation only: no source, build, settings, cards, original
media or running game changed; no new gameplay session or unrelated job started.

Accepted build: `92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.
Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.

### 2026-09-27 — Requested D07A/D07B/D08/D08A status refresh

Updated the four current job definitions with delivered behavior, linked evidence,
explicit human feedback and remaining acceptance. D07A and D07B stay **Needs
playtest**, D08 stays **Done**, and D08A stays **Needs playtest**, with dependencies
unchanged. Wheel weapon selection is human-confirmed; the full item/era matrix is
still open. D08C's accepted both-order jump behavior is linked from D08. Historical
work-log statuses remain dated history. Documentation-only; no build or game launch.

### 2026-09-27 — User gameplay feedback; D08B and apartment follow-up selected

User reports roughly five minutes of comfortable play: body/gun facing looks good,
no trouble aiming at enemies, M used the portable medkit, number keys and wheel
switched weapons, Q worked with Mighty Boot, and Alt-wheel adjusted camera distance.
This is human confirmation of those behaviors, not an instrumented all-option,
all-weapon/era, full-health/depletion or camera-terrain sweep. D07A/D07B/D08A retain
Needs playtest for their remaining explicit criteria; accepted D08/D08C stay Done.

User authorizes autonomous work on the next suitable job in order: D08B is selected
and In progress. Added D08D for their apartment-secret request, also In progress
for investigation. A player process is active; do not interrupt it or run competing
gameplay tests. Continue static investigation until the user closes their session.

### 2026-09-27 — D08B native suspension fix; D08D original gate located

D08B: reproduced a missing gameplay-context check in attached traversal eligibility;
new native assertion fails before the source fix and passes after it. Three context
gates plus restoration are checked; all existing native controls cases and 57
Python tests (including owned-disc integration) pass. No player rebuild or gameplay
replay while the user's game remains active; D08B remains In progress.

D08D: exact LEVEL00 static analysis locates the lights-off condition requiring
conversation bit 8 and unopened bit 0x400. Candidate switch kind 24 and dialogue
kind 4 require live identity confirmation. No trigger change yet; In progress.
See [implementation/research handoff](documentation/35-traversal-and-apartment-followup.md).
Player binary, settings/cards and original media remain unchanged.

### 2026-09-27 — D08B bounded repairs and D08D convenience delivered

User closed their session. Built using the local import/patch/generation workflow,
then incremental source builds. Three D08B repairs cover suspended input, retained
E-owned redraw and automatic attached-state capture. Native failures reproduced
the first two; log review exposed old helper F10 fallback before the third fix.
Final strict SDL route confirms capture without F10, descent and bottom redraw;
a later enemy-occupied upper exit remains unresolved. D08B stays In progress.

D08D changes only the Modernized lights-off condition, preserving original secret
body/one-shot guard and dialogue. The fixture-negative prerequisite replay opens
the bed, counts once, permits repeat toggles/dialogue and acquires five pipe bombs
naturally; 6 selects without throwing. Untouched no-conversation access is not
claimed. D08D is Needs playtest. This also adds bounded natural pickup/selection
evidence to D08A, without closing its other item/era/depletion criteria.

57 Python tests and four native suites pass. Final build, gameplay/Vanilla reports,
fixture declarations, failed routes and hashes are in
[delivery evidence](documentation/reports/d08b-d08d-delivery.json).
No player cards/settings or original media were modified; no generated C hand edit.
Accepted D08/D08C remain Done; no new job started.

### 2026-09-27 — Apartment accepted; furniture and weapon responsiveness selected

User confirms lights-first opens the bed without conversation: D08D Done. User
also confirms automatic weapon redraw when dismounting a ladder. D08B remains
In progress: new reports describe close-contact jumps stopped at the bed edge,
walking unable to leave bed/couch, and couch approach-dependent collision.
Investigate original support/clearance rules before changing collision. D08E tracks
the explicitly requested click-to-draw and modest weapon-transition speedup.
Accepted D08C input ordering remains valid; terrain response is the new scope.

### 2026-09-27 — D08E delivered; D08B furniture reproduced

Bound fire now draws a settled holstered weapon, then fires only while held.
Original equip/stow upper tracks run at 1.5x; animation events and firing cadence
stay original. 57 Python tests, four native suites and final 14-checkpoint Vanilla
replay pass. Private live pistol checks confirm held fire and short-click release
without delayed shots. D08E Needs playtest for feel and broader weapon families.

D08B: reproduced bed-contact vertical97 fallback, successful stepped-back jump,
normal walking stopped at the edge, and Shift-running off without a jump. Static
walk/run drop dispatch explains the different edge policy. Furniture fixes remain
unimplemented; retain user couch report and broader D08B scope. Enemy-health zero
fixture did not remove the landing actors; later ascent cannot prove stall cause.
No player settings/cards, original media or generated C were edited. See
[focused handoff](documentation/36-weapon-response-and-furniture.md).

### 2026-09-27 — D08E accepted; D08B terrain iteration selected

User confirms click-to-draw/held firing, no delayed shot after release and improved
1.5x transition feel: bounded D08E accepted Done. Broader era coverage remains a
regression concern. User selects D08B furniture work: close-contact jump entry,
walking off small drops and continuous movement through short-drop landing. Also
reports low-priority pipe-bomb collection from the pillow area before the bed opens;
document and investigate after movement. Preserve accepted weapon behavior/Vanilla.

### 2026-09-27 — Furniture iteration delivered; D08B remains In progress

Modernized now rechecks original forward clearance during an owned ascending jump,
using current foot height for the temporary query and retaining the existing vertical
impulse. Walking accepts bounded small drops through the original run/fall path at
walking displacement; short falls preserve live input through landing. One-time
fall direction alignment respects later collisions. Original gravity, damage,
collision and Vanilla remain authoritative.

Final private replay confirms three contact bed entries, normal bed/couch walk-off,
held-Shift bed landing without idle recovery, release to idle and ladder-exit redraw.
Central couch entry succeeds; end/oblique attempts do not establish a general fix.
Pillow sweep did not reproduce the user's early pipe-bomb report; retain it open.
57 Python tests including disc integration, four native suites, state/input assertions
and a final 14-checkpoint Vanilla replay pass. Player settings/cards/state unchanged;
all private games exited. No original-media or generated-C edits. See
[engineering notes](documentation/37-furniture-traversal.md) and
[evidence](documentation/reports/d08f-terrain.json). Human feel and broader D08B
acceptance remain outstanding; no subsequent backlog job started.

### 2026-09-27 — Furniture feedback and debugging cheats selected

User finds close-contact bed entry still inconsistent; a tiny step back helps.
Short-drop landing is substantially better but retains a separate drop phase;
keep further fluidity work open. Light-switch E targeting feels awkward. User
reports no early pipe-bomb pickup this time and confirms natural pickup after
opening; this is successful playtest evidence, not proof of a pickup fix (none
was implemented). D08G is selected for typed Duke-style debugging cheats.

### 2026-09-27 — D08G typed cheats delivered; Needs playtest

Added 12 spellings using original TTK grants/toggles plus hostile-only hide/show,
with captured Modernized input and on-screen feedback. The user supplied the
original Duke 3D list; supported equivalents, omissions and three extra debug codes
are documented in [D08G notes](documentation/38-debug-cheats.md) and the manual.
57 Python tests, four native suites, patch-stack verification, final live cheat
checks and a reviewed 14-checkpoint Vanilla replay pass. Hostiles 9 → 0 → 9 while
NPCs stay three; wounded health retained, ammo toggle verified with actual firing.
Player files unchanged; private sessions exited. Hidden enemies must be shown
before save/load; ownership is session/scene-local. Broader campaign and human
acceptance remain open. Furniture/switch feedback stays with D08B; no pickup fix
claimed. [Final evidence](documentation/reports/d08g-cheats.json).

### 2026-09-27 — User feedback queued before context reset

User accepts the delivered cheats; D08G is Done for that bounded scope. Added
D08G1 for exact Duke3D confirmation wording, D07C for coherent modern aiming/body
facing and pitched RPG projectile coverage, D08H for apartment movement/hidden
pickup/switch interaction, and D19A for supplied Duke fonts. Latest early pipe-bomb
pickup report supersedes the previous non-reproduction. This turn changes planning
and evidence only: no runtime fix, build, launch or player-state mutation.
Recommended next-session order: D08G1 → D07C → D08H → D19A. All four are scoped and
ready; see [resume brief](documentation/39-next-iteration-brief.md) for exact feedback,
asset inventory, source leads, acceptance and a ready-to-paste continuation prompt.

## 2026-09-27 — D08G1 exact wording

Changed only success strings in `cheats.inc`. Private Xvfb/cards/profile run
`iteration40-wording-baseline` reviewed all ten result captures, including both
god/monster directions and dnstuff/dnitems. Added debugging-code and refusal
messages retain their distinct wording. D08G1 Done; D07C follows in selected order.

### D07C implementation — 2026-09-27

Reproduced held-aim facing separation and the rejected RPG caller before fixing them.
View input, supported weapon callers and the energy post-constructor now share
physical view aim. Base-family held/unheld, pitched RPG flights/explosions, sustained
streams and isolated upgraded-dispatch fixtures pass. Native input/controls/aim
checks and 57 Python checks pass. Needs playtest for feel and broader campaign
coverage; see [implementation evidence](documentation/40-feedback-implementation.md).

### D08H implementation — 2026-09-27

Reproduced concealed pickup, contact-jump failure, switch misses and small-drop
slowdown in isolated original apartment routes. Fixed animated-foot obstacle
clearance, bounded approach-speed inheritance, the concealed callback and the
original switch ray intersection. Fresh concealed/exposed pickup, repeated switch
angles, subsequent NPC conversation, furniture drops/jumps, release, large falls
and ladder/redraw checks pass. Needs playtest for subjective feel and broader
campaign coverage; original collision and secret progression remain authoritative.

### D19A and final verification — 2026-09-27

Done for the existing message-renderer scope. SHA-pinned local PNG assets generate
a deterministic pack with original palette/alpha, source credits and explicit
mapping/fallback for small and Atomic styles. All ten exact confirmations and
three window sizes reviewed in OpenGL and Software; transparent compositing,
integer glyph scaling, wrapping, expiry and post-resize gameplay pass. Vanilla
retains original presentation. 60 Python tests and five native suites pass;
missing/corrupt assets fall back. All private sessions closed; player files and
original archives unchanged. Current binary and detailed limits are recorded in
[implementation notes](documentation/40-feedback-implementation.md).

### 2026-09-27 — User playtest acceptance and next planning pass

D07C accepted for tested aiming/RPG behavior. D08H bed/couch entry and smooth
walk/run departure explicitly accepted; hidden pickup and switch/NPC human checks
remain unreported. Added D08G2, D08I, D08J, D08K and D08A1 for the next focused pass;
D19B is deferred with modern menus. Read-only EDuke32/local source investigation
confirms bracket cycling exists but lacks selection feedback. No runtime edits,
builds, gameplay input or settings/card writes this turn. See
[feedback, source evidence, acceptance and continuation prompt](documentation/41-playtest-follow-up-plan.md).

### 2026-09-27 — Selected traversal and inventory feedback delivered

D08G2, D08I, D08J and D08A1 move to **Needs playtest**. Silent centered cheat results
and a temporary inventory strip pass both-renderer/resize/input checks. Inventory
shares original menu selection and charge percentages; cycling never activates.
Run-start misses were traced to the first-stride gate and replayed successfully in
two 18-case sweeps. Separate small-edge grace is bounded to six input frames and
original clearance. Armed ladder failures were reproduced before fixes: E-owned
stow needed its final blend, and weapon-switch intent needed stable-state gating.
Pistol/shotgun first/second ladder, switching, redraw and ammo checks now pass.
Accepted bed/couch entry and departures were replayed, including actual airborne
release. Original contact, animation events and collision remain authoritative.

D08K moves to **Blocked** after inspection of original animation/root/skeleton
commands and collision paths: existing movement is full-body rolling, with standing
movement clearance and no validated low-gait override pipeline. No misleading slow
roll or sliding idle was substituted. See [audit](documentation/43-crouch-walk-audit.md).
D19B remains deferred with modern menus.

Normal regeneration/build and ordered patch stack, six native suites, 60 Python
tests (2 conditional skips) and the isolated 14-checkpoint Vanilla route pass.
All private games closed; all 11 player configuration/card/state hashes unchanged.
Original assets and unrelated local work preserved. See
[implementation, evidence and limits](documentation/44-traversal-inventory-implementation.md)
and [resumable handoff](documentation/42-traversal-feedback-progress.md).

### 2026-09-27 — Speech-to-text playtest accepted; next pass checkpointed

D08G2 and bounded D08J are Done by explicit user acceptance. D08I remains Needs
playtest for the close-bed Shift-preheld W+Space vertical-jump case; broader jump
responsiveness is praised. D08A1 design accepted, activation follow-up selected:
user explicitly requests contextual Enter use without pause, retaining ordinary
Enter pause elsewhere, U and custom bindings. D10A turning and D18A audible crackle
investigations added as Todo; neither cause nor regression is established.
Next order: D10A, D18A, D08I, D08A1. D08K blocked, D19B deferred. Documentation-only
checkpoint: no runtime changes, builds or game interaction. See [precise reports,
acceptance and restart state](documentation/45-playtest-turning-audio-follow-up.md).


## 2026-09-27 — Selected D10A / D18A / D08I / D08A1 follow-up

Worked in the selected order with private Xvfb displays/cards/preferences. Full
evidence and resumable checkpoints: [46](documentation/46-turning-audio-inventory-progress.md).

- D10A: 64 sweep comparisons (16 street, 24 mixed club/street, 16 focused
  interior and 8 interior while firing) preserved requested yaw, including Shift,
  bump states and held precision input.
  No supported turning repair established; physical mouse/compositor feel remains
  Needs playtest. Accepted aiming/collision paths unchanged.
- D18A: real PulseAudio monitor/runtime PCM captured. Club playback showed output
  starvation, worse during gunfire; full-scale output clipping was not observed.
  Earlier-stage clipping/source artifacts are not excluded. Apartment no-poll
  segments had no starvation; matched comparisons are recorded in note46. No
  speculative buffer/mixer/speed change. Investigation remains open.
- D08I: 19 valid close-bed comparisons retained the original rising-clearance
  handoff and landed forward. One non-contact attempt is excluded. No lost
  direction reproduced, and no traversal change made; Needs playtest with the
  reported exact hardware/geometry/timing. Six-frame edge grace unchanged.
- D08A1: reproduced visible-strip Enter opening pause. Implemented contextual
  Enter through existing item_use, retaining ownership until key release so
  timeout/repeat/focus cannot turn a held activation into pause. U/custom bindings
  and ordinary Enter elsewhere preserved; use hint displays the actual binding.
  Full GL/default and Software/O-P-L checks and native guards pass. Needs playtest.

Preserved accepted aiming, furniture, ladder transfers and silent cheats. D08K
remains Blocked; D19B remains deferred. No player settings/card, asset or unrelated
source edits. No commits. Release/test identities and remaining coverage are in46.

## 2026-09-27 — Playtest acceptance; D08A1 strip; D08J aim-held stow

Player accepted D08I bed jumps and D10A turning feel; Enter inventory use confirmed.
D18A remains In progress (no crackle heard this session; club starvation unrepaired).
Implemented silent empty inventory and tiles-only strip (D08A1 Needs playtest).
Reproduced armed second-ladder miss when right-aim was held across takeoff: flight
lease was skipped so E stow never finished; fixed by owning flight_valid while aim
is held without redirecting aimed jump arcs. Private aim-before-jump attach recovered.
Added backlog D08L / D11A / D18B. D08K blocked; D19B deferred. Evidence:
[documentation/47-playtest-iteration.md](documentation/47-playtest-iteration.md).
Binary `57380caedddaf28621378dcc241886a234bff85dc1dd97882f6c6d50e55365cf`. No commits.

## 2026-09-28 — D08J armed stow reliability; D08A1/D18A closed; F unbound

Player accepted inventory strip and closed audio crackle (reopen if it returns).
Armed ladder grabs still felt harder than holstered. E-owned stow now 4x on
ground and in flight; jump+E refreshes lease; shorter airborne holster pulse.
8x boost rejected after it skipped upper events. Private early-E and nominal
armed attaches passed. Default DisplayPerf/F binding removed; D26 console Todo.
Evidence: documentation/48-armed-ladder-reliability.md.
Binary 42eeaf3dc31a7f7a94ae9201e0c47f32cb23c1680edcfca994acb5cb20eccbab. No commits.

## 2026-09-28 — D08J Done; plan D27/D28/D08A2/D26

Player accepted armed second-ladder grabs. Documentation-only plan in
documentation/49-controls-hud-plan.md. Added D27 (RUN MODE + Shift clunk), D28
(Scroll Lock holster + WEAPON LOWERED/RAISED), D08A2 (EDuke32 bottom-left icon;
Auto not Otto). D26 remains FPS via backtick console; Scroll Lock is holster only.
No runtime/build this checkpoint.

## 2026-09-28 — Queue D27 → D28 → D08A2 → D26 delivered

Implemented the planned controls/HUD queue. D27: Caps Lock RUN MODE quotes;
Modernized gait soft-switch removes Shift-run restart clunk. D28: default holster
Scroll Lock + WEAPON LOWERED/RAISED; profile v8 migrates C; captured C no longer
hard-holsters. D08A2: bottom-left active gadget + green %-tiles 3010–3021 (Auto for
biomask). D26: backtick console with `fps`/`help`; F unbound. Native input/controls/
inventory tests and profile unit tests pass. All four **Needs playtest**. Evidence:
documentation/50-controls-hud-delivery.md. Binary
57c2b097dd98e28473fc644a20d9d3a2730deb32ecbca0db1d0465e8564646ea. No commits.

## 2026-09-28 — Hotfix stiff gait + inventory/%/kerning

User reported frozen stiff posture on walk→run (D27 soft-switch). Restored frame-0
gait restart. Inventory inset into 4:3 frame; % uses tile 3076 green-swapped inside
the icon; centered message advance tightened by 1. Clunk silence remains open.
Binary e3e517beb5606ac35e05fb69236cbde357f639b65a8568e2e9f518f9f5eaa964. No commits.

## 2026-09-28 — Inventory: strip + icons; drop bottom-left; M quote

User: restore switcher, remove bottom-left host gadget, M = medkit % message only.
Strip centered again with research/inv icons (2460/2461/2467/2468). D08A2 scope
revised. Binary 6d3cdd09073b895427d684459f1b6851796c2accc348f0d8ba4051f6c82f8dd9.
No commits.

## 2026-09-28 — Inventory strip polish + EDuke crosshair (session complete)

Player-tuned frame locked at 50×60 / frame_dy=-6; green palette-22 digits/%;
Enter=use / Escape=pause; direct keys skip switcher; CROSSHAIR tile 2523 with
alpha blend. D08A2 revised scope marked Done. Findings:
documentation/51-session-inventory-crosshair.md. Binary
f09b657e31ded01e987d4b39695d469c452bdda621f6f6aa68dd01ebf8edec97. No commits.

## 2026-09-28 — Late polish lock-in (session complete)

Quake drop-down console + Messages smallfont (quotes and console). Owned copy at
recomp/assets/fonts/Messages (no research/ build links for that art). I crosshair
toggle fixed (was outside weapon_previous edge loop); Right Shift remains Select
inventory. Findings: documentation/52-session-late-polish.md. Binary
2afe9956c6c06120cf840db2263ce7619e32827dd158c26dba14c25ee3362c78. No commits.

## 2026-09-28 — Console scrollback, swim bridge, inventory quotes

D26: console echo/scrollback; help multi-line; fps persistent top-left debug block;
no toast flashes for command feedback. Crosshair halved. D27: plant-aligned gait
switch (no +0x68 restart). D28 Done (user accepted). D08M interim underwater
WASD→D-pad. Findings: documentation/53-session-console-swim.md.
Binary f0c5ad2ab9a52e6d26353630916f3ba5272c7c964b3907f77230b8475da5c890. No commits.

## 2026-09-28 — Console/crosshair Done; Shift gait revert; clunk deferred

D26 Done (user signed off console). Half-size crosshair + I toggle accepted.
D27: restored immediate frame-0 walk↔run (plant-aligned delay rejected — lag +
clunk remained); Caps Lock quotes Done; Shift-run SFX silence deferred low-priority.
Bio Mask/scuba left for later user review.
Binary 076945b35e06edc1d7e978ac8b7876d856ebe74a3cf4855e5c511b217fc549a8. No commits.

## 2026-09-28 — D08M camera-relative swim redesign

Modern swim adapter (`swim.inc`): look-yaw W via face+D-pad bridge, underwater
look-pitch vertical, Space rise / Ctrl descend, surface horizontal clamp,
W+Space mantle (anim 96, weapon-agnostic, generous probes). Camera orbit kept in
water. ttk-controls-test + ttk-input-test PASS. Needs water-map playtest.
Findings: documentation/54-swim-redesign.md.
Binary 6edc846d4f7bede4d4b452a39a20d18ffa8a3dea62155dee33c0e1fc961cadc6. No commits.

## 2026-09-28 — D08M wade jump + deep swim root-cause fix

Wade: short Space → standing prep 96 (suppress Cross→176 dip); no host VY overlay.
Deep swim: D-pad bridge no longer blocked by camera lease; host look-relative
velocity; Space/Ctrl = VY only (no body flip); lip mantle tightened.
ttk-controls-test + ttk-input-test PASS. Needs subway/sewer playtest.
Findings: documentation/54-swim-redesign.md.
Binary 5bd15215ba61c4b7b3b9ffc4f0044853093ce4d69d12a0c2d091f2eba203f04a. No commits.

## 2026-09-28 — D08M swim locomotion redesign

Stopped host world-space root/ballistic fight with tank `8005201c`. Free swim:
face_view + WASD→D-pad horizontal; host +0x1f8 vertical only. Fixed deep/wade
classification (WASD bridge). Wade jump uses land-like anim 96 + 8003e2d0.
ttk-controls-test + ttk-input-test PASS. Needs subway/sewer playtest.
Findings: documentation/54-swim-redesign.md.
Binary 4304407bf979aaf9f03af20c32bfaeca2b58c9b399f6c3594c41604a6b14d075. No commits.

## 2026-09-28 — D08M swim playtest iteration (strafe/vertical/mantle)

Fixed Forward-inject killing A/D/S in free swim; Space/Ctrl via host Y step;
land-scale wade/mantle impulse; friendlier Space lip exit; removed SCUBA toast;
added backlog D08N (Duke3D scuba + HRP). ttk-controls/input tests PASS.
Findings: documentation/54-swim-redesign.md.
Binary f8ec18f8f7c10abb9be81c145c55dbe83bdafae7f4ee83209d80b76fd0f6a9db. No commits.

## 2026-09-28 — D08M swim playtest iteration 2 (class/strafe/jump)

Deep water misclassified as wade → Forward inject + dead vertical. Fixed free-swim
gate; airborne 98 + reasserted −11000 for wade/mantle; softer Space lip exit.
ttk-controls/input tests PASS. Needs sewer/shallow retest.
Findings: documentation/54-swim-redesign.md.
Binary a93d96a3b081525ab23c60cae4435b8490b72a55a065006d1496ac12cfc9d6a1. No commits.

## 2026-09-28 — D08M swim playtest iteration 3 (no leap-loop)

Reverted anim-98/−11000 exit hop that leap-looped deep water. Wade = land −9000;
Space alone rises; mantle = Space+forward+ledge. ttk-controls/input PASS.
Findings: documentation/54-swim-redesign.md.
Binary 758b90753c165855f2096535f9c3f43c90c9093ce37fc34f43a5adff41298119. No commits.

## 2026-09-28 — D08M swim research rewrite (Square / 8003e2d0)

Deep review: Space→Square is original swim thrust; host vertical fought it
(surface hop). Wade `8003e2d0(96)` invalid (index anim−97). Free swim keeps
Square; wade/mantle use 97|98. ttk-controls/input PASS. Needs playtest.
Findings: documentation/54-swim-redesign.md.
Binary 561a9357fcaa585fb083fe78533c9f4c3d977d9905ec0fe7b1ac714b822671e7. No commits.

## 2026-09-28 — D08M upright free swim (no Square flip)

Shallow jump accepted. Free swim suppresses Square (176 upside-down); host
Space/Ctrl vertical; ledge exit W+Space; wade directed XZ boost + water holdoff.
ttk-controls/input PASS. Needs deep-water + subway ledge retest.
Findings: documentation/54-swim-redesign.md.
Binary 359ad05710bd7b8542f76f4696ad2d57c2ad170b5d12de0d2b55f434a03f8d09. No commits.

## 2026-09-28 — D08M free-swim rewrite (kill hop loop)

False mantle: headroom counted as ledge + held Space+W → airborne 98 hops.
Pure Y vertical; mantle needs fresh press + standable ledge; Ctrl dives from
surface. Wade jump preserved. ttk-controls/input PASS. Needs deep-water retest.
Findings: documentation/54-swim-redesign.md.
Binary 4cdb813c6d0e50eead6aadbfb316d9379318e314bfee31a9faefcf9f76f61201. No commits.

## 2026-09-28 — D08M host-owned free swim + close ledge

D-pad tank swim dropped (no strafe / sliding idle). Host WASD+Space/Ctrl.
Shallow Space-facing-ledge hop. ttk-controls/input PASS. Needs retest.
Findings: documentation/54-swim-redesign.md.
Binary 13ec110b1af63f84b98357646bfb9713462c02d96cd158d967d061ec3de41def. No commits.

## 2026-09-28 — D08M water-state recovery (abort host position swim)

Host XZ/Y + forced water flag + clearing fall VY froze entry mid-air and left
oxygen/swim-anim above water. Restored original water ownership + D-pad/Square.
Wade ledge jump preserved. ttk-controls/input PASS. Needs deep-water retest.
Findings: documentation/54-swim-redesign.md.
Binary fa4f9ca8f2c85a089aeff6cc2653a4aa3723f27042ff24c7096e76bfe4937759. No commits.

## 2026-09-28 — D08M research artifact + root-bridge swim

Wrote documentation/55-swim-controls-research.md. Free swim: Square suppressed;
soft Y ascend/descend; D-pad+stick+root bridge; ledge anti-bounce. No world XYZ
or invented water flag. ttk-controls/input PASS. Needs subway/sewer playtest.
Findings: documentation/55-swim-controls-research.md, 54-swim-redesign.md.
Binary 47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc. No commits.

## 2026-09-28 — D08M closed / D08O next (docs only)

User playtest: ledge Done; Space↑ W/S E-mantle OK; A/D and Ctrl↓ fail; simplify
exit to mantle-only. Stood up 55/54 solidly; D08M → Done (foundation); carved
D08O Todo as NEXT PICKUP (strafe, Ctrl dive, mantle-only exit). No swim.inc
changes. Manual/status/handoff/matrix synced. No commits.


## 2026-09-28 — D08O implemented (Needs playtest)

Root-caused deep free-swim gaps on 8005201c: A/D were D-pad Left/Right (turn
71/70) and stick-X (turn); strafe is the layout's L2/R2 words (800d1a68/1b80 →
anims 88–93). Free swim now injects layout-resolved strafe bits and no longer
writes stick X. Ctrl dive gated on floor margin (+0x1c8−0x1e0) instead of
"already >0x100 under surface". Free-swim Space ledge exit removed (E/mantle
only); wade jump 8003e2d0(97|98) untouched. ttk-controls/input PASS; shallow
subway regression clean (W/S/A/D, Space 98, Ctrl 105, no turn). Deep water
unreachable from the savestate → Needs playtest. Binary
d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e. No commits.

## 2026-09-28 — D08O.2/.3: native dive + wade ledge assist (Needs playtest)

Playtest: A/D strafe, Space↑ and mantle good; Ctrl still dead; subway shallow →
platform jump unreliable. (1) Ctrl: the original's native swim states (800455bc,
+0x22c 4/5) pin body Y to the surface (80044c18), so soft-Y can never dive; Ctrl
now calls the original dive 80045564 (state 5 / anim 133) and underwater the
host steers body yaw/pitch camera-relatively from WASD/Space/Ctrl and injects
Square for the original thrust; auto-surface unchanged. Verified in deep zone 35
by teleport (dive, 4-way thrust, look-up rise, Ctrl descend, Space surface).
(2) Ledge: measured 98 rise ~560 vs ledge 635; wall-touch cancel, 300–600 bump
107, only 700–1250 landed; probe reach ~350. New swim_ledge_ahead (standing probe
from 0/320/640/960 along view, XZ restored) + two-phase 97 → 98 switch when the
foot clears the ledge top, 97 VY kept via delayed_vertical, XZ speed scaled to
distance. 24/24 lab jumps landed (wall, 350–1300, run/stand, Space/W/A, ±53°).
Plain wall + W+Space → vertical hop instead of splash cancel. ttk-controls/input
PASS; shallow + deep regressions clean. Binary
71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5. No commits.

## 2026-09-28 — D08O.4: ledge jump back to the plain 98 arc (accepted → D08O Done)

Playtest accepted the swimming (Ctrl dive, underwater WASD/Space/Ctrl) and the
Bio Mask icon, but the 97 → 98 ledge hop looked far too high; user wants the
realistic simple jump of the previous cut, fixed if possible. Reworked: Space at
a ledge launches directed 98 as before, with VY −9000 via delayed_vertical at
the 8003ebf4 hook (98 integrates ≈VY/96 per tick → rise ≈840 vs 560), XZ owned
per tick by swim_ledge_ballistic: held at 0 until the foot clears the lip when
within one probe step (0/320) of the wall (98's first stride into the face was
the 107 bump / frame-one cancel), then 3400/4000; from further back 1900 +
3.5/unit (max 6400). Probe scan extended to 1280. Fixed a stale swim_ledge_pending
surviving a landing (now drops on anim ≠ 98 / state ≠ 9). Tuning trail: −5600
apex 90 short (0/9); −9000 + 2.7/unit 8/9 (oblique too far); +1280 scan 8/9
(1280 bucket under-speed); 3.5/unit 13/15 (near-oblique first-stride bump);
near-bucket hold 15/15. ttk-controls/input/inventory PASS; shallow (W/S/A/D,
Space 98, Ctrl 105) and deep (dive, thrust, rise/descend, auto-surface, surface
strafe) regressions unchanged. Binary
78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb. No commits.

## 2026-09-28 — D08O closed (playtest 3 accepted)

User: "everything here is confirmed working" — swimming (strafe, dive,
underwater controls, mantle exit), the round-3 ledge jump and the
`biomask-small.png` item-switcher icon. D08O → **Done**; docs 00/09/54/55 and
GAME_MANUAL updated to accepted. No code change; binary
78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb. No commits.

## 2026-09-28 — Bio Mask item-switcher icon (asset only)

Replaced the Duke3D scuba tile (2468) in slot 2 of `recomp/assets/ttk-inv-icons.pack`
with the user's own Bio Mask art `research/inv/biomask3.png` (17×14, cut from
`research/inv/biomask-asset/biomask1.png`); entry tile id 0 = custom art.
Provenance JSON updated. Pack format/loader unchanged; ttk-inventory-test PASS;
offline strip render shows the mask in the Bio Mask cell. No binary change
(runtime asset copied by the `ttk-inv-icon-assets` target). No commits.

Update (same day): slot 2 now carries the user's revised art
`research/inv/biomask-small.png` (16×13, tile id 0); provenance JSON updated;
ttk-inventory-test PASS; no binary change. Accepted by the user the same day. No commits.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User directed that a scuba device must not be added. D08N moved from Todo to
**Cancelled**. No code or asset change: there was no scuba prototype. TTK
underwater air stays original and automatic; Bio Mask stays the gas mask.
Player manual, status/handoff, swim note 55, and the D16A HRP note now say
scuba is not a future item. Historical work-log line that added D08N remains
as history. Binary unchanged
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`. No commits.

## 2026-09-28 — Playtest jobs accepted; D08P turret control recovery

User accepted all remaining Needs-playtest jobs from ongoing play: D07A, D07B,
D08A, D08H, and D10 distance. D08B/D10 broader work stays In progress. D08N
scuba stays cancelled.

## 2026-09-28 — D08P turret wade: depth gate, strafe fallback, Escape recapture

Playtest of the first tank-WASD fallback: mouse dead, A/D turned, Escape
dropped Modernized, crosshair gone, water felt “too deep”. Isolated load of
host savestate slot 1 (player cards not written): depth `0x100`, anim 63,
normal camera, `+0x224` without `0x20000000`. Host free-swim used `0x200` and
treated anim 70 (D-pad turn) as underwater. Free swim now matches original
`≥0x281` / states 4–5; A/D fallback is L2/R2 strafe; Escape recapture from
`0x8005a210`; reticle follows camera-only locomotion. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`. Note:
documentation/56-scripted-camera-controls.md. No commits.

## 2026-09-28 — D08P F7 slot 2: recapture after savestate load

Playtest of iteration 2 from F7 UI slot 2 (turret around the corner): modern
controls gone entirely. Isolated dump of `state_800AB6FC_slot01.pst`: same
wade as before, apartment words already patched. F10 had cleared
`initial_capture`; F7 released capture and did not set it again; the F7 menu
could consume `capture_offer` while `allow_capture` was false. F7 now
requests recapture like Escape even if the mouse is already free; offers
are kept until capture is allowed; identity remaps the complete patched
LEVEL00 pair. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs
playtest. Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`. Note:
documentation/56-scripted-camera-controls.md. No commits.

## 2026-09-28 — D08P identity reader stall

Playtest of the slot-2 recapture binary: gameplay and audio ran slow.
Apartment pair classification ran on every guarded identity word, many
times per frame. The check is now only on `0x800cc57c` / `0x800cc580`.
F7 recapture and slot-2 overlay identity are unchanged. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`. No
commits.

## 2026-09-28 — D08P waist-deep water as land locomotion

Playtest after the identity-stall fix: speed was normal, but the turret
wade crawled, mouse look died in the water, and after leaving only walk
worked (no Shift). Isolated slot 2 already uses land run anim 78 on
forward; Modernized failed `state()` on tank turns 70/71 and injected
Walk. Waist-deep / mid water now keeps the land camera lease, faces the
view, converts 70/71 to the run gait, and does not inject Walk. True deep
swim unchanged. ttk-input-test, ttk-aim-test, ttk-controls-test PASS.
Needs playtest. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`. No
commits.

## 2026-09-29 — D08P accepted

User playtested F7 slot 3 (ledge) with binary `79a3fd5b…`: "it really
works. perfectly." D08P → **Done**. Documentation committed and pushed.

## 2026-09-29 — D08P overlay scratch identity; mid-depth wade handler owned

User: the whole ledge area (F7 UI slot 3) loses run, mouse and jump for
good; depth is irrelevant. Their session log: `[TTK lease] inactive
(identity)` from the ledge on. Live per-guard diff on the isolated slot 3
copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's trailing
scratch vector, the output of the zone script's `0x8007177c` hit test
(overlay `0x800cb1a4`). Overlay guard trimmed to the 9652-byte code/table
body (`ttk_state_probe.py` too); controls test proves the vector write keeps
the lease and the last table word is still guarded. Same water is depth 512:
the resident dispatcher swaps every gait for clips 80/81 (Vanilla measured
8–14 units/frame), handler `0x800539f8`; the track advance writes the root
after the player-update hook, so the handler entry is hooked (added to
`mod_function_entry_funcs`, regenerated: one generated line; `build.py`
stops at the pre-existing runtime patch-stack drift, so generation ran
directly with the built recompiler). Root retargeted to camera-relative WASD
at the land run band; guards `0x800486b0`/260, `0x800539f8`/452,
`0x800788e0`/200; 80/81 join the wade-jump set. Live: 45–58 units/frame,
steering, strafe, jump 98, 0 refusals, turn/run/jump after the zone; slot 2
clean; wall stop while wading is the original handler (turn resumes).
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`. No
commits.

## 2026-09-29 — D08P held keys across recapture; fresh capture offers

User: "still exactly the same issues" on iteration 4. Live Xvfb + xdotool
reproduction (isolated slot 2 copy; software and OpenGL; debug load and the
real F7 menu; the user's profile) shows the wade at dry-ground run speed
with mouse look, so the wade path is sound; slow headings beside the spawn
are the original wall slide. Reproduced: a Shift/W physically held through
F7 load, Escape/resume or F10 was wiped by `clear()` at capture → walk gait
with Walk pad (10.7 units/frame) until Shift was re-pressed. Also found:
Escape's release offered recapture in the same update, so the original
pause menu was captured (Enter/X swallowed). Capture resyncs bound
keys/mouse buttons from SDL device state; offers expire after 8 frames;
sustained tank fallback shows `ORIGINAL MOVEMENT (reason)` /
`MODERN MOVEMENT RESUMED`; `run.py` mirrors stderr into
`recomp/build-local/logs/session-*.log` (`--no-session-log` opts out).
Verified live after the change (Shift through F10: 76 at 50.6; Shift+W
through Escape/resume: paused uncaptured, 78 at 53.9 on resume; mouse
after resume). ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs
playtest. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`. No
commits.

## 2026-09-29 — D08Q modern jetpack flight controls (Needs playtest)

User: in the jetpack "i cannot control it at all aside from spacebar to
ascend. wasd are blocked, modern controls fail here completely". New job
D08Q. Baseline probe on `79a3fd5b…` confirmed: original mode 10 (anims
163–170) had no lease, so no mouse; WASD only via tank fallback relative to
an unturnable body heading; idle gravity landed Duke in ~1–2 s; Ctrl inert.
Research (read-only): entry `8004aaf8` (Square + `358&3==3` + fuel), handler
`8004ade0` (body-space thrust, Square lift, Up/Down/L2/R2 thrust, hover
lock `0x08000000` with base `+0x860`, bit 31 = no gravity while any input,
`8003ea58` integration, `8004ac08` root apply with the floor probe), cut-out
→ anim 108 + `8003ff6c`. New `recomp/src/ttk/jetpack.inc` (host layer from
the `8005a210` hook): camera-only lease for mode 10 and the 108 fall after
cut-out, `face_view`, idle → hover lock at current height, Ctrl → lock off +
`+0x1f8 = 2800` (~48 units/update, lands via the original probe), WASD →
lock off + `+0x1e8 = 18` trim (level flight), `input_pad()` W/S → Up/Down,
A/D → layout strafe pads, `select_weapon` lets J toggle in flight. Tried and
rejected: driving the hover base `+0x860` (the pin only holds small offsets;
near a floor `8004ac08` pulls toward ~0x200 above it) and trim 29 (sank).
Guards for `8004aaf8`/`8004ac08`/`8004ade0`. Live Xvfb probes (isolated
cards, level 1 turret room + ledge): headings −33/154/−120/61° vs camera
−30°, level within ±25, speeds 25/29/24/14 units/frame, mouse `looks` 0→1,
Ctrl 489/30 frames → anim 105, J off → 108 fall with live mouse → ground
run at 45.7. ttk-input-test (new jetpack case), ttk-aim-test,
ttk-controls-test PASS (controls test needs an original overlay fixture,
e.g. `analysis/pc-input/d08-camera-final/`; the d08p-turret one is
Modernized-patched). Binary
`481dd2ec1c8c33cda1a77542c2a6f06bad845778ee62b1ff7d87e1d4a7a15692`. Docs
57/00/09 and GAME_MANUAL updated. No commits.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Needs playtest)

User hit "that stuttering audio/slowness issue" before testing D08Q.
Isolated Xvfb probe with the runtime's own telemetry (`audio_stats`,
`phase_profile`, `phase_hot`, `frame`): Modernized 47.5–49.3 fps, 600–780
underruns per 20–30 s, output fill 17–34 ms (target 180); Vanilla 60.0 fps,
0 underruns, fill 264; Sep‑27 baseline binary 57.5 fps. Counters added to
`identity()`: 46 calls/frame × 108 µs = 5.0 ms/frame = 24 % of wall time —
every call re-read all 20,305 guarded words through `psx_mod_read_word`.
Fix in `code_identity.h` (memcmp on `g_psx_ram` + exact fallback),
`modern_controls.cpp` / `weapon_aim.cpp` (`IdentityMemo` per host frame ×
`g_dirty_ram_code_gen`), `pc_input` (`input_host_frame()`). After: 59.94
fps, 0 underruns, fill 267 ms, 1.00 check/frame at 28 µs (0.17 %); jetpack
flight 60.1 fps; jetpack behaviour probe unchanged. Tests model stores and
code pokes as generation bumps; new memo case. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. New job D23A (Needs playtest). Note
58. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.
No commits.

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

User: stutter "only on the fmv at the beginning" after D23A; "make the
playback more robust ... if we can do it better, we should". Isolated probe
(started in the Cursor session, finished here): intro at ~52 fps with
continuous underruns; `overlay_loader_status` showed `OVERLAY CACHE HASH
MISMATCH` - the runtime read `gc15256f69` but the only shards were older
namespaces. The config hash includes `mod_function_entry` hooks, so the 28 Sep
hook additions stranded the 27 Sep shard. Rebuilt shard: 59.92 fps, 0
underruns, fill 267 ms, interpreter share 30 % -> 4 %; user confirmed smooth.
Hardening: `build_movie_overlay.py --if-ready --quiet` (skip when not
prepared, one-line result, rewrite capture.json only on change);
`ttk-movie-shard` ALL target after `psx-runtime`, added to the `local-dev`
build preset; `run.py` `ensure_movie_shard()` before launch (warns, still
launches); `fmv_poll.c` `note_movie_shard()` logs the decoder state on
change with `overlay_loader_last_msg()`. Verified headless with scratch cards:
current -> "active (19 functions)"; shard moved aside + direct launch ->
WARNING with the mismatch text; `run.py` -> "rebuilt for the current config"
-> active. Found and fixed: four `test_player_profiles.py` launch tests
patched `subprocess.call` while `run.py` now launches via `Popen`, so the
suite started the real game on `saves/local-play/` (cards had 0 occupied
blocks; no savestate touched). They now pass `--no-session-log` and stub
the shard check. 64 local Python tests OK. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits.

## 2026-09-29 - D08Q accepted (Done)

User playtest: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." D08Q moves to Done. Remaining notes in
57-jetpack-controls.md (abrupt stop on release, settle after Ctrl near a
floor, fuel drain while hovering, unprobed ceilings/water/damage) stay as
possible polish, not open acceptance items. D23A stays Needs playtest until
the user comments on in-game stutter.

## 2026-09-29 - D23A accepted (Done)

User playtest: "the in game stutter fix works fine". D23A moves to Done. D18A's historical club starvation
was not re-measured; if it recurs, check `identity_checks` and
`audio_stats.out.fill_ms` first (58-modernized-frame-budget.md).

## 2026-09-29 - Menu system backlogged; D08R narrowed to CLI

User direction: player-selectable options (starting with the D08R jetpack
scheme) should eventually live in a general Sonic 3 A.I.R.-style
customization menu built by hacking the original in-game menu, with a
responsiveness overhaul (D19B). Recorded as the D19 direction. D19 needs a
plan-mode design and a design artifact before implementation; not scheduled
yet. D08R scope revised to a persisted profile setting and `run.py` flag
only. Docs only; no code, build or launch.

## 2026-09-29 - D08B accepted (Done)

User direction: "d08b can be marked as completely done man, its fine. accept
it all", then "i have tested all of them!". D08B moves to Done on user
playtest. The user reports testing every previously recorded limit
(inconsistent contact jumps, residual bed run-off slowdown, early pipe-bomb
pickup from the unopened bed, outer-end/oblique couch entry, ladder ascent
stall with a pig cop on the landing) and accepting the results. No automated
replay or build was run for this change. D08L still covers inertial edge
run-off as a separate feature. Docs only; no code, build or launch.

## 2026-09-29 - D08R selectable jetpack scheme (Needs playtest)

Implementation: `player_profiles.py` schema 11 adds Modernized control
`jetpack` (`modern` | `classic`, v10 and older migrate to `modern` with a
backup); `run.py --jetpack` saves it and exports `DNTTK_JETPACK` (`modern` in
Vanilla); `describe()` prints the scheme. Runtime: `jetpack_classic()` gate;
Classic drops the `jet` camera lease clause, the host hover/descent/face_view
layer and the fall grace; `jetpack_classic_input_ready()` bridges W/S/A/D to
D-pad Up/Down/Left/Right in `input_pad()` (no tank fallback) and keeps the J
shortcut in flight. No terminal-menu entry (D19 owns in-game exposure).

Research: the original flight turns with D-pad Left/Right (live probe) even
though `8004ade0` never reads them; turning is not thrust, so gravity applies
unless Up/Down/Square is held. The original hover toggle table `800d1cf0`
resolves to physical L1 = Modernized walk (Shift).

Evidence (binary
`b588f926cf3cc300bdbf5873a3a0a4e7b78074c3fd0eccbe6acd060361e2da6d`,
isolated Xvfb, scratch copy of the D08Q cards, user's game closed): Classic W
along the body, A/D turn (yaw -421 / +391 in 30 frames), W identical to the
arrow-key D-pad Up, Shift hover holds (dy -28 / 60 frames), Ctrl inert, J cut-out
fall then ground lease, no tank fallback. Modern re-probed matching D08Q
numbers. 68 Python tests, `ttk-input-test` (new Classic pad case),
`ttk-controls-test`, `ttk-aim-test` PASS.

Limits: user playtest of Classic outstanding; fuel-out mid-flight, ceilings,
water, damage and controller not probed. The player's saved settings file
migrates to schema 11 (with a backup) on the next launch. No commits.

## 2026-09-29 - D08R revision 2: Classic keeps modern controls (Needs playtest)

User playtest of revision 1: "with jetpack classic, im having trouble turning
with the mouse ... the controls are slipping in and out of our non-modern
controls ... jetpack classic should basically be that, burst style jetpack with
space to boost ... but the modern controls, wasd, mouse look, should all be
retained". Classic now shares the Modern camera lease, `face_view`, WASD bridge,
J shortcut and cut-out fall grace; `jetpack_update()` skips only the host
vertical layer (hover on release, Ctrl descent, level trim). The D-pad
Left/Right bridge and the lease exclusion were removed. Isolated probe on
binary `1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`:
lease held on every flight sample, mouse turns Duke in the air, W and D follow
the camera, release sinks with no host hover, Ctrl adds nothing, J fall keeps
the mouse, no tank-fallback messages. 68 Python tests and the three native
suites PASS. Classic playtest outstanding. No commits.

## 2026-09-29 - D08R accepted (Done); D08Q1 opened

User playtest of revision 2: "it's absolutely rock solid." D08R moves to Done.
Same message requests faster Modern Ctrl descent at the underwater dive speed;
scoped as D08Q1 (In progress).

## 2026-09-29 - D08Q1 faster Modern Ctrl descent (Needs playtest)

`jetpack.inc` `k_jet_descend_velocity` 2800 -> 5500 (Modern only; Classic
returns before the host vertical layer). Target: underwater Ctrl dive,
~29 units/frame from the D08O deep lab log (`/tmp/swimlab/deep4.log`,
y -1438 -> -888 over 19 frames). Isolated probe on binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`: steady
Ctrl 29.5 units/frame (5100 gave 27.0), Ctrl+W ~26-33, landing to anim 63 with
health 10000 -> 10000; Ctrl 30 frames dy 955 (D08Q 459). Modern hover, W/S/A/D
headings, mouse and Space re-probed unchanged; Classic re-probed unchanged
(0 lease losses, no host descent). `ttk-input-test`, `ttk-controls-test`,
`ttk-aim-test` PASS. User feel check outstanding. No commits.

## 2026-09-29 - D08Q1 accepted (Done)

User playtest: "verified working!!!" D08Q1 moves to Done. Binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

## 2026-09-29 - D08S cancelled; D10 selected

User: scratch D08S ("i am so happy with our current jetpack controls now"),
may revisit. Nothing was implemented; the original scope is kept in the job text.
User selected D10. With no specific camera complaint, the user chose these
options: an optional shoulder offset (off by default), a recenter key (not
automatic), and a saved Alt+wheel distance.

## 2026-09-29 - D10 recenter, shoulder offset, saved distance (Needs playtest)

New rebindable actions `camera_recenter` (V) and `camera_shoulder` (H), handled
on press in `pc_input.cpp` and never queued as guest commands. `orbit_begin`
eases the orbit to Duke's heading and the rest pitch (the first seeded original
follow pitch). Any mouse motion cancels the swing. `orbit_constraint` moves the
requested eye along the view's right row by 0.22 x boom (192-640 units),
eased, before the original constraint solve. The look target, matrix and
collision are unchanged; view aim traces from the solved eye. Profile schema 12
adds controls `camera_distance`/`shoulder` and the two actions; migration never
takes a bound input. The runtime writes `<settings>.camera-state` 30 frames
after a change, and `run.py` merges it after exit and at the next launch. Flags:
`--camera-distance original|768..6144`, `--shoulder center|right|left`.

Evidence on binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`: 71 Python
tests; `ttk-input-test`, `ttk-controls-test` (new D10 case) and `ttk-aim-test`
PASS. Isolated Xvfb probes on the turret-room and ledge savestates: pitch
recenter to rest within about 40 frames, mouse cancel, right-shoulder lateral
637.5/640 (turret room), left pulled in to 544 by the original solve, and
screenshots with no geometry clipping. A right-shoulder shot fired with 0
rejections. Alt+wheel +576 saved; clean exit printed `Saved camera distance
3627, shoulder left.`; the relaunch restored radius 3627. The Vanilla
regression route exited 0. Finding: on the ground Duke's body already follows
the view (D07A/D07C), including while strafing and with original weapon aim.
V therefore mostly levels pitch; its yaw swing matters only where facing
differs. Needs playtest for feel (offset size, swing speed). The broader D10
doors/corners/tight rooms/vertical traversal acceptance remains open. No
commits.

## 2026-09-29 - D10 accepted (Done)

User after about five minutes of play on binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`: "it's done,
fully accepted. this is amazing work." D10 moves to Done on explicit user
acceptance. The evidence is that play session plus the automated and
isolated-probe results above (two level-1 rooms). No per-location campaign
playtest of doors, corners, tight rooms or vertical traversal was recorded.
D11 is now ready (D08, D10 Done). No commits.

## 2026-09-29 - D11 first-person prototype (Needs playtest)

Modernized with the independent camera gets an optional eye-level view. **P**
(new action `camera_view`, after `camera_shoulder` so it is an edge, not a
guest command) toggles it; profile schema 13 adds control `view` (third by
default) and the action; `--view first|third`; the runtime side file now
carries `view`, merged by `run.py` like distance and shoulder. Vanilla and the
`original` camera option never use it.

Implementation (`recomp/src/ttk/first_person.inc`, wired from
`modern_controls.cpp`; hooks `0x800348D8`, `0x8002A038`, `0x800B4D9C` added to
`game.local.toml`, regenerated): the eye is Duke's neck joint (joint 9 of the
19-joint model at `[player+0x3c]`) plus 96 up, with only the neck's sway about
the root smoothed. It is fed through the D06 orbit anchor into the unchanged
original camera update. The original 768 minimum distance is relaxed only
inside the constraint call and restored before the update ends (a captured
relaxed value is repaired). The follow easing rates go to 1 so the eye does
not trail. Duke's head joint record gets the original skip bit only between
his draw and the next object-list step. The render's per-frame `SetGeomScreen`
call gets H 256 instead of 386 (`camera[+0x42]` is never written): at 386 a
wall Duke faced went black because the renderer drops vertices nearer than
H/2. A Space-only standing jump keeps a camera-only lease while the eye view
is live (no motion change). Swimming and jetpack flight blend back to third
person; unleased states switch to the original camera as before.

Evidence on binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`: 74 Python
tests (new: v12->v13 migration without taking a custom P, optional side-file
view, `--view` env); `ttk-input-test` (P toggle), `ttk-controls-test` (new D11
case: anchor, boom relax/restore/repair, projection argument, head flag
window, lease loss, third person untouched) and `ttk-aim-test` PASS. Isolated
Xvfb route with fresh private cards (`recomp/analysis/d11-first-person/runs/
route-final`): all 14 checks pass - first-level spawn in first person, 7
view-aimed shots with actor hits and 0 rejections, club door waypoints,
doorway threshold sweep with street and club both rendered, entry hall and
main room sweeps, look up/down with no body in view, Space-only jump stays
active, crouch lowers the eye, P to third person and back with original
globals, no fallbacks. Earlier runs: eye within about 25 of the neck while
running; death falls back to the original camera. Frame budget in the club:
59.87 fps third person, 59.94 first person, 0 underruns. Vanilla regression
route `d11-vanilla` exit 0. Movie shard current (native decoder active).

Limits: steep-angle wall contact can still drop or tear the nearest wall
polygons (new job D11B); the original obstruction rays did not report the
touched wall; no hands or held weapon are drawn (D12); switching to an
unleased state is a hard cut to the original camera; field of view fixed at
about 64 degrees (FOV options are D14). Tested on the first map's street and
club interior and the turret-room corridor, not the campaign. Needs playtest
for feel (eye height, FOV, bob) and the user's own route. No commits.

## 2026-09-29 - D11 accepted (Done)

User after playing binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5` for a while:
"i have been playing it for a while, and im insanely happy with it. mark all as accepted. this is phenomenal, and it's like a dream come true." D11 moves to Done on explicit user acceptance, with the
automated route and test evidence above. The recorded limits stand: steep-angle
wall contact (D11B), no hands or held weapon (D12), fixed field of view (D14),
first-map coverage only. D11A and D12 are now ready.

## 2026-09-29 - D11A cancelled

User: "we can disregard D11A, the "p" button works so well for going to first
person mode." D11A moves to Cancelled. No code changed. D11B and D12 remain the
first-person follow-ups.

## 2026-09-29 - D11B / In progress (investigation only, no code changed)

The D11 note's cause was wrong: `0x8002ef90` (outcode bit `0x10` at
`0x8002f1e0`) is a horizontal grid surface renderer, not the walls. The GTE RTP
ring on the isolated club slot shows room geometry going through two
hand-written renderers:
- `0x80011020` world mesh renderer (vertices on a 1024 grid, types 0x60 GT3 and
  0x61 GT4, fog by DPCS, texture animation, light table). It drops a polygon if
  any vertex has SZ 0 (at or behind the eye). Vertices nearer than H/2
  saturate the GTE divide, so polygons get wrong screen coordinates (tears).
  If every vertex is under 0x2000 the polygon goes to a screen-space subdivision
  path (`0x80012960`), which does not fix the projection.
- `0x80010000` object mesh renderer (props); it flags vertices with SZ < H.
Callers `0x8003733c`, `0x80037c4c`, `0x80062db0` pass the render context
`0x800d67a8`, ordering table `0x800d27a0` (2048 buckets) and bitmap
`0x800d26a0`.

Planned fix (eye view only): hook `0x80011020`; for a mesh with unsafe
vertices, render only those polygons on the host with 3D frustum clipping and
subdivision (same colors, UVs, tpage/clut, OT rule `min(minSZ>>5,0x7ff)` and
packet allocation), and pass the original a copy of the mesh without them, so
all other polygons use the original code unchanged. Third person and Vanilla
stay on the original renderer.

## 2026-09-29 - D11B / Needs playtest

Implementation (`recomp/src/ttk/near_clip.cpp`, new hooks `0x80010000` and
`0x80011020` in `game.local.toml`, regenerated: one generated line each;
`first_person_view_live()` and `first_person_duke_drawing()` added to
`modern_controls`; debug JSON `fp.near`; developer switch `DNTTK_NEAR_CLIP=0`).
While the eye view is live, each world or object mesh with a polygon that has an
unsafe corner (GTE divide saturated, IR clamped, projected beyond +-1000) or a
corner nearer than 1536 units has those polygons drawn by the host. They are
clipped in view space (near 16, guard band +-480 x +-240), cut along
whole-texel lines until small, and emitted as GT3/G3 packets. Colors, fog, light
table, texture animation, command bits, packet arena and OT slot follow each
renderer. The original routine then draws a copy of the mesh without those
polygons, so everything else stays on the original path. Object polygons with
every corner nearer than H stay dropped as in the original; Duke's own model is
left alone (D12). Guards: SHA-256 over both routines.

Evidence on binary
`d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa` (isolated
Xvfb, private route cards slot 1, Duke placed with debug position writes on
those cards only; captures in `recomp/analysis/d11b-near-clip/`):
- Club red-panel side wall: -90..+90 sweep. With clipping off, looking along the
  wall (+15..+45) the near wall is black and head-on the frame bends. With
  clipping on the wall is continuous with straight, perspective-correct texture
  lines.
- Club entry corridor side wall: off shows black gaps at -30..-15 and
  +15..+30; on is continuous.
- Stage block at spawn: floor near the eye now draws (off: missing).
- 0 refusals, 0 copy overflows. Frame budget in the club: first person 59.96
  fps on, 60.04 off; third person 60.03; 0 underruns.
- Third person (P toggle, live): 0 near-clip calls, projection 386 and minimum
  distance 768 intact. Vanilla route `d11b-vanilla` exit 0, captures normal.
- `ttk-near-test` (new), `ttk-controls-test` (D07 LEVEL00 fixture; the D08P
  fixtures fail an overlay-code assertion at `0x800cc57c` unrelated to this
  change), `ttk-input-test`, `ttk-aim-test` PASS; Python 74 OK (2 skipped);
  movie shard current.

Limits: club interior only (side wall, stage, entry corridor), not the rest of
the first level or the campaign; affine mapping remains inside each small
piece; object polygons entirely within H stay dropped; Duke's body is not
clipped (D12); host pieces ignore the runtime's widescreen X squash (the
project is 4:3; D14). Needs the user's own play near walls. No commits.

## 2026-09-29 - D11B / Needs playtest (second pass: apartment artifacts)

User after playing the first pass: "it looks very good, but i think there is a
kind of popping or artifacting ... inside the hooker's apartment ... the closet
... the back wall of the closet, which pops through the wall somehow" (the
closet door itself was fixed), and asked for a switch to compare. The switch
already existed: `DNTTK_NEAR_CLIP=0` on the launch command (run.py passes the
environment through).

Reproduced on fresh private cards (route street -> alley ladder -> apartment
window; debug state in `recomp/analysis/d11b-near-clip/apartment-cards`). The
wardrobe is a prop (object renderer) flush against a world wall; the first
pass sorted each host piece by its nearest corner, so wall and wardrobe pieces
interleaved (sawtooth). Changes: pieces sort by average depth; host prop pieces
4 OT slots nearer (actors excluded, tracked from `0x800348d8` to
`0x8001ca4c`); props taken over to 3072 units so original prop polygons do not
lose to host wall pieces; new corners round outward to close T-junction
hairlines; tighter depth split (1.25 / 16 px). A per-frame packet budget was
added after heavier splitting overran the 139,744-byte render ring (screen
garbage in an experiment): 64 KB host packets per frame and never past 60% of
the ring; otherwise pieces stop splitting or the mesh stays original.
Developer switches documented in `documentation/62-d11b-near-clip.md`
(`DNTTK_NEAR_CLIP=0|world|object`, `DNTTK_NEAR_TINT=1`, bias, sort).

Evidence on binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`: apartment
closet sweep, 24-frame walk to the wardrobe and five-spot room captures clean
(unclipped: torn/see-through wardrobe, black ceiling wedges); club side wall and
corridor unchanged; host packets peak 42-46 KB/frame, 0 skipped, 0 fallbacks;
59.92 fps clipped / 59.94 unclipped / 59.94 third person, 0 underruns; Vanilla
route `d11b-vanilla-2` exit 0; native tests and Python 74 OK; movie shard
current. Remaining: painter's sorting (a prop up to 128 units behind a nearby
wall piece could draw over it; actors excluded); tested in the club and the
apartment only. Needs the user's replay of the apartment. No commits.

## 2026-09-29 - D11B / Needs playtest (third pass: conservative default)

User at 1080p: black grid lines on floors outdoors and in the subway, faint
lines on objects, diagonal artifacts on the wardrobe, floor texture popping,
none in third person; asked whether the earlier state was better and what the
options are. Reproduced by capturing the presented window at 1920x1080
(`screenshot_file` reads the software raster and missed it): dotted dark lines
along host piece edges. The outward corner rounding caused most of them; the
subdivision seams and the -4 prop bias caused the rest. The default is now a
conservative mode: take over only polygons the original gets wrong (unsafe
corners, beyond the GPU's 1023x511 primitive limit, or props entirely within
H, which made the wardrobe see-through up close), clip them, emit without
subdivision, sort each as one polygon like the original, floor rounding. The
subdividing mode stays available as `DNTTK_NEAR_MODE=full`; `DNTTK_NEAR_CLIP=0`
is off.

Evidence on binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`: 1080p
street without dark lines; apartment views, roam and the walk into the
wardrobe clean and solid where the unclipped build is torn or see-through;
club side wall and corridor gaps filled; 60.01 / 59.97 / 59.71 fps (on / off /
third person), 0 underruns, host packets at most about 1.2 KB per frame;
Vanilla `d11b-vanilla-3` exit 0; native tests, Python 74 OK; movie shard
current. Subway not tested here (no isolated subway state). Trade-off: close
surfaces keep the PS1 affine texture bend and whole-polygon sorting, as in
the original. No commits.

## 2026-09-29 - D11B / Needs playtest (user review; occluder fade in first person)

User after playing the conservative default: "the rendering that you've set for
default runs the very best it looks the very best"; peripheral glitching of the
Vanilla renderer in first person is gone; clip off (`DNTTK_NEAR_CLIP=0`) stays
as Vanilla rendering; full mode "is just no good" (black floor lines) - keep
for research only. Remaining: "occasionally the very smallest slight bit of
black line artefacting on the ground ... negligible", and the subway
card-reader door turns invisible when Duke walks up to it in first person.
The user's DuckStation screenshot (`research/vanilla.png`) shows the same door
translucent in the original game in third person.

Cause: the original occluder fade. The prop loop (`0x80031fa0..0x80032078`)
makes a prop semi-transparent and dims it by distance when it is nearer than
the fade distance (camera+0xa0, about 291) and its screen rectangle overlaps
Duke's (camera+0xa8), via `0x8002ee50` (rectangle overlap, single caller). In
the eye view Duke's rectangle covers the whole screen (-218,-16 466x579), so
any prop walked up to fades. Fix: new hook `0x8002EE50` (regenerated, one
generated line); in the eye view only, the call from `0x80032008` gets an empty
off-screen rectangle as a0, so the prop draws solid. Third person and Vanilla
keep the original fade. `DNTTK_FP_OCCLUDER_FADE=1` restores it; logged as a
D19 menu toggle at the user's request, together with the near-clip mode.

Evidence on binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73` (private copy
of the user's F7 slot 4 savestate in
`recomp/analysis/d11b-near-clip/subway-cards`, player files untouched):
walking into the street sign pole, original fade shows it ghostly translucent,
the new default draws it solid (41 tests intercepted, 0 in third person);
`recomp/analysis/d11b-near-clip/third-pass/pole.png`. 59.87 fps first person,
59.78 third, 0 underruns; Vanilla `d11b-vanilla-4` exit 0; native and Python
tests pass; movie shard current. The subway door itself was not reached in the
isolated run (it is past the station guards); needs the user's replay there.
The faint ground-line residue in conservative mode was not reproduced at 1080p
street captures. No commits.

## 2026-09-29 - Ad hoc: Jetpack inventory icon replaced

At the user's request the Modernized inventory strip's Jetpack icon (item 1,
was Duke3D tile 2467) is now the user's own `research/inv/jetpack-icon.png`
(16x12). `recomp/assets/ttk-inv-icons.pack` rebuilt with only that entry
changed (backup `recomp/analysis/d11b-near-clip/ttk-inv-icons.pack.before-jetpack`),
provenance JSON updated, redeployed beside the executable (no rebuild of code
needed). Verified in the inventory strip (`]`) on an isolated run. No commits.

## 2026-09-29 - D11B accepted (Done)

User after playing binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available").
D11B moves to Done: conservative near clipping is the default, props no longer
fade see-through in the eye view, `DNTTK_NEAR_CLIP=0` remains Vanilla-style
rendering, `DNTTK_NEAR_MODE=full` is kept for research only. Both switches are
logged as D19 menu toggles. Remaining notes: occasional faint ground-line
residue the user called negligible; first-map areas only, not the campaign.
The jetpack inventory sprite change is confirmed in game. D12 (hands and
weapon) is the next first-person job.

## 2026-09-29 - D12 first-person weapons (Needs playtest)

User asked to continue at the recommended point (D12) and, mid-session: "if we
can ACTUALLY use the real hands and weapons from this game, that will be the
most perfect way to go for it". Delivered with the game's own assets only.

Research: the actor draw `0x800348d8` draws the weapon (`0x80033e40`), muzzle
flash (`0x800341e4`) and held item (`0x80033f5c`) from the hand joint (model
byte `+0x33`, joint 7) after `0x800292a0`; the hand's near cull (depth < H)
skipped all of it in the eye view. Joint matrices are stored world-to-joint
(transposed by `0x80010d60` before camera `+0x20` times it). `duke_drawing`
turned out to be cleared by the draw's own callees before the joint loop, so
D12 bounds Duke's draw separately.

Implementation (`recomp/src/ttk/first_person.inc`, `near_clip.cpp`,
`modern_controls.cpp`; hooks `0x800292A0`, `0x80033E40`, `0x800341E4`,
`0x80033F5C` added to `game.local.toml` and regenerated with the recompiler
directly): the hand transform and its attached draws get a guest-allocated
matrix (Duke's joints never written); placement anchored in view space; a
ready pose from each weapon's firing pose (torso-relative, seeds for slots
4-11 surveyed in this build); flash kick; viewmodel packets appended to
ordering-table slot 0 when Duke's draw ends so walls and Duke's chest cannot
cover them; arm joints 2-6 culled at the eye; close-camera fade neutralized for
the viewmodel only; SHA-256 guards for `0x800348d8` and `0x800341e4`;
`ttk_input` JSON buffer 2048 -> 8192.

Evidence on binary
`1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`: 13
weapons (slots 1, 2, 4-14) captured at rest and firing, all drawn; pitch
sweep 0/+-35/+-60 stable for pistol and shotgun with the flash on the muzzle;
walk, run, crouch, jump, fast turns and wall contact keep the weapon in view
and on top; third person unchanged; 59.95 fps first person / 59.96 third,
0 underruns; Vanilla route `d12-vanilla` exit 0; native tests
(`ttk-controls-test` with a new D12 case, aim, near, scene, input) PASS;
Python 74 OK. Limits: instant holster/draw, right hand only, ready pose
replaces reload/pump motion, projectiles leave from the real hand, slot 8/9
large, first-map street only. Needs the user's playtest for placement, size
and feel. No commits.

## 2026-09-29 - D12 twin cannons framed like the Devastator (Needs playtest)

User: "This is an amazing start". Asked for weapon key 5 (slot 8, the twin
cannon launcher) to show "the two Canons visible at the bottom" like Duke 3D's
Devastator (reference images in `research/`), using the strip-club mirror to
see how Duke holds it. Delivered: slot 8 offset (-20, 240, -100), 42 degrees
muzzle-up, scale 1.2 via the original weapon draw's scale argument; the
cannons rise from the bottom corners and the joining block stays a strip at
the bottom edge (the model's middle shares the cannons' top plane, so it
cannot be framed out entirely). The viewmodel moved from ordering-table slot
0 to slot 1 so the HUD draws over it (the right cannon had covered the ammo
box). New developer overrides per slot for offset, tilt and scale. Club
mirror checked: the reflection keeps Duke's head, arms and real weapon pose.
Binary `b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`;
native tests and Python 74 OK. No commits.

## 2026-09-29 - D12 weapon framing tweaks (Needs playtest); D12A queued

User: "it's amazing!!" Pistol, Devastator (slot 8), flamethrower (slot 10),
special shotgun (slot 6), crossbow, rocket launcher, Python, dynamite and Holy
Hand Grenade accepted as they look. Requested tweaks delivered as per-weapon
display tilt/offset defaults (no pose or asset changes): shotgun (slot 5,
key 3) offset (40, 0, 0), tilt -10 pitch / -30 yaw, so the barrel and pump
show; gatling (slot 7, key 4) offset (30, -10, 330), tilt -10 / -25 / roll 10,
right of centre and pointing in toward the crosshair; throwing blades slot 1
tilt -30 / -20 / roll 40 and slot 2 tilt -40 so they read as blades instead of
a vertical line. Captured at rest and firing (flash on the gatling muzzle).
New backlog job D12A (kick stays in first person), queued next by the user.

## 2026-09-29 - D12 framing pass 2 (Needs playtest)

User accepted knife, axe, pistol, Devastator, pipe bomb/dynamite/Holy Hand
Grenade, flamethrower (slot 9, key 8), double barrel (slot 6, key 9) and
crossbow as they look; "this is all polish, everything is working". Retuned:
shotgun (slot 5, key 3) less sideways, offset (30, -15, 0), tilt 3 / -15;
gatling (slot 7, key 4) at the bottom edge pointing up toward the crosshair,
offset (30, 55, 290), tilt 8 / -22 / roll 10; energy weapon (slot 10, key 7)
and freezer (slot 11, key 0) scaled 1.4, raised 20, tilted -12 (freezer yaw
-10) so more barrel and tip show. Bringing them closer made them vanish (the
hand joint fell inside the near cull), so scale is used instead. Captured at
rest and firing (freezer effect at its tip). Binary
`29a31eb40642108e1c8b9b1f319e17e81fcb6b2139ed45a9d8ef2a728429146e`. Next job remains D12A.

## 2026-09-29 - D12 framing pass 3 (Needs playtest)

User: shotgun (key 3) and number 4 should have the grip lower and the muzzle
raised so they point at the crosshair; number 9 a slight version of the same;
5, 6, 7 and 8 fine. Pitch tilts: slot 5 -12 (yaw -15), slot 7 -8 (yaw -22,
roll 10), slot 6 -6. Negative pitch raises the muzzle (the code comment had
the sign backwards; corrected). Captured at rest and firing (flash at the
barrel tips). Binary
`f56bc9a570014a30a386efcc31e9f955872508f5c58e26d512b5325d1f8f7b3d`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 4: hide the floating hand (Needs playtest)

User: pass 3 "absolutely phenomenal"; asked whether positioning alone can
hide the gap where Duke's arm is missing (a floating hand), for example by
lowering the pistol toward the bottom of the screen as the gatling already
is. All weapons reviewed: the pistol (slot 4) and knife (slot 1) showed a cut
wrist; the others already run off the bottom edge. Pistol offset (-10, 65, 0),
tilt -10 (muzzle raised so it still points at the crosshair); knife offset
(-60, 60, 0). The glove now enters from the bottom edge; pistol flash stays
on the muzzle, stable looking down. Binary
`08af082dc6d5bb750120a7927ede9757d6233c5c1c4cbcb4544fade3c6bf5741`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 5 (Needs playtest)

User: numbers 3 and 4 a bit lower, tilted very slightly to the right, and
re-aimed at the crosshair. Shotgun (slot 5) offset (30, 10, 0), tilt -16 /
-15 / roll 6; gatling (slot 7) offset (30, 80, 290), tilt -12 / -22 / roll 16.
A yaw-only alternative (muzzle turned 4 degrees right instead of a lean) was
captured and left as an override. Captured at rest and firing. Binary
`cc33b1e015057ae21b84052d9e9bb3fa0c90935f4fabcb4e1f6a2c5fdb28f7c6`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 6 (Needs playtest)

User accepted the shotgun (number 3). Number 4 (gatling, slot 7) moved about
"two inches to the right" at 1080p (view x 30 -> 110) and re-aimed at the
crosshair (yaw -22 -> -26); a -31 yaw variant turned too far across.
Captured at rest and firing. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next job remains D12A.

## 2026-09-30 - D12 accepted (Done)

User: "finally, we can mark this as accepted!!" D12 moves to Done on explicit
user acceptance after six framing passes, with the automated evidence in the
earlier entries. Recorded limits stand: instant holster/draw, right hand only,
reload/pump motion replaced by the flash kick, projectiles leave from Duke's
real hand, first-map street and club coverage only. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next: D12A (kick stays in first person).

## 2026-09-30 - D12A first-person quick kick (Needs playtest)

User: in first person the kick's "duke-fu spin around and crazy moves" are
unnecessary; wants a Duke Nukem 3D style kick using his leg, "more accurate to
aim". Implemented (`recomp/src/ttk/kick.inc`, no new hook addresses, no
generated-C change):

- In the Modernized eye view, **Q** is a 0.45 s quick kick with any weapon,
  holstered, walking, running, crouched or jumping. Weapon stays drawn.
- The original boot attack (`0x800517a4`, random animation 112..115) is still
  reachable with Boot selected (Q, E at idle, held attack); in the eye view its
  request is converted at the lower-body initializer `0x800493a4` into the
  quick kick, and the eye view keeps its lease for that one update.
- The hit is the original sphere `0x800a979c` (radius 96, type 10, damage
  `[0x800d21fc] * 20`, x4 with steroids, attacker Duke) placed 340 along the
  view, on each player update of the extension (about 4), as the original does
  for the foot on every update of 112..115. The original kick sound
  (`0x8006b270` id `0x1000`) plays as the leg snaps out.
- Duke's own right-leg joints 14..17 are drawn in front of the eye from three
  poses recorded from the original straight front kick 115 (rest, chamber,
  extension), on top of walls, rising from the lower left with the boot just
  under the crosshair.

Evidence (isolated private instance, fresh private cards, god mode, pig cop
on the first street): pistol Q 3750 -> 2250 per kick, three kicks kill;
Boot + E and Boot + held attack convert (animation stays 63) and hit; running
and jumping kicks keep first person (0 fallbacks); third person unchanged
(armed Q inert, Boot + Q plays original 114/115); 59.89 / 59.93 fps without /
with repeated kicks, 0 underruns; Vanilla route `d12a-vanilla` exit 0;
native suites PASS with a new D12A case; Python tests OK. Limits: no
knockback (the original's reaction 84 did not occur with the quick kick's
hits at any tested height or reach), 1500 per quick kick versus up to a kill
for a point-blank original kick that lasts 2.5 times longer, right leg only,
first street and pig cops only. Needs the user's playtest for feel, framing
and balance. Binary
`93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`.

## 2026-09-30 - D12A follow-up: crosshair hits, Boot left-click while moving, thigh (Needs playtest)

User after the first playtest: "The kick looks amazing"; no knockback and the
lower damage per kick are fine; garbage bags (hit only when crouching) and the
pallet by the subway entrance should be kicked where the crosshair is;
left-click with key 1 should kick while moving; the missing thigh needs tuning
and options. Changes: the kick sphere goes to where the original query-only
crosshair trace (`0x8006d980`) meets a surface or actor within the foot's
reach (480 horizontally), else 340 along the view; a held attack with the Boot
selected starts the quick kick in first person while moving; the thigh is
drawn end for end from the knee (its hip origin is inside the loop's near
cull), `DNTTK_FP_KICK_THIGH=0` for the old look. Evidence: alley garbage bag
struck from about 520 with the crosshair on it (private copy of the user's
slot-5 state); running Boot left-click kicks twice and keeps running; pig cop
1500 per kick; 59.95 fps; Vanilla route `d12a-vanilla-2` exit 0; native and
Python tests PASS. Not located: the subway pallet. Thigh framing is still open
for the user. Binary
`e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`.

## 2026-09-30 - D12A thigh nudge (Needs playtest)

User: "incredibly fun"; nudge only the thigh down by 10% and test. The thigh
matrix alone moves down the screen by 10% of the screen height at its
knee-end depth (`DNTTK_FP_KICK_THIGH_DROP`, default 0.10); calf and boot
unchanged. Captured at frames 7 and 13 before/after
(`recomp/analysis/d12a-kick/runs/thigh-drop-sheet.png`); `ttk-controls-test`
PASS (thigh offset is along the view's down axis only). Binary
`1a915e56d629844357cb36cc2d19ababa363134b1aea7b8e5c52a3db2a3bc358`.

## 2026-09-30 - D12A: thigh 0.13, E never kicks, Q chains (Needs playtest)

User: thigh drop 0.13 is the sweet spot; E into a wall could kick (only Q
should); Q should chain freely with any weapon while running, like the Boot's
held left-click. Changes: thigh default 0.13; in the eye view an original
Boot kick request converts to the quick kick only when the attack is held
(E's Action request restarts idle with no kick); a Q during a kick queues the
next, and a held Q repeats. Evidence: Boot + E no kick (3 suppressed); pistol
into a wall + E no kick; running pistol Q mash chains, held Q 7 kicks in 3 s;
native suites, Python and Vanilla route `d12a-vanilla-3` PASS. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D12A accepted (Done)

User: "its done! accepted. ... this has been yet another amazing feat of engineering." D12A moves to Done on explicit user acceptance after three playtest
passes (crosshair hits, Boot left-click while moving, thigh drawn and tuned to
0.13, E never kicks, Q chains), with the automated evidence in the earlier
entries. Recorded limits stand and were accepted: no knockback, lower damage
per kick than a point-blank original kick, right leg only, first street and
alley coverage only (the subway pallet was not located). Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D08L inertial edge run-off (Needs playtest)

Failing comparison first (isolated instance, private apartment-state copies,
real keys): the apartment bed run-off and walk-off were already continuous
(~47.5 and ~9.5 per frame), but running off the fire-escape platform outside
the apartment window (drop >= 2048) fell at 3058 (16.5 per frame, a third of
run speed) with the mouse camera frozen, because the D08F short-fall lease
only covered 256-768 drops. Also found: the stride estimate can spike to its
14000 clamp at a run start, so 1 of 8 bed run-offs left faster than a running
jump (same on the pre-change binary). Change, Modernized only: the lease now
also covers a running fall over 768 that directly follows a ground stride;
every departure is capped at 10000 running (running jump 10085) or 2048
walking; edge-jump grace stays 768-only. After: platform run-off 9172 through
the fall, 2700 units of air travel, running landing; mouse look live in third
and first person; running jump unchanged; Space mid-fall gives no jump; walking
still stops at the edge; bed spike capped at 10000, other samples unchanged.
Native suites (new run-off cases), Python (74, 2 skipped) and Vanilla route
`d08l-vanilla-1` exit 0. Limits: feel unconfirmed; only this platform and the
bed measured; very high falls and wall-adjacent ledges not swept. Details:
[D08L note](documentation/66-d08l-edge-run-off.md). Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D08L accepted (Done)

User: "im happy with that!" D08L moves to Done on explicit user acceptance of
the inertial run-off, with the automated before/after evidence in the previous
entry. Recorded limits stand: only the fire-escape platform and the apartment
bed were measured; very high falls and wall-adjacent ledges were not swept.
Binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D13 higher internal resolution and display scaling (Needs playtest)

Scope and decisions: exposed the runtime's existing supersampling (1-4x),
fullscreen tri-state, window width and present filter as per-profile launcher
preferences instead of adding a renderer feature. A reviewed runtime patch
(`time-to-kill-zzzzzzzz-presentation-cli.patch`) adds `--internal-scale`,
`--display`, `--window-width` and `--output-filter` CLI overrides; profile
schema 14 stores them (schema 13 migrates with a backup). Vanilla keeps 1x
(original presentation); Modernized defaults to 2x. The software renderer
always runs at 1x because its supersampling halves the frame rate (29 fps at
2x); the saved scale applies again with OpenGL.
Files or build identity: `recomp/tools/local/player_profiles.py`, `run.py`,
tests; binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`.
Verification and evidence: isolated Xvfb instance (Mesa CPU OpenGL, worst
case): OpenGL 1x/2x/3x/4x 59.9/59.9/59.5/50.3 fps in the apartment; software
1x 60.4, 2x 29.1. 1x vs 4x window captures show sharper geometry and model
edges, same texels, identical HUD. Intro FMV framing identical at 1x and 4x;
pause menu correct at 4x/nearest; saved 4x/borderless/1280/nearest survived a
relaunch with no flags; borderless/exclusive went fullscreen under metacity
with a centered 4:3 image. Vanilla route `d13-vanilla-1` exit 0 at 1x; Python
79 OK (2 skipped); native `ttk-input-test` PASS.
Remaining limitations / next action: user playtest on the real desktop and
GPU (fps at 2x-4x, exclusive fullscreen mode change, visual preference).
4:3 only (widescreen is D14). Pre-existing unrelated issue recorded: the
runtime patch stack check fails on `host_osd.c` drift from the
inventory-strip patch. Details:
[D13 note](documentation/67-d13-resolution-display.md).

## 2026-09-30 - D08U backlogged (Todo)

The user reported the problem during the D13 playtest: there is no way to latch
onto a ladder from the platform at its top and climb down. Reproduction: the
user's savestate slot 12. Added D08U (depends on D08, D08J) with investigation
and acceptance criteria. No code changed; D13 remains Needs playtest.

## 2026-09-30 - D13 playtest follow-up (Needs playtest)

User: "I have just been playing at 4x and it's holding 60 FPS the whole time it
just looks amazing." They asked for 4x as the Modernized default, which it now
is. Alt+Enter did not leave fullscreen. Cause: the runtime keymap compared
modifiers exactly, so a real left-Alt event (0x0100) never matched the stored
KMOD_ALT (0x0300). Ctrl+F had the same problem. Fixed by comparing Ctrl/Alt/
Shift groups (`host_keymap.c`, with new unit cases). A related finding: in
this SDL3 build "exclusive" was identical to borderless. Exclusive now sets
the desktop display mode explicitly (`psx_apply_fullscreen_display_mode`).
Both fixes are in the D13 runtime patch. Verified under metacity on Xvfb:
Alt+Enter and Ctrl+F toggle windowed/fullscreen in both modes, plain Enter
does not. Vanilla route `d13-vanilla-2` exit 0, Python 79 OK, host keymap and
native input tests PASS. Binary
`ecc9328065b8a6c3311423e1640936b1835e1517d35b923605609595e2f3a95d`.
The user's other reports are not caused by D13 and were backlogged: D10B
(headless Duke and see-through doors, same at 1x and with the original camera)
and D08V (sewer mantle lease flicker). Next: the user confirms Alt+Enter and
exclusive on their desktop.

## 2026-09-30 - D13 windowed default and remembered display (Needs playtest)

User: "make windowed the default rather than fullscreen ... it should also save
the last known config". Windowed was already the built-in default. The
user's profile opened fullscreen because it had saved `exclusive`, so it is
now set to windowed (the v14 file was backed up). New: the runtime writes the
window state at exit (`--presentation-state` file: windowed or fullscreen
after any Alt+Enter, borderless or exclusive, last windowed width, 0 when
maximised). The launcher folds it into the profile that was played. Profile
schema 15 separates `display` (how the game opens) from `fullscreen_mode`
(what Alt+Enter enters, passed as `--fullscreen-mode`); v14 files migrate with
a backup. Verified under metacity: exclusive -> Alt+Enter -> resize to 1280
-> close saved `windowed / exclusive / 1280`; the relaunch opened windowed at
1280x960 and Alt+Enter entered exclusive; quitting fullscreen saved
`exclusive`. Python 82 OK (3 new), Vanilla route `d13-vanilla-3` exit 0,
`ttk-input-test` PASS, patch checked forward and reverse. Binary
`7bd2a001a295330397d45c73ca9bc2ca0bc575c0d889bab3259d06778b3a3d1c`.

## 2026-09-30 - D13 F11 fullscreen key (Needs playtest)

User: "make f11 the key to go fullscreen instead of this alt enter thing".
F11 is now the only default fullscreen key (runtime `host_keymap.c` default,
in the D13 patch); Alt+Enter and Ctrl+F are unbound. The modifier-matching
fix stays for rebound hotkeys, with keymap tests moved to a config rebind.
Launcher text, the manual and the docs now say F11. Verified under metacity:
F11 toggles window <-> exclusive, Alt+Enter does nothing. Keymap test PASS,
Python 82 OK, `ttk-input-test` PASS, Vanilla route `d13-vanilla-4` exit 0.
Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D11C backlogged; D10B corrected

The user noticed that the headless Duke happens only in slot 12. Confirmed on
a private copy: the head-joint hide bit that first person sets while drawing
was saved inside the savestate (flag `01`; slot 1 `00`). Nothing clears it
after a load, because `first_person_draw` never takes ownership of a bit that
is already set. Added D11C (depends on D11). D10B no longer claims the headless
head; it keeps the close-camera transparency and the doors. No code changed.

## 2026-09-30 - D13 accepted (Done)

User: "correct correct correct, D13 is good. I'd say let's approve it." D13 is
Done: per-profile internal resolution (Modernized 4x at 60 fps on the user's
GTX 1080 Ti, Vanilla 1x), windowed by default with the last display state
remembered, F11 fullscreen with a real exclusive mode, and linear/nearest
output. Evidence is in the preceding D13 entries and
[the D13 note](documentation/67-d13-resolution-display.md). Remaining,
recorded: 4:3 only (widescreen is D14); the runtime patch stack check still
fails on the pre-existing `host_osd.c` drift. Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D08W backlogged (Todo)

The user reported that in the first subway area's shallow water, A/D (with
Shift) plus Space always jumps forward, so sideways jumps are impossible.
Added D08W (depends on D08, D08C) to fix later. No code changed.

## 2026-09-30 - D16A cancelled; D08A3 backlogged (Todo)

User: the work done with the original assets "is the way to go"; HRP is
"probably unnecessary, considering how far we've taken it". D16A is Cancelled.
As its only replacement, D08A3 (depends on D08A2) will extract the original TTK
medkit, biomask, jetpack, steroids (and goggles) art to PNG and swap it into the
inventory switcher in place of the Duke3D tiles. No code changed.

## 2026-09-30 - D17 scope expanded: high refresh rate rendering (Todo)

The user supplied a backlog brief for arbitrary / high refresh rate support.
It overlaps D17 (presentation smoothness without faster simulation), so D17 was
expanded rather than duplicated: Match Display / 30-240 / Unlimited frame-rate
option, render timing decoupled from gameplay, interpolation, mouse-look
responsiveness, pacing, and a numeric 30/60/120/180/240 comparison. The full
brief is [documentation/68-d17-high-refresh-brief.md](documentation/68-d17-high-refresh-brief.md).
Audit and plan come first when selected. No code changed.

## 2026-09-30 - D11C stale head-hide reclaim (Needs playtest)

Chose the load-side option: it also repairs saves that already carry the bit
(slot 12) and rewind snapshots, needs no runtime patch and no save-time write.
`first_person_draw` (`recomp/src/ttk/first_person.inc`) now takes Duke's draw
(`duke`, identity checked) separately from the eye view (`eye`). At Duke's
draw entry, after releasing our own hide, a bit 0 still set on Duke's joint 9
record is unowned and is cleared, keeping the record's other flag bits; in
first person the normal hide then takes ownership as before. New debug counter
`fp.head_reclaims`. Vanilla is unchanged (the hook returns before any
Modernized work).

Original-game check (static, owned disc): in `SLUS_005.83` the only joint
record flag writes are `ori 4` at `0x8009d974` and `0x800a33c8` (bit 4). None
of the 30 unique `.OVR` payloads stores a byte at `+0x44` or sets bit 0 on a
joint record; the executable's five byte `|1` read-modify-writes are other
structures (GPU primitive code byte `+7`, `+3`, `+0x27e`). The head joint is
never hidden by the game itself, so clearing an unowned bit cannot undo a real
hide. Dynamic tracing of every level was not done.

Evidence:
- `ttk-controls-test` PASS with new cases: in third person a record of `0x11`
  becomes `0x10` at Duke's draw (other actors and the other draw loop leave it
  alone; counter +1); in first person a stale bit is adopted for that draw and
  released at the list step. `ttk-input-test`, `ttk-aim-test` PASS; Python 82
  OK (2 skipped).
- Private copy of slot 12 (`recomp/analysis/d13-resolution/cards`, identical
  to the player's file, which was not touched), run `d11c-fixed`, Xvfb :93,
  Modernized third person: after load Duke's joint flags are all `00`,
  `head_reclaims` 1, and the head is drawn
  (`runs/d11c-fixed-third.png`). P into first person: head hidden, 29 hides,
  no further reclaims, bit clear between frames; back to third person, head
  shown. Six saves taken in first person on private slots 4-9 all load with
  the bit clear (none happened to land inside the draw window).
- Player binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

Remaining: the user's own confirmation that slot 12 shows Duke's head. A save
still stores the bit if taken inside the window; it is harmless only with this
build, and loading such a save in Vanilla (no hooks) would still show a
headless Duke. The slot-12 file is not rewritten.

## 2026-09-30 - D11C accepted (Done)

User, after loading slot 12: "duke's head is back, mark as complete! well
done". D11C is Done. Remaining, recorded: a save taken inside the draw window
still stores the bit (cleared on load in Modernized; Vanilla would still show
it headless), and the slot-12 file is not rewritten. Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

## 2026-09-30 - D07D red dot off by default in Modernized (Needs playtest)

Profile: schema 16 (`player_profiles.py`). `DEFAULT_CONTROLS['red_dot']` is
now `False`. A file older than v16 whose Modernized profile still says
`red_dot: true` is switched off once, with the usual recovery backup and a
notice ("run.py --red-dot on restores it"). How an explicit choice is told
apart from the old default: it cannot be for pre-v16 files (the old default and
a deliberate "on" were stored identically), so every pre-v16 "on" is treated as
the old default, per the user's request. From v16 on, the stored value is the
player's own choice and later loads keep it. Vanilla still launches with
`DNTTK_RED_DOT=1` whatever is stored.

Hook (`weapon_aim.cpp` `marker_hook`): the old code needed an entry lease at
`0x80033AF8` plus `player_identity_ready()` (gameplay context, camera<->player
back-links and the full LEVEL00 control identity), and stepped aside in the
runtime's precise-interpreter slices. Any of those could let the dot through:
scripted/detached cameras, other maps, or a frame whose entry hook was skipped.
(The "precision aim" in the brief was `g_precise_mode`, the runtime's
interpreter slice, not right-mouse aim; held Mouse2 was already covered.) Now
ownership is proven at the single enqueue `0x8002BC18` with RA `0x80033DB0`:
S4 is Duke (`0x800D7198`; the routine never saves or writes S4, and its
only caller `0x80035434` passes the actor there), the marker frame's saved RA
at `sp+0x40` is `0x8003543C`, the primitive is the same textured quad, and the
weapon/marker code identity (`aim_guards`, all main executable) matches.
Static check on the owned disc: `jal 0x80033AF8` occurs once in the
executable and in none of the 30 unique overlays. Lockstep verification, call
bail and Vanilla still step aside. The quad is collapsed exactly as before; no
aim, target, option or damage state is touched.

Evidence:
- `ttk-aim-test` PASS with new cases: hides without an entry lease or player
  readiness and during `g_precise_mode`; no write for another actor, a foreign
  frame RA, lockstep, Vanilla, or changed marker code; red dot on leaves the
  quad. Earlier assist/aim-vector assertions unchanged.
- Python 85 OK (2 skipped), including new schema-16 migration/launch tests.
- New route `pc_input_probe.py --controls red-dot` (`red_dot_probe.py`):
  natural pistol scans, OpenGL, Xvfb. `d07d-off-gl`: hidden markers third 51,
  held Mouse2 59, first person 65. `d07d-on-gl`: 0 in every segment; targets
  acquired in both runs. The captures do not show the dot clearly either way
  (muzzle flash, red lighting), so they are not visual proof.
- The old `--controls aim-options` route now fails in its state sampler
  (LEVEL00 full-hash check does not accept the D08D apartment patch) and the
  menu D04 diagonal check; both predate this job and were not changed.
- Player binary `db5bf9640893ae582acefa27fca8f98f27017bd6f1aa4c956d203cf31e6ec67b`.

Remaining: the user's own look at play with and without targets in third and
first person, held aim, jetpack, swimming and a scripted camera; the first
launch migrates the saved profile (backup kept).

## 2026-09-30 - D08T1 hold to grab, E always mantles (Needs playtest)

Binding model taken from the brief's proposal (the user asked for autonomous
work): `grab` Mouse2 and `grab_alt` Alt (either key), both rebindable;
`original_aim` Unbound in Modernized; with `original` weapon aiming or original
camera the `grab` input is precision aim (R1) and only Alt grabs. Profile schema
17 moves Mouse2 from `original_aim` to `grab` (custom precision-aim inputs kept).

Before the change (documented in `documentation/70-d08t1-grab-manipulate.md`):
the `0x80051CF0` mask stopped E's Cross from ever climbing a pushable object,
E started the grab and let go, and Space ran a host climb. Now: the
`0x80051CF0` mask applies only while Grab owns Cross, so W + E climbs the
dumpster like any climbable object; a new generated entry hook on the original
idle Action `0x80051890` (callers `0x800467F4`/`0x80052C14`) masks E's fresh
Cross at a pushable-only object, so E never grabs. Grab is held: request with
automatic holster, latch while held, W/S camera-relative push/pull (original
motion), release/E/blocked/hit/pause/focus end it, a fresh press is needed to
grab again, Space/fire/Circle/R1 wait. Directions stay neutral until the
original leaves 119..121 after letting go (a live run showed a held W feeding
the running push cycle through the tank fallback). Space climb code removed.
Prompts: `HOLD RMB TO GRAB` (first two touches) and `W/S PUSH/PULL - RELEASE
RMB TO LET GO` (first three grabs), with the live binding name.

Mouse2 audit: Modernized with `view` aiming loses R1 where view aim was not
ready (airborne, unsupported weapons), the held-RMB original jump arc and
held-RMB jetpack facing hold; all remain reachable by binding `original_aim`.

Evidence (binary `66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`):
- Live, private copy of the D08T slot-5 alley state
  (`recomp/analysis/d08t1-grab/grab.py`), Xvfb real keys/mouse, third + view,
  first + view, third + original (Alt): W + E climbs to the top and never
  grabs; E alone does nothing (16-24 idle masks); hold Grab grabs, pushes
  2582 -> 3539 (shove runs on to 3795, original lets go, no regrab while held),
  fresh grab + S pulls 2582 -> 1639 with mouse look live; release then W + E
  climbs; Grab while walking in grabs, never climbs; W + E while grabbing lets
  go then climbs; pause ends the grab; Grab against other things does nothing
  and sends no R1; with `original` aiming RMB is R1 and does not grab. First
  person blends to the orbit while grabbing.
- Vanilla original pad, gapless route (`vanilla.py`): grab 121, push +1185,
  pull -923, no climb (D08T measured +1202 / -928).
- `ttk-input-test`, `ttk-controls-test`, `ttk-aim-test` PASS; Python 87 OK
  (2 skipped). D07D red-dot route rerun on this binary: hidden 53/58/63.
- `build.py` stops in its runtime patch-stack check (reviewed patches no longer
  apply cleanly over live runtime edits; not changed); generation was run
  directly with the same `psxrecomp_cli.py generate` command.

Remaining: user playtest of the feel (hold vs. release timing, Alt in their
desktop environment, prompts), other pushable objects, and a pushable object
that is also a switch (keeps original precedence; none known).

## 2026-09-30 - D08V sewer mantle lease (Needs playtest)

Reproduced on a private copy of slot 12 (`recomp/analysis/d08v-sewer`; the
player's file untouched): 180 degrees from the save, W + E mantles a ledge
(139/140, mode 8). Refusals matched the report: mantle 8/0 and 8/8 (camera
lease never covered attached traversal; pad lease needed mode == previous),
gait 0/8 and 0/9 just after, falls 107/108 9/9. Change: camera-only lease for
mantle/hang/pull-up (mode 8 134..142, mode 6 147..153, mode 7 149..153, entry
frames included), unowned 107/108 falls (not the owned short fall) and gait
while previous is 8/9; locomotion excludes the mantle/fall states (no facing or
gait writes); traversal pad lease accepts mantle/hang entry frames; new
`traversal_camera_ready()` counts as a modern lease (fall keeps Up/Down and
strafe pads); first person blends to the orbit there.

Evidence (binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`):
mantle timeline identical before/after with the orbit camera live throughout;
16-case sweep tank fallbacks 12 -> 0, no mantle lost; mid-mantle captures third
and first person; native and Python suites PASS; D08T1 route unchanged.
Remaining: user playtest in the sewers (other hangs were not reached), the
unconfirmed "mantling felt harder" (not reproduced; original routine unchanged).

## 2026-09-30 - D07D, D08T1, D08V accepted (Done)

User, after the three-job session: "awesome. accept". D07D, D08T1 and D08V are
Done on binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`.
Recorded limits stay: D07D captures were not visual proof; D08T1 verified on
the alley dumpster only (a started shove finishes; switch-flagged pushables
keep original precedence); D08V exercised one sewer ledge (other hangs follow
the same rule). `build.py` still stops in its stale runtime patch-stack check.
D08X is now unblocked by D08V.

## 2026-09-30 - D08U top-of-ladder mount (Needs playtest)

Research on private copies of slot 12 (sewer) and the alley state: the slot-12
ladder is a plain ladder (type 308, flag 0x200), a flat panel whose top is 507
below the walkway, bottom at a floor near Y 510. The original has no top mount
for plain ladders (only 185 from below and the airborne catch -> 156 -> 186);
Vanilla edge inputs, backing off the edge and a forced airborne reach all end
in the water. A holstered fixture writing the catch's fields at the top gave a
complete original climb, bottom and top exit. Change (Modernized only): E at a
ladder top (panel top 300..800 below Duke, within 420 of the panel and its
width + 120, settled ground; E stows first) writes the catch's attachment
(+0x17c, +0x180, +0x1c4, mode 3, anim 156) and blends Duke over the 12 updates
of 156 to the far-face climbing line, facing the ladder; S on a plain ladder
adds Cross so Duke climbs down and steps off; the original 190 top exit and
185 bottom step-off join the D08V camera-only lease with neutral directions
(no tank fallback or `ORIGINAL MOVEMENT`, no backstep 82 on landing);
`E TO CLIMB DOWN` hint twice a session; guards for 156's end, its table entry
and `0x80073ae8`.

Evidence (binary `6129f2ab76e533ec9670bf17eb1a9c4f08471fa2d0fa7d883fd82a11da945683`):
slot-12 mount and full descent in third and first person with pistol stow and
redraw; alley ladder climb up (exit leased), E mount at its top, descent and
redraw; walk/run-off, strafe past and E away never mount; Vanilla edge routes
identical to baseline; native controls (new D08U fixture), input and aim
tests PASS; Python 87 OK. Player's slot file untouched.
Remaining: user playtest of the feel of the mount blend (a host blend over the
original transfer, since the game has no top-mount animation), the second
alley ladder and other ladders not exercised.

## 2026-09-30 - D08X hold-E airborne ledge grab (Needs playtest)

Traced the original reach: `0x800557f8` enters 109 only after an animation
event arms it (`+0x224 |= 0x800000`), then `0x80055208` acquisition runs before
the flight collision. A standing jump at a close wall with a ledge in reach
(slot-12 walkway wall, 1024 high) hits it first (107) and bounces, in Vanilla
as well as Modernized. Change: while E asks for a reach during an original
jump (98/103/104, mode 9), the owned ballistic hook sets the arming bit at
once (not over blocking bits 0x241, never with precision aim or in Vanilla);
the original then enters 109 on the held Cross and decides the catch.

Evidence (binary `bd153dd540359b1fa5eafc84a9ea04492099abd20e7e8479d511b33d36b065b3`):
W + E + Space at the slot-12 wall now grabs (148, mode 6) and pulls up (140)
3/3 armed and 2/2 holstered with late E, first person 2/2; without E still
bounces 3/3; pit, walkway-end and tall-wall jumps unchanged; alley running jump
reaches at takeoff but the original does not catch that low object (same end
as before); D08 alley ladder transfer attaches 2/2; Vanilla bounce identical;
native controls (new D08X fixture) and input tests PASS.
Remaining: user playtest; apartment exterior and crystal-2 ledges not reached
(no private state); armed late-E in a short jump still loses to the stow.

## 2026-09-30 - D08W shallow-water jump direction (Needs playtest)

Reproduced on a private copy of UI slot 1 (subway tunnel, ankle-deep water):
A, D, W, S and run-then-strafe jumps all went forward (about 1600 forward,
10 sideways). Cause: `swim_boost_horizontal` rescaled the original launch
velocity (aimed along Duke's facing) instead of aiming it along the held
direction; the D08O ledge/climb assists also caught strafes. Fix (`swim.inc`):
aim along the held direction at max(original speed, wade minimum); assists
only for Space alone or W. Evidence (binary `7ca63455...3328`): subway A
(10, -1612), run-then-A (10, -1612), S (-1650, 17), run-then-D (-12, 1238);
crystal-2 wade (UI slots 2 and 3) follows the held direction too; suites PASS.
Remaining: user playtest; the platform ledge assist was not triggered here.

## 2026-09-30 - D08A3 TTK inventory icons (Needs playtest)

The original inventory screen has no icons; the game's own item art is the HUD
indicator set (jetpack, Bio Mask, goggles, steroids, health cross) in
`/DATA/FONTS.RAW` (raw VRAM image at (960, 0), 4bpp cells with their CLUTs),
indexed by the HUD sprite table at `0x800c44b4..0x800c4504`. New
`recomp/tools/local/build_ttk_inv_icons.py` rebuilds the pack from the disc
(pinned hash), keys out the cell background, keeps the selection frame, and
writes provenance and review PNGs (local only). Medkit uses the health cross
(no medkit icon exists); steroids extracted only. Live capture shows the new
icons with green % and frame unchanged; missing-pack fallback and a new
reproducibility test pass. Remaining: user confirms the look in play.

## 2026-09-30 - D14 widescreen first pass (In progress)

The framework clamps PSX widescreen to 4:3 unless a trusted activation plugin
requests an aspect. Added plugin `ttk.widescreen` (acts only for Modernized
with `DNTTK_WIDESCREEN=16:9`), a default-enabled plugin-only preloaded package,
and CMake staging for `recomp/mods/preloaded/packages`. run.py sets nothing,
so the player build stays 4:3; a default launch is pixel-identical to the
baseline. Private 16:9 experiment: the wider view renders, but TTK's 4:3
screen-space culling leaves black gaps on the widened right side, and the
ammo box is not anchored. Next: TTK cull sites in `[widescreen.cull]` plus
regeneration, right-HUD anchoring, then a `--widescreen` profile option.
Native D14 activation test PASS.

## 2026-10-01 - Playtest: D08U, D08W, D08A3 Done; D08X follow-up

User playtest of the autonomous chain (binary `db5288c2...3689`):
- D08U: "I completely accept that": E at the slot-12 ladder top grabs on,
  third-person descent flawless; running off the edge still drops.
- D08W: sideways and forward water jumps "all good ... completely accepted".
- D08A3: original HUD icons for health/medkit, jetpack, Bio Mask and night
  vision: "phenomenal ... I love that they are the authentic icons".
- D08X: ledges in the drained-water area catch reliably when standing below
  and jumping with E ("100%"); the ones that failed were too high. Follow-up:
  in the boxes room (wooden crates; user saved UI slot 11) Shift + W + Space
  with E held bounces off a box instead of mounting it mid-jump. D08X back to
  In progress for that case.
- D14: user asked for a 16:9 preview launch; run.py now honours a dev-only
  `DNTTK_GAME_CONFIG` override for the private 16:9 config copy.

## 2026-10-01 - D08X boxes-room crate mantle (Needs playtest)

Reproduced on a private copy of the user's UI slot 11: Shift + W + Space with E
into a crate bounced (107) 4/4. The crate (type 20, climbable 0x80/0x40) only
becomes the contact after the reach's acquisition, so it never caught. New
`ledge_reach_mantle()`: at the start of an E bounce off a climbable object whose
top is 0x100..0x3ff above Duke's feet (contact or cell-list fallback, in front),
start the original height-matched mantle 134..138 from mid-air. Evidence
(binary `9bade2a5...ba9c`): straight crate 3/4 on top (miss = jump from contact),
diagonal crate on top when reached, no-E bounces unchanged 8/8, wall ledge grab
and alley ladder transfer unchanged, native fixture and suites PASS. Also: the
user's standing permission to close an open game is now in AGENTS.md and the skill.

## 2026-10-01 - D08X crate hang and identity loss fixed (Needs playtest)

User: an E jump into crates under the opening hung Duke on a crate and modern
controls were lost. Cause 1 (pre-existing): the original object hang toggles
bit 0x40 of flag-table entries 149/152/153, which the controls identity guard
treated as code; guard split around them, plus a failing-guard log line.
Cause 2: that hang (mode 7) cannot climb when the space above is blocked; in
Modernized S lets go, and W with no climb for 40 updates lets go on its own.
Binary `bf368b68...bd45`; stack 3/3 release with modern controls kept;
regressions unchanged; suites PASS.

## 2026-10-01 - D08X first-tier crate hang (Needs playtest)

User: grabbing the first tier of a stacked crate glitched and A/D spun Duke
round. Cause: the reach catches the upper crate at the first tier's top into
the original pole-style object hang (mode 7), where A/D circle the object. In
Modernized a mode-7 catch on a crate-type object (0xc0) now lets go at once
(2..3 frames, then the normal fall/bounce). Binary `c1c49119...6fe7`;
crate mantle and offset-stack climb unchanged; suites PASS.

## 2026-10-01 - D08X higher grab confidence (Needs playtest)

User asked for higher confidence when jump-grabbing ledges. The original reach
probes 425 above Duke's root, so ledges just above the apex were missed. New
`ledge_reach_lift()` retries the original acquisition during an E reach with
Duke raised 160/320/480; the original decides the catch; Duke is then eased up
from his real height (no pop). Binary `43eda09c...0f61`: 2048 wall still
refused at 480 (caught at an experimental 960, smooth rise); 1024 wall, crates,
stack release, ladder transfer and ladder-top mount unchanged; native retry
test; suites PASS. Playtest: the drained-water ledges.

## 2026-10-01 - D08X angle forgiveness (Needs playtest)

User: crate-top jump to the ledge needs Duke perfectly square. The original
facing test allows 45 degrees of the body heading, which follows the camera in
flight. The reach retry now also tries the heading turned +-25 and +-51
degrees; the original catch squares Duke. Binary `8bb01385...1701`: mid-jump
camera turns -35/-50 now catch, approach sweep -15..+15 all catch; regressions
unchanged; suites PASS.

## 2026-10-01 - D08X crate-to-crate mantles (Needs playtest)

User: crate-to-crate mantles low confidence at an angle; keep it game-wide. The
jump mantle now works by proximity to any climbable object's own rotated box
through the whole E jump, uses the original ground line-up (0x8007ec4c) before
every mantle, takes the full climb 139 for steps just above the mantle range,
and skips objects with something stacked on them. Binary `ccb4eb2a...0107`:
crate-to-crate 18/18 from -45..45 degrees; no-E unchanged; floor crate, stack
drop, alley object, wall grab, ladder transfer unchanged; suites PASS.

## 2026-10-01 - D08X flush hang (Needs playtest)

User: hands sometimes grab the air above the ledge until a shimmy. The catch
kept the probe height; the shimmy settles the root 454..463 below the ledge
top. E-jump catches now ease to ledge top + 456 (old rise-by-lift easing
removed). Binary `60161f81...369c`; hang matches the shimmy within ~5 on
two ledges; regressions unchanged; suites PASS.

## 2026-10-01 - D08X accepted (Done)

User: "thats it, fully accepted this!!!" after the flush-hang build
(`60161f81...369c`). D08X covers: early E reach, crate and object mantles from
any angle (original line-up, full climb, stacked-object skip), reach retries
(lift up to 480, heading +-51 degrees), flush hang settle, object-hang release
and the identity-guard fix for the original flag-table toggles.

## 2026-10-01 - D14 preview triage; session accepted

User accepted everything else from the session ("everything else wins for me
and i accept"). D14 16:9 preview reported off centre with large geometry
holes; 2-minute triage points at TTK's 4:3 projection centre, draw area and
culling under the native-wide path. Backlogged with ordered next steps; D14
stays In progress for a fresh session.

## 2026-10-01 - D14 widescreen (Needs playtest)

User asked for widescreen at least as good as DuckStation fullscreen. Causes of
the broken preview: `nw_hud_corners` shifts every untagged polygon for a title
without a sprite anchor (off centre, torn), and TTK culls against its 4:3 portal
root rectangle `0x800d2210` (holes). Native-wide (original projection, wide
surface, 1:1 present) kept over the squash path after an A/B.
- `widescreen_view_rect()` widens `0x800d2210` by the margin at the render's
  projection load (`0x800b4d9c`, ra `0x8002e4d0`) before the portal walk.
- D11B conservative near clip also runs in third person while widescreen is live.
- HUD: new hooks `0x8008BA30` (status bar: layout `0x800dd778` x moved out by the
  margin) and `0x8001FC44` (restore); regenerated.
- Plugin accepts 16:9/16:10/21:9 and `auto` (adaptive to 21:9); profile schema 18
  `controls.widescreen` (default 16:9), `run.py --widescreen`, settings choice W;
  `game.local.toml [widescreen] gte_game_mode = true`.
Evidence (binary `3f726b91...af2c`): margin black 0.39 -> 0.11 at fixed headings,
near-wall margins 0.945 -> 0.000; HUD corners at 16:9/21:9; pause/Select/night
vision/first person/inventory strip; FMV/title/menus pillarboxed; `auto` resize
16:9 -> 21:9 -> 4:3; run.py path; DuckStation (portable, private config) shows
the same view stretched by 4/3. ttk-controls-test, ttk-near-test, ttk-input-test,
Python 92 OK; Vanilla route `d14-vanilla-1` exit 0. Software-GL fps at 4x drops
about 20-30% (fill rate); real-GPU fps not measured. Needs a playtest across
levels.

## 2026-10-01 - D14 widescreen accepted (Done)

User: "im very happy with it! i accept!" (binary `3f726b91...af2c`). Case study
recorded in the D14 notes: DuckStation's widescreen hack gives the same wider
view but stretches every 2D element (text, HUD, menus) and Duke by 4/3, while
the recomp renders the extra columns natively and keeps text, HUD and movies
at their original proportions.

## 2026-10-01 - D08Y backlogged with research

User report: slot 5 gap jump bounces off the far platform unless started 3-4
feet early with E. Private-copy sweep (takeoff 0..900 back, E and no E,
current binary): no E 0/10 cross; E lands 0..200, catches 700..900 (all via the
D08X turned retry), bounces in a 300..600 dead band. Cause: the D08X jump
mantle accepts only climbable objects; this edge is level geometry. Job D08Y
added as Todo with options and acceptance;
[research notes](documentation/77-d08y-gap-jump-research.md). No code changed.

## 2026-10-01 - D08Y implemented (Needs playtest)

`ledge_reach.inc`: `ledge_reach_drop()` retries the original acquisition
`0x80055208` lowered 120..480 during an E reach; a ledge catch 0x60..0x4c0 above
the feet becomes the original height mantle at the real height (misses and
ladder/object catches restore the whole player record). `ledge_step_up()` turns
the first updates of a bounce after a 98/103/104 jump into mantle 134 when a
ledge's top is at most 0x100 above the feet, presenting the acquisition's
held-Cross bit for the isolated call only. Slot 5 (private copy, binary
`dce01716db09cbd3b72038d99e3051ef0fbe61fd91a16ac1802d2e994cad209c`): E 0..900
back 10/10 armed + 4/4 holstered across (900 back: catch, then pull-up 140,
2/2); no E 0..300 back across 4/4, 400..900 bounce as before. Regressions vs a
D08Y-disabled baseline: slot-12 wall grab/pull-up, crate mantles, crate-to-crate
8/8, ladder-top regress and the alley ladderjump fixture (no attach on either
build) unchanged; angle +15 now mantles onto the ledge instead of catch and
pull-up (same end). Vanilla: D08Y counters 0. Native controls (new D08Y
fixture), input, aim and Python 92 OK. Finding: a Vanilla running jump from the
edge crosses this gap; Modernized's flies ~3% slower horizontally (53.1 vs 54.9
units/frame) and falls short. Not changed here.

## 2026-10-01 - D08Y round 2: lip launch restored, reach hitch fixed (Needs playtest)

User playtest 1: the jump works (cleared or a quick mantle), but a brief hard
pause follows it; changing the jump is acceptable if the levels need it.
Hitch: per-update timing showed the D08X + D08Y isolated acquisitions at up to
27 ms per reach update. `original_call()` copies by words, D08Y snapshots by
words, the lowered retry runs only descending and unturned: ~6.5 ms average.
Jump: identical flight physics; Vanilla holds a jump pressed near a gap until
the lip (`0x80078c0c` look-ahead queues `+0x228` bit 4), which the Modernized
`0x800780b4` hook had overridden. It now leaves the look-ahead alone before a
drop over 768 and `pc_input` holds Square for a queued edge jump (40 updates
after a press). Slot 5: no E 8/10 land (misses pressed >1024 out), E 14/14.
Bed jumps still immediate; fire-escape presses launch from the lip. Regression
routes, Vanilla and suites as in documentation/77. Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`.

## 2026-10-01 - D08Y round 3: freeze frames and pop smoothed (Needs playtest)

User: the jump is made now; an odd freeze frame / pop remains. Rail-like jump
kept; a manual modern jump is backlogged as D08Z. `DNTTK_FRAME_TRACE=1`
showed every E-reach frame late (~50 ms, 5.7 ms of hooks). One budgeted
scheduler (`ledge_reach_retry()`, 4 core + 1 extra isolated acquisitions per
update), two step-up tries per bounce update, and changed-words-only restores:
reach frames 33.3 ms median, none over 40 ms. The E jump mantle no longer snaps
Duke ~400 units to the catch point; it glides there over its first updates.
Slot 5 and all regression routes as in documentation/77. Binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`.

## 2026-10-01 - D08Y round 4: freezes were emulated-CPU overruns (Needs playtest)

User session log (`DNTTK_FRAME_TRACE`): 386 of 4,203 frames at 20 fps in
clusters while running; Modernized hooks ~0.03 ms in them. Reproduced on the
player's GPU offscreen with a new diagnostics-only driver (`DNTTK_TEST_DRIVE`):
144 (third person) / 59 (first person) slow frames per ~869. Cause: TTK's own
code exceeds the PlayStation CPU budget per frame in views the Modernized
camera and widescreen reveal. Fix: runtime CPU overclock (new reviewed patch;
device timing, game speed, save states unchanged; skips exempt; movie poll
adjusted) and profile schema 19 `cpu_overclock` (Modernized default 150,
`run.py --cpu-overclock`). Result 0 / 0 slow frames; intro movie real time;
jumps unchanged; suites pass. Xvfb timing proved misleading (documented).
Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`.

## 2026-10-01 - D08Y round 5: overclock only in gameplay (Needs playtest)

User: audio slowdowns with the round-4 build. Reproduced: boot/loading at
150% fell behind real time (14,000 underruns). The overclock is now leased
from the Modernized player update (lapses 3 fields after gameplay stops) and
pauses for 5 s if emulation falls behind real time. Real GPU and audio: boot
0 underruns, gameplay 59.95 fields/s, 0-1 slow frames. Binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`.

## 2026-10-01 - D08Y accepted (Done)

User: "the stuttering and audio issues, and freezeframes are now fixed.
confirmed", after earlier confirming the slot-5 gap is made consistently.
Read-only system check on request: no Timeshift snapshot, scheduled job or
disk stall during the play sessions; Cinnamon idles at ~36% CPU and the
storage drive (sdb) reports 113 C (noted to the user, unrelated to the game).
Next job: user's choice (D08Z manual modern jump is backlogged).

## 2026-10-01 - D08Z manual jump style implemented (Needs playtest)

Measured what makes the jump feel on rails (slot 5, first person): the lip
launch near gaps, a fixed ~1 s arc with no air control, and a 14..18 field
crouch before standing/walking jumps. New Modernized option `jump`
(profile schema 20, `run.py --jump assisted|manual`, `--settings` choice J,
`DNTTK_JUMP`; Vanilla always assisted). Manual (`recomp/src/ttk/manual_jump.inc`):
jump on the press before gaps too, edge grace for every run-off (14 input
frames), bounded air steering from the owned ballistic update (98/103/104,
owned short falls, E reach 109; wade jumps excluded), preparation 96 x3.
Variable jump height and extra air speed considered and not taken. Evidence:
slot-5 timing table and same-binary regression comparison in
documentation/78-d08z-manual-jump.md; native controls/input/aim pass, Python
95 OK. Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`. Playtest: feel of steering rate and grace
(`DNTTK_AIR_CONTROL` scales the rate without a rebuild).

## 2026-10-01 - D08Z accepted (Done)

User played the manual style: "i love it. lock it in." The player's profile
keeps `jump: manual`; `assisted` remains the default for new profiles and
Vanilla is unchanged. Tuning values stay as shipped (steering 0.14 of the
takeoff speed per update, 14-frame edge grace, preparation x3). Binary
`4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`.

## 2026-10-02 - D17 high refresh rate with redrawn in-between frames (Needs playtest)

Audit and plan first (documentation/80, sections 1-8). TTK simulates at 30 fps
with a variable delta (5 per field, flip `0x8001fcbc`); presenting faster never
touches that. Measurements showed TTK's own renderer costs about 11 ms per extra
image on this CPU while the game uses about 25 of every 33 ms, so the user chose
the **parallel redraw**:
- New Modernized option `frame_rate` (schema 21, `run.py --frame-rate
  display|30|60|120|144|165|180|240|unlimited`, settings choice F; default 60,
  Vanilla always 60). `display` follows the window's monitor and re-reads it on
  display/mode changes.
- Runtime patch `time-to-kill-zzzzzzzzzz-render-replay.patch`: display
  deadlines served from device-service edges and the pacer (even presents
  whatever the guest does), frozen-machine replay sessions with full VRAM/GPU
  save and restore, forked worker processes that redraw published frames in
  record mode, image cache, cadence ring and `render_replay` / `replay_dump`
  debug commands; opt-in idle-skip extensions.
- Plugin `frame_replay.cpp`: publishes each frame at `0x80026164`, queues the
  previous frame at composition end (`0x8001fba0`), interpolates camera
  (`0x800d6eb0` rotation, view, eye, anchor, distance, room), Duke's joint
  matrices and every object-loop transform through `0x800292a0` (recorded live,
  substituted in the worker), ships first-person draw state, and draws redraws
  ahead of their present within 1.6 ms per field. Hooks `0x80026164`,
  `0x800632b0`, `0x80031d10`, `0x80032e78`, `0x80031c14`, `0x8001fba0`
  (regenerated).
Evidence (documentation/80 section 11): redraws equal the real image at alpha 0
and the next one at alpha 1 (street third/first person, animated objects);
per-frame RAM+MMIO fingerprints identical to 60 over 630 frames at 30, 120, 144,
165, 240 with driven input and at 180/unlimited without input; driven input at
180/unlimited sometimes differs from one pad sample (open). Cadence p50 equals
the target at every rate (180: 5.56 ms, p99 7.39), guest at real time, no
underruns; 62-72% of game frames get a redraw (about 50 distinct images/s).
Python 97 OK, native input/controls/aim pass, Vanilla route `d17-vanilla-1`
exit 0. Binary `67127d8073983fc6c4a1cc9bee2d520d7aae40e0e3ebcd2e568d4742c098849e`. Needs the real 180 Hz monitor: smoothness, tearing
(swap interval 0) and `display`. Linux only for redraws; elsewhere pacing only.

## 2026-10-02 - D17 playtest review: heavy scenes, doorways, FMV, kick (Needs playtest)

The first playtest: the strip-club street dropped frames with occasional audio
artifacts while the sewer was smooth; outdoor textures less stable while moving
and easier views through doors; FMVs flickering and dim; the quick-kick leg
left visible after the kick. Profiled rather than lowering the target
(documentation/80 section 13; new dev profiler `PSX_PROF`, `PSX_PROF_CALLERS`,
`PSX_PROF_REPLAY`):
- The emulation thread was saturated in heavy scenes (guest work 900-970 ms/s),
  so the governor refused most redraws. Largest costs: a forensic display ring
  reading the whole VRAM back from the GPU every frame (9.2%), always-on
  per-store/per-block forensic observers (about 8%), GL driver revalidation from
  binding and unbinding the FBO, program and VAO around every batch, a host
  clock read at every device event, and a full SPU state copy per sample query.
- Fixes: `PSX_FORENSICS=0` for player sessions (launcher, not with
  `--diagnostics`); a GL binding cache (batch CPU time halved, images and window
  pixel-identical with it on and off); the presentation tick every 4096 guest
  cycles; `spu_ctrl_reg()`. Guest work in heavy slots fell to 530-680 ms/s.
- In-betweens now follow the rate (round(rate/30) - 1: 3 at 120 Hz, 5 at 180)
  with a budget that grows with them, and no preparation starts too close to a
  present. At the player's settings (120 Hz, 100% CPU, first person) all ten
  test slots, street and ladder room included, show a distinct image on 69-81%
  of presents (ceiling 75%, 83% at 20 fps), no underruns.
- Doorways: in-betweens took the camera room from the nearer frame; when the
  rooms differ the worker now runs the game's portal walk `0x80039dd0` (stored
  by the camera update at `0x8003b0f8`) for the interpolated eye. Verified
  pixel-neutral when the room is unchanged and effective by a probe.
- FMV: CPU-path and blank presents invalidate the replay presenter (no redraws
  during the intro; luma 42.2 against 41.5 at 60 Hz). Kick: kick state ships
  with each job; kick scratch is allocated before the first kick (it reforked
  the workers). Dumps show no leg after a kick.
Evidence: equivalence to 60 Hz holds (630 frames; first person slot 3 at 144 has
identical RAM and scratchpad every frame, only a load-time burst in the
cumulative MMIO hash); Python 97 OK; Vanilla unchanged; runtime patch
regenerated (17 files, reverse-check OK). Binary
`5ffcbd643ae8e37f924d794ac76ca68938c4cb64f39de997115197ae7f0dc1e3`. Remaining:
PS1 vertex snapping and affine warp are more visible with small camera steps
(geometry correction is off for this game); a third-person camera lerp can pass
a wall corner; above 120 Hz the heaviest scenes get fewer in-betweens than the
rate asks for. Needs the real monitor: smoothness on the street, doorways, FMVs,
kicks, tearing and `display`.

## 2026-10-02 - D17 second playtest: choppiness, popping near the eye, fps readout (Needs playtest)

User report at Match Display (180 Hz), fresh game: FMVs now right; slight
choppiness outside the strip club, in the apartment and the subway control
room; textures and geometry popping near the eye (sink, wardrobe sides, light
switch, subway walls, power button); console `fps` stuck at 60. The session log
showed the guest falling behind real time. Findings and fixes
(documentation/80 section 14):
- Offscreen scripted input (`DNTTK_TEST_INPUT`, developer) and new harness tools
  (`steer.py`, `monitor.py`, `popsweep.py`, `seqcheck.sh`, `shift2.py`) with the
  D11B apartment and subway states reproduced the reports.
- Repeated images: the six-entry image cache thrashed (in-betweens evicted
  before being shown, then redrawn); the picture moved on two or three presents
  of a game frame and held for four. Cache 16 entries, evicting past frames
  first.
- Game slowdown: whole-surface save/restore around every redraw (about 300 MB)
  saturated the GPU memory bus and stalled the emulation (apartment turn: 7
  guest fields per second). Copy-on-write saves of what each redraw writes (two
  rectangles), verified exact by a surface-hash self-check.
- In-betweens follow the measured game frame interval (eight at 20 fps).
- Popping: out-of-order images (above); rotation blends now exact at both ends
  (endpoint residuals; alpha-0 redraws pixel-exact); the view matrix slerped
  with its row scales instead of lerped (weapon drift); near-clip edge cuts
  computed in canonical order (seam cracks).
- `fps` overlay and title show presents, distinct images and game images per
  second above 60.
Evidence at the offscreen GPU's lowest clocks (worst case), 180 Hz, first
person: apartment, street and subway hold 60-62 guest fields per second with
about 180 distinct images per second (every present advances). Gameplay
equivalence holds (630 frames). Python 97 OK; Vanilla unchanged; patch
regenerated. Binary
`71a1d0a20ab7d0a09d0dd03d7ce0f483f4d63f305866ab94544fda0fef6441fd`.
Remaining: native-pixel snapping and the original meshes' T-junction cracks
(inherent; geometry correction cannot resolve TTK's CPU-built packets);
`ttk-input-test` needs a live desktop. Needs playtest on the 180 Hz monitor.

## 2026-10-02 - D17 accepted; D17A, D17B, D17C added; D17C experiment (Needs playtest)

**D17 Done (user-accepted).** Third playthrough at Match Display (180 Hz):
about 180 FPS with very good stability, FMVs correct, fps readout correct. The
user proposed accepting D17 and moving the remaining issues to separate jobs;
agreed: the job's goal (rendering at the display's rate with unchanged game
timing) is met, and the remaining issues are specific refinements.

**New jobs.** D17A (texture/geometry instability: popping, flicker, black
areas; places and hypotheses recorded), D17B (mouse responsiveness and input
latency at high refresh; measurement plan recorded), D17C (view bob).

**D17C experiment.** Measured first: in first person the eye follows Duke's
root position (`player+8`), and the walk and run cycles lift that root above
its standing height and back by about 55 units a step on a flat floor (the
apartment); the camera rotation and projection do not oscillate. With
`DNTTK_VIEW_BOB=off` (environment, Modernized first person) the eye follows the
bottom of that cycle: any descent at once, a rise within 80 units only very
slowly, a larger rise (jump, step up, climb) at the usual rate. Eye height range
over 1.5 s on the flat floor: walking 55 to 10 units, running 59 to 21 (slow
drift of a few units, no per-step swing). The bob is only a camera position, so
removing it saves no rendering work. Default behaviour (bob on) and third
person are unchanged. Launch:
`DNTTK_VIEW_BOB=off python3 recomp/tools/local/run.py`.

## 2026-10-02 - D17C parked; D17A first investigation (In progress)

**D17C parked.** User verdict on `DNTTK_VIEW_BOB=off`: no noticeable
difference; the wobble they see is the PS1 geometry wobble on floors and walls
everywhere, which belongs to D17A. The bob looks innocent. Parked, to revisit
after D17A in the user's comprehensive test. The experiment switch stays
(environment only, default unchanged).

**D17A.** Details: [D17A notes](documentation/81-d17a-instability.md).
Ruled out with measurements: in-betweens drawing different game state (alpha 1
equals the next real image in 12 savestate slots, subway, apartment and club,
at 150% and 100% CPU, apart from known 30 fps content), unfaithful redraws
(alpha 0 exact, about 1800 redraws), texture uploads between frames (none
during play), out-of-order presents (trace monotonic), wrong-buffer capture
(new runtime counter: 0).

Reproduced: near-wall flicker in first person at 16:9. In the strip club,
against the orange-framed wall panels, consecutive presents go back and forth
(a near wall feature moves or disappears and returns). Measured: 8-22
back-and-forth presents per 18 sequences by default; 0 with
`DNTTK_NEAR_MODE=full`, 0 at 4:3, 0 with the world near clip off, 0 with alpha
1. Cause localized to the D11B conservative world near clip: its pieces of a
very wide near wall polygon draw last within their ordering-table slot, and a
neighbouring original polygon whose key sits on a slot boundary changes slot
with a one-unit rounding wobble, so it is covered in one image and visible in
the next. Real frames do the same at 30 per second; at 180 Hz it shows up to
five times per game frame. Two fixes tried and removed (slot bias: no change;
linking pieces at the chain close: near walls vanished in the apartment and
street). Not reproduced yet: apartment with the lights off, alley platform,
club stairs and stools, street pavement (savestates from the user at those
places would make them testable).

Kept: the near clip's frame accounting now ships with each redraw job
(`DNTTK_REPLAY_NEAR_STATE=0` restores the old behaviour), diagnostics
(`DNTTK_REPLAY_LOG=1`, runtime `multi_area`), harness tools. Alpha 0 rechecked
after the change (apartment, street, club: exact). Runtime patch regenerated.
Binary `04ec89349eb4064cd0623755f18d551ebb0229dc494d3cf2f0f35b0fb9f9cf97`.
No player-visible change yet; the flicker remains. Next: sort the conservative
world pieces stably against same-slot original polygons (insert at the taken
polygon's own position in the mesh order), then rerun the club test (target 0)
and the apartment/street still A/B.

## 2026-10-02 - D17A precise near geometry, D17B late camera (Needs playtest)

From the user's strip-club test and savestates (UI 4-9; F7 UI slot N = file
N-1). Details and tables: [D17A notes](documentation/81-d17a-instability.md),
second pass.

**D17B.** Mouse slowdown with the dancers: the game drops to 20 fps there at
CPU 100% (emulation stays at 60 fields/s, turn rate unchanged); in-betweens
blended two game cameras, so a mouse step took about 66 ms (30 fps) or 95 ms
(20 fps) to show. New late camera: every present is a redraw with the newest
mouse look (input pumped plus SDL motion peeked), submitted just in time, alpha
0 redraw instead of the real image, nth present shows in-between n. Step
response about 17-19 ms at 20 and 30 game fps; presented yaw monotonic.
`DNTTK_LATE_CAMERA=0` restores the old blend.

**D17A.** Runtime host precise vertex channel (exact positions, depth and
texture coordinates for a plugin's own packets, through live frames and redraw
streams). The near clip's full mode now uses it and is the default: tables and
walls beside the eye have correct perspective, no seams or slivers, the club
wall flicker is gone (3 back-and-forth presents in 27 sequences, normal turning),
the chair no longer pokes through the table. `DNTTK_NEAR_MODE=conservative` for
comparison.

Checks: alpha 0 exact in four places; club driven gameplay equivalence 60 vs
180 identical 631/631 (three runs); Python 97 OK; ttk-near-test (link stubs
fixed), ttk-controls-test, ttk-aim-test PASS; Vanilla route d17a-vanilla-1
normal. Offscreen cost: guest 60 fields/s everywhere, 93-180 distinct images/s
(GPU at idle clocks). Not reproduced: Duke stuck after loading UI 3. Runtime
patch regenerated (17 files). Binary
`85cf07ac50012aa597f0f980d7ae85f527d22ba747078e745eddbd0f3d08565d`.

## 2026-10-02 - D17B late camera off by default after playtest (In progress)

User: new game at Match Display extremely jerky. Reproduced from boot: at CPU
100% the opening street leaves too little emulation-thread time for a redraw
per present, and the late camera repeats images (judder). Late camera now off
by default (`DNTTK_LATE_CAMERA=1` developer); D17A precise near geometry stays
on. Next: present-time reprojection for mouse turning. See
documentation/81-d17a-instability.md.

## 2026-10-02 - D17A/B/C third playtest: closet, neck, idle eye, view bob, late camera with pacing (Needs playtest)

User savestates UI 1-5 (private copies `cards-user3`). Details:
[D17A notes](documentation/81-d17a-instability.md), third pass.

**D17A.** Closet triangles over the medkit (UI 3): host triangles now write and
test a depth buffer among themselves (`PSX_HOST_DEPTH=0` off); the weapon's
pieces skip it. Collar noise on the woman (UI 4): GL sampling limits come from
the exact UVs (`PSX_HOST_UV=0` off). Missing apartment wall (UI 5): present and
stable with the current build.

**D17C.** Idle breathing (UI 1) measured: the idle animation lifts Duke's root
about 18 units and the eye followed it. The eye now takes its height from the
camera pivot and x/z through a dead band: idle eye position exactly constant;
crouch, jump, steps and drops still move it; PS1 wobble while moving kept.
Deliberate view bob from distance walked, Modernized option `view_bob`
off/subtle/on/strong (default on; `--view-bob`, settings `B`; profile schema
22; `DNTTK_VIEW_BOB` also takes a numeric scale).

**D17B.** Club mouse (UI 2): TTK at 15-20 game fps with the dancers; the mouse
is direct angles (no analogue emulation, sensitivity unchanged). Late camera
back on by default with adaptive pacing (new image every 1st, 2nd or 3rd
refresh from the measured repeat share; `DNTTK_LATE_PACING=0` off,
`DNTTK_LATE_CAMERA=0` blend). Real display 180 Hz: club settles at every 2nd
refresh (70-92 distinct/s), opening street every 3rd (60/s, even); step
response about 17-19 ms (was 66-130 ms).

Checks: alpha 0 exact (club, driven input, 12 sequences); Vanilla route
`d17a-vanilla-2` normal; Python suite 98 OK; native near/controls/aim PASS.
Runtime patch regenerated (17 files). Binary
`b1193a90c1f25d87f79ba8257d7926745127a3a6e7dc02faa56e9472a24f4d68`. Open: Duke
stuck after the earlier UI 3 load not reproduced; present-time reprojection if
paced turning still feels heavy.

## 2026-10-03 - D17A see-through regression fixed; D17B dancer jerk; D17C idle stability accepted (Needs playtest)

Fourth playtest: idle stability accepted by the user (D17C idle part); collar
fixed; club slowdown better. New: see-through outdoors and in the apartment,
and a jerk sweeping across the dancers. User states UI 1-4, private copies
`cards-user4`. Details: [D17A notes](documentation/81-d17a-instability.md),
fourth pass.

**D17A.** The holes came from the third pass's host depth buffer: outdoors TTK
does not clear its frame, so depth from an earlier frame hid new walls. Depth
is now cleared per drawing area (GP0 E3/E4) and per redraw session, before the
first host triangle. UI 1, 2, 4 solid in stills and while moving; closet fix
kept.

**D17B.** Causes measured at the dancers: in-betweens planned per refresh
while pacing presented every 2nd/3rd (positions advanced a third of the way,
then jumped); frames longer than planned (last image repeated with an old
look); finished images refused by the budget at their own present. Fixed:
plan per paced present, overrun redraws while the game has not flipped
(alpha 1, newest look), due images drawn regardless of the budget, lead 16 ms.
Real display at the dancers: 2.6-5.2% of moving presents repeat (was about 7%
and more with the jumps). No actor pop at the screen edge (checked).

**D17C.** Idle stability accepted; view bob option unchanged; awaiting the
user's view on the bob settings.

Checks: alpha 0 exact (late camera off; club, apartment, doorway); Vanilla
route `d17a-vanilla-3` normal; Python suite 98 OK; native near/controls/aim
PASS. Runtime patch regenerated (17 files). Binary
`f9f64faa4e939326b5a10f4eb0b6cbdaa1d31d47af979a25100986f6378865ea`.

## 2026-10-03 - D17B loss of control fixed, late camera rebuilt; D17A/C (Needs playtest)

Fifth playtest: turning jerky from a fresh game; Duke lost all movement after
loading UI 4 and stayed frozen across loads; tree/statue and apartment
see-through confirmed fixed (user); a table artifact at the club exit (UI 1)
and closet popping (UI 6) reported. Details:
[D17A notes](documentation/81-d17a-instability.md), fifth pass.

**Loss of control.** The game's tap-or-hold use/holster button (Circle): held
past about 100 ticks of game-frame time it enters the hold-to-select inventory
mode (`+0x224 0x200`), which stops Duke until the release. Our layer sent
Circle as a hold while fire drew a holstered weapon and as 4-6 frame pulses
for E stow/restore; at 15-20 game fps that crossed the threshold. Host Circle
requests are now single taps (pressed until the game samples it). Also: after
a savestate load the camera kept the previous yaw and turned Duke; new runtime
`psx_mod_savestate_loads()` re-seeds the camera and drops pending host
requests on a load.

**Jerkiness.** Fourth-pass frame-end redraws were made almost every present
and starved the redraws that mattered. The late camera now makes one redraw
per present, positioned at that present's time in the game frame, fed only for
the coming present, with the mouse look sampled at a fixed lead from
timestamped events, an adaptive lead and pacing with hysteresis. Real display
at 4x: holds of 2+ presents while turning 0.1/s (street) and 0.6/s (dancers).

**Not reproduced:** UI 1 table, UI 6 closet popping (no back-and-forth
presents in 54 sequences). D17C idle stability unchanged.

Checks: alpha 0 exact (late camera off, club); loads 3 -> 4, 5, 0, 4 walk;
Circle tap test; Vanilla `d17a-vanilla-4` normal; Python 98 OK; native
near/controls/aim PASS. Patch regenerated (18 files). Binary
`5f7406a6465570624fe129aaadcecee5960d80cca8707bab5fa1adbe642d2b39`.


## 2026-10-03 - D17A/B systemic renderer quality pass (Needs playtest)

See [renderer quality evidence](documentation/82-renderer-quality-pass.md).
Fixed replay writes outside the saved wide framebuffer band, depth scissoring,
fallback depth backup and post-snapshot pending-clear restoration. Aligned eye
setup before special room walks; transform substitution survives changed
traversal order; precise replay vertices require current-session provenance.
Removed prefetch-only batch flushes, fixed Unlimited planning capacity/double
rate division, and used worker readiness tails for adaptive sampling lead.
D17 accepted high refresh and D17C accepted idle behavior remain the baseline.

Profiling: worker p95 21.3 ms, actor p95 8.3 ms. Final real-display club turn:
180 target about 85 distinct images/s, present p99 13.0 ms; Unlimited about
100 distinct/s, p99 11.9 ms. Still not stable 180, and no mouse-to-photon claim.
459 final 2x live-surface isolation checks pass; 24 normal capture sequences
show no detected backsteps. Endpoint comparisons 409/412 exact, three street
turning comparisons unclassified. Native near/controls/aim/input pass;
Python 98 pass. Patch regenerated with 19 runtime files and reverse-check
passed. Player preferences/cards untouched. Full playtest remains required.

Binary SHA-256:
`a81d2d2869ee0fcd116b741a34984cf7e6e6b189de6431362966d7100263df41`.

Final safety checks: no-input 60/180 RAM/MMIO 630/630 identical. Driven
60/180 629/631; two transient RAM mismatches remain unclassified, MMIO exact;
repeated driven 60/60 631/631. Vanilla fresh-card route exits 0 with normal
reviewed captures and compiled FMV decoder active. These limits are recorded
in the quality note, not treated as a clean campaign acceptance.

Real-mouse A/B: candidate and retained baseline both retain long low-motion/
repeat runs; present p99 about 18.64 ms in both. The yaw-based hold heuristic
is not an image-freeze or photon-latency measurement. No responsiveness win
claimed. Candidate restored and test instances closed.


## 2026-10-03 - D17B opening/club stabilization candidate (Needs playtest)

The user's fresh-game report became the primary acceptance route, with the
alley/apartment comparison and private UI slots 8-11. Profiled systemic causes:
scene/actor CPU cost, native-frame prediction and idle-budget scheduling,
inconsistent native-code validation after restore, an initialized hot math
routine falling to the interpreter, excess GL submission, unnecessary worker
reforks when movie code was invalidated, and a worker timeout publication race.

The candidate uses a timestamped presentation timeline, tail-aware camera lead,
exact guarded native math code, coherent worker/restore code validation,
compatible ordered batching, and corrected worker lifecycle/timing. Vanilla
remains available; the D17C idle eye and PS1 movement character are preserved.
No individual object coordinates were patched.

Final fresh boot, CPU 100%, first person, 4x, 180 Hz: from the first valid
redraw, p99 6.0 ms and maximum 8.3 ms, zero audio underruns and failed/killed
workers. Final Match Display dancers turn: 179.5 distinct images/s, p99 6.18 ms,
59.97 emulation fields/s and zero audio underruns. Apartment comparison:
179.8 distinct images/s, p99 5.98 ms. Real mouse injection to traced camera:
median 17.85 ms club / 12.48 ms apartment, not input-to-photon latency.
Unlimited remains adaptive; final turning sample 169.2 distinct images/s,
p99 6.41 ms, realtime emulation and zero audio underruns.

Tests: 98 Python, native controls/aim/input/near clip, compiled code-guard and
timing/deadline tests; 37,748 native/interpreter math comparisons without a
divergence; 2,115 worker GP0 comparisons and 4,224 full-image batch comparisons
without differences; pause/load/header regressions; Vanilla fresh-game route;
543/543 cycle-aligned full-RAM matches between no-input 60/180 runs. Full
campaign and audible listening remain user playtest work. D17A closet/furniture
popping remains open; do not call it eliminated by these performance results.

Build SHA-256 `ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`,
codegen `8bab543c`. Known player saves remain compatible. Player settings/cards
untouched; nothing committed. Details: [documentation/83-opening-responsiveness.md](documentation/83-opening-responsiveness.md).


## 2026-10-03 - D17A/B next-save camera stability pass (Needs playtest)

Latest private UI 1/2 slowdown, 3 subway, 4 opening, 5 early club, 8 minor
slowdown, 10 late club. Details and limitations:
[84-warmup-and-camera-stability.md](documentation/84-warmup-and-camera-stability.md).

Measured dancer updates legitimately span five guest fields; a four-field
camera timeout reset accumulated mouse look. Removed that timeout while keeping
explicit ownership/epoch invalidation. Native tests cover 5/8/12-field gaps.
Late-job expiry now feeds readiness and permits two presentation intervals of
grace. Compatible opaque/translucent batching reduces CPU/GL work; first-use
scratch reservation avoids worker replacements; pinned SDL X11 timestamps now
use the correct units and preserve event spacing. GPU diagnostics do not wait
for incomplete queries.

In a matched 65-second 500 Hz real-mouse club comparison, camera turns over
one degree fell from 54 to zero, maximum 7.66 to 0.79 degrees. Presentation p99
alone was about 6.1 ms in both: FPS hid the camera defect. Slow slots recover
realtime emulation with zero audio underruns. Final 180 Hz sweeps retain about
179-180 distinct images/s. Fresh gameplay p99 6.07 ms, max 7.83 ms.

Unlimited still exposes stale-camera episodes; subway peripheral culling did
not reproduce on the bounded 60/180 Hz route and is not declared fixed. Earlier
closet/furniture pops, actual-device audio listening and full campaign remain
playtest work. Vanilla stays available. Player originals untouched; no generated
C or media edited. Final SHA-256:
`28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.

Final verification: 100 Python tests; native controls/input/aim/near suites;
2,848 exact GP0 timing comparisons and 3,056 exact RGBA batch comparisons;
third-person club, first kick/shot without refork, pause/resume/repeated loads,
and Vanilla intro-to-gameplay route. All twelve original states match their
hashes/mtimes. No game left running.


## 2026-10-03 - accepted baseline, bounded movement polish and D17D split

User accepts opening/club/apartment playability as a major quality milestone.
Preserve `28554051...` as the rollback baseline. New private UI 12 Shift+W/S
shows saturated movement interpolation despite mostly regular presentation;
complete snapshots often arrive too late for the current preparation window.
Larger buffering and stronger phase correction did not solve this adequately
and were rejected. No interpolation/culling/audio behavior changed.

Retained: ordinary launches disable per-batch GL timing diagnostics; explicit
profiling enables them. Added opt-in pose capture/completion tracing. Final
opening/dancer mouse tests preserve about 179-180 images/s, zero underruns and
no camera turns over one degree. Slot 12 translation and slot 10 source/audio
starvation remain open; slot 1's minor crackle was not reproduced as an underrun.
100 Python tests pass and original 12 saves remain byte/mtime-identical.

D17D Todo separately records furniture (also visible at 60 Hz), closet and
peripheral culling imperfections. No geometry workaround shipped. Details,
rejected experiments and final measurements:
[85-movement-polish-and-isolated-artifacts.md](documentation/85-movement-polish-and-isolated-artifacts.md).
Final executable `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
D17A/B remain Needs playtest with the accepted playability baseline preserved.


## 2026-10-03 - D17A and D17B ACCEPTED; 120 FPS quality baseline

User explicitly accepts both jobs after natural play through a significant
portion of level 1 at 120 and 180 FPS. Parent systemic work is closed, not
held open by isolated polish issues. 120 is the primary uncompromising quality
regression target; 180+ remains excellent high-refresh support, 240+ robustness
and compatibility, Unlimited stress/debug. No technical ceiling or simulation
speed change. D17D-J and D18C retain the valid residual cases and dated private
state identities. No follow-up implementation started during closeout.

Accepted implementation/baseline, evidence, limits and frame-rate explanation:
[86-d17-acceptance-and-regression-baseline.md](documentation/86-d17-acceptance-and-regression-baseline.md).
Local executable/source snapshot preserved. User explicitly revoked the old
blanket recomp exclusion during closeout: authored implementation, tools, tests,
complete framework patch and pinned dependencies are now committed alongside
documentation. Media/generated output/player data remain excluded.


## 2026-10-03 - D17D near-coplanar shelf depth (Needs playtest)

User authorized autonomous background testing and a systemic investigation;
then clarified that genuinely coplanar placement may remain a documented
limitation. Exact prop FT3s `0x8012983c/0x8012984c` share the floor plane to
within one unit in the traced quantized transforms. Integer screen snapping
with unchanged view Z also made independently subdivided depth planes cross.

Near polygons now evaluate depth on the source plane at their snapped raster
positions, independently of unchanged texture perspective. Static props get a
two-unit normal depth tolerance for the transform contact ambiguity; actors
and first-person weapons do not. No per-object coordinate fix or culling
change. Truly coincident/intersecting surfaces remain intrinsically ambiguous;
this does not close D17E/F/J or promise all popping fixed.

Offscreen before/after captures at 60/120/180, slow yaw/pitch and high-refresh
intermediate images show the intact lower shelf. Opening/club/subway/closet
regressions inspected; 190 surface restore checks have zero mismatches.
100 Python tests (98 pass, 2 skip), four native suites and runtime codec /
guard / deadline tests pass. Input capture tests use private Xvfb, since SDL
offscreen does not support relative mouse capture. Full framework patch
exported and verified byte-for-byte against a clean pinned dependency.
All 12 original player savestate hashes and mtimes match. No player settings
or media changed. Candidate `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
Details and measured limits: [87](documentation/87-d17d-contact-depth.md).


## 2026-10-03 - D17D/F accepted; blood and tabletop props queued

User: "its genuinely fixed"; confirms the prop in UI 9, closet in UI 11,
and closet inside the strip club. D17D and the related closet job D17F are
Accepted on this explicit player evidence. Accepted implementation remains
`b88d0e5`, binary `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.

Added D17K for ground blood immediately ahead in UI 12, and D17L for props
on the table directly ahead in UI 11 that cut off/reappear when walking
forwards/backwards. Both are Todo for the next round, with cause unverified.
Record current private save identities before reproduction; no new save copy,
code edit, build or game launch was performed for this documentation closeout.
The next session waits for the user's job selection after clearing context.


## 2026-10-03 - D22A portal transition control/view loss queued

Added the user's new bug: UI slot 7 -> walk into the portal -> start the next
level -> all Modernized controls and first-person view are lost. D22A is Todo,
linked to D22 campaign/overlay coverage but scoped as a separate transition bug.
Cause is unverified; future investigation must retain identity guards and use a
verified private copy of the current save. No implementation, build or launch
was performed. D17K/L remain queued and D17D/F remain accepted.

## 2026-10-03 - D17C accepted; D17E/K/L selected

User confirms the camera is much more stable and is happy with its current look.
D17C is Done on that player acceptance, including the previously accepted idle
stability. No camera behavior changed for this closeout. D17E, D17K and D17L
are selected together for bounded visual investigation; D17D/F remain accepted.

## 2026-10-03 - D17E/K/L visual candidate

D17E is Needs playtest for closure: bounded full-width subway wall/edge/stair
routes at 60/120/180 did not reproduce the missing-wall report. No culling
workaround was introduced. D17K and D17L are Needs playtest on a bounded
candidate: preserve original farthest-source-corner world ordering for native
blood; keep compact static props in the enhanced depth path across the near
radius, with opaque faces grouped to avoid excessive draw-state changes.

The existing shelf/closet plane correction and contact tolerance remain intact.
The compact-prop rule uses authenticated caller code and local mesh extent,
not mesh IDs, coordinates or save-slot tests. Actors, large distant scenery and Vanilla keep their prior takeover behavior;
translucent faces retain individual depth sorting. Broad prop takeover
was rejected on measured rendering/audio cost. See
[88](documentation/88-d17e-k-l-visuals.md) for current-save identities, test
results, timing limits and player checks. D17C is separately Done on the user's
camera-look acceptance; D17D/F remain accepted. No other job started.

## 2026-10-04 - D17E/K accepted; isolated cup remains open; two new bugs

Recorded explicit user acceptance of D17E subway periphery and D17K ground
blood. D17C remains Done; preserve accepted D17D/F as well. D17L returns to
Todo because the specific cup still flickers, despite stable nearby bar props.
Read-only inspection of the unchanged UI 11 state identifies table/cup meshes
and shared bar/table prop prototypes; residual instance-level cause remains
unproven. Added separate Todo jobs D17M (UI 9 ladder ammo occlusion) and D17N
(UI 12 diagonal wall vibration), with new save hashes and no assumed causes.
No gameplay code, executable, settings or player saves changed; no game launch.
See [inspection and acceptance record](documentation/89-d17-playtest-followup.md).

## 2026-10-04 - D17L unchanged on subsequent user retest

The user tested again after documentation commit `215ff05` and sees no change.
D17L remains open (Todo); no additional gameplay fix was made or accepted.
Recorded this checkpoint at the user's request. D17E/K stay Accepted, D17C
stays Done, and D17M/N remain separate Todo backlog jobs. Documentation only;
no build, game launch or player-file changes were performed.

## 2026-10-04 - Immediate next: D08Q2 post-Continue jetpack unavailable

Recorded the user's new UI slot 10 state: death, Continue, `dnstuff`, then
jetpack activation unavailable and the inventory picker skips the item.
D08Q2 is Todo - immediate next investigation, ahead of D17 follow-ups. Cause
is unverified. Preserved a private save/card snapshot with hash manifest;
no gameplay changes, build or launch. D17L remains open; D17E/K stay Accepted.


## 2026-10-04 - D08Q2 post-Continue jetpack repair (Needs playtest)

Confirmed slot 10 has full fuel but stranded pending `0x8001`. Original reset
clears the model/active state without clearing pending; `dnstuff` retains it.
A private delayed-deployment fixture plus real RPG death/Continue reproduces it.
Recover only the authenticated alive, closed/off/owned jetpack with no active
transition. Both flight schemes, picker/J, fuel depletion/rejection and private
save/load pass; four native suites and the player build pass. All 28 player
card/state hashes and mtimes match intake. No framework/rendering change.
Natural map-pickup replay and user acceptance remain. No next job started.
[Implementation, exact fixture, verification and limitations](documentation/90-d08q2-jetpack-continue.md).


## 2026-10-04 - D08Q2 accepted; D08Q3 queued next

User: "confirmed fixed! accepted." D08Q2 is Done on explicit acceptance of
repair `2f02967`. Prior controlled-test limits remain recorded; no new tests
are claimed. The separate report that jetpack aiming is unusable and the
crosshair disappears is D08Q3, Todo - next. This session only updates the
backlog/status/handoff and commits/pushes documentation. No gameplay edits,
build or game launch; no other job started.


## 2026-10-04 - D08Q3 flight aiming and crosshair (Needs playtest)

Selected by the user for autonomous implementation. Reproduced armed jetpack
flight firing the original pistol while view-adapted shots stayed at zero and
the reticle was hidden. Shot/beam, reticle and presentation gates required
movement eligibility that intentionally excludes flight. Reused the existing
authenticated jetpack lease without enabling ground locomotion writes.

Both schemes pass private pistol, shotgun, RPG, energy, flame and freezer firing,
mouse yaw/pitch, movement/ascent/descent input, reticle toggle, J-off, fuel-out,
landing and first-person fallback/return checks. Native aiming, controls, input
and inventory HUD pass; local-dev build and movie/math shard checks pass.
D08Q2 native regression remains passing. Player data: 28 file hashes/mtimes
unchanged. No framework, generated C or renderer change.

User confirmation and broader thrown/upgraded weapon live coverage remain;
weapon switching still waits for the ground. See [note 91](documentation/91-d08q3-jetpack-aim.md)
for exact evidence and limitations. No other job started.


## 2026-10-04 - D08Q3 accepted; D17O sky investigation queued next

User accepts D08Q3: "i accept this! great work." Marked Done, preserving
`f284dc1` and its recorded verification limits. Next: D17O, Todo - next.
User reports glitchy sky movement when looking up from UI slot 10, explicitly
not limited to jetpack use. Determine how the game renders its sky before
choosing a stabilization; substantial engineering documentation is required.

Preserved current player data as a private immutable intake (28 files), with
hashes/mtimes; slot 10 has changed since the earlier jetpack fixture. See
[note 92](documentation/92-d17o-sky-intake.md) for the exact identity and work plan.
Documentation/intake only: no sky investigation, gameplay edits, build or launch.


## 2026-10-04 - D17O camera-relative sky redraw repair (Needs playtest)

The original backdrop and two rotating cloud bands translate to the current
eye. High-refresh world-object interpolation incorrectly captured them under
the last object and substituted older origins. Three authenticated resident
callers now retain the original sky construction. Sampled displacement:
maximum 1075.397 units before, zero in all 360 candidate trace samples.

Owned-disc model provenance, exact call/coordinate/timer/ordering contracts,
A/B diagnostics, visual sequences and rejected hypotheses are documented in
[note 92](documentation/92-d17o-sky-intake.md). Private ground/flight checks cover
60/120/180 and another street location; native near/input/aim/controls/inventory,
player build/shard checks, Modern/Classic flight transitions and 1106 surface
isolation checks pass. All 28 player hashes/mtimes remain unchanged.

120 FPS measured routes are clean. Busy club 180 FPS throughput/audio limits
also reproduce on the previous path. Original geometry and coarse cloud motion
remain; full campaign skies and visual acceptance are unverified. D17O stays
Needs playtest until the user confirms the reported sky appearance. No other
job started; no framework edits or generated/retail files committed.


## 2026-10-04 - D17O accepted (Done)

User: "amazing work!!! accepted. well done. document. commit. push".
D17O is Done, user-accepted. Preserve the camera-relative sky repair in
`25734d1` and the evidence in [note 92](documentation/92-d17o-sky-intake.md).
The preceding Needs playtest entry is historical. Original sky geometry and
coarse animation limits, busy-club 180 FPS throughput/audio limits, and broader
campaign coverage remain as documented; acceptance does not close other jobs.
Documentation-only closeout: no new gameplay test, code change, build or launch.
No next job selected or started.


### R01 - DisruptorRecomp architecture and modernization reference research

Research-only job selected 2026-10-04. Inspect a pinned, read-only upstream
checkout under ignored `research/`; compare geometry precision, gap handling,
perspective textures, sky, interpolation, input latency, CPU scheduling,
widescreen and supporting systems with current TTK. No source import, gameplay
edit, build, launch or replacement of accepted D17/D17B/D17O work.

Acceptance: record acquisition/revisions and ignore verification; produce a
source/function-cited architecture review with applicability classifications,
backlog mapping, licensing/provenance and ranked bounded experiments; recommend
whether to retain the checkout and identify the top three experiments.

Work log (2026-10-04, In progress): verified `/research/` ignore rule before
cloning; existing psxrecomp working-tree modifications are preserved.

Work log (2026-10-04, Done): completed the [source-cited comparative review](documentation/93-disruptor-reference-research.md)
at Disruptor revision `408f214d3109dbc6cbdded7edd128cbf8de6466a` and framework
`193a60b805e1eaa853129d6ccf63022440d4b143`. Retained the clean reference under
ignored `research/DisruptorRecomp`; verified real-file/nested ignore behavior
and no tracked research files. Findings distinguish image blending from TTK
worker redraws, gap-covering rims from culling/depth repairs, panorama sky from
TTK sky meshes, and guest overclock from host execution cost. Top experiments:
primitive/provenance census, optional D15 geometry/texture precision, then
bounded timing-overhead attribution. Recorded license text discrepancy before
any future source adaptation. No code import, build, launch, gameplay/renderer
change, media/save/card/preferences write or new implementation job. Existing
framework modifications preserved. Research acceptance met by static review;
upstream performance claims were not independently benchmarked.

User direction (2026-10-04): retain Disruptor as research only. Learn techniques
and write purpose-built TTK implementations; no 1:1 copying or mechanical
translation of third-party source. This direction does not start an experiment.

## 2026-10-04 - D17L selected; per-instance diagnostic preparation

User selected D17L autonomously. Status is In progress. Added an independently
written, opt-in title trace identifying static instances, source faces,
projection/clipping decisions and emitted host packets with ordering/depth.
Offline packet-equivalence, trace-contract and controls checks pass; player
build succeeds. UI 11 private-copy identity matches the report. No game launch,
new failing-frame reproduction or cause/fix claim. Explicit isolated launch
authorization is pending under AGENTS.md. Full acceptance remains unmet.
See [diagnostic contract and remaining work](documentation/94-d17l-primitive-diagnostics.md).

## 2026-10-04 - D17L native tabletop overdraw repaired (Needs playtest)

User authorized background coding/test launches and requested replacing the
old launch restriction; AGENTS.md now records that standing permission.
Actual title/GP0 provenance identifies cup instance 0x801de8d0 and a later
native top face on supporting table 0x801de6f0. The table bypassed host depth
and overwrote the lower cup. No instance/mesh address is a repair condition.

Depth-only upper faces of authenticated opaque static box meshes preserve
native coordinates, affine UVs, diagonal, NCLIP and AVSZ ordering. A broader
surface experiment was rejected for cost. Final 60/120/180 approach/retreat,
accepted visual regression routes, native packet/control tests, 102 Python
tests (two skips), 312 clean replay restore comparisons and Vanilla exclusion
pass. 120 timing samples have zero output underruns; 180 remains below target
in busy scenes, with a small additional candidate deficit. Player visual and
audio acceptance remains outstanding. No player files, original media,
generated code or framework changes. Research inspired measurement only;
all implementation is purpose-built TTK code. See [note 95](documentation/95-d17l-tabletop-depth.md).

## 2026-10-04 - D17L accepted (Done)

User: "first off, i accept this as complete!" D17L is Done, user-accepted.
Preserve implementation `1e6fb0a` and the evidence in
[note 95](documentation/95-d17l-tabletop-depth.md). Earlier Needs playtest and unresolved
candidate entries below are historical. Busy-scene 180 FPS limitations remain;
this acceptance does not close other jobs or establish full campaign coverage.

The user clarified the workflow: implement and test, obtain acceptance, then
wait for an explicit instruction to document, commit and push. They authorized
this closeout in the next message. AGENTS.md records that approval requirement;
background development testing remains authorized. This closeout changes only
documentation and instructions, with no new build, launch or gameplay test.
No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D15 accepted; D17P immediate follow-up

The user accepts the final slot 5 black-area, slot 6 closet and previous slot 8
opacity fixes, plus previously accepted stability, floor and idle improvements.
They explicitly authorize documenting, committing and pushing the implementation
and backlog update. D15 is Accepted; D17P records the replacement subway slot 8
as a separate immediate follow-up. See [note 96](documentation/96-d15-accepted-precision.md)
for implementation, user evidence, automated/private verification and limits.

## 2026-10-04 - D17P distant bands candidate (Needs playtest)

Implemented the Modernized Draw distance option for the UI slot 8 subway bands.
The bands are three original rendering limits (far cut-off with a squeezed fade,
integer portal rectangles that drop distant ceiling strips, and integer NCLIP on
one-pixel faces); Vanilla shows them too. `extended` (default in Modernized)
doubles the render-only limits, widens portal rectangles by 2 native pixels and
uses exact PGXP signs for NCLIP when precision is Corrected. Profile schema 24,
`--draw-distance`, settings D. Private measurements: matched views clean, 12-slot
sweep without game-rate, budget or replay-miss regressions, 120 Hz cost unchanged,
Vanilla untouched. Tests pass. Codegen hash unchanged, so existing savestates
load. The framework's unregistered `test_host_vertex_depth.py` was repaired and
registered. Needs the user's playtest; nothing committed.
See [note 97](documentation/97-d17p-distant-bands.md).

## 2026-10-04 - D17P accepted (Done)

User: "i fully accept this fix." D17P is Done, user-accepted, with the Draw
distance candidate (executable SHA256
`5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`) and the
evidence and limits in [note 97](documentation/97-d17p-distant-bands.md). The user
authorized documentation, commit and push. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.

