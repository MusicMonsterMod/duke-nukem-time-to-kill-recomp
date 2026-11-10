# Time to Kill jobs - Items, inventory and HUD

Job descriptions and work logs for D08A and its children: weapon and item
shortcuts, inventory, gadgets and HUD icons. Status and dependencies are on
[the job board](../MODERNIZATION_JOBS.md); the dated session log is in the
`log-YYYY-MM.md` files beside this one. Moved verbatim from the board on
2026-11-10 (D58), one section per job in ID order.

## Jobs

### D08A — EDuke32-style weapon and item shortcuts

**Done — user accepted remaining playtest, 2026-09-28.** Wheel switching was
already accepted; remaining pickup/era/medkit/detonator checks were accepted
in ongoing campaign play. Keep documented TTK-only substitutions.

**Implemented 2026-09-27.** The delivered mapping uses
verified TTK executable identities and original use paths, with EDuke32 semantic
groups and the authorized substitutions on 7–9. Explicit user
requirements: **M uses the portable medkit; 6 selects pipe bombs**. Number keys
must represent familiar weapon roles, not simply the existing TTK slot offset.
Cover the complete weapon/item inventory and era variants, documenting sensible
choices for TTK-only items and absent Duke 3D equivalents.

Implementation and evidence: [controls engineering note](documentation/33-controls-shortcuts.md).
The [research proposal](documentation/32-eduke32-weapon-item-plan.md) is historical;
the user authorized its number groups, responsive held Ctrl crouch, and bounded
Alt-wheel camera distance. Q uses original standing Boot melee only; armed quick kick remains unproven;
R has no stored steroid dose in TTK. Their concrete limitations are documented.
Custom bindings are preserved with explicit migration of old defaults.

The local EDuke32 research and copied reference configs informed the layout;
owned-executable weapon records and original inventory/equipment routines establish
TTK identities independently of manual order. The complete mapping is in the
engineering note and [player manual](GAME_MANUAL.md). Repeated number-group presses
cycle usable alternatives; wheel up/down selects previous/next usable weapon;
X restores observed successful equipment. Zero-spare-ammo pipe-bomb detonator
access is retained when a live bomb remains. M/J/B/N, brackets and U use verified
original inventory paths. Q is restricted to safe original standing Boot behavior;
R cannot use a stored steroid dose because TTK has none.

**Human evidence:** the user explicitly confirms wheel weapon switching works and
praises the controls. Their confirmed Alt-wheel distance behavior belongs to the
bounded D10 feature. Neither report establishes every weapon/item shortcut.
Automated delivery evidence includes 57 Python tests, four native suites, a private
SDL inventory-fixture route and a 14-checkpoint Vanilla route; see
[recorded checks](documentation/reports/d08a-controls.json). Fixture ownership is
not evidence of natural item acquisition across eras.

**Remaining playtest:** real pickups/era variants, medkit use including a stable
full-health case, depletion and live detonator access, repeated/rapid commands,
interrupted actions, custom bindings and menu/focus transitions. Record specific human results as they arrive. Remaining playtest was accepted
2026-09-28.

Preserve the accepted movement and aiming, **E interaction-only (never fire)**,
**C holster**, Escape/Enter Start, and F10 capture semantics. This is a weapon/item
mapping pass, not permission to copy conflicting EDuke32 movement/menu bindings.
Keep Vanilla unchanged, preserve custom bindings and saved settings with an
explicit default-binding migration, and document new player-facing shortcuts.

**Acceptance:** all available weapons/items have a documented binding or explicit
unsupported/no-equivalent explanation; M consumes only an owned usable medkit
through the original heal/use path; 6 selects an owned pipe bomb without throwing
or detonating it; number keys select the intended weapon/era counterpart. Verify
empty/unowned inventory, full health, depletion, repeat presses, rebinding conflicts,
menu/focus isolation, and Vanilla regression. No inventory grants or damage/health
rule changes. User playtesting must confirm the familiar layout and preserved feel.

### D08A1 — Visible EDuke32-style inventory cycling

**Done — explicit user acceptance (2026-09-28).** Silent empty start and tiles-only
strip confirmed. Enter use remains. See [iteration 48](documentation/48-armed-ladder-reliability.md).
EDuke32 bottom-left single-icon HUD is tracked as **D08A2**.

**Follow-up implemented; Needs playtest.** Enter uses the highlighted gadget while the strip is visible, with one press owned through timeout/focus until release. Ordinary Enter pause elsewhere and U/custom bindings are retained. Default GL and custom O/P/L Software checks pass. See [current evidence and limits](documentation/46-turning-audio-inventory-progress.md).

**Needs playtest; implemented and privately verified.** Shared original-menu
selection, temporary named tiles/arrow/charge, custom bindings and both renderers
verified. See [evidence](documentation/44-traversal-inventory-implementation.md).
Follow-up to D08A: Brackets already select eligible TTK gadget
IDs 5,1,2,3 (medkit, jetpack, biomask, goggles) without using them; U activates the
selection. The existing host selection has no visible feedback in that branch.
Audit original TTK inventory selection/HUD paths before adding an overlay.
Use EDuke32's temporary owned-item strip, selection arrow and item-name feedback
as the interaction/presentation reference. Keep TTK item identity, quantities and
activation rules; no invented Duke3D-only inventory or borrowed mismatched icons.

**Acceptance:** both directions wrap and skip unavailable entries; zero/one/many
items, depletion, active toggles, original-menu selection, direct shortcuts and U
agree on the visible selected item. Cycling never consumes/activates an item.
Show a temporary readable selection strip/marker and name with the existing Duke
font, using inspected original TTK art where suitable and documented fallback.
Test rapid/repeated input, custom bindings, timeout, capture/menu/focus, HUD overlap
and resize in both renderers; preserve Vanilla. See the pinned reference in
[follow-up brief](documentation/41-playtest-follow-up-plan.md).

### D08A2 — EDuke32 bottom-left inventory icon and green %

**Playtest accepted (revised scope) — 2026-09-28 evening.** Always-on bottom-left
host gadget rejected. Delivered instead: centered **[ / ]** strip with
`research/inv` icons, green palette-22 THREEBYFIVE % (`03. Green (Palette 22)`),
EDuke ARROW `tile0020` frame locked at **50×60** / `frame_dy=-6`, Enter uses
selected item (Escape pauses), direct keys skip the switcher, **M** quote-only.
EDuke CROSSHAIR `tile2523` also landed (aim display). Evidence:
[51-session-inventory-crosshair.md](documentation/51-session-inventory-crosshair.md).

**Acceptance (revised):** brackets show the picker; Enter/U use always while
captured; M is message-only; no persistent bottom-left host gadget; Vanilla
preserved.

### D08A3 - Original TTK inventory icons for the switcher

**Done (2026-10-01, user-accepted).** The switcher now uses the game's own HUD item
icons from `/DATA/FONTS.RAW`, rebuilt by `build_ttk_inv_icons.py`. See
[source, builder and evidence](documentation/75-d08a3-ttk-inventory-icons.md).

The D08A2 switcher draws its gadget icons from Duke3D/EDuke32 tiles
(`research/inv`, packed into `recomp/assets/ttk-inv-icons.pack`, format
`TTKICO2`, loaded by `recomp/src/ttk/inventory_hud.cpp`). Replace them with
Time to Kill's own item art: extract the original TTK medkit, biomask, jetpack
and steroids images (and night-vision goggles, which the switcher also shows)
from the player's disc data, render them to PNG for review, then rebuild the
icon pack from those PNGs.

Locate the art from the original HUD/menu inventory display or the pickup
sprites/models, whichever is the game's own 2D icon; record source file, offset,
CLUT/palette and any transparency handling. Steroids activate on pickup and are
not a switcher entry today; extract their icon anyway and use it only where a
steroids icon is actually displayed, without adding a new switcher item. Keep
the extracted PNGs local only (retail-derived, like `research/`); the extractor
script and provenance notes may be documented.

**Acceptance:** PNGs for each item match the in-game original art (checked
against the vanilla inventory display or pickups); the switcher shows TTK icons
at readable size in the locked 50x60 cell with the green % and ARROW frame
unchanged; the pack builder is reproducible from the disc; a missing pack still
falls back cleanly; Vanilla unchanged. User confirms the look in play.

### D08A4 - EDuke32-style portable steroids (pick up, store, use with R)

**Done (user-accepted, 2026-10-07):** "mechanically, the steroids work
perfectly. you pick them up, you can press r to run, and thats it ... i accept
this job as complete now as it's functional". The countdown display is
follow-up D08A8. See [note 127](documentation/127-d08a4-portable-steroids.md).

User request, 2026-10-06. "Eduke style steroids, where you actually
pick up the roids as an item, and it appears in our items list. it is invoked
with the R key." In TTK steroids activate on pickup and there is no stored
dose (D08A recorded that R had nothing to use; D08A3 extracted the steroids
icon but kept it out of the switcher).

**Scope (Modernized, optional rule change):** a steroids pickup is stored as an
inventory item instead of activating; it appears in the D08A1 switcher with
the original TTK steroids icon (D08A3) and its count; **R** (and selecting it
in the switcher and using it) starts the original steroids effect and uses
one dose. Research first: the original pickup handler and the steroids
effect/timer, whether TTK keeps any spare inventory field a dose count can
live in or the host must hold it (and how that survives savestates, memory
card saves, death and Continue, and level changes), what happens on pickup
while already full, and whether `dnhyper` should give a stored dose or
activate. Because this changes an original rule, make it a Modernized setting
for the future menu (D19); Vanilla keeps activate-on-pickup.

**Acceptance:** in Modernized, picking up steroids in at least two levels
stores a dose shown in the switcher; R activates the original effect and
consumes it; R with none does nothing harmful; the dose persists through a
save/load and a level change; Vanilla unchanged; the user confirms.

**Work log (2026-10-07, Needs playtest).** Research: the original still has a
held steroids item (item 4, `+0x364` bit 0 with the amount `+0x366`; the
inventory grant `0x8003d738` gives it, the Select menu's toggle `0x80089494`
would run it, the level-end snapshot `0x80083348` and the card save keep it).
Only the pickup case `0x800827e8` skips it by setting bit 1 (running). EDuke32
source and Duke 3D `GAME.CON`/`USER.CON` checked: one held at a time, R only
when held, quote 12 `USED STEROIDS`, `dnhyper` starts them.
Implementation (`recomp/src/ttk/steroids.inc`): in Modernized with `steroids`
`portable` (default), a pickup is held instead of run, settled at the pickup
tail's own sound call (new entry hook `0x8006B73C`, one regenerated line,
savestates still load); held: more steroids stay on the ground; running: the
original refresh. R (and Enter/U on steroids) set bit 1 and play the pickup
sound. Switcher order medkit, steroids, jetpack, Bio Mask, goggles; HUD box with
the user's `hud-steroids.png` (15-colour cell from the new
`reduce_icon_15col.py`). Profile schema 29 (`--steroids portable|original`,
`--settings` S). A first design kept a dose count in flag bits 8-11 and was
replaced when a write trace showed the level-end snapshot (and so the card
save) keeps only bit 0.
Evidence (private copies of the user's cards and savestates, level 0 and level
6): pickup held (flags 1, 9000) with the original message; second pickup left;
R runs the effect, `USED STEROIDS`, switcher drains with the active mark;
R running/none refused; refresh while running; savestate reload, level
completion with the stats-screen save, card load (pause, Load) after using it,
and death with Continue all keep it; `original` and Vanilla keep the original
rule. Native and Python suites pass (two new D08A4 native groups).
Limits: needs the user's playtest; spawned pickups (`spawn steroids`, the real
type 638 and pickup code) rather than natural placements; `level N` travel
clears every gadget (original Restart reset); the original Select inventory
does not list held steroids.

### D08A5 - Mission item tracking (shipped design F)

**Done on user acceptance (2026-10-07): "i fully accept!"** Redesigned by the
user after the first try. The shipped look is **design F**: the mission row on
its own with `,` / `.` (previous / next) and the info card at the top. Design E
(row under the gadgets, `\` browsing) was the first playtest and is superseded.
`[` / `]` show gadgets only; Enter / U while the mission inventory is open try
the browsed item (D08A18). Original strafe (Comma/Period) is unbound by
default; profile schema 28. Data is the original Select inventory's own
per-level list (name function `0x80087d4c`). Not done: controller input. See
[note 114](documentation/114-d08a5-mission-tracking.md). Font-picker section 8
labels F as shipped and keeps A-E as history.

**User request and approved design, 2026-10-07.** "lock it in and stand
up a ticket to get that built in the game". For the first time, the game shows
which mission items a level asks for and which are found. The design was
iterated with the user as mockup E in the D24A research page (section 8,
`research/TTK-UI-and-Font-Research.html`, local). Modernized only; Vanilla and
the original Select inventory are unchanged.

**Approved design (build exactly this).** The panel coordinates below are in
the 640x480 overlay space and scale like the switcher.

1. *Gadget row (existing `[` / `]` switcher):* unchanged behaviour and layout,
   with one change. The selection frame is the project frame
   (`assets/ui/item-frame.png`) palette-swapped to the old Duke 3D `tile0020`
   orange ramp:
   `#341c00 #442800 #583000 #6c3800 #804000 #904800 #a45000 #b45404 #cc6818
   #d47430 #d88444`.
   Each frame pixel's brightness, normalised over the frame, picks a step, so
   the bevel and shading carry over. Only colour values are reused, no Duke 3D
   pixels.
2. *Mission row:* a second, read-only row under the gadgets whenever the
   switcher is open. It sits on a panel of translucent grey
   `rgba(12,14,20,0.62)` with a 1 px `#3a4150` outline:
   - one slot per mission item of the current level (counts expanded: Family
     Jewel x3, Skeleton Key x2);
   - the user's 16x16 icon at 2x (`assets/ui/items/`, via `items.json`) in a
     grey project frame;
   - missing items drawn as a dim grey silhouette (greyscale, about 45%
     brightness);
   - "MISSION" bottom left (Microfont, `#848484`) and "found/total" bottom right
     (Microfont, `#989c58`, green `#5fd35f` when all are found).
3. *Browsing:* while the switcher is open, `\` steps through the mission row
   (wrapping). The browsed slot's own frame is palette-swapped to the Console
   steel ramp
   (`#0c0c18 #14182c #202840 #405080 #5c70a0 #7088b4 #84a4cc #94b0d8 #a4c4e4
   #b4d4f4 #bcdcfc`), with no extra outline. The bottom line stays
   "MISSION" and the count. `[` / `]` and Enter never act on mission items.
4. *Item card while browsing:* a fixed 592x52 card at the top of the screen
   (x 24, y 14), in the same translucent grey and outline as the mission panel,
   the same for found and missing:
   - the browsed item's icon at 2x in the steel frame;
   - its name in TTK Medium Italic with the navy shadow (gold CLUT 225 when
     found, blue CLUT 226 when missing);
   - its type below in System 8x8 `#848484` ("KEY CARD", "CRYSTAL", "COMBO
     PIECE", "JEWEL", "KEY");
   - "FOUND" (`#5fd35f`) or "NOT FOUND YET" (`#848484`) right-aligned.
   It clears about 2.5 s after the last `\` press, or when the switcher
   closes.
5. *Controller:* D-pad up/down moves focus between the gadget row and the
   mission row while the switcher is open.

**Data.**
- Each level's set and names come from the D24A survey (the original Select
  inventory names) and `assets/ui/items/items.json`.
- Found state is read live from the player's mission flags: key slots
  `+876`/`+880`; item slots `+884`..`+892` and `+908`..`+916`, as the pickup
  dispatcher `0x80081a48` sets them.
- Verify per level which flag each item uses before relying on it. Seen so far:
  - level 1 papers: `+908/912/916`;
  - level 6 jewels: `+908/912/916`;
  - skeleton keys: `+876/880/884`;
  - level 0 crystals: not yet confirmed.
- Levels without mission items (8, 12, Challenge Stages, bosses) show no
  mission row.

**Acceptance.**
- The row, card, frames and colours match the approved mockup in every level
  with mission items.
- Found and missing follow real pickups, savestate loads and level travel.
- `\` browsing and the card timeout work.
- Gadget cycling and Enter are unchanged.
- Vanilla is unchanged.
- The user confirms the look in play.

### D08A6 - Selected gadget shown on the HUD (approved design A)

**Done on user acceptance (2026-10-07): "i fully accept!"** The selected gadget's box is at the
original item slot over ammo, drawn by the status bar's own calls (lit when on,
dim digits at colour 0x31 when off, medkit always lit); an unselected jetpack
that is on moves one row up (20 rows, the original's step); Bio Mask / goggles
keep their original place left of ammo (82,89) when not selected. See
[note 115](documentation/115-d08a6-selected-gadget-hud.md).

**User request and pick, 2026-10-07.** "Enter has the capability of
using the currently selected inventory item, however the HUD does not show the
user what the currently selected inventory item actually is ... say the user
just presses enter without looking, its pot luck." The look must "truly belong
in this HUD", with the same borders and style as the rest of it. A full HUD
redesign is a separate, later matter. Three mockups were drawn in section 9 of
the local D24A page (`recomp/analysis/d24a-fonts/ttk-font-picker.html`, built
by `make_picker.py` from `picker.template.html`; retail-derived, local only).
The user picked **A**: "i think we just go with A. that's my pick." B (left
stack) and C (Duke 3D centre box) stay on the page for reference.

**Design A (build this).** Modernized only; Vanilla unchanged.

1. *Where:* the original item slot. While a gadget is on, the original status
   bar stacks that gadget's box over the ammo box at the bottom right (Vanilla
   capture `analysis/d08a3-ttk-icons/item-jetpack-toggled.png`). In
   Modernized that box is always shown for the gadget selected with `[` / `]`
   (the one Enter / U uses).
2. *Parts, only the HUD's own:* the disc's 46x16 icon/number box (sprite
   record `0x800c44a4`; icon cell x 2..17, divider at x 18, number cell
   x 19..44), the gadget's HUD icon (D08A3 records: health cross for the
   medkit `0x800c44e4`, jetpack `0x800c44b4`, Bio Mask `0x800c44c4`, goggles
   `0x800c4504`) and the red HUD digits (CLUT 227), at the same scale and
   pixel grid as the health and ammo boxes. No new art, no `%`.
3. *Number:* the selected gadget's charge, as the Select screen lists it.
4. *On / off:* lit digits (CLUT 227, as the original) when the gadget is on;
   dim digits when it is selected but off (mockup: CLUT 227 colours at 38%
   brightness). The medkit has no on state and always reads lit.
5. *Another gadget on:* if a gadget other than the selected one is on, its own
   original box (lit) stacks one row higher, so nothing the original shows is
   lost. Mockup stacking step: 14 rows at 1x (28 px in the 640x480 overlay),
   with the lower box drawn over the upper one, as the original draws ammo
   over the item box. Check the real step against the original status bar.
6. *Nothing owned:* no box (and the original behaviour when nothing is on).
7. *Switcher:* `[` / `]` and the existing strip are unchanged; the box follows
   the selection immediately. Mission inventory (`,` / `.`) does not touch it.

**Research first.** Find how the status bar (`0x8008BA30`) decides and draws
the active item box and its stacking offset; decide whether to drive the
original draw (preferred: its own sprites, palette, layout and widescreen
anchoring from D14) or draw a host copy that matches it pixel for pixel. The
host selection lives with the D08A1 switcher (`recomp/src/ttk/inventory_hud.cpp`).
Confirm where each gadget's charge and on flags live (`player+0x358` jetpack,
`+0x360` Bio Mask / goggles branch) and whether the original can show two
active boxes at once.

**Open (decide while building, or ask):** a short flash when the selection
changes or Enter uses it; the medkit's number if charge proves misleading.

**Acceptance.**
- In Modernized the selected gadget's box is always visible at the original
  item slot, and matches the box the original draws (borders, icon, digits).
- It is right after cycling both ways, Enter / U use, direct shortcuts (M, 6,
  etc.), depletion to zero, toggling on/off, another gadget being on,
  savestate load, death / Continue and level change.
- Dim / lit state follows the real on flag.
- Default GL and Software renderers; 4:3 and widescreen corners; resize.
- Vanilla HUD unchanged.
- The user confirms the look in play.

### D08A7 - Custom medkit gadget icon (switcher strip and HUD box)

**Status: Done (user-accepted 2026-10-07).** Built as planned, with one change
from the plan: the live survey found the game fills VRAM x 960-991 from row
223 down at run time, so the cell is at (960,205), not row 208. Details:
[note 116](documentation/116-d08a7-medkit-icon.md).

**User request, 2026-10-07.** "Instead of the red cross for the medkit's
inventory representation, we can go for this sprite that i have made ... The
health icon red cross remains the same." Art: `research/inv/medkit.png`
(16x16 RGBA, 176 colours, by MusicMonsterMod). A 15-colour reduction for
the HUD box, made at the user's request: `research/inv/medkit-15col.png`
(k-means in PSX 15-bit colour, no dithering, about half the error of a
median-cut reduction). Both are previewed in section 9 of the local D24A page
(`recomp/analysis/d24a-fonts/ttk-font-picker.html`): the D08A7 card and
mockup A, which now opens on the medkit.

**Why it is separate from health.** The medkit box and the health box both
pass sprite record `0x800c44e4` (the HUD health cross) to `0x8008b678`
(`gadget_icon()` in `recomp/src/ttk/gadget_hud.inc`). Changing that cell
would change health too, so the medkit needs its own cell, palette and record.

**Plan.** Modernized only; Vanilla unchanged.

1. *Art in the repository:* copy the user's original and 15-colour PNGs into
   `recomp/assets/ui/items/` (own art, like the D24A mission icons) and list
   them in `items.json` / the assets README.
2. *Switcher strip (`[` / `]`):* `build_ttk_inv_icons.py` uses the full-colour
   art for item 5 instead of the health cross (host pack, no colour limit).
   The pack's provenance notes the change.
3. *HUD box:* write the 15-colour art as a 4bpp cell plus a 16-entry CLUT
   (index 0 transparent) into unused space in the HUD sheet (`FONTS.RAW` at
   VRAM 960,0). Read of the disc file found empty 16x16 cells (for example
   rows 208-255, VRAM x 960..) and 60 unused CLUT rows in column x 1008
   (for example y 53-71, 95-108). Put a 16-byte HUD record (CLUT id, 16x16,
   VRAM x/y) in guest memory or a host-owned slot the call can read, and
   return it from `gadget_icon()` for the medkit only.
4. *Keep it there:* find what reloads or overwrites VRAM (level load, FMV,
   savestate load, menus, D17 worker replay) and re-upload after each, or
   prove the area is never touched. Savestates must not need to carry it
   (do not edit hashed framework headers; see codegen-hash note).

**Research first.** Confirm in a live VRAM dump, in several levels and
after FMV / savestate load, that the chosen cell and CLUT row stay empty.
Check whether the HUD sheet is uploaded once or per level.

**Acceptance.**
- In Modernized, the medkit's selected-gadget box shows the user's medkit
  icon at the same place, scale and pixel grid as the other gadget icons;
  the health box keeps the cross.
- The `[` / `]` switcher strip shows the full-colour medkit.
- Correct after level change, savestate load, death / Continue, FMV,
  pause / Select menu, 60 and 120 fps (D17 workers), GL and Software,
  4:3 and widescreen.
- No other HUD sprite, font or texture is disturbed in any visited level.
- Vanilla HUD unchanged.
- The user confirms the look in play.

### D08A8 - Steroids countdown in the steroids HUD box

**Done (user-accepted, 2026-10-07):** "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." See the work log below and
[note 127](documentation/127-d08a4-portable-steroids.md) (D08A8 section).

User request, 2026-10-07 (after accepting D08A4; video
`research/screencaps/Video_2026-10-07_22-03-53.mp4`, reference
`research/screencaps/ttk-roids.png`): "what genuinely doesnt make sense to me is
why the armor icon is used when the coundtown for the steroids is displayed.
I'd really love if that entire countdown could be delegated to the steroids
thing you see there in the screenshot."

While steroids run, the original status bar (`0x8008bd94`) shows their
remaining percent in the armor element with the armor icon. In Modernized,
show the whole countdown in the D08A6 gadget box with the pill-bottle icon
(the box in `ttk-roids.png`, top right) and leave the armor element to armor.
Research first: how `0x8008bcf4..0x8008be00` chooses between armor and
steroids for that element, and what the box should show when steroids run
while another gadget is selected (stack it like the running jetpack, or switch
the box to steroids).

**Acceptance:** in Modernized, running steroids count down in the steroids box
with the pill icon; the armor element shows only armor (or stays hidden as the
original does without armor); held, running and refreshed steroids, a selected
other gadget, savestates and both renderers checked; Vanilla unchanged; the
user confirms.

**Work log (2026-10-07, Needs playtest).** Research: element 2 of the status
bar (`0x8008bcf4..0x8008bf00`) is shown while `+0x364` bit 1 (steroids running)
is on or armour `+0x234` > 0, and draws steroids' percent (`amount * 100 /
9000`) instead of armour (`armour / 100`, at least 1) whenever bit 1 is on, always
with the armour icon `0x800c44f4`. Nothing else in the status bar reads bit 1.
Decision on the open question: running steroids stack like a jetpack that is on.
Implementation: running steroids stay selected (`usable_item(4)` is now held
or running, so after R the box you were looking at counts down in place; R /
Enter still only take a held one). `gadget_hud.inc` draws them lit with the pill
icon in the slot when selected, else stacked above it (after an unselected
jetpack that is on). For the status bar draw only, bit 1 is turned off so
element 2 is the plain armour element; it is turned back on at the first hook
after the status bar returns (`0x8001fc44`, or new lightweight entry hook
`0x8002E850` on the composition path that skips it; also at the next status bar
entry and `0x800b4d9c`). One regenerated line; codegen hash unchanged. Only with
`steroids` `portable`; `original` and Vanilla unchanged.
Evidence (executable `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`; private Xvfb runs on a fresh copy of the
player's cards and savestates, `recomp/analysis/d08a8-steroids-hud/`, local):
held 100 then R: lit pill box counting down in place, no armour element with
armour 0, drain and effect unaffected (timer falls normally, ends at 0 and the
box goes); `]` to the jetpack (off): steroids one row up; jetpack on and
selected: same; Bio Mask selected with jetpack on: jetpack row 1, steroids row 2;
armour 50 while running: the armour element shows 50 with the armour icon,
steroids stay in their box; savestate save/reload while running; R while running
does nothing; after the effect the armour element works as before. GL 4:3, GL
16:9 at 120 fps (replay workers), Software 4:3 and 16:9 look the same. Vanilla
with steroids forced on still shows them in the armour element. Suites:
`ttk-controls-test` (LEVEL00 fixture `d08-camera-final`, LEVEL01, all levels;
40 groups), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK (2 skipped),
`level_overlay_guards.py --check`.
Limits: needs the user's look in play; death while steroids run and a
natural (non-spawned) pickup while running were not exercised; with
`steroids` `original` in Modernized the original armour-element countdown stays
(there is no steroids box there).

### D08A9 - Picked-up inventory item becomes the switcher selection

**Done (user-accepted 2026-10-08: "fully accepted, working beautifully").**
Executable `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`
is the regression baseline.

**User request, 2026-10-07** (on accepting D08A8): "when an inventory
item is picked up, that item should be the one selected in the switcher. i.e.
you pick up biomask, then that should be the selected item. you pick up
jetpack, that should be the selected item etc. that's how it worked in duke3d
and i want that feel to exist here."

**Scope (Modernized only; Vanilla unchanged):** when Duke picks up a gadget
(jetpack, Bio Mask, goggles, medkit, portable steroids), the `[ / ]` selection
(`selected_item` and the original menu ID `0x800c3f94`, which the D08A6 HUD box
reads) moves to it at once, so the HUD box and the next Enter / U use it.
Mission items and keys never change the gadget selection (D08A5 owns them).

**Research first:** the pickup dispatcher `0x80081a48` cases that grant each
gadget (D08A4 found steroids at `0x800827e8`) and whether a refill of an
owned gadget, a full one left on the ground and cheat/inventory grants
(`0x8003d738`) should count; how EDuke32 does it (`P_AddInventory` / the
`addinventory` CON command set `inven_icon` on every inventory pickup) and
whether a refill there also selects; whether a pickup during a gadget's
activation (`+0x8000` pending), in flight, or while the switcher strip is open
should wait or apply; that the D17 replay workers see it (guest ID, not host
state).

**Acceptance:** in Modernized, picking up each gadget type selects it in the
switcher and the HUD box immediately, in at least two levels; a held steroids
pickup selects steroids; mission items and keys leave the selection alone;
Enter / U then uses the picked-up item; savestate, level change and Continue
keep the selection consistent; Vanilla unchanged; the user confirms the feel.

**Work log 2026-10-07 - built (Needs playtest).** Executable
`c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`.
[Note 128](documentation/128-d08a9-pickup-selection.md).

- Research: every gadget pickup goes through the dispatcher `0x80081a48`; the
  jetpack, Bio Mask, goggles and medkit cases take the pickup only while the
  amount is below capacity, then set bit 0 and the full amount (refills
  included, as EDuke32's `P_AddInventory` sets `inven_icon` on refills). Only
  multiplayer starts an activation there. Keys and mission items never touch
  items 1-5; the `dninventory` grant `0x8003d738` bypasses the dispatcher.
- Implementation: `recomp/src/ttk/pickup_select.inc`. Items 1-5 are compared
  around the dispatcher call (entry, and its caller's next call `0x8001ca4c`
  with ra `0x8007fe78`, both existing hooks); a gadget that became owned or
  gained amount becomes the selection through `remember_item` (guest menu ID
  `0x800c3f94`, so the D08A6 box and D17 replay workers follow). While any
  gadget is mid-activation (`+0x8000`) it waits for the next item poll.
  Savestate loads in between cancel it. No strip pop-up, no new hooks, no
  codegen change.
- Evidence: private Xvfb runs (`recomp/analysis/d08a9-pickup-select/`):
  level 0 (60 fps) and level 6 (120 fps) - new jetpack/Bio Mask/goggles/medkit/
  steroids each selected with their HUD box; refills of Bio Mask, medkit,
  goggles and jetpack selected; full goggles/medkit stay on the ground with
  the selection unchanged; keys leave it; `dninventory` leaves it; Enter after
  a goggles / jetpack pickup switches it on; steroids picked up while running
  refresh and select; a pickup during a (test-forced) activation waited and
  applied when it cleared; savestate reload keeps the selection. Native
  `ttk-controls-test` (new D08A9 group), `ttk-input-test`,
  `ttk-inventory-test`, Python 131 OK, `level_overlay_guards.py --check`,
  `check_repo.py`.
- Limits: pickups were spawned (`spawn`, same types and original code), not
  natural placements; death/Continue was not reached in the scripted runs
  (writing health 0 did not kill Duke); Vanilla gating verified by code path
  and counters only (`spawn` is Modernized-only); a real jetpack-transition
  pickup was simulated with the pending bit.

### D08A10 - Experimental: steroids heartbeat sound loop

**Done on user acceptance (2026-10-08): "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!"** Executable
`146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca` is the regression baseline.

225 bpm on the steroids timer, with the exact
sound Shift makes in Modernized (the walk/run toggle click `0x0001` through
`0x8006bbd8`). User on the first build: "the beat is right but the sound is
wrong" (it used `0x1012`; the footstep was the wrong guess for the Shift sound).
See the work log below and [note 129](documentation/129-d08a10-steroids-heartbeat.md).

**Experimental; user request, 2026-10-08.** The user expects it to be
simple but may not like the result, so it may be reverted: keep it small,
isolated and easy to remove (or behind a profile option), and get the user's
verdict before building on it.

"In duke3d, when taking the steroids, a sound plays of duke swallowing the
pills over and over for the duration of the steroid use. the rhythm is
226bpm. ... we could use the sound that duke makes in ttk when you hit shift
... it sounds good enough like a heartbeat. unless of course there is an
actual heartbeat sound available like this."

**Reference:** `research/screencaps/Video_2026-10-07_23-59-03.mp4` (local,
Duke 3D, the latest video when the job was opened): listen for the repeating
steroids sound and confirm the rhythm (user: 226 bpm, about 265 ms per beat).

**Scope (Modernized, portable steroids only; Vanilla unchanged):**

- While steroids run (`+0x364` bit 1, D08A4/D08A8), play a short sound at
  Duke on a steady 226 bpm rhythm; stop at once when the effect ends
  (timer out, damage cut `0x800a4154`, death, level change, Continue,
  savestate load).
- Sound source, in order of preference: a real heartbeat-like sound already in
  the game's own sound banks, if one exists; otherwise the sound Duke makes
  when Shift is pressed to run (the walk/run gait plant sound noted under
  D27). Use the original's own sound call (`0x8006b73c`, as the D08A4 use
  sound does) so no external audio is added. Identify the exact sound ID.
- Time the beats on the game clock, not the host frame rate, so 60/120 fps and
  the D17 replay workers do not change the rhythm or double the sound.
- Easy to revert: one `.inc` and its hook-ups, optionally a `steroids_sound`
  profile choice (on/off).

**Research first:** list candidate sounds (sound bank IDs played by the Shift
gait restart, heartbeat or pulse sounds in any level bank) and how the sound
call picks a bank per level; whether a looping voice would starve Duke's
voice lines or the music (D18B, D18C); the Duke 3D timing from the video.

**Acceptance:** in Modernized, R starts the beat at about 226 bpm for the full
steroids duration in at least two levels, at 60 and 120 fps; it stops when
the effect ends or is cut, and on death, savestate load and level change;
other sounds and voices are not cut off; Vanilla and `steroids` `original`
unchanged; the user decides to keep, change or revert it.

**Work log (2026-10-08, Needs playtest).**

- Reference: the video's steroids sound runs 13.5 s, median 268 ms between
  beats (224 bpm): Duke 3D's `DUKE_HARTBEAT` every 8 tics at 30 Hz, 225 bpm.
- Sounds: walking and Shift play only the footstep `0x2000`/`0x2001`
  (`0x80048378`). No named heartbeat exists; a debug `sfx <id>` survey decoded
  each sound's sample from SPU RAM. Bank 1 (`0x1000`-`0x101d`, Duke's own, in
  every level tried) has `0x1012`, a 313 ms low double thump (95% of its energy
  under 250 Hz), chosen as the default. 225 bpm previews of it, the footstep,
  `0x1000` and `0x101c`, plus the Duke 3D reference, are in
  `recomp/analysis/d08a10-beat/preview/` (local).
- Implementation: `recomp/src/ttk/steroids_beat.inc`, checked from the
  authenticated player update (existing hook `0x8005a210`/`0x80041b34`): one
  beat each 80 timer units from the dose start (300 units/s, 266.7 ms), first
  beat at once, through the original sound call `0x8006b73c` on a private
  stack. Portable steroids only, Duke alive; savestate loads resync. No new
  hooks or codegen change. `DNTTK_STEROID_BEAT=off|<id>`; debug
  `controls.steroid_beat` with a 32-entry sound-call log; console `sfx <id>`.
- Evidence (executable
  `d0d1f09355d69f4ea3ff1a1a6ea7737bf261f512d83443b3d0f3edf20c861acf`, private
  Xvfb runs): identical beat intervals at 60 and 120 fps (15, 15, 18 fields =
  250, 250, 300 ms, mean 225 bpm); pickup + R beats; savestate load without a
  burst; damage cuts continue the beat; `level 6` travel stops it; level 6
  beats from the same sample; `original` and `off` silent. Suites:
  `ttk-controls-test`, `ttk-input-test`, `ttk-inventory-test`, Python 131 OK,
  `level_overlay_guards.py --check`, `check_repo.py`.
- Limits: in 20 Hz scenes beats land on 50 ms steps (slightly uneven
  250/250/300 ms; exact 266.7 ms at 30 Hz); death during steroids and Vanilla
  verified by code path only; no profile option (environment variable only)
  until the user decides to keep it.

**Work log (2026-10-08, sound fix, Needs playtest).** User: "the beat is right
but the sound is wrong. i want the sound specifically when you press shift on
the keyboard on modern controls. you used the footstep sound." SPU KEYONs
around Shift presses (standing and walking) show sample `0x012F0` at pitch
`0x228`-`0x22F` on press and release: sound `0x0001`, the walk/run toggle
click, played by the player update through the non-positional call
`0x8006bbd8(1)`, not the `0x8006b73c` call the first survey logged. The beat
now makes that exact call. Same sample and pitch spread as a real Shift press
in levels 0 and 6; rhythm unchanged (15, 15, 18 fields). Suites pass.
Executable `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`.
Preview `recomp/analysis/d08a10-beat/preview/beat-shift.wav` (local).

### D08A11 - `dnhyper` countdown in the steroids HUD box

**Done (user-accepted 2026-10-08: "this job is fully accepted").** Executable
`2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892` is the
regression baseline. Work log below.

**User request, 2026-10-08:** "dnhyper uses the old ui element with the
armor rather than our new ui element with the steroids to display the
countdown, we need to make dnhyper use the new dedicated countdown on the
right hand side."

With portable steroids (Modernized), a dose taken with R counts down in the
steroids HUD box on the right (D08A8), and the original armor element shows
only armor. After `dnhyper`, the countdown still shows in the original armor
element with the armor icon.

**Likely cause (to verify first):** `dnhyper` sets `player+0x364` bit 1
(running) without bit 0 (owned). D08A8's status-bar hide and the HUD box use
`steroids_running()` / `steroids_owned()` (`steroids.inc`), which require
bit 0, so the original status bar still sees bit 1 and draws element 2 with
the armor icon, and the steroids box does not show. A natural pickup while
steroids run and other paths that set bit 1 alone may behave the same.

**Scope:** Modernized with `steroids` = `portable`. Any running steroids,
including `dnhyper`, count down in the steroids box (and keep the switcher's
active mark and the D08A10 heartbeat) while the armor element shows only
armor; when the effect ends, nothing is left held that was not held before
(a `dnhyper` dose must not become a stored item). Decide whether to give
`dnhyper` the owned bit while it runs (as a used dose has) or widen the HUD
predicates; keep `original` and Vanilla unchanged.

**Acceptance:** after `dnhyper` the countdown shows only in the steroids box,
draining and lit, with the armor element showing armor (or hidden with no
armor); it ends cleanly with no held steroids left behind; R-used doses,
pickups, savestates and the heartbeat behave as before; at 60 and 120 fps;
`original` and Vanilla unchanged.

**Work log (2026-10-08, Needs playtest).** Cause confirmed: `dnhyper`
(`cheats.inc`) sets `+0x364` bit 1 only, and `steroids_owned()` /
`steroids_running()` required bit 0, so the D08A8 status-bar hide never ran and
the box never drew. Decision: widen the predicates rather than give `dnhyper`
an owned bit. Running is now bit 1 with an amount, with or without bit 0, so
any path that runs steroids without the owned bit is covered; the drain
`0x800414a0` clears bits 0-1 at 0, so nothing is left held. Changes
(`steroids.inc`): `steroids_owned()` accepts bit 0 or bit 1; the HUD restore no
longer needs bit 0 to put bit 1 back, and skips the restore if a savestate
loaded between hide and restore (it keeps the loaded flags) or the amount is 0.
`pickup_select.inc`: for steroids a pickup that refreshes a `dnhyper` run counts
as a refill (selects steroids), as for a used dose. `steroid_doses()` is
unchanged, so R / Enter still take only a held dose. No new hooks, no codegen
change. Evidence (executable `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892`; private Xvfb runs on a copy of the
D08A8 cards, `recomp/analysis/d08a11-dnhyper/`, local), at 60 and 120 fps:
`dnhyper` with armour 0 - flags `0x2`, the lit pill box counts down stacked
above the selected jetpack box, no armour element; steroids selected - counts
down in place; R while it runs does nothing; armour 50 - the armour element
shows 50 with the armour icon, steroids stay in the box; savestate save and
reload while running; the effect ends with flags 0, no held dose, the box
gone; a steroids pickup while `dnhyper` runs refreshes, selects steroids and
ends with nothing held; a held pickup then R still counts down in the box.
`DNTTK_STEROIDS=original`: `dnhyper` keeps the original armour-element
countdown (no hides). Suites: `ttk-controls-test` (`d08-camera-final` fixture,
LEVEL01, all levels), `ttk-input-test`, `ttk-inventory-test`, Python 131 OK
(2 skipped), `level_overlay_guards.py --check`, `check_repo.py`.
Limits: the heartbeat already treated bit 1 alone as running (unchanged, not
re-listened); `dnhyper` while a dose is held runs that dose and it is used up
when the effect ends (the original keeps one amount per item); Vanilla verified
by code path only (all changes are behind `portable_steroids()`); software
renderer not re-shot (the draw path is D08A8's, unchanged).

### D08A12 - Modern dynamite handling / safe weapon selection

**Done (user-accepted, 2026-10-08).** User: "perfectly done." Executable
`2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d` is the regression baseline. User request, 2026-10-08; selected and
implemented 2026-10-08. See the work log below and
[note 138](documentation/138-d08a12-modern-dynamite.md).

**Problem.** Dynamite (slot 6: Pipe Bomb, Dynamite, Holy Hand Grenade) is
reached by pressing 6 twice or with the mouse wheel. Once Duke holds it the
fuse is effectively committed: if it is not thrown it explodes in his hand. So
selecting the weapon commits the player to using it; they cannot scroll to it,
look, change their mind and switch back. The original pad had no rapid weapon
scrolling, so the original rule is far more intrusive under our wheel and
number-key selection. Design principle (user): **selecting a weapon is not the
same thing as firing a weapon.**

**What we already know (do not rediscover):**
- D08A ([note 33](documentation/33-controls-shortcuts.md)): lit dynamite is an
  original exception: `0x8004dea4` / `0x8004e018` run its fuse until thrown or
  detonated, and a requested next weapon waits for that original completion.
  Shortcuts neither stow it artificially nor throw it automatically.
- D08Q5 ([note 108](documentation/108-d08q5-jetpack-weapon-switch.md)): drawn
  dynamite runs its original fuse; a switch away waits for it, also in jetpack
  flight. Grenade and dynamite share the original throw handler with their own
  gravity and fuse ([note 40](documentation/40-feedback-implementation.md)).
- Weapon groups and wheel order: [note 32](documentation/32-eduke32-weapon-item-plan.md),
  [note 33](documentation/33-controls-shortcuts.md).

**Research first.**
1. The original state machine: is there a distinction such as equipped/unlit
   -> fuse lit -> throw -> explosion, or does the draw itself start the timer?
   Where the fuse value lives, what starts and advances it, and what the
   in-hand detonation path is.
2. Whether equip and ignition can be separated cleanly. Ideal (if not
   invasive): select = equip safely; primary fire = light/arm; release or a
   second action = throw; once armed, normal fuse rules apply. Do not add
   complexity purely for realism.
3. What switching away from an equipped or armed dynamite does to weapon state
   today (the pending switch, ammo, animation, the D08A history).
4. The smallest robust Modern implementation. Acceptable fallback (user):
   while held, the fuse never runs out (the fuse animation/sound may keep
   sizzling indefinitely), and switching away is allowed without using a
   stick.

**Option.** Dynamite Behaviour: **Modern** (default in the Modernized
profile) / **Original** (exact PlayStation behaviour, fuse can explode in
Duke's hand). Integrate with the existing profile/settings architecture (as
`steroids` `portable` / `original` does in the Modernized controls profile),
not a one-off config. Vanilla stays original.

**Do not change** explosion damage, blast radius, ammo capacity, throw
physics, enemy damage or environmental interactions unless the arming state
genuinely requires it.

**Acceptance.**
1. The original equip/arm/fuse/throw state machine is documented.
2. Whether equip and ignition separate cleanly is answered.
3. Consequences of switching away from equipped or armed dynamite are known.
4. Modern behaviour is the smallest robust change.
5. Original behaviour stays available and unchanged (and Vanilla).
6. Mouse-wheel selection passes through dynamite without consuming it or
   hurting Duke: e.g. shotgun -> wheel -> dynamite -> wheel -> another weapon,
   rapidly.
7. Selecting dynamite and then changing your mind is safe in Modern.
8. Throwing still behaves normally once the player chooses to use it.
9. Savestates and weapon state stay sane with dynamite equipped or armed.
10. The setting integrates cleanly with the existing modernization options.

**Work log (2026-10-08, Needs playtest).** Answers to the research questions
([note 138](documentation/138-d08a12-modern-dynamite.md)):
1. **Equip is ignition in the original.** The draw completion `0x8004e5ac` sets
   the fuse `+0x250` = 4000 (`0x800c4ed8`). The held-throwable handler
   `0x8004dea4` (case `0x8004e018`) burns it (about 3.7 s at 60 Hz). At 0 it
   explodes in hand. Otherwise it plays sparks and the sizzle loop and runs
   the Cross throw check `0x8004dc74`. The throw passes the remaining fuse to
   the stick.
2. **They separate cleanly.** All the burning happens inside that one handler
   call.
3. **Switching away.** The dynamite case ignores weapon requests and the
   holster button, which the other throwables honour, so a switch waits until
   the throw or the blast.
4. **Implementation.** Three opt-in entry hooks (`0x8004dea4`, sparks
   `0x8006d594`, sound `0x8006b270`; regenerated, codegen hash unchanged):
   - Until Cross is pressed where the original would charge, a drawn stick is
     unlit: fuse held full, sparks dark, sizzle refused.
   - A request or the holster button stows it exactly as the Holy Hand
     Grenade's case does.
   - Cross lights it with a full fuse, and the original runs untouched from
     then on.

   Profile schema 32 `dynamite` `modern|original` (`--dynamite`, menu T);
   Vanilla always original. Six new guards.

Evidence (executable `c5b97f7956d80fa8bd3a3b6010e6466c5143f91682ce2c122b89840e523c7e59`;
private Xvfb, card copies):
- **Modern:** held 6 s unlit (fuse 3951 steady, no spark, no sizzle). 3 stows
  it, and rapid and slow wheel pass through it with no ammo or health change.
  A savestate round trip keeps it unlit and switchable. LMB lights it (spark
  and sizzle) and throws it: one stick spent, explosion, empty hand.
- **Original:** the fuse runs from the draw and 3 only queues. It exploded in
  hand 226 fields after the draw, then the shotgun was drawn.
- **Suites:** `ttk-controls-test` 47 groups (new D08A12 group),
  `ttk-input-test`, Python 142 OK (10 skipped), `level_overlay_guards.py --check`.

Follow-up (2026-10-08, user: "the only thing i want is for the next stick of
dynamite to be drawn after throwing the first one"):
- **Redraw:** after a throw Modern draws the next stick (unlit) through the 6
  key's request once fire is let go.
- **When it drops:** another weapon or request, no sticks left, a savestate
  load, or `original`.
- **Charge bit:** charge bit 8 now also counts as lit.
- **Live:** four throws in a row redrew. With the last stick there was no
  redraw and the original picked the grenade. Holding fire keeps throwing.
- **Executable:** `2d54e00b64bcf38917e41db47805338f75977b0b63a0868cb962242ffaaadb9d`.

Limits:
- **Not run live:** E's stow and jetpack flight.
- **Thrown fuse:** a thrown stick's fuse now counts from the fire press (nearly
  full), not from the draw.
- **After lighting:** a lit stick still follows the original rules.

### D08A13 - Steroid duration independent of damage and armor

**Done (user-accepted 2026-10-08: "i accept that this works").** See the
work log entries of the same date and
[note 127](documentation/127-d08a4-portable-steroids.md#d08a13---steroids-independent-of-damage).
User request, 2026-10-08.

**Problem.** With steroids running, taking damage shortens the remaining
steroid duration: activate steroids, watch the countdown in the steroids HUD
box (D08A8), let an enemy shoot Duke, and the countdown drops with each hit.
The user wants none of this. Design rule (user): **steroids and armor are
completely independent systems.** Steroids are a temporary performance
enhancement with their own timer; armor is protection from incoming damage.
Damage must never consume steroid duration, and steroids must never change
armor. The steroids box (D08A8, D08A11) is now the authoritative player-facing
countdown, so steroids no longer need the armor element for a number.

**What we already know (verify, do not assume):**
[note 127](documentation/127-d08a4-portable-steroids.md) records, from the
original code, item 4 (steroids) as player `+0x364` flags (bit 0 held, bit 1
running) with its own amount/timer at `+0x366` (full 9000, `0x800c2722`);
armor is a separate field `+0x234`. The drain `0x800414a0` lowers `+0x366` by
the frame step while bit 1 is on and clears bits 0-1 at 0. A **damage cut
`0x800a4154` takes 1500 from a running amount and can end the effect**. So
the steroid duration is probably not stored in armor at all: the original
game itself charges steroid time for each hit, and the old status bar showed
that countdown in the armor element with the armor icon, which made the two
look like one resource. This is a lead to confirm, not a finding.

**Investigate first and document** (in note 127 or a new note):
1. The original pickup/activation path, the drain, and exactly what
   `0x800a4154` does: which damage routine(s) call it, for which damage types
   (bullets, explosions, falls, melee, drowning), whether it runs before or
   after armor absorbs damage, whether it depends on armor at all, and
   whether 1500 is a constant or scaled by damage.
2. Whether any other path ties `+0x366` / bit 1 to armor `+0x234` (armor
   pickups, the status bar, the level-end snapshot `0x80083348`, card save and
   load, savestates).
3. Whether the coupling is original (Vanilla does it) or something our
   modernization added or exposed (D08A4 portable doses, D08A8 HUD hide,
   D08A11 `dnhyper` predicates, D08A10 heartbeat). Confirm in Vanilla /
   `steroids` `original` as well as Modernized `portable`.
4. Whether the effect ends early on a hit today and what clears with it
   (bits 0-1, the HUD box, the heartbeat).

**Scope.** Modernized with `steroids` `portable`: the steroid timer is only
lowered by its own drain; incoming damage never touches `+0x366` or the
running bit; armor and health take damage by the existing rules. Prefer the
smallest clean change (e.g. skipping the damage cut for steroids in
Modernized) over new duplicate state, since the steroid timer already exists
on its own. Vanilla and `steroids` `original` keep the original rule; if the
research shows a strict original mode needs the coupling, record that before
changing anything global. Whether this is its own setting or part of
`portable` is a decision to report, not to invent silently.

**Preserve:** portable doses and R (D08A4), the steroids box countdown and
row stacking (D08A8), `dnhyper` in the box (D08A11), the heartbeat and its
start/stop (D08A10, D27A), the steroid kick and other effects, activation and
expiry, pickup selection (D08A9).

**Acceptance.**
1. The coupling is traced and documented (original or ours, and why).
2. Steroids + incoming damage: activate, note the remaining amount, get shot
   repeatedly; armor and health behave normally and the steroid amount is
   exactly what it would have been unhurt (compare against the drain rate,
   frame-step aware, at 60 and 120 fps).
3. Steroids with armor: the two values change independently.
4. Steroids with zero armor: steroids work normally.
5. Armor without steroids: unchanged.
6. Natural expiry: the box reaches zero, the effect and heartbeat end.
7. Save/load and savestate while steroids run: the steroid state is correct
   and not reconstructed from armor.
8. `dnhyper` and an R dose both behave as above.
9. Vanilla and `steroids` `original` unchanged.
10. User check: "activate steroids, stand in front of an enemy and get shot;
    the steroid countdown does not react."

### D08A14 - Death and Continue end steroids

**Done (user-accepted 2026-10-08: "fully accepted").** Death clears running and held steroids in
Modernized `portable`; the user chose that a held dose is lost too. See the
work log entry of the same date and
[note 127](documentation/127-d08a4-portable-steroids.md#d08a14---death-ends-steroids).
User request, 2026-10-08.

**Problem (user):** "if you are using steroids and you die, and use a
continue, you should not still be using steroids and it should be gone from
the inventory." Since D08A13, Duke can die while steroids run (the original
shield made that nearly impossible). Observed in the D08A13 runs: the timer
pauses while Duke is dead (flags kept, `dnhyper` 0x2 and an R dose 0x3), and
after Continue (Cross) the effect resumes with the time left and drains.

**Lead (verify):** TTK's Continue keeps items (D08A4: a held dose survives
death and Continue; D08Q2 handles the jetpack after death and Continue). The
running effect is `+0x364` bit 1 with the amount `+0x366`; the held item is
bit 0. The level-start reset `0x8003fd98` / `0x8003fe10` zeroes items only in
some cases. Find the death/Continue path (D08Q2's notes and `deathroute.py`)
and clear item 4 there in Modernized `portable`.

**Scope and decision to confirm when selected:** steroids running at death
end and are removed (bits 0-1 and the amount cleared): no countdown, no HUD
box, no heartbeat after Continue, nothing in the switcher. Whether a held,
unused dose is also lost at death (Duke 3D loses inventory on death) is a
question for the user, not to be decided silently. Vanilla and `steroids`
`original` unchanged.

**Acceptance.**
1. `dnhyper`, die, Continue: no steroids running, no box, no heartbeat,
   nothing in `[ / ]`.
2. R dose, die, Continue: the same; the dose is gone.
3. Held dose decision as agreed with the user.
4. Savestate taken while dead and reloaded, then Continue: the same.
5. Card save/load and level completion with steroids unchanged (D08A4).
6. Vanilla and `original` unchanged.

### D08A15 - Heartbeat silent when steroids restart mid-run

**Done (user-accepted 2026-10-08: "fully accepted").** Cause found and fixed: a refresh to the full
amount never read above the run's first reading, so the rhythm did not
restart. **Corrected by the user (2026-10-08) and folded in:** a pickup
mid-run now stops the run and leaves a full held dose. See the work log
entries of the same date and
[note 129](documentation/129-d08a10-steroids-heartbeat.md#d08a15---restart-mid-run-beats-at-once).
User request, 2026-10-08.

**Correction (user, 2026-10-08, from a Duke 3D playthrough, "that familiar
feel"):** while steroids run,

| Action | Result |
| --- | --- |
| `dnhyper` | refills to full **and keeps running** (heartbeat restarts at once) |
| `dnstuff`, `dnitems`, `dninventory` | stop the run; a full dose is held (D08G4) |
| Picking up steroids | stops the run; a full dose is held |

The struck-through parts below assumed the original refresh (a pickup
mid-run refills and keeps running); that is no longer the design.

**Problem (user):** "if starting steroids before a previous cycle of
steroids is over, the sound doesnt play for a few seconds at the beginning."

**Lead (verify):** R refuses while steroids run, so a restart mid-run is
~~a pickup that refreshes them (the original refresh, D08A4),~~ `dnhyper`
again, or using a new dose just as the old one ends. `steroids_beat` (D08A10,
`steroids_beat.inc`) restarts the rhythm when the amount rises above the
starting amount and plays the first beat at once; check whether that beat is
refused (the sound routine refuses an id already playing, `0x8006b7b0`, as
found in D12C), whether `beat_start`/`beat_bucket` miss a refresh (a refresh
to 9000 from a high amount, or a refresh in the same update as the drain),
and whether the D08A13 damage hide or the D08A8 HUD hide hides bit 1 at the
beat check. Reproduce first and record which case it is.

**Acceptance.** A pickup mid-run stops the run and leaves a full held dose
(no countdown, no heartbeat; R starts it fresh). A restart mid-run
(~~pickup refresh,~~ `dnhyper`, a dose taken right at the end) beats at once and keeps the normal rhythm from the new
start; a fresh start, natural expiry and the D27A silent Shift are unchanged;
60 and 120 fps.

### D08A16 - Power-up coin icons (human design ticket)

**Todo. User request, 2026-10-08.** Backlog only; do not start until
selected. The **user designs the art**; the agent's part is the placeholders
and the plumbing to drop the finished icons in.

**Background (user):** TTK has three special items, all Duke nuke-symbol
coins that start on pickup and run an invisible timed effect: **invincibility**
(spawn name `invulnerability`, type 1047), **invisibility** (1045) and
**Double Duke** (1046) (type numbers from the D26E spawn table,
[note 111](documentation/111-d26e-debug-spawn.md)). They are TTK's analogs of
Quake's Pentagram of Protection, Ring of Shadows and Quad Damage. D08A17 gives
them real countdowns; first they need icons in the same style as the
inventory icons (D24A, the user's steroids pill icon, D08A4/D08A7).

**When the ticket begins:** add a **placeholder section** for the three icons
to the local font/UI picker page `recomp/analysis/d24a-fonts/ttk-font-picker.html`
(local, not tracked), styled like its existing item-icon sections: one
placeholder per item, labelled with its name, the Quake analog and the type
number, at the HUD box size (16x16 in a 4bpp cell, as the steroids box) with a
larger preview. No switcher-strip variant: power-ups are never held, and if
they ever were, the switcher would use the same icon (user, 2026-10-08).
Then the user draws the icons.

**Done (user-accepted, 2026-10-08).** The user's three icons are in
`recomp/assets/ui/items/` with their 15-colour copies; see the work log.

**Done when:** the user has delivered the three icons (PNG sources in
`recomp/assets/ui/items/`, as `hud-steroids.png`), each reduced to 15 colours
plus transparent with `tools/local/reduce_icon_15col.py`, and accepted them in
the picker. No gameplay change in this ticket.

### D08A17 - Power-up countdowns (invincibility, invisibility, Double Duke)

**Done (user-accepted 2026-10-08: "confirmed it's all working as intended").**
Executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline. User request,
2026-10-08. See the work log below and [note 132](documentation/132-d08a17-powerup-countdowns.md).

**Goal:** a real, visible countdown for each running power-up, like the
steroids box (D08A8, D08A11): when a coin is picked up its effect starts at
once (as now, original behavior), and a HUD box with the item's icon (D08A16)
counts down to zero, the way Quake shows a running power-up. Modernized only;
Vanilla unchanged.

**Investigate first (verify, do not assume):**
1. Where each effect keeps its timer and flags: player fields or globals, the
   full duration, the drain (per frame step, like the steroids drain
   `0x800414a0`?), what ends it and what it clears; the pickup cases for 1045,
   1046 and 1047 in the dispatcher `0x80081a48`.
2. Whether a second coin refreshes, stacks or is refused; what death,
   Continue, level completion, card save/load and savestates do to a running
   effect; what `dnkroz` god mode shares with invincibility.
3. Any original on-screen sign of the effects (tint, sound, status bar) to
   keep.

**Scope (to confirm with the user when selected):** one box per running
power-up, stacked with the steroids box and the selected-gadget box (D08A8
row rules), showing a percent or seconds as the user prefers; consistent
with D08A14 (whether death ends them) and the D08A13 independence rule
(power-ups do not touch armor or steroids). Whether they also get a sound
cue (like the D08A10 heartbeat) or an expiry warning is the user's call.

**Acceptance (draft).** Each coin: pickup starts the effect and its box
counts down at the drain rate to zero, then the box goes with the effect; two
or three at once stack cleanly with steroids and the selected gadget;
savestate, level change and death behave as agreed; 60 and 120 fps; GL and
Software renderers; Vanilla unchanged.

**Decisions (user, 2026-10-08, when selected):** percent like steroids; the
original's 5 s invincibility after Continue shows a box; no expiry warning.

**Work log (2026-10-08, Needs playtest).**

- Research: each coin pickup case of `0x80081a48` writes 6000 (20 s) to its
  own player halfword: invincibility (1047) `+0x8a0`, invisibility (1045)
  `+0x89c`, Double Duke (1046) `+0x89e`; a second coin sets it back to 6000.
  Duke's update drains them by the frame step (`0x80042208..0x80042294`); the
  reset `0x8003f948` zeroes all three at death and level start, and Continue
  then sets `+0x8a0` = 1500 (`0x80042f58`). `dnkroz` is the separate
  `0x800c3cc6`. So death already ends them in the original (as D08A14 wants);
  savestates keep them; a level change ends them.
- Implementation (`gadget_hud.inc`, the existing status bar hook): every
  running timer draws a lit box with its D08A16 icon and `timer * 100 / 6000`,
  stacked a row each above the selected gadget, an unselected jetpack that is
  on and running steroids (order invincibility, invisibility, Double Duke).
  Icons as 4bpp cells at VRAM (968/972/976,205), palettes (1008,208-210),
  uploaded with each box like the medkit and steroids. `items.json` lists them
  as HUD-only art; `test_ui_art.py` checks the cell tables and reductions.
  Debug `controls.powerups`. Timers are only read; no hook, guard or
  generated-code change.
- Evidence (executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`, private
  Xvfb, `recomp/analysis/d08a17-powerups/`, local): one coin, three coins,
  the full seven-box stack (Bio Mask selected, jetpack on, steroids, three
  coins) fits the right column; a box goes when its timer ends; savestate
  reload; death clears all; Continue shows invincibility at 19 then it goes;
  60 and 120 fps; GL 4:3 / 16:9 and Software 4:3 (Software 16:9 cuts the right
  column, existing since D08A6); Vanilla with timers written draws nothing.
  Suites: `ttk-controls-test` (fixture, LEVEL01, all levels), `ttk-input-test`,
  `ttk-inventory-test`, Python 132 OK (2 skipped), `level_overlay_guards.py
  --check`, `check_repo.py`.
- Limits: natural (non-spawned) coins, two-player and boss levels not
  exercised; no profile option (part of Modernized); the number floors like
  steroids, so the last ~0.2 s reads 0.

### D08A18 - Mission inventory: Enter uses the browsed item ("can't use this here")

**Done (user-accepted 2026-10-08: "accepted and confirmed working!!!").**
The user picked mockup B in the cheat-message
style ("option B, but in the console steel palette, just like our cheat
messages ... the font should be exactly the same"). It is built and verified
offscreen. See [note 133](documentation/133-d08a18-mission-item-use.md)
and the work log below. Modernized only; Vanilla and the original Select
inventory are unchanged.

**Background (user):** "if you hover over an item and press enter, currently it
just immediately closes". Today (D08A5, [note
114](documentation/114-d08a5-mission-tracking.md)) Enter or U while the mission
inventory (`,` / `.`) is open just closes it, and it otherwise closes 2.5 s
after the last press. Enter was moved off the gadgets there because the user
instinctively presses it while browsing mission items.

**Picked direction (user, 2026-10-08): option C, use it.** "i actually think
option C is the only one i really like." No info panel and no hold: Enter makes
Duke try to use the browsed item. Where it applies, it does what using it the
original way does; anywhere else a short centred message appears, such as
"CAN'T USE THIS HERE". Options A (hold), B (info panel), D (pin then use), E
(missing-item hint), F (mission sheet) and G (examine) were offered and not
taken.

**Research first.**
- How TTK uses each kind of mission item (key cards, skeleton keys, crystals,
  combo pieces, jewels, papers): automatically on touching the door / socket,
  on the action button at it, or from the original Select inventory. Find the
  check (which flag, which door / socket actor, which range) and whether there
  is a use path that can be driven from the inventory without skipping the
  door's own scripts, sounds, messages or flag changes.
- Whether the disc has an original refusal string or sound (none found in the
  D24A string survey yet; check pickup / door messages and the "need a key"
  style messages). Prefer an original string and sound if one exists.
- If an item can only ever be used by touch, decide with the user what Enter
  does near its door (trigger the same use) and away from it (message only).

**Mockup.** Before game code, show the refusal message (and the success case,
if it differs from the original's) in a new section of the local font/UI picker
page `recomp/analysis/d24a-fonts/ttk-font-picker.html` (built by
`make_picker.py` from `picker.template.html`; local, not tracked): wording,
font, colour and placement against the mission inventory and card from
section 8. The user approves the wording and look.

**Open (decide with the user):** whether the mission inventory closes after a
use or a refusal; whether a refusal plays a sound; whether found but already
used items (if the game tracks that) say something different; what Enter does
on a missing (not found yet) item.

**Acceptance.**
- In Modernized, Enter (and U) on a browsed, found mission item at the place it
  belongs has the same effect as the original way of using it (same scripts,
  flags, sounds and messages), and never uses a gadget.
- Anywhere else it shows only the approved message; nothing in the level
  changes.
- `,` / `.`, `[` / `]`, the 2.5 s timeout when Enter is not pressed, level
  travel, savestate load and death behave as in D08A5.
- Checked with at least one item of each kind that a level uses.
- Levels without mission items are unchanged; Vanilla is unchanged.
- The user confirms it in play.

**Work log 2026-10-08 (research and mockups).**
- The original never uses mission items from an inventory: the Select
  screen's selectable check rejects items >= 6. Seen in Vanilla too: the cursor
  never reaches the mission rows.
- Duke uses an item by pressing action at a lock. The lock's class handler
  says which item it wants (mode 2). Duke pulls the item out (animation 47),
  which clears the item's flag, and the lock opens.
- Shared lock helper: `0x80092a84`. Crystal receptacles: types 180-182 want
  items 11-13. Level 0's card readers (type 478) call the helper with items
  6 and 7.
- Build plan: Enter asks the object in front of Duke (`0x80077f28`, as the
  action press does) for its item. If it is the browsed, found item, Enter
  starts the normal E interaction and the original does the rest. Otherwise a
  message only.
- The disc has no refusal string; the message is new host text.
- Found on the way: a used item's flag is cleared, so the D08A5 mission
  inventory shows a used key as "NOT FOUND YET". This is offered to the user
  as an open question.
- Mockups in section 12 of the local picker (A card status, B centred quote,
  C both, plus the success case), rendered headless with no script errors.
- Not done: a live use at a lock (placing Duke by memory writes put him on the
  wrong floor); overlay handlers of levels other than 0.

**Work log 2026-10-08 (built, Needs playtest).**
- User pick: B, exactly the cheat-message style (`input_notice`: TTK Big
  Italic, Console steel).
- `shortcuts.inc` `mission_use`: Enter / U while the mission inventory is open
  closes it and tries the browsed item.
  - Not found: NOT FOUND YET.
  - Otherwise it asks the object in front of Duke (`0x80077f28` -> `+0x174`;
    item-lock type flags) for its wanted item (handler mode 2) and whether it
    can be used now (mode 0).
  - Right item: the normal E interaction (`input_request_interaction`). Else
    CAN'T USE THIS HERE, and nothing changes.
- New `mission_browsed_item()` in `inventory_hud.cpp`; test stubs added to
  `modern_controls_native.cpp`. Framework untouched.
- Verified offscreen on copies of the user's cards:
  - level 0: both messages away from locks, gadgets untouched;
  - at the empty red-crystal holder, the red crystal was used through the
    original path (animations 47 and 267, holder filled, flag used up, weapon
    redrawn);
  - another item there was refused with nothing changed.
- Suites: inventory, input, controls (44), Python 132 OK, overlay guards,
  `check_repo`.
- Defaults (not asked): one message for every refusal, no sound, used items
  not tracked (they show as missing).
- Not done: a card reader or key door live; levels other than 0; a real
  playtest.

### D08A19 - Used mission items: USED status and a green tick (user art)

**Done (user-accepted 2026-10-08). User request, 2026-10-08:** "When we find a mission item, in the
switcher it changes to FOUND. When a mission item has been used, change to USED
and we should indicate that on it's icon in the switcher too. For this, refer
to the file "Subway Security Key Used.png" inside the research/inv directory. I
want a placeholder for a GREEN tick i will make".

- Today (D08A5) the info panel says FOUND or NOT FOUND YET. Once the item has
  been used (consumed by its door, switch or socket, including through the
  D08A18 Enter use), say USED and draw the tick over the icon's bottom-right
  corner in the switcher strip and the info panel, as in the mockup
  (`research/inv/Subway Security Key Used.png`, where a gold check stands in
  for the green tick).
- Art (the user's): `recomp/assets/ui/items/mark-used.png`, recommended
  **8x8 px**, transparent, 1-px dark outline, drawn on the 16x16 icon's grid
  over pixels x 8..15, y 8..15 (10x10 also fits). Placeholder and previews:
  font picker section 14 (local `recomp/analysis/d24a-fonts/`).
- Research first: how "used" is known per item (the original removes or
  flags the inventory entry when a key is consumed; persistence across saves
  and level changes, as D08A5 tracks FOUND), and which status colour USED takes.

**Acceptance:** a used mission item reads USED with the user's tick on its
icon in the switcher and the info panel, in at least two levels; found but
unused items still read FOUND; saves keep the state; Vanilla and the original
Select inventory unchanged; the user confirms.

**Accepted 2026-10-08.** User: "well thats absolutely superb. I fully accept, this is the sort of design extension i wanted, introducing something the original game never had, and it looks completely at home. I'd call this feature finished." Executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8` is the regression baseline.

**Work log 2026-10-08 (Needs playtest).** The user delivered the tick
(`research/inv/mark-used.png`, 8x8 green with a dark outline) and asked for
used items in full colour with the green tick. Details in
[note 137](documentation/137-d08a19-used-mission-items.md).
- Font picker first: section 14 now shows the delivered tick on every mission
  design (`recomp/analysis/d24a-fonts/`, local); the art is copied to the
  tracked `recomp/assets/ui/items/mark-used.png`.
- Research: for items 6..16 the original only tests, sets and clears bit 0 of
  `player+0x354+4*i` (pickups `|= 1`, use at animation 47 `8004e4b8` `&= ~1`
  with `+0x3b8 = 1`, `+0x3b9 = item`; level reset zeroes it; `91fc4` gives a
  held item back with `|= 1`). Nothing else marks "used".
- Built: `mission_used_track()` (shortcuts.inc, Modernized only, every update,
  captured or not) sets marker `0x4000` in the item's own flag when bit 0 is
  clear while Duke holds that item, and drops it when bit 0 comes back. It
  lives in guest RAM, so savestates and card saves keep it and a level reset
  clears it. The HUD reads used = marker set and bit 0 clear: full-colour icon
  plus the tick at the icon's bottom-right (2x) in the strip and on the card,
  status USED in the mockup's gold (new font set 12 `#ffdc30`), counted as
  collected. D08A18 Enter on a used item: "ALREADY USED".
- Pack: `ttk-mission-items.pack` kind 5 = the tick.
- Verified (build, private Xvfb Modernized on card copies,
  `recomp/analysis/d08a19-used/`, local): level 0 live use of the red crystal
  at its holder with Enter (holder opened, flag `0x0001 -> 0x4000`), strip and
  card show the tick and USED, count 2/5, Enter says ALREADY USED; savestate
  saved, the older slot loaded (`0x1`), the saved one back (`0x4000`). Level 6
  (UI slot 2 copy): the original's use writes staged on a skeleton key mark it
  (`0x4000`), strip and card show it, given back drops it (`0x1`). Vanilla, same
  staging: never marked. Suites: `ttk-inventory-test` (new used/found-again/
  never-found cases), `ttk-controls-test`, `ttk-input-test`, `ttk-font-test`,
  Python 139 OK (2 skipped), `level_overlay_guards.py --check`,
  `check_repo.py`. Executable `d67705eba48232029a90647cc781f3201d0aae29799678466b2b5b9337b5dbe8`.
- Not verified: a real key door or card reader use (only the crystal holder
  was used live); a memory-card save and reload after a use; the item name
  stays orange as for FOUND (the mockup's blue name came from a not-found
  screenshot).

### D08A20 - Challenge item shown on the HUD once found (user art)

**Done (user-accepted, 2026-10-09: "fully accepted. ... this is it, excellent!!!").**
Shipped layout is **option A**: 14x14 icon in the native box's left cell (divider
as the right edge), top-right. Executable
`a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95`. Note
[139](documentation/139-d08a20-challenge-hud.md).

**Was Needs playtest. User request, 2026-10-08:** "Challenge item - it says surprise on the
screen. Should show on screen that we got the timer device thing, as an item on
our hud. Add the placeholder for me to make an item on the ttk font picker
document and i will make an original asset based off the in game item."

Found (read-only SLUS-00583): the challenge item is object type 633. The pickup
dispatcher `0x80081a48` sends it (`0x80081d04`) to `0x80082cfc`, which sets
`player+0x85e` = 1 and queues pickup message 204, "SURPRISE" (the strings also
hold 250 "CHALLENGE FOUND:"). Nothing on the HUD shows it afterwards. In game
it is a round black device with two red tips joined by a silver bar.

**Placeholder done (2026-10-08):** the local font picker
(`recomp/analysis/d24a-fonts/ttk-font-picker.html`, built by `make_picker.py`)
has section 15, "Challenge item - placeholder for your art": a 16x16 HUD-box
slot (`recomp/assets/ui/items/hud-challenge.png`, reduced to 15 colours with
`tools/local/reduce_icon_15col.py`), 8x/2x/1x and 15-colour previews, three
in-game reference frames (`spawn 633` in level 0, private offscreen run,
`recomp/analysis/challenge-item/`) and a HUD mockup.

**Adopted (2026-10-09, D08A21 closeout):** now tracked as
`recomp/assets/ui/items/hud-challenge.png` and `hud-challenge-15col.png`
(items.json CHALLENGE ITEM, type 633, `"hud": true`); the full 16x16 version
is `recomp/assets/ui/items/16px/hud-challenge.png`. Not drawn yet.

**Icon delivered (2026-10-08):** `research/inv/hud-challenge.png` (16x16, art
14x14 inside a 1-pixel transparent border, 152 colours), with its 15-colour
reduction `research/inv/hud-challenge-15col.png` made by the project tool. Both
are shown in the picker (read from `research/inv` until the job copies them to
`recomp/assets/ui/items/`). User: it "will actually sit in the top right". The
picker's mockup now shows the top-right corner, with the ammo box's margin, in
four variants: A icon cell only (the box's left cell, its divider as the
edge), B the full 46x16 box with an empty number field, C the full box with
the D08A19 green tick in the number field, D the icon alone. The campaign HUD
leaves that corner empty; the challenge stages use it for their kill counter
but have no challenge item.

To decide when the job starts: which top-right variant (A-D), how long it
stays (rest of the level is assumed), whether the box
shows a tick or nothing in the number field, whether the SURPRISE message
changes (for example CHALLENGE FOUND), whether it appears in the [ / ]
switcher, and what loading a save or restarting the level does (`+0x85e` is
saved and restored by the original; verify). Modernized only.

**Acceptance:** the user's icon is delivered and accepted in the picker; in
Modernized, picking up the challenge item shows it on the HUD as decided, in
at least two levels, and it survives save/load as the original flag does;
Vanilla unchanged; the user confirms.

**Work log (2026-10-09, Done - option A accepted):** user: "fully accepted."
During playtest they switched from D to A: 14x14 icon in the box's left cell.
`gadget_hud.inc` draws `hud-challenge-15col.png` with `cell_only` (box width
19); right edge matches the ammo box. Font picker section 15 marks A accepted;
section 8 reworked (E shipped first, A-D historical). `test_ui_art.py` OK.
Executable `a047b37bbb833b84902a3ad344fff2e16c22ae3dc0093ad441d80c3eb9235a95`.

**Earlier work log (2026-10-09, option D):** user selected D, icon alone, and
the larger art under `recomp/assets/ui/items/16px/`. Verified offscreen with
spawned type 633 in levels 0 and 5, savestate round trips, Vanilla clear.
Executable then: `ec8633b095aa70d3a2393613b216a75d47e80322fcb29b4b5a3f26210962bc1c`.

### D08A21 - Re-export HUD item icons with 14x14 art inside the 16x16 canvas

**Done (2026-10-09, user-accepted: "completely approve this change so the small
swapout job can be closed off and accepted").** The user's 14x14 icons are the
active art; the full 16x16 versions are kept in `recomp/assets/ui/items/16px/`.
At acceptance the user added `research/inv/challenge-16.gif`, the full 16x16
challenge icon: converted losslessly (single frame, transparency kept) to
`16px/hud-challenge.png` with its 15-colour copy; the 14x14 challenge icon was
adopted as `assets/ui/items/hud-challenge.png` (+ `-15col`), listed in
`items.json` as CHALLENGE ITEM (`"hud": true`, so the mission pack skips it;
the pack stays byte-identical). Nothing draws it until D08A20.

Was Todo, waiting for the user's art. User request, 2026-10-08: "re-export the
medkit, steroids, and invincibility etc coins as those icons all touch the
border so im still making them 16x16 but the actual image is 14x14 within. so
we will swap them out. i am working on the images now and will let you know
when to update the document."

Current art touching the 16x16 edge (opaque bounding boxes): medkit
`gadget-medkit.png` (0,1)-(16,14), full width; steroids `hud-steroids.png`
(4,0)-(12,16), full height; `hud-invincibility.png`, `hud-invisibility.png`,
`hud-double-duke.png` all (0,0)-(16,16). The new versions keep the 16x16
canvas with the art in the inner 14x14 (a 1-pixel transparent border), like
the challenge icon (D08A20, `research/inv/hud-challenge.png`).

When the user delivers:

1. Update the local font picker first (`recomp/analysis/d24a-fonts/`, sections
   9-11 and the medkit card) so the new icons can be compared with the old ones;
   the user accepts them there.
2. Copy the sources into `recomp/assets/ui/items/` under the same names and make
   each `-15col.png` with `recomp/tools/local/reduce_icon_15col.py`.
3. Regenerate the 4bpp cell and CLUT tables in `recomp/src/ttk/gadget_hud.inc`
   (`k_medkit_cell`, `k_steroids_cell`, `k_invincibility_cell`,
   `k_invisibility_cell`, `k_double_duke_cell` and their CLUTs) and the
   inventory-icon builds that read them (`tools/local/build_ttk_inv_icons.py`:
   medkit item 5, steroids item 4, and the switcher strip's full-colour
   copies); `tests/local/test_ui_art.py` and `test_ttk_inv_icons.py` must pass.
4. Check in game (Modernized): the switcher strip, the selected-gadget box, the
   steroids and power-up countdown boxes; the icons sit inside their boxes
   without touching the frame.

No gameplay change. Vanilla unchanged.

**Acceptance:** all five icons replaced from the user's 14x14-in-16x16 art,
accepted in the picker, tables and tests regenerated, shown correctly in every
HUD place they appear; the user confirms.

**Work log, 2026-10-08 (Needs playtest).** User: "you will find these new
icons designed at 14px, which we should use now, but i dont want you to
totally get rid of the 16px icons ... for now, can you swap those icons in."

- Delivered in `research/inv/items-14/` (16x16 canvases; art in (1,1)-(15,15),
  medkit (1,2)-(15,13), steroids (5,1)-(11,15)). `hud-invincibility-14.png.png`
  had a doubled extension; copied under the right name.
- The old full-16 art and its 15-colour copies moved (git mv) to
  `recomp/assets/ui/items/16px/` with a README; nothing in the build reads
  them. The new art took the active names, so `items.json`, CMake, the
  inventory pack builder and the tests keep their paths.
- 15-colour copies for all five made with `reduce_icon_15col.py` (the medkit's
  was a hand-picked reduction before; now the tool's, and the reproducibility
  test covers it). The five `k_*_cell` / `k_*_clut` tables in
  `recomp/src/ttk/gadget_hud.inc` regenerated from them.
- Tests: `test_ui_art.py` (new `test_hud_icons_leave_a_border`: active art
  and 15-colour copies stay inside the inner 14x14, the 16px originals exist)
  and `test_ttk_inv_icons.py` OK. The switcher-strip size bound was relaxed
  from 8 to 6 px, since the trimmed steroids pill is now 6x14.
- Build: executable `72429e350806e0639f36e50cff0cb969f18df1662a2336794e17bd2263614c21`;
  `ttk-inv-icons.pack` rebuilt.
- Offscreen check (private card copy, `recomp/analysis/d08a21-icons/`): the
  three coin boxes, steroids box and medkit gadget box all show the icon with
  a dark gap inside the frame (before: touching it); the switcher strip shows
  the new medkit and steroids (slightly smaller than before).
- Font picker rebuilt: the power-up, steroids and medkit cards read the new
  active art.

Not verified: the user's look in play at their resolution and renderer.

### D08A22 - Switcher strips and mission card scale with the window like the native HUD

**Done (2026-10-18, user-accepted: "fully accepted!!").** Executable
`67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546` is the regression baseline. Was Needs playtest; executable
`67eb7bc2d189b38624879a25f9b8c76c5567aa40a61e357a5e07352f394d4546`.

Implementation (framework, exported into
`recomp/patches/time-to-kill-accepted-source.patch`; a pristine pinned checkout
plus the patch reproduces both files): the inventory/mission strip and the
mission card now take their scale from the fitted game picture, not from an
integer floor of the window height. Scale = picture height / 480 (fractional,
nearest sampling, at least 1x), where the picture is the letterbox rect the
game itself is drawn into (`letterbox_rect` in `gpu_gl_renderer.c`;
`SDL_GetRenderLogicalPresentationRect` in the Software path of
`host_osd.c`). The game's own HUD is drawn at picture height / 240 and the
strip art is authored at 2x, so the two now match at every size. The strip and
card are also centred and placed inside the picture (pillarbox or letterbox)
instead of the whole window. The available width for the strip/card is the
picture width. Only the Modernized host strips changed; Vanilla draws none of
them.

Evidence (offscreen Xvfb, private card copy of UI slot 3, port 9331,
`recomp/analysis/d08a22-scale/sizes.py`, local only): eight window sizes
(640x480, 800x600, 960x720, 1200x900, 1280x720, 1600x900, 1440x1080,
1920x1080) with the gadget strip and the mission strip + card, in OpenGL 4:3,
OpenGL 16:9 and Software 4:3. Normalised to the picture, the strip has the
same proportions at all eight sizes (`sheet-strip.png`). Same icon in the
strip and the native HUD box (jetpack flame): OpenGL 4:3 18 vs 18 px at
1440x1080 and 10 vs 10 at 800x600 (the old code gave 16 and 8); OpenGL 16:9
14 vs 13 at 1440x1080; Software 4:3 visually the same size (about 64 vs 68 px
jetpack). Mission card fits inside the picture at 640x480 16:9 (letterboxed)
and at 1920x1080 4:3 (pillarboxed).

Limits: at non-integer scales nearest sampling makes some art pixels one output
pixel wider than others, as with the native HUD at the same scale. Below a
480-line picture (16:9 in a 640x480 or 800x600 window) the strip stays at 1x,
slightly larger than the native HUD (8 vs 7 px), so it stays crisp. Not yet
seen by the user in real windows or in exclusive fullscreen.

**Was Todo. User request, 2026-10-09:** "the scale of our new UI elements matches
the scale of the native ui elements, specifically where the icons are used,
specifically our inventory and mission item switchers. because those icons
appear smaller than the ones inside the native ui only when the window is
small. so usually in fullscreen etc this problem doesnt exist."

Lead (read, not yet measured): the strips and the mission info card are
host-drawn images (`ttk_inventory_image` / `ttk_mission_card_image`,
`recomp/src/ttk/inventory_hud.cpp`, icons at 2x in a ~480-tall logical space)
presented by `host_osd.c` (pinned framework, about line 913) with
`scale = output_height / 480`, an integer floor, at least 1. The game's own
HUD scales continuously with the window. So below 960 output pixels the
strip is drawn at 1x while the native HUD is at up to 2x (for example a
900-tall window: strip 1.0 vs native about 1.9); at 1080 it is 2 vs 2.25, close
enough that fullscreen looks right. The non-GL path (`#else`, `ui`) needs the
same check.

Direction: give the host strips the native HUD's scale (the same factor the
game's 2D HUD gets from the window and the 4:3 / widescreen layout), either
fractional with nearest sampling or integer rounding chosen per size, keeping
icons, frames, the microfont digits and `%` sharp; check the font-drawn card
text. Framework change: export it with `export_runtime_patch.py`.

**Acceptance:** at several window sizes (for example 640x480, 960x720,
1280x720, 1600x900 and fullscreen), OpenGL and Software, 4:3 and widescreen,
the inventory and mission switcher icons are the same size as the native HUD
icons (steroids/medkit boxes) and stay crisp; layout does not overflow at small
sizes; Vanilla unchanged; the user confirms.

### D08A23 - Instant night vision

**Status:** Done, user-accepted 2026-10-25 with explicit lock-in and closeout authorization. [Note 244](documentation/244-d08a23-instant-night-vision.md).
N must toggle night vision immediately, including while firing, with no holster,
equip animation, shot delay or movement lock. Goggles can simply appear on Duke's
face. Add a persistent independent settings toggle; disabled retains the original
activation behavior. Modernized only; Vanilla unchanged. Cover N and selected-item
activation, on/off, held fire, empty charge, depletion, save/load and both views.

Initial source research: `input_bindings.def` maps goggles to N. In
`shortcuts.inc`, goggles are item 3; `use_item()` requests native `0x800402d0`
through inventory flags at player+0x360 (charge +0x362). `select_weapon()` rejects
ordinary gadget input while fire/interact is held or the upper-body weapon is not
idle. Bypassing the visible animation alone therefore cannot meet the request.
Before implementation, trace the native toggle's effect/model and charge lifecycle;
reuse its semantics without stealing weapon animation ownership. Verify resident
code identity and all writes. Setting: NIGHT VISION MODERN / ORIGINAL, Modern by default; applied live on CONFIRM.

### D08A24 - Automatic and instant Bio Mask

**Status:** Done, user-accepted 2026-10-25. User: "lock it in, close it out".
Implementation and documentation closeout authorized; private slot 11 tested.
The user also requested instant manual use while shooting and supplied a licensed
gas-decompression sound for on/off, with edits authorized.

AUTO BIO MASK ON (default) equips an owned, charged mask before native toxic-area
head-exposure damage. B and inventory Enter toggle it instantly without changing
weapon, animation or camera; firing continues. The independent persistent toggle
is in OPTIONS > ADVANCED > ITEMS. OFF retains original manual behavior; Vanilla
stays original. Native charge/protection/depletion rules remain.

The mask removes after three consecutive native updates outside head exposure,
with one removal cue and no further charge drain. Brief boundary dips reset the
grace. Manual removal in gas, or replacement by Modern goggles, suppresses
auto-use until exit/re-entry. Loads start a fresh exposure check: active gas saves
stay protected; active clean-air saves remove after the grace. Charge is retained. No generic liquid,
scuba or other-hazard protection is introduced.

The edited 0.55-second gas cue plays on successful automatic/manual activation
and manual/automatic removal, following effects volume. Empty attempts, held
repeats, depletion and restoring active gas saves are silent. Build/native/profile tests and private
60/120 FPS routes pass; user accepted control feel, sound mix and automatic exit.
See [note 246](documentation/246-d08a24-auto-bio-mask.md). Commit/push and
acceptance closeout authorized; no next job started.

### D08A25 - Night vision on/off sound

**Status:** Done, user-accepted 2026-10-25 with explicit lock-in and closeout.
The selected A-tight (0.24 seconds) cue on activation and manual removal is
accepted in game. D08A23 remains complete. [Note 245](documentation/245-d08a25-night-vision-sound.md).
User requested a new backlog job, then sound options before game integration.
Shorten and reshape the supplied camera-flash effect to follow the duration,
pitch contour and envelope of the Duke 3D goggles reference.

Sources: [Old camera flash on Pixabay](https://pixabay.com/sound-effects/technology-old-camera-flash-6800/),
listed under the Pixabay Content License; local source
`research/freesound_community-old-camera-flash-6800.mp3`. Reference only:
`research/screencaps/nv-sound.mp4`. Keep source media and audition exports local.

Acceptance:
- Present several trimmed, pitch/envelope-shaped options and obtain the user's
  sound selection before wiring the chosen asset into gameplay.
- Play the approved cue once when Modern night vision successfully changes
  on or off through N or selected-item activation, including while firing.
  User requested matching removal audio on 2026-10-25.
- No cue on depletion, rejected/empty activation, held-key repeats,
  pause/resume or loading an already-active state. No added input/fire delay.
- Respect effects volume/mute; verify repeated toggles, both camera views and
  mixing with gunfire. Preserve native battery, visuals and Original/Vanilla.
- Record source/license provenance and reproducible edits for the selected
  asset; user sound and in-game mix acceptance required before Done.

The chosen cue is integrated. Build/native tests, private 60/120 routes and host
PCM checks pass; user accepted the in-game mix. See note 245 for limits.

### D08A26 - Bio Mask first-person visor / respirator overlay

**Status:** Done (2026-10-25). User accepted the integrated low-poly mesh and
final native-lighting pass, then explicitly authorized commit, push and closeout.
The real textured mesh follows active-mask state, view bob and weapon lighting.
Implementation and evidence: [note 247](documentation/247-d08a26-bio-mask-mesh.md).

Add a Modernized first-person overlay while Duke actually wears the Bio Mask.
Preferred starting direction: low central respirator fitting and ribbed tubing
along the bottom, inspired by the presentation in Duke 3D. A full helmet rim is
optional, not required. User liked the first Photon silhouette and requested a
more grungy, low-poly PS1/N64 appearance. Preserve sightline, weapons and HUD.
D08A24 is accepted; reuse its actual native active state, not merely item
ownership, inventory selection or gas presence. D08N remains cancelled: this
adds no scuba item, underwater oxygen or protection changes. Vanilla unchanged.

Acceptance:
- Art direction reviewed before integration: original editable Blender mesh,
  exact mirrored hose geometry, low-poly faceted shading and restrained
  materials matching TTK. Review viewpoint renders and game-scale composition.
  Retail Duke 3D pixels remain local references only.
- Appears/disappears correctly on manual and automatic equip/removal, depletion,
  goggles replacement, death, load/restart and first/third-person transitions.
  Only the active first-person view receives the overlay; menus/cutscenes do not
  retain a stale mask. Existing sounds, charge and protection remain unchanged.
- Follows the established view-bob/motion settings and presentation clock with
  restrained head-attached motion, no independent unsynchronised oscillator or
  doubled camera bob. Verify bob off and available strengths, idle/walk/run,
  crouch/jump/land, camera turning and action-specific perspective changes.
- Verify smooth 60/high-refresh presentation, 4:3 and 16:9 viewport anchoring,
  scaling without distortion, transparent edges with movement margin, weapon,
  crosshair and HUD readability. Keep effects unobtrusive; user play acceptance
  is required. Use private settings/save/card copies for later gameplay tests.

Research: existing bob state is in recomp/src/ttk/first_person.inc;
[note 246](documentation/246-d08a24-auto-bio-mask.md) documents native mask state.
Local references: ~/Documents/dukeextract/tiles_out/tile2581.png
and research/Screenshot_2026-10-25_13-13-02.png. Half-sprite/mirroring is a
possible art optimization, not a required renderer design.

External candidate: https://github.com/saegiru/UltimateDoomVisor/blob/master/HIRES/Visor/169/A_DMG00.png
Repository tree e3d911674c0efbc69e6699db19889b58bc862bf7 inspected on 2026-10-25:
no LICENSE/COPYING file found. CREDITS.txt gives informal permission to modify
and use the HUD, but credits WildWeasel, 2800fps, Lightsource and Yamil for
various visor artwork without mapping this exact PNG to its author. Exact
asset provenance/redistribution terms remain unresolved; do not ship it on
that evidence alone. Credits: https://github.com/saegiru/UltimateDoomVisor/blob/master/CREDITS.txt
Original Photon concept directory: ~/.photon/artifacts/duke-biomask/.
No external or retail asset is included in the concept.

### D08A27 - Toxic gas fills the widescreen viewport

**Status:** Done (2026-10-25). User accepted the widescreen gas fix and explicitly
authorized closeout. Implementation and evidence: [note 248](documentation/248-d08a27-widescreen-gas.md).
User reports that toxic gas remains confined to the original 4:3 area while
playing widescreen. Use a private copy of UI slot 11 (file slot10) to reproduce.

Acceptance:
- Identify the native gas draw and extend its coverage to the active gameplay
  viewport at 16:9 and wider/auto aspect ratios, without stretching the world.
- Preserve native gas colour, opacity, animation and entry/exit timing; no change
  to toxic damage, Bio Mask charge/protection, or non-gas screen effects.
- Verify full coverage through camera movement, window resizing, first/third
  person, mask on/off and 60/high-refresh replay. No seams or uncovered side bands.
- Native 4:3, menus, cutscenes and Vanilla retain their existing presentation.
- Record before/after evidence from slot 11. User play acceptance required.

### D08A28 - Bio Mask PS1 authenticity pass

**Status: Done. Revision 5 accepted and closeout authorized 2026-10-26.**
Deliberately demake the accepted original 3D mask to look native to 1998 TTK.
[Note 264](documentation/264-d08a28-biomask-ps1-authenticity.md) preserves the full
artistic scope, initial pipeline inspection, reference study, delivery and tests.
Study Energy Weapon (keyboard 7 / record 10), Flamethrower and especially the
Buffalo Rifle pump grip before revising geometry or textures. Match their angular
construction and apparent pixel density; diagnose UV/projection/sampling causes
of stretch instead of blindly reducing texture size or polygon count.
Adapt the existing Blender pipeline, preserve a restorable accepted revision,
simplify tubing/housing and texture detail while retaining design, framing,
lighting, true 3D, gameplay and runtime behavior. No global weapon/filter changes.
Acceptance requires matched in-game original/revision/reference comparisons,
rollback evidence and lighting/lifecycle/performance checks. Numeric counts and
Blender previews alone cannot establish artistic acceptance. Accepted revision 5: 184 triangles, plain five-sided tubes without geometric ribs,
Buffalo-style painted grip bands and Flamethrower-style metal edge/recess detail. Build/native suites, private 60/120 lifecycle,
lighting, software and Vanilla checks pass; original art is preserved. User
accepted the final Buffalo-barrel steel gradients and requested foundational style
documentation. See [note 265](documentation/265-native-mesh-art-style.md) for the
reusable recipe and note 264 for evidence and limits.

### D08A29 - Scroll-wheel weapon changes while running

**Status: Done (user-accepted 2026-10-26). Dependencies: D08A, D08AA (complete).** User wants wheel weapon
cycling without releasing run. Reproduce forward/back/strafe/diagonal running,
Shift and RUN MODE, wheel both directions, base/upgraded and remote transitions.
Investigate command consumption, movement eligibility and upper-body idle gates;
wheel ingestion itself has no Shift gate. See note 269; cause not established.
Acceptance: eligible owned weapons cycle while movement/run stays continuous,
without lost notches, unintended firing or deferred surprise changes. Preserve
number keys, firing/reload safety, Alt-wheel camera distance, menus, swim/flight,
first/third view and Vanilla. Candidate recognizes native run-ready upper loops
18/19; build/native suites and bounded live checks pass. User accepted the
result and authorized closeout. The scripted live remote switch-out limit
remains recorded separately from user acceptance. See [note 270](documentation/270-d08a29-running-weapon-selection.md).

### D08A30 - Reverse weapon-wheel direction

**Status: Done (user-accepted). Dependency: D08A29 (Done).** User requests the opposite of the
current weapon-wheel mapping: **wheel up advances to the next weapon; wheel
down returns to the previous weapon**. Selected and implemented 2026-10-26.
Keep the weapon cycle's ordering and availability rules; reverse only the wheel's
mapping to previous/next, not number-key groups or the last-weapon action.

Acceptance: captured Modernized gameplay uses up = next and down = previous,
including while running; multiple notches and fractional wheel accumulation
follow the same direction. Preserve firing/reload/draw safety, Alt-wheel camera
distance, menu and console scrolling, swimming/flight and Vanilla. Keep explicit
previous/next keyboard actions unchanged. Update the manual and input tests when
implemented. Test evidence and user acceptance are recorded below.

### D08A31 - Number-key weapon selection while holding run

**Status: Done - closed with no action required (user-directed 2026-10-26). Dependencies: D08A29, D08A30 (Done).**
Investigated Left Shift + W + 1/2 slowing Duke without selecting a weapon.
Physical trace shows missing number events and unexpected Shift releases;
3/4 arrive normally. User reports the same failure in EDuke. Right Shift
and Caps Lock RUN MODE work, verified by user and trace. Keyboard/host input
is implicated; the specific hardware cause is not conclusively established.
User: "close this one with no action required and close us out".
No gameplay/control changes. Temporary tracing and diagnostic test additions
removed; normal player rebuilt. Documentation closeout and commit/push
explicitly authorized. [Evidence and limits](documentation/274-d08a31-running-number-keys.md).
Left Shift remains the intended run binding. No next job started.

### D08A32 - A distinct ammo icon for every weapon in the bottom-right HUD

**Status: Done (user-accepted 2026-11-07; [note 407](documentation/407-d08a32-ammo-icons.md)). Dependencies: D08A21, D24A.** The
bottom-right ammo box uses the same (animated) ammunition icon for every
weapon; for example the flamethrower shows animated machine-gun cartridges.
The user wants a proper icon per weapon/ammo type, made like the accepted
16x16 gadget icons (steroids, medkit, invincibility, invisibility, Double
Duke, HUD challenge): 14x14 artwork inside a 16x16 transparent canvas so it
never touches the frame in the framed HUD (see the TTK font picker,
`research/TTK-UI-and-Font-Research.html`, and D08A21).

Process to automate and evaluate: capture each piece of ammunition in game at
its best angle, ideally isolated; reduce to 14x14 with bilinear reduction in
Photon (the photon skills); fill the feathered, semi-transparent edge by
stacking duplicate layers until opaque; place it in a 16x16 canvas. Research
first: how the native HUD draws the ammo icon and its animation, whether a
static icon per weapon is better than animating (likely), and which ammo
types the weapons actually share. Produce an artifact with all the icon
designs for the user's review before any game integration.

**Acceptance:** user-approved icon set (review artifact first); each weapon
shows its own ammo icon in the bottom-right box, framed and borderless HUD,
all HUD scales; tracked or disc-derived sources per D24A; Vanilla unchanged.

### D08A33 - EQUIP PICKUPS: IF NEW (default) / NEVER, with new weapons auto-equipped

**Status: Done (user-accepted 2026-11-02: "i accept the pickups job!!"). Dependencies: D08A9, D08A, D19 (in-game options).** EDuke32
has an "equip pickups" choice. TTK should have the same option with two values:

- **IF NEW** (the new default): picking up a weapon Duke does not already own
  (for example the Gatling gun) switches to it at once. Picking up an inventory
  item or object he does not already have (for example steroids) makes it the
  selected item. It is only selected, not used: Enter then uses it.
- **NEVER**: nothing is equipped or selected automatically. Today's weapon
  behaviour is NEVER.

User: "having tested it out, this functionality appears to exist perfectly
for the items, just not the weapons." Items already follow D08A9
(`recomp/src/ttk/pickup_select.inc`): a picked-up gadget becomes the [ / ]
selection, and refills select it too. The work is mainly new weapons, plus
making the option govern both parts.

Initial notes (not yet researched): find the weapon pickup path, mark a
weapon as new when its ownership bit goes from 0 to 1, and switch through the
original weapon-change request (as D08A29/D08A30 wheel changes do) so the
draw animation, ammo and HUD stay native. Decide the cases that should not
switch:
- Duke is firing, climbing, swimming, pushing or mid-throw;
- the `dnstuff`/`dnitems` cheat grants (EDuke32 cheats do not switch);
- an ammo pickup for a weapon he already owns. User (2026-11-02): ammo
  never affects it in EDuke32; only a new weapon switches.

Under NEVER, the D08A9 item selection is off as well. Save the option in the
Modernized profile and show it in the in-game OPTIONS. Vanilla stays
unchanged (NEVER, original behaviour).

Test scene (user): player UI slot 12 has the Gatling gun right in front of
Duke; spawning items or weapons is also fine.

**Acceptance:** with IF NEW, a new weapon is drawn on pickup and a new item
becomes the selection without being used; owned weapons and their ammo never
switch; NEVER switches nothing; the option is saved and shown in OPTIONS;
Vanilla unchanged; the user's playtest (Gatling gun and steroids).

**Work log 2026-11-02 - built (Needs playtest).** Executable
`2fd0b986485196af4eb17d7f6cd86315579e738f0db1c337198e0a0be426831b`.
[Note 361](documentation/361-d08a33-equip-pickups.md).

- Research: weapons are given by the dispatcher's helper `0x800818ec`; it sets
  ownership bit 0 of `+0x2c4+4*slot` only for an unowned weapon, else adds
  ammo; ammo pickups (`0x80081720`) never touch it. The helper's own weapon
  request runs only in multiplayer.
- Implementation: `pickup_select.inc` snapshots weapon ownership with the D08A9
  items around the dispatcher call; a weapon that became owned is drawn from
  `select_weapon` through the number keys' request (`+0x3ba`, flag 4) and
  gate, waiting up to 60 player updates (about two seconds) when Duke is not
  ready (fire held, climbing, pushing, mid-throw); a weapon key, savestate
  load or death drops it. NEVER also stops the D08A9 item selection. OPTIONS >
  ADVANCED > ITEMS > EQUIP PICKUPS (live), profile schema 61
  (`equip_pickups`, if-new default), `run.py --equip-pickups`, terminal
  choice E; Vanilla always never. No hooks, codegen or framework change.
- Evidence: `ttk-controls-test` 122 groups (new D08A33 group), input, aim and
  near suites, Python profile/input tests (new EquipPickupsOptionTest). Live
  private slot-12 runs: IF NEW draws the Gatling gun at the pickup (HUD 150);
  with fire held it waits and draws on release; an owned Gatling's ammo
  pickup (10 -> 160) and `dnstuff` do not switch; NEVER keeps the shotgun;
  spawned steroids become the selection (held, not used) under IF NEW and
  leave a selected medkit under NEVER; the OPTIONS row changed with real keys
  saves `never` to the profile. Player files unchanged.
- Limits: the user's playtest; only the Gatling gun was picked up live;
  climbing/pushing/swimming/jetpack pickups not exercised live; Vanilla by
  construction and native test. The unrelated `test_primitive_trace`
  (near-renderer, files not touched here) fails in the Python discovery run.

**Work log 2026-11-02 - spawn weapon names; console holster report.** User,
while testing: "can you add spawn numbers for the weapons, when you type the
"spawn" command in on its own" and "duke will holster his weapon when you are
in the terminal". `spawn` (and `items`) now list "Weapons here" and "Ammo
here" by name and type (`spawn.inc`); the types come from the pickup
dispatcher and were checked live in level 0: every listed weapon gave its slot
and was drawn under IF NEW, every ammo pickup only added ammo without a switch
(table in note 361). The holster in the console did not reproduce offscreen
(typing, Enter, clicks, 25 seconds open, the player's settings copy); waiting
for the exact steps, no fix made.

### D08A34 - The medkit cannot be used in water (underwater or on the surface)

**Status: Done (user-accepted 2026-11-03: "perfect. lock it in, close it out"). Dependencies: D08A7.** User:
"There seems to be a bug where you can't use the medkit underwater and you
can't use the Med kit either on the surface of the water. use slot 11 to
test." Fixture: UI slot 11 (file slot 10, re-saved 2026-11-03, the sewer with
water ahead); test on private copies, spawn the medkit (`spawn medkit`) and
take damage (`hurt=`) as needed. Not investigated yet: first find whether the
original game refuses the medkit while swimming (Vanilla, same spot) or
whether a Modernized swimming path (D08O swim controls, the use/inventory
input while swimming) drops the request. Fix the cause game-wide, keep
Vanilla's own rule if the original refuses it there.

**Acceptance:** in Modernized the medkit heals underwater and on the water
surface when Duke is hurt (slot 11); grounded use unchanged; Vanilla
unchanged; the user's playtest.

**Work log (2026-11-03).** Found: `select_weapon` (`src/ttk/shortcuts.inc`)
handed M and Enter-on-medkit to `use_item` only in its last loop, behind the
ground lease (`movement_ready`, idle weapon, fire/aim/interact released). In
the swim states only weapon choices pass (`weapons_only`), so the medkit was
skipped. The original `800402d0` medkit branch (player+0x368 bit 15) only
moves charge (+0x36a) into health (+0x32) up to the type maximum, with no
mode check. Change: with a live medkit capacity (`800c2712+4*5` non-zero, the
instant heal), M / Enter on the medkit are taken before the lease in every
live gameplay state, once per command, only while health is above zero; the
last loop skips the medkit then. A zero capacity (the boot table, the
original equipment request with its animation) keeps the old lease, so the
D08Q flight refusal case still holds. Vanilla never reaches `select_weapon`
(`interaction_alive` is Modernized only). Whether Vanilla's own Select menu
heals in water was not run.

Evidence (`recomp/analysis/d08a34/probe.py`, ignored; offscreen, private copy
of the player's cards, slot 11 = file slot 10, `spawn medkit`, `hurt=2,N`,
`act=medkit` / `act=item_use`; player files hash-checked unchanged):

| Case | Before (HEAD build) | After |
| --- | --- | --- |
| Ground, M / Enter | heals / heals | heals / heals |
| Under water (mode 5), M / Enter | nothing / nothing | heals / heals |
| Surface (mode 4), M / Enter | nothing / nothing | heals / heals |

`ttk-controls-test` gains a D08A34 case (live capacity, fire held: zero
health refused, then one heal call with the pending bit cleared, no equipment
request); it, `ttk-input-test` and `ttk-inventory-test` pass. Accepted by the
user on 2026-11-03.

### D08A35 - Cowboy hat stays on under the Bio Mask

**Status: Done (revision 2 user-accepted 2026-11-04; [note 383](documentation/383-d08a35-cowboy-hat-bio-mask.md)). Dependencies: D08A24, D08A26, D08A23.** User
(2026-11-04, UI slot 3, third person, cowboy costume): "when you walk into the
toxic gas ... the biomask comes on but Duke loses his cowboy hat ... there is
no reason for this to happen we could absolutely rework the model so that the
cowboy hat stays on Duke's Head".

Cause (research, 2026-11-04): the mask is not drawn on top of the head. The
native equip at `8004e894..8004e94c` swaps the head joint's mesh pointer for
one shared masked-head mesh (`[800ddb18]+0x1030`), and night-vision goggles do
the same with `+0x10c4`; removal restores `[[800ddb14]+4]`. Those meshes are
the bare-headed default Duke, so the Old West costume (levels 1, 2, 3, 27)
loses its hat while either is worn.

Scope: in Modernized, a cowboy Duke wearing the Bio Mask (and the goggles,
same mechanism) keeps his hat in third person: the hat from the costume's own
head mesh sits on the masked head at the same place, lit and drawn like the
rest of Duke, with no clipping through the mask straps. Other costumes, first
person (head hidden), the native equip/drain/depletion and Vanilla stay
unchanged. Savestates and card saves keep working (no stray pointers saved).

Acceptance: slot 3 third person, walk into gas: mask on, hat on; leave or
deplete: original head and hat; goggles likewise; a non-cowboy level unchanged;
Vanilla unchanged; user judges the look in game.

Accepted 2026-11-04 at revision 2: a draw-time private head copy, see
[note 383](documentation/383-d08a35-cowboy-hat-bio-mask.md).

### D08A37 - Weapon switching while firing or jumping

**Status: Done (user-accepted 2026-11-07: "its working"; [note 409](documentation/409-d08a37-switch-while-firing.md)). Dependencies: D08A36, D10H4.** User,
2026-11-07, at the D08Z8 closeout: "i want a backlog ticket for that yes -
weapon switching while firing", "and weapon switching while jumping", "i
cant seem to switch specifically to the desert eagle". Today a number key, the wheel or ; / ' pressed
while fire is held does nothing until fire is released and the weapon settles.
EDuke32 switches at once while the fire key is still down (the new weapon then
fires if it is held).

Known (measured during D08Z8, no code changed): `select_weapon`
(`recomp/src/ttk/shortcuts.inc`) takes a weapon command only with the upper
body in an idle pose (5, 18, 19, 20, 29, 35, 39) and no pending change
(`+0x224 & 4`); held fire keeps every weapon on its fire tracks (Desert Eagle
16 / 17, Gatling 33 / 34, shotgun 23-26), so the command is dropped. The same
on flat ground with no climb, for every weapon. The original's weapon request
is `+0x3ba` = weapon with `+0x224 |= 4` (D08A33 / D08A36); whether it serves a
request from a fire track, and what a host change must stop on the old weapon
(the Gatling's loop `+0x260` and spin flag 0x800, D08Z8 revision 6), is to be
researched first.

Also measured at the closeout (private copy of slot 8, executable of the
D08Z8 closeout): in a jump without fire D08A36 works (shotgun -> Desert Eagle
with 2, Desert Eagle -> shotgun with 3, drawn before the landing); with fire
held the key does nothing in the air too. A key pressed while the weapon is
not ready (a landing 105, a draw, a fire track) is dropped, not queued: twice
a key had no effect (2 right at a landing; 4 just after an air switch's draw).
On the ground 3 -> 2, 4 -> 2 and the wheel to the Desert Eagle all worked;
the Desert Eagle report did not reproduce otherwise and needs the user's
exact situation (a save or the keys pressed).

To decide with the user: queue the command until the current shot completes
(the next fire-event keyframe) or cut the fire track at once; whether held fire
carries to the new weapon after its draw (EDuke32 style); air and swim
switching (D08A36, D08O5) the same way.

**Acceptance:** with fire held, a number key, the wheel and ; / ' change weapon
promptly on the ground, in the air and swimming; a key pressed while the
weapon is not ready is served as soon as it is (not lost); switching to the
Desert Eagle works in the user's case; the old weapon's sounds and
state ended cleanly (no Gatling loop), held fire continuing with the new
weapon as decided; no change without a command; Vanilla unchanged; the user's
playtest.

Work log, 2026-11-07 (Needs playtest): decided with the user, finish the
shot then switch, and held fire carries to the new weapon. Modernized weapon
commands queue (1.5 s) with the game's fire input released while they wait
and during the draw; post-shot tracks are cut to rest (DE 14, Gatling/Freezer
32, shotgun 26/23); the original holster/draw run; fire resumes at the ready
loop. Measured on private copies for every weapon group, a jump, a queued
press during a draw, release, same weapon and the wheel; Vanilla unchanged.
Not run live: swimming, jetpack flight. The Desert Eagle report did not
reproduce separately. Executable
`9adb09035646ff7a2c2745e5ad07cf7f3394cd796b458f4a77f8e853d9c55d79`, [note 409](documentation/409-d08a37-switch-while-firing.md).

### D08A38 - Energy weapon ammo icon, clean-edge alternate

**Status: Done (user-accepted 2026-11-08: "its perfect"; [note 407](documentation/407-d08a32-ammo-icons.md#d08a38-clean-edge-energy-atom)). Dependencies: D08A32.** User,
2026-11-08: "can we make an alternate animation for the energy weapon cell
... i essentially want the exact same thing, exact same rules we came up with
and everything, but none of the fuzzy edges ... dont disregard what we
currently have ... i highly recommend simply editing these frames to neaten
the edges, and those stray pixels etc." The D08A32 atom (8 frames, 60 ms,
shot, held-fire loop and settle on the rest pose) stays as chosen; the
alternate edits those same frames only at the edges. Acceptance: the user
picks the chosen atom or the alternate on the review artifact
(https://claude.ai/artifact/Aj8Kvjz9NjSpoqXFUUTCqu, private); if the
alternate wins, swap the eight HUD cells in game and keep Vanilla's belt.

### D08A39 - Mission item HUD text overlap

**Status: Done (user-accepted 2026-11-08: "lock it in and close it out. this is a winner"; [note 114](documentation/114-d08a5-mission-tracking.md#d08a39-label-and-count-never-overlap)). Dependencies: none known.**
User: "in level 3 there's only one item so when you use the greater than
less than signs to look through them the word mission and the one slash one
they're overlapped ... we need some kind of clause in how that looks so
that doesn't happen in that little UI element". Find the owner of the
selected-item label and count (D08A5 mission tracking, D08A6 selected
gadget HUD, notes 114 and 115) and lay the label and count out so they
never overlap, whatever the count's width, in every HUD size and aspect.
Acceptance: matched shots of the level 3 mission item before and after;
user review.

### D08A40 - Laser Gatling ammo icon

**Status: Done (user-accepted 2026-11-09: "lock it all in and close us out"; user picked candidate B: "type B"; user, 2026-11-09: "the laser gatling gun ammo icon needs to change, as it still shows the default ammunition belt"). Dependencies: D08A32, D08A38.**
With the upgrade (record 28) the bottom-right ammo box still shows the
regular Gatling's ammunition belt, because there has never been a custom
Laser Gatling icon (user: "we dont have a custom icon for the laser gatling
gun, and the job will be for us to create one"). Create a Laser Gatling ammo icon in the
D08A32 process and style (14x14 in a 16x16 canvas, original TTK-style art,
flat orthographic look, red-pink to match its laser bolts; animated, like the
energy weapon's frames - user: "i would like it animated of course"), add the
candidates to the ammo icon review artifact
(https://claude.ai/artifact/Aj8Kvjz9NjSpoqXFUUTCqu) for the user's choice,
then show it in game when the Gatling's upgrade is held. Check the other
upgraded records (HiTemp Flamethrower 27, Incendiary RPG 29) for the same
fallback and report. Acceptance: user-approved icon on the artifact, then
in game, framed and borderless HUD, all HUD scales; regular Gatling and
Vanilla unchanged.

**Work log (2026-11-09, awaiting the user's pick).** Three animated
candidates (8 frames like the energy atom: rest, then 1-7 at 60 ms per shot,
looping while held; at most 15 exact PSX colours; red-pink ramp around the
bolts' colour plus gunmetal): A laser energy cell on the diagonal with a
surge along its charge; B an industrial laser charge canister (inspired by
the user's Duke3D freezethrower ammo idea, drawn new) with a steady beam in
its window and a beam between two emitters, no arcs (user: "we want some
kind of more stable laser beam type vibe"); C a three-bolt volley with a
pulse running down each bolt. Frames in `recomp/assets/ui/ammo/`
(`laser-gatling-{a,b,c}-0..7.png` and `-15col` copies), on the ammo icon
artifact. In-game notes: the HUD picks the icon by the status bar weapon
byte `+0x3ba` (`ammo_live_view()`, `ammo_hud.inc`), which stays 7 with the
upgrade, so the upgraded Gatling needs its own entry keyed on inventory bit
8; its ammo counter is `+0x336` (not `+0x2c6+4*7`), which shot detection must
read; the HiTemp (27) and Incendiary RPG (29) show their base icons and
count at `+0x33a`/`+0x332`.

**Work log (2026-11-09, Needs playtest).** User: "type B". `ammo.json` lists
B's frames for record 28 (`laser-gatling-b-0..7`); `build_ammo_icons.py`
covers records 0-29 and regenerated `ammo_icons.h`; `test_ui_art.py` expects
record 28. `ammo_live_view()` (`ammo_hud.inc`) shows an upgraded weapon's
record icon when it has one (slot 7 with inventory bit 8 -> 28; the HiTemp
and Incendiary RPG keep their base icons, having none), and record 28 uses
the energy weapon's cycle. Its ammo is `+0x2c6+4*28` = `+0x336`, the
upgraded Gatling's own counter, so shots are counted. A and C moved to
local `research/d08a40-candidates/`. Offscreen check (level 0, borderless,
60 Hz): regular Gatling view -1 (the game's belt), upgraded rest on B frame
0, held fire cycled frames 1-7 over 9 shots and settled on 0. Manual updated.
`test_ui_art.py` (12) and `--check` pass. Needs the user's playtest (framed
HUD and other scales not checked separately; same draw path as D08A32).

### D08A42 - Laser Gatling ammo icon: candidate A in rich red

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"). Dependencies: D08A40.**
User, 2026-11-09: "change the laser gatling gun icon to our option A. can you
also just work over our option A again to make the colors more red, as this to
my eyes reads more on the pink scale unfortunately. I prefer the richer red
tones." Replace D08A40's chosen B (laser charge canister) with candidate A
(laser energy cell) on record 28, recoloured from its pink ramp to rich reds.
Acceptance: the user approves the red A on the ammo icon artifact
(https://claude.ai/artifact/Aj8Kvjz9NjSpoqXFUUTCqu) and in play; same
animation and HUD path as D08A40; regular Gatling and Vanilla unchanged.

**Work log (2026-11-09, Needs playtest).** A's seven pink colours (hue about
340) were mapped one to one onto a red ramp in exact PSX 5-bit colours
(57,8,16 -> 49,0,0; 107,8,41 -> 107,0,0; 165,16,57 -> 165,8,0; 222,41,90 ->
222,16,8; 255,90,140 -> 247,33,24; 255,165,198 -> 255,99,90; core 255,247,247
-> 255,239,239); gunmetal, shapes and the eight frames unchanged, still at
most 15 colours. A first pass leaned orange in the light tints and was pulled
back to pure red. `laser-gatling-a-0..7` (and `-15col`) are now in
`recomp/assets/ui/ammo/`, `ammo.json` record 28 lists them, B moved to local
`research/d08a40-candidates/`; `build_ammo_icons.py` regenerated
`ammo_icons.h` (`--check` passes), `test_ui_art.py` 12 OK. Rebuilt; offscreen
private run (`analysis/d08a40/hudlab.py`, level 0, borderless, 60 Hz, player
card hashes unchanged): regular Gatling shows the belt (view -1), upgraded
rests on frame 0, held fire cycled 1-7 over 9 shots and settled on 0; the red
sits next to the HUD's red counters. Before/after on the artifact. Not
checked: framed HUD and other scales separately (same draw path), 180 Hz.
