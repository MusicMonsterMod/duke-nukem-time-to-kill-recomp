# Playtest feedback and next implementation plan — 2026-09-27

**Implementation update:** this planning checkpoint is historical. The five selected
jobs were handled in the following pass; use [current status](00-status.md),
[handoff](42-traversal-feedback-progress.md) and [evidence](44-traversal-inventory-implementation.md).
Latest user acceptance and the newly selected turning/audio/bed/Enter follow-ups
are in [the current brief](45-playtest-turning-audio-follow-up.md); true crouch
walking retains its documented asset/collision blocker. The continuation prompt below records the original scope.


Planning-only checkpoint after the player's live report, transcribed by
**speech-to-text**. No game was launched or controlled, no runtime code was changed,
and no build, player profile or memory-card write was performed. Current binary
remains `b382456bc0c02936bf780deda9e5bfacaeb3b965199ad538ea11ffc7c6336f69`.
The canonical scopes and acceptance criteria are in [the job board](../MODERNIZATION_JOBS.md).

## Accepted results and evidence limits

- **D07C Done for the bounded user-tested scope.** Aiming is “massively improved.”
  RPG now shoots where aimed, follows the modern crosshair, and the weapons the
  user tried align correctly through their tested firing/aim modes. Do not infer
  exhaustive weapon/era, assistance-setting or full-campaign human coverage.
- **D08H furniture movement accepted.** Standing right against the bed and jumping
  onto it was “perfect”; walking and running off no longer pause Duke. Couch entry
  is much easier. Preserve these behaviors as explicit regression requirements.
- **D08H remains Needs playtest for its other acceptance points.** This report did
  not state a fresh concealed/exposed pipe-bomb test or switch-before-NPC result.
  Earlier isolated technical evidence remains valid; do not invent human sign-off.
- D08G1 wording and D19A font implementation stay Done. New entry/placement requests
  are the focused D08G2 follow-up. D08A retains its existing broader playtest limits.

## New reports, separated from hypotheses

| Report / request | Planned job | Priority |
| --- | --- | --- |
| Cheat letters appear while typing; show only completed results, centered like Duke3D | D08G2 | Small first change |
| Shift+W then Space almost immediately sometimes does not jump | D08I | High |
| Jump near a bed/platform edge sometimes does not register | D08I, separate reproduction | High |
| Run-jump with pistol drawn, then E fails to catch second exterior ladder | D08J | High |
| Holstering should be automatic for first ladder and airborne transfers | D08J | High |
| Crouch+movement rolls; user wants slow crouched walking, potentially new animation | D08K | Near-term investigation/prototype |
| Brackets should select inventory with EDuke32-style visual feedback | D08A1 | Near-term |
| Menus feel clunky, laggy and slow | D19B, integrated with D19 | Deferred backlog |

Do not conflate run-start timing with edge support or armed ladder eligibility.
No live reproduction was attempted during this planning pass. The user says they
are quitting; any next session must still detect and avoid an active game.

## Read-only source findings

Reference checkout: `research/eduke32`, official upstream
`https://voidpoint.io/terminx/eduke32.git`, pinned local revision
`ec5824db81817866f70da326d3811bb0f52b3517`.
Online cross-check: the official [EDukeWiki input definitions](https://wiki.eduke32.com/wiki/Pre-defined_values)
identify Inventory, Inventory_Left and Inventory_Right as distinct actions;
[Inven icon](https://wiki.eduke32.com/wiki/Inven_icon) documents the selected HUD item.
The concrete behavior below comes from the inspected pinned source, not from an
assumption that every EDuke32 HUD configuration looks identical.

### Bracket selection and presentation

- `source/duke3d/src/_functio.h:299` and `:366`: modern/classic default tables bind
  `[` and `]` to Inventory_Left / Inventory_Right.
- `sector.cpp:2778`: cycling starts a two-second `invdisptime`, steps left/right
  with wraparound, skips zero-amount entries, changes `inven_icon` and sends the
  selected item's name through `P_DoQuote`. Selection is distinct from activation.
  Default order: first aid, steroids, holoduke, jetpack, night vision, scuba, boots.
  Script events may override selection; those are not required for TTK.
- `sbar.cpp:393`, `G_DrawInventory`: draws only owned/remaining items as a strip
  near the lower screen/HUD and an arrow indicating selection. Layout shifts with
  mini/full/alternate HUD settings; it is not a universal fixed coordinate.
- `screens.cpp:1151` draws that strip while the timer is positive;
  `player.cpp:3517` decrements it.
- Our `recomp/src/ttk/shortcuts.inc:104` already cycles eligible TTK slots
  `{5,1,2,3}` = medkit, jetpack, biomask, goggles. `item_use` invokes the selected
  original item path, and direct shortcuts update the host selection. The cycling
  branch itself only updates `selected_item`; it publishes no selection feedback.
  See [existing mapping and evidence](33-controls-shortcuts.md).

Plan: verify original TTK inventory-menu/HUD synchronization and available item art,
then add the temporary strip, selection marker and name. Keep actual TTK gadgets
and charges; do not draw a Duke3D holoduke/scuba icon for an unrelated item or imply
stored steroids (TTK activates steroids on pickup). Validate selection changes,
use, depletion, direct shortcuts and original-menu changes together. Existing
brackets are not proof of discoverable or accepted player-facing behavior.

### Cheat entry and placement

`recomp/src/ttk/pc_input.cpp:181` explicitly publishes the partial cheat buffer or
“Cheat cancelled” through `input_notice`; this is a concrete source explanation
for the reported typing echo. Remove that presentation in D08G2, preserving the
parser and input/reset guards. Valid completed commands keep exactly one result;
incomplete/invalid/cancelled/expired text stays silent. A completed but refused
request may retain truthful failure feedback; never claim an effect happened.

EDuke32 `source/duke3d/src/text.cpp:458`, `G_PrintGameQuotes`, draws ordinary quote
text at x=160 in its 320-wide logical coordinates using `TEXT_XCENTER`; y is the
upper quote base plus configured offset. Reserved quotes have special placement.
Thus “middle of the screen” is interpreted here as **horizontal centering in the
upper message area**, consistent with the user's Duke3D reference, not vertical
centering over the crosshair. Make placement easy to adjust if the player meant
literal screen center. Verify composed output in both renderers and three sizes;
do not relocate volume or unrelated OSD panels.

### Jump, ladder and crouch leads — not proven causes

- `pc_input.cpp` already buffers jump for eight input sequences and offers
  `input_take_jump()`. `modern_controls.cpp` has guarded walking/idle takeoff
  handling and preserves original running selection. A transition/consume gap is
  plausible, but frame-by-frame input/support/animation evidence must establish it.
- For edges, distinguish a valid grounded press dropped by gating from a press
  after support is lost. A short explicit grace window may be appropriate for the
  desired feel, but is not a diagnosis or a license for double/infinite jumps.
- Armed ladder misses need original contact, E lifetime and equip/stow traces.
  Existing accepted holstered transfers and automatic interaction stow are not
  proof that an airborne pistol-drawn approach is covered.
- `crouch.inc` labels animation 176 enter/reverse, 178 idle and 181–184 movement.
  Those labels do not establish crouch-walk clips: inspect actual animation,
  skeleton, collision and original roll handlers. No new skeletal animation or
  asset pipeline has yet been proven necessary or available.

## Proposed next pass

1. **D08G2:** remove entry chatter and center completed results; verify parser/input
   regressions and both renderers.
2. **D08I:** reproduce run-start and edge misses independently, implement bounded
   responsiveness fixes, measure input-to-takeoff and replay accepted furniture.
3. **D08J:** reproduce armed first/second-ladder cases, make required weapon
   transitions automatic through original events, replay accepted ladder routes.
4. **D08K:** audit movement/animation assets and build a safe bounded crouch-walk
   prototype if feasible. Record precise limitations or a concrete asset-pipeline
   blocker; do not silently replace it with faster rolling.
5. **D08A1:** verify TTK selection ownership and add the EDuke32-style visible
   inventory selection, retaining existing bindings and original item semantics.

**D19B stays deferred** with modern menu design. Record baseline input-to-visible
response before changing delays/repeat/animations. Do not globally accelerate the
game to make menus feel faster. Broader D19 menu construction is not implicitly
selected by the next pass.

All five next-pass jobs preserve Vanilla, accepted controls/aiming/furniture,
original assets, player saves/settings and isolated test discipline. Reproduce
reported gameplay misses before claiming a cause. Use normal regeneration and the
reviewed runtime patch stack. Update the board, status, handoff, manual and evidence
with truthful Done / Needs playtest / concrete blocked outcomes. User feel remains
human acceptance, not something a native fixture can establish.

## Continuation prompt

```text
$continue-duke-recomp

Read MODERNIZATION_JOBS.md, documentation/00-status.md,
documentation/09-handoff.md and documentation/41-playtest-follow-up-plan.md
in /home/spartacus/Desktop/dn-ttk.

Work autonomously in this order: D08G2, D08I, D08J, D08K, D08A1.
These jobs are selected; do not ask me to choose again. D19B is backlog only
and stays deferred with the modern menu redesign.

Use Duke3D/EDuke32 as the interaction reference and follow the brief's
source findings, exact feedback and acceptance criteria. Reproduce quick
run-start/edge jump misses and armed ladder-grab failures before claiming
a cause. For D08K, inspect the original animation/collision assets and
implement a bounded crouch-walk prototype if feasible; document any concrete
blocker rather than pretending rolling is crouch walking. Bracket cycling
already exists: verify it and add the planned selection feedback.

Preserve Vanilla, user-accepted aiming and bed/couch movement, existing
bindings, original assets and my saves/settings. Use isolated tests and
avoid interfering with an active game. Complete and verify the scoped
work, update the documentation, and finish with what I should test and
the launch command. Do not start the deferred menu redesign.
```

Current launch (unchanged):
`python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture.
