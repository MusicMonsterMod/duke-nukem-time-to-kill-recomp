# Time to Kill — modernization jobs

This is the canonical job list for our **Duke Nukem: Time to Kill** PC project, targeting the owned US SLUS-00583 disc. The ambition is a polished, game-specific PC edition: faithful original play plus an optional modern experience. This is a plan, not a list of features already available.

Invoke **`$continue-duke-recomp`** (Codex) or **`/continue-duke-recomp`** (Claude Code) to see the current jobs and choose one. You can also request a job directly: **`$continue-duke-recomp work on D01`** or **`/continue-duke-recomp work on D01`**. The skill reads this file rather than keeping a second backlog. It must not automatically start the next job.

**Latest accepted job (2026-10-08): D08Z2 (mantle pose; frozen legs after a corner bump); executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression baseline. Next: D08Z3.** Earlier (2026-10-08): D08Z1 (jump wall slide); executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. Earlier (2026-10-08): D08A17 (power-up countdowns); executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`. Earlier accepted jobs (2026-10-08): D08A14 (death ends steroids), D08A15 (heartbeat restart; a pickup mid-run stops steroids) and D08G4 (inventory cheats give full armor); executable `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.** Earlier: D08J2 (pole/chain A/D direction), D08U2 (climb off a ladder past an enemy), D08O3 (E in water keeps the weapon) and D08O2A (natural swim-fire pose v2), 2026-10-07.** Previous baseline `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e` (D08U2). **D08J2 (pole/chain A/D direction) Accepted 2026-10-07**: executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the current regression baseline ([note 122](documentation/122-d08j2-pole-chain-sidestep.md)). Todo: D08J4 (ceiling monkey-bar drops, UI slot 3), D08J3 (free camera while climbing). Previous baseline `e75b50f156137fa2377e4643407b2efbd16f2e976fa4d697afcbceb228e2e554` (D08O3). **D23E (western-town stutter) Accepted 2026-10-07**; **D08O2A Accepted** ([note 119](documentation/119-d08o2a-natural-swim-fire-pose.md)), **D08O3 Accepted** ([note 120](documentation/120-d08o3-e-in-water.md)); **D23F (fast CPU timing) Accepted 2026-10-05 and now the Modernized default**; executable `e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966` (includes D23E), [note 104](documentation/104-d23e-busy-scene-stutter.md), [note 105](documentation/105-d23f-fast-timing.md). Next suggested: D23G (finish the fast path) and D17S (auto frame-rate default).

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
| D08A4 | EDuke32-style portable steroids: pick up, store in items, use with R | Done (user-accepted) | D08A1, D08A3, D22B |
| D08A8 | Steroids countdown in the steroids HUD box (pill icon), not the armor element | Done (user-accepted) | D08A4, D08A6 |
| D08A9 | Picked-up inventory item becomes the switcher selection (Duke 3D feel) | Done (user-accepted) | D08A1, D08A4, D08A6 |
| D08A10 | Experimental: steroids heartbeat sound loop while they run (226 bpm, Duke 3D feel; may be reverted) | Done (user-accepted) | D08A4, D08A8 |
| D08A11 | `dnhyper` countdown in the steroids HUD box (D08A8), not the old armor element | Done (user-accepted) | D08A8, D08G |
| D08A12 | Modern dynamite: selecting it is safe (no forced fuse); Dynamite Behaviour Modern / Original | Todo | D08A, D08Q5 |
| D08A13 | Steroids independent of damage and armor: being hit never shortens the steroid countdown (Modern) | Done (user-accepted) | D08A4, D08A8, D08A11 |
| D08A14 | Death and Continue end steroids: no running effect after Continue, steroids gone from the inventory (Modern) | Done (user-accepted) | D08A4, D08A13 |
| D08A15 | Steroids heartbeat silent for a few seconds when steroids restart before the previous run ends; a pickup mid-run stops them, full dose held | Done (user-accepted) | D08A10, D08A13 |
| D08A5 | Mission item tracking: mission inventory on , / . (design E, revised) | Done | D08A1, D08A3, D24A |
| D08A6 | Selected gadget shown on the HUD: original item slot (design A) | Done | D08A1, D08A3 |
| D08A7 | Custom medkit gadget icon (switcher strip and HUD box) | Done | D08A6 |
| D08B | Broader traversal and scripted-camera coverage | Done | D08 |
| D08C | Directional jumps from standstill — accepted both input orders | Done | D08 |
| D08D | Apartment light-switch secret convenience | Done | D08 |
| D08E | Responsive holstered fire and weapon transitions | Done | D08 |
| D08G | Typed Duke-style debugging cheats — user accepted | Done | D04 |
| D08G1 | Original Duke3D cheat confirmation wording | Done | D08G |
| D08G2 | Silent cheat entry and centered confirmations | Done | D08G1, D19A |
| D08A16 | Power-up coin icons: invincibility, invisibility, Double Duke (human design; placeholder section in the font picker) | Done (user-accepted) | D08A8, D24A |
| D08A17 | Power-up countdowns: invincibility, invisibility and Double Duke get HUD boxes counting down, like steroids (Quake-style) | Done (user-accepted) | D08A16, D08A8 |
| D08A18 | Mission inventory: Enter tries to use the browsed item; elsewhere "can't use this here" (cheat-message style) | Done (user-accepted) | D08A5, D24A |
| D08G3 | `dnupgrade` cheat: upgrade all weapons (Laser Gatling etc.) | Accepted | D08G2 |
| D08G4 | `dnstuff`, `dnitems`, `dninventory` also give 100% armor; running steroids stop and a full dose is held | Done (user-accepted) | D08G |
| D08H | Apartment furniture, hidden pickup and switch targeting | Done | D08 |
| D08I | Responsive run-start and edge jumps | Done | D08 |
| D08J | Armed airborne ladder grabs and automatic weapon transitions | Done | D08, D08E |
| D08K | True crouch walking and animation feasibility | Blocked | D08 |
| D08L | Inertial platform edge run-off | Done | D08 |
| D08M | Modern underwater swimming controls (foundation) | Done | D08 |
| D08O | Deep free-swim polish (strafe, Ctrl dive, mantle-only exit) | Done | D08M |
| D08O1 | Fire weapons while swimming (Modernized, game-wide; medieval UI slot 2) | Accepted | D07C, D08O, D22B |
| D08O2 | Weapon points forward while swimming and firing in motion (v1: upper body to the view) | Accepted | D08O1 |
| D08O2A | Natural swim-fire pose v2: torso stays in the stroke, arms raised to fire and head looking up | Accepted | D08O2 |
| D08O3 | E in water keeps the weapon: redraw when no climb-out follows the stow | Accepted | D08O1 |
| D08N | Duke3D-style scuba gear item | Cancelled (out of scope) | — |
| D08P | Crystal-2 turret / scripted-camera control recovery | Done | D08 |
| D08Q | Modern jetpack flight controls | Done | D08 |
| D08R | Selectable jetpack scheme: Modern / Classic (WASD), CLI quick ship | Done | D08Q |
| D08Q1 | Faster Modern jetpack Ctrl descent (underwater dive speed) | Done | D08Q |
| D08Q2 | Jetpack unavailable after death, Continue and dnstuff (slot 10) | Done | D08Q, D08A1, D08G |
| D08Q3 | Jetpack weapon aiming and missing crosshair | Done | D07C, D08Q, D08Q2, D08R |
| D08S | EDuke32-style jetpack scheme (instant J on/off, midair, 61 s fuel) | Todo (reopened 2026-10-06) | D08R, D08Q3 |
| D08Q4 | Duke3D-style `dnkroz`: health to 100 and unlimited jetpack fuel | Accepted | D08G, D08Q |
| D08Q5 | Weapon switching while flying the jetpack (number keys and wheel) | Done (user-accepted) | D08A, D08Q3 |
| D08Q6 | Modern jetpack altitude creeps upward in level flight (hold height unless Space/Ctrl) | Accepted | D08Q, D08Q1 |
| D08T | Pushable objects: modern grab/push/pull and climb (alley dumpster) | Done | D08 |
| D08T1 | Separate push/pull from mantling: E always mantles, hold RMB to grab | Done | D08T, D04 |
| D08T2 | Duke-symbol pushable blocks cannot be pushed with Modern controls (RMB grab, game-wide) | Accepted | D08T1, D22B |
| D08U | Top-of-ladder mount: grab a ladder from a platform and climb down | Done | D08, D08J |
| D08U1 | Ladder that cannot be descended with E (player slot 12), systemic ladder-top coverage | Accepted | D08U, D22B |
| D08U2 | Enemy standing at a ladder top blocks the climb-off: Duke should push it back (player slot 5) | Accepted | D08U, D08J |
| D08T3 | Free manual push and pull while holding Grab (no fixed-size shoves) | Todo | D08T2 |
| D08V | Sewer mantle/hang modern-control coverage (slot 12 area) | Done | D08, D08B |
| D08W | Subway shallow-water sideways jumps (A/D + Space jumps forward) | Done | D08, D08C |
| D08X | Hold-E airborne ledge grab and mantle (ladder-grab feel for ledges) | Done | D08, D08J, D08V |
| D08Y | Gap jump dead band: jump-mantle level-geometry ledges (slot 5 gap) | Done | D08X |
| D08Z | Optional manual modern jump (player-timed takeoff, air control) | Done | D08Y |
| D08Z1 | Keep jump momentum when bumping a wall (EDuke32-style, menu-toggleable) | Done (user-accepted) | D08Z, D22B |
| D08Z2 | Mantle regression since D08Z1: the mantle plays as a static leg pose | Done (user-accepted) | D08Z1 |
| D08Z3 | Occasional landing after a jump where Duke is stuck for about a second before he can move | Todo | D08Z1 |
| D08J1 | Hold-E run-up grab for overhead ladders (slot-6 ladder) | Done | D08J, D08X, D08U |
| D08J2 | Poles and chains: A/D turn the wrong way (A turns right, D left) | Accepted | D08J |
| D08J4 | Ceiling monkey-bar climbing: camera-relative travel, no mid-span drops (player UI slot 3) | Accepted | D08J, D22B |
| D08J3 | Free camera while on ladders, poles and chains (investigation + usability testing) | Accepted | D06, D08J |
| D08J5 | Climb down chains (and poles): reach the bottom and let go or step off (player UI slot 3) | Done | D08J2, D08J3 |
| D08V1 | Modernized controls lost for the rest of the session after a missed chain jump; modern controls must never be lost | Done | D08V, D08J5 |
| D09 | Modern controller support | Todo | D05, D06, D07 |
| D10 | Third-person camera polish | Done | D08 |
| D10A | Rapid mouse turning and Shift-running investigation | Done | D06, D07C, D08 |
| D10B | Tight-space third-person camera: translucent Duke and see-through doors | Todo | D10, D11B |
| D11 | First-person playable prototype | Done | D08, D10 |
| D11B | First-person near-wall polygon clipping | Done | D11 |
| D11A | Scroll-wheel zoom lock into first-person | Cancelled (P toggle suffices) | - |
| D12 | First-person weapons and state polish | Done | D11 |
| D12A | First-person quick kick without leaving the eye view | Done | D12 |
| D12B | Costume-aware first-person kick leg, game-wide (LEVEL01 slots 8/9) | Accepted | D12A, D22A |
| D12C | Kick impact sound on a real hit only (wall, crate, actor); empty-air and out-of-range kicks stay silent | Done (user-accepted) | D12A |
| D11C | Savestates can keep Duke's first-person head hidden (slot 12) | Done | D11 |
| D11D | First-person eye height from Duke's real proportions, game-wide (LEVEL01 slot 8) | Accepted | D11, D22A |
| D11E | First person while swimming (underwater eye view, game-wide) | Todo | D08O, D11, D22B |
| D11F | First person while flying the jetpack (game-wide, all schemes) | Todo | D11, D08Q3, D22B |
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
| D17N | Diagonal wall artifacts during movement (world subdivision) | Accepted | D17A |
| D17Q | Residual wall/surface flicker: Corrected-renderer stability polish | Done (user-accepted) | D17N |
| D17O | Unstable sky appearance when looking up (slot 10) | Done (user-accepted) | D17A, D17B |
| D17P | Distant horizontal black bands in new subway slot 8 | Done (user-accepted) | D15 |
| D17R | Sky turns black toward the left/right edges when looking up, game-wide (LEVEL01 slot 9, medieval UI slot 2) | Todo | D14, D17O, D22A |
| D17S | Auto frame-rate default: follow the display up to a cap, step down on faster monitors | Todo | D17, D23F |
| D17C | View bob: disable experiment, then a stable modern camera | Done (user-accepted) | D11 |
| D18 | FMV and audio presentation safeguards | Todo | D01, D02 |
| D18A | Voice/music/gunfire crackle investigation | Done | D01, D02 |
| D18B | Concurrent voice with music (no music mute) | Todo | D18, D21 |
| D18C | Load-sensitive crackle at construction signs and train ledge | Todo | D17B, D18A |
| D18D | Music silent after death and Continue until Duke's next voice line | Todo | D01 |
| D19 | Modern in-game menus, settings and input prompts (Sonic 3 A.I.R.-style customization; plan mode + artifact first) | Todo | D02, D04, D13 |
| D19A | Duke font assets for host messages and modern UI | Done | D04 |
| D19B | Responsive modern menu navigation and transitions | Todo | D02, D04 |
| D20 | Save management and optional quick saves | Todo | D01, D02 |
| D21 | Accessibility and sound controls | Todo | D04, D19 |
| D22 | Campaign fidelity and overlay coverage | Todo | D01 |
| D22A | Portal transition loses Modernized controls and first person (slot 7) | Accepted | D05, D06, D11 |
| D22B | Modern controls and first person in every level and transition (no classic fallback) | Accepted | D22A, D26A |
| D22C | Level 11 starts with Legacy controls until F10 (control mode must persist) | Accepted | D22B |
| D23 | Performance budgets and long-session stability | Todo | D01 |
| D23A | Modernized frame-budget regression (guard identity cost) | Done | D08 |
| D23B | Intro FMV stutter: stranded native movie shard | Done | D23 |
| D23C | Medieval castle moat slowdown with Necros active (profile, do not optimise blind) | Todo | D23, D22B |
| D23D | Minor slowdown around the strip-club-type area of Level 9 (low priority) | Todo | D23, D22B |
| D23E | Stutter with enemies on screen while walking (UI slots 9 and 10) | Accepted | D23, D22B |
| D23F | Faster timing model: emulation-thread budget for 150% CPU at high refresh (big) | Accepted | D23E, D17 |
| D23G | Finish the fast path: dispatch, overlays, interpreter, observers and redraw cost (all-in-one) | Todo | D23F |
| D23H | Presents fall from 120 to about 60 over extended play (savestate hitches, sticky shedding) | Accepted | D23E, D23F |
| D24 | Linux / Windows player build and disc import | Todo | D19, D22, D23 |
| D24A | Public clone gives the full experience: fonts, inventory icons/digits from a proper tracked or disc-derived source (no research/ dependency) | Accepted | D19A, D08A3 |
| D24B | Savestate menu (F7) dressed in the TTK fonts and disc art | Accepted | D24A |
| D24C | TTK-font `!` drawn from I and period; console prompt `>` instead of `]` | Done (user-accepted) | D24A |
| D25 | Modernized edition release acceptance | Todo | D08, D08A, D08B, D09, D10, D14, D17, D18, D20, D21, D24 |
| D26 | Backtick debug console (fps and helpers) | Done | D04 |
| D26A | Debug level-select panel for whole-game testing | Accepted | D26, D22A |
| D26E | Debug `spawn <item>` console command (EDuke32-style) | Accepted | D26A |
| D26F | Spawning in-game items, continued: level crystals as custom pickups (`spawn 1761/2761/3761`) | Done | D26E, D08A5 |
| D26B | Opening the console leaves first person | Todo | D26, D11 |
| D26C | Console command history (Up/Down) | Accepted | D26 |
| D26D | Level select: authoritative order, numbering, names and categories | Todo | D26A |
| D27 | Caps Lock RUN MODE quotes; Shift-run clunk silence deferred | Done (quotes); clunk deferred low-priority | D04, D19A |
| D27A | Modern Shift/run is silent: remove the walk/run toggle click `0x0001` from the Shift path, keep the D08A10 heartbeat | Done (user-accepted) | D27, D08A10 |
| D28 | Scroll Lock holster and WEAPON LOWERED/RAISED quotes | Done | D04, D19A |
| D29 | Progression items and objectives legibility: research and design first (Level 2 bank-vault notes) | Todo | D19A, D22B, D26A |
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

### D08A4 - EDuke32-style portable steroids (pick up, store, use with R)

**Done (user-accepted, 2026-10-07):** "mechanically, the steroids work
perfectly. you pick them up, you can press r to run, and thats it ... i accept
this job as complete now as it's functional". The countdown display is
follow-up D08A8. See [note 127](documentation/127-d08a4-portable-steroids.md).

User request, 2026-10-06. "Eduke style steroids, where you actually
pick up the roids as an item, and it appears in our items list. it is invoked
with the R key." In TTK steroids activate on pickup and there is no stored
dose (D08A recorded that R had nothing to use; D08A3 extracted the steroids
icon but kept it out of the switcher).

**Scope (Modernized, optional rule change):** a steroids pickup is stored as an
inventory item instead of activating; it appears in the D08A1 switcher with
the original TTK steroids icon (D08A3) and its count; **R** (and selecting it
in the switcher and using it) starts the original steroids effect and uses
one dose. Research first: the original pickup handler and the steroids
effect/timer, whether TTK keeps any spare inventory field a dose count can
live in or the host must hold it (and how that survives savestates, memory
card saves, death and Continue, and level changes), what happens on pickup
while already full, and whether `dnhyper` should give a stored dose or
activate. Because this changes an original rule, make it a Modernized setting
for the future menu (D19); Vanilla keeps activate-on-pickup.

**Acceptance:** in Modernized, picking up steroids in at least two levels
stores a dose shown in the switcher; R activates the original effect and
consumes it; R with none does nothing harmful; the dose persists through a
save/load and a level change; Vanilla unchanged; the user confirms.

**Work log (2026-10-07, Needs playtest).** Research: the original still has a
held steroids item (item 4, `+0x364` bit 0 with the amount `+0x366`; the
inventory grant `0x8003d738` gives it, the Select menu's toggle `0x80089494`
would run it, the level-end snapshot `0x80083348` and the card save keep it).
Only the pickup case `0x800827e8` skips it by setting bit 1 (running). EDuke32
source and Duke 3D `GAME.CON`/`USER.CON` checked: one held at a time, R only
when held, quote 12 `USED STEROIDS`, `dnhyper` starts them.
Implementation (`recomp/src/ttk/steroids.inc`): in Modernized with `steroids`
`portable` (default), a pickup is held instead of run, settled at the pickup
tail's own sound call (new entry hook `0x8006B73C`, one regenerated line,
savestates still load); held: more steroids stay on the ground; running: the
original refresh. R (and Enter/U on steroids) set bit 1 and play the pickup
sound. Switcher order medkit, steroids, jetpack, Bio Mask, goggles; HUD box with
the user's `hud-steroids.png` (15-colour cell from the new
`reduce_icon_15col.py`). Profile schema 29 (`--steroids portable|original`,
`--settings` S). A first design kept a dose count in flag bits 8-11 and was
replaced when a write trace showed the level-end snapshot (and so the card
save) keeps only bit 0.
Evidence (private copies of the user's cards and savestates, level 0 and level
6): pickup held (flags 1, 9000) with the original message; second pickup left;
R runs the effect, `USED STEROIDS`, switcher drains with the active mark;
R running/none refused; refresh while running; savestate reload, level
completion with the stats-screen save, card load (pause, Load) after using it,
and death with Continue all keep it; `original` and Vanilla keep the original
rule. Native and Python suites pass (two new D08A4 native groups).
Limits: needs the user's playtest; spawned pickups (`spawn steroids`, the real
type 638 and pickup code) rather than natural placements; `level N` travel
clears every gadget (original Restart reset); the original Select inventory
does not list held steroids.


### D08A8 - Steroids countdown in the steroids HUD box

**Done (user-accepted, 2026-10-07):** "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." See the work log below and
[note 127](documentation/127-d08a4-portable-steroids.md) (D08A8 section).

User request, 2026-10-07 (after accepting D08A4; video
`research/screencaps/Video_2026-10-07_22-03-53.mp4`, reference
`research/screencaps/ttk-roids.png`): "what genuinely doesnt make sense to me is
why the armor icon is used when the coundtown for the steroids is displayed.
I'd really love if that entire countdown could be delegated to the steroids
thing you see there in the screenshot."

While steroids run, the original status bar (`0x8008bd94`) shows their
remaining percent in the armor element with the armor icon. In Modernized,
show the whole countdown in the D08A6 gadget box with the pill-bottle icon
(the box in `ttk-roids.png`, top right) and leave the armor element to armor.
Research first: how `0x8008bcf4..0x8008be00` chooses between armor and
steroids for that element, and what the box should show when steroids run
while another gadget is selected (stack it like the running jetpack, or switch
the box to steroids).

**Acceptance:** in Modernized, running steroids count down in the steroids box
with the pill icon; the armor element shows only armor (or stays hidden as the
original does without armor); held, running and refreshed steroids, a selected
other gadget, savestates and both renderers checked; Vanilla unchanged; the
user confirms.

**Work log (2026-10-07, Needs playtest).** Research: element 2 of the status
bar (`0x8008bcf4..0x8008bf00`) is shown while `+0x364` bit 1 (steroids running)
is on or armour `+0x234` > 0, and draws steroids' percent (`amount * 100 /
9000`) instead of armour (`armour / 100`, at least 1) whenever bit 1 is on, always
with the armour icon `0x800c44f4`. Nothing else in the status bar reads bit 1.
Decision on the open question: running steroids stack like a jetpack that is on.
Implementation: running steroids stay selected (`usable_item(4)` is now held
or running, so after R the box you were looking at counts down in place; R /
Enter still only take a held one). `gadget_hud.inc` draws them lit with the pill
icon in the slot when selected, else stacked above it (after an unselected
jetpack that is on). For the status bar draw only, bit 1 is turned off so
element 2 is the plain armour element; it is turned back on at the first hook
after the status bar returns (`0x8001fc44`, or new lightweight entry hook
`0x8002E850` on the composition path that skips it; also at the next status bar
entry and `0x800b4d9c`). One regenerated line; codegen hash unchanged. Only with
`steroids` `portable`; `original` and Vanilla unchanged.
Evidence (executable `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`; private Xvfb runs on a fresh copy of the
player's cards and savestates, `recomp/analysis/d08a8-steroids-hud/`, local):
held 100 then R: lit pill box counting down in place, no armour element with
armour 0, drain and effect unaffected (timer falls normally, ends at 0 and the
box goes); `]` to the jetpack (off): steroids one row up; jetpack on and
selected: same; Bio Mask selected with jetpack on: jetpack row 1, steroids row 2;
armour 50 while running: the armour element shows 50 with the armour icon,
steroids stay in their box; savestate save/reload while running; R while running
does nothing; after the effect the armour element works as before. GL 4:3, GL
16:9 at 120 fps (replay workers), Software 4:3 and 16:9 look the same. Vanilla
with steroids forced on still shows them in the armour element. Suites:
`ttk-controls-test` (LEVEL00 fixture `d08-camera-final`, LEVEL01, all levels;
40 groups), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK (2 skipped),
`level_overlay_guards.py --check`.
Limits: needs the user's look in play; death while steroids run and a
natural (non-spawned) pickup while running were not exercised; with
`steroids` `original` in Modernized the original armour-element countdown stays
(there is no steroids box there).

### D08A9 - Picked-up inventory item becomes the switcher selection

**Done (user-accepted 2026-10-08: "fully accepted, working beautifully").**
Executable `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`
is the regression baseline.

**User request, 2026-10-07** (on accepting D08A8): "when an inventory
item is picked up, that item should be the one selected in the switcher. i.e.
you pick up biomask, then that should be the selected item. you pick up
jetpack, that should be the selected item etc. that's how it worked in duke3d
and i want that feel to exist here."

**Scope (Modernized only; Vanilla unchanged):** when Duke picks up a gadget
(jetpack, Bio Mask, goggles, medkit, portable steroids), the `[ / ]` selection
(`selected_item` and the original menu ID `0x800c3f94`, which the D08A6 HUD box
reads) moves to it at once, so the HUD box and the next Enter / U use it.
Mission items and keys never change the gadget selection (D08A5 owns them).

**Research first:** the pickup dispatcher `0x80081a48` cases that grant each
gadget (D08A4 found steroids at `0x800827e8`) and whether a refill of an
owned gadget, a full one left on the ground and cheat/inventory grants
(`0x8003d738`) should count; how EDuke32 does it (`P_AddInventory` / the
`addinventory` CON command set `inven_icon` on every inventory pickup) and
whether a refill there also selects; whether a pickup during a gadget's
activation (`+0x8000` pending), in flight, or while the switcher strip is open
should wait or apply; that the D17 replay workers see it (guest ID, not host
state).

**Acceptance:** in Modernized, picking up each gadget type selects it in the
switcher and the HUD box immediately, in at least two levels; a held steroids
pickup selects steroids; mission items and keys leave the selection alone;
Enter / U then uses the picked-up item; savestate, level change and Continue
keep the selection consistent; Vanilla unchanged; the user confirms the feel.

**Work log 2026-10-07 - built (Needs playtest).** Executable
`c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`.
[Note 128](documentation/128-d08a9-pickup-selection.md).

- Research: every gadget pickup goes through the dispatcher `0x80081a48`; the
  jetpack, Bio Mask, goggles and medkit cases take the pickup only while the
  amount is below capacity, then set bit 0 and the full amount (refills
  included, as EDuke32's `P_AddInventory` sets `inven_icon` on refills). Only
  multiplayer starts an activation there. Keys and mission items never touch
  items 1-5; the `dninventory` grant `0x8003d738` bypasses the dispatcher.
- Implementation: `recomp/src/ttk/pickup_select.inc`. Items 1-5 are compared
  around the dispatcher call (entry, and its caller's next call `0x8001ca4c`
  with ra `0x8007fe78`, both existing hooks); a gadget that became owned or
  gained amount becomes the selection through `remember_item` (guest menu ID
  `0x800c3f94`, so the D08A6 box and D17 replay workers follow). While any
  gadget is mid-activation (`+0x8000`) it waits for the next item poll.
  Savestate loads in between cancel it. No strip pop-up, no new hooks, no
  codegen change.
- Evidence: private Xvfb runs (`recomp/analysis/d08a9-pickup-select/`):
  level 0 (60 fps) and level 6 (120 fps) - new jetpack/Bio Mask/goggles/medkit/
  steroids each selected with their HUD box; refills of Bio Mask, medkit,
  goggles and jetpack selected; full goggles/medkit stay on the ground with
  the selection unchanged; keys leave it; `dninventory` leaves it; Enter after
  a goggles / jetpack pickup switches it on; steroids picked up while running
  refresh and select; a pickup during a (test-forced) activation waited and
  applied when it cleared; savestate reload keeps the selection. Native
  `ttk-controls-test` (new D08A9 group), `ttk-input-test`,
  `ttk-inventory-test`, Python 131 OK, `level_overlay_guards.py --check`,
  `check_repo.py`.
- Limits: pickups were spawned (`spawn`, same types and original code), not
  natural placements; death/Continue was not reached in the scripted runs
  (writing health 0 did not kill Duke); Vanilla gating verified by code path
  and counters only (`spawn` is Modernized-only); a real jetpack-transition
  pickup was simulated with the pending bit.

### D08A10 - Experimental: steroids heartbeat sound loop

**Done on user acceptance (2026-10-08): "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!"** Executable
`146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca` is the regression baseline.

225 bpm on the steroids timer, with the exact
sound Shift makes in Modernized (the walk/run toggle click `0x0001` through
`0x8006bbd8`). User on the first build: "the beat is right but the sound is
wrong" (it used `0x1012`; the footstep was the wrong guess for the Shift sound).
See the work log below and [note 129](documentation/129-d08a10-steroids-heartbeat.md).

**Experimental; user request, 2026-10-08.** The user expects it to be
simple but may not like the result, so it may be reverted: keep it small,
isolated and easy to remove (or behind a profile option), and get the user's
verdict before building on it.

"In duke3d, when taking the steroids, a sound plays of duke swallowing the
pills over and over for the duration of the steroid use. the rhythm is
226bpm. ... we could use the sound that duke makes in ttk when you hit shift
... it sounds good enough like a heartbeat. unless of course there is an
actual heartbeat sound available like this."

**Reference:** `research/screencaps/Video_2026-10-07_23-59-03.mp4` (local,
Duke 3D, the latest video when the job was opened): listen for the repeating
steroids sound and confirm the rhythm (user: 226 bpm, about 265 ms per beat).

**Scope (Modernized, portable steroids only; Vanilla unchanged):**

- While steroids run (`+0x364` bit 1, D08A4/D08A8), play a short sound at
  Duke on a steady 226 bpm rhythm; stop at once when the effect ends
  (timer out, damage cut `0x800a4154`, death, level change, Continue,
  savestate load).
- Sound source, in order of preference: a real heartbeat-like sound already in
  the game's own sound banks, if one exists; otherwise the sound Duke makes
  when Shift is pressed to run (the walk/run gait plant sound noted under
  D27). Use the original's own sound call (`0x8006b73c`, as the D08A4 use
  sound does) so no external audio is added. Identify the exact sound ID.
- Time the beats on the game clock, not the host frame rate, so 60/120 fps and
  the D17 replay workers do not change the rhythm or double the sound.
- Easy to revert: one `.inc` and its hook-ups, optionally a `steroids_sound`
  profile choice (on/off).

**Research first:** list candidate sounds (sound bank IDs played by the Shift
gait restart, heartbeat or pulse sounds in any level bank) and how the sound
call picks a bank per level; whether a looping voice would starve Duke's
voice lines or the music (D18B, D18C); the Duke 3D timing from the video.

**Acceptance:** in Modernized, R starts the beat at about 226 bpm for the full
steroids duration in at least two levels, at 60 and 120 fps; it stops when
the effect ends or is cut, and on death, savestate load and level change;
other sounds and voices are not cut off; Vanilla and `steroids` `original`
unchanged; the user decides to keep, change or revert it.

**Work log (2026-10-08, Needs playtest).**

- Reference: the video's steroids sound runs 13.5 s, median 268 ms between
  beats (224 bpm): Duke 3D's `DUKE_HARTBEAT` every 8 tics at 30 Hz, 225 bpm.
- Sounds: walking and Shift play only the footstep `0x2000`/`0x2001`
  (`0x80048378`). No named heartbeat exists; a debug `sfx <id>` survey decoded
  each sound's sample from SPU RAM. Bank 1 (`0x1000`-`0x101d`, Duke's own, in
  every level tried) has `0x1012`, a 313 ms low double thump (95% of its energy
  under 250 Hz), chosen as the default. 225 bpm previews of it, the footstep,
  `0x1000` and `0x101c`, plus the Duke 3D reference, are in
  `recomp/analysis/d08a10-beat/preview/` (local).
- Implementation: `recomp/src/ttk/steroids_beat.inc`, checked from the
  authenticated player update (existing hook `0x8005a210`/`0x80041b34`): one
  beat each 80 timer units from the dose start (300 units/s, 266.7 ms), first
  beat at once, through the original sound call `0x8006b73c` on a private
  stack. Portable steroids only, Duke alive; savestate loads resync. No new
  hooks or codegen change. `DNTTK_STEROID_BEAT=off|<id>`; debug
  `controls.steroid_beat` with a 32-entry sound-call log; console `sfx <id>`.
- Evidence (executable
  `d0d1f09355d69f4ea3ff1a1a6ea7737bf261f512d83443b3d0f3edf20c861acf`, private
  Xvfb runs): identical beat intervals at 60 and 120 fps (15, 15, 18 fields =
  250, 250, 300 ms, mean 225 bpm); pickup + R beats; savestate load without a
  burst; damage cuts continue the beat; `level 6` travel stops it; level 6
  beats from the same sample; `original` and `off` silent. Suites:
  `ttk-controls-test`, `ttk-input-test`, `ttk-inventory-test`, Python 131 OK,
  `level_overlay_guards.py --check`, `check_repo.py`.
- Limits: in 20 Hz scenes beats land on 50 ms steps (slightly uneven
  250/250/300 ms; exact 266.7 ms at 30 Hz); death during steroids and Vanilla
  verified by code path only; no profile option (environment variable only)
  until the user decides to keep it.

**Work log (2026-10-08, sound fix, Needs playtest).** User: "the beat is right
but the sound is wrong. i want the sound specifically when you press shift on
the keyboard on modern controls. you used the footstep sound." SPU KEYONs
around Shift presses (standing and walking) show sample `0x012F0` at pitch
`0x228`-`0x22F` on press and release: sound `0x0001`, the walk/run toggle
click, played by the player update through the non-positional call
`0x8006bbd8(1)`, not the `0x8006b73c` call the first survey logged. The beat
now makes that exact call. Same sample and pitch spread as a real Shift press
in levels 0 and 6; rhythm unchanged (15, 15, 18 fields). Suites pass.
Executable `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`.
Preview `recomp/analysis/d08a10-beat/preview/beat-shift.wav` (local).

### D08A11 - `dnhyper` countdown in the steroids HUD box

**Done (user-accepted 2026-10-08: "this job is fully accepted").** Executable
`2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892` is the
regression baseline. Work log below.

**User request, 2026-10-08:** "dnhyper uses the old ui element with the
armor rather than our new ui element with the steroids to display the
countdown, we need to make dnhyper use the new dedicated countdown on the
right hand side."

With portable steroids (Modernized), a dose taken with R counts down in the
steroids HUD box on the right (D08A8), and the original armor element shows
only armor. After `dnhyper`, the countdown still shows in the original armor
element with the armor icon.

**Likely cause (to verify first):** `dnhyper` sets `player+0x364` bit 1
(running) without bit 0 (owned). D08A8's status-bar hide and the HUD box use
`steroids_running()` / `steroids_owned()` (`steroids.inc`), which require
bit 0, so the original status bar still sees bit 1 and draws element 2 with
the armor icon, and the steroids box does not show. A natural pickup while
steroids run and other paths that set bit 1 alone may behave the same.

**Scope:** Modernized with `steroids` = `portable`. Any running steroids,
including `dnhyper`, count down in the steroids box (and keep the switcher's
active mark and the D08A10 heartbeat) while the armor element shows only
armor; when the effect ends, nothing is left held that was not held before
(a `dnhyper` dose must not become a stored item). Decide whether to give
`dnhyper` the owned bit while it runs (as a used dose has) or widen the HUD
predicates; keep `original` and Vanilla unchanged.

**Acceptance:** after `dnhyper` the countdown shows only in the steroids box,
draining and lit, with the armor element showing armor (or hidden with no
armor); it ends cleanly with no held steroids left behind; R-used doses,
pickups, savestates and the heartbeat behave as before; at 60 and 120 fps;
`original` and Vanilla unchanged.

**Work log (2026-10-08, Needs playtest).** Cause confirmed: `dnhyper`
(`cheats.inc`) sets `+0x364` bit 1 only, and `steroids_owned()` /
`steroids_running()` required bit 0, so the D08A8 status-bar hide never ran and
the box never drew. Decision: widen the predicates rather than give `dnhyper`
an owned bit. Running is now bit 1 with an amount, with or without bit 0, so
any path that runs steroids without the owned bit is covered; the drain
`0x800414a0` clears bits 0-1 at 0, so nothing is left held. Changes
(`steroids.inc`): `steroids_owned()` accepts bit 0 or bit 1; the HUD restore no
longer needs bit 0 to put bit 1 back, and skips the restore if a savestate
loaded between hide and restore (it keeps the loaded flags) or the amount is 0.
`pickup_select.inc`: for steroids a pickup that refreshes a `dnhyper` run counts
as a refill (selects steroids), as for a used dose. `steroid_doses()` is
unchanged, so R / Enter still take only a held dose. No new hooks, no codegen
change. Evidence (executable `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892`; private Xvfb runs on a copy of the
D08A8 cards, `recomp/analysis/d08a11-dnhyper/`, local), at 60 and 120 fps:
`dnhyper` with armour 0 - flags `0x2`, the lit pill box counts down stacked
above the selected jetpack box, no armour element; steroids selected - counts
down in place; R while it runs does nothing; armour 50 - the armour element
shows 50 with the armour icon, steroids stay in the box; savestate save and
reload while running; the effect ends with flags 0, no held dose, the box
gone; a steroids pickup while `dnhyper` runs refreshes, selects steroids and
ends with nothing held; a held pickup then R still counts down in the box.
`DNTTK_STEROIDS=original`: `dnhyper` keeps the original armour-element
countdown (no hides). Suites: `ttk-controls-test` (`d08-camera-final` fixture,
LEVEL01, all levels), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK
(2 skipped), `level_overlay_guards.py --check`, `check_repo.py`.
Limits: the heartbeat already treated bit 1 alone as running (unchanged, not
re-listened); `dnhyper` while a dose is held runs that dose and it is used up
when the effect ends (the original keeps one amount per item); Vanilla verified
by code path only (all changes are behind `portable_steroids()`); software
renderer not re-shot (the draw path is D08A8's, unchanged).

### D08A12 - Modern dynamite handling / safe weapon selection

**Todo. User request, 2026-10-08.** Investigate first; do not implement until
selected.

**Problem.** Dynamite (slot 6: Pipe Bomb, Dynamite, Holy Hand Grenade) is
reached by pressing 6 twice or with the mouse wheel. Once Duke holds it the
fuse is effectively committed: if it is not thrown it explodes in his hand. So
selecting the weapon commits the player to using it; they cannot scroll to it,
look, change their mind and switch back. The original pad had no rapid weapon
scrolling, so the original rule is far more intrusive under our wheel and
number-key selection. Design principle (user): **selecting a weapon is not the
same thing as firing a weapon.**

**What we already know (do not rediscover):**
- D08A ([note 33](documentation/33-controls-shortcuts.md)): lit dynamite is an
  original exception: `0x8004dea4` / `0x8004e018` run its fuse until thrown or
  detonated, and a requested next weapon waits for that original completion.
  Shortcuts neither stow it artificially nor throw it automatically.
- D08Q5 ([note 108](documentation/108-d08q5-jetpack-weapon-switch.md)): drawn
  dynamite runs its original fuse; a switch away waits for it, also in jetpack
  flight. Grenade and dynamite share the original throw handler with their own
  gravity and fuse ([note 40](documentation/40-feedback-implementation.md)).
- Weapon groups and wheel order: [note 32](documentation/32-eduke32-weapon-item-plan.md),
  [note 33](documentation/33-controls-shortcuts.md).

**Research first.**
1. The original state machine: is there a distinction such as equipped/unlit
   -> fuse lit -> throw -> explosion, or does the draw itself start the timer?
   Where the fuse value lives, what starts and advances it, and what the
   in-hand detonation path is.
2. Whether equip and ignition can be separated cleanly. Ideal (if not
   invasive): select = equip safely; primary fire = light/arm; release or a
   second action = throw; once armed, normal fuse rules apply. Do not add
   complexity purely for realism.
3. What switching away from an equipped or armed dynamite does to weapon state
   today (the pending switch, ammo, animation, the D08A history).
4. The smallest robust Modern implementation. Acceptable fallback (user):
   while held, the fuse never runs out (the fuse animation/sound may keep
   sizzling indefinitely), and switching away is allowed without using a
   stick.

**Option.** Dynamite Behaviour: **Modern** (default in the Modernized
profile) / **Original** (exact PlayStation behaviour, fuse can explode in
Duke's hand). Integrate with the existing profile/settings architecture (as
`steroids` `portable` / `original` does in the Modernized controls profile),
not a one-off config. Vanilla stays original.

**Do not change** explosion damage, blast radius, ammo capacity, throw
physics, enemy damage or environmental interactions unless the arming state
genuinely requires it.

**Acceptance.**
1. The original equip/arm/fuse/throw state machine is documented.
2. Whether equip and ignition separate cleanly is answered.
3. Consequences of switching away from equipped or armed dynamite are known.
4. Modern behaviour is the smallest robust change.
5. Original behaviour stays available and unchanged (and Vanilla).
6. Mouse-wheel selection passes through dynamite without consuming it or
   hurting Duke: e.g. shotgun -> wheel -> dynamite -> wheel -> another weapon,
   rapidly.
7. Selecting dynamite and then changing your mind is safe in Modern.
8. Throwing still behaves normally once the player chooses to use it.
9. Savestates and weapon state stay sane with dynamite equipped or armed.
10. The setting integrates cleanly with the existing modernization options.

### D08A13 - Steroid duration independent of damage and armor

**Done (user-accepted 2026-10-08: "i accept that this works").** See the
work log entries of the same date and
[note 127](documentation/127-d08a4-portable-steroids.md#d08a13---steroids-independent-of-damage).
User request, 2026-10-08.

**Problem.** With steroids running, taking damage shortens the remaining
steroid duration: activate steroids, watch the countdown in the steroids HUD
box (D08A8), let an enemy shoot Duke, and the countdown drops with each hit.
The user wants none of this. Design rule (user): **steroids and armor are
completely independent systems.** Steroids are a temporary performance
enhancement with their own timer; armor is protection from incoming damage.
Damage must never consume steroid duration, and steroids must never change
armor. The steroids box (D08A8, D08A11) is now the authoritative player-facing
countdown, so steroids no longer need the armor element for a number.

**What we already know (verify, do not assume):**
[note 127](documentation/127-d08a4-portable-steroids.md) records, from the
original code, item 4 (steroids) as player `+0x364` flags (bit 0 held, bit 1
running) with its own amount/timer at `+0x366` (full 9000, `0x800c2722`);
armor is a separate field `+0x234`. The drain `0x800414a0` lowers `+0x366` by
the frame step while bit 1 is on and clears bits 0-1 at 0. A **damage cut
`0x800a4154` takes 1500 from a running amount and can end the effect**. So
the steroid duration is probably not stored in armor at all: the original
game itself charges steroid time for each hit, and the old status bar showed
that countdown in the armor element with the armor icon, which made the two
look like one resource. This is a lead to confirm, not a finding.

**Investigate first and document** (in note 127 or a new note):
1. The original pickup/activation path, the drain, and exactly what
   `0x800a4154` does: which damage routine(s) call it, for which damage types
   (bullets, explosions, falls, melee, drowning), whether it runs before or
   after armor absorbs damage, whether it depends on armor at all, and
   whether 1500 is a constant or scaled by damage.
2. Whether any other path ties `+0x366` / bit 1 to armor `+0x234` (armor
   pickups, the status bar, the level-end snapshot `0x80083348`, card save and
   load, savestates).
3. Whether the coupling is original (Vanilla does it) or something our
   modernization added or exposed (D08A4 portable doses, D08A8 HUD hide,
   D08A11 `dnhyper` predicates, D08A10 heartbeat). Confirm in Vanilla /
   `steroids` `original` as well as Modernized `portable`.
4. Whether the effect ends early on a hit today and what clears with it
   (bits 0-1, the HUD box, the heartbeat).

**Scope.** Modernized with `steroids` `portable`: the steroid timer is only
lowered by its own drain; incoming damage never touches `+0x366` or the
running bit; armor and health take damage by the existing rules. Prefer the
smallest clean change (e.g. skipping the damage cut for steroids in
Modernized) over new duplicate state, since the steroid timer already exists
on its own. Vanilla and `steroids` `original` keep the original rule; if the
research shows a strict original mode needs the coupling, record that before
changing anything global. Whether this is its own setting or part of
`portable` is a decision to report, not to invent silently.

**Preserve:** portable doses and R (D08A4), the steroids box countdown and
row stacking (D08A8), `dnhyper` in the box (D08A11), the heartbeat and its
start/stop (D08A10, D27A), the steroid kick and other effects, activation and
expiry, pickup selection (D08A9).

**Acceptance.**
1. The coupling is traced and documented (original or ours, and why).
2. Steroids + incoming damage: activate, note the remaining amount, get shot
   repeatedly; armor and health behave normally and the steroid amount is
   exactly what it would have been unhurt (compare against the drain rate,
   frame-step aware, at 60 and 120 fps).
3. Steroids with armor: the two values change independently.
4. Steroids with zero armor: steroids work normally.
5. Armor without steroids: unchanged.
6. Natural expiry: the box reaches zero, the effect and heartbeat end.
7. Save/load and savestate while steroids run: the steroid state is correct
   and not reconstructed from armor.
8. `dnhyper` and an R dose both behave as above.
9. Vanilla and `steroids` `original` unchanged.
10. User check: "activate steroids, stand in front of an enemy and get shot;
    the steroid countdown does not react."

### D08A14 - Death and Continue end steroids

**Done (user-accepted 2026-10-08: "fully accepted").** Death clears running and held steroids in
Modernized `portable`; the user chose that a held dose is lost too. See the
work log entry of the same date and
[note 127](documentation/127-d08a4-portable-steroids.md#d08a14---death-ends-steroids).
User request, 2026-10-08.

**Problem (user):** "if you are using steroids and you die, and use a
continue, you should not still be using steroids and it should be gone from
the inventory." Since D08A13, Duke can die while steroids run (the original
shield made that nearly impossible). Observed in the D08A13 runs: the timer
pauses while Duke is dead (flags kept, `dnhyper` 0x2 and an R dose 0x3), and
after Continue (Cross) the effect resumes with the time left and drains.

**Lead (verify):** TTK's Continue keeps items (D08A4: a held dose survives
death and Continue; D08Q2 handles the jetpack after death and Continue). The
running effect is `+0x364` bit 1 with the amount `+0x366`; the held item is
bit 0. The level-start reset `0x8003fd98` / `0x8003fe10` zeroes items only in
some cases. Find the death/Continue path (D08Q2's notes and `deathroute.py`)
and clear item 4 there in Modernized `portable`.

**Scope and decision to confirm when selected:** steroids running at death
end and are removed (bits 0-1 and the amount cleared): no countdown, no HUD
box, no heartbeat after Continue, nothing in the switcher. Whether a held,
unused dose is also lost at death (Duke 3D loses inventory on death) is a
question for the user, not to be decided silently. Vanilla and `steroids`
`original` unchanged.

**Acceptance.**
1. `dnhyper`, die, Continue: no steroids running, no box, no heartbeat,
   nothing in `[ / ]`.
2. R dose, die, Continue: the same; the dose is gone.
3. Held dose decision as agreed with the user.
4. Savestate taken while dead and reloaded, then Continue: the same.
5. Card save/load and level completion with steroids unchanged (D08A4).
6. Vanilla and `original` unchanged.

### D08A15 - Heartbeat silent when steroids restart mid-run

**Done (user-accepted 2026-10-08: "fully accepted").** Cause found and fixed: a refresh to the full
amount never read above the run's first reading, so the rhythm did not
restart. **Corrected by the user (2026-10-08) and folded in:** a pickup
mid-run now stops the run and leaves a full held dose. See the work log
entries of the same date and
[note 129](documentation/129-d08a10-steroids-heartbeat.md#d08a15---restart-mid-run-beats-at-once).
User request, 2026-10-08.

**Correction (user, 2026-10-08, from a Duke 3D playthrough, "that familiar
feel"):** while steroids run,

| Action | Result |
| --- | --- |
| `dnhyper` | refills to full **and keeps running** (heartbeat restarts at once) |
| `dnstuff`, `dnitems`, `dninventory` | stop the run; a full dose is held (D08G4) |
| Picking up steroids | stops the run; a full dose is held |

The struck-through parts below assumed the original refresh (a pickup
mid-run refills and keeps running); that is no longer the design.

**Problem (user):** "if starting steroids before a previous cycle of
steroids is over, the sound doesnt play for a few seconds at the beginning."

**Lead (verify):** R refuses while steroids run, so a restart mid-run is
~~a pickup that refreshes them (the original refresh, D08A4),~~ `dnhyper`
again, or using a new dose just as the old one ends. `steroids_beat` (D08A10,
`steroids_beat.inc`) restarts the rhythm when the amount rises above the
starting amount and plays the first beat at once; check whether that beat is
refused (the sound routine refuses an id already playing, `0x8006b7b0`, as
found in D12C), whether `beat_start`/`beat_bucket` miss a refresh (a refresh
to 9000 from a high amount, or a refresh in the same update as the drain),
and whether the D08A13 damage hide or the D08A8 HUD hide hides bit 1 at the
beat check. Reproduce first and record which case it is.

**Acceptance.** A pickup mid-run stops the run and leaves a full held dose
(no countdown, no heartbeat; R starts it fresh). A restart mid-run
(~~pickup refresh,~~ `dnhyper`, a dose taken right at the end) beats at once and keeps the normal rhythm from the new
start; a fresh start, natural expiry and the D27A silent Shift are unchanged;
60 and 120 fps.

### D08A16 - Power-up coin icons (human design ticket)

**Todo. User request, 2026-10-08.** Backlog only; do not start until
selected. The **user designs the art**; the agent's part is the placeholders
and the plumbing to drop the finished icons in.

**Background (user):** TTK has three special items, all Duke nuke-symbol
coins that start on pickup and run an invisible timed effect: **invincibility**
(spawn name `invulnerability`, type 1047), **invisibility** (1045) and
**Double Duke** (1046) (type numbers from the D26E spawn table,
[note 111](documentation/111-d26e-debug-spawn.md)). They are TTK's analogs of
Quake's Pentagram of Protection, Ring of Shadows and Quad Damage. D08A17 gives
them real countdowns; first they need icons in the same style as the
inventory icons (D24A, the user's steroids pill icon, D08A4/D08A7).

**When the ticket begins:** add a **placeholder section** for the three icons
to the local font/UI picker page `recomp/analysis/d24a-fonts/ttk-font-picker.html`
(local, not tracked), styled like its existing item-icon sections: one
placeholder per item, labelled with its name, the Quake analog and the type
number, at the HUD box size (16x16 in a 4bpp cell, as the steroids box) with a
larger preview. No switcher-strip variant: power-ups are never held, and if
they ever were, the switcher would use the same icon (user, 2026-10-08).
Then the user draws the icons.

**Done (user-accepted, 2026-10-08).** The user's three icons are in
`recomp/assets/ui/items/` with their 15-colour copies; see the work log.

**Done when:** the user has delivered the three icons (PNG sources in
`recomp/assets/ui/items/`, as `hud-steroids.png`), each reduced to 15 colours
plus transparent with `tools/local/reduce_icon_15col.py`, and accepted them in
the picker. No gameplay change in this ticket.

### D08A17 - Power-up countdowns (invincibility, invisibility, Double Duke)

**Done (user-accepted 2026-10-08: "confirmed it's all working as intended").**
Executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline. User request,
2026-10-08. See the work log below and [note 132](documentation/132-d08a17-powerup-countdowns.md).

**Goal:** a real, visible countdown for each running power-up, like the
steroids box (D08A8, D08A11): when a coin is picked up its effect starts at
once (as now, original behavior), and a HUD box with the item's icon (D08A16)
counts down to zero, the way Quake shows a running power-up. Modernized only;
Vanilla unchanged.

**Investigate first (verify, do not assume):**
1. Where each effect keeps its timer and flags: player fields or globals, the
   full duration, the drain (per frame step, like the steroids drain
   `0x800414a0`?), what ends it and what it clears; the pickup cases for 1045,
   1046 and 1047 in the dispatcher `0x80081a48`.
2. Whether a second coin refreshes, stacks or is refused; what death,
   Continue, level completion, card save/load and savestates do to a running
   effect; what `dnkroz` god mode shares with invincibility.
3. Any original on-screen sign of the effects (tint, sound, status bar) to
   keep.

**Scope (to confirm with the user when selected):** one box per running
power-up, stacked with the steroids box and the selected-gadget box (D08A8
row rules), showing a percent or seconds as the user prefers; consistent
with D08A14 (whether death ends them) and the D08A13 independence rule
(power-ups do not touch armor or steroids). Whether they also get a sound
cue (like the D08A10 heartbeat) or an expiry warning is the user's call.

**Acceptance (draft).** Each coin: pickup starts the effect and its box
counts down at the drain rate to zero, then the box goes with the effect; two
or three at once stack cleanly with steroids and the selected gadget;
savestate, level change and death behave as agreed; 60 and 120 fps; GL and
Software renderers; Vanilla unchanged.

**Decisions (user, 2026-10-08, when selected):** percent like steroids; the
original's 5 s invincibility after Continue shows a box; no expiry warning.

**Work log (2026-10-08, Needs playtest).**

- Research: each coin pickup case of `0x80081a48` writes 6000 (20 s) to its
  own player halfword: invincibility (1047) `+0x8a0`, invisibility (1045)
  `+0x89c`, Double Duke (1046) `+0x89e`; a second coin sets it back to 6000.
  Duke's update drains them by the frame step (`0x80042208..0x80042294`); the
  reset `0x8003f948` zeroes all three at death and level start, and Continue
  then sets `+0x8a0` = 1500 (`0x80042f58`). `dnkroz` is the separate
  `0x800c3cc6`. So death already ends them in the original (as D08A14 wants);
  savestates keep them; a level change ends them.
- Implementation (`gadget_hud.inc`, the existing status bar hook): every
  running timer draws a lit box with its D08A16 icon and `timer * 100 / 6000`,
  stacked a row each above the selected gadget, an unselected jetpack that is
  on and running steroids (order invincibility, invisibility, Double Duke).
  Icons as 4bpp cells at VRAM (968/972/976,205), palettes (1008,208-210),
  uploaded with each box like the medkit and steroids. `items.json` lists them
  as HUD-only art; `test_ui_art.py` checks the cell tables and reductions.
  Debug `controls.powerups`. Timers are only read; no hook, guard or
  generated-code change.
- Evidence (executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`, private
  Xvfb, `recomp/analysis/d08a17-powerups/`, local): one coin, three coins,
  the full seven-box stack (Bio Mask selected, jetpack on, steroids, three
  coins) fits the right column; a box goes when its timer ends; savestate
  reload; death clears all; Continue shows invincibility at 19 then it goes;
  60 and 120 fps; GL 4:3 / 16:9 and Software 4:3 (Software 16:9 cuts the right
  column, existing since D08A6); Vanilla with timers written draws nothing.
  Suites: `ttk-controls-test` (fixture, LEVEL01, all levels), `ttk-input-test`,
  `ttk-inventory-test`, Python 132 OK (2 skipped), `level_overlay_guards.py
  --check`, `check_repo.py`.
- Limits: natural (non-spawned) coins, two-player and boss levels not
  exercised; no profile option (part of Modernized); the number floors like
  steroids, so the last ~0.2 s reads 0.

### D08A18 - Mission inventory: Enter uses the browsed item ("can't use this here")

**Done (user-accepted 2026-10-08: "accepted and confirmed working!!!").**
The user picked mockup B in the cheat-message
style ("option B, but in the console steel palette, just like our cheat
messages ... the font should be exactly the same"). It is built and verified
offscreen. See [note 133](documentation/133-d08a18-mission-item-use.md)
and the work log below. Modernized only; Vanilla and the original Select
inventory are unchanged.

**Background (user):** "if you hover over an item and press enter, currently it
just immediately closes". Today (D08A5, [note
114](documentation/114-d08a5-mission-tracking.md)) Enter or U while the mission
inventory (`,` / `.`) is open just closes it, and it otherwise closes 2.5 s
after the last press. Enter was moved off the gadgets there because the user
instinctively presses it while browsing mission items.

**Picked direction (user, 2026-10-08): option C, use it.** "i actually think
option C is the only one i really like." No info panel and no hold: Enter makes
Duke try to use the browsed item. Where it applies, it does what using it the
original way does; anywhere else a short centred message appears, such as
"CAN'T USE THIS HERE". Options A (hold), B (info panel), D (pin then use), E
(missing-item hint), F (mission sheet) and G (examine) were offered and not
taken.

**Research first.**
- How TTK uses each kind of mission item (key cards, skeleton keys, crystals,
  combo pieces, jewels, papers): automatically on touching the door / socket,
  on the action button at it, or from the original Select inventory. Find the
  check (which flag, which door / socket actor, which range) and whether there
  is a use path that can be driven from the inventory without skipping the
  door's own scripts, sounds, messages or flag changes.
- Whether the disc has an original refusal string or sound (none found in the
  D24A string survey yet; check pickup / door messages and the "need a key"
  style messages). Prefer an original string and sound if one exists.
- If an item can only ever be used by touch, decide with the user what Enter
  does near its door (trigger the same use) and away from it (message only).

**Mockup.** Before game code, show the refusal message (and the success case,
if it differs from the original's) in a new section of the local font/UI picker
page `recomp/analysis/d24a-fonts/ttk-font-picker.html` (built by
`make_picker.py` from `picker.template.html`; local, not tracked): wording,
font, colour and placement against the mission inventory and card from
section 8. The user approves the wording and look.

**Open (decide with the user):** whether the mission inventory closes after a
use or a refusal; whether a refusal plays a sound; whether found but already
used items (if the game tracks that) say something different; what Enter does
on a missing (not found yet) item.

**Acceptance.**
- In Modernized, Enter (and U) on a browsed, found mission item at the place it
  belongs has the same effect as the original way of using it (same scripts,
  flags, sounds and messages), and never uses a gadget.
- Anywhere else it shows only the approved message; nothing in the level
  changes.
- `,` / `.`, `[` / `]`, the 2.5 s timeout when Enter is not pressed, level
  travel, savestate load and death behave as in D08A5.
- Checked with at least one item of each kind that a level uses.
- Levels without mission items are unchanged; Vanilla is unchanged.
- The user confirms it in play.

**Work log 2026-10-08 (research and mockups).**
- The original never uses mission items from an inventory: the Select
  screen's selectable check rejects items >= 6. Seen in Vanilla too: the cursor
  never reaches the mission rows.
- Duke uses an item by pressing action at a lock. The lock's class handler
  says which item it wants (mode 2). Duke pulls the item out (animation 47),
  which clears the item's flag, and the lock opens.
- Shared lock helper: `0x80092a84`. Crystal receptacles: types 180-182 want
  items 11-13. Level 0's card readers (type 478) call the helper with items
  6 and 7.
- Build plan: Enter asks the object in front of Duke (`0x80077f28`, as the
  action press does) for its item. If it is the browsed, found item, Enter
  starts the normal E interaction and the original does the rest. Otherwise a
  message only.
- The disc has no refusal string; the message is new host text.
- Found on the way: a used item's flag is cleared, so the D08A5 mission
  inventory shows a used key as "NOT FOUND YET". This is offered to the user
  as an open question.
- Mockups in section 12 of the local picker (A card status, B centred quote,
  C both, plus the success case), rendered headless with no script errors.
- Not done: a live use at a lock (placing Duke by memory writes put him on the
  wrong floor); overlay handlers of levels other than 0.

**Work log 2026-10-08 (built, Needs playtest).**
- User pick: B, exactly the cheat-message style (`input_notice`: TTK Big
  Italic, Console steel).
- `shortcuts.inc` `mission_use`: Enter / U while the mission inventory is open
  closes it and tries the browsed item.
  - Not found: NOT FOUND YET.
  - Otherwise it asks the object in front of Duke (`0x80077f28` -> `+0x174`;
    item-lock type flags) for its wanted item (handler mode 2) and whether it
    can be used now (mode 0).
  - Right item: the normal E interaction (`input_request_interaction`). Else
    CAN'T USE THIS HERE, and nothing changes.
- New `mission_browsed_item()` in `inventory_hud.cpp`; test stubs added to
  `modern_controls_native.cpp`. Framework untouched.
- Verified offscreen on copies of the user's cards:
  - level 0: both messages away from locks, gadgets untouched;
  - at the empty red-crystal holder, the red crystal was used through the
    original path (animations 47 and 267, holder filled, flag used up, weapon
    redrawn);
  - another item there was refused with nothing changed.
- Suites: inventory, input, controls (44), Python 132 OK, overlay guards,
  `check_repo`.
- Defaults (not asked): one message for every refusal, no sound, used items
  not tracked (they show as missing).
- Not done: a card reader or key door live; levels other than 0; a real
  playtest.

### D08A5 - Mission item tracking in the item switcher (approved design E)

**Done on user acceptance (2026-10-07): "i fully accept!"** Redesigned by the
user after the first try.
Mission items are now their own **mission inventory** on `,` / `.` (previous /
next), shown in the switcher's place with the item card at the top; `[` / `]`
show gadgets only; Enter / U while it is open just close it. Original strafe
(Comma/Period) is unbound by default; profile schema 28. The `\` binding and
the original spec's items 2-3 (row under the gadgets, `\` browsing) are
superseded. Data is the original Select inventory's own per-level list (name
function `0x80087d4c`). Not done: controller input. See
[note 114](documentation/114-d08a5-mission-tracking.md).

**User request and approved design, 2026-10-07.** "lock it in and stand
up a ticket to get that built in the game". For the first time, the game shows
which mission items a level asks for and which are found. The design was
iterated with the user as mockup E in the D24A research page (section 8,
`research/TTK-UI-and-Font-Research.html`, local). Modernized only; Vanilla and
the original Select inventory are unchanged.

**Approved design (build exactly this).** The panel coordinates below are in
the 640x480 overlay space and scale like the switcher.

1. *Gadget row (existing `[` / `]` switcher):* unchanged behaviour and layout,
   with one change. The selection frame is the project frame
   (`assets/ui/item-frame.png`) palette-swapped to the old Duke 3D `tile0020`
   orange ramp:
   `#341c00 #442800 #583000 #6c3800 #804000 #904800 #a45000 #b45404 #cc6818
   #d47430 #d88444`.
   Each frame pixel's brightness, normalised over the frame, picks a step, so
   the bevel and shading carry over. Only colour values are reused, no Duke 3D
   pixels.
2. *Mission row:* a second, read-only row under the gadgets whenever the
   switcher is open. It sits on a panel of translucent grey
   `rgba(12,14,20,0.62)` with a 1 px `#3a4150` outline:
   - one slot per mission item of the current level (counts expanded: Family
     Jewel x3, Skeleton Key x2);
   - the user's 16x16 icon at 2x (`assets/ui/items/`, via `items.json`) in a
     grey project frame;
   - missing items drawn as a dim grey silhouette (greyscale, about 45%
     brightness);
   - "MISSION" bottom left (Microfont, `#848484`) and "found/total" bottom right
     (Microfont, `#989c58`, green `#5fd35f` when all are found).
3. *Browsing:* while the switcher is open, `\` steps through the mission row
   (wrapping). The browsed slot's own frame is palette-swapped to the Console
   steel ramp
   (`#0c0c18 #14182c #202840 #405080 #5c70a0 #7088b4 #84a4cc #94b0d8 #a4c4e4
   #b4d4f4 #bcdcfc`), with no extra outline. The bottom line stays
   "MISSION" and the count. `[` / `]` and Enter never act on mission items.
4. *Item card while browsing:* a fixed 592x52 card at the top of the screen
   (x 24, y 14), in the same translucent grey and outline as the mission panel,
   the same for found and missing:
   - the browsed item's icon at 2x in the steel frame;
   - its name in TTK Medium Italic with the navy shadow (gold CLUT 225 when
     found, blue CLUT 226 when missing);
   - its type below in System 8x8 `#848484` ("KEY CARD", "CRYSTAL", "COMBO
     PIECE", "JEWEL", "KEY");
   - "FOUND" (`#5fd35f`) or "NOT FOUND YET" (`#848484`) right-aligned.
   It clears about 2.5 s after the last `\` press, or when the switcher
   closes.
5. *Controller:* D-pad up/down moves focus between the gadget row and the
   mission row while the switcher is open.

**Data.**
- Each level's set and names come from the D24A survey (the original Select
  inventory names) and `assets/ui/items/items.json`.
- Found state is read live from the player's mission flags: key slots
  `+876`/`+880`; item slots `+884`..`+892` and `+908`..`+916`, as the pickup
  dispatcher `0x80081a48` sets them.
- Verify per level which flag each item uses before relying on it. Seen so far:
  - level 1 papers: `+908/912/916`;
  - level 6 jewels: `+908/912/916`;
  - skeleton keys: `+876/880/884`;
  - level 0 crystals: not yet confirmed.
- Levels without mission items (8, 12, Challenge Stages, bosses) show no
  mission row.

**Acceptance.**
- The row, card, frames and colours match the approved mockup in every level
  with mission items.
- Found and missing follow real pickups, savestate loads and level travel.
- `\` browsing and the card timeout work.
- Gadget cycling and Enter are unchanged.
- Vanilla is unchanged.
- The user confirms the look in play.
### D08A6 - Selected gadget shown on the HUD (approved design A)

**Done on user acceptance (2026-10-07): "i fully accept!"** The selected gadget's box is at the
original item slot over ammo, drawn by the status bar's own calls (lit when on,
dim digits at colour 0x31 when off, medkit always lit); an unselected jetpack
that is on moves one row up (20 rows, the original's step); Bio Mask / goggles
keep their original place left of ammo (82,89) when not selected. See
[note 115](documentation/115-d08a6-selected-gadget-hud.md).

**User request and pick, 2026-10-07.** "Enter has the capability of
using the currently selected inventory item, however the HUD does not show the
user what the currently selected inventory item actually is ... say the user
just presses enter without looking, its pot luck." The look must "truly belong
in this HUD", with the same borders and style as the rest of it. A full HUD
redesign is a separate, later matter. Three mockups were drawn in section 9 of
the local D24A page (`recomp/analysis/d24a-fonts/ttk-font-picker.html`, built
by `make_picker.py` from `picker.template.html`; retail-derived, local only).
The user picked **A**: "i think we just go with A. that's my pick." B (left
stack) and C (Duke 3D centre box) stay on the page for reference.

**Design A (build this).** Modernized only; Vanilla unchanged.

1. *Where:* the original item slot. While a gadget is on, the original status
   bar stacks that gadget's box over the ammo box at the bottom right (Vanilla
   capture `analysis/d08a3-ttk-icons/item-jetpack-toggled.png`). In
   Modernized that box is always shown for the gadget selected with `[` / `]`
   (the one Enter / U uses).
2. *Parts, only the HUD's own:* the disc's 46x16 icon/number box (sprite
   record `0x800c44a4`; icon cell x 2..17, divider at x 18, number cell
   x 19..44), the gadget's HUD icon (D08A3 records: health cross for the
   medkit `0x800c44e4`, jetpack `0x800c44b4`, Bio Mask `0x800c44c4`, goggles
   `0x800c4504`) and the red HUD digits (CLUT 227), at the same scale and
   pixel grid as the health and ammo boxes. No new art, no `%`.
3. *Number:* the selected gadget's charge, as the Select screen lists it.
4. *On / off:* lit digits (CLUT 227, as the original) when the gadget is on;
   dim digits when it is selected but off (mockup: CLUT 227 colours at 38%
   brightness). The medkit has no on state and always reads lit.
5. *Another gadget on:* if a gadget other than the selected one is on, its own
   original box (lit) stacks one row higher, so nothing the original shows is
   lost. Mockup stacking step: 14 rows at 1x (28 px in the 640x480 overlay),
   with the lower box drawn over the upper one, as the original draws ammo
   over the item box. Check the real step against the original status bar.
6. *Nothing owned:* no box (and the original behaviour when nothing is on).
7. *Switcher:* `[` / `]` and the existing strip are unchanged; the box follows
   the selection immediately. Mission inventory (`,` / `.`) does not touch it.

**Research first.** Find how the status bar (`0x8008BA30`) decides and draws
the active item box and its stacking offset; decide whether to drive the
original draw (preferred: its own sprites, palette, layout and widescreen
anchoring from D14) or draw a host copy that matches it pixel for pixel. The
host selection lives with the D08A1 switcher (`recomp/src/ttk/inventory_hud.cpp`).
Confirm where each gadget's charge and on flags live (`player+0x358` jetpack,
`+0x360` Bio Mask / goggles branch) and whether the original can show two
active boxes at once.

**Open (decide while building, or ask):** a short flash when the selection
changes or Enter uses it; the medkit's number if charge proves misleading.

**Acceptance.**
- In Modernized the selected gadget's box is always visible at the original
  item slot, and matches the box the original draws (borders, icon, digits).
- It is right after cycling both ways, Enter / U use, direct shortcuts (M, 6,
  etc.), depletion to zero, toggling on/off, another gadget being on,
  savestate load, death / Continue and level change.
- Dim / lit state follows the real on flag.
- Default GL and Software renderers; 4:3 and widescreen corners; resize.
- Vanilla HUD unchanged.
- The user confirms the look in play.

### D08A7 - Custom medkit gadget icon (switcher strip and HUD box)

**Status: Done (user-accepted 2026-10-07).** Built as planned, with one change
from the plan: the live survey found the game fills VRAM x 960-991 from row
223 down at run time, so the cell is at (960,205), not row 208. Details:
[note 116](documentation/116-d08a7-medkit-icon.md).

**User request, 2026-10-07.** "Instead of the red cross for the medkit's
inventory representation, we can go for this sprite that i have made ... The
health icon red cross remains the same." Art: `research/inv/medkit.png`
(16x16 RGBA, 176 colours, by MusicMonsterMod). A 15-colour reduction for
the HUD box, made at the user's request: `research/inv/medkit-15col.png`
(k-means in PSX 15-bit colour, no dithering, about half the error of a
median-cut reduction). Both are previewed in section 9 of the local D24A page
(`recomp/analysis/d24a-fonts/ttk-font-picker.html`): the D08A7 card and
mockup A, which now opens on the medkit.

**Why it is separate from health.** The medkit box and the health box both
pass sprite record `0x800c44e4` (the HUD health cross) to `0x8008b678`
(`gadget_icon()` in `recomp/src/ttk/gadget_hud.inc`). Changing that cell
would change health too, so the medkit needs its own cell, palette and record.

**Plan.** Modernized only; Vanilla unchanged.

1. *Art in the repository:* copy the user's original and 15-colour PNGs into
   `recomp/assets/ui/items/` (own art, like the D24A mission icons) and list
   them in `items.json` / the assets README.
2. *Switcher strip (`[` / `]`):* `build_ttk_inv_icons.py` uses the full-colour
   art for item 5 instead of the health cross (host pack, no colour limit).
   The pack's provenance notes the change.
3. *HUD box:* write the 15-colour art as a 4bpp cell plus a 16-entry CLUT
   (index 0 transparent) into unused space in the HUD sheet (`FONTS.RAW` at
   VRAM 960,0). Read of the disc file found empty 16x16 cells (for example
   rows 208-255, VRAM x 960..) and 60 unused CLUT rows in column x 1008
   (for example y 53-71, 95-108). Put a 16-byte HUD record (CLUT id, 16x16,
   VRAM x/y) in guest memory or a host-owned slot the call can read, and
   return it from `gadget_icon()` for the medkit only.
4. *Keep it there:* find what reloads or overwrites VRAM (level load, FMV,
   savestate load, menus, D17 worker replay) and re-upload after each, or
   prove the area is never touched. Savestates must not need to carry it
   (do not edit hashed framework headers; see codegen-hash note).

**Research first.** Confirm in a live VRAM dump, in several levels and
after FMV / savestate load, that the chosen cell and CLUT row stay empty.
Check whether the HUD sheet is uploaded once or per level.

**Acceptance.**
- In Modernized, the medkit's selected-gadget box shows the user's medkit
  icon at the same place, scale and pixel grid as the other gadget icons;
  the health box keeps the cross.
- The `[` / `]` switcher strip shows the full-colour medkit.
- Correct after level change, savestate load, death / Continue, FMV,
  pause / Select menu, 60 and 120 fps (D17 workers), GL and Software,
  4:3 and widescreen.
- No other HUD sprite, font or texture is disturbed in any visited level.
- Vanilla HUD unchanged.
- The user confirms the look in play.

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

### D08G3 - `dnupgrade` cheat: upgrade every weapon

**Accepted by the user, 2026-10-06** ("accepted!!"). Implemented and verified live in levels 6 and
12 (medieval and Rome) on private copies; see the work log and
[note 38](documentation/38-debug-cheats.md). Upgrades owned weapons (ammo
carried to the upgraded form) and makes the rest arrive upgraded. Grants no
weapons.

**User request, 2026-10-06.** "another cheat - dnupgrade, which
upgrades all weapons to their upgraded form. laser gatling gun etc."

**Scope:** a new typed cheat in the D08G family (silent entry, centered Duke
font confirmation as in D08G1/D08G2, same solid-ground rule as the other
cheats unless research shows it is safe elsewhere). It gives each weapon the
original upgrade the game itself grants through its upgrade pickups: known
pairs from note 33 are inventory flag 8 resolving Gatling 7 -> 28 Laser
Gatling, RPG 8 -> 29 Incendiary RPG and Flamethrower 9 -> 27 HiTemp
Flamethrower (`8003df40` resolves upgrades). Research first: the complete
list of upgradable weapons per era, how an original upgrade pickup sets the
flag and ammo (the upgraded form reads ammo from its resolved record), what
happens to a weapon Duke does not own yet (decide with the user: upgrade only
owned weapons, or grant and upgrade like `dnstuff`), whether the currently
drawn weapon switches to its upgraded model immediately, and save/load,
level change and death/Continue persistence. Use original data paths only.

**Acceptance:** typing `dnupgrade` shows its confirmation and every eligible
weapon becomes its upgraded form (Laser Gatling, Incendiary RPG, HiTemp
Flamethrower and any others found), firing and switching normally, in at
least two eras; the upgrades persist through a save/load; other cheats and
Vanilla unchanged; the user confirms.

### D08G4 - `dnstuff`, `dnitems` and `dninventory` also give 100% armor

**Done (user-accepted 2026-10-08: "fully accepted").** Full armor from all three. **Scope changed by
the user when selected** (acceptance 4 below is superseded): "if steroids are
already running, and the user types one of these cheat codes, steroids should
be filled back up, and the usage is also stopped ... user types dnstuff,
steroid usage stopps, full steroids are in inventory. the idea is that armor
has nothing to do with steroids in the new world. typing in any of these
codes would replenish armor to 100 either way". See the work log entry of the
same date and [note 38](documentation/38-debug-cheats.md).
User request, 2026-10-08.

**Change.** Each of `dnstuff`, `dnitems` and `dninventory` additionally sets
Duke's armor to 100%. That is the whole gameplay change. Today none of them
gives armor ([note 38](documentation/38-debug-cheats.md) says `dnitems` "does
not add a separate armor grant"; update that note).

**Do not change** anything else they do: weapons, ammo, inventory and
charges, keys, the D08A9 rule that the inventory grant leaves the gadget
selection alone, confirmations, the solid-ground rule. Do not make the three
equivalent or merge their implementations beyond what they already share
(`cheat_codes.h`, `cheats.inc`); other cheats (`dnweapons`, `dnkeys`,
`dnhyper`, ...) are out of scope.

**Steroid independence (see D08A13).** The armor grant must not touch steroid
state: player `+0x364` item-4 flags and the `+0x366` timer stay exactly as
they were. Example: armor 25% and steroids at 40 s before the cheat; armor
100% and steroids still at 40 s after.

**Check first:** the armor field (`+0x234`, which the status bar shows as
`armour / 100`, note 127) and the value an original full armor pickup sets,
so "100%" is the game's own full armor, not a guess; whether setting it
directly needs anything else the pickup does (HUD refresh, a flag).

**Acceptance.**
1. `dnstuff` with less than full armor: armor becomes 100%; everything else
   as before.
2. `dnitems`: same.
3. `dninventory`: same.
4. Each of the three while steroids run (an R dose and `dnhyper`): armor
   becomes 100% and the steroid timer and flags are unchanged (compare
   `+0x366` against the drain alone).
5. Armor shows in the status bar armor element; other cheats unchanged.

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

### D08O1 - Fire weapons while swimming (Modernized, game-wide)

**Accepted (2026-10-06).** User: "i completely accept that this works!
mechanically, it does exactly what it's meant to, but the only issue is the
animation" - the weapon points downwards while Duke swims and fires; that is
D08O2. Underwater the original tests fire before
the swim thrust, so held fire stopped Duke (idle 127); Modernized now hides
fire from the swim handler only while a swim direction is held, so he keeps
swimming and the weapon keeps firing. Swim shots use view aiming and the
crosshair; weapon keys work in water, limited to the weapons the original
allows there. Vanilla unchanged. Level 6 verified live; the user's location
and a second level remain. [Implementation and evidence](documentation/107-d08o1-swim-fire.md).

User request, 2026-10-05: "Duke cannot shoot and swim at the same
time so that needs to be addressed just like we did with the jet pack work."
Reproduction: UI slot 2 (savestate file 01, SHA-256
`756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`), first
medieval level. Test only on dated private copies; verify the hash first.

**Scope:** in Modernized, Duke can fire his weapon while swimming (surface and
deep free-swim, D08M/D08O), aimed at the view with the enabled crosshair, the
way D08Q3 made jetpack flight a shooting state. First establish what the
original allows in each water state (surface, shallow, deep), which weapons it
permits underwater, and how the weapon is stowed/drawn on entering and leaving
water; separate missing Modernized eligibility (aim/reticle/lease guards that
exclude swim modes 4/5) from an original restriction. Where the original
forbids firing, propose the modern behaviour (and any weapons that should stay
unusable) to the user before changing rules. Reuse the D08Q3 approach: swim
eligibility in the weapon-aim and reticle paths, weapon presentation, beam and
projectile completion, transitions into and out of water. Work with D11E
(first person while swimming).

**Acceptance:** in Modernized, Duke fires the agreed weapons while swimming in
at least two levels, aimed at the view with the crosshair, in third person (and
first person once D11E exists); entering/leaving water, holster and weapon
switching stay clean; the jetpack and ground aiming are unchanged; Vanilla
unchanged; the user confirms.

### D08O2 - Weapon points forward while swimming and firing in motion

**Accepted as v1 (2026-10-07)** (user: "it works and i accept this as a v1"; the more natural pose is D08O2A) ([note 118](documentation/118-d08o2-swim-weapon-forward.md)). **User request, 2026-10-06.** After accepting D08O1: "make the weapon
point forward when in use when swimming and in motion, as it currently points
downwards." While Duke swims (underwater thrust 128-130, and check the surface
strokes 123-126) and fires, the firing upper-body animation plays over the
stroke and the weapon points down; shots already go to the crosshair.

**Scope (Modernized):** the weapon and arms point forward along the view (the
crosshair) while swimming and firing in motion. Research first: how the swim
stroke and the upper-body weapon track combine (upper anims 9/10 pistol,
24-28 shotgun/rifle/crossbow, 29/33/34 Gatling, 39/41 pipe bomb), whether the
D07/D08Q3 presentation hook (`0x80097a44`, upper-body aim direction) runs in
the swim states and why it does not lift the arms there, the muzzle position
measured in D08O1 (about 420 units lower during the stroke), and the body
facing (the original thrust moves along body yaw/pitch, so Duke faces the
swim direction, not the view). Prefer original animation data and the
existing aim path over new art. Note 107 has the D08O1 evidence and lab.

**Acceptance:** in Modernized, while swimming in each direction and firing
each underwater weapon, the weapon visibly points toward the crosshair in
third person; idle underwater firing, surface swimming, ground, jetpack and
Vanilla unchanged; swim-and-fire mechanics from D08O1 unchanged; the user
confirms.

### D08O2A - Natural swim-fire pose v2 (arms raised, head up, torso in the stroke)

**Accepted (2026-10-07)** (user: "this is actually rock solid ... I'm happy with it and its 100% playable"; the last-shot-down issue from the capture was fixed in a follow-up the same day) ([note 119](documentation/119-d08o2a-natural-swim-fire-pose.md)).
Only the shoulders now take the floating-fire arm pose (aimed at the
crosshair) while the chest, head and legs keep the original stroke; v1's
chest turn is gone. The head stays original per the user ("his head is always
facing in the right direction anyway"). **User request, 2026-10-07**, on accepting D08O2 v1: "i want v2 in the
backlog though, which will be a more natural pose rather than his entire
torso standing up, so it will involve moving the arms in the position as if
firing up and his head looking up."

**Scope (Modernized):** while Duke swims underwater and fires, his torso and
legs keep the swim stroke (body pitched along the swim direction); only the
arms come up into a firing position toward the crosshair and the head turns
to look along the view. Replaces the v1 whole-upper-body turn.

**Research start (from D08O2, [note 118](documentation/118-d08o2-swim-weapon-forward.md)):**
v1 sets `+0x224 |= 0x100` so the original builder `0x80097c04` aims joint
`[model+0x2d]` (joint 1, which carries both arms, the head and the torso)
and `[model+0x34]` (joint 9, neck/head) through `0x80097a44`. `0x200000`
aims joint 9 only and leaves the arms in the stroke. Arm joints are 3-8
(joint 7 is the gun hand, 4 the other hand; 5/8 measured near the
shoulders). The builder also has a mode 3/6/7 path that aims
`[model+0x31]` with `0x800978f8` for upper anims 8/11. Candidates: aim the
shoulder/upper-arm joints (and joint 9 for the head) instead of joint 1,
for example by calling the aim helper for those joints from a hook on
`0x80097b90` (child composition) or by a dedicated joint-matrix pass after
the build, keeping the original firing upper animation. Prefer original
animation data and the existing aim path; check the first-person weapon
joints (D12) and the replayed frames at high refresh.

**Acceptance:** in Modernized, while swimming in each direction and firing
each underwater weapon, the torso and legs stay in the stroke, the arms and
weapon point toward the crosshair and the head looks along the view, in
third person; floating fire, surface swimming, ground, jetpack and Vanilla
unchanged; D08O1 mechanics unchanged; the user confirms.

### D08O3 - E in water keeps the weapon

**Accepted (2026-10-07)** (user: "fully passed my playtest. feels great to play. i accept this job now as complete!!") ([note 120](documentation/120-d08o3-e-in-water.md)).
**User request, 2026-10-07:** "pressing E while swimming holstered my weapon
and i had to press one of the numbers to draw it. I dont want that to happen."
The original climbs out of water only with the weapon stowed, so E keeps
stowing first; the user chose "redraw if no climb" over never stowing (needs
the side-effecting ledge probe) or ignoring E in water.

**Acceptance:** in Modernized, E while swimming or floating with a weapon out
and no ledge to climb leaves Duke armed again within about half a second,
without stopping his swimming; E at a ledge still climbs out and draws on
land; E on land and Vanilla unchanged; the user confirms.

### D08J2 - Poles and chains: A/D turn the wrong way

**Accepted (2026-10-07, user: "fully and completely accept it all").**
Executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the new regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`. Cause: original. On a pole or
chain (hang-climb 192..195) Left/Right reach the sidestep `0x800439e4`,
where D-pad Left carries Duke round the object to his own right (Vanilla
private run confirms it); our A was D-pad Left. Fix (Modernized, by state, so
every pole and chain): A sends D-pad Right and D sends Left there
(`pole_sidestep_ready()`, `pc_input.cpp`). Ladders (186..189), climbing
walls and Vanilla unchanged. Private slot-3 runs: A goes left, D right, in
the third-person and first-person profiles; W/S climb; suites pass.
[Note 122](documentation/122-d08j2-pole-chain-sidestep.md).

**Todo. User request, 2026-10-07:** "when climbing poles or chains, left and
right are inverted, A turns right, and D turns left! i would like the
opposite to happen." In Modernized, while Duke is on a pole or chain, A
should turn (or circle) him left and D right, matching the screen.

**Research start:** find the pole/chain climb states (attached traversal,
compare the ladder anims 147-156 / 185-211 in `traversal_state_ready()`) and
how A/D reach the original there (D-pad Left/Right turn buttons, L2/R2
words, or the traversal camera-only lease that keeps the original
directional buttons, D08V). Establish whether the original itself turns this
way relative to the camera (third-person view from behind or in front of the
pole) or whether our mapping flips it; fix it game-wide for every pole and
chain, not one level. Check ladders are unaffected.

**Test location:** the user's UI slot 3 is a good place to playtest chain
climbing (user, 2026-10-07). Test only on dated private copies of that
savestate; verify its hash first and never use the player's cards.

**Acceptance:** in Modernized, on every pole and chain tried, A turns Duke
left and D right as seen on screen, in third person (and first person where
supported); ladders, ground movement and Vanilla unchanged; the user
confirms.

### D08J3 - Free camera while on ladders, poles and chains

**Accepted (2026-10-07, user: "we have an absolute winner once again!! it's
working well, very fun, looks great").** Executable
`5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520` (with climbs centring the view) is the
new regression baseline.
**Built 2026-10-07 (Needs playtest).** Executable
`2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`. Finding: the climb keeps the
normal camera and Duke's pivot; in mode 3 the original only swaps in a high
look-down boom (0, 2000, -2000), and the host orbit lease stopped there (it
already covered mounts, the top exit, mantles, hangs and ceilings). Change
(Modernized): mode-3 climbs (147..156, 185..211, with entry frames) join the
camera-only traversal lease, so the mouse orbit, zoom, shoulder and V recentre
work while climbing; the original still moves Duke. First-person profile shows
the orbit during climbs (the D08V hang rule). Poles/chains: A/D follow the
screen from any side (D08J2 swap undone when the camera looks at Duke's
front, latched while a key is held). Private runs: ladder slot 5 full orbit
and climb, chain slot 3 A screen-left / D screen-right from behind, front and
side, ladder top mount / enemy exit / descent regression unchanged, Vanilla
original boom; suites pass. User playtest: "it works perfectly!"; follow-up built: climbs centre the
view whatever the H shoulder setting (eases back after), executable
`5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`.
[Note 124](documentation/124-d08j3-climb-camera.md).

**Todo. User request, 2026-10-07:** "free camera while on ladders/climbing,
and of course this will require deep investigation and user friendliness
testing." In Modernized, the mouse camera should stay free (orbit and look)
while Duke climbs, instead of the original climb camera.

**Research start:** what owns the camera on ladders, poles and chains today
(the D08V camera-only traversal lease, attached states, original scripted
climb cameras), how climbing input is resolved relative to the camera (up /
down / sideways, mount and dismount, the D08U ladder-top mount and D08J
grabs), and what breaks if the view is free (direction of climb input when
looking away, collision with the wall the ladder is on, first-person).
Prototype behind the Modernized profile and test usability with the user
before settling defaults (for example free orbit with automatic recentre,
or a limited look range). Chain climbing: the user's UI slot 3 (see D08J2).

**Acceptance:** in Modernized, the camera can be moved freely while
climbing ladders, poles and chains; climbing controls stay predictable from
any view; mounting, dismounting and ladder-top/airborne grabs still work;
no camera clipping regressions; Vanilla unchanged; the user confirms after
usability testing.

### D08J5 - Climb down chains

**Accepted (2026-10-07, user: "that definitely works, and i accept it ... it
works perfectly").** Executable
`3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be` is the new
regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`. Finding: from
the slot-3 platform no original move reaches the chain (type 842, flags
`0x400`, top 509 below the floor, about 350 past where walking stops): the
jump overshoots into the pit and there is no top mount. Once attached, the
original descends with Down and steps off onto the walkway (191), but its
down probe (`0x8007d65c`) accepts a floor above Duke's feet, so on the
platform side near the top Down lifted him back onto the platform (Vanilla
too). Change (Modernized, `pole_climb.inc`, by flags and state): E at the top
of a pole or chain (the D08U request and hint) attaches it as the airborne
catch does (154) and swings Duke half a turn round it to the far side while
lowering him; a new codegen hook `0x8003964C` makes the down probe ignore
floors above Duke; Ctrl lets go anywhere, S at an end with nothing below lets
go; the 196/191 exits keep neutral directions. Private slot-3 runs: E mount,
S to the walkway and step-off (third and first person), W back out at the
top, Ctrl/Space let go, A/D screen-relative, platform-side S descends; Vanilla
unchanged; slot-5 ladder regression matches; suites pass.
[Note 125](documentation/125-d08j5-chain-descent.md).

**Todo. User request, 2026-10-07:** "Next up, we have to stand up a backlog job
which is the ability to climb down chains. Slot 3 is a great one for testing
this with as theres a chain right in front of us ready to attempt climbind
down."

**Test location:** the user's UI slot 3 (savestate file 02), re-saved again
2026-10-07 18:56 with a chain right in front of Duke (SHA-256
`b4a5d740570335bca3ee41951d3462b53d684e3fc028a0a1c7fcffc8e551acf8`; earlier copies `a519f8bc...`
(D08J4, top of the chain) and `d85450de...` (D08J2, under it) are other
saves). Test only on dated private copies; verify the hash first and never use
the player's cards.

**Research start:** how Duke gets onto this chain from where he stands (E
grab, a jump to it, or a top-of-chain mount like D08U's ladder-top mount),
what S does on the hang-climb (192..195, mode 3; D08J2 saw S descend a short
way at slot 3's lower chain) and where the original stops him: the chain's
lower end, a bottom hang, an automatic drop, or no descent at all. Compare
Vanilla (D-pad Down, Square) with Modernized (S, Ctrl, Space) to establish
whether the limit is original or ours (the D08U1 ladder S/let-go rules and
the D08X hang release are known Modernized interventions on similar states).
Cover poles with the same state where they share it. Fix by state, game-wide,
not this chain alone.

**Acceptance:** in Modernized, Duke can climb down the slot-3 chain from the
top to its lower end with S, and at the bottom steps off or lets go
predictably (and can let go anywhere with Ctrl or Space); W still climbs, A/D
and the D08J3 free camera unchanged; Vanilla unchanged; the user confirms.

### D08V1 - Modernized controls lost after a missed chain jump

**Accepted (2026-10-07, user: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area").** Executable
`cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43` is the new regression baseline. Playtest log
`session-20261007-202257.log`: two chain top exits (196), no identity failure.
**Built 2026-10-07 (was Needs playtest).** Executable
`cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`. Cause: the
original pole/hang side probe `0x800439e4` sets or clears bit 0x40 of the
upper-body flag entry of Duke's current animation (store `0x80043bec`, indexed;
its `lui` sits in a branch delay slot, so the earlier review missed it). On
every chain top exit that is 196. The code guard read it as changed code and the
lease stayed off for the session. It reproduced on the first private run (E
mount, W, top exit). The tank fallback then had no turn. Change: the flag table
moved to masked `state_guards` (`code_identity.h` `masked_identity`): bit 0x40
of all 280 entries, and the low half of entry 265 that the bonus levels'
`BONUS.OVR` writes, are state, and every other bit stays authenticated. An
identity refusal now turns Duke with the mouse through the original D-pad turn
and shows `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`. It still recovers
when the bytes return. New `tools/local/guard_writer_audit.py` scans the disc
for stores into guarded ranges (delay-slot aware) and diffs a live game. The
only other hit, `0x8001bd14`, is unreachable, and the other guard sets are
clean. Private slot-3 runs: top exit, two catch jumps, a missed jump and fall
keep the lease; a live audit shows 0 guarded words differ; a forced loss turns
with the mouse (+400 counts gives +491), moves, and recovers on restore; Vanilla
host idle. Suites pass; Python 126 OK. Bonus levels were not played.
[Note 126](documentation/126-d08v1-control-loss.md).


**Todo. User report, 2026-10-07:** "where i missed a jump, and somehow duke
never went back into modern controls mode, and ive lost the ability to control
him. this used to happen in some of the earlier builds, but we must basically
make sure that modern controls take precidence over everything, aggressively
hooking so this doesnt happen, because theres essentially no way back other
than to quit the game and reopen it i believe."

**Testbed:** the user's session log, copied before rotation to
`recomp/analysis/control-loss-20261007/session-20261007-195012.log` (binary
`3c0ca76e...`, the D08J5 baseline). Use dated private copies of the player's
saves only; the user's slot 3 (chain) is the nearest savestate.

**First evidence (read-only look at the log and the live game, 2026-10-07):**
- After several D08J5 chain mounts and climbs, a W + E jump caught the chain in
  the air (148 -> 154). At that moment the log shows `[TTK identity] guard 52
  (0x800c2a8c, 504 bytes) changed at 0x800c2b34: 0x00000021, expected
  0x00000061`, then `WASD tank fallback (... state)`, Duke climbed out with the
  top exit 196, and from then on every line is `[TTK lease] inactive
  (identity)` / `ORIGINAL MOVEMENT (identity)` until the user gave up.
- Live RAM (game still running, read through the debug port): the only
  difference from the executable in that guard is that byte. `0x800c2b34` is
  entry 196 of the original upper-body animation flag table (`0x800c2824 + 4 *
  anim`); 196 is the pole/chain top exit. The original toggles bit 0x40 of these
  entries at run time (already known for 149/152/153, excluded from the guards
  for that reason). So a game-state write was treated as tampered code, and
  `identity()` turned the whole Modernized lease off for the session with no
  recovery path.

**Scope:**
- Find every original writer of that table (`0x80055438` object-hang clear,
  `0x8004c8b8` ledge-hang set, and whatever touched 196 in a chain catch), and
  exclude runtime-written entries from the code guards (or guard the table
  with the toggled bit masked), game-wide.
- Make modern controls robust: a data-table or other non-code mismatch must
  never permanently drop the lease; identity loss should be re-checked and
  recover when the bytes return or prove to be state; distinguish real code
  tampering (fail closed) from known game-state tables; never leave the player
  with neither modern nor usable original controls; show what happened.
- Verify in a reproduction of the missed chain jump (slot 3) and audit the
  remaining guards for other runtime-written data.

**Acceptance:** the slot-3 missed jump / chain catch / top exit sequence keeps
Modernized controls; no guard trips on game state across the tested climbs,
hangs and exits; a forced identity loss recovers or degrades to working
controls rather than locking the player out; Vanilla unchanged; the user
confirms.

### D08J4 - Ceiling monkey-bar climbing drops Duke at the wrong points

**Accepted (2026-10-07, user: "it's a complete winner for me. I totally accept").**
Executable `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa` is the new regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`. User video
`research/screencaps/Video_2026-10-07_17-49-43.mp4`: jumping for a ledge Duke
grabs the ceiling, "all tank controls! ... you move the way duke is facing",
and he falls off. The ceiling hang is original mode 7 (149..151) under a
climbable object (flags `0x80`, here grate type 759). Falls: ours. D08X's
object-hang release let go when S was held, or after W had been held 40 updates
without a climb, so travel with W dropped Duke after about two seconds; raw
original Up never let go and stops at the ends. Tank feel: original (Up/Down
along the facing, Left/Right turn on the spot). Fix (Modernized, by state and
flags, every climbable ceiling; `ceiling_hang.inc`): WASD steer Duke
camera-relatively (host turns his yaw, the original Up carries him), Ctrl or
Space lets go; D08X now releases only crates (`0xc0`). Private slot-3 runs:
W/A/D/S/diagonals/mouse-turned W travel within 10 degrees of the camera
direction (A -98 and D +84 include the hand-swing wobble), no drops in 120-240
frames, ends hold, Ctrl/Space let go; Vanilla untouched (0 host updates);
suites pass. The user's red-carpet ledge was not located offscreen.
[Note 123](documentation/123-d08j4-ceiling-hang.md).

**Todo. User request, 2026-10-07:** "the ceiling climbing seems to drop off at
the wrong points in that same save slot (i just climbed the chain and re-saved
at that point). duke is supposed to be able to jump and climb across this
ceiling like monkey bars but he just randomly falls. Duke can actually make
all the jumps now with modern controls too. I wanna look into some kind of fix
for this area. it's not broken, it's still playable, fortunately."

**Test location:** the user's UI slot 3 (savestate file 02), re-saved
2026-10-07 at the top of the medieval chain (SHA-256
`a519f8bc0c2f8f23c2c61283793a548f8d2ede687cdd810e0c98f512eff262a6`; the
D08J2 copy `d85450de...` is the earlier save under the chain). Test only on
dated private copies; verify the hash first and never use the player's cards.

**Research start:** identify the ceiling-hang state (mode, animations, the
attached object or ceiling geometry in `+0x17c`/`+0x180`) and the original
test that lets go. Establish whether the drops are original (Vanilla, D-pad
only) or come from Modernized input (WASD fed as original buttons in the
attached-traversal block, camera-relative direction, held E or jump), and
whether they fall at gaps between the hang surfaces or in mid-span. Fix it
game-wide by state, not this room alone.

**Acceptance:** in Modernized, Duke can traverse the slot-3 ceiling end to end
without falling except where the player lets go or the ceiling really ends;
jumps between sections still work; Vanilla unchanged; the user confirms.

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

### D08S - EDuke32-style jetpack (instant J on/off, 61-second fuel)

**Todo. Reopened 2026-10-06 by user request** (cancelled 2026-09-29, may
revisit). User: "eduke style jetpack, where you press J and immediately begin
flying, and the jetpack fuel lasts for 61 seconds, matching that of eduke,
pressing j again stops flying." Nothing implemented yet.

**Scope.** A third Modernized jetpack scheme, alongside Modern and Classic
(D08Q, D08R), selectable by `--jetpack` and later the menu (D19). **J** starts
flying immediately and pressing **J** again stops flying immediately, with no
graceful lift-off, landing or cut-out animation. It must also work **in
midair** (while jumping or falling), which TTK does not allow: the original
entry `8004aaf8` requires Square on the ground and rejects the airborne and
landing states (anim 105/106 and the `+0x224 & 0x241` flags). Controls while
flying: **Space** ascends, **Ctrl** descends, **WASD** moves relative to the
view, mouse turns camera and Duke together. No input holds position (Duke3D
hover). D08Q3 view aiming and crosshair stay available.

**Fuel:** a full jetpack lasts **61 seconds** of flight, matching EDuke32,
draining only while the pack is on (hovering included). Research how TTK's
fuel `+0x35a` and drain `-= dt` (`8004ade0`) map to seconds, what a pickup
grants, and how the HUD green % should read so 100% means 61 s in this scheme.

Research first: whether mode 10 can be entered directly from airborne modes
with the original handler kept intact, or whether this scheme needs a host
flight model that drives the root position and state flags itself; what the
instant J-off should do (Duke3D drops straight into a fall); animation choice
for instant take-off; collision with ceilings and water. Keep all writes gated
on Modernized + the selected scheme; Vanilla and the other two schemes
unchanged. Related: D08Q4 (`dnkroz` unlimited fuel), D11F (first person).

**Acceptance:** with the EDuke32 scheme selected, J toggles flight instantly
on the ground and in midair; Space/Ctrl climb and descend at steady rates;
WASD flies relative to the view; releasing input holds position; a full pack
gives 61 s of flight; J off drops Duke into a normal fall with controls live;
fuel-out behaves like J off; Modern, Classic and Vanilla unchanged; the user
confirms.

### D08Q4 - Duke3D-style `dnkroz`: health to 100 and unlimited jetpack fuel

**Follow-up 2026-10-06 (needs playtest):** user: "it should also give the
jetpack too". While god mode is on, Duke has the jetpack at full fuel
([note 38](documentation/38-debug-cheats.md)).

**Accepted by the user, 2026-10-06** ("accepted!!"). Implemented; verified live in Modern and
Classic flight on private copies; see the work log and
[note 38](documentation/38-debug-cheats.md). Atomic Health above 100 is the
original pickup and was not driven live.

**User request, 2026-10-06.** "dnkroz should also allow for unlimited
jetpack, that's just the behavior in eduke also." While `dnkroz` /
`dncornholio` god mode is on (D08G), jetpack fuel does not drain in every
jetpack scheme; switching god mode off resumes normal drain from the current
amount. `dnkroz` stays god mode; unlimited jetpack is an addition, not a
replacement (user, 2026-10-06: "dnkroz is still god mode, but in duke3d it
also gave unlimited jetpack"). User, 2026-10-06, on Duke3D's `dnkroz`:
"health just goes to 100 and jetpack fuel stays on full too. duke can
actually receive more health but only by taking an atomic health." So turning
god mode on sets health to 100 (today's `dnkroz` does not restore health,
GAME_MANUAL) and keeps jetpack fuel full; health above 100 comes only from
TTK's equivalent of the Atomic Health pickup, up to the real maximum of 200
(user, 2026-10-06: "in which case it reaches the real health max of 200");
identify which original item plays that role and how TTK stores the cap. Check the Duke3D/EDuke32 source to confirm
these details and match them. Lead: the original mode-10 drain is
skipped when `800c3cc4` is set (note 57). The D08Q3 lab
(`analysis/d08q3-jetpack-aim/transitions.py`) cleared `800c3cc4` to "pause the
fuel cheat", so it is probably the original's own unlimited-fuel flag:
confirm what sets it before adding a host override. Also cover the D08Q2 dnstuff/Continue paths.

**Acceptance:** turning god mode on sets health to 100 and fills the jetpack;
with god mode on, flight in each scheme lasts indefinitely and the HUD % does
not fall; health goes above 100 only through the Atomic Health equivalent,
up to 200; with it off, fuel drains as before; no change without
the cheat; Vanilla unchanged unless the cheats already apply there.

### D08Q5 - Weapon switching while flying the jetpack

**Done (user-accepted 2026-10-06):** "excellent!! i accept."
In Modernized flight the weapon shortcuts
(1-0, wheel, semicolon/apostrophe, X) now reach the original weapon request;
the original mode-10 state completes it with the normal upper-body
holster/draw while flight continues. Verified live in Modern and Classic
flight with every weapon, firing after each switch; ground, swim and Vanilla
unchanged; suites pass.
[Implementation and evidence](documentation/108-d08q5-jetpack-weapon-switch.md).

**User request, 2026-10-06.** "allow for weapon switching while using
jetpack, both wheel and numbers." In Modernized flight (mode 10, every
jetpack scheme: Modern, Classic and the future EDuke32 scheme D08S), the
number keys 1-0 and the mouse wheel (plus semicolon/apostrophe and X, the
D08A shortcuts) change weapons as they do on the ground. Today D08A
selectors reject requests during flight.

Research first: whether the original allows weapon changes in mode 10 at all
(draw/holster animations on the upper body during anim 163/165 and the
lift-off/landing 106/108), which weapons the original permits while flying,
how the D08Q3 flight aim and crosshair follow a newly drawn weapon, and
whether a holstered or empty pack state matters. Use the original
equipment/draw path; do not swap models directly. Keep D08A groups, burst
resolution and the underwater rules (D08O1) unchanged.

**Acceptance:** while flying in each scheme, number keys and the wheel cycle
through owned weapons with the normal draw, and the new weapon fires with
flight aiming and the crosshair; flight, fuel and landing are unaffected;
ground and swimming switching unchanged; Vanilla unchanged; the user
confirms.

### D08Q6 - Modern jetpack altitude creeps upward in level flight

**Accepted by the user, 2026-10-06** ("i accept this as fixed! the test passes
and this feels great"), on revision 5b (binary `2a49b29acbd0de49...`).
Revision 5: Modern flight model rewritten.
The real cause of "cannot move with WASD unless ascending or descending" was
the original `8004ac08`, which drops horizontal motion whenever the
vertical root is 0. A precisely held height made that common. Modern now owns
the flight velocity at the original's speeds (16000 horizontal, -11800
climb). The original still gets its flight pads (flame, lean poses, fuel), and
their thrust is cancelled from the post-handler reading. Height is held with
the floor cap, and a corrected +-2 vertical step keeps horizontal moves
applied. History follows.

Revision 1: Modern flight keeps one held altitude,
captured when Space or Ctrl ends. Level flight steers back to it, hover pins to
it without the original bob, and it is capped at the original floor approach
height over higher ground. A lab circuit of about 2400 updates ends within
9 units of the start, compared with about 460 units of climb in six legs
before. Revision 2 (after the user found it jerky) parks the bob at its flat
sine peak instead of phase 0 and slews the base gently, so hover is flat and
stops settle over 3 updates. See
[note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

**User report, 2026-10-06.** "duke keeps gaining height when using the
jetpack in modern mode. how i tested this was using dnkroz, and elevating to
a particular height, and in the first level i just kept circling round the
apartment building and through the alleyway and noticed duke was getting
higher and higher. i was also trying to look up quite a lot and i think that
had something to do with it. anyway, duke's elevation should just be fixed
when jetpacking unless the player uses ctrl or space"

**Scope:** in the Modern jetpack scheme (D08Q), Duke's altitude must stay
fixed during level flight and hover; only Space (climb) and Ctrl (descend)
change it. Classic (D08R) keeps its original burst/gravity physics and is out
of scope; the future D08S scheme should share whatever fix is made for Modern.

Research first: reproduce the climb on a private LEVEL01 copy with `dnkroz`
(long circuits around the apartment building and alley, with and without
pitching the view up, mouse turning, W/A/S/D combinations and diagonal
input). Measure Y per update against view pitch, yaw rate and held
directions. Candidates to test, not conclusions: the fixed
`k_jet_level_trim` (+0x1e8 = 18) only cancels the measured single-direction
lift, so diagonal input, view pitch or the D08Q3 view-aim presentation (body
lean +0x87c..+0x880 or pitch reaching the mode-10 thrust rotation) may leave
a residual climb; a release/re-engage of the hover pin at a drifted height
would also ratchet upward. Prefer holding the altitude captured when vertical
input ends over tuning a constant.

**Acceptance:** in Modern flight Duke's height stays fixed while hovering,
moving in any direction, turning and looking up or down for long circuits
(including the user's LEVEL01 apartment/alley route); Space climbs and Ctrl
descends as before, and releasing them holds the new height; landing,
fuel, J off, firing and D08Q5 weapon switching unchanged; Classic and Vanilla
unchanged; the user confirms.

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

### D08T2 - Duke-symbol pushable blocks with Modern controls (game-wide)

**Accepted (2026-10-05, user: "i accept everything aside from the ladder
glitch").** The hint changes are accepted too; the slot-12 ladder continues
under D08U1. Earlier: Needs playtest. Cause: the block is an ordinary original
pushable object (type 924, flags `0x081890e2`), but `object_type_flags()` in
`push.inc` refused types `>= 512`; the original type table holds 1062 types
(`0x7428` bytes allocated at `0x8001b634`). Fixed in the shared lookup, so
grab/push, the ladder-top mount and the ledge/crate mantle now see every type
(4 more pushable, 6 more ladder, 114 more climbable types). RMB pushes the
slot-1 block in third and first person; dumpster and Vanilla unchanged.
Candidate `06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
[Note 103](documentation/103-d08t2-object-type-range.md).

Selected 2026-10-05. Reproduction: UI slot 1 (savestate file
00, SHA-256 `ab8d2e156b6bdaf802f7a454449a07eaa8a47f530f99108a96fb0b8abefcbc72`,
written 2026-10-05 01:15), first medieval level: the block directly ahead
should push. Guide: on the right side of the castle, before the moat enters
under the wall, a ledge runs along the castle wall (jetpack onto it); the wall
with the atomic symbol pushes to open a secret (power-ups and ammo). Fix the
system for every pushable object, not this block. Private copies:
`recomp/analysis/d08t2-20261005/states/`.

User report, 2026-10-05 (D22B game-wide playtest).** Blocks marked
with the Duke Nukem symbol are meant to be pushed. With Modern controls they
cannot be pushed; after switching to Legacy controls the same block pushes
correctly, so the original interaction works and our modern mapping does not
reach it. Seen in the Roman area (one), the medieval area (one) and two near
the start of the medieval area. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Direction from the user: E = interact, RMB = grab/hold/push. For these
objects investigate RMB as the modern interaction: approach, hold RMB to
engage, move to push/manipulate, release RMB to let go. First inspect how the
original interaction actually works for these blocks (object type/flags,
which original button and state it uses, how it differs from the D08T alley
dumpster that RMB already handles), then fit it into the D08T/D08T1 grab
design rather than mapping another Legacy button.

**Game-wide:** every pushable object, not one block.

**Acceptance:** the reported blocks in at least two eras push with RMB in first
and third person; the alley dumpster still works; release and focus loss let
go; Vanilla/Legacy unchanged; the user confirms.

### D08T3 - Free manual push and pull while holding Grab

**Todo. User request, 2026-10-05 (D08T2 playtest).** "The rmb pull and push
mechanic seems to work in set amounts at a time ... hold rmb and press W and
duke will push the stone a set amount (im guessing 1130 units) and then stop
and release. I would love to give the user the actual freedom to properly move
the block manually by holding rmb, properly grabbing, and pushing and pulling
the stone block that way." Suggested as the next job.

Today the original push 120 / pull 119 (`0x80048e84`, object motion
`0x800910a0`) is one fixed shove of about 1130-1200 units along one world
axis chosen from Duke's heading; D08T1 lets it run to its end, and Duke then
lets go. Investigate how the original moves the object per update (step size,
axis, collision and blocking checks, the end-of-shove release, sound), whether
a shove can be continued, stopped early or chained while Grab and W/S stay
held, and what drives the push sound and animation loop. Then design modern
manipulation: while Grab is held, W/S move the object continuously (and stop
when released), Duke stays attached until Grab is released, and blocked
objects stop cleanly. Keep the original collision and puzzle rules (no pushing
through walls, keep axis-aligned puzzle positions such as the Duke-symbol
secret walls) unless the user agrees otherwise. Game-wide: every pushable type.

**Acceptance:** with Grab held, W/S push and pull a pushable object by any
amount the player chooses, stop when W/S is released, keep holding until Grab
is released, and stop at obstructions; the dumpster and the Duke-symbol
blocks work in third and first person; puzzles that need a pushed block still
complete; Vanilla unchanged; the user confirms.

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

### D08U2 - Enemy at a ladder top blocks the climb-off

**Accepted (2026-10-07, user: "fully accepted. this is exactly what i wanted. well done!").**
Executable
`e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`. Cause:
original. Resting on the top rung with Up, the handler's exit test
`0x8007d240` first casts a short segment forward (`0x8007cde8`) against
Duke's cell objects; the slot-5 LARD (type 57), about 530 units past the
ladder, is hit, so the exit 190 is refused for as long as it stands there.
Fix (Modernized, game-wide, `ladder_top.inc` `ladder_actor_exit`): when only
living characters (class 6 or 8) block that segment, the host runs the
original exit test with them out of the way for the length of the call and,
if it passes, starts the original exit exactly as the handler would. Walls
and other objects still refuse; the enemy is not moved (pushing was marked
optional by the user). Private runs: slot 5 in third and first person climbs
off (190 -> standing, pistol redrawn); second alley ladder with an enemy
respawned on its landing climbs off and lands 300 units short of it; D08U
top mount and S descent, D08U1 slot-12 ladder unchanged; suites pass.
[Note 121](documentation/121-d08u2-ladder-top-enemy.md).

User clarification: "whether duke actually pushes
the enemy back or not is not a hard factor in this job, but Duke should
certainly be able to make the climb".

**User request, 2026-10-07:** "when an enemy is stood at the top of a
ladder, duke cannot climb onto the surface. Duke should push into the enemy
forcing it back." In Modernized, climbing off the top of a ladder must not
be blocked by an enemy standing on the landing: Duke pushes the enemy back
and completes the climb-off.

**Test location:** the user's UI slot 5 is a good playtest for this (user,
2026-10-07). Test only on dated private copies of that savestate; verify
its hash first and never use the player's cards.

**Research start:** how the original ladder-top exit (the climb-off from the
ladder anims, compare D08U / D08U1 and `traversal_state_ready()`) tests for
room on the landing, and whether an actor in that space makes it refuse,
wait or loop; whether this also happens in Vanilla (original behaviour) or
only through our ladder handling. Prefer the original actor-push / collision
response (the way Duke and enemies already shove each other on the ground)
over moving the enemy by hand; keep it game-wide, not per level, and make
sure the enemy cannot be pushed through walls or off ledges unfairly.

**Acceptance:** in Modernized, with an enemy standing at a ladder top, Duke
climbs off onto the surface and the enemy is pushed back, at slot 5 and at
least one other ladder; normal ladder exits, ladder-top mounts and grabs
unchanged; Vanilla unchanged unless the user agrees otherwise; the user
confirms.

### D08U1 - Ladder that cannot be descended with E (player slot 12)

**Accepted (2026-10-05, user: "accepted!! done, commit. great work").**
Executable `4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83` is the regression baseline. Causes, in order found:
object type 639 refused by the old 512 type bound (D08T2); the ladder ends
above its floor, so the original swung Duke into a bottom-rung hang (now
stopped on the last rung via the original probe `0x8007ded0` and let go with
Square); and, the user's actual failure, the mount blend cut short when the
game drops frames, leaving Duke above the climbing line (now finished while he
rests). See the work log and [note 103](documentation/103-d08t2-object-type-range.md).

**Needs playtest (2026-10-05, second fix).** User, after the first fix: "Duke
cannot climb down the ladder ... when you start pressing S, he just glitches
out. we need to truly fix that." The first fix only let Duke drop out of the
original bottom-rung hang, so the sideways 156 -> 211 swing still played.
Second fix: the bottom is now decided before the swing. At each player update
on a plain ladder (186..189) the host asks the original's own probe
`0x8007ded0` (no ladder and no floor 500 below Duke). The 188/189 case at
`0x80044128` decides from the descent flag `+0x6a`, not from Down, and in the
same update; so when the probe is true during a step down, the flag is
cleared and the original takes its own stop-at-this-rung branch. On that
last rung S sends the original Square let-go from the climbing pose (fall 108,
landing 105), never the hang. Real keys on the slot-12 copy: holding S (third
and first person) climbs down and drops straight to the floor from the last
rung; tapping S stops on the last rung and the next tap drops; W from the
last rung climbs back up and exits at the top (190). Ladders that reach their
floor never trip the probe: the D08U sewer ladder keeps the 185 step-off and
the alley ladder climb is unchanged. Candidate `cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.

First fix (superseded). Two causes. (1) The slot-12 ladder is object
type 639; the shared type lookup refused types `>= 512` (fixed by D08T2): E
now mounts it. (2) User retest: "when you press S to go down, duke starts
glitching". This ladder ends about 1000 units above the floor. At its lowest
rung the original (`0x8007ded0` finds no ladder 500 below) goes 156 -> 211 ->
207, a hang from the bottom rung; there original Square lets go and Up climbs
back, but Down (S) only flips 211/207 with Duke's body in the floor. The
original pad alone does the same, so it is an original state our top mount
now reaches. Fix (game-wide, `ladder_top.inc` `ladder_bottom_hang()`,
`pc_input.cpp`): S in that hang sends the original Square (let go) instead of
Down; on the way in (211) neither. Real keys, both views: S held from the top
reaches the floor (108 fall, landing, Modernized control, weapon redrawn).
The D08U sewer ladder still steps off with 185. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.
[Note 103](documentation/103-d08t2-object-type-range.md).

**Todo. User report, 2026-10-05 (D22B game-wide playtest).** Player UI slot 12
(savestate file 11 in `recomp/saves/local-play/openbios`, written 2026-10-05
00:37, SHA-256
`1608ee9c890ebb30b0311f055e6f13d21a18e61697a61557e384e82b48a59afd`): a ladder
just around the other side of the wall should be climbable downward. With
Modern controls, E does not start the descent. It is the first ladder found
where the modern controls fail. Test only on a dated private copy; verify the
hash first.

Determine why this ladder differs from those that work (D08U top-of-ladder
mount: object type/flags, collision box, reach/side/top gates, approach
direction, level overlay, original attach path) and fix the shared rule, not
this ladder.

**Acceptance:** the slot-12 ladder descends with E; the D08U/D08J1 ladders and
LEVEL00 slots 8 still work; Vanilla unchanged; the user confirms.

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

### D08Z1 - Keep jump momentum when bumping a wall (EDuke32-style air control)

**Done (2026-10-08, user-accepted: "mechanically this feels significantly better, where we can call the actual job as accepted.").** Follow-ups split out as D08Z2
(mantle pose regression) and D08Z3 (stuck after some landings).

Was Needs playtest (2026-10-08). Modernized `jump_walls` = `slide` (default;
`run.py --jump-walls original` restores the bounce; Vanilla unchanged). Before
each airborne update the host runs the original integration and sweep in
isolation; a wall only loses the velocity into it, a ceiling stops the rise,
so Duke keeps his arc and slides along with no 107, rumble or sound. Ledge
grabs, jump mantles and the low-lip scramble start from the contact instead of
the bounce. Evidence: no 107 in 48 wall jumps over three levels (8 of 16 bounced with
`original` where compared), D08X/D08Y/D08U/D08W/D08Z routes unchanged. Revision 1
(slot-6 monument corner still bounced): corner turn search and an original
bounce safety net. Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. [Note 134](documentation/134-d08z1-jump-wall-slide.md).

**User request, 2026-10-06.** "i dont want duke to lose his motion when
you bump into a wall when jumping, i want that behavior to be more like eduke
also, duke should be able to jump into a wall without consequence." Today a
jump that touches a wall cancels Duke's travel (wall-touch cancel and the 107
bump seen during D08Y/D08Z), so he drops and loses the jump.

**Scope (Modernized, optional):** in the air, wall contact only removes the
velocity component into the wall; Duke slides along it and keeps his height,
arc and the remaining horizontal motion, with no bump/stagger animation and no
lost control, as in EDuke32. Research first: where the original airborne
handlers detect wall contact and zero or reverse velocity, which animations it
triggers (107 and others), how this interacts with the D08X/D08Y ledge grab
and jump-mantle (a wall with a reachable ledge must still be grabbable), with
D08Z manual jump air control, and with ceilings and steep slopes. Make it a
Modernized setting, on by default unless the user decides otherwise, so it can
be exposed in the future menu (D19); off restores current behavior.

**Game-wide:** the shared airborne collision path, every level.

**Acceptance:** with the option on, standing and running jumps into straight
and angled walls keep their arc and slide along the wall in at least two
levels; ledge grabs and jump-mantles still work; ground wall bumps, swimming,
jetpack and Vanilla unchanged; option off matches today; the user confirms.

### D08Z2 - Mantle pose regression since D08Z1

**Done (2026-10-08, user-accepted: "accepted!!").** Cause: the D08Y low-lip step-up (no E) could
start armed, and since D08Z1 it starts from every wall slide. The original
mantles only with a free upper body (`0x80051cf0` refuses on upper-table bit
8) and sets only the lower animation. So the legs played 134 while the upper
track held the static weapon-ready pose 20. Every jump mantle now starts in one
place (`jump_mantle_start`). A weapon pose on the upper track is handed to the
mantle and its block put back when the mantle ends, so the weapon fires again.
Holstered E mantles, the slot-12 grab and pull-up, and the crate mantles were
already full body and match `original` frame for frame.
Revision 1: the D08Z1 safety net's late 107 undo left 107's leg joints in
track 3's joint mask, freezing the legs (also in savestates); the undo now
happens at the bounce's contact sound (new hook `0x8006BBD8`).
[Note 135](documentation/135-d08z2-mantle-upper-body.md).

Was Todo. User report, 2026-10-08, at D08Z1 acceptance:** "this has also
introduced a regression where his mantling animation is now some static pose
with his legs and looks weird." Next job by the user's choice.

Leads (not yet investigated): D08Z1 starts the D08X jump mantle, D08X/D08Y
lowered retries and the D08Y low-lip step-up from a recent wall slide
(`jump_wall_recent()`) as well as from the first updates of a bounce 107. In
those cases Duke enters the mantle (134..140) from a flight animation
(98/103/104/109) instead of 107; the mantle starts with `+0x68/+0x6a` reset,
but the upper body or the D08Z1 safety net's 107 undo (which restores
`+0x60/+0x68/+0x6a` at the next runner call) could leave a stale pose. Also
check whether walk-in (ground) mantles are affected, and `jump_walls original`.

**Acceptance:** every mantle (ground walk-in, jump mantle onto crates and
ledges, low-lip scramble) plays its full original animation in Modernized with
`jump_walls slide`; Vanilla and `original` unchanged; the D08Z1 slide and
corner behavior stay as accepted; the user confirms.

### D08Z3 - Stuck for a moment after some landings

**Todo. User report, 2026-10-08, at D08Z1 acceptance:** "there is the
occasional jump where duke lands and he's stuck for a second before being able
to move again."

Leads (not yet investigated): a landing that follows wall slides or a corner
turn; the safety net's restored flight animation meeting the landing; the
landing animation the original picks (`0x80054c04`) after a slide (a hard
landing such as 127/128 holds input for a while). Reproduce first (which
level, jump style, wall or none), compare `jump_walls original`.

**Acceptance:** no landing after an ordinary jump locks movement in
Modernized beyond the original's own landings; reproduction route documented;
the user confirms.

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

### D11D - First-person eye height from Duke's real proportions (game-wide)

**Accepted - user confirmed 2026-10-05:** "i can confirm that i accept both jobs as complete!" Cause: the cowboy costume (levels 1, 2, 3
and 27, the Old West levels) orders Duke's 19 joints differently. The eye
followed joint 9, which is the neck in every other costume but a hip-height
part in the cowboy model, so the eye sat about 300 units low (waist height).
The eye, head hide, weapon torso reference and arm hiding now find their
joints through the model's own part table (`desc+0x24`, the same table the
game reads the right hand from). The neck is now 691-705 above the toes in
all 21 selectable levels. Shared with D12B; see the work log.

**User report, 2026-10-04 (D22A playtest).** UI slot 8: stand next to
the dancer in first person and Duke appears to be at about her waist height.
Press **P**: in third person Duke is about her height or slightly taller. The
first-person eye/camera height does not match Duke's actual size.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Investigate Duke's actual actor/model height, the first-person camera origin
and eye anchor (`first_person_anchor`, D11/D11C), our view offsets and
projection, stance/crouch state, and whether the result varies between levels,
costumes or player states. Derive a believable eye height from Duke's real
proportions.

**Game-wide:** fix the shared first-person system. Do not add a LEVEL01 or
per-level camera offset.

**Regression evidence, 2026-10-05 (user, D22B game-wide playtest):** Duke's
first-person height looks correct in the medieval levels and in the Roman /
HOG HEAVEN area, believable against the surrounding characters. The LEVEL01
report above is still open; any fix must keep those levels looking right.

**Acceptance:** in slot 8 the dancer and Duke read as similar heights in both
views; the first map's accepted first-person routes (apartment, club, subway,
crouch, ladders, swim) still look right; the user confirms.

### D11E - First person while swimming (game-wide)

**Todo. Found by the D22B survey, 2026-10-04.** In BLOOD BATHS (and any deep
water) Duke swims in modes 4/5 (anims 122-128). The Modernized swim controls
and mouse camera own it (D08M/D08O), but `first_person.inc` deliberately treats
swim modes as unsupported, so a first-person player is shown the orbit view
until Duke leaves the water. Reach it with `level 12` and walk into the pool.

Give swimming a first-person eye view (head/eye anchor while the body pitches,
near-surface and waterline behavior, weapon/hands presentation, the D08O
mantle-only exit), or document why a state must stay in the orbit.

**Game-wide:** the shared swim/first-person system, not one level's pool.

**Acceptance:** first person stays active through entering, swimming, diving,
surfacing and leaving the water in at least two different levels; third person
and Vanilla unchanged; user confirms it reads well.

### D11F - First person while flying the jetpack (game-wide)

**Todo. User request, 2026-10-06.** "add another job in for first person
jetpack." Today jetpack flight (mode 10) is unsupported by `first_person.inc`,
so a first-person player is switched back to the third-person orbit while
flying (GAME_MANUAL: "Swimming and jetpack flight switch back to third").

Give jetpack flight a first-person eye view for every jetpack scheme (Modern,
Classic, and the EDuke32 scheme from D08S once it exists): eye anchor through
take-off, hover bob, flight lean and landing; weapon/hands presentation and
the D08Q3 view aiming and crosshair in first person; ceilings and near-wall
clipping (D11B); clean hand-off at J on/off, fuel-out and death. Document any
state that must stay in the orbit.

**Game-wide:** the shared jetpack/first-person system, not one level.

**Acceptance:** first person stays active through take-off, flight, hover,
firing and landing in at least two levels with each jetpack scheme; third
person, ground first person and Vanilla unchanged; the user confirms it reads
well.

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

### D12B - Costume-aware first-person kick leg (game-wide)

**Accepted - user confirmed 2026-10-05** (with D11D). Rule: the kick draws Duke's right-leg
*parts* 5-8 (hip, knee, ankle, toe) from whichever model he currently wears,
found through the model's part table. Medieval and Roman were right because
their models share the first map's joint order (leg joints 14-17); the
cowboy model has the leg at joints 5-8, where joints 14-17 hold other parts.
Later levels inherit the rule. Shared cause with D11D; see the work log.

**User report, 2026-10-04 (D22A playtest).** UI slot 8 or 9: Duke wears
a different costume in LEVEL01, but the first-person quick kick (D12A) still
shows the first map's leg appearance.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Find how the original game represents Duke's per-level costume/appearance
(model, texture page/CLUT, part tables) and make the first-person kick leg
follow Duke's current visual state rather than a fixed leg. Do not hard-code
"level 2 uses leg B" if the game has a meaningful current-costume concept to
derive from.

**Acceptance:** the kick leg matches Duke's third-person costume in the first
map and in LEVEL01, by the same rule; the user confirms. Record the rule so
later levels inherit it.

**Regression evidence, 2026-10-05 (user, D22B game-wide playtest):** the
first-person kick already shows Duke's medieval costume ("working beautifully")
and his Roman / HOG HEAVEN costume ("looks fantastic"). Find out why those eras
are right and LEVEL01 is not before changing anything, and keep them right.


### D12C - Kick impact sound on a real hit only

**Done - user accepted 2026-10-08:** "perfect!! fully accepted." Implemented 2026-10-08. A kick that connects plays the
game's own wall-bump thud `0x2007` (bank 2 entry 7, SPU sample `0x1BAA0`, the
bump call `0x8006b270(0x2007, Duke+4, 0x800)` at `0x800538bc`, no vocal in
it), played one octave up at three times its table volume (user tuning), once,
at the first contact (a still-playing `0x2007` from the wall bump is stopped
first); empty air, missed and out-of-range kicks stay
silent. Contact is the damage sphere's own hit result (it stores the attacker
in its frame only on a hit); walls are the original segment query's world hit
where the boot reaches. First-person quick kick and the third-person original
kick (new entry hook `0x800A979C`, ra `0x80049098` only; the host makes the
same sphere call and the original call is left touching nothing), Modernized
only. See [note 131](documentation/131-d12c-kick-impact.md).

User request, 2026-10-08. Small audio/game-feel job.

**Problem.** When Duke's kick connects (a wall, a breakable crate, an enemy or
another object that takes melee damage) there is little or no physical impact
sound, so the hit feels weightless. It is most noticeable with crates that need
several kicks: the individual kicks are nearly silent.

**Rule.** The sound comes from the authoritative kick hit result, never from the
button press or an animation frame:

- kick -> no collision -> silent;
- kick -> wall -> impact sound;
- kick -> crate/object -> impact sound;
- kick -> actor/enemy -> impact sound.

Do not build a second, audio-only approximation of the kick collision if the
existing melee/kick logic already says whether the attack connected.

**Candidate sound (research clue, verify).** When Duke runs or jumps into a
wall, two sounds can play: Duke's grunt/vocal, and a generic physical
impact/thud under it. The user wants that **non-vocal thud**, never the grunt.
It may be a Duke 3D-inherited sound; treat that only as a clue. It already
exists in TTK's resources (the game plays it on wall collision): find the sound
the game is already playing; do not add a new Duke 3D asset. Useful tools from
D08A10/D27A: the `0x8006b73c` entry log (`controls.steroid_beat.log`), SPU
KEYON capture (`spu_events`) to tell which sample is which, the debug
`sfx <id>` console command, and the note that non-positional sounds use
`0x8006bbd8` instead ([note 129](documentation/129-d08a10-steroids-heartbeat.md),
[note 130](documentation/130-d27a-silent-shift.md)). D12A has the first-person
quick kick's hit sphere and sound notes.

**Avoid duplicates.** Some targets may already play their own impact/damage
sound. Trace the kick/damage/collision paths and place the sound at the most
appropriate common point, so one kick never plays the same sample two or three
times through different paths. Where a target already gives fitting impact
feedback, keep it rather than layering an identical sound on top. Enemy pain
sounds, vocals and damage sounds stay as they are; the new impact complements
them.

**Scope.** Audio only. Do not change kick damage, range, timing, animation,
quick-kick controls, object health, enemy damage reactions or collision
geometry. Vanilla stays original; Modernized-only unless the user decides
otherwise when selecting it.

**Acceptance.**
1. The existing wall-collision impact sound is located.
2. It is separated from Duke's accompanying grunt/vocal.
3. Its sound resource/ID (bank, index, SPU sample) is recorded.
4. The authoritative successful-hit point in the kick logic is identified.
5. The cleanest trigger point is chosen and documented.
6. Empty-air kicks stay silent.
7. A wall kick plays the sound once, at contact.
8. Each successful kick on a breakable object (e.g. a multi-kick crate) plays
   the sound; damage and destruction unchanged.
9. A successful kick on an enemy/actor gives fitting impact feedback.
10. Failed/out-of-range kicks stay silent (seeing a target in front of Duke is
    not enough; the boot must connect).
11. No duplicate playback where the target already has related impact/damage
    audio.
12. All existing kick damage and gameplay behaviour is preserved.

Goal: "If Duke's boot actually hits something, I should hear the impact. If his
boot hits nothing, I should hear nothing."

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

### D17S - Auto frame-rate default: follow the display up to a cap

**Todo. Opened 2026-10-05.** The user worries about 120 Hz as the default and
asked for an automatic default: 120 when the display supports it, or the next
step down; players can choose higher. The existing `display` value follows the
monitor's refresh without a cap. Proposed `auto` (new Modernized default):
the display's refresh rate when it is 120 Hz or less; on faster monitors the
largest rate at or below the cap that divides the refresh evenly, so every
picture still lands on a refresh (240 -> 120, 144 -> 72 or 144 (to be
measured and decided), 165 -> 82.5 or 165). Explicit values stay available.
Re-evaluate when the display changes (window moved, fullscreen).

**Acceptance:** `auto` picks the documented rate on 60, 120, 144 and 240 Hz
modes (measured, plus unit tests of the rule); existing explicit choices are
kept; the user judges the default on their display.

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

**Accepted (user playtest, 2026-10-04).** After substantial play the user
reported a major improvement to the zigzag/vibrating texture effect, most
obviously in UI slot 12 and across the game: "The result is excellent." (The
user's message labelled this job D17P; it is D17N. D17P remains the earlier
accepted draw-distance job.) Accepted executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Do not extend D17N; remaining subtle flicker is D17Q.

Candidate record: Cause: the original world renderer sends
every polygon within `0x2000` units to a screen-space subdivision
(`0x80012960`/`0x8001205c`) whose midpoints are integer screen averages with
rounded UVs and no projection data, so D15's perspective-correct path skipped
them and each piece was drawn affine; the bends follow piece diagonals and move
as the eye moves. This is general, not slot 12 specific. With Corrected texture
precision, polygons whose corners all have exact projections and fit the GPU
limit are now drawn whole (new hooks `0x800114EC`/`0x8001160C`, guarded by the
existing renderer digest). Offscreen evidence: straight panel borders across
120 Hz walking sequences; 12-slot sweep with exact-projection share 0.29-0.43
rising to 0.73-1.00, lower packet use, unchanged game rate, zero budget hits or
replay misses; Original textures and Vanilla unchanged. Tests pass. Candidate
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`. User to
confirm on W/S routes in UI 12 and elsewhere. See
[cause, implementation and evidence](documentation/98-d17n-world-subdivision.md).

### D17Q - Residual wall/surface flicker: Corrected-renderer stability polish

**Done, user-accepted (2026-10-04).** After extended play the user wrote
"the whole job is 100% accepted", and praised how cleanly the game now
plays. Accepted executable
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960` is the new regression baseline. Candidate record follows; see
[cause, fix and evidence](documentation/99-d17q-uv-seams.md). The remaining diagonal line
(UI 12 panel at the crosshair on load, flickering on W/S) was a one-texel UV
seam. The GL/VK backends applied the 2D mirrored-sprite UV bump per triangle,
and a wall triangle with an exactly vertical integer edge qualified while its
partner did not, so the two halves of one polygon sampled one texel apart. As
movement changed the integer corners, the seam toggled. Positions, depths and
UVs were verified consistent; offline exact rendering is seamless. Fix:
perspective-corrected 3D triangles use full UV limits and no sprite bump
(`gpu_uv.h`, GL and VK; `PSX_UV_3D_LEGACY=1` compares). Original textures and
Vanilla are pixel-identical; 12-slot 120 Hz sweep and 60/180 Hz: zero replay
misses; tests pass; codegen hash unchanged. **User to confirm** steadier walls
on W/S routes in UI 12 and elsewhere (confirmed). Originally requested by the
user after accepting D17N. With the systemic
subdivision zigzag gone, some walls and surfaces can still show subtle
flickering or residual instability during movement. This is final polish, not
a reopening of D17N.

**Objective:** investigate the remaining wall/surface flicker and determine how
close Corrected geometry and textures can get to genuinely stable rendering
during movement, while Original textures, Vanilla and the software renderer
keep the original PS1 behaviour.

**Investigation:** do not assume D17N's cause. First establish, with matched
private captures and primitive/packet evidence, what actually moves or changes
between frames on affected surfaces. Candidate directions only, not diagnoses:
residual PS1 coordinate quantisation, geometry transformation, clipping and
near-plane handling, polygons still on the subdivided/fallback path (the D17N
`subdivided_kept` counter and polygons without exact projections), vertex and
depth precision, texture coordinates (including integer UVs and texel
snapping), culling/visibility, high-refresh interpolation and other renderer
behaviour. Reuse the method and history of
[D17N](documentation/98-d17n-world-subdivision.md),
[D17P](documentation/97-d17p-distant-bands.md),
[D15](documentation/96-d15-accepted-precision.md),
[R01](documentation/93-disruptor-reference-research.md) and the D17A/D11B
clipping notes where relevant.

**Acceptance:** identify the remaining sources with evidence; implement bounded
Corrected-path improvements where justified, or document unavoidable limits.
Verify 60/120/180 W/S and turning routes across the private slots with no
game-rate, budget or replay-miss regressions, preserve the accepted
D15/D17N/D17P and D17C/D/E/F/K/L/O baselines and the Original/Vanilla paths,
and the user confirms steadier surfaces in play.

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

### D17R - Sky turns black toward the edges when looking up (game-wide)

**Todo. User report, 2026-10-04 (D22A playtest).** UI slot 9: look up. The
central sky renders correctly, but toward the left and right edges of the view
it turns black/dark, as if the edges had become night.

Reproduction saves are the player's own UI slots (savestate files 07/08 in
`recomp/saves/local-play/openbios`, written 2026-10-04 18:01/18:04): UI slot 8
= file 07 SHA-256 `7699132e7e6df9ac13e8925fe5240656a2002ac2c90aeece7b0c174b054dc119`,
UI slot 9 = file 08 SHA-256 `f88cd1a65e67ed0cd59f3c811fb54e52222db6e8dfd210dd30a80929340e5bb6`,
both in LEVEL01 (the Old West map the user calls Level 2). Test only on dated
private copies; verify these hashes first, since slot numbers get reused.

Investigate the underlying sky rendering, not a level patch. Directions, not
assumed causes: the original sky/background geometry and coverage, widescreen
and FOV expansion (D14), clipping, projection, culling, geometry limits and the
original renderer's assumptions about the visible horizontal field, including
our first-person presentation. Reuse D17O ([note 92](documentation/92-d17o-sky-intake.md)),
R01 and the renderer notes.

**Also seen, 2026-10-05 (user, after D22C):** UI slot 2 (savestate file 01,
SHA-256 `756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`,
written 2026-10-05 09:10), first medieval level: "strange behaviour with the
sky that's directly ahead of you". The user suspects the same cause as the
black peripheral sky and asked to record it here rather than as a new job.
Confirm whether it is the same symptom (edges going black) or a different
sky artifact before assuming one cause; split it out if it is not.

**Also reproduced, 2026-10-05 (user, D22B game-wide playtest):** the Roman /
HOG HEAVEN environment (`level 10`) shows the same black sky toward the
peripheral edges of the view. The problem is game-wide, not specific to
LEVEL01's sky.

**Acceptance:** slot 9 and HOG HEAVEN show a continuous sky to both edges in 16:9 and the
original aspect, first and third person; the first map's accepted sky (D17O
slot 10) is unchanged; the solution is shared sky rendering that holds across
levels; the user confirms.

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

### D18D - Music silent after death and Continue until Duke's next voice line

**Todo - user report (2026-10-04).** After Duke is killed and the player
chooses Continue, level music does not resume. It only starts once Duke says
a voice line. Check whether Vanilla behaves the same and whether original
hardware/DuckStation (portable copy only) does too, before assigning cause.

**Investigation:** trace the Continue/respawn path for CD-XA music restart,
SPU/CD mute or volume state and the voice-line path that apparently restores
it. Distinguish an original-game behavior from a recomp/host audio defect.

**Acceptance:** after death and Continue, level music resumes promptly in the
tested levels without waiting for a voice line, if reference behavior shows
that is correct; no regression to voice playback, FMV audio, D18A/D18C
crackle routes or pause/menu audio. Use private save/card copies only.

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

**Accepted (2026-10-04, user playtest).** "Level 2 is working and is completely
playable. The teleport/level transition is functioning correctly." Cause found
and fixed: the control lease
authenticated only the LEVEL00 overlay body; the portal loads LEVEL01.OVR over
the same base, so it failed closed. LEVEL01 is now an authenticated level
overlay alongside LEVEL00. See the work log and
[note 100](documentation/100-d22a-portal-level-identity.md).

User report on 2026-10-03:
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

### D22B - Modern controls and first person in every level and transition

**Accepted (2026-10-05, user: "Accepted.").** The user then played large parts
of the game through `level N`: armed rolls (Ctrl) in Level 9 work, and general
game-wide playability is "very encouraging". Executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the
regression baseline. Candidate summary: all 21 selectable levels now authenticate
(one rule derived from the owned disc by `tools/local/level_overlay_guards.py`);
dodge rolls and steep-slope slides keep the mouse camera and view; the
original's own back-steps/strafes are taken over by the lease; unowned jumps
keep the camera in third person too. Transitions (statistics screen, savestate,
pause, level select) return by themselves. Swimming first person is D11E.
See the work log and [note 102](documentation/102-d22b-every-level.md).

Selected as the follow-up to D26A, 2026-10-04. User, on
accepting D26A: "the modern controls dont carry over, and first person etc. so
stand up the next job ... as making all that work." D26A now reaches every
level: `levels` / `level N` in the backtick console. Offscreen survey
(documentation/101): Modernized controls and first person are live only in
LEVEL00 (TIME TO KILL) and LEVEL01 (DUKE HILL). In the other 19 selectable
levels (2, 3, 5-12, 21-29) the lease refuses (`[TTK identity] level overlay tag
... is not an authenticated level`, `fp=lease`), so the original pad controls
and third-person view return. Scope includes the selected first-/third-person
view, mouse look, view aiming and the Modernized conveniences, not only
movement.

Earlier direction, 2026-10-04: "There should basically be no situation
where the classic controls override modern controls ideally. We want full,
total modernization, and full ownership of the controls system end to end."

D22A made the lease accept one more authenticated level (LEVEL01). Every other
level, and every state the lease does not yet cover, still drops back to the
original pad layout. This job removes that as a normal player experience:

- Level coverage: verify each LEVELxx.OVR (load base, tag word, run-time data
  such as the LEVEL00/LEVEL01 scratch tail) and authenticate it in
  `level_overlays[]`, or replace the per-level allowlist with an equally strict
  but general rule derived from the owned disc. Use D26A to reach each level.
  Keep the fail-closed principle for genuinely unknown code; the goal is to
  verify it, not to bypass it.
- State coverage: inventory every gameplay state where the lease refuses
  today (for example ladders, scripted cameras, vehicles/turrets, cutscene
  hand-offs, death/Continue, level start/end) and give each a modern
  equivalent or a documented reason it must stay original.
- Transitions: level loads, portals, Continue, savestate loads and menus
  return to the same Modernized controls and view without player action.
- Diagnostics: a lease refusal in normal play is a bug to report with its
  reason (`[TTK lease]` / `[TTK identity]`), not a silent fallback.

**Acceptance:** in every level `levels` lists, reached with D26A and through
the natural transitions, Modernized controls (movement, mouse look, aiming,
fire, jump/traversal) and the selected first- or third-person view stay in
charge from level start to level exit, including transitions and savestates; any remaining original-control state is listed with its reason
and a follow-up job. Vanilla stays original. Fix shared systems, not
individual levels, wherever the evidence allows.

### D22C - Level 11 starts with Legacy controls (control mode must persist)

**Accepted (2026-10-05, user: "I accept this work").** Earlier: Needs playtest. Reproduced and fixed; not specific to Level
11. An F10 release followed by an F10 recapture left automatic capture opted
out, so the next host release (opening the backtick console for `level N`,
menus) arrived in the new level with the mouse free and the original
controls until F10. F10 that captures now opts back in. Candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`. See the
work log.

User report, 2026-10-05 (D22B game-wide playtest): loading Level 11
(LET THE GAMES BEGIN with `level 11`) unexpectedly started in Legacy controls;
F10 restored Modern controls. Level transitions and the debug level select must
not change the player's selected control scheme. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Investigate the reset path: level or player-state initialization, mouse
capture (F10 is the capture toggle, so check whether capture was released
rather than the mode changed), savestate/level-select behaviour, configuration
reload, or a lease refusal specific to that level. The D22B offscreen survey
reached Level 11 with the lease live, so reproduce with the user's real
window/focus path too.

**Acceptance:** Level 11, reached by `level 11` and by natural progression,
starts in the selected control mode with capture as before; other levels
unchanged; the user confirms.

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

### D23C - Medieval castle moat slowdown with Necros (performance polish)

**Todo, later. User report, 2026-10-05 (D22B game-wide playtest).** In the
medieval castle/moat level (reported as Level 6), the initial section slows
down, particularly while the Necros around the moat are active. Gameplay is
not broken and stays completely playable, but the quality drop is perceptible
compared with better-performing areas. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Do not optimise blind. Profile first and determine whether the slowdown
correlates mainly with the number/type of active enemies, Necro AI or
animation, visibility/render workload, original simulation workload,
effects/projectiles, high-refresh interpolation, or another system. Reuse
the D23A/D17 frame-budget tooling (`phase_profile`, `phase_hot`, fps stats).

**Acceptance:** a measured profile names the dominant cost; a fix, if any, is
shared and verified against the accepted baselines; the user confirms.

### D23D - Minor slowdown in Level 9's strip-club-type area (low priority)

**Todo, low priority. User report, 2026-10-05.** Level 9 plays very well
overall; looking closely, there is a small slowdown around the strip-club-type
area. Extremely minor and not blocking. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering. Profile with the D23C
method when performance polish is scheduled.

### D23E - Stutter with enemies on screen while walking (UI slots 9 and 10)

**Accepted 2026-10-07** (user: these are "now working significantly better"). **User report, 2026-10-05.** Load UI save slot 9 (file 08), look down
toward the enemies on the ground, then walk left and right: the game stutters
in this area. The user suspects the characters on screen cause it. UI slot 10
(file 09) may show the same effect: turn on `dnkroz` first because enemies
attack immediately, then walk up and down the strip. Reported savestates
(`recomp/saves/local-play/openbios`, 2026-10-05; copy them for tests, never
write to the originals):

- UI 9, `state_800AB6FC_slot08.pst`, SHA-256
  `3fe93bb39eb114f5c22d349ed030a68a14b14b1cfac86b336b762cd507525c8f`
- UI 10, `state_800AB6FC_slot09.pst`, SHA-256
  `c1dc55452203ab3864da17a440278bded255608679698604d92aa5959d09ceaf`

Record which level each slot is in. Possibly related to D23C (moat Necros),
D23D (Level 9 strip-club area) and D17H (platform movement stutter): check
whether this is the same cause before fixing anything separately, and merge
or cross-reference the jobs if it is.

Do not optimise blind. Profile with the D23C method (`phase_profile`,
`phase_hot`, fps stats) and determine whether the stutter follows the number
or type of visible characters (skinning, animation, AI), render workload,
original simulation time, or high-refresh interpolation/presentation. Compare
the same route with the enemies looked away from, with `dnmonsters` off, in
Vanilla and Modernized, and at 60 and 120 Hz.

**Acceptance:** a measured profile names the dominant cost on both routes; any
fix is game-wide (not per level), keeps original gameplay timing and passes the
accepted regression baselines; the user confirms the stutter is gone or
acceptably reduced.

**Work log 2026-10-05 - Needs playtest.** Both slots are in the western town.
Measured offscreen on the real GPU with the player's settings and private
copies ([note 104](documentation/104-d23e-busy-scene-stutter.md)). Two causes:
(1) at the player's `cpu_overclock 100` the 16:9 view gives the emulated CPU
more than a 3-field frame's work, so game frames land on 4 fields (15 fps)
with 5-6 field steps (Vanilla 4:3: steady 3 in slot 9; slot 10 is 4-5 in
Vanilla too); draw distance has no effect. (2) At 120 Hz the default 150%
could not hold: late-camera redraws (120/s, about 1.4 ms each, 19% of the
emulation thread) pushed emulation behind real time, and the D08Y safety net
paused the overclock for 5 s at a time (20 <-> 15 fps swings). At 150% and
60 Hz both slots hold a steady 3. Fix (game code only): the redraws shed load
first (`replay_shed_load`: presents every 2nd/3rd refresh, existing step-up
rule), and the overclock pauses only when nothing is left to shed. 150%,
120 Hz: slot 9 from about 30% to 93% of frames at 2-3 fields, slot 10 98% at
3 over 90 s; light slots unchanged. Native tests pass. Candidate
`18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`.
Remaining: the user plays slots 9 and 10 at 150% (and judges the lower present
rate there) and real audio; at 100% the stutter is the emulated CPU itself.
Not shown to be the same cause as D23C/D23D/D17H; measure those the same way.

**Follow-up 2026-10-05.** The user confirmed 150% relieves the stutter but
prefers 100% for now: 150% felt different, with more visible seams and a
slight audio degradation at the start. Measured: identical still images and
wall subdivision at both speeds; at 150% and 120 Hz the picture rate drops in
light scenes too (slot 1 75/s, slot 9 54/s against 120 at 100%), because one
emulation thread cannot do both. That work moved to D23F. D23E stays Needs
playtest at 100% (its shedding applies whenever emulation falls behind).

### D23F - Faster timing model: emulation-thread budget for 150% at high refresh

**Accepted (2026-10-05).** User: "the behaviour is amazing. the behaviour is
absolutely beautiful, and a real pleasure to play ... the responsiveness is
literally like PC accurate now", and asked to make it "the entire new default
method of launch". `cpu_timing` defaults to fast from profile schema 26 (older
saved `accurate` switches once; Vanilla stays accurate). Follow-up work is
D23G.

Implemented as a Modernized profile setting `cpu_timing` (`run.py --cpu-timing
fast|accurate`):
a purpose-built timing model for the recompiled game code, force-included
into the generated shards (generated C and hashed headers untouched, so
savestates still load), leased from gameplay like the overclock. At 150% and
120 Hz slots 1, 9 and 10 keep 120 presents per second with no shedding or
overclock pauses; slot 9/10 60 s routes steady 3 fields. See
[note 105](documentation/105-d23f-fast-timing.md) and the work log. Remaining
for acceptance: the user's playtest of feel and audio, and the decision on
the default.

**Opened 2026-10-05 from D23E.** The user wants the best gameplay
as the default and likes the faster timing model concept from the Disruptor
reference ([note 93](documentation/93-disruptor-reference-research.md),
section 9). They play at 100% CPU for now: at 150% the game felt "different,
not smoother" and seams looked more visible.

D23E evidence ([note 104](documentation/104-d23e-busy-scene-stutter.md)):
150% draws pixel-identical still images and the same whole/subdivided wall
counts as 100%. What changes is the picture rate: on the player's PC
(i7-5960X, GTX 1080 Ti) one emulation thread cannot run the faster game
(more game frames per second) and 120 in-between pictures at once, even
walking in a light scene (slot 1: 120 presents/s at 100%, 75 at 150%; slot 9:
120 against 54). Either presents drop (D23E shedding) or, before D23E, the
overclock paused 5 s at a time.

Emulation thread at 150%, moving in slot 1 (`PSX_PROF`): cycle-timing model
about 34% (`psx_cyc_base`, `psx_cyc_lds`, `psx_cyc_charge`, `psx_cyc_step`,
instruction-cache simulation, dependency tracking), interrupt checks about 4%,
the recompiled game code 4%, GL driver and redraw/batch paths about 12%,
always-on observers (call/trace logging, `debug_server_cyc_observe`,
`fntrace_record`, `xprobe_event`, parity/overlay watches) about 4-5%.

Work, in order:

1. **Observers off in player sessions** (small, low risk): find which
   per-store/per-call observers still run with `PSX_FORENSICS=0` and gate them;
   helps 100% too.
2. **Faster timing model** (the core): a design study before code. Disruptor's
   `disruptor_fast_timing.h` levels: no per-block diagnostics, no game
   instruction-cache simulation, static per-basic-block cycle charges, flat
   charges for eligible RAM loads, an inlined branch-edge check that services
   interrupts and accumulated charges at a threshold, explicit fallbacks
   (device/MMIO/BIOS, polling edges, save/debug maintenance). Decide what is
   computed at recompile time versus runtime, how it coexists with the TTK
   overclock lease (`psx_overclock_compress`), render workers
   (`g_psx_render_untimed`) and idle skipping, and whether it changes hashed
   framework headers or generated code identity (savestate compatibility: see
   the codegen-hash rule; the user accepts losing savestates for this job). Framework edits go through
   `export_runtime_patch.py`.
3. **Cheaper in-between pictures** (optional): fewer GL state changes per
   redraw feed, or drawing redraws off the emulation thread.

User direction (2026-10-05): "im completely happy to build anything new rather
than patch over old crap". Prefer a clean, purpose-built timing system over
layering more patches onto the existing cycle model, even when that is larger;
keep the framework patch export workflow. Savestates: "i genuinely dont mind
if we lose save states" (2026-10-05), so D23F may change hashed framework
headers or generated code identity; say so when it happens, and tests still
use private copies (never the player's files).

Vanilla keeps the accurate model unless the user decides otherwise; the fast
model is a Modernized option until proven.

**Acceptance:** at 150% and 120 Hz the player's PC keeps 120 presents per
second in light scenes and the western town (slots 1, 9, 10) without shedding
or overclock pauses; game frames at least as steady as D23E's 60 Hz result
(slot 9 steady 3 fields); audio, FMV, loading, CD streaming, savestates and
the accepted regression baselines unchanged; Vanilla untouched; the user
judges the feel at 150% and decides whether it becomes the default.

### D23G - Finish the fast path: dispatch, overlays, interpreter, observers and redraw cost

**Todo, all-in-one. Opened 2026-10-05 from D23F's limits** ([note 105](documentation/105-d23f-fast-timing.md)).
D23F moved the statically recompiled game code to the fast model. What is
left on the emulation thread (fast, slot 9, 150%): the call/return dispatch
path about 13% (`psx_dispatch_impl` -> BIOS table miss -> `dirty_ram_dispatch`
-> `psx_game_find_entry`, `dirty_ram_text_native_ok_ranges_from`,
`dirty_ram_is_dirty`, plus `fntrace_record`, `xprobe_event`,
`text_xlate_on_dispatch` on every hop), GL driver and redraw feed about 5-12%,
level overlay libraries about 2%, the interpreter about 5%.

Work, measured one by one with the D23F harness:

1. **Dispatch fast path**: a purpose-built lookup for clean game text
   (direct table or cache keyed by address, invalidated by the existing
   dirty-RAM/overlay generation) so a call or return does not walk the BIOS
   table and the dirty-RAM checks. Framework change through
   `export_runtime_patch.py`; keep identical targets (counter checks).
2. **Dispatch-path observers off in player sessions** (`fntrace_record`
   rings, `xprobe_event`, parity/overlay watches that are diagnostics),
   still available with `PSX_FORENSICS` and armed traces.
3. **Fast timing for level overlay code** (compiled overlay libraries) and,
   if measurable, the dirty-RAM interpreter, with the same lease and the same
   calibration.
4. **Cheaper in-between pictures**: fewer GL state changes per redraw feed or
   feeding redraws off the emulation thread.
5. **Vanilla host cost**: the D23F flag tests cost Vanilla about 3 points of
   host CPU (guest behavior identical); remove if a clean design allows.

Also investigate why the Vanilla regression route now diverges from its
long-standing captures with or without D23F (last matching run before D23E).

**Acceptance:** each step shows a measured emulation-thread saving with game
frame rate, savestates, audio, FMV and loading unchanged; Vanilla guest
behavior unchanged; the user's playtest finds no regression and the same
feel.

### D23H - Presents fall from 120 to about 60 over extended play

**Accepted (2026-10-06).** User, after playing with several saves: "i fully
accept ... this is absolutely wonderful work." See the work log below and
[note 106](documentation/106-d23h-present-rate-recovery.md).

**Opened 2026-10-06 from the user's evening play on the D23F default
build** (fast timing, 150%, 120 Hz, executable `e0737a9f...`). User report:
"after like 10 mins of gameplay, the frame rate goes from the buttery smooth
120 to what feels/looks like a lower one, like 60 or something." It does not
come back by itself during play.
After reviewing the log evidence, the user agrees the drop follows savestate
use (2026-10-06).

**Log evidence** (`recomp/build-local/logs/`, local only; the launcher keeps
the last five sessions):

| Session | Length | Saves | Loads | Presents reduced | Overclock paused 5 s |
| --- | --- | --- | --- | --- | --- |
| `session-20261005-233139` | about 26 min | 4 | 0 | 2 | 2 |
| `session-20261005-235829` | about 25 min | 6 | 2 | 2 | 5 |
| `session-20261006-002316` | about 2 min | 1 | 1 | 1 | 0 |
| `session-20261005-212814` | about 11 min | 0 | 3 | 1 | 0 |

- All 13 slowdown events in these sessions come right after a savestate save
  or load. There is no other `emulation behind real time` line. The measured
  host rate in that second is 6.6-32.3 frames/s, so a save stalls the
  emulation thread for a large part of a second. Loads report about 17 ms of
  work, so the save side (compression or file write on the emulation
  thread?) is the suspect. Not measured yet.
- The game logic rate itself holds: the `[FPS]` lines in `233139` show a
  steady 59.9 game fps (1.00x) between events. The drop the user sees is in
  presents (in-between pictures), not in game speed.
- Presents per second are not in the session log, so the "about 60" has not
  been measured directly. It is consistent with a present divisor of 2.

**Likely mechanism (from code reading; not yet reproduced):**

1. `overclock_lease()` (`src/ttk/modern_controls.cpp`) sees the save hitch as
   "emulation behind real time" and calls `replay_shed_load()` (D23E), which
   raises the present divisor from 1 to 2 (120 to 60 presents/s at 120 Hz).
   If it is already at its limit, it pauses the overclock for 5 s instead.
2. Stepping back up (`late_pace()`, `src/ttk/frame_replay.cpp`) needs
   `4 * pace_backoff` consecutive clean windows of 90 presents (under 2%
   repeats). Any window between 2% and 10% resets the count.
3. `pace_backoff` doubles (up to 32) whenever a slowdown follows within 3
   windows of a step-up, and **it never decays**. One-off hitches are
   treated the same as a busy scene. After a few saves in a session,
   recovery needs up to 128 clean windows: at 60 presents/s that is over
   3 minutes of perfectly clean play, and busy scenes rarely manage that.
   So the game appears to stay at 60.

The D23E comment says "a one-off hitch ... costs a few seconds", which holds
only for the first hitch of a session.

**Other causes to rule out** before settling on the above: a slow time-based
growth (memory, GL objects, worker processes, the trace or frame rings,
savestate buffers), thermal or GPU clock drops, and the separate
`late_pace()` step-down when repeats exceed 10% (not logged at all).

**Work:**

1. **Measure first.** Log present-rate changes (`pace_div` up or down with
   the reason: emulation shed, repeats, recovery) and a one-line summary
   every minute or so (presents/s, game fps, `pace_div`, `pace_backoff`, RSS)
   in the player session log, cheaply. Reproduce with a private profile copy
   and test card/state copies: play 15-20 minutes offscreen with periodic
   saves, and a control run with no saves. Confirm whether presents stay
   at 60 and whether the decline occurs without saves.
2. **Savestate saves must not stall gameplay**: take the snapshot on the
   emulation thread (the copy only) and compress and write it on another
   thread, or otherwise show where the time goes. Saves, loads and their
   rejection rules must not change (hashed headers untouched).
3. **Shedding must recover from one-off hitches**: ignore or discount the
   seconds around a known hitch (save, load, level load, worker start), and
   let `pace_backoff` decay with clean time so the session's history does
   not keep 60 forever. A genuinely busy scene must still settle on one
   steady rate (the D23E aim).
4. If step 1 shows a slow time-based decline as well, find it and fix it in
   the same job.

**Acceptance:** in a 20+ minute Modernized session at 150%/120 Hz with
several saves and loads, presents return to the display rate within a few
seconds of each save or load and stay there in scenes that held 120 at the
start. The busy western town (UI slots 9/10) still settles calmly as in
D23E. Savestates stay compatible, there is no audio regression, Vanilla is
unchanged, and the user's long play session confirms it.

**Work log 2026-10-06 - Needs playtest.** Measured offscreen (private profile
copy at the player's settings, private states, port 9273; harness
`recomp/analysis/d23h-20261006/`). The suspected `pace_backoff` was not the
cause. **A shed was permanent:** at 120 Hz and Match Display the default
present timeline never calls `late_pace()` (only at Unlimited), so nothing ever
undid a D23E shed. One hitch gave 60 presents/s and two gave 40 until restart.
**One-off hitches shed:** a save stalls emulation 83-98 ms (loads 36-57 ms), and a
second containing an F7 menu visit is far lower, so any single bad second shed.
Fix (game code only, `src/ttk`): `replay_load_window()` owns shedding in both
directions, judged once per gameplay second by `overclock_lease()` in every
frame-rate mode. It sheds only after two behind seconds in a row, does not judge
gaps (menus, loads) or seconds while the overclock is paused, steps up after
5 s x backoff clean seconds, doubles backoff on a failed try (max 16), and halves
it per clean minute at full rate. Added `[TTK pace]` session-log lines (each
change with its reason; a per-minute summary with presents/s and RSS) and
`render_replay` fields. Evidence: A/B with a 0.8 s stop every 20 s and a save
every 30 s: before 120 -> 60 -> 40 for good, with 5 overclock pauses; after
120 throughout. 21-minute run with 20 saves, 8 loads and 13 stops: 120 in 124
of 125 ten-second blocks; the one shed (a stop and a save in the same second)
recovered in 5 s. A 15 s sustained load returns 40 -> 60 -> 120 about 15 s after
it ends. Western town UI slots 9 and 10 with hitches: 119-120, no change.
Unlimited unchanged (repeat pacing only). Native and unit tests pass. No framework or
hashed header change (savestates compatible); Vanilla does not reach this code.
Executable `c6c221d1...`. Remaining: the user's long play session; Unlimited's
own repeat backoff still does not decay; saves still cost about 90 ms on the
emulation thread (no longer affects the rate); RSS rose 400 -> 466 MB over 21
min (rate unaffected, not investigated).

### D24 — Linux / Windows player build and disc import

Create reproducible player builds with a simple launch flow, settings/save locations and clear owned-disc import errors. Verify Windows independently rather than extrapolating from Linux. Package permitted runtime components; keep original disc assets and personal saves out of redistributable artifacts.

**Acceptance:** a clean installation on each claimed platform imports the supported disc, launches, saves/loads and closes successfully. Document dependencies, licenses, build identity and known limitations in the player instructions.

### D24A - Public clone gives the full experience (assets out of research/)

**Accepted 2026-10-07** (user: "now working significantly better"). **No Duke Nukem 3D art remains** ([note 112](documentation/112-d24a-open-assets.md)). The user confirms the look; the fresh-clone test is re-run after commit. Route (a) chosen, all Duke 3D art removed.
Art direction (2026-10-06): the user will hand-make original, openly licensed
icons in the TTK style for mission items, keys/key cards and steroids. The disc
has no 2D art for those. Fonts, digits, buttons and HUD sprites remain disc-derived.
User: "We are going fully open, so the duke fonts must be removed. we need to
replace them with 1:1 analogues." Font picker built; waiting for the user's
font/palette choices per use (see work log).

**User request, 2026-10-06.** "the font assets etc need to be moved into
a proper director, not in the research. i need to make this whole project
available online for anyone to download and play!"

**Evidence (2026-10-06 fresh-clone test).** A clean `git clone
--recurse-submodules` of `main` (5f9e705), plus an owned USA disc in `game/`,
built with `build.py` (exit 0). All 69 generated files were byte-identical to
the working copy, as were `recomp/src` and the patched framework. Jetpack,
fire, landing, fuel and Vanilla checks gave the same results as the working
build. Missing from the fresh build (present only in the ignored
`recomp/assets/` of the working copy, built from local `research/`):
- `ttk-inv-icons.pack` (+ provenance): D08A3 inventory icons, extracted from
  the TTK disc's `/DATA/FONTS.RAW` by `build_ttk_inv_icons.py`.
  `build.py` never runs that step.
- `ttk-inv-digits.pack` (+ provenance): the D08A2 green inventory digits and
  %, from Duke Nukem 3D art tiles under `research/inv/font/`.
- `fonts/Messages` (+ provenance): the D19A Duke message font, from Duke Nukem
  3D / Atomic Edition tiles under `research/`. Without it the generic host
  font is used.

**Scope.**
1. Disc-derived assets: `build.py` extracts `ttk-inv-icons` from the user's
   own disc (`/DATA/FONTS.RAW`) automatically and deterministically. The output
   is ignored build/asset output, never committed, and gets a clear error if
   the disc lacks it.
2. Fonts and digits: move every build input out of `research/` into a proper,
   documented project location that the build consumes. **Licensing
   constraint:** the current glyphs and digits are Duke Nukem 3D art, and
   AGENTS.md forbids committing extracted retail assets. Decide (with the
   user) before committing any of them:
   - (a) derive equivalents from the TTK disc at build time (TTK's own fonts
     in `FONTS.RAW`), so every owner gets them;
   - (b) an original, freely licensed font/digit set made for this project and
     tracked in the repo;
   - (c) an optional "supply your own Duke Nukem 3D files" import with a
     documented path, falling back to (a) or (b).
   Do not publish Duke 3D-derived art without a licence that permits it.
3. `check_repo.py` and the docs updated so the build never reads `research/`,
   and the README states exactly what a clone plus a disc gives.
4. Re-run the fresh-clone test (empty folder, README steps only, owned disc)
   and confirm that fonts, icons and digits are present and match the
   accepted look.

**Acceptance:** a fresh clone, plus the README steps and an owned USA disc,
produces a build with the message font, inventory icons and digits (by the
chosen route) and no dependency on `research/`. No retail or third-party art
is committed without a permitting licence. The user confirms the look.

### D24B - Savestate menu (F7) dressed in the TTK fonts and disc art

**Accepted (2026-10-06): "im happy with all progress tonight"; built to the approved mockup** ([note 113](documentation/113-d24b-savestate-menu.md)). User: "i was also wondering if we
could dress up our save screen while we have the fonts open too". Asked which
screen; the user chose the recomp savestate slots (F7).

Proposal: the font picker's "Savestate menu (F7) redesign - mockup".
- Header: "SAVE STATES" in TTK Big Italic gold.
- Slot cards: thumbnail; slot title and level name in TTK Medium Italic gold
  (blue when selected); date and level line in System 8x8.
- Prompts: the disc's button sprites with Medium Italic labels.
- Backdrop: dark steel with the disc's radiation emblem.
- Text drawn at 1x with the navy shadow.
The panel is the framework's `psx_savestate_menu.c` (640x480). The plan is a
game-specific render hook (the framework panel stays the fallback, and Vanilla
gets the same look or keeps the original, to be decided), plus reading the level
index from each savestate for its name.

**Acceptance:** F7 shows the approved design with correct slot data, load/save
and navigation unchanged; the fallback works without the font pack.

### D24C - TTK-font exclamation mark and `>` console prompt

**Done (user-accepted, 2026-10-08): "all accepted".** User: "in the case of cheats like dnstuff which
gives the message "giving everything!" we dont have an exclaimation mark in this
font ... I'm thinking we could fake an exclaimation mark with the I character and
the period. I want the console prompt character itself to be a > rather than a |
also." Two small, related font jobs done together at the user's request.

- The TTK Big and Medium Italic fonts have no `!`, so it fell back to the 8x8
  system font. `build_ttk_fonts.py` now draws one from each font's own `I` and
  `.`: the I's top as the stem, tapered towards the bottom (Big 3 px, Medium
  2 px) and closed with the I's own bottom edge moved up the slant; a clear row;
  then the period minus its top rows (Big 2, Medium 1) on the baseline. Every
  set built from those fonts (messages, headings, console, savestate panel
  titles and slots) gets it.
- The console prompt and the echoed command lines use `>` (a native Medium
  Italic glyph) instead of `]`, which the font lacked and fell back to a
  thin system-font glyph that read as `|`.

**Acceptance:** `dnstuff` shows GIVING EVERYTHING! with an exclamation mark in the
message font; the console prompt and command echoes start with `>`.

### D25 — Modernized edition release acceptance

Run the documented campaign and regression checks against both presets in a frozen candidate build. Reconcile manual, settings, supported platforms and feature claims. Choose release scope explicitly; optional first-person or HD packs need their own completed acceptance work to be advertised.

**Acceptance:** no known release-blocking progression, input, save or playback issue in the claimed scope; comparison evidence shows Vanilla preserved and Modernized usable; reproducible build and player instructions are complete. Record remaining issues visibly.

### D26 — Backtick debug console (fps and helpers)

**Done — explicit user acceptance (2026-09-28).** Console look and behaviour signed
off: scrollback echo, multi-line `help`, unknown/errors in console only, `fps`
persistent top-left debug block, `clear` / `quit`. F unbound; Scroll Lock is holster.

**Acceptance met:** backtick open/close; scrollback; persistent fps overlay;
gameplay/capture/menus intact.

### D26E - Debug spawn console command

**Accepted (2026-10-06): "im happy with all progress tonight".** User: "is there a command we can use that can
summon one of these items in front of duke? for debugging purposes", then "yes
spawn indeed", pointing at EDuke32's `spawn`
(https://wiki.eduke32.com/wiki/Spawn). For the D24A icon work.

`spawn <item|type>` and `items` in the backtick console (Modernized only).
The item is created about 600 units in front of Duke by the original
`CreateObject` (`0x80095a74`), the routine a destroyed container or enemy uses
to drop its item. Types whose model the current level has not loaded are
refused. Details: [note 111](documentation/111-d26e-debug-spawn.md).

**Acceptance:** in Modernized gameplay, `spawn` puts a named or numbered item in
front of Duke, it is picked up normally, and refusals are clear (unknown item,
not in this level, Vanilla, not in gameplay). Vanilla and the level select are
unchanged. The user confirms it is useful for the icon work.

### D26F - Spawning in-game items, continued (mission items)

**Done on user acceptance (2026-10-07): "this is awesome. it works! the
crystals really do work."** Built as approved: `spawn 1761` / `2761` /
`3761` drop the real red / blue / green crystal, collected by walking over it.
Also the project's first custom-pickup contract (user: "the wider effect here
is we're now about to learn how to make custom pickups. so document this
heavily"): [note 117](documentation/117-d26f-crystals.md), "Custom pickups".
Research, as first logged: User: "in the first level (which you can easily reach by going to slot
4), can you check why spawning the green crystal doesnt show it in the
inventory? spawning the subway key on the other hand does show it in the
inventory. My guess was that the crystals are all palette swapped versions of
the green crystal and the in game items have special properties", then "we can
stand this up as a logged research job too. 'ability to spawn in game items
continued' or something".

**Findings (verified).** See [note 117](documentation/117-d26f-crystals.md).
- `spawn green energy crystal` makes type 761, a walk-over pickup whose original
  dispatcher case sets inventory item 14 (`+0x38c`) and shows message 115
  "GREEN ENERGY CRYSTAL". The level's crystals are items 11-13, so nothing named
  appears. In level 1 item 14 is the Scrap of Paper and in level 6 the first
  Family Jewel. Where a level loads 761's model, the same spawn would mark
  those as found (not tested).
- The real crystals are different objects: types 176 (red), 177 (blue) and 178
  (green). They are not walk-over pickups. Each sits in a holder (types
  180/181), and Duke takes it with the action button. The original routine
  `0x80091cec` puts it in his hand and then sets item 11, 12 or 13.
- Palette swap: correct. The three crystal models are the same mesh and texture
  page. Only each face's CLUT differs. Type 761 uses the green crystal's model.
- The subway key (153) works because its dispatcher case sets item 6, which
  level 0 names.

**Approved design (2026-10-07, user: "that's how it should work, i love it!").**
Spawnable mission crystals. It combines what the cheats show (a crystal counts
once its flag is set) with `spawn` (the real crystal object in front of Duke).
1. `spawn 1761` / `2761` / `3761` (or `spawn red/blue/green energy crystal`)
   creates the game's own crystal object, type 176 / 177 / 178, in front of
   Duke with the original `CreateObject`.
2. These types have no walk-over pickup, so our code supplies one:
   - touch: Duke within the original pickup range of a crystal we spawned;
   - collect: set item 11 / 12 / 13 (one item, as the cheat does);
   - feedback: the original message (113 / 114 / 115) and pickup sound
     (`0x7005`), written the way the original pickup does;
   - remove: the object leaves the world through the game's own routines.
3. The Select inventory, the D08A5 mission inventory and the receptacles read
   the flag, so they follow with no extra code.

Rules:
- `spawn 761` stays the original generic crystal (item 14). In crystal levels
  the console notes which item it really sets.
- 1761-3761 work only in levels 0, 5 and 9. Elsewhere they are refused clearly.
- If the crystal is already found, the spawn still works and says so.
- Console only (Modernized). Vanilla is unchanged.

Open points for the build: pickup range and the original sound, message and
removal routines; crystal models in levels 5 and 9; receptacle acceptance in
play; a spawned crystal across savestate load or level change (forgotten).

**Acceptance.**
- In levels 0, 5 and 9, each of 1761 / 2761 / 3761 appears as the right colour
  and is collected by walking over it, with the original message and sound.
- The crystal then shows in the Select inventory and the mission inventory, and
  the level's receptacle accepts it.
- Refusals and notes are clear (other levels, 761, already found).
- Other spawns, Vanilla and the level crystals' own take are unchanged.
- The user confirms in play.

### D26A - Debug level-select panel for whole-game testing

**Accepted - 2026-10-04.** User: "excellent! mark as accepted." Backtick console `levels` lists the 21 levels
the original title level-select cheat offers (game names from `0x800c3d2c`);
`level N` ends the current level with the pause menu's restart code (0xfd) and
switches the index inside the original mode 1 init, which loads level N.
Offscreen: all 21 levels load in one session with their own overlay, Old West
round trips, savestate save/travel/load, Vanilla, pause/title refusals. Candidate
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`. See
[note 101](documentation/101-d26a-level-select.md).

User request, 2026-10-04: testing infrastructure so the user can
travel through the whole game, find problems, make savestate reproductions and
stand up small jobs without replaying normal progression.

Required: open a debug level selector, see the available levels, select one,
enter it quickly, then play normally and use savestates.

- Keep the UI simple: the cheapest reliable debug list (D26 console, a host
  overlay or an existing PSXRecomp/runtime debug menu primitive). Proper menu
  style is D19, much later.
- Use the game's own level identifiers/data and its own level-start path where
  possible (LEVELxx/DBxx tables, the level index at `0x800be570`); avoid
  duplicated hard-coded lists.
- Development-only shortcut: it must not change normal progression, saves or
  memory cards, and is opened by an explicit debug hotkey or console command.
  Document how to open it in the manual.

**Acceptance:** every selectable level loads into normal play from the panel,
savestates work there, normal progression and saves are untouched, and the
user can use it to survey the game (enabling D22B, D11D, D12B and D17R
verification).

### D26B - Opening the console leaves first person

**Todo, small. User report, 2026-10-05.** Opening the backtick console switches
the game out of first person. The console is an overlay: opening and closing it
must return the player to exactly the perspective and state they had. Check
whether the console's input context or paused frames release the camera lease
(`first_person_release`) and how the view comes back.

**Acceptance:** open/close the console in first and third person; the view and
capture are unchanged afterwards; the user confirms.

### D26C - Console command history

**Accepted (2026-10-06): "im happy with all progress tonight".** Built the same
evening the user asked again ("please allow me to press up in the console to get
the last command typed. also a history command would be pretty neat too"):
Up/Down recall (newest first; Down past the newest clears the line) and a
`history` command (64 kept, consecutive duplicates once), in the framework
`main.cpp` console. Checked through the real console: Up recalled
`spawn steroids` and Enter ran it; `history` listed both commands. See
[note 111](documentation/111-d26e-debug-spawn.md).

**User request, 2026-10-05.** Up Arrow recalls the
previous command, Down Arrow the next, like a standard shell. Do not
over-design it.

**Acceptance:** Up/Down walk the session's entered commands; editing and Enter
behave as before.

### D26D - Level select: authoritative order, numbering, names and categories

**Todo. User request, 2026-10-05 (most important tooling issue from the
game-wide playtest).** The D26A list follows the title cheat's level indices
(0-3, 5-12, 21-29), which do not match the game's campaign numbering: there are
gaps (no 4) and odd numbering later on, so "Level 12 has this bug" is
ambiguous. Do not fill gaps by guessing.

Use the game's own data as the authority: level index and overlay tables
(`0x800be570`, file id `0x1ad + index`), the name table (`0x800c3d2c`), the
next-level function `80027fc0` (campaign order, challenge-stage and boss
branches), the title cheat cycle `80022d48` and any other level-select or
indexing structures the original has; reuse them rather than an invented
numbering. Cross-check the campaign progression with the complete guide
<https://gamefaqs.gamespot.com/ps/197177-duke-nukem-time-to-kill/faqs/3834>,
which also documents the original's cheat level select (regular levels,
bosses, challenge stages).

The console should distinguish campaign levels (gameplay order, campaign
number and name), boss levels (chronological position or clearly marked),
challenge stages (separate or clearly labelled), and other valid maps
(multiplayer, test, unused, special) that load safely, listed separately.
For each entry record the display/campaign number, name, internal level/map
ID, category and chronological order. Keep the UI simple; correctness and
reproducibility matter, not styling. Keep a way to address the internal
index so existing notes stay usable.

**Acceptance:** the list matches the game's own progression data and the
guide, every entry loads as before, earlier reports that used console indices
(D22C, D23C, D23D, D08T2) are re-mapped to the new numbering; the user
confirms.

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

### D29 - Progression items and objectives legibility (research and design first)

**Todo. Opened 2026-10-06 from the user's Level 2 playthrough.** A gameplay
modernization and player-legibility job, not a conventional bug. Modernized
profile only; Vanilla keeps the original presentation.

**Use case (Level 2, bank vault).** Duke collects pieces of paper whose
contents are needed later to open the bank vault. The original game tracks
this well enough for the level to work, but in the user's playthrough:

- it was not clear that an important piece of paper had been collected;
- there was no lasting way to inspect what had been collected;
- nothing said the papers held information relevant to the vault;
- at the vault, little connected the earlier pickups to what the game
  expected.

The user needed an external walkthrough. The recomp should not require one
just because progression information is effectively invisible.

**Design principle: remove unnecessary obscurity without removing
discovery.** No quest markers, no automatic puzzle solutions, no reduction
of the game to following instructions. The player should have access to
information Duke has already acquired: if Duke picked up and read a note,
the player can inspect what was on it and make the connection themselves.
For example, "Picked up: Torn Note" (or whatever matches the real item),
then later inspect it and see its numbers. Do not say "this is part of the
vault combination" unless the original context or a carefully improved
objective genuinely warrants it.

**Phase 1 (this job): investigation and design. Do not build the full system.**
Design game-wide; Level 2 is the reproduction case, not a hard-coded target.
Verify addresses and the active overlay (LEVEL02.OVR) before any hook, and use
private profile and save/card copies with the level-select panel (D26A).

Document, in a new engineering note:

1. **How the Level 2 paper/code/vault progression works internally**: what
   happens when each paper is collected; whether the papers are inventory
   items, flags, pickups, level-state variables or scripted events; whether
   the code or numbers exist as data (and whether they vary per game); how
   the vault checks progression.
2. **What items and information the game currently tracks** (inventory
   slots, key items, per-level flags), and whether the game already holds
   item names or descriptions that are never shown.
3. **What feedback the original game gives** on pickup (message, sound, HUD
   change) for progression items versus ammo and health.
4. **Whether the game's own item/progression data can be the source** of
   the information (read original state rather than inventing a parallel
   record). This is about data only: the D08A1-A3 inventory switcher is
   **not** to be extended for progression items (user, 2026-10-06).
5. **Objectives**: how the original Objectives screen stores and selects its
   text; whether text can safely be changed, extended or updated as the
   player discovers things (and the save compatibility of doing so).
6. **Game-wide audit** of similar opaque progression: documents, codes, keys
   or unusual objects, switch sequences, discovered information, invisible
   progression state, things Duke has seen or read, pickups whose purpose is
   not communicated. List them with level and type; do not redesign them in
   this job.
7. **A proposed lightweight game-wide inventory/information model**: which
   items qualify as "progression/information" items, what the player can
   inspect (name, what it contained), persistence across saves, loads and
   level transitions, and how it is derived from original game state.
8. **Presentation options** (no final visual design yet): a brief pickup
   notification distinct from ammo and health (text, sound or HUD cue),
   persistent inspectable entries (HUD panel, pause-menu inventory or
   objectives screen section), or a combination. Must not be intrusive. Uses
   the Duke font assets (D19A); later visual language belongs with D19.
   The user is considering presentation options and will bring them to
   this job (2026-10-06); fold those in before proposing a design.
9. **Objective guidance recommendation**: how much extra guidance to add
   without spoiling puzzles. Favour goals ("Find a way into the bank vault.")
   over solutions ("Collect all three notes ... enter 1234."). Objectives may
   become slightly more specific after relevant discoveries, never revealing
   the full answer.
10. **The smallest sensible first implementation to build and playtest**,
    likely the pickup notification plus inspectable entries for the Level 2
    notes through a game-wide mechanism, scoped as a follow-up job.

At minimum the eventual system must let the player answer **"What important
things have I collected?"** and **"What information did those things
contain?"**

**Acceptance (phase 1):** the engineering note answers items 1-10 with
evidence (addresses, overlay, data, observed behavior in a test run), the
audit list covers the full campaign as far as the level select can reach,
and the user reviews and approves the proposed model, presentation options,
objective guidance level and first implementation scope. Implementation
jobs are added to the board from that approval; nothing player-facing ships
in this phase.

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


## 2026-10-04 - D18D added; D17N world subdivision candidate (Needs playtest)

Added backlog job D18D from the user's report: after death and Continue, level
music stays silent until Duke's next voice line. Not started.

D17N: the vibrating diagonal wall lines come from the original world
renderer's screen-space subdivision of every polygon within `0x2000` units.
Its midpoints carry no projection data, so the pieces were drawn affine even
with Corrected textures. With Corrected texture precision, qualifying world
polygons are now drawn whole with exact perspective (new hooks `0x800114EC`
and `0x8001160C`). Private 120 Hz walking sequences show straight borders; the
12-slot sweep shows no game-rate, budget or replay-miss regressions and lower
packet use; Original textures and Vanilla are unchanged. Tests pass; codegen
hash unchanged. Needs the user's playtest; nothing committed.
See [note 98](documentation/98-d17n-world-subdivision.md).

## 2026-10-04 - D17N accepted; D17Q added

User playtest: D17N "has achieved its objective" ("The result is excellent.";
the message called it D17P). D17N is Accepted; executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Cause, solution and fallback behaviour are recorded in
[note 98](documentation/98-d17n-world-subdivision.md). New Todo job D17Q covers
the remaining subtle wall/surface flicker as independent Corrected-renderer
stability polish. The user authorized documentation, commit and push.


## 2026-10-04 - D22A portal level identity candidate (Needs playtest)

Reproduced from a dated private copy of UI slot 7 (file 06, SHA-256
`b05b9437...`) offscreen. After the portal and statistics screen, Duke stood
in the Old West street with original controls; the log named the cause:
guard 119, the LEVEL00.OVR body at `0x800ca968`, now held tag 4. RAM/disc
correlation shows LEVEL01.OVR resident at the same base, differing only in
its final 16 bytes, a hit-position scratch vector the overlay passes to
`0x8007177c`, the same layout as LEVEL00's D08P tail.

Fix (`control_guards.inc`, `modern_controls.cpp`, `apartment_interaction.inc`):
the level body moved out of the resident guards into an authenticated
`level_overlays[]` table keyed by the overlay's tag word (LEVEL00 tag 3,
LEVEL01 tag 4, code/table bodies hashed from the owned disc). Identity requires
the resident guards plus the matching level body; unknown levels still fail
closed. The apartment secret patch, light-switch targeting and concealed
pickup now require LEVEL00 explicitly. No hooks or guest behavior changed.

Evidence: `ttk-controls-test` gains a LEVEL01 group (optional owned
LEVEL01.OVR argument) and links again (renderer stubs); 32 groups pass.
Input/aim/near tests pass. Offscreen in LEVEL01, first person: movement,
mouse look, jump, view-aimed fire (27 shots, through a private Q binding),
Start pause/resume. Third person: portal twice in one session (savestate back
to LEVEL00 between), then a 90-second wander with zero identity refusals; the
overlay still differs only in its tail. All 12 LEVEL00 slots keep the lease;
Vanilla shows no Modernized activity. Candidate
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0`; framework
patch and codegen hash unchanged.

Limits: only LEVEL00/LEVEL01 are authenticated; later levels keep original
controls until each is verified. Escape capture release and Mouse1 were not
driven offscreen. **Needs playtest:** UI slot 7 -> portal -> continue, then
check movement, mouse look, fire, Escape pause/resume and P view toggle in
the Old West level. Nothing committed.
See [note 100](documentation/100-d22a-portal-level-identity.md).

## 2026-10-04 - D22A accepted; D11D, D12B, D17R, D22B and D26A added

User playtest: "I can confirm and accept D22A as working." "Level 2 is working
and is completely playable. The teleport/level transition is functioning
correctly and I can proceed into the level and play normally." D22A is
Accepted; executable
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0` is the new
regression baseline. The user authorized documentation, commit and push.

New Todo jobs from that playtest (player saves UI 8/9 = files 07/08 in
LEVEL01, hashes in the job entries):

- D11D: first-person eye height too low (slot 8, next to the dancer).
- D12B: first-person kick leg ignores Duke's LEVEL01 costume (slots 8/9).
- D17R: sky black toward the left/right edges when looking up (slot 9).
- D26A: debug level-select panel for surveying the whole game.
- D22B: Modernized controls own every level, state and transition end to
  end; the classic controls should not take over during normal play.

User direction for all of them: fix the shared system, not the level. No next
job selected or started.

## 2026-10-04 - D26A debug level select, Needs playtest

Implementation: `recomp/src/ttk/level_select.cpp` (console commands, SHA-256
code identity, deferred index write on the init's `8002b9f4` call) and the
framework console/debug-port hooks in the exported patch. The game's mode
machine, level flow and name tables are decoded in
[note 101](documentation/101-d26a-level-select.md).

A first build wrote the level index with the restart request. Old West pairs
then crashed: the old level ran one more frame and called the new level's
per-level player routine inside the old overlay. The index now changes only
inside the original init, before its loader reads it.

Evidence (offscreen, private card copy): `levels` lists 0-3, 5-12 and 21-29
with the game's names. `level N` for all 21 levels in one session, each in
11-13 s, reaching normal play with that level's own LEVELxx.OVR resident (byte
equal to the disc except the scratch tail). Old West round trip 1-2-3-1-2-1-3.
Modernized DUKE HILL after travel: lease ready, first person, movement.
Savestate save in a selected level, travel elsewhere, load: restored. Pause
menu and pre-game refusals. Vanilla travel. ttk-controls/input/aim/near tests
pass; codegen and savestates unchanged.

Limits: travel arrivals start with the restart's loadout (health 100). Level
completion after a selected level was not exercised. Levels other than LEVEL00
and LEVEL01 still use original controls (D22B). Accepted by the user 2026-10-04 ("excellent! mark as accepted"). D22B (controls
and first person in every level) is the next job.


## 2026-10-04 - D22B every level and transition, Needs playtest

User: "let's work on D22B. proceed autonomously!"

Level coverage: every LEVELxx.OVR has one layout (tag, strings, jump tables,
code, then one data block whose first reference is the code end; the level
writes only trailing bytes: hit vectors passed to `0x8007177c`, its own
`sb`/`sh`/`sw`). New `recomp/tools/local/level_overlay_guards.py` derives each
body from the owned disc, rejects any level that breaks that layout, reproduces
the accepted LEVEL00/LEVEL01 digests exactly, and `--check`s
`control_guards.inc`, which now authenticates all 21 selectable levels.
Unknown tags (LEVEL04/13/14/30, arenas) and cross-level tags fail closed.

State coverage (60 s scripted play in every level, first and third person):
dodge rolls 157-162 and the steep-slope slide (mode 2, set only at
`0x80055fb4`; 99/108 off the slope) keep the camera-only lease and the
selected view, with no directional pads (`committed_camera_ready`). The
original's own back-steps 82-85 and strafes 88-93, started when a pad was held
as an original move ended, are now taken over by the lease
(`original_step_anim`, separate from `land_gait_anim`). Unowned jumps keep the
camera-only lease in third person as in first person (`jump_camera`, was
`eye_jump`), and the landing poses 94/95/106 keep it from their first frame.

Files: `recomp/src/ttk/control_guards.inc`, `modern_controls.cpp/.h`,
`first_person.inc`, `pc_input.cpp`, `recomp/tools/local/level_overlay_guards.py`,
`recomp/tests/local/modern_controls_native.cpp`, `pc_input_native.cpp`,
`test_level_overlay_guards.py`; `GAME_MANUAL.md`; note 102. No hook, generated
C, framework or hashed header change; patch and codegen hash unchanged.
Candidate `5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017`.

Evidence: `ttk-controls-test` 34 groups (new D22B all-level identity and
roll/slide/step/landing/third-person jump groups), `ttk-input-test`,
`ttk-aim-test`, `ttk-near-test`, Python suite 110 tests. Offscreen
(`recomp/analysis/d22b-20261004`, private cards, port 9247): baseline lease
refused in 19 levels; candidate zero identity refusals across all 21 levels;
first person active on 4,751 of 4,800 samples (46 swim/D08V orbit by design,
3 one jump); a third-person rerun kept the camera lease on every sample (an
earlier run showed intermittent jump camera gaps in two levels, cause not
established); statistics screen into the
next level (6 pairs), savestate across levels, pause, LEVEL00 slots 0-11 and
Vanilla unchanged. See [note 102](documentation/102-d22b-every-level.md).

Remaining: swimming first person (new D11E); D08V mantles/hangs/unowned falls
keep their orbit view; real death/Continue and natural level exits outside
LEVEL00 not exercised offscreen; occasional jump camera gaps (one first-person
jump in CHALLENGE STAGE 2, one third-person run in two levels) are not
explained. User playtest required.

## 2026-10-05 - D22B accepted; game-wide playtest backlog

User: "Accepted." D22B is Accepted; executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the
regression baseline. The user then used `level N` for a broad playtest.

Passed (regression evidence): medieval Duke first-person height good;
medieval costume-aware kick good; Roman / HOG HEAVEN height good and kick
good; Level 9 armed rolls (Ctrl) good; frame rate generally good; game-wide
playability very encouraging. Recorded on D11D and D12B.

New Todo jobs (none started): D23C medieval moat/Necro slowdown (profile
later), D23D Level 9 strip-club-area slowdown (low priority), D08U1 player
slot 12 ladder cannot be descended with E, D08T2 Duke-symbol pushable blocks
need a modern RMB grab (game-wide), D22C Level 11 started in Legacy controls,
D26B opening the console leaves first person, D26C console Up/Down history,
D26D authoritative level-select order/numbering/names/categories. D17R
updated with the HOG HEAVEN sky reproduction (no duplicate job).
Documentation only; no code, build or launch. No next job selected.

## 2026-10-05 - D22C control mode persists through level select, Needs playtest

Selected by the user ("you might not even be able to replicate it").
Reproduced on the accepted D22B build with real SDL keys (xdotool, private
Xvfb :95, private card copy, port 9247;
`recomp/analysis/d22c-20261005/repro.py`):

| Case | Before | After |
| --- | --- | --- |
| A: no F10 in the session, `level 11` then `level 12` | captured, lease ready, first person | same |
| B: F10 (release), F10 (capture), then `level 11` and `level 12` | not captured, `ready` false, first person `lease` | captured, lease ready, first person |

Cause: `pc_input.cpp` cleared `initial_capture` (automatic gameplay capture)
on every F10, including the F10 that recaptures. Only Escape or another F key
set it again. Opening the console releases the mouse, so after any earlier
F10 pair the next `level N` (or other host release) left Duke uncaptured,
which looks like Legacy controls until F10. Nothing about Level 11 itself.

Fix (`recomp/src/ttk/pc_input.cpp`, game code, not framework): an F10 that
releases still opts out of automatic capture; an F10 that captures opts back
in. Escape/F7/focus behaviour is unchanged. `ttk-input-test` gained a D22C
group (F10 pair, console-style release, offers recapture; an F10 release
still stays free through offers). ttk-input-test, ttk-controls-test (owned
LEVEL00/LEVEL01/levels dir), ttk-aim-test and ttk-near-test pass. Vanilla
ignores F10 (input module returns before key handling), unchanged. Codegen
hash unchanged. Candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`.
Manual updated (F10 capture opts back in).

Not exercised: natural progression into Level 11 (the user's report was
`level 11`); the reported session's exact key history is unknown, so this is
the reproduced mechanism, not proof it was the user's only path. User
confirmation required.

## 2026-10-05 - D08T2 object type range (pushables, ladders, mantles), Needs playtest

D22C accepted by the user ("I accept this work"); D08T2 selected; D08O1
(fire while swimming) added and D17R updated with the UI slot 2 medieval sky
report.

D08T2 cause: the UI slot 1 medieval Duke-symbol block is object type 924
(flags `0x081890e2`, pushable and climbable). `push.inc`
`object_type_flags()` refused types `>= 512`; the original allocates 1062
types (`0x7428` bytes at `0x8001b634`). Bound now `0x7428 / 28`. The same
lookup serves D08U ladder tops and D08X mantles, so 4 pushable, 6 ladder and
114 climbable types (of a single global table) were invisible to Modernized.

Evidence (real keys, private copies, A/B with the old bound): RMB grabs and
pushes the slot-1 block 1129 (third) / 1126 (first person), old bound moves it
0; E alone still never grabs; the D08U1 slot-12 ladder (type 639) now mounts
with E and S descends to the bottom (old bound: nothing); D08T1 dumpster suite
unchanged; Vanilla dumpster push +1185 / pull -936. Native controls (new type
range cases), input and aim tests pass. Candidate
`06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
[Note 103](documentation/103-d08t2-object-type-range.md).

Not exercised: the Roman-area block and the two early medieval blocks; other
newly visible ladders and mantles. User playtest required for D08T2 and
D08U1.

## 2026-10-05 - D08T2 retest: hints, slot-12 ladder bottom; D08T3 added

User retest of the D08T2 candidate. (1) "the dialogue at the top does not
appear ... after interacting with a pushable block once, the dialogue at the
top doesnt appear ever again": not a D08T2 regression; D08T1 showed
`HOLD RMB TO GRAB` only on the first 2 touches and the push/pull line only on
the first 3 grabs of a session, and the newly pushable medieval blocks now
use them up. Hints now show on every fresh touch and every grab (and the D08U
ladder-top `E TO CLIMB DOWN` on every fresh ladder top), at most once per 300
input frames (about 5 s), and log `[TTK input] Hint: ...`. Live: three touches
and two grabs each logged their hint (the debug screenshot does not include
host overlays). (2) Slot-12 ladder S descent glitch: original bottom-rung
hang 207 (ladder ends above the floor); S now lets go with the original
Square. See D08U1. (3) New Todo D08T3: free manual push/pull.

Native input (new bottom-hang and hint-cooldown cases), controls and aim tests
pass. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.

## 2026-10-05 - D08T2 accepted; D08U1 ladder bottom fixed properly

User: "i accept everything aside from the ladder glitch as demonstrated in
slot 12 ... continue working on it and make that ladder descendable." D08T2
(type range, hints) Accepted.

D08U1 diagnosis on the private slot-12 copy, frame by frame: the ladder's
lowest rung is about 990 above its floor (the sewer ladder's is about 420).
At the bottom of a step down the original 188/189 case (`0x80044128`) calls
`0x8007ded0`; it is true (nothing 500 below), so the original plays 156 ->
211 (Duke swings sideways off the ladder, legs flailing) -> 207 (hanging from
the bottom rung). The original pad alone does the same; Square lets go from
any climbing pose (fall 108, landing), Up climbs back. The earlier fix only
made S drop out of 207, so the swing remained.

Fix (`ladder_top.inc` `ladder_end_update()`, run from the player update, and
`pc_input.cpp`): on a plain ladder (186..189), ask `0x8007ded0` through the
isolated original call (touch fields +0x174/+0x178 restored). True during a
step down (188/189 with `+0x6a`): clear `+0x6a`, so the original stops at
this rung. On that rung S sends Square instead of Down/Cross. A first attempt
with a 300-unit lookahead broke the sewer ladder (its probe point fell below
the floor), so the rule uses the original's own probe at Duke's position only.
The 207 fallback stays. Debug: `ladder_top.end_probes/end_hits/end_stops`.

Evidence (real keys, private copies): slot 12 hold S, third and first person:
188/189 -> 108 -> landing on the floor, no 156/211/207, weapon redrawn; tap
S: steps down, stops on the last rung (end_stops 1-2), next tap drops; W from
the last rung climbs to the top exit 190. Sewer ladder (D08U state): 188 ->
185 step-off -> floor, unchanged. Alley ladder D08U run: same states and end
position as accepted, end_hits 0. Native: controls (new D08U1 group: stop on
open bottom, touch fields kept, resting poses untouched, Vanilla never
probes), input (last-rung Square without Down/Cross), aim pass. Candidate
`cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`. User playtest required.

## 2026-10-05 - D08U1 still failing for the user; robustness fixes and ladder diagnostics

User: "the issue still exists. i cannot climb down it at all. duke mounts the
ladder from the top, and then glitches out when you try to press S."

The user's session log (12:17, candidate `cf90d942...`) shows repeated
186 -> 189 -> 187 -> 188 lease lines and three `E TO CLIMB DOWN` hints, and
never 156/211/207 or a drop (108): Duke never reached the ladder bottom, and
was back on the platform between attempts. Not reproduced offscreen with a
private copy of the user's own profile (first person, 120 fps, manual jump,
view bob, scale 4) on the real GPU: held S, tapped S, E held with S, E taps
alternating with S taps, E alone, and mouse look (yaw, pitch, both) during S
all descend; frames show the normal orbit view on the ladder.

Found and fixed on the way: (1) with E held, the end probe missed by a few
units (the original moves Duke 20-40 units in the update before its own
probe), so the swing still happened: the probe now looks 96 units lower (300
was too far for the sewer ladder). (2) After the stop Duke rests a little
higher where the probe reads closed: the last rung is latched until he climbs
150 up or leaves the ladder. (3) A held E kept Cross pressed, and the original
ignores the let-go while Cross is held: Cross is released while letting go.
All variants now reach the floor; the sewer ladder keeps 185.
`DNTTK_TEST_INPUT` (diagnostics only) now turns key changes into key events,
so scripted runs exercise press-driven actions such as E.

New session-log line on every ladder pose change:
`[TTK ladder] anim=... mode=... y=... obj=... keys=WSE pad=UDXQ end=...` (keys
held; original Up/Down/Cross/Square sent; last-rung latch). Native tests pass.
Candidate `071487b1f35bcbfd9bf4734c69fd06926fb1b1c11283c2c29dce80156428784a`. Waiting for the user's description and a session
log of the failure.

## 2026-10-05 - D08U1 root cause found in the user's log: mount blend cut short

User: "it produced the same effect. yes i hold down S." The new
`[TTK ladder]` lines (session 12:36) show the mount ending at y -9114 and
every later pose (186/189/187/188 with S, pad Down+Cross) staying at -9114.
Every offscreen run ends the mount at -8901, on the climbing line. The host
blends Duke from the platform to the line over 12 player updates of the
original 156; when the game drops frames (the user's scale-4 rendering) the
original 156 advances several frames per update and ends after about 8, the
blend stopped at about two thirds, and Duke was left 213 units above the
climbing line, where the original plays the step poses without moving him.

Reproduced with a diagnostic blend length (`DNTTK_LADDER_MOUNT_UPDATES=18`,
diagnostics only): the mount ends at -9115 and, with the previous code, S
loops the poses frozen there, the user's exact report. Fix
(`ladder_top.inc`): if the original leaves 156 for the resting poses 186/187
before the blend is done, the host finishes the blend there, and Up/Down are
not sent meanwhile (`ladder_mount_finishing()`); a step already started gets
the remainder at once. Debug `ladder_top.mount_finishes`. With the fix, the
same 18- and 30-update cases reach -8901 and S descends to the floor; the
normal case, the sewer ladder (185 step-off) and native tests are unchanged.
Candidate `4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`. User playtest required.

## 2026-10-05 - D08U1 accepted; closeout

User: "accepted!! done, commit. great work". D08U1 Accepted; D08T2 and D22C
were accepted earlier today. Executable `4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83` is the regression
baseline. Documentation, commit authorized. No next job selected.

## 2026-10-05 - D11D and D12B: first-person joints found by part (cowboy costume)

User selected D11D and D12B together and added: "it's all the cowboy levels
actually". Both now Needs playtest. Nothing committed.

**Cause.** Duke's model record (`player+0x40`) has a part -> joint table at
`+0x24` (the game already reads the right hand from `+0x33` = part 0xf), and
each 0x28-byte joint record at `+0x44` carries its part id in byte 1. Dumped
from private copies of the user's UI slots 8/9 (files 07/08, hashes verified)
and the first street:

| Part | First map, medieval, Roman, all others | Cowboy (levels 1, 2, 3, 27) |
| --- | --- | --- |
| Neck 0x10 | joint 9 | joint 15 |
| Spine 0x09 | joint 1 | joint 10 |
| Right leg 0x05-0x08 | joints 14-17 | joints 5-8 |
| Arms 0x0a-0x0e | joints 2-6 | joints 16, 17, 18, 11, 12 |
| Right hand 0x0f | joint 7 | joint 13 |

The host hard-coded joint 9 (eye, head hide), 1 (weapon torso reference),
2-6 (arm hiding) and 14-17 (kick leg). In the cowboy model joint 9 is part
0x15 at hip height, so the eye sat at Duke's waist and his real head was never
hidden. Joints 14-17 hold part 0x14, the neck and two arm parts, so the kick
posed the wrong meshes. The hand already used the table, which is why the
weapon still appeared.

**Change** (`recomp/src/ttk/first_person.inc`, `kick.inc`). `duke_part_joint`
/ `duke_joint_part` resolve joints through the table and check each joint's
record byte 1 matches. The eye and head hide use part 0x10, the torso
reference part 0x09, arm hiding parts 0x0a-0x0e, and the kick leg parts
0x05-0x08 (key index = part - 5). At Duke's draw entry a stale hide bit is
reclaimed on any joint record, not only the neck, because older builds set it
on cowboy joint 9. The original never sets bit 0 on a joint record (D11C).
There are no per-level offsets or new hooks.

**Evidence.** Binary
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`. Scripts
and captures: `recomp/analysis/d11d-d12b-20261005/` (private cards, offscreen,
ports 9261/9262).
- Survey of all 21 selectable levels (`models.py`, `models.json`): the table
  is consistent with every joint record in every model. Levels 1, 2, 3 and 27
  use the cowboy order; all others use the first-map order. The neck is 691-705
  above the right toe everywhere, first person is `active` in every level, and
  the head hide runs.
- Slots 8/9 (`verify.py`): the eye follows joint 15, 701/708 above the toe,
  versus 701 on the first street. Before the fix, joint 9 sat 89 below the
  root, so the old eye was about 300 lower. Captures: the near dancer's head is
  about at eye level, and in third person Duke stands about her height
  (`shots/after-slot7-*.png`). A quick kick held at frame 13
  (`DNTTK_FP_KICK_FREEZE=13`) shows the cowboy jeans and the brown cowboy boot;
  the first street shows the original leg.
- Native: `ttk-controls-test` with a first-map table, plus a new cowboy case:
  the eye anchor follows joint 15 and not the hip-height joint 9, joint 15 is
  hidden, a stale bit on joint 9 is reclaimed, the kick takes joints 5-8 and
  leaves 14/15 alone. `ttk-aim-test`, `ttk-near-test` and `ttk-input-test`
  PASS. `ttk-scene-test` was not run because it needs an owned RAM fixture
  and is unaffected.
- Vanilla route `analysis/vanilla-regression/d11d-d12b-vanilla` exit 0;
  captures show the original camera.

**Limits.** The kick poses are still the first-map 115 keys. In the cowboy
model the leg's rest positions are within a few units of the first map's, but
the framing was judged only from captures. Cowboy-level weapon framing also
changes, because the torso reference was a hip part before. Not playtested
by the user.

## 2026-10-05 - D11D and D12B accepted; closeout

User: "i can confirm that i accept both jobs as complete! document commit
push". D11D and D12B Accepted. Executable
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b` is the
regression baseline. Documentation, commit and push authorized. No next job
selected.

## 2026-10-05 - D23F fast CPU timing, Needs playtest

**Implementation.** New Modernized setting `cpu_timing` (profile schema 25,
`run.py --cpu-timing accurate|fast`, default accurate; Vanilla always
accurate; `DNTTK_CPU_TIMING`). `recomp/src/ttk/fast_timing.h` is
force-included by `recomp/CMakeLists.txt` in front of every
`generated/SLUS_005.83_full_*.c` and redirects the emitter's timing hooks to
inline functions that test one flag. With it set: one charge per block from
its instruction count (1.5625 cycles each), main-RAM loads read directly with
6 extra cycles, no instruction-cache or pipeline simulation, and the branch
edge runs the full interrupt check only at a device deadline, a pending
interrupt, a COP0 software interrupt or every 64th edge (savestates, debug,
lease). Stalls, GTE, MMIO and stores are unchanged. `fast_timing.c` leases
the model from the Modernized player update (12 fields, so menus, movies and
loading are accurate) and reports `ttk_input.input.cpu_timing`. No framework
source changed; codegen hash still `0x8bab543c`. Calibration build
(`-DTTK_FT_CALIBRATE=ON`, `fast_timing_calibrate.h`) fitted the accurate model
at 0.92 per instruction + 5.7 per RAM load + 6.9 per cache miss (r2 0.996);
the constants were then matched on game frame rate at 100% (slots 1/9/10:
24.9/20.2/15.1 fast against 25.0/19.2/15.2 accurate).

**Evidence.** Candidate
`e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966`
(includes D23E). Harness `recomp/analysis/d23f-20261005/` (offscreen real GPU,
player profile copy, private savestate copies and port, silent audio).
- 150%, 120 Hz, walk and turn 30 s: presents per second slot 1 42.7 -> 120.1,
  slot 9 40.1 -> 120.1, slot 10 40.0 -> 120.2, every refresh, no `[TTK cpu]`
  events (accurate had 3-5); game frames per second 28.1 -> 29.8, 20.6 ->
  26.2, 17.5 -> 21.6; emulation thread 0.92 -> 0.73, 0.91 -> 0.73, 0.93 ->
  0.82 of a core.
- 60 s D23E routes at 150%/120 Hz, fast: slot 9 strafe 3 fields 1194 of 1204
  frames, slot 10 walk 1193 of 1204, 120 presents/s, no underruns, no pauses.
- 100%, 60 Hz: emulation thread about 0.75 -> 0.40 in slots 1, 5, 9, 10.
- Savestate saved while fast (private slot 7, 90 ms) and reloaded correctly.
- Native `ttk-input-test`, `ttk-aim-test`, `ttk-near-test`,
  `ttk-controls-test` PASS; profile/launcher unit tests (64) PASS with a new
  schema 25 test.
- Vanilla keeps the accurate model with the same game frame rate; host cost
  of the flag tests about 3 points (0.72 against 0.69 without the include).
  The Vanilla route completed (exit 0) but diverged from its long-standing
  captures 3 of 3; a build without the D23F include diverged the same way 3 of
  3, so D23F is not the cause. The cause is not established (last matching
  run d11d-d12b-vanilla, before D23E).

**Limits.** Not played by the user; audio only on the dummy device. Level
overlay libraries, the interpreter and the BIOS keep the accurate model.
Dispatch-path observers still run (about 2%); the next large host cost is the
call/return dispatch path (about 13%). Step 3 (cheaper redraws) not done.

## 2026-10-05 - D23F accepted; fast timing is the Modernized default

User: "the behaviour is amazing. the behaviour is absolutely beautiful, and a
real pleasure to play. Everything looks stunning and the responsiveness is
literally like PC accurate now", and asked to accept it as the new default.
D23F Accepted. Profile schema 26: `cpu_timing` defaults to `fast`; a saved
`accurate` from schema 25 (the default for a day) switches once with a notice,
later choices are kept; Vanilla always accurate. Profile/launcher tests: 65
PASS. No executable change. New jobs: D23G (finish the fast path, all-in-one)
and D17S (auto frame-rate default with a cap). Documentation, commit and push
not yet authorized.

## 2026-10-06 - D08O1 fire while swimming, Needs playtest

User clarification: "its when swimming underwater", "you cannot be in motion
while swimming and shooting", "you can shoot while still in water".
Reproduced in level 6 FAMILY JEWELS (the UI slot 2 moat in OBEY OR DIE is not
swimmable water): W 2892 units in 90 frames, any swim direction with fire
held 0 units, anim 127. Cause: the original underwater handlers (`800455bc`,
idle 127 / thrust 128-130) test fire before the Square thrust.

Implementation (Modernized only): entry hooks `0x800455bc` and `0x80055e80`
(regenerated); the Cross word's bit 0 is hidden from the swim handler while
fire, a weapon and a swim direction are held, and restored at the next
player-update step, animation start or camera update. `swim_weapon_ready()`
joins weapon view aiming, shots, beam completion and the reticle. Weapon
selection runs from the swim handler and, in water, offers only the weapons
whose original record has flag `0x04` (Crossbow, Desert Eagle, Combat
Shotgun, Buffalo Rifle, Gatling/Laser Gatling, Pipe Bomb); the original
remaps or stalls the others. Guards added for the handler, its table, the
callback table and `80055e80`. Codegen hash unchanged; savestates load.

Evidence (private lab, Xvfb, private copies; player files unchanged):
underwater W/A/Ctrl/Space + fire swim 1135-2826 units with 9-10 adapted
shots; six allowed weapons swim and fire; underwater weapon keys/wheel select
only allowed weapons; Ctrl + fire dives, Space + fire surfaces; ground and
jetpack firing unchanged; Vanilla still stops to shoot. Native aim (new swim
case), controls, input, near, inventory and font suites and 112 Python tests
pass. Build `711a313d983f78b1929431f68a0443fcf0f5a70fb3e2a424444d9d5515e12eb7`.
[Note 107](documentation/107-d08o1-swim-fire.md).

**Limits.** One level driven live (level 12 not reached by script); Duke's
body faces the swim direction while firing (shots follow the view); first
person in water is D11E; holstered fire does not draw in water (original).
Needs the user's playtest at their location and a second level. Not committed.

## 2026-10-06 - Backlog: D08A4, D08S reopened, D08Q4, D08Z1, D11F

User requests recorded as Todo: D08A4 EDuke32-style portable steroids (stored
item, R), D08S reopened as the EDuke32 jetpack (instant J on/off, midair, 61 s
of fuel), D08Q4 `dnkroz` unlimited jetpack fuel, D11F first person while
flying the jetpack, D08Z1 keep jump momentum when bumping a wall (Modernized
option for the future menu).

## 2026-10-06 - D08O1 accepted; D08O2 queued

User: "i completely accept that this works! mechanically, it does exactly what
it's meant to, but the only issue is the animation". D08O1 Accepted. New Todo
D08O2: the weapon points downwards while swimming and firing in motion; make
it point forward. D08Q4 now records Duke3D's `dnkroz`: god mode, health to
100, unlimited jetpack fuel, and only Atomic Health raising health to the real
maximum of 200. Committed and pushed at the user's request.

## 2026-10-06 - D08G3 `dnupgrade` and D08Q4 `dnkroz` (Needs playtest)

User selected both. Research (read-only SLUS-00583):

- Upgrades are bit `0x8` on a weapon record (`player+0x2c4+4*id`).
  `8003df40` resolves Gatling 7 -> 28 (Laser Gatling), RPG 8 -> 29
  (Incendiary) and Flamethrower 9 -> 27 (HiTemp), with ammo from the
  resolved record. Desert Eagle 4, Shotgun 5 and Energy Weapon 10 upgrade in
  place.
- The persistent upgrade mask is `player+0x85f`, bit i -> weapon
  4/5/7/8/9/10. Challenge stages 21-26 set bit (level - 21) (`800950cc`).
  Saves store the mask (`800833ec`). Load (`8008328c`), the restart loadout
  (`8003f4d8`) and weapon pickups (`80081e3c`...) reapply it.
- The original cheat dispatcher has an unused upgrade-all entry (`80083a38`
  -> `8003f5cc`). It sets the bits on both player records but not the mask,
  so its upgrades would not survive a save.
- Health pickups call `80096cac(player, percent, over)`. Normal health is 10%
  up to the type maximum (`types[+0x2c]+0xc`, 10000 = HUD 100). ATOMIC HEALTH
  (string 0xad) is 50% with `over` = 1, capped at twice the maximum (200).
  TTK already has the Atomic Health ceiling; nothing was added.
- God mode `800c3cc6` is written only by the cheat dispatcher, so it survives
  death, Continue and level changes. `800c3cc4` (which skips the mode-10
  drain) is unlimited ammo and was left alone. Jetpack capacity is the live
  item table `800c2716` (9000).
- EDuke32 (`actors.cpp`) pins health to max and jetpack to 1599 every tick in
  god mode, which would also cut an Atomic surplus. Per the user's
  description, the surplus up to 200 is kept.

Implementation (`src/ttk/cheats.inc`, `cheat_codes.h`, `modern_controls.cpp`):

- `dnupgrade` sets bit 8 on weapons 4/5/7/8/9/10 and ORs `0x3f` into the
  mask. For an owned Gatling, RPG or Flamethrower it also marks the upgraded
  record owned and gives it at least the base ammo, capped at the upgraded
  capacity (`800c4594+44*id`: 250, 8, 200). Without this the upgraded weapon
  counts as empty in normal play: verified live, firing switched to the rifle
  and 4 skipped the Gatling. It grants no unowned weapon (use `dnweapons`). Confirmation: "Weapons
  Upgraded". It has the same solid-ground and Modernized gate as the other
  cheats.
- `god_mode_update()` runs while god mode is on: health is raised to the
  type maximum, never lowered, and an owned jetpack is held at the item cap.
  It runs at the `8005a210` step-table entry and again from the `80058120`
  poll, which follows this update's mode-10 drain. Without the second call
  the HUD floored 8985/9000 to 99%. Toggling god mode on applies it at once.
  Off resumes normal drain from the current amount.
- New SHA guards: `8003df40` (112 bytes), `80096cac` (152 bytes). No hook
  list, generated code or hashed header changed. Existing savestates still
  load.

Evidence (private cards/profile, port 9361, Xvfb; lab
`recomp/analysis/d08g3-q4-20261006/`):

- Level 6 (FAMILY JEWELS) and level 12 (BLOOD BATHS, after `dnstuff`):
  `dnupgrade` set flags `0x1 -> 0x9` on all six and mask `0x3f`. The Gatling
  already in hand fired from record 28 straight away (250 -> 240, record 7
  unchanged). Flamethrower fired from 27 (200 -> 196) and RPG from 29
  (8 -> 6).
- With record 28 emptied, as in normal play: before the carry-over, the upgraded
  Gatling was skipped. After it, 28 got 250 from the 400 base rounds, fired
  (250 -> 241) and 4 reselected it.
- A savestate save/load kept the mask and flags; another state showed mask 0.
- Underwater, the cheat is refused with "stand on solid ground".
- `dnkroz` on from 30 health and 4000 fuel gave 100 and 9000. A poked 150
  surplus was kept, and 30 health was raised back to 100. 240 frames of
  flight in Modern and in Classic read fuel 9000 on every sample. With god
  mode off, flight drained normally (8525 -> 7405).
- Vanilla: god flag poked on, health and fuel unchanged, typed `dnupgrade`
  ignored.
- Suites: ttk-controls-test (new D08Q4/D08G3 case, all-levels fixtures),
  ttk-input-test (parses every code, including `dnupgrade`), ttk-aim-test,
  ttk-near-test, ttk-font-test, ttk-inventory-test, Python 112 OK
  (2 skipped). ttk-scene-test was not run (needs a captured exit RAM).
- Build SHA-256: `5f9b65c99b25d62794e942f2a9d1b1e239fb53a401816e7cc5fbf316c30f5bb8`.

Limits: an Atomic Health pickup was not driven live (the surplus was poked).
Death/Continue and memory-card save/load of the mask are established by code
reading, not driven. The ammo carry-over is a cheat convenience: the base
record keeps its own ammo, which is unused while upgraded. Whether to grant
unowned weapons too is the user's call; currently they arrive upgraded when
picked up. Needs the user's playtest.

## 2026-10-06 - D08G3 and D08Q4 accepted; D08Q5 queued

User: "accepted!!" for `dnupgrade` and the Duke3D-style `dnkroz`. New Todo
D08Q5: weapon switching (number keys and wheel) while flying the jetpack.
Committed and pushed at the user's request.

## 2026-10-06 - D08Q6 Modern jetpack altitude hold (Needs playtest)

Reproduced in LEVEL00: about 30-140 units of climb per 120-frame WASD leg.
There were two host-layer causes. The fixed level trim left about 2 units per
update of lift, and every release re-anchored the hover at the current height
(plus up to 128 units of the original bob). View pitch does not move Duke;
hovering alone bobbed +/-128. The fix holds the altitude captured when Space
or Ctrl ends. Level flight gets a proportional `+0x1f8` steer, hover gets a
slewed base with the bob phase held at 0, and the target is capped at the
original 0x200 floor approach height. Verified in the lab:

- A random turning/pitching circuit ends within 9 units.
- Low flight holds within 15 units with no landing.
- Ctrl landing, J fall, firing and D08Q5 switching are unchanged.
- Classic and Vanilla are unchanged.
- The native suites (new D08Q6 group) and the Python tests pass.

Next: the user's playtest on the LEVEL01 apartment/alley route. The Modern
hover bob is removed; ask if they want it back as a visual effect.
[Note 109](documentation/109-d08q6-jetpack-altitude-hold.md). Not committed.

## 2026-10-06 - D08Q6 revision 2: smooth hover (Needs playtest)

User: "the jetpack does not control smoothly now, it's very jerky". Per-update
traces showed revision 1 twitching +17/-25 about every 6 updates in hover, with
a +21 step on each stop. Holding the bob phase at 0 left one timestep of the
steep part of the sine (`8004b474`: bob = 128 sin(6 (phase + dt))), and the
error-driven slew echoed it. Revision 2 parks the phase at the flat peak
(`171 - dt`), keeps the base 128 below the target and slews it at most 3 per
update. Hover is now 0 per update, stops settle +3/+3/+2, and flight matches
the old build's smoothness. The altitude hold is unchanged: the circuit ends at
0, low flight holds, and Ctrl landing, J fall, firing and switching work.
Suites pass. Not committed.

## 2026-10-06 - D08Q4 follow-up: `dnkroz` gives the jetpack (needs playtest)

The user's D08Q6 retest showed that `dnkroz` did not give a jetpack. Confirmed
live: from a clean start god mode turned on but the pack stayed unowned.
Unlimited fuel for an owned pack already worked. The user asked for it to also
give the jetpack. Turning god mode on now sets the owned bit as the original
inventory cheat `8003d738` does, and `god_mode_update` fills it. Verified
live: `dnkroz`, then J, then flight at full fuel, and off keeps the pack. The
native D08Q4 group and the suites pass. Not committed.

## 2026-10-06 - D08Q6 revision 3: Shift and arrow keys in flight (Needs playtest)

User: "something is causing the shift key to change height, and i cant use the
arrow keys to move around". Shift (walk = L1) reached the original hover
toggle, which re-anchored the hover 128 units off the parked-peak base. It is
now withheld in Modern flight (Classic keeps it), and hover re-seats if
anything else moves the base. The arrow keys sent the raw D-pad: thrust the
host held in the hover pin, and turns that `face_view` overrode. In flight they
now fly exactly like WASD. `dnkroz` now keeps the jetpack while god mode is
on, because god mode can survive travel and savestates that drop the pack.
Live: arrows fly, Shift changes nothing, 1297 mixed updates give a largest
step of 7 and a net of 0. Suites pass. Not committed.
[Note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 4: Right Shift dropped capture; idle creep (Needs playtest)

The user reported "duke cannot seem to move ... with wasd keys, unless you are
ascending or descending" and slow idle creep to the right. The user's session
log showed capture repeatedly released in flight. Right Shift (the fixed
escape hatch's Select, not excluded while captured) reached
`input_pad_context()`, which silently released capture. The host stopped with
the original hover lock on, WASD crawled, and Space cleared the lock.
Game-wide fix: captured, Right Shift is Shift and never Select. The idle creep
was a stalled horizontal coast (bv 203/-21), now zeroed below 256 while
hovering. Both were reproduced and fixed live on a copy of the user's
profile. Suites pass. Not committed.
[Note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 5: Modern flight model rewritten (Needs playtest)

The user's 16:54 session ran the revision 4 build with no capture loss, yet
flight still felt "really broken". From-scratch re-evaluation:

- Calibrated the handler: bv * dt / 1024 per update.
- Found that `8004ac08` drops horizontal motion whenever the vertical root
  is 0 (8004ac28 beq to 8004ad1c). This was the real "cannot move unless
  ascending/descending" cause, made common by the D08Q6 precise hold.
- Replaced the layered Modern design (original momentum + hover lock + host
  patches) with a host-owned flight velocity: eased camera-relative
  WASD/arrows at 1600, Space -1500, Ctrl +1640, a held height with the floor
  cap, a +-1 vertical step while moving, and dt fuel drain. The original gets
  no flight pads.

Live on a copy of the user's profile: moves at every height, steady 15 units
per update with about 0.15 s ramps, height within +-5 on the circuit. Landing,
J fall, firing, switching, Classic and Vanilla are unchanged. All suites pass.
Duke keeps the hover pose while moving. Not committed.
[Note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 5b: original speed, flame and poses (Needs playtest)

User: "now the jetpack has no fire etc, and moves very slowly". The 5a speeds
came from a stale units-per-frame note and were about 10x too slow. A
post-handler trace of the original gave a horizontal top of about 16000 and a
climb of -11832, now the Modern targets. With no pads fed, the original drew
no flame and no lean poses. The pads are fed again and their thrust is
cancelled using the handler's thrust read right after it (`0x80058120`), and
the flame flag is set while hovering. The +-2 vertical step now corrects for
the applied-root offset learned after the handler (a new `8004ac08` hook
would need regenerated code). Screenshots match the original; height holds;
landing, J, fire, switching, fuel, Classic and Vanilla are unchanged; all
suites pass. Not committed.
[Note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 accepted

User: "i accept this as fixed! the test passes and this feels great" (revision
5b). The `dnkroz` jetpack grant (D08Q4 follow-up) was part of the same test
flow. Not committed.

## 2026-10-06 - Backlog: D24A public clone gives the full experience

A fresh clone of 5f9e705 built and played identically, except for the
locally built visual assets: the TTK inventory icons (disc-derived, not run by
`build.py`), and the Duke message font and green digits (Duke Nukem 3D art
from `research/`). New Todo D24A moves these out of `research/` into a proper
build route, subject to the licensing decision recorded in the job.

## 2026-10-06 - D24A: Time to Kill disc font survey and picker

The user chose route (a) and asked for every Duke Nukem 3D font and digit to be
replaced by Time to Kill's own. A survey of the owned disc found four fonts:
- **TTK Big Italic** (`/DATA/FONTS.RAW`, VRAM y 0..70): 17 px, 48 glyphs,
  shaded ramp. `, - . / 0-9 A-Z : ; < > = ? @ '`.
- **TTK Medium Italic** (y 72..106): 11 px, 50 glyphs, the same set plus (c)
  and TM. The original pause menu and briefing font.
- **TTK HUD Digits** (x 992..1018, y 109..119): `% - : 0-9`.
- **TTK System 8x8** (`SLUS_005.83` at `0x800c7d70`, PSY-Q `FntPrint` font,
  128x32 1-bit, ASCII 0x20..0x5F): the original Select inventory list font.
None has lower case. The italic fonts lack `! % ( ) & + "`. The CLUTs are
in `FONTS.RAW` column x=1008. CLUT 225 (gold) matches the pause menu text in a
live capture (`analysis/d26a-20261004/shots/paused.png`) colour for colour. The
same file holds the button sprites (records `0x800c43b4..0x800c4474`: cross,
circle, square, triangle, L1/L2/R1/R2, Start, Select, D-pad) next to the HUD
item icons.

A local picker (`recomp/analysis/d24a-fonts/ttk-font-picker.html`, retail-
derived, ignored, generated by `make_picker.py` in the same folder) renders
every font and palette from the disc. It lets the user choose a font,
palette/colour, scale and spacing for each use (menus, inventory, mission
items, saves, messages, numbers, console), then copy the choices. No build or
runtime change has been made yet. Next: once the user chooses, add a build-time
extractor for the chosen fonts and digits. Then remove the Duke 3D font, digit
and frame inputs and the `research/` dependency, and re-run the fresh-clone
test.

## 2026-10-06 - D24A: UI element survey, armor correction, steroids

On the user's review, the icon D08A3 recorded as steroids (`0x800c44f4`) is the
armor icon (its routine `0x8008be00` draws `player+0x234 / 100`). The icon at
`0x800c44d4` is oxygen (`0x8008c560`, the air timer, shown underwater). The
skull at `0x800c4554` is drawn by `0x8008c6a8` with a count: `player+0x3ae`
in the two-player arenas and `0x800d253c` in Challenge Stages 1-6 (probably
kills). Corrected doc 75, the icon builder label and its test (Python test OK).

The picker gained a "UI elements on the disc" section:
- the full button set;
- the HUD icons with what draws them;
- the health (36x16, `0x800beb08`) and item/ammo (46x16, `0x800c44a4`)
  number frames;
- the 4-frame scrolling ammo icon (`0x800c4514..44`);
- the 96x96 radiation disc (`0x800c4454`, use not traced);
- an approximate HUD mock-up;
- art no record references (a 64x64 hatch plate and an 18x66 rim, palette
  guessed);
- the nine full-screen MDEC stills, decoded at 512x240 by wrapping each BS
  frame in synthetic STR sectors for ffmpeg: COPY and WARNING (boot notices)
  and FAIL00..06 (game-over pictures).
Font records at `0x800c4484..0x800c449c` give the default CLUTs: 225 (gold) for
the italic fonts, 227 (red) for the HUD digits, and 242/243/244 (one colour each:
#dedede, #00bdef, #848484) for the system font. The picker now defaults to them.

Steroids: no 2D steroid art exists on the disc. The pickup (`0x800827e8`, object
type 638 at object+44) sets `player+0x364` value 2, and the HUD has no branch
for it. Options recorded for the user: an in-game capture of the 3D pickup
(needs a known location), then locate its texture in the level VRAM pages
(disc-derived, can be built), or render the live model (format undecoded).

## 2026-10-06 - D24A: per-level mission items and keys

The user will draw original open icons for the mission items, keys and key
cards, and asked for a per-level placeholder list. Survey
(`recomp/analysis/d24a-fonts/items/survey.py`): private Xvfb Modernized run,
private card copy. Each level loads with the console `level N`, `dnitems` is
typed (it calls the original level-aware key/item grant), and the original
Select inventory is captured. Results, as the game names them:
- 0 TIME TO KILL: Subway Security Key, Transport Room ID, Red/Blue/Green Energy Crystal.
- 1 DUKE HILL: Skeleton Key, Scrap of Paper, Old Note, Torn Paper (the combo pieces).
- 2 MINER 69ER: Skeleton Key x2.
- 3 GOLD AND GUNS: Skeleton Key.
- 5 OBEY OR DIE: Warehouse Key, Red/Blue/Green Energy Crystal.
- 6 FAMILY JEWELS: Skeleton Key x2, Family Jewel x3.
- 7 RESISTANCE IS FEUDAL: Gantry Key, Valve Key.
- 8 HOLY TERROR: none.
- 9 PIG FACTORY: Lab Key, Valve Room Key, Red/Blue/Green Energy Crystal.
- 10 HOG HEAVEN: Skeleton Key x2.
- 11 LET THE GAMES BEGIN: Skeleton Key.
- 12 BLOOD BATHS: none.
- 21-26 (Challenge Stages): the Select inventory does not open.
- 27-29 (bosses): no mission items.
That makes 15 distinct designs. The picker page has a "Mission items and keys"
section: a design checklist (16x16 suggested, file name per design), each level's
objectives (strings 325..410), item slots and the reference capture. A PNG
dropped on a design previews it everywhere (stored in browser storage only).
The survey shows what the original grant gives. It has not been cross-checked
by playing each level for the physical pickups.

## 2026-10-06 - D26E: debug spawn console command (Needs playtest)

New job at the user's request (EDuke32 `spawn`). The backtick console gains
`items` and `spawn <name|type>`. They queue the type, and the Modernized update
(after the cheats, `0x8005a210`) calls the original `CreateObject` (`0x80095a74`)
on a copied CPU, with a position 600 units ahead of Duke and his cell as the
hint. The new object gets `+0x35 = 1`, as the drop routine `0x80096b98` sets.
A new code guard covers `0x80095a74`. `cheat_call` gained optional a1, a
stack buffer for a2 and the v0 result; the existing monster-toggle calls are
unchanged. Framework: two `help` lines in `main.cpp`, exported to the patch.
Evidence (private Xvfb Modernized runs, private card copy):
- Level 6: `spawn steroids` spawned type 638. Walking into it showed the
  original STEROIDS message and set `+0x364` to 2.
- Level 0: `spawn key 1` / `key 2` spawned 153/154. Picking them up showed
  SUBWAY SECURITY KEY / TRANSPORT ROOM ID, and both appeared in the Select
  inventory.
- Level 1: `spawn scrap of paper` and `spawn torn paper` were both picked up
  and appeared in the inventory.
- Refusals: unknown name or number; `spawn 547`/`854` in level 0 ("not loaded
  in this level"); Vanilla ("Modernized profile only"); a list at `items`.
- `ttk-controls-test` passes; `level_overlay_guards.py --check` matches; the
  framework patch is exported.
Limits: the level 0 green crystal (761) spawns and is visible, but it was not
seen in the inventory after walking over it, so its pickup is unconfirmed. Level
0's red and blue crystals were not found as types at level start (their models
are not loaded then), so they have no names yet. Types 155-157 share the key
model and set flags that level 0's inventory does not name. Non-pickup types
(for example scenery 858) can be spawned by number and can block Duke.

## 2026-10-06 - D24A: first original UI art (selection frame)

The user made the first original asset, `item-frame.png` (25x23 RGBA, same size
as the Duke Nukem 3D tile 20 frame). It is imported into the project at
`recomp/assets/ui/item-frame.png` (tracked; byte-identical to the research copy;
`assets/ui/README.md` records it as original project art under the repository
licence). `build_ttk_inv_icons.py` now writes the pack's selection frame (kind 1)
from that PNG (`--frame`, stdlib RGBA reader) instead of copying the old pack's
Duke 3D frame. The runtime format is unchanged (tile field 0, ignored by the loader).
Evidence: the rebuilt pack's frame equals the PNG pixel for pixel. The builder test
(updated: the frame must equal the project PNG) passes. A private Xvfb Modernized
run in level 0 (`dnitems`, `]` three times) shows the grey frame moving across
Bio Mask, goggles and medkit. The previous pack is kept locally as
`recomp/analysis/d08a3-ttk-icons/ttk-inv-icons.before-d24a-frame.pack`. The
picker page gained an "Original project art" section. The digits, fonts and
the font/icon build steps are still the remaining D24A work.

## 2026-10-06 - D24A: original crosshair

The user hand-drew `crosshair.png` (9x9, original lime `#80ff00`, same shape as
the EDuke32 CROSSHAIR tile 2523 it replaces). It is imported at
`recomp/assets/ui/crosshair.png` (tracked). The compiled `k_crosshair` table in
`weapon_aim.cpp` now holds its pixels, and the new `tests/local/test_ui_art.py`
fails if the table and the PNG differ (it also checks the item frame).
Evidence: the UI art and icon builder tests pass; `ttk-aim-test` and
`ttk-controls-test` pass. A private Xvfb Modernized run in level 0 (weapon drawn,
view aim) shows 12 pixels of exactly `#80ff00` at the screen centre in the
crosshair shape. The manual is updated; the picker's "Original project art"
section now shows both assets.

## 2026-10-06 - D24A: open switcher digits (3x5 Microfont); credits

The user first proposed Mythic Pixels (CagyTrain, FontStruct). Its FontStruct
Non-Commercial EULA forbids distribution (2.3), modification (2.4) and
commercial use (2.2), so it was not used. Nothing from it was added to the
project. The user then chose the 3x5 Microfont by nimaid (CC0 1.0,
https://github.com/nimaid/microfont, commit 8c57fbb). Its 3x5 digits and `%`
replace the Duke Nukem 3D THREEBYFIVE digits glyph for glyph.
- The upstream `3x5-Microfont_1D.png` and its `LICENSE` are tracked, unchanged,
  in `recomp/assets/ui/fonts/microfont/` (with `SOURCE.md`).
- New `tools/local/build_ttk_inv_digits.py` builds the TTKDIG3 pack (tiles
  3010-3019 and 3076, unchanged format), in the old switcher green `#989c58`.
- CMake now generates `ttk-inv-digits.pack` in every build from the tracked
  sheet. It no longer copies a local Duke 3D pack, so a fresh clone gets the
  digits with no `research/` input. The old pack is kept locally under
  `recomp/analysis/d08a3-ttk-icons/`.
- Evidence: `test_ui_art.py` (builder reproducible, every glyph equal to the
  sheet, CC0 licence present), the icon builder test and `ttk-inventory-test`
  pass. The repo check passes. A private Xvfb Modernized run (level 0,
  `dnitems`, `]`) shows "100%" under each switcher icon in Microfont,
  `#989c58`, with the new frame.
- README gained a Credits section: PSXRecomp (RetroPortingToolKit; Matthew
  Stanley and team; PolyForm Noncommercial 1.0.0, with its third-party
  attributions), Alexbeav's PS1 Recomps (psxrecomp-ports; our submodule is
  pinned from Alexbeav/psxrecomp), recomp-ui (MIT), 3x5 Microfont (CC0) and
  the original UI art.
- Still open in D24A: the message/menu fonts (Duke 3D message font and Atomic
  headings) and the disc-derived icon extraction in the normal build.

## 2026-10-06 - D24A: TTK disc fonts replace the Duke 3D fonts (Needs playtest)

The user chose fonts per use in the picker. What exists today:
- messages: TTK Big Italic, Console steel, 1x, shadow;
- console: TTK Medium Italic, Console steel, 1x, shadow;
- headings: Big Italic, gold, 2x;
- missing glyphs: the system 8x8.
New `build_ttk_fonts.py` builds `ttk-fonts.pack` (TTKFONT2) from the prepared
disc at build time. New `ttk_font.cpp` renders it with the same API, so the
framework is unchanged. CMake also builds the HUD icon pack from the disc (it
is byte-identical to the accepted pack). The Duke 3D font builder, its tests and
`duke_font.cpp` are removed. With the earlier Microfont digits, frame and
crosshair, no Duke Nukem 3D art or `research/` input remains. Evidence:
- the native font test and all 114 Python tests pass, as do the inventory and
  controls tests;
- a private run shows the console and a cheat message in the new fonts.
Remaining: the user's look confirmation and the post-commit fresh-clone test.
See [note 112](documentation/112-d24a-open-assets.md).

## 2026-10-06 - D24A follow-up: in-game font sizes, scrollable console

The user found the chosen fonts much bigger in the game than in the picker. The
picker drew one font pixel per screen pixel. The game stretches the console to
the window width (/640) and scales messages and overlays by height/480 (rounded
down). The picker now has a **Preview size** selector:
- the user's 1461-wide window, at 16:9 and 4:3;
- 1080p, 1440p and 4K;
- a custom size;
- the old 1:1 view.
It applies those rules, corrected for display DPI, and the per-use defaults
are now the user's current choices, so the sizes can be re-picked. No font
change in the game yet; waiting for the new picks.

The console is now scrollable (framework `host_osd.c`/`main.cpp`, exported to
the patch; no header change):
- 512-line history (was 24);
- PgUp/PgDn 8 lines, mouse wheel 3, Ctrl+Home to the oldest, End to the newest;
- new output returns to the bottom;
- a `-- MORE BELOW --` marker shows while scrolled up.
Evidence: build OK. A private Xvfb run (four commands, then PgUp twice, wheel
down, End) shows the view moving back, forward and returning to the newest
lines.

## 2026-10-06 - D24B: savestate menu in TTK fonts and disc art (Needs playtest)

User: "build it out!" after the mockup. New `src/ttk/savestate_panel.cpp` draws
the F7 panel (640x480) to the approved design, in Modernized:
- steel gradient backdrop with the disc's radiation emblem watermark;
- header "SAVE STATES" (Big Italic gold) with the F7 label and slot range;
- three slot cards: 104x78 thumbnail, "SLOT NN - <level name>" (Medium Italic
  gold, blue when selected), save time and "LEVEL N <era>" in System 8x8;
- a prompt bar with the disc's button sprites.
The font pack gained five panel sets (8 in all), and the builder now also
writes `ttk-ui.pack` (TTKUI1: Up/Down, Cross, Square, Circle, Triangle and the
radiation emblem from the HUD sprite table). The level comes from a `.ttk`
sidecar written beside the slot (`level N`) when a save succeeds, while guest
RAM still holds the saved state; older slots show the number and time only.
Framework: `psx_savestate_menu_set_panel_renderer()` (the generic panel stays
the fallback) and a `ttk_savestate_saved()` call in the save notice; no header
change; the patch is exported.
Evidence (private Xvfb runs, private card copy):
- Level 6: F7, Down, Shift+Enter saved slot 2 and wrote `level 6`; the panel
  showed "SLOT 02 - FAMILY JEWELS", "LEVEL 6 MEDIEVAL", the new thumbnail and
  the other slots unchanged.
- From level 1, loading slot 2 through the panel reached level 6, in
  Modernized and in Vanilla.
- Vanilla shows the framework panel.
- 115 Python tests OK (new UI sprite-pack test, 8-set font pack).
- `ttk-font-test`, `ttk-inventory-test` and `ttk-controls-test` pass; the repo
  check passes.

## 2026-10-06 - D24A follow-up: half-scale text (user, 1080p)

The user still found the console and cheat messages much bigger in the game
than in the previews, and asked for half scale. They play at 1080p, where the
overlay scale (drawable height / 480) is 2 and the 640-wide console stretched
3x.
- Messages and the fps/debug text now draw at half the overlay scale (whole
  pixels, at least 1x), in both the OpenGL compositor (`gpu_gl_renderer.c`)
  and the SDL path (`host_osd.c`).
- The console canvas is now 1280x440 (was 640x220), so each font pixel takes
  half the screen space and the console still covers the same top third.
- The inventory switcher, crosshair, volume bar and the F7 panel are unchanged.
Evidence: a private Xvfb run in a real 1920x1080 window shows the console lines
at half their old height, with more history visible, and "GOD MODE: OFF" at
1x (about 17 px). The picker's Preview size uses the new rules and defaults to
1920x1080. The framework patch is exported.

## 2026-10-06 - D26E fix: spawn from the real console

The user always got "Spawn unavailable here". Typing in the console released the
capture, and the spawn update required captured input on the next frame (the
earlier tests used the debug-port console, which does not release it). The spawn
now needs only a verified Modernized gameplay player and waits up to ~5 s. A
private copy of the user's UI slot 4 spawned steroids through the real console.
See [note 111](documentation/111-d26e-debug-spawn.md).

## 2026-10-06 - D26E: landing height is the game's; camera-direction spawn; console history

User: spawned items wedge half into the ground; also asked for Up to recall the
last command and a `history` command.
- The landing height is the game's own. Our spawn uses the original drop
  (`+0x35 = 1`). On the user's slot 4 street, a spawned item and a pig cop's
  natural ammo drop both rest at y -9276. The original fall overrides any start
  height.
- Spawns now go along the camera view instead of Duke's body facing.
- Console: Up/Down recall typed commands and `history` lists them (64 kept).
Evidence: private runs on a copy of the user's UI slot 4. Spawn through the
real console, the kill-and-drop comparison, the W walk toward the item, and the
Up recall and `history` output. See
[note 111](documentation/111-d26e-debug-spawn.md).



## 2026-10-06 - Session close: D24B and D26E accepted

User: "im happy with all progress tonight. the document is superb and an amazing
piece of research." D24B and D26E accepted. D24A keeps Needs playtest only for
the fresh-clone test against the pushed commit. The research page is linked from
`research/TTK-UI-and-Font-Research.html` (local). Next: the user designs the
open mission-item, key and steroids icons.

## 2026-10-07 - D24A/D26E check: level 7 keys are keycards in the original

The user asked whether `gantry key` (type 153) in medieval level 7 is right,
since it spawned as a blue keycard. It is the game's own object. Level 7's
actor 4 (type 57) carries type 153 and actor 5 carries type 154, and the pickup
names them GANTRY KEY and VALVE KEY there. Model 420 is the same flat card
(box -37..37 x -64..64, zero thickness) in levels 0, 5, 7 and 9. Levels 5 and
9 place their 153/154 keys directly. So all seven key-slot items share the
keycard model; only Skeleton Keys differ. The research page now labels them as
keycards.

## 2026-10-07 - D24A: the user's mission item, keycard and steroids icons

User: "all the icon images have now been created". Nine original 16x16 RGBA
icons by MusicMonsterMod are imported (unchanged) into
`recomp/assets/ui/items/`:
- keycard: all seven key-slot items share it, as the game uses one keycard
  object;
- skeleton key;
- red, blue and green energy crystals;
- scrap of paper: also Torn Paper (user-confirmed: "scrap of paper can be used
  for both scrap of paper and torn paper. old note is unique");
- old note;
- family jewel;
- steroids (HUD).
`items.json` maps each of the 15 mission items and steroids to its file, the
game's object types and levels. `test_ui_art.py` checks that every design is
covered, every file is 16x16 RGBA, and no file is unused (test OK; repo check
OK). The research page loads them into its design slots and lists them under
"Original project art". Nothing in the game draws them yet: they await the
expanded mission item inventory and D08A4 (portable steroids).

## 2026-10-07 - Mission item tracking: mockups for the user's choice

The user asked for mockups of ways to track mission items in the game ("a bit
like how you did save states"). The research page (section 8) shows four
options, with the user's icons, the chosen TTK fonts, disc HUD and button art,
each level's real items and objectives, and a per-level gameplay frame:
- A: a mission panel on a key, pausing like F7;
- B: an always-on HUD tracker, top right;
- C: a pickup card with progress;
- D: a mission row in the `]` switcher.
Found / missing states can be toggled. The game already keeps each mission item
as a player flag (`+876`/`+880` key slots, `+884..+892`, `+908..+916`), so any
option can read live state. No job is scheduled until the user picks.

## 2026-10-07 - D08A5 queued: mission item tracking (design E locked)

The user approved mockup E ("lock it in and stand up a ticket to get that built
in the game"). E is the mission row under the `]` switcher:
- an orange tile0020-palette selection frame on the gadgets;
- grey framed mission slots with silhouettes for missing items;
- "MISSION" and the count at the bottom;
- `\` browsing that turns the browsed frame steel blue;
- a fixed 592-wide card at the top with the name, type and
  FOUND / NOT FOUND YET.
New Todo D08A5 records the exact spec. The research page marks E as chosen.

## 2026-10-07 - D08A5 built: mission item tracking (Needs playtest)

Implementation of the locked design E. Details and evidence in
[note 114](documentation/114-d08a5-mission-tracking.md).

- **Data:** the original Select inventory lists item i when its name function
  `0x80087d4c` names it for the level and `player+0x354+4*i` bit 0 is set.
  Running that function for levels 0-31 gives the mission table now compiled
  into `inventory_hud.cpp`. Corrections to the ticket: crystals are items
  11-13 (`+896/900/904`); level 6's skeleton keys are items 7-8 (`+880/884`).
  `test_mission_items.py` re-derives the table from the prepared executable.
- **Look:** mission row under the gadgets (translucent grey panel, grey framed
  slots, silhouettes, 2x Microfont "MISSION" and count, green when complete);
  gadget selection frame in the tile0020 orange; `\` browsing turns the slot
  frame steel and shows the 592x52 card at the top (name gold/blue, type,
  FOUND / NOT FOUND YET), clearing 2.5 s after the last press.
- **Behaviour additions to note:** `\` opens the switcher on the first slot when
  it is closed; the switcher opens for the mission row alone when Duke carries
  no gadget.
- **Code:** `inventory_hud.cpp/.h`, `shortcuts.inc` (level and flags each
  update), new action `mission_browse` (Backslash) in `input_bindings.def`,
  `pc_input.cpp` / `pc_input.py`, profile schema 27 adds it to saved
  bindings; new `build_ttk_mission_items.py`
  (`ttk-mission-items.pack`, every build, no disc); font sets 8-11 in
  `build_ttk_fonts.py`; framework `host_osd_card_image` (GL and SDL
  presenters) exported to the accepted patch. Codegen hash unchanged.
- **Evidence:** `ttk-inventory-test`, `ttk-input-test`, `ttk-font-test`, Python
  120 OK (6 skipped). Private Xvfb Modernized run: level 6 row 0/5, card and
  steel frame follow `\`, timeout closes both, a real jewel pickup shows 1/5;
  level 0 shows 0/5; level 8 has no row or card. Vanilla shows nothing.
  `ttk-controls-test` fails at its first assertion with the available
  fixtures with or without this job (stale fixture).
- **Limits:** no controller D-pad focus (spec item 5); the block sits about
  26 px higher than the mockup because the switcher keeps its anchor; only one
  live pickup type was walked; the user's look confirmation is pending.

## 2026-10-07 - D08A5 playtest fixes: UK backslash key, even frame borders

User report: "\\ did not seem to allow me to
  cycle": the user's keyboard is UK (`gb`, pc105). There the key that types `\`
  is left of Z (SDL `NONUSBACKSLASH`, 100); scancode 49 types `#`. The default
  binding now also answers that key and any key whose layout keycode is `\`.
  Verified on a private Xvfb run with the `gb` layout set before launch,
  sending the raw key (keycode 94): `]` opened the row and `\` stepped the steel
  frame 1 -> 2 -> 3. "Bottom border ... looks thinner than the top": the 25x23
  frame was stretched to 42x39 by plain nearest-neighbour, giving the mission
  slot frames a 2 px top and 1 px bottom (right side thinner too), and the
  orange gadget frame 3 px / 2 px. Both stretchers now sample pixel centres
  (2/2 and 3/3). The mission panel's outline was also the surface's last line;
  the surface now has 2 transparent lines under it (132 lines).

Still Needs playtest: the user confirms the look and browsing in play.

## 2026-10-07 - D08A5 redesign: separate mission inventory on , / .

User, after trying the combined switcher: "separated out the inventory with the
mission items ... revert the inventory back to [/] and thats it, just
inventory. then the mission inventory replaces whatever ,/. are bound to
(visually looks like </> so thats cool)". Reason: an instinctive Enter while
browsing mission items switched the jetpack on. "then you can remove the usage
of \ and #".

- `[` / `]`: gadgets only (orange selection frame kept).
- `,` / `.`: mission inventory (panel in the switcher's place, steel-framed
  selected slot, item card at the top). First press opens on the item last
  shown, then previous / next with wrap; closes 2.5 s after the last press.
  `[` / `]` and `,` / `.` close each other's view.
- Enter / U while the mission inventory is open close it and are not used
  (also in jetpack flight).
- Bindings: `original_strafe_left/right` default Unbound; new
  `mission_previous` (Comma) / `mission_next` (Period); the `\` action and its
  UK alias removed. Profile schema 28 migrates Comma/Period from strafe when
  strafe still holds them, and drops schema 27's `mission_browse`.
- Evidence: `ttk-inventory-test`, `ttk-input-test`, Python 120 OK, scratch copy
  of the player's profile migrated 27 -> 28 correctly. Private Xvfb level 6:
  gadgets only on `]`; `.` / `,` browse with wrap; Enter closed it with the
  jetpack unchanged and the next Enter switched it on; `[` returns to gadgets.
- Still Needs playtest: the user confirms it in play.

## 2026-10-07 - D08A5 accepted

User, after playtesting the separate mission inventory: "i fully accept!".
D08A5 is Done. Open for later: controller input for the gadget switcher and
the mission inventory.

## 2026-10-07 - D08A6 built: selected gadget on the HUD (Needs playtest)

Design A is in the game (Modernized only). Details in
[note 115](documentation/115-d08a6-selected-gadget-hud.md).

- **Research:** status bar `0x8008ba30` draws each element from the layout
  `0x800dd778` and state `0x800dd7b8`. The jetpack box is element 4 at
  (169,69), 20 rows over ammo (box 16 + 4); Bio Mask / goggles are element 5 at
  (82,89), left of ammo, sliding out of it. They are never on together (the
  original switches the other off), so the original shows at most two gadget
  boxes. The medkit has no HUD element. A box is `0x8008b98c` (number),
  `0x8008b454` (red digits, flags 0x310, POLY_FT4 glyphs at colour 0x80) and
  `0x8008b678` twice (icon, then box `0x800c44a4`).
- **Build:** new `src/ttk/gadget_hud.inc`, called at the status bar entry after
  the D14 shift. A selected gadget that is on is the original box (element 5
  moved into the slot for Bio Mask / goggles); otherwise the box is drawn with
  the original's own calls, and off gadgets get their glyph packets at colour
  0x31 (38%). An unselected jetpack that is on moves one row up.
  `widescreen.inc` now saves and restores the whole layout (x and y) for every
  single-player status bar draw. No framework, codegen or profile change.
- **Evidence:** private Xvfb runs (`recomp/analysis/d08a6-hud/`, local): all four
  selections with dim / lit digits, cycling both ways, N / J / Enter on and
  off, jetpack bumped up while medkit or Bio Mask is selected, savestate load,
  level change, GL 4:3 and 16:9, Software 4:3, Vanilla with gadgets given (no
  box). `ttk-inventory-test`, `ttk-input-test`, Python 120 OK.
- **Limits:** death / Continue, in-play depletion and resize not run. Software at
  16:9 cuts off the whole right HUD with or without this change (existing
  D14 / Software issue). Medkit reads "100" with a health cross. No flash on
  change. Unselected Bio Mask / goggles stay at their original place instead
  of stacking. The user confirms the look in play.

## 2026-10-07 - D08A6 playtest fix: box followed a stale selection at 120 fps

User: "medkit was selected in the new box, but i selected jetpack from the
inventory picker, and now both medkit and jetpack appear in the bottom right";
"just by hovering over the item, you should be making it hot. enter is what
actually uses it."

- **Cause (reproduced):** with the user's profile (first person, 120 fps) the
  in-between images are redrawn by D17 worker processes that replay the frame
  on a copy of guest memory, status bar included. The box read the host
  selection, which is stale in a worker (the medkit), so replayed images
  showed the old selection while live images showed the new one. 60 fps has no
  replays, which is why the first tests passed. Intermittent: two of four runs.
- **Fix:** the box reads the selection from guest memory, the original menu's
  remembered ID `0x800c3f94` (already written by `[` / `]`); `select_weapon`
  now also puts that ID back on the selection when the original menu left it
  on a key or an unowned item (unless that item is mid-activation).
- **Hover everywhere:** `[` / `]` now move the selection in every live
  gameplay state (jetpack flight, swimming, firing, weapon animations);
  before, they were ignored there. Only Enter / U keep the use restrictions.
- **In place:** when the selected jetpack switches on, its original slide up
  out of the ammo box is skipped (slide offset set to its resting 0), so the
  lit box replaces the dim one.
- **Evidence:** private Xvfb runs from a copy of the user's savestate slot 3
  with the user's settings (first person, 120 fps, 150% CPU, 16:9): `dnstuff`,
  M, `]`, Enter, three runs, four captures per step: the box is the medkit,
  then the dim jetpack, then the lit jetpack in every capture; in flight `[`
  moves to the medkit (jetpack box one row up, fuel counting down). 60 fps
  third person the same. Native tests and Python 120 OK.

## 2026-10-07 - D08A6 accepted

User, after playtesting the fix: "that's absolutely superb. great and amazing
quality work! i fully accept." D08A6 is Done. Open for later: the medkit's
number and a flash on change (left open in the spec), Software 16:9 right HUD
cut (existing D14 issue).

## 2026-10-07 - D08A7 queued: custom medkit gadget icon

User supplied `research/inv/medkit.png` and asked for it to replace the
health cross as the medkit's inventory picture (strip and HUD box), with
health unchanged. Findings: the box draws the medkit with the health record
`0x800c44e4`, so a new 4bpp cell, CLUT and record are needed; the HUD sheet
has unused cells and CLUT rows; the HUD box limit is 15 colours plus
transparent. Made `research/inv/medkit-15col.png` (k-means, PSX 15-bit, no
dither) at the user's request. D24A page: `make_picker.py` reads both PNGs
(the 15-colour one for the HUD box mockups, the original for the strip),
section 9 has a D08A7 card and mockup A opens on the medkit; checked in
headless Chrome. New Todo D08A7. No game code changed.

## 2026-10-07 - D08A7 built: custom medkit gadget icon (Needs playtest)

- **Art:** the user's `research/inv/medkit.png` and its 15-colour reduction are
  tracked as `recomp/assets/ui/items/gadget-medkit.png` and
  `gadget-medkit-15col.png` (own art), listed in `items.json` (`"gadget": 5`;
  the mission item builder skips gadget entries).
- **Strip:** `build_ttk_inv_icons.py` takes item 5 from `--medkit`
  (default the full-colour PNG), trimmed; provenance records it; CMake
  depends on the PNG.
- **HUD box:** `src/ttk/gadget_hud.inc` draws the medkit with its own record
  (the cross record `0x800c44e4` with a new CLUT id and cell, on the saved
  stack). The 4bpp cell (960,205) and palette row (1008,206) sit in rows the
  disc's HUD sheet leaves empty. Each frame the medkit box is drawn, a 47-word
  packet (GP0 `A0` cell, `A0` palette, `01` cache clear) from the HUD packet
  ring goes into ordering-table slot 0 through the original add `0x8002bc18`,
  ahead of the icon. The game's own DMA carries it, so no GP0 write races a
  transfer, and level loads, savestates and D17 workers need nothing extra.
  The health box keeps the cross. No framework, codegen or profile change.
- **Research:** record format and UV / tpage derivation from `0x8008b678`;
  OT add semantics from `0x8002bc18` (head insert); linked-list DMA is
  asynchronous (`dma.c`), which ruled out direct GP0 writes. Live VRAM
  survey against `/DATA/FONTS.RAW`: rows 205-222 stay empty (intro movie,
  title, savestate load, pause, Select screen, levels 1, 2, 3, 5, 7, 9); the
  game fills x 960-991 from row 223 down at run time (610 texels), which the
  first planned cell (rows 208-223) would have touched.
- **Evidence:** private Xvfb runs (`recomp/analysis/d08a7-medkit/`, local):
  cell and palette in VRAM equal the tables once the box draws; GL 16:9 and
  4:3, Software 4:3 captures show the medkit in the box, the cross in the
  health box and the full-colour medkit in the strip; 120 fps first person
  16:9: four consecutive captures, savestate saved with the medkit selected,
  level change and reload, M use; Vanilla with all inventory: no box and no
  upload. `test_ui_art.py` re-derives the tables from the PNG;
  `ttk-inventory-test`, Python 121 OK (2 skipped), `check_repo.py` OK.
- **Limits:** death / Continue, an in-play FMV, resize, a statistics-screen
  transition and the bonus / challenge / boss levels not run. Software 16:9
  right HUD cut is the existing D14 issue. The user confirms the look in play.

## 2026-10-07 - D08A7 accepted

User, after playtesting: "amaazing work. i accept". D08A7 is Done. The
untested states listed above stay as notes, not open blockers.

## 2026-10-07 - D26F research: why the spawned green crystal is not in the inventory

User report from level 0 (UI slot 4): `spawn subway security key` shows in the
inventory but `spawn green energy crystal` does not. Research only; no game
code changed.

Evidence:
- Private Xvfb runs on a copy of the user's cards, slot 4, level 0. Walking
  over a spawned 761 changed only item 14 (0 -> 1); items 11-13 stayed 0.
- The pickup dispatcher `0x80081a48` has no case that writes `+0x380/384/388`.
  761 is the only case with the crystal message.
- `0x80091cec` (player animations 0x83/0x10c) takes the crystal from the holder
  at `player+0x290`. Holder state 1/2/3 picks the level object slot
  (`0x80092d78`: level 0 -> 6/10/19, level 5 -> 2/3/4, level 9 -> 19/20/21).
  Crystal type 176/177/178 picks item 11/12/13.
- In UI slot 3 (level 0), those slots hold 176, 178 and 177 inside holders 180,
  180 and 181.
- Model data: identical vertices. Face CLUT red `0x0bb9`, green `0x1538`.

Correction: D26E's limit "red and blue crystals are not loaded" was wrong.
Their models are loaded. They are just not walk-over pickup types.

Remaining: the user chooses a fix (A or B). A real crystal take from its holder
was not played; the path is from code reading plus the live object table.

## 2026-10-07 - D26F built: spawnable crystals as custom pickups (Needs playtest)

Design approved by the user ("that's how it should work, i love it!"). The user
also asked for heavy documentation, since this is how custom pickups will be
made: [note 117](documentation/117-d26f-crystals.md) has the full recipe.

- **Player-facing:** in levels 0, 5 and 9, `spawn 1761` / `2761` / `3761` (or
  `spawn red / blue / green energy crystal`) drop the level's real crystal,
  types 176 / 177 / 178. Walking over it collects it with the original message
  (113-115) and pickup sound. It sets item 11 / 12 / 13, so the Select
  inventory, the mission inventory and the receptacles follow.
  - "(already found)" when the crystal is already found;
  - other levels refuse;
  - `spawn 761` stays the generic crystal (item 14) and the console says so;
  - `items` lists the crystal numbers.
- **How:** `recomp/src/ttk/spawn.inc`.
  - Create with `0x80095a74`, mark `+0x35 = 1`, run the pickup update
    `0x800814d4` once to start the original fall/bounce. Clear the engine's
    landing value `0xff` to 0, as a pickup's own update does.
  - Collect on the pickup loop's touch test, in the dispatcher tail's order:
    unlink from cell, flag, message, sound, bounce handle, active list, free
    list.
  - The tracked list is forgotten on savestate load, level change, a type
    change or a level-owned object.
- **Evidence:** private Xvfb runs from UI slot 4 (copy).
  - Level 0: all three visible, resting at the item rest height, collected
    (flags 11-13 only, messages 113-115, objects freed); one collected
    mid-bounce.
  - The mission inventory showed 3/5 with the crystals; "already found" and
    the 761 note shown.
  - Levels 5 and 9: all three collected (level 9 green on a second run; the
    first scripted walk did not reach it). Level 1 refused.
  - `ttk-controls-test`, `ttk-input-test`, Python 121 OK (2 skipped),
    `level_overlay_guards.py --check` match. GAME_MANUAL updated.
- **Remaining:**
  - user playtest;
  - receptacle acceptance in play (the flag is what it reads; not driven);
  - a savestate load or level change with a crystal lying in the world (not
    run);
  - Vanilla refusal is the unchanged D26E path, not rerun.

## 2026-10-07 - D26F accepted

User, after playtesting: "this is awesome. it works! the crystals really do
work. lock it all in, document, commit, push". D26F is Done. The untested
cases (a savestate load or level change with a spawned crystal in the world,
the Vanilla rerun) stay as notes, not open blockers. The custom pickup recipe
in note 117 is the reference for future custom pickups.

## 2026-10-07 - D23E and D24A accepted

- User: "we can mark D23E and D24A both as accepted. we've proven these are now
  working significantly better". D23E (busy-scene stutter, [note 104](documentation/104-d23e-busy-scene-stutter.md))
  and D24A (open assets, [note 112](documentation/112-d24a-open-assets.md)) are
  Accepted. No code changed for this closeout.

## 2026-10-07 - D08O2 weapon forward while swimming and firing (Needs playtest)

- **Cause:** the player update builds Duke's model with the arm-aiming
  builder `0x80097c04` only when `+0x224` has `0x100` or `0x200000` (test at
  `0x800426f4`). Firing while floating sets `0x100`; the thrust strokes that
  D08O1 lets fire never do, so the plain builder `0x800987cc` ran and the arms
  played the stroke (gun hand about 0 units along the view, 243 when floating).
- **Change (Modernized):** new entry hook `0x800411b8` (the last call before
  that test) sets `0x100` while Duke is underwater, firing (or finishing the
  shot) with a weapon out and view aim active; it is cleared at the neck's aim
  call, the build's last, with fallbacks. Joint 1 (arms) and joint 9 (head)
  then take the view direction through the existing D07A hook, as when firing
  while floating. Skipped for an update if Duke touches an object whose
  callback `0x800411b8` would run. Guards added; codegen hash unchanged.
- **Evidence:** private level 6 lab, six underwater weapons x W/A/D/Ctrl: gun
  hand along the view matches each weapon's floating-fire pose (Desert Eagle
  243, shotgun about 205, Gatling about 115, pipe bomb about 150, Buffalo about 180,
  crossbow about 160); swimming, shots and sets/clears balanced; same at 120 fps.
  Floating fire, surface fire, ground fire and Vanilla unchanged. Controls,
  aim, input, near and Python suites pass.
- **Limits:** the legs keep the stroke, so strafing or backing twists at the
  waist; flight and holster not re-driven live (no path by construction);
  only level 6. Next: the user's playtest.
  [Note 118](documentation/118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D08O2 accepted as v1; D08O2A queued

- User: "it works and i accept this as a v1. i want v2 in the backlog
  though, which will be a more natural pose rather than his entire torso
  standing up, so it will involve moving the arms in the position as if
  firing up and his head looking up." D08O2 Accepted; new Todo D08O2A.

## 2026-10-07 - D08O2A natural swim-fire pose v2 (Needs playtest)

- **Change (Modernized):** v1's chest aim (`+0x224 |= 0x100`) is replaced.
  `0x800411b8` now only arms a host flag (no game memory written). A new
  entry hook on the matrix composition `0x800b42ec`, at the plain build
  `0x800987cc` (ra `0x8009899c`) or the aim build `0x80097c04` (ra
  `0x800980d0`), rewrites the two shoulders' local rotation `L` to
  `P^T*A*L` (`P` the stroking chest, `A` the `0x80097a44` look-at of the
  view): the arms take exactly the floating-fire pose toward the crosshair
  while staying attached to the chest. Chest, head and legs keep the stroke.
  The head is left original after the user said it already looks ahead.
  Guards for `0x800987cc` and `0x800b42ec`; codegen hash unchanged.
- **Evidence:** private level 6 lab at 120 fps, six underwater weapons x
  W/A/D/Ctrl: gun hand along the view near each weapon's floating-fire value
  (Desert Eagle 228-254 vs 245; no fire 92), chest axis unchanged from
  swimming without firing, two shoulders per build, swimming and adapted
  shots continue. Floating, surface, ground fire and Vanilla unchanged.
  Controls, aim, input, near and Python suites pass.
- **Limits:** the chase camera cannot judge the pose from the side; possible
  arm/head clipping at steep aim unverified; jetpack and holster not driven
  under Xvfb (no path by construction); only level 6. Next: the user's
  playtest. [Note 119](documentation/119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O2A single-shot fix (Needs playtest)

- User: "ok this is excellent, but doing one single shot while swimming
  forward will result in duke shooting downards. holding shoot then shows
  him shooting forwards as expected" (testing from UI slot 2).
- Cause: after release the arms stayed aimed only while the upper animation
  seen at the press played; a tap moves on to the shot's lowering animation
  (Desert Eagle 10 -> 9), which dropped the arms. Now they stay aimed until
  the upper animation returns to the one from before the press (cap 90
  updates). Lab tap traces for four weapons keep the hand forward through
  the whole shot; held fire, Vanilla and suites unchanged.
  [Note 119](documentation/119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O2A accepted

- User, after watching their playtest capture: "this isnt a showstopper,
  this is actually rock solid ... I'm happy with it and its 100% playable."
  Known minor issue from the capture (about 0:16-0:17): when a burst ends,
  one final shot can fire with the weapon pointing downwards. Not yet
  investigated; the user asked for a commit first as a rollback point.

## 2026-10-07 - D08O2A follow-up: last shot pointing down when surfacing (accepted)

- From the user's capture: a final shot fired pointing down as Duke reached
  the surface while the shot's lowering played. The arm aim's release tail
  now continues into the surface states, and it follows the original's
  upper animation flags (`0x800c2824`: shot animations `0x809` continue,
  rest/ready poses end it) instead of the pre-press animation, which also
  fixes an intermittent stuck aim (up to 3 s) after firing from a float.
  Lab: taps, surfacing, held fire, surface, ground and Vanilla checked;
  suites pass. User: "commit it at this point!! again another rock solid
  milestone." Known minor issue resolved. [Note 119](documentation/119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O3 E in water keeps the weapon (Needs playtest)

- User: "pressing E while swimming holstered my weapon and i had to press
  one of the numbers to draw it." The E redraw needed the ground lease.
  Armed Cross at a water ledge fires instead of climbing, so E keeps
  stowing; user chose automatic redraw when no climb-out follows.
- A Circle redraw could end a stroke and leave Duke idle near geometry
  (original thrust-start clearance check `0x8007c788`), so the water redraw
  uses the original weapon request (as the number keys) instead.
- Lab: surface walls, ledge climb-out, underwater idle and 11 swimming runs
  behave; land E, Vanilla and swim-fire unchanged; suites pass.
  [Note 120](documentation/120-d08o3-e-in-water.md).

## 2026-10-07 - D08O3 accepted; D08J2 and D08J3 queued

- User: "fully passed my playtest. feels great to play. i accept this job
  now as complete!!" D08O3 Accepted.
- New Todo D08J2: on poles and chains A turns right and D turns left; the
  user wants the opposite. User: "slot 3 is great to playtest the chain
  climbing" (UI slot 3).
- New Todo D08J3: free camera while on ladders and climbing, with deep
  investigation and usability testing.

## 2026-10-07 - D08U2 queued

- User: "when an enemy is stood at the top of a ladder, duke cannot climb
  onto the surface. Duke should push into the enemy forcing it back. slot 5
  is a great playtest for this one." New Todo D08U2.

## 2026-10-07 - D08J2 pole and chain A/D direction (Needs playtest)

- Cause: the original sidestep on poles and chains (`0x800439e4`, anims
  192..195) carries Duke to his right for D-pad Left; Vanilla does the same.
- Modernized now sends A as Right and D as Left in that state only. Private
  UI slot-3 runs (third and first-person profiles) go left with A and right
  with D; ladders and Vanilla unchanged; suites pass.
  [Note 122](documentation/122-d08j2-pole-chain-sidestep.md).

## 2026-10-07 - D08J2 accepted; D08J4 queued

- User: "fully and completely accept it all." D08J2 Accepted; executable
  `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the regression baseline.
- New Todo D08J4: the ceiling monkey-bar climb in UI slot 3 drops Duke at
  the wrong points ("he just randomly falls"; still playable). The user
  re-saved slot 3 at the top of the chain.

## 2026-10-07 - D08A4 portable steroids (Needs playtest)

- Modernized (`steroids` `portable`, default): a steroids pickup is held, shown
  in the `[ / ]` switcher after the medkit and in the HUD item box with the
  user's pill-bottle icon; R (or Enter/U on it) takes it with `USED STEROIDS`.
  One at a time; a pickup while they run refreshes them (original).
- Built on the original's own held steroids item (item 4 bit 0 with its
  amount), so savestates, level completion, the card save and load, and
  Continue keep it like the jetpack. New hook `0x8006B73C`; profile schema 29.
- Private runs on copies of the user's cards and savestates (levels 0 and 6);
  suites pass. [Note 127](documentation/127-d08a4-portable-steroids.md).

## 2026-10-07 - D08A4 accepted; D08A8 queued

- User: "mechanically, the steroids work perfectly ... i accept this job as
  complete now as it's functional". D08A4 Done; executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5` is the
  regression baseline.
- New Todo D08A8: show the running steroids countdown in the steroids HUD box
  (pill icon, `research/screencaps/ttk-roids.png`) instead of the original
  armor element and icon.

## 2026-10-07 - D08A8 steroids countdown in the steroids box (Needs playtest)

- Modernized (`steroids` `portable`): running steroids count down in their own
  HUD box with the pill icon (lit), in the item slot when selected, else stacked
  above it like a jetpack that is on; the armour element shows only armour.
  Running steroids stay selected after R so the box counts down in place.
- Status bar element 2 sees steroids as not running only during its draw (bit
  1 off, back on at the first hook after the status bar; new lightweight hook
  `0x8002E850`, one regenerated line, savestates still load).
- Private runs (GL/Software, 4:3/16:9, 60/120 fps), Vanilla unchanged; suites
  pass. Executable `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`.

## 2026-10-07 - D08A8 accepted; D08A9 queued

- User: "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." D08A8 Done; executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8` is the regression baseline.
- New Todo D08A9: a picked-up gadget becomes the `[ / ]` selection (and the HUD
  box), as in Duke 3D.

## 2026-10-08 - D27A Modern Shift/run is silent (Needs playtest)

- **Cause:** Modern Shift holds the original run button (L1). The player
  update `0x800412A4` toggle (`0x8004178c`..`0x800419f0`) calls
  `0x8006bbd8(1)` whenever the run state differs from the one it stored at
  `player+0x27b`, so every press and release clicked.
- **Fix:** `recomp/src/ttk/run_click.inc` and a new entry hook on `0x800412A4`
  (one regenerated line; codegen hash `0x8bab543c` unchanged, savestates
  load). In Modernized it stores the state the toggle is about to set at
  `+0x27b` first, computed exactly as the original does, so the original sees
  no change and stays silent while still setting `+0x224` bit 1 itself. New
  guard over the toggle region. `DNTTK_RUN_CLICK=original` keeps the click.
- **Evidence:** SPU KEYONs of sample `0x012F0`, per leg from a fresh load:
  Shift taps 10 -> 0, hold/release 2 -> 0, walking taps 8 -> 0; run 4093 vs walk
  667 units in 1.2 s; `dnhyper` beat 19 beats at 15/15/18 fields with or
  without Shift taps (the `original` control gives 29: 10 clicks added).
  Vanilla Q (L1) still clicks as before. Suites and Python 131 pass. Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`.
  [Note 130](documentation/130-d27a-silent-shift.md).
- **Not yet verified:** the user's ear in play; Caps Lock autorun not measured
  separately.

## 2026-10-08 - D27A accepted; D12C queued

- User: "i fully accept this. great work. i have wanted this one for a long
  time so this minor change has a huge impact on me as a player." D27A Done;
  executable `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`
  is the regression baseline.
- New Todo D12C: a kick impact sound only when the boot really connects (wall,
  crate, actor), using the wall-collision thud the game already has (not
  Duke's grunt); empty-air and out-of-range kicks stay silent.

## 2026-10-08 - D12C kick impact sound (Needs playtest)

- **Sound:** the wall bump (`0x800537f4`..`0x800538bc`: animation 94/95, pad
  rumble, `0x8006b270(0x2007, Duke+4, 0x800)`) plays `0x2007`, SPU sample
  `0x1BAA0`; measured as the only new KEYON in the bump frame. No vocal in
  that call. Guarded (208 bytes).
- **Hit:** `0x800a979c` returns nothing; on a hit it stores the attacker at
  `sp+0x10` of its frame and calls the struck object's handler. Host calls
  clear and read that slot (`kick_call(..., &struck)`). Walls: original
  segment query `0x8006d980`, kind 1, from Duke's body along the view's
  heading at boot height (eye view) or from his root to the foot (third
  person). One thud per kick, at the first contact.
- **Third person:** new entry hook `0x800A979C` (one regenerated line; codegen
  hash `8bab543c` unchanged, savestates load). For the kick case's call only
  (ra `0x80049098`, Duke, 96, 10), Modernized with identity: the host makes
  the same call, then gives the original radius `0xf0000000` (touches
  nothing), so damage is applied once by the original routine.
- **Evidence:** eye view: empty air, open ground looking down, missed and 800
  away silent; pig cop 1 thud with the kick sound, damage as before; head-on
  wall (bump 94 seen first) 1 thud per kick; garbage bag 1 thud and breaks as
  before, silent after. Third person: club door 1 thud; pig cop 3 kicks, each
  kill, 1 thud at first contact; with `DNTTK_KICK_IMPACT=off` (original call)
  114 and 115 kill and 113 missed: the original's own spread of outcomes.
  Vanilla route `d12c-vanilla` exit 0. `ttk-controls-test` (new D12C case,
  LEVEL01, all levels), input, inventory, aim, near, font suites; Python 131
  OK; overlay guards; `check_repo.py`. Executable
  `3873288acd438d2e367ca2c0fffa20f0953134de61af7715522fec0ffcc87c98`.
  [Note 131](documentation/131-d12c-kick-impact.md).
- **Not yet verified:** the user's ear in play; a multi-kick crate (none in
  the private saves); other levels.

## 2026-10-08 - D12C revision: the thud one octave up (Needs playtest)

- User: "i think i hear something but i cant tell if it's the right noise ...
  it seems very quiet." Measured: 92% of `0x2007`'s energy is below 150 Hz;
  street ambience masks it. User chose preview 2 (the same thud an octave up).
- The impact uses `0x8006b73c(0x2007, Duke+4, 0x800, 48)`: the same call as
  `0x8006b270` with the voice pitch byte as an argument (`0x40` = original);
  measured SPU pitch `0x398` + 23 per step, 48 -> `0x7E8` (was `0x3F6`).
  `DNTTK_KICK_IMPACT_PITCH` overrides it. Guards for `0x8006b73c` and the
  pitch read `0x80068738`. Executable `b62af2b911cfdcc7c300ac025f00f1218b6f8c7439bbd8eda460ce9243283e3d`.
  [Note 131](documentation/131-d12c-kick-impact.md).

## 2026-10-08 - D12C revision: the thud at twice its volume (Needs playtest)

- User: "double it's volume". Layering a second call does not work (the
  routine refuses a second instance). The host doubles the impact sound's own
  table volume `+0x6a` (5192 -> 10384, cap `0x3fff`) through the handle the
  call returns; live voice volume measured `0xC2A` -> `0x1855` (right).
  `DNTTK_KICK_IMPACT_GAIN=1..4` overrides. Guards for the volume reads.
  Controls suite passes. Executable `de112f13e75afef2e31543f48f09ec58c49ba4f6ada9a93b914d2e66cf0d9dbd`.

## 2026-10-08 - D12C revision: x3 volume; first kick at a wall no longer silent (Needs playtest)

- User: multiplier 3; "sometimes ... the first kick doesnt make a noise ...
  when holding kick down the first one is sometimes silent".
- Default gain 3 (live `0xC2A` -> `0x2480`).
- Cause: the sound routine refuses an id already playing (`0x8006b7b0`), and
  walking into a wall plays the same `0x2007` quietly. Reproduced with
  `sfx 0x2007` then a kick: no kick thud. Fix: stop a playing `0x2007` with
  the game's stop `0x80068900` first; reproduction then plays the kick thud
  (`replaced` 1). Guard added. Executable `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f`.

## 2026-10-08 - D12C accepted

- User: "perfect!! fully accepted." D12C Done. Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f` is the regression baseline.

## 2026-10-08 - D08A13 queued

- New Todo D08A13 (user request): steroid duration independent of damage and
  armor in Modernized. Not started. Lead from note 127: the original damage
  cut `0x800a4154` takes 1500 from the running steroid amount `+0x366`; armor
  `+0x234` is a separate field, so the coupling looks original rather than
  stored in armor. To be confirmed when the job is selected.

## 2026-10-08 - D08G4 queued

- New Todo D08G4 (user request): `dnstuff`, `dnitems` and `dninventory` each
  also set armor to 100%, with no other change and no effect on steroid state
  (pairs with D08A13). Not started.

## 2026-10-08 - D08A13 steroids independent of damage (Needs playtest)

- **Traced (acceptance 1): the coupling is original.** Duke's damage handler
  is `0x800a40a8` (class table `0x800c5ed4`; a0 victim player, a1 attacker,
  a2 damage, a3 type, stack args source and 6th). Right after the self-damage
  and attacker checks, before difficulty scaling, armor or health, it tests
  `+0x364` bit 1 at `0x800a4154`: running steroids lose 1500 from `+0x366`
  (or end, bits 0-1 cleared, at 1500 or less) and the handler **returns 0
  with no damage at all**. So the original steroids are a shield that every
  hit spends (6 hits end a full 9000). It does not depend on armor or on the
  damage amount, and nothing ties `+0x366` to armor `+0x234`: the only
  writers of `+0x364`/`+0x366` are the pickup, the drain, the use routine and
  this cut (plus the item-indexed grant, snapshot, restore and level reset).
  It is not ours: Vanilla shows it (below). The handler returns 1 only on
  death (after its death call), which callers read (e.g. `0x80054dc0`).
- **Implementation (Modernized, `steroids` `portable`):** new entry hook
  `0x800A40A8` (`game.local.toml`; one regenerated line, codegen hash
  unchanged, savestates load). For Duke as victim with steroids running, bit 1
  is off for that call only, so the original applies the hit by its normal
  path (armor absorbs 75% as usual, health, pain, death and its own return
  value) and the timer is untouched. Bit 1 comes back at the first TTK hook
  after the call (top of `hook_body`, and the Duke update `0x800412a4`, sound,
  damage-sphere and `0x8002e850` entries). Every in-game reader of bit 1 runs
  from a hooked function: the drain (inside Duke's update), the kick
  (`0x80048410`), the status bar (`0x8008ba30`), the heartbeat; the original
  Select menu (`0x80088134`) only in the pause menu. Host predicates
  (`steroids_running`, `steroids_owned`, doses) count the hidden bit as on.
  Not restored after a savestate load or once the timer reached 0. A new
  code guard covers the whole handler (`0x800a40a8`, 1812 bytes).
  `DNTTK_STEROID_SHIELD=original` keeps the original shield (diagnostics).
  Decision to report: part of `portable`, not a new setting.
- **Evidence** (private Xvfb runs, copy of the cards and savestates in
  `recomp/analysis/d08a13-steroid-damage/`, local; UI savestate slot 3, level
  0, a pig cop shooting Duke; `t1.py`, `t2.py`):

  | Check | Result |
  | --- | --- |
  | `dnhyper`, no armor, 60 fps | 5 hits, health -750 each, timer drops 3525 over 705 frames (5 per frame = the drain); worst per-frame drop 9; no 1500 jumps |
  | Same, 120 fps | 8 hits, 3525 over 704 frames, worst 7.5; HUD: steroids box counting, no armor element |
  | Armor 50 + `dnhyper` | 5 hits: armor -562, health -187 each (the original split); timer 3495 over 700 frames |
  | R dose (spawned pickup) | flags 3; 10 hits, timer 3510 over 700 frames; savestate save and reload: flags 3, timer continues |
  | Unhurt comparison (slot 4) | 8805 -> 5355 over the same 600 frames as 8805 -> 5400 with 7 hits (slot 3) |
  | `DNTTK_STEROID_SHIELD=original` / `steroids` `original` / Vanilla | original shield: hits absorbed (Vanilla: 4 timer jumps of 1515, health unchanged), effect over within 700 frames, then a hit takes 750 |
  | Death while running (`dnhyper`, R dose) | dies; timer frozen while dead (flags kept); Continue (Cross): health 10000, steroids resume with the remaining time and drain |
  | Suites | `ttk-controls-test` 42 groups incl. new D08A13 group (LEVEL01, all levels), `ttk-input-test`, `ttk-inventory-test`, `ttk-aim-test`, `ttk-near-test`, `ttk-font-test`; Python 131 OK (2 skipped); overlay guards; Vanilla route `d08a13-vanilla` exit 0 |

  Executable `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05`.
- **Gameplay change to note:** in Modernized steroids no longer protect Duke
  at all (in the original they cancelled hits). Duke can now die while they
  run; TTK's Continue keeps them running with the time left, as it keeps
  every item.
- **Not yet verified:** the user's playtest; natural (non-spawned) pickups;
  other damage sources (explosions, falls, drowning) were not isolated; any
  that goes through this handler takes the same path, one that does not was
  never shielded; two-player games (player one only).

## 2026-10-08 - D08A13 accepted; D08A14, D08A15 queued

- User: "i accept that this works. mark as done." D08A13 Done. Executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05` is the
  regression baseline.
- New Todo D08A14 (user request): death and Continue end running steroids and
  remove them from the inventory. Not started.
- New Todo D08A15 (user request): the heartbeat is silent for a few seconds
  when steroids restart before the previous run ends. Not started.

## 2026-10-08 - D08A14 death ends steroids (Needs playtest)

- **User decision when selected:** a held, unused dose is lost at death too
  (Duke 3D), not only running steroids.
- **Original behavior (traced live):** dead = player word 0 bit 1, health 0,
  gameplay state `0x800bcbb0` still 1; Duke's update stops draining `+0x366`,
  so the timer stands still; Continue clears bit 1, sets health 10000 and the
  effect resumes. Nothing in the original clears item 4 there.
- **Implementation:** `steroids_death_clear()` (`steroids.inc`) from
  `hook_body`, Modernized `portable` only: while Duke is dead it clears item 4
  bits 0-1 and the amount (the drain's own end state) and the D08A8/D08A13
  hide marks; other bits kept. No new hook or generated code. Debug
  `controls.steroids.death_clears`.
- **Evidence** (`recomp/analysis/d08a14-steroid-death/`, local; slot 3):
  `dnhyper`, an R dose and a held dose each cleared at death; after Continue
  no box, no new heartbeat beats, selection off steroids, R and `]` do
  nothing. Steroids written while dead, savestate saved and reloaded while
  dead: cleared. `original` and Vanilla keep a dose through death and
  Continue. `ttk-controls-test` new D08A14 group (43 groups), input,
  inventory, Python 131 OK (2 skipped), overlay guards. Executable
  `da4a09b04ca07466d001839c0b812d071b94fa79e4829979eeb734e1c27fc687`.
- **Not verified:** the user's playtest; falls, drowning and explosion deaths
  (covered by the dead state, not exercised); card save/load and level
  completion were not re-run (unchanged paths).

## 2026-10-08 - D08A15 heartbeat restart (Needs playtest)

- **Reproduced:** `dnhyper` again mid-run (slot 4) and a pickup refresh mid-run
  (slot 3) each gave 0 beats in the next 300 frames. **Cause:** the beat
  restarted only when the amount rose above the run's first reading, which is
  already 9000 minus one drain step; a refresh to 9000 is drained to that same
  value before the check, so the old beat count held the next beat back for
  as long as the old run had lasted. Not the sound routine's refusal and not
  the D08A8/D08A13 hides.
- **Fix:** `steroids_beat.inc` keeps the previous amount; any rise (only the
  drain lowers it) restarts the rhythm with a beat at once. Debug
  `controls.steroid_beat.restarts`.
- **Evidence** (`recomp/analysis/d08a15-beat-restart/`, local): both restarts
  at 60 and 120 fps beat at once and keep the rhythm (23-25 beats in 300
  frames); natural expiry stops the beat; a new dose right after expiry beats
  at frame 0. `ttk-controls-test` 43 groups, `ttk-input-test`. Executable
  `3454c39cd58eda3ad6e9b805e82603e68e64df18d1a281a0e79ec6a41c0290e4`.
- **Not verified:** the user's listen; D27A silent Shift not re-run (its code
  is untouched). One early 120 fps run ended with steroids off for an
  unexplained reason; 7 reruns did not repeat it.

## 2026-10-08 - D08G4 inventory cheats give full armor (Needs playtest)

- **Checked first:** the original full armor pickup (`0x80082470..0x8008248c`)
  stores 10000 in `+0x234` when it is lower (the smaller armor adds and caps at
  10000); no flag or HUD call, the status bar reads the field (shows 100).
  Live: a spawned armor pickup took 0 and 2500 to 10000.
- **Found:** the existing inventory grant (`0x8003d738`) refills running
  steroids and leaves them running (`dnhyper` at 3835 -> flags 3, 9000), which
  conflicted with acceptance 4. Asked the user, who chose: running steroids
  stop and a full dose is held; armor is set either way.
- **Implementation** (`cheats.inc`, `steroids.inc`): after the grant
  succeeds for `dnstuff`, `dnitems` or `dninventory`, `steroids_cheat_grant()`
  clears bit 1 of running portable steroids (flags 1, full amount) and armor
  below 10000 becomes 10000. Nothing else changes: `dnkeys`, `dnweapons` and
  the other codes, weapons, keys, the D08A9 selection rule.
- **Evidence** (`recomp/analysis/d08g4-cheat-armor/`, local): each of the
  three with armor 2500: armor 10000; with `dnhyper` running (slot 4) and an
  R dose running (slot 3): flags 1, 9000 after, no countdown, no further beats;
  with no steroids: a held dose as before. The user's flow (`dnstuff`, R,
  `dnstuff`, R): held -> running with beats -> held, 9000, beats stop ->
  running again. HUD: armor element 100 with the armor icon, steroids box 100.
  `ttk-controls-test` new D08G4 group (44 groups), input, inventory, Python
  131 OK (2 skipped), overlay guards, `check_repo.py`. Final executable (all
  three jobs) `a4cd7a4f8efcfc44ef86e816adc3b245fcde9a733acd1e9f503fdf51e0235ef0`;
  D08A14 (dose, death, Continue) and D08A15 (`dnhyper` restart) re-run on it.
- **Not verified:** the user's playtest; `steroids` `original` with the
  cheats (the stop is portable only; armor applies in every Modernized profile).

## 2026-10-08 - D08A15 corrected: a pickup mid-run stops steroids (Needs playtest)

- **User correction** (from a Duke 3D playthrough): `dnhyper` mid-run refills
  and keeps running; the inventory cheats (D08G4) and a steroids pickup stop
  the run and leave a full held dose. The first D08A15 pass left pickups to
  the original refresh (refill and keep running, D08A4); the job entry now
  shows that part struck through.
- **Implementation:** `steroids_pickup` (`steroids.inc`) no longer leaves a
  pickup during a run to the original; it takes it like a pickup with nothing
  held: original message, sound and removal, then at the tail's sound call
  bit 1 off, bit 0 on, full amount (and the D08A8/D08A13 hide marks dropped so
  the run stays stopped). `original` and Vanilla keep the original refresh.
  The D08A15 heartbeat fix still covers `dnhyper` mid-run.
- **Evidence** (`recomp/analysis/d08a15-beat-restart/t4.py`, slot 3): pickup
  during a `dnhyper` run (3835) and during an R dose (3850): flags 1, 9000,
  held dose 1, beats stop, STEROIDS message, box at a held 100; R then runs
  it; `dnhyper` mid-run refills and keeps running with beats. Native D08A4
  pickup group updated (running dose and `dnhyper` run both stop and are
  held); `ttk-controls-test` 44 groups, input, inventory, Python 131 OK (2
  skipped), overlay guards. Executable
  `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472`.
- **Not verified:** the user's playtest; natural (non-spawned) pickups.

## 2026-10-08 - D08A14, D08A15, D08G4 accepted; D08A16, D08A17 queued

- User: "fully accepted and ready to close it out." D08A14 (death ends
  steroids), D08A15 (heartbeat restart; corrected: a pickup mid-run stops the
  run and leaves a full held dose) and D08G4 (inventory cheats give full
  armor; running steroids stop, full dose held) are Done. Executable
  `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.
- New Todo D08A16 (user request): power-up coin icons for invincibility,
  invisibility and Double Duke, a human design ticket; when it begins, a
  placeholder section goes into the local `ttk-font-picker.html`.
- New Todo D08A17 (user request): real HUD countdowns for the three power-ups
  like the steroids box, after the D08A16 icons. Not started.

## 2026-10-08 - D08A16 picker section for the power-up icons (waiting for art)

- **Picker:** `recomp/analysis/d24a-fonts/ttk-font-picker.html` (local) has
  a new section 11, "Power-up coins". It has one card per coin with its name,
  type, Quake analog, in-game look and file names (`hud-invincibility.png`,
  `hud-invisibility.png`, `hud-double-duke.png`, optional `gadget-<slug>.png`
  strip variants). Each card has drop/click slots and previews: as drawn at
  8x, a 15-colour preview (a JavaScript port of `reduce_icon_15col.py`; the
  tool stays authoritative), 2x HUD size and 1x. Dropped art is stored in
  browser localStorage only. A "Running together" mockup stacks steroids
  plus the three power-up boxes over the ammo box with the disc's 46x16 box
  and the red HUD digits, with a toggle per box; the numbers are made up.
  `make_picker.py` reads delivered art from `recomp/assets/ui/items/`
  automatically.
- **References:** each coin was spawned (D26E `spawn`) in level 0 in a
  private offscreen first-person run and burst-captured;
  `analysis/d08a16-coins/pick_refs.py` keeps the three most face-on frames
  per coin.
- **Found:** all three are the same octagonal-rimmed radiation-trefoil coin;
  only the coin colour and glow differ. Invincibility (1047) is gold with an
  orange glow, invisibility (1045) silver-violet with a blue glow and Double
  Duke (1046) silver with a pink-red glow. A spawned coin spins while it
  falls, then lies flat.
- **Checked:** headless Chrome render of the section, empty and with a
  stand-in icon (the steroids pill), plus the mockup.
- **Remaining:** the user draws the three icons. Then reduce them to 15
  colours with the tool, the user accepts them in the picker, and the job is
  Done. No gameplay change.

## 2026-10-08 - D08A16 icons delivered (awaiting acceptance)

- The user drew `hud-invincibility.png`, `hud-invisibility.png` and
  `hud-double-duke.png` (16x16; delivered in
  `research/inv/inventory-items-custom-musicmonster/`). They are copied to
  `recomp/assets/ui/items/`, with `-15col` copies made by
  `reduce_icon_15col.py`. The picker now shows the as-drawn art next to the
  tool's reduction, and they are in the stacked HUD mockup. Not yet in
  `items.json` or the build; that is D08A17. Waiting for the user's
  acceptance in the picker.
- User, 2026-10-08: no switcher-strip variant and no portable power-ups. If
  power-ups were ever held, the strip would use these same icons. The strip
  slot is removed from the picker.

## 2026-10-08 - D08A16 accepted

- User: "fully accepted." D08A16 Done. Icons:
  `recomp/assets/ui/items/hud-invincibility.png`, `hud-invisibility.png`,
  `hud-double-duke.png` and their `-15col` reductions. No gameplay change.
  D08A17 (the countdown boxes) is ready to start and uses them.

## 2026-10-08 - D08A17 power-up countdowns (Needs playtest)

- Invincibility, invisibility and Double Duke each show a lit box with the
  user's D08A16 coin icon and a percent countdown while they run, stacked
  above the selected gadget, an active jetpack and running steroids
  (Modernized). The Continue protection shows as invincibility from 25.
  No expiry warning (user). Executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`;
  [note 132](documentation/132-d08a17-powerup-countdowns.md).
- Not verified: the user's playtest; natural (non-spawned) coins.

## 2026-10-08 - D08A17 accepted

- User: "confirmed it's all working as intended!" D08A17 Done. Executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline.

## 2026-10-08 - D24C TTK-font `!` and `>` console prompt (Needs playtest)

- `build_ttk_fonts.py`: `exclamation()` builds `!` for the Big and Medium
  Italic fonts from their own `I` and `.` (design picked from rendered
  variants; the full-height period read as `:`, so the dot is trimmed and the
  stem tapered). `!` left the system-font fallback list of sets 0-5.
- Framework `host_osd.c` (prompt, empty-line fallback) and `main.cpp`
  (echoed commands): `] ` became `> `. Exported to the patch, which applies to
  the pinned clean framework; no header change.
- Evidence: new `test_exclamation_from_own_font`; `ttk-font-test` passes
  ("Giving Everything!" in all styles); 133 local Python tests OK (2 skipped);
  repo check OK. A private Xvfb run (`analysis/d24c-glyphs/t1.py`, private
  card copy) shows GIVING EVERYTHING! with the new `!` and the console with
  `> FPS` and `> DNST`. Executable
  `b38f4da0153962f93cc7ca01ecf2636c29c876569511fc6b1b019b06b5138b12`.
- Font picker (local, `recomp/analysis/d24a-fonts/`): `make_picker.py` adds
  the same `!` through `exclamation()`, so the glyph sheets match the game. New
  section 13 shows before/after for the message and console fonts and both
  prompts. It renders in headless Chrome with no script errors.
- Not verified: the user's look at their own window size.

## 2026-10-08 - D24C accepted

- User: "all accepted". D24C Done (the `!` glyph, the `>` prompt and the
  font picker section).

## 2026-10-08 - D08Z1 jump wall slide (Needs playtest)

- Research: the airborne handler `0x80055904` bounces Duke off any wall within
  45 degrees of head-on (`0x8003ef08`: half-speed reflection, rise zeroed, 107,
  rumble, sound `0x2008`) and deflects him away at 0.25..0.75 speed otherwise
  (`0x8003ef78`); a steep ceiling bounces too.
- Change: `jump_walls.inc`, a second plugin at the existing `0x8003EBF4` entry.
  The isolated original integration and sweep `0x8007a98c`; wall -> remove the
  into-wall velocity (+128 outward drift, harder push-off on a re-touch);
  rising into a ceiling -> stop the rise; corner -> up to three passes, then
  stop horizontal. The ledge helpers accept a recent slide in place of the
  bounce's first updates. Profile schema 30 `jump_walls` slide/original,
  `--jump-walls`, menu K, `DNTTK_JUMP_WALLS`. Guards for `0x8007a98c` and
  `0x80079f4c`. No generated code change; savestates load.
- Evidence: see note 134 (three levels, both jump styles, all regression
  routes, native 45 groups, Python 136 OK). Executable `c91a3ef128d9f98d923efe858fef75f432b8d729b2ffe5f16654f0034a56b627`.
- Open: the user's playtest (feel, any wall that still bounces, whether a
  head-on thud is wanted). Nothing committed.

## 2026-10-08 - D08Z1 revision 1: monument corner (Needs playtest)

- User: the slot-6 monument corner still bounced; otherwise the mechanic
  "feels much more modern and great". Reproduced (manual, first person): 2/8
  corner jumps played 107, the rest stopped dead.
- Change: corner turn search (15-degree steps toward the slide, first heading
  the original sweep clears), push-off fallback, and a safety net that turns a
  remaining original bounce/deflection into a slide and undoes its 107 in the
  same update. Guard `0x8003ef08`.
- Evidence: slot 6 corner 0/21 bounces across styles; slots 5 and 12 no 107;
  routes unchanged; native 45 groups, Python 136 OK. Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. Note 134.

## 2026-10-08 - D08Z1 accepted; D08Z2, D08Z3 queued

- User: "mechanically this feels significantly better, where we can call the actual job as accepted." D08Z1 Done; executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34` is the regression baseline.
- New jobs from the same playtest: D08Z2 mantle animation now a static leg
  pose (regression, next job), D08Z3 occasional landing where Duke is stuck for
  about a second.

## 2026-10-08 - D08Z2 mantle upper body (Needs playtest)

- Cause: the D08Y low-lip step-up (no E) could start with a weapon drawn, and
  since D08Z1 it starts from every wall slide. The original mantles only with
  a free upper body (`0x80051cf0` refuses on upper-table bit 8) and sets only
  the lower animation, so the legs played 134 while the upper track held the
  static weapon-ready pose 20. Holstered E mantles, crate mantles and the
  slot-12 grab and pull-up were full body and matched `original` frame for frame.
- Change: `jump_mantle_start` (one start for the crate, lowered-catch and
  step-up mantles) hands a weapon pose on the upper track to the mantle;
  `mantle_upper_restore` puts the kept upper block back when the mantle ends
  (without it the weapon stayed lowered and would not fire).
- Evidence: armed slot-5 step-ups play full body, weapon pose back one update
  after, Mouse1 fires; D08Z1 regression routes same outcomes; native 45
  groups (D08Y group extended), Python 136 OK. Executable `9de8da617862f53e545ac3c8be0f23623979fea2265944e2f3cdfaa3ec3f96b0`.
  [Note 135](documentation/135-d08z2-mantle-upper-body.md).
- Open: the user's playtest. Nothing committed.

## 2026-10-08 - D08Z2 revision 1: legs frozen after a corner bump (Needs playtest)

- User: after a bump at the statue (UI slot 8) Duke's lower half stays locked
  in one pose whatever he does (UI slot 6).
- Cause (D08Z1 safety net): its 107 undo ran at the next animation runner
  call, after the update had started 107 on the tracks; track 3's OR-only
  joint mask kept 107's leg joints (`0x1c00` -> `0x1cef`), freezing the legs,
  also in savestates. Reproduced at the monument corner; the original bounce
  never does it.
- Change: undo at the bounce's contact sound `0x8006bbd8` (ra `0x80055a10`),
  right after the 107 write; new opt-in hook, regenerated, codegen hash
  unchanged, savestates load.
- Evidence: three corner catches, mask stays `0x1c00`, legs stride after;
  native 45 groups, Python 136 OK. Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986`.
  [Note 135](documentation/135-d08z2-mantle-upper-body.md#revision-1-legs-frozen-after-a-corner-bump).
- The player's new UI slot 6 keeps the frozen legs (saved polluted state).
  Nothing committed.

## 2026-10-08 - D08Z2 accepted

- User: "accepted!!". D08Z2 Done (armed scramble plays full body; the D08Z1
  safety net no longer freezes the legs). Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression
  baseline. Next: D08Z3.
