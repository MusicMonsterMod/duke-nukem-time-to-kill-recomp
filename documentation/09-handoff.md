# Next-session handoff

## 2026-10-08 - D08Z2 accepted

- User: "accepted!!". D08Z2 Done (armed scramble plays full body; the D08Z1
  safety net no longer freezes the legs). Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986` is the regression
  baseline. Next: D08Z3.

## 2026-10-08 - D08Z2 revision 1: legs frozen after a corner bump (Needs playtest)

- User: after a bump at the statue (UI slot 8) Duke's lower half stays locked
  in one pose whatever he does (UI slot 6).
- Cause (D08Z1 safety net): its 107 undo ran at the next animation runner
  call, after the update had started 107 on the tracks; track 3's OR-only
  joint mask kept 107's leg joints (`0x1c00` -> `0x1cef`), freezing the legs,
  also in savestates. Reproduced at the monument corner; the original bounce
  never does it.
- Change: undo at the bounce's contact sound `0x8006bbd8` (ra `0x80055a10`),
  right after the 107 write; new opt-in hook, regenerated, codegen hash
  unchanged, savestates load.
- Evidence: three corner catches, mask stays `0x1c00`, legs stride after;
  native 45 groups, Python 136 OK. Executable `bfd41760f9c66f2f03cf859f820f7b7d27cd8991e8ce1f304f5456ea29bf8986`.
  [Note 135](135-d08z2-mantle-upper-body.md#revision-1-legs-frozen-after-a-corner-bump).
- The player's new UI slot 6 keeps the frozen legs (saved polluted state).
  Nothing committed.

## 2026-10-08 - D08Z2 mantle upper body (Needs playtest)

- Cause: the D08Y low-lip step-up (no E) could start with a weapon drawn, and
  since D08Z1 it starts from every wall slide. The original mantles only with
  a free upper body (`0x80051cf0` refuses on upper-table bit 8) and sets only
  the lower animation, so the legs played 134 while the upper track held the
  static weapon-ready pose 20. Holstered E mantles, crate mantles and the
  slot-12 grab and pull-up were full body and matched `original` frame for frame.
- Change: `jump_mantle_start` (one start for the crate, lowered-catch and
  step-up mantles) hands a weapon pose on the upper track to the mantle;
  `mantle_upper_restore` puts the kept upper block back when the mantle ends
  (without it the weapon stayed lowered and would not fire).
- Evidence: armed slot-5 step-ups play full body, weapon pose back one update
  after, Mouse1 fires; D08Z1 regression routes same outcomes; native 45
  groups (D08Y group extended), Python 136 OK. Executable `9de8da617862f53e545ac3c8be0f23623979fea2265944e2f3cdfaa3ec3f96b0`.
  [Note 135](135-d08z2-mantle-upper-body.md).
- Open: the user's playtest. Nothing committed.

## 2026-10-08 - D08Z1 accepted; D08Z2, D08Z3 queued

- User: "mechanically this feels significantly better, where we can call the actual job as accepted." D08Z1 Done; executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34` is the regression baseline.
- D08Z2 (next): the mantle now plays as a static leg pose, a regression from
  D08Z1. D08Z3: occasionally Duke is stuck for about a second after landing.
  Leads in the job board.

## 2026-10-08 - D08Z1 revision 1: monument corner (Needs playtest)

- The slot-6 monument corner bounced (convex corners defeated the wall clip).
  Jumps now turn around a corner in 15-degree steps, and any original bounce
  that still happens is turned into a slide, with its 107 undone in the same
  update. Slot 6 corner: no 107 in 21 jumps; earlier routes unchanged.
  Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. [Note 134](134-d08z1-jump-wall-slide.md).

## 2026-10-08 - D08Z1 jump wall slide (Needs playtest)

- Modernized jumps no longer bounce off walls: a wall only stops the motion
  into it, so Duke keeps his arc and slides along it (EDuke32 style); rising
  into a ceiling stops the rise and keeps the forward speed. No 107 stagger,
  rumble or contact sound. `run.py --jump-walls original` (menu K) restores
  the bounce; Vanilla unchanged. Profile schema 30.
- Verified offscreen: no 107 in 48 wall jumps over three levels, both jump
  styles; D08X/D08Y/D08U/D08W/D08Z routes keep their grabs, mantles and
  scrambles; native 45 groups, Python 136 OK, `check_repo.py`. Executable
  `c91a3ef128d9f98d923efe858fef75f432b8d729b2ffe5f16654f0034a56b627`. [Note 134](134-d08z1-jump-wall-slide.md).
- Needs the user's playtest. Nothing committed.

## 2026-10-08 - D24C accepted

- User: "all accepted". D24C Done: TTK-font `!` and `>` console prompt.

## 2026-10-08 - D24C TTK-font `!` and `>` console prompt (Needs playtest)

- Messages, headings and the console now draw `!` from the TTK font's own I
  and period (build-time, `build_ttk_fonts.py`) instead of the 8x8 system font.
- The console prompt and echoed commands start with `>` instead of `]`.
- Verified in a private offscreen run (`dnstuff`, console); suites pass. Needs
  the user's look in their own window.

## 2026-10-08 - D08A18 accepted

- User: "accepted and confirmed working!!!" D08A18 Done.

## 2026-10-08 - D08A18 built (Needs playtest)

- Enter / U on the browsed mission item: at the lock that takes it, the normal E
  interaction (the original uses it up); elsewhere CAN'T USE THIS HERE, or NOT
  FOUND YET, as cheat-style notices. Note 133.
- Verified offscreen in level 0 (crystal holder use, both refusals); suites
  pass. Card readers / key doors and other levels need the user's playtest.

## 2026-10-08 - D08A17 accepted

- User: "confirmed it's all working as intended!" D08A17 Done; executable
  `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9` is the regression baseline.

## 2026-10-08 - D08A17 power-up countdowns (Needs playtest)

- Invincibility (`+0x8a0`), invisibility (`+0x89c`) and Double Duke (`+0x89e`)
  are 6000-unit timers (20 s) set by their pickups and drained by Duke's
  update; death zeroes them and Continue gives 1500 invincibility (original).
  In Modernized each running one now has a lit box with the user's D08A16
  icon counting down in percent, stacked above the selected gadget, an
  active jetpack and running steroids (`gadget_hud.inc`). User choices:
  percent, the Continue protection shows, no expiry warning.
- Executable `8f338cde1c2dc4cf73da747621cffafe2478512f6419078a41d11b0d3b9268a9`;
  private Xvfb evidence and suites pass;
  [note 132](132-d08a17-powerup-countdowns.md). Waiting for the user's
  playtest. Not committed.

## 2026-10-08 - D08A16 accepted

- User: "fully accepted." D08A16 Done: the user's power-up icons are in
  `recomp/assets/ui/items/` (`hud-invincibility`, `hud-invisibility`,
  `hud-double-duke`, each with a `-15col` copy). There is no switcher-strip
  variant (power-ups are never held). D08A17 (countdown boxes) is now ready.
  Not committed.

## 2026-10-08 - D08A16 picker section ready (waiting for the user's art)

- D08A16 In progress: local picker `recomp/analysis/d24a-fonts/ttk-font-picker.html`
  section 11 has placeholders, in-game coin references (all one radiation
  coin; gold/orange invincibility, silver-violet/blue invisibility,
  silver/pink-red Double Duke), 15-colour previews and a stacked HUD mockup.
  Next: the user delivers `hud-invincibility.png`, `hud-invisibility.png`,
  `hud-double-duke.png` to `recomp/assets/ui/items/`; reduce them with
  `reduce_icon_15col.py`, rerun `make_picker.py`, user accepts. No gameplay
  change; nothing committed.

## 2026-10-08 - D08A14, D08A15, D08G4 accepted; D08A16, D08A17 queued

- User: "fully accepted and ready to close it out." D08A14, D08A15 (with the
  pickup correction) and D08G4 Done. Executable
  `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472` is the regression baseline.
- Queued: D08A16 (power-up coin icons for invincibility, invisibility and
  Double Duke; human design ticket, placeholders in the local font picker
  when it begins) and D08A17 (HUD countdowns for the three power-ups, after
  the icons). Not started.

## 2026-10-08 - D08A14, D08A15, D08G4 (Needs playtest)

- **D08A14:** death ends steroids in Modernized `portable`: running and held
  doses are cleared while Duke is dead (user chose that a held dose is lost
  too), so after Continue there is no box, heartbeat or switcher entry.
  `original` and Vanilla keep them. [Note 127](127-d08a4-portable-steroids.md#d08a14---death-ends-steroids).
- **D08A15:** the heartbeat restarts with a beat at once when `dnhyper`
  refreshes steroids mid-run. Corrected by the user: a steroids pickup mid-run
  now stops the run and leaves a full held dose (Duke 3D feel), like the
  inventory cheats; executable `87fe38d4f0ea8560312f53171345c207f332e26b6f4d2ed7220e1afae66ba472`. Cause: a refresh to 9000 never read
  above the run's first reading (9000 minus one drain step), so the old beat
  count held the next beat back. [Note 129](129-d08a10-steroids-heartbeat.md#d08a15---restart-mid-run-beats-at-once).
- **D08G4:** `dnstuff`, `dnitems`, `dninventory` set armor to 100 (10000, the
  original full armor value); running steroids stop and a full dose is held
  (user rule when selected). [Note 38](38-debug-cheats.md).
- Final executable `a4cd7a4f8efcfc44ef86e816adc3b245fcde9a733acd1e9f503fdf51e0235ef0`.
  Suites: `ttk-controls-test` 44 groups (new D08A14, D08G4), input,
  inventory, Python 131 OK (2 skipped), overlay guards, `check_repo.py`.
- **Not yet verified:** the user's playtests. Nothing committed.

## 2026-10-08 - D08A13 accepted; D08A14, D08A15 queued

- User: "i accept that this works." D08A13 Done; executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05` is the
  regression baseline.
- Queued: D08A14 (death and Continue end steroids and remove them from the
  inventory) and D08A15 (heartbeat silent for a few seconds when steroids
  restart mid-run). Not started.

## 2026-10-08 - D08A13 steroids independent of damage (Needs playtest)

- **Found:** the coupling is original. Duke's damage handler `0x800a40a8`
  checks running steroids first (`0x800a4154`): the hit is cancelled entirely
  (no armor or health loss) and costs the steroids 1500 of 9000. The armor
  element only displayed the countdown.
- **Changed (Modernized, `steroids` `portable`):** new entry hook
  `0x800A40A8` (one regenerated line, codegen hash unchanged, savestates
  load) turns the running bit off for Duke's damage call only. The hit takes
  the normal path (armor 75%, health, death and its return value), and the
  timer is untouched. The bit returns at the next TTK hook, before the drain,
  kick, status bar or heartbeat read it. Vanilla, `original` and
  `DNTTK_STEROID_SHIELD=original` keep the shield.
- **Evidence:** pig cop on savestate slot 3: 5-10 hits per run, timer drains
  5 per frame regardless (60 and 120 fps, armor, `dnhyper`, R dose,
  savestate); Vanilla 4 absorbed hits of 1500. Death and Continue resume the
  remaining time. Suites, Python 131, overlay guards, Vanilla route. Executable
  `5f7052656083349168451c0e217fb24fcd9b0a3372d188a38d0319b459c01f05`.
  [Note 127](127-d08a4-portable-steroids.md#d08a13---steroids-independent-of-damage).
- **Note for the user:** steroids no longer protect Duke in Modernized.
- **Not yet verified:** the user's playtest; explosions, falls and drowning
  in isolation; natural pickups.

## 2026-10-08 - D12C accepted

- User: "perfect!! fully accepted." Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f` is the regression baseline.
- D12C: a connecting kick (first or third person, Modernized) plays the wall
  bump thud `0x2007` one octave up at three times its volume, once at the
  first contact; misses stay silent. [Note 131](131-d12c-kick-impact.md).

## 2026-10-08 - D12C kick impact sound (Needs playtest)

- **What:** in Modernized, a kick that connects (an enemy, a breakable or
  other object, a wall) plays the game's own wall-bump thud `0x2007` once, at
  the first contact; empty-air, missed and out-of-range kicks stay silent.
  First-person quick kick and the third-person original kick.
- **How:** the damage sphere `0x800a979c` stores the attacker in its own
  frame only when it strikes something; host calls read that slot. Walls use
  the original segment query where the boot reaches. Third person: new entry
  hook `0x800A979C` (one regenerated line, codegen hash unchanged, savestates
  load) makes the kick's sphere call on the host and leaves the original call
  touching nothing, so damage is applied once. `DNTTK_KICK_IMPACT=off` turns it off.
- **Evidence:** private runs on the first street and the alley (pig cop, club
  door, garbage bag, head-on wall, empty air, floor, out of range); suites,
  Python 131, Vanilla route. Executable
  `3873288acd438d2e367ca2c0fffa20f0953134de61af7715522fec0ffcc87c98`.
  [Note 131](131-d12c-kick-impact.md).
- **Not yet verified:** the user's ear in play; a multi-kick crate; other levels.
- **Revision (same day):** the user found it very quiet; 92% of the thud's
  energy is below 150 Hz. It now plays one octave up (user's pick):
  `0x8006b73c` with pitch byte 48, SPU pitch `0x7E8`. Executable
  `b62af2b911cfdcc7c300ac025f00f1218b6f8c7439bbd8eda460ce9243283e3d`.
- **Revision 2:** user asked to double its volume: the impact's own sound
  object volume `+0x6a` is doubled (5192 -> 10384); live voice `0xC2A` ->
  `0x1855`. Executable `de112f13e75afef2e31543f48f09ec58c49ba4f6ada9a93b914d2e66cf0d9dbd`.
- **Revision 3:** multiplier 3 by default; the silent first kick at a wall
  was the routine refusing a `0x2007` already playing (Duke's quiet wall
  bump). A playing `0x2007` is now stopped first (`0x80068900`). Executable
  `7508b61525de474d12482cf32bbb524b5b48a6101fa5a903887a8f79edf55b7f`.

## 2026-10-08 - D27A accepted; D12C queued

- User: "i fully accept this. great work. i have wanted this one for a long
  time so this minor change has a huge impact on me as a player." Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829` is the
  regression baseline.
- New Todo D12C: kick impact sound driven by the real kick hit result (wall,
  crate, actor), using the existing wall-collision thud (not Duke's grunt);
  empty-air and out-of-range kicks stay silent; no duplicates over targets'
  own damage sounds; kick gameplay unchanged.

## 2026-10-08 - D27A Modern Shift/run is silent (Needs playtest)

- **What:** in Modernized, pressing, holding and releasing Shift no longer
  plays the walk/run click (`0x0001`); running is unchanged and the D08A10
  steroids heartbeat still plays the same click on its beat.
- **How:** a new entry hook on the player update `0x800412A4` stores the run
  state the original toggle is about to set at `player+0x27b` first, so the
  original sees no change and never clicks (`run_click.inc`; one regenerated
  line, codegen hash unchanged, savestates load). Vanilla's L1 toggle still
  clicks. `DNTTK_RUN_CLICK=original` keeps the click.
- **Evidence:** SPU KEYON counts of the click sample per leg: Shift taps 10 -> 0,
  hold/release 2 -> 0, walking taps 8 -> 0; beat identical with or without
  Shift (19 beats, 15/15/18 fields). Suites and Python 131 pass. Executable
  `71e6c7ae711ec30aaf596470b4a2d8f28ec7d22c84520cfb8e8e214b20370829`.
  [Note 130](130-d27a-silent-shift.md).
- **Not yet verified:** the user's ear in play.

## 2026-10-08 - D08A11 accepted; D27A and D08A12 queued

- User: "this job is fully accepted." Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892` is the
  regression baseline.
- New Todo D27A: Modern Shift/run silent. The sound is the known walk/run
  toggle click `0x0001` via `0x8006bbd8(1)` (note 129); remove it from the
  Modern Shift path only and keep the D08A10 heartbeat, which uses it.
- New Todo D08A12: Modern dynamite - selecting it (6 twice, mouse wheel) must
  not commit Duke to a fuse; research the original equip/arm/fuse state
  machine first; Dynamite Behaviour Modern (Modernized default) / Original.

## 2026-10-08 - D08A11 `dnhyper` in the steroids box (Needs playtest)

- **What:** with portable steroids, `dnhyper` now counts down in the D08A8
  steroids box (lit pill icon, stacked or selected like a used dose) and the
  armour element shows only armour. It ends with nothing held.
- **Cause and fix:** `dnhyper` sets the running bit without the owned bit;
  the HUD predicates in `steroids.inc` now treat the running bit alone as
  running steroids, and the status-bar hide/restore no longer depends on the
  owned bit (savestate-safe). A pickup during a `dnhyper` run selects steroids
  (`pickup_select.inc`). R still takes only a held dose.
- **Evidence:** 60 and 120 fps private runs (no armour, armour 50, savestate
  reload, end, pickup refresh, R dose regression, `original` profile); suites
  and Python 131 pass. Executable
  `2ebef6a88598649b2fade42d6306a253d1b7228df7cc57763e5f041791fde892`.
  [Note 127](127-d08a4-portable-steroids.md) (D08A11 section).
- **Not yet verified:** the user's look in play.

## 2026-10-08 - D08A10 accepted; D08A11 queued

- User: "I love it, that sounds pretty much the same as the one in duke3d now. I accept this job as done!!" Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca` is the regression baseline.
  [Note 129](129-d08a10-steroids-heartbeat.md).
- New Todo D08A11: `dnhyper` shows its countdown in the original armor
  element instead of the D08A8 steroids box. Likely cause: `dnhyper` sets the
  running bit without the owned bit that the HUD predicates require.

## 2026-10-08 - D08A10 sound fix: the Shift click (Needs playtest)

- User on the first build: "the beat is right but the sound is wrong. i want
  the sound specifically when you press shift". Found it: sound `0x0001`, the
  walk/run toggle click, played through the non-positional call
  `0x8006bbd8(1)` (SPU sample `0x012F0`, pitch `0x228`-`0x22F`). The beat now
  makes that exact call; same sample and pitch as a real Shift press in levels
  0 and 6, rhythm unchanged. Suites pass. Executable
  `146e7ce5a9e5a762d64f3ccd9a878285ea05952d1b8065a0a28e3bc690ecdcca`.
  [Note 129](129-d08a10-steroids-heartbeat.md).

## 2026-10-08 - D08A10 steroids heartbeat (experimental, Needs playtest)

- **What:** in Modernized with portable steroids, a low heartbeat thump (the
  game's own sound `0x1012`, Duke's bank 1) plays at Duke while steroids run,
  225 bpm as Duke 3D's `DUKE_HARTBEAT` (reference video: 268 ms between beats).
  Stops with the effect, a damage cut skips ahead, death and level change end
  it. `DNTTK_STEROID_BEAT=off` silences it, `=<id>` tries another sound (the
  Shift-run footstep is `0x2000`). Console `sfx <id>` auditions any sound.
- **How:** `steroids_beat.inc`, one beat per 80 units of the steroids timer
  (300 units/s) from the existing authenticated player-update hook; the
  original sound call on a private stack. No new hooks or codegen change.
- **Evidence:** identical rhythm at 60 and 120 fps; pickup + R, savestate load,
  damage cuts, `level 6`, level 6 play, `original`/`off`; suites and Python 131
  pass. Executable
  `d0d1f09355d69f4ea3ff1a1a6ea7737bf261f512d83443b3d0f3edf20c861acf`.
  [Note 129](129-d08a10-steroids-heartbeat.md). Previews:
  `recomp/analysis/d08a10-beat/preview/` (local).
- **Not yet verified:** the user's ear (keep, change sound, or revert); the
  rhythm steps 250/250/300 ms in 20 Hz scenes; death while running.

## 2026-10-08 - D08A9 accepted; D08A10 queued (experimental)

- User: "fully accepted, working beautifully." Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6` is the
  regression baseline. [Note 128](128-d08a9-pickup-selection.md).
- New Todo D08A10 (experimental, may be reverted): while steroids run, a
  heartbeat-like sound at 226 bpm as in Duke 3D, using a game sound (an
  existing heartbeat if any, else Duke's Shift-run sound). Reference video
  `research/screencaps/Video_2026-10-07_23-59-03.mp4`.

## 2026-10-07 - D08A9 picked-up gadget becomes the selection (Needs playtest)

- **What:** in Modernized, picking up a jetpack, Bio Mask, goggles, medkit or
  steroids (or topping one up) makes it the `[ / ]` selection and the HUD box
  at once, as in Duke 3D; Enter / U then use it. Keys, mission items, full
  gadgets left on the ground and `dninventory` leave the selection alone.
- **How:** `pickup_select.inc` compares items 1-5 around the original pickup
  dispatcher (`0x80081a48` entry, `0x8001ca4c` ra `0x8007fe78`; existing
  hooks) and selects through the guest menu ID `0x800c3f94`. Waits while a
  gadget is mid-activation. No new hooks or codegen change.
- **Evidence:** private Xvfb runs in levels 0 (60 fps) and 6 (120 fps), new
  native D08A9 group, suites and Python 131 pass. Executable
  `c4feb970850e28eeaeaecad473926da3056f94057e80de881511230b738a46f6`.
  [Note 128](128-d08a9-pickup-selection.md).
- **Not yet verified:** natural (non-spawned) pickups, death/Continue, the
  user's feel.

## 2026-10-07 - D08A8 accepted; D08A9 queued

- User: "you're better at this than i am, because the consideration to move it up a row when switching, and on steroids, was chef's kiss level excellence. this is phenomenally good ... i accept this as complete." Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8` is the regression baseline.
  [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A9: when Duke picks up a gadget (jetpack, Bio Mask, goggles,
  medkit, steroids), it becomes the `[ / ]` selection and the HUD box, as in
  Duke 3D. Mission items and keys leave the selection alone.

## 2026-10-07 - D08A8 steroids countdown in the steroids box (Needs playtest)

- **What:** in Modernized (`steroids` `portable`), running steroids count down
  in their own HUD box with the pill icon, lit: in the item slot while selected
  (they now stay selected after R, so the box counts down in place), otherwise
  one box row above (above an unselected jetpack that is on). The armour element
  on the left shows only armour, and stays hidden with no armour.
- **How:** status bar element 2 shows steroids whenever `+0x364` bit 1 is on.
  For the status bar draw only, bit 1 is off; it comes back at the first hook
  after the status bar returns (`0x8001fc44`, or new lightweight hook
  `0x8002E850`; one regenerated line, savestates still load).
- **Evidence:** private Xvfb runs (GL/Software, 4:3/16:9, 60/120 fps), Vanilla
  unchanged; native suites and Python 131 pass. Executable
  `e9e0cfa7aeec88ace33f794b4a831ebc0b536b09bd4dce54df6e52e85865ffc8`. [Note 127](127-d08a4-portable-steroids.md).
- **Accepted** (see the entry above).

## 2026-10-07 - D08A4 accepted; D08A8 queued

- User: "mechanically, the steroids work perfectly. you pick them up, you can
  press r to run, and thats it ... i accept this job as complete now as it's
  functional". Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5` is the
  regression baseline. [Note 127](127-d08a4-portable-steroids.md).
- New Todo D08A8: the running countdown uses the original armor element and
  icon; show it in the steroids box with the pill icon instead (reference
  `research/screencaps/ttk-roids.png`).

## 2026-10-07 - D08A4 built: portable steroids, EDuke32 style (Needs playtest)

- **What:** in Modernized (`steroids` `portable`, the default), picking up
  steroids keeps them: STEROIDS shows as before, and they appear in the `[ / ]`
  switcher after the medkit and in the HUD item box with the user's own
  pill-bottle icon. **R**, or Enter/U with steroids selected, takes them
  (`USED STEROIDS`, the pickup sound, the original effect). One at a time: more
  steroids stay on the ground while one is held; a pickup while they run
  refreshes them, as in the original. `original` and Vanilla keep the original
  rule. `run.py --steroids portable|original`, `--settings` S (profile schema 29).
- **How:** the original still has a held steroids item (item 4, `+0x364` bit 0
  with its amount `+0x366`); only its pickup case `0x800827e8` runs it at once.
  The pickup is turned into a held item at the pickup tail's own sound call (new
  entry hook `0x8006B73C`, one regenerated line; savestates still load). A first
  design with a count in spare flag bits was replaced after a write trace showed
  the level-end snapshot `0x80083348`, which the card save writes, keeps only
  bit 0.
- **Evidence:** private copies of the user's cards and savestates, levels 0 and
  6: pickup held, second pickup left, R runs it, refresh while running;
  savestate reload, level completion with the stats-screen card save, loading
  that save, and death with Continue keep it; `original` and Vanilla unchanged;
  suites pass. Executable
  `f91c4ec92e7494dd8854b98b0e2332fc17f9c1f8280e7b85798e1b4c9ea10bd5`.
  [Note 127](127-d08a4-portable-steroids.md). Next: the user's playtest.

## 2026-10-07 - D08V1 accepted

- User: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area". Their playtest log `session-20261007-202257.log` shows
  two chain top exits (196) with no identity failure.
  Executable `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`
  is the regression baseline. [Note 126](126-d08v1-control-loss.md).

## 2026-10-07 - D08V1 built: Modernized controls never lost to a chain exit (Needs playtest)

- **What:** climbing a chain or pole out at the top (the original exit 196,
  which the user reached after a mid-air catch) no longer switches Modernized
  controls off for the rest of the session. If the game's code check ever fails
  for real, the screen says `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`,
  the mouse turns Duke and WASD move him, and Modernized returns when the check
  passes. Vanilla unchanged.
- **Cause:** the original pole/hang side probe `0x800439e4` toggles bit 0x40 of
  the flag-table entry of Duke's current animation. A code guard covered that
  table, so the game's own write (`0x61 -> 0x21` on entry 196) read as tampered
  code. The fallback had no way to turn.
- **Fix:** the table is now a masked state guard (bit 0x40 on all 280 entries,
  and entry 265's low half for `BONUS.OVR`), with every other bit still
  authenticated. New `guard_writer_audit.py` (static, delay-slot aware, and live
  via `--port`) found no other reachable writer into any guard set.
- **Evidence:** private copy of UI slot 3. The baseline reproduced the lockout
  on the first top exit. Fixed: top exit, catch jumps, a missed jump and fall
  keep the lease; the live audit is clean; a forced loss turns with the mouse,
  moves and recovers; Vanilla host idle; suites pass. Executable
  `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`.
  [Note 126](126-d08v1-control-loss.md). Next: the user's playtest.

## 2026-10-07 - D08J5 accepted; D08V1 queued

- User: "that definitely works, and i accept it ... it works perfectly."
  Executable `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`
  is the regression baseline. [Note 125](125-d08j5-chain-descent.md).
- New job D08V1: after a missed jump that caught the chain in the air, the
  code guard on the upper-body animation flag table (`0x800c2a8c`) tripped on
  entry 196 (the original cleared bit 0x40 at run time) and Modernized
  controls stayed off for the rest of the session. Log copied to
  `recomp/analysis/control-loss-20261007/`. Modern controls must never be lost
  this way.

## 2026-10-07 - D08J5 built: climb down chains and poles (Needs playtest)

- **What:** in Modernized, at the top of a chain or pole press **E**
  (`E TO CLIMB DOWN` shows): Duke grabs it, swings round to the far side and
  hangs facing the platform. **S** climbs all the way down and steps off at the
  bottom; **W** climbs back up; **Ctrl** or **Space** lets go anywhere; S at an
  end with nothing below lets go. Vanilla unchanged.
- **Cause:** the original has no top mount for poles/chains and the jump from
  the slot-3 platform overshoots. Its down probe also accepts a floor above
  Duke, so on the platform side near the top Down climbed him back out
  (original, Vanilla too); a new codegen hook `0x8003964C` stops that in
  Modernized (regenerated one line; savestates still load).
- **Evidence:** private copy of UI slot 3: mount, full descent and step-off in
  third and first person, W exit, Ctrl/Space, A/D, platform-side descent;
  Vanilla and the slot-5 ladder regression unchanged; suites pass. Executable
  `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`.
  [Note 125](125-d08j5-chain-descent.md). Next: the user's playtest.

## 2026-10-07 - D08J3 accepted; D08J5 queued

- User: "we have an absolute winner once again!! it's working well, very fun,
  looks great." Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520` is the regression baseline.
  [Note 124](124-d08j3-climb-camera.md).
- Backlog: D08J5, climbing down chains (UI slot 3, re-saved with a chain in
  front of Duke).

## 2026-10-07 - D08J3 follow-up: climbs centre the view (Needs playtest)

- User: "it works perfectly!" and asked that climbs centre the view whatever
  the over-the-shoulder setting. Built: the shoulder offset eases to centre
  during ladder/pole/chain climbs and back afterwards; the saved preference is
  unchanged. Chain run and suites pass. Executable
  `5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`. [Note 124](124-d08j3-climb-camera.md).

## 2026-10-07 - D08J3 built: free camera on ladders, poles and chains (Needs playtest)

- **What:** in Modernized, the mouse camera stays free while Duke climbs
  ladders, poles, chains and climbing walls (orbit, look up/down, Alt+wheel,
  V recentre). On poles and chains A/D go left/right on screen from any
  camera side. Vanilla unchanged.
- **Finding:** the climb keeps the normal camera; in mode 3 the original only
  swaps in a high look-down boom, and the host orbit lease stopped there.
- **Evidence:** private copies of UI slots 5 (ladder) and 3 (chain): full
  orbit while climbing, climbing unchanged, A/D screen-relative behind, front
  and side; ladder top/enemy/descent regression and Vanilla unchanged; suites
  pass. Executable `2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`.
  [Note 124](124-d08j3-climb-camera.md). Next: the user's usability playtest.

## 2026-10-07 - D08J4 accepted

- User: "it's a complete winner for me. I totally accept." Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa` is the regression baseline. [Note 123](123-d08j4-ceiling-hang.md).

## 2026-10-07 - D08J4 built: climbable ceilings (Needs playtest)

- **What:** in Modernized, hanging from a climbable ceiling (monkey bars),
  WASD move Duke relative to the camera like walking and he no longer lets go
  in mid-travel; at an end he keeps hanging. Ctrl or Space lets go. Vanilla
  unchanged.
- **Cause:** the falls were ours: the D08X object-hang release let go after W
  had been held about 40 updates (and on S). The tank steering is original
  (mode 7: Up/Down along the facing, Left/Right turn).
- **Evidence:** private copy of UI slot 3: camera-relative travel in all
  directions, no drops, ends hold, Ctrl/Space let go; suites pass. Executable
  `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`.
  [Note 123](123-d08j4-ceiling-hang.md). Next: the user's playtest, including
  whether the red-carpet ledge can be reached.

## 2026-10-07 - D08J2 accepted; D08J4 queued

- User: "fully and completely accept it all." Executable
  `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the regression
  baseline. [Note 122](122-d08j2-pole-chain-sidestep.md).
- Backlog: D08J4, ceiling monkey-bar climbing drops Duke at the wrong points
  (UI slot 3, re-saved at the top of the chain).

## 2026-10-07 - D08J2 built: pole and chain A/D direction (Needs playtest)

- **What:** in Modernized, while Duke hangs on a pole or chain, A takes him
  round it to the left and D to the right on screen. Vanilla unchanged.
- **Cause:** original. Left/Right on the hang-climb (192..195) reach the
  sidestep `0x800439e4`; D-pad Left carries Duke to his own right, then
  `0x80043c08` turns him to face the object. A was D-pad Left.
- **Evidence:** private copy of UI slot 3 (the chain): A/D in third person and
  the first-person profile, W/S climbing; Vanilla D-pad unchanged; suites pass.
  Executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`.
  [Note 122](122-d08j2-pole-chain-sidestep.md).

## 2026-10-07 - D08U2 accepted

- User: "fully accepted. this is exactly what i wanted. well done!" Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e` is the
  regression baseline. [Note 121](121-d08u2-ladder-top-enemy.md).

## 2026-10-07 - D08U2 built: climb off a ladder past an enemy (Needs playtest)

- **What:** in Modernized, an enemy (or other character) standing near a
  ladder top no longer keeps Duke on the top rung: holding W, he climbs off
  with the original exit and stops short of the enemy. Walls and other
  objects still block; the enemy is not pushed (user: optional). Vanilla
  unchanged.
- **Cause:** original. The exit test `0x8007d240` refuses when its forward
  probe `0x8007cde8` meets any cell object, characters included.
- **Evidence:** private copies of UI slots 5 and 12: slot 5 in both views,
  the second alley ladder with an enemy respawned on its landing, D08U/D08U1
  mounts and descents unchanged; suites pass. Executable
  `e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`.
  [Note 121](121-d08u2-ladder-top-enemy.md).

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

User accepted after a play session with several saves. Not committed; no next
job selected.

## 2026-10-06 - D23H implemented (Needs playtest)

Cause measured: a shed was permanent at fixed/display rates (`late_pace` runs
only at Unlimited on the present timeline), and one-off hitches (saves 83-98 ms,
F7 menu) shed. New `replay_load_window()` (`src/ttk/frame_replay.cpp`), driven
by `overclock_lease()`: shed on two behind seconds in a row, step up after clean
seconds with decaying backoff. `[TTK pace]` log lines show every change and a
per-minute summary. Offscreen A/B and a 21-minute run hold 120. Next: the user
plays a long session with saves (the default settings) and checks the session
log's `[TTK pace]` lines. Not committed.

## 2026-10-06 - D29 added (progression items and objectives legibility)

From the user's Level 2 playthrough: the bank-vault papers can be collected
without noticing, cannot be inspected later, and nothing connects them to the
vault (the user needed a walkthrough). New Todo D29, research and design
first: how the papers, the code data and the vault check work internally,
what the game tracks and shows, whether original item data and Objectives
text can be read or updated (not by extending the inventory switcher; the
user will bring presentation options), a game-wide audit of similar opaque
progression, a lightweight information model, presentation options, objective
guidance without spoilers, and a smallest first build. Principle: remove
unnecessary obscurity without removing discovery. Not started.

## 2026-10-06 - D23H added (presents fall to about 60 over long play)

The user reports that after about 10 minutes of play on the D23F default, presents
fall from 120 to what looks like 60 and stay there. The session logs tie all 13
slowdown events to savestate saves and loads (about a second's stall). Likely
mechanism: D23E sheds presents to every 2nd refresh, and the step-up backoff
(`pace_backoff`) doubles and never decays, so after a few saves 120 needs minutes of
perfectly clean play to come back. Not reproduced or measured yet: presents/s
are not logged. Job written up with the evidence and a measure-first plan; to
continue next session. Nothing built or committed.

## 2026-10-05 - D23F accepted; fast timing is the Modernized default

User accepted D23F ("absolutely beautiful ... responsiveness is literally
like PC accurate now") and made it the default: profile schema 26,
`cpu_timing` fast by default (older saved accurate switches once). New Todo
jobs D23G (finish the fast path: dispatch, overlays, interpreter, observers,
redraws) and D17S (auto frame-rate default capped at 120). Documentation,
commit and push not yet authorized. No next job selected.

## 2026-10-05 - D23F fast CPU timing: Needs playtest

New Modernized setting `--cpu-timing fast` (default accurate; Vanilla always
accurate): a purpose-built timing model for the recompiled game code,
force-included into the generated shards, leased from gameplay. Generated C
and hashed headers untouched (savestates still load). At 150%/120 Hz slots
1, 9, 10 keep 120 presents/s (accurate: about 40) with no shedding or pauses;
the emulation thread drops by about 40% at 100%. Candidate
`e0737a9f712aa5622d1b057ee5071e12c33cc9a18de8e6d40ee8eee31083f966`,
[note 105](105-d23f-fast-timing.md). The Vanilla route diverges from its old
captures with or without D23F (cause not established). Nothing committed.
Next: the user plays with `--cpu-timing fast --cpu-overclock 150` at 120 Hz
and decides whether fast becomes the default.

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

## 2026-10-05 - D23E added (enemy-area stutter)

New Todo D23E: stutter while walking left/right looking down at enemies in UI
slot 9, and walking the strip in UI slot 10 with `dnkroz`. Savestate hashes
are recorded in the job. Profile first; check overlap with D23C, D23D and
D17H. Not started. No next job selected.

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


## 2026-10-03 - D17A/B accepted and implementation published

Read [86](86-d17-acceptance-and-regression-baseline.md) first. D17A and D17B
are **Accepted**, explicitly closed after natural play at 120/180 FPS. Do not
reopen the broad parents for residual defects. The board has D17D-J and D18C
with current/historical private save identities. No next job was started.
120 is the primary quality baseline, 180+ supported high refresh, 240+
robustness, Unlimited stress/debug. Do not impose a 120 ceiling or destabilize
accepted mouse response, opening/club/apartment, idle eye, PS1 character or FMVs.

Important repository correction: `recomp/` source/tools/tests/configuration are
now tracked at the root. Never restore its blanket exclusion. Framework/UI are
pinned root submodules; build applies the complete tracked
`recomp/patches/time-to-kill-accepted-source.patch`. Old incremental patches
are historical only. Export future framework edits with
`python3 recomp/tools/local/export_runtime_patch.py`; unchanged gitlinks do not
save local modifications. Media, generated C, builds, saves, analysis, research
and extracted retail assets stay ignored. Original disc dump is now under game/.

Accepted executable and local snapshot hashes are in note 86. The local binary
is not replaced by the separate clean-source verification build. Do not infer
new 120 numeric benchmarks from the earlier 180 synthetic tests; user acceptance
is a natural-play result. A field clock near 60 is not the Unique image rate:
120 can show 120 freshly rendered views without 120 simulation updates.


## 2026-10-03 - accepted baseline and bounded movement polish

Read [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md)
first. User accepts the previous build as the first polished/enjoyable baseline.
Preserved executable/source: local `polish-baseline`; new private states:
`cards-user8` (UI 12 club Shift+W/S; UI 10 now train-platform ledge; UI 9 furniture).
Never reuse old slot descriptions without checking the current manifest.

Retained change: `run.py` defaults PSX_GL_PERF off except when
PSX_REPLAY_PROFILE=1; explicit overrides survive. Only native addition is
DNTTK_POSE_TRACE=1 capture/completion timestamps. Accepted renderer algorithms,
90 ms world delay, mouse sampling and audio remain unchanged. Faster phase
correction and 130 ms buffering were tested and removed/rejected.

Slot 12 remains open: roughly 67 ms capture intervals plus about 38 ms
composition time and advance scheduling exhaust the coherent snapshot pair.
Regular present intervals and fresh late mouse rotation do not imply smooth
world translation. Next work should address snapshot availability without
blindly adding movement latency or speculative geometry. Slot 10 can drain
audio under movement stress; do not declare crackle solved from slot 1's
zero-underrun result. Full train sequence and actual audio-device listening
remain unverified. D17D Todo tracks isolated furniture/closet/subway artifacts;
slot 9's board defect appears at 60 Hz too. No visual workaround shipped.

100 Python tests pass; real movement/jump/fire and opening/dancer mouse routes
recorded. Opening/dancer camera steps stay under one degree, approximately
179-180 images/s and zero underruns. Original 12 states match hashes/mtimes.
Build `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`,
codegen `8bab543c`. Game closed; no commits or player-setting changes.


## 2026-10-03 - save-driven camera stability candidate (D17A/B Needs playtest)

Read [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md)
first. New private states are `cards-user7`, UI N = file N-1: 1/2 slow,
3 subway peripheral culling, 4 improved opening, 5 early club, 8 minor slowdown,
10 later club. Old reports' slot meanings are obsolete. Never test against the
player's originals. Original hashes are in `warmup-baseline/save-manifest.json`.

Preserve these fixes: camera ownership no longer expires after four fields;
late redraws get two presents' grace and unfinished expirations inform the
readiness estimator; mixed opaque/dual-source batches retain original order;
GPU profiling avoids blocking query readback; pinned fetched SDL3 X11 events
map milliseconds into SDL nanoseconds; known render/kick/aim scratch is reserved
before workers fork. SDL repair is idempotent and fails closed on source drift.

Fixed 180 Hz is substantially improved in measured real 500 Hz mouse sweeps.
Unlimited remains a failing adaptive-scheduler stress case (114 ms stale camera
sample); do not report that mode as solved. Subway culling was not reproduced
in the bounded full-width capture route, so no claimed fix. Further campaign
visibility, actual audio listening and subjective mouse acceptance need the
user's test. Codegen remains `8bab543c`. Final executable SHA-256:
`28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
Final checks: 100 Python tests and four native suites pass; 2,848 GP0 and
3,056 RGBA comparisons are exact. Third-person club, kick/shot, pause/resume,
repeated loads and the Vanilla route pass their bounded checks. Original save
hashes/mtimes match. No commits or player-setting changes. Game left closed.


## 2026-10-03 - latest opening/club candidate (D17B Needs playtest)

Read [83-opening-responsiveness.md](83-opening-responsiveness.md) first; it
supersedes the older performance conclusions below. The player's saved first
person / Match Display / CPU 100% settings are unchanged. The final cold route
has zero audio underruns or failed workers, camera p99 6.0 ms and max 8.3 ms
from the first valid redraw. Final Match Display club turning delivers 179.5
distinct images/s, p99 6.18 ms and realtime emulation. Unlimited is adaptive,
not guaranteed to outperform a fixed target. Minor UI 8/9/11 geometry pops
remain open; UI 10 is the good comparison. Full campaign and audio listening
still need the user's playtest. Do not mark D17B Done without that acceptance.

Important fixes to preserve: per-image code-guard invalidation and reconstruction
after restore; exact initialized math shard; deferred-cycle-aware pipeline
deadlines; timestamped interpolation; ordered translucent batching; worker
refresh on new DLLs rather than invalidated movie functions; atomic worker
start timestamps and guarded timeout subtraction. The old restore bitmap clear
was unsafe and must not be restored as a performance shortcut. The math shard
uses normal byte validation. CPU 150% is a different stress workload and did
not establish the stock-CPU result.

Codegen `8bab543c`; known old player and intermediate saves explicitly
compatible, wrong hash/ABI rejected. Final binary SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
Runtime and generator patch reverse checks pass. The 98-test Python suite,
new compiled guard/deadline tests, GP0/RGBA comparisons and Vanilla route pass;
see note 83 for the old structural-test limitation and RAM/MMIO interpretation.
Nothing committed. Tests use private cards/profiles under local analysis.
The player build is left closed; launch with `python3 recomp/tools/local/run.py`.

## 2026-10-03 - renderer quality pass (D17A/B Needs playtest)

Read [82-renderer-quality-pass.md](82-renderer-quality-pass.md) first. The new
candidate fixes verified replay writes outside the saved live framebuffer
band and related depth restoration defects. Camera/visibility provenance and
Unlimited scheduling also received systemic fixes. D17C idle behavior remains
unchanged. No claim of stable 180 distinct images/s in the club: the worker
CPU tail remains the main measured limit (p95 about 21 ms). Final real-display
180 target gives about 85 distinct images/s in the turning sample; Unlimited
about 100. Treat the earlier "refresh or two" response claims as superseded.

Surface checks: 227 copy-on-write + 232 fallback, zero mismatches. Normal
visual route: 24 sequences without detected backsteps. Endpoint route: 409/412
pixel-exact; three street images differ during turning and lack generation
labels to classify. Do not call this a clean endpoint pass. Use private card
copies and the quality scripts; no player saves were used. Next performance
work should measure worker dispatch/cycle/cache costs and actual display/input
latency, not optimize average FPS. Campaign playtest is still required.
No-input RAM/MMIO equivalence is 630/630; driven 60/180 has two transient
RAM mismatches (629/631), with MMIO exact and repeated 60/60 exact. Keep that
open; earlier synthetic-driver sensitivity is a clue, not a diagnosis.
Vanilla fresh-card capture route passes. The final real-mouse sweep still
has long low-motion/repeat runs; the retained baseline does too, with nearly
identical p99 presentation intervals. The yaw-threshold heuristic cannot
prove image freezes or input latency. See the A/B details in note 82.
Nothing committed. The player build
is left closed; launch with run.py.

## 2026-10-03 - fifth pass: loss of control, late camera rebuilt (Needs playtest)

Ask the user for a fresh-game playtest: turning smoothness (start area and
club), loads of several saves in a row, holstered weapon + fire. Not
reproduced: UI 1 table artifact, UI 6 closet popping; ask for a screenshot or
a description. Measure turning with `realsweep.sh` + `holds.py` (STEP_S=0.001
PX=1) on the real display only. New switches: `DNTTK_LATE_SAMPLE`,
`DNTTK_LATE_LOG`; removed: `DNTTK_LATE_OVERRUN`, `DNTTK_LATE_JIT`. Nothing
committed.

## 2026-10-03 - D17A see-through fixed, D17B dancer jerk (Needs playtest)

Ask the user to replay UI 1-4 (`cards-user4` copies): doorway, statue,
apartment (no see-through), and sweeping across the dancers. Idle stability is
accepted: do not change the D17C eye. If turning still hitches, measure with
`realsweep.sh` + `staleclass.py` (real window and mouse); the remaining limit
is worker latency at 4x, and the next step would be reprojection with the
world and the HUD/weapon captured separately. New switches:
`DNTTK_LATE_OVERRUN`, `DNTTK_LATE_DUE_FREE`; lead default 16 ms. Nothing
committed.

## 2026-10-02 - D17A/B/C third pass (Needs playtest)

Ask the user to replay UI 1-5 (`cards-user3` copies) at Match Display: club
turning with the dancers (late camera, paced), idle stability and view bob
settings, closet, collar, apartment wall. If turning still feels heavy at
every 2nd/3rd refresh, next is present-time reprojection on the GPU. Developer
switches added: `DNTTK_LATE_PACING`, `DNTTK_VIEW_BOB` (numeric scale),
`PSX_HOST_DEPTH`, `PSX_HOST_UV`. Open: Duke stuck after the earlier UI 3 load.
Nothing committed.

## 2026-10-02 - D17A precise near geometry, D17B late camera (Needs playtest)

Ask the user to replay the strip club (dancers in view, turning) and the UI 4-9
states at Match Display. If it feels heavy, compare `DNTTK_LATE_CAMERA=0` and
the 150% CPU option. Developer switches: `DNTTK_LATE_CAMERA`,
`DNTTK_LATE_LEAD_MS`, `DNTTK_LATE_PEEK`, `DNTTK_LATE_JIT`, `DNTTK_LATE_TEST_YAW`,
`DNTTK_NEAR_MODE=conservative|full`, `DNTTK_NEAR_PRECISE`,
`DNTTK_NEAR_SPLIT_PX`, `DNTTK_NEAR_SPLIT_RATIO`, `DNTTK_REPLAY_NEAR_STATE`,
`DNTTK_REPLAY_LOG`. Open: Duke stuck after loading UI 3 not reproduced; cost on
the real display unmeasured (offscreen 93-180 distinct/s). Nothing committed.

## 2026-10-02 - D17C parked; D17A in progress

D17C parked (user saw no difference with the bob off). D17A: in-between
fidelity proven (alpha 0/1), near-wall flicker reproduced in the club at
16:9 first person and localized to the conservative world near clip's piece
ordering (documentation/81-d17a-instability.md). Next: insert the taken
polygon's pieces at its own position in the mesh order; test with
`recomp/analysis/d17-high-refresh/cwtest.py TAG` (STEER=1, CARDS=cards-club,
SLOT=1, OC=100, SCALE=2; target 0 back-and-forth) and `deferab.py` stills
(apartment/street, no vanished walls). Ask the user for F7 savestates at the
other reported places. Nothing committed.

## 2026-10-02 - D17 accepted; D17A/B/C added; D17C experiment (Needs playtest)

**Next session starts with D17C** (user's choice): get the user's playtest
verdict on `DNTTK_VIEW_BOB=off`, then keep it off, make it an option, or
redesign it. After that D17A or D17B.

D17 is Done (user-accepted). Next jobs on the board: D17A (instability and
black areas at the places the user listed), D17B (mouse-to-present latency
measurement, then a late camera update if the 30 Hz camera is the cause), D17C
(view bob). D17C test: `DNTTK_VIEW_BOB=off python3 recomp/tools/local/run.py`
(first person; environment only, not saved). Measure with
`recomp/analysis/d17-high-refresh/bobframes.py KEYS SECS` (per-field root and
eye height; STEER=1). Nothing committed.

## 2026-10-02 - D17 second playtest fixes (Needs playtest)

Recheck at Match Display (180 Hz): the street, the apartment (sink, wardrobe,
light switch), the subway past the EXIT sign and the power button should now be
smooth and stable; backtick console `fps` should show about 180 FPS, a Unique
count near it and Game 30. Developer switches added: `DNTTK_TEST_INPUT=FILE`
(scripted input), `PSX_REPLAY_COW=0` / `PSX_REPLAY_COW_CHECK=1`. Harness:
`steer.py`, `monitor.py`, `popsweep.py`, `seqcheck.sh`, `shift2.py`,
`CARDS=cards-apartment|cards-subway` (private copies of the D11B states).
Offscreen runs keep the GPU at its P5 clocks (no display), so they are a worst
case. Open: inherent snapping and original-mesh cracks; `ttk-input-test` needs
a live desktop session. Nothing committed.

## 2026-10-02 - D17 playtest review fixes (Needs playtest)

Recheck the first playtest's problems at the saved rate (`run.py`; the profile
has 120): the strip-club street and other heavy outdoor areas should now be as
smooth as the sewer with no audio artifacts; walking through doors should not
show other rooms; the intro and level movies should look like 60 Hz (no
flicker, normal brightness); after a first-person quick kick (Q) the leg should
be gone. Player sessions now run with `PSX_FORENSICS=0` (launcher; use
`--diagnostics` or `PSX_FORENSICS=1` for the forensic rings). New developer
switches: `PSX_GL_STATE_CACHE=0`, `PSX_PROF=FILE` with `PSX_PROF_CALLERS=1` or
`PSX_PROF_REPLAY=1` (profiler, `symprof.py`), `DNTTK_ROOM_WALK=always|off|probe`,
`DNTTK_TEST_KICK=<frames>` (with `DNTTK_TEST_DRIVE`). Harness additions in
`recomp/analysis/d17-high-refresh`: `slotscan.sh` (per-slot coverage and load),
`mipsdis.py` (static code from the generated C), `genpatch.py` (regenerates the
runtime patch). Open: vertex snapping and affine warp with small camera steps
(geometry correction off), third-person camera lerp around wall corners, fewer
in-betweens than asked above 120 Hz in the heaviest scenes. Nothing committed.

## 2026-10-02 - D17 high refresh rate (Needs playtest)

Playtest on the 180 Hz monitor: `run.py --frame-rate display` (saved;
`--frame-rate 60` returns to the original). Check: camera turns, running and
enemies look smoother than at 60 with no change in game speed; no tearing in
windowed and fullscreen (if exclusive fullscreen tears, try
`PSX_REPLAY_VSYNC=1`); `--frame-rate 30` and `120/144/240` also behave;
first person, widescreen, menus, movies and level loads; any flicker between
real and in-between frames (a mismatch) or stutter in busy scenes. Developer
switches: `DNTTK_FRAME_INTERP=off` (pacing only), `DNTTK_INTERP_STEPS`,
`DNTTK_REPLAY_BUDGET_MS`, `DNTTK_REPLAY_SLACK_MS`, `DNTTK_REPLAY_WORKERS`,
`DNTTK_REPLAY_TEST=alpha0|alpha1` (fidelity), `PSX_FP_RAM_HASH=1`
(equivalence). Harness: `recomp/analysis/d17-high-refresh` (`d17.py`,
`trip.sh`, `equiv.py`, `cadence.sh`). Runtime patch
`time-to-kill-zzzzzzzzzz-render-replay.patch` (new files render_replay.c/h,
render_worker.c). Regenerated hooks: `0x80026164`, `0x800632B0`,
`0x80031D10`, `0x80032E78`, `0x80031C14`, `0x8001FBA0`. Open: driven-input
equivalence at 180/unlimited (one pad sample), vsync on the real display,
moving room geometry. Pre-existing: `ttk-near-test` link error
(`frame_trace_*`). Nothing committed.

## 2026-10-01 - D08J1 accepted (Done)

User playtest: "genuinely working solidly." Committed. Next job: user's choice.

## 2026-10-01 - D08J1 overhead ladder grab (Needs playtest)

Playtest at save slot 6: hold E and run (or walk) at the ladder; Duke should
leap and grab it with no Space, then W climbs. Also: stop under it and tap E;
start a little to one side; try with a weapon drawn (E stows first; pressed
late while running, Duke stops at the wall and then leaps straight up).
Without E he should stop at the wall as before. Tuning:
`ladder_leap_distance()` windows (750/800/450), `leap_side` (180),
`leap_step` (70), rise limits 300..1400 in `recomp/src/ttk/ladder_top.inc`;
`DNTTK_TRAVERSAL_TRACE=1` logs `ttk-ladder-leap`. Scripts:
`recomp/analysis/d08j1-ladder-run` (`sweep.py`, `leap2.py`, `regress.sh`).
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Nothing committed.

## 2026-10-01 - D08Z accepted (Done)

User: "i love it. lock it in." Player profile stays on `jump: manual`
(assisted remains the default for new profiles). Next job: user's choice.

## 2026-10-01 - D08Z manual jump style (Needs playtest)

Playtest with `run.py --jump manual` (saved; `--jump assisted` returns to the
default). Check: running jumps leave on the press, the slot-5 gap by timing,
late presses just after an edge, steering with the mouse + W and braking with S
mid-air, quicker standing jumps, E grabs/mantles still work. Tuning knobs:
`air_accel_fraction` (0.14), `manual_edge_grace` (14), `quick_takeoff_scale`
(3) in `recomp/src/ttk/manual_jump.inc`; `DNTTK_AIR_CONTROL=0..3` scales the
steering without a rebuild. Analysis scripts: `recomp/analysis/d08z-manual-jump`
(`measure.py`, `gap.py`, `pit.py`, `regress.sh`). Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`.
Nothing committed.

## 2026-10-01 - D08Y accepted (Done)

User confirmed the slot-5 jump and that stutter, audio slowdowns and freeze
frames are fixed (round 5: leased 150% CPU overclock, lip-launch jumps, jump
mantles, budgeted reach retries). Nothing committed yet. Next job: user's
choice; D08Z (optional manual modern jump) is in the backlog. User found
`--cpu-overclock 100` and `150` equally playable; 150 stays the default.

## 2026-10-01 - D08Y round 5: overclock leased to gameplay (Needs playtest)

Boot/loading audio slowdowns at 150% fixed: the overclock is renewed from the
player update and lapses outside gameplay; a safety net pauses it for 5 s if
emulation falls behind (`[TTK cpu]` log line). Binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`. If the user still
hears slowdowns, check the session log for `[TTK cpu]` lines first.

## 2026-10-01 - D08Y round 4: emulated CPU overclock (Needs playtest)

Playtest: play normally (first person, 16:9) for a few minutes, slot 5 and
elsewhere; freezes while running should be gone. The profile migrates to
schema 19 on launch (backup kept) with `cpu_overclock` 150. If a scene still
drops, try `run.py --cpu-overclock 175` and report it; `--cpu-overclock 100`
restores the original speed. Measure performance on the real GPU, not Xvfb:
SDL offscreen + `DNTTK_TEST_DRIVE`, see documentation/10-tooling.md and
`recomp/analysis/d08y-gap-jump/gpuperf.py`. Runtime change:
`recomp/patches/time-to-kill-zzzzzzzzz-cpu-overclock.patch`. Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`.
No commits until asked.

## 2026-10-01 - D08Y round 3: smoothing (Needs playtest)

Freeze frames during E reaches and the jump-mantle pop fixed; binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`.
Check: E jumps at slot 5 and other ledges feel smooth, no hitch, no snap into
the mantle. Tuning: `reach_core_budget`/`reach_extra_budget` and the try lists
in `ledge_reach_retry()`, glide in `mantle_glide()` (`ledge_reach.inc`).
Diagnose with `DNTTK_FRAME_TRACE=1` (frames over 25 ms or 3 ms of hooks).
D08Z (manual modern jump) is backlogged. No commits until asked.

## 2026-10-01 - D08Y round 2 (Needs playtest)

Playtest: slot 5, run at the gap and tap Space anywhere in the last few steps:
Duke should leap from the very lip and land (no E needed). Check the hitch
after E jumps is gone, and that bed/couch jumps in the apartment still fire
instantly. Also any other gap or pit: jumps pressed near it now leave from the
lip like the original. Code: `0x800780b4` hook (`modern_controls.cpp`, 768
threshold), `edge_jump_queued()` + `pc_input.cpp` hold (40 updates),
`ledge_reach.inc` (D08Y mantles), `shortcuts.inc` word copies. Private scripts:
`recomp/analysis/d08y-gap-jump` (`sweep.py`, `mjump.py`, `furniture.py`,
`latency.py`, `regress2/`). Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. No commits until asked.

## 2026-10-01 - D14 widescreen accepted (Done)

User: "im very happy with it! i accept!" Committed with the case study
(DuckStation stretches 2D by 4/3; native-wide does not). Next job: user's choice.

## 2026-10-01 - D14 widescreen (Needs playtest)

Modernized defaults to native 16:9 (profile schema 18, `run.py --widescreen
off|16:9|16:10|21:9|auto`). Fixes: dropped `nw_hud_corners` (it shifted world
polygons: the off-centre, torn preview), widened TTK's portal root rectangle
`0x800d2210` at the render's projection load, D11B near clip in third person,
HUD layout `0x800dd778` moved out while the status bar draws (new hooks
`0x8008BA30`/`0x8001FC44`, regenerated). Binary `3f726b91...af2c`. Next: user
playtest across levels at 16:9 (and 21:9/auto if wanted); real-GPU fps.
`build.py` stops at the runtime-patch check (`time-to-kill-stopped-window.patch`);
the generate/build steps were run directly (04-build-and-run.md).
DuckStation testing: the flatpak ignores an `XDG_CONFIG_HOME` override and uses
the player's config (resume-on-exit on); use the portable copy in
`recomp/analysis/d14-widescreen/r2/ds/app/bin` (`portable.txt`).
Nothing is committed yet.

## 2026-10-01 - Session locked in; next: D14

All session work accepted (D08U, D08W, D08A3, D08X Done). D14 16:9 preview is
off centre with geometry holes; start the next session from the ordered steps
in documentation/76-d14-widescreen-first-pass.md (A/B `native_wide = false`
first). Standing permission: close the player's open game and continue.

## 2026-10-01 - D08X accepted (Done)

User: "thats it, fully accepted this!!!". Binary `60161f81...369c`. Nothing committed yet.

## 2026-10-01 - D08X flush hang (Needs playtest)

E-jump ledge catches settle to the shimmy-aligned hang height (ledge top +
456). Binary `60161f81...369c`.

## 2026-10-01 - D08X crate-to-crate mantles (Needs playtest)

Jump mantles onto any climbable object are proximity-based with the original
line-up; crate-to-crate 18/18 at any angle to 45 degrees. Binary `ccb4eb2a...0107`.

## 2026-10-01 - D08X angle forgiveness (Needs playtest)

E reach retries the original acquisition with the heading turned +-25/+-51
degrees as well as lifted. Binary `8bb01385...1701`. Playtest: slot 11 crate
top to the ledge in front at an angle.

## 2026-10-01 - D08X higher grab confidence (Needs playtest)

E reach retries the original acquisition up to 480 higher, easing Duke into
the hang. Binary `43eda09c...0f61`. Playtest the drained-water ledges.

## 2026-10-01 - D08X first-tier crate hang (Needs playtest)

A jump catching a stacked crate into the original pole-style hang (mode 7,
A/D circled Duke round it) now lets go at once. Binary `c1c49119...6fe7`.

## 2026-10-01 - D08X crate hang fix (Needs playtest)

Fixed: object hang toggling flag-table bits broke the identity guard (all
Modernized controls lost); stuck crate hang now lets go (S, or W without a
climb). Binary `bf368b68...bd45`. New log line `[TTK identity] guard N` names
any future guard failure.

## 2026-10-01 - D08X crate mantle (Needs playtest)

Boxes room (private copy of user UI slot 11): an E running jump into a crate now
starts the original mantle mid-jump instead of bouncing. Binary `9bade2a5...ba9c`.
Playtest: Shift + W + Space with E held into the crates from a few steps back.
Standing permission (2026-10-01): close the player's open game and continue.

## 2026-10-01 - Playtest: D08U, D08W, D08A3 Done; D08X boxes follow-up

User accepted D08U, D08W and D08A3. D08X ledge grabs accepted; next work is the
boxes room (private copy of user UI slot 11 in recomp/analysis/d08x-boxes):
Shift + W + Space with E held should mount a crate mid-jump instead of bouncing.
16:9 preview: `DNTTK_WIDESCREEN=16:9 DNTTK_GAME_CONFIG=recomp/analysis/d14-widescreen/game-16x9-preview.toml run.py` (Modernized profile).

## 2026-09-30 - Autonomous chain: D08U (Needs playtest), then D08X, D08W, D08A3, D14 start

User authorized an unattended chain: D08U, D08X, D08W, D08A3, then start D14;
no commits; a combined playtest later. D08U: E at a ladder top lowers Duke onto
it (host blend over the original 156 transfer); S climbs down and steps off.
Binary `6129f2ab...5683`. Details: documentation/72-d08u-ladder-top.md.
D08X (Needs playtest): E held through a jump arms the original reach at
takeoff, so close ledges are caught. Binary `bd153dd5...65b3`. Details:
documentation/73-d08x-ledge-grab.md.
D08W (Needs playtest): shallow-water jumps follow the held direction.
Binary `7ca63455...3328`. Details: documentation/74-d08w-wade-jump-direction.md.
D08A3 (Needs playtest): switcher icons are now TTK's own HUD art from
/DATA/FONTS.RAW (pack rebuilt by build_ttk_inv_icons.py). Details:
documentation/75-d08a3-ttk-inventory-icons.md.
D14 (In progress, first pass): inert 16:9 activation plugin and package;
experiment shows TTK's 4:3 culling gaps at the widened edge; no player option
yet. Details: documentation/76-d14-widescreen-first-pass.md.
Final binary for the playtest: `db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`.

### Playtest checklist for this chain

1. D08U: walk to the edge of the sewer walkway next to the ladder (slot 12),
   press E. Duke should stow, turn and lower onto the ladder smoothly. Hold S:
   he climbs all the way down and steps off, pistol back out. Also try the top
   of the first alley ladder. Walking or running off the edge without E must
   still drop as before.
2. D08X: hold E and jump (W + Space) at a ledge within reach, for example the
   wall at the side of the slot-12 walkway. Duke should grab and pull up.
   Without E it bounces as before. Try running jumps and ledges elsewhere
   (apartment exterior, crystal-2); armed, hold E a moment before jumping.
3. D08W: in the subway shallow water (slot 1), A/D/S + Space should jump in
   that direction, including after running forward. Also the crystal-2 wade.
4. D08A3: press [ or ] to show the switcher; the icons should be TTK's own HUD
   art (health cross for medkit, jetpack, Bio Mask skull, goggles), green %
   and frame unchanged.
5. General: ladders, mantles, run-jump ladder transfers and Vanilla feel
   unchanged.

## 2026-09-30 - D04A accepted (Done)

User: "perfect, accept" on binary `198673f5...ded68e`. Escape pauses with a
free cursor and resumes with automatic recapture. Next job: the user chooses.

## 2026-09-30 - D04A Escape mouse release (Needs playtest)

Ad hoc user job: Escape sometimes left the mouse captured. Recapture raced a
missed Start tap; now Escape pulses Start and holds auto-recapture until the
pause stops gameplay offers. Only `recomp/src/ttk/pc_input.cpp` and
`recomp/tests/local/pc_input_native.cpp` changed. Needs the user's windowed
playtest; look for `Mouse released (Escape)` in the session log.
Playtest fix: the resume Escape no longer holds recapture (it had left the
mouse free after resuming). Rebuilt: binary `198673f5...ded68e`.

## 2026-09-30 - D07D, D08T1, D08V accepted (Done)

User: "awesome. accept". All three Done on binary `123c7910...693e`. D08X
(hold-E ledge grab) is now unblocked. Next job: the user chooses from the board.

## 2026-09-30 - D08V implemented (Needs playtest); D07D, D08T1, D08V await playtest

Camera-only lease through mantle/hang/pull-up, post-mantle 0/8 and 0/9 gait
and unowned 107/108 falls; `traversal_camera_ready()` replaces the tank
fallback there. Repro and sweep scripts in `recomp/analysis/d08v-sewer`.
Binary `123c7910...693e`. D08X (depends on D08V) stays blocked on the D08V
playtest. Next job: the user chooses from the board.

## 2026-09-30 - D08T1 implemented (Needs playtest)

Hold Grab (RMB/Alt) grabs pushables; E mantles them like any climbable.
New generated entry hook `0x80051890` (E's Cross masked at the idle grab);
`0x80051CF0` mask only while Grab owns Cross. Schema 17 (`grab`, `grab_alt`;
`original_aim` Unbound). `build.py` stops in the stale runtime patch-stack
check; regenerate with `psxrecomp_cli.py generate` directly (see
documentation/70). Binary `66097cf8...3015`.

## 2026-09-30 - D07D implemented (Needs playtest)

Red dot off by default in Modernized (schema 16, one-time migration with a
backup). `marker_hook` owns the dot at enqueue `0x8002BC18`/RA `0x80033DB0`
using S4 == Duke and the frame RA `0x8003543C`; no entry lease or LEVEL00
gate. Tests: `ttk-aim-test`, Python 85 OK, `pc_input_probe --controls red-dot`
on/off runs. Playtest: look for the dot with targets in third/first person,
held aim, jetpack, swimming, scripted cameras. The legacy `aim-options` route
fails in stale sampler/D04 checks (not changed). Binary `db5bf964...ec67b`.

## 2026-09-30 - D07D added (no red autoaim dot in Modernized)

New Todo D07D: the user finds the original red autoaim dot next to the modern
crosshair confusing. `red_dot` is off by default in every Modernized profile
(with profile migration), covering every state where `marker_hook` currently
steps aside; the option stays, and Vanilla keeps the dot. Next job: the user
chooses from the board.

## 2026-09-30 - D08T1 added (E always mantles, hold RMB to grab)

New Todo D08T1 from a user brief: pushable objects must not change traversal
controls. E always mantles; a new rebindable held Grab / Manipulate action
(default right mouse in Modernized, where precision aim is not needed; Vanilla
keeps it; Alt is the proposed second binding and the legacy-aim grab)
grabs, W/S push/pull, release lets go. Audit the Mouse2 /
`original_aim` paths first. Brief:
`documentation/69-d08t1-grab-manipulate-brief.md`. Next job: the user chooses
from the board.

## 2026-09-30 - D08X added (hold-E ledge grab)

New Todo D08X: in Modernized, holding E while jumping toward a ledge at
grabbing height should reliably catch it and mantle up, like run-jump + E on
ladders (D08J). Trace the original ledge eligibility (anims 134-142, 6/7
hanging dispatches) before choosing a fix. Next job: the user chooses from
the board.

## 2026-09-30 - D11C Done (user accepted)

User: "duke's head is back, mark as complete! well done". Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`. Next
job: the user chooses from the board.

## 2026-09-30 - D11C stale head-hide reclaim (Needs playtest)

`first_person_draw(duke,eye)` clears an unowned bit 0 on Duke's joint 9
record at Duke's draw entry (debug `fp.head_reclaims`), then hides it again
only in first person. The original never sets that bit (static scan of the exe
and the 30 unique overlays). A private slot-12 copy loads with the head drawn in
third person; first person still hides it. Awaiting the user's slot-12 check;
then D11C can be Done. Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

## 2026-09-30 - D17 expanded to high refresh rate rendering

User brief for Match Display / 30-240 / Unlimited rendering without changing
gameplay timing was folded into D17 (still Todo, depends on D01, D13). Brief:
`documentation/68-d17-high-refresh-brief.md`. Audit and plan first. Next job: the
user chooses from the board.

## 2026-09-30 - D16A cancelled; D08A3 added

D16A (HRP research) is Cancelled by the user; original TTK assets are the
direction. New Todo D08A3: extract original TTK medkit/biomask/jetpack/steroids
(and goggles) icons to PNG and rebuild `ttk-inv-icons.pack` so the switcher no
longer uses Duke3D tiles. Next job: the user chooses from the board.

## 2026-09-30 - D13 Done (user accepted)

User: "correct correct correct, D13 is good. I'd say let's approve it." Final
binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`. New backlog from the D13 playtest: D08U (top-of-ladder mount),
D08V (sewer mantle lease), D08W (subway shallow-water sideways jumps), D10B
(tight-space camera transparency/doors), D11C (savestate stuck head-hide
flag, slot 12). Next job: the user chooses from the board.

## 2026-09-30 - D13 F11 fullscreen key (Needs playtest)

F11 is the only default fullscreen key (`host_keymap.c`); Alt+Enter and Ctrl+F
are unbound. The D13 patch was regenerated and checked forward/reverse. Launcher
text says "F11:". Awaiting the user's confirmation of windowed start, F11 and
the remembered state; then D13 can be Done. Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D13 windowed default + remembered display (Needs playtest)

Schema 15 (`fullscreen_mode`), runtime `--fullscreen-mode` and
`--presentation-state` (written in `shutdown_runtime`), and
`absorb_presentation_state` / `save_presentation_state` in the launcher. The
user's profile is set to windowed at 4x (backup
`recomp/config/player-profiles.json.recovered-2a7d56cb...`). Awaiting the user's
confirmation of windowed start, Alt+Enter and the remembered state; then D13
Done. Binary `7bd2a001a295330397d45c73ca9bc2ca0bc575c0d889bab3259d06778b3a3d1c`.

## 2026-09-30 - D13 playtest follow-up (Needs playtest)

User confirmed 4x at 60 fps; Modernized default now 4x. Fixed Alt+Enter
(`host_keymap.c` exact modifier compare, now `mod_groups`) and SDL3 exclusive
(`psx_apply_fullscreen_display_mode` in main.cpp); both in the D13 runtime
patch, which was regenerated and checked forward/reverse. Awaiting the user's
confirmation, then D13 can be Done. New backlog from the same playtest: D08U
(top-of-ladder mount), D08V (sewer mantle lease), D10B (tight-space camera).
Slot 12 is `saves/local-play/openbios/state_800AB6FC_slot11.pst`; a private
copy is in `recomp/analysis/d13-resolution/cards/openbios/`
(`probes/start.sh RUN 11 ...`). `ptrace` attach is blocked on this machine, so
use temporary log lines instead of gdb attach. Binary `ecc9328065b8a6c3311423e1640936b1835e1517d35b923605609595e2f3a95d`.

## 2026-09-30 - D13 resolution and display (Needs playtest)

Runtime patch `recomp/patches/time-to-kill-zzzzzzzz-presentation-cli.patch`
adds `--internal-scale/--display/--window-width/--output-filter`; profile
schema 14 (`player_profiles.py`) saves them per profile and `run.py` passes
them (`presentation_args`; software forces 1x via `effective_scale`). Test
harness: `recomp/analysis/d13-resolution/` (`probes/start.sh RUN SLOT
[run.py args]`, `stop.sh`, `measure.py RUN`; port 9193, display :93, `WM=1`
starts metacity for fullscreen checks). `screenshot_hires` does not see the
OpenGL hr FBO; use window captures. Awaiting the user's desktop test (fps at
2x-4x on the GTX 1080 Ti, exclusive fullscreen). Pre-existing: patch-stack
check fails on `host_osd.c` drift vs the inventory-strip patch (see the D13
note). Binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`.

## 2026-09-30 - D08L Done (user accepted)

User: "im happy with that!" Final binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`. Next job: the user chooses
from the board (remaining Todo includes D09, D13, D19B, D20).

## 2026-09-30 - D08L inertial edge run-off (accepted)

Running falls over 768 that directly follow a ground stride now get the D08F
short-fall lease (`recomp/src/ttk/terrain.inc`, previous-animation gate 72..79,
`large_fall`, counter `run_offs`); all departures capped 10000 running / 2048
walking in the `0x8003ebf4` ballistic hook (`modern_controls.cpp`). Note: the
adapter never sees mode 9/0 on the first fall update (already 9/9), so do not
gate on it. Test state: `recomp/analysis/d08l-edge/apt-cards` slot 1
(apartment), 2 (on the bed), 3 (fire-escape platform, run west off the end);
probes port 9191, display :91 (`probes/start.sh`, `stop.sh`, `tr.py`).
Previous binary `before.bin` (`1452391c...`). Open follow-ups: other
exterior ledges, very high falls. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D12A Done (user accepted)

User: "its done! accepted. ... this has been yet another amazing feat of engineering." Final binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`. Next job: the user
chooses from the board (D12A is closed; remaining Todo includes D08L, D09, D13).

## 2026-09-30 - D12A thigh 0.13, E never kicks, Q chains (accepted)

Thigh drop default 0.13 (user's choice). `kick_convert` kicks only with the
attack held (E suppressed); Q queues during a kick, held Q repeats. Awaiting
the user's test. Binary `1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D12A follow-up (Needs playtest)

After the user's first playtest: sphere at the crosshair trace hit
(`view_segment_query` in weapon_aim.cpp, within 480 horizontal) else 340;
held attack with Boot kicks in first person while moving; thigh drawn end
for end from the knee (`DNTTK_FP_KICK_THIGH=0` off). Garbage bags: alley
state in `recomp/analysis/d12a-kick/alley-cards` (slot index 4), bags
`0x801dcf44`/`0x801dcd64`. Private copies of the user's slots 0-4 in
`recomp/analysis/d12a-kick/user-cards` (index 3 = street near the subway
stairs; the pallet was not identified). Open: thigh framing options, the
pallet. Binary `e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`.

## 2026-09-30 - D12A first-person quick kick (Needs playtest)

Q in the eye view = host quick kick (`recomp/src/ttk/kick.inc`); Boot-selected
original requests (112..115) are converted at `0x800493a4`. Hit = original
sphere `0x800a979c` along the view on each update of frames 8..20; leg =
joints 14..17 from 115 poses via the D12 private-matrix path. Framing knobs
`DNTTK_FP_KICK`, `DNTTK_FP_KICK_TILT`, `DNTTK_FP_KICK_REACH`,
`DNTTK_FP_KICK_FREEZE`. Test state: `recomp/analysis/d12a-kick/fresh-cards`
savestate slot 3 (first street, god mode, pig cop ahead); probes use port 9181
and display :81 (`probes/start.sh`, `stop.sh`). Previous binary `before.bin`
(`941593c0...`), sources in `before-src/`/`after-src/`. Open: user playtest of
feel/framing/damage; knockback (enemy reaction 84) not reproduced. Binary
`93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`.

## 2026-09-30 - D08T pushable objects (Done, user-accepted)

E grabs / W,S push-pull / E lets go / Space climbs, for objects flagged
`0x08000000`; see `documentation/64-d08t-pushable-objects.md`. Test state:
private copy of the user's slot-5 alley savestate in
`recomp/analysis/d08t-push/cards` (slot index 4), sources before the change in
`.../before-src/`, previous binary `before.bin` (`65226a9d...`). New generated
hook `0x80051CF0` (regenerated with `psxrecomp_cli.py generate` as in the D12
note, then `cmake --build --preset local-dev` and `build_movie_overlay.py`).
Probes use port 9171 (`PORT` env). User playtest: "it works so much better than the original now. this is it rock solid. confidence level is very high." Possible
follow-up: other pushable objects/maps; host pre-alignment if the line-up angle
ever feels strict. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

## 2026-09-30 - D12 Done (user accepted); next D12A

User: "finally, we can mark this as accepted!!" First-person weapons are Done
on binary `65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next job chosen by the user: D12A, keep the kick (Q) in first
person. Per-weapon framing lives in the offset/tilt/scale tables in
`recomp/src/ttk/first_person.inc`; `DNTTK_FP_WEAPON_SLOT<n>`, `..._TILT<n>` and
`..._SCALE<n>` override them for one launch.

## 2026-09-29 - D12 framing tweaks; next job D12A

User accepted most weapons as they look; shotgun, gatling and throwing blades
retuned (per-slot tilt/offset tables in `first_person.inc`). Next job chosen
by the user: D12A, keep the kick (Q) in first person.

## 2026-09-29 - D12 twin cannons (Needs playtest)

Slot 8 (key 5) framed after the Devastator references; HUD now over the
weapon (viewmodel in OT slot 1). Tune per weapon without rebuilding:
`DNTTK_FP_WEAPON_SLOT8=x,y,z`, `DNTTK_FP_WEAPON_TILT8=p,y,r`,
`DNTTK_FP_WEAPON_SCALE8=f`. Club test state: private copy of the D11 route
cards in `recomp/analysis/d12-first-person-weapons/club-cards` (slot 1, main
room facing the mirror after a 180-degree turn). Binary
`b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`.

## 2026-09-29 - D12 first-person weapons (Needs playtest)

Real hand + weapon meshes drawn as a viewmodel in first person; see
`documentation/63-d12-first-person-weapons.md`. Test state: private copy of
the street savestate in `recomp/analysis/d12-first-person-weapons/cards`
(slot 1; `PSX_LOAD_SLOT=1`), sources backed up in `.../before/`. `build.py`
still stops at the runtime patch drift: after changing hooks run
`psxrecomp_cli.py generate` with `PSXRECOMP_GAME`/`PSXRECOMP_BIOS` set to
`build-recompiler/`, then `cmake --build --preset local-dev` and
`build_movie_overlay.py`. Before launching an isolated instance, stop any
earlier one on port 9177 (a stale instance answered one survey). Re-survey
seeds with `DNTTK_FP_WEAPON_TRACE=1` (fire each weapon standing, level view)
if placement math changes. Next: user playtest of placement/size/feel; then
holster/draw animation, left hand, projectile origin. Binary
`1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`. No commits.

## 2026-09-29 - D11B Done (user accepted); jetpack sprite confirmed

User: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available"). Next first-person job: D12 (hands and weapon). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

## 2026-09-29 - D11B occluder fade in first person (Needs playtest)

User: conservative default is the best-looking; keep `DNTTK_NEAR_CLIP=0` as
Vanilla rendering and `full` for research. Invisible subway door = original
occluder fade (`0x80031fa0` loop, overlap test `0x8002ee50`, fade distance
camera+0xa0, Duke rect camera+0xa8). New hook `0x8002EE50` hands the test an
empty rect in the eye view; `DNTTK_FP_OCCLUDER_FADE=1` restores it. Both are
logged as D19 menu toggles. Private copies of the user's F7 slots in
`recomp/analysis/d11b-near-clip/subway-cards` (file slot 01 = F7 slot 4,
02-04 = F7 slots 1-3). Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`. No commits.

## 2026-09-29 - D11B third pass: conservative default (Needs playtest)

User saw dotted black lines on floors at 1080p with the subdividing mode. Test
at 1080p by capturing the window (`g.wshot`, `XRES=1920x1080` in
`recomp/analysis/d11b-near-clip/probes/launch.sh`, `look.py`); the runtime's
`screenshot_file` shows the software raster, not the OpenGL window. Default is
now `conservative` (clip only what the original gets wrong, no subdivision,
whole-polygon sort); `DNTTK_NEAR_MODE=full` keeps the subdividing mode;
`DNTTK_NEAR_CLIP=0` is off. Street test state:
`recomp/analysis/d11b-near-clip/street-cards` slot 1 (fresh game, street
spawn). Binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`. No commits.

## 2026-09-29 - D11B second pass (apartment artifacts; Needs playtest)

User reported comb/popping on the apartment wardrobe ("closet"). Fixed by
average-depth piece sorting, a -4 OT prop bias (actors excluded via
`first_person_actor_drawing()`), prop takeover to 3072, outward rounding of new
corners, and a per-frame packet budget (the render arena is a 139,744-byte
ring; see `documentation/62-d11b-near-clip.md`). Apartment test state:
`recomp/analysis/d11b-near-clip/apartment-cards` slot 1 (debug savestate on
private cards); probes in `recomp/analysis/d11b-near-clip/probes/`
(`closet.py`, `walkseq.py`, `roam.py`, `apt_route.py`). Compare with
`DNTTK_NEAR_CLIP=0`; `DNTTK_NEAR_TINT=1` colors host pieces by renderer. Binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`. No commits.

## 2026-09-29 - D11B near-wall clipping (Needs playtest)

Code: `recomp/src/ttk/near_clip.cpp` (+ `near_clip.h`), hooks `0x80010000`
and `0x80011020` added to `game.local.toml` and regenerated with the built
recompiler (`PSXRECOMP_GAME=build-recompiler/psxrecomp-game ... psxrecomp_cli.py
generate`), wired through `first_person_view_live()` /
`first_person_duke_drawing()` in `modern_controls`. New native target
`ttk-near-test`. `DNTTK_NEAR_CLIP=0` turns it off for A/B.

Findings worth keeping: the level is drawn by two hand-written renderers at the
start of the executable (world `0x80011020`, object `0x80010000`); the GTE RTP
ring's `ra` for them is garbage (both use `$ra` as a scratch register - 0x100 is
H). `0x8002ef90` is a horizontal grid surface, not walls. Near the eye one
texel spans 15-25 pixels, so any host subdivision must cut on whole-texel lines
or straight texture lines kink.

Probe helpers: `recomp/analysis/d11b-near-clip/probes/` (`sweep.py TAG X Z YAW`
places Duke on the private route cards and sweeps; `perf.sh`; private port
9177, display :77). Wall spots: club side wall (5486, -4096) yaw -61.3; entry
corridor (5442, -800) yaw -90. D11 binary backup
`recomp/analysis/d11-first-person/d11-accepted-dd7b85b4.bin`, toml backup
`game.local.toml.before-d11b`. Binary
`d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa`. No commits.

## 2026-09-29 - D11 first-person prototype (Done, user accepted)

Code: `recomp/src/ttk/first_person.inc` (header comment lists every original
address it relies on), wired in `modern_controls.cpp` (`fp_blend`,
`eye_jump`, `orbit_constraint` blend, hooks `0x800348d8`, `0x8001ca4c`,
`0x8002a038`, `0x800b4d9c`), `pc_input` action `camera_view` (P) placed after
`camera_shoulder` (wire payload now 39 entries), profile schema 13 `view`,
`--view`. New `game.local.toml` hooks `0x800348D8`, `0x8002A038`,
`0x800B4D9C`; regenerated with the built recompiler (one file per hook); the
launcher rebuilt the movie shard.

Findings worth keeping: camera `+0x42` is H = 160/tan(angle at `+0x40`/2) =
386; the camera sits H/2 behind its anchor; the world renderer flags vertices
nearer than H/2 and the joint draw skips joints nearer than H. Duke's model
record at `[player+0x40]`: 19 joints, record `+0x44+0x28*j`, byte 0 bit 0 skips
the joint, byte 2 level; joint 9 is the neck/head. The original camera eases
per axis through `0x8002a038` with rates from `0x800c0d4c`. Save states are
taken at scheduler boundaries (VSync waits), but the debug server reads mid
frame, so a debug read can see the relaxed 768 word; the runtime repairs any
captured value.

Probe helpers are in the session scratchpad (`g.py`, `route.py`,
`wallcheck.py`, `perf.sh`, `launch.sh`/`start.sh`/`stop.sh`; private port 9177,
display :77). Fixture copies: `recomp/analysis/d11-first-person/` (profile,
cards from D10, `route-cards` slot 1 = club main room, first person; the D10
binary backup `d10-accepted-84669bb6.bin`). Developer override
`DNTTK_FP_PROJECTION=160..386` for FOV/near tests. User accepted D11 after extended play. Next:
D11B (near-wall polygons) and D12 (hands and
weapon) follow; D11A was cancelled (the P toggle suffices). Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`. No commits.

## 2026-09-29 - D10 Done (user accepted); D08S cancelled

User: "it's done, fully accepted" after about five minutes of play. D11 (first-person
prototype) is now ready. No commits.

### D10 recenter / shoulder / saved distance implementation notes

D08S cancelled at the user's request (keep the scope text; may revisit). D10
options pass: actions `camera_recenter` (V) and `camera_shoulder` (H) sit
before `weapon_previous` in `input_bindings.def`, so they are edges, not guest
commands. Wire payloads now have 38 entries; the native test fixtures were
updated. The camera side is in `orbit_begin`/`orbit_constraint`
(`modern_controls.cpp`, debug JSON `shoulder_offset`, `recentering`,
`recenters`, `rest_pitch`). Persistence: runtime side file
`<settings>.camera-state`, merged by `run.py` after exit and at launch; profile
schema 12. The probe is in the session scratchpad (`d10_probe.py`; phases
main/relaunch/nudge/strafe; `SLOT=1|2`, `EXTRA` launcher flags). The fixture
copy is `recomp/analysis/d10-camera`. The debug `quit` returns "emu busy or
frozen"; SIGTERM the game child to exit cleanly. Duke's heading equals the view
on foot, so any recenter yaw work must target states where facing diverges.
Binary `84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`. No
commits.

## 2026-09-29 - D08R Done; D08Q1 faster Modern Ctrl descent (Done)

D08R accepted ("it's absolutely rock solid"). D08Q1: `k_jet_descend_velocity`
5500 in `jetpack.inc` (Modern only) gives 29.5 units/frame, the underwater
Ctrl dive rate from `/tmp/swimlab/deep4.log`. User: "verified working!!!". Probe script `descent` in the
scratchpad `jet_probe.py`; health is `player+0x32`. Binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`. No commits.

## 2026-09-29 - D08R selectable jetpack scheme (revision 2, now Done)

`--jetpack classic|modern` -> profile schema 11 control `jetpack` ->
`DNTTK_JETPACK`. Gate `jetpack_classic()` in `modern_controls.cpp`. Classic
shares the entire Modern control path (lease, `face_view`, WASD bridge, J,
fall grace); `jetpack_update()` in `jetpack.inc` returns before the host
vertical layer (hover, Ctrl descent, trim). Do not give Classic the original
camera or D-pad turning again: the user rejected that (revision 1) because
mouse turning failed and controls slipped out of modern. Probe harness copy
lives in the session scratchpad (D08Q `/tmp/jetlab/jet_probe.py` plus
`classic2`, `wcheck`, `turn2`, `tables`; `PORT`/`EXTRA` env). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`. No
commits until asked.

## 2026-09-29 - D08B accepted (Done)

User: "d08b can be marked as completely done man, its fine. accept it all"
and "i have tested all of them!". The user playtested the old D08B limits
(contact jumps, bed run-off slowdown, unopened-bed pipe bomb, oblique couch
entry, pig-cop ladder stall) and accepted them. Board and status updated; no
build or automated replay. D10 is the only
In progress job. No commits until asked.

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

Intro-only stutter after D23A. Cause: `psxrecomp-game --overlay-config-hash`
covers the `game.local.toml` hook list, so each new host hook moves the
movie shard's cache folder (`cache/SLUS-00583/gcc/linux-x64/cg10_..._gc<hash>_f0`)
and the loader silently interprets MOVIE.OVR. Now: `ttk-movie-shard` target in
the `local-dev` build preset, `run.py` `ensure_movie_shard()`, and
`fmv_poll.c` logs `ttk-fmv: native movie decoder active (19 functions)` or a
WARNING carrying `overlay_loader_last_msg()`. After adding a hook, just build
or launch normally. If the intro stutters: check the `ttk-fmv:` line in the
session log first. Launch tests in `tests/local/test_player_profiles.py`
must pass `--no-session-log` and stub `run.ensure_movie_shard`, or they
start the real game on the player's cards. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits until asked.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Done, user-accepted)

User: "that stuttering audio/slowness issue" before the D08Q playtest.
Measured with the runtime's telemetry in an isolated Xvfb instance
(`/tmp` harness; `audio_stats`, `phase_profile`, `phase_hot`, `frame`,
`ttk_input`): Modernized 47.5–49.3 fps + continuous underruns, Vanilla
60.0 clean, Sep‑27 binary 57.5. Cause: `identity()` walked all 20,305
guarded words via `psx_mod_read_word` on every call, 46 calls/frame ×
108 µs = 5 ms/frame (24 % wall). Fix: `code_identity.h` memcmp fast path
on `g_psx_ram` (exact per-word fallback keeps the apartment substitute),
`IdentityMemo` per `input_host_frame()` × `g_dirty_ram_code_gen` in
`modern_controls.cpp` and `weapon_aim.cpp`. Now 1 check/frame, 28 µs,
59.94 fps, 0 underruns, fill 267 ms. Debug JSON `identity_calls`,
`identity_checks`, `identity_us`. Tests: mocks define `g_psx_ram` /
`g_dirty_ram_code_gen`, `poke()`/`pokeb()` bump the generation; new memo
case. All three native suites PASS. If stutter returns: check
`identity_checks` ≈ 1/frame and `audio_stats.out.fill_ms` vs target first;
`perf` is unavailable (paranoid=4). Do not add per-call guard walks again;
new guards are now nearly free. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. No
commits until asked.

## 2026-09-29 — D08Q modern jetpack flight controls (Done, user-accepted)

Accepted: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."


User: jetpack uncontrollable except Space; WASD "blocked"; modern controls
fail. Isolated baseline on `79a3fd5b…` (turret-room / ledge states copied to
`recomp/analysis/d08q-jetpack/`, `dninventory` cheat, J, Space): original
mode 10 (anims 163–170) matched no lease → no mouse; WASD via tank fallback
relative to an unturnable body; idle gravity landed Duke in 1–2 s. New
`recomp/src/ttk/jetpack.inc` from the existing `8005a210` hook, camera-only
lease clause `jet` in `state()`, `jetpack_input_ready()` for `input_pad()`
(W/S → Up/Down, A/D → layout strafe pads) and `select_weapon` (J only in
flight), `locomotion_input_ready`/`airborne_input_ready` false in flight and
the 108 fall grace. Host writes: hover lock `+0x224 |= 0x08000000` + base
`+0x860 = Y` when idle; Ctrl → lock off, `+0x1f8 = 2800`, `+0x1e8 = 0`; WASD
→ lock off, `+0x1f8 = 0`, `+0x1e8 = 18` trim; `face_view()`. Do **not**
drive `+0x860` for descent or hover during WASD (57-jetpack-controls.md,
"Rejected on evidence"). Guards `8004aaf8`/272, `8004ac08`/472,
`8004ade0`/1972. Live probes: headings within 4° of the camera, level ±25,
25–29 units/frame, Ctrl 489/30 frames → 105, J off → 108 with live mouse →
ground run. ttk-input-test, ttk-aim-test, ttk-controls-test PASS (controls
test: use `analysis/pc-input/d08-camera-final/level00-guard-fixture.bin`;
the `d08p-turret` fixture is Modernized-patched and fails the Vanilla
assertion). Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`. Playtest:
J, Space, mouse, WASD, release → hover, Ctrl → land, J in the air. If the
feel is off, the tunables are `k_jet_descend_velocity` (2800) and
`k_jet_level_trim` (18) in jetpack.inc; `DNTTK_JET_TRACE=1` logs each
update. No commits until asked.

## 2026-09-29 — D08P overlay scratch identity + mid-depth wade handler (Done)

User: the whole ledge area (F7 UI slot 3) kills run, mouse and jump for good;
depth is not the cause. Their `session-20260929-005059.log` showed
`[TTK lease] inactive (identity)` from the ledge onward. Live guard diff on the
isolated slot 3 copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's
trailing scratch vector, written by the zone script's `0x8007177c` hit test
from overlay code `0x800cb1a4`. The overlay guard now covers 9652 bytes
(code/tables), digest `274d71dd…`; `ttk_state_probe.py` matches; the controls
test proves the vector write keeps the lease and the last table word is still
guarded. Then the same water (depth 512, original mode 1 / clips 80/81,
Vanilla 8–14 units/frame) got its handler `0x800539f8` hooked — added to
`game.local.toml` `mod_function_entry_funcs`, regenerated (one generated line;
`build.py` itself stops at the pre-existing runtime patch-stack drift, so
generation was run directly with the built recompiler) — retargeting the
world root to camera-relative WASD in the land run band (gain 4, 80..120 per
tick). Guards added for `0x800486b0`/260, `0x800539f8`/452, `0x800788e0`/200.
Live: 45–58 units/frame, mouse steering, strafe, wade jump 98, 0 refusals,
turn/run/jump after the zone; slot 2 regression clean. Wading into a wall
idles (original handler); turning resumes. ttk-input-test, ttk-aim-test,
ttk-controls-test PASS. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`.
Isolated cards: `recomp/analysis/d08p-slot2/cards/openbios/` now holds copies
of state slots 01 and 02. User playtest: "it really works. perfectly" →
D08P **Done**; docs committed and pushed. Next session: pick the next job from
the board (no job auto-started).

## 2026-09-29 — D08P held keys across recapture, fresh capture offers (Needs playtest)

User: iteration 4 changed nothing for them (session log shows a fresh launch
and F7 slot 2 load 30 s before the report). Live Xvfb + xdotool probes
(`/tmp/d08p_live_probe*.py`, isolated `recomp/analysis/d08p-slot2/`, software
and OpenGL, debug load and the real `F7 → 2 → l` menu, the user's own profile)
all show the wade at 48–54 units/frame with mouse look; slow headings are the
original wall slide beside the spawn. Reproduced instead: a Shift/W held
through any recapture was wiped by `clear()` → walk gait + Walk pad until
re-pressed. Fixes: capture resyncs bound keys/mouse buttons from SDL device
state; capture offers expire after 8 frames so the original pause menu is not
captured; sustained tank fallback announces `ORIGINAL MOVEMENT (reason)`;
`run.py` mirrors stderr to `recomp/build-local/logs/session-*.log`. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`.
If the playtest still degrades, read the session log's `[TTK lease]` /
`[TTK input]` lines first. Turret itself not reached in probes (S doorway →
bridge → deep water; N platform → W walkway dead end). No commits until asked.

## 2026-09-28 — D08P waist-deep land locomotion (Needs playtest)

User: after speed recovered, the turret wade still crawled, mouse look was
dead in the water, and after leaving they only walked with no Shift. Isolated
slot 2 already uses land run anim 78 on D-pad forward; Modernized dropped
the lease on tank turns 70/71 and injected Walk. Waist-deep / mid water
now keeps the land camera lease, faces the view, converts 70/71 to run,
and does not inject Walk. ttk-input-test, ttk-aim-test, ttk-controls-test
PASS. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`.
Playtest F7 slot 2: run + mouse in the wade and after stepping out. No
commits until asked.

## 2026-09-28 — D08P identity reader stall (Needs playtest)

User: after the slot-2 recapture binary, everything ran slow including
audio. The apartment pair check ran on every word of every identity
guard, many times per frame. It now only inspects the two LEVEL00
apartment words. F7 recapture behavior is unchanged.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`.
Restart the player build; the running session will not pick this up.
No commits until asked.

## 2026-09-28 — D08P F7 slot 2 recapture (Needs playtest, iteration 3)

User: iteration 2 did not restore the turret room; F7 UI slot 2 (file
`state_800AB6FC_slot01.pst`, turret around the corner) lost Modernized
entirely. Isolated dump matches the flooded wade (depth `0x100`, anim 63,
normal camera, apartment words already patched). F10 left
`initial_capture` false; F7 released the mouse and did not request
recapture; the F7 menu could eat `capture_offer`. F7 now sets
`initial_capture` even if the cursor is already free; offers stay until
`allow_capture`; identity remaps the complete patched LEVEL00 pair.
ttk-input-test, ttk-aim-test, ttk-controls-test PASS (including F7
recapture and slot-2 patched overlay). Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`.
Playtest: F7 load slot 2, mouse/WASD/strafe/crosshair without a manual
F10. No commits until asked.

## 2026-09-28 — D08P turret wade (Needs playtest, iteration 2)

The placed checkpoint is host F7 savestate slot 1, not a memory card. Isolated
dump: flooded turret room, depth `0x100`, anim 63, normal camera, no
`0x20000000`. First tank fallback made A/D turn (D-pad L/R → anims 71/70) and
classified 70 as free swim, which hid the crosshair and blocked Escape
recapture. Fixes: free swim only at original `≥0x281` or states 4/5; A/D
fallback L2/R2 strafe; recapture from `0x8005a210`; reticle on camera-only
locomotion. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`.
Playtest from that savestate: mouse, A/D strafe, crosshair, Escape resume.
No commits until asked.

## 2026-09-28 — D08P turret WASD freeze (Needs playtest)

Crystal-2 ceiling turret (after first crystal, broken-bridge water corridor)
dropped the modern camera lease. Captured WASD was inert because move actions
use pad 0 and Forward inject requires the lease. F10 could not restore WASD.
First fix: captured play without a lease feeds original D-pad tank bits
(iteration 2 replaced A/D turn with strafe). stderr
`[TTK input] WASD tank fallback` and `[TTK lease] inactive (...)`. Playtest
that room; if the window ignores keys entirely, that is a guest halt, not this
path. D07A/B, D08A, D08H and D10 distance closed on user play acceptance.
Binary `fd9e4c4d015673ec6fa1dcdedb3bcd60d9b74ddb4377786afb5a241d0f4e4c28`.
No commits until asked.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User: do not add a scuba device to this game. D08N is **Cancelled**. TTK
underwater air stays original and automatic; Bio Mask stays gas-only. No scuba
prototype, inventory slot, HRP scuba art, or toast. Historical notes that once
listed D08N as a swim follow-on are superseded. Next job: pick from the Todo
list (D08L, D09, and other open rows; not D08N). No code change. No commits.

## 2026-09-28 - game/ drop folder (tested)

First run of `run.py` / `build.py` creates `game/` with a README. Importer accepts
Redump USA `.cue`/`.bin` or CloneCD `.img` (hashes `708c0404...` and `230a34c2...`)
when the EXE pin matches. Europe/PAL `SLES-01515` prints a console error. User
tested the drop-folder flow. Original media is never overwritten.

## 2026-09-28 — D08O closed (accepted); nothing pending from the swim work

Playtest 3 on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb` accepted
everything: A/D strafe, Ctrl dive, underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the round-3 shallow → platform ledge jump, and the `biomask-small.png`
item-switcher icon. D08O is **Done**; the entries below are history. Do not
reopen the swim/ledge code without a reported regression (55 §6 rules 7–9).
Next job: pick from the Todo list in `MODERNIZATION_JOBS.md` (D08L, D09 are
open; D08N scuba is cancelled). No commits.

## 2026-09-28 — D08O.4 ledge jump: plain 98 arc, taller (Needs playtest)

Playtest on binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.
Swimming (Ctrl dive, underwater WASD/Space/Ctrl, A/D strafe, mantle) and the
Bio Mask icon are **accepted** — do not touch. Only the ledge jump changed:

1. **Subway shallow water → platform** (west platform, room 21 → 57). From
   touching the wall, from a few steps back, running or standing, Space alone
   or with W: Duke does the ordinary directed jump (98) — not the tall 97 hop of
   round 2 — about a head higher than his normal shallow-water jump, and lands
   on the platform. At the wall / one step back he rises first and moves onto
   the platform only once his feet are above the lip (XZ held, then 3400/4000);
   from further back he travels from takeoff (1900 + 3.5·distance, max 6400).
   Landing is 98's normal landing (sometimes the hard 105). From ≳1650 units the
   ledge is out of probe reach and the plain 98 falls short — intended.
   `ledge_assists` in the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1`
   prints `ttk-ledge probe/launch/clear/abort/ballistic` and per-tick `ttk-swim`
   lines to stderr (`runs/<name>/runtime.log` in the lab).
2. If it still looks too high: `k_ledge_vy` (−9000; 98's own ≈−6900, rise scales
   ≈ VY/96·ticks — −8000 was not tried, apex margin at −9000 is ≈200 above the
   635 ledge). If it bumps the wall: `k_ledge_hold_distance` (320) / the hold
   speeds. If it overshoots into the back wall: `k_ledge_speed_per_unit` (3.5).
3. Regressions to keep: everything in the D08O.2/.3 entry below.

Mechanics (`swim.inc`): `swim_ledge_ahead` (probe 0/320/640/960/1280),
`swim_try_wade_jump` → `swim_start_airborne_jump(…,98)` + `delayed_vertical =
k_ledge_vy` (applied at the `8003ebf4` ballistic hook, `terrain_epoch` gated),
`swim_ledge_ballistic` (called from the same hook each ballistic tick: hold
XZ = 0 below the lip when `swim_ledge_hold_xz`, else exact XZ via
`swim_boost_want` / `swim_boost_horizontal`; clears `swim_ledge_pending` once the
foot is 32 above the top). `swim_update` drops a stale pending flag whenever
anim ≠ 98 or state ≠ 9. Lab: `/tmp/swimlab/ledge17.json` (15 trials),
`deep3.json`, `walk`. No commits.

## 2026-09-28 — D08O.2/.3 native dive + wade ledge assist (Needs playtest)

Playtest on binary
`71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5`:

1. **Ctrl at the surface** → Duke dives (original anim 133, `+0x22c` = 5).
   Underwater: **W/A/S/D** swim where the camera looks (thrust 128–130),
   **Space** ascends, **Ctrl** descends, looking up/down + W follows the view.
   Duke surfaces automatically (122, state 4); Ctrl again re-dives. If Ctrl is
   still inert: `swim_dives` in the debug JSON must increment; `swim_try_dive`
   refuses while `+0x224 & 0x241` or `+0x244` (timer) is non-zero.
2. **Subway shallow water → platform** (west platform with the hazard stripe,
   room 21 → 57). From touching the wall, from a few steps back, running or
   standing, Space alone or with W: Duke should hop high and land on the
   platform every time, with no bump-off (107) and no splash-cancel. Landing
   is the original hard-landing 105 (the 97 arc is higher than 98's). From
   >≈1300 units the plain 98 still falls short — intended. `ledge_assists` in
   the debug JSON counts assisted jumps; `DNTTK_SWIM_TRACE=1` prints
   `ttk-ledge launch/switch/abort` and probe lines to stderr.
3. Facing a plain wall in shallow water with W+Space → vertical hop (was a
   cancelled 98 splash).
4. Regressions to keep: A/D surface strafe (123/124), Space ascend only, E /
   mantle exit, shallow W/S/A/D wade, Space 98 in open water, Ctrl 105.

Mechanics (`swim.inc`): `swim_ledge_ahead` (standing probe `800797f4` from
0/320/640/960 along the view, world XZ written and restored inside the query),
`swim_try_wade_jump` launches 97 and arms `swim_ledge_pending`,
`swim_ledge_assist_update` (early hook, runs without the swim lease) switches to
98 when `+0x10` ≤ ledge top − 96, keeps VY via `delayed_vertical`/`delayed_takeoff`
(applied at the `8003ebf4` ballistic hook), XZ = 2200 + 3.2·distance (max 5400)
via `swim_boost_want`; `swim_reassert_boost` now also runs in flight.
Native swim: `swim_try_dive` (`80045564`), `swim_steer_underwater`,
`swim_thrust_input_ready` (Square injection in `pc_input.cpp`). Lab scripts live
outside the repo in `/tmp/swimlab` (`swim_lab.py`, `ledge*.json`, `deep3.json`).
No commits.

## 2026-09-28 — D08O implemented (Needs playtest)

Playtest the deep-water trio on binary
`d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e`:

1. **A/D** while swimming → camera-relative strafe (anims 88–93), no turning.
   If A/D still turn, dump `800d1a68/800d1b80[player+0x233]` — the strafe pad
   bits are resolved from those layout words (default L2=0x100 / R2=0x200).
2. **Ctrl** at the surface and mid-water → descend; **Space** → ascend to surface.
   Dive stops at `+0x1c8 − 0x1e0` (floor margin) or `surface + 0x1800`.
3. **Space** at a ledge no longer hops out; leave via **E** / original mantle.
   Shallow wade jump / ledge hop (`8003e2d0(97|98)`) is unchanged.

Regression already confirmed in the shallow subway savestate: W/S/A/D wade with
stable heading, Space wade jump 98, Ctrl crouch 105. ttk-controls-test and
ttk-input-test PASS. No deep-water savestate exists; deep zones (rooms 31, 35,
42, 64, 65, 68, 72) sit behind a keypad door (−7400,51700) and a fence
(40636,49390) from the subway state, and teleporting snaps Duke to geometry.

Code: `recomp/src/ttk/swim.inc` (`swim_strafe_pads`, `swim_apply_vertical`,
`swim_try_mantle` removed), `pc_input.cpp` free-swim block,
`modern_controls.h`, `tests/local/pc_input_native.cpp` stub. No commits.

## 2026-09-28 — D08O marked next (docs only)

**Next job to pick up: D08O** — deep free-swim polish.

Locked design in `documentation/55-swim-controls-research.md` (§1.2, §8) and
`documentation/54-swim-redesign.md`:

1. Fix A/D camera-relative strafe (W/S already OK)
2. Fix Ctrl descend from surface / mid-depth
3. Strip free-swim Space→ledge exit → **mantle / E only** (keep shallow wade jump)

D08M closed: shallow jump + ledge hop Done (do not reopen unless regression).
Space↑, W/S, E mantle accepted on binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`.

Hard rules: no invented `+0x20c`, no entry `+0x1f8` clear, no world-XYZ swim.

This session: documentation/design only — no `swim.inc` changes.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

No commits.
