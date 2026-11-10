# Time to Kill jobs - Menus and UI

Job descriptions and work logs for D19-D21, D26, D29, D42 and D43. Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D19 — Modern in-game menus, settings and input prompts

**Done (user-accepted 2026-10-18: "fully accept!!").** Remaining items continue in D19G (key bindings, live display/camera changes), D19F/D19F1 (PC prompt icons) and D09 (controller navigation). **Stage 1 implemented 2026-10-18.** OPTIONS on the title
menu and in the pause menu lists SOUND, CONTROLLER, GAME SETTINGS, PC
CONTROLS, DISPLAY and MODERNIZED. The three new screens are drawn entirely by
the game's own text, sprite, slider and sound code (no host drawing), on an
unused screen slot with no handler, so they match the originals and the D17
interpolation redraws them. Confirmed settings are saved to the profile
(`<profile>.menu-settings`, folded in by run.py); crosshair, aim assist, red
dot, jump, jump walls, landing, dynamite and view bob apply at once, the rest
say NEXT LAUNCH. Not yet: key bindings in PC CONTROLS, live display/camera
changes, PC prompt icons (D19F). Executable
`fb169a561aa15a979b4b10b4903dbd5758b17708ebe24b669f58966faf16760b`. Note [145](documentation/145-d19-game-menu.md).

**Direction revised by the user 2026-10-18:** Sonic 3 A.I.R. is no longer the
template. The modern menu replicates the existing TTK menu: the same font,
colours, layout, sliders and button prompts, with the PC/Modernized options as
extra screens in that style. An interactive HTML mockup (private artifact
https://claude.ai/artifact/6FxFwzfzdCp2doDjSesfHA, not tracked: it uses
disc-derived glyphs) was well received ("i love it!!!"). It shows the original screens plus
proposed PC CONTROLS, DISPLAY and MODERNIZED screens under OPTIONS. Measured
from the captures: the menus draw the big italic font (`/DATA/FONTS.RAW`, CLUT
225 gold, 226 blue highlight, 224 grey) squeezed horizontally to 0.725 of its
height scale, each glyph overlapping the previous one by about 3 px (the
captures fit 5 px; 3 px reads better, user 2026-10-18). EDuke32
(`research/eduke32-menu.odt`) remains an optional reference only. The
PC prompt icons are D19F.

**Direction set by the user 2026-09-29 (backlog; not started).** The target
is a customization menu in the spirit of Sonic 3 A.I.R.: one aesthetically
pleasing place to pick Modernized options (control schemes such as the D08R
jetpack scheme, camera, aim, crosshair, display, audio and later
customizations), with clear per-option descriptions and good defaults.
Preferred approach: hack the original TTK in-game menu rather than bolt on a
separate host screen, and give it a responsiveness overhaul (see D19B) as part
of the same work. Until then, new options ship as persisted profile settings
with `run.py` CLI switches. **Menu toggles already requested (user
2026-09-29):** first-person near clipping (D11B: `conservative` default,
`off` = Vanilla rendering via `DNTTK_NEAR_CLIP=0`; the subdividing `full`
mode stays a developer/research option, not a player choice) and the
first-person occluder fade (D11B: off by default in the eye view;
`DNTTK_FP_OCCLUDER_FADE=1` restores the original see-through props).
**Before any implementation:** design in plan
mode and publish a design artifact (menu structure, option list, visual
style, navigation and how it hooks the original menu) for user review. There
is a lot to consider; this is a dedicated future session, not a side task.

Build a modern in-game menu system, using EDuke32 as a design reference for navigation, option organization and customization. Investigate reuse of suitable open-source menu code, recording its exact revision/license and compatibility with this runtime before adopting it; source availability alone does not establish integration suitability. Preserve the original menus as an option and support safe pause/resume, keyboard/mouse and controller navigation. Build an accessible Time to Kill settings surface for mode, implemented controls, display and audio options. Show actual bindings in help and prompts; provide reset and safe handling of display changes. Avoid exposing developer terminology in normal player flows.

**Acceptance:** a player can select a mode, change bindings/display settings and recover defaults without editing files. Keyboard/controller navigation works. The root [game manual](GAME_MANUAL.md) matches the shipped build and labels planned features separately.

### D19A — Duke font assets for host messages and modern UI

**Done (2026-09-27), bounded existing-renderer message pass.** Both font styles,
asset fallbacks and OpenGL/Software captures verified; see
[implementation evidence](documentation/40-feedback-implementation.md). Replace the generic host pixel font with the supplied Duke-style assets,
starting with cheat confirmations and extending shared modern message/prompt
rendering consistently. Small/message font for ordinary text; Atomic menu font
for suitable headings. Do not silently replace original TTK/Vanilla presentation.
No D13 dependency for the bounded existing-renderer message pass; coordinate later
menu work with D19. Inspect actual glyph coverage, spacing, transparency and palette
before selecting an asset path. Preserve provenance/credits and original archives.

Sources exist locally: `research/fonts/DukeNukemSmallFont.pk3`,
`DukeNukemAtomicFont.pk3`, the PNG sprite ZIP in that directory, and `DUKE3D.GRP`.
The PK3 metadata describes Doom-paletted glyphs, not a drop-in TTF; PNG sprites or
an atlas extracted from owned GRP tiles may be easier. EDuke32 names identify font
tiles; its engine source alone is not proof it bundles those art assets. See the
[inspected asset inventory](documentation/39-next-iteration-brief.md).

**Acceptance:** deterministic glyph mapping/atlas generation, intentional case and
missing-character fallback, correct colors/transparency, readable spacing/scaling,
and original cheat wording (D08G1) shown in the selected Duke font. Review gameplay
captures in supported OpenGL/software paths, representative punctuation/digits and
window sizes, no clipping/blur regressions, packaged asset loading and clean fallback.
Do not silently download substitute assets or remove original credits.

### D19B — Responsive modern menu navigation and transitions

**Todo; deferred with D19 menu design.** The user wants the responsiveness
overhaul done on the hacked original in-game menu as part of the D19 design
(2026-09-29). User reports the menu system is clunky,
laggy and has terrible responsiveness. Profile navigation, selection, back,
opening/closing and screen transitions separately from gameplay performance.
Measure event receipt, guest processing, presentation delay, repeat/debounce and
animation pacing before assigning a cause. Investigation can precede D13; final
modern-menu integration belongs with D19.

**Acceptance:** baseline and improved input-to-visible-response measurements plus
repeatable keyboard/mouse/controller routes; taps register once, held repeat is
predictable, rapid navigation/back works, and modern transitions do not impose
avoidable dead time. Define budgets from measurements, not an unsupported FPS
promise. Preserve safe pause/resume, focus handling, settings, audio/FMV and the
original-menu option. No global simulation-speed change to make menus feel fast.

### D19C - Interim: more responsive original menus

**Accepted (2026-10-18), demo-backed menus.** User: "its more responsive when the demo is playing, but when the loading spinner plays it is less responsive. lets commit at this point". The loading-spinner case and a sweep of every menu screen continue in D19E. Note [142](documentation/142-d19c-menu-response.md). **Opened 2026-10-18.** User request after D17T: improve responsiveness
on the original menus themselves, before our own menu system (D19/D19B).
D17T smoothed motion and held short taps, but the frontend logic still runs
one tick per 6 fields (the flip `0x8001fcbc`), so the cursor, screen changes
and confirmations react at 10 Hz and menu text changes only 10 times a second
(note 140).

The attract demo is recorded input and desyncs if the frontend ticks faster,
so a faster menu must not change the demo. Investigate options such as: a
faster tick only while a menu (not the demo) owns input, if the menu code
and its animations tolerate it (check what is frame-counted, e.g. the
`0x800bdea0` counter, versus delta-timed); host-side drawing of the menu
cursor/text at present rate; lower input-to-screen latency (sample input
nearer the tick, shorter frontend world delay while a menu is open). Measure
input-to-screen latency before and after.

**Acceptance:** measured lower input-to-screen latency on the title menu and
its sub-menus with no missed or doubled inputs; the attract demo unchanged;
menu animations at their original speed; Vanilla unchanged; the user judges
the feel.

**Result (2026-10-18, implemented):** the attract demo plays behind every
menu screen (main menu, LOAD GAME), so the 10 Hz tick must stay. Of the
220 ms average press-to-screen time on the main menu at 120, the D17T
interpolation delay contributed a constant 131 ms (in-betweens were drawn
from the older image of each pair). In-betweens are now drawn from the newer
image with the world blended back toward the older poses: menu text,
highlight and screen changes arrive one image (about 100 ms) sooner, the
motion is unchanged. Measured: 220 -> 124 ms mean (range 71-171), 24 of 24
taps one step each, smoothness unchanged (119 distinct presents/s), demo,
gameplay start, 60 and Vanilla unchanged. The guest's own one-tick wait
(about 88 ms with its frame work) remains. Executable
`1e1a4795ab8af038f9c95b50d2159cab10bfb1f4a363d84b1ac741025e3d6828`. Details in note 142.

### D19D - Interim: Esc backs out of sub-menus; PC-friendly menu navigation

**Done (accepted 2026-10-18).** User: "amazing work". Note [144](documentation/144-d19d-menu-keys.md). **Opened 2026-10-18.** User request: allow Esc to exit sub-menus, and
improve navigation where the mapped PS1 controls fall short (for example
having to know which PS1 button means Back or Confirm on the keyboard).
Interim for the original menus, before D19.

Map out the frontend menus (title, options, load game, difficulty and any
others, plus the in-game pause menu) and which PS1 button each uses for back
and confirm. Then give Modernized PC keys consistent meaning: Esc backs out
of a sub-menu (and does what Back does at the top level, never quitting the
game by surprise); Enter confirms; arrow keys move; consider Backspace as an
alias for Back and mouse wheel/click where it is safe. Keep the existing Esc
behaviour in gameplay (D04A pause/mouse release) and the escape-hatch keys.
Show the correct key names if prompts are drawn by us.

**Acceptance:** in every frontend menu and sub-menu, Esc goes back one level
and Enter confirms (tested per screen); no regression of Esc in gameplay,
pause or the F7 savestate menu; Vanilla unchanged; `GAME_MANUAL.md` lists the
menu keys.

### D19E - Menu responsiveness close-out: loading spinner and a full menu sweep

**Done (accepted 2026-10-18).** User: "excellent work". Note [143](documentation/143-d19e-menu-closeout.md). **Opened 2026-10-18.** User report after D19C: menus are more
responsive while the attract demo plays behind them, but **less responsive
while the loading spinner plays** (the D17U spinning-logo screen between demo
scenes, which the frontend draws while a menu can be open). Fix that case,
then sweep the entire original menu system for the best responsiveness and
close out the interim menu work before our own menus (D19/D19B).

Starting points: D19C drew frontend in-betweens from the newer image of each
pair; D17U loading images still use their own path (`submit_job`'s loading
branch draws from the older image with the logo angle interpolated, world
delay `newest_frontend()` from the loading interval), and the tap hold
(`frontend_input.cpp`) applies in mode 0 only. Measure first: use the D19C
`render_replay` -> `plugin.frontend.menu` latency diagnostic (it only pairs
frontend images today; extend it to loading images and gameplay-mode loads)
and compare D17T/D19C/D17U off switches (`DNTTK_FRONTEND_NEWEST`,
`DNTTK_LOADING_INTERP`, `DNTTK_FRONTEND_INTERP`). Find out what the menu
does while the spinner shows (does the tick still read the pad, does the
state handler run, is the menu drawn at all, are presses lost or delayed by
the load), and whether the loading delay or the logo path adds latency.

Sweep: every frontend screen and transition - PRESS START, main menu, ONE
PLAYER / difficulty and the path into gameplay, TWO PLAYER, LOAD GAME
(empty and with a private test card save), OPTIONS and each sub-screen, the
in-game pause menu and its sub-menus, the statistics/level-end screens -
measuring press-to-screen latency, missed or doubled inputs, and repeat
behaviour on held keys, at 120 and 180 and at 60/Vanilla. Fix what can be
improved without changing the guest's pace while the demo or a load runs;
record what cannot. Keep D19D (Esc/Enter mapping) separate unless the user
merges it.

**Acceptance:** menus over the loading spinner at least as responsive as
before D17U/D19C (measured) and ideally as responsive as over the demo; a
per-screen latency table for the whole menu system with no missed or
doubled inputs; smoothness, the attract demo, loading time, gameplay, 60 and
Vanilla unchanged; the user judges the feel and closes out the menu work.

**Result (2026-10-18, implemented):** while a demo scene loads behind a menu
the tick first blocks in the CD load for about 1.8 s (the original hides the
menu), then runs with gaps of up to 45 fields under the spinning logo. The
D17T 6-field tap hold lost taps there (24 of 30 landed), and it lost 11 of 11
Cancel taps on LOAD GAME with a saved game (card polling starves pad reads).
Loading images were also redrawn from the older image (191-220 ms press to
screen). Now a Modernized frontend press is held until the guest's pad
history `0x800d1aa4` shows the tick read it. Taps made meanwhile are queued
(up to 6) and delivered with a read release between them, and contact bounce
is ignored. Menu loading images are drawn from the newer image. Result: 90
of 90 taps over the spinner at 120 (and 30/30 at 180 and 60), 100-113 ms
press to screen over the spinner. Every screen in the sweep (main, difficulty,
two player, options, sound, controller, game settings, LOAD GAME with a save,
PRESS START, pause, statistics, SAVE GAME) took every tap, one step each.
Demo-backed menus, smoothness, load timing, gameplay entry and Vanilla are
unchanged. Executable
`0092e2b261b333e0f1848e77b6a6a824079efbaba886e2fb158b3b9023e51aaf`. Per-screen table in note 143.

### D19F - PC button icons in the exact style of the original prompts

**Todo (opened 2026-10-18).** User request after the D19 menu mockup. The
original prompts (SELECT, CONFIRM, CANCEL, CHANGE, DEFAULT...) show PS1 button
sprites: 16x16, 16-colour, from the HUD sprite table in `SLUS_005.83`
(Up/Down `0x800c4474`, Cross `0x800c4414`, Square `0x800c4424`, Circle
`0x800c4404`, Triangle `0x800c43f4`; cells and CLUTs in `/DATA/FONTS.RAW`,
extracted by `tools/local/build_ttk_fonts.py` `ui_pack()`). CHANGE is the
Up/Down sprite turned sideways; the L1/R1/L2/R2 bars on CONFIGURE CONTROLLER
are not yet identified. Since D19D a Modernized player presses Enter, Escape,
Backspace, V, C, the arrows and the wheel, but the prompts still show PS1
buttons.

Make a PC set replicated in the absolutely exact style of the originals:
same 16x16 cell and baseline, the same light direction, bevel, outline,
metal finish and colour count, readable at the game's prompt scale. At
least: Enter, Escape, Backspace, Space, the arrow keys (up/down pair and
left/right pair, matching SELECT and CHANGE), V, C, left/right mouse
button and the wheel, plus a generic keycap that can carry a key name for
bindable actions. Draw them with the Photon Studio plugin (photon
vector/edit skills) and export indexed PNGs; the user may draw some
themselves.

Provenance decides where they live: art drawn fresh by the project can be
tracked in `recomp/assets/ui/` (like `item-frame.png`, with a README row);
anything built from disc pixels (for example the original round button
base) must instead be composed at build time from the player's disc, like
D24A. Decide this before drawing.

Then use them: Modernized menu prompts show the PC icons for the keys D19D
maps (a setting can keep the PS1 icons, and D09 controllers keep them),
and the D19 menus use the same set. Vanilla keeps the original prompts.

**Acceptance:** the user approves the set side by side with the original
sprites at 1x and at in-game scale; every Modernized menu prompt shows the
key that actually does that action; Vanilla and the controller prompts
unchanged; provenance and licence recorded.

### D19F1 - Prompt icons follow the input device in use

**Todo (opened 2026-10-18).** User request: once controller support (D09)
can tell keyboard/mouse input from controller input, the menu and in-game
prompts switch icons automatically, like any modern game: the D19F PC
icons after a key, click or mouse move, the PS1 button sprites (or the
connected pad's own layout, if D09 adds one) after a controller press or
stick move. Switch on the last device actually used, with a small dead
zone so a resting stick or a stray mouse twitch does not flicker the
prompts; change between images, never mid-draw. A fixed choice
(automatic / keyboard / controller) stays available as a setting.
Vanilla keeps the original PS1 prompts.

**Acceptance:** with a keyboard and a controller both connected, every
Modernized prompt shows the device last used and changes within one image
of switching; no flicker from idle devices; the fixed settings hold;
Vanilla unchanged.

### D19G - In-game OPTIONS stage 2: key bindings and live display/camera

**Done (user-accepted 2026-10-20: "looking good!! lock it in and close this job out").** PC CONTROLS > KEY BINDINGS: 43 actions in
five amber-headed groups; Enter, then a key or mouse button binds it at once
and saves it (a taken key swaps, Escape cancels, Backspace clears, V asks
RESTORE DEFAULT KEYS?). Now live on CONFIRM: mouse sensitivity and invert,
display mode, widescreen (a plain window takes the new shape), camera, view,
shoulder, camera distance, weapon aim, and frame rate unless launched at 60.
Design approved from a mockup made of the disc's own font, CLUTs and button
sprites over a live capture, composed in Photon (local, retail-derived:
`recomp/analysis/d19g-bindings/`). Executable
`0d42f4523da0c8ec32abd470a6ccc03f1e84ec7dc5bc905265ea33cd3f3b03da`,
[note 159](documentation/159-d19g-options-stage-2.md). Not verified: the real
display (borderless / exclusive, window reshape under a window manager) and
the user's feel. **Opened 2026-10-18.** Follow-up to D19 stage 1 (note 145). Add a KEY
BINDINGS screen under PC CONTROLS in the same style (pick an action, press a
key or mouse button, swap on conflict, DEFAULT restores; bindings saved like
`run.py --bind`), and apply the NEXT LAUNCH options live where safe (camera,
view, shoulder, distance, weapon aim, mouse, widescreen, frame rate, display
mode).

**Acceptance:** bindings changed and restored in game take effect at once and
persist; each newly live option changes without a restart; Vanilla
unchanged; the user judges the feel.

**Revised 2026-10-20 (user review):** labels follow what the keys do: the
`inventory` action is **CROSSHAIR** (I toggles the crosshair; the inventory
screen is Right Shift), **USE ITEM** shows **ENTER / U** (Enter always uses
the item), the weapon keys are named after the first weapon of each group
(MIGHTY BOOT ... FREEZER), PREVIOUS / NEXT ITEM show **[** and **]** (built
from the menu font's own I and hyphen; the font has no brackets), and a
one-line hint explains GRAB 2 (a second grab key, Alt by default), CROSSHAIR,
USE ITEM and the stacked weapon groups. New backlog from the same review:
D07F, D10C, D19G1.

### D19G1 - FRAME RATE live after a 60 launch

**Done (user-accepted 2026-10-20: "it works! i can confirm it works").** FRAME RATE in the in-game
OPTIONS now starts replay presentation mid-session from a 60 launch, and 60
stops it from any launch (back to the original present path); software
renderer rows say NEXT LAUNCH. Executable
`bbfaf117ca261268335c0c19847663a3c02ac7185d24c77fda9ca81f38eecc29`, note
[164](documentation/164-d19g1-live-frame-rate.md).

Opened 2026-10-20. A game started at 60 has no replay
presentation, so FRAME RATE says NEXT LAUNCH there (D19G). Start replay
presentation (and its render workers) mid-session when the player picks
another rate, and stop it again at 60, without a hitch or a change to the
60 Hz baseline.

**Acceptance:** from a 60 launch, 120 / 144 / Display apply without a restart
and back to 60; the 60 presentation is unchanged until a change is made;
Vanilla unchanged.

### D19H - In-game pause menu rebuilt like the title menus

**Done (user-accepted 2026-10-19: "accepted!! this works and it's functional").**
Follow-ups: D19I (dimmed text), D19J (main menu), D19K (integrated LOAD
GAME), D19L (menus during demo loads). Implemented 2026-10-19. Layout B chosen by the user
(2026-10-19). In Modernized, Escape opens RESUME / MISSION BRIEFING / OPTIONS
/ LOAD GAME / RESTART LEVEL / MAIN MENU under the level name; the death screen
lists CONTINUE / LOAD GAME / RESTART LEVEL / QUIT (challenge stages: REPLAY
LEVEL / GIVE UP), all drawn by the game's own calls on the free screen values
8 and 9, with YES / NO prompts. Every choice runs exactly what the original's
handler and main loop do with it (no one-frame flash of the old screen); the
original LOAD screens are used as they are. Menu taps are counted from key
events, so short taps register on the slow death screen. Also fixed: D19's
glyph-table guard covered two menu variables, so the D19 menus fell back to
the originals in some states. Offscreen: every choice, the challenge
variants, Vanilla and the title OPTIONS match the originals. Executable
`681ae485de9d04bbd91949f4811c7d62c65859d486414a51e94371ef5adc1ca2`. Note
[147](documentation/147-d19h-pause-menu.md). Not verified: the real display,
the death list from captured play with a real mouse, LOAD with a save.

**Continues (user 2026-10-19):** CONTINUE keeps the original's confirmation
(CONFIRM CONTINUE, YES first) for now. The user expects to scrap the
continues system later; when that happens, update the death list, its
heading and this job's note.

**Opened 2026-10-19.** User 2026-10-19: the menus
before gameplay now work well because D19 rebuilt them from the game's own
calls; the menu Escape opens during gameplay must work the same way and "is
unusable right now (without modern controls)". Today the pause menu
(mode 1, screen `0x800be56a` = 12) is the mission briefing with four
PlayStation prompts and no cursor (Cross RESUME, Circle LOAD, Square OPTIONS,
Triangle QUIT, then a RESTART / MAIN MENU / CANCEL prompt). A PC player must
know that C is LOAD and V is OPTIONS (D19D), and Escape means RESUME there
rather than "back". The death screen (`0x800be56a` = 15: CONTINUE / LOAD /
RESTART / QUIT with confirmation prompts) has the same problem.

Scope:

- A pause menu drawn like OPTIONS (note 145: the game's own text, sprite and
  sound calls, on an unused screen slot, no host drawing): a vertical list
  with the blue highlight, held-key repeat, mouse wheel, SELECT / CONFIRM /
  RETURN prompts. Items: RESUME, OPTIONS, LOAD GAME, RESTART LEVEL, MAIN MENU.
  Each item reaches the original's own path (RESUME, `0x800be56a` 2 / 5, the
  QUIT prompt's restart and main-menu code), so saving, loading and quitting
  stay the game's.
- The mission briefing (level name, story text, goals) stays available.
  Layout chosen from the design artifact.
- RESTART LEVEL and MAIN MENU ask before acting (NO highlighted). Escape on
  the pause list resumes, and inside any sub-screen or prompt it goes back
  one level. Enter confirms. Fixed keys that do not depend on bindings or on
  modern-control capture (F10 on or off, Legacy controls or Modern controls).
- The death screen gets the same list: CONTINUE (with the remaining count),
  LOAD GAME, RESTART LEVEL, MAIN MENU. Escape does nothing at the top level
  (there is nothing to resume) and cancels in prompts.
- Vanilla keeps the original pause and death screens. A code-identity check
  falls back to the originals on a mismatch, as D19 does.

**Before implementation:** an interactive design artifact (layout, items,
prompts, keys, death screen) for user review.
Published 2026-10-19 as a second artboard, "D19H in-game pause and death
menus", on the D19 mockup canvas (private,
https://claude.ai/artifact/6FxFwzfzdCp2doDjSesfHA, not tracked: disc-derived
glyphs). Tweaks: layout A (briefing above the list) or B (list first,
MISSION BRIEFING as an item), death screen, PC-key prompts. The user chose
layout B and kept CONTINUE's confirmation.

**Acceptance:** from captured play, F10-released play and Legacy controls,
Escape opens the list with RESUME highlighted; every item works with arrows,
Enter and Escape only (no PlayStation-button knowledge); OPTIONS and its
screens, LOAD with a saved game, RESTART and MAIN MENU reach the same
results as the originals; the death-screen list continues, loads, restarts
and quits; taps register once at 60/120/180; savestates taken on the new
screens load safely; Vanilla unchanged; the user judges it on their display.

### D19I - In-game menu text dimmed

**Done (user-accepted 2026-10-19: "it's working as intended").** Implemented 2026-10-19. Cause confirmed: our pages were
drawn at the view composition, into the world's drawing chain, so the paused
world's darkening quad (`0x80025f28`, drawn in the chain after it) covered
them and nearer world polygons (Duke) were drawn over them. In gameplay they
are now drawn in the pause-dispatch tick, after the quad, where the original
draws its pause text; a final choice draws the page before acting, so RESUME
never shows an image without the list. The title menus still draw at the
composition. Measured (brightest tenth of the text pixels): pause list
135, 101, 42 -> 255, 187, 68; briefing 135, 95, 25 -> 255, 176, 34, the
original briefing's exact value; prompts, pause OPTIONS, settings, death
list and CONFIRM CONTINUE all at 255 red like the original; paused world
darkening unchanged. 120 Hz present sequences: no image without the text.
Executable
`f99b46632c62d1ce43a674b3a494c8246dc5513c0c1b2651d3692db12a557f9c`, note
[148](documentation/148-d19i-menu-brightness.md). The user judged it on
the real display.

**Opened 2026-10-19.** User at D19H acceptance: the text "is dimmed and
needs a touch of work to make it look right".

Research (2026-10-19, D19H screenshots at 16:9, 4x): the brightest tenth of
the text pixels averages RGB (133, 101, 58) on our pause list, death list,
briefing and the D19 pause OPTIONS, against (254, 190, 74) for the
original's pause text in the same scene and (252, 208, 107) for D19's title
OPTIONS. So the same text calls give full brightness on the title and half in
gameplay. Leading cause, to be confirmed: before the pause dispatch the main
loop calls `0x80025f28`, which sets up a full-screen quad (`0x800be4f8`) and
inserts it into the ordering table at depth 1 (`0x8002bc18`), the darkening
over the paused world. The original's pause text is drawn inside the
dispatch, after that quad. Ours is drawn at the composition hook
(`0x80026164`) earlier in the tick, so it ends up under the quad.
Experiments: draw the gameplay pages from the dispatch tick hook (after
`0x80025f28`, before the D17 capture, once per image), or put our text at the
original's ordering-table depth. Check the D17 redraws keep the result.

**Acceptance:** text on the pause list, death list, briefing, prompts and
pause OPTIONS and its settings screens matches the original pause text's
brightness and colour (measured and judged on the real display); the paused
world stays darkened; title menus unchanged; Vanilla unchanged.

### D19J - Title main menu re-implemented

**Done (user-accepted 2026-10-19: "fully accept").** Implemented 2026-10-19. In Modernized, PRESS START
leads to our main menu in the original's look: ONE PLAYER, TWO PLAYER, LOAD
GAME, OPTIONS, QUIT GAME (QUIT GAME? YES / NO, NO first; YES closes the
program). CHOOSE DIFFICULTY LEVEL, TWO PLAYER OPTIONS and CHOOSE CHARACTER
are ours too, drawn with the originals' calls, text ids and positions on
state 12 with the free sub-screen 12, keeping their state in the originals'
variables. A new entry hook on the frontend tick (`0x80022324`) replaces
states 6-9 before their handlers draw. Starting a game hands the last page
to the original handler with one Cross press; it draws the same image and
starts the game itself. CHOOSE CHARACTER lets one keyboard choose for both
players (player 1, then player 2: the PC build has no second pad, so the
original could not get past it). LOAD GAME stays the original screen
(D19K). Idle timeout back to PRESS START kept. Framework fix found on the
way: a savestate admitted at an interrupt return inside a plugin's guest
call longjmped out of it and crashed the run; saves and loads now wait for
`psx_mod_host_call_end()`. Offscreen: every page, both game starts (same
stored results as the original handlers), LOAD with a save, OPTIONS, QUIT
GAME exit 0, idle, taps 23/23 and 31/31 through demo loads at 60/120/180,
no original-menu image, 12 saves / 4 loads on our pages, Vanilla original.
Executable `a0246cadbcdb5ee4719dc6e5b08bfa8a0945cf696e47dd22e4a1af27e6873a3d`,
[note 150](documentation/150-d19j-title-menu.md). Not verified: the user's
display; two player past its start. Seen in both builds: D19O.

**Opened 2026-10-19.** User: "completely replace the main menu with a
re-implementation of it, just like we did options ... exactly the same stuff,
likely, as how we just accomplished the in-game menu."

Scope: the frontend's PRESS START / main menu (ONE PLAYER, TWO PLAYER, LOAD
GAME, OPTIONS), CHOOSE DIFFICULTY LEVEL, the TWO PLAYER OPTIONS and CHOOSE
CHARACTER screens, drawn with the game's own text, sprite and sound calls
(notes 145 and 147) over the attract demo. Each choice runs the original
handler's own result: no handler draw for a frame, nothing host-drawn, and
the code-identity fallback to the originals.

Research (2026-10-19): the frontend tick `0x80022324` dispatches the state
`0x800d24e4` through table `0x800131b0` (13 states: 5 PRESS START over the
demo, 6 opening the menu, 7 main menu with cursor `0x800d24e6`, 8 ONE PLAYER
/ difficulty (`0x800c3d7c`), 9 CHOOSE CHARACTER (`0x800dd1b0`), 10 LOAD GAME,
12 OPTIONS with sub-screen `0x800d24d2`; handlers `0x80022394`-`0x80023240`).
D19 already owns state 12 through the hook at `0x800220f4`. The main menu
needs a free state or sub-state that the frontend leaves alone, and the
transitions (6 -> 7, the difficulty start into mode 1, two-player setup)
reproduced from the handlers, as D19H did for the pause menu. The attract
demo must keep running behind (D17T, D19C: it desyncs if the frontend ticks
faster). Escape, Enter and the wheel follow D19D.

**Acceptance:** every title choice works with arrows, Enter, Escape and the
wheel and reaches the same result as the original (one player at each
difficulty, two player, LOAD GAME, OPTIONS), plus D19M's QUIT GAME (QUIT
GAME? YES / NO, the same `SDL_QUIT` close); taps are never lost (D19E
numbers or better); no frame of the original screen shows between screens;
the attract demo is unchanged; Vanilla keeps the original; the user judges it.

### D19K - LOAD GAME from our savestate menu, inside the menus

**Done (user-accepted 2026-10-20: "confirmed now working as intended. lock it in").**
Executable `37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9` is the regression baseline. Follow-ups: D19Q (SAVE
GAME screen), D19R (startup card probe hang).

Implemented 2026-10-20; [note 169](documentation/169-d19k-load-game.md).
Design: layout A of the private D19K mockup canvas (user, 2026-10-20: "it's
got to be A!"). LOAD GAME on the title, pause and death lists opens our LOAD
GAME page: the 12 savestate slots in the main menu's font (number and level
name; EMPTY and saves from another build greyed), opening on slot 01 (revision 2),
and beside it the chosen slot's thumbnail (a host image placed on the game
picture), era and save time. Enter asks LOAD THIS GAME? (thumbnail, SLOT NN -
LEVEL, and in game UNSAVED PROGRESS WILL BE LOST, revision 1); Enter loads
the slot as F7 does, Escape goes back (to the list on LOAD GAME). Defaults
taken for the open questions: details are LEVEL / ERA / SAVED only (no
difficulty, health or armor); memory card saves stay Vanilla's; the
statistics screen's SAVE GAME is left for D20. Framework: a thumbnail draw in
both present paths, and `boot_state_check_buffer` no longer reports every
slot compatible when the caller passes no reason buffer. Offscreen (private
cards and savestates, 120 Hz, 16:9 and the 4:3 title): pause, title and death
lists open the page and load (pause and death: level 0 from slot 03; title:
mode 1 level 0); an edited codegen hash shows CANNOT LOAD and Enter does
nothing; a body-damaged slot fails, keeps the game and shows THIS SAVE COULD
NOT BE LOADED; a fresh save shows its level name and is selected; keys behind
the LOAD GAME press are kept; Escape returns on LOAD GAME; Vanilla reaches the
original memory card LOAD. Python 160 OK (12 skipped), `ttk-input-test`,
`ttk-live-env-test`, `check_repo.py`; pristine pin plus the exported patch
reproduces all 47 framework files. Executable
`2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`. Not verified: the user's display,
real mouse capture after a load from the title, two-player.

**Work log (2026-10-20, revision 2, Needs playtest).** User: "superb,
everything is working as intended! ... item 1 should be selected by default."
LOAD GAME now always opens on slot 01 (it opened on the newest save, slot 10
in the user's list). Offscreen: pause and title open on 01. Executable
`37966f924ec6348e1dfcb4a8ce17c5ad320c8b3535c40c4f8610a542abd2afe9`.

**Work log (2026-10-20, revision 1, Needs playtest).** User: "The
loading-screen work is looking absolutely fantastic"; three asks: (1) the
date / time in Console steel TTK Medium Italic, (2) remove the startup memory
card checks from Modernized without losing real initialisation, (3) a new
SAVE GAME backlog job (D19Q, below); plus, mid-turn, "progress since your last
save will be lost could also be console steel, it looks dark too".
- (1) The save time and the in-game warning are drawn by the host in the
  console's set (TTK Medium Italic, Console steel, navy drop shadow), one pixel
  of extra letter spacing, at 2x in a 960x720 composition of the 4:3 menu area
  (a third larger than the console's own size, so it reads beside the menu
  font). The warning is now UNSAVED PROGRESS WILL BE LOST (the long line was
  wider than the menu area at that size). Thumbnail and text are one host
  image (alpha-blended in GL). Without the font pack the guest text stays.
- (2) Investigation: frontend state 1 shows CHECKING MEMORY CARD and probes
  the card (`0x80082e6c`); state 2 shows CHECKING SAVE GAME FILE, reads the
  file (`0x80083490`) and unpacks the saved settings (`0x80083070`: GORE,
  STATUS BARS, GAME SPEED and pad layouts, which D19N keeps on the card);
  state 3 is the no card / not formatted / no free block message, left with
  CONTINUE for state 4 (PRESS START). In Modernized the card work runs
  unchanged, both CHECKING texts are not drawn (an entry hook on the text call
  `0x8008b454` points those two strings at their empty end, only from states 1
  and 2; hook added to `game.local.toml`, regenerated, codegen hash unchanged
  `0x8bab543c`), the original's tick waits before the probe and the read are
  skipped, and state 3 continues at once as its CONTINUE does. The title
  heading stays, as on PRESS START that follows. Cold boot with the private
  card: no CHECKING text, the settings unpack ran (`card_loads` 1), state 1 to
  PRESS START 2.4 s (state 1, the card probe, is about 1.8 s of black screen,
  the emulated card itself, as before). Blank (unformatted) card: no message,
  PRESS START 0.6 s after state 1; Vanilla shows the original NOT FORMATTED
  message with CONTINUE.
- Regression: pause, title and death loads; incompatible slot; tests (Python
  160 OK, `ttk-input-test`, `ttk-live-env-test`, `check_repo.py`); the pin plus
  the exported patch reproduces all 47 framework files. Executable
  `2856af4971e912b38805cf51e43fa949c65f197e721abbcf147a410cd31c9d46`.

**Opened 2026-10-19.** User: access to "our load game menu, but more
integrated into this menu rather than feeling like a totally separate
screen. This will almost certainly replace the memory card based load game
entirely", which the user considers safe.

Scope: LOAD GAME on the title menu (D19J), the pause list and the death list
(D19H) opens a load screen in the same menu style that lists our savestates
(the D24B F7 slots: thumbnail, slot, level name, time), drawn as part of
these menus rather than as the separate F7 panel. Choosing a slot loads it
(the existing savestate path); Escape returns to the list on LOAD GAME.
In Modernized the memory-card LOAD GAME (`0x80086e8c`, frontend state 10,
pause screen 5, death prompt 3) is no longer offered.

Research (2026-10-19): all three LOAD GAME entries call the same original
handler `0x80086e8c` (from `0x80023200`, `0x800260e8` and `0x80086af0`).
Savestates already restore from any mode (F7, level select). To settle:
- how saving fits: the statistics screen's SAVE GAME still writes the memory
  card; whether it should write a savestate slot instead (see D20, save
  management);
- how existing memory-card saves stay reachable (keep them in Vanilla only,
  or an import);
- drawing the thumbnails inside the guest-drawn menu (host overlay or
  VRAM upload) without breaking the D17 redraws.
Never touch the player's own cards or savestates in testing.

**Acceptance:** LOAD GAME on the title, pause and death menus lists the
savestates in the menu style and loads the chosen one; empty and incompatible
slots are handled; the memory-card LOAD is gone from Modernized and still
works in Vanilla; the user judges it.

### D19L - Menus stay up while demo scenes load

**Done (user-accepted 2026-10-19 with D19O; implemented with D19O round 2, [note 155](documentation/155-d19o-menu-flashback.md#round-2-the-users-playtest-2026-10-19)).** User: "menus work seamlessly during load
screens between demos, just like our modernized menu"; expected to follow
partly from D19J.

Research (D19E, note 143): when a demo scene ends behind an open title menu,
the frontend loads the next scene in sub-state `0x800d24e2`: in sub-state 0
(about 1.8 s) the load setup `0x8002334c` blocks in the CD load and only the
loading logo is drawn (no tick, no pad read, **the menu disappears**); in
sub-state 1 (about 6 s) the menu shows over the spinning logo but some ticks
are 10-45 fields apart. D19E made presses wait rather than vanish, but the
menu still vanishes for sub-state 0 and lags over the spinner. With our own
menus (D19J), the host can keep drawing the current screen from the
loading-logo hook (`0x80025554`, already used by D19) and handle navigation
there; only actions that change the guest state wait for the next tick.
Alternatives to measure: keep the current demo scene looping (or paused)
while a menu other than PRESS START is open, so no load happens.

**Acceptance:** with the title menu open across demo scene loads, the menu
never disappears, the cursor moves at the D19E menu rate during the load and
spinner, and choices made then act as soon as the guest can; the demo and its
timing are unchanged; Vanilla unchanged.

### D19M - QUIT GAME closes the program

**Done (user-accepted 2026-10-19: "perfect!! thats working").** Implemented 2026-10-19. QUIT GAME is the last item of
the pause list (after MAIN MENU) and of the death list (after QUIT, or GIVE
UP in the challenge stages). It asks QUIT GAME? with NO highlighted; NO and
Escape return to the item; YES pushes `SDL_QUIT`, the window close button's
own event, so the main loop runs its usual shutdown (exit 0) and run.py
folds in the confirmed settings. User choices 2026-10-19: the death list gets
QUIT GAME and keeps its QUIT (game over); the title main menu gets the item
in D19J's re-implementation, not as an interim patch on the original menu.
Evidence (offscreen, private profile/cards/savestates,
`recomp/analysis/d19m-quit`): pause YES exit 0 in 0.23 s, death YES exit 0
in 0.18 s, Escape and NO back on QUIT GAME, a setting confirmed in PC
CONTROLS kept in the profile after QUIT GAME; regressions MAIN MENU YES
(mode 3), challenge GIVE UP (statistics, then the game goes on), Vanilla
original pause. Executable
`de5a152192ba04979e408a436c5f5541f72c6c36aade7e96d4091c86a7937d43`,
[note 149](documentation/149-d19m-quit-game.md). Not verified: the real
window on the user's display. Remaining: the title main menu item (D19J:
accepted 2026-10-19).

**Opened 2026-10-19.** User: "add a quit menu option, which closes the
entire program". Today the only menu exits are MAIN MENU (pause list) and
QUIT on the death list, which goes to the original game over screen; closing
the program needs the window's close button (D19D made Escape never quit).

Scope:

- A QUIT GAME item at the end of the pause list (after MAIN MENU) and on the
  title main menu (in D19J's re-implementation, or as an interim item on the
  original main menu). Optionally on the death list: decide with the user
  (its QUIT already means "give up to game over").
- REALLY QUIT? style YES / NO prompt with NO highlighted, as the other
  destructive choices.
- YES runs the same clean shutdown as closing the window: confirmed settings
  written (D19 `DNTTK_MENU_SETTINGS_FILE`, folded in by run.py), audio and
  renderer shut down, no crash trace. Never touches saves or memory cards.
- Vanilla keeps the original menus (no item there unless the user asks).

**Acceptance:** from the pause list and the title main menu, QUIT GAME, YES
closes the program cleanly within a moment (exit code 0, settings kept), NO
and Escape return to the item; the death list as decided; Vanilla unchanged.

### D19N - One OPTIONS screen; the rest under ADVANCED

**Done (user-accepted 2026-10-20: "lock it in and close it out!!").** Layout A built as approved: OPTIONS is
SOUND (original), CONTROLS, DISPLAY, GAMEPLAY and ADVANCED (one list under
amber headings). AUTO AIM (profile, default OFF; ADVANCED > ORIGINAL GAME
beside RED DOT) replaces AIM ASSIST and is written into the game; GORE, STATUS BARS and GAME SPEED are the original's
values in game memory (card as before); the original CONTROLLER is gone from
Modernized, which forces the default pad layout; DISPLAY row is WINDOW;
dependent rows are dimmed with what they need. A card copy keeps Vanilla's
AUTO AIM and pad layout in saves made in Modernized. Two hooks regenerated
(`0x80082F64`, `0x80089920`). Executable
`6b7f5415dd0f18044358548e80661ca8aa91606a44ea8b1ad4e1417a40cd7a33`,
[note 166](documentation/166-d19n-options-layout.md). Opened 2026-10-19. User: consolidate "some of the existing vanilla
menus into the modernized menu, as i think there are likely some unnecessary
or possibly conflicting crossovers, so the result wont be called Modernized
menu and it will just be like an options screen, and then all the other stuff
will be advanced".

Current OPTIONS (D19, note 145): the original SOUND, CONTROLLER and GAME
SETTINGS screens, then our PC CONTROLS, DISPLAY and MODERNIZED.

Research (2026-10-19, first pass):

- GAME SETTINGS (original): AUTO AIM, GORE, STATUS BARS, GAME SPEED slider.
  AUTO AIM overlaps MODERNIZED WEAPON AIM / AIM ASSIST / RED DOT (D07B-D07D):
  with view aim, the original auto aim and our AIM ASSIST ORIGINAL LOCK may
  fight or mean the same thing. STATUS BARS overlaps our HUD work (D08A
  boxes). GAME SPEED is the original's own speed setting, unrelated to CPU
  SPEED or FRAME RATE but easy to confuse with them.
- CONTROLLER (original): CONFIGURE CONTROLLER assigns PlayStation buttons;
  with PC CONTROLS and the key bindings planned in D19G, two places set
  controls. Keep the original only for gamepads (D09) or fold it into one
  CONTROLS screen.
- SOUND (original): volumes; D21 plans more sound settings. No conflict found
  yet.
- MODERNIZED mixes everyday choices (CAMERA, VIEW, CROSSHAIR) with
  per-feature original/modern switches (JUMP WALLS, LANDING, DYNAMITE,
  STEROIDS, JETPACK). DISPLAY mixes everyday choices (RESOLUTION, DISPLAY,
  WIDESCREEN, FRAME RATE) with technical ones (CPU SPEED, CPU TIMING,
  GEOMETRY, TEXTURES, RENDERER).
- Write-back paths differ: the original screens store into game RAM (and the
  memory card's settings), ours into the profile. Merged rows must keep one
  source of truth.

Scope:

- Before implementation: a full table of every setting (original and ours),
  what it controls, overlaps and conflicts, and a proposed layout, published
  as a design artifact for the user to choose from. Likely shape: OPTIONS
  (SOUND, CONTROLS, DISPLAY, GAMEPLAY with the everyday rows) and ADVANCED
  (per-feature original/modern switches, renderer and CPU rows, PROFILE).
  "MODERNIZED" no longer appears as a screen name.
- Conflicting settings resolved (one row, or one disabled by the other with a
  description saying why), never two rows silently fighting.
- Same menu style and keys as D19; title and pause OPTIONS both.

Design for review (2026-10-20): private artifact
https://claude.ai/artifact/4a4rvMENnieMK9VGNX4Uzr (no retail assets). It lists
all 39 settings, three layouts (A: SOUND / CONTROLS / DISPLAY / GAMEPLAY /
ADVANCED list, recommended; B: ADVANCED as four screens; C: minimal
reshuffle), conflicts C1-C7 and five decisions. New findings: the original
SOUND is SOUND EFFECTS / MUSIC / SPEECH sliders plus MODE; AIM ASSIST
ORIGINAL LOCK only works with the original AUTO AIM on (note 24); Modernized
keyboard play sends fixed PS1 buttons, so a changed CONFIGURE CONTROLLER
layout probably breaks it (code reading, not tested); STATUS BARS vs our HUD
boxes and GAME SPEED's exact effect still need checking in game.

**Acceptance:** the user approves the layout; every setting from the old
screens is reachable or deliberately removed (listed in the note); no two
rows control the same thing; values persist as before (profile and the
original's settings); Vanilla keeps the original OPTIONS.

Work log 2026-10-20 (build): user choices from the design page: 1a layout
A, 2a AUTO AIM in the profile default OFF (card value overridden), 3a
CONFIGURE CONTROLLER removed and the default layout forced, 4a GAME SPEED in
ADVANCED > ORIGINAL GAME, 5a WINDOW. Decoded GAME SETTINGS (word `0x800c3f9c`:
bit 0 AUTO AIM, bits 1-2 GORE, bit 3 STATUS BARS, bit 4 vibration; GAME SPEED
`0x800c3fa0` applied by `0x80089920` to Duke `0x800c3c80` and enemies
`0x800c3d14`), the pad layouts (`0x800d1b60`, default `0x800bda7c`) and the
card pack / unpack (`0x80082f64` / `0x80083070`). STATUS BARS already hides
our HUD boxes (they hook the status bar call the game skips). The grey CLUT
row drew boxes, so dimmed rows lower the glyph modulation instead. Offscreen
(`recomp/analysis/d19n-options`): every screen, SOUND round trip (title and
pause), ADVANCED scroll and headings, dimmed RED DOT / WEAPON AIM, GAME SPEED
40 (Duke 4530, enemies 5280), GORE NONE + STATUS BARS OFF + AUTO AIM ON (no
HUD after resume, profile `original-lock`), statistics SAVE (card gets AUTO
AIM ON, play restored to OFF), death-screen LOAD of it (card copy ON kept),
`--vanilla` untouched. Python 158 OK (12 skipped), `ttk-input-test`,
`ttk-live-env-test`. Not verified: AUTO AIM ON in play, GAME SPEED feel, a
Vanilla pad layout round trip end to end; the user's look on the real display.

Work log 2026-10-20 (review pass): user found the title CONTROLS screen blank
and ADVANCED vanishing at GAME SPEED (title only, at 120). Cause: slider
objects in framework GPU-DMA memory, outside the D17 frontend replay's copy;
now the original SOUND screen's slider objects in guest RAM. Slider rows take
two places in long lists (GAME SPEED no longer overlaps ORIGINAL GAME). AUTO
AIM moved to ADVANCED > ORIGINAL GAME beside RED DOT. USE ITEM is Enter only
(default Unbound; profile schema 36 clears the old U). User note: Duke 3D's U
toggles mouse aiming; keep U free until a closer EDuke32-like use is chosen.
Offscreen at 120 (title and pause) and tests pass; note 166.

### D19O - Frontend redraws flash back to the previous menu image

**Done (user-accepted 2026-10-19: "accepted! lets bring this one to a solid close"; opened 2026-10-19, found in D19J).** On the title, after a menu
change the screen shows the new image, then the previous real image again
for up to one guest image, then the new one. Measured at 120 Hz offscreen
(`recomp/analysis/d19j-title`, note 150): PRESS START -> main menu shows the
menu for 6-7 presents, PRESS START again for 13, then the menu, in the
original menus (`DNTTK_GAME_MENU=off`) and in D19J alike. A cursor move on
the original main menu: the new highlight, then the old one for 13 presents;
on D19J's menu one present. Likely cause, to be confirmed: the D19C
newest-image redraws (`frame_replay.cpp`) present the newer captured image
before the older image's real present.

**Acceptance:** after a menu change no present shows the older image again
(present sequences at 60/120/180 over the demo and the loading spinner);
menu latency no worse than D19E; the attract demo's motion unchanged;
Vanilla unchanged.

**Implemented 2026-10-19 ([note 155](documentation/155-d19o-menu-flashback.md)).**
Cause: the flash follows a replay timeline reset, not every menu change. D19J
measured it straight after the 0.8 s stall its own back-to-back dumps cause;
steady play is clean. A reset happens after a host stall and at every switch
between demo and loading-logo images. It dropped every image, so for about
100 ms the presenter showed the real image, which shows a menu change one to
two guest images after the D19C/D19E redraws (a cursor move 22 presents
later). `frame_replay.cpp` now redraws the newest menu image on its own until
a pair exists and keeps the last redraw across such a reset (mode 0 only;
`DNTTK_MENU_ALONE=off` restores the old fallback; `plugin.frontend.kept` /
`.alone`). Framework diagnostic: `replay_dump <base>-seq96` (96 presents, one
sequence, no stall inside it). Evidence (executable `22671989...5161`): SIGSTOP
stalls at the menu change, 8 runs at 120/180 on both menus, no flash and no real
image (off: 6 of 6 flash); dump-stall method 0 vs 13 presents; demo scene load
switches 0-3 real presents outside the hidden-menu load (off: 9-34); steady
PRESS START and cursor moves clean at 120/180 (60 has no redraws); latency
106 ms mean press to screen (off 112, D19E 110); 118-120 / 178-179 distinct
presents a second; gameplay and Vanilla unchanged; tests pass. Not verified:
the user's display.

**Round 2 2026-10-19 (user playtest: judder at scene changes and loading
screens; held Down stops when the demo scene changes).** Measured on D19J's
menu: at the blocking load -> logo and logo -> demo hand-overs the guest
builds an image twice and our menu drew only the first pass (menu gone 12-13
and 24 presents); the logo paused more than 15 fields there and the real
image came back; held keys did nothing for the 1.9 s blocking load (no pad
read) and then ran several items at once. Fixed in `game_menu.cpp` (draw per
call site; keys read and title moves run at the load loop; repeat goes on
from now), `pc_input.cpp` (`input_menu_poll`) and `frame_replay.cpp`
(`menu_hold`). This is also D19L's acceptance (menu never disappears, cursor
moves during the load, choices act at the next tick, load timing unchanged);
recorded there. Executable `114a7753...38b2`; a permanent held-key stop did
not reproduce (X11 auto-repeat on Xvfb, 5 minutes from a cold boot). Not
verified: the user's display.

### D19P - Death screen: QUIT TO MENU and QUIT GAME

**Done (user-accepted 2026-10-19: "lock it in ... it's great").** The death list reads CONTINUE, LOAD GAME,
RESTART LEVEL, QUIT TO MENU, QUIT GAME. QUIT TO MENU asks QUIT TO MENU? (NO
highlighted); YES goes through the game over screen to the title (user
choice). MAIN MENU, GIVE UP and QUIT GAME unchanged (user choice). Found
while testing and fixed: a D19 identity guard covered the selected item
`0x800c3f94`, so a session whose first menu use came with an item selected
kept the original menus. Evidence: level 0 (slot 8 and a fresh death) full
flow to the title; level 1 shows the new list after the guard fix.
Executable `959bc0d76db2f1af64b666e84a9a57046394cea10ac36305877f3bfc9594fa63`,
[note 153](documentation/153-d19p-quit-to-menu.md). Not verified: YES in
level 1 and in first person (harness keys), the user's playtest.

**Opened. User request, 2026-10-19:** "on the death screen you have quit, and
quit game. maybe that should be quit to menu, and quit game."

The death list (D19H, D19M) reads CONTINUE, LOAD GAME, RESTART LEVEL, QUIT,
QUIT GAME. QUIT keeps the original's meaning (give up: the game over screen,
then the title; user choice for D19M), but beside QUIT GAME the two read as
the same action. Rename it **QUIT TO MENU**, in the TTK menu font, and make
the action match the name: decide with the user whether it still passes
through the original game over screen or goes straight to the title main
menu (as the pause list's MAIN MENU does), and whether it asks YES / NO like
MAIN MENU and QUIT GAME. The challenge stages' GIVE UP and the pause list's
MAIN MENU stay as they are unless the user asks; consider one consistent
wording across both lists.

**Acceptance:** the death list shows QUIT TO MENU and QUIT GAME; QUIT TO MENU
reaches the title menu (with or without the game over screen, as agreed) from
first and third person, in at least two levels; QUIT GAME still closes the
program; Vanilla keeps the original death screen; the user confirms.

### D19Q - SAVE GAME screen to match LOAD GAME

**Done (user-accepted 2026-10-20: "im happy with this"; implemented
autonomously at the user's request), [note 171](documentation/171-d19q-save-game.md).** SAVE GAME on the
pause list (beside LOAD GAME) opens LOAD GAME's page with every slot open: an
empty slot saves at once, a used one asks OVERWRITE THIS SAVE? (NO first);
SAVED TO SLOT NN or THE GAME COULD NOT BE SAVED (slot unchanged: the framework
now writes slots atomically). The thumbnail is the game when the pause opened,
not the menu; LOAD GAME on such a save resumes play at once. Also fixed: the
menu identity guard no longer covers GAME SETTINGS' card-restored values (an
early F7 load could disable every modern menu). Executable
`15e3d7b2c761613f7f3999d107b0ae251fc333ad5322d85bf377f44daffe999b`.

**Opened 2026-10-20.** User: "we currently have a Load Game option but
no corresponding Save Game option. We should introduce a Save Game screen that
aesthetically matches our newly developed Load Game interface ... a coherent
pair of Save Game / Load Game interfaces that feel like two parts of the same
system." Depends on D19K (its page, slot list, host image and steel text are
the design and code to reuse). Not implemented yet.

Intended behaviour (Modernized; Vanilla keeps the original saves):
- SAVE GAME in an appropriate menu place (candidates: the pause list beside
  LOAD GAME; the statistics screen's SAVE GAME between levels, which today
  writes the memory card).
- A SAVE GAME page in D19K's layout, fonts, highlighting and keys: the 12
  slots with level name, thumbnail, era and save time; empty slots offered
  as NEW SAVE (or similar).
- Enter on an empty slot saves; on an occupied slot asks OVERWRITE SLOT NN?
  (NO first, as the other destructive prompts). Escape goes back to the list it
  came from on SAVE GAME.
- Clear feedback: SAVED (slot, level) on success; a visible failure on a
  refused or failed save, the slot unchanged.

Existing systems to reuse (investigated 2026-10-20 during D19K):
- Framework savestates: `savestate_request_save(slot)` staged for the next
  safe point, `savestate_pending()`, `savestate_take_save_failed()`,
  `savestate_status_json()` (generation, last_ok), the slot thumbnail
  captured after a successful save, `savestate_slot_compatible()` (fixed in
  D19K to answer without a reason buffer). Saves already wait while a plugin
  guest call is open (D19J).
- `ttk_savestate_saved()` (D24B) writes the level sidecar after a successful
  save, gameplay only.
- D19K's slot reading, list drawing, host overlay (`ttk_menu_thumb_image`)
  and Console steel text.
- The original memory card SAVE GAME (statistics screen) and the D19N card
  settings pack (`0x80082f64`), which must stay intact for Vanilla.

Questions to settle first:
- What state a save from the pause menu should hold: the paused state with
  our list open (a load then reopens the pause list, as F7 saves taken there
  do today) or the moment before pausing, resuming straight into play.
- The thumbnail: the framework captures the presented image after the save,
  which on a menu page is the menu (D19K's test slot 12 shows the death list).
  It likely needs the last gameplay image from before the menu opened.
- Whether the statistics screen's SAVE GAME becomes ours in Modernized, and how
  that relates to D20 (save management, quick saves) and the memory card's
  settings, which still load at startup (D19K).
- Saving where the original cannot (cutscenes, loads, two-player): which
  states are refused, and how the page says so.
- Failed writes (disk full, permissions) and a slot saved by another build:
  overwrite allowed after the prompt, with what wording.

**Acceptance:** SAVE GAME opens a page that reads as LOAD GAME's pair; saving to
an empty slot and overwriting (after its prompt) both produce a slot that
LOAD GAME lists with the right level name, a gameplay thumbnail and time, and
that loads back to the saved moment; a refused or failed save says so and
leaves the slot unchanged; Escape backs out without saving; Vanilla saves
unchanged; the player's own cards and savestates are never used in testing;
the user judges it.

### D19Q1 - SAVE GAME thumbnail shows the save screen

**Done (user-accepted 2026-10-20: "confirmed working!").** Cause found and fixed in the framework:
`savestate_grab_thumb` read CPU VRAM without a sync. The OpenGL renderer
presents from its own framebuffer, and CPU VRAM only catches up on a readback.
A savestate write does one, so the picture kept at pause entry was the image
at the **previous save**: after one SAVE GAME, the next one got that save's
OVERWRITE THIS SAVE? page. First person was not the cause. Fix: the grab
syncs first (`gl_renderer_sync_cpu` / `vk_renderer_sync_cpu`, skipped on
depth24 scanout, as screenshots do). F7 saves and the rewind thumbnails share
`gpu_display_pixel_argb`. F7's capture follows the save's readback and was
already correct; rewind (`psx_rewind.c capture_thumb`) is unchanged.

Evidence (offscreen, OpenGL, 4x, 16:9, 120 Hz, private profile and
savestates `recomp/analysis/d19k-load/cards-q1`, `k.py`; local). Executable
`92178369fc5153178009edf801bd5c07299fee5d6d9bab927ea6ca2b6fc3e043`:
- Two SAVE GAME saves in one session (slot 01, resume, move, slot 02):
  without the fix, slot 02's picture was slot 01's OVERWRITE THIS SAVE? page
  with YES highlighted, the player's report reproduced. With the fix, slot 02
  shows the new position with no menu text.
- First person (captured play, eye view): the picture is the eye view, no
  menu.
- Overwrite after LOAD GAME (play resumed, then pause, then save): fresh
  picture of play.
- Vanilla F7 save: picture of play; Vanilla play is unchanged (no gameplay
  code touched).
- Python 160 OK (2 skipped), `ttk-input-test`, `ttk-live-env-test`. A
  pristine pinned framework plus the exported patch reproduces all 48
  modified framework files. No hashed header touched.

Remaining: the user's judgement in their own game. A save after an F7 load
into the pause menu still has no picture of play (note 171 limits). The
player's existing slot 1 keeps its old picture until it is saved again.

**Opened 2026-10-20 (brief research).** User: "when you save the
game from the new save screen the screenshot that it saves in the preview
actually has the text of the save screen in it so it actually says overwrite
this save and so on inside the screenshot ... within slot one which is my
actual game."

Evidence (read only; the player's files untouched): player UI slot 1
(`saves/local-play/openbios/state_800AB6FC_slot00.*`, level 3 GOLD AND GUNS,
sidecar `level 3` / `resume 1`) has as its thumbnail SAVE GAME's
OVERWRITE THIS SAVE? page (SLOT 01 - GOLD AND GUNS, YES highlighted, prompts)
over a darkened third-person view of Duke. Local copies:
`recomp/analysis/d19q1-thumb/`.

Brief findings:
- `resume 1` proves `ttk::game_menu_saved` ran for SAVE GAME's own save, but
  the picture kept at pause entry was not written over the framework's
  capture: `savestate_write_thumb` was skipped (`pause_thumb_ok` false, or
  one of `owned` / paused / `pause_screen == ours_sub` false at that moment)
  or failed, or the kept picture was itself a menu image.
- The D19Q tests that passed were all in third person. With the player's
  profile in **first person**, the pause screen draws the world from a
  third-person camera (Duke visible behind the list,
  `first-person-pause.png`), which is what the slot 1 picture shows under the
  prompt. So in first person the image grabbed at pause entry
  (`sub == pause_top`) may already be a pause image, or the grab is skipped on
  that path. Not yet run: SAVE GAME end to end in first person (the
  harness's captured-input mode did not move the menu cursor; drive it
  uncaptured after switching view, or through the debug pad).
- Other paths that leave no picture of play by design: a save after an F7
  load into the pause menu (note 171 limits).

Direction: take the picture of play from the last gameplay presentation
before the pause (first and third person), not from whatever the guest
displays at pause entry; never fall back to a capture of our own pages (a
plain darkened world without the menu text would be the minimum).

**Acceptance:** SAVE GAME in first and third person, new and overwritten
slots, from a fresh pause and after a LOAD GAME, always stores a picture of
play with no menu text; LOAD GAME and F7 show it; Vanilla unchanged; the
user judges it.

### D19Q2 - Saves restore paused

**Todo (2026-10-22), user-requested investigation.** During D17M intake, the current UI slot 10 restored to the pause menu through the debug savestate loader. The user reports saves while paused have occurred more than once and suspects a small regression. Do not assume the diagnostic load path proves a player LOAD GAME regression.

**Acceptance:** use private save/card copies to compare SAVE GAME, quicksave and relevant legacy/debug save/load paths from gameplay and menus. Establish which paths preserve or clear pause and menu state; verify intended player loads resume reliably with working controls and mouse capture. Preserve deliberate pause behavior and Vanilla; no player saves/cards modified. Record reproduction evidence and fix only a confirmed regression. Deferred until selected; D17M remains the active job.

### D19Q3 - Every Modernized gameplay load resumes immediately

Status: Done (user-confirmed 2026-10-26). [Note 256](documentation/256-d19q3-always-resume.md). User reports slot 9 remains paused after D37A and explicitly
requests that games never start paused (2026-10-26). This supersedes D19Q2's
intentional pause preservation in Modernized loads. Use the successful load
generation, independent of sidecar and load route, to resume living gameplay
through the existing guarded native sound/music and pause-reset path. Preserve
normal later Escape pauses, death/statistics screens and Vanilla behavior.
Test slot 9, repeat loads, LOAD GAME/quickload/F7-equivalent debug loads,
metadata-free paused saves, already-running saves and ordinary pause/resume.
No automatic acceptance closeout, commit or push.

### D19Q4 - Intermittent UI flashes on load

**Status: Done (user-accepted 2026-10-27).**

Both symptoms reproduced and fixed in the candidate. Private 60/120 FPS load-route
checks, inventory/controls and D37A regressions pass. [Note 279](documentation/279-d19q4-load-ui-flashes.md)
records causes, evidence and limits. User accepted and authorized closeout, commit
and push: "lock it in and close it out".

User report, 2026-10-26: sometimes text resembling "overwrite the previous save"
(user paraphrase, exact wording unconfirmed) briefly blinks on screen while loading.
Sometimes the inventory selector operated with [ / ] also appears upon loading.
Record these as two symptoms of load-time UI presentation; a shared cause is not
established. No particular slot, load route, timing or reproduction frequency has
yet been established. Do not assume a save was overwritten or inventory changed.

When explicitly selected, reproduce using private settings/cards and save copies;
compare LOAD GAME, quickload and F7, including paused/running and repeated loads.
Trace menu confirmation rendering, input edges and inventory-overlay lifetime
across the load boundary. Preserve D19Q3's immediate resume and D37A's animation
recovery. Acceptance: neither unrelated overwrite text nor the inventory selector
flashes on load; deliberate overwrite confirmations and [ / ] inventory selection
still work, with ordinary pause/resume and Vanilla preserved.

The original intake did not authorize implementation. User selected D19Q4 on
2026-10-27; candidate work is recorded above.

### D19R - Startup card probe hang

**Done (2026-10-20; the user delegated testing and closeout: "you can easily
test that yourself and close it out"), [note 170](documentation/170-d19r-card-wait.md).**
The card probe and file read (states 1 and 2) now run their VBlanks unpaced
with the framework idle skip on for that window only (new mod API
`psx_mod_request_unpaced`): 1.5 s + 0.4 s of black became about 0.24 s, the
same guest fields, saved settings still loaded; blank and fresh cards continue;
Vanilla unchanged (paced, texts shown). Executable
`2fb1bd3cc0e8ee78fd94c41da6c66c39a5fe7d5ff4eddd813e321bc2f2833408`.

**Opened 2026-10-20.** User, after D19K: "in place of where it used to
have the checking memory card message on startup, it now just hangs briefly
and nothing of note happens. wonder if we can just remove that hang card
entirely." Depends on D19K (which hid the CHECKING texts and skipped the
original's waits, but kept the card work).

Research (2026-10-20, `recomp/analysis/d19k-load/hang.py`, local; cold boot
offscreen, Modernized, private card with a save):
- The pause is frontend state 1's card probe `0x80082e6c(0,1)`: card status
  `0x80020b44` (via `0x80020a74`), the card check `0x80020d78`, then the
  directory listing `0x80021754` and a name compare per file (`0x800bb73c`)
  for the game's save. Settings are read afterwards in state 2 (`0x80083490`,
  then `0x80083070`), which is short.
- State 1 lasted 1.5 s (90 fields). Sampled guest PCs were almost all in
  kernel RAM `0x354c` / `0x3530` (the BIOS card routines waiting), with the SIO
  registers (`0x1F801040`...) in hand. The game's main loop is blocked inside
  the probe, so no image is presented: the screen stays black.
- The raw transfer is not the cost: a directory read is about 2 KB, roughly
  70 ms at the framework's card byte timing (`SIO_BAUD_CYCLES_DEFAULT` 1088,
  `sio.c`). The BIOS card driver appears to advance about once per VBlank
  (90 fields for the probe), so the wait is paced by frames, not bytes.
- Not yet measured: which BIOS card call takes the fields (fntrace on the
  `0x80020b44`, `0x80020d78`, `0x80021754` calls), and whether Vanilla takes
  the same time (it should: same code; the text was simply on screen then).

Options to evaluate (no change made yet):
- Run the emulation unpaced while the guest waits in the BIOS card driver at
  startup and nothing new is presented (a general "fast card wait", like the
  CD-ROM acceleration after savestate loads); game logic unchanged, the wait
  takes a fraction of a second.
- Keep the last presented image (the title backdrop) up during the wait
  instead of black, so the pause reads as part of the title appearing.
- Move the probe off the critical path: start PRESS START at once and run the
  card probe and settings read later (risky: the original orders them; the
  settings must be in place before play).
- Read the settings block from the card file on the host and skip the probe
  (an HLE of the game's own check; least faithful, last resort).

**Acceptance:** after the intro movies (or Escape) Modernized goes to the title
with no visible hang or black gap from the card check; the saved settings
(GORE, STATUS BARS, GAME SPEED, pad layouts) still load from the card; a blank
or missing card still continues; memory card saving in Vanilla and the
statistics screen are unaffected; Vanilla startup unchanged; the user judges it.

### D19S - Delete saves from the LOAD GAME screen

**Done (user-accepted 2026-10-21: "perfect. lock it in"), [note 175](documentation/175-d19s-delete-saves.md).**
Delete on a used slot of LOAD GAME (title, pause, death) or SAVE GAME asks
DELETE SAVED GAME: with the slot's name in quotes, in Console steel lettering
(user request), under its picture, YES / NO with NO first. YES removes that
slot's file, thumbnail and level sidecar through new framework call
`savestate_delete_slot`; the list shows EMPTY at once and says DELETED SLOT
NN, or SLOT NN COULD NOT BE DELETED with the slot intact. Delete is a fixed
menu key (like Enter / Escape), not a KEY BINDINGS row, and has no prompt in
the bottom bar yet (D19F). Executable
`64e051548f31702e0509e9d9cb0131ef749e438c345ab2ca50ea4ee51a4d5974`.
Offscreen tests on private savestate copies passed (note 175). Follow-up:
D19T (real double quotes in the menu font).

**Opened 2026-10-21.** User: "from the load game screen,
we can delete saves. for modern keyboard controls, the user can press delete
against a save, and it should bring up a dialogue "delete saved game:<br>
"<savegame-name>"". Depends on D19K (the LOAD GAME page and slot list) and
D19Q (its prompts, atomic slot writes and SAVED / failure messages).

Intended behaviour (Modernized; Vanilla keeps the original memory card saves):
- On LOAD GAME's page (title, pause and death lists), Delete on a used slot
  opens a prompt in the page's style, two lines:
  `DELETE SAVED GAME:` then the save's name in quotes, e.g.
  `"SLOT 03 - <LEVEL NAME>"` (the name as the list shows it; slots have no
  player-given names today).
- YES / NO with NO first, as OVERWRITE THIS SAVE? and the other destructive
  prompts. Enter on YES deletes; Escape or NO goes back to the list with the
  same slot selected.
- After a delete the slot shows EMPTY at once (thumbnail, era and save time
  cleared); a short DELETED confirmation, or a visible failure with the slot
  unchanged if the files cannot be removed.
- Delete on an EMPTY slot does nothing. Greyed saves from another build can
  be deleted (they cannot be loaded, so this is how to clear them).
- Decide during the job whether SAVE GAME's page (D19Q) offers the same key,
  and whether the Delete key should be rebindable in KEY BINDINGS (MENU) and
  shown as a prompt (ties to D19F).
- Removes only the chosen slot's savestate files (and its thumbnail) through
  the framework's slot API, never memory cards or other slots. Rewind and F7
  saves keep working.

**Acceptance:** Delete on a used slot shows the two-line prompt with the
right name; NO / Escape changes nothing; YES empties exactly that slot, the
list updates, LOAD GAME and SAVE GAME stay correct afterwards and across a
restart; failure is reported; title, pause and death lists all work; tested
on private copies of savestates only; Vanilla unchanged; the user confirms.

### D19T - Real double quote marks in the menu font

**Done (user-accepted 2026-10-21: "that's awesome. lock it in close it out."), [note 176](documentation/176-d19t-menu-double-quotes.md).**
Any `"` in a Modernized menu string now draws as a real double quote in both
menu sizes: two of the font's own apostrophes, closing as they are, opening
turned half round (texture flipped on both axes), alternating open / close.
Widths and alignment count the quotes. Offscreen checks passed in both
sizes. First playtest: the steel name line on DELETE SAVED GAME: still showed
the fallback quotes; the font pack (Big / Medium Italic) now has the same
quotes, alternating, and that line shows them. Identity guard passes,
Vanilla boots, tests pass. Executable
`52f5f1a3ae67040f6e9d52d544ba914e7115c1e38985a23ce066937c22aea29c`.

**Opened 2026-10-21.** User, accepting D19S: "i want a
backlog job standing up for a real " and " for this font." The game's menu
text call (glyph call `0x8008b210`, `font_small` / `font_menu` in
`game_menu.cpp`) has no double-quote glyph: `"` comes out as a single quote
mark (seen in D19S's DELETE SAVED GAME: prompt before its name line moved to
Console steel, which has real quotes).

Intended behaviour (Modernized; Vanilla unchanged):
- Opening and closing double quotes (the user's " and ") in the menu font,
  drawn in the disc font's own style (gold, outline, size and spacing of its
  apostrophe and letters), for both the small and the large menu sizes.
- Usable by every Modernized menu string that wants quotes; nothing in the
  original's own text changes.
- Find first how the glyph table maps characters (D19 glyph tables from
  `0x800c3f38`, the identity guard in `game_menu.cpp`) and whether a new
  glyph can come from the existing font pack / host overlay path or needs
  pixels added to the font texture page in VRAM.

**Acceptance:** a test string with " and " in both menu sizes shows real
double quotes that match the font; no other glyph changes; menu identity
guard still passes; Vanilla unchanged; the user judges it.

### D19T1 - No quotes around the name on DELETE SAVED GAME:

**Done (user-accepted 2026-10-21: "perfect. again lock it in close it out").**
Opened 2026-10-21. User (ad hoc): "on the load game menu, i want
the quotes for the name to be removed when you delete a slot. im glad we
created the glyphs, but after seeing it, it will look cleaner without them."

Implementation: `draw_delete_ask` in `recomp/src/ttk/game_menu.cpp` builds the
name line as `SLOT nn - <name>` (or `SLOT nn`) with no surrounding `"`, in
both the Console steel path and the menu-font fallback. The D19T quote glyphs
(menu font and font pack) are untouched and remain available to any string
that contains `"`. Same prompt on SAVE GAME's Delete. Incremental build links;
not launched (one-string change). Executable
`95c420cfcfe6e4e25945f29405a14890f0892bb1f4096dc93c012f49c748db19`.

**Acceptance:** Delete on a used LOAD GAME / SAVE GAME slot shows DELETE SAVED
GAME: with the name line unquoted; nothing else changes; the user judges it.

### D19U - Live minus/plus status-bar controls

**Status: Done (user-accepted; closeout authorized 2026-10-28).**

User request, 2026-10-26: Duke3D-style live HUD adjustment using `-` and `+`.
Initial implementation: `-` hides the status bars, exactly equivalent to the current
STATUS BARS OFF setting; `+` restores them, equivalent to STATUS BARS ON. Apply
immediately during Modernized gameplay without opening a menu or restarting.
This is status-bar visibility, not shrinking the rendered game viewport.

When selected, integrate with the existing action/binding system and status-bar
setting so menu and keyboard state agree. Inspect number-row minus/plus/equal and
keypad handling and existing conflicts before finalizing bindings. Keep these
shortcuts out of console/text entry, binding capture and menu navigation. Preserve
crosshair, aiming and unrelated HUD behavior according to existing STATUS BARS
semantics, rather than hiding additional elements arbitrarily. Vanilla unchanged.

Acceptance for this initial job: live hide/show through `-`/`+`, identical to the
existing OFF/ON behavior; repeat presses at either endpoint are harmless; menu
state remains synchronized; pause/load/level transitions do not introduce a
conflicting HUD state. Document implemented bindings and verify user playtest.
D19U can complete independently of the later D19U1 borderless enhancement.

### D19U1 - Borderless status bars and three-level HUD selection

**Status: Done (user-accepted; closeout authorized 2026-10-28, following D19U foundation).**

User request: add a clean borderless intermediate presentation that removes all
status-bar borders while retaining their useful contents. Once implemented, the
ordered HUD levels are:

| Level | Presentation |
| --- | --- |
| Full | Current status bars with their borders |
| Borderless | Status-bar contents retained, all their borders removed |
| Hidden | Existing STATUS BARS OFF behavior |

`-` moves Full -> Borderless -> Hidden. `+` reverses that sequence:
Hidden -> Borderless -> Full. Clamp at the endpoints, without wrapping. Thus the
first minus press from Full removes only the borders, and the second hides the
bars. This adds an intermediate state to D19U's initial two-state behavior.

When selected, inventory the native and modern status-bar components, distinguish
borders from contents, and establish consistent borderless presentation. Keep
values/icons readable and aligned at supported window sizes and aspect ratios;
use D08A22's completed fitted-picture scaling work as a reference and coordinate
with D07G where relevant. Preserve the existing Full and Hidden appearances.
Expose/synchronize the three levels through the settings and binding behavior
established by D19U, with coherent save/load and level-transition behavior.

Acceptance: both directions step immediately through exactly these three levels;
Borderless retains status information with no status-bar borders; Full and Hidden
match existing behavior; no stale frames, accidental text-entry shortcuts or
Vanilla changes. Visual review and gameplay acceptance required.

No investigation, source changes, build or launch for either job until selected.

### D19U2 - Borderless HUD: no black squares behind icons

**Status: Done (user-accepted 2026-11-01: "lock it in close it out!"; [note 350](documentation/350-d19u2-borderless-icons.md)). Dependencies: D19U1.** In Borderless (D19U1, `-` from Full)
the box borders are removed (`widescreen.inc` zero-width box record
`0x800c44a4`), but icons such as the health cross and the jetpack gadget icon
still sit on an opaque black square; the ammo icon has none (offscreen
check, fixture slot 10, `recomp/analysis/d17ar3p4/hud-b-crop.png`). The
[ / ] switcher already draws the jetpack, Bio Mask and night-vision icons
with transparent backgrounds, so suitable art is likely available. Inventory
every Borderless element (native and our own boxes: health, armor, ammo,
gadget, power-up and steroids countdowns, challenge item), find whether each
square comes from the icon cell (CLUT colour 0 opaque, a filled cell) or a
separate fill, and draw the contents without it, reusing switcher art where
the native art has no transparent version. Full and Hidden unchanged; Vanilla
unchanged. Visual review at the player's window sizes.

**2026-11-01 implementation (Needs playtest; note 350).** The squares are in
the icon cells: each native HUD icon's background is an opaque black palette
entry (`0x8000`); the ammo strip's is transparent and the air icon is a full
water tile. In Borderless the cross, armour, jetpack, Bio Mask, goggles and
counter are drawn from keyed copies built each frame from live VRAM: black
connected to the cell edge becomes the palette's transparent entry (the
switcher's rule; black inside an icon stays), loaded through OT slot 1 into
the empty band x 984-1007 row 205, with the records pointed at the copies only
inside the HUD scope. Own art (medkit, steroids, power-ups, challenge) was
already transparent. Verified offscreen at 60 and 120 Hz (slots 10 and 7,
gadgets on/off, 12 consecutive replay presents), Full/Hidden/Vanilla
unchanged, native controls suite with a new D19U2 group. Not seen: counter
(levels 21-26), air under water, 21:9, the player's 180 Hz display.

**2026-11-01 size fix (Needs playtest).** User: the cross seemed one pixel
smaller in Borderless. Confirmed (28 vs 30 screen pixels wide): the original
sprite routine clamps the right texture edge at 255, so cells at x 1020 (cross,
Bio Mask) draw 15 columns stretched over 16; the copies drew at true size.
Their copies now keep column 1020 (blank sheet rows 54 and 100), and the
cross matches Full pixel for pixel. Pause-menu fonts unchanged; native suite
passes.

**2026-11-01 accepted and closed.** User: "i like it!", then after the size
fix: "lock it in close it out!". D19U2 is Done. Commit/push as MusicMonsterMod
authorized. Retained limits: counter (levels 21-26), air icon under water,
21:9 and the 180 Hz display were not seen in testing.

### D19V - Remove redundant Advanced Settings view toggles

**Status: Done (user-accepted; closeout authorized 2026-10-28).**
Remove the user's "Use interaction" and "Mirror" options from Advanced Settings.
First trace each setting's consumers and defaults. Preserve intended default
play and underlying interaction/rendering functionality; remove obsolete UI
descriptions and gracefully normalize or migrate existing saved configurations.
Read the accepted D36 audit before changing action perspective ownership.
Acceptance: options/descriptions removed, fresh and existing configurations
behave correctly, interactions/mirrors and accepted first-person behavior remain
intact, Vanilla unchanged.
Historical intake observation: `recomp/src/ttk/game_menu.cpp` defined these as
USE INTERACTION VIEW (`use_view`) and MIRROR VIEW (`mirror_view`) under ACTION
PERSPECTIVE. They are view selectors; this does not establish all consumers or
migration requirements. Do not mistake MIRROR VIEW for a rendering enable switch.
Accepted result removes both rows and makes Use/Mirror follow global VIEW. Schema 57
removes old overrides without resetting other valid preferences. Player build,
105 profile tests and native controls/input/aim/near suites pass. User accepted:
"perfectly done, lock it in and close it out". Implementation, tests and
documentation commit/push closeout authorized.
[note 295](documentation/295-d19v-action-view-cleanup.md) preserves evidence
limits separately from user acceptance.

### D19W - Separate keyboard and gamepad bindings pages

**Status: Todo. Backlog only; not started. Related: D19G, D09/D09A.**
Rename KEY BINDINGS to KEYBOARD KEY BINDINGS and add a GAMEPAD KEY BINDINGS
category beside it. Place both categories immediately below CONTROL SCHEME
and before MOUSE LOOK on the Controls page. Provide gamepad binding controls
consistent with the existing keyboard page; inspect existing input mapping,
conflicts, persistence and defaults before implementation.
Acceptance: requested order/names, usable pad navigation and rebinding, settings
persist/reload, keyboard bindings preserved, relevant Legacy/Modern and device
assignment behavior verified. Resolve any unsupported bindings explicitly.

### D19X - First-person FOV slider in Gameplay settings

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D14A, D19G, D19N.**
After D14A is complete, add "First-person FOV" directly after "View Bob" in
Gameplay settings, matching the existing menu style. Display degrees using
D14A's established convention and validated range. Support keyboard, gamepad
and mouse adjustment; apply live where practical, persist the value, and include
it in the screen's Defaults action. Preserve the existing view as the default.

Acceptance: correct placement/style and degree display; all three input methods;
live behavior and any limitations documented; Defaults restores the original
view; setting survives a full restart; switching first/third person preserves
the value while leaving third-person projection unchanged. Test migration of
existing settings and invalid values using D14A's configuration contract.

### D19Y - Convert third-person Camera Distance to a slider

**Status: Done (user-accepted; closeout authorized 2026-10-28). Dependencies: D19G, D19N. Related: D06, D14A, D19X.**
Convert Gameplay's existing "Camera Distance" choice control into a slider,
matching the existing menu style. Make its third-person scope clear in the
label or description. This controls camera distance, not first-person FOV;
it does not depend on implementing D14A or D19X.

When selected, inspect the existing distance preference, Original behavior and
Alt + mouse-wheel adjustment before choosing slider units, limits and step.
Preserve the current default and existing saved distances, including the
meaning of Original; do not assume that Original means a literal zero-distance
camera. Support keyboard, gamepad and mouse adjustment, apply live where
practical, persist the setting and include it in Gameplay's Defaults action.

Acceptance: usable slider with clear third-person scope; safe range and step;
correct live adjustment, existing shortcut synchronization, Defaults and restart
persistence; existing settings migrate without losing intended distance.
Verify first/third-person switching retains the distance preference without
changing first-person projection or eye placement. Preserve camera obstruction
handling, scripted camera ownership and Vanilla. No implementation authorized
by this intake.

### D19Z - Full mouse support for the menu UI

**Status: Todo. Backlog only; not started. Dependencies: D19. Related: D19B, D19F/F1, D19W/X/Y.**
Implement genuine pointer navigation in Modernized menus: hover feedback,
clicking the intended row/control, choosing options, clicking and dragging
sliders, scrolling lists and operating visible actions and confirmation dialogs.
Retain the working wheel, keyboard and gamepad navigation; avoid stale pointer
position overriding deliberate keyboard/gamepad selection. Cover title, pause,
settings and bindings, save/load, death, statistics and their reachable submenus;
inventory the actual screens when selected and explicitly resolve any exception.
Expose mouse-operable equivalents for necessary keyboard-only actions such as
Defaults and save deletion, preserving existing confirmation semantics. Binding
capture must not accidentally bind the click that opened it, and must offer a
mouse-operable cancel route. Do not require keyboard shortcuts to finish normal
menu navigation and settings use.

Inspect pointer coordinates against the actual presented menu rectangle,
letterboxing, scaling, window size and DPI. Own pointer presses/releases across
screen changes so clicks do not activate a newly opened screen or leak into
shooting on resume. Preserve focus handling and automatic gameplay mouse capture.
D19B remains the separate responsiveness job; D19F/F1 remain prompt-art/device
jobs, not substitutes for pointer hit testing.

Acceptance: complete mouse-only menu routes with accurate hover/click targets,
option changes, slider dragging, scrollable lists and confirmations; keyboard
and pad coexist correctly; windowed/fullscreen, aspect/DPI scaling and focus
transitions work; no click-through or gameplay input leakage; Vanilla unchanged.
Full feature closeout includes D19Z1 and D19Z3. Core pointer work can be verified
with a temporary standard cursor while custom art is pending.

Minimal source evidence: `game_menu.h`'s `MenuKeys` and
`pc_input.cpp:menu_owns_keys` carry directional/confirm/back actions and wheel
input; `game_menu.cpp:game_menu_input` maps wheel steps to list movement. These
inspected interfaces do not carry absolute pointer coordinates or hover/drag
state. This is source evidence, not an exhaustive runtime UI audit or a claim
that the framework cannot support a pointer.

### D19Z1 - Design and implement the top-left menu Back control

**Status: Todo. Backlog only; not started. Depends on: D19Z core pointer support. Related: D19F.**
Design a Back icon matching the established TTK menu art and place a clickable
control at the top left of the presented menu area, with suitable padding and
hit-target size. Use it consistently wherever Back/Cancel is valid so the user
can navigate entirely with the mouse. Define root-menu and pause-root behavior
explicitly; do not invent a parent or turn Back into an unexpected quit action.
Reuse the existing context-sensitive back/cancel semantics, including draft
settings and dialogs. Provide clear hover/pressed feedback and readable scaling.

Acceptance: approved icon appearance, correct top-left placement across menu
layouts/aspects, reliable target and feedback, mouse-only return/cancel routes,
correct root behavior, and no bypass of confirmations or changed save/apply
semantics. Record original-art or disc-derived provenance before integration.

### D19Z2 - Design a Time to Kill mouse cursor

**Status: Todo. Backlog only; not started. Related: D19Z, D19Z1, D19F.**
Create a cursor design consistent with the game's established menu fonts, art
and palette. Present candidates and a recommended design over representative
light, dark and busy menu backgrounds at native and scaled sizes. Define the
precise click hotspot, dimensions, transparency, outline/contrast and scaling
rules; specify any additional pointer states only where useful. Keep the tip
unambiguous and the pointer readable without obscuring small controls.

Acceptance: user-approved design and preview evidence, editable source and
export assets, documented hotspot/scaling/state specification and provenance.
Original project art may be tracked; retail-derived pixels must follow the
existing owned-disc extraction rules. This design job does not install a cursor
or change runtime input. No art creation is authorized by this intake.

### D19Z3 - Implement the approved custom menu cursor

**Status: Todo. Backlog only; not started. Dependencies: D19Z2, D19Z core pointer support.**
Integrate the approved cursor using the appropriate existing host/UI path after
inspecting its capabilities. Apply the specified hotspot, scaling and any
approved states. Show the pointer when menus accept mouse input, avoid duplicate
OS/game cursors, and hide/release or capture it appropriately on gameplay,
focus and window transitions. Define mixed-input visibility behavior without
flicker or interference with keyboard/gamepad navigation. Keep cursor position
and hit testing aligned through window resizing, DPI and aspect changes.

Acceptance: approved art rendered clearly, hotspot matches activation position,
consistent movement and visibility across supported menus, windowed/fullscreen
and scaling checks, correct focus/resume behavior, no duplicate cursor or leaked
clicks, and Vanilla unchanged. Validate with D19Z1's Back target and D19Z's
sliders/lists; record platform coverage and any unverified cases.

### D19AC - Crash on opening OPTIONS from the pause menu

**Status: Done (user-confirmed 2026-10-30, [note 328](documentation/328-d19ac-options-crash.md)). Related: D19AB.**
User (2026-10-30, 07:23): after an F7 load of UI slot 11 (also slot 2),
Escape then OPTIONS crashes the game, every time, even after waiting about
30 seconds. Cause: five SIGSEGV core dumps, all in `game_menu_input` at the
mouse-wheel slider check. On the OPTIONS list `screen` is -1, and D19AB's
`view_slider(screens[screen].rows[cursor])` dropped the old `screen==2`
short-circuit, so every menu tick read `screens[-1]`, heap memory before the
vector; it crashed whenever that memory held an invalid pointer. Fix: guard
with `screen>=0`. Reproduced offscreen on the previous build (5 of 6 runs
died, same stack); fixed build 10 of 10 survived on UI slots 11 and 2 with
the OPTIONS list confirmed open. Remaining: user playtest.

### D20 - EDuke32-style quick save and quick load

**Done (user-accepted 2026-10-21: "its all working!"), [note 177](documentation/177-d20-quick-save.md).**
Scoped with the user on 2026-10-21: "mimic exactly how this all works in
duke3d/eduke32 ... quicksave for example is f6 - it is a hotkey that updates
the last used save slot automatically ... quick load is f9. quicksave, and
quickload, are also console commands that do exactly this". EDuke32's source
was read for the behaviour; the user took the recommended choices ("go with
your recommendations"): the quick slot is the slot last saved to or loaded
from (EDuke32's load-resets-last-save quirk not copied), not kept across
launches, no level-start autosave, Modernized only, and the original memory
card half of this job split to D20B.

Implementation: F6 saves over the quick slot without pausing (GAME SAVED), or
opens SAVE GAME with no slot yet; dead: CAN'T SAVE WHILE DEAD. F9 loads it at
once (a SAVE GAME save resumes play), or opens LOAD GAME with no usable slot;
it works on the death list too. F2 / F3 open SAVE GAME / LOAD GAME straight
from play; Escape returns to play and a save there closes the page with GAME
SAVED. Console `quicksave` / `quickload`. KEY BINDINGS section SAVES (QUICK
SAVE, QUICK LOAD; F4, F5, F6, F9, F12 bindable); launcher schema 38. The
framework's F6 port swap is off in Modernized (F1 menu keeps it). Code:
`pc_input.cpp` (`quick_press`, `input_quick`), `game_menu.cpp`
(`game_menu_quick`, `quick_poll`, direct pages), `level_select.cpp`
(console), framework `main.cpp` (help, port-swap gate; exported patch).

Evidence: offscreen private runs (note 177 table): F6 and F9 round trip on
slot 05 with identical start and loaded frames, console commands, F2 / F3
pages and Escape, save to an empty slot from F2, F9 fallback with the slot
deleted, death-list F3 / F6 / F9, a new game from the title (F6 opens SAVE
GAME), KEY BINDINGS rows. `ttk-input-test` D20 cases, Python 164 OK, patch
verified against the pristine pin. Executable
`20f8edc1d1d6260e59747df7d31ae4a438f4081a8eb19ee402ea9924f1b3e930`.

Limits: found a pre-existing framework bug, D20A: a savestate taken within
about 1.5 s after a load can halt the emulator when loaded after a restart
(F7 and SAVE GAME too). The manual tells players to play a moment after a
load before saving until it is fixed. Not run in a visible window; the user's
playtest judges the feel.

**Acceptance:** F6 / F9 / F2 / F3 and the console commands behave as above in
one-player Modernized play; quick saves restore the saved scenario and other
builds' saves are refused (LOAD GAME opens); Vanilla unchanged; the user
judges it in play.

### D20A - Savestates taken shortly after a load halt when loaded after a restart

Found during D20 (2026-10-21), [note 177](documentation/177-d20-quick-save.md#d20a-savestates-taken-shortly-after-a-load-framework-pre-existing).
A save taken about 0.3-1.5 s after a savestate load can load fine in the
session that made it but halt the emulator after a restart: `FATAL: GPU GP0
unknown command 0xFA (word 0xFA000045)`, DMA channel 2 linked list at
`0x80F40BD0`. Reproduced with the framework's own save path (F7), the console
and F6, Modernized and Vanilla, 60 and 120 Hz; saved DMA state is idle and the
CPU is in libetc VSync. Find what the snapshot misses after a load (CD-ROM
streaming state is a candidate) and fix it in the framework.

**Acceptance:** saves taken at any time after a load (0-3 s sweep) load after
a restart; existing good savestates still load; patch exported.

**Done (user-accepted 2026-10-21 with D20C), fixed with D20C, [note 178](documentation/178-d20c-savestate-mod-memory.md).**
Same cause as D20C: the halting list address `0x80F40BD0` is in the mod
GPU-DMA aperture (the near clip's packet arena, region 2), which savestates did
not carry; in a fresh process the arena was not yet allocated and the link
folded into main RAM. Evidence: the 0-3 s sweep (nine delays) loads after a
restart 36 / 36 (Modernized and Vanilla, two source saves); D20A's four
recorded bad saves now load and run in a fresh process; existing saves load
(codegen hash unchanged); patch exported and verified. The manual's "play a
moment after a load before saving" warning is replaced.

### D20B - Statistics screen rebuilt; SAVE opens our SAVE GAME

Split from D20 (2026-10-21). Rescoped with the user on 2026-10-21 (option B
of the statistics screen page): "my inclination is to totally remove the
built in save feature in every way ... We absolutely dont need 2 places to
save", then "lets go for option B". Part 1 (this job): rebuild the
statistics screen as our own, in the original's look, with SAVE opening our
SAVE GAME; nothing in Modernized writes a game save to the memory card.
Part 2 is D20D (richer lines), with the counters that need research in D20E.
Vanilla keeps the original screen and card save. The card's settings
(D19N) are unaffected.

**Acceptance:** after a one-player level the statistics screen looks as the
original (lines, positions, wait, prompts); CONFIRM continues to the next
level; SAVE opens SAVE GAME, a save there returns to the statistics screen
and its slot loads back onto it; no memory card save is reachable in
Modernized; two-player and Vanilla unchanged; the user judges it in play.

**Done (user-accepted 2026-10-21: "i can confirm its now working and positioned correctly with the overlap ... i think we can close out the current work, just to break this task down a bit, as it is indeed working fine now."), [note 179](documentation/179-d20b-statistics-screen.md).** Follow-ups opened by the user: D20F (selectable CONTINUE / SAVE GAME) and D20D (richer statistics, already opened).

First implementation (2026-10-21):
Implementation: `game_menu.cpp` Page::Stats. The original mode 2 screen
(0x800277dc, challenge stages 0x80027214) still runs underneath: its timer
0x800d2670 is held at 300 so it never shows its prompts or reads the pad, and
its text call is pointed at the string's end. Our page is drawn at the flip
0x8001fcbc called from the run handler (new hook), with the original's own
text ids, flags and arithmetic, after the original's 300-field wait.
CONFIRM writes the original's Cross result (-1, its sound) at that flip;
SAVE (V) opens SAVE GAME (D19Q's page; Escape back, a save returns with
GAME SAVED). The picture is the level's last image, taken at gameplay's exit
0x800270e8 (new hook; by mode 2's first flip the screen is already black).
The slot sidecar records `level N` and `complete 1` (no `resume`); LOAD
GAME shows LEVEL COMPLETE instead of the era, and loading it opens the
statistics screen with its prompts at once. Identity guards added for mode 2
and the mode 1 exit. Two hooks regenerated; codegen hash 0x8bab543c
unchanged.

Evidence (offscreen, private cards and savestates
`recomp/analysis/d20b-stats`, slot 5 with the level ended by end code 4):
no prompts and X ignored during the wait; CONFIRM / SAVE after it, glyph for
glyph as the original's capture; V -> SAVE GAME; save to an empty slot and
over a used one (OVERWRITE THIS SAVE?) -> back with GAME SAVED; CONTINUE ->
DUKE HILL (level 1); F3 LOAD GAME shows TIME TO KILL / LEVEL COMPLETE with
the level's last picture, and loads onto the statistics screen, prompts
showing; CONTINUE from there -> level 1. Variants drawn by rewriting the
level index on screen: boss (no secrets row), challenge stage FAILED /
COMPLETED, final level (CONTINUE only), level 1 (no challenge row). Vanilla:
the original screen, Square opens the memory card SAVE GAME, Triangle
cancels. Python 164 OK; `ttk-input-test`, `ttk-near-test`, `ttk-sky-test`,
`ttk-aim-test`. Executable
`b217e8898cbfa52a2baabe0efa01d13a3cbea83d9b016950fff636a6757bbbf4`.

Testing aid: console `endlevel` (one-player play) ends the level as its exit
does (end code 4), refused where `level N` is; listed in `help` (framework
`main.cpp`, patch exported and verified against the pristine pin, 50 files).
Executable with it:
`c4bc40729ab7a1efaf7ac90b3ec8492bbdc7bd2a7ad2c9cf123c574a182bc82a`.

First playtest (2026-10-21, user): works; on SAVE GAME from the statistics
screen the date overlapped the line under the picture in a 1444x1020 window
(`research/screencaps/overlap.png`). Cause: the host image (picture, steel
date) was placed in the display-aspect (16:9) fit, but menu frames outside
gameplay are presented fitted at 4:3; in a window taller than 16:9 the two
differ (in a 16:9 window they match, hence not seen offscreen). Fix
(framework `gpu_gl_renderer.c`): the image follows the rect of the last
present that showed a picture. Checked in a real 1444x1020 window: aligned on
the statistics SAVE GAME (4:3) and the pause SAVE GAME (16:9). The title LOAD
GAME in tall windows had the same cause. Patch exported, verified (50 files),
codegen hash unchanged. Executable
`f9687072f1387bb2b343cc291aa0d2bbb6501ad87f5f53fdcdf681d0888d2866`.

Limits: offscreen only, the level ended by writing its end code (not by
reaching an exit in play); the challenge-stage and boss variants were checked
for drawing only. The original's 5-second wait before the prompts is kept
(part 1 is faithful); whether to shorten it can go with D20D. Two-player
statistics are the original's (they had no SAVE).

### D20C - Colours sometimes wrong after loading a savestate

Opened 2026-10-21 (user, after accepting D20): "when loading, sometimes the
colors are screwed up? i cant explain, try loading several times. i was
testing with my save in slot one." Intermittent; seen with quick load / LOAD
GAME of the player's UI slot 1. Not investigated yet. Reproduce on a private
copy of a comparable save (never the player's own files): load repeatedly,
capture the bad frames, and find which state the load leaves wrong (palette /
CLUT VRAM, texture page, renderer cache, D17 replay presentation). Check
whether F7 loads show it too and whether it relates to D20A.

**Acceptance:** repeated loads of the reported kind of save show correct
colours every time; the cause is recorded; Vanilla unchanged.

**Done (user-accepted 2026-10-21: "rock solid. it's working! the artifacting is no longer present upon load. i recreated the steps here and it's all fine."), [note 178](documentation/178-d20c-savestate-mod-memory.md).**
Reproduced on a private level 3 mine save with the player's settings (first
person, 120 Hz, OpenGL 4x): F6 once, then walk/turn and F9 repeatedly. Before
the fix, load 4 showed scrambled textures, load 7 wrong textures and colours
that persisted, and load 8 halted (`GP0 unknown command 0xF4`). Third-person
debug loads with no play in between were always clean.

Cause: mods can allocate guest-addressable memory (Expansion 1 and a GPU-DMA
aperture). The first-person near clip links its host packets (an arena in the
aperture) into the game's ordering table, but savestates did not save that
memory. A loaded ordering table then linked into whatever this process had
written there since; walking it drew garbage and uploaded into VRAM (textures
and CLUTs stayed wrong) or halted.

Fix: framework savestate section `BS_SEC_MODMEM` (both regions, allocation
records, bytes; restored on load when the layout matches; any unrestored part
of the aperture, and the whole aperture for older saves without the section,
is filled with end-of-list packet headers so stale links end the list).
`near_clip.cpp` reserves its arena at startup and mirrors its current region
in the saved memory, moving past the loaded region after a load. Codegen hash
unchanged: all existing saves load.

Evidence: 20 / 20 F9 loads of a new-format save and 20 / 20 loads of an
old-format save clean in first person (no FATAL, shots match); D20A sweep and
bad saves pass (above); `ttk-near-test`, `ttk-input-test`, Python 164 OK,
framework savestate guards; pristine pin + exported patch reproduces all 50
framework files. Executable
`03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`.

Limits: offscreen only; the player's own slot 1 was not used. An old-format
save's first frame after loading may be partly drawn (its packets are gone);
saving again stores the new format.

### D20D - Richer statistics screen

Opened 2026-10-21 (option B, part 2); confirmed as its own job by the user
when D20B was accepted. Builds on the D20F list layout. Add the lines
already known to our code: level name and difficulty, health and armor
carried, mission items found of total (D08A5), and a best time per level and
difficulty kept in our own file beside the savestates (never the card), with
NEW BEST when beaten. Possibly the cheats used this level (D08G). Layout in
the game's fonts; mock-up first, as the user approved the D19 menus.

**Acceptance:** a user-approved layout; every value matches the game; best
times persist across launches and are per difficulty; Vanilla unchanged.

**Done (user-accepted 2026-10-21: "accepted! ... close this work out, fully accepted").** D11G opened from the same message.

**Work log (2026-10-21, Needs playtest; [note 181](documentation/181-d20d-richer-statistics.md)).**
Mock-up drawn in the game (real fonts and values); user: "layout is approved",
"dont mention cheats", the game timer (rewound by savestate loads) "is fine",
and asked for the real `%` and `!` as in the font picker.
Implementation (`game_menu.cpp`, `draw_stats`): level name, DIFFICULTY:
(`0x800c3d7c`, the game's difficulty names), then 13 apart ENEMIES KILLED,
TIME TAKEN, BEST TIME, CONTINUES REMAINING, SECRETS FOUND, MISSION ITEMS
(D08A5 table via new `mission_tally`, held or used; levels that have them),
CHALLENGE FOUND, HEALTH / ARMOR (`62% / 0%`); the D20F list keeps its place.
Best times: `ttk-best-times.txt` beside the savestates, `level difficulty ms`;
a finished level records a first or shorter time and shows NEW BEST! (blue);
a savestate loaded onto the screen shows the stored best only. `!` and `%` in
the game's small font: the shipped D24C / D24E glyphs drawn as 1-row runs
copied from the font's own I, '.' and '/' (`Guest::say_runs`, D19T-style
alignment). Font picker: '(' ')' '%' and double quotes added from the
shipped builders, new section 16 (user request).
Evidence (offscreen, `recomp/analysis/d20d-stats/t1.py`): record, slower,
faster, other difficulty, persistence across launches, load onto the screen
(no NEW BEST!), CONTINUE -> level 1, mission 2 OF 5 with one used, 137% / 50%,
boss / challenge / final / level 1 variants, Vanilla original and file
untouched. Python 164 OK, `ttk-input-test`, `ttk-near-test`, `check_repo.py`.
No hook or framework change. Executable
`076f949136de1f29bd0675da8cefd3a1fd7222a73468880c6014e888ad803d5d`.
Limits: offscreen only; values set by memory writes, not a real level exit.

### D20E - Statistics research: enemy total, accuracy, damage

Opened 2026-10-21 with D20D. Find (or count in our hooks) the level's enemy
total for "kills X of Y" (difficulty and scripted spawns can change it),
shots fired and hit, and damage taken. Show a value only once verified
across several levels.

**Acceptance:** each counter is documented with its source and checked on at
least three levels; unverified ones stay off the screen.

**Done (user-accepted 2026-10-22: "lock it in, this is complete. close it out").**

**Work log (2026-10-22, Needs playtest; [note 183](documentation/183-d20e-statistics-research.md)).**
User (during the work): "i am not bothered about including accuracy ... we
can mention it in passing somewhere"; no backlog job for it. Damage taken is
parked with it (the original has no counter; it would need our own hook and a
savestate sidecar).
Kills `+0x3b6` come from the credit routine `0x8003d8a4` (Duke as killer
only). The enemy total is the level load's own rule for its enemies-left
count `0x800d253c` (`0x800242b0`: kind-1 placements, type flag bit 0, not
Duke, not 11 excluded types), applied to the placement table `*0x800de720`.
The live count is not used: the strip-club dancers (922, 1013) are counted
and credited as kills but never subtracted. Implementation: `game_menu.cpp`
`enemy_total`; ENEMIES KILLED reads `X OF Y` (plain count if the table does
not read sensibly). No hook, guard or framework change.
Evidence (offscreen, private cards `recomp/analysis/d20e-stats`): rule equal
to the game's count on levels 0, 1, 2, 5, 6, 9, 21 (60, 58, 47, 59, 44, 30,
18) and the same on difficulty 0 and 3 (levels 0, 1, 5, 9); real kills with
the original auto-aim on levels 0, 1, 5, 9 (each enemy kill: kills +1, left
-1; dancers: kills +1, left unchanged); death + Continue keeps kills and the
total; the table intact on the statistics screen; Modernized screen shows
`1 OF 60` (level 0 after a kill) and `0 OF 59` (level 5). Shots `+0x3b2` =
ammo used, weapon slots 1-11; hits `+0x3b4` count pellets, splash, props and
self-damage but not flyers (note 183). Python 164 OK, `ttk-input-test`,
`ttk-near-test`, `check_repo.py`. Executable
`156b7e93695588bf65a8e1ed64957a1dbe1686d07adea819665668c2d79f61ef`.
Limits: no level was fully cleared (X = Y not seen in play); dancers are part
of Y as the game counts them; levels ended with `endlevel`.

### D20F - Statistics screen choices as a selectable list

Opened 2026-10-21 (user, after accepting D20B): "the statistics screen
firstly using "V" is legacy controls, and is in no way clear to the end user,
who currently we can assume is using a mouse and keyboard, that V is the way
to invoke the save screen for example. so can continue and save game be made
selectable?" Replace the PS1 prompt pair (Cross CONFIRM / Square SAVE) with
a highlighted list in the style of the pause and death lists: CONTINUE and
SAVE GAME (CONTINUE first), Up / Down and the wheel move, Enter / X choose.
CONTINUE only where the original offers no SAVE (before the final ending,
two-player keeps the original). V may stay as a shortcut. Keep the
original's wait before the choices appear, or settle with the user whether
it should be shorter. Layout to agree with the user first (the five lines
may need to move up for the list).

**Acceptance:** with only a keyboard and mouse the player can see and reach
both choices; CONTINUE and SAVE GAME behave as in D20B; the user approves the
layout; Vanilla unchanged.

**Done (user-accepted 2026-10-21: "thats so much better! lock it in, close it out, 10/10"), [note 180](documentation/180-d20f-statistics-list.md).**
Implementation (`game_menu.cpp`, Page::Stats): the five lines sit 25 higher;
under them, at a fixed place (row + 95, 19 apart, menu font, blue
highlighted) CONTINUE and SAVE GAME, with SELECT / CONFIRM at the bottom
(the death list's prompts). Up / Down and the wheel move, Enter / X chooses;
V still opens SAVE GAME. Escape on SAVE GAME, or a save there, returns with
SAVE GAME highlighted (as the pause list returns to its item). CONTINUE only,
already highlighted, where the original offers no SAVE (before the final
ending, or a Vanilla card save on the page). The list appears after the
original's wait, unchanged; keys during the wait do nothing. Correction to
D20B: that wait is about 1 s, not 5 s (300 fields at 10 per tick, measured
in Vanilla and ours alike).

The user offered a simpler alternative while this was built (Enter for
CONTINUE, F2 for SAVE GAME as prompts); the list was already working, so it
is presented for the layout decision.

Evidence (offscreen, private cards `recomp/analysis/d20b-stats`, `f1.py`,
`f2.py`, `f5.py`): Down, wheel up / down move; Enter on SAVE GAME opens
SAVE GAME, Escape returns on SAVE GAME, Up + Enter on CONTINUE -> DUKE HILL;
save to slot 9 -> back on SAVE GAME with GAME SAVED (sidecar `level 0`,
`complete 1`); level 29 -> CONTINUE only, Down does nothing; V still opens
SAVE GAME; Down / Enter / X during the wait ignored. Vanilla (`v1.py`):
original screen, Square card SAVE GAME, Triangle back. Python 164 OK,
`ttk-input-test`, `ttk-near-test`, `check_repo.py`. No hook or framework
change; codegen hash unchanged. Executable
`3e1b96bc8faab8834e9c9e2ed3ffaceb236ed72de7e4c2fcd7142b1be13be496`.

Limits: offscreen only; the user judges the layout in a window.

### D20G - UI slot 11 reload fatal GPU halt: regression assessment

**Done (user-accepted 2026-10-24).** Investigation closed, not a claimed fix.
User requested retaining the lessons for any similar recurrence. 41 private
restores per build (32 diagnostic, eight menu plus setup), no fatal on accepted
D08T7 or pre-D08T7 D08T5. Cause and regression origin remain unresolved. No
behavioral fix justified; concrete packet/aperture diagnostic follow-up in
[note 213](documentation/213-d20g-slot11-load-halt.md). If another savestate
reload produces a GPU halt, bad colours or scrambled geometry, consult this
Done job and its recurrence checklist before investigating anew. D08T8 remains Todo.

At 08:46:44 BST, UI slot 11 (internal/file index 10) restored successfully, then
halted on GPU GP0 opcode 0x1F, word 0x1F20161D. Same slot loaded earlier in the
session. Preserved evidence and exact facts:
[note 213](documentation/213-d20g-slot11-load-halt.md).

Scope: determine whether this is a D08T7/recent regression, a pre-existing
save/load defect, or presently unreproduced; decide whether a fix is necessary.
Compare the same private save copies and relevant session/load sequence on the
accepted D08T7 build and pre-D08T7 D08T5 baseline. Review D20A/D20C restore fixes,
GPU/DMA state and host history. Do not infer causation from opcode alone or
silently suppress it. Preserve player files and Vanilla.

Acceptance:
- Report reproducibility and baseline/candidate results with hashes, settings,
  actual load sequence, and which findings are observations versus hypotheses.
- Explain the best-supported failure path and whether a fix is justified. A
  non-reproduction is not proof of absence or proof that a save is corrupt.
- If unresolved, preserve a concrete diagnostic next step; if a fix is needed,
  scope a bounded follow-up with regression tests rather than guess a patch.
- Record a reviewable investigation result before returning to push/pull work.

### D20H - Slot-5 intermittent extreme colour corruption on load

**Status: Done (user-confirmed 2026-10-30; earlier: cause found and fixed in D17AM4, [note 326](documentation/326-d17am3-am4-unified-default.md)). Related: D20C, D37A.**
D17AM4: reproduced on a private level-2 pose pair (load mid-frame state,
captures, load another): corrupted colours, then a GP0 0x1F halt. A load
resumes the saved frame, whose OT links into the mirrored host packet
region, but the near path kept this process's cursor until the next frame
boundary and could overwrite those packets. Now re-synchronised at every
hook: the rest of a loaded frame goes to the next region. HEAD unified 3
corrupted captures and 1 halt in 27 cycles; fixed 0 in 50.
Private current UI slot 5, repeated loads and screencaps/crazy-colors.png. Record attempts/outcomes and transient versus persistent corruption; inspect restore/resource/state lifecycle independently until a shared cause is proven.

Reproduction/fixture: pending inventory. Cause: unconfirmed. Changes: none at
registration. Validation: pending. Remaining: reproduction, diagnosis, correction
and appropriate visual/regression evidence; preserve player originals.

### D20I - LEVEL STATS (time, kills, secrets) in the bottom-left corner; LEVEL STATS and FPS toggles in DISPLAY

**Status: Done (user-accepted 2026-11-03: "lock it in and close it out, this is awesome!"; [note 376](documentation/376-d20i-level-stats.md)). Dependencies: D20E, D20D, D24A.** EDuke32 shows three
small lines above its status bar in the bottom-left corner: `T: 3:03.39`,
`K: 12/36`, `S: 2/7`, the letters red and the values a light gold-beige
(reference: `research/screencaps/level-stats.png`). The user wants the same
in Modernized play, positioned similarly (bottom left, above the HUD,
following the presented 4:3 / 16:9 rectangle and the HUD scale).

Initial research: the values already exist from the statistics screen work
([note 179](documentation/179-d20b-statistics-screen.md),
[note 181](documentation/181-d20d-richer-statistics.md),
[note 183](documentation/183-d20e-statistics-research.md)): the level timer
is player `+0x398` in ms (a savestate load rewinds it), secrets found are
`+0x3b0` of `0x800beb18` with the total the statistics screen shows (levels
27-30 have no secrets row), and kills X OF Y use the D20E level-load enemy
rule. Font: the console `fps` block uses TTK Medium Italic with the project
palette "Console steel" and a drop shadow (GAME_MANUAL, console section); the
D24A font picker (`research/TTK-UI-and-Font-Research.html`, local) documents
how disc fonts take a palette and how a project palette was added. Plan: the
same font, a red palette for the letters and a gold palette for the values;
if the disc's red is too dark at this size, add a bright red project palette
the way Console steel was added. Show the user swatches before building.
Settings (user, 2026-11-02): the feature is called LEVEL STATS. It gets an
ON / OFF row in OPTIONS > DISPLAY (`game_menu.cpp`, the DISPLAY page beside
WINDOW, RESOLUTION, FRAME RATE), saved in the Modernized profile with the
usual schema migration, applied at once. In the same job, add an FPS row to
DISPLAY that turns the compact statistics block on and off, the one the
console `fps` command toggles today (framework console, `main.cpp`); the row
and the console command must stay in step and the choice is saved. Open
questions: the time format (EDuke32 shows minutes:seconds.hundredths), levels
without secrets, whether LEVEL STATS hides with the HUD levels (D19U1), and
where LEVEL STATS sits when the FPS block is also on (FPS is top left).

User advice (2026-11-02): in EDuke32 the kill total is dynamic; it can go up
during a level when enemies are added after the start. Do the same, clean
room (from observed behavior, not EDuke32's source): Y counts enemies that
exist or have existed this level, so a spawned enemy raises it. D20E's Y is
the level-load rule over the placement table and is fixed; it found one
runtime increment of the game's own enemies-left count (challenge stage 25,
type `0x7f`) but did not survey mid-level spawns (spawners, triggers,
reinforcements). The job must find where the game creates enemies after the
load and count those that pass the same rule, keep the dancer / non-hostile
exclusions consistent with D20E, and decide whether the statistics screen's
`X OF Y` follows the same dynamic total so the two never disagree.

**Acceptance:** three lines in the bottom-left corner during Modernized
gameplay that match the statistics screen values (time, kills X/Y with the
dynamic total, secrets X/Y), readable red and gold in the TTK font, correct in
4:3 and 16:9 and at every HUD size, no effect on frame rate; OPTIONS > DISPLAY
has LEVEL STATS and FPS rows that apply at once and persist across launches,
FPS in step with the console command; GAME_MANUAL updated; Vanilla unchanged;
the user's playtest.

### D20J - LEVEL STATS make room for status panels (oxygen meter and others)

**Status: Done (user-accepted 2026-11-06; [note 406](documentation/406-d20j-air-box-right.md)). 2026-11-06: the user chose to move the air box to the right with the other countdowns instead of moving LEVEL STATS; see the work log. Dependencies: D20I.** User (2026-11-05): "the oxygen meter
goes behind the stats panel. that stats panel needs to act dynamically, as
with our other HUD elements, and the oxygen meter or any other status panels
that want to appear there move it upwards." Reference screenshot named
`stats-oxygen.png` (screencaps folder); not yet on disk as of 2026-11-05, so
ask the user for it before starting. The D20I `T:` `K:` `S:` block (bottom
left, above the HUD, [note 376](documentation/376-d20i-level-stats.md)) is
drawn over the oxygen meter underwater. Find every native panel that can
appear in that bottom-left band (oxygen meter first; also check the jetpack
fuel, the held item / inventory panel, boss or mission meters, and the
status-bar styles) and lift the LEVEL STATS block above whichever is showing
that frame, through the same placement the other HUD elements use, following
the presented 4:3 / 16:9 rectangle and the HUD scale. Move it smoothly or in
one step as the user prefers, never hidden by a meter. Test underwater with
the oxygen meter, then with the others, at 60 and 180 Hz; LEVEL STATS off
and Vanilla unchanged.

### D21 - Reworked SOUND menu and configurable C held crouch

**Status: Done (user-accepted and closeout authorized 2026-10-27).**
Rework SOUND in the existing TTK menu style: MASTER, SOUND EFFECTS, MUSIC,
SPEECH and stereo/mono MODE. Reuse existing audio owners and confirmed-settings
persistence. C is HELD CROUCH: press to keep crouching, press again to stand.
Ctrl or Space cancels the toggle without immediately crouching or jumping;
release and press again for the normal action. It is configurable in KEY
BINDINGS; Left Ctrl remains momentary. Custom mappings
are preserved. This supersedes the initial mistaken secondary-hold implementation.

User explicitly rejected the accessibility panel. No new accessibility screen,
view-bob relocation, text sizing or subtitle work in this job. The user later
clarified that the requested C binding is a crouch toggle.
Concurrent voice/music remains D18B, not a prerequisite for this menu redesign.
[Note 282](documentation/282-d21-accessibility-sound-design.md) records the current
scope, source findings, preview and verification requirements.

**Acceptance:** correct channel/action behavior, confirm/cancel and persistence;
original-style title/pause menu at 60/120; configurable held C crouch with safe
migration and overlapping inputs; existing options and Vanilla preserved.

### D21A - Momentary crouch takes over the C toggle

**Status: Done (user-accepted; closeout authorized 2026-10-27). Dependency: D21 (Done).**
User confirms D08K12 works, then requests this focused D21 correction: while
C-toggle crouch is active, pressing the configured momentary crouch action
clears the toggle but keeps Duke crouched for the entire hold. Release requests
standing through existing clearance checks. Preserve Space cancellation,
ordinary momentary crouch, repeated input, custom keyboard/mouse bindings,
save/load behavior and D08K12 crouch running. Acceptance: no standing on Ctrl
press/hold; stand on release where there is room; low clearance stays safe.
User accepted the correction and authorized closeout, commit and push.

### D26 — Backtick debug console (fps and helpers)

**Done — explicit user acceptance (2026-09-28).** Console look and behaviour signed
off: scrollback echo, multi-line `help`, unknown/errors in console only, `fps`
persistent top-left debug block, `clear` / `quit`. F unbound; Scroll Lock is holster.

**Acceptance met:** backtick open/close; scrollback; persistent fps overlay;
gameplay/capture/menus intact.

### D26A - Debug level-select panel for whole-game testing

**Accepted - 2026-10-04.** User: "excellent! mark as accepted." Backtick console `levels` lists the 21 levels
the original title level-select cheat offers (game names from `0x800c3d2c`);
`level N` ends the current level with the pause menu's restart code (0xfd) and
switches the index inside the original mode 1 init, which loads level N.
Offscreen: all 21 levels load in one session with their own overlay, Old West
round trips, savestate save/travel/load, Vanilla, pause/title refusals. Candidate
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`. See
[note 101](documentation/101-d26a-level-select.md).

User request, 2026-10-04: testing infrastructure so the user can
travel through the whole game, find problems, make savestate reproductions and
stand up small jobs without replaying normal progression.

Required: open a debug level selector, see the available levels, select one,
enter it quickly, then play normally and use savestates.

- Keep the UI simple: the cheapest reliable debug list (D26 console, a host
  overlay or an existing PSXRecomp/runtime debug menu primitive). Proper menu
  style is D19, much later.
- Use the game's own level identifiers/data and its own level-start path where
  possible (LEVELxx/DBxx tables, the level index at `0x800be570`); avoid
  duplicated hard-coded lists.
- Development-only shortcut: it must not change normal progression, saves or
  memory cards, and is opened by an explicit debug hotkey or console command.
  Document how to open it in the manual.

**Acceptance:** every selectable level loads into normal play from the panel,
savestates work there, normal progression and saves are untouched, and the
user can use it to survey the game (enabling D22B, D11D, D12B and D17R
verification).

### D26B - Own view and control selections across transitions

**Done (user-accepted 2026-10-23).** Implementation and verification: [note 207](documentation/207-d26b-view-ownership.md). Expanded by the user from the console report to
ownership of first/third-person view and Modern/Legacy controls across console,
menus, focus, saves/loads and gameplay transitions. Investigate all writers and
separate persistent player intent, temporary input suspension, and authenticated
camera/state support. Preserve explicit selections and identity guards; do not
silently substitute Legacy controls when input capture is temporarily absent.

**Acceptance:** console/menu/focus and save/load routes preserve the selected
view and control scheme, with reliable automatic recovery and no unintended
third-person flash caused by input suspension. Explicit settings changes still
work. Test first/third person and Modern/Legacy, paused and gameplay saves,
repeated/rapid transitions and supported traversal. Explain genuine scripted or
unsupported camera overrides and verify return to the selected view. Vanilla
remains unchanged. User gameplay confirmation required.

### D26C - Console command history

**Accepted (2026-10-06): "im happy with all progress tonight".** Built the same
evening the user asked again ("please allow me to press up in the console to get
the last command typed. also a history command would be pretty neat too"):
Up/Down recall (newest first; Down past the newest clears the line) and a
`history` command (64 kept, consecutive duplicates once), in the framework
`main.cpp` console. Checked through the real console: Up recalled
`spawn steroids` and Enter ran it; `history` listed both commands. See
[note 111](documentation/111-d26e-debug-spawn.md).

**User request, 2026-10-05.** Up Arrow recalls the
previous command, Down Arrow the next, like a standard shell. Do not
over-design it.

**Acceptance:** Up/Down walk the session's entered commands; editing and Enter
behave as before.

### D26D - Level select: authoritative order, numbering, names and categories

**Todo. User request, 2026-10-05 (most important tooling issue from the
game-wide playtest).** The D26A list follows the title cheat's level indices
(0-3, 5-12, 21-29), which do not match the game's campaign numbering: there are
gaps (no 4) and odd numbering later on, so "Level 12 has this bug" is
ambiguous. Do not fill gaps by guessing.

Use the game's own data as the authority: level index and overlay tables
(`0x800be570`, file id `0x1ad + index`), the name table (`0x800c3d2c`), the
next-level function `80027fc0` (campaign order, challenge-stage and boss
branches), the title cheat cycle `80022d48` and any other level-select or
indexing structures the original has; reuse them rather than an invented
numbering. Cross-check the campaign progression with the complete guide
<https://gamefaqs.gamespot.com/ps/197177-duke-nukem-time-to-kill/faqs/3834>,
which also documents the original's cheat level select (regular levels,
bosses, challenge stages).

The console should distinguish campaign levels (gameplay order, campaign
number and name), boss levels (chronological position or clearly marked),
challenge stages (separate or clearly labelled), and other valid maps
(multiplayer, test, unused, special) that load safely, listed separately.
For each entry record the display/campaign number, name, internal level/map
ID, category and chronological order. Keep the UI simple; correctness and
reproducibility matter, not styling. Keep a way to address the internal
index so existing notes stay usable.

**Acceptance:** the list matches the game's own progression data and the
guide, every entry loads as before, earlier reports that used console indices
(D22C, D23C, D23D, D08T2) are re-mapped to the new numbering; the user
confirms.

### D26E - Debug spawn console command

**Accepted (2026-10-06): "im happy with all progress tonight".** User: "is there a command we can use that can
summon one of these items in front of duke? for debugging purposes", then "yes
spawn indeed", pointing at EDuke32's `spawn`
(https://wiki.eduke32.com/wiki/Spawn). For the D24A icon work.

`spawn <item|type>` and `items` in the backtick console (Modernized only).
The item is created about 600 units in front of Duke by the original
`CreateObject` (`0x80095a74`), the routine a destroyed container or enemy uses
to drop its item. Types whose model the current level has not loaded are
refused. Details: [note 111](documentation/111-d26e-debug-spawn.md).

**Acceptance:** in Modernized gameplay, `spawn` puts a named or numbered item in
front of Duke, it is picked up normally, and refusals are clear (unknown item,
not in this level, Vanilla, not in gameplay). Vanilla and the level select are
unchanged. The user confirms it is useful for the icon work.

### D26F - Spawning in-game items, continued (mission items)

**Done on user acceptance (2026-10-07): "this is awesome. it works! the
crystals really do work."** Built as approved: `spawn 1761` / `2761` /
`3761` drop the real red / blue / green crystal, collected by walking over it.
Also the project's first custom-pickup contract (user: "the wider effect here
is we're now about to learn how to make custom pickups. so document this
heavily"): [note 117](documentation/117-d26f-crystals.md), "Custom pickups".
Research, as first logged: User: "in the first level (which you can easily reach by going to slot
4), can you check why spawning the green crystal doesnt show it in the
inventory? spawning the subway key on the other hand does show it in the
inventory. My guess was that the crystals are all palette swapped versions of
the green crystal and the in game items have special properties", then "we can
stand this up as a logged research job too. 'ability to spawn in game items
continued' or something".

**Findings (verified).** See [note 117](documentation/117-d26f-crystals.md).
- `spawn green energy crystal` makes type 761, a walk-over pickup whose original
  dispatcher case sets inventory item 14 (`+0x38c`) and shows message 115
  "GREEN ENERGY CRYSTAL". The level's crystals are items 11-13, so nothing named
  appears. In level 1 item 14 is the Scrap of Paper and in level 6 the first
  Family Jewel. Where a level loads 761's model, the same spawn would mark
  those as found (not tested).
- The real crystals are different objects: types 176 (red), 177 (blue) and 178
  (green). They are not walk-over pickups. Each sits in a holder (types
  180/181), and Duke takes it with the action button. The original routine
  `0x80091cec` puts it in his hand and then sets item 11, 12 or 13.
- Palette swap: correct. The three crystal models are the same mesh and texture
  page. Only each face's CLUT differs. Type 761 uses the green crystal's model.
- The subway key (153) works because its dispatcher case sets item 6, which
  level 0 names.

**Approved design (2026-10-07, user: "that's how it should work, i love it!").**
Spawnable mission crystals. It combines what the cheats show (a crystal counts
once its flag is set) with `spawn` (the real crystal object in front of Duke).
1. `spawn 1761` / `2761` / `3761` (or `spawn red/blue/green energy crystal`)
   creates the game's own crystal object, type 176 / 177 / 178, in front of
   Duke with the original `CreateObject`.
2. These types have no walk-over pickup, so our code supplies one:
   - touch: Duke within the original pickup range of a crystal we spawned;
   - collect: set item 11 / 12 / 13 (one item, as the cheat does);
   - feedback: the original message (113 / 114 / 115) and pickup sound
     (`0x7005`), written the way the original pickup does;
   - remove: the object leaves the world through the game's own routines.
3. The Select inventory, the D08A5 mission inventory and the receptacles read
   the flag, so they follow with no extra code.

Rules:
- `spawn 761` stays the original generic crystal (item 14). In crystal levels
  the console notes which item it really sets.
- 1761-3761 work only in levels 0, 5 and 9. Elsewhere they are refused clearly.
- If the crystal is already found, the spawn still works and says so.
- Console only (Modernized). Vanilla is unchanged.

Open points for the build: pickup range and the original sound, message and
removal routines; crystal models in levels 5 and 9; receptacle acceptance in
play; a spawned crystal across savestate load or level change (forgotten).

**Acceptance.**
- In levels 0, 5 and 9, each of 1761 / 2761 / 3761 appears as the right colour
  and is collected by walking over it, with the original message and sound.
- The crystal then shows in the Select inventory and the mission inventory, and
  the level's receptacle accepts it.
- Refusals and notes are clear (other levels, 761, already found).
- Other spawns, Vanilla and the level crystals' own take are unchanged.
- The user confirms in play.

### D26G - Console key repeat

**Done (user-accepted 2026-10-20): "lock it in and close it out!!".** Held Backspace and held characters now take
the OS key repeat (framework `main.cpp` `debug_console_event` no longer drops
SDL repeat events for them). Checked with real X keys; see the 2026-10-20 work
log. User request, 2026-10-08:** "Hold down backspace or other characters
within the console to repeat". Holding Backspace or a character key in the
backtick console should repeat it after the usual delay, like a desktop text
field (SDL key repeat or a host timer), without affecting gameplay input.

**Acceptance:** held Backspace deletes repeatedly and held characters repeat
in the console; gameplay keys and command history (D26C) unchanged; the user
confirms.

### D26H - Spawned weapons land far enough ahead of Duke

**Status: Done (user-accepted 2026-11-02). Dependencies: D26E (debug spawn).** User: "make sure the
weapons spawn a little in front of duke because when i spawn a flamethrower
for example, duke automatically picks it up." `spawn` places the item
`spawn_distance` (600 units) along the camera view (`recomp/src/ttk/spawn.inc`),
then it bounces and settles like a dropped item; a weapon such as the
flamethrower can end up within Duke's pickup reach straight away.

Initial notes (not researched): measure where spawned weapons settle versus
Duke's pickup radius, per weapon type; place weapons (and possibly all
pickups) far enough ahead that a standing Duke does not touch them, without
pushing them through walls or off ledges (fall back to the original distance,
or refuse, when the spot ahead is blocked).

**Acceptance:** spawning each weapon in front of a standing Duke leaves it on
the ground in view until he walks onto it; walls, ledges and other items still
behave; the user's playtest (flamethrower).

### D26I - Console: the numpad types

**Status: Done (user-accepted 2026-11-03: "confirmed working"). Dependencies: D26 (console), D26G (key repeat).** User:
"make the numpad work with the console." The console's key handler
(`debug_console_event` in the runtime `main.cpp`, a framework file tracked
through `recomp/patches/time-to-kill-accepted-source.patch`) maps only the
main-row letters, digits, space, `-` and `=`; keypad Enter already runs the
line, but keypad digits and other keypad keys type nothing.

Initial notes (not researched): map keypad 0-9 (with and without Num Lock, as
a desktop text field does), and decide on keypad `-`, `+`, `.`, `/`, `*`;
keep repeat (D26G) and make sure keypad keys still never reach gameplay while
the console is open. Framework edit: export the runtime patch afterwards.

**Acceptance:** `spawn 641` typed on the numpad spawns the freezer; keypad
keys repeat like the main row; nothing reaches gameplay while the console is
open; the user's check.

### D26J - Console keeps Duke's movement state (crouch, jetpack)

**Status: Done (user-accepted 2026-11-02; [note 362](documentation/362-d26j-console-input-hold.md)).
Dependencies: D26 (console), D26B (view ownership across the
console), D08K (crouch), D08Q (jetpack). Shares the console input path with
D26K.** Priority medium.

Observed (user): crouched Duke stands up when the console opens; Duke flying
the jetpack falls to the ground when the console opens. Expected: both states
continue unchanged while the console is open and after it closes.

Investigation:
- Confirm which of these causes it: the release itself (`input_release` /
  `clear`), the inactive frame (`frame.active`), the new epoch (jetpack fall
  grace, `jet_epoch`), or `crouch_toggle_context()` clearing `crouch_latched`
  in `input_frame()`.
- Separate toggle states (C crouch latch, jetpack on/off, altitude hold) from
  held inputs (Ctrl crouch, Space/climb, movement keys). A held key cannot
  stay held after its key-up is consumed by the console; decide whether it
  freezes or is released, and keep the C toggle and the jetpack itself.
- For the jetpack, decide how flight is kept while the console has focus:
  hold the current altitude (the D08Q6 hold) with no thrust, rather than
  losing the owner.
- Audit, without fixing beyond scope: swimming, ladders/poles, push-pull grab,
  precise aim and steroids.
- The frame should be "active, no new input" while the console is open, not
  "inactive"; console keys must still never reach gameplay.

Acceptance (private save copies; offscreen first, then the user's check):
1. Toggle-crouch with C, open the console, wait 10 s, close it: Duke stays
   crouched throughout, with no stand-up animation.
2. Fly the jetpack at a held altitude, open the console, wait 10 s, close
   it: Duke stays at that altitude (no fall), with the jetpack running and
   fuel use as the original's while hovering.
3. With the console open, type W, A, S, D, C, Space, E and the digits: none
   moves Duke, crouches, jumps, uses or switches anything.
4. After closing, the next gameplay input works on the first press, and the
   view/mouse capture returns as today (D26B).
5. Holding Ctrl (momentary crouch) or W while opening the console has a
   documented, consistent result (released, not stuck).
6. Vanilla unchanged; `ttk-controls-test` and `ttk-input-test` pass, with a
   new native group for the console keeping crouch and jetpack.

### D26K - No weapon holster while the console is open

**Status: Done (user-accepted 2026-11-02; [note 363](documentation/363-d26k-console-idle-hold.md)). Dependencies: D26 (console). Shares the console input path
with D26J (likely fixed together, tested separately).** Priority low-medium.

Observed (user, reported again after D08A33): with the console open, Duke
eventually goes idle and holsters his weapon. Expected: the equipped weapon
stays in its ready pose while the console is open, and closing the console
causes no draw or holster.

Known: during D08A33 this did not reproduce offscreen (slot 12 copy, about 25
s open, typing and clicks): equipment 2, the drawn weapon and the ready loop
did not change ([note 361](documentation/361-d08a33-equip-pickups.md), "Reported, not reproduced").
Get the user's exact steps (weapon, view, time open, mouse or pad).

Investigation:
- Find what holsters the weapon: the original idle timer/animation (an
  inactivity count), or a Modernized holster path that runs on an inactive
  frame or a new input epoch (`holster_pulse`, interaction and push state
  cleared by `clear()`).
- Determine whether console time counts as player inactivity, and suppress
  only the console's share without changing normal idle behaviour.
- Check that opening/closing the console never resets the weapon animation
  (upper-body animation `+0x74`, equipment `+0x3b8/+0x3b9`).

Acceptance (private save copies; at least two weapon slots plus fists/a
gadget, first and third person):
1. Open the console for 60 s (longer than the idle holster time, measured
   first with the console closed): no holster, the weapon stays visible
   ready.
2. Close the console: no draw or holster animation; firing works at once.
3. With the console closed, the original idle holster still happens after
   the same time as before.
4. Vanilla unchanged; native controls tests pass.

### D29 - Progression items and objectives legibility (research and design first)

**Cancelled 2026-10-30 (user decision): covered by D08A5/D08A18/D08A19.** The
mission inventory on `,` / `.` answers the job's core questions: it lists every
mission item the level needs (papers included) with found/total, unfound items
as silhouettes, a name card with FOUND / NOT FOUND YET / USED, Enter to use the
browsed item at its lock and a green tick once used, all kept by saves. The
Level 2 papers are combo pieces the game consumes at the lock, not codes to
read, so "what information did it contain" does not apply. Not pursued: a
distinct pickup notification, rewritten objective text and a separate
campaign-wide legibility audit; D22H still verifies every progression item and
gate works. A concrete gap seen later becomes its own small job. The original
brief is kept below for reference.

**Opened 2026-10-06 from the user's Level 2 playthrough.** A gameplay
modernization and player-legibility job, not a conventional bug. Modernized
profile only; Vanilla keeps the original presentation.

**Use case (Level 2, bank vault).** Duke collects pieces of paper whose
contents are needed later to open the bank vault. The original game tracks
this well enough for the level to work, but in the user's playthrough:

- it was not clear that an important piece of paper had been collected;
- there was no lasting way to inspect what had been collected;
- nothing said the papers held information relevant to the vault;
- at the vault, little connected the earlier pickups to what the game
  expected.

The user needed an external walkthrough. The recomp should not require one
just because progression information is effectively invisible.

**Design principle: remove unnecessary obscurity without removing
discovery.** No quest markers, no automatic puzzle solutions, no reduction
of the game to following instructions. The player should have access to
information Duke has already acquired: if Duke picked up and read a note,
the player can inspect what was on it and make the connection themselves.
For example, "Picked up: Torn Note" (or whatever matches the real item),
then later inspect it and see its numbers. Do not say "this is part of the
vault combination" unless the original context or a carefully improved
objective genuinely warrants it.

**Phase 1 (this job): investigation and design. Do not build the full system.**
Design game-wide; Level 2 is the reproduction case, not a hard-coded target.
Verify addresses and the active overlay (LEVEL02.OVR) before any hook, and use
private profile and save/card copies with the level-select panel (D26A).

Document, in a new engineering note:

1. **How the Level 2 paper/code/vault progression works internally**: what
   happens when each paper is collected; whether the papers are inventory
   items, flags, pickups, level-state variables or scripted events; whether
   the code or numbers exist as data (and whether they vary per game); how
   the vault checks progression.
2. **What items and information the game currently tracks** (inventory
   slots, key items, per-level flags), and whether the game already holds
   item names or descriptions that are never shown.
3. **What feedback the original game gives** on pickup (message, sound, HUD
   change) for progression items versus ammo and health.
4. **Whether the game's own item/progression data can be the source** of
   the information (read original state rather than inventing a parallel
   record). This is about data only: the D08A1-A3 inventory switcher is
   **not** to be extended for progression items (user, 2026-10-06).
5. **Objectives**: how the original Objectives screen stores and selects its
   text; whether text can safely be changed, extended or updated as the
   player discovers things (and the save compatibility of doing so).
6. **Game-wide audit** of similar opaque progression: documents, codes, keys
   or unusual objects, switch sequences, discovered information, invisible
   progression state, things Duke has seen or read, pickups whose purpose is
   not communicated. List them with level and type; do not redesign them in
   this job.
7. **A proposed lightweight game-wide inventory/information model**: which
   items qualify as "progression/information" items, what the player can
   inspect (name, what it contained), persistence across saves, loads and
   level transitions, and how it is derived from original game state.
8. **Presentation options** (no final visual design yet): a brief pickup
   notification distinct from ammo and health (text, sound or HUD cue),
   persistent inspectable entries (HUD panel, pause-menu inventory or
   objectives screen section), or a combination. Must not be intrusive. Uses
   the Duke font assets (D19A); later visual language belongs with D19.
   The user is considering presentation options and will bring them to
   this job (2026-10-06); fold those in before proposing a design.
9. **Objective guidance recommendation**: how much extra guidance to add
   without spoiling puzzles. Favour goals ("Find a way into the bank vault.")
   over solutions ("Collect all three notes ... enter 1234."). Objectives may
   become slightly more specific after relevant discoveries, never revealing
   the full answer.
10. **The smallest sensible first implementation to build and playtest**,
    likely the pickup notification plus inspectable entries for the Level 2
    notes through a game-wide mechanism, scoped as a follow-up job.

At minimum the eventual system must let the player answer **"What important
things have I collected?"** and **"What information did those things
contain?"**

**Acceptance (phase 1):** the engineering note answers items 1-10 with
evidence (addresses, overlay, data, observed behavior in a test run), the
audit list covers the full campaign as far as the level select can reach,
and the user reviews and approves the proposed model, presentation options,
objective guidance level and first implementation scope. Implementation
jobs are added to the board from that approval; nothing player-facing ships
in this phase.

### D42 - Death state and menu: stay on the death scene, Open opens the menu, unlimited continues

**Status: Done (user-accepted 2026-11-03; [note 373](documentation/373-d42-death-scene.md)). Dependencies: D19H, D19P, D19M, D19K, D26 (console). Prerequisite of D08G9.**
Today a death in Modernized goes straight to the D19H death list under the
original's "N CONTINUES REMAINING" (`game_menu.cpp` `draw_death`, menu value
`0x800be56a` = 15, continues byte `0x800d7411`). The user wants Duke 3D /
EDuke32 behaviour with TTK's own menu:

1. **Death scene.** Duke dies and the game stays on the death scene; no menu
   opens. Third person keeps the original death animation and camera.
   First person shows a floor-level view of the scene, as Duke 3D does (see
   decision 1). A short line at the top of the screen, in the existing
   notice style, reads like "PRESS <key> TO OPEN MENU", naming the key bound
   to USE / MANTLE (`interact`, D19G KEY BINDINGS), not a fixed E.
2. **Menu.** Open/Interact on the death scene opens the existing death list
   (D19H style, no EDuke32-style menu). Escape on the list closes it and
   returns to the death scene; Escape in its prompts still backs out one
   level (D19D). Duke stays dead throughout; opening, navigating and closing
   the menu changes nothing.
3. **Unlimited continues.** CONTINUE always works; the "N CONTINUES
   REMAINING" line and every other countdown go. Keep CONTINUE, LOAD GAME,
   RESTART LEVEL, QUIT TO MENU and QUIT GAME (and the challenge stages'
   REPLAY LEVEL / GIVE UP) with their established meaning (note 147 table:
   CONTINUE `0x800d71cd` = 2, RESTART `0x800c3f34`, QUIT TO MENU via the game
   over screen, note 153). Removes the 2026-10-19 "keep CONFIRM CONTINUE for
   now" interim recorded under D19H.
4. **Console and cheats.** The backtick console opens and closes on the death
   scene (and over the menu, or the menu yields to it; decide) without
   starting a respawn, restart or menu action. Typed `dnkroz` reaches the
   cheat system while dead; D08G8 currently refuses every cheat while dead
   with a message, which stays until D08G9 adds the revive.

Investigate first: how the original reaches menu 15 from death (the death
flag `0x800be55c`, the delay before the list, D35L's actor death flags that
drop the modern first-person view), what CONTINUE restores versus RESTART
LEVEL (checkpoint, inventory, health, timers), what happens today at zero
continues (game over), where else the count is read or decremented
(challenge REPLAY decrements it, `game_menu.cpp`; the statistics screen's
CONTINUES line reads the per-level used count at stats `+0x279`), and how
the Escape pause key, the D19 menu ownership and the death screen interact.
Pin the count instead of deleting it if that keeps savestates, card saves
and the statistics screen consistent.

**Acceptance:** no automatic death menu; first person keeps a visible
floor-level scene, third person its original animation; the top line names
the bound Open key and follows a rebind; Open opens the existing list,
Escape returns to the death scene, Duke still dead; unlimited continues, no
remaining-continues text anywhere; console opens and closes while dead with
no state change; CONTINUE, LOAD GAME, RESTART LEVEL, QUIT TO MENU, QUIT GAME
and the challenge choices behave as before; savestates from before the
change still load (dead and alive); 4:3 and 16:9, 120 and 180 Hz; Vanilla
unchanged (original death screen and continues).

**Open decisions (ask when selected):**
1. First-person death camera. D35L (user, 2026-10-25) accepted the current
   behaviour, where death drops first person to the original third-person
   death presentation, and closed the DEATH VIEW research with no action.
   This request asks for a floor-level first-person death view: confirm it
   reverses D35L for first person (and whether gibbed/drowned deaths, where
   there is no body to lie with, use the same view).
2. Escape on the death scene. Recommendation: Escape also opens the death
   list (EDuke32 opens its menu on Escape; in TTK Escape already opens the
   pause list in life), and Escape on the list closes it.
3. The Open key opens the menu rather than EDuke32's restart / "LOAD LAST
   GAME" prompt; confirm CONTINUE stays the first highlighted item.
4. Whether the statistics screen keeps its CONTINUES used line once the
   count is unlimited.
5. Controller: which button stands for Open and how the line names it
   before D19F icons exist.

**Risks:** the original's death sequence may time out or advance on its own
(game over, demo, attract) if menu 15 is held back; the original continue
path may depend on the count (zero -> game over) and the challenge stages
reuse it; D19 identity guards cover the death handler (`0x8008647c`);
savestates taken on the old death list must still resume sensibly; D36M /
D39 death fades and D18D music after Continue must not regress.

### D43 - Level title card: the level's name over the start of play (Duke 3D style)

**Status: Done (user-accepted 2026-11-03; [note 374](documentation/374-d43-level-title.md)). Dependencies: none required (uses D19A fonts, D08A22 scaling, D19 menu text). Independent of D42.**
When a level begins, show its name prominently near the centre of the
screen, as Duke 3D shows "HOLLYWOOD HOLOCAUST": over live play, no input,
gameplay never paused, then it fades out by itself (the reference holds
about 1.5 s and fades within about 2 s). Same in first and third person.

Initial research: level names are the game's own string ids at
`0x800c3d2c` (per level, read through the string table `0x800c60f4`);
`level_select.cpp` and `game_menu.cpp` `level_title` already draw them in
the menu font, so the text and the original UI text path exist.
`level_index` `0x800be570`; mode 1 level load `0x80025438` (guarded in
`level_select.cpp`) is a candidate level-entry event.

Investigate: the level-entry event for a new level, the next level after
statistics, RESTART LEVEL, CONTINUE and a savestate load; challenge stages
(21-26) and two-player arenas; any original loading card or mission
briefing on entry that already names the level; collisions with the D08A22
mission card, top-centre notices and cheat confirmations, the HUD and
widescreen margins; scaling across 4:3 / 16:9 and window sizes (follow the
presented game rect as the native HUD does).

**Acceptance:** the correct name on every level entered; centred, large,
in a Duke 3D-like treatment in the game's own font and colours; gone after a
short interval with no input; play never interrupted; 4:3 and 16:9 at 120
and 180 Hz; first and third person; the defined load / restart / continue
rules tested; Vanilla unchanged.

**Open decisions (ask when selected):**
1. When it shows. EDuke32 shows it on a new level and on loading a save (the
   death reference shows it after LOAD LAST GAME). Recommendation: new
   level, RESTART LEVEL and savestate load; not on CONTINUE after a death or
   a quickload within the same level (decide).
2. Treatment: the red menu font as in Duke 3D or TTK's large title font;
   colour, hold and fade times; an OPTIONS > DISPLAY toggle (as LEVEL STATS
   in D20I).
