# Time to Kill — modernization jobs

This is the canonical job list for our **Duke Nukem: Time to Kill** PC project, targeting the owned US SLUS-00583 disc. The ambition is a polished, game-specific PC edition: faithful original play plus an optional modern experience. This is a plan, not a list of features already available.

Invoke **`$continue-duke-recomp`** (Codex) or **`/continue-duke-recomp`** (Claude Code) to see the current jobs and choose one. You can also request a job directly: **`$continue-duke-recomp work on D01`** or **`/continue-duke-recomp work on D01`**. The skill reads this file rather than keeping a second backlog. It must not automatically start the next job.

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
| D05 | Camera-relative WASD movement | Done | D03, D04 |
| D06 | Independent third-person mouse camera | Done | D03, D04 |
| D07 | Modern weapon aiming and crosshair | Done | D05, D06 |
| D07A | View-aligned Duke facing and weapon presentation | Done | D07 |
| D07B | Optional assisted view aiming and display controls | Done | D07 |
| D07C | Unified Duke3D-style aiming, facing and projectile coverage | Done | D07, D04 |
| D08 | Modern traversal controls — accepted iteration | Done | D05, D06, D07 |
| D08A | EDuke32-style weapon and item shortcuts | Done | D04, D08 |
| D08A1 | Visible EDuke32-style inventory cycling | Done | D04, D19A |
| D08A2 | EDuke32 bottom-left inventory icon and green % | Done (revised: strip + green %) | D08A1, D19A |
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
| D08L | Inertial platform edge run-off | Todo | D08 |
| D08M | Modern underwater swimming controls (foundation) | Done | D08 |
| D08O | Deep free-swim polish (strafe, Ctrl dive, mantle-only exit) | Done | D08M |
| D08N | Duke3D-style scuba gear item | Cancelled (out of scope) | — |
| D08P | Crystal-2 turret / scripted-camera control recovery | Done | D08 |
| D08Q | Modern jetpack flight controls | Done | D08 |
| D08R | Selectable jetpack scheme: Modern / Classic (WASD), CLI quick ship | Done | D08Q |
| D08Q1 | Faster Modern jetpack Ctrl descent (underwater dive speed) | Done | D08Q |
| D08S | Duke3D-style jetpack scheme (instant J on/off, midair) | Todo | D08R |
| D09 | Modern controller support | Todo | D05, D06, D07 |
| D10 | Third-person camera polish | In progress | D08 |
| D10A | Rapid mouse turning and Shift-running investigation | Done | D06, D07C, D08 |
| D11 | First-person playable prototype | Todo | D08, D10 |
| D11A | Scroll-wheel zoom lock into first-person | Todo | D10, D11 |
| D12 | First-person weapons and state polish | Todo | D11 |
| D13 | Higher internal resolution and display scaling | Todo | D02 |
| D14 | Widescreen, FOV and visibility | Todo | D03, D13 |
| D15 | Optional geometry and texture precision | Todo | D13 |
| D16 | Texture filtering and game-specific HD assets | Todo | D13 |
| D16A | HRP assets and first-person weapon research (later) | Todo | D11, D13, D16 |
| D17 | Presentation smoothness without faster simulation | Todo | D01, D13 |
| D18 | FMV and audio presentation safeguards | Todo | D01, D02 |
| D18A | Voice/music/gunfire crackle investigation | Done | D01, D02 |
| D18B | Concurrent voice with music (no music mute) | Todo | D18, D21 |
| D19 | Modern in-game menus, settings and input prompts (Sonic 3 A.I.R.-style customization; plan mode + artifact first) | Todo | D02, D04, D13 |
| D19A | Duke font assets for host messages and modern UI | Done | D04 |
| D19B | Responsive modern menu navigation and transitions | Todo | D02, D04 |
| D20 | Save management and optional quick saves | Todo | D01, D02 |
| D21 | Accessibility and sound controls | Todo | D04, D19 |
| D22 | Campaign fidelity and overlay coverage | Todo | D01 |
| D23 | Performance budgets and long-session stability | Todo | D01 |
| D23A | Modernized frame-budget regression (guard identity cost) | Done | D08 |
| D23B | Intro FMV stutter: stranded native movie shard | Done | D23 |
| D24 | Linux / Windows player build and disc import | Todo | D19, D22, D23 |
| D25 | Modernized edition release acceptance | Todo | D08, D08A, D08B, D09, D10, D14, D17, D18, D20, D21, D24 |
| D26 | Backtick debug console (fps and helpers) | Done | D04 |
| D27 | Caps Lock RUN MODE quotes; Shift-run clunk silence deferred | Done (quotes); clunk deferred low-priority | D04, D19A |
| D28 | Scroll Lock holster and WEAPON LOWERED/RAISED quotes | Done | D04, D19A |

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

**Todo from 2026-09-27 playtest.** Running off platform edges still feels like it
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

**Todo (later).** User (2026-09-29): a third jetpack scheme that replicates
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

### D09 — Modern controller support

Provide left-stick movement, right-stick look, configurable sensitivity/inversion, dead zones and sensible action bindings using the modern action layer. Support switching input devices and disconnect/reconnect.

**Acceptance:** stick movement is predictable, no drift at rest on the tested controller, menus and gameplay agree on bindings, and disconnects do not leave held actions. Record tested hardware and limitations.

### D10 — Third-person camera polish

**Distance portion accepted 2026-09-28.** The broader job remains In progress.
See [bounded implementation and evidence](documentation/33-controls-shortcuts.md).

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

### D11 — First-person playable prototype

Build an optional eye-level camera using the verified movement and aiming foundation. Investigate head/body visibility, near-plane clipping and room/portal culling. Keep third-person available as a fallback for unsupported states.

**Acceptance:** a documented first-level route is playable with correct collision and shot direction, no obstructing head geometry and no missing rooms caused by the camera offset. Mark unsupported states explicitly; moving the camera alone does not complete the job.

### D11A — Scroll-wheel zoom lock into first-person

**Todo from 2026-09-27 playtest.** Player wants Fallout/Skyrim-style entry: scroll
the existing third-person distance inward until the view locks into first-person,
then scroll out to return. Coordinate with D10 distance and D11 eye-level camera;
do not break Alt-wheel distance, weapon wheel, or Vanilla.

**Acceptance:** continuous distance scroll reaches a documented first-person lock
without FOV hacks alone; unlock returns to the preferred third-person distance;
unsupported states fall back cleanly; aiming/movement remain coherent.

### D12 — First-person weapons and state polish

Resolve held-weapon presentation, body visibility, eye height, recoil, traversal, death and scripted sequences. Decide whether existing geometry is sufficient before proposing new assets.

**Acceptance:** tested weapon/state combinations are usable without clipping or misleading muzzle placement; camera transitions remain coherent; third-person and Vanilla still pass their routes. Document any optional new assets and their provenance.

## Graphics and playback

### D13 — Higher internal resolution and display scaling

Explicit user priority: selectable high-resolution settings. Inspect and reuse applicable renderer capabilities, then expose tested internal-resolution choices, fullscreen/window modes and output scaling. Distinguish rendering more scene detail from enlarging a low-resolution image. Preserve original-resolution presentation.

**Acceptance:** comparison captures demonstrate the effect, settings survive restart, UI/FMV sizing remains correct and tested performance is recorded. Verify the active build actually supports each offered option.

### D14 — Widescreen, FOV and visibility

Explicit user priority: widescreen support. Render a wider view without stretching actors. Correct aspect, FOV, HUD anchoring, menus and room/portal visibility; define how original movies and fixed compositions are framed.

**Acceptance:** 4:3 and 16:9 pass representative indoor/outdoor tests without geometry popping at the added edges, misplaced aiming or stretched UI. Additional aspect ratios may remain explicitly unsupported.

### D15 — Optional geometry and texture precision

Investigate renderer support for reducing vertex jitter and perspective distortion, preserving authentic behavior as an option. Compare effects on animated geometry, effects and seams before enabling enhancements by default.

**Acceptance:** captures show actual improvements and any remaining artifacts; toggles restore original presentation; collision, visibility and game timing remain unaffected. Unsupported renderer features are documented rather than simulated by ineffective settings.

### D16 — Texture filtering and game-specific HD assets

Offer tested nearest/filtered presentation and investigate a narrowly scoped replacement-texture path. Preserve palette changes, animation, transparency and texture-page reuse. Establish a small verified asset sample before attempting a large upscale pass.

**Acceptance:** the sample replaces the intended textures only, handles their variants, has a fallback to disc assets and records asset provenance. Filtering, higher resolution and replacement textures are separate settings. A full HD pack is a later asset-production job scoped from this investigation.

### D16A — HRP assets and first-person weapon research (later)

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

Measure unique rendered frames, guest timing, host presentation and input latency independently. Investigate interpolation only where the game's data permits it; reset interpolation across teleports, room loads and camera cuts.

**Acceptance:** measured pacing improves without speeding up movement, scripts, audio or cutscenes; discontinuities do not smear or blend incorrectly. Report achieved unique-frame cadence honestly: a 60 Hz guest clock is not proof of 60 unique game images per second.

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

### D19 — Modern in-game menus, settings and input prompts

**Direction set by the user 2026-09-29 (backlog; not started).** The target
is a customization menu in the spirit of Sonic 3 A.I.R.: one aesthetically
pleasing place to pick Modernized options (control schemes such as the D08R
jetpack scheme, camera, aim, crosshair, display, audio and later
customizations), with clear per-option descriptions and good defaults.
Preferred approach: hack the original TTK in-game menu rather than bolt on a
separate host screen, and give it a responsiveness overhaul (see D19B) as part
of the same work. Until then, new options ship as persisted profile settings
with `run.py` CLI switches. **Before any implementation:** design in plan
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
