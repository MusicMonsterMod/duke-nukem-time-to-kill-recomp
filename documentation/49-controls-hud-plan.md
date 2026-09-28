# Next controls/HUD plan — 2026-09-28

Documentation-only. No runtime edits in this checkpoint. Player may have a game open.

Wiki reference (quotes): [EDuke32 Pre-defined values](https://wiki.eduke32.com/wiki/Pre-defined_values).

## Closed this playtest

| ID | Result |
| --- | --- |
| **D08J** | **Done** — armed run-jump+E to second ladder accepted (4× successful attempts). |
| **D08A1** | Already Done (tiles strip); presentation follow-up below is a new job. |
| **D18A** | Already Done (no crackle). |
| **F key** | Confirmed: no longer toggles FPS. |

## Selected next work (recommended order)

### 1. D27 — Caps Lock RUN MODE quotes + Shift clunk silence

**Caps Lock:** when toggling autorun, show centered host quotes (same path as cheat confirmations):

- `RUN MODE ON` (wiki quote 86)
- `RUN MODE OFF` (wiki quote 85 — **no colon**)

**Shift:** holding Left Shift to run currently plays an unwanted “clunk.” Reproduce, identify source (original gait/footstep vs host), and stop that sound for Modernized speed-modifier hold without muting legitimate footsteps or Vanilla.

**Acceptance:** Caps Lock shows the two quotes once per toggle; Shift-run has no clunk; walk/run feel and Vanilla preserved.

### 2. D28 — Scroll Lock holster + WEAPON LOWERED / RAISED

Default holster binding **C → Scroll Lock** (Duke3D-style). On deliberate holster key use:

- Holster → quote 73 `"WEAPON LOWERED"`
- Draw again → quote 74 `"WEAPON RAISED"`

Use the same centered Duke-font message path as cheats. Keep hard-coded `C` fallback out of the Modernized path once Scroll Lock is default (preserve custom rebinds). Update manual.

**Scroll Lock vs FPS conflict:** you also asked Scroll Lock for FPS. **This plan assigns Scroll Lock to holster.** FPS stays unbound by default until **D26** (backtick `fps`) or you pick another key (e.g. Pause). Do not put both on Scroll Lock.

**Longer-term note (not this job):** holster should become optional/redundant for normal play (E already auto-stows); keep the key for screenshots / explicit lower.

**Acceptance:** Scroll Lock draw/holster; correct quotes; E auto-stow/ladder unchanged; F still not FPS.

### 3. D08A2 — EDuke32-style active inventory icon (bottom-left)

Replace (or supplement) the full bracket strip-on-use feel so **using/selecting** shows the **currently selected item only**, bottom-left, like EDuke32:

- Icon bottom-left; green %-digits from Duke3D tiles **3010–3021** at bottom-right of the icon; `%` included.
- **Jetpack / night vision / scuba (biomask) / boots:** status word above the % — `ON` / `OFF` / **`Auto`** (scuba & boots use **Auto**, not “Otto”).
- **Medkit / steroids:** percentage only (no ON/OFF/Auto row).
- HoloDuke not in TTK — skip.
- Prefer owned Duke3D ART/GRP assets with provenance; fall back to current Duke font if tiles unavailable. Brackets still cycle; Enter/U still use.

**Acceptance:** visual match to EDuke32 layout for owned TTK gadgets; both renderers; Vanilla untouched; strip-on-cycle policy documented (full strip vs single icon).

### 4. D26 — Backtick debug console (includes FPS)

Unchanged intent: backtick console with at least `fps` / `help`. Preferred home for FPS now that F is free and Scroll Lock is holster. Optional later: expose FPS in modern settings (D19/D21).

## Deferred / unchanged

- **D08L** edge run-off inertia, **D11A** scroll-lock FP, **D18B** voice+music — backlog.
- **D08K** Blocked; **D19B** deferred.
- Full USER.CON quote catalog — only LOWERED/RAISED + RUN MODE for now; widen later from the wiki page.

## Preserve

Accepted aiming, armed ladder transfers, silent empty inventory start, tiles strip until D08A2 ships, Vanilla, U/custom bindings, ordinary Enter pause outside inventory, furniture/edge grace, silent cheats.

## Build / launch (unchanged until next implementation)

Current player binary from iteration 48:
`42eeaf3dc31a7f7a94ae9201e0c47f32cb23c1680edcfca994acb5cb20eccbab`.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```
