# D08Q2 - jetpack pending flag after Continue

2026-10-04. **Done, user-accepted:** "confirmed fixed! accepted."
The reported state and a controlled fresh interrupted-deployment/death/Continue
sequence are repaired. Acceptance closes D08Q2; a natural map-pickup replay was
not performed in the automated work. The separate jetpack aiming/missing-crosshair
report is [D08Q3](../MODERNIZATION_JOBS.md#d08q3---jetpack-weapon-aiming-and-missing-crosshair),
queued next and not started. Accepted implementation: `2f02967`.

## Cause and owned-game evidence

The preserved UI slot 10 (file 09) has jetpack flags `player+0x358 = 0x8001`,
full fuel `+0x35a = 9000`, closed model phase `+0x84c = 0`, item owner
`+0x884 = 1`, and no open/close transition in `+0x224`. Duke is alive in
normal gameplay. The pending bit `0x8000` never clears on the old build.
Both `usable_item()` and the shortcut's pending-item exclusion reject it.
The picker skips the pack, and J cannot activate it.

Read-only decoding of the owned executable establishes:

- `800402d0` starts the requested item operation. Jetpack deployment sets
  `+0x224 & 0x1000`; closing uses `0x10000000` through `8003ff6c`.
- `8004001c` advances the original model transition. At its closed/open
  endpoints, `8004010c..80040130` clears pending and both transition flags.
- The reset routine `8003f948` zeros model phase at `8003fcf4`, then clears
  the jetpack's active bit at `8003fd28..8003fd2c` while preserving pending.
- The original inventory grant `8003d738` ORs ownership and refills charge;
  it preserves pending. Repeating `dnstuff` therefore cannot repair this state.

A private live test used real SDL J/RPG input, an explicit one-health fixture,
and the original explosion/death/Continue menus. Extending the existing equip
countdown to 10000 made death during deployment repeatable. This is a timing
fixture, not evidence of the precise action/timing in the user's original death.
Before death: `0x8003`, phase 1/2, deployment running. Death stops the transition;
Continue returns phase 0 and flags `0x8001`. `dnstuff` retains `0x8001` and J
remains blocked on the baseline. A separate completed-deployment death/Continue
control returns normal `0x1` and stays usable without the fix.

## Bounded repair

`shortcuts.inc`, at the existing authenticated selection hook, clears only
jetpack pending when all these conditions hold:

- Active Modernized gameplay, original actor/camera/overlay/code guards and
  normal movement lease; primary player, alive, current/previous mode 0.
- Jetpack owned, off, pending; model fully closed, item transition owner 1.
- No open/close transition or item/equipment request flags.

Fuel, ownership, other inventory flags and animation state are untouched.
Recovery runs before picker eligibility, so existing affected savestates work
without entering another cheat. The original completion routine's 288 bytes
are added to the code identity guards. No new hook, generated C, framework,
renderer, cheat grant or flight-physics change.

## Verification

Private evidence: `recomp/analysis/d08q2-jetpack-20261004/`, port 9342,
private profile/cards, Xvfb display 94 for real SDL keys/mouse. No player launch.

| Check | Result |
| --- | --- |
| Reported slot 10, both flight schemes | `0x8001 -> 0x1`, fuel stays 9000; picker selects item 1, J opens/closes normally |
| Fresh delayed-deployment/RPG death/Continue/`dnstuff` | Baseline remains `0x8001`; candidate returns `0x1`, then J reaches `0x3`/phase 5 |
| Modern flight | Space ascent, hover, W/A movement, Ctrl descent and mouse; counters include 54 flight updates, 4 hover engagements and 7 descent updates |
| Classic flight | Original burst/gravity path with modern movement/mouse; classic counter advances, host hover/descent counters stay zero |
| Off and private save/load | Closed/off record and remaining fuel preserved: 8205 Modern, 8475 Classic in these routes |
| Empty fuel | J refuses activation; picker skips the pack |
| Depletion fixture | Original flight drains 80 fuel to 0 and returns Duke to ground, pack off |
| First-person slot 10, 120 target | Repair succeeds; eye view active, supported and fully blended |
| Vanilla slot 10 | No host repair: original `0x8001`/9000 record retained |
| Native controls suite | Repair, picker/J, other-bit preservation, empty fuel, transition endpoints, active/missing items, death, capture, Vanilla and changed-code rejection pass |
| Input, aiming, inventory HUD suites | Pass |
| Player build | Incremental local-dev build and movie/math shard checks pass |
| Player cards and intake copy | All 28 files retain intake SHA-256 and mtime values |

Build SHA-256:
`c8f979d50bd502ded05294fde2e55b59c90f132f386d21a89865538ca8836a41`.

The live tests are functional checks, not new rendering/performance or audio
claims. Ordinary owned/off/on records are unchanged in native tests; a natural
map jetpack pickup was not replayed. Existing acquisition/grant code is unchanged.
The user's original timing is unknown, although the stranded state and a
concrete original reset path producing it are established.

## Player check

Launch the saved profile, load UI slot 10, and test bracket inventory selection,
J on/off, Space takeoff and the chosen Modern/Classic flight controls. F10 captures
Modernized controls if needed. No repeat `dnstuff` should be necessary. Include a
normal jetpack pickup when convenient. The user has now accepted D08Q2; this
route remains useful for regression checks.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
