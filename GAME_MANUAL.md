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

Click the game window to give it keyboard focus. Press **Enter** to skip the intro. Use the arrow keys in menus, **Enter** (or **X**) to confirm and **Escape** to go back (Modernized; see [menu keys](#menu-keys-modernized)). Choose a new game to start Duke's campaign.

If the executable is missing, follow [build and run](documentation/04-build-and-run.md). If the prepared disc is missing, the launcher searches `game/` for a known USA dump and copies it into ignored `recomp/disc/`. Original files are never overwritten.

The tools accept a dump whose image SHA-256 matches a known USA copy (CloneCD or Redump USA MODE2/2352) and whose boot EXE is `SLUS_005.83`. A Europe/PAL disc, or a USA image with a different hash, prints an error on the console with the accepted SHA-256 values and does not launch.

## Profiles and settings

Every launch starts in **Modernized**. To play the original game (Vanilla:
original controls, camera, menus and presentation), add `--vanilla`:

```sh
python3 recomp/tools/local/run.py --vanilla
```

`--vanilla` applies to that launch only; the next plain launch is Modernized
again. (Earlier builds remembered Vanilla; a saved Vanilla choice moves to
Modernized once, with a notice, and keeps your Vanilla settings.)

The terminal settings menu changes the profile you launch:

```sh
python3 recomp/tools/local/run.py --settings
python3 recomp/tools/local/run.py --vanilla --settings
```

Change OpenGL/software rendering, the display options, or restore that
profile's defaults. Save and return, then launch normally.
Each profile remembers its own renderer and resolution/display settings. Modernized also remembers its PC action bindings, camera selection, mouse sensitivity and inversion. Changes take effect on the next launch. In Modernized most of these can also be changed in the game, from OPTIONS ([in-game OPTIONS](#in-game-options-modernized)).
**Modernized movement and camera have passed user playtesting in the first-level preview.**
Supported first-map climbing uses the original traversal routines with Modernized
input. In the crystal-2 flooded turret corridor, waist-deep water uses the same
camera-relative movement, mouse look and Shift walk/run as dry ground - it is not a swim.
The deeper wade along the right-hand ledge (where the original slows Duke to a
forward-only wade) also moves at the land walk or run pace toward where you look, strafes, and
jumps on Space. Wading straight into a wall stops Duke, as in the original:
turn the view and he moves again. Reaching that ledge no longer switches the
whole area to original controls (a level-script write used to fail the code
identity check).
Escape then resume recaptures, and keys you keep holding across that pause
(Shift, W, Ctrl…) stay held — releasing and re-pressing Shift is not needed.
F7 load of the turret-corner savestates (UI slots 2 and 3) also recaptures automatically,
again keeping held keys. Capture is not taken inside the original pause menu.
Modern controls never switch to tank movement when mouse capture or a guarded
control hook is temporarily unavailable. Capture returns automatically when
verified gameplay resumes. Unknown code still fails its identity checks; the
hooks are not forced onto it. Report any remaining loss of control with the
session log in `recomp/build-local/logs/`.
**F10 has no action.** **Tab has no action** either: the fast-forward key was
removed (2026-10-29) and Tab is reserved for a future game action. Use Escape to pause and free the cursor, then resume to
recapture. In **OPTIONS > CONTROLS > CONTROL SCHEME**, choose **MODERN**
(default) or explicitly choose **LEGACY** and CONFIRM. The choice applies
immediately and survives relaunches; loading a save cannot change it.
Changing to **LEGACY** also sets **AUTO AIM ORIGINAL**, **RED DOT ON** and
**CROSSHAIR OFF**. Changing back to **MODERN** sets **AUTO AIM HITSCAN**,
**RED DOT OFF** and **CROSSHAIR ON**. These are starting presets: you can
adjust the individual settings afterward. Manual adjustments survive restarts
and save loads; they reset only when you confirm a different control scheme.
Cancelling the Controls page changes none of them.

Legacy uses original turning and aiming with the configured action bindings:
W/S forward/back, A/D or Left/Right turn, Space jumps, left mouse fires/actions,
Ctrl crouches, right mouse aims, Scroll Lock draws/holsters, and I opens the
original inventory. Modernized menus and save features remain available.
Choose MODERN in the same settings panel to return to independent movement,
camera and aiming. Vanilla remains a separate one-launch option.

### Resolution and display

Each profile remembers its own picture settings. In `--settings`, choose **R**
and enter four values, for example `2 windowed 0 linear`:

- **Internal resolution** `1`-`4`: draws the 3D world at that many times the
  original resolution, so models and edges get sharper (textures keep their
  original look). `1` is the original. Vanilla starts at 1, Modernized at 4.
  Higher values need the OpenGL renderer; with the software renderer the game
  always uses 1 and keeps your choice for later.
- **Display**: `windowed` (the default), `borderless` (fullscreen at your
  desktop resolution) or `exclusive` (true exclusive fullscreen). **F11**
  switches between a window and fullscreen in game, using the last
  fullscreen kind you chose. The game remembers how you left it: if you
  quit in a window (including one you resized), or in fullscreen, it opens
  the same way next time.
- **Window width**: `0` fits the window to your screen; otherwise 640-7680
  pixels. The height follows the picture shape (4:3, or the widescreen
  shape below).
- **Output filter**: `linear` smooths the picture when it is enlarged to the
  window; `nearest` keeps hard pixels.

The same settings are available as launcher flags, for example
`python3 recomp/tools/local/run.py --internal-scale 3 --display borderless`.
Changes take effect on the next launch. If the game slows down, lower the
internal resolution.

### Widescreen (Modernized)

Modernized opens in **16:9 widescreen** by default. You see more of the world
at the left and right; nothing is stretched, and the middle of the picture is
exactly what the original showed. The health and ammo boxes move to the screen
corners. Movies stay 4:3 with black bars at the sides. Loading backgrounds,
including those behind menus, repeat their artwork to fill the widescreen
picture; the spinning logo and menu text keep their original shape. Aiming
and the crosshair are unchanged. Unlike an emulator's
widescreen hack, text, the HUD and menus keep their original shape instead of
being stretched sideways.

Choose the shape with `--widescreen`, or with **W** in `--settings`:

- `16:9` (default), `16:10` or `21:9`: a fixed widescreen shape.
- `auto`: follows the window, from 4:3 up to 21:9. In borderless fullscreen it
  matches your monitor.
- `off`: the original 4:3 picture.

For example `python3 recomp/tools/local/run.py --widescreen 21:9 --display borderless`.
This updates your saved Modernized preferences. Vanilla always uses the
original 4:3 picture.

### Frame rate (Modernized)

The game keeps its original simulation speed at every setting: Duke, enemies,
weapons, physics and timers do not run faster when you raise the display rate.
Native scene updates normally run at 30 frames per second and can fall lower
in busy areas. High refresh draws additional pictures between those updates.

Choose it with `--frame-rate`, with **F** in `--settings`, or in game under
OPTIONS > DISPLAY > FRAME RATE, which applies as soon as you confirm, also
after a launch at 60 (OpenGL renderer; with the software renderer it waits
for the next launch):

- `auto` (default for new Modernized settings): one picture for every
  display refresh, up to 240 FPS. Examples: 60 Hz -> 60 FPS, 120 Hz -> 120,
  144 Hz -> 144, 165 Hz -> 165, 180 Hz -> 180, 240 Hz -> 240; faster
  displays use an even fraction (360 Hz -> 180). Where a scene is too heavy
  for the computer to keep up, pictures drop evenly to every second or third
  refresh (180 Hz -> 90 or 60) instead of slowing the game, and return to the
  full rate once the view is lighter. Unknown refresh falls back to 60.
  AUTO follows display changes and preserves fractional rates such as 59.94.
  Existing saved choices stay selected; choose AUTO explicitly to try it.
- `60`: the original presentation.
- `display`: your monitor's refresh rate (for example 180 on a 180 Hz screen).
  It follows the window if you move it to another monitor.
- `30`: each new game image is shown exactly once.
- `120`, `144`, `165`, `180`, `240`: a fixed target rate.
- `unlimited`: the fastest paced redraw rate the scene can sustain; useful
  for stress testing, with no promise of one new image per monitor refresh.

Above 60, the extra pictures in between are drawn by the game's own renderer
with the camera, Duke and moving objects placed part-way between two game
frames, so turning and movement look smoother. Each refresh of your display
gets its own picture where the PC keeps up (at 120 Hz, three in-between
pictures per game frame). This uses spare CPU cores (Linux); at very high rates
the busiest scenes get fewer in-between pictures and repeat one now and then.
Particles, flashes and the HUD still update 30 times a second.

The title screen, the demo behind PRESS START and the main menu are smooth
too. The original game updates them only 10 times a second; above 60 they get
in-between pictures like gameplay, and the demo still plays exactly as
recorded. Menu text and the highlighted entry still change at the original
rate, but in Modernized above 60 the new highlight or screen appears about a
tenth of a second sooner than before. In Modernized, a quick tap on a menu key is held until the menu has
read it, so short presses are no longer lost, even while a loading screen
plays behind the menu or on LOAD GAME (where the memory card slows the
game's reading of the controller). Quick taps made while one is still
waiting are kept and carried out in order. While the next demo scene starts
loading, the original hides the menu for about two seconds; keys pressed then
take effect when it comes back. Over the spinning logo the menu responds as
quickly as over the demo. Holding a key repeats as before. The intro movie
and Vanilla are unchanged.

Loading screens (the spinning radiation logo) are smooth as well: above 60 the
logo turns evenly at your frame rate instead of stepping 10 times a second
(between demo scenes) or 30 times a second (level loads). Loading takes
exactly as long as before. When the game pauses its own drawing for a moment
during a load, the logo pauses too, as in the original.

The sky keeps its original cloud layers and animation. The user-accepted D17O
fix keeps those layers centered on the current view during high-refresh turning.
It applies automatically. Original sky geometry and coarse animation steps remain.

A level with no sky of its own may close an opening with a painted sky picture
(GOLD AND GUNS's mine shaft). Modernized opens that skylight onto Duke Hill's
sunset sky, drawn like the game's own sky: it turns as you look around, never
moves as you walk, and its clouds drift. Vanilla keeps the painted picture.

In PIG FACTORY, parts of the flooded room's pool are water you can swim in,
but the original never drew their surface: the two square shafts off the pool
(beside the brick pillar and across from it), where looking down from the
ledges showed dry walls, and the two corners of the sloping beach on the far
side. Modernized draws the pool's water across all four. Vanilla keeps them as
shipped.

Above 60, each redraw uses recent mouse movement for the view direction
(the eye position, Duke and other objects stay in step with the game).
Sampling is scheduled ahead of presentation to allow the worker to finish;
that lead adapts to recent slow redraws. This adds latency, especially in busy
scenes. Shots and aiming still follow the game's own camera updates.
Fixed-rate and Match Display modes retain the requested cadence. A missed
deadline can still repeat an image. **D17A and D17B are player-accepted.**
120 and 180 FPS are the primary quality/regression targets (AUTO on 120 Hz
and 180 Hz displays); 144 and 165 FPS are checked on every test save, 240
FPS is supported with even steps down in heavy scenes, and Unlimited is a
stress/debug mode. When a heavy view would slow the game enough to be heard
(the sound buffer running low, including just after loading a save), the
frame rate steps down at once instead of letting the sound crackle; it steps
back up when the view gets lighter.

The original-style 60 setting normally presents roughly 30 new game images
per second with repeated views. 120 is not secretly 60: it can render about
120 distinct intermediate views/s while gameplay keeps its original update
rate. Under load, some views can repeat. The overlay separates FPS (presents),
Unique (distinct submitted images including redraws), Game (compositions),
Guest (fields) and RT (realtime speed). See the
[accepted baseline and focused follow-ups](documentation/86-d17-acceptance-and-regression-baseline.md).
The FPS readout counts presentations, including any repeated images.

Holding fire with the weapon holstered draws it with a single tap of the
original holster button, so Duke never drops into the hold-to-select item
mode by accident. Holding your own holster key still opens it, as in the
original.

Modernized improves perspective and clipping for nearby walls, floors, tables
and props. The depth correction is player-accepted for the club furniture's
lower shelf and the apartment/strip-club closets. Ground blood and the tested tabletop props are also player-accepted.
Other isolated visual reports remain separate follow-ups.
The original PS1 character during movement is preserved; Vanilla keeps the
original presentation.

Since D17AM3 Modernized draws every wall, floor, prop and
character itself at every distance, with the game's own colours, textures and
drawing order plus a depth test, so distant scenery no longer overlaps in the
wrong order and nothing changes look at the distance where the old near and
far drawing paths met. The sky, water, effects, weapon, HUD and menus keep
their accepted handling. Wading through flooded areas, the water around Duke
is now drawn instead of vanishing. A cause of extreme colour corruption (and
a rare stop) just after loading a savestate is fixed (D20H).

### Geometry and texture precision (Modernized, OpenGL)

D15's player-accepted options independently reduce geometry jitter and texture
perspective distortion. Choose **G** in `--settings`, or use
`--geometry-precision original|corrected` and
`--texture-precision original|corrected`. They apply on the next launch and
save your preferences. Both default to `original`; choose `corrected` for the
stabilized presentation. Vanilla and the software renderer use Original.

For the accepted 120 Hz presentation:

```sh
python3 recomp/tools/local/run.py --renderer opengl --frame-rate 120 --geometry-precision corrected --texture-precision corrected
```

This updates saved preferences. Correction preserves the game's simulation
and collision, and some characteristic PS1 movement remains. The accepted
floor seams, idle settling and close-up opacity fixes remain in place.

With texture precision `corrected`, nearby walls, floors and ceilings are also
drawn as whole surfaces instead of the original's screen-space pieces, so
patterns and borders no longer bend along diagonal lines or shimmer while you
walk. With `original` textures the original pieces remain.

### Draw distance (Modernized)

The original game stops drawing a few rooms ahead, so long views such as the
subway corridor or the castle moat end in a black box, and thin black lines can
cross distant ceilings and floors. `extended` (the Modernized default) draws as
far as the PlayStation's depth range allows (65535 units, two to three times
the original) and closes those distant seams; with geometry or texture
precision `corrected` it also keeps thin distant surfaces.

Outdoors, the original only draws neighbouring areas through their openings,
so flying high with the jetpack showed pieces of the level popping in and out
as you turned. With `extended`, outdoor areas you can see over the walls are
drawn whole. Some spots stay black because the level has no scenery there.
Over the walls you see only the open air you are in: from the moat or the
jetpack the whole moat and the outside of the castle, from inside a walled
courtyard just that courtyard and what its openings show. The two faces of a
castle wall belong to different areas and were built to be seen only from
their own side, so the far face no longer pokes up above the battlements as
stray spires and tips. This holds for both renderers, Polymorph and Classic.
In wide views such as the castle from the jetpack, distant walls no longer
flicker with black holes while you turn: every distant piece now fits in the
frame instead of some being dropped from frame to frame.

With `extended` the game no longer decides which parts of the level are drawn
from the openings it can see through: every surface in view is drawn, in every
level, so scenery does not pop in and out or get cut off at the screen edges
as you turn, walk or fly. Enemies, doors and everything else still play as in
the original.

The original also creates enemies only when you come within its own short view
range and removes them past it, which `extended` would show in the open. With
`extended`, enemies are placed out to the new view limit, as in Duke 3D: a
distant enemy stands where it is and does not react until you come within the
original range, or until you hurt it (a shot, a kick or a blast wakes it for
good). Near you the game plays as the original: the same enemies, the same
limit on how many are alive at once.

Past that limit of live enemies, every other enemy the level places is shown
asleep where it waits: standing still, not yet in play, on security monitors
and in distant views alike. When the game brings that enemy into play it takes
the sleeping one's place. Shooting a sleeping enemy does not wake it yet.

`original` keeps the original limits and behavior. Choose **D** in
`--settings` or `--draw-distance original|extended`; it applies on the next
launch and saves your preference. Vanilla always uses the original. A very
long view can still end in black beyond 65535 units.

### Renderer: Polymorph or Classic (Modernized)

**Polymorph** is this project's renderer and the default: every wall, prop
and character is drawn through one geometry path at every distance, the PC
decides which parts of the world are in view (with Extended draw distance),
and at high refresh rates the game stops drawing frames nobody sees, so the
in-between pictures come faster. **Classic** keeps the game's own drawing
order and visibility, as before Polymorph, with this project's earlier fixes
for distant seams and for views over outdoor walls. First person and widescreen work
in both. Choose it in OPTIONS > DISPLAY > RENDERER: it switches as soon as you
confirm, mid-level, as in EDuke32, and saves your preference. The backtick
console's `renderer polymorph` / `renderer classic` switches for this session
only (`renderer` alone names the current one). `--world-renderer
polymorph|classic` saves the choice from the launcher. A screen flash in
progress stops when you switch to Classic. Vanilla always uses Classic.

### Security monitors (Modernized, Polymorph, D53)

With **SECURITY MONITORS** set to **DUKE 3D** (the default), the game's
security monitors look and work like Duke Nukem 3D's: each is a larger TV in
the style of Duke 3D's own, with a warm grey bezel and a glass screen showing
SECURITY in olive letters, the feed's status and a blinking cursor. The
screen gives off a soft olive light onto the wall around it. Use a monitor
(the action button) and the camera view gets Duke 3D's camera frame: four
corner marks, REC and a blinking red record light, so you always know you are
looking through a security camera. Use it again to step back. The monitors
work the same as before; only their look changes. Keypads you can use are lit up
too, so the part you use stands out, and the lightning power sign beside a
power switch pulses with electric blue light on the wall (either setting). **TTK** keeps the original
monitors and camera views. Change it in **OPTIONS > GAMEPLAY > SECURITY
MONITORS** (it switches at once and saves) or with the launcher's
`--security-monitors duke3d|ttk`. The Classic renderer and Vanilla always
show the original monitors.

### Dynamic lights (Modernized, Polymorph, D41A, D41B, D41I, D41K)

With **DYNAMIC LIGHTS** on (the default), blasts and Duke's muzzle flash put
real light into the world: rockets, grenades, pipe bombs, dynamite, TNT
barrels and characters blown apart light the walls, floors, props and
characters around them in a warm flash that fades over about half a second,
each shot Duke fires lights the ground and walls around his gun for a
moment, and rockets, freezer and energy rounds carry their own light as they fly. Each
gun has its own flash at the point its round leaves: warm for the Desert
Eagle, shotgun, Gatling, Buffalo Rifle and RPG (the shotgun, rifle and RPG
bigger), violet-blue for the freezer, green for the energy weapon, and a
flickering fire light along the flamethrower's jet. In Modernized, a Combat Shotgun shot
fired with the gun lowered puffs and flashes at the barrel, like held shots;
the original put that first puff at Duke's left shoulder (D47). The flamethrowers'
flames light their surroundings as they fly, and anything that burns (an
enemy set alight, Duke himself) glows with a flickering fire light for as
long as it burns. Enemies' shots flash at their guns too: warm for the pig
cops, turrets, bosses and Moloch's gatlings, violet for the gunship's bolts,
and the robotic turret's rockets carry their light as Duke's do. A round
that hits a wall makes a short spark of light there. Thrown weapons and crossbow bolts make no flash; their blasts
light as before, the TNT barrel and dynamite the biggest. The light falls off with distance and faces the surfaces it reaches;
the game's own painted lighting stays underneath. Duke himself, his
first-person weapon, arms and legs take the light too (while a power-up
effect plays on him he keeps his original look). Light rounds smoothly over
characters and props instead of showing their flat facets, and lights only
the side of a figure that faces it. Props (consoles, tables, machines, the
subway train) brighten under a light as much as the walls beside them, and a
strongly coloured light keeps its colour on them instead of washing to grey.
It is a first version: the light still
passes through walls. Turn it off in **OPTIONS > DISPLAY >
DYNAMIC LIGHTS** (it switches at once and saves); the launcher accepts
`--dynamic-lights on|off` and terminal `--settings` uses **I Dynamic
lights**. The Classic renderer and Vanilla have no dynamic lights.

The backtick console's `light` places a warm test light above Duke where he
stands; `light <r> <g> <b> [radius]` picks its colour (0 to 4 for each, 1 is
about as strong as a lamp) and reach (500 to 30000 units, 4000 by default),
and `light off` removes the test lights. `light at <x> <y> <z> <r> <g> <b>
[radius]` puts the test light at a world position instead (colour up to 32,
reach 100 to 30000), for measuring. `light smooth off` shows the
faceted light on models for comparison until `light smooth on` (or the next
launch), and `light fire off` / `light fire on` turns the fire lights
(flames and burning) off and on for the session.

**Level lights.** Levels can also have placed lights of their own, "lamps":
neon signs, lamps and coloured rooms cast real light on walls, props, enemies
and Duke, on top of the painted light. Every level has a first set, found
automatically: one lamp per coloured pool of the painted light, and one at
each glowing prop (EXIT and SECURITY signs, neon, tube lights, lit panels,
lanterns) in its own colour. Lanterns flicker like flames; where the original
light ripples (the game's own animated light), its lamps ripple with it.
Level 0 (Los Angeles) also keeps its flickering pink lamp at the club sign. A
lamp lights only while its room is in view, so a sign behind a wall does not
tint the room in front of it. They come and go with DYNAMIC LIGHTS. The
backtick console edits them:

- `lamp list` shows the level's lamps; `lamp off` / `lamp on` hides and shows
  them for the session (to compare).
- `lamp add [r g b [radius]]` puts a lamp where the crosshair meets a wall
  (`lamp here` at Duke); `lamp pick [n]` selects the lamp nearest the
  crosshair (or number n). The selected lamp blinks for a moment.
- On the selected lamp: `lamp move`, `lamp nudge dx dy dz` (y up is
  negative), `lamp color r g b`, `lamp radius R`, `lamp power K`, `lamp style
  steady|flicker|pulse|candle|strobe|broken`, or `lamp style wave [phase]
  [depth]` to follow the game's own animated light (phase 0 to 31, depth 0
  to 1), `lamp note text`, `lamp del`.
- `lamp save` writes the level's lamps (`recomp/assets/lights/levelNN.lights`),
  `lamp reload` goes back to the saved file, and `lamp clear` removes them all.
- `lamp scan [threshold]` replaces the lamps found from the painted light with
  a fresh set (170 by default; lower finds more); `lamp signs` replaces the
  lamps at glowing props. Each keeps the other's lamps and the ones you
  placed.
- `lamp gate off` lets every nearby lamp light through walls again for the
  session (to compare); `lamp gate on` restores the rule.

Signs and neon keep their own look under any light: they give light, and
neon signs take none on their letters, so they stay as crisp as drawn. Only
the neon face itself is left out: the rest of the prop it sits on (a control
panel beside its screen, the bar's shelves) takes light. Lamps still shine through walls (shadows are a
later job).

### Portable steroids (Modernized)

With `portable` (the Modernized default), steroids work as in EDuke32: picking
them up shows STEROIDS as before but keeps them, and they appear in the **[ / ]**
switcher (right after the medkit) and the HUD item box with their own pill-bottle
icon at 100%. **R**, or **Enter** with steroids selected, takes them:
`USED STEROIDS` shows, the pickup sound plays and the original steroids effect
runs for its usual time. While it runs, the steroids box counts down (lit, with
the pill icon) and steroids stay selected, so the box you were looking at counts
down in place; if you select another gadget, the steroids box moves one row up
(above a jetpack that is on). The armor box on the left shows only armor. The
switcher shows them draining with the active mark.
Duke carries one at a time: while you hold one, more steroids stay where they
are until you have used it. Picking steroids up while they are already running
stops them and keeps the new ones at full, ready for R (as in Duke 3D); typing
`dnhyper` while they run refills them and they keep running. R with none held, while they run, or while
Duke is dead does nothing.

Steroids and armor are separate (D08A13). In the original, running steroids
were also a shield: every hit Duke took was cancelled and cost the steroids
1500 of their 9000 units (about 5 seconds each), so a few shots ended them
early. With `portable`, steroids are only the timed effect: hits damage armor
and health as they would without steroids, and the countdown keeps its own
pace however often Duke is hit. Duke can now die while steroids run. Death
ends them, as in Duke 3D (D08A14): running steroids stop and any steroids you
were carrying are lost, so after Continue there is no countdown, no heartbeat
and nothing in **[ / ]**. `original` and Vanilla keep the original shield and
keep steroids through death and Continue.

Held steroids are kept like the jetpack: through savestates, level completion
and the save offered after a level (and loading it), but not through death. The
level-select travel (`level N`) restarts like the original Restart, which
clears every gadget, steroids included. The original Select inventory does not
list them. `original` keeps the original rule (steroids start on pickup and R
does nothing). Choose **S** in `--settings` or `--steroids portable|original`; it
applies on the next launch and saves your preference. Vanilla always uses the
original rule. See [notes](documentation/127-d08a4-portable-steroids.md).

**Heartbeat (experimental, D08A10).** With `portable`, the click Duke's walk/run
toggle makes (the click Shift used to make before D27A silenced it; Vanilla's
run button still makes it) repeats while steroids run,
about 225 beats a minute, like Duke 3D's steroids heartbeat. It follows game time, not the frame rate, and
stops when the effect ends, is cut by damage, or Duke dies. To silence it,
start the game with the environment variable `DNTTK_STEROID_BEAT=off`; to try
another game sound, `DNTTK_STEROID_BEAT=<id>` (for example `0x1012`, a low
thump). `original` and Vanilla have no heartbeat.

### Power-up countdowns (Modernized, D08A17)

The three coin power-ups, **invincibility**, **invisibility** and **Double
Duke**, start the moment you pick them up and last 20 seconds, as in the
original. In Modernized each running one also gets its own lit box in the
right-hand HUD column, with its own coin icon, counting down in percent like
the steroids box: 100 at pickup, then down to 0, when the box goes with the
effect. Picking up the same coin again sets it back to 100. Running boxes stack
upwards above the selected gadget, a jetpack that is on and running steroids:
invincibility first, then invisibility, then Double Duke. There is no expiry
sound or blink.

Death ends all three (the original already does). After Continue the original
gives Duke about 5 seconds of invincibility while he gets back up; its box shows
that too, starting at 25. Savestates keep them; a level change ends them.
`dnkroz` is a separate god mode and shows no box. Vanilla has no boxes. See
[notes](documentation/132-d08a17-powerup-countdowns.md).

While a power-up runs, Modernized also gives the whole screen a light tint,
the way Quake colours the view for its power-ups (D36M), in each coin's own
colour: gold for invincibility, violet for invisibility and the rose pink of
the Double Duke coin's aura, about an eighth of the way to the colour
(`palfrom 8`). Two or three at once mix their
colours. A hit flashes over the tint and fades back down to it; pickups do
not flash while a power-up runs (the coin itself included). The tint ends the
moment the power-up does, including the Continue invincibility, and at once
when Duke dies. It follows **SCREEN FLASHES** (REDUCED halves it, OFF removes
it). Classic and Vanilla show no tint.

### Dynamite (Modernized, D08A12, accepted 2026-10-08)

In the original game drawing the dynamite lights its fuse at once: it has to be
thrown within about 4 seconds or it explodes in Duke's hand, and a weapon
switch waits for it. With `modern` (the Modernized default) selecting dynamite
only draws it: the stick stays unlit (no sparks, no fizz) for as long as you
hold it, and **6**, the other number keys, the mouse wheel, **X** or **E** put it
away again. Scrolling past it with the wheel is always safe. Fire lights it: the
sparks and fizz start, and releasing fire throws it as before (hold for a longer
throw). From that press the fuse is the original full fuse, so a thrown stick
explodes as in the original; damage, blast and ammo are unchanged. As in Duke 3D,
the next stick is drawn (unlit) as soon as the throw ends; with none left the
game picks another weapon as usual. Holding fire keeps throwing.

`original` keeps the original rule. Vanilla always uses it. Choose **T** in
`--settings`, or:

```
python3 recomp/tools/local/run.py --dynamite original --show-settings
python3 recomp/tools/local/run.py --dynamite modern --show-settings
```

See [notes](documentation/138-d08a12-modern-dynamite.md).

### Swimming (Modernized, D08AB)

**SWIMMING** in OPTIONS > ADVANCED selects water movement behavior.
`MODERN` (the Modernized default) is the D08AA pace: walk, or run with **Shift**,
at the land speeds in shallow water, wading, on the surface and underwater.
`ORIGINAL` keeps the modern controls but restores the original water speeds:
shallow and waist-deep water always run, the surface strokes are the original
fixed speeds (fast forward, slower back, a slow sideways stroke) and
underwater thrust is the original fixed speed, whatever Shift says.
With `MODERN`, walking on the bottom uses normal land jumps, air control
and landing, following the selected JUMPING style. `ORIGINAL` retains the
older wade jumps and special ledge help. Deep-water diving, ascent and
swim-fire controls remain the same in both.
A change applies at once, even mid-swim, and is saved. Vanilla always uses the
original speeds. Choose **U** in `--settings`, or:

```
python3 recomp/tools/local/run.py --swimming original --show-settings
python3 recomp/tools/local/run.py --swimming modern --show-settings
```

See [notes](documentation/157-d08ab-swimming-option.md).

**SWIMMING VIEW** in OPTIONS > ADVANCED > ACTION PERSPECTIVE is independent
of swimming pace. With global VIEW set to FIRST PERSON, Modern controls and
Independent camera, choose FIRST PERSON (default) to keep the eye view or
THIRD PERSON to use the existing third-person swim camera. CONFIRM
applies and saves the choice, including mid-swim. Leaving water follows the
current global VIEW; switching to third person during swimming stays third
person afterward. Water climb-outs still use the mantle's third-person view.

In first-person swimming, Duke's backpack is hidden from the eye view so it
does not swing across the screen during ascent and descent. It remains visible
in third person. With the weapon holstered, Duke's swimming arms reach in
from the screen edge as connected arms, the same as when climbing.

The launcher also accepts `--swim-view first` or `--swim-view legacy-third`;
these update the saved preference. Add `--show-settings` to save without launching.


**JETPACK VIEW** in OPTIONS > ADVANCED > ACTION PERSPECTIVE selects
FIRST PERSON (default) or THIRD PERSON while flying with global VIEW set to
FIRST PERSON, Modern controls and Independent camera. CONFIRM applies and
saves it immediately, including during flight. Global THIRD PERSON always
wins. Leaving flight returns to the current global VIEW.

The choice is independent of JETPACK's Modern / Classic / EDuke32 movement
and fuel scheme. It does not change flight controls, hover or reflected flames.
The launcher accepts `--jetpack-view first` or `--jetpack-view legacy-third`;
these update the saved preference. Add `--show-settings` to save without launching.

### View bob (Modernized, first person)

Standing still, the first-person view settles into a substantially stable
image (the floor and walls no longer visibly breathe with Duke's idle animation). While you
walk or run, the PlayStation's slight geometry wobble is kept, and a view bob
in the style of Duke Nukem 3D gently dips the eye at one steady pace: running
dips deeper, not faster, and the view moves forward evenly rather than with
each stride. Choose its strength with
`--view-bob off|subtle|on|strong` (default `on`) or **B** in `--settings`.
Crouching, jumping, stairs and drops move the eye as before at every setting;
**off** keeps the eye fixed to Duke with no bob.

If the busiest areas (for example the strip club with the dancers in view)
still feel heavy, the emulated CPU option (`--cpu-overclock 150`, or the
settings menu) keeps the game at 30 frames per second there; it does not change
game speed. The western town (for example walking the main street or
looking at the chickens and riders) is one of the busiest places: at the
original CPU speed (`--cpu-overclock 100`) and 16:9 it runs at about 15 frames
per second with uneven steps, while 150% keeps it at a steady 20 (the original
4:3 game runs it at 12-20). If your PC cannot draw every in-between picture
there and still emulate the faster CPU, the game now shows a picture on every
2nd or 3rd display refresh (evenly) instead of giving up the faster CPU, and
returns to every refresh a few seconds after the scene gets lighter.

**Fast CPU timing (Modernized default, D23F).**
Gameplay uses a lighter timing model for the emulated PlayStation CPU. It runs the game at the same emulated speed but uses
far less of your PC's processor, so at 150% and 120 Hz even the western town
keeps a picture on every display refresh. Menus, movies and loading always use
the accurate model. `--cpu-timing accurate` restores the original
model (`--cpu-timing fast` returns to the default); Vanilla always uses it. Savestates work with either setting. The developer
console's `fps` command (backtick) shows what reaches the screen above 60: FPS
(pictures shown per second), Unique (how many of them were different) and Game
(new game frames, normally 30).

For example `python3 recomp/tools/local/run.py --frame-rate display`. This
updates your saved Modernized preferences. Vanilla always uses 60.

Use `--show-settings` to inspect preferences without launching (add
`--vanilla` for Vanilla's), and `--reset-profile vanilla` (or `modernized`) to
restore only that profile's defaults. `--renderer software` remembers the
choice for the profile you launch (`--vanilla --renderer software` for Vanilla). Profile preferences live in `recomp/config/player-profiles.json`;
switching profiles keeps your existing memory-card location. Vanilla keyboard
and physical-controller bindings retain the runtime configuration.

## Modernized apartment secret

In Modernized mode, switching off the first apartment's lights can open the bed
secret without first talking to the woman. Dialogue remains available and the
secret is counted once. Vanilla keeps the original conversation prerequisite.
The user has confirmed the natural switch-first approach works.

## Modernized movement and camera preview

**Supported levels:** Modernized controls and the selected first- or
third-person view work in every level the original level select offers: the
campaign (TIME TO KILL, DUKE HILL through BLOOD BATHS), the six challenge
stages and the three bosses. They carry over by themselves through portals,
the statistics screen into the next level, the pause menu, savestate loads and
the debug level select. The apartment conveniences (light switch, bed pickup)
belong to the first map only. Duke's original dodge rolls keep the mouse
camera and your view; the roll itself plays out as in the original, and WASD
takes over again as soon as Duke is back on his feet. Sliding down a slope too
steep to stand on works the same way. In third person, jumps (including a
Space-only standing jump) keeps the mouse camera.

Modernized gameplay captures the mouse and enables PC actions automatically.
Pause, inventory, focus loss and host menus (including **F7** savestates) release it; verified gameplay can
capture again after returning. **Escape** pauses/resumes; the pause menu always frees the mouse cursor and
resuming captures it again. **Enter** uses the
currently selected inventory gadget (EDuke-style); it never opens the pause menu.
**F10 has no action.** Control scheme selection is only in
**OPTIONS > CONTROLS > CONTROL SCHEME**, followed by CONFIRM.
Loading an F7 savestate restores automatic capture once gameplay is live again.
In supported first-level standing/walking states,
mouse movement orbits the third-person camera. With view weapon aiming enabled,
Duke turns to face the view horizontally when holstered or using a supported weapon.

In Modernized third person, hold **middle mouse** and move the mouse to inspect
Duke and his surroundings. Walking, running, strafing and firing keep the heading
and aim you had before the inspection. The crosshair hides until the camera
returns. Release middle mouse to ease back; **V** also returns the camera (release
middle mouse before starting another orbit). Releasing immediately restores mouse
steering while the camera eases back. Shoulder and distance controls
remain available. **OPTIONS > KEY BINDINGS > CAMERA > FREE LOOK** can rebind or
unbind this action. An existing custom middle-button binding is preserved, with
Free Look initially unbound in that case.

Free Look is limited to supported ground locomotion. First person, swimming,
jetpack flight, attached traversal and scripted cameras retain their existing
controls. Entering an unsupported state, changing view, loading or releasing
capture clears the inspection. Vanilla is unchanged.

**Camera against walls (Modernized third person, D10F1, accepted 2026-11-04).** The
orbit camera stays on its own line behind Duke and never sits inside a wall or a
large piece of furniture such as the apartment wardrobe: when something solid
is in the way it moves in front of it at once, then eases back out (after about
a tenth of a second, over about a third of a second) when the way clears, so it
no longer jumps across a doorway. The closest it comes is about 450 units, and
the shoulder offset shrinks with it. Small objects such as crates still fade
out of the way as before, and Duke still fades when the camera is close. On the
monkey bars the camera stays below the grill, so looking down shows the floor
underneath. Free Look keeps Duke in view the same way. Your camera distance and
shoulder settings are not changed. The Original camera and Vanilla are
unchanged.

**Camera during doors and other use actions (D10F2, accepted 2026-11-05).** While
Duke kicks a door open, uses a switch, wheel, statue, toilet or mirror, or plays
an idle fidget, the third-person camera stays where it was and the mouse still
turns it, with the same wall handling as above. Before, it briefly swung away
to the original camera's position, which could put it behind a wall or the door
frame. Duke's own action, facing and timing are unchanged.

While captured, use **left mouse** to fire/action, **Space** to jump, **Scroll Lock** (tap) to
draw/holster, **hold Left Ctrl** to crouch and release to stand, or **tap C** to stay crouched and tap it again to stand (with W/A/S/D: crouch walk; standing requires overhead clearance, and if Duke ends up kneeling somewhere even lower than a crouch he can always crawl back out through space at least as roomy), **Left Shift** as the speed modifier,
**hold right mouse** (or **Alt**) to grab a pushable object, **comma/period** to browse the mission items (see Mission items below; the original sidesteps are unbound, A/D strafe),
and **I** for inventory. **E is interaction-only and never fires.** If necessary,
a tap automatically requests holstering and then interacts once that animation
finishes. Keep E held for continuous action while climbing. Your weapon is
automatically drawn again after the interaction ends and normal movement returns.
This temporary holster request survives pause and inventory. Supported ladder/ledge
states recapture automatically after pause.
**Playable piano (Modernized).** Tap **E** at the saloon piano to play
MusicMonsterMod's piano performance. Duke keeps his native interaction animation.
The sound comes from the piano: it fades as you walk away and becomes silent
outside its audible radius. Returning while it is still playing reveals the
current part of the tune. Holding E or pressing repeatedly will not layer or
restart an active performance. Pause/load stops it. Effects and master volume
apply; Vanilla retains its original behavior. Distance falloff does not model
walls or closed doors.

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

Backward, sideways and diagonal jumps (D08Z7, accepted 2026-11-03) take off
the same way as a forward jump, in the direction you hold, also when Space is
held through a landing, a hard landing or a bump against a wall: Duke never
jumps forward while you hold S. When something right behind Duke refuses the
jump, he hops straight up and drifts the way you pressed (into the obstacle),
not forward. Running backward or sideways with **Shift** gives the same
running jump as running forward. If the jump lands Duke on a slope too
steep to stand on, a jump from the slide also follows the keys you hold:
toward the slope he climbs it as before, any other way he jumps that way.
With the manual jump style, a jump started with Space alone can be steered
too: jump, then hold S and he comes down behind where he took off.

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
states, settled jetpack flight and swimming, **left-click draws the selected weapon; keep holding to fire**. A quick
click released during drawing leaves it drawn without firing a delayed shot.
Boot retains its unarmed attack. Ordinary ground draw/holster animations run at
1.5x; E-owned airborne stow and its final blend use a bounded 4x animation budget; original firing speed is unchanged. On supported ladders and ledges, **W/S** feed
the original up/down controls, **A/D** feed left/right, and **E** keeps the action
held. The game controls the attached climbing motion and camera. In a settled
ledge hang, **W** pulls up when the landing is clear; **Space** or **S** releases.
Release Space and E before asking for a fresh grab. The apartment platform
pull-up correction (D08T8) is accepted.

**Jumping in shallow water (Modernized, D08AC).** With SWIMMING MODERN,
walking on the bottom uses the same movement, jumps and landings as dry
ground. Third-person walking and running retain their complete land animation
cycles through grounded mid-depth water (D08AD, accepted).
Space alone jumps vertically; W/A/S/D plus Space jumps in the held
camera-relative direction, with Shift for a running jump. The selected
JUMPING style controls the arc and air steering. Water effects remain.
Use ordinary jumps to reach platforms; the older water-specific jump boost
and ledge help remain available only with SWIMMING ORIGINAL.
Deep-water entry resistance is restored (D08AE, accepted). D08AF
blends held movement into swimming as the downward plunge slows,
reducing the hesitation before ordinary swim control takes over (accepted).
Releasing movement retains native plunge deceleration.

**Grabbing a ledge in the air (Modernized, D08X, awaiting playtest).** Hold **E**
as you jump at a ledge within reach and Duke reaches for it from takeoff, catches
it and, with **W** (or E) still held, pulls himself up. Without E the jump
behaves as before, and in the original game a jump that close hits the wall and
bounces off. If a weapon is out, hold E a moment before jumping so the automatic
stow can finish; the game's own rules still decide whether a ledge can be
caught (height, facing and headroom). A jump with **E** held into a wooden
crate or similar climbable object climbs onto it mid-jump, also at an angle and
from one crate to a higher one beside it, as long as its top is within climbing
reach and nothing is stacked on it.
If a jump catches a crate in a stack where there is no room to climb, Duke lets
go straight away and drops back. Any other object he hangs from is a climbable
ceiling (see below). With E held, Duke catches a ledge even when he is not square to it (up to about
50 degrees more than before) and also reaches about an arm's length
higher than the original jump allows, easing up into the hang.

**Smoother busy scenes (Modernized, D08Y, awaiting playtest).** The Modernized
camera and widescreen show more of each level than the original camera, which
could make the emulated PlayStation run out of processing time and drop to 20
fps for a moment (felt as a freeze). Modernized now emulates a faster CPU (150%)
so those views stay at 30 fps; game speed, sound and movies are unchanged. It is
only used while you play in a level (loading, menus and movies run at the
original speed) and pauses itself if your PC cannot keep up.
`run.py --cpu-overclock 100` restores the original speed (125, 175 and 200 are
also available); Vanilla always uses the original speed.

**Running jumps at gaps (Modernized, D08Y, awaiting playtest).** As in the
original game, a running jump pressed in the last few steps before a gap or pit
waits for the edge and leaps from the lip, so you no longer have to time Space
to the last moment. A quick tap is enough. Near furniture and small steps the
jump still fires immediately.

**Manual jump style (optional, Modernized, D08Z, accepted 2026-10-01).** The jump
above is the default (`assisted`): near a gap the game picks the takeoff point,
and once Duke is in the air the arc is fixed. The `manual` style hands the
jump to you:

- Duke leaves the ground the moment you press **Space**, also near gaps, so
  you time the takeoff yourself.
- Pressed a fraction of a second too late, just after running off an edge, the
  jump still happens (about 0.15 s of grace).
- In the air, **W/A/S/D** steer relative to the camera: turn the jump with the
  mouse and W, brake or back out with S. Steering is capped at takeoff speed;
  with no key held he keeps his momentum. This also works while
  reaching for a ledge with **E**.
- When **Shift + W** is blocked against a dry step, keep it held and press
  **Space**. Once the rising jump clears the lip, forward steering can build
  toward normal running-jump speed instead of retaining the slower standing
  jump cap. Clearance and collision still apply; jump height is unchanged.
  A fresh Space tap also starts the jump during the brief grounded stopping
  animation; you do not need to wait for idle or press twice.
  D08Z6, including first-press response, was accepted on 2026-10-24.
- Standing and walking jumps leave the ground about three times sooner (a short
  crouch instead of a quarter-second wind-up).

The gap help described below (low-lip scramble, E climb) still applies. Gravity,
jump height and running speed are the original ones. Select it once; it is
remembered:

```
python3 recomp/tools/local/run.py --jump manual --show-settings
python3 recomp/tools/local/run.py --jump assisted --show-settings
```

(`--settings`, choice J, does the same.) Vanilla always uses the original jump.

**Jumping into walls (Modernized, D08Z1, accepted 2026-10-08).** In the original
game a jump that touches a wall bounces Duke back with a stagger and the jump is
lost. In Modernized (`slide`, the default) a wall only stops the part of his
motion that goes into it, as in EDuke32: jump straight at a wall and he stops
against it and drops down its face; jump at it at an angle and he slides along
it, keeping his height, arc and the rest of his speed. Bumping your head on a
ceiling stops the rise but keeps your forward speed. There is no stagger,
rumble or impact sound. Ledge grabs with **E** and the low-lip scramble still
catch a wall with a ledge in reach. Both jump styles use it. To get the
original bounce back:

```
python3 recomp/tools/local/run.py --jump-walls original --show-settings
python3 recomp/tools/local/run.py --jump-walls slide --show-settings
```

(`--settings`, choice K, does the same.) Vanilla always keeps the original
bounce.

**Wall backflip (Modernized, D08Z5).** In the original game a jump that lands
on a face too steep to stand on (the sloped body of a car, a steep slope) makes
Duke slip back in a backflip-like pose and then throws him several feet
backwards as he drops off it. In Modernized (`modern`, the default) he still
slips and slides down the steep face, but drops straight down where it ends,
so he stays next to the car or slope instead of being thrown away from it.
Pressing Jump while he slips makes the original do a backflip off the face.
In Modernized Duke jumps up the face instead: he rises and drifts uphill,
lands on it a little higher and slips again, so with well-timed jumps you can
work your way up (as in Duke Nukem 3D), and sometimes onto the car.
First and third person behave the same. Wall contact in the air is the
separate JUMP WALLS setting above. In game: OPTIONS > ADVANCED > MOVEMENT >
WALL BACKFLIP. To get the original back:

```
python3 recomp/tools/local/run.py --wall-backflip original --show-settings
python3 recomp/tools/local/run.py --wall-backflip modern --show-settings
```

(`--settings`, choice P, does the same.) Vanilla always keeps the original.

**Landing (Modernized, D08Z3, accepted 2026-10-08).** In the original game some
landings stand Duke still for about 0.6 s whatever you press: a jump straight
up (Space alone, even if you steer it in the air) and a jump whose direction
key you let go before touching down. In
Modernized (`modern`, the default) Duke still settles into his landing pose
when you press nothing, but hold a direction as he lands (or press one while
he settles) and he walks or runs off at once; press Space as he lands and he
jumps again. With CROUCH MODERN, hold Ctrl in the air to enter the kneel or
crouch walk directly on a soft landing. Jumps that already landed straight into a run (a direction held
through a directional or running jump) are unchanged, and so are fall damage
and landings in water. To get the original landing back:

```
python3 recomp/tools/local/run.py --landing original --show-settings
python3 recomp/tools/local/run.py --landing modern --show-settings
```

(`--settings`, choice L, does the same.) Vanilla always keeps the original
landing.

**Jumping a gap that is slightly too long (Modernized, D08Y, awaiting playtest).**
When a running or directional jump reaches the far edge with Duke's feet just
below its top, he no longer bounces off and falls. If the lip is low (up to
about a foot and a half), he scrambles up onto it with a small mantle, with or without
**E** and even with a weapon out. With **E** held, he also climbs up from
lower, as long as the edge is within climbing reach (up to about hip height
above his feet), where the original game had only the bounce; a higher edge is caught
and hung from as before. Taller walls still bounce. Vanilla is unchanged.

**Climbing down a ladder from the top (Modernized, D08U).**
Stand on the platform at the top of a ladder, near the edge where it hangs, and
press **E**. The screen shows `E TO CLIMB DOWN` when you reach one (at most
once every few seconds). Duke stows his weapon if needed, turns to face the ladder and lowers
himself onto it, then the game's own ladder climbing takes over. Hold **S** to
climb down: Duke climbs all the way to the bottom and steps off onto the floor
(the original game needs Down plus the action button for that last step, which
S now includes). Some ladders end above the floor; on those Duke stops on the last rung and
S lets go, so he drops cleanly to the floor (hold S to go straight down, or
tap S again on the last rung). **W** from the last rung climbs back up. His weapon comes back out once he is standing again, as with
other E climbs. Walking or running off the edge without E still drops or jumps
as before, and E away from a ladder top does its normal job. Vanilla has no top
mount, so there the only way down is to jump.

**Climbing down a wall from the top (Modernized, D35I1).**
At a reachable climbing-wall edge, `E TO CLIMB DOWN` offers a way back down.
Press **E** to stow your weapon and lower onto the wall, then hold **S** to
descend. **W/S** climb up/down, **A/D** move sideways, and **Space** releases.
Your sideways position is retained when mounting. The camera remains free;
after climbing back onto the platform, movement follows the view again.

**Grabbing a ladder that hangs high on a wall (Modernized, D08J1).**
Some ladders start well above the floor, out of reach from the ground (about
Duke's height up).
Hold **E** and run or walk at the ladder: when Duke is close enough he jumps
up by himself and grabs it, then the normal ladder climbing takes over (W
climbs). Standing under it, a press of **E** does the same. You do not need
Space, and Duke stows his weapon first if he is holding one. He lines himself
up with the ladder in the air if you were a little to one side. Without E
nothing changes: Duke stops at the wall as before. Vanilla is unchanged; there
you jump with E held.

**Climbing off a ladder with an enemy at the top (Modernized, D08U2).**
In the original game an enemy (or other character) standing near the top of a
ladder keeps Duke on the top rung for as long as it stays there. In
Modernized, keep holding **W** at the top and Duke climbs off onto the platform
as usual. He stops short of the enemy rather than shoving it aside. Walls,
crates and other solid objects at the top still stop him. Vanilla keeps the
original behavior.

**Poles and chains (Modernized, D08J2).**
While Duke hangs on a pole or chain, **A** takes him round it to the left and
**D** to the right, as seen on screen. **W/S** climb up and down as before. The
original game's left/right buttons work the other way round, and Vanilla keeps
that.

**Climbing down chains and poles (Modernized, D08J5).**
Stand at the edge of a platform where a chain or pole hangs just below and
press **E** (the screen shows `E TO CLIMB DOWN`, as for ladders). Duke stows
his weapon if needed, grabs it, swings round to its far side and hangs there
facing the platform. Hold **S** to climb all the way down: where there is a
floor he steps off onto it; at an end with nothing below, S lets go and he
drops. **W** climbs back up (at the top he climbs off onto the platform).
**Ctrl** or **Space** lets go at any height. Near the top, S no longer pulls
Duke back up onto the platform beside the chain, as the original game does.
Vanilla is unchanged (no top grab; the only way on is a jump).

**Shooting while climbing (Modernized, D08J6).**
On a ladder, pole, chain or climbing wall, hold **left click**: Duke stops
climbing, draws his sidearm (the Desert Eagle slot) and fires where the
crosshair points, so you can turn the camera and shoot behind or below him.
A short click only draws. Like Uncharted and Tomb Raider, only the one-handed
sidearm comes out on a climb: if another weapon is selected, the sidearm
takes its place and your weapon comes back when Duke holsters. If he
reaches the ground with the sidearm still out (after letting go armed, or an
armed pull-up from a ledge), he keeps it until you choose a weapon (number
keys or the wheel). **SIDEARM RETURN: AUTO** instead brings your weapon back
by itself once you have been off the trigger for about 0.75 seconds (see
Sidearm return below). Press **W/S** (or **A/D**) to
climb on: Duke holsters by himself and keeps climbing. **Space** lets go with
the weapon out. With no sidearm ammo, left click does nothing on a climb.
Number keys do not change weapons on a climb. In third person Duke keeps one
hand on the rung, chain or wall hold while the sidearm is out: his chest and
head turn toward where you aim (up to about 50 degrees for the chest), and
while firing the gun arm points at the crosshair. On a ladder or climbing
wall (and in a ledge hang), aim far round to his right or behind him and he
swings out on the hand that holds on, opening his body like a door so the
pistol reaches round (D08J6F); aim to his left and he fires across his body.
He can shoot in every direction. On a pole or chain, with the
sidearm out, Duke also goes round it as you turn the camera (D08J6E), keeping
the chain a little to his left so it never blocks the shot; a wall or ledge
beside the chain stops him. In first person the pistol always points at the
crosshair. Vanilla is unchanged.

**Shooting from a ledge hang (Modernized, D08J7).**
Hanging from a ledge, hold **left click**: Duke keeps one hand on the lip,
draws his sidearm and fires where the crosshair points, so you can shoot
enemies along the wall beside you. A short click only draws, and the same
sidearm swap as on climbs applies. In third person, while the sidearm is out
in the hang, his chest and head turn toward where you aim and the firing arm
points at the crosshair; his other hand stays on the lip. As on a ladder he
swings out on that hand for aims behind him, so he can shoot in every
direction. **W** pulls Duke up one-handed with the
sidearm still out, firing or not: no holstering, and the same speed as an
unarmed pull-up. He hauls himself up on his left hand, leaning onto it, with
the pistol held up beside his head until he is over the lip. In first person you
see it too: his left hand grips the lip and pushes him up, and the pistol
rises beside the view. Keep left
click held and he shoots the whole way up: the pistol comes down from beside
his head and fires at the crosshair, then goes back up when you let go. A
shot aimed into the ledge in front of him hits the ledge; aim over the lip
or along the wall. He lands ready to shoot. **A/D** (shimmy) holster by
themselves and then move. **S** or **Space** lets go with the pistol still
out, so Duke lands ready to shoot. After either he keeps the sidearm until
you choose a weapon (**SIDEARM RETURN: AUTO** brings yours back once you have
been off the trigger for about 0.75 seconds; see above).
The pull-up itself is quicker than the original (about 1 second instead of
1.7). Vanilla is unchanged.

**Shooting while climbing over (Modernized, D08J8).**
Whenever Duke climbs over something, hold **left click** and he draws his
sidearm at once and shoots the whole way, one hand on the climb: mantling
onto a ledge or a crate (with a running jump too), pulling himself up,
climbing out of the water, and stepping off the top of a ladder, chain or
climbing wall. The climb never stops for it and he lands ready to shoot. In third person his chest and
pistol turn toward where you aim, within the same limits as on climbs. The
same sidearm swap and **SIDEARM RETURN** rule as on climbs apply. If an armed
climb up a wall ends in a ledge hang, keep **W** or **E** held and he pulls
himself up one-handed, still shooting. With no sidearm ammo, left click
during the climb brings your own weapon up the moment he lands instead.
You can also start the climb while shooting (D08Z8). With the Desert Eagle
out, walk or run into the ledge and press **E**, or run-jump at it with
**E**, or jump at a ledge to grab it: Duke climbs with the pistol in his
hand and, with left click held, keeps shooting, up the face or from the hang,
through the pull-up and onto the top. With another weapon out, walking into
a ledge with **E**, Duke stops firing just long enough to put it away and
climb, then the sidearm shoots on through the climb; jumping at a ledge he
grabs it with the sidearm drawn in place of your weapon, as on every climb
(**SIDEARM RETURN** decides when your weapon comes back). A left click pressed after **E** still cancels the E, as
before, and **E** standing still or at a ladder puts the weapon away as usual. Held fire on ladders, chains, climbing walls, monkey bars and from a ledge
hang now repeats as fast as on the ground. Vanilla is unchanged.

**Doors open with E, and never stop the shooting (Modernized, D08J8, D08J9).**
Press **E** at a door with a weapon out: Duke kicks it open as usual, with
the gun still up (no holstering), and keeps shooting the whole time, kick
included. Every weapon fires exactly as it does on open ground: same rate,
the shotgun pumps between shots. E works whether or not
you are already holding left click, and holding the mouse down through the
whole kick keeps the weapon firing. Left click never opens doors: at a door
it just shoots. Holstered, E opens doors as before. Vanilla is unchanged.
Point-blank rockets and Freezer shots at a closed door still hurt or freeze
Duke, as in the original.

**Free camera while climbing (Modernized, D08J3).**
On ladders, poles, chains and climbing walls the mouse camera stays yours: turn
it all the way round Duke, look up the ladder or down to the floor, zoom with
Alt+wheel, and press **V** to swing it back behind him. Climbing works as before
(W/S up and down). On poles and chains **A** and **D** always take Duke left and
right as seen on screen, whichever side you view him from. While he climbs the
view is centred behind him even if you play over the shoulder (H); your
shoulder side comes back when he gets off. Turned toward the wall, ledge or
bars he is holding, the camera stops in front of them and comes in close (Duke
fades out) rather than passing into the wall (D08J3A). With global First Person selected,
**LADDER VIEW** in OPTIONS > ADVANCED > ACTION PERSPECTIVE chooses FIRST PERSON
(default) or THIRD PERSON for plain ladders. CONFIRM applies and saves it.
First Person keeps a steady body-height eye through mounting, climbing, hanging
at the bottom and exiting. Mouse look does not turn Duke on the ladder; his
hands keep their native climbing poses. Global Third Person always wins.
`--ladder-view first` or `--ladder-view legacy-third` also saves this choice.
**CHAIN/POLE VIEW** in the same menu offers FIRST PERSON (default) / THIRD
PERSON for chains and pole-class climbable objects. The eye stays steady above
Duke as he climbs or circles the column; his hands keep their climbing poses.
Mouse look stays independent, including the top mount's half-turn. Held A/D
keeps its direction when you look across to the other side; release and press
again to choose the direction relative to the new view. CONFIRM applies and
saves the choice; `--pole-view first` or `--pole-view legacy-third` also saves it.
**MANTLE VIEW** offers FIRST PERSON (default) / THIRD PERSON for ledge hangs,
shimmying, pull-ups and water climb-outs. The eye follows Duke over the lip and
his hands keep their native poses. In first person a hang holds the eye steady
just behind the lip (both hands in view looking ahead); his legs hang below,
only a sliver of them at the bottom of the view when you look straight down.
E catches/climbs, W pulls up, A/D shimmy and
Space or S drops from a settled ledge hang as before. CONFIRM applies and saves
it; `--mantle-view first` or `--mantle-view legacy-third` also saves the choice.
**WALL CLIMB VIEW** offers FIRST PERSON (default) / THIRD PERSON for climbing
walls, including catches, mounts and exits. CONFIRM applies and saves it;
`--wall-view first` or `--wall-view legacy-third` also saves the choice.
With Modern controls, **W/S** climb up/down and **A/D** move left/right along
the wall. Hold two directions to climb diagonally, at the same overall speed.
Releasing stops promptly; changing direction does not wait for a full stroke.
Mouse look is independent of the climbing direction. **Space** lets go.
The wall's edges, ceiling limits, panel transfers and exits still constrain
travel. At slot 10's upper limit, move left along the wall to reach its exit.
The movement works with either view choice. Vanilla keeps its original
climbing movement and camera.

**Climbable ceilings / monkey bars (Modernized, D08J4).**
A jump with **E** held under a climbable ceiling (a grate or bars) makes Duke
grab it and hang by his hands. Then **WASD** move him relative to the camera,
like walking: W goes where the camera looks, A and D go left and right on
screen, S comes back toward the camera, and the mouse camera stays free. Duke
turns to face the way you steer and goes hand over hand. At the end of the
ceiling he stops and keeps hanging. Press **Ctrl** or **Space** to let go.
Hold **left click** while hanging there (Modernized, D08J6D): Duke stops, keeps
one hand on the bars, draws his sidearm and fires where the crosshair points.
If he is moving hand over hand, he stops at the next grip first. The
sidearm swap, the short click (draw only) and SIDEARM RETURN work as on
climbs. **WASD** holster by themselves and then move on; **Ctrl** or
**Space** let go with the pistol still out, so Duke lands ready to shoot. In
third person his chest, head and firing arm turn toward the aim as on climbs,
and with the sidearm out Duke turns on the bars to face wherever the camera
looks (Modernized, D08J6E), so you can shoot all the way round. The
first-person view shows the normal sidearm for now. Vanilla is unchanged.
With global First Person selected, **OVERHEAD VIEW** in OPTIONS > ADVANCED >
ACTION PERSPECTIVE chooses FIRST PERSON (default) or THIRD PERSON. CONFIRM
applies and saves it. This is separate from MANTLE VIEW and changes only the
view; the existing eye and native hands are retained. The launcher also accepts
`--overhead-view first` or `--overhead-view legacy-third` (saved preferences).
The original game steers like a tank here (Up/Down along Duke's facing,
Left/Right only turn him), and Vanilla keeps that.

**Use interactions and idle gestures (Modernized).** With global VIEW set to
FIRST PERSON, idle gestures and holding the holster button for the original
inventory keep the eye view. Inventory still suspends movement until release.

Supported door/key actions (including the hinged-door right-leg kick), crystal
pickup/return, toilet, statue, wheel, lever, NPC and mirror interactions follow
the global VIEW selection. Duke keeps the action's normal timing and alignment,
uses the item, and redraws his weapon when it finishes. In first person, look
down toward the interaction to see his animated hands.

In Modernized, Duke's reflection stays opaque when close to a mirror.
In Modernized, E also works at a mirror behind a counter, dresser or other
furniture: face it from up to about twice the usual reach and Duke admires
himself and says his mirror line, as at any other mirror.
First-person grooming hands and forearms clip continuously at the camera while
keeping the original animation. Intentional power-up effects still apply.

Number keys now select familiar weapon groups when owned and usable:

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

Upgrades stay on the base weapon's key. **Wheel up** selects the next usable
weapon; **wheel down** selects the previous one. **Semicolon/apostrophe** still
selects previous/next. These work while running with a two-handed
weapon (Shift or RUN MODE), and in Modernized also in mid air (jumps and falls), where the change plays
twice as fast so the new weapon can fire before you land. **X** restores the last successfully equipped weapon.
In Modernized a weapon key, the wheel or **X** also works while you hold fire,
in a draw, a reload or a landing: the choice waits (up to about 1.5 s) instead of
being lost. With fire held, the shot already on its way finishes, the old weapon
is put away (the Gatling's spin stops as when you let go) and the new one is
drawn; if fire is still held when it is ready, it starts firing (as in Duke 3D).
Pressing the key of the weapon already out does nothing and the burst goes on.
E or a holster key cancels a waiting choice. Vanilla is unchanged. In Modernized a drawn dynamite stays unlit until you fire, so you can
switch away from it (see Dynamite below); once lit, or with `--dynamite original`,
it must finish its original throw/fuse before a requested weapon switch completes. A usable pipe-bomb remote remains accessible without spare ammo.
**M** uses an owned portable medkit (in Modernized also while swimming, flying the jetpack or firing), **J** toggles the jetpack, **N** night vision,
and **B** the Bio Mask (TTK's gas mask). Modern night vision and enabled Auto Bio
Mask toggle instantly while firing; original gadget modes retain their equip
and weapon-redraw animations. **J** also switches the jetpack
off in mid-flight (a controlled fall follows). Underwater air still equips automatically.
**[ / ]** cycle eligible gadgets; **Enter** uses the selected gadget. In Modernized, with **EQUIP PICKUPS** on **IF NEW** (the default, see below), a gadget you pick up (jetpack, Bio Mask, goggles, medkit, steroids), or top up when it was not full, becomes the selected one at once, as in Duke 3D, so its HUD box shows and **Enter** use it; keys and mission items leave the selection alone. **, / .** browse the level's mission items (see Mission items below). **R takes steroids** (see Portable steroids below). In third person, **Q** is the quick kick with any weapon, standing, walking or
running or jumping (see Third-person quick kick below; **KICK: ORIGINAL** restores the
original kick, only while standing still with Mighty Boot already selected on 1).
Q never selects another weapon or presses fire. In first person Q is a quick
kick with any weapon (see below).

**Alt + wheel up** brings the camera closer; **Alt + wheel down** moves it farther.
Distance is clamped and smoothed, retains wall collision, and remembers your
preference despite wall compression. It is now **saved in the Modernized profile**
and restored on the next launch. It does not change FOV or switch weapons.
You can also use **Options > Display > Camera Distance** (after First-person FOV). This third-person
slider offers **Original** (the native follow distance) or 768-6144 game units,
with 128-unit adjustments. Existing custom distances are retained. Left/right
or the mouse wheel adjusts the selected slider; Confirm applies and saves it,
and Cancel discards edits. Display Defaults restores Original.

**V** recenters the camera: it swings smoothly back to the original follow angle
behind Duke. Moving the mouse cancels the swing. On foot Duke already turns with
the view, so V mostly levels the camera's up/down angle.
**H** cycles the camera shoulder: centered, right shoulder, left shoulder. Shots
still go to the crosshair; walls still pull the camera in. The side you choose
is saved too. Both keys are rebindable (`camera_recenter`, `camera_shoulder`).

Third-person view uses the game's native obstruction handling, including
translucent foreground objects that keep Duke visible during combat and his
native close-camera fade. D10B's additional camera compression, automatic
shoulder centering and patterned body fade are disabled by default.

**Look keys (Duke 3D style).** Hold **Page Up** / **Page Down** to look up or
down; let go and the view eases back to the normal angle. Hold **Home** /
**End** to look up or down and keep the view there (it stays while you walk,
and the mouse still adjusts it); tap Page Up or Page Down to bring it back.
They work in first and third person and are rebindable in KEY BINDINGS
(CAMERA: LOOK UP, LOOK DOWN, LOCK LOOK UP, LOCK LOOK DOWN) or with `--bind`
(`look_up`, `look_down`, `lock_look_up`, `lock_look_down`). Vanilla has none of them.

**Keyboard turning (Duke 3D style).** Hold the **Left** / **Right** arrow to
turn the view left or right, and Duke with it, as the mouse does. The turn
starts slowly for an eighth of a second, then turns at Duke 3D's speed (about
80 degrees a second walking, twice that running). W/A/S/D and the mouse work
as before and add to it. It works everywhere in Modernized play: on foot, in
first and third person, swimming and flying the jetpack. When the original
camera takes over (turrets, scripted views) the arrows turn Duke the original
way. In gameplay the arrows no longer reach the original D-pad, so the jetpack
no longer strafes on Left / Right (A / D still do); menus use the arrows as
before. Rebind them in KEY BINDINGS (CAMERA: TURN LEFT, TURN RIGHT) or with
`--bind` (`turn_left`, `turn_right`). Vanilla is unchanged.
Save a distance or side without launching:

```sh
python3 recomp/tools/local/run.py --camera-distance original --shoulder center --show-settings
python3 recomp/tools/local/run.py --camera-distance 3000 --shoulder right --show-settings
```

`--camera-distance` takes `original` or 768-6144 game units; `--shoulder` takes
`center`, `right` or `left`. Both update your saved Modernized preferences.

**P** toggles **first person** (prototype, Modernized with the independent
camera). The view moves to Duke's eyes with a wider field of view; the mouse
looks, WASD moves relative to the view, and shots go to the crosshair (in the
air too, when jumping or falling). Crouch
lowers the view, and jumps keep it. Duke's head is hidden; his legs can be seen
when looking down during ordinary movement, with polygon clipping at the eye. With a weapon
drawn you see Duke's own gloved hand and connected arm holding it at the lower
right, always in front of walls, with the original muzzle flash and a small kick
when firing. Two-handed weapons also show a connected support hand and arm;
held mission items and the remote use a connected right arm. The Gatling support
hand rests against its underside at idle, including the Laser Gatling. The Buffalo Rifle
lifts with its hands for the cocking stroke, then returns to its ready grip.
Its left support hand keeps a stable contact while turning, and its right
wrist retains the ready alignment while running.
The shotgun, Gatling, Energy Weapon, Flamethrower and Freezethrower use restrained
first-person arm motion while running, including their upgrades. The Gatling and
Freezethrower retain their walking support grip while running; the Flamethrower
and Energy Weapon keep their firing support grips at rest and in motion.
With empty hands (Boot), visible hands stay connected to their native swinging
arms while walking and running, including when looking down. The unarmed
swing sits low in the view. Empty-handed jetpack flight also keeps visible hands
connected to their native arms, including when flying backwards.
Each weapon is framed to point at the crosshair; the HUD stays on top.
Upgraded weapons use the same first-person framing as their base versions,
including the Incendiary RPG in Challenge Stage 4. Holstering or drawing makes it disappear or
appear at once. **Q** in first person is a quick kick, like Duke Nukem 3D's:
Duke's leg snaps out from the lower left and kicks whatever the crosshair is
on within leg reach (look down at a low prop such as a garbage bag to kick
it), with any weapon still in hand, while standing, moving or jumping. With
the Mighty Boot selected (key 1), holding the left mouse button kicks too,
also while running. E never kicks in first person. Tap Q repeatedly or hold
it to chain kicks while running with any weapon. Kicks do not knock enemies
back the way the original full kick sometimes does. In Modernized, a kick that
connects (first or third person) lands with a solid thud (Duke's wall-bump
thud, pitched up so it carries): once per kick, on an enemy, a breakable object or
a wall. A kick that hits nothing stays silent. Q displays Duke's plain left leg,
without a holster. Its entry and recovery ease below the screen edge. Like the
weapon view, Q keeps the same screen size and framing at every world FOV. The
separate hinged-door kick uses his right leg with the same fixed display FOV,
including its thigh attachment. Kick reach, damage and timing
are unchanged.

When Duke is frozen in first person, the view stays at his eyes with a blue
tint until he thaws. Movement, aiming and attacks remain locked while frozen.
Switching explicitly to third person still works.

First-person view stays active while swimming at the surface, diving and
swimming underwater. The swimming eye stays steady through the stroke, with
no walking bob; mouse look, movement and firing use the existing swim controls.
Space ascends, Ctrl dives, and E remains the way to climb out. MANTLE VIEW chooses first or third person for the climb-out.
First person also stays active during jetpack takeoff, flight and landing
with Modern, Classic and EDuke32 flight controls. The eye stays centered above
Duke through flight lean; aiming and the crosshair work as on the ground.
With empty hands his arms keep the same screen size and framing at every
world FOV, like the weapon view and his legs.
In Modernized first person Duke's whole body turns with the mouse at the
display rate, so his arms and legs stay with the view on fast turns.
Plain ladders retain first person by default, with LADDER VIEW offering the
existing third-person camera. Chains and poles also retain first person by
default, with CHAIN/POLE VIEW offering third person. Ledges retain first person
by default, with MANTLE VIEW offering third person. Climbing walls still show
the free third-person camera; scripted and
turret cameras use the original camera. A death in first person keeps first
person with the view on the floor; a death in third person uses the original
death camera. Dodge rolls
and slides down steep slopes stay in first person, and so do falls (off an edge, a
car roof or after bumping a wall). Pressing P
again returns to third person. Console, pause, menus and focus loss retain the
selected perspective and stop gameplay input. Returning to play captures input
automatically without resetting the camera direction. Loading a save keeps your
current view and control choices; the loaded world supplies the viewing direction.
In Modernized, gameplay saves resume immediately after loading, including saves
made while paused and loads through F7. No extra Escape press is needed.
Loading restores inventory silently and waits for fresh gameplay instead of
briefly showing menu text stored in the save.

Explicit LEGACY controls or ORIGINAL camera use the original camera; your
first/third-person choice remains saved for when you return to MODERN with the
INDEPENDENT camera. Unsupported traversal temporarily uses third person without
changing that choice. The choice is saved; to set it without launching:

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
Walk is the default on each launch. Hold Left Shift to run (silently: no click on press or release); **Caps Lock** toggles
autorun (centered `RUN MODE ON` / `RUN MODE OFF`), and Shift then temporarily walks.
Autorun survives capture/focus/pause
changes within that session, but resets off on relaunch; it does not follow the
keyboard Caps Lock light. A Shift (or any bound gameplay key) that is already
held when the mouse is recaptured - after Escape/resume, F7 load or inventory - is read from the keyboard state, so running continues without re-pressing. Your existing speed-modifier binding is preserved
(the binding editor calls it `walk` for compatibility). These speed controls apply
to supported Modernized locomotion, including wading and swimming (D08AA: the same walk and
run speeds as on land); Legacy is available only through the explicit control scheme setting. Diagonals are
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
**Space** uses ordinary land jumps with SWIMMING MODERN (D08AC), including
normal running jumps onto platforms. SWIMMING ORIGINAL retains the older
wade jump and special ledge assistance.
**Deep-water entry (D08AE, accepted):** jumping or running into deep
water now hands control to the original water-entry response near the surface,
while shallow-water jumps retain their land behavior. Swimming controls and
speeds are unchanged.
On the surface of deep water, face the camera: **W/S** swim forward/back,
**A/D** strafe (camera-relative), **Ctrl** dives for as long as you hold it
(let go and Duke stops, unless W/A/S/D is held - D08O4). Underwater, **W/A/S/D**
swim where the camera looks, **Space** swims up, **Ctrl** swims down, and
Duke surfaces automatically when he reaches the top.
**Walk and run in water (D08AA):** Duke walks by default and runs with
**Shift** (or Caps Lock autorun) everywhere: in shallow and waist-deep water,
on the surface and underwater, at the same two speeds as on dry ground and in
every direction (sideways and backward strokes included). Underwater, Space
and Ctrl keep their own up/down speed. Like on land, walking stops at the edge
of a drop such as a pool rim; run or jump to go over it. **SWIMMING ORIGINAL**
(D08AB, below) brings back the original water speeds and wade jumps. Leave deep water with
**E** / mantle — Space never hops you out at a ledge. E puts the weapon away
to climb; if there is no ledge to climb, Duke draws it again by himself
(D08O3). Water/oxygen stay with the original game.
Details: [documentation/55-swim-controls-research.md](documentation/55-swim-controls-research.md).
Underwater air remains automatic; there is no scuba item and none will be added.
**Shooting while swimming (D08O1, Modernized):** hold **left mouse** while
you swim. On the surface and underwater Duke keeps swimming (W/A/S/D, Space,
Ctrl) while he fires, and shots go to the crosshair like on land. The original
game stopped Duke underwater whenever fire was held; Vanilla still does.
While he swims and fires underwater, Duke raises his arms and weapon toward
the crosshair (D08O2A) while his body, head and legs keep the swim stroke.
Number keys, **'** / **;** and the mouse wheel switch weapons in the water too,
but only between the weapons the original allows there (Desert Eagle,
Combat Shotgun, Gatling Gun / Laser Gatling, Buffalo Rifle, Crossbow and Pipe
Bomb); the others (Boot, knife, axe, RPG, Energy Weapon, Flamethrower,
Freezer, Dynamite, Holy Hand Grenade) are skipped. With the weapon holstered,
**left-click draws it on the surface and underwater, as on land; keep holding
to fire** (D08O5). A quick click only draws. If the holstered weapon is one
the water does not allow, the game's own water substitute is drawn (for
example the Crossbow); with none available, fire does nothing. Swimming
strokes keep going while the weapon comes out. Vanilla keeps the original
(holstered fire does nothing in water). First person stays active in the water (D11E).

**Modernized jetpack**: **J** immediately starts flight. From the ground it
gives a short automatic lift, then hovers. Crouching (**C** or a held
**Ctrl**), Duke stands up into the same lift and hover: the **C** crouch ends,
and a **Ctrl** still held from the crouch does not descend until you release it
and press it again. Where there is no room to stand, **J** does nothing and Duke
stays crouched. During a jump or fall it arrests
vertical movement and holds the activation height, without the ground boost.
**Space** climbs further. A quick J-off/J-on can restart flight while the pack is still
closing. In the air the **mouse** turns Duke and the
camera together (so do **Left** / **Right**), **W/A/S/D** (or **Up** / **Down**) fly relative to the camera at the
walking or running pace with a short start and stop. **Shift** reverses the
current walk/run choice, as on land (including **Caps Lock** run mode). Active
steroids boost horizontal flight by the same proportion as land movement.
Releasing every key **hovers** perfectly still, **Ctrl** descends quickly (the same speed as diving
underwater) to a low hover at ground level. The sound and flames stay on;
**Space** climbs again without restarting the pack, and
**J** switches the pack off for a controlled fall with the camera still live.
Duke's height stays where **Space** or **Ctrl** left it: flying, turning,
looking up or down and hovering do not change it (D08Q6; there is no hover
bob in Modern). Descent checks the surface
beneath Duke: leaving a platform lets **Ctrl** continue down to lower ground,
and releasing it holds the new height. Flight adjusts vertically along rising
floors, descending ceilings and small steps where Duke fits. Modern/EDuke32
follow a contacted slope during ordinary WASD flight; holding **Space** or
**Ctrl** is not required. Following ends when the surface ends. Holding **Space**
at a ceiling or the outdoor height limit settles against the boundary instead
of repeatedly bumping it. Walls and gaps with insufficient clearance still
block travel. Classic shares the clearance handling while retaining its
original thrust and landing behavior.
**Water** (Modern and EDuke32, D08Q10): **J** also starts the jetpack while
swimming on the surface or underwater. On the surface he lifts off exactly
like a ground start, to the same height above the water. Underwater, as in
EDuke32, he stays exactly where he is and then flies freely: **Space** up (and
out), **Ctrl** down, **W/A/S/D** around; he breathes bubbles as when swimming,
and the pack trails more as it thrusts. He is still holding his breath: the
air meter keeps running down (and drowning hurts) until his head is out of the
water. Breaking the surface plays the same splash and gasp as surfacing while
swimming. While the pack is on, the water surface is a floor from above, so he
cannot fly back in: **Ctrl** stops just above it and Duke hovers there, and
flying over the water never sinks him back in. **J** off over water drops him
in to swim as before; **J** off or running out of fuel under water goes
straight back to swimming where he is, without a splash. Classic keeps the
original rule: no jetpack in water.
Fuel drains while flying and while hovering (the original rule); when it runs
out Duke falls the same way. The ground controls return the moment he lands.
With view aiming enabled, aim with the mouse and fire the equipped weapon with
**LMB** during flight in either scheme. The enabled crosshair stays visible;
**I** still toggles it. Switch weapons in flight as on the ground: **1-0**,
the **wheel**, **semicolon/apostrophe** and **X** (D08Q5), with the normal
draw while Duke keeps flying; wait for an attack or draw to finish before the
next switch. Gadget keys other than **J** and the quick kick still wait for
the ground. Dynamite thrown close below Duke knocks him out of flight with its
blast (the original reaction). Flight retains your selected first- or third-person
view, including takeoff, switching the pack off and landing.
Details: [documentation/57-jetpack-controls.md](documentation/57-jetpack-controls.md).
In Modernized, a jetpack left unavailable by interrupted deployment and Continue
now recovers when normal captured ground play resumes, including affected saves.
This restores selection/J without refilling fuel or granting a missing pack.


**EDuke32 jetpack (optional, Modernized only, D08S candidate)**: **J** starts
flight immediately: a short automatic lift from the ground, or a hover at
the activation height during a jump/fall (no added boost). **J** again switches
it off immediately into a normal fall. **Space** climbs, **Ctrl** descends,
**W/A/S/D** fly relative to the view, and releasing input hovers. A full pack
provides 61 seconds of flight, including hover; pausing does not use fuel.
The existing percentage shows the remaining share of that duration. Fuel-out
also switches the pack off. God mode keeps unlimited fuel. Descending to ground
level keeps the sound and flames running until **J** switches the pack off.
Water works as for Modern above: **J** starts in or under water and Duke
holds his position until **Space**, and the surface is a floor while flying (D08Q10). Flight retains your selected first- or third-person view.

Select **EDUKE32** in **OPTIONS > ADVANCED**, **JETPACK** under the MOVEMENT
heading (next launch), or launch with the following command. This saves the preference:

```sh
python3 recomp/tools/local/run.py --jetpack eduke32
```

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
python3 recomp/tools/local/run.py --camera independent --mouse-sensitivity 0.12 --invert-y off --show-settings
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
crosshair; Duke still turns to face where the camera looks, so the original
shot and auto-aim go that way. Selecting original camera also retains original body/gun presentation.
With `view` aiming, right mouse is **Grab** (see pushing objects below), not
aim: view aiming already aims where you look. Select `original` weapon aiming or
original camera to get the original right-mouse precision aim back; then **Alt**
grabs. The precision-aim action (`original_aim`) is unbound by default and can be
bound to another input. Vanilla retains right-mouse precision aim.
**9 Aiming display / assistance** configures three independent settings:
`eduke32`, `hitscan` (the default), `off` or `original-lock` assistance,
crosshair on/off, and red dot on/off. `eduke32` and `hitscan` are EDuke32's auto aim described under
[Auto aim](#auto-aim-d10h3). `original-lock` uses an existing game target
within six degrees of view center, with separate camera and muzzle visibility
checks, and turns the original game's auto aim on for its target acquisition.
Unsupported weapons retain original aiming. The crosshair marks view center;
assistance can correct within its cone. The red dot remains the original
game's marker. Hiding either marker does not change assistance, autoaim target
acquisition or damage.

**Red dot off by default in Modernized (D07D):** Modernized shows only the
modern crosshair; the original red autoaim dot is hidden in third and first
person, while holding precision aim, and on every map and state where the game
draws it. Profiles saved before this change are switched to red dot off once
(with a backup and a notice); after that, your own choice is kept. Turn the dot
back on with `--red-dot on` or settings item 9. `original` weapon aiming hides
the modern crosshair, so with `original` aiming turn the red dot on if you want
a marker. Vanilla always shows the original red dot.

Vanilla always retains original aiming. Save without launching with:

```sh
python3 recomp/tools/local/run.py --weapon-aim original --show-settings
```

For assisted view aiming with the crosshair (the red dot is already off by default):

```sh
python3 recomp/tools/local/run.py --weapon-aim view --aim-assist original-lock --crosshair on --show-settings
```

For original weapon aiming with the original red dot as the marker:

```sh
python3 recomp/tools/local/run.py --weapon-aim original --red-dot on --show-settings
```

`--aim-assist original-lock` is **AUTO AIM ORIGINAL** in OPTIONS > ADVANCED >
COMBAT; it also turns the original game's auto aim on. The red dot can also
show the native aim point without a target lock. Modernized plays with **AUTO
AIM HITSCAN** by default (`--aim-assist hitscan`), so add
`--aim-assist original-lock` to the last command for the original auto aim.

These commands save preferences without launching. Old profiles are backed up
and migrated automatically, preserving custom bindings; use `--show-bindings`
to inspect them. See [the follow-up controls and aiming notes](documentation/24-d07-controls-and-aim-options.md).


The crosshair describes the intended view target, not a confirmed hit. Shots
start at the muzzle, so nearby cover can intercept them even when the camera
sees the target. The adapter checks for a muzzle protruding through cover.
Original pellet spread, projectile travel, gravity, bounce and damage rules
remain. No target leading or gravity compensation is added.

**Point-blank shots (Modernized, third person, D10G1).** The gun is lower
than the camera, so when Duke stands close to a wall the crosshair point is
well above the muzzle. Instead of climbing steeply toward that point, a round
fired at a wall or prop within about 500 units leaves close to your view
direction and strikes a little below the crosshair, blending in smoothly with
distance, and never more than 35 degrees off the view. To tune the height for
the session, open the backtick console and type `pointblank <lift> [max]`:
lift 0 strikes at gun height, 1 aims at the crosshair (default 0.35); max is
the largest angle off the view, 5 to 45 degrees (default 35).
`pointblank default` restores both. Shots at an enemy under the crosshair, assisted shots, throws,
first person, `original` aiming and Vanilla are unchanged.

**Shots follow the view on screen (Modernized, D10G3).** At FRAME RATE 120 or
higher the picture you see is newer than the game's own camera, so a shot now
leaves from the view you are looking at, turning or strafing, instead of the
camera one game update older. At 60 Hz nothing changes. To compare, open the
backtick console and type `shotview off` (session only); `shotview on` or
`shotview default` turns it back on.

**Shoot what you see (Modernized, D10G4).** At FRAME RATE 120 or higher the
picture shows enemies a moment (about 0.09 s) behind where they really are.
A shot now tests the crosshair against each enemy where the picture drew it,
and the round then flies at that same spot on the enemy where it stands now,
so a running enemy under the crosshair is hit instead of the wall behind it.
Walls and cover still stop rounds as before, and the weapon's own firing
delay is unchanged. At 60 Hz nothing changes. To compare, open the backtick
console and type `shootseen off` (session only); `shootseen on` or
`shootseen default` turns it back on.

**The energy beam leaves the gun (Modernized, D49).** At FRAME RATE 120 or
higher, in first person, the energy weapon's beam is drawn from the gun toward
the crosshair while you turn or move, instead of trailing where you pointed a
moment earlier. Only the picture changes: what the beam hits is the same.
The glow and sparks where it strikes a wall appear at the end of the beam
you see (D49A), and the puffs where your bullets hit a wall appear under the
crosshair, then settle where the rounds really struck. At 60 Hz, in third
person and in Vanilla nothing changes. To compare, open the backtick console
and type `beamfollow off` (session only); `beamfollow on` or `beamfollow
default` turns it back on.

**The energy beam ripples smoothly (Modernized, D49C).** At FRAME RATE 120 or
higher the beam's ripple moves on every frame shown, not only about 30 times
a second, in first and third person. Its green and white stripes still flicker
at the original rate. To compare, type `beamanim off` in the backtick console
(session only); `beamanim on` or `beamanim default` turns it back on.

**Laser Gatling bolts fly straight from the barrel (Modernized, D49F).** In
first person the Laser Gatling's bolts now start at the tip of the barrel you
see and fly one straight line to the crosshair, and the gun is turned in a
little so its barrel points where the bolts go. The regular Gatling Gun is
held in the same turned-in position. Before, a bolt left the barrel
along one line and then bent onto the line to the crosshair. Bolts still land
on the crosshair. When the barrel would be in a wall, below a floor you are
looking down at, or far off to the side of a very close target, the bolt
starts a little nearer your eye instead, so it still lands where you aim. The
regular Gatling Gun's rounds, third person and Vanilla are unchanged. To
compare, open the backtick console and type `boltlaunch off` (session only,
also restores the old pose of both Gatlings); `boltlaunch on` or `boltlaunch
default` turns it back on.

**Rounds that leave on an enemy land (Modernized, D10H2).** The crosshair now
asks the same question as the round itself: it is "on" an enemy only where
the enemy's body is, not the slightly wider outline the game uses for other
checks. A round aimed at an enemy also follows it through the enemy's own
step in the same game update, so it goes through the spot the crosshair met.
Speed, spread, damage and cover are the weapon's own. To compare, open the
backtick console and type `roundfollow off` (session only); `roundfollow on`
or `roundfollow default` turns it back on.

First-map checks now cover the pistol, shotgun, rifle, Gatling, RPG, freezer,
crossbow, knife/axe, pipe bomb, grenade, dynamite, flamethrower and energy weapon.
RPG flight and explosions follow pitched view aim. Throws keep their original
charge, speed, gravity and fuse. The **energy weapon** (key 7) locks onto the
enemy under the crosshair and damages it, within its original reach (about
6000 units); a beam that misses does no damage, as in the original without a
lock. The upgraded Energy Weapon also zaps up to two more enemies within 45
degrees of your view, as the original upgrade does (D22D). Optional
assistance still requires visibility. Upgraded
variants have private dispatch-fixture coverage. Natural campaign acquisition,
other eras and your aiming feel still need playtesting. The crosshair disappears
when holstered, released or outside supported states/maps.
See [weapon coverage and remaining checks](documentation/21-modern-weapon-aiming.md).

In Modernized, **arrows**, **X** (confirm/fire), **Z** (menu back/quick-turn),
**Scroll Lock** (draw/holster; shows centered `WEAPON LOWERED` / `WEAPON RAISED`),
**Enter** (use selected inventory gadget) and **Right Shift** (inventory) always
remain available. Menu Circle still accepts **C** when uncaptured. Use Right Shift to close inventory. These fixed keys keep menus
operable after rebinding. Vanilla keeps the different original keys below. Escape pauses.

#### Menu keys (Modernized)

The original menus show PlayStation buttons. In Modernized the keyboard works
the same way on every menu screen:

| Key | In the menus |
| --- | --- |
| **Arrow keys** or **mouse wheel** | Move the highlight (the wheel works on screens with a list) |
| **Enter** | Confirm (Cross). On PRESS START it opens the menu |
| **Escape** or **Backspace** | Back one level / cancel (Triangle) |
| **X** / **C** / **Z** / **V** | Cross / Circle / Triangle / Square, for the buttons the screen shows |

- **Title main menu:** ONE PLAYER, TWO PLAYER, LOAD GAME, OPTIONS and QUIT
  GAME, in the original's look over the attract demo. Arrows or the wheel
  choose (hold to repeat), Enter confirms. ONE PLAYER opens CHOOSE DIFFICULTY
  LEVEL (Enter starts the game, Escape returns). TWO PLAYER opens TWO PLAYER
  OPTIONS (Left/Right change level, time, kills and split), then CHOOSE
  CHARACTER: each controller chooses and confirms its own character. Once
  both are confirmed, either controller confirms START. Triangle cancels that
  controller's choice or goes back a page. Keyboard menu input belongs to
  player 1. LOAD GAME lists your
  savestates (below). QUIT GAME asks QUIT GAME? (NO highlighted) and
  closes the program. Left alone on the main menu, the game goes back to
  PRESS START after about five minutes, as the original does.
- **Pause menu (Escape in game):** a list under the level name: RESUME,
  MISSION BRIEFING (the original story and goals), OPTIONS, SAVE GAME,
  LOAD GAME, RESTART LEVEL, MAIN MENU and QUIT GAME. Arrows or the wheel choose, Enter
  confirms, Escape resumes. RESTART LEVEL, MAIN MENU and QUIT GAME ask
  YES / NO first (NO is highlighted). QUIT GAME closes the program, keeping
  your confirmed settings, the same as closing the window. Challenge stages
  have no RESTART LEVEL; SAVE GAME and LOAD GAME are there in one-player
  games, as LOAD is in the original.
- **SAVE GAME** (pause menu) is LOAD GAME's page with every slot open: an
  empty slot (NEW SAVE beside the list) saves at once; a used one asks
  OVERWRITE THIS SAVE? with NO highlighted. The page then says SAVED TO SLOT
  NN, or THE GAME COULD NOT BE SAVED (the slot keeps what it had). The
  picture is the game as it was when you paused, not the menu. Loading such
  a save puts you straight back into play through LOAD GAME, quickload or F7,
  including older saves without resume metadata.
- **Death:** when Duke dies the game stays on the death scene, as in Duke 3D:
  no menu opens. The line at the top reads PRESS E TO OPEN MENU (it names
  whatever key USE / MANTLE is bound to). In first person the view is Duke
  3D's: it drops with Duke to near the floor, levels out, tilts to a random
  angle and turns toward whoever killed him; none of Duke is shown. In third
  person the original death animation and camera play. Mouse look is off
  while dead. The music keeps playing. On the death scene:
  - **E (USE)** opens the death menu: CONTINUE, LOAD GAME, RESTART LEVEL,
    QUIT TO MENU and QUIT GAME (challenge stages: REPLAY LEVEL, GIVE UP and
    QUIT GAME). **Escape** on that menu closes it and returns to the death
    scene; Duke stays dead.
  - **Escape** opens the normal pause menu (RESUME returns to the death scene;
    SAVE GAME is not offered while Duke is dead).
  - The console (`` ` ``) opens and closes without changing anything.
  - Typing `dnkroz` (or `dncornholio`) brings Duke back up where he fell,
    with full health and GOD MODE: ON, as in Duke 3D. Other cheats say
    Cheat unavailable - Duke is dead.
  Continues are unlimited: there is no continues count and no game over for
  running out. CONTINUE (and REPLAY LEVEL) act at once, with no CONFIRM
  CONTINUE, and bring Duke back at the last checkpoint, as before. QUIT TO MENU asks QUIT TO MENU?
  and gives up through the game over screen to the title menu; QUIT GAME
  closes the program. The others ask YES / NO with NO highlighted, and
  Escape cancels a prompt. Vanilla (`run.py --vanilla`) keeps the original
  death screen and continues.
- **Level title:** when a level starts (a new level, the next level after the
  statistics screen, RESTART LEVEL or loading a save, but not CONTINUE after
  a death), its name shows in red just under the crosshair, as in Duke 3D, for about three seconds, fading out by itself. Play is never
  interrupted; pausing hides it and holds its time.
- **Level stats** (OPTIONS > DISPLAY > LEVEL STATS, on by default): three
  small lines in the bottom-left corner during play, as EDuke32 shows them:
  `T:` the level time (minutes:seconds.hundredths), `K:` enemies killed out
  of the level's total, `S:` secrets found out of the level's secrets. The
  letters are red and the numbers a light gold; a count you have completed
  (all enemies, all secrets, or a level with none) turns red. They sit just
  above the health box, follow 4:3 and widescreen and the HUD size, and drop
  to the bottom edge when STATUS BARS is HIDDEN. They match the statistics
  screen: the time is the level's own timer (it stops while Duke is dead,
  and loading a savestate rewinds it), and the kill total grows when the game
  sends reinforcements (killing a dancer or other innocent calls four, and
  some levels send them by script), as EDuke32's does. Pausing, menus and the
  statistics screen hide them. In Modernized the air box (under water or
  while Duke gets his breath back) is drawn on the right, at the top of the
  stack of countdowns above the ammo box, so it never sits under the level
  stats. It appears in place, like the other boxes. In Modernized the ammo
  box shows a picture of the current weapon's own ammunition instead of the
  same cartridge belt for every weapon (the Gatling gun keeps its belt, which
  steps one round per shot; once upgraded to the Laser Gatling it shows a
  red laser energy cell instead). Some of them move: the crossbow's bolts leave the
  stack one by one (fewer than three left shows what is left, then an empty
  box), the Desert Eagle's magazine follows its 22-round clip and is swapped
  for a full one when Duke reloads, the energy weapon's atom tumbles and
  flares on each shot, a surge runs along the Laser Gatling's cell on each shot and the freezer charge flashes icy bright. Vanilla
  keeps the original belt. **FPS** on the same page shows or hides the
  frame rate statistics block at the top left: it is the same block as the
  console's `fps` command, and the two stay in step (typing `fps` changes the
  setting, which is saved). Both apply as soon as you confirm. The launcher
  accepts `--level-stats on|off` and `--fps on|off` (saved); terminal
  `--settings` uses **V Level stats** and **Y FPS display**. Vanilla shows
  neither by default (the console `fps` command still works there for the
  session).
- **LOAD GAME** (title, pause and death menus) lists the 12 savestate slots
  (the same slots as F7) with the level name, opening on slot 01.
  The chosen slot's picture, era and save time (in the console's steel
  lettering) are shown beside the list.
  Enter asks LOAD THIS GAME?; Enter again loads it, Escape goes back.
  Empty slots and saves from another build are greyed and cannot be chosen.
  Slots saved before level names were recorded read SAVED GAME. Memory card
  saves are no longer offered here in Modernized; Vanilla
  (`run.py --vanilla`) keeps the original memory card LOAD GAME.
- **Statistics screen** (after each level, one player): the original screen's
  look, with more lines: the level name and DIFFICULTY, then ENEMIES KILLED,
  TIME TAKEN, BEST TIME, SECRETS FOUND, MISSION ITEMS
  (found or used, on levels that have them), CHALLENGE FOUND and HEALTH /
  ARMOR (COMPLETED or FAILED after a challenge stage). BEST TIME is your
  fastest finish of that level on that difficulty; a faster one reads NEW
  BEST! and is kept in `ttk-best-times.txt` beside your savestates (never on
  the memory card). Loading a savestate onto this screen shows the stored
  best without recording again. After the original's short wait (about a
  second) a list appears under the lines: CONTINUE goes on to the next level,
  SAVE GAME opens SAVE GAME (a savestate slot, not the memory card). Up /
  Down or the mouse wheel choose, Enter (X) confirms; V is a shortcut for
  SAVE GAME. A save there returns to
  the statistics screen with GAME SAVED; Escape on SAVE GAME also returns.
  Such a save shows the level's last picture and LEVEL COMPLETE on LOAD GAME,
  and loading it opens the statistics screen again, ready to continue. SAVE
  is not offered before the final ending (CONTINUE only). Modernized never
  writes a game save to the memory card; Vanilla keeps the original card SAVE.
- **Deleting a save:** on LOAD GAME or SAVE GAME, press Delete on a used slot.
  DELETE SAVED GAME: asks with the slot's name in steel lettering (NO highlighted); YES removes
  that save (and its picture) and the slot reads EMPTY, with DELETED SLOT NN
  below the list. If the files cannot be removed the slot is kept and the page
  says SLOT NN COULD NOT BE DELETED. Saves from another build (greyed on LOAD
  GAME) can be deleted this way. Delete does nothing on an empty slot, and
  never touches memory cards.
- **Startup:** the memory card check runs at full speed, so the title follows
  the intro movies after a short black moment.
  Modernized no longer shows the PlayStation memory card checks
  (CHECKING MEMORY CARD, CHECKING SAVE GAME FILE, or the no card / not
  formatted message). The card is still read in the background, so the game
  settings kept on it (gore, status bars, game speed) still load. Vanilla
  shows the original checks.
- **Escape never quits.** Inside sub-screens and prompts Escape goes back one
  level. Escape on the main menu and PRESS START does nothing (QUIT GAME is
  on the main menu list).
- These lists free the mouse and work the same with modern controls captured,
  released for a menu or set to the original camera. Gameplay recaptures the
  mouse after RESUME or CONTINUE.
- **CONFIGURE CONTROLLER** assigns whichever pad button you press to the
  highlighted action. There, Escape and Enter both mean Start ("PRESS START TO
  CONTINUE"), which opens its CONFIRM / CANCEL prompt; then Enter confirms and
  Escape cancels. Z, X, C and V still assign Triangle, Cross, Circle and Square.
- **V** is Square, for example SAVE on the statistics screen (a shortcut for
  SAVE GAME in Modernized, the memory card in Vanilla).
- During gameplay Escape still pauses, Enter still uses the selected gadget
  and Backspace does nothing. Escape and Enter still skip the intro movies.
  The F7 savestate menu and the console keep their own keys.
- Vanilla keeps the original keyboard layout.

**KEY BINDINGS > HELD CROUCH** (C by default) toggles the crouch stance:
press once to stay crouched after releasing the key, then press again to stand
when there is room. **CROUCH** (Left Ctrl) remains momentary: hold to crouch,
release to stand. Pressing Ctrl while C has crouch toggled on takes over the
stance: the toggle clears, Duke stays crouched while Ctrl is held, and releasing
Ctrl stands him up when there is room. **Space** cancels
C's crouch toggle and stands Duke up without jumping. Release Space and press
again to jump; this follows the configured JUMP binding.

HELD CROUCH remains set across pause/focus changes. Loading, death, rebinding,
switching away from Modern controls or entering swimming, jetpack or unsupported
traversal clears it. C does not toggle underwater/jetpack descent; Ctrl retains
its existing dive/descent action. A custom C mapping is preserved during profile
migration; HELD CROUCH then starts unbound.

#### In-game OPTIONS (Modernized)

OPTIONS, on the title menu and in the pause menu, lists five screens, all in
the original's style:

- **SOUND**: MASTER, SOUND EFFECTS, MUSIC and SPEECH volume sliders, plus
  stereo / mono MODE. Left/Right changes the highlighted value; CONFIRM
  applies and saves the page, CANCEL discards edits, and DEFAULTS resets the
  draft to full volume and stereo. Confirmed settings survive game loads.
  MASTER uses the existing host volume, including its numpad shortcuts.
  Modernized keeps level music playing under speech, with independent MUSIC
  and SPEECH levels. Vanilla retains the original alternating playback.
- **CONTROLS**: mouse sensitivity (slider), invert mouse and **KEY
  BINDINGS** (below).
- **DISPLAY**: window (windowed / borderless / exclusive), resolution,
  widescreen, frame rate, renderer (Polymorph / Classic), first-person FOV,
  camera distance, **LEVEL STATS** and **FPS** (see Level stats below).
- **GAMEPLAY**: view, shoulder, camera distance, crosshair, gore, status bars
  and view bob.
- **ADVANCED**: one list under headings. CAMERA AND AIM: camera, weapon aim, first-person legs.
  MOVEMENT: jump, jump walls, wall backflip, crouch, swimming, jetpack.
  COMBAT: kick (third person: quick / original), weapon feel, **auto aim**.
  ITEMS: dynamite, steroids. GRAPHICS: backend (OpenGL / software), filter, draw distance,
  geometry, textures.
  PERFORMANCE: CPU speed, CPU timing. ORIGINAL GAME: red dot, game speed.

**Auto aim** (**HITSCAN** by default, see [Auto aim](#auto-aim-d10h3)) gives
the Desert Eagle, Combat Shotgun, Buffalo Rifle and Gatling EDuke32's auto aim
(**EDUKE32**: every gun).
**ORIGINAL** lets the original game's target lock help your aim instead: with
view aiming it nudges a shot onto the enemy the game has locked when that
enemy is near the crosshair, and with original weapon aiming it is the
original auto aim. The red dot shows the native aim
point during supported aiming/firing poses. It works with auto aim off too;
it is not a permanently visible crosshair. **RED DOT ON hides the modern
crosshair**, including its hotkey, to avoid two markers. Turning RED DOT OFF
restores the previous crosshair preference. Auto aim remains independent.
In Legacy, draw the weapon with
Circle/B, then fire with Cross/A. **Gore** and **game speed** remain the original GAME SETTINGS options,
kept on the memory card as before. Game speed is how fast Duke and the enemies move (50 is the
original's default); it is not the frame rate. **Status bars** now offers **Full**, **Borderless** and **Hidden** in
Options > Gameplay. During Modernized gameplay, **-** steps Full -> Borderless
-> Hidden, and **= / +** steps back. Each press moves one level and stops at
an endpoint. Borderless retains the icons and numbers, removing their frames
and the black squares behind the icons;
Hidden retains the previous Off behavior. Crosshair and inventory browsing are
independent. The choice applies immediately and persists across saves, levels
and restarts. Menu changes still require Confirm; Cancel and Defaults work as
usual. These shortcuts do not act in menus, binding capture or the console.

**LESS HUD** and **MORE HUD** can be rebound in Key Bindings, including to
keypad minus/plus or mouse buttons. The defaults use the number-row keys;
Shift is optional for plus. Vanilla keeps its original status-bar behavior.

The original CONTROLLER screen (PlayStation button layout) is not offered in
Modernized, and Modernized always plays on the default layout, because the
keyboard controls are built on it. A layout or auto aim setting you saved in
Vanilla stays on the memory card: Modernized saves keep it for Vanilla.

A dimmed row has no effect with the current settings; its description says
what it needs (for example shoulder and camera distance need camera
INDEPENDENT, resolution needs the OpenGL backend).

Up/Down choose (hold to repeat; the list scrolls), Left/Right change the value
(hold a slider to speed up), **Enter** confirms and keeps the changes,
**Escape** cancels them, **V** (Square, DEFAULT) restores the screen's
defaults. The line above the buttons describes the highlighted option.
These apply as soon as you confirm: mouse sensitivity, invert mouse, window,
widescreen, camera, view, shoulder, camera distance, weapon aim, crosshair,
auto aim, gore, status bars, game speed, red dot, jump, jump walls, landing,
swimming, dynamite, view bob and renderer. Frame rate applies at once too, from any launch rate (choosing 60 returns
to the original presentation); with the software renderer it says
**NEXT LAUNCH**, like backend, resolution, filter, draw distance,
geometry, textures, CPU speed, CPU timing, jetpack and steroids, which apply
the next time you start the game. A new widescreen shape also reshapes a plain
window (fullscreen and maximised windows show bars instead). Confirmed
settings are saved to your profile when the game closes, the same settings
`run.py` and `--settings` change. There is no profile choice here: Vanilla,
with the original menus, is `run.py --vanilla`.

**KEY BINDINGS** (CONTROLS, highlight it and press Enter) lists every
Modernized action in groups: MOVEMENT, WEAPONS, ITEMS, CAMERA and ORIGINAL
CONTROLS (the weapon keys are named after the first weapon of their group;
CROSSHAIR is the I key's action; USE ITEM shows ENTER, which always uses the
item; you can add a second key there, none by default). Highlight an action, press **Enter**, then press the new
key or mouse button:

- If another action had that key, the two swap; the line above the buttons
  says which action moved and to what.
- **Escape** cancels the change; **Backspace** leaves the action with no key.
- Keys outside the bindable set (Escape, Enter, Tab, the Up / Down arrows, F, the
  function keys and the other keys `run.py --bind` refuses) are refused;
  C can only be the holster's key.
- A change works straight away and is saved to your profile.
- **V** (DEFAULT) asks RESTORE DEFAULT KEYS? and puts every key back on YES.
- **Escape** returns to CONTROLS.

`run.py --bind` and `--settings` (6 PC bindings) still set the same bindings
outside the game.

**Backtick (`)** opens a Quake-style drop-down developer console (TTK Medium
Italic font, `>` prompt). Typed commands and their replies stay in the console scrollback (`help`,
`fps`, `clear`, `quit`, `palfrom` (see Screen flashes), `earthquake` (see Earthquakes), `renderer` (see Renderer), `light` (see Dynamic lights), unknown-command errors). `fps` also toggles a compact
persistent statistics block in the top-left of the game view after you close the
console (F remains unbound for gameplay); in Modernized it is the same setting as
OPTIONS > DISPLAY > FPS and is saved. Escape or backtick closes the console.
The console keeps 512 lines: **PgUp / PgDn** or the **mouse wheel** scroll back
through them, **Ctrl+Home** jumps to the oldest and **End** to the newest. New
output scrolls back to the bottom. **Up / Down** recall the commands you typed
(newest first; Down past the newest clears the line), and `history` lists them. Holding Backspace or a character key repeats it, like a desktop text field. The numpad types too: with Num Lock on its digits and `.`, and `- + * /` always; with Num Lock off its 8 / 2 recall commands, 9 / 3 scroll and 1 jumps to the newest line. Keypad Enter runs the line. While scrolled up, `-- MORE BELOW: PGDN / END --`
shows above the prompt.
In Modernized play the console does not pause the game and does not change
Duke's state: a C crouch stays crouched and a hovering jetpack holds its
altitude while you type. Nothing you type reaches Duke. Keys held when it opens
are let go; one still held when you close the console acts again at once.
The mouse is free while the console is open (moving it does not turn the view)
and captured again when it closes. Switching to another window while the
console is open does not change Duke's state either.
Time in the console does not count as standing idle: Duke does not put his
weapon away or start an idle fidget while it is open, and the idle countdown
continues where it was when you close it.

**First-person FOV.** In Modernized, **Options > Display > First-person FOV**
is directly after Frame Rate, followed by Camera Distance (as in EDuke32's display menu). The slider ranges from **60 to 120**, in one-degree
steps, measured horizontally at 4:3; widescreen
shows more horizontally at the same setting. Select the row with Up/Down,
then adjust with Left/Right, the controller D-pad, or the mouse wheel.
Confirm applies the change for resumed gameplay and saves it across restarts;
Cancel discards edits. Fresh or unset configurations and Display Defaults use **90**. Explicitly saved
preferences are preserved, including the previous default of 64. View changes
and save loads retain the preference.
The backtick console's `fov 75` remains a session-only diagnostic override;
`fov` reports the effective, integer-projection-quantized angle, and
`fov default` selects 90 for that session; `fov 64` selects the previous default.
Weapon/held-item display, their connected arms, Q and the door-kick leg retain
their original size and framing as world FOV changes. Contact arms follow the
world view, with environment lighting preserved.
Modernized first person shows ordinary legs by default while keeping the torso
hidden. OPTIONS > ADVANCED > CAMERA AND AIM > FIRST-PERSON LEGS switches
them On or Off. Confirm applies and saves the choice; Defaults restores On.
Arms, weapons/items, Q and native door kicks remain available with either choice.
Third person and reflections retain the complete Duke. The displaced-eye safety
guard still protects native empty-handed arms when an obstruction moves the eye
outside Duke. The same guard protects locomotion legs; climbing, swimming and
interaction legs use their existing action-camera ownership. Legs keep native
animation and world clipping, with the fixed display FOV used by weapons/kicks.
The sky backdrop adapts to world FOV and aspect ratio to maintain side coverage.
Third person keeps its own projection.
For a single launch, `DNTTK_FP_FOV=75 python3 recomp/tools/local/run.py`
sets the same developer control; an invalid launch value falls back to default.

**Debug level select (testing).** In a one-player game, open the console and
type `levels` to list the levels the original level-select cheat offers, with
the game's own names: 0 TIME TO KILL, 1-3 and 5-12 (the campaign), 21-26 (the
challenge stages) and 27-29 (the bosses). The current level is marked `*`.
Type `level N` (for example `level 1` for DUKE HILL). The console closes and the
game ends the current level the same way as its pause-menu restart, then loads
level N from its normal start, with no statistics screen. Duke starts with what
the game's restart gives him (full health); use the debugging cheats (`dnstuff`)
if a level needs more. Play and savestates then work normally. It works in
Vanilla and Modernized.
The level select is refused on the title screen, in the pause menu, while a
level is already ending and in two-player games. It does not write memory cards
or saves. Modernized controls and the selected view work in every listed
level.

**End the level (testing).** In a one-player game, open the console and type
`endlevel`: the level ends as if you reached its exit, so the statistics screen
follows, then the next level. Refused in the same cases as the level select.

**Debug spawn (testing, Modernized).** Like EDuke32's `spawn`: in captured
Modernized gameplay, open the console and type `spawn <item>` (for example
`spawn steroids`, `spawn skeleton key`, `spawn torn paper`) or `spawn <number>`.
The console closes and the item drops a few steps ahead, in the direction the
camera looks, using the game's own item-drop code, so walking over it picks it up
normally. An item big enough to reach Duke from there (the flamethrower, RPG and buffalo rifle
always; most weapons when the camera looks diagonally) lands farther out, up to
40 degrees to one side if that way is clearer, so Duke does not pick it up where
he stands. If a wall, ledge or solid object (such as a
dumpster) blocks every such spot, the item drops at the usual distance and the
message adds "(close: way ahead blocked)"; step back and spawn again. It bounces and settles exactly as an enemy's dropped item does, so it can
look partly sunk into the ground, as the game's own drops do. `items` (or `spawn` on its own) lists what this level can spawn: items by name,
then **weapons** and their **ammo** by name and number (D08A33, for example
`gatling gun (41)`, `freezer (641)`, `gatling ammo (562)`), then every other
pickup by number. The weapon numbers are: throwing knife 45, throwing axe 1,
crossbow 21, desert eagle 59, combat shotgun 66, buffalo rifle 10, gatling gun
41, rpg 47, flamethrower 37, energy weapon 34, freezer 641, pipe bomb 58, holy
hand grenade 608, dynamite 126; ammo: crossbow 609, desert eagle 33, shotgun
65, buffalo rifle 124, gatling 562, rpg 106, flamethrower 38, energy weapon
125, freezer 642. Only those this level has loaded are listed and spawnable. Each level loads only its own item models, so an item
from another level (crystals in Duke Hill, for example) is refused rather than
spawned invisible. Mission keys take this level's name (`key 1` is the Subway
Security Key in level 0 and the Warehouse Key in level 5). Vanilla refuses it.

*Energy crystals.* In the levels with crystals (0 Time to Kill, 5 Obey or Die,
9 Pig Factory), `spawn 1761`, `spawn 2761` and `spawn 3761` (or
`spawn red energy crystal`, `blue energy crystal`, `green energy crystal`) drop
the level's own red, blue or green crystal. Walk over it to collect it: the
original pickup message and sound play, and it counts as found everywhere (the
Select inventory, the `,` / `.` mission inventory and the crystal receptacles).
In the normal game you take crystals from their holders with the action button;
these numbers exist only in the console. If you already have that crystal, the
console says "(already found)". Other levels refuse them. `spawn 761` is a
different, generic crystal from the game data: it sets a hidden inventory slot,
not the level's crystals, and the console says so.

`spawn enemy` on its own (or `enemies`) lists the enemies you can spawn in
this level: type number, name where known (50 Necro), how many the level
places, and "(non-hostile)" for characters that do not fight. `spawn enemy <type>` brings one of them to life about 1500 units in front of
the view (for example `spawn enemy 50`, a Necro, in level 6). It uses the
level's own spawner: an enemy of that type not yet in play is moved there and
created the original way (one already in play is moved instead), and if the
level's limit of live enemies is reached the farthest one is removed first.
Only types the level places can be spawned.

**Debug sound audition (Modernized).** `sfx <id>` in the console plays a game
sound at Duke, `id` = bank * 0x1000 + index (for example `sfx 0x1012`, a low
thump; `sfx 0x100f`, the pickup sound). Bank 1 (`0x1000` to
`0x101d`) is Duke's own and is loaded in every level tried.

**I** toggles the Modernized crosshair on/off (EDuke-style). The original TTK
weapons/inventory screen stays on **Right Shift** (Select); press it again to close.

Choose **6 PC bindings** in `--settings` to edit Modernized actions. The editor
lists accepted keys and reports conflicting assignments without changing them.
You can also save a binding and return without launching:

```sh
python3 recomp/tools/local/run.py --bind jump=J
python3 recomp/tools/local/run.py --show-bindings
```

The labels use US physical key positions. Mouse1/2/3 mean left/right/middle;
Mouse4/5 are side buttons. To swap occupied inputs, enter both assignments at
once. Restoring the Modernized profile restores its bindings, camera preferences and renderer.
Keep custom runtime host shortcuts separate from these action bindings.

## Modern single-player controller controls

The first assigned SDL controller drives Modernized campaign
movement and the independent camera. Keyboard and mouse remain usable alongside
it. Gameplay capture happens automatically; Start opens pause.

| Control (PlayStation / Xbox) | Action |
| --- | --- |
| Left stick | Move relative to the camera |
| Right stick | Look and aim |
| L3 / left-stick click, held | Run (walk while RUN MODE is on) |
| R2 / RT | Fire; draw a holstered weapon |
| L2 / LT, held | Grab and manipulate; precision aim with Original aiming |
| Cross / A | Jump; ascend in water or flight |
| Circle / B, held | Crouch; descend in water or flight |
| Square / X | Interact, mantle or climb |
| Triangle / Y | Draw or holster |
| L1 / LB and R1 / RB | Previous and next weapon |
| R3 / right-stick click | Quick kick |
| D-pad left / right | Previous / next gadget |
| D-pad up | Use selected gadget |
| D-pad down | Toggle jetpack |
| Select / Back | First-/third-person view |
| Start / Menu | Pause |

**OPTIONS > CONTROLS** offers controller look speed (30-360 degrees/second),
radial stick deadzone (5-50 percent) and independent vertical inversion. Defaults
are 180 degrees/second, 20 percent and inversion off. Confirm changes to save.
Controller action mappings are fixed; KEY BINDINGS edits
keyboard/mouse actions. Stick direction is analog; walking/running retain the
accepted game speeds. Buttons/triggers held through a menu or focus transition
must be released before they can act again.

Vanilla and classic split-screen retain their original controller controls.
Controller behavior was user-accepted on 2026-10-27. The separate physical
two-pad selection report remains undiagnosed; keyboard/gamepad as separate
local player seats is queued as D09A.

## Classic two-player split screen

For **keyboard P1 and gamepad P2**, first open **OPTIONS > CONTROLS**, set
**TWO PLAYER INPUT** to **KEYS AND PAD**, and confirm. This saves the preference.
Choose TWO PLAYER. Keyboard arrows and Enter select/confirm P1; the first
assigned gamepad selects/confirms P2. Confirm again when both are ready.

P1 uses W/S to move, A/D to turn, Space to jump, Ctrl to crouch, Shift to walk,
X or left mouse to fire/action, C or Scroll Lock to draw/holster, E to act,
and Alt or right mouse for original precision aim. Hold C with left/right to
cycle weapons. Escape pauses and Enter resumes. Bound actions follow your
keyboard preferences; arrow keys and the fixed X/C controls also work.
P2 retains the original pad controls. Split screen has original turning and
camera rules, without mouse look. Unplugging the pad leaves P2 neutral until
reconnected. **TWO PADS** restores the existing two-controller setup below.


Connect two SDL-recognised controllers, launch normally, and choose **TWO
PLAYER**. The first available controller drives player 1 and the next drives
player 2. In menus, use D-pad/left stick to choose, Cross/A to confirm and
Triangle/Y to go back. Each controller chooses its own character; confirm again
once both choices are locked to start. The screen now states whether two
controllers are connected and reminds each player to confirm.

Matches use the original turning, weapon and movement controls shown in the
controller columns below. Circle/B draws or holsters the weapon; Cross/A fires
when drawn. Start/Menu on either controller pauses. Both vertical and horizontal
split use the original 4:3 picture and native viewport update cadence. Your
single-player Modernized camera, widescreen and frame-rate preferences remain
saved and resume in single-player play.

The results screen offers Cross/A to replay and Triangle/Y to quit. Choosing
MAIN MENU from pause shows the results, then Quit returns directly to the main
menu. Existing one-player savestates remain loadable; no save conversion is
required.

Accepted foundation (2026-10-27): the user confirms keyboard/gamepad routing
and local two-player work. This mode retains the original movement, camera and
viewport cadence. D34C is the queued modernization follow-up for independent
per-player controls, cameras, aiming and smoother viewport presentation; those
features are not implemented yet. The earlier character-selection report's
cause remains undiagnosed; acceptance does not imply exhaustive two-pad or
all-arena testing.

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

### Quick save and quick load (Modernized, D20)

These work the way they do in Duke Nukem 3D / EDuke32, in one-player games:

- **F6 quick save** saves over the last slot you used this session (saved to
  or loaded from, by any route: SAVE GAME, LOAD GAME, F7 or the quick keys)
  and says GAME SAVED. Play does not stop. If you have not used a slot yet
  since starting the program, F6 opens SAVE GAME instead. Dead, it says
  CAN'T SAVE WHILE DEAD.
- **F9 quick load** loads that slot at once, with no question, and play goes
  on from it. With no slot used yet (or if that save is gone or from another
  build) it opens LOAD GAME. F9 also works on the death screen.
- **F2** opens SAVE GAME and **F3** opens LOAD GAME straight from play.
  Escape there goes back to play, and a save made there returns to play with
  GAME SAVED. (F3 on the death screen opens LOAD GAME, and Escape returns to
  the death screen.)
- The quick keys do nothing in the title menus, the pause menu and two-player
  games. QUICK SAVE and QUICK LOAD can be rebound in KEY BINDINGS (section
  SAVES; F4, F5, F6, F9 and F12 can be used there). F2 / F3 are fixed.
- The console has the same commands: `quicksave` and `quickload` ("Can't
  quicksave while not in a game." outside one).
- The last slot used is not remembered when you close the program, as in
  EDuke32.
- In Modernized, F6 no longer swaps controller ports; that stays in the F1
  runtime menu. Vanilla (`run.py --vanilla`) keeps F6 as the port swap and has
  no quick keys.
- Fixed (D20C, which also covers D20A): loading a save could sometimes show
  scrambled textures or wrong colours (mostly in first person), and a save
  made just after a load could crash when loaded after a restart. Saves now
  carry the enhancement memory the picture depends on. Saves made by earlier
  builds also load cleanly, though the first frame after loading may be
  partly drawn.

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
before adding forward travel. Running off any ledge keeps Duke's running speed
into the fall, like Duke 3D, and the mouse camera stays live until he lands
(never faster than a running jump). Walking off small furniture drops keeps
walking speed; walking still stops at large ledges. The apartment's concealed pipe bombs wait for
the bed secret to open. Nearby switch targeting is more forgiving; the original
interaction and lights-before-dialogue secret remain. Vanilla retains its original
rules. Bed/couch contact, walk/run departures and release checks passed privately;
your movement feel and broader terrain still need playtesting.

**Mantles and ledge hangs (Modernized, D08V):** climbing onto ledges, hanging
and pulling up, and falls that did not start as your own jump keep the mouse
camera and modern controls instead of switching to the original camera for a
moment. Duke's climb itself is the original animation. In first person the view
steps out behind Duke during the climb and returns afterwards. Ladders, poles
and chains keep the mouse camera too (D08J3).

### Pushing and climbing objects (Modernized)

Some objects can be pushed, such as the green dumpster in the first map's alley
and the blocks and walls marked with the Duke symbol in later eras.
Being pushable never changes how you climb: **E** works on it exactly as on any
other object you can climb.

| Input | Result |
| --- | --- |
| **E** (with **W** into it) | Climb it, like any climbable object. E alone while standing does nothing |
| **Hold right mouse** (or **Alt**) while touching it | Grab it (your weapon is stowed first if needed). Holding W into it is fine |
| **W / S** while holding the button | Push / pull. Relative to the camera: W pushes when you look at the object |
| **Release** the button | Let go. Your weapon comes back |
| **E** while holding it | Let go, then E works as usual (W + E climbs) |

Grab is a held action: Duke holds the object only while you hold the button.
After Duke lets go by himself (for example, he is hit or you pause),
release and press again to grab once more. Space, fire and weapon changes wait
until you let go. W/S move the object continuously: release movement to stop
while keeping your grip, or reverse without regrabbing. Release Grab to stop
and let go. Touching a pushable object shows which button
grabs, and each grab shows the push/pull keys (at most once every few seconds). With `original` weapon aiming or camera, only **Alt** grabs (right mouse
is precision aim there); both are rebindable (`grab`, `grab_alt`).

An object you have pushed can always be grabbed again along the line it
moved on, wherever it ended up, so pushing one into a recess or a corner no
longer leaves it stuck (D08T11, awaiting playtest). Objects the level placed
as fixed scenery, such as the street dumpster in level 5, still cannot be moved. A pushed object stops
against bars, gates and frames like any wall, and so does Duke while pushing (D08T13).

Mouse look keeps working while you hold an object. With global VIEW set to
FIRST PERSON, **PUSH/PULL VIEW** in OPTIONS > ADVANCED > ACTION PERSPECTIVE
selects FIRST PERSON (default) or the existing THIRD PERSON interaction view.
CONFIRM applies and saves it. The eye stays over Duke's body, and in first
person both hands hold the object's near face at chest height while you grab,
push, pull and let go, instead of the third-person turn-and-shove arms (D35D1). Push/pull and use-interaction arms
now clip at the visible geometry instead of disappearing as whole segments
when their joint origins approach the eye. Global THIRD PERSON always wins,
and releasing the object follows the current global VIEW. E-to-mantle still
uses the existing third-person mantle camera. The launcher accepts
`--push-view first` or `--push-view legacy-third`; these save the preference.
Face the object roughly square, as in the original. Objects stop moving when something blocks
them or their footprint reaches unsupported ground. Duke keeps holding the
object, so you can reverse away from the obstruction. Existing floating objects
in older saves are not repositioned.

The first-level alley dumpster stops short of the ladder platform, at the
position recorded in the reference slot 7, leaving room for the ladder approach.
You can reverse while keeping your grip, or let go, walk back into contact and
hold Grab again to pull it back. This also works after loading it at the limit.
The alley dumpster, the movable level-5 dumpster, the verified level-1 box
(reported in slot 5), and the three verified medieval symbol-block lanes retain
this grab permission along their travel lane; face an
end squarely. The separate fixed level-5 dumpster stays fixed. Collision,
floor support and standing room still decide whether movement is safe.
Older saves with the dumpster closer to the ladder keep their position and can
retreat when a safe grip and supported path are available. Modern standing and crouched walks can leave the
dumpster's edge (D08L1).

Some pushable objects are fixed in place where the level puts them, as in the
original, for example a street dumpster in level 5 (debug console numbering),
which is a step you climb. Those show no grab prompt and the grab button does
nothing at them; **E** (with **W**) still climbs them.

In **Vanilla** the original rules apply: stand still facing the object with no
weapon drawn, hold **X**, then press **Up** to push or **Down** to pull. Holding a
direction together with X climbs instead.

### Typed debugging cheats (Modernized)

With mouse capture active, type a code directly - no console or Enter. Codes work
anywhere Duke is alive: on the ground, jumping, swimming, under water, flying the
jetpack, on ladders and climbing. They are refused while Duke is dead or during a
scene. A message confirms the result. The first D can move Duke slightly, so
start away from ledges. Escape or focus loss cancels typing; F10 has no action.
The mouse keeps turning the camera while you type. Any key that is not the next
letter of a code (W, Space and so on) or a mouse click cancels the code and does
its normal job straight away.

| Code | Effect |
| --- | --- |
| `dnmonsters` | Toggle enemies hidden/shown; NPCs, pickups and switches remain |
| `dnkroz` / `dncornholio` | Toggle god mode: health to at least 100, and Duke has the jetpack at full fuel the whole time it is on (Atomic Health can still take Duke to 200). It is a toggle: if god mode is already on, typing it turns it off. The confirmation reads GOD MODE: ON or GOD MODE: OFF; switching off flashes the screen red, as a hit does in Duke Nukem 3D (Polymorph renderer; Classic shows no flash), and Duke sometimes grunts (no damage) |
| `dnstuff` | Grant all weapons/ammo, inventory and keys, and full (100) armor |
| `dnkeys` | Grant keys |
| `dnweapons` | Grant weapons and ammo |
| `dninventory` | Grant inventory and full (100) armor. With portable steroids this includes one held steroids; if steroids are running they stop and a full dose is held instead, ready for R |
| `dnitems` | Grant inventory and keys, and full (100) armor |
| `dnhyper` | Activate/refill steroids (starts the effect directly, as in EDuke32; with portable steroids it counts down in the steroids box and is never kept as a held dose) |
| `dnammo` | Refill ammo for owned weapons |
| `dnhealth` | Restore 100 health |
| `dnunlimited` | Toggle unlimited ammo/charges |
| `dnupgrade` | Upgrade every weapon (Laser Gatling, Incendiary RPG, HiTemp Flamethrower, plus Desert Eagle, Shotgun and Energy Weapon); weapons picked up later arrive upgraded, and the upgrade stays through saves |
| `dntlod` | Just for fun: shows "Play Zaxtor's Oblivion!" |

The three inventory cheats (`dnstuff`, `dninventory`, `dnitems`) treat steroids the same way: a running dose stops and comes back as a full held dose (Modernized portable steroids). Armor and steroids are separate, so the armor grant never changes the steroids.

These use Time to Kill's inventory equivalents. Use a test save and show enemies
again before saving or loading: hidden-enemy state is session/scene-local, and its
save/load round trip is not supported. Codes are unavailable in Vanilla, menus,
multiplayer, while dead, or during attached traversal. No clipping or level-warp
cheat is included. See [implementation and testing notes](documentation/38-debug-cheats.md).

### Savestate menu (F7)

**F7** opens the savestate slot browser (12 slots). In Modernized it uses Time to
Kill's own look:
- "SAVE STATES" in TTK Big Italic gold;
- one card per slot with its thumbnail, the slot number and the level name
  (the selected slot in blue), the save time, and the level and its era;
- the PlayStation button prompts from the disc.

F6 / F9 quick save and quick load use the same slots (see Quick save and
quick load). The keys are unchanged: arrows pick a slot, **Enter** or **L** loads,
**Shift+Enter** or **S** saves, **Escape** goes back. The level is recorded when
you save, so slots saved before this version show only their number and time
until saved again. Vanilla, or a build without the disc-derived packs, shows the
plain framework menu.

### Message and console fonts

Modernized messages and the developer console use Time to Kill's own fonts,
taken from your disc when the game is built (no Duke Nukem 3D art):

- **Messages** (cheats, notices, `MEDKIT 75%` and similar): TTK Big Italic in
  the blue-silver "Console steel" colours with a dark navy shadow.
- **Console** (backtick): TTK Medium Italic, Console steel, with the same shadow.

All text is shown in capitals, as in the original game. Both are drawn at half the overlay scale (1x at 1080p; the console canvas is 1280 wide), so they stay compact on large screens. A few symbols the TTK
fonts lack (`! % ( ) & + "` and similar) use the game's own small 8x8 font. The
original game text and Vanilla are unchanged. If the font pack is missing (no
prepared disc), messages use the generic host font.

### Instant night vision (D08A23)

With Modern controls, **N** toggles night vision immediately, including while
firing, moving or changing weapons. There is no goggles equip/removal animation,
holster or added shot delay. **Enter** on the selected goggles does the same;
holding either key toggles once. The original TTK visual effect, battery capacity
and drain remain. Switching off preserves the remaining charge; depletion turns
the goggles off automatically. Goggles replace the Bio Mask, as the original
face-equipment system allows one of those items at a time.

Modern night vision plays the same short electronic cue when the goggles turn
on or are manually switched off. It follows the SOUND effects volume and host
volume. Empty activation, battery depletion and restoring active goggles from
a save do not play the cue. Original night vision and Vanilla retain their original audio.

**OPTIONS > ADVANCED > ITEMS > NIGHT VISION** offers **MODERN** (default) and
**ORIGINAL** (the original equip animation). Confirm applies it immediately and
saves the choice. Vanilla retains original behavior. The launcher also accepts
`--night-vision modern|original`; this saves the preference. In `--settings`,
choose **N Night vision**.

### Cowboy hat under the Bio Mask (D08A35)

In the Old West levels, Duke keeps his cowboy hat in third person while the Bio
Mask or the night-vision goggles are on (Modernized; Vanilla shows the original
bare head).

### First-person Bio Mask mesh (D08A26 / D08A28)

While the Bio Mask is active in Modernized first person, a low-poly respirator
and angular five-sided hoses appear along the bottom of the view. The dark grip
textures and graded steel match the original weapons' visual style. Their brightness and tint
follow the drawn weapon's native lighting; with the weapon holstered, they use
Duke's current brightness. They follow View Bob
(off/subtle/on/strong), stay beneath the HUD, and disappear when the mask is
removed or the view changes to third person. This adds no protection or charge
changes. Use **B** to toggle an owned, charged mask, or let automatic gas
protection equip it. Vanilla keeps its original presentation.

Modernized toxic gas fills the active widescreen viewport, including 21:9 and
auto resizing. Its native appearance and gas/Bio Mask behavior are preserved.

### Automatic and instant Bio Mask (D08A24)

**OPTIONS > ADVANCED > ITEMS > AUTO BIO MASK** defaults to **ON**. An owned,
charged mask activates instantly when native toxic gas reaches Duke's head,
before gas damage. **B**, or **Enter** with the mask selected, also toggles it
instantly. Movement, camera view and firing continue without an equip animation.
Holding the key toggles once. The mask and goggles remain mutually exclusive.
Your edited gas-decompression cue plays on activation and manual or automatic
removal, following effects volume. Empty attempts and depletion are silent.

The mask removes automatically after three consecutive native game updates
outside gas exposure, stopping charge drain. This short exit grace prevents
flicker at the boundary; entry protection is immediate. B can still switch it on
manually in clean air. Removing it in gas, or replacing it with Modern goggles,
suppresses automatic activation until Duke leaves and re-enters. Loading a save
starts a fresh exposure check: an active mask in gas stays on silently; an active
mask in clean air removes with the usual cue after the exit grace. No mask or no charge means no protection. The mask does
not protect against bullets, falls, water or other non-gas hazards.

**OFF** restores original manual Bio Mask behavior. Confirm applies and saves the
choice immediately. Vanilla stays original. The launcher accepts
`--auto-bio-mask on|off` (saves the preference); terminal `--settings` uses
**M Auto Bio Mask**.

### Equip pickups (D08A33)

**OPTIONS > ADVANCED > ITEMS > EQUIP PICKUPS** works like EDuke32's option of
the same name. **IF NEW** (the default): picking up a weapon Duke does not have
yet (for example the Gatling gun) draws it at once, with its normal draw
animation, ammo and HUD; picking up a gadget makes it the selected item. A
gadget is only selected, never used: **Enter** uses it. A weapon Duke already
has, and ammo, never switch. If Duke is firing, climbing, pushing or in the
middle of a throw or another weapon animation, the new weapon is drawn once he
is ready and fire is let go, within about two seconds; pressing a weapon key
yourself first keeps your choice. Weapons from cheats (`dnstuff`) never switch.

**NEVER** equips and selects nothing, as in the original game. A gadget is
still selected when it is the only one you carry. Confirm applies and saves the
choice immediately. Vanilla always uses NEVER. The launcher accepts
`--equip-pickups if-new|never` (saves the preference); terminal `--settings`
uses **E Equip pickups**.

### Third-person quick kick (D12E1)

In third person, **Q** is a fast MMA-style roundhouse with the left leg, the
third-person partner of the first-person quick kick: the same cadence (the shin
is out about 0.15 s after the press, about 0.45 s a kick), reach and damage.
Duke's hips turn over into the kick while his arms, chest and weapon stay near
your aim, so you can keep firing while he kicks, with any weapon or holstered,
standing, walking, running or in a jump. Tap Q repeatedly or hold it to chain
kicks. With the Mighty Boot selected (key 1), holding the
left mouse button kicks too. While the leg is out Duke slows to a walk, and he
steps a little into each kick (never through a wall or an edge). A kick that
connects lands with the impact thud (its pitch varies a little from kick to
kick), a burst of sparks at the contact, a brief hold of the strike pose and a
small camera punch; a kick into empty air only whooshes. Kicks do not knock
enemies back. In a jump the other leg keeps the jump's pose and there is no
step. Not crouched, swimming, climbing or on the jetpack (Q does nothing
there, as before). E never kicks.

**OPTIONS > ADVANCED > COMBAT > KICK** chooses **QUICK** (the default) or
**ORIGINAL** (the original full-body kicks, only standing still with the Boot
selected and holstered). First person always uses its own quick kick. Vanilla
always uses the original. The launcher accepts
`--third-person-kick quick|original` (saves the preference); terminal
`--settings` uses **Q Third-person kick**.

### Weapon feel (D10H4, D10H6, D10H9)

**OPTIONS > ADVANCED > COMBAT > WEAPON FEEL** chooses how the guns with a Duke
Nukem 3D counterpart answer the fire button: the Desert Eagle (key 2), Combat
Shotgun (3), Gatling Gun (4), RPG (5) and Freezer (0).

- **RETRO** (the default) is a close simulation of Duke Nukem 3D (EDuke32):
  - Every click fires, even a very quick tap, and the round leaves on the
    game's next update after the click: about 50-70 ms for the Desert Eagle,
    Gatling, RPG and Freezer and about 85-100 ms for the Combat Shotgun,
    instead of about 200-350 ms from a lowered gun.
  - Holding the button fires at a steady rate, standing, walking, running,
    crouching, jumping or swimming: the Desert Eagle 6 shots a second, the
    Gatling 10 and the Freezer 7.5. The Combat Shotgun and RPG keep their
    original rate. While you hold fire on one of these guns the game runs at
    CPU SPEED 200 (or your own setting if it is higher), so busy moments do not
    make the rate uneven; your CPU SPEED returns once you let go. To try
    without it, launch with `--fire-cpu off` (this launch only).
  - With one of these guns up, running into a wall just stops Duke: he no
    longer stumbles and lowers the gun, so you keep firing.
  - In water the game itself slows down while you fire, so held shots there
    can come a little early or late (the average rate holds).
  - **Energy Weapon (key 7) and Flamethrower (key 8):** every click fires too.
    The gun comes up at once and the beam starts on the game's next update
    (about 50-100 ms instead of about 300). The Flamethrower ignites in one
    update instead of three, so its first fireball comes after about
    80-140 ms instead of about 450. After you let go, the gun stays up for a
    second, so a click in that time fires within about 50 ms (Energy Weapon)
    or 50-85 ms (Flamethrower); before, the gun lowered at once and a click
    waited for it to lower and come back up (about 450-600 ms). Their held
    stream is unchanged.
- **MODERN** keeps the original timing: the original raise speed and fire
  rates (with their uneven spacing), and a tap shorter than one game update
  may not fire. On the Energy Weapon and Flamethrower a click shorter than
  their raise (about 300 / 450 ms) fires nothing.

Ammo, damage and spread are unchanged, and so are the Energy Weapon's lock and
reach and the Flamethrower's fireballs. The Crossbow, Buffalo Rifle and the
throwables keep their original timing. Vanilla always uses the original
timing. The launcher accepts `--weapon-feel retro|modern` (saves the
preference); terminal `--settings` uses **H Weapon feel**. A profile saved by
the first build (which called RETRO "EDUKE32") keeps the choice under the new
name.

### Auto aim (D10H3)

**OPTIONS > ADVANCED > COMBAT > AUTO AIM** chooses how much the game helps your
aim in Modernized view aiming:

- **HITSCAN** (the default) is EDuke32's auto aim on its "hitscan" setting:
  when you fire the Desert Eagle (key 2), Combat Shotgun (3), Gatling Gun (4)
  or Buffalo Rifle (9), the round goes exactly at the nearest enemy
  within about 8.4 degrees either side of the crosshair (about 17 degrees
  across) that you can see and that the gun can reach, and that is roughly on
  screen vertically. It only ever picks enemies: dancers, civilians and other
  NPCs, and props, are never targeted (you can still shoot them by aiming at
  them yourself). An enemy already under the crosshair is chosen first, so
  you can pick the farther of two. The shotgun's pellets still spread around
  that line. Walls, doors and other cover still stop the round. The RPG,
  Freezer, Crossbow, Flamethrower, Energy Weapon and throwables are not
  assisted.
- **EDUKE32** is EDuke32's auto aim on its "all" setting: the same aim for
  every gun, the RPG, Freezer, Crossbow, Flamethrower and Energy Weapon too
  (its beam locks onto the auto aimed enemy). Throwables (pipe bombs,
  grenades, dynamite, knife, axe) keep their own throw, as in EDuke32.
- **OFF**: every round goes where the crosshair points.
- **ORIGINAL**: the original game's target lock helps your aim within six
  degrees, and the original's own auto aim is on (as before this option).

Profiles saved before this option that had auto aim off (the old default)
switch to **HITSCAN** once, and so do the test builds' DUKE 3D and EDUKE32
choices (both were the hitscan behaviour); afterwards your choice is kept. Vanilla always uses
the original game's aiming. The launcher accepts
`--aim-assist eduke32|hitscan|off|original-lock` (saves the preference). The backtick
console's `autoaim` shows the cone, and `autoaim 64` (in 2048ths of a turn
either side; `autoaim default` is EDuke32's 48) changes it for the session.

### Sidearm return (D08J7C)

On a ladder, chain, pole, climbing wall or ledge hang, left click swaps your
selected weapon for the one-handed sidearm. **OPTIONS > ADVANCED > ITEMS >
SIDEARM RETURN** decides what happens once Duke is back on the ground with the
sidearm still out (after letting go armed or an armed pull-up). **NEVER** (the
default): he keeps the sidearm until you choose a weapon. **AUTO**: he keeps
the sidearm while you keep shooting, and your weapon comes back by itself
about 0.75 seconds after your last shot; choosing a weapon yourself first
keeps your choice. Holstering while still on a climb gives your selection back
either way. Confirm applies and saves the choice immediately. Vanilla has no
sidearm swap. The launcher accepts `--sidearm-return auto|never` (saves the
preference); terminal `--settings` uses **A Sidearm return**.

### Screen flashes (D39C)

Modernized can flash the whole screen with a colour, the way Duke Nukem 3D
does for hits and pickups: the picture jumps toward the colour, then fades
back smoothly over a moment. Turning off the god-mode cheat flashes, and so
does any hit that actually hurts Duke (D39E):

- **Shots, blows and most hits**: red, as in Duke 3D (`palfrom 32 32`). A
  light hit, under 3% of health and armour together, is a softer red
  (`palfrom 16 32`, Duke 3D's kick).
- **Blasts**: red; a heavy one (15% or more, or a killing blast) is Duke 3D's
  strong RPG red (`palfrom 48 52`).
- **Necro bolts**: Duke 3D's purple Octabrain flash (`palfrom 48 48 0 48`).
- **Frozen** (Freezer): Duke Nukem 3D's frozen blue (`palfrom 16 0 0 24`)
  for as long as Duke is frozen, gone the moment he thaws. It now shows in third person too;
  SCREEN FLASHES REDUCED makes it a quarter, OFF turns it off. The Classic
  renderer keeps the older tint.
- **Falls**: soft red; a heavy fall (25% or more) the full red.
- **Toxic gas**: a sickly yellow-green while it hurts Duke.

A hit that does not hurt Duke never flashes: god mode, invincibility, a worn
Bio Mask in gas. A light hit does not cut short a stronger flash that is
still fading.

Running power-ups hold a light tint of their own (D36M; see the power-up
countdowns above).

Pickups flash too, as in Duke 3D (D39F): a light green for items, ammo,
keys and inventory (`palfrom 16 0 32`), a stronger green for a weapon
(`palfrom 32 0 32`). Only a pickup Duke actually takes flashes; walking over
a full item does nothing, and nothing flashes while a power-up tint runs.
When Duke dies, the killing hit's flash fades out as usual instead of
staying on the death screen. Explosions flash only when they hurt Duke (the
strong red above).

**OPTIONS > GAMEPLAY > SCREEN FLASHES** (accessibility):

- **ON** (the default): flashes at their full Duke 3D strength.
- **REDUCED**: half as strong, and never more than a quarter of the way to
  the colour. Durations are unchanged.
- **OFF**: no flashes are drawn.

Confirm applies and saves the choice immediately. Vanilla never flashes. The
launcher accepts `--screen-flashes on|reduced|off` (saves the preference);
terminal `--settings` uses **X Screen flashes**. Console flashes need the
Polymorph renderer (Classic answers that it cannot show them). The Classic
renderer shows no god-mode, hit or pickup flashes; only the frozen tint
remains there.

The backtick console's `palfrom I [R G B]` (or `flash`) starts a flash with
Duke 3D's own numbers: intensity `I` from 0 to 255 (63 is the strongest; above
63 stays at full strength for longer; 0 clears a flash) and red, green and
blue from 0 to 63 (missing ones are 0). Each 1 of intensity lasts one 30 Hz
tic. A new flash always replaces the current one. The console closes so you
can see it. Examples: `palfrom 32 32` (the god-mode off red), `palfrom 48 52`
(an RPG hit), `flash 16 0 32` (an item pickup), `palfrom 32 63 63 63`
(white). It only works in play, in Modernized.

### Earthquakes (D44A, D44B, D44C)

Modernized shakes the whole level the way Duke Nukem 3D does when a big
charge goes off (Duke 3D's earthquake, as EDuke32 plays it):

- **What starts one**: a TNT barrel going off, and an explosive-vest pig
  cop's bomb. Each one shakes for about half a second (16 of Duke 3D's
  30 Hz tics); a chain of barrels keeps it going, each blast starting it
  again, so a room of TNT gives one long earthquake. Rockets, grenades, pipe
  bombs, the Holy Hand Grenade and dynamite never cause one (as in Duke 3D);
  they keep the game's own short camera jolt.
- **Level events** (D44B): the subway train crashing at the end of the
  tunnel in TIME TO KILL (started from the ticket booth) shakes for about
  1.6 seconds from the moment of impact.
- **The game's own big shakes** (D44C): the robot turrets that drive around
  OBEY OR DIE and PIG FACTORY blowing up, and WING'ED DEATH and MOLOCH THE GATEKEEPER landing or stomping,
  give one blast's earthquake each. In MINER 69ER the rooms that rumble
  (the game's own tremor) shake as an earthquake for as long as Duke stays in
  them, ending about half a second after he leaves.
- **What it looks like**: the view jumps up and down every tic and twitches a
  little to the right every other pair of tics, at full strength until it
  stops dead. It is the same everywhere in the level, however far away the
  blast was. Only the world moves: the status bars, the crosshair and, in
  first person, Duke's hands and weapon stay still. In third person Duke
  shakes with the ground he stands on.
- **Everywhere Duke is**: it keeps shaking while he jumps, falls, swims, flies
  the jetpack or climbs (Duke 3D stops it in the air; here it does not).
- **Controller**: the pad rumbles with it, as faintly as EDuke32's does, when
  vibration is on in the game's controller options.

Aiming, movement and the camera's own motion are never moved by it: shots go
where the steady crosshair points. **OPTIONS > GAMEPLAY > EARTHQUAKES** turns
it **ON** (the default) or **OFF** (barrels, vests, the bosses and the mine
then give the game's own jolt or tremor). Confirm applies and saves the choice immediately. Vanilla never
has earthquakes. The launcher accepts `--earthquakes on|off` (saves the
preference); terminal `--settings` uses **Z Earthquakes**.

The backtick console's `earthquake <seconds>` starts one by hand (decimals
work: `earthquake 1` is one second, `earthquake 2.5`); `earthquake` on its own
is one blast's worth, and `earthquake 0` stops one. Like a blast, it replaces
an earthquake already running rather than adding to it, and the console closes
so you can see it. It only works in play, in Modernized.

### Inventory feedback and crouch scope (2026-09-28)

In Modernized gameplay, **[ / ]** open the temporary gadget **switcher** (centered
strip with Time to Kill's own HUD item icons (D08A3: jetpack,
Bio Mask and goggles; the medkit and held steroids are the project's own sprites) and green 3x5 Microfont charge); **Enter**
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
the project's own lime-green 9x9 crosshair (it replaced EDuke's CROSSHAIR tile
2523). It stays at least 18 pixels wide in small windows and scales with the
fitted game picture at larger sizes, with crisp edges. **I** still toggles it.
The selector frame is the project's own metal frame. See
[session findings](documentation/51-session-inventory-crosshair.md).

**Mission items (D08A5).** **, / .** (comma and period, the < and > keys) open
the mission inventory in levels with mission items. It shows in place of the
gadget switcher: one framed slot per key, crystal, paper or jewel the level asks
for (the same list as the original Select inventory), with items you have not
found yet as dim grey silhouettes, and "MISSION" and found/total at the bottom
(green when everything is found). The selected slot's frame is blue, and a card
at the top of the screen shows its name, type and FOUND or NOT FOUND YET. The
first press opens on the item you last looked at; then **.** moves to the next
item and **,** to the previous one (wrapping). It closes about 2.5 seconds after
your last press. **Enter** while it is open closes it and makes Duke try
to use the item shown (D08A18); no gadget is toggled. Standing at the lock that
takes it (a card reader, a key door, a crystal holder or receptacle), it does
exactly what **E** there does: Duke puts his weapon away, takes the item out and
uses it, and the original uses it up. Anywhere else, "CAN'T USE THIS HERE"
appears (in the cheat-message style) and nothing changes; an item you have not
found yet says "NOT FOUND YET". Once an item has been used at its lock
(D08A19), it keeps its full-colour icon with a green tick in the strip and on
the card, the card says USED (in gold), it still counts toward found/total, and
Enter on it says "ALREADY USED". Saves and savestates keep this; a new level
starts clean. **[ / ]** switch to the gadgets. The
gadget switcher itself shows gadgets only; its selected frame is now orange.
Levels without mission items ignore **, / .**. Keyboard only for now. Vanilla is
unchanged; the original L1/R1 sidesteps that comma/period used to send are
unbound in Modernized (A/D strafe everywhere) and can be rebound.

**Selected gadget on the HUD (D08A6).** In Modernized the gadget that **Enter**
will use always has its own box in the bottom right corner, just above
the ammo box, in the status bar's own style: the gadget's icon (a medkit case
for the medkit; the health box keeps its red cross) and its charge in the red HUD digits. Bright digits mean
the gadget is on; dim digits mean it is selected but off. The medkit is always
bright. The box changes as soon as you move to another gadget with **[ / ]**, even in flight or while firing; only **Enter** use it. If the jetpack is
on while another gadget is selected, its box moves one row higher; the Bio Mask
and goggles boxes show in their usual place left of the ammo box. With no
gadget, there is no box. Vanilla is unchanged.

Typed cheats stay silent until a completed result appears, horizontally centered
at the top of the screen. Partial, invalid and cancelled entries display nothing.

### Crouch walking and running (Modernized)

With **CROUCH: MODERN** (the default; OPTIONS > ADVANCED > MOVEMENT, or
`run.py --crouch modern|original`), **hold Left Ctrl** or **tap C** to crouch,
then use **W/A/S/D** to move low in any direction. **Hold Left Shift** to move
at running pace while staying crouched; release it to return to crouch-walk pace.
These follow your configured crouch and speed bindings. Caps Lock RUN MODE works
as it does while standing: Shift temporarily selects the opposite pace.
Walking and running use separate low animations. Running uses longer strides,
a stronger arm swing and brief airborne recovery; pressing or releasing the
speed modifier blends between the gaits. Both follow actual movement distance.
Sustained backwards movement stays in Modern crouch while the stance is active.
Duke keeps facing where you look, so you can strafe and back up while aiming
and firing as usual;
held still he holds the original one-knee kneel, blending into the crouch walk
when you move and back to the kneel when you stop. He stops at walls and blocking
objects, and stands up when you release Ctrl where there is room. Standing walks and Modern crouch walks can leave ledges without jumping:
Duke falls when support ends, using the original gravity and landing. This includes
walking off dumpsters and crates; it does not let pushed objects cross their safe
edges. Released where there is no room to stand, he stays low and W/A/S/D keep
crouch walking (never the roll) until there is room, then he stands. **CROUCH:
ORIGINAL** keeps the original roll on crouch plus a direction. Vanilla always rolls.
With **CROUCH: MODERN**, holding Ctrl in the air starts crouch on a
soft touchdown: keep a direction held to crouch walk, or release the direction
to kneel. The flight pose blends into crouch without a recovery wait. Hard
landings still finish their recovery; pressing Ctrl only after landing keeps
the existing ground transition.
In first person, Modern crouching keeps the knife, axe, crossbow, pipe bomb,
Holy Hand Grenade and dynamite in their standing view positions while kneeling
and crouch walking; their throw and reload motions still play.
Modern crouching also works in shallow and mid-depth water while Duke is
walking on the bottom, with either SWIMMING setting. Hold Ctrl to kneel or
crouch walk; release to stand when there is room. Surface swimming and
underwater swimming keep their usual controls, including Ctrl to dive.
The gait is built from your disc when the game is built (no extra files to fetch).
Known limits: no footstep sounds while crouch walking, and diagonals use
the nearest of the four directions' animations. The accepted walk/run poses
were authored on Duke's skeleton after study of the supplied MGS references.
The earlier captured walk by abdlilah ben (CC BY 4.0, see README Credits) remains
in the authoring pipeline for comparison. Duke's retail fingers remain closed.
See [the accepted movement record](documentation/286-d08k13-d08k14-crouch-movement.md)
for the local animation preview and verification limits.

## First-person body presentation (D36J5)

Modernized first person shows legs during movement, jumps, climbing, swimming,
jetpack flight and interactions such as toilet use, without the torso. FIRST-PERSON LEGS in OPTIONS > ADVANCED defaults On. Off
restores the no-body view while keeping arms, weapons, held items and Q/door
kicks. Third person, reflected Duke and Vanilla retain their existing presentation.

D36H5 gives climbing, mantling and interaction gestures a dedicated first-person
arm pose. Hands stay at their original contact points while shoulders and
elbows are posed for the eye view, so shoulder ends continue out of frame.
D36H7 lets the hidden shoulder move modestly toward distant
contacts to reduce arm stretching. Hands stay on their targets; very distant
contacts can still require some extension.
The accepted traversal rig now also covers push/pull, use gestures and mirror
grooming, including toilets, vending machines, wheels, levers, cards, crystals
and statues. Weapon grips and controls stay the same. This presentation is
user-accepted; see [coverage, limits and the future-report guide](documentation/306-d36h5-interaction-arm-coverage.md).
If an arm looks wrong elsewhere, report the activity or current save slot.
The guide records how to reproduce, diagnose and extend the accepted solution.

For a one-launch developer comparison with the earlier visible-body policy:

```sh
DNTTK_FP_BODY=visible python3 recomp/tools/local/run.py
```

Only the exact value `visible` selects this full-body diagnostic comparison.
Omitted or empty uses the saved legs preference; `hidden` or unrecognized
nonempty values hide ordinary body parts. The comparison retains the
displaced-eye safety guard and does not save a setting. Normal launch:

```sh
python3 recomp/tools/local/run.py
```
