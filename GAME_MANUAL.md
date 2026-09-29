# Duke Nukem: Time to Kill — PC game manual

This project brings your USA PlayStation copy of **Duke Nukem: Time to Kill** to PC. It is a development build: the opening level and movies are running smoothly, while full-game accuracy is still being checked. Modern controls are being implemented first for third-person play; first-person remains planned. Vanilla uses the original controls below. Modernized has a first-level preview of camera-relative WASD and an independent mouse camera. A guarded mouse-aiming and crosshair preview is now available; weapon and state coverage remains limited (see below).

## Start playing

The first time you run the launcher or build script, it creates `game/` in this folder (with a short README). Put your owned **USA SLUS-00583** dump there: the dump folder itself, or the image files. Europe/PAL (`SLES-01515`) is a different SKU and will not work.

Accepted dump layouts:

- Redump-style `.cue` + `.bin` (`TRACK 01 MODE2/2352`)
- CloneCD `.img` + `.ccd` + `.sub`
- A raw MODE2/2352 `.bin` or `.img`

Then open a terminal in this folder and run:

```sh
python3 recomp/tools/local/run.py
```

Click the game window to give it keyboard focus. Press **Enter** to skip the intro or pause. Use the arrow keys in menus and **X** to confirm. Choose a new game to start Duke's campaign.

If the executable is missing, follow [build and run](documentation/04-build-and-run.md). If the prepared disc is missing, the launcher searches `game/` for a known USA dump and copies it into ignored `recomp/disc/`. Original files are never overwritten.

The tools accept a dump whose image SHA-256 matches a known USA copy (CloneCD or Redump USA MODE2/2352) and whose boot EXE is `SLUS_005.83`. A Europe/PAL disc, or a USA image with a different hash, prints an error on the console with the accepted SHA-256 values and does not launch.

## Profiles and settings

Modernized is the default for new settings. Existing saved mode choices are retained.
Vanilla remains available in the terminal settings menu:

```sh
python3 recomp/tools/local/run.py --settings
```

Choose Vanilla or Modernized preview, change OpenGL/software rendering, or
restore the selected profile's defaults. Save and return, then launch normally.
Each profile remembers its own renderer. Modernized also remembers its PC action bindings, camera selection, mouse sensitivity and inversion. Changes take effect on the next launch.
**Modernized movement and camera have passed user playtesting in the first-level preview.
Supported first-map climbing uses the original traversal routines with Modernized
input. In the crystal-2 flooded turret corridor, waist-deep water uses the same
camera-relative run, mouse look and Shift as dry ground — it is not a swim.
The deeper wade along the right-hand ledge (where the original slows Duke to a
forward-only wade) also moves at run pace toward where you look, strafes, and
jumps on Space. Wading straight into a wall stops Duke, as in the original:
turn the view and he moves again. Reaching that ledge no longer switches the
whole area to original controls (a level-script write used to fail the code
identity check).
Escape then resume recaptures, and keys you keep holding across that pause
(Shift, W, Ctrl…) stay held — releasing and re-pressing Shift is not needed.
F7 load of the turret-corner savestates (UI slots 2 and 3) also recaptures automatically,
again keeping held keys. Capture is not taken inside the original pause menu.
If a scripted camera still drops the modern lease, W/S walk and A/D strafe
(original L2/R2) until the normal camera returns — A/D must not tank-turn.
When that fallback lasts more than about a second the screen shows
`ORIGINAL MOVEMENT (reason)` and `MODERN MOVEMENT RESUMED` when the lease
returns; the reason (`state`, `identity`, `context`, `released`) is the one
to report. `run.py` mirrors these `[TTK …]` lines into
`recomp/build-local/logs/session-*.log` (last five launches;
`--no-session-log` disables).
F10 toggles mouse capture only; press it again if the
cursor is free. A frozen window that ignores F10 is a separate halt.**

You can also select and launch directly with `--mode vanilla` or
`--mode modernized`. Use `--show-settings` to inspect preferences without
launching, and `--reset-profile vanilla` (or `modernized`) to restore only that
profile's defaults. `--renderer software` now remembers the choice for the
selected profile. Profile preferences live in `recomp/config/player-profiles.json`;
switching profiles keeps your existing memory-card location. Vanilla keyboard
and physical-controller bindings retain the runtime configuration.

## Modernized apartment secret

In Modernized mode, switching off the first apartment's lights can open the bed
secret without first talking to the woman. Dialogue remains available and the
secret is counted once. Vanilla keeps the original conversation prerequisite.
The user has confirmed the natural switch-first approach works.

## Modernized movement and camera preview

Modernized gameplay captures the mouse and enables PC actions automatically.
Pause, inventory, focus loss and host menus (including **F7** savestates) release it; verified gameplay can
capture again after returning. **Escape** pauses/resumes. **Enter** uses the
currently selected inventory gadget (EDuke-style); it never opens the pause menu.
**F10** explicitly toggles capture; use it to opt out
of automatic capture until you capture again. Pausing while captured restores
automatic capture on return. Loading an F7 savestate also restores automatic
capture once gameplay is live again. In supported first-level standing/walking states,
mouse movement orbits the third-person camera. With view weapon aiming enabled,
Duke turns to face the view horizontally when holstered or using a supported weapon.

While captured, use **left mouse** to fire/action, **Space** to jump, **Scroll Lock** (tap) to
draw/holster, **hold Left Ctrl** to crouch and release to stand (when overhead clearance permits), **Left Shift** as the speed modifier,
**right mouse** to hold aim (the same view aim for supported Modernized weapons), **comma/period** for original sidesteps,
and **I** for inventory. **E is interaction-only and never fires.** If necessary,
a tap automatically requests holstering and then interacts once that animation
finishes. Keep E held for continuous action while climbing. Your weapon is
automatically drawn again after the interaction ends and normal movement returns.
This temporary holster request survives pause and inventory. Supported ladder/ledge
states can recapture automatically after pause; F10 still lets you opt out.
Short jump taps are buffered for about 130 ms in supported grounded movement.
Running jumps respond to your takeoff request instead of waiting for an edge. A fresh
run-start jump no longer waits for the first stride to finish. A six-input-frame
grace window also accepts late presses on supported small furniture drops (up to
768 world units), subject to original headroom and forward clearance. This does
not apply to large exterior falls and cannot supply a second jump.
**Directional jumps work in either input order, without a run-up:**

| Input | Result |
| --- | --- |
| W/A/S/D, then Space | Jump in that camera-relative direction, even from standing or walking |
| Space, then W/A/S/D during takeoff preparation | Pick up that direction and jump toward it |
| A diagonal plus Space, in either order during preparation | Jump diagonally |
| Space alone | Jump straight up |

Once a direction is chosen, releasing the key during preparation does not cancel
it. Preparation does not restart, and there is no midair steering after takeoff.
Original clearance checks can refuse a directional jump near an obstruction.
This input-order behavior was **accepted by the player on 2026-09-27**; see the
[controls record](documentation/34-standing-directional-jumps.md).

Armed airborne **E** requests accelerate only the required original stow
transition so the weapon can clear before ladder contact. Attachment still needs
valid original contact; safe exit restores the intended weapon automatically.
If you press E during a weapon switch, the request waits for the weapon to settle
before stowing it.

**E prepares a reach through the current running jump**: you can release it before
reaching the next ladder, then use W to climb once attached. The automatic holster
and redraw still apply; you do not need C for a transfer. Original collision and
traversal rules determine whether a surface can be caught.
If you deliberately holster with Scroll Lock, E leaves it holstered. In supported ground
states, **left-click draws the selected weapon; keep holding to fire**. A quick
click released during drawing leaves it drawn without firing a delayed shot.
Boot retains its unarmed attack. Ordinary ground draw/holster animations run at
1.5x; E-owned airborne stow and its final blend use a bounded 4x animation budget; original firing speed is unchanged. On supported ladders and ledges, **W/S** feed
the original up/down controls, **A/D** feed left/right, and **E** keeps the action
held. The game controls the attached climbing motion and camera. Number keys now select familiar weapon groups when owned and usable:

| Key | Weapon group (press again to cycle alternatives) |
| --- | --- |
| 1 | Mighty Boot / Throwing Knife / Throwing Axe |
| 2 | Desert Eagle |
| 3 | Combat Shotgun |
| 4 | Gatling Gun |
| 5 | RPG |
| 6 | Pipe Bomb / Dynamite / Holy Hand Grenade |
| 7 | Energy Weapon |
| 8 | Flamethrower |
| 9 | Buffalo Rifle / Crossbow |
| 0 | Freezer |

Upgrades stay on the base weapon's key. **Wheel up/down** (or **semicolon/apostrophe**)
selects previous/next usable weapon; **X** restores the last successfully equipped
weapon. Lit dynamite must finish its original throw/fuse before a requested
weapon switch completes. A usable pipe-bomb remote remains accessible without spare ammo.
**M** uses an owned portable medkit, **J** toggles the jetpack, **N** night vision,
and **B** the Bio Mask through original use rules (TTK’s own gas mask — not scuba
or boots). Wait for gadget equip/remove and
weapon redraw to finish before another shortcut. **J** also switches the jetpack
off in mid-flight (a controlled fall follows). Underwater air still equips automatically.
**[ / ]** cycle eligible gadgets; **U** uses the selected gadget. **R has no stored-dose action:** TTK activates
steroids on pickup. **Q kicks only while standing still with Mighty Boot already selected on 1.**
Armed quick kick is unavailable; Q never selects another weapon or presses fire.

**Alt + wheel up** brings the camera closer; **Alt + wheel down** moves it farther.
Distance is clamped and smoothed, retains wall collision, and remembers your
preference despite wall compression. It is now **saved in the Modernized profile**
and restored on the next launch. It does not change FOV or switch weapons.

**V** recenters the camera: it swings smoothly back to the original follow angle
behind Duke. Moving the mouse cancels the swing. On foot Duke already turns with
the view, so V mostly levels the camera's up/down angle.
**H** cycles the camera shoulder: centered, right shoulder, left shoulder. Shots
still go to the crosshair; walls still pull the camera in. The side you choose
is saved too. Both keys are rebindable (`camera_recenter`, `camera_shoulder`).
Save a distance or side without launching:

```sh
python3 recomp/tools/local/run.py --camera-distance original --shoulder center --show-settings
python3 recomp/tools/local/run.py --camera-distance 3000 --shoulder right --show-settings
```

`--camera-distance` takes `original` or 768-6144 game units; `--shoulder` takes
`center`, `right` or `left`. Both update your saved Modernized preferences.

**P** toggles **first person** (prototype, Modernized with the independent
camera). The view moves to Duke's eyes with a wider field of view; the mouse
looks, WASD moves relative to the view, and shots go to the crosshair. Crouch
lowers the view, and jumps keep it. Duke's head and body are hidden. With a weapon
drawn you see Duke's own gloved hand holding it at the lower right, always in
front of walls, with the original muzzle flash and a small kick when firing.
Each weapon is framed to point at the crosshair; the HUD stays on top. Holstering or drawing makes it disappear or
appear at once. Swimming and jetpack flight switch back to third
person on their own and return when you land; ladders, ledges, scripted and
turret cameras, death and menus use the original camera as before. Pressing P
again returns to third person. The choice is saved; to set it without
launching:

```sh
python3 recomp/tools/local/run.py --view first --show-settings
python3 recomp/tools/local/run.py --view third --show-settings
```

Walls, floors and props right next to you are now drawn correctly in first
person (no more black or broken wall pieces when you press against a wall and
look along it). This is new and still being playtested; if a wall still breaks
up somewhere, note where you were. To compare with the unclipped view, launch
once with the clipping off (this does not change your saved settings):

```sh
DNTTK_NEAR_CLIP=0 python3 recomp/tools/local/run.py
```

In first person, props you walk right up to (doors, sign poles) now stay
solid. The original game fades them to see-through so Duke stays visible in
third person; third person keeps that. To see the original fade in first person
for one launch:

```sh
DNTTK_FP_OCCLUDER_FADE=1 python3 recomp/tools/local/run.py
```

Both switches are planned options for the future settings menu.
The key is rebindable (`camera_view`). Ctrl crouch uses a quicker original transition, not toggle or
Z fallback. Controls are currently bounded to supported first-map states and
still need gameplay acceptance. Existing customized bindings are retained; use
`--show-bindings` to see migrated keys. These actions assume the game's default
Controller layout.

**W/S move forward/back relative to the camera; A/D strafe relative to it.**
Walk is the default on each launch. Hold Left Shift to run; **Caps Lock** toggles
autorun (centered `RUN MODE ON` / `RUN MODE OFF`), and Shift then temporarily walks.
Autorun survives capture/focus/pause
changes within that session, but resets off on relaunch; it does not follow the
keyboard Caps Lock light. A Shift (or any bound gameplay key) that is already
held when the mouse is recaptured — after Escape/resume, F7 load, inventory or
F10 — is read from the keyboard state, so running continues without re-pressing. Your existing speed-modifier binding is preserved
(the binding editor calls it `walk` for compatibility). These speed controls apply
to supported Modernized locomotion; original fallback controls remain available. Diagonals are
normalized. Walk/run changes restart the corresponding gait promptly (soft-keeping walk
phase inside a run anim caused a stiff frozen pose and was reverted); speed and deceleration come from its original animation, and the
original collision routines remain active. Normal stride speed pulses are smoothed
and released-input braking is shorter. Jump gravity and climbing motion remain
original. Duke’s facing remains independent of
movement. In supported normal first-level states, the new crosshair marks the
view target and supported shots aim toward it from the muzzle. Existing aiming animations now point the arms/gun along the view, including
pitch, while Duke stays upright. Recoil, draw/holster and poses without an aiming
branch retain their original animation; close-range muzzle parallax can differ.
Sideways/backward movement still uses the original footwork. This D07A change
requires playtesting; see [behavior and limits](documentation/22-view-facing.md).

Mouse pitch is limited to 60° up/down. Walls still constrain camera position,
but no longer redirect the supported mouse-look orientation toward Duke. There is no automatic recenter while the independent
camera owns control. Releasing capture returns camera ownership to the game;
recapturing seeds from its current view and discards old mouse motion. Supported
running jumps keep takeoff movement direction and the normal mouse camera. Keep
the movement direction held to continue running after a clear landing; releasing
it allows the original stop. There is no air steering. Obstacle impacts retain
the original recovery. Both alternating running-jump animations are supported.
Standing directional jumps also retain their takeoff direction and mouse camera;
they use TTK’s original standing-directional arc. **Modernized swimming** (D08M foundation and D08O polish both Done): shallow-water
**Space** jumps like on land. Facing a raised ledge such as the subway
platforms, **Space** (alone or with W) hops you up onto it from the wall or
from a run-up — the ordinary jump, a little taller, that waits for the feet to
clear the lip before moving in; no distance puzzle. Facing a plain wall,
W+Space is a vertical hop.
On the surface of deep water, face the camera: **W/S** swim forward/back,
**A/D** strafe (camera-relative), **Ctrl** dives. Underwater, **W/A/S/D**
swim where the camera looks, **Space** swims up, **Ctrl** swims down, and
Duke surfaces automatically when he reaches the top. Leave the water with
**E** / mantle — Space never hops you out at a ledge. Water/oxygen stay with
the original game.
Details: [documentation/55-swim-controls-research.md](documentation/55-swim-controls-research.md).
Underwater air remains automatic; there is no scuba item and none will be added.

**Modernized jetpack**: switch it on with **J**, then
**Space** lifts off and climbs. In the air the **mouse** turns Duke and the
camera together, **W/A/S/D** fly relative to the camera at level height
(strafe is a little slower, as in the original), releasing every key
**hovers** in place, **Ctrl** descends quickly (the same speed as diving
underwater) until a soft landing, and
**J** switches the pack off for a controlled fall with the camera still live.
Fuel drains while flying and while hovering (the original rule); when it runs
out Duke falls the same way. The ground controls return the moment he lands.
Details: [documentation/57-jetpack-controls.md](documentation/57-jetpack-controls.md).

**Classic jetpack (optional, Modernized only)**: modern controls with the
original burst-style flight. Select it once from the terminal; it is remembered:

```
python3 recomp/tools/local/run.py --jetpack classic --show-settings
python3 recomp/tools/local/run.py --jetpack modern --show-settings
```

Modern (above) is the default. Classic keeps the same controls - mouse turns
Duke and the camera, **W/A/S/D** fly relative to the camera, **J** toggles the
pack - but the flight is the original burst style: **Space** boosts, and the
original gravity pulls Duke down whenever you are not boosting. There is no
hover on release and no **Ctrl** descent; **Shift** toggles the original hover.
Fuel, lift and cut-out are the original's. Vanilla is unaffected by this choice.
Standing-jump input ordering and feel are accepted;
broader terrain and campaign coverage remain separate work.
For a fully original camera in Modernized, select **7 Camera** in settings and
choose `original`. That menu also edits sensitivity (degrees per mouse count,
0.01–2; default 0.12) and vertical inversion. Vanilla always uses its original camera.

For example, save preferences without launching:

```sh
python3 recomp/tools/local/run.py --mode modernized --camera independent --mouse-sensitivity 0.12 --invert-y off --show-settings
```

The wall-facing clearance check now follows your requested WASD direction, and
mouse orbit remains active during the two verified wall-bump animations. The
final view update also retains mouse yaw/pitch when a wall pushes the camera
to one side. Original close-camera body fading remains.
Walls, slopes, steps, tight camera spaces and progression still need playtesting
with these controls. Release capture and use arrows when a state falls back to
the original game.

**Weapon aiming preview (D07):** select **8 Weapon aiming** in settings.
The original red dot indicates autoaim targeting; it is not a laser sight.
`view` aims supported shots from the physical muzzle toward the view target;
it defaults to unassisted aiming. At close contact the shot origin retracts
when needed to avoid aiming backwards from an overlapping muzzle; original cover
and damage rules still apply. `original` restores the original
weapon path, including the in-game auto-aim setting, and hides the modern
crosshair. Selecting original camera also retains original body/gun presentation.
Holding right mouse keeps the same view target, body facing and weapon pitch in
supported Modernized states. Select `original` weapon aiming or original camera
for the original right-mouse precision behavior. Vanilla retains it as well.
**9 Aiming display / assistance** configures three independent settings:
`off` or experimental `original-lock` assistance, crosshair on/off, and red dot
on/off. Assistance uses an existing game target within six degrees of view center,
with separate camera and muzzle visibility checks. Enable original autoaim in the
game for its target acquisition. Unsupported weapons retain original aiming.
The crosshair marks view center; assistance can correct within that cone. The
red dot remains the original game's marker. Hiding either marker does not change
assistance. These new options and natural weapon use still need playtesting.
Vanilla always retains original aiming. Save without launching with:

```sh
python3 recomp/tools/local/run.py --weapon-aim original --show-settings
```

For assisted view aiming with a crosshair and no original red dot:

```sh
python3 recomp/tools/local/run.py --mode modernized --weapon-aim view --aim-assist original-lock --crosshair on --red-dot off --show-settings
```

These commands save preferences without launching. Old profiles are backed up
and migrated automatically, preserving custom bindings; use `--show-bindings`
to inspect them. See [the follow-up controls and aiming notes](documentation/24-d07-controls-and-aim-options.md).


The crosshair describes the intended view target, not a confirmed hit. Shots
start at the muzzle, so nearby cover can intercept them even when the camera
sees the target. The adapter checks for a muzzle protruding through cover.
Original pellet spread, projectile travel, gravity, bounce and damage rules
remain. No target leading or gravity compensation is added.

First-map checks now cover the pistol, shotgun, rifle, Gatling, RPG, freezer,
crossbow, knife/axe, pipe bomb, grenade, dynamite, flamethrower and energy weapon.
RPG flight and explosions follow pitched view aim. Throws keep their original
charge, speed, gravity and fuse. Energy uses the view direction instead of an
implicit actor lock; optional assistance still requires visibility. Upgraded
variants have private dispatch-fixture coverage. Natural campaign acquisition,
other eras and your aiming feel still need playtesting. The crosshair disappears
when holstered, released or outside supported states/maps.
See [weapon coverage and remaining checks](documentation/21-modern-weapon-aiming.md).

In Modernized, **arrows**, **X** (confirm/fire), **Z** (menu back/quick-turn),
**Scroll Lock** (draw/holster; shows centered `WEAPON LOWERED` / `WEAPON RAISED`),
**Enter** (use selected inventory gadget) and **Right Shift** (inventory) always
remain available. Menu Circle still accepts **C** when uncaptured. Use Right Shift to close inventory. These fixed keys keep menus
operable after rebinding. Vanilla keeps the different original keys below. Escape pauses.

**Backtick (`)** opens a Quake-style drop-down developer console (smaller Duke
font). Typed commands and their replies stay in the console scrollback (`help`,
`fps`, `clear`, `quit`, unknown-command errors). `fps` also toggles a compact
persistent statistics block in the top-left of the game view after you close the
console (F remains unbound for gameplay). Escape or backtick closes the console.

**I** toggles the Modernized crosshair on/off (EDuke-style). The original TTK
weapons/inventory screen stays on **Right Shift** (Select); press it again to close.

Choose **6 PC bindings** in `--settings` to edit Modernized actions. The editor
lists accepted keys and reports conflicting assignments without changing them.
You can also save a binding and return without launching:

```sh
python3 recomp/tools/local/run.py --mode modernized --bind jump=J
python3 recomp/tools/local/run.py --show-bindings
```

The labels use US physical key positions. Mouse1/2/3 mean left/right/middle;
Mouse4/5 are side buttons. To swap occupied inputs, enter both assignments at
once. Restoring the Modernized profile restores its bindings, camera preferences and renderer.
Keep custom runtime host shortcuts separate from these action bindings.

## Vanilla default controls

These are the original game's default actions. Changing its Controller options changes those actions. PC keyboard bindings translate keys into PlayStation buttons.

| Action | PC key | PlayStation button | Xbox-style controller |
|---|---|---|---|
| Move forward/back; turn left/right | Arrow keys | D-pad | D-pad / left stick |
| Fire; action with weapon holstered | X | Cross | A |
| Jump; swim/thrust | Z | Square | X |
| Draw/holster weapon | S | Circle | B |
| Cycle weapons | Hold S + Left/Right | Hold Circle + Left/Right | Hold B + Left/Right |
| Quick turn; hold to crouch/stand | A | Triangle | Y |
| Walk | Hold Q | L1 | Left bumper |
| Precision look/aim | Hold W + arrows | R1 + D-pad | Right bumper + direction |
| Strafe left | E | L2 | Left trigger |
| Strafe right | R | R2 | Right trigger |
| Weapons/inventory | Right Shift | Select | Back / View |
| Pause/options | Escape | Start | Start / Menu |

Use **Up + X** to grab/climb where the game allows. Movement follows the original turning controls; mouse aiming is not implemented.

For keyboard remapping, edit the runtime's `keybinds.ini` after closing the game. Its startup log prints the loaded file path. Keep the in-game Controller layout at its default when using this table. Controller support depends on SDL recognizing your device.

## Saving and loading

Save when offered after completing a level. Load from the game's menus. Re-entry points and continues are not permanent saves.

The normal launcher stores memory-card data under `recomp/saves/local-play/`. Keep that folder between builds, and back it up while the game is closed. Development playback tests use separate save folders.

## Playback and troubleshooting

Earlier FMV checks reached normal intro speed without audio underruns. **Stuttering audio with slowed play in Modernized mode** (Vanilla unaffected) had a measured cause — the control layer's original-code check was re-reading 81 KB of guest memory about 46 times every frame, costing 5 ms of each 16.7 ms frame and starving the audio output — and is fixed (confirmed in play; see [58-modernized-frame-budget.md](documentation/58-modernized-frame-budget.md)). If it returns, note the scene and send the session log from `recomp/build-local/logs/`. **Intro-movie stutter** had a separate cause: the compiled movie code was left behind when the control hooks changed. The build and the launcher now refresh it automatically (about a fifth of a second), and the session log says `ttk-fmv: native movie decoder active` or prints a warning with the fix ([59-fmv-shard-namespace.md](documentation/59-fmv-shard-namespace.md)). Earlier crackle notes: [46](documentation/46-turning-audio-inventory-progress.md). Keep `recomp/build-local/cache/` with the executable: it contains the compiled movie code. If it is missing, rebuild it with `python3 recomp/tools/local/build_movie_overlay.py`. Close other game instances before testing. Press Enter to skip the intro if needed. Full-campaign accuracy is still being verified.

To try the software renderer:

```sh
python3 recomp/tools/local/run.py --renderer software
```

Close the game window to exit. When reporting a problem, include the level or movie, what happened, and whether it also happens after restarting. Technical progress and evidence live in [the project documentation](documentation/README.md).

Original action descriptions were checked against the [USA instruction manual](https://dlf.emu-land.net/manuals/psx/Duke%20Nukem%20-%20Time%20to%20Kill%20%28U%29.pdf), printed pages 4–9. Keyboard and controller mappings were checked against this checkout's runtime defaults.

### Furniture movement

Contact furniture jumps wait for the original foot position to clear the obstacle
before adding forward travel. Small furniture drops retain approach speed; large
falls keep their original behavior. The apartment's concealed pipe bombs wait for
the bed secret to open. Nearby switch targeting is more forgiving; the original
interaction and lights-before-dialogue secret remain. Vanilla retains its original
rules. Bed/couch contact, walk/run departures and release checks passed privately;
your movement feel and broader terrain still need playtesting.

### Typed debugging cheats (Modernized)

Stand on solid ground with mouse capture active and type a code directly—no console
or Enter. A message confirms the result. The first D can move Duke slightly, so
start away from ledges. F10 toggles capture; Escape or focus loss cancels typing.

| Code | Effect |
| --- | --- |
| `dnmonsters` | Toggle enemies hidden/shown; NPCs, pickups and switches remain |
| `dnkroz` / `dncornholio` | Toggle god mode (does not restore health) |
| `dnstuff` | Grant all weapons/ammo, inventory and keys |
| `dnkeys` | Grant keys |
| `dnweapons` | Grant weapons and ammo |
| `dninventory` | Grant inventory |
| `dnitems` | Grant inventory and keys |
| `dnhyper` | Activate/refill steroids |
| `dnammo` | Refill ammo for owned weapons |
| `dnhealth` | Restore 100 health |
| `dnunlimited` | Toggle unlimited ammo/charges |

These use Time to Kill's inventory equivalents. Use a test save and show enemies
again before saving or loading: hidden-enemy state is session/scene-local, and its
save/load round trip is not supported. Codes are unavailable in Vanilla, menus,
multiplayer, while dead, or during attached traversal. No clipping or level-warp
cheat is included. See [implementation and testing notes](documentation/38-debug-cheats.md).

### Duke message fonts

Modernized host messages and the developer console use the owned Duke Messages
sprites under `recomp/assets/fonts/Messages` (built into `ttk-fonts.pack`). Their
letter art has uppercase shapes; the underlying confirmation wording remains
unchanged. The original game text and Vanilla presentation remain original. Keep
`ttk-fonts.pack` and its provenance JSON beside the executable when copying a
local build. If the pack is absent or invalid, messages use the generic host font.
The Atomic style is available to shared heading rendering; current messages and
console text use the small Messages style. Research extracts are reference only —
copy into `recomp/assets` before shipping. See
[implementation and verification](documentation/40-feedback-implementation.md)
and [session wrap](documentation/52-session-late-polish.md).

### Inventory feedback and crouch scope (2026-09-28)

In Modernized gameplay, **[ / ]** open the temporary gadget **switcher** (centered
strip with Duke inventory icons and green THREEBYFIVE charge); **Enter** or **U**
activates the currently selected gadget (whether or not the strip is showing).
Holding Enter activates at most once. Empty inventory stays silent. Direct keys
**M / J / B / N** select and use medkit / jetpack / Bio Mask / night vision **without**
opening the switcher (**M** shows centered `MEDKIT N%` when owned; **J** / **B** / **N**
show `JETPACK ON/OFF`, `BIO MASK ON/OFF`, `NIGHT VISION ON/OFF`). Bio Mask is TTK’s
own gadget — not scuba or boots. Underwater air still works automatically (no
`SCUBA GEAR ON` toast and no scuba item). Gold corner marks an active toggle.
Depletion falls back to the first usable gadget. Menus and released capture hide
the strip. Selector frame is locked at 50×60 with a −6px frame-only vertical
nudge. Existing custom bindings still apply. The Modernized view crosshair is
EDuke’s CROSSHAIR tile (2523). See
[session findings](documentation/51-session-inventory-crosshair.md).

Typed cheats stay silent until a completed result appears, horizontally centered
at the top of the screen. Partial, invalid and cancelled entries display nothing.

True crouch walking is **not implemented**. Held crouch plus direction retains the
original roll. The [asset/collision audit](documentation/43-crouch-walk-audit.md)
records the concrete work needed for a safe low gait; original assets are unchanged.
