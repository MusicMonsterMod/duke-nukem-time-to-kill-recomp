# D19 - In-game OPTIONS: the original menu, extended

Job: [D19](../MODERNIZATION_JOBS.md) (with D19B). Status: Done (user-accepted 2026-10-18: "fully accept!!"); follow-ups D19G, D19F, D19F1.
Design: the approved mockup (private artifact, user 2026-10-18: "i love it",
"absolutely rock solid") replicates the existing TTK menu; Sonic 3 A.I.R. is
no longer the template.

## What the player gets (Modernized)

OPTIONS, from the title menu and from the pause menu, lists:

    SOUND / CONTROLLER / GAME SETTINGS   (the original screens, unchanged)
    PC CONTROLS / DISPLAY / MODERNIZED   (new, in the same style)

The new screens follow GAME SETTINGS: a gold heading, `LABEL: VALUE` rows
(the highlighted label blue), sliders with the radiation disc, a one- or
two-line description of the highlighted option, and the prompt row SELECT,
CHANGE, CONFIRM, CANCEL, with DEFAULT (Square / V) on the heading line.
Lists longer than five rows scroll nine at a time. Left/Right change a
value, CONFIRM keeps the changes, CANCEL drops them, DEFAULT restores the
screen's defaults in the draft.

| Screen | Options |
| --- | --- |
| PC CONTROLS | MOUSE SENSITIVITY (slider, 0.01-2.00 degrees per count), INVERT MOUSE |
| DISPLAY | RENDERER, RESOLUTION, DISPLAY, FILTER, WIDESCREEN, FRAME RATE, DRAW DISTANCE, GEOMETRY, TEXTURES, CPU SPEED, CPU TIMING |
| MODERNIZED | CAMERA (PROFILE removed in D33), VIEW, SHOULDER, CAMERA DISTANCE, WEAPON AIM, CROSSHAIR, AIM ASSIST, RED DOT, JUMP, JUMP WALLS, LANDING, JETPACK, DYNAMITE, STEROIDS, VIEW BOB |

Applied at once: CROSSHAIR, AIM ASSIST, RED DOT, JUMP, JUMP WALLS, LANDING,
DYNAMITE, VIEW BOB. Everything else says "NEXT LAUNCH" in its description.
Every confirmed screen is saved: run.py folds it into the profile when the
game closes (and at the next launch if the game did not close normally).

Keys on these screens: arrows (held: repeat after 350 ms, every 80 ms;
sliders speed up after 1.2 s), Enter or X confirm, Escape, Backspace or Z
back, V default, mouse wheel moves. Vanilla keeps the original OPTIONS.

## How it works

Nothing is drawn by the host. The screens are built from the game's own
calls, so glyphs, colours, spacing, buttons and sliders are the originals,
and the D17 interpolation redraws them like any menu:

| Call | Use |
| --- | --- |
| `0x8008b454` text (string, x, y, flags) | items, headings, rows, descriptions. Flags: font << 8 (0 small, 1 menu), 4 centred, 8 right-aligned, 1 blue |
| `0x8008b3c8` width (string, font) | prompt placement, description wrapping |
| `0x8008b678` sprite (record, x, y, 0, [sp+16] = 100) | prompt buttons: records `0x800c43b4 + 16 * id` (4 Triangle, 6 Cross, 7 Square, 11 Left/Right, 12 Up/Down) |
| `0x80083c40` / `0x80083edc` slider init / draw | sliders. Objects in GPU-DMA mod memory (their bar packet is linked by 24-bit tag); label index 0 (an empty string), our label drawn at x0 + w/3 |
| `0x8006bbd8` sound (1 move, 0 confirm/back) | menu sounds |

Calls run through `psx_dispatch_call` on a saved stack and scratchpad, as the
D08A6 HUD does.

Layout is the originals' arithmetic over the screen geometry at
`0x800d2210` (x0, y0, w, h; at 16:9 -341, -120, 682, 240): OPTIONS items 19
apart, centred on the original three (`(h - 57) / 2 + y0`); headings at
`y0 + 171h / 2048`; GAME SETTINGS rows from `y0 / 2 + 10`, 20 apart (14 in the
compact layout), label right-aligned at -10, value at +10; prompts at
`-y0 - h / 16 - 19`, three w/4 apart, four at -3w/8, 5 - w/8, w/8 - 5, 3w/8.

### Screens and state

Both contexts keep a screen value with an unused slot that has no handler:

| Context | Screen value | OPTIONS | Ours | Back | Cursor | Original handler |
| --- | --- | --- | --- | --- | --- | --- |
| Title (mode 0, state `0x800d24e4` = 12) | `0x800d24d2` (table `0x80013200`) | 9 | 8 | 10 (main menu) | `0x800c3de0` | `0x800841c4` |
| Pause (mode 1) | `0x800be56a` (table `0x80013558`) | 2 | 8 | 12 (pause menu) | `0x800c3f32` | `0x80086174` |

On slot 8 the game draws no menu and reads no menu input; the attract demo
(title) or the paused world keeps running behind.

- Tick hooks (entry hooks, new in `game.local.toml`): `0x800220f4` (the
  frontend background, called by state 12 just before its dispatch) and
  `0x80025fb0` (the pause dispatch). There the menu applies the keys'
  actions (sounds, screen changes, the original cursor) and turns an
  original OPTIONS (9 / 2) into ours (8) before its handler can draw: the
  original OPTIONS is never shown, also on the first image.
- SOUND, CONTROLLER and GAME SETTINGS: our CONFIRM sets the original's
  cursor and its own return value (7, 4, 11; title also clears the
  controller index `0x800be56c`). They return OPTIONS (9 / 2), which becomes
  ours again with that item highlighted.
- Back: the original's return (10 / 12) with the cursor reset. Since D19J
  the title goes back to our main menu (sub-screen 12, [note 150](150-d19j-title-menu.md)) and
  gameplay to the D19H pause list.
- Draw: from frame_replay's composition (`0x80026164`) and loading-logo
  (`0x80025554`) entry hooks on live frames, before the D17 capture, at most
  once per image (`0x800bdbd0`). Replays redraw the captured packets.
- Keys: `input_pad()` (pc_input.cpp) asks the menu first; while it owns the
  keys the pad reads as nothing pressed. Keys still held when it hands back
  to an original screen are released until pressed again.
- A savestate taken on one of our screens loads to OPTIONS.

Code identity: the handler tables, both dispatches and handlers, the slider,
text, prompt and sprite code, font and sprite tables and the text table are
checked (SHA-256) before the first use; a mismatch keeps the original
OPTIONS. `DNTTK_GAME_MENU=off` (developer) does the same.
D19H (note 147) moved the start of the glyph-table guard from `0x800c3f30` to
`0x800c3f38`: the old range covered the menu variables `0x800c3f32` and
`0x800c3f34`, so a state where they differed kept the original menus.

### Settings

`game_menu.cpp` starts from the launch environment (`DNTTK_*`, and
`DNTTK_MENU_PRESENTATION` for the display values), keeps a draft per screen
and on CONFIRM writes the whole state to `DNTTK_MENU_SETTINGS_FILE`
(`<profile>.menu-settings`, JSON version 1: active profile, Modernized
controls, presentation). `player_profiles.absorb_menu_settings()` validates
it with the normal profile rules and merges it (run.py, after the game
closes and at launch); an invalid file is discarded.

Live options read `ttk_live_env(name)` (`live_env.cpp`) instead of
`getenv()`: the value last confirmed in the menu, else the environment. Jump
walls and view bob are read each time now (they were cached at first use).

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19-menu`, copied from D19E; slot 1 attract demo, 5
gameplay). Executable `fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b`.

| Check | Result |
| --- | --- |
| Title OPTIONS (`m1.py`) | six items in the original font, SOUND highlighted; prompts SELECT / CONFIRM / CANCEL |
| PC CONTROLS, DISPLAY, MODERNIZED | GAME SETTINGS look; the slider is the original's; descriptions wrap to two lines |
| Change + CONFIRM | side file holds every value, the changed one updated |
| SOUND and back | original SOUND screen; back to our OPTIONS with SOUND highlighted |
| Back from OPTIONS | main menu (state 7) |
| Pause (`m2.py pause`) | OPTIONS with RETURN, MODERNIZED, confirm, back to the pause menu, resume |
| Held Down 1 s (`r.py`) | 10 rows, list scrolls; two wheel notches move two rows |
| Idle 90 s on our screen (`m2.py title`) | stays on the screen |
| 4:3 at 120 (`w.py`) | layout fits; 118-119 distinct presents a second, guest 9.9 images |
| Vanilla (`v.py`) | original OPTIONS (sub-screen 9), three-item cursor |
| Tests | Python 146 OK (12 skipped), `ttk-input-test`, `ttk-live-env-test`, `check_repo.py` |

Not verified: each live option's effect in gameplay (no weapon could be drawn
in the offscreen test), the feel on the real display.

## Limits (stage 1)

- D19N ([note 166](166-d19n-options-layout.md)) replaced this layout:
  OPTIONS is now SOUND, CONTROLS, DISPLAY, GAMEPLAY and ADVANCED.

- Key bindings and the live display, camera, aim and mouse settings came in
  stage 2 (D19G, [note 159](159-d19g-options-stage-2.md)). Renderer,
  resolution, filter, draw distance, precision, CPU, jetpack and steroids
  still apply at the next launch. (PROFILE was removed in D33: Vanilla is
  `run.py --vanilla`, one launch.)
- Mouse clicks are not used (the click that focuses the window would confirm).
- Prompts show PS1 buttons (PC icons: D19F, D19F1).
