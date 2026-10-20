# D19G - In-game OPTIONS stage 2: KEY BINDINGS and live settings

Job: [D19G](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "looking good!! lock it in and close this job out").
Follow-up to D19 ([note 145](145-d19-game-menu.md)).
Executable `0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da`.

## Design (user, 2026-10-20)

A mockup made from the disc's own interface art, composed in Photon over a
live capture, was approved as drawn ("Looks right, build it"): five screens,
PC CONTROLS with a KEY BINDINGS row, the list, waiting for a key, a swap on
conflict and DEFAULT asking first. User choices: a new key applies and is
saved at once (no draft); the actions are grouped under section headings.

The mockup is local and retail-derived (ignored, never committed):
`recomp/analysis/d19g-bindings/` holds `glyphs.py` (TTK fonts, CLUTs and
button sprites from `/DATA/FONTS.RAW` and `SLUS_005.83`), `compose.py` (game
coordinates mapped as the capture shows them: positions 1.15 x 2.4 px per
unit, glyphs 1.39 x 2.4, word space 8), `screens.py`, the editable Photon
document `d19g-key-bindings-mockup.psd` (120 layers in 5 artboards) and its
PNG export.

## What the player gets (Modernized)

**PC CONTROLS** gains a third row, **KEY BINDINGS: >** ("THE KEY OR MOUSE
BUTTON FOR EACH ACTION. CONFIRM OPENS THE LIST"). Enter opens:

| Part | Behaviour |
| --- | --- |
| Heading | KEY BINDINGS (menu font, gold), DEFAULT (Square / V) on the heading line |
| List | 43 actions under MOVEMENT, WEAPONS, ITEMS, CAMERA, ORIGINAL CONTROLS; compact layout (14 apart, 9 visible, scroll mark); `LABEL: KEY`, cursor label blue |
| Section headings | small font, amber ramp (CLUT row 239), skipped by the cursor |
| Enter | the value turns blue `PRESS A KEY`; description `PRESS A KEY OR MOUSE BUTTON.` / `ESCAPE CANCELS, BACKSPACE CLEARS`; one prompt, CANCEL |
| A key or mouse button | bound at once and saved; a key another action had swaps (that action takes the old key; C never moves off holster), description `E WAS USE / MANTLE: THAT IS ON SPACE NOW` |
| Refused keys | not in the bindable set: `THAT KEY IS RESERVED. PRESS ANOTHER`; C for anything but holster: `C IS THE HOLSTER KEY ONLY. PRESS ANOTHER`; still waiting |
| Escape / Backspace | cancel / no key (Unbound) |
| V | RESTORE DEFAULT KEYS? YES / NO (NO first); YES restores every default, `DEFAULT KEYS RESTORED` |
| Escape (list) | back to PC CONTROLS, KEY BINDINGS highlighted |

Key names: letters, digits, SPACE, LEFT CTRL, SHIFT (either Shift), ALT
(either Alt), SCROLL LOCK, [ and ], SEMICOLON, APOSTROPHE, COMMA, PERIOD,
LEFT/RIGHT/MIDDLE MOUSE, MOUSE 4/5, NONE. The menu font has no brackets: `[`
is its own L (stem and bottom bar in one glyph, no seam) with a hyphen as the
top bar; `]` is I with hyphen bars (an I-built `[` showed a dark nick where
the stem's outline met the bottom bar); sentences name them LEFT / RIGHT BRACKET. Right
Shift and Right Alt bind as SHIFT and ALT, as play reads them.

Labels follow what the keys do (user review, 2026-10-20): the `inventory`
action is CROSSHAIR (I toggles the crosshair; the inventory screen is the
fixed Right Shift); USE ITEM shows ENTER / U (Enter, fixed, always uses the
item; U is the bindable key); the ten weapon keys are named after the first
weapon of their group (MIGHTY BOOT, DESERT EAGLE, COMBAT SHOTGUN, GATLING GUN,
RPG, PIPE BOMB, ENERGY WEAPON, FLAMETHROWER, BUFFALO RIFLE, FREEZER). A
one-line hint explains GRAB, GRAB 2 (a second grab key, Alt; the only grab
with original aiming), CROSSHAIR, USE ITEM and the stacked groups. Backlog
from the same review: D07F (airborne shots ignore the crosshair), D10C (Duke
3D look keys), D19G1 (FRAME RATE live after a 60 launch).

**Live settings.** On CONFIRM these now apply at once (no NEXT LAUNCH): MOUSE
SENSITIVITY, INVERT MOUSE, DISPLAY (windowed / borderless / exclusive),
WIDESCREEN, CAMERA, VIEW, SHOULDER, CAMERA DISTANCE, WEAPON AIM, and FRAME
RATE when the game was started at a rate other than 60 (a 60 launch has no
replay presentation; the row then says NEXT LAUNCH). Still NEXT LAUNCH:
RENDERER, RESOLUTION, FILTER, DRAW DISTANCE, GEOMETRY, TEXTURES, CPU SPEED,
CPU TIMING, JETPACK, STEROIDS. MODERNIZED opens on the live VIEW, SHOULDER
and CAMERA DISTANCE (P, H and Alt + wheel change them in game). VIEW and
SHOULDER descriptions name the actions, since their keys can change.

## How it works

- `pc_input.cpp`: `input_binding_code`, `input_default_code`,
  `input_code_bindable` (the launch payload's own rules: `allowed()`, C only
  for holster), `input_set_binding_codes` (binds[] live; the menu owns the
  keys meanwhile, so nothing is cleared), `input_capture_key` /
  `input_take_captured_key`: while on, the next key-down (no repeats) or mouse
  button goes to the menu only, before menu taps, cheats, capture or actions.
  View and shoulder getters and setters for the menu. Camera mode and weapon
  aim read `ttk_live_env`.
- `modern_controls.cpp` / `weapon_aim.cpp`: mouse sensitivity, invert, camera
  mode and weapon aim read `ttk_live_env`. `camera_set_distance` sets the
  preferred boom as Alt + wheel would (0: the original boom, read from the
  camera as `orbit_begin` does) and persists through the camera side file.
- `frame_replay.cpp`: `frame_rate_set` re-targets replay presentation
  (`psx_mod_request_present_replay_rate`), interpolation on above 60 / display
  / unlimited, load shedding reset.
- Framework (tracked patch): `psx_mod_request_fullscreen_mode`,
  `psx_mod_request_display_aspect` and `psx_mod_request_present_replay_rate`
  queue a request applied on the present path (`sdl_vblank_present_body`,
  where F11 and the adaptive aspect run). A fixed aspect also reshapes a plain
  window at its width, narrowing instead when it would exceed the display's
  usable height. `gl_renderer_set_replay_target_hz` in the GL renderer.
  Declarations in `mod_plugins.h` / `gpu_gl_renderer.h` (not codegen-hashed:
  savestates still load).
- `game_menu.cpp`: Page::Keys, Act::RestoreKeys, the KEY BINDINGS row as a
  link row, `apply_live` per changed row on CONFIRM, `refresh_from_game`.
  Section headings: the text call (0x8008b454) offers only gold, blue (flag
  1) and red (flag 0x20); `say_clut` shifts the CLUT halfword (+14) of the
  40-byte glyph packets that call just added at 0x800d67a8 to row 239 (amber;
  entry 0 transparent like gold's). Row 221 (the mockup's silver) is another
  sprite's palette with an opaque entry 0: it drew a box, so it is not used.
- Side file (`DNTTK_MENU_SETTINGS_FILE`, version 1) gains `bindings`
  (action -> profile token, `tests/local/test_pc_input.py` keeps the menu's
  token table equal to `pc_input.TOKENS`); `absorb_menu_settings` validates
  them with `pc_input.validate` and replaces the Modernized bindings; an
  invalid set discards the file. Older files without `bindings` keep the
  profile's.
- Diagnostics: `ttk_input` reports `binds` (live codes) and
  `controls.mouse_deg` (turn per mouse count). Test-only steer word `play`
  (DNTTK_TEST_INPUT) makes that frame captured play while menus run
  uncaptured.

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19g-bindings`, copied from D33; load 5 = gameplay, load
3 = title main menu). Scripts: `k.py` (flow), `k3.py` (rebind and play),
`l.py`, `m.py`, `n.py`, `o.py`, `s.py` (live settings), `w.py`, `x2.py`
(widescreen, display on a private Xvfb), `t2.py` (title), `v.py` (Vanilla).

| Check | Result |
| --- | --- |
| Pause > OPTIONS > PC CONTROLS > KEY BINDINGS | as the mockup, drawn by the game's calls |
| JUMP, Enter, E | JUMP E, USE / MANTLE SPACE, `E WAS USE / MANTLE: THAT IS ON SPACE NOW` |
| Enter on Enter / C | `THAT KEY IS RESERVED`, `C IS THE HOLSTER KEY ONLY`; still waiting |
| V, YES | RESTORE DEFAULT KEYS?, every default back |
| JUMP -> Q, resume | live codes JUMP 44 -> 20; Q requests a jump (8 frames pending), Space does not; profile after exit: jump Q, quick_kick Space |
| MOUSE SENSITIVITY 0.12 -> 0.20, CONFIRM | `mouse_deg` 0.12 -> 0.20 at once; a later CANCEL keeps 0.20 |
| VIEW, CAMERA DISTANCE 2048, WIDESCREEN OFF | first person off, preferred boom 2048, 4:3 picture and HUD, all without a restart; profile saved |
| 4:3 launch, WIDESCREEN 16:9 | the paused world widens (letterboxed in the 4:3 window offscreen) |
| 16:9 launch, 21:9 / AUTO / 16:9 | wide picture each time; Xvfb window 1920x1080 -> 1920x822 for 21:9, 1440x1080 for OFF (fits the screen) |
| FRAME RATE 120 -> 144 live | replay target 144, 4 steps, 144 images shown a second, interpolation on |
| DISPLAY EXCLUSIVE | the switch is issued (`exclusive fullscreen mode 1920x1080`); Xvfb cannot change mode and SDL reverts |
| Title OPTIONS > KEY BINDINGS | over the attract demo; scrolls into WEAPONS; DEFAULT asks; back to PC CONTROLS, OPTIONS |
| `--vanilla` | `Profile: Vanilla (this launch only)`, original 4:3 picture and HUD |
| Python | 153 OK (2 skipped); new: side-file bindings (valid, conflicts, reserved, missing, C), menu tokens = profile tokens |
| Native | `ttk-input-test` (links `live_env.cpp` now), `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all levels), `ttk-aim-test`, `ttk-near-test`, `ttk-live-env-test`, `ttk-font-test`, `ttk-inventory-test` |
| Patch | exported; applies cleanly to the pinned framework commit; `check_repo.py` OK |

## Not verified / limits

- The real display: borderless and exclusive switching, the window reshape
  with a window manager, the feel of the whole page. The user judges.
- A FRAME RATE change after a 60 launch still waits for the next launch.
- Gamepad navigation of the page (D09); prompts show PS1 buttons (D19F).
- Bindings for Vanilla stay with `keybinds.ini`; this page is Modernized only.
- Turn-rate sampling in the street savestate is unreliable (an enemy hits
  Duke); sensitivity was checked by the value the camera uses.
