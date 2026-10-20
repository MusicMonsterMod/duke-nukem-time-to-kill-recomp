# D19N - One OPTIONS screen, the rest under ADVANCED

Job: [D19N](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-20: "lock it in and close it out!!").
Follows D19 ([note 145](145-d19-game-menu.md)) and D19G
([note 159](159-d19g-options-stage-2.md)).
Executable `6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33` (first build `ce2b297e...`; review pass below).

## Design (user, 2026-10-20)

A private design page (https://claude.ai/artifact/4a4rvMENnieMK9VGNX4Uzr, no
retail assets) listed all 39 settings, three layouts, seven conflicts and five
decisions. The user chose: layout A; AUTO AIM stored in the profile, default
OFF, written into the game (a card's value is overridden in Modernized);
CONFIGURE CONTROLLER removed until D09 with the default pad layout forced in
Modernized; GAME SPEED under ADVANCED > ORIGINAL GAME; the DISPLAY row renamed
WINDOW.

## What the player gets (Modernized)

    OPTIONS: SOUND / CONTROLS / DISPLAY / GAMEPLAY / ADVANCED

| Screen | Rows |
| --- | --- |
| SOUND | the original screen, unchanged: SOUND EFFECTS, MUSIC, SPEECH, MODE |
| CONTROLS | MOUSE SENSITIVITY, INVERT MOUSE, KEY BINDINGS |
| DISPLAY | WINDOW, RESOLUTION, WIDESCREEN, FRAME RATE |
| GAMEPLAY | VIEW, SHOULDER, CAMERA DISTANCE, CROSSHAIR, GORE, STATUS BARS, VIEW BOB |
| ADVANCED | CAMERA AND AIM: CAMERA, WEAPON AIM. MOVEMENT: JUMP, JUMP WALLS, LANDING, SWIMMING, JETPACK. ITEMS: DYNAMITE, STEROIDS. GRAPHICS: RENDERER, FILTER, DRAW DISTANCE, GEOMETRY, TEXTURES. PERFORMANCE: CPU SPEED, CPU TIMING. ORIGINAL GAME: AUTO AIM, RED DOT, GAME SPEED |

ADVANCED is one scrolling list; the section headings are amber (as in KEY
BINDINGS) and the cursor skips them. "MODERNIZED", "PC CONTROLS", the original
CONTROLLER and GAME SETTINGS screens are no longer listed. Vanilla keeps the
original OPTIONS (SOUND, CONTROLLER, GAME SETTINGS).

Every setting of the old screens is reachable except two, removed on purpose:

- **AIM ASSIST** became **AUTO AIM** (same profile key `aim_assist`: OFF =
  `off`, ON = `original-lock`). ON also turns the original's auto aim on in
  the game, which the assist and the red dot need (note 24).
- **CONFIGURE CONTROLLER** (the PS1 pad layout): Modernized keys send fixed
  pad buttons (Space is Square, E is Cross, ...), so a changed layout would
  scramble them. Not offered until gamepads get their own bindings (D09).

Rows with no effect are drawn dimmed, still changeable, and their
description names what they need:

| Row | Needs |
| --- | --- |
| SHOULDER, CAMERA DISTANCE, WEAPON AIM | CAMERA INDEPENDENT |
| CROSSHAIR | WEAPON AIM VIEW |
| RED DOT | AUTO AIM ON (the row above it) |
| RESOLUTION, GEOMETRY, TEXTURES | RENDERER OPENGL |

## Review pass (user, 2026-10-20)

- **Title CONTROLS blank; ADVANCED vanished at GAME SPEED.** On the title at a
  high frame rate (the user's 120), any of our screens with a slider in view
  showed nothing (keys and sounds still worked); the pause menu was fine.
  Reproduced offscreen at 120 (fine at 60). Our slider objects were in the
  framework's GPU-DMA memory, which the D17 frontend replay's copy of the
  machine does not hold. They now use the original SOUND screen's three slider
  objects in guest RAM (`0x800dd290`, `+0x40`, `+0x80`; SOUND initialises them
  each time it opens and never runs while our screens are up).
- **GAME SPEED overlapped ORIGINAL GAME.** In the compact layout a slider row
  now takes two row places and is drawn halfway down them.
- **AUTO AIM moved** from GAMEPLAY to ADVANCED > ORIGINAL GAME, with RED DOT
  (its marker) beside it and GAME SPEED below.
- **USE ITEM is Enter only.** `item_use` defaults to Unbound (Enter always
  uses the item); KEY BINDINGS shows ENTER, hint "ENTER USES IT. A SECOND KEY
  IS OPTIONAL". Profile schema 36 clears a saved U (the old default) for
  `item_use`; a key the player chose stays. Side note from the user: in Duke 3D
  U toggles mouse aiming; Modernized has no such toggle (F10 is the closest),
  so U stays free until a better EDuke32-like use is found.

Evidence (offscreen, 120): title CONTROLS with its slider; ADVANCED up from
CAMERA to GAME SPEED, and back down past the end to CAMERA, all drawn; pause
ADVANCED bottom; KEY BINDINGS USE ITEM: ENTER. Python 160 OK (12 skipped,
new: schema 36 drops the default U, keeps a custom key), `ttk-input-test`
(default USE ITEM Unbound), `ttk-live-env-test`.

## The original GAME SETTINGS, decoded

Handler `0x800899cc` (frontend sub-screen 11, pause 11), rows by jump table
`0x80017040`:

| Value | Where | Meaning |
| --- | --- | --- |
| AUTO AIM | `0x800c3f9c` bit 0 | 1 = ON (text 0x0a), read by the aim code `0x80057fc0`, `0x80058088` |
| GORE | bits 1-2 | 0-2, names from `0x800c3fa4` (text 0xe5-0xe7 NONE, SOME, LOTS) |
| STATUS BARS | bit 3 | 1 = ON; `0x8002653c` calls the status bar `0x8008ba30` only when set |
| VIBRATION FUNCTION | bit 4 | shown only with bit 5 (a vibrating pad); not a PC row |
| GAME SPEED | `0x800c3fa0` | 0-100, step 5, default 50; CONFIRM calls `0x80089920` |

`0x80089920(speed)` sets Duke's animation rate `0x800c3c80` (4600 at 50, 14
per step, read into player `+0xb0` at `0x80041478`) and the enemies'
`0x800c3d14` (5350 at 50, doubled on the hardest difficulty, capped 0x1c00;
read at `0x80065138`). Until GAME SETTINGS is confirmed or a card loads, the
executable's own 4800 / 5096 apply. The executable's default word is `0x1d`
(AUTO AIM, LOTS, STATUS BARS, vibration on). GAMMA has a text id but no row.

Pad layouts: 16 bytes per pad at `0x800d1b60`, default `0x800bda7c`
(`0x8001e3c0` resets, `0x8001e3e4` / `0x8001e410` copy out / in). The memory
card's settings are packed by `0x80082f64` (from the SAVE GAME screen
`0x80087480` via `0x80083404`) and unpacked by `0x80083070`, which ends with
`0x80089920` (return `0x8008315c`).

## How it works

- `game_menu.cpp`: four screens (CONTROLS, DISPLAY, GAMEPLAY, ADVANCED);
  `Row::heading` rows (drawn with `say_clut` amber, skipped by the cursor,
  scrolled like KEY BINDINGS); `Section::Game` rows read the game on opening
  the screen and write it on CONFIRM, never the profile; GAME SPEED calls
  `0x80089920` at the next tick, as the original's CONFIRM does.
- Dimmed rows: `say_dim` sets the texture modulation of the glyph packets the
  text call just added to 0x48 (full is 0x80), as D08A6 dims an off gadget.
  The font's grey CLUT row 224 was tried first: its entry 0 is opaque and it
  drew a box behind every glyph.
- `enforce_modern_settings` (each live composition): the default pad layout
  for both pads and AUTO AIM bit 0 from the profile's aim assist. A card copy
  keeps what the game had before Modernized changed it (captured once, and
  again after each card load): `0x80082f64` (entry hook) writes it back before
  the pack, so a save made in Modernized keeps the Vanilla layout and AUTO AIM;
  the next frame restores Modernized's values. The unpack's `0x80089920` call
  (return `0x8008315c`) captures the loaded values, then enforces.
- Two entry hooks added to `game.local.toml` (`0x80082F64`, `0x80089920`) and
  regenerated with `psxrecomp_cli.py generate`; codegen hash unchanged.
  New code identity guards: `0x80089920`, the GAME SETTINGS handler, the
  pack/unpack, the layout copies and the default layout.
- STATUS BARS needed no new code: every HUD addition (gadget box, steroids
  and power-up countdowns, challenge item) is drawn from the status bar's
  entry hook, which the game skips when STATUS BARS is off.

## Evidence

Offscreen on the real GPU, private profile, cards and savestates
(`recomp/analysis/d19n-options`, copied from D19G; load 5 = gameplay, 3 =
title). Scripts: `n.py` (menus), `c.py` (statistics screen, card save and
load), `v.py` (Vanilla).

| Check | Result |
| --- | --- |
| Pause > OPTIONS | SOUND, CONTROLS, DISPLAY, GAMEPLAY, ADVANCED; each screen as listed |
| SOUND | the original screen; back to our OPTIONS, SOUND highlighted (title and pause) |
| ADVANCED | amber headings, scroll to GAME SPEED (slider), Escape back with ADVANCED highlighted |
| Dimmed rows | RED DOT with AUTO AIM OFF: dimmed, "NEEDS AUTO AIM ON, IN GAMEPLAY"; CAMERA ORIGINAL dims WEAPON AIM |
| GAME SPEED 50 -> 40, CONFIRM | `0x800c3fa0` 40, Duke 4530, enemies 5280 (the original's arithmetic) |
| GORE NONE, STATUS BARS OFF, AUTO AIM ON, CONFIRM | word `0x1c` -> `0x11`; resume: no status bars; profile `aim_assist` `original-lock` |
| Launch (AUTO AIM OFF) | savestate word `0x1d` -> `0x1c`; both pad layouts default |
| Statistics SAVE | during the save screen the word reads `0x1d` (the card's AUTO AIM), then gameplay `0x1c` again; `card_packs` 1 |
| Death screen LOAD GAME of that save | `card_loads` 1, card AUTO AIM ON kept, play stays OFF |
| `--vanilla` | word and a custom pad-layout byte untouched; original pause screen |
| Tests | Python 158 OK (12 skipped); `ttk-input-test`, `ttk-live-env-test` |

## Not verified / limits

- AUTO AIM ON in play (the assisted shot and the red dot) and GAME SPEED's
  feel: the user judges.
- A pad layout changed in Vanilla, saved, loaded in Modernized and saved
  again: the same card-copy path as AUTO AIM (tested), not run end to end.
- Behaviour change: Modernized now plays with the original auto aim off unless
  AUTO AIM is ON. Shots that fall back to the original aim (D07E stale lease,
  WEAPON AIM ORIGINAL) no longer get the original target lock by default.
- Gamepads keep the default layout until D09.
