# D34B - Classic two-controller split screen

Status: Closed for now at user request, 2026-10-27. Physical-controller acceptance deferred to D09.

## Scope and authorization

Restore the original two-player rules and controls inside a Modernized session.
D09 modern controller gameplay, per-player modern cameras, high-refresh split
screen and networking remain separate work. Original mode alternates viewport
updates, so native match motion is not equivalent to the single-player replay
renderer at 120 Hz.

The initial source audit confirmed the compatibility gate in the job board:
`MAX_PLAYERS 1` produces a different SIO snapshot layout from `MAX_PLAYERS 2`.
The existing loader rejects that old section size. The user explicitly
instructed "proceed" after the proposal to preserve existing saves by adding a
backward-compatible loader first. No player saves or memory cards are migrated
or overwritten.

## Candidate implementation

- Build and game configuration expose two digital controller seats. Each seat
  requests a separate SDL controller; the runtime already avoids assigning the
  same device to two seats. Keyboard input remains available to player 1.
- The SIO reader recognises the old one-seat section, including its pre-rumble
  variant, and reads fields with the saved seat count. Missing seat state starts
  neutral. Current two-seat saves retain the existing two-seat wire format.
- PC menus accept mapped digital controller buttons. Character selection assigns
  each seat its own row and confirmation bit. A new confirm after both rows are
  locked starts the match through the original handler.
- Two-player gameplay bypasses single-player replay presentation and resets its
  history at entry/exit. The renderer classifies split screen as native 4:3;
  title widescreen portal/near-clip helpers also stand aside.
- Modern keyboard/mouse capture and the single-player CPU timing leases step
  aside during the match. Original controller simulation remains game-owned.
- Explicit MAIN MENU from pause retains the game's results screen. Triangle Quit
  from multiplayer results returns to the main menu instead of reopening match
  configuration, including after natural time-limit completion.

## Evidence

Verified native test: `python3 recomp/tools/local/test_sio_compat.py` compiles
actual SIO snapshot code for one and two seats. A one-seat encoder fixture loads
in the two-seat reader, retaining pad configuration, mid-poll response state,
rumble values and memory-card sector/data fields. The pre-rumble variant loads;
truncated and oversized sections are rejected. Two-seat round trips preserve
seat 2. These are synthetic module tests, not whole-game save-load evidence.

The complete runtime patch exported through
`recomp/tools/local/export_runtime_patch.py` passes `git apply --check` against
an archive of the pinned framework commit. Pre-existing framework changes remain
included. No dependency pointer update substitutes for the patch.

Private runtime harness and evidence: `recomp/analysis/d34b/`. It creates two
virtual SDL gamepads routed through the normal device/port sampling path, uses private profiles/cards and a
private debug port, and cleans up its devices/process. The diagnostic-only
`DNTTK_TEST_CONTROLLERS` file holds two decimal SDL button masks; ordinary
player sessions never enable it. Virtual-device results do
not replace user testing with two physical controllers on the real display.

## Runtime and regression evidence

The player build and native input, controls and aim suites pass. The input suite
runs under private Xvfb; SDL offscreen cannot supply the relative-mouse capture
that this suite requires. Its new D34B case verifies match-entry release and
single-player Modern capture recovery. Controls include the owned overlay set.

Private SDL routes observed:

- An old one-seat title savestate loads successfully in the two-seat player.
- P1 Right changes only P1's character; P2 Right changes only P2's character.
  Independent confirmations set lock bits 1 and 2. Either seat can start after
  both confirmations.
- In the vertical MESA arena, P1's forward input changes only P1's X/Z, and P2's
  changes only P2's X/Z. The idle player's vertical settling is not movement
  cross-talk. The measured field cadence was 59.93/s.
- Start from either seat opens the PC pause menu; Cross resumes. Native drawing,
  firing, jumping, a credited P1 kill and P2 respawn were observed. A private
  two-seat match save/load also succeeds.
- Both split orientations show both native viewports in a pillarboxed 4:3 image.
  Bounded captures during idle, travel and firing show no missing half or held
  loading image. A running-match counter interval had 121 host presents, 24 game
  flips and zero replay draws/sessions. It does not establish 120 Hz viewport
  animation or exhaustive flicker absence.
- MAIN MENU from pause reaches native results, showing the recorded kill. P2's
  Triangle quit returns to the main menu; the input scheme reports Modern again.

The original reported frozen spinner and flicker were not reproduced in a
separate pre-change binary during this session. Their source paths and the
candidate's observed correction are distinct evidence, not an old/new visual
A/B proof.

A horizontal match reached its five-minute time limit naturally at 301.14 s of
monitoring, with no timer writes, and displayed native results. The final build
also loaded that private results snapshot and returned to MAIN MENU with P2
Triangle. Evidence: `natural-match.json`, `time-limit-results.png`,
`final-results-quit.png` in the private evidence directory.

Private X11/Xvfb campaign checks load the old one-seat slot 5 successfully.
Modernized W moves the player, Escape releases capture for pause, and Escape
resumes with capture restored. Vanilla Up moves the player with legacy controls
and no Modernized capture. Evidence: `single-modern-final.json` and
`single-vanilla-final.json`, corresponding PNGs and redacted route/runtime logs.
The software Xvfb run logs pacing backoff; it is an input/regression smoke test,
not performance acceptance. No player cards or settings were used.

Final executable SHA-256: `0a233c353b9b3fd53ca20f6090697fe070460372e929c14f3309ff3e6f69ad9f`.
The exported framework patch applies to a clean archive of its pinned commit;
every modified tracked framework file matches the applied result. Repository
privacy checks and both tree whitespace checks pass.

## Outstanding verification

Two physical controllers and real-display user acceptance remain. Testing is
bounded to MESA and the named savepoints, not all arenas or the full campaign.
Native alternating viewport cadence remains; this job does not provide modern
high-refresh split-screen animation. No acceptance, commit or push performed.


## User-requested closeout, 2026-10-27

User reports being stuck at Red Duke / Blue Duke and requests no further code:
"lets lock it in and close it out for now". Preserve this implementation and
close D34B for now, with the real-controller blocker and remaining match/display
acceptance carried into D09. The blocker is user-observed; its cause has not
been diagnosed, and virtual-pad success does not resolve it. D09 remains Todo.

The earlier no-commit statements describe the candidate stage. This instruction
authorizes committing and pushing existing authored implementation, tests,
manual and closeout documentation as MusicMonsterMod. No code changes, rebuild
or launch during closeout. The executable and previous bounded evidence remain
unchanged. This is not a claim that all original acceptance criteria passed.
