# D08 — traversal and control ownership

D08’s current controls iteration is **Done on explicit user acceptance**: every
tested ladder transfer worked as expected and E grabbing delivered the requested
fluidity. The remaining coverage items in this matrix now belong to **D08B**.
The bounded standing/walking directional-jump and both-input-orders iteration is
also **Done as D08C**, on explicit user acceptance (2026-09-27).
[Accepted contract and evidence](34-standing-directional-jumps.md). Weapon/item
shortcuts are implemented separately under **D08A**, with remaining playtest gaps.

The authorized movement follow-up implements both running-jump variants, held
input through clear landings, and directional/action handoff during authenticated
attached traversal. See [reproductions and evidence](26-d08-movement-followup.md) and
[interaction/respawn polish](27-d08-control-polish.md).
This matrix is bounded first-map evidence; progression, security-card interactions,
swimming and scripted cameras remain required work.

| State / transition | Current owner | Evidence | Broader D08B coverage / next check |
| --- | --- | --- | --- |
| Normal idle / walk / run, LEVEL00, state 0/0 | Modern movement/orbit; original root motion and collision | Accepted D05/D06; D07 continuation speed transition replay | Slopes, steps and long strafing/backpedal routes |
| Wall bump 94/95, state 0/0 | Modern orbit; original movement response | D07A entrance-wall route | Club interior/corners, held movement and escape |
| Running takeoff 9/0 -> airborne 103/104, 9/9, state 9/9 | One-time horizontal takeoff direction; original ballistics/collision; leased modern normal camera | D08 six-jump replay verifies left/right/forward/back direction; both variants covered by native checks | Diagonal jumps, slopes, ceilings, capture/focus midair |
| Running landing | Original landing selector; held movement request survives the owned jump | User confirms all-direction continuity; narrow 0/9 landing seam now covered; impacts 107/108 retain recovery 105 | Long play, slopes/ledges, speed changes during flight |
| Running reach 109, state 9/9 | Modern input/camera lease continues the owned running jump; original reach/collision owns attachment | Reproduced second-ladder failure when Cross disappeared; delivery holds reach after E release, catches ladder and climbs to upper platform with automatic redraw | Other ladders, oblique approaches and interrupted transfers |
| Standing/walking directional request, 63 / 72–75 | Guarded buffered request starts original preparation 96; running and active jetpack paths remain original | User confirms W→Space; native and real SDL short-run-up cases pass | Other surfaces/maps and combined state transitions; input-order feel accepted |
| Neutral/directional preparation 96, state 0/0 | Modern camera/input lease; no synthetic Forward until first direction; original animation and clearance | User explicitly accepts Space→W; six real SDL Space→direction cases and native late-input/release/focus checks | Near-wall/headroom variations, moving supports; no animation restart |
| Standing directional flight 98, state 9/0 → 9/9 | Once-only horizontal redirection; original ballistics/collision and owned camera/input/E reach | Directional/diagonal SDL cases and once-only native checks; D08C accepted | Slopes, ceilings, combined standing-jump ladder approaches and other maps |
| Vertical flight 97 and other unsupported jump animations | Original motion/state; no midair directional promotion | Space-alone controls remain 97; native airborne rejection and Vanilla vertical-jump capture | Other jump families separately; full airborne steering not implemented |
| Draw/holster / numbered request | Original equipment animation; guarded selected-slot request | Natural pistol holster in prior facing route; new number selection fixture | Natural acquisition and all weapon/era variants |
| E interaction request | Temporary original holster, unarmed Cross, automatic redraw after normal ownership; never armed Cross | Armed ladder mount/climb/exit/redraw and apartment E-only pass; manual holster preserved; pause/death/cancellation guards | More objects and weapon/era variants; actual cash register auto-holster route |
| Ledge grab / pull-up 139/149/140, states 8/6 | Original movement/facing/camera; holstered W/S/A/D/E input handoff | Car-park pillar reproduced stuck 149 with old build; current W/E reaches top and normal state | Other walls, shimmy, release, vault variants |
| Ladder mount / climb / exit 185/186/188/189/190, states 8/3 | Original motion/facing/camera; directional/action handoff only while attached | Subway-alley ladder ascends to platform and returns to normal standing | [Second-ladder transfer verified](31-ladder-transfer.md); descent, other ladders/maps |
| Crouch / quick-turn / roll | Held Ctrl requests original crouch with faster transition and clearance-checked release; other states retain original ownership | Bounded crouch SDL/native evidence in [controls note](33-controls-shortcuts.md); no blanket terrain/roll acceptance | Low ceilings, moving supports and combined jump/crouch transitions; preserve accepted jumps |
| Swimming / underwater | Modernized adapter (D08M Done; **D08O next**) | Partial | Ledge/shallow Done; A/D+Ctrl↓+mantle-only exit → D08O. [55](55-swim-controls-research.md) |
| Inventory / pause / focus / capture | Automatic verified gameplay capture; Escape/Enter Start, F7 savestate recapture and explicit F10 opt-out; menu release | Automatic entry and pause resume replay; native F7 recapture and uneaten offer; existing manual focus suite | Longer menu/overlay routes and traversal transitions |
| Script / cutscene / interaction camera | Original camera; captured WASD falls back to tank W/S + L2/R2 strafe (D08P) | Guarded supported normal-camera callers; tank fallback when lease inactive; F7 slot-2 recapture | Confirm return lease and mouse look after crystal-2 turret |
| Death / respawn / level change | Original death/restart; independent requested boom retained; dead ownership rejected | Two enemy death/restart replays recover camera distance without F10; private health fixture | Other respawn locations, map load and saved-game recovery |
| Security card / next area | Original interaction | User's prior successful progression retest | Repeat isolated route with new controls; earlier freeze cause remains unproven |

Tests must use private memory cards/profiles and refuse an existing player game.
Before adding a state, verify actor/caller/overlay identity, original input use,
movement-vector production, collision-query mode and camera ownership. A camera
whitelist change alone is insufficient traversal compatibility. Preserve ordinary
Vanilla, original camera/aim, and a safe uncaptured arrow/X/C/Z fallback.

The original research snapshot fields and `continuation_probe.py` provide the
starting data capture. Extend them with state-specific assertions and reviewed
composed images; a numeric route alone cannot certify animations or progression.

## D08B follow-up — 2026-09-27

Native suspension fixtures reproduce and fix a missing gameplay-context gate in
attached traversal input. Frontend, inventory/pause and suspension now reject
WASD/E eligibility even when attached state bytes remain. Recovery restores the
existing gate. This is source/native evidence only; player rebuild and actual
ladder interruption/descent/progression replays await closure of the active player
session. See [follow-up](35-traversal-and-apartment-followup.md).

### Final D08B bounded results — 2026-09-27

- Native fixtures verify no attached input in suspended contexts; E-owned redraw
  intent survives pause without granting draw/input permission there.
- Final private SDL replay requires automatic capture (F10 fallback disabled),
  resumes attached climbing after pause, descends the first alley ladder and
  exits to normal standing with pistol redrawn and ammo 200. Captures reviewed.
- A later re-ascent stops in original animation 186 despite W/Up reaching SIO.
  The upper landing is visibly occupied by a pig cop; exact obstruction cause
  remains unproven. Do not count that as a successful upper exit.
- The apartment script replay preserves original switch animation, bed movement,
  one-time secret count, subsequent dialogue and natural five-pipe-bomb pickup.
  It uses an explicit conversation-bit reset fixture; untouched switch-first
  access is still a user playtest, not recorded natural progression.

[Focused implementation and limits](35-traversal-and-apartment-followup.md),
[structured evidence](reports/d08b-d08d-delivery.json). Security-card, swimming,
other maps and broader transition criteria remain open; D08B is In progress.

## 2026-09-27 furniture and weapon follow-up

User confirms ladder-exit redraw and untouched apartment lights-first route.
D08E adds settled-ground bound-fire draw and 1.5x equip/stow upper tracks; pistol
hold/release checks and Vanilla pass. D08B furniture remains open: close-contact
bed jump selects vertical97, stepping back permits98, walking stops at the edge,
Shift-run leaves it. Couch remains user evidence. The enemy-health zero fixture
left actors live; it does not prove the upper-landing stall cause. See
[focused contract](36-weapon-response-and-furniture.md).
