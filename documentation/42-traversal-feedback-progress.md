# Delivery checkpoint — 2026-09-27

## Latest checkpoint — user acceptance and next-pass selection (2026-09-27)

**D08G2 and bounded D08J Done by user acceptance.** Armed exterior ladder transfer
worked twice; silent centered cheats are correct. **D08I Needs playtest/refinement:**
substantially improved, but close-bed Shift-preheld W+Space sometimes jumps straight
up. **D08A1 Needs playtest:** design accepted; user explicitly wants Enter to use
the highlighted item. Contextual Enter must consume the press without pausing,
retain ordinary pause elsewhere and preserve U/custom bindings. Not implemented yet.

Next selected order: **D10A** rapid mouse/Shift-running turning investigation,
**D18A** voice/music/gunfire crackle investigation, **D08I** residual bed jump,
**D08A1** contextual Enter activation and guidance. No choose-again step.
Read [exact speech-to-text feedback, acceptance and resume state](45-playtest-turning-audio-follow-up.md).
Shift involvement and audio regression/cause are unproven. Preserve accepted aiming,
furniture, ladder and inventory design. D08K stays Blocked; D19B stays deferred.

This checkpoint changed documentation only: no runtime edit/build/test/game input.
The player may have an active game; do not interfere. Previous delivery hashes and
verification below remain historical evidence, not tests of these new reports.
Current inventory activation is still U; requested Enter behavior is pending.
Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized auto-captures; F10 toggles capture. Continue maintaining precise checkpoints.


## Historical checkpoint — selected feedback delivered (2026-09-27)

**D08G2, D08I, D08J and D08A1: Needs playtest.** Implemented and privately
verified: silent centered cheat results; responsive run-start and bounded small-edge
jump grace; E-owned ladder stow/redraw including weapon-switch intent; visible
inventory strip synchronized with original gadget selection. **D08K: Blocked** on
validated low-gait assets/import and variable-height movement collision; the audit
is complete, but no fake slow-roll/sliding prototype was shipped. **D19B deferred.**

Read [implementation/evidence](44-traversal-inventory-implementation.md),
[crouch audit](43-crouch-walk-audit.md) and
[resumable checkpoint](42-traversal-feedback-progress.md). All five selections have
been handled for this pass; do not repeat implementation or start deferred menus.
Next action is the player's test/feedback, or explicit animation-pipeline work.

Current binary SHA-256:
`f89f1ca92d5a7e0445022d98e6d9925bf893249bd9fa5bd410c7eec99e3c3bcc`.
Normal build and patch stack, six native suites, 60 Python tests (2 conditional
skips), both-renderer UI captures and the 14-checkpoint Vanilla route pass.
Two 18-case run-start sweeps pass; late edge, pistol/shotgun ladder transfers,
weapon-switch E, redraw/ammo and accepted bed/couch movement were replayed.
All private games are closed. All 11 player config/card/state hashes unchanged;
original media/assets and unrelated local source work preserved. No commits made.

Test quick run-start/late furniture-edge jumps; armed E first/second ladder and
weapon-switch timing; brackets/U/I selection; silent cheats and centered results.
Recheck accepted aiming/furniture feel. Crouch+direction still rolls.
Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture. Broader campaign/era/weapon
coverage remains unclaimed. Historical checkpoints below are superseded.

## Resumption details

There is no active helper or game to resume. Final verification manifest:
`recomp/analysis/iteration42/verification.json`. Source changes: pc_input.cpp/.h,
modern_controls.cpp, terrain.inc, shortcuts.inc, control_guards.inc, duke_font.cpp/.h,
new inventory_hud.cpp/.h, native tests/CMake and two ordered runtime patches.
Original generated C was never hand-edited. Durable crouch decoder:
`recomp/tools/local/audit_crouch_assets.py`; full derived data stays in local analysis.

The evidence report 44 distinguishes failed reproductions, invalid helper runs and
successful final replays. In particular: shotgun stow needed its final blend0/1;
E during manual switching needed stable-state gating; the last transition-ladder
route's early height assertion was a helper duration issue, then original climbing
and the subsequent transfer passed. Do not reopen these as uninvestigated failures.

The user requested continuous progress documentation for switching agents. The
chronology below was recorded throughout implementation; its process IDs, "active"
labels and pending checks are historical and superseded by this delivery summary.
Weekly account quota is not visible to the agent, so no percentage was invented.

## Historical implementation checkpoints

### Initial checkpoint — 2026-09-27

User authorized, in order: D08G2, D08I, D08J, D08K, D08A1. Do not ask selection
again. D19B remains deferred. User requests frequent resumable documentation for
an agent switch; weekly quota is not visible to the agent. This file is an active
checkpoint, not a completion report. Read alongside 41-playtest-follow-up-plan.md.

## Preservation and working state

No player game was running at entry. Existing dirty source/submodule edits were
retained. Player config/cards hashes: `recomp/analysis/iteration42/preservation.json`.
All test sessions use private Xvfb, dummy audio, isolated cards/settings and ports
9164/9165. Do not send input to the player display or port 9123. Do not overwrite
original media or generated C. Source repository is `recomp/`; root docs are separate.

## D08G2 implemented, verification underway

- `src/ttk/pc_input.cpp` no longer publishes partial/cancelled cheat text. Parser,
  pending command and reset behavior unchanged. Results use a centered OSD API.
- Runtime header/host OSD/GL changes are preserved as ordered patch
  `patches/time-to-kill-zzzzz-centered-quotes.patch`. Ordinary toasts, FPS/status,
  volume and other panels retain placement.
- `duke_font.cpp` style 2 centers each wrapped line; style 0/1 retained. First GL
  captures predate that small refinement; Software session uses it.
- Native input passes, including silent parser/aliases/reset/expiry assertions.
  Patch stack verification passed before the final line-centering refinement.
- `analysis/iteration42/quotes.py NAME` drives all result labels/aliases at
  320x240, 640x480, 1024x768, silent partial/invalid/expired captures, then pauses.
- GL evidence: `analysis/pc-input/iteration42-baseline/quotes.json` and PNGs.
  320 Got All Weapons/Ammo was visually reviewed (block centered, wrapped).
  Software evidence session is still underway. Final renderer capture review,
  final GL wrapped-line refinement check and native font checks remain.

## D08I reproduced and first source fix drafted (NOT BUILT)

Baseline binary had only D08G2 changes; traversal unchanged.
`analysis/iteration42/run-start.py` sweeps offsets 0,1,2,3,4,6,8,12,18 and two Shift
orders using real SDL inputs. Nominal frame waits include scheduling overhead;
actual frame receipt/animation/support/velocity are recorded.

First sweep near spawn wall reproduced misses but entered original wall-bump 95;
this is an obstruction case, not evidence of open-ground cause. Preserved in
`analysis/pc-input/iteration42-baseline/run-start-{rows,results}.json`.

Moved naturally to street around Z=5342. The separate `street-start-*` captures
reproduce genuine grounded misses: at frame 20672 jump is on pad 32751, run 76,
state 0/0, floor delta 0, player+228 bit 8. Buffer releases around 20680; the stride
ends around 20689. No flight. Other short offsets reproduce this.
Original 80052298 sets bit 8 on run entry; 800539B4 gates jump on it; 8004B938 clears
it at stride completion. Thus the first stride delay outlasts the 8-sequence buffer.

Draft fix in `modern_controls.cpp` clears ONLY that bit for a fresh buffered press
at authenticated 80053500 callback; all original run floor/obstruction/jetpack/
takeoff checks remain. `pc_input.h/.cpp` adds non-consuming `input_jump_pending()`;
committed ballistic takeoff consumes it once. Input debug JSON now includes
sequence and jump_remaining and has a larger buffer. Native fixture stub and new
assertions must be added before building. Helper script `analysis/iteration42/edit-run.py`
was already applied: do not rerun it blindly.

Edge misses are NOT yet reproduced. Current private baseline is naturally routing
to apartment for separate bed/couch edge tests. Do not claim edge cause or fix yet.

## Next work, in order

1. Finish G2 capture verification. Add native run-start guard tests; build candidate
   after collecting edge baseline. Replay measured street sweep, original input
   orders, walls, furniture and Vanilla.
2. Reproduce late bed/platform edge jump separately, distinguish grounded/airborne;
   implement only justified bounded changes (no double jump or reach increase).
3. D08J: armed run-jump then E to second exterior ladder vs holstered. Prior accepted
   transfer used E BEFORE running (see documentation/31-ladder-transfer.md), which
   is not the newly reported airborne armed path. Original eligibility/stow tracing
   and fix still pending. Do not claim a cause yet.
4. D08K actual asset audit/prototype feasibility. Preliminary read-only findings:
   ANM records are start/count pairs, FRM records 8 bytes (loader/593b4 evidence).
   Resident animation->four tracks at 800c2d64; runtime ANM 800d8328, FRM pointer
   800d8324. 176 tracks 109/111/112/110, 178 tracks 105..108; 181–184 tracks
   277/279/280/278, 281/283/284/282, 269/271/272/270, 273/275/276/274. Actual
   DB00 ANM/FRM/ROT/HIE/COL bytes inspected, encoding/viable gait not yet established.
   181/182 dispatch 80054308, 183/184 dispatch 80054218. Need pose/root-motion/
   collision inspection, not labels. No crouch implementation changes yet.
5. D08A1 original inventory ownership/art audit and visible strip implementation
   pending. Existing `shortcuts.inc` host selected_item has no menu synchronization
   or cycling notice. Do not claim implemented.

## Active private processes and control helpers

Check processes before acting; these identifiers can become stale.
- `iteration42-baseline`, port 9164, GL, initial PID 94384. Driver runs
  `DNTTK_PROBE_EXPLORE=1 DNTTK_TRAVERSAL_TRACE=1 pc_input_probe.py --controls traversal
  --automatic-entry --renderer opengl`. Original pause between operations.
- `iteration42-quotes-software`, port 9165, Software, isolated-concurrent flag.
  quotes.py process is testing it; do not issue concurrent inputs to that session.
- `analysis/iteration42/apartment.py iteration42-baseline` is currently routing
  naturally to apartment. See `analysis/iteration42/apartment-baseline.log`.
  It writes `fresh-route.json`, pauses, touches `route-finished` when done.
- `analysis/iteration-40/live.py` provides isolated Live(NAME) helper. Validates
  private cards/process/display. Use `DNTTK_TEST_PORT=9165` for Software. resume()
  requires original pause; pause() releases fire and enters original pause. Never
  use unsupported debug pause, process SIGSTOP, or player cards.
- `analysis/d08-feedback/op.py NAME JSON` communicates through the probe's private
  request/response files. A `{ "stop": true }` request exits the owned exploration
  session. Direct Live routes must not overlap request-file operations.
- Build logs under `analysis/iteration42/`; G2 current source refinement compiled
  in `build-g2-final.log`. No D08I candidate build yet. No commits made.

When finished: update board/status/handoff/manual and focused evidence truthfully;
Needs playtest for human acceptance, concrete blocker if crouch cannot be safe.
Always finish with player test instructions and
`python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.

## Update: run candidate and edge baseline

G2 Software finished and exited 0; 320 wrapped lines and 1024 centered text reviewed.
Native input/controls/font checks pass. Run-start draft is now built and running in
`iteration42-run-candidate`, GL port 9165. `candidate-start.py` and run-start.py are
actively sweeping; do not send concurrent input there. Initial candidate street
Z=6239 differs from baseline Z=5342; interpret wall contacts separately. Early
formerly missed run offsets now launch in 1–3 sampled guest frames.

Baseline apartment reached after navigating around a lamp post at (15510,7288);
route at X=15893 succeeded. The first failed route and waypoint oscillation are
excluded from acceptance. `edge-baseline.json` shows grounded walking presses at
X=8907 and X=9183 both launch original 98. `late-edge.json` separately reproduces
no jump after original short-drop 108/state9 begins (floor delta383). This is
post-support input, distinct from the stride lockout. Late-edge helper initially
called look during landing105, so direction remained west; the measured fall is
valid, but do not describe it as eastward. No location/state fixtures.

A six-input-sequence edge-grace prototype is drafted in terrain.inc: only an
owned small drop, recent verified ground, descending velocity, same input epoch,
original current headroom and forward clearance, then original 3e2d0 velocity
constructor and original jump animation. It consumes one press, expires on lost
ownership and cannot reapply during jump/large fall. This draft is native-testing
and NOT in the live run candidate. Native test added for age/epoch/large fall/
headroom/obstruction/upward-motion refusals and no second impulse.

Baseline GL port9164 currently runs `ladder-baseline.py`: returns through apartment
window, then armed run+jump before E. See ladder-baseline.log and resulting
armed-ladder-baseline.json. D08J cause still unproven; no ladder changes yet.

## Update: live run-start sweep passes

`iteration42-run-candidate/street-candidate-results.json`: all 18/18 combinations
entered flight. Fresh running cases generally 1–3 sampled guest frames; walking/
standing retain original ~15–17-frame preparation. One later run sample was six
frames. Baseline open-ground missed four together-order offsets. Positions differ
and nominal waits include capture overhead; do not claim precise millisecond latency.
Native edge fixture initially failed because its CPU stack was below original_call's
safe range, not a production failure. Corrected to real stack range; all seven
positive/negative edge cases now pass. Source edge trace is opt-in via existing
DNTTK_TRAVERSAL_TRACE. New edge-inclusive binary is building/running as
`iteration42-edge-candidate`, port9166 Software, followed by fresh-apartment.py
(god/hide only, no grants), natural route and contact-bed jump. See edge-candidate-*.log.

D08J early airborne E succeeded on unchanged ladder baseline: pistol equipment2,
upper5 -> stow6 -> equipment0/reach109 -> attached148/state3. E later than takeoff
is therefore not universally broken. `late-ladder.py ... armed` now testing E only
at X<=5500 during same transfer. Baseline descended original ladder with S, detached
using rebound jump J and landed on ground. No positional fixture. No ladder fix yet.
`iteration42-run-candidate` port9165 is naturally routing into apartment separately;
currently lacks edge change, so do not use it to certify edge grace.

## Update: edge grace live success, ladder comparison pending

Edge-inclusive Software candidate naturally reached the apartment and repeated
contact bed jump. `edge-candidate.json`: late press in original108 produced exactly
one edge_jumps increment and a new original jump. See edge-live.log (edge_jumps1).
Native guard suite passes after fixing test-stack setup. Full edge/furniture/
Vanilla final regressions remain; the six-frame policy applies only to small drops
(<=768 world units), not large exterior falls. Preserve that documented limit.

Baseline GL and run-only GL sessions now exited cleanly. The holstered late E
comparison at X=5212 caught original148/state3 (`iteration42-run-candidate/
late-ladder-holstered.json`). Returning along the ground from the second ladder
hit actual alley geometry; failed return routes are excluded. A fresh session
`iteration42-late-armed`, port9165 GL, uses the edge/run candidate BUT unchanged
weapon logic, to reproduce the same late-E armed case. `fresh-platform.py` is still
booting/routing; do not input concurrently. Port9166 edge candidate is paused in
apartment after edge test. Old port9164 is no longer active.

Crouch read-only audit in `analysis/iteration42/asset-audit.py/.json`: decoded actual
DB00 ANM start/count, FRM event/duration/ROT ranges, signed packed root translation
and joint0 components using original readers593b4/58c70/597e4. Clips181–184 contain
large root travel and joint0 rotations exceeding180degrees; they are rolls, not
low walking. 178/179 stationary crouch sequences have no alternating translating
foot gait. No prototype or definitive blocker declared yet; visual pose/collision
inspection remains. Never call slowing the roll a crouch-walk implementation.

## Update: D08J failure reproduced and candidate drafted

`iteration42-late-armed/late-ladder-armed.json` establishes failure with E at X5395:
frame10805 run-flight104/upper5/equipment2; frame10822 X4504 still upper6/equipment2;
frame10834 original bounce107, equipment0 too late; then floor recovery105. Its
later held-E grab of a DIFFERENT lower ledge (140/state8) is not second-ladder
success. Holstered comparator at X5212 attached148/state3. Early airborne armed
E had succeeded, so this is a contact/stow timing problem, not universal armed denial.

Candidate `input_auto_stow_pending()` requires active captured E-owned pending
stow. Only its authenticated airborne upper transition gets 4x time budget through
original event processing, once per update. Ordinary grounded 1.5x transition and
all contact/attachment rules unchanged. Native controls checks pass all representative
upper tracks with no repeat budget; input ownership/focus tests pass after making
the old fixture's holster readiness reflect actual airborne production permission.
Candidate binary is building (`build-j.log`). Live second-ladder replay still pending.
Late-armed baseline session was asked to close; check process state before reuse.

Port9166 paused edge candidate now running crouch-audit.py, recording all four
original crouch+direction rolls and actual model mapping. Do not overlap input.

## Update: ladder candidate caught; crouch audit resolved

D08J candidate replay matches failure's E position exactly (X5395): frame10811
flight104/equipment2; frame10828 original ladder148/equipment0; frame10832 state3/3.
Settled207 is attached underside traversal, visibly confirmed in screenshot.
Evidence `iteration42-ladder-candidate/late-ladder-armed.json`. Automatic redraw
on safe exit and additional timings still need verification. Port9165 candidate
is paused attached; port9166 edge candidate paused crouched in apartment. Neither
has inventory code yet. No helper input scripts currently running.

D08K audit documented in43-crouch-walk-audit.md: actual root/joint tumble + live
roll and fixed standing movement clearance establish why no safe low-walk prototype
can be delivered from current assets/pipeline. No fake sliding/slow-roll substitute.
Mark Blocked with that concrete dependency at final board update.

D08A1 original menu audit: 800885e0..88608 rebuilds cursor from selected item halfword
800c3f94; 8008938c..893c8 writes it when browsing. Thus sharing that selected ID
synchronizes without touching list/cursor. Usable gadget menu presentation at
80089798..898a8 is original text-name/arrow drawing, not suitable standalone icons.
Use explicit named text tiles with original TTK names and charges as documented
fallback. Implementing dedicated temporary strip next; existing bindings unchanged.

## Update: inventory implementation and validation state

D08J safe exit passed: private ladder-exit.json shows release/drop/recovery then
original idle63, equipment2, weapon4 (pistol); screenshot automatic-redraw.png.
Old ladder and edge sessions have been asked to stop; check before reusing ports.

D08A1 implemented in inventory_hud.cpp/.h and shortcuts.inc; dedicated renderer
patch time-to-kill-zzzzzz-inventory-strip.patch. Temporary two-second Duke-font
named tiles, arrow, actual amounts and * / On for active gadgets. Selected halfword
shared with original menu (authenticated initialization/browser ranges added).
Original menu gadget presentation is text/arrow, so named tiles are fallback.
Pending native/live review, not accepted yet. Build initially caught missing
initializer_list in new native test; corrected. A too-small font raster area
would hide short tile labels; corrected before final candidate launch.
Native controls initially caught an unnecessary unchanged selection write; now
remember_item writes only on actual difference. Capture/gameplay guards added.
Depleted selection falls back to first eligible item before processing next input;
fixture expectation corrected to that explicit policy (no game effect mutation).
60 Python tests pass (2 conditional skips). Native input/aim pass. Controls and new
inventory tests rebuilding; font/scene and final patch stack/full build remain.

Active GL session iteration42-inventory-gl port9164 runs the *first* inventory
binary (before tile layout/no-op write fixes). quotes.py is currently testing final
G2 centered wrapping there; do not overlap inputs. No inventory visual acceptance
from this older candidate. Final module candidate build: build-inventory-final.log.
Next: launch final Software/GL candidates, validate strip/menu/U/timeout/resize,
additional ladder timing/stow class, furniture regression, final isolated Vanilla,
then documentation/manual/board and preservation verification. No player data writes.

## Update: inventory live findings require one refinement

Both first inventory candidates showed raw timer amounts wrapping inside tiles.
Original menu895fc..8963c uses amount*100/live capacity; live capacity table gives
9000/13500/18000/10000 for jet/bio/goggles/medkit. Renderer now uses that percentage,
with an active corner marker and selected-name On. Do NOT accept earlier raw-value
screenshots as final. build-inventory-percent.log rebuilds this refinement.

Software live confirms zero inventory and cycle-only item-byte immutability across
both directions/sizes. Initial menu/U checks were INVALID: Select disappeared on
capture release, and U was pressed before the original biomask pending event
finished. Original-menu reproduction: captured I/RightShift fails; releasing F10
first then RightShift opens actual inventory. input_pad_context cleared Select on
its first sampling, before the guest menu update. Added a bounded six-input-frame Select
pulse after capture release, cleared by focus/reset. This is an A1 menu entry
correctness seam; D19B menu animation/repeat redesign remains deferred.
Actual original text menu screenshots in menu-uncaptured.json and uncaptured-*.png;
menu browsing/activation still needs measured settled keys on new candidate.

Software port9165 and GL port9164 are still OLD inventory binaries, currently
finished input scripts (Software left after uncaptured-menu return, GL paused).
Do not use them for final percentage/pulse acceptance. Full build passed before
this small refinement; rerun incremental/native and final build receipt afterward.

## Update: inventory mechanics pass, final layout captures booting

New Select pulse verified live in GL and Software: captured I opens actual menu,
medkit selection is retained; Right/Up changes original selection to goggles3;
after close host selected_item agrees. B completes biomask on, settled U switches
it off. Cycle bytes remain unchanged, rapid presses/held no-repeat, expiry,
released capture and focus transitions pass. Evidence iteration42-inventory-percent-
{gl,software}/inventory-verify.json. Software uses custom O/P/L bindings. Native
input/controls/inventory checks pass. These mechanics sessions are now closed.

The percent symbol still wrapped inside narrow tiles in visual review. Final layout
now puts bare percentage numbers in tiles (same as original menu) and the explicit
percentage alongside the selected full name. New final visual sessions:
- iteration42-inventory-layout-gl port9166, default bindings; boot/layout helper active.
- iteration42-inventory-layout-software port9164, default bindings; boot/layout helper active.
Do not overlap input. They use final text layout build-inventory-layout.log.

iteration42-inventory-software port9165 remains older HUD but current traversal
code. Natural apartment route succeeded; contact bed jump plus three walk/run
departure/contact-jump repeats pass. bed-side.py currently running, then couch
and large-fall/extra stow-track replay remain. Tests grant original items in this
session; do not use it as concealed-pickup acquisition proof. No player changes.

## Update: broader stow track exposed a second original transition

18/18 additional run-start sweeps pass with host turbo enabled (`iteration42-
inventory-layout-gl/street-turbo-results.json`), fresh run samples2–4 guest frames;
standing preparation remains. Final layout captures in both renderers reviewed:
no wrapped numbers, active97% label, arrow/marker and HUD spacing correct.
Couch release during actual108/state9 now passes (`couch-release.json`).

Shotgun transfer in old-UI/current-traversal Software9165 FAILED. Preserve it as
`shotgun-transfer.json`: upper20 ->21 ->1, equipment0 byX4959, but upper1 persists
past contact and original bounce107. Reach input was present (pad49135). The
original800557f8 checks upper animation-table bit8 at558a0; table0/1 both0x2009,
so generic final blend still blocks reaching. Pistol6 completed via0 quickly;
shotgun's1 did not. This is distinct from equipment still drawn at contact.

Refining source: retain E-owned airborne animation budgeting after equipment0
through generic upper0/1, using original events. input_auto_stow_pending now
includes bounded airborne_interact lease as well as pending; still capture/focus/
restore/interaction_started/deadline guarded. Native tail checks/build and matched
live replay pending. Do not call D08J finished yet. No attachment/reach changes.
Port9165 paused attached to a later lower ledge after failed shotgun attempt;
port9166 paused street after turbo sweep; port9164 final layout session closed.

## Update: tail candidate built, fresh live verification active

Native controls/input tail checks pass (native-{controls,input}-tail.log). New
binary built inbuild-stow-tail.log; full normal workflow running again in
full-build-release.log. Source now includes bounded generic0/1 post-holster blend
budget. No other weapon/reach/physics changes.

Only active game: iteration42-shotgun-tail, GL privateport9165; fresh-shotgun.py
is booting/routing. It grants via original dnweapons, selects shotgun3 before the
first ladder, then tests first-ladder auto-stow, second transfer atX<=6000 and safe
redraw/ammo. Do not send concurrent input. Earlier failed shotgun route starts at
X8901/Z13679; new fresh ladder route may differ, so don't claim exact matched
shotgun positions. Pistol failure/success comparison did match E atX5395.
All other iteration42 sessions asked to close. Next: evaluate this replay; if pass,
final pistol regression/large-fall checks if practical, isolated Vanilla14-checkpoint
route, final documentation/status/manual/preservation/launch. No player files changed
(all11 preservation hashes matched inpreservation-verified.json).

Also booting a final pistol regression in Software: iteration42-pistol-final,
privateport9164, fresh-platform.py active. It repeats the exact late-E threshold
X<=5500 with the tail refinement. Do not overlap input. Shotgun-tail9165 has
entered gameplay and is selecting shotgun before its natural first-ladder route.
The final full-build-release workflow passed; later source edits are comments only.

## Update: shotgun tail live passes; transition-intent seam reproduced

Fresh iteration42-shotgun-tail: first armed shotgun ladder route succeeded;
second airborne transfer E atX5671, upper21->1->original148/state3, safe exit
restores shotgun5/upper20; ammo remains50. Actual screenshots reviewed. Starting
positions differ from the failed shotgun replay, disclosed in44. Final pistol
Software replay also caught the second ladder with the tail change (207/state3).

A final acceptance check during manual weapon switching exposed another existing
intent race: transition-intent.json inshotgun-tail shows held E while switching
shotgun->pistol. upper21/equipment2/flags4 ->1/equipment0/flags4 ->0/equipment0/
flags0 ->6/equipment2 ->5/equipment2. E ended still armed: its stow pulse was spent
inside the unrelated switch, and transient equipment0 consumed the pending request.

Refinement building inbuild-transition-intent.log: holster readiness now requires
stable original armed idle and no switch flag4; interaction readiness rejects
flag4 and original upper-table bit8. Pending E therefore waits through a manual
switch and its final blend rather than treating temporary equipment0 as complete.
Native actual-gate + pending-intent regression checks added. Original contact/events,
request deadline and firing suppression unchanged. Must verify this final candidate
live before closing D08J. Existing shots and inventory/furniture evidence still valid.

## Current final-candidate verification processes

Native input and controls pass after stable-switch gates (native-*-transition.log).
Full normal build running infull-build-transition-final.log.
- iteration42-transition-final Software9164: fresh-transition.py active. Grants
  through original codes, runs reproduced manual-switch/held-E check (expects
  holstered while held and intended pistol after release), then deliberately
  switches to shotgun exactly while pressing E/walking up first ladder, then
  selects pistol and runs late second transfer plus safe exit.
- iteration42-shotgun-final GL9165: fresh-shotgun.py active. Armed first ladder,
  shotgun second transfer and safe redraw/ammo.
Both booting; do not overlap inputs. Earlier sessions closed. After these pass,
close all private games and run the original isolated Vanilla14-checkpoint route;
finish board/status/handoff/manual/evidence and preservation. D19B still deferred.

## Update: final shotgun and manual-switch E checks pass

iteration42-shotgun-final passes armed first ladder, second ladder, redraw/ammo
with stable-switch gates. Asked to close. iteration42-transition-final reproducer
now ends held E at equipment0/upper63; release restores intended pistol4: pass.
Its deliberate switch+E first-ladder approach attached190/state8, but the old
fixed-duration route asserted platform height too early (Y-11027). This is not a
missed grab: helper did not allocate extra climb time after waiting for the manual
switch. finish-transition-route.py is currently continuing W through original
climb, then pistol second transfer and exit. Inspect its completion before final
status. No source change made in response to that route-timing assertion.

## Update: final traversal verification complete

Final transition approach completed after extra original climb time. Actual first
ladder190/state8 ->platform63; subsequent late pistol transfer attached186/state3
and safe exit restored63/equipment2/pistol4. See iteration42-transition-final/
first-platform-transition-completion.json, late-ladder-armed.json, ladder-exit.json.
transition-route-completion.log reports pass. Both final stow families now verified
with the stable-switch gates. The earlier fixed route-height failure is explicitly
excluded as a grab failure. Asked last private session to close. Next: Vanilla14
checkpoints on the current binary, final doc/status/manual and preservation checks.
