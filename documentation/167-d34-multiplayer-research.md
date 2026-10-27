# D34 - Multiplayer investigation and feasibility (research)

Job: [D34](../MODERNIZATION_JOBS.md). Status: research delivered 2026-10-20,
awaiting the user's review. Shareable version: [TTK Multiplayer Research (D34)](https://claude.ai/code/artifact/07cd1540-1c0c-43ef-8d1b-d4b5428ba81f) (private Claude Docs page; share it from its Share menu). **Research only:** no gameplay, engine,
framework or build change was made for this note.

Evidence labels, as in [the documentation index](README.md):

- **Verified (static)** - read from our source, the framework source, or
  the owned SLUS-00583 executable (disassembled from `recomp/disc/SLUS_005.83`
  with a throwaway script; generated C was only read, never edited).
- **User report** - the 2026-10-20 playtest below, not reproduced here.
- **Hypothesis** - a plausible explanation that still needs a run to prove.
- **Recommendation** - a proposal for the user to approve or change.

No game run was made for this note. Every runtime claim below is either a
user report or a hypothesis, and Phase A (section 10) checks them.

## 0. Summary

1. **The original two-player mode is one shared simulation with two players
   and two cameras, drawn one viewport per game frame (verified).** It is a
   split-screen deathmatch with LEVEL, TIME, KILLS and SPLIT options, six
   arenas (levels 15-20) and character choice. The game's data layout is
   built for exactly two players.
2. **Our build was never set up for a second player (verified).** The
   framework is compiled with `MAX_PLAYERS 1`, so console port 2 has no pad.
   Every Modernized system (controls, aim, first person, high refresh, HUD,
   cheats, console) assumes player 1 alone and either fails closed or turns
   itself off when two players are present. The two-player arenas are also
   outside the control identity guard. Together these explain the reported
   legacy controls and 30-60 FPS. Our leading hypothesis for the frozen
   spinner and flicker is the same family of causes (section 2), still to be
   proved with a run.
3. **The framework already has console-level netplay (verified, not used by
   us).** psxrecomp ships `recomp-net` and `retcomm-rbengine` (both MIT):
   delay-sync and GGPO-style rollback over UDP, LAN and direct IP, optional
   ICE/TURN, a WebSocket lobby, state digests and desync checks. It syncs
   *pad input*, and each peer runs the whole game. It is disabled in our
   build and has no encryption.
4. **Recommended architecture (recommendation):** deterministic
   input-synchronised simulation (lockstep first, rollback later) of the
   game's own two-player mode, with *presentation decoupled per peer*. Each
   machine runs the identical two-player simulation, and our D17 replay
   renderer draws only the local player's view, full screen and at high
   refresh. Modern controls become a synchronised per-tick "intent" record
   instead of direct host writes. A server that does not run the game (input
   relay, no disc needed) gives self-hosting with no central service.
5. **More than two players is not feasible on this route without major
   engine work (verified layout, hypothesis cost).** The player and camera
   tables, the frame function and the arenas are sized for two. A
   server-authoritative design that replicates world state would need most of
   the game's actor system reverse-engineered first, and is a separate,
   much larger programme (section 9).
6. **Smallest proof of concept (section 12):** two game instances on one
   machine (then LAN) in the original two-player mode, joined by pad-level
   delay-sync, reaching the arena and staying in sync (digests match) for ten
   minutes of play. Vanilla-equivalent simulation, no modern controls yet. It
   answers the one question that decides everything else: is our build
   deterministic enough for input sync?

## 1. Current multiplayer architecture

### 1.1 Frontend and match setup (verified)

From [note 150](150-d19j-title-menu.md) and `recomp/src/ttk/game_menu.cpp`
(`game_menu.cpp:166-170`):

| Item | Address | Notes |
| --- | --- | --- |
| Players chosen on the main menu | `0x800bdea4` | 1 = ONE PLAYER, 2 = TWO PLAYER |
| Players in the running game | `0x800c27bc` (word) | read throughout the game and our hooks |
| TWO PLAYER OPTIONS handler | `0x8008484c` | rows: LEVEL `0x800c3dfc`, TIME `0x800c3dfe`, KILLS `0x800c3e00`, SPLIT `0x800dd1b2` |
| Split mode in play | `0x800be564` | 1 = vertical, 2 = horizontal (menu value inverted) |
| Arena list, formats, characters | `0x80016e8c`, `0x80016e98`, `0x80016ea4` | the level choice is stored as level + 15 (`0x800bdeaa`) |
| CHOOSE CHARACTER handler | `0x80085060` | choices `0x800dd1b8` / `0x800dd1bc`, locks `0x800dd1c0` |

The original CHOOSE CHARACTER reads **each player's own pad**. In the PC
build only one pad exists, so the original could not get past this page. D19J
made one keyboard choose for both players (`game_menu.cpp:1468`). Note 150
records a two-player start reaching mode 1, level 15 (MESA), characters 2 / 1,
split mode 2. No one had played past the start before the user's 2026-10-20
test.

Arenas: levels 15-20 (MESA, CASTLE and four others; exact names come from the
game's string table, [note 101](101-d26a-level-select.md)). The title-screen
level-select cheat skips them, so they are reachable only through TWO PLAYER.

### 1.2 One simulation, two players (verified)

- **Player table:** player 1 at `0x800d7198`, stride `0x8a4`
  ([note 18](18-player-camera-research.md)). The frame function `0x8002666c`
  reads `+0x224` of both `0x800d7198` and `0x800d7198 + 0x8a4` (instructions
  at `0x80026818` and `0x8002682c`). The composition `0x80026164` loops over
  `0x800c27bc` players and calls `0x80040584` for each one
  (`0x800264ec`-`0x8002651c`).
- **Camera table:** normal cameras at `0x800d6eb0`, stride `0xb0`. Scripted
  or alternate cameras start at `0x800d7010` = `0x800d6eb0 + 2 * 0xb0`, and
  the composition picks between them per player by player flag `0x20000000`
  (`0x8002620c`-`0x8002626c`). The alternate table starting right after two
  normal slots is strong evidence that the camera layout is sized for
  **exactly two players**.
- **Input:** `0x80056770` reads per-player inputs through tables
  `0x800d1600`, `0x800d1a98` and `0x800d14e8` (note 18). The frontend pad
  history has pad 1 from `+0x10` and pad 2 from `+0x54`
  (`game_menu.cpp:171`).
- **Match rules exist in the game:** time and kill limits (TWO PLAYER
  OPTIONS), a multiplayer respawn wait of `0x63` on the spawn path
  (`recomp/src/ttk/spawn.inc:93`,
  [note 117](117-d26f-crystals.md)), per-player message slots
  (`0x800dcd90 + 2 * player`), a results screen at the end of a match
  (user report), and two-player item rules (items 0-5 are cleared at level
  start in two-player games, [note 127](127-d08a4-portable-steroids.md);
  jetpack, Bio Mask and goggles start differently, `pickup_select.inc:10`).

### 1.3 Rendering: one viewport per game frame (verified)

The composition `0x80026164` draws **one** player's view per call:

1. It reads the view index `0x800d2654` and computes the player
   (`0x800d7198 + index * 0x8a4`) and camera (`0x800d6eb0 + index * 0xb0`,
   or the alternate table).
2. `0x8001d718(index)` sets up that player's viewport. The split mode
   `0x800be564` picks the HUD offsets.
3. At the end (`0x800265e8`-`0x8002660c`) it increments `0x800d2654` and
   wraps it to 0 at `0x800c27bc`, unless the pause flag `0x800c3c4c` is set.

The frame function calls the composition once per frame (`jal 0x80026164` at
`0x800268ec`; our D17 capture point is its return `0x800268f4`). In a
two-player game each half of the screen is therefore redrawn on **every other
game frame**. At the game's 30 (or 20) simulation frames per second, each
player sees about 15 (or 10) new images per second. This is the original
design. The PS1 hid it by leaving the other half of the frame buffer intact.

### 1.4 How the modern layer treats two players (verified)

| System | Where | Behaviour with `0x800c27bc` = 2 |
| --- | --- | --- |
| Pad ports | `recomp/CMakeLists.txt:38` `MAX_PLAYERS 1`; `game.toml` `players = 1` | the framework's `pad_buttons[PSX_MAX_PLAYERS]` has one entry (`psxrecomp/runtime/src/sio.c:48`): **port 2 has no controller** |
| Modernized controls, aim, first person | `modern_controls.cpp:29` fixes `player=0x800d7198`, `camera=0x800d6eb0` | player 1 only, by construction |
| Control identity guard | `control_guards.inc:227` `level_overlays[]` | 21 one-player levels only; the arenas fail closed with `[TTK identity]` ([note 102](102-d22b-every-level.md)) -> **legacy controls** |
| D17 high-refresh replay | `frame_replay.cpp:867` | gameplay images are captured only when `0x800c27bc == 1` -> **no high-refresh gameplay** |
| Widescreen HUD | `widescreen.inc:76` | returns when players >= 2 (split screen keeps 4:3, [note 76](76-d14-widescreen-first-pass.md)) |
| Cheats, spawn, level select, apartment helpers | `cheats.inc:91,127`, `spawn.inc:125,154`, `level_select.cpp:84`, `apartment_interaction.inc:16,41` | refused unless one player |
| Pause list LOAD GAME | `game_menu.cpp:599` | shown only for one player |
| D23F fast timing and CPU overclock lease | `modern_controls.cpp:908-910` | renewed from player 1's update only. The overclock pauses from **host wall-clock** speed (`overclock_lease`, `modern_controls.cpp:886`), so it is not deterministic across machines |

The modern layer is deliberately single-player and fail-closed. That was the
right choice for the campaign. It is also why two-player mode falls back to
the original experience.

## 2. Reproduction findings and suspected failure points

The user's 2026-10-20 sequence (TWO PLAYER, MESA, Red Duke and Blue Duke,
START) and what the code suggests for each step:

| Symptom (user report) | Explanation | Label |
| --- | --- | --- |
| Loading spinner stops progressing; music becomes audible | D17U shows loading screens as replay images. When the level starts, gameplay images are refused for two players (`frame_replay.cpp:867`), so the presenter probably keeps showing the last loading redraw while the arena already runs behind it. The music is the arena's. | Hypothesis (strong) |
| Enter opens an options/pause screen | Enter is the pause key in gameplay (D19D/D19H). The match was already running, so Enter paused it. The pause path draws through the menu presentation (D19O `menu_alone`), which is when the real split-screen picture first appears. | Hypothesis |
| Split screen flickers on the left and right | Two candidates: (a) each half is redrawn every other frame (section 1.3), and our presentation does not keep the stale half as a PS1 frame buffer would (replay or real-image switching, renderer buffer handling); (b) at 16:9 the area outside the 4:3 picture is not cleared for split screen, because widescreen skips players >= 2. | Hypothesis |
| Legacy controls after RESUME | Identity guard fails closed on the arenas, and modern controls bind to player 1 only. | Verified cause (static) |
| About 30-60 FPS instead of 120 | No gameplay replay for two players, so presentation is the original's, and each viewport updates at half the game rate. | Verified cause (static) |
| Exit: pause, MAIN MENU, YES -> results screen -> Escape -> TWO PLAYER OPTIONS -> Escape -> main menu | The game's own end-of-match path (statistics/results, mode 2), then the frontend returns to the page the match started from. This is probably the original flow, not a bug. It still feels disconnected because it is the only path in the game without our menus, keys and presentation. | Hypothesis (to compare with Vanilla) |
| Player 2 cannot act | Port 2 has no pad (`MAX_PLAYERS 1`). | Verified cause (static) |

**Shared cause:** almost every symptom comes from one fact. The modern layer
(input, presentation, timing, HUD, menus) was built and guarded for one
player, and two-player mode is the one place where those guards switch the
game back to its original paths in the middle of a modern session. The fix
is not a set of patches to the frozen spinner or the flicker. It is a
purpose-built multi-player model for the modern layer (sections 4 and 10).

## 3. Reusable systems and architectural limits

**Reusable (verified to exist):**

- The game's own deathmatch: arenas, spawn points, respawn, time and kill
  limits, character skins, scoring and results screen, split HUD layouts.
- Per-player state in a fixed-stride table, so modern hooks can be made
  per-player by index rather than rewritten.
- D17 frame replay: renders the game's composition on a **frozen copy** in
  worker processes and never changes the live guest (`frame_replay.cpp:1-22`).
  Rendering a view that the live game did not draw is therefore side-effect
  free. This is the key to "each player gets a full-screen view" without
  changing the simulation.
- D19 menus: the TWO PLAYER OPTIONS and CHOOSE CHARACTER pages already run on
  the original variables, so a lobby can feed them.
- Framework netplay (`psxrecomp/docs/NETPLAY.md`, `psx_netplay.c`,
  `psx_netplay_rb.c`, `lib/recomp-net`, `lib/retcomm-rbengine`): sessions,
  input delay, rollback, snapshots, state digests (`netplay_state_digest.c`),
  dump-strict disc gates (`game.toml [netplay]`), LAN beacons, direct IP and
  ICE.
- Framework controller routing: per-port SDL gamepads or keyboard
  (`psxrecomp/runtime/src/main.cpp:365-405`), up to 8 seats with multitap.

**Limits (verified unless marked):**

- Two player and two camera slots (section 1.2). More players means growing
  guest tables inside a fixed PS1 memory map: very hard (hypothesis: the
  tables are followed by other globals, so they cannot simply be extended).
- One viewport per frame in the live game.
- Arena data has two-player spawns and item sets (hypothesis: not yet
  inspected).
- The game simulates at 30 Hz, or 20 Hz when a frame overruns, with the step
  size taken from elapsed video fields (frame_replay header). The simulation
  step therefore depends on emulated timing. Lockstep needs every peer's CPU
  timing model to give identical cycle counts. The accurate model and D23F
  fast timing are both deterministic functions of the code, but the overclock
  pause and the fast-timing lease decisions use host wall-clock time
  (section 1.4), which is not.
- Modern controls write guest memory from host state sampled at host times
  (`pc_input.cpp:150`, mouse look, the D07E lease, late-sampled yaw). In a
  synchronised session every such write must come from synced data.
- The framework's netplay disables mods for sessions (`NETPLAY.md`,
  "Mods: disabled for all netplay sessions"). Our modern layer is statically
  linked game code rather than a mod package, but how that policy applies to
  plugins registered through `mod_plugins.h` has not been checked.
- `PSX_MAX_PLAYERS` sizes framework arrays that may be saved in savestates
  (`sio.c` `PSX_BSS` arrays). Raising it to 2 may change the savestate layout
  and invalidate existing savestates (hypothesis).

## 4. Local split-screen restoration options

The goal is two players on one PC, at least one on a controller, with
independent movement, camera and aim, our renderer and display settings, no
flicker, smooth pacing, and working menus, character selection, pause and
exit.

**Step 1 - two real input seats (recommendation).** Build with
`MAX_PLAYERS 2`. Seat assignment in our profile: seat 1 = keyboard and mouse
or controller 1, seat 2 = the next controller. "Keyboard and mouse plus one
controller" works because seats are host routing. Two controllers with no
keyboard must work too. Two keyboard players on one keyboard is not
recommended: there is one mouse, and modern controls need one per player. A
press-to-join prompt on CHOOSE CHARACTER replaces D19J's one-keyboard
workaround. Vanilla gets real two-pad input this way, unchanged in rules.

**Step 2 - per-player modern layer.** Turn the fixed `player` / `camera`
constants into a seat index (`0x800d7198 + seat * 0x8a4`, camera
`0x800d6eb0 + seat * 0xb0`) for modern controls, aim, first person, view bob,
look keys and HUD. Add the six arenas to `control_guards.inc` through
`level_overlay_guards.py`, as D22B did for the campaign. Campaign-only systems
stay one-player (cheats, spawn, level select, mission inventory, steroids
HUD). The controller path depends on D09 (controller support): modern
controls on a gamepad (twin-stick move and look, triggers to fire) do not
exist yet.

**Step 3 - split-screen presentation built for it (recommendation).** The
live game redraws one half per frame. Two ways to present it:

| Option | How | Pros | Cons |
| --- | --- | --- | --- |
| A. Persistent halves | The presenter keeps each half from its latest image (a PS1-like frame buffer) | Small change; no replay needed | Each player still at 15 images/s; no 120 FPS |
| **B. Replay per viewport (recommended)** | D17 replay renders *both* views from each captured frame (forcing the view index and viewport in the frozen copy) and interpolates each, then composes them into the split layout at the display rate | Both halves at the display rate, flicker gone by construction, our widescreen and resolution apply per half | Twice the replay work per frame; needs replay to accept players = 2, and an extra layout pass |

Option B reuses the mechanism that already gives one-player 120 FPS. The cost
is a second replay job per captured frame. Today one-player replays run at
roughly 12 ms per redraw in busy scenes (frame_replay comments). Two views at
120 Hz would need more workers, or 60 Hz per half as a fallback.

**Step 4 - menus and flow.** Start the match from our pages straight into
gameplay presentation (no stale loading image), pause per match (either
player can pause; the list says which), our results screen style and a
"REMATCH / CHANGE ARENA / MAIN MENU" exit that returns to our main menu
instead of stepping back through the setup pages.

## 5. Online multiplayer feasibility

### 5.1 The two families of design

| | Input-synchronised (lockstep / rollback) | Server-authoritative state replication |
| --- | --- | --- |
| What travels | Each player's input per tick (a few bytes) | World state from the server (positions, actors, projectiles, events) |
| Who simulates | Every peer runs the identical whole game | The server; clients predict and interpolate |
| Needs | Bit-exact determinism | A complete model of the game's entities, to serialise them |
| Used by | Doom, Duke Nukem 3D (original), StarCraft, fighting games (GGPO), emulator netplay (RetroArch, Dolphin), psxrecomp `recomp-net` | Quake/QuakeWorld, Source, Unreal, Zandronum, Odamex |
| Fit for TTK | **High**: the game is a black-box PS1 simulation we can run identically. The game already has two-player rules. | **Low now**: TTK's actors, collision, scripts and weapons are mostly unmapped. Replicating them means reverse-engineering most of the game. |

**Verdict (recommendation):** input-synchronised simulation is the only
realistic route to online play with the game's own rules. Server-authoritative
replication is a possible long-term path for more than two players (section
9), not a starting point.

### 5.2 Input-synchronised design for TTK

- **Simulation:** every peer runs the same two-player match. A simulation tick
  is one game frame (the player update, about 33 ms). Lockstep waits for both
  players' input for tick N + D. Rollback predicts the remote input,
  simulates, and re-simulates from a snapshot when the real input differs.
- **What is input:** for Vanilla rules, the pad bytes, as the framework does
  now. For Modernized play, a fixed-size **intent record** per player per
  tick: move vector, view yaw and pitch, held actions (fire, jump, crouch,
  use, grab), weapon or item selection. Every modern hook then reads intent
  from the synced record, never directly from SDL or the clock. This is the
  main engineering cost and it should also benefit single-player (one input
  path, testable by replay).
- **Own view, full screen, per peer:** the live simulation stays the original
  two-player game, including its split-screen drawing, so the guest state is
  identical on both peers. Each peer's D17 replay renders **only its own
  player's view** full screen at the display rate (section 4, option B with
  one view). Presentation never feeds back into the simulation, so peers can
  use different resolutions, frame rates and widescreen settings, as the
  framework's "dual raster" note intends.
- **Latency:** at 30 Hz a delay of D = 2 ticks is about 66 ms of input lag
  before network time. Mouse look is the critical part. The camera can be
  turned **locally at once in presentation** (D17 already late-samples the
  yaw for replays), while the authoritative aim follows a tick later. Shots
  use the synced aim, so a fast flick may hit what was under the crosshair
  one tick ago. Rollback (P = 2-4 ticks) removes most of the waiting.
- **Rollback cost:** a snapshot is the whole machine (2 MB RAM, 1 MB VRAM, SPU
  state and device state). A re-simulation of P ticks runs the game P extra
  times inside one frame. One-player Modernized play already needs D23F fast
  timing to keep 150% CPU at 120 Hz, and a two-player arena draws a view each
  frame. Rollback is therefore a **measured** later step: lockstep first.
- **Lag compensation:** not needed in the input-sync model. Every peer
  simulates the same shots against the same state; the trade-off is input
  delay, or rollback corrections, instead of "favour the shooter".
- **Combat, pickups, inventory, deaths, respawns, scoring and match rules:**
  all come from the game's simulation and are identical on every peer by
  construction, once determinism holds. Nothing to replicate.
- **Animations, audio, effects, world events:** simulated identically. Audio
  is produced per peer. Rollback re-simulation must not play sounds twice
  (the framework handles SPU state in snapshots; audible artefacts on
  correction need testing).
- **Join, disconnect, reconnect:** lockstep requires both players from the
  start. Late join or reconnect means a full state transfer (the framework
  has one: `NP_XFER_*` in `psx_netplay.c`). A dropped player can be frozen
  in place or handed to an idle input until they return. The match never
  needs AI.
- **Desync detection:** the framework hashes state per tick
  (`netplay_state_digest.c`). On a mismatch, resynchronise from the host's
  state and log the first diverging tick for diagnosis.

### 5.3 What the timing architecture must change

1. All host-wall-clock decisions that change guest execution (the overclock
   pause, the fast-timing lease lapse, any `steady_clock` gating of hooks)
   must be fixed per session or derived from simulation ticks.
2. One CPU timing model per session (both peers fast, or both accurate),
   recorded in the session caps like the framework's BIOS settle.
3. Modern hooks consume only the synced intent record and guest state.
4. Savestates, the console, cheats, spawn, level select and the debug port
   are disabled or session-wide in a networked match.
5. Determinism tests: two instances fed the same recorded intent must give
   identical digests every tick. This also gives a valuable regression tool
   for single-player.

## 6. Networking technology comparison

Licences and features are from each project's public documentation (not
re-verified in this session, except the two vendored libraries).

| Option | Purpose and fit | Strengths | Weaknesses | Security | Platforms | Licence | Integration |
| --- | --- | --- | --- | --- | --- | --- | --- |
| **recomp-net + retcomm-rbengine** (vendored in `psxrecomp/lib`) | Delay-sync and rollback of opaque per-slot inputs; LAN, direct IP, ICE, lobby | Already wired to this runtime (`psx_netplay*.c`), proven on MotK, digests and state transfer exist | Pad-level design, our intent record needs a wider input blob; default online lobby is a third-party host (`netplay.retcomm.net`); v0.1 | **No encryption or authentication**; LAN lobby password is plaintext | Linux, Windows | MIT (libjuice for ICE: MPL-2.0) | Lowest: build flags plus TTK seats and intent |
| **ENet** | Reliable and unreliable UDP channels, connection management | Small, stable, used by many games (including Duke3D source ports such as EDuke32) | No encryption, no NAT traversal, no rollback | Add your own (libsodium) | All | MIT | Low; we would write session and sync logic |
| **GameNetworkingSockets** (Valve) | Message transport with reliability, fragmentation, encryption | Encrypted and authenticated connections by default, good congestion handling, optional P2P with ICE | Larger dependency (OpenSSL or libsodium, protobuf), C++ | Strong built-in | All | BSD-3-Clause | Medium |
| **yojimbo / netcode** (Glenn Fiedler) | Secure client/server connections with connect tokens | Designed against spoofing and amplification, small | Client/server only, connect tokens assume a web backend (can be self-issued) | Strong (libsodium) | All | BSD-3-Clause | Medium |
| **GGPO** | Rollback for deterministic games | The reference rollback design | Windows-centred, unmaintained upstream, duplicates what retcomm-rbengine does here | None built-in | Windows (ports exist) | MIT | Medium, and redundant |
| **libdatachannel** (WebRTC data channels) | P2P with ICE, DTLS encryption | NAT traversal plus encryption in one | Heavier; still needs signalling | DTLS | All | MPL-2.0 | Medium |
| **QUIC** (msquic, quiche) | Encrypted transport | Modern and robust | Overkill for small input packets, large | TLS 1.3 | All | MIT / BSD-2 | High |
| **libsodium** (with any of the above) | Crypto primitives | Small, audited, easy | Not a transport | Key exchange and authenticated encryption | All | ISC | Low |

**Recommendation:** start on **recomp-net / retcomm-rbengine**, already in
the tree and designed for this runtime. Keep the transport behind our own
small session interface. Add an authenticated-encryption layer (libsodium,
Noise-style handshake keyed by a session password or host key) before any
internet release. Revisit GameNetworkingSockets only if we outgrow
recomp-net's transport. The psxrecomp framework itself is PolyForm
Noncommercial, which already limits distribution to non-commercial use. All
the libraries above are compatible with that.

Lessons from open-source engines, for this design:

- **Doom / Duke Nukem 3D and their source ports** (Chocolate Doom, EDuke32):
  deterministic input lockstep works well for 2-8 players on LAN and degrades
  with latency. Desync detection and demo-style input recording are essential
  debugging tools. Our intent record is that "demo".
- **QuakeWorld / Quake 3:** client-side prediction of one's own movement
  plus delta-compressed snapshots. Relevant only to the authoritative route
  (section 9). The local-camera trick in 5.2 is the same "own view first"
  idea.
- **GGPO and emulator netplay (RetroArch, Dolphin):** rollback on whole-machine
  savestates is practical when the snapshot and re-simulation are cheap. TTK
  needs measurement first (5.2).
- **Zandronum / Odamex:** retrofitting client/server onto a lockstep engine
  took years and per-actor work. Evidence for keeping TTK on input sync.

## 7. Hosting and connectivity options

| Mode | How | Needs |
| --- | --- | --- |
| Local split-screen | One process, two seats | Section 4 |
| LAN | LAN broadcast discovery (recomp-net `lan_beacon.h`) plus direct join | Nothing external |
| Direct IP | Host listens on a UDP port; guest enters address:port | Port forwarding on the host's router, or UPnP/NAT-PMP (optional, off by default) |
| Listen server | The host player's game is seat 1 and the session hub | Default for 2 players |
| Headless relay server | A tiny process that only forwards inputs, checks versions and runs the match lobby; **no game, no disc, no BIOS** | Self-hosted on a VPS or home server; same binary on Linux and Windows |
| Optional relay for hard NAT | The headless relay above, or TURN (coturn) when ICE is enabled | Only when direct UDP fails |
| Headless authoritative server | Runs the game itself with no window | Needs the disc on the server. Useful later for desync arbitration or the section 9 route, not needed for two-player lockstep |

**Discovery:** LAN browsing; direct IP; an optional, self-hostable server
list (a static JSON list or the open-source `recomp-net-server`). No accounts.
No default third-party lobby: if the framework's `netplay.retcomm.net`
default is ever enabled, it should be opt-in.

**Administration (relay or listen host):** a config file (port, password,
max matches, allowed arenas and rules, rate limits) plus a few console
commands (kick, end match, list).

**Compatibility checks at connect:** protocol version; our executable or
build identity (as recorded per job); codegen hash; disc fingerprint
(`game.toml [netplay] required_disc_fp`, already filled for our CloneCD
identity); timing model and session rules. A mismatch refuses with a clear
reason, before any match state is exchanged.

**Cross-platform:** Linux and Windows peers must produce identical
simulation. The generated C and timing are integer code, but the compiler,
floating point in host hooks (the modern layer uses `double`/`float` in
control math) and library differences must be checked. Modern-hook maths that
feeds the simulation should be fixed-point or quantised into the intent record
before use. Cross-platform determinism gets its own Phase C test.

## 8. Security and trust model

- **Trust boundaries:**
  - Peers trust each other's *inputs* only. In lockstep, no peer can inject
    state: a tampered simulation diverges and the digest check catches it.
  - The relay is trusted only to forward packets. It sees no plaintext if
    traffic is end-to-end encrypted between players.
  - The host is trusted for match settings and the start signal (framework:
    slot 0 is the session authority), not for game state.
- **Cheating:** speed hacks and teleporting are impossible in input sync,
  because the simulation is shared. Wall hacks and ESP are possible, because
  every peer has the full state (inherent to lockstep; acceptable for a
  community game). Aim assistance by a modified client is possible, as in any
  game, and out of scope. Input validation: intent values are range-clamped
  (yaw delta per tick, move vector length, action bits).
- **Malformed packets:** fixed-size, versioned, length-checked messages;
  drop anything that fails parsing; never index arrays with peer-supplied
  values unchecked; fuzz the parser (Phase E).
- **Denial of service and amplification:** a stateless connect challenge
  (cookie) before allocating any session state. Responses never larger than
  requests until the handshake completes. Per-address rate limits on the
  relay and listen host. Bounded queues.
- **Encryption and authentication:**
  - LAN: optional.
  - Internet: authenticated encryption for all match traffic (libsodium
    secretbox or AEAD after a key exchange). Sessions keyed from a match
    password, or a host public key shared out of band. This protects inputs
    against spoofing and session hijack and keeps chat private. No accounts
    are needed.
  - recomp-net has none of this today (verified).
- **Local safety:** a networked match never writes the player's memory cards
  or savestates, and never loads state from a peer outside the framework's
  checked transfer.

## 9. Player-count scalability

- **Two players:** native to the game. Recommended target for all phases up to
  E.
- **Three or four (multitap):** the framework can deliver 8 pads, but the
  game reads two ports and has two player and two camera slots (section 1.2).
  Extending guest tables means relocating structures in a fixed PS1 memory
  map and finding every reference in generated and overlay code: a research
  project with high risk (hypothesis; Phase F investigation).
- **More players by host-side extension (hypothesis):** keep the game's two
  real players, and represent extra players as host-managed actors that use
  the game's own enemy and actor system (D26E spawn work shows actors can be
  created). This would need a deep actor model and is only worth studying
  after Phase D.
- **Server-authoritative replication:** scales in principle, but needs the
  world state model that we do not have (section 5.1). Long-term only.

**Conclusion:** do not promise more than two players. Treat it as a Phase F
research question with a go / no-go decision.

## 10. Recommended architecture and phased roadmap

**Architecture (recommendation):** one simulation model for local and online
play. The game's own two-player match runs with deterministic inputs (pad
bytes for Vanilla, an intent record for Modernized). Presentation is
per-viewport D17 replay (split-screen locally, own view full screen online).
Sessions use recomp-net behind our own interface, a self-hostable relay, and
authenticated encryption for internet play.

Proposed follow-up jobs (IDs suggested, not yet on the board; the user
decides which to open):

| Phase | Proposed job | Deliverable | Depends on |
| --- | --- | --- | --- |
| A - Discovery | D34A | Run the user's sequence offscreen and on the display. Confirm or reject the section 2 hypotheses with the frame replay debug JSON. Map arena spawns, rules variables and results screen. Measure the live two-player frame cost. Compare the exit flow with Vanilla. Check savestate impact of `MAX_PLAYERS 2`. | D34 |
| B1 - Two seats | D34B | `MAX_PLAYERS 2`, seat routing, press-to-join, Vanilla two-pad play working | D34A |
| B2 - Modern split-screen | D34C | Per-seat modern layer, arenas in the identity guard, replay per viewport (option B), menus and exit flow | D34B, D09 (controller), D19F1 (prompts) |
| C - Determinism and prototype | D34D | Intent record; fixed timing per session; determinism test (two instances, same inputs, matching digests); LAN lockstep two-player match on recomp-net, own view full screen | D34B (C can start before B2 for Vanilla rules) |
| D - Gameplay online | D34E | Modernized intent online, local camera prediction, disconnect and reconnect, match lifecycle (start, results, rematch); then measured rollback | D34C, D34D |
| E - Hosting and security | D34F | Headless relay, direct IP with port guidance, LAN browser, config and admin, version gate, encryption, rate limits, fuzzing | D34D |
| F - Expansion | D34G | More-than-two go / no-go study, arena extras, server list, spectators, quality of life | D34E, D34F |

Order notes: Phase C (determinism) is the real feasibility gate and can run
in parallel with B2, because the proof of concept uses Vanilla rules. If C
fails in a way that cannot be fixed (for example, the generated code or
renderer leaks host state into the simulation), online play stops there and
local split-screen still stands on its own.

## 11. Risks, unknowns and open questions

| Risk or unknown | Impact | How to retire it |
| --- | --- | --- |
| Hidden non-determinism (host time in hooks, uninitialised memory, CD timing, audio or IRQ timing, renderer readback into guest memory) | Online infeasible | Phase C digest test; framework already lockstep-tested on another title |
| CD loading time differs per machine and changes simulation timing | Desync at level start | The framework netplay already handles CD/FMV lockstep (`NETPLAY.md`); verify on TTK arenas |
| Rollback too expensive (snapshot plus re-simulation within a frame) | Online feel limited to delay-based | Measure in Phase D; delay-based at 30 Hz may be acceptable |
| `MAX_PLAYERS 2` changes the savestate format | Old savestates rejected | Check in Phase A; plan a migration or accept with notice |
| Per-viewport replay doubles worker load | Split-screen below 120 FPS | Measure; 60 Hz per half fallback |
| Modern controls need a full rewrite to an intent path | Large Phase C/D cost | Do it once, behind a single-player-tested interface |
| Framework netplay disables mods in sessions | Our layer could be switched off online | Read the policy in `psx_netplay.c` / mod runtime in Phase A |
| Arena data limits (two spawn sets, item tables) | Limits on rules and player count | Phase A mapping |
| Cross-platform float differences in host hooks | Linux-Windows desync | Quantise intent; cross-platform digest test |
| Framework upstream changes to netplay | Merge cost | Keep TTK code behind our own interface; pin as now |

Open questions for the user:

1. Is two players online enough for a first release, with more than two only
   as a later study?
2. Accept input delay (about 66 ms plus network) for the first online
   version, with rollback as a measured follow-up?
3. Vanilla rules online too (pad sync, simplest), or Modernized only?
4. **Decided (user, 2026-10-20):** online and LAN play are full screen,
   each player's own view on their own machine, never split screen. Split
   screen is for local play only.
5. Is a small self-hosted relay acceptable as the official "server", with
   direct IP as the default?

## 12. Proposed initial proof of concept

**Goal:** prove that our build can keep two instances of TTK's two-player
mode in bit-exact sync from pad inputs. Everything else depends on that.

1. A local build variant with `MAX_PLAYERS 2` and the framework's netplay
   enabled for LAN only (`PSX_NETPLAY`, `RNET_ENABLE_ICE=OFF`). Private
   profile, cards and settings; no lobby server.
2. Two instances on one machine (then two machines on a LAN), accurate CPU
   timing, Vanilla presentation, no Modernized hooks, so that only the
   framework's proven pad path is tested.
3. Start TWO PLAYER on MESA from both instances; each instance's keyboard
   drives its own seat.
4. Play ten minutes: move, fire, die, respawn, reach the kill limit, results
   screen.
5. **Pass:** per-tick state digests match throughout; both screens show the
   same match; no stall longer than one second on LAN. **Then:** repeat with
   fast timing (fixed per session) and with Modernized hooks reading a
   minimal intent record (move and yaw only).
6. **Fail:** record the first diverging tick and the diverging memory region.
   Decide whether the cause can be fixed.

Expected size: small, because it is mostly build configuration and test
harness. It changes nothing in the player build until the user accepts a
follow-up job.

## Files read for this note

`recomp/src/ttk/frame_replay.cpp`, `modern_controls.cpp`, `game_menu.cpp`,
`control_guards.inc`, `widescreen.inc`, `spawn.inc`, `cheats.inc`,
`level_select.cpp`, `pickup_select.inc`, `fast_timing.c`;
`recomp/CMakeLists.txt`, `recomp/game.toml`, `recomp/game.local.toml`;
`recomp/psxrecomp/docs/NETPLAY.md`, `NETPLAY_TOPOLOGY.md`;
`recomp/psxrecomp/runtime/runtime.cmake`, `src/main.cpp`, `src/sio.c`,
`src/psx_netplay.c`; `recomp/psxrecomp/lib/recomp-net` (README, headers);
`recomp/psxrecomp/lib/retcomm-rbengine/README.md`; notes 18, 76, 101, 102,
105, 117, 127, 147, 150. Disassembly of `0x80026164`-`0x80026c40` from the
owned executable.

## Follow-up opened, 2026-10-27

D09A's keyboard/gamepad seats are user-accepted. The user explicitly requests
the next local two-player modernization job, so D34C is now open as Todo in
`MODERNIZATION_JOBS.md`; its current scope and acceptance criteria live there.
D09 and D09A plus the retained D34B implementation are its prerequisites.
The proposed D19F1 dependency above is narrowed: complete live prompt icons
remain D19F1, while clear existing textual seat labels permit D34C controls work.
No D34C implementation starts with this backlog update. Other unselected
research proposals remain proposals; online/LAN stays separate.
