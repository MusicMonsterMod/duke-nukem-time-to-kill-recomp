# D14 - Widescreen first pass (research, not a player option yet)

Status: **In progress** (2026-09-30). Nothing changes for the player: Vanilla and
the default Modernized profile stay 4:3, and there is no launcher option yet.

## How widescreen works in this framework

- On PSX the framework clamps any `[video] aspect_ratio` in `game.toml` or
  `settings.toml` back to 4:3 ("widescreen is mod-owned on PSX"). Only a
  trusted **activation plugin**, selected by an enabled mod package feature, may
  call `psx_mod_set_fixed_display_aspect()` after the mod plan is committed and
  before the window exists.
- The per-game `[widescreen]` block in the game config tunes the effect. The
  runtime reads options such as `gte_game_mode`, `hud_sprt_squash` and
  `nw_hud_corners` at launch. Cull and sprite-tag sites are generation-time:
  they need the recompiler to emit code and a regeneration.
- The default path is "native-wide": the game renders a wider view with a 1:1
  present and the HUD squashed back to its proportions.

## What was added (inert by default)

- `recomp/src/ttk/widescreen.inc`: activation plugin `ttk.widescreen`. It
  requests 16:9 only when `DNTTK_INPUT_MODE=modernized` and
  `DNTTK_WIDESCREEN=16:9`. Native test: Vanilla, unset or `off` stay 4:3.
- `recomp/mods/preloaded/packages/dnttk.presentation.widescreen/1.0.0/manifest.toml`:
  format-5 package, one default-enabled feature selecting that plugin (no
  patches, no disc writes). Savestates do not bind to the mod plan fingerprint.
- `recomp/CMakeLists.txt`: stages `mods/preloaded/packages` beside the
  executable after the framework catalog.
- `run.py` does **not** set `DNTTK_WIDESCREEN` yet, so the player build is 4:3.
  A default 4:3 launch with the package present is pixel-identical to the
  baseline capture.

## Experiment (private config copy, private cards, Xvfb captures)

`recomp/analysis/d14-widescreen/ws.py`, Modernized, `DNTTK_WIDESCREEN=16:9`,
states from private copies of UI slots 1, 5 and 12:

- `gte_game_mode`, `hud_sprt_squash`: 16:9 engages at game entry; the wider view
  renders and the orbit camera is unaffected. The HUD keeps its proportions but
  stays inside the central 4:3 area.
- Adding `nw_hud_corners`: health and inventory boxes move to the left corner;
  the ammo box does not reach the right corner.
- **Missing geometry at the widened right side** in all three scenes (black
  bands where walls and floor should be). TTK culls polygons against its own
  4:3 screen range; the widened view exposes the gap. This is the main blocker.

## Next steps for D14

1. Find TTK's screen-space cull sites (the X range tests after projection in
   the world/object draw paths) and describe them in `[widescreen.cull]`
   (`screen_x_sites`, `slti_sites`, ...), then regenerate and rebuild.
2. HUD anchoring: identify the right-hand HUD packets (ammo box, item icons) for
   `nw_hud_corners` or a TTK-specific HUD offset.
3. Then decide FOV (native-wide keeps vertical FOV and widens horizontally),
   first-person and scripted cameras, movie framing (FMV stays 4:3,
   pillarboxed) and menus, and add `run.py --widescreen off|16:9` (Modernized
   profile schema bump) with captures at 4:3 and 16:9.

## User preview (2026-10-01) and 2-minute triage

User ran the 16:9 preview (`DNTTK_WIDESCREEN=16:9`, private config with
`gte_game_mode`, `hud_sprt_squash`, `nw_hud_corners`): "tremendously broken":
the picture is **off centre** and has **tearing / large holes** in geometry.
Work deferred to a fresh session.

Triage (no code changed):
- The preview uses the framework's default **native-wide** path (the game
  renders a wider view, presented 1:1). Every private capture showed the
  missing geometry and the extra reveal on the **right** side only, which fits
  the game's own 4:3 assumptions still being in force: its projection centre
  (GTE offset), its GPU drawing area / clip rectangle (512 wide) and its
  screen-space culling. One-sided reveal = off-centre picture; clipped or culled
  polygons = holes.
- The runtime's stderr session log does not record the widescreen setup lines
  (they go to stdout); a diagnostic run should capture stdout too.

Next session, in order:
1. A/B the presentation path: `native_wide = false` (projection squash +
   stretched present) versus native-wide, with captures at the same savestates
   (private copies of UI slots 1, 5, 11, 12). If the squash path is centred,
   prefer it as the first shippable mode.
2. For native-wide: find TTK's GTE offset (OFX) / drawing-area setup and the
   screen-X cull sites; describe them in `[widescreen]` / `[widescreen.cull]`
   (cull sites need regeneration) and re-check centring and holes.
3. HUD anchoring (right-hand ammo/item boxes), then the run.py option.
