# D12 - First-person weapons

**Done - user accepted 2026-09-30:** "finally, we can mark this as accepted!!" In the Modernized eye view (D11) Duke's own
right hand and the original weapon mesh from the disc are drawn in front of
the eye, like a first-person viewmodel. No new or imported assets: every
polygon, texture and muzzle flash is the game's own, taken from the same
draw that renders Duke in third person. Third person and Vanilla are
unchanged.

## How the original draws a held weapon

`0x800348d8` draws an actor joint by joint. For each joint it calls
`0x800292a0(camera, joint matrix)`, which loads the GTE transform and returns
the joint's depth; a joint nearer than GTE H (or past the far limit) is
skipped with everything attached to it. After the joint's own mesh the loop
draws, for the joint in model record byte `+0x33` (Duke: joint 7, the right
hand) only:

| Call (return address) | When | What |
| --- | --- | --- |
| `0x80033e40(hand, mesh, player+0x238, player+0x856)` (`0x8003531c`) | `player+0x3b8` = 2 | Weapon mesh: slot table `0x800c4590` (44 bytes per slot), mesh index at `+0xe` into the mesh table at `[0x800ddb18]` |
| `0x800341e4(hand, slot, camera, muzzle vector)` (`0x80035348`) | player word 0 bit 3 | Muzzle flash, placed at the hand plus transpose(hand) times the slot's muzzle SVECTOR (`+0x20` of the entry) |
| `0x80033f5c(hand, slot, camera)` (`0x80035250`) | `player+0x3b8` = 1 | Held inventory item |

In the eye view the hand is within about 100 units of the eye, so the loop's
near cull skipped the hand and the weapon; that is why D11 showed neither.

Joint matrices are stored world-to-joint: `0x800292a0` transposes the joint
matrix (`0x80010d60`) and multiplies camera `+0x20` (camera `+0` with the x
row scaled by 1.6) by it. A joint's world rotation is the transpose of its
stored matrix. This matters once the view is pitched.

## What D12 changes

New entry hooks `0x800292A0`, `0x80033E40`, `0x800341E4`, `0x80033F5C` in
`game.local.toml` (regenerated; see the handoff note on running the
generator directly). Code in `recomp/src/ttk/first_person.inc` (weapon),
`near_clip.cpp` (drawing on top) and `modern_controls.cpp` (dispatch, debug
JSON). All of it runs only while the eye view is live (Modernized,
independent camera, blend at least 0.5, orbit lease) and a weapon or item is
held.

- **Private matrix.** At the hand's transform call (return `0x80034c6c`,
  `s4` Duke, `s7` the hand joint, `a1` Duke's live hand matrix) `a1` is
  pointed at a guest-allocated copy. The weapon, flash and item calls get the
  same copy in `a0`. Duke's joint matrices are never written.
- **Placement.** The copy holds the hand as a viewmodel: position anchored at
  (110, 150, 520) in view space (x right, y down, z ahead), orientation shown
  relative to a level eye and pitched with the view. A per-weapon offset table
  moves slot 7 (grip far right) and slot 8 (shoulder twin launcher) further
  out. Developer overrides: `DNTTK_FP_WEAPON=x,y,z`,
  `DNTTK_FP_WEAPON_MOTION=0..1`, `DNTTK_FP_WEAPON_VIEW=0` (off).
- **Ready pose (default).** At rest Duke carries the pistol raised and the
  shotgun across his body; from the eye that shows the weapon's side or points
  it off screen. Each weapon is instead held in its own firing pose: the hand
  relative to the torso joint (joint 1, the arms' parent) and the torso's pose
  relative to a level eye, recorded on the first muzzle-flash frame. Seeds for
  slots 4-11 were surveyed in this build (street, standing, level view) and
  are in the source; a weapon without a seed records its pose on its first
  shot in first person. Each flash frame adds a short kick (up to 7 degrees
  muzzle up and 30 units back, decaying by 0.7 a frame); the flash itself is
  the original, drawn from the same matrix, so it stays on the muzzle.
  Modernized view aiming (D07A) turns Duke's arms toward the view even at
  rest, so blending in the live pose added the view pitch twice; the ready
  pose therefore does not follow the live arm. `DNTTK_FP_WEAPON_POSE=animated`
  shows the live torso-relative animation instead (research).
- **Drawn on top.** While the hand, weapon, flash or item is drawn, the D11B
  object path takes every polygon of the mesh. Packets are written as usual
  but linked only when Duke's draw ends (`0x8001ca4c`, return `0x800376b4`):
  sorted farthest first and appended after the tail of ordering-table slot 1,
  drawn after everything except slot 0. The HUD (health and ammo boxes) is in
  slot 0 and stays on top of the weapon; slot 0 otherwise holds only geometry
  nearer than 32 units. A wall between the eye and the weapon cannot cover or
  cut it, and Duke's chest (near the eye when looking down) no longer draws
  over it. Packets from an earlier frame are discarded, never linked.
- **Twin cannons (slot 8, key 5).** Framed after the user's Duke 3D
  Devastator references (`research/devastator*.{jpeg,webp}`,
  `research/jason-oliveri-dev2902.jpg`): offset (-20, 240, -100), 42 degrees
  muzzle-up, weapon scale 1.2 (the original draw's own scale argument, `a3`
  of `0x80033e40`). The two cannons rise from the bottom corners; the TTK
  model is one slab whose middle block shares the cannons' top plane, so no
  placement hides it completely and it stays a strip at the bottom edge. The
  hand joint must stay deeper than GTE H (256) or the loop culls the weapon,
  which limits how close it can sit. Per-weapon developer overrides:
  `DNTTK_FP_WEAPON_SLOT<n>=x,y,z`, `DNTTK_FP_WEAPON_TILT<n>=pitch,yaw,roll`,
  `DNTTK_FP_WEAPON_SCALE<n>=factor`.
- **Framing tweaks (user playtests).** Shotgun (slot 5, key 3): offset
  (30, 10, 0), tilt -16 / -15 / roll 6, barrel and pump visible, low, leaning
  slightly right, muzzle raised toward the crosshair. Gatling (slot 7, key 4):
  offset (110, 80, 290), tilt -12 / -26 / roll 16, low and to the right,
  muzzle raised toward the crosshair. Double
  barrel (slot 6, key 9): tilt -6. Energy weapon (slot 10,
  key 7) and freezer (slot 11, key 0): scale 1.4, offset (0, -20, 0), tilt
  -12 (freezer yaw -10); moving them closer instead made them vanish (near
  cull). Throwing blades: slot 1 tilt -30 / -20 / roll 40, slot 2 tilt -40.
  Pistol (slot 4) offset (-10, 65, 0), tilt -10, and knife (slot 1) offset
  (-60, 60, 0): low enough that the glove's cut wrist is below the screen
  edge, so no floating hand shows (only the right hand is drawn; the arm
  joints are hidden in the eye view).
  Tilt is pitch about the level x axis (negative raises a forward-pointing
  muzzle and lowers the grip), yaw (positive turns the muzzle right) and roll, in degrees about
  the grip.
- **Mirrors.** The club's wall mirror draws Duke outside the eye-view draw,
  so the reflection keeps his head, arms and the real third-person weapon
  pose (checked with the twin cannons and the shotgun).
- **Arms hidden.** Joints 2-6 (upper arms, forearms, left hand) get a matrix
  at the eye so the loop's own near cull skips them while the viewmodel is
  shown: the right hand has moved away from them, and two-handed carries held
  them across the view.
- **Close-camera fade.** For the root joint the loop makes the rest of the
  actor semi-transparent and dark when the root is nearer than 1024
  (`0x80034e18`: context `+0x48` = `0x2000000`, `+0x4c` = `0x600000`, flat
  colors and GTE back color = depth/8). In the eye view the root clears the
  near cull only when looking down about 55 degrees or more, and the weapon
  then drew faint. For the viewmodel draws the context and back color get
  Duke's plain values (brightness `player+0x1b`) and are restored at the next
  joint. Actors translucent on purpose (word 0 bits `0x40`, `0x2000`,
  `0x4000`) keep theirs.
- **Guards.** SHA-256 code guards added for the actor draw `0x800348d8`
  (3112 bytes) and the flash routine `0x800341e4` (700 bytes); the eye-view
  weapon path also requires the existing identity check.

Debug JSON `ttk_input` -> `controls` -> `fp` -> `weapon`: `hands`, `draws`,
`flashes`, `items`, `readies`, `pose`, `recorded`, `arm_hides`,
`fade_clears`; `fp` -> `near` -> `viewmodel`: `meshes`, `packets`,
`discards`. The `ttk_input` JSON buffer was raised from 2048 to 8192 bytes
(it was truncating). `DNTTK_FP_WEAPON_TRACE=1` logs each recorded firing pose
in seed-table form.

## Evidence

Binary `1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`
(copy in `recomp/analysis/d12-first-person-weapons/d12-candidate.bin`);
cannon framing and HUD order on
`b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`
(`d12-cannons.bin`), native and Python tests PASS again.
Isolated Xvfb runs on a private copy of the D11B street savestate
(`recomp/analysis/d12-first-person-weapons/cards`, slot 1). Captures are
window grabs (what the player sees).

- All 13 weapons reachable with the typed `dnweapons` cheat (slots 1, 2 and
  4-14), each at rest and after firing: every one is drawn as a first-person
  weapon. Slots 4-11 use their firing poses; slots 9 and 10 fire from their
  carry pose; slots 1, 2 and 12-14 (melee and thrown items, no muzzle vector
  or flash) show their live animation relative to the torso.
- Pitch sweep (0, +-35, +-60 degrees) with pistol and shotgun: the weapon
  stays put in the view, at rest and firing; the muzzle flash draws on the
  pistol's muzzle at 60 degrees down.
- Walking, running, crouching, jumping, fast turns: the weapon stays in view.
  Pressed against the club street wall: drawn over the wall.
- P to third person: Duke with the raised pistol as before; viewmodel
  counters do not advance.
- Frame budget, street, pistol drawn, 15 s walking with mouse sweeps: first
  person 59.95 fps, third person 59.96 fps, 0 audio underruns each.
- Vanilla regression route `analysis/vanilla-regression/d12-vanilla` exit 0;
  draw, fire, holster, forward and turn captures show the original camera.
- Native: `ttk-controls-test` (new D12 case: only the draw loop's hand call
  and its attached draws are redirected, arm joints get the eye matrix, other
  joints, callers, actors, holstered and third person are untouched, Duke's
  joint matrices are byte-identical afterwards), `ttk-aim-test`,
  `ttk-near-test`, `ttk-scene-test`, `ttk-input-test` PASS. Python 74 tests
  OK (2 skipped).

## Limits

- Holstering and drawing are instant: the weapon appears or disappears when
  the held state changes; the raise and lower animations are not shown.
- The ready pose does not follow the live arm animation, so reload or pump
  motions are replaced by the flash kick. `DNTTK_FP_WEAPON_POSE=animated`
  shows the live arm for comparison.
- Projectiles still leave from Duke's real hand (D07 physical muzzle), which
  sits slightly off the drawn weapon; shots still go to the crosshair.
- Only the right hand is drawn. Two-handed weapons show the weapon without
  the left hand.
- The weapon is lit from its real world orientation, so looking steeply down
  makes it a little darker.
- Slot 9 fills a large part of the view. The twin cannons' middle block
  shows as a strip at the bottom edge (see above).
- Weapon survey on the first map's street only; the other eras' weapons,
  enemies' reactions and the campaign are not covered.
- Death, swimming, jetpack flight, ladders, ledges and scripted cameras
  already leave the eye view (D11), so no weapon is drawn in them; eye height
  and recoil feel still need the user's playtest.
