# Current status - 2026-10-04

## 2026-10-07 - D08U2 queued

- Backlog: D08U2, an enemy standing at a ladder top stops Duke climbing off;
  Duke should push it back. Test location: the user's UI slot 5.

## 2026-10-07 - D08O3 accepted; D08J2 and D08J3 queued

- User: "fully passed my playtest. feels great to play. i accept this job
  now as complete!!" [Note 120](120-d08o3-e-in-water.md).
- Backlog: D08J2 (poles and chains: A turns right, D turns left; make A
  left and D right; chain test location: the user's UI slot 3) and D08J3 (free camera while climbing ladders, poles
  and chains; needs investigation and usability testing).

## 2026-10-07 - D08O3 built: E in water keeps the weapon (Needs playtest)

- **What:** E while swimming still stows the weapon (needed to climb out at
  a ledge), but without a climb-out Duke draws it again automatically,
  without stopping his swimming. Land E and Vanilla unchanged.
- **Evidence:** private level 6 lab: walls, ledge, underwater, 11 swimming
  runs; regressions and suites pass. [Note 120](120-d08o3-e-in-water.md).

## 2026-10-07 - D08O2A accepted

- User: "this is actually rock solid ... I'm happy with it and its 100%
  playable." Known minor issue: one final shot at the end of a burst can
  fire with the weapon pointing down (seen in the user's capture).

## 2026-10-07 - D08O2A built: natural swim-fire pose v2 (Needs playtest)

- **What:** in Modernized, swimming underwater while firing now raises only
  Duke's arms and weapon toward the crosshair; the chest, head and legs keep
  the swim stroke (v1 stood the whole upper body up). Vanilla unchanged.
- **How:** new hook on the matrix composition `0x800b42ec` re-orients the
  two shoulders to their floating-fire pose relative to the view, at either
  model build; no aim bit or other game memory is written.
- **Evidence:** private level 6 lab, six weapons x four directions, 120 fps:
  gun hand near the floating-fire value, chest unchanged from plain
  swimming; floating, surface, ground and Vanilla unchanged; suites pass.
  [Note 119](119-d08o2a-natural-swim-fire-pose.md).

## 2026-10-07 - D08O2 accepted (v1); D08O2A queued

- User: "it works and i accept this as a v1". The more natural pose (torso
  stays in the stroke, arms raised to fire, head looking up) is Todo D08O2A.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D08O2 built: weapon forward while swimming and firing (Needs playtest)

- **What:** in Modernized, swimming underwater while firing turns Duke's
  upper body and weapon toward the crosshair, as when he fires floating still;
  the legs keep the stroke. Vanilla unchanged.
- **How:** new hook `0x800411b8` sets the original aim bit `0x100` for Duke's
  model build only (cleared at the neck's aim call), so the original arm-aim
  builder runs during the stroke and the D07A hook gives it the view.
- **Evidence:** private level 6 lab, six weapons x four directions, gun hand
  along the view equals the floating-fire pose; 60 and 120 fps; Vanilla,
  ground and surface unchanged; suites pass.
  [Note 118](118-d08o2-swim-weapon-forward.md).

## 2026-10-07 - D23E and D24A accepted

- User: "we can mark D23E and D24A both as accepted. we've proven these are
  now working significantly better". D23E (busy-scene stutter,
  [note 104](104-d23e-busy-scene-stutter.md)) and D24A (open assets,
  [note 112](112-d24a-open-assets.md)) are Accepted. No code changed.

## 2026-10-07 - D08A7 accepted: custom medkit gadget icon

- User: "amaazing work. i accept" (Done). The medkit shows the user's sprite
  in the switcher and the HUD box; health keeps the cross.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A7 built: custom medkit gadget icon (Needs playtest)

- **What:** in Modernized the medkit's inventory picture is the user's own
  sprite, full colour in the `[` / `]` strip and 15 colours in the D08A6 HUD
  box. The health box keeps the cross. Vanilla unchanged.
- **How:** the strip pack takes item 5 from `assets/ui/items/gadget-medkit.png`;
  the HUD box draws a medkit record whose 4bpp cell and palette sit in rows the
  disc's HUD sheet leaves empty (cell 960,205, palette 1008,206), loaded by a
  GP0 `A0` packet in the HUD ordering table each frame the box is drawn.
- **Evidence:** live VRAM survey (band empty in intro, title, pause, Select,
  six levels; the game fills x 960-991 from row 223 at run time), GL 4:3 /
  16:9, Software 4:3, 120 fps with savestate reload, Vanilla no upload.
  [Note 116](116-d08a7-medkit-icon.md).

## 2026-10-07 - D08A6 accepted: selected gadget on the HUD

- User: "i fully accept!" (Done). The gadget picked with `[` / `]` always has
  its original-style box over ammo, lit when on, dim when off; it follows the
  selection at every frame rate. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 playtest fix (Needs playtest)

- User saw the medkit box stay after hovering the jetpack, with both shown.
  Cause: at 120 fps the D17 worker redraws replay the status bar on a copy of
  guest memory, where the host selection was stale. The box now reads the
  original menu ID `0x800c3f94`, kept equal to the selection.
- `[` / `]` now hover in every gameplay state (flight, swimming, firing); the
  selected jetpack lights in place without sliding out of the ammo box.
- Verified from a copy of the user's savestate with the user's settings,
  three runs. [Note 115](115-d08a6-selected-gadget-hud.md).

## 2026-10-07 - D08A6 built: selected gadget on the HUD (Needs playtest)

- **What:** in Modernized the gadget chosen with `[` / `]` (the one Enter / U
  uses) always has a box at the original item slot over ammo: the HUD's own
  box, icon and red digits with its charge. Lit when on, dim digits when off,
  medkit (health cross) always lit. An unselected jetpack that is on moves
  one row up; Bio Mask / goggles keep their original place left of ammo.
- **How:** the status bar's own calls, from its entry hook after the D14 shift
  (`src/ttk/gadget_hud.inc`); dim digits are the original glyph packets at
  colour 0x31. The whole HUD layout is now saved and restored each draw.
- **Evidence:** private Xvfb runs: every selection, cycling, N / J / Enter,
  stacking, savestate load, level change, GL 4:3 / 16:9, Software 4:3, Vanilla
  unchanged; native tests and Python 120 OK.
- **Open:** user look confirmation; death / Continue and resize not run;
  Software 16:9 cuts the right HUD with or without this job (existing).
  [Note 115](115-d08a6-selected-gadget-hud.md).







## 2026-10-07 - D08A5 accepted: mission inventory on , / .

- User revision after the first try: mission items are their own **mission
  inventory** on `,` / `.` (previous / next, the < > keys), in the switcher's
  place with the item card at the top. `[` / `]` show gadgets only. Enter / U
  while it is open just close it, so an instinctive Enter no longer toggles
  the jetpack. The `\` binding is gone.
- Original strafe (Comma/Period, PS L1/R1) is unbound by default; profile
  schema 28 moves the keys and drops schema 27's `\` action.
- Verified in native tests, Python 120 OK and a private Xvfb run (browse,
  wrap, Enter swallow, `[` back to gadgets). User: "i fully accept!" (Done).
  Open for later: controller input for both inventories.
  [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - D08A5 built: mission item tracking (Needs playtest)

- **What:** design E is in the game (Modernized only). `[` / `]` show a mission
  row under the gadgets in every level with mission items; `\` browses it and
  shows the item card at the top (FOUND / NOT FOUND YET), clearing 2.5 s after
  the last press. The gadget frame is the tile0020 orange swap.
- **Playtest fixes:** `\` now works on UK keyboards (the key left of Z, SDL
  `NONUSBACKSLASH`); mission and gadget frame borders are even (pixel-centre
  stretch) and the panel outline has margin below it.
- **Profiles:** schema 27 adds the `\` binding to saved profiles (custom
  bindings kept); older launchers refuse the migrated file.
- **Data:** the original Select inventory's own list (name function
  `0x80087d4c`, flag `player+0x354+4*i` bit 0). Crystals are items 11-13 and
  level 6's skeleton keys items 7-8, not the ticket's guesses.
- **Evidence:** native and Python suites pass; private Xvfb runs show the row,
  card, timeout, a live jewel pickup (1/5), level travel and no row in level 8;
  Vanilla unchanged. `ttk-controls-test` fails on stale fixtures with or
  without the change.
- **Open:** user look confirmation in play; controller D-pad focus is not
  implemented (no controller path opens the switcher); the block sits about
  26 px higher than the mockup. [Note 114](114-d08a5-mission-tracking.md).

## 2026-10-07 - Mission item icons in; tracking design E locked; D08A5 queued

- **Icons:** the user's original 16x16 icons are tracked in
  `recomp/assets/ui/items/`, with `items.json` mapping all 15 mission items
  and steroids to a file, the game's object types and levels:
  - one keycard drawing serves all seven key-slot items;
  - Scrap of Paper also serves Torn Paper (user-confirmed);
  - every other item has its own drawing.
  `test_ui_art.py` checks coverage and format.
- **Keycards:** every key-slot key (types 153/154, named per level) is the same
  flat keycard object in the game, including medieval level 7. A pig carries
  and drops it there. Checked in RAM.
- **Tracking design:** the user compared four mockups (A panel, B HUD tracker,
  C pickup card, D switcher row) in the research page and iterated a fifth,
  **E**, which is locked: the mission row under the `]` switcher, `\`
  browsing, and a card at the top. Exact spec in **D08A5** on the board.
- **Next session: build D08A5.** Start from the board entry. The working
  mockup code (layout, palette swaps, card) is `drawE()` and `swapped()` in
  `recomp/analysis/d24a-fonts/picker.template.html` (local).
  - Switcher code: `src/ttk/inventory_hud.cpp`; packs via
    `tools/local/build_ttk_inv_icons.py`.
  - Item names and found state: the player's mission flags (`+876`..`+916`,
    see the job). Level crystals (items 11-13) are confirmed by D26F
    ([note 117](117-d26f-crystals.md)): taken from holders, not walk-over
    pickups; `spawn 761` (generic crystal) sets item 14 instead. D26F (accepted)
    adds `spawn 1761/2761/3761`: the real crystals as custom
    walk-over pickups (recipe in note 117).

## 2026-10-06 - D24A, D24B, D26E: open assets, TTK fonts, savestate menu, debug spawn

User, end of session: "im happy with all progress tonight. the document is
superb and an amazing piece of research."

- **D24A (Needs playtest: fresh-clone test).** No Duke Nukem 3D art or
  `research/` input remains:
  - TTK disc fonts (Big and Medium Italic, system 8x8) are built from the
    player's disc;
  - switcher digits are the CC0 3x5 Microfont;
  - the selection frame and crosshair are the user's original art;
  - HUD icons are built from the disc in every build;
  - messages and the console draw at half scale, in the "Console steel"
    palette with a navy shadow.
  The README credits PSXRecomp, Alexbeav's PS1 Recomps, recomp-ui and
  Microfont. Remaining: the fresh-clone test against the pushed commit.
  [Note 112](112-d24a-open-assets.md).
- **D24B (Accepted).** The F7 savestate menu uses the TTK fonts and disc art,
  with level names recorded per slot. [Note 113](113-d24b-savestate-menu.md).
- **D26E (Accepted).** The console has `spawn <item>` and `items`, now working
  from the real console and along the camera view; the half-sunk landing is
  the game's own drop. It also gained scrollback (512 lines, PgUp/PgDn/wheel),
  Up/Down command recall and `history` (D26C, accepted).
  [Note 111](111-d26e-debug-spawn.md).
- **Research page:** the local UI and font research page (fonts, palettes, UI
  sprites, BS stills, per-level mission items, design slots, savestate mockup)
  is `recomp/analysis/d24a-fonts/ttk-font-picker.html`. It is retail-derived
  and local only; `research/TTK-UI-and-Font-Research.html` links to it.
- **Next:** the user designs the open icons (15 mission items and keys, plus
  steroids) for the placeholders on that page.

## 2026-10-06 - D24A queued (public clone completeness)

A fresh-clone test of `main` built and played the same as the working copy,
but lacked the TTK inventory icons (disc-derived, not built by `build.py`),
the Duke message font and the green digits (Duke 3D art in `research/`). New
Todo D24A covers moving them into a proper build route. It notes that Duke 3D
art cannot be committed without a licence, and lists the options (derive from
the TTK disc, an original font, or user-supplied files).

## 2026-10-06 - D08Q6 accepted

User accepted the Modern jetpack fix ("i accept this as fixed! the test passes
and this feels great"): revision 5b, with height held, the original's speed,
flame and poses. Not committed.

## 2026-10-06 - D08Q6 revision 5b: original speed, flame and poses (Needs playtest)

Modern flight now moves at the original jetpack's top speed (about 10x the
5a cut) and shows the original flame (moving, climbing and hovering) and lean
poses. The original's flight inputs are fed again and their thrust is
cancelled so the host still owns the velocity. Height hold, landing, J, fire,
switching and fuel are verified live; Classic and Vanilla are unchanged; all
suites pass. Next: the user's playtest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 5: Modern flight model rewritten (Needs playtest)

Root cause found: the original root apply `8004ac08` throws away horizontal
motion whenever the vertical root is 0, so a precisely held height left WASD
dead unless Space or Ctrl was held. Modern flight was rebuilt from scratch.
The host owns the flight velocity: eased camera-relative WASD/arrows,
Space/Ctrl, height hold with the floor cap, and a +-1 vertical step while
moving. The original keeps collision, landing, cut-outs, firing and fuel
(drained at the original rate). Verified live with a copy of the user's
profile; all suites pass. Duke keeps the hover pose while moving. Next: the
user's playtest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 4: Right Shift dropped capture; idle creep (Needs playtest)

From the user's session log: pressing Right Shift sent the original Select,
which silently released Modernized capture mid-flight. The host stopped with
the hover lock on, so WASD barely moved until Space. Right Shift now counts as
Shift and never sends Select while captured (game-wide; on foot it runs like
Left Shift). The idle creep was a stalled horizontal coast, now stopped while
hovering. Both were reproduced and fixed live. Suites pass. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 revision 3: Shift and arrow keys in flight (Needs playtest)

Shift no longer reaches the original hover toggle in Modern flight; that
toggle was dropping Duke 128 units against the new hover. The arrow keys now
fly like WASD; before, they sent raw D-pad input that the hover pin throttled.
`dnkroz` keeps the jetpack the whole time god mode is on. Live: arrows fly,
Shift has no effect on height, and the largest step is 7 units. Suites pass.
Next: the user's retest. See [note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - `dnkroz` now gives the jetpack (D08Q4 follow-up)

`dnkroz` turning god mode on now also grants the jetpack (full fuel) when Duke
lacks one, at the user's request during the D08Q6 retest. Unlimited fuel for
an owned pack already worked. Cheats are still refused in mid-air. Verified
live and in the native suite. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08Q6 revision 2: smooth hover (Needs playtest)

The user found revision 1 jerky. Its hover twitched +17/-25 about every 6
updates, because phase 0 left one timestep of the steep part of the bob sine and
the error-driven slew echoed it. Revision 2 parks the bob at its flat peak and
slews the base gently. Hover is flat and stops settle over 3 updates, while the
altitude still holds. Suites pass. Next: the user's retest. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q6 Modern jetpack altitude hold (Needs playtest)

Modern flight no longer creeps upward. The altitude captured when Space or
Ctrl ends is held through flight, turning, looking up/down and hovering.
Level flight steers back to it, hover pins to it with the original bob off, and
it is capped at the original floor approach height over higher ground. The lab
circuit ends within 9 units (before: about 460 units of climb in six legs).
Ctrl landing, J fall, firing, weapon switching, Classic and Vanilla are
unchanged. Suites pass. Next: the user's LEVEL01 playtest. Not committed. See
[note 109](109-d08q6-jetpack-altitude-hold.md).

## 2026-10-06 - D08Q5 accepted; D08Q6 queued

User accepted weapon switching in jetpack flight ("excellent!! i accept.").
New Todo D08Q6: in Modern flight Duke's altitude creeps upward over long
circuits (LEVEL01 apartment/alley, `dnkroz`, much looking up); height should
stay fixed unless Space or Ctrl is used. Not committed.

## 2026-10-06 - D08Q5 weapon switching in jetpack flight (Needs playtest)

Number keys, the wheel, semicolon/apostrophe and X now switch weapons while
flying (Modern and Classic). The original mode-10 state already completes the
ground weapon request with the normal holster/draw; our shortcut layer had
stopped at the J check. Verified live with every weapon, firing after each
switch; swim rules, ground and Vanilla unchanged; suites pass. Dynamite's own
blast still knocks Duke out of flight (original). Next: the user's playtest.
Not committed. See [note 108](108-d08q5-jetpack-weapon-switch.md).

## 2026-10-06 - D08G3 and D08Q4 accepted; D08Q5 queued

User accepted `dnupgrade` and the Duke3D-style `dnkroz`. New Todo D08Q5:
weapon switching with number keys and the wheel while flying the jetpack.
Committed and pushed.

## 2026-10-06 - D08G3 `dnupgrade` and D08Q4 `dnkroz` (Needs playtest)

`dnupgrade` upgrades weapons 4/5/7/8/9/10 through the original upgrade bit and
the persistent mask `+0x85f` (which saves and pickups reapply), carrying ammo
to the Laser Gatling, Incendiary RPG and HiTemp Flamethrower records.
`dnkroz` now raises health to 100 (never lowering an Atomic Health surplus;
the original pickup already caps at 200) and keeps an owned jetpack full in
every scheme. Verified live in levels 6 and 12, Modern and Classic flight, a
savestate round trip and Vanilla, on private copies; suites pass. Next: the
user's playtest. Not committed. See
[note 38](38-debug-cheats.md).

## 2026-10-06 - D08O1 accepted; D08O2 queued

User accepted fire while swimming ("mechanically, it does exactly what it's
meant to"). Remaining issue, now Todo D08O2: the weapon points downwards while
Duke swims and fires; make it point forward. D08Q4 scope extended to Duke3D's
`dnkroz` (health to 100, unlimited jetpack, Atomic Health to 200). Committed
and pushed.

## 2026-10-06 - D08O1 fire while swimming (Needs playtest)

Underwater the original tests fire before the swim thrust, so holding fire
stopped Duke. Modernized now keeps him swimming while he fires (fire hidden
from the swim handler only, restored right after; new hooks `0x800455bc`,
`0x80055e80`, codegen hash unchanged). Swim shots use view aiming and the
crosshair; weapon keys work in water, limited to the weapons the original
allows there. Verified live in level 6 on private copies; Vanilla, ground and
jetpack firing unchanged; suites pass. Next: the user's playtest at their
location and a second level. Not committed. See
[note 107](107-d08o1-swim-fire.md).

## 2026-10-06 - Backlog: D08A4, D08S reopened, D08Q4, D08Z1, D11F; D08O1 started

User requests: D08A4 EDuke32-style portable steroids (stored item, R to use);
D08S reopened as the EDuke32 jetpack scheme (instant J on/off, midair, 61 s
of fuel); D08Q4 `dnkroz` gives unlimited jetpack fuel; D11F first person while
flying the jetpack; D08Z1 keep jump momentum when bumping a wall (Modernized
option for the future menu). All Todo. D08O1 (fire while swimming) selected
and In progress.

## 2026-10-06 - D23H accepted

The user played with several saves and accepted D23H: presents recover after
saves and hitches. Documentation, commit and push not yet authorized.

## 2026-10-06 - D23H: presents recover after saves and hitches (Needs playtest)

At 120 Hz and Match Display a D23E shed could never be undone, and any single
slow second (a savestate save is about 90 ms, an F7 menu visit more) shed. So
presents fell to 60 and then 40 for the rest of the session. Load shedding now
needs two behind seconds in a row and steps back up by itself in every
frame-rate mode. Offscreen: 120 presents/s through a 21-minute session with
20 saves, 8 loads and 13 simulated hitches. Awaiting the user's long play
session. See [note 106](106-d23h-present-rate-recovery.md).

## 2026-10-05 - D23F accepted: fast CPU timing is the Modernized default

The user accepted D23F; Modernized now uses fast CPU timing unless
`--cpu-timing accurate` is chosen. Next candidates: D23G (finish the fast
path) and D17S (auto frame-rate default).

## 2026-10-05 - D23F fast CPU timing: Needs playtest

`run.py --cpu-timing fast` (Modernized option, default accurate) runs the
recompiled game code on a lighter, calibrated timing model during gameplay.
At 150% and 120 Hz the western town (slots 9, 10) and slot 1 present every
refresh (120/s, was about 40) without overclock pauses; game speed and the
emulated CPU speed are unchanged. Not yet playtested. See
[note 105](105-d23f-fast-timing.md).

## 2026-10-05 - D23F added: faster timing model (big Todo)

The user prefers 100% CPU for now. At 150% and 120 Hz one emulation thread
cannot run the faster game and 120 redraws per second, so presents drop even
in light scenes (slot 1 75/s, slot 9 54/s); still images are identical. New
big Todo D23F: observers off in player sessions, then a design study of a
Disruptor-style fast timing model (cycle model is about 34% of the thread).
D23E stays Needs playtest at 100%. User authorized commit and push.

## 2026-10-05 - D23E western-town stutter: Needs playtest

Slots 9 and 10 are in the western town. At the player's `cpu_overclock 100`
the 16:9 view overloads the emulated CPU (game frames 4 fields with 5-6 steps;
Vanilla 4:3 slot 9 is a steady 3). At 120 Hz the default 150% could not hold:
redraws took 19% of the emulation thread, emulation fell behind and the
safety net paused the overclock 5 s at a time. Now the redraws shed load
first (presents every 2nd/3rd refresh) and the overclock stays. 150%/120 Hz:
slot 9 about 93% of frames at 2-3 fields (was about 30%), slot 10 98% at 3.
Candidate `18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`,
[note 104](104-d23e-busy-scene-stutter.md). Nothing committed. Next: the user
plays slots 9/10 at `--cpu-overclock 150` and judges turning and audio.

## 2026-10-05 - D11D and D12B accepted; closeout

User: "i can confirm that i accept both jobs as complete! document commit
push". First-person joints are found by part through the model's table, so
the cowboy levels (1, 2, 3, 27) get the right eye height, head hide and kick
leg. Regression baseline
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
No next job selected.

## 2026-10-05 - D11D and D12B: first-person joints found by part, Needs playtest

The cowboy costume (levels 1, 2, 3, 27) orders Duke's joints differently: the
neck is joint 15 there, not 9, and the right leg is joints 5-8, not 14-17. The
eye, head hide, weapon torso, arm hiding and kick leg now find joints through
the model's own part table (`desc+0x24`). The neck sits 691-705 above the toes
in all 21 levels, and the cowboy kick shows the jeans and brown boot.
Medieval, Roman and the first map are unchanged. Candidate
`0694b59dde72db57a536cdc3df4a20eff0866fdd0ca30dcad1da7d0f9ccfca4b`.
Nothing committed. Next: the user checks slot 8 (dancer height) and the kick
in a cowboy level. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 accepted; D08T2/D22C accepted; closeout

User: "accepted!! done, commit. great work". Accepted today: D22C (F10
recapture opts back in to automatic capture), D08T2 (object type range 1062:
Duke-symbol blocks push with RMB; hints on every contact) and D08U1 (slot-12
ladder: last-rung stop and let-go, mount blend finished after dropped frames).
Regression baseline
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
New Todo: D08T3 (free manual push/pull, suggested next), D08O1 (fire while
swimming); D17R gained the UI slot 2 medieval sky report. No next job
selected.

## 2026-10-05 - D08U1 cause found: mount blend cut short by dropped frames

The user's `[TTK ladder]` log showed the mount ending at y -9114 (offscreen
always -8901), and Duke frozen there while S looped the step poses. With
dropped frames the original 156 transfer ends before the host's 12-update
blend, leaving Duke above the climbing line. Reproduced with a longer
diagnostic blend (stuck at -9115 with the old code); the host now finishes
the blend while Duke rests on the ladder. Candidate
`4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08U1 not reproduced; robustness fixes and ladder log line

User: slot-12 ladder still glitches on S. Their log shows Duke never reached
the bottom (only 186-189 near the top, three retries). Not reproduced with a
private copy of their profile (first person, 120 fps) for held/tapped S, E
held, E taps or mouse look. Fixed: E-held swing at the bottom, rest-pose
latch, E's Cross blocking the let-go. New `[TTK ladder]` session-log line per
ladder pose change. Candidate
`071487b1f35bcbfd9bf4734c69fd06926fb1b1c11283c2c29dce80156428784a`.
Next: the user retries and describes the glitch; read their newest
`recomp/build-local/logs/session-*.log`.

## 2026-10-05 - D08T2 accepted; D08U1 slot-12 ladder bottom fixed, Needs playtest

User accepted D08T2 (type range, hints) except the slot-12 ladder ("he just
glitches out"). Cause: the ladder's last rung is ~990 above the floor, so the
original swings off sideways (156 -> 211) into a bottom-rung hang (207). Now
the player update asks the original's own probe `0x8007ded0`; at an open
bottom the descent flag is cleared so the original stops on the last rung,
and S lets go with the original Square (clean fall and landing). Hold or tap
S both work in third and first person; W climbs back; the sewer ladder keeps
its 185 step-off. Candidate
`cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.
Nothing committed. Next suggested: D08T3 (free push/pull).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 retest fixes; D08T3 added

User retest: hints never reappeared (D08T1 capped them per session; now every
fresh touch/grab/ladder top, at most every ~5 s); slot-12 ladder S glitch
(the ladder ends above the floor and the original hangs from its bottom rung;
S now lets go with the original Square and Duke drops to the floor). New Todo
D08T3: free manual push/pull while holding Grab, suggested as the next job.
D08T2 and D08U1 Needs playtest. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D08T2 Needs playtest (also fixes D08U1)

The medieval Duke-symbol block (UI slot 1) is object type 924; the shared
object type lookup in `push.inc` refused types `>= 512`, but the original has
1062. Fixed from the original allocation (`0x7428 / 28`). RMB now grabs and
pushes it in third and first person; the D08U1 slot-12 ladder (type 639) now
mounts with E and descends; dumpster and Vanilla unchanged; native tests pass.
114 more climbable types now reach the D08X mantle too. Candidate
`06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
Nothing committed. [Note 103](103-d08t2-object-type-range.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22C accepted; D08T2 in progress; D08O1 added

User: "I accept this work." D22C is Accepted; candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b` is the new
regression baseline. D08T2 selected (UI slot 1, file 00, medieval
Duke-symbol block). New Todo D08O1: fire weapons while swimming (UI slot 2,
file 01), modelled on the D08Q3 jetpack work. D17R gained the UI slot 2
medieval sky report ("strange behaviour with the sky directly ahead").

## 2026-10-05 - D22C control mode persists through level select, Needs playtest

Reproduced with real keys: after an F10 release and F10 recapture, every
later `level N` (11 and 12 tested) arrived with the mouse free, which looks
like Legacy controls until F10. F10 that captures now opts back in to
automatic capture (`recomp/src/ttk/pc_input.cpp`); an F10 release still opts
out. Not specific to Level 11. Native input/controls/aim/near tests pass.
Candidate `f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`.
Nothing committed. Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-05 - D22B accepted; game-wide playtest backlog

User: "Accepted." D22B is Accepted; executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the new
regression baseline (the candidate entry below is historical). The user's
`level N` playtest passed: first-person height and costume-aware kick in the
medieval and Roman/HOG HEAVEN eras, Level 9 armed rolls, general playability.
New Todo jobs, none started: D23C (medieval moat/Necro slowdown, profile
later), D23D (Level 9 strip-club-area slowdown, low priority), D08U1 (player
slot 12 ladder, E cannot descend; file 11 SHA-256 `1608ee9c...`), D08T2
(Duke-symbol pushable blocks, modern RMB grab game-wide), D22C (Level 11
started in Legacy controls), D26B (console leaves first person), D26C (console
history), D26D (authoritative level-select numbering; level numbers in these
reports are console indices until D26D). D17R now also reproduces in HOG
HEAVEN. Documentation only. No next job selected.

## 2026-10-04 - D22B every level candidate, Needs playtest

Modernized controls and the selected first- or third-person view now work in
all 21 levels `levels` lists. The lease authenticates each level by a body
derived from the owned disc (`recomp/tools/local/level_overlay_guards.py`,
`--check` keeps `control_guards.inc` honest). Dodge rolls and steep-slope
slides keep the mouse camera and view; the original's own back-steps and
strafes are taken over by the lease; unowned jumps keep the camera in third
person too; landing poses 94/95/106 keep it. Statistics screen, savestates,
pause and level select return Modernized control by themselves. Offscreen:
zero identity refusals in every level; LEVEL00 slots and Vanilla unchanged.
Not covered: first person while swimming (new D11E), real death/Continue and
natural level exits outside LEVEL00; occasional jump camera gaps seen in two
runs are unexplained. Candidate
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017`; codegen
hash unchanged, savestates load. Nothing committed. See
[note 102](102-d22b-every-level.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D26A accepted; next D22B

User: "excellent! mark as accepted." D26A is Accepted; executable
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851` is the new
regression baseline. The candidate entry below is historical. **Next: D22B,
Todo - next** (not started): Modernized controls and first person in every
level. Today only LEVEL00 and LEVEL01 authenticate; the other 19 selectable
levels fall back to original controls and third person. Use `levels` /
`level N` to reach them. The user authorized documentation, commit and push.

## 2026-10-04 - D26A debug level select candidate

Backtick console: `levels` lists the 21 levels the original title-screen
level-select cheat offers, with the game's own names (0 TIME TO KILL, 1-3, 5-12,
21-26 challenge stages, 27-29 bosses). `level N` ends the current level with the
pause menu's restart code and loads level N through the original mode 1 init.
The level index changes only inside that init; changing it earlier crashed Old
West pairs. Offscreen: all 21 levels load in one session with their own
overlay; savestates, Vanilla and the pause/title refusals work. Modernized
controls stay limited to LEVEL00/LEVEL01 (D22B). Candidate
`58f4fb3532f384edb74291b398b992c066364912a40edd07cfe04f3560804851`; codegen hash
unchanged. **Needs playtest:** in a game, backtick, `levels`, `level 2`,
savestate there. Nothing committed. See
[note 101](101-d26a-level-select.md).
Launch: `python3 recomp/tools/local/run.py`.

## 2026-10-04 - D22A accepted; new Level 2 jobs

User playtest: "I can confirm and accept D22A as working." The portal into
LEVEL01 (the user's Level 2) works and the level is completely playable with
Modernized controls. D22A is Accepted; executable
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0` is the new
regression baseline. The candidate entry below is historical. See
[note 100](100-d22a-portal-level-identity.md).

New Todo jobs (reproductions: player UI slots 8/9 = files 07/08, LEVEL01;
hashes in the board entries): D11D first-person eye height (slot 8), D12B
costume-aware first-person kick (slots 8/9), D17R sky black at the edges when
looking up (slot 9), D26A debug level-select panel, D22B Modernized controls
owning every level, state and transition end to end. The user's rule: fix the
shared system, not the level. The user plans to survey the rest of the game
with D26A. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D22A portal level identity candidate, Needs playtest

Cause: the Modernized lease authenticated only the LEVEL00 overlay body; the
first map's portal loads LEVEL01.OVR over the same base `0x800ca968`, so the
lease failed closed and original controls/third person returned. LEVEL01 is
now an authenticated level overlay (tag-keyed `level_overlays[]` in
`control_guards.inc`, owned-disc digests, same scratch-tail rule as LEVEL00);
unknown levels still fail closed and the apartment conveniences require
LEVEL00. Offscreen: movement, mouse look, jump, fire, pause/resume and first-
and third-person all work after the portal, repeated in one session, with zero
identity refusals; the 12 LEVEL00 slots and Vanilla are unchanged. Tests pass;
codegen hash unchanged. Candidate
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0`.
**Needs playtest:** UI slot 7 -> portal -> continue, then play the Old West
level with Modernized controls in first and third person. Nothing committed.
See [note 100](100-d22a-portal-level-identity.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q accepted (Done)

User, after extended play: "the whole job is 100% accepted", and
praised how cleanly the game now plays. D17Q (residual wall
flicker from per-triangle UV seams) is Done, user-accepted. Accepted
executable `a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960` is the new regression baseline. The candidate entry below is
historical. See [note 99](99-d17q-uv-seams.md) for cause, fix and limits. The
user authorized documentation, commit and push. No next job selected or
started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17Q UV seam candidate, Needs playtest

The diagonal line still visible after D17N (UI 12 panel at the crosshair on
load, flickering on W/S) is a one-texel UV seam, not geometry. The GL/VK
backends applied the 2D mirrored-sprite UV bump (`gpu_uv.h`) per triangle.
A world-wall triangle with an exactly vertical integer edge qualified while
its partner did not, so the two halves of one polygon sampled one texel
apart, and movement toggled which halves qualified. Traced positions, integer
SZ and UVs form one consistent perspective map (about 0.03 texel); an offline
exact render is seamless. Fix: perspective-corrected 3D triangles use full UV
limits and no sprite bump (`PSX_UV_3D_LEGACY=1` compares). Original textures
and Vanilla are pixel-identical; UI 12 and the UI 11 stage canopy lose their
seams. The 12-slot 120 Hz sweep and 60/180 Hz runs had zero replay misses.
Tests pass and the codegen hash is unchanged, so savestates load. Candidate
`a6f8c8cbaa3329028c5aed15fd26ca6a2dc45975e0482be8c17723d4af7cb960`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with
texture precision `corrected`. Nothing committed. See
[note 99](99-d17q-uv-seams.md).
Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N accepted; D17Q added

User playtest: "The result is excellent." D17N (world subdivision zigzag; the
user's message called it D17P) is Accepted. Accepted executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f` is the new
regression baseline. Cause, solution and fallback behaviour: [note 98](98-d17n-world-subdivision.md).
The candidate entry below is historical. New Todo job D17Q: residual subtle
wall/surface flicker as independent Corrected-renderer stability polish (see
the job board; references D17N, D17P, D15 and R01). D18D (music after Continue)
remains Todo. The user authorized documentation, commit and push. No next job
selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17N world subdivision candidate, Needs playtest

The vibrating diagonal lines on walls (UI slot 12, and generally) come from the
original world renderer: every polygon within `0x2000` units goes through a
screen-space subdivision (`0x80012960` triangles, `0x8001205c` quads) whose
midpoints are integer screen averages with rounded UVs and no projection data.
D15's Corrected textures therefore skipped those pieces and they were drawn
affine; the bends follow piece diagonals and move with the eye. With Corrected
texture precision, world polygons whose corners all have exact projections and
fit the GPU primitive limit are now drawn whole with exact perspective (hooks
`0x800114EC`/`0x8001160C`; `DNTTK_WORLD_SUBDIVISION=1` compares the old path).
Original textures, Vanilla and software are unchanged.

Private offscreen evidence: straight panel borders across 120 Hz W/S sequences;
12-slot sweep, exact-projection share 0.29-0.43 to 0.73-1.00, lower packet use,
game rate unchanged, zero budget hits or replay misses; 60/180 Hz checked.
Tests pass; codegen hash unchanged, so savestates load. Candidate executable
`12f42ccf0962c67791e467c208e3409b9dbc5fded9e991da7e7ce86919b1c31f`.
**Needs playtest:** walk forwards/backwards in UI 12 and elsewhere with texture
precision `corrected`; recheck accepted D15/D17 cases. Nothing committed.
New backlog job D18D (music silent after death and Continue) was added, not
started. See [note 98](98-d17n-world-subdivision.md).

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P accepted (Done)

User: "i fully accept this fix." D17P is Done, user-accepted: the Modernized
Draw distance option (`extended` default) removes the UI slot 8 subway bands.
Accepted executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
The user authorized documentation, commit and push. The candidate entry below
is historical. See [note 97](97-d17p-distant-bands.md) for cause, evidence and
limits. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17P distant bands candidate, Needs playtest

The UI slot 8 subway bands are three original rendering limits, present in
Vanilla too: the far cut-off (rooms past the render limit are never drawn and
the fog fade before it spans a pixel or two), integer portal rectangles a pixel
or two short of distant openings (whole ceiling strips dropped), and integer
NCLIP on one-pixel distant faces. New Modernized **Draw distance** option,
`extended` by default (`original` restores the limits; Vanilla always original):
render-only limits doubled, portal rectangles 2 native pixels wider, exact PGXP
NCLIP signs when geometry or texture precision is Corrected. Profile schema 24,
`run.py --draw-distance original|extended`, settings menu **D**.

Private offscreen evidence: matched 4x views show a continuous ceiling and the
real corridor end; all 12 private slots keep their game rate with ring use at
most 50% and no budget hits or replay misses; 120 Hz cost unchanged; Vanilla
untouched; existing savestates still load (codegen hash unchanged). Tests pass.
Candidate executable `5130824841bfc816e09243d47bb3ecd3635bd2fb3d2519ed06e49b511f75ae50`.
**Needs playtest:** walk forwards in UI slot 8 and look around; recheck the
accepted D15 cases. Nothing committed. See
[note 97](97-d17p-distant-bands.md).

Launch: `python3 recomp/tools/local/run.py` (the first launch migrates saved
settings to schema 24 with Draw distance `extended` and keeps a recovery copy).


## 2026-10-04 - D15 Accepted; new subway slot 8 is D17P

The user accepts the final playtest: slot 5 left-side black/missing areas no
longer reproduce, slot 6 closet and surrounding furniture are stable, and the
previous slot 8 door remains opaque. Geometry stability, floor seams and idle
polish were already accepted. D15 is **Accepted** at the primary 120 Hz target.
See [implementation, tests, user evidence and limits](96-d15-accepted-precision.md).

The user explicitly authorized documentation, implementation commit and push
on the current branch. Preserve the title changes and exported framework patch;
Vanilla remains available and precision is optional in Modernized/OpenGL.
Accepted executable: `35756df57b9a6ddd31ee0dabdffb51876faf7a3b0c05effb78be285dfd9dc9a7`.

**Immediate next job: D17P**, selected by the user. They replaced UI slot 8
with a subway corridor state: walk forwards and watch multiple horizontal
black bands in distant geometry/textures. Treat "possible culling" as a
hypothesis, compare rendering paths, and preserve the accepted D15 baseline.
This is separate from the accepted previous slot 8 opacity case. The new
state identity, private-copy intake and acceptance criteria are on the
[canonical board](../MODERNIZATION_JOBS.md#d17p---distant-horizontal-black-bands-in-the-new-subway-slot-8).
No technical cause is established by intake alone.

Launch: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L accepted (Done)

User: "first off, i accept this as complete!" D17L is Done, user-accepted.
Preserve implementation `1e6fb0a` and the evidence in
[note 95](95-d17l-tabletop-depth.md). Earlier Needs playtest and unresolved
candidate entries below are historical. Busy-scene 180 FPS limitations remain;
this acceptance does not close other jobs or establish full campaign coverage.

The user clarified the workflow: implement and test, obtain acceptance, then
wait for an explicit instruction to document, commit and push. They authorized
this closeout in the next message. AGENTS.md records that approval requirement;
background development testing remains authorized. This closeout changes only
documentation and instructions, with no new build, launch or gameplay test.
No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17L tabletop depth fix, Needs playtest

Runtime primitive/provenance capture identifies the remaining cup cut-off:
its depth-tested triangles were followed by a native tabletop quad that
painted over them without depth testing. The fix gives eligible opaque upper
faces of authenticated static eight-corner boxes depth while retaining native
integer positions, affine textures, quad topology, culling and OT order.
The broad upper-surface experiment was rejected on measured cost.

Approach/retreat captures at 60/120/180 retain the full cup. Accepted shelf,
closets, blood and subway routes remain intact; replay restore oracle passes
312 checks, Vanilla reports no host depth/replay workers, and Python runs 102
tests: 100 pass, two conditional skips. Primary 120 samples have zero output
underruns; 180 remains below target in these busy scenes with a small added
candidate deficit. User visual/audio acceptance is still required.

The user replaced the no-launch restriction: background/offscreen development
tests are authorized; foreground windows are reserved for necessary visual
verification. AGENTS.md records this. Player files are unchanged and test
games are closed. No Disruptor code was copied; no framework/generated code
changed. See [cause, fix and evidence](95-d17l-tabletop-depth.md).
Launch: `python3 recomp/tools/local/run.py`; F7 UI 11, F10 if capture is needed.


## 2026-10-04 - D17L selected; offline primitive diagnostics ready

D17L is In progress for the bounded cup diagnosis. Added opt-in title-owned
per-instance/per-face tracing and offline observational-equivalence checks.
No Disruptor code copied. The private UI 11 hash matches note 89. Native packet
and controls fixtures pass; the player build succeeds. No new game run or
cause/fix is claimed. Isolated diagnostic launch authorization was requested
under AGENTS.md and is pending. See [scope, evidence and continuation](94-d17l-primitive-diagnostics.md).
R01, D08Q3 and D17O remain Done; accepted rendering baselines are unchanged.


## 2026-10-04 - R01 DisruptorRecomp reference research complete

[Comparative review and pinned source index](93-disruptor-reference-research.md).
R01 is Done as a research-only job. Retain clean, ignored
`research/DisruptorRecomp` at `408f214d3109dbc6cbdded7edd128cbf8de6466a`,
framework `193a60b805e1eaa853129d6ccf63022440d4b143`; project patches were read,
not applied. The review records acquisition, architecture, all requested systems,
source/functions, applicability, licensing/provenance and ranked experiments.

Recommended next choices: a primitive/provenance census for D17L/M/N, D15's
optional Original/Corrected geometry with independent texture correction, and
measured CPU-accounting overhead. No experiment selected or started. Disruptor's
image crossfade does not replace D17; its gap rims do not establish fixes for
TTK culling/depth; TTK already has gameplay-scoped CPU overclock. Preserve
accepted D17/D17B/D17O and the primary 120 FPS baseline.

User confirms research-only reuse: learn techniques and write purpose-built TTK
code, with no 1:1 copying or mechanical translation.

The included license text differs from standard PolyForm Noncommercial despite
its label; clarify before future code adaptation. No source imported. Ignore
and clean-reference checks passed. Static research only: no build, launch,
gameplay/renderer edit or player-file write; no new gameplay evidence claimed.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O accepted (Done)

User: "amazing work!!! accepted. well done. document. commit. push".
D17O is Done, user-accepted. Preserve implementation `25734d1` and player binary
`8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
The preceding implementation's Needs playtest status below is historical.

[Architecture, cause, verification and remaining limits](92-d17o-sky-intake.md).
Original sky geometry/coarse animation and busy-club 180 FPS limitations remain;
this acceptance does not establish full campaign coverage or close other jobs.
Documentation-only acceptance closeout: no new gameplay test, build, launch or
player-file write. No next job selected or started.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D17O sky camera-relative replay repair (Needs playtest)

Verified original sky architecture and a high-refresh defect: the three
camera-relative sky matrices were recorded under the last world object, then
substituted with older world-space matrices. A narrow, resident-code-authenticated
exemption retains the original sky transforms, art and timer. In the sampled
turn sweep, camera-origin displacement fell from up to 1075.397 units to zero.
No framework, generated C, flight-control, clipping or media changes.

Private captures cover 60/120/180, ground first person, third-person flight,
multiple headings/pitches and a second street location. Near/input/aim/controls/
inventory suites and final player/shard checks pass. Modern/Classic pistol fire,
reticle eligibility, actual pack-off, landing/first-person return and reflight
pass. 1106 worker surface checks had zero mismatch. All 28 player file hashes
and mtimes match intake. Existing runtime patch stack verified already applied.

Primary 120 FPS routes were clean. Busy club 180 FPS still repeats frames and
can underrun audio, reproduced with the previous sky path too; do not claim
universal 180 FPS stability. Original coarse cloud motion and high-pitch geometry
limits remain. User sky appearance confirmation is required; no next job started.

[Full architecture, provenance, diagnosis, A/B evidence and limits](92-d17o-sky-intake.md).
Build SHA-256: `8b7253ce043395795e6c8002d1e0079f549e756b9df34cce77332e2722af588d`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10 and look/turn/move;
F10 captures Modernized input if needed. The fix applies to high-refresh redraws.


## 2026-10-04 - D08Q3 accepted; next D17O sky appearance

User accepts D08Q3: "i accept this! great work." Done; preserve `f284dc1`.
The older Needs playtest entry below is historical. No new gameplay test was
performed for this acceptance closeout.

**Next: D17O, Todo - next.** Load UI slot 10 (file 09), look up at the sky and
investigate its strange/glitchy movement. This is not jetpack-specific. First
establish how the original game renders/animates sky, then diagnose and test a
bounded stabilization. The user explicitly requires substantial documentation
of architecture, cause, evidence, implementation and remaining limitations.
[Canonical job](../MODERNIZATION_JOBS.md#d17o---unstable-sky-appearance-when-looking-up-slot-10).

[Detailed intake/plan and exact save identity](92-d17o-sky-intake.md).
Immutable private snapshot: `recomp/analysis/d17o-sky-intake-20261004/cards-intake`;
28 files with adjacent hash/mtime manifest. Current slot 10 differs from the
older jetpack fixture. Use a separate writable copy and private preferences.
Do not assume a skybox, parallax defect or high-refresh cause from the report.
Preserve Vanilla, accepted visual work and D08Q2/Q3.

This checkpoint only records acceptance/backlog and copies intake data. Sky
investigation has not started; no code edit, build, game launch or player-file
write. Commit/push this handoff before the user clears context.
Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q3 implemented, Needs playtest

Flight now admits Modernized view aiming, weapon presentation, energy-beam
completion and the enabled crosshair through the existing guarded jetpack lease.
Ground movement eligibility, flight physics, Vanilla and D08Q2 are preserved.

Private Modern/Classic routes verify six weapon types, mouse yaw/pitch, firing
while moving/climbing/descending, I toggle, J-off, fuel exhaustion, landing and
first-person fallback/return. Native aiming/controls/input/inventory suites and
the player build pass. All 28 player file hashes/mtimes match intake.
[Cause, verification and coverage limits](91-d08q3-jetpack-aim.md).

User confirmation remains. Select weapons before takeoff; flight switching is
unchanged. Thrown/upgraded variants are not all separately live-validated.
No next job started. Build SHA-256:
`5b4add2a896a3ab5ae16e5e02fb8163d551ef3b4a9eaad954ebd6c3837500327`.

Launch: `python3 recomp/tools/local/run.py`. F10 captures Modernized input if
needed. Check the reported jetpack aiming/crosshair in both flight schemes.


## 2026-10-04 - D08Q2 accepted; next D08Q3 jetpack aiming/crosshair

User confirms D08Q2: "confirmed fixed! accepted." Marked Done; preserve the
accepted recovery from `2f02967`. Earlier Needs playtest and immediate-next
D08Q2 entries below are historical. No additional gameplay tests in this closeout;
the natural map-pickup replay remains outside the recorded automated evidence.

**Next: D08Q3, Todo - next.** User cannot aim to shoot while using the jetpack,
and the crosshair is missing. Cause, affected weapons/scheme and exact location
are unverified. Read the [canonical job](../MODERNIZATION_JOBS.md#d08q3---jetpack-weapon-aiming-and-missing-crosshair)
for scope/acceptance. Investigate view aiming and reticle eligibility separately;
preserve Modern/Classic flight, ground aiming, existing flight camera transitions,
Vanilla and the accepted D08Q2 fix. The old slot-10 private snapshot is a regression
fixture, not yet a confirmed reproduction of this new issue.

Documentation only this session: D08Q3 has not started; no code edits, build or
launch. Launch when wanted: `python3 recomp/tools/local/run.py`.


## 2026-10-04 - D08Q2 repaired, Needs playtest

The reported UI slot 10 is repaired automatically in captured Modernized ground
play: original jetpack pending `0x8001` becomes owned/off `0x1`, with full fuel
unchanged. Death during deployment can strand pending across Continue; the
original `dnstuff` grant preserves it. A private extended-deployment timing
fixture plus real RPG death/Continue reproduced the baseline failure and verifies
the repair. The user's exact original timing is not established.

Only the inactive closed/off endpoint is repaired. Picker/J, Modern and Classic
flight, fuel rejection/depletion and private save/load pass. Native controls,
input, aiming and inventory suites and the player build pass. All 28 player
card/state hashes/mtimes still match intake. Framework/renderer unchanged.
Natural map-pickup replay and user acceptance remain; no next job selected.
See [D08Q2 evidence](90-d08q2-jetpack-continue.md).

Build: `c8f979d50bd502ded05294fde2e55b59c90f132f386d21a89865538ca8836a41`.
Launch: `python3 recomp/tools/local/run.py`. Load UI slot 10, try brackets,
J and Space; F10 captures Modernized input if needed. No repeat cheat required.


## 2026-10-04 - Immediate next investigation: D08Q2 jetpack unavailable

The user will clear context after this checkpoint. **Start next with D08Q2**,
not another D17 job. Status: Todo - immediate next; investigation has not
started. User sequence: died -> Continue -> returned to gameplay -> entered
`dnstuff` -> jetpack cannot be used, and the inventory picker skips it.
Failing state is **UI save slot 10 (file 09)**.

Private snapshot: `recomp/analysis/d08q2-jetpack-20261004/cards-reported`;
manifest alongside it. Slot hash:
`161253aa41f4f2c407f57ef7cff1371ba7889c4bdeba793183f9045b6184e18e`.
Use private cards/settings. The saved result may not reproduce the preceding
transition by itself; compare working inventory and the full death/Continue/
cheat sequence. Trace ownership, fuel, picker eligibility and activation
alongside Continue/reset and cheat grants. Cause is unknown; picker exclusion
is evidence to inspect inventory state, not proof of a particular defect.
See the canonical [D08Q2 job](../MODERNIZATION_JOBS.md#d08q2---jetpack-unavailable-after-death-continue-and-dnstuff).

This checkpoint only records the report and preserves a copy. No gameplay
code, build, launch or player-file changes. D17L stays open; D17M/N remain
queued; D17C/D/E/F/K accepted behavior must be preserved.


## 2026-10-04 - D17E/K accepted; D17L open; new D17M/N backlog

The user accepts D17E subway peripheral visibility and D17K stable ground
blood. Preserve their current behavior from `72085d7`. D17C remains Done;
D17D/F remain Accepted. D17L returns to Todo: the isolated cup still flickers,
while nearby bar cups/bowl remain stable. It does not merely await acceptance.

Subsequent user retest on 2026-10-04 reports no visible change. D17L remains
open (Todo); no new fix is claimed. This checkpoint documents that retest
only. Further implementation is deferred.

Offline inspection of current UI 11 confirms polygonal table/cup meshes and
shared cup/bowl prototypes across table and bar placements. The residual
instance-level failure is not yet explained; the next L pass must trace the
specific cup and occluding surface across good/bad frames. No gameplay changes
or game launch occurred in this follow-up.

New separate Todo jobs: D17M, UI 9 shotgun ammo visible through its platform
while climbing the ladder; D17N, UI 12 jagged diagonal/vibrating walls during
forward/back movement. Causes remain unclassified. Slots 9/12 have changed;
preserve older private shelf/blood states for accepted regression checks.
[User results, current save hashes and mesh inspection](89-d17-playtest-followup.md).

Launch from this workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17C accepted; D17E/K/L visual candidate

D17C is Done on the user's acceptance of the current camera stability and look.
D17E/K/L were selected together and are now Needs playtest. D17D/F remain
accepted; no other job started.

- D17E: no missing peripheral subway wall reproduced on full-width wall/edge
  and stair routes at 60/120/180. Awaiting player closure, not a claimed new
  visibility fix.
- D17K: enhanced floor pieces now retain the original source polygon's
  farthest-corner ordering key, preserving native blood in front of the floor.
- D17L: compact static props (up to 256 local units per axis, authenticated
  caller) keep depth across the near radius. Opaque faces share an ordering
  key to avoid excessive draw-state changes. Actors and larger distant
  scenery keep the previous takeover boundary; Vanilla remains available.

[Causes, private save identities, implementation and verification](88-d17e-k-l-visuals.md).
Current UI 11 is the table report, UI 12 the blood report; older closet states
were checked separately. The accepted shelf/closets and camera are preserved.
Final 120-target samples have zero audio underruns, worker failures or new
mesh-budget fallbacks. At 180, final club samples are about 178-179 distinct
images/s, with a small table-route output underrun still measured; do not claim
zero-cost 180 FPS or actual-device audio acceptance. User should also listen
for crackle at their usual refresh setting.

New owned-EXE packet/guard tests and existing near-plane math pass. Python:
98 passed, two skipped. Original player files/settings were not written.
Framework patch and generated game code are unchanged.
Candidate SHA-256: `151ea6aab6957eeb2c3da5cf990700d566a227eb723eb829d13600146c098520`.

Historical candidate next action (superseded by the 2026-10-04 report above):
user playtest of UI slots 3, 11 and 12, starting at 120
and then their usual refresh setting. F10 captures Modernized input if needed.
Launch from the workspace: `python3 recomp/tools/local/run.py`.


## 2026-10-03 - D17D/F accepted; next-round visual and portal bugs

The user explicitly accepts the UI 9 prop, UI 11 closet and closet inside
the strip club as fixed. D17D and the related closet job D17F are Accepted.
Preserve the source-plane depth correction and bounded static-prop contact
tolerance from `b88d0e5`; do not reopen these jobs for the separate new reports.
Accepted binary: `7c3b7600e145e4d0f0c7d8899817ecb8afaba6e7eabfd0d4f0eaf52930ff658c`.
[Cause, implementation, tests and user acceptance](87-d17d-contact-depth.md).

Next-round backlog (all Todo, none started):

- D17K: UI save slot 12, blood on the ground directly in front of Duke.
  User describes coplanar blood; exact renderer/placement cause unverified.
- D17L: UI save slot 11, props sitting on the table directly ahead. Walk
  forwards/backwards: props get cut off and reappear as Duke moves closer.
  This is distinct from the accepted closet in that save.
- D22A: UI save slot 7 -> walk into the portal -> start the next level.
  Duke loses all Modernized controls and first-person view. Cause unverified;
  inspect transition/overlay coverage and control/view state without bypassing
  identity guards. Preserve the selected mode and camera when gameplay resumes.

Read the new board entries for scope and acceptance. Verify current save hashes
and make dated private copies before testing; slot numbers alone do not prove
that an older `cards-user8` snapshot is still the current reproduction. Use
background/offscreen tests and preserve the accepted UI 9/11/club fixes.
The user will clear context and choose the next job; no implementation starts
from this handoff alone. This closeout changed documentation only, with no
build, game launch or player-data changes. Launch: `python3 recomp/tools/local/run.py`.


**D17A ACCEPTED. D17B ACCEPTED (2026-10-03).** The user completed natural
first-level play at 120 and 180 FPS and explicitly closed the parent jobs.
120 FPS is the primary quality/regression baseline; 180+ excellent high-refresh
support, 240+ robustness/compatibility, Unlimited stress/debug. No technical
ceiling or gameplay-speed change. Residuals are D17D-J and D18C, not reasons
to keep the accepted parent jobs open. No follow-up implementation started.

Accepted binary: `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
Preserve mouse response, opening/club playability, apartment/culling fixes,
FMVs/audio, FPS reporting, idle stability and intentional PS1 motion.
[Acceptance, source baseline and reproduction ledger](86-d17-acceptance-and-regression-baseline.md).

The repository now tracks the actual `recomp/` implementation and complete
framework patch with pinned dependencies. The prior documentation-only exclusion
was explicitly revoked by the user. ROMs/generated game code/builds/saves/research
remain ignored. Original disc dump moved intact under `game/`; prepared runtime
copy retained. Older statuses and results below are historical and superseded.


**Latest: accepted playability baseline preserved; bounded polish investigation.**
The user reports the opening, strip club and apartment are now smooth, enjoyable
and substantially more stable. This acceptance applies to build `28554051...`.
The follow-up disables unnecessary per-batch GPU timing queries in ordinary
launches and adds opt-in pose-readiness tracing. No interpolation, culling,
physics, audio-buffer or mouse-sampling change is shipped.

Slot 12 Shift+W/S still exhausts usable interpolation history while presentation
intervals remain mostly regular; complete geometry arrives too late for some
planned movement frames. Larger buffering and stronger phase correction were
rejected. Slot 10 can starve audio production under the scripted stress route;
slot 1's crackle did not reproduce as an underrun. These remain open polish
items. Opening/dancer mouse regressions retain about 179-180 images/s, zero
underruns and no camera turn over one degree. D17D now holds isolated visual
artifacts (slot 9 also reproduces at 60 Hz). D17A/B remain Needs playtest.
See [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md).
Current executable SHA-256:
`1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.


**Latest: save-driven camera stability candidate, D17A/B Needs playtest.**
This supersedes note 83's performance conclusions. Five-field dancer updates
were incorrectly resetting camera yaw; expired replay jobs also hid their
cost from scheduling. Those causes are fixed, compatible opaque/translucent
batches reduce the slow states' main-thread submission cost, and X11 mouse
timestamps are repaired. Known scratch allocations now precede worker startup.
Final 180 Hz saved-state sweeps produce about 179-180 distinct images/s with
zero audio underruns and no adjacent camera turn over one degree. The fresh
opening has presentation p99 6.07 ms, max 7.83 ms from its first camera redraw.

Unlimited still exposes stale-camera episodes. Subway UI slot 3 did not
reproduce on the bounded 60/180 Hz capture route and remains open, alongside
earlier minor geometry pops. Player saves/cards/settings remain untouched.
Build SHA-256 `28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
See [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md).
The next step is a player playtest of this bounded candidate, especially UI 5.


**Latest: opening/club responsiveness candidate, D17B Needs playtest.** This
supersedes the older performance results below. Final fresh boot at CPU 100%,
first person, 4x, 180 Hz: camera presentation p99 6.0 ms, maximum 8.3 ms from
the first valid redraw, zero audio underruns or failed/killed workers. Final
Match Display dancers turn: 179.5 distinct images/s, p99 6.18 ms, realtime
emulation and zero audio underruns; apartment comparison 179.8 images/s,
p99 5.98 ms. Mouse injection to traced camera presentation: median 17.85 ms
club / 12.48 ms apartment. These are bounded software measurements.

Systemic fixes cover timestamped scheduling, restored/worker code-state
coherence, an exactly guarded initialized math routine, GL batching, unnecessary
worker replacement for invalidated movie code, and a worker timeout race.
Vanilla and the accepted idle-eye behavior remain available. Minor closet and
furniture pops remain D17A playtest work. Existing player settings/saves/cards
are untouched. Build SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
See [83-opening-responsiveness.md](83-opening-responsiveness.md) for evidence,
limits and reproduction. The next step is the user's fresh-game full playtest.

**2026-10-03 renderer quality pass:** D17A/B have a new stabilization
candidate, with replay color/depth isolation fixes, coherent eye setup for
room walks, transform lookup and vertex-provenance safeguards, and tail-aware
redraw scheduling. Final 2x isolation: 459 checks, zero mismatches. Normal
visual route: 24 sequences, no detected back-and-forth jumps. Performance is
still below the target in the club: the final 180-target run produced about
85 distinct images/s while turning; Unlimited about 100. Do not interpret the
older response-time/smoothness claims below as acceptance of this workload.
See [82-renderer-quality-pass.md](82-renderer-quality-pass.md) for profiles,
remaining endpoint uncertainty, regression evidence and build identity.

**2026-10-03 (later):** loss of control fixed (host Circle requests are taps;
held Circle entered the game's inventory hold, `+0x224 0x200`); camera
re-seeded after savestate loads; late camera rebuilt (one redraw per present,
fixed-lead mouse sampling, adaptive lead, pacing hysteresis).

**2026-10-03:** see-through outdoors and in the apartment fixed (host depth
cleared per frame); dancer jerk addressed (in-betweens planned per paced
present, overrun redraws, lead 16 ms); idle stability user-accepted.

**D17A / D17B / D17C - Needs playtest.** Late camera on with adaptive pacing:
every present uses the newest mouse look (step response about 17-19 ms, was
66-130 ms), a new image every 1st, 2nd or 3rd refresh as the PC keeps up. Near
geometry: host precise vertices (exact positions, depth, texture coordinates)
plus a depth buffer among them; correct perspective beside the eye, closet and
collar artifacts gone. First person: stable idle eye (no breathing world), PS1
wobble kept while moving, deliberate view bob option `view_bob`
off/subtle/on/strong. See documentation/81-d17a-instability.md.

**D17 high refresh rate - Done (user-accepted 2026-10-02).** Modernized
`--frame-rate display|30|60|120|144|165|180|240|unlimited` (default 60;
Vanilla always 60): the runtime presents at the display's rate and TTK's own
renderer draws interpolated in-between images in worker processes; game logic,
physics, animation timing and audio unchanged. The user saw about 180 FPS at
Match Display with very good stability, correct FMVs and an accurate fps
readout. Follow-ups: D17A (texture/geometry instability, black areas), D17B
(mouse responsiveness and input latency). Details:
documentation/80-d17-high-refresh-audit.md.

**D08J1 overhead ladder grab - Done (user-accepted).** New job from the user's
slot-6 report. That ladder's bottom rung is about 1040 above the floor, so
the original ground mount never sees it and walking into it stops at the
wall; jumps from the wrong distance bounce (107). Modernized now jumps for
Duke once per E hold when he heads at such a ladder inside the catching
window (run 750, walk 800, standing 450 from the panel), keeps the reach held
through the leap (an E tap at the wall works) and eases him along the panel
in the air; the original catch and climb do the rest. Private slot-6 copy:
all E approaches caught (first/third person, manual/assisted jump, armed or
holstered), none without E; D08U/D08X/D08Y/D08W regressions unchanged.
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Details: documentation/79-d08j1-ladder-leap.md.

**D08Z manual jump style - Done (user-accepted).** Optional Modernized
`--jump manual` (assisted stays the default): jump on the press near gaps,
~0.15 s grace after running off an edge, WASD air steering up to the takeoff
speed (also during the E reach), standing/walking takeoff ~3x quicker. Slot-5
gap clearable by timing alone (last ~200 units or just after the edge; earlier
presses scramble up the low lip; E mantles from 1000 back). Assisted unchanged.
Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`. Details: documentation/78-d08z-manual-jump.md.

**D08Y gap jump - Done (user-accepted).** Jump, freezes and audio confirmed fixed. The overclock now applies only in
gameplay and backs off if emulation falls behind (fixed boot/loading audio
slowdowns; binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`). Round 4: Random 20 fps freezes were
the emulated CPU overrunning in views the Modernized camera/widescreen reveal;
Modernized now emulates the CPU at 150% (`--cpu-overclock`, Vanilla stock): 0
slow frames on the player's GPU (was 144/59). Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`. Earlier: Reach freeze frames
and the jump-mantle pop fixed (binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`). The gap was not a level-design
slip: the original launches a run jump pressed near a gap from the lip, and
Modernized had switched that off. Restored for real gaps (furniture keeps the
instant jump); a tapped Space works like the original's held button. Slot 5:
without E 8/10 clean landings (was 0/10), with E 14/14. The hitch after E jumps
(up to 27 ms per update) is down to ~6.5 ms. Mid-air mantles remain the safety
net. Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. Details: documentation/77-d08y-gap-jump-research.md.

**D14 widescreen - Done (user-accepted).** Modernized opens in native 16:9 (also
16:10, 21:9, auto; `--widescreen off` for 4:3): wider view with the original
projection, no stretching, no holes at the widened edges (TTK portal root
rectangle widened; near clip in third person), HUD in the corners, movies and
2D menus 4:3. Vanilla unchanged. Binary
`3f726b9173ef236ca7c5f38b5c12b6713a877f85b6bbd5c0b2a8aca5e392af2c`. Unlike
DuckStation's widescreen hack, text, HUD and movies keep their original
proportions. Details: documentation/76-d14-widescreen-first-pass.md.

**D08U, D08W, D08A3, D08X - Done (user-accepted).** Ladder-top mount and full
descent; shallow-water jumps in the held direction; original TTK HUD icons in
the switcher; hold-E jump grabs and mantles (ledges and climbable objects at an
angle, extra reach, flush hang, stacked-crate release, identity-guard fix).
D14 widescreen: first pass only (inert 16:9 plugin; culling gaps open). Binary
`60161f81ab42a013ebfa326d6b80b97772b29b9d8fb051b598ddba45cbde369c`.

**Autonomous chain (user away): D08U, D08X, D08W, D08A3 Needs playtest; D14
In progress.** E at a ladder top climbs down onto it and S descends and steps
off (D08U); E held through a jump catches close ledges (D08X); shallow-water
jumps follow the held direction (D08W); the switcher uses TTK's own HUD item
icons rebuilt from /DATA/FONTS.RAW (D08A3); widescreen first pass found the
activation path and TTK's culling blocker, no player option yet (D14). Binary
`db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`. Native
and Python suites pass. Details: documentation/72 to 76 and 09-handoff.md.

**D04A Escape frees the mouse - Done (user-accepted).** Escape now pulses Start so a
quick tap reaches the original pause poll, and automatic recapture waits until
gameplay has actually stopped (pause open) instead of 12 frames. Native input
test PASS; resume Escape no longer blocks recapture; binary
`198673f5f5643aed5f25c543b79ec69ed0216d69e2f211a075394f3b86ded68e`. User: "perfect, accept".

**D08V sewer mantle lease - Done (user-accepted).** Mantles, hangs and pull-ups,
the frames right after them and falls no jump owns now keep the mouse camera
and modern buttons (no `ORIGINAL MOVEMENT` flicker); the original traversal
routines are unchanged. Private slot-12 sweep: tank fallbacks 12 -> 0, mantles
identical. Current binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`
(includes D07D and D08T1). Details: documentation/71-d08v-sewer-mantle-lease.md.

**D08T1 hold to grab, E always mantles - Done (user-accepted).** In Modernized, E
climbs a pushable object (the alley dumpster) exactly like any climbable
object; holding right mouse (or Alt) grabs it, W/S push/pull, releasing lets
go. With `original` weapon aiming right mouse stays precision aim and Alt
grabs. Profile schema 17. Live private-state checks in third and first person
and with legacy aiming pass; Vanilla push/pull unchanged. Binary
`66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`
(includes D07D). Details: documentation/70-d08t1-grab-manipulate.md.

**D07D red dot off by default in Modernized - Done (user-accepted).** Profile schema
16 turns the original red autoaim dot off in Modernized (older profiles are
migrated off once, with a backup; later choices are kept; `--red-dot on`
restores it). The hide hook now proves ownership at the marker's own enqueue,
so it no longer depends on the first-map camera/identity lease. Isolated
OpenGL route: hidden markers in third person (51), held aim (59) and first
person (65); none with the dot on. Vanilla unchanged. Binary
`db5bf9640893ae582acefa27fca8f98f27017bd6f1aa4c956d203cf31e6ec67b`.

**D13 higher internal resolution and display scaling - Done (user: "correct correct correct, D13 is good. I'd say let's approve it.").** Each
profile now saves an internal 3D resolution (1x original to 4x with OpenGL),
display mode (windowed, borderless or exclusive fullscreen), window width and
output filter (linear or nearest): `run.py --settings` choice R, or
`--internal-scale`, `--display`, `--window-width`, `--output-filter`.
Vanilla stays 1x; Modernized defaults to 4x (512x240 gameplay drawn at
2048x960; user: "holding 60 FPS the whole time it just looks amazing"). The software renderer always runs at 1x (29 fps at 2x). In an
isolated CPU-OpenGL instance 1x-3x held ~60 fps and 4x 50 fps; FMV framing,
HUD and pause menu are unchanged; settings survive a relaunch. Playtest
follow-up: **F11** is now the fullscreen key (replacing Alt+Enter/Ctrl+F,
which never worked because of a runtime modifier-matching bug, now fixed), and exclusive is now a real exclusive mode under SDL3.
The game now opens windowed by default and remembers
how it was left (windowed or fullscreen, exclusive or borderless, window
width). All accepted by the user. Separate backlog from the playtest:
D08U (top-of-ladder mount), D08V (sewer mantle flicker), D08W (subway
shallow-water sideways jumps), D10B (close-camera transparency, doors) and
D11C (the slot-12 headless Duke: a first-person head-hide flag saved inside
the savestate). Details: documentation/67-d13-resolution-display.md. Binary
`79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

**D11C slot-12 headless Duke - Done (user: "duke's head is back").** A first-person head-hide
bit captured in a savestate is now reclaimed at Duke's next draw (Modernized,
either view). The original never sets that bit (static check of the executable
and all overlays). Headless check on a private slot-12 copy: head shown in
third person, still hidden in first person. Binary
`d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

**D08L inertial edge run-off - Done (user: "im happy with that!").** Running off a large ledge in
Modernized no longer brakes: on the fire-escape platform outside the apartment
window the original fall cut Duke from ~47 to 16.5 units per frame (velocity
3058) and froze the mouse camera; he now keeps his running speed (9172) and
the camera through the fall and lands into his run. Walking still stops at
large edges (original), the running jump is unchanged and the late-jump grace
still applies to small drops only. Every edge departure is capped below the
running jump (a run-start stride spike could launch the bed run-off at 14000).
Vanilla route exit 0; native suites and Python PASS. Limit: only this
platform and the bed were measured. Details:
documentation/66-d08l-edge-run-off.md. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

**D12A first-person quick kick - Done (user: "its done! accepted. ... this has been yet another amazing feat of engineering.").** In the Modernized eye
view, **Q** is now a short Duke 3D style kick: Duke's own right leg (original
meshes, posed from the original straight front kick) snaps out in front of the
eye with any weapon still drawn, standing, moving or in a jump, and the view
never leaves first person. The hit uses the original kick's damage sphere,
radius and damage where the crosshair ray meets something within reach
(about 4 contacts, 1500 to a pig cop; three kicks kill; low props like the
alley garbage bags are hit without crouching). With the Boot selected a held
left-click kicks while moving; the thigh is now drawn. With the Boot selected, the original spinning kicks (Q, E,
held attack) become the quick kick in first person. Third person and Vanilla
keep the original kick. Checked in an isolated instance against pig cops on
the first street; 59.9 fps; Vanilla route exit 0; native suites PASS. Limit:
no knockback and lower damage per kick (both accepted by the user); thigh
13% lower on screen (user's choice). Only Q or the attack kicks (never E);
Q chains while running with any weapon. Details:
documentation/65-d12a-first-person-kick.md. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

**D08T pushable objects - Done (user: "it works so much better than the original now. this is it rock solid. confidence level is very high.").** User report: the alley dumpster
could not be grabbed in Modernized; E climbed it or did nothing. In
Modernized, **E** now grabs any original pushable object (even with W held
into it), **W/S** push/pull relative to the camera, **E** lets go, and
**Space** climbs a climbable one. Mouse look stays live while grabbing, and
first person blends to the orbit during the grab. The original grab, push/pull
and climb animations run unchanged; Vanilla keeps the original Action rules.
Checked with real keys from a private copy of the user's slot-5 state (third
and first person); Vanilla route exit 0; native suites PASS. Only the dumpster
was checked. Details: documentation/64-d08t-pushable-objects.md. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

**D12 first-person weapons - Done (user: "finally, we can mark this as accepted!!").** Next: D12A (kick stays in first person). In the Modernized eye view
Duke's own right hand and the original weapon mesh are drawn in front of the
eye (no new assets), held in each weapon's firing pose, always on top of
walls, with the original muzzle flash and a short kick. All 13 weapons on the
first map checked; pitch-stable; 59.95 fps; third person and Vanilla
unchanged. The twin cannons (key 5) are framed like Duke 3D's Devastator;
the HUD stays on top. Limits: instant holster/draw, no left hand, no reload
motion. Details: documentation/63-d12-first-person-weapons.md. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798` (after framing pass 6).

**D11B first-person near-wall clipping - Done (user: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available")).** In the Modernized
eye view, walls, floors and props beside the eye are clipped and drawn by the
host instead of tearing or turning black; everything else stays on the
original renderers (`0x80011020` world, `0x80010000` object). Club side wall
and entry corridor sweeps render continuous walls where the unclipped build
shows black gaps; first person 59.96 fps, third person and Vanilla unchanged.
Third pass: after the user's 1080p report of black floor lines, the default
is a conservative mode (clip only what the original gets wrong, no
subdivision). Compare with `DNTTK_NEAR_CLIP=0`; the subdividing mode is
`DNTTK_NEAR_MODE=full` (research only). User: conservative "looks the very
best". Props no longer fade see-through in the eye view (original occluder
fade; `DNTTK_FP_OCCLUDER_FADE=1` restores). Both are future D19 menu toggles.
Details: documentation/62-d11b-near-clip.md. Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

**D11 first-person prototype - Done (user: "this is phenomenal, and it's like a dream come true").** Modernized (independent
camera): **P** toggles an eye-level view with a wider field of view (about 64
degrees); saved in profile schema 13 (`--view first|third`, default third).
The eye follows Duke's neck through the original camera solve; his head is
hidden only during his own draw; mouse look, camera-relative WASD, view aiming,
crouch and jumps all work in it. Swimming and jetpack flight return to third
person. An isolated first-level route (street, club door fight, doorway,
entry hall, main room) passed all 14 checks at 59.9 fps with 0 underruns;
Vanilla route exit 0. Known limit: steep-angle wall contact can drop wall
polygons (new job D11B). Hands and weapon: D12. Details:
documentation/61-d11-first-person.md. Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`.

**D10 Done (user: "it's done, fully accepted"); D08S cancelled (may revisit).** Modernized
camera: **V** recenters (smooth swing to the original follow angle; the mouse
cancels it), **H** cycles centered / right / left shoulder (off by default),
and Alt+wheel distance plus shoulder side are now saved in the profile (schema
12; `--camera-distance`, `--shoulder`). Shots still meet the crosshair and walls
still constrain the camera. On foot Duke already faces the view, so V mostly
levels pitch. Details: documentation/60-d10-camera-polish.md. Binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`.

**D08R Done (user-accepted: "it's absolutely rock solid").** **D08Q1 Done
(user: "verified working!!!"):** Modern jetpack Ctrl descent 2800 -> 5500, steady 29.5
units/frame to match the underwater Ctrl dive; soft landings, no damage.
Binary `9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

**D08R selectable jetpack scheme (Done).** `run.py --jetpack
classic|modern` (persisted, Modernized, default modern). Classic (revision 2,
after user feedback) keeps all modern controls - mouse look, camera-relative
WASD, J - with the original burst physics: Space boost, gravity, no host
hover or Ctrl descent. Details: documentation/57-jetpack-controls.md (D08R
section). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`.

**D23B Done - intro FMV stutter was a stranded native movie shard.** The
shard's cache folder is keyed by the `game.local.toml` hook list; the 28 Sep
hook additions moved it, so the intro decoder ran interpreted (~52 fps,
continuous underruns). Rebuilt: 59.92 fps, 0 underruns; the user confirmed
smooth playback. The build preset and `run.py` now re-check the shard on every
build/launch (~0.2 s), and the session log says `ttk-fmv: native movie
decoder active` or warns. Details: documentation/59-fmv-shard-namespace.md.
Binary `3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`.

**D23A Done (user-accepted) - the Modernized stutter/slowness had a measured
cause and is fixed in the current build.** Isolated probe: Modernized ran
at 47.5–49.3 fps with continuous audio underruns (output fill 17–34 ms
against a 180 ms target) while Vanilla ran 60 fps clean. `ttk::identity()`
re-read all 106 guards (81 KB) word-by-word through the guest bus on every
call — 46 calls per frame, 108 µs each, 5 ms of every 16.7 ms frame (24 %
of wall time). It now memcmps the runtime RAM image once per host frame
(re-verifying immediately on any RAM-code generation change) and costs
28 µs per frame. Result: 59.94 fps, 0 underruns, fill 267 ms; jetpack flight
60.1 fps. Details: documentation/58-modernized-frame-budget.md.

**D08Q Done (user-accepted) - modern jetpack flight.** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." User: in the jetpack only
Space worked, WASD were "blocked", mouse dead. Original mode 10 had no lease;
WASD reached it only through the tank fallback relative to an unturnable body,
and its idle gravity landed Duke within seconds. New host layer
(`recomp/src/ttk/jetpack.inc`) over the unchanged original handler: mouse
orbit with the body facing the view (so the original body-relative thrust is
camera-relative), W/S/A/D → original Up/Down + L2/R2, Space climb (original),
release everything → hover (original hover lock at the current height), Ctrl
→ steady descent to a soft landing, J in the air → original cut-out fall with
the camera still live. Probes: headings within 4° of the camera, level flight
±25 units, 25–29 units/frame forward/back, Ctrl 489 units/30 frames then
landing, ground controls resume. Known: stop-on-release is immediate; after
Ctrl release Duke settles ~130–300 units more near a floor; fuel drains while
hovering (original rule). Details: documentation/57-jetpack-controls.md.

Binary `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

**D08P Done (user-accepted) — area-wide control loss was overlay identity;
mid-depth wade now runs at land pace.** The user's session log showed `ident=0` from the
flooded-corridor ledge onward. The LEVEL00 zone script writes a hit position
into the overlay's trailing 16-byte scratch vector (`0x800ccf1c`), which the
9668-byte overlay guard covered, so identity failed permanently the first time
that script fired. The guard is now the 9652-byte code/table body. Separately,
that water is depth 512: the original swaps every gait for wade clips 80/81
(Vanilla 8–14 units/frame, forward only). The wade handler `0x800539f8` is
now hooked (new generated entry) and its root retargeted to camera-relative
WASD at the land run band: 45–58 units/frame, mouse steering, strafe, Space
jump; 0 identity refusals across and past the ledge. Wading straight into a
wall idles as in the original; turn the view to resume. Details:
documentation/56-scripted-camera-controls.md (iteration 6).

Pending playtest jobs D07A, D07B, D08A, D08H and D10 distance were **Done** on
user acceptance of ongoing play. D08N scuba remains cancelled. D10 broader work
stays In progress. D08B is Done on 2026-09-29 after the user playtested all
of its recorded limits (see the job board work log).

Previous binary `79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`
(D08P acceptance).

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
