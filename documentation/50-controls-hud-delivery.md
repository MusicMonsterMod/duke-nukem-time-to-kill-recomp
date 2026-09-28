# Controls/HUD delivery — D27 → D28 → D08A2 → D26

2026-09-28. Implements [49-controls-hud-plan.md](49-controls-hud-plan.md).

## Build

SHA256 `57c2b097dd98e28473fc644a20d9d3a2730deb32ecbca0db1d0465e8564646ea`.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

Keep `ttk-inv-digits.pack` beside the executable (CMake copies it into `build-local/`).

## D27 — RUN MODE + Shift clunk

- Caps Lock → centered `RUN MODE ON` / `RUN MODE OFF` via `input_notice`.
- Walk↔run at `0x80048410` soft-switches anim pairs **without** zeroing `+0x68/+0x6a`,
  so the original plant/start SFX no longer fires on Shift-run (Modernized only).
- Step-drop dispatch still restarts from idle as before.

## D28 — Scroll Lock holster

- Default `holster=ScrollLock` (scancode 71). Profile schema **v8** migrates old
  default `C` → `ScrollLock`; custom holster binds preserved.
- Deliberate holster key: `WEAPON LOWERED` / `WEAPON RAISED`. E auto-stow silent.
- Captured gameplay suppresses hard-coded C→Circle; menus still use C when uncaptured.
- `input_init` always restores compile-time defaults before applying env overrides.

## D08A2 — Bottom-left active gadget

- Replaces full strip-on-use with a single selected icon, bottom-left, both renderers.
- Green %-digits from owned `DUKE3D.GRP` `TILES011.ART` tiles 3010–3021
  (`recomp/assets/ttk-inv-digits.pack` + provenance JSON). Font fallback if pack missing.
- Jetpack/NV: ON/OFF; biomask: Auto; medkit: % only. Brackets/Enter/U unchanged.

## D26 — Backtick console

- `` ` `` opens/closes a host console (releases capture). `fps` toggles existing
  telemetry; `help` lists commands. F remains unbound.

## Automated evidence

- `ttk-input-test` — Caps Lock quotes, Scroll Lock holster quotes, C no longer holsters when captured.
- `ttk-controls-test` — gait/apartment/aim regression fixture.
- `ttk-inventory-test` — single-icon layout + green digits with pack.
- `test_pc_input` / `test_player_profiles` — v8 migration, ScrollLock token.

## Playtest focus

1. Caps Lock quotes once each; Shift-run without clunk; ordinary footsteps OK.
2. Scroll Lock draw/holster quotes; E ladder/stow silent; F not FPS.
3. Bracket/use → bottom-left icon + green % / Auto; both renderers if convenient.
4. Backtick → `fps` / `help` / close.

Preserve aiming, furniture, ladders, silent cheats, Vanilla.

## Hotfix — stiff gait regression (same day)

**Cause:** D27 soft-switched walk↔run by changing anim ID while keeping `+0x68/+0x6a`
(plant phase). Walk and run tracks are not phase-compatible; Duke froze in a stiff
pose with broken upper/weapon motion.

**Fix:** restore original frame-0 restart on gait change. Clunk silence is deferred
to a sound-path approach (do not reuse plant phase).

**Also this hotfix:**
- Inventory inset into the fitted 4:3 output frame (`24*scale` from frame left).
- `%` glyph from tile **3076**, palette-swapped to digit green (`TTKDIG2` pack).
- Percent drawn **inside** the icon box bottom-right.
- Centered Duke-font messages tighten advance by 1 (closer to EDuke32 kerning).

## Hotfix — inventory policy revision (same day)

User clarification: keep the centered **[ / ]** strip switcher (with Duke icons);
remove the bottom-left always-on gadget. **M** shows `MEDKIT N%` only (silent if
unowned). Original in-game jetpack/% HUD remains authoritative for equipped gear.

Binary after this revision: see status/handoff.
