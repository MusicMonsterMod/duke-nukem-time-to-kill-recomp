# D17AR6 - Renderer choice in OPTIONS: Polymorph and Classic

2026-10-31. Status: Done (accepted 2026-11-01: "lock it in, close us out"). The user named our renderer **Polymorph**
(note 341); this job offers it and the earlier drawing as a player choice.

Evidence is local under `recomp/analysis/d17ar6/` (`ab.py`, `menu.py`,
`ro.py`; logs and shots). Every run used private copies of the D17AQ fixture
card and savestates and a private profile, and hash-checked the player's saves
and profile before and after.

## What the choice maps to

| Switch | Polymorph (default) | Classic |
| --- | --- | --- |
| Unified geometry (D17AM) | on | off (`DNTTK_UNIFIED_GEOMETRY=0`) |
| Host world visibility (D17AR2/3) | on | off (`DNTTK_HOST_VISIBILITY=0`) |
| Redraw-only live frames (D17AR3) | on | off (`DNTTK_REDRAW_ONLY=0`) |
| Near path (D17) | on | on |

**Why the near path stays on in Classic.** The job listed it among the
switches, so it was measured first. With `DNTTK_NEAR_CLIP=0` third person
matched Polymorph in six views (slots 3, 2, 10), but first person broke: the
D36 arm presentation changed (bare forearm in place of the gloved arm) and
close walls vanished (slot 10: sky visible through a wall at arm's length;
`near0/`). The near path is the first-person and widescreen foundation from
before Polymorph, not part of it. With it kept, Classic first person matches
Polymorph in all nine views checked (`cmp-first-near.png`). `DNTTK_NEAR_CLIP`
stays a developer A/B.

## Implementation

- Profile: Modernized control `world_renderer` (`polymorph` / `classic`),
  added as a recovered field (missing or invalid -> `polymorph`), no schema
  bump (`player_profiles.py`). `--settings` summary line "World renderer".
- Launcher: `--world-renderer polymorph|classic` saves it. `run.py` exports
  `DNTTK_WORLD_RENDERER` (Vanilla: `classic`) and, for Modernized Classic,
  sets the three switches to 0 with `setdefault`, so explicit developer
  values still win. Vanilla sets none of them; it never reaches these paths
  (input_modernized guards), so Vanilla play is unchanged.
- OPTIONS > DISPLAY: new row **RENDERER: POLYMORPH / CLASSIC** after FRAME
  RATE, NEXT LAUNCH (the switches are read once at start and the redraw
  workers inherit them). The ADVANCED > GRAPHICS OpenGL / software row is
  renamed **BACKEND** so two rows are not both called RENDERER; its dimmed-row
  hint now reads NEEDS BACKEND OPENGL. `--renderer` keeps its meaning
  (backend). No framework changes.

## Tests

- Round trip with real keys under Xvfb (`menu.py`): Polymorph launch (host
  visibility on, unified on, near on) -> DISPLAY, RENDERER Right to CLASSIC,
  CONFIRM -> side file `world_renderer: classic` -> quit, profile saved
  `classic` -> relaunch: host visibility off (0 host frames), unified off,
  near on; the row reopens on CLASSIC. `--vanilla`: no host frames, draw
  distance original. `--world-renderer polymorph`: all on again.
- Console in each launch (`ro.py`): Classic "Redraw-only live frames off",
  "Host world visibility off"; Polymorph both on.
- `test_player_profiles` (112, new `WorldRendererTest`: recovery, CLI, env,
  developer override, Vanilla, menu side file, invalid value),
  `ttk-near-test`, `ttk-input-test`, `test_auto_frame_rate` pass;
  `check_repo.py` clean.

## Limits

- Not live: the choice applies on the next launch. A live switch would need
  the unified and near-path switches to take a runtime override and reach the
  forked redraw workers; possible later. D40 made it live ([note 371](371-d40-live-renderer-switch.md)).
- Classic at high refresh has no redraw-only frames, so heavy views reach
  high rates less often than Polymorph (expected; not measured here).
- The console replies still say "(developer)" for the switches.
