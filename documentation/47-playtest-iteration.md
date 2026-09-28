# Playtest acceptance and iteration 47 — 2026-09-27

## Player feedback (this session)

Accepted without reopening:

- Aiming feel remains strongly improved.
- Inventory **Enter** activation works; player is happy with the interaction.
- Close bed on/off jumps are fully accepted (“superb fix”).
- Rapid turning / Shift-running no longer felt wrong on this pass.
- No audible crackle during strip-club play on this pass.

Requested presentation changes (implemented this iteration):

- Do not show a “No inventory” startup message; empty inventory stays silent.
- Inventory strip should be tiles-only (Duke3D-style switcher), without long name /
  “Enter or U to use” guidance text.

Reported regression:

- Jumping to the second exterior ladder with the gun drawn failed; holstered
  succeeded immediately. Isolated reproduction found ordinary armed (no aim)
  transfers still attached; **holding right-aim across takeoff** blocked
  `flight_valid`, so E-owned airborne stow never ran and the grab missed.

Requested backlog (added, not started):

- Mouse-wheel zoom that locks into first-person (Fallout/Skyrim style), building
  on existing third-person distance.
- More inertial platform edge run-off (Duke3D-like; reduce artificial slowdown).
- Voice clips must not mute music; expose master / music / SFX volume channels
  for the future menu system (EDuke32-like).

## Implementation

### D08A1 inventory strip

`inventory_hud.cpp`: empty announce no longer opens the strip; empty render
returns no image; removed name/use-hint rows; height 60 tile strip only.
Native inventory test updated and passing.

### D08J aim-held airborne stow

`modern_controls.cpp`: establish the flight lease even when Mouse2/`original_aim`
is still held after takeoff; keep original aimed jump arc (no ballistic redirect
while aim is held). This restores mid-air E stow during aimed armed transfers.

Evidence:

- Pre-fix: `iteration47-ladder-aimbefore` — aim-before-jump failed; equip stayed
  2 through flight; no 148 attach.
- Post-fix: `iteration47-ladder-aimfix` — mid-air stow by transfer-19, attach
  148, settled 186/[3,3]. Summary:
  `recomp/analysis/pc-input/iteration47-ladder-aimfix/aim-before-jump-summary.json`.
- Nominal armed (no aim) still attached earlier:
  `iteration47-ladder-compare2/ladder-summary.json`.

Native: controls, inventory, and input suites passed on the delivery binary.

## Status decisions

| Job | Status | Reason |
| --- | --- | --- |
| D08I | Done | Explicit bed acceptance |
| D10A | Done | User no longer reproduces; isolated sweeps never lost yaw |
| D08A1 | Needs playtest | Enter accepted; silent empty + tiles-only strip needs confirmation |
| D08J | Needs playtest | Aim-held stow fix verified privately; player confirm armed/aim ladder |
| D18A | In progress | User heard no crackle this session; club starvation unrepaired; do not claim fixed |
| D08K | Blocked | Unchanged |
| D19B | Deferred | Unchanged |

New backlog IDs: **D08L**, **D11A**, **D18B** (see board). D21 absorbs menu volume
channel exposure when menus arrive.

## Build / preservation

Binary SHA256:
`57380caedddaf28621378dcc241886a234bff85dc1dd97882f6c6d50e55365cf`.

Player settings/cards unchanged vs iteration47 preservation snapshot. No commits.
Original assets untouched. Private test games closed; dummy audio used for ladder
routes (no Pulse sink required).

## What to test

1. Fresh start: no “No inventory” toast; strip appears only when you own gadgets.
2. Medkit/brackets: tiles + charge + arrow only; Enter still uses; U still works;
   Enter outside the strip still pauses.
3. Exterior platforms: pistol drawn, optionally holding right-aim, run-jump+E to
   the second ladder; also confirm holstered still works.
4. Optional listen pass in the club for crackle (D18A still open).

Launch:

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

Modernized captures automatically; F10 toggles capture.
