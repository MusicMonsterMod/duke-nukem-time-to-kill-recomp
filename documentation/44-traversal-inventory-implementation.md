# Selected feedback implementation — 2026-09-27

**Subsequent user playtest:** D08G2 and bounded D08J explicitly accepted (Done).
D08I substantially improved with one close-bed Shift/W/Space refinement pending.
D08A1 design accepted; user now explicitly requests Enter activation of the
highlighted item without pausing. That change is not in the build documented here.
See [current reports and next work](45-playtest-turning-audio-follow-up.md).
Statuses and test requests below describe the historical delivery checkpoint.


Implementation and verification report. Final outcome/status and remaining human checks are in
[the handoff checkpoint](42-traversal-feedback-progress.md). Scope: D08G2, D08I,
D08J, D08K, D08A1; D19B remains deferred. No campaign-wide acceptance is implied.

## Silent cheat entry (D08G2)

Partial/invalid/expired/cancelled typed input no longer publishes notices. Exact
completed-result wording and effect guards remain unchanged. Dedicated centered
message API and Duke-font style2 center each wrapped line in the upper quote
area in OpenGL and Software. Volume/status panels retain their placement.

The main toggle/grant labels and aliases, silent partials and expiry were captured at320x240,
640x480,1024x768. Final GL wrapped Got All Weapons/Ammo and expiry reviewed in
`iteration42-inventory-gl`; Software counterparts in `iteration42-quotes-software`.
Native parser checks cover aliases, reset and silence. Runtime changes are retained
in `time-to-kill-zzzzz-centered-quotes.patch`.

## Separate jump reproductions (D08I)

A first sweep at the spawn wall entered original bump95; excluded from open-ground
diagnosis. A natural street route then reproduced four misses among nine tested
Shift+W press offsets, while the alternate W-then-Shift order worked. Evidence:
`iteration42-baseline/street-start-{rows,results}.json`. At frame20672, pad32751
contained Jump, floor delta0, lower76, normal state0/0, and player+228 bit8 was set.
The jump buffer ended around20680; the first stride released that bit around20689.
Original code sets it at80052298, checks it at800539b4 and clears it at8004b938.

The authenticated80053500 hook clears only bit8 for a fresh buffered directional
request in supported normal Modernized movement. Original floor, obstruction,
jetpack and takeoff logic still decides whether to launch. Committed ballistic
takeoff consumes the request once. Replay `iteration42-run-candidate/
street-candidate-results.json` entered flight in18/18 cases; fresh running normally
1–3 sampled guest frames, one6. Standing/walking retain preparation of about15–17.
Baseline and candidate street positions differ; nominal waits include SDL/debug
scheduling. These are measured gameplay samples, not exact millisecond guarantees.

A separate apartment test established that supported walking jumps on the bed
worked, while a press just after original small-drop108/state9 began did not.
The six-input-sequence grace is explicitly a new bounded policy: same input epoch,
recent verified ground, owned small drop, descending velocity, no object-relative
support, current headroom>=0x370 and original forward probe clear with floor0..768.
It invokes original8003e2d0 velocity construction for jump98/103 and consumes once.
No position/gravity write or second airborne impulse. Large exterior falls are
excluded. `iteration42-edge-candidate/edge-candidate.json` records one late-edge
launch. Native tests reject stale/foreign epochs, age7, large fall, upward motion,
obstruction and inadequate headroom.

## Armed ladder timing (D08J)

Early airborne E with pistol drawn succeeded on unchanged weapon logic. A late
holstered E at X5212 also caught148/state3. The comparable armed case failed:
`iteration42-late-armed/late-ladder-armed.json`, E at X5395/frame10805, flight104,
equipment2/upper5. At contact X4504/frame10822, stow6 still had equipment2; original
bounce107 followed at10834. Equipment cleared too late. A subsequent lower ledge
catch140 is explicitly excluded from second-ladder success.

The candidate accelerates only E-owned airborne original upper stow tracks by4x
once per authenticated animation update. Pending interaction, capture/focus,
restore ownership, deadline, actor/caller/stack and original transition IDs are
required. Ordinary grounded1.5x transitions remain. No ladder contact/reach or
attachment mutation was added.

Candidate replay used the exact failing E position X5395: frame10811 original
flight104/equipment2, frame10828 attached148/equipment0, frame10832 state3/3.
Screenshot confirms the same ladder; settled207 is underside traversal, not ground.
`iteration42-ladder-candidate/ladder-exit.json` then drops/lands and restores original
pistol4/equipment2/idle63 without firing. Native tests exercise materially different
upper transition tracks, ownership rejection and once-per-update budgeting.

## Crouch feasibility (D08K)

Blocked with concrete inspected asset and collision dependencies; see
[the original-asset audit](43-crouch-walk-audit.md). No fake slower roll or sliding
idle was shipped. Existing crouch/roll controls and original assets are preserved.

## Inventory (D08A1)

Original menu885e0..88608 rebuilds its cursor from selected halfword800c3f94;
8938c..893c8 updates that ID during browsing. The guarded shortcut adapter shares
that ID, never rewrites the list/cursor, and observes menu changes. Gadget records
remain player+354+4ID flags and +356+4ID charge. Types/capacities are original live
800c2710 table values. The menu895fc..8963c computes amount*100/live capacity;
this percentage is now used on tiles rather than raw timer units. Pending
activation remains visible; active empty toggles remain selectable; depleted
selection falls back to the first eligible gadget and empty U is harmless.

Original gadget rendering at89798..898a8 uses names and a selection arrow. Named
MED/JET/BIO/NV tiles are the documented fallback: original TTK identity and Duke
font, no mismatched borrowed icons. Two-second bottom strip above the health/ammo
HUD has an arrow, selected name, percentage and active marker. Original-menu,
capture/focus/scene invalidation hides it. The surface is independent of cheat
quotes and preserves each renderer's transforms. Patch:
`time-to-kill-zzzzzz-inventory-strip.patch`.

An actual menu-entry failure was also reproduced: captured I/RightShift cleared
Select at the first host sampling; F10 release then RightShift opened the menu.
A six-input-frame Select pulse survives that specific capture release and is cleared on
other resets/focus loss. It changes no original menu animation/repeat timings.
Initial raw-charge screenshots and the first premature U test are excluded from
acceptance. Native selection checks pass zero/one/many, both-direction wrapping,
byte-identical cycle-only inventory, menu ID synchronization, pending/active and
depletion rules. Renderer tests cover transparency/arrow/text/size, expiry and
capture/menu/epoch/mode invalidation. Final live presentation/menu/U review ongoing.

### Verified inventory follow-up

`iteration42-inventory-percent-{gl,software}/inventory-verify.json`: actual captured
I entry, retained medkit5, original-menu browse to goggles3 and host synchronization,
direct biomask2 then settled U off, cycle-only byte immutability, rapid presses,
held-repeat suppression, capture/focus/timeout all pass. Software uses custom
O/P/L instead of brackets/U. An earlier U test during pending activation was
correctly refused and excluded. Both renderer scripts completed without failures.

Final typography uses tile numbers matching the original menu, with explicit
percentage in the selected full-name line; this avoids wrapping a percent symbol
in a narrow tile. Final320/640/1024 captures in `iteration42-inventory-layout-{gl,
software}` were reviewed, including a97% active biomask, gold marker, alpha,
selection arrow and original HUD spacing. The final extra negative-x clipping
guard affects only unusually narrow layouts; native coverage includes240-pixel
available width as well as the three requested window sizes.


### Furniture regression

Natural apartment route on current traversal code in `iteration42-inventory-software`:
contact bed entry, three walk/run departure-and-return jumps and side entry pass.
Sampled departures use walking/running and short-fall108, without stationary105
recovery during held movement. Two couch contact jumps and walk/run departures
also pass. The original helper's third jump started short of contact and was
excluded as a release test. A corrected route in `couch-release.json` reached the
couch, released input during actual108/state9 at(11376,-12063,9807), then settled
normally to63/state0. No position fixtures or original asset writes.

### Additional timing and stow findings

Host-turbo run-start replay on the final traversal path passed18/18 combinations
(`iteration42-inventory-layout-gl/street-turbo-results.json`). Fresh run launches
were sampled2–4 guest frames after press; standing/walking still prepare normally.
This varies host pacing, not the game's physics tick or global movement speed.

A subsequent shotgun test found a second transition in the E-owned stow chain.
`iteration42-inventory-software/shotgun-transfer.json`: upper20 ->21 ->1, equipment
already0 byX4959, held reach present on pad49135, yet upper1 remained until contact
and bounce107. Original557f8 checks upper-table bit8 at558a0; generic blend0/1 both
have descriptor2009 and therefore still block the reach. The earlier pistol6 path
finished its generic blend quickly enough. The initial equipment-clear-only
budget was insufficient for this longer two-handed transition.

Refinement keeps the bounded E-owned airborne lease through generic blend0/1 after
equipment0, still processing original events once per animation update. It does
not overwrite a pose, clear the original eligibility bit or alter ladder contact.
Native tests cover both tails and focus/deadline cancellation. At this intermediate stage, live replay was still pending; the following paragraphs
record its outcome. Native fixtures alone were not accepted as gameplay proof.

The tail candidate then passed a fresh armed shotgun first-ladder route, second
transfer and safe exit (`iteration42-shotgun-tail/shotgun-transfer.json`). E was
received atX5671; original upper21->1 completed before148/state3. Exit restored
shotgun5/upper20/equipment2; ammo stayed50. Screenshots of actual attachment and
redrawn shotgun were reviewed. Pistol Software replay with the same tail change
also caught the second ladder. These are two materially different stow tracks;
native fixtures cover the remaining transition families, not live campaign proof.

One final weapon-switch acceptance probe reproduced an intent race on unchanged
switch gating: while holding E during shotgun->pistol, the original switch passed
through equipment0, consumed E, then drew the pistol and stayed armed. Evidence:
`iteration42-shotgun-tail/transition-intent.json` (upper21/flags4 ->1/equipment0/
flags4 ->0/equipment0 ->6/equipment2 ->5/equipment2). The request must not treat
that temporary equipment0 as completed interaction stow.

The final candidate requires original stable armed idle/no switch flag4 before
spending the holster pulse, and rejects flag4/upper-table bit8 before spending E
on interaction. Its bounded pending lease can therefore wait through a manual
switch; the intended new weapon and all original events remain authoritative.
Native actual-gate and pending-intent tests pass. Final live results follow.


### Final ladder/transition results

Final stable-switch candidate: `iteration42-shotgun-final` passes first ladder,
second transfer, safe redraw and unchanged50 shotgun ammo. In Software,
`iteration42-transition-final/transition-intent.json` now ends held E holstered
(upper63/equipment0); release restores intended pistol4. A deliberate weapon
switch exactly when pressing E at the first ladder attaches190/state8. The fixed
route's platform-height assertion was too early because the switch takes time;
continuing original W climbing reaches normal platform movement. This timing
assertion is not classified as a missed grab. See
`first-platform-transition-completion.json`. Subsequent late pistol transfer
attaches186/state3, then safe exit returns63/equipment2/pistol4 without manual C.

All movement occurred through original controls, animation events and collision.
No original position fixture, remote attachment, forced shot or broad animation
speedup was introduced. Pistol/shotgun live routes plus native other-track budgets
are bounded first-map evidence; broader weapons/eras and human feel remain playtest.

## Final verification and limits

Player binary SHA-256:
`f89f1ca92d5a7e0445022d98e6d9925bf893249bd9fa5bd410c7eec99e3c3bcc`.
Normal import/regeneration/ordered-patch/build/movie-overlay workflow passed
(`analysis/iteration42/full-build-transition-final.log`). Six native suites pass:
input, guarded controls, aiming, scene lifetime, font and inventory rasterizer.
Python suite: 60 tests, 2 conditional skips. Ordered runtime stack verifies installed.
The private Vanilla route passed 14 checkpoints and exited 0; inventory and turn
captures reviewed (`analysis/vanilla-regression/iteration42-final`).

All private games closed. Eleven original player configuration/card/state hashes
match the entry checkpoint; original media/font assets were not edited. Source and
binary hashes and test-log pointers are in `analysis/iteration42/verification.json`.
Existing unrelated dirty/untracked work was preserved; no commits were made.

D08G2/D08I/D08J/D08A1 are **Needs playtest** for user acceptance; D08K is **Blocked**
with the concrete audit in43. D19B stays deferred. Evidence is bounded to supported
first-map controls and inspected original paths. It does not establish every
weapon/era/terrain/FPS/campaign case. Large-fall exits retained original108/105,
with no edge-grace launches; native tests additionally reject fresh edge requests
on large falls. No new all-campaign or all-weapon claim is made.

Please test quick Shift+W→jump and late bed/couch edge presses, armed E on the two
exterior ladders (including during weapon switching), bracket cycling/U and I-menu
selection, and silent typed cheats with centered results. Recheck accepted aiming
and furniture feel. Crouch+direction still rolls; there is no low-walk prototype.
Launch with saved preferences:
`python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture.
