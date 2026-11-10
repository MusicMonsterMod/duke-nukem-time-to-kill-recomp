# Time to Kill jobs - Lighting

Job descriptions and work logs for D41 and its children, D54-D57. Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D41 - Foundational lighting system (pre-job)

**Status: Done (user-accepted; closeout authorized 2026-11-08; case study published: https://claude.ai/artifact/M1MNyrZqtgzEFc7s72jSXh, [note 413](documentation/413-d41-lighting-case-study.md); child jobs D41A-D41H added; the user authorized autonomous work on the child jobs, and D41A is implemented). Dependencies: none. Lighting itself is implemented under the child jobs, not under this ID.**
User, 2026-11-03: "i think we make a foundational lighting system for this
game. to bring it in line with other source ports like gzdoom, uzdoom,
eduke32, ironwail and so on. Real, genuine lighting"; "that job will actually
spawn a bunch of other jobs when i want to start this. It should also stand
up an artifact which can have screenshots and case studies of how lighting
has been achieved elsewhere, and to see what our options are in terms of the
implementation of real lighting, in the way that these retro engines have
acomplished it".

When selected:
1. **Case-study artifact** (published page, updated as the study grows):
   screenshots and how each port lights its world. GZDoom / UZDoom and
   EDuke32 still light from the maps' baked sector light, but with dynamic
   lights, light falloff and modern renderers on top (EDuke32: Polymost
   shading and visibility, Polymer per-pixel lights, the explosion world
   light-up measured in D39F); Ironwail / Quake: lightmaps, dynamic lights and
   lightstyles, a polygon engine closer to TTK's "n-space" engine (the user's
   name for it). Other relevant ports as found. Captures from the user's own
   installs on private copies where possible.
2. **TTK today:** survey the engine's own lighting tricks (vertex colours,
   light-style effects, explosion and muzzle effects, fog/depth cue) and what
   Polymorph already knows of each draw (geometry, normals, world positions).
3. **Options:** baked-from-original versus dynamic, per-vertex versus
   per-pixel, lightmaps, shadowing, cost at 180 Hz with render replay, and
   how Vanilla and Classic stay untouched.
4. **Child jobs:** a proposed breakdown (foundation, dynamic lights,
   explosion and weapon lights replacing D39F's removed flashes, level
   lighting) added to the board as Todo for the user to select.

Inputs: [note 370](documentation/370-d39-screen-flash-framework.md) D39F
"Blast research" (EDuke32 explosion light-up measurements, TTK explosion code
map); the D39 present tint. **Acceptance:** the artifact is published and
reviewed with the user, and the child jobs are on the board; no lighting code
under D41 itself.

### D41A - Light foundation

**Status: Done (user-accepted after playtest; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 5). Dependencies: D41.**
Polymorph gives every world, prop and actor corner its world position and the
OpenGL textured shader adds the frame's lights per pixel to the original vertex
colour (falloff, facing, soft shoulder). Sources: blasts (the D39F blast sound
calls, the TNT barrel's shake request) and Duke's muzzle flash; console
`light`. Per-image light lists travel to redraw workers in RenderState and to
the GPU in the replay stream. OPTIONS > DISPLAY > DYNAMIC LIGHTS (schema 71,
`--dynamic-lights`). Vanilla and Classic unchanged. Acceptance: the user
playtests blasts and gunfire in several levels at the player's frame rate,
judges look and strength, and confirms no slowdown and the option's off state.

### D41B - Weapon and projectile lights

**Status: Done (user-accepted after playtest: "the lighting looks absolutely incredible!! the brightness of the lighting is so beautiful but not intrusive. i love how the light follows the projectiles"; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) sections 7 and 10). Dependencies: D41A.**
Rounds carry light in flight (RPG 8, freezer 13, energy 15); every round's
launch (`0x800718ac`, Duke and enemies) lights a muzzle flash at its launch
point in its type's colour; wall hits (`0x8006f1b8`) light a short spark; the
energy weapon and flamethrower (no rounds) light from Duke's hand flash;
throws and bolts make no flash; blasts sized per source (barrel, dynamite,
pipe bomb or grenade, rocket, gib). Burning enemies, fire and enemy weapon
types other than the pig cop's moved to D41I.

### D41C - Duke and first person in the light

**Status: Done (user-accepted after playtest; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 8). Dependencies: D41A.** Duke's own model is drawn by the original
renderer (D12) and the first-person layer is painter-only, so neither takes
dynamic light or ambient dimming (in the D41A dimmed preview Duke stays
bright). Give Duke's model, legs, arms, weapon and held items world positions
(or a lighting path of their own) in both views. Read the D36 audit (note 257)
and D36H5 notes first; preserve their ownership boundaries. Acceptance:
matched shots of Duke beside a blast and under a test light, first and third
person, no presentation regressions.

### D41D - Level light sources

**Status: Done (user-accepted 2026-11-09: "excellent lock it in and close it out"; per-level lamp files, `lamp` editor, painted-light scan, level 0 first set, signs keep their look under light; [note 428](documentation/428-d41d-level-lights.md)). The remaining parts are D41D1. Dependencies: D41A.** Lights the levels themselves own: survey
emitters per level (neon, lamps, braziers, fires, monitors) by texture page and
CLUT and by peaks in the painted vertex light; decode the colour-index and
animated light tables (`0x800c37e4`, `ctx+0x68`) into light styles that drive
real lights; a data file of lights per level (like Duke-RT `LIGHTOVR` or
`.rtlights`) written by us, and a console light editor to place, colour and
save lights. Acceptance: a first set of levels with their lamps and signs
casting real light, user review per level.

### D41D1 - Level light sources, second part

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; `lamp signs`, `wave` style, room gate, neon signs take no light on their letters, first sets for all 21 selectable levels; [note 429](documentation/429-d41d1-signs-waves-levels.md)). Dependencies: D41D.** User (2026-11-09): signs emitting real
light, "i like that idea". Find emitters by texture page and CLUT (EXIT signs,
neon, monitors, lit panels) and give each a lamp in its own colour, the sign
itself kept crisp by the D41D shader rule; decode the animated light tables
(`ctx+0x68`) into lamp styles that follow the original flicker; first lamp
sets for more levels with the `lamp` editor. Acceptance: signs visibly light
their surroundings in level 0, then each new level reviewed by the user.

### D41E - Relit levels

**Status: Done (2026-11-10; [note 441](documentation/441-d41e-relit-levels.md)). Dependencies: D41C, D41D, D41F (all Done).** A LIGHTING option: Original
(no dynamic lights), Enhanced (D41A-D41D on the painted light) and Relit (the
painted light dimmed per level and room so placed lights, shadows and blasts
carry the scene). Readability checks level by level (remasters that relit
vertex-lit games were criticised for areas too dark to read). Acceptance: the
user plays relit levels and approves each one.

### D41F - Shadows

**Status: Done (2026-11-10; [note 439](documentation/439-d41f-shadows.md)). Selected as the D41E prerequisite. Dependencies: D41A.** Research spike, then shadow maps for
chosen lights (blasts, placed lamps) rendered from the host's world-space room
meshes, so light stops at walls; PCF filtering; cost limits at 180 Hz. GZDoom's
1D shadow maps do not fit TTK's 3D rooms. Acceptance: matched shots of a light
behind a wall and a lamp throwing shadows, measured frame cost.

### D41G - Light scale and cost

**Status: Todo. Dependencies: D41A.** Clustered light lists (Ironwail) when
lights exceed a few dozen; lighting for untextured polygons (G3/F3), a policy
for sprites and translucent faces (emissive or lit); measured cost per frame
at 120/180/240 Hz on busy levels. Acceptance: measured budgets and no
regressions on the D17 performance routes.

### D41H - Smooth light on models

**Status: Done (user-accepted after playtest: "it looks good still! ... it looks great"; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 9). Dependencies: D41C.** Per-vertex normals for actor and prop
meshes (averaged per mesh, cached by mesh) so light rounds over bodies instead
of showing flat facets; optional later: normal or specular detail. Acceptance:
matched shots of an enemy under a test light, faceted and smooth.

Work log 2026-11-08: `near_clip.cpp` `smooth_model_normals` gives every lit
actor and prop corner (Duke too) the area-weighted mean of the mesh's face
normals at that corner, within a 65 degree crease, welded across texture
seams, outward (front faces face the eye), recomputed per draw rather than
cached so animated meshes stay correct (about 4.6 us per mesh). The GL
shader interpolates them; model light is one-sided with a slight wrap,
double-sided faces and the world keep D41A's two-sided facing. Console
`light smooth on|off|default`. Matched offscreen shots (private savestate
reloaded faceted and smooth): a figure, a pig cop, Duke, the street pole and
the pistol, `research/before-after/d41h-smooth-light/`. 180 Hz 0 missed
redraws. Remaining: user playtest of the look in both views; normal or
specular detail stays optional later work.

### D41I - Fire and enemy weapon lights

**Status: Done (user-accepted after playtest: "the lighting looks spectacular"; burned-away tree fix verified in the lab; closeout authorized 2026-11-08; [note 413](documentation/413-d41-lighting-case-study.md) section 11). Dependencies: D41B.**
The D41B scope not done: burning enemies and fire (find the burn and flame
effect sources and give them flickering fire light), and per-type values for
enemy weapons other than the pig cop's rounds (type 2): turrets, flyers and
bosses currently take the default warm flash and spark (note 413 section 10
table). Probe their projectile types on the levels that place them (D22I
census, note 412) and set colours and sizes. Acceptance: user playtest.

Work log 2026-11-08: burning objects light while their flag word has bit
`0x800` (the bit the actor draw checks before adding flame sprites to the
joints; scanned each tick over the placement rows and Duke, nearest five);
the flamethrowers' flame records (pool types 14 and 26, flag bit 0) light
along the stream; enemy weapon types from a lab census get their own
values (warm guns for types 3, 5, 29, 32, 34 and 35, violet for the level 3
gunship's bolts 31, RPG-like rockets 24, 28 and 33). Console `light fire
on|off`. Matched stills: a burning pig cop lights the street around it;
180 Hz 0 missed redraws. Flyers and types 847, 848 and 901 did not fire in
the census and take the default warm flash. Needs the user's playtest.

Playtest round 1 (user, 2026-11-08): "When I burn an enemy the lighting
effects are just beautiful"; the gunship's projectile lights "are very well
done indeed". Bug: in UI slot 8, burning the tree in front of Duke left its
light on after the tree had burned away. Cause (private slot 8 copy): the
tree's flag word goes 0x802 while it burns and 0x804 when it is gone,
keeping 0x800 (two other props read 0x804 from the start); a burning pig
cop reads 0x803, then 0x804 when its burn ends. Fix: an object burns while
bit 0x800 is set and bit 0x4 is clear. Retest on the copy: the tree lights
only while it burns and the wall stays dark after; a burning pig cop still
lights for its whole burn (about 6 s). The user's separate report of the
mission item text overlap is D08A39.

### D41J - Upgraded weapon light colours (HiTemp Flamethrower, Laser Gatling)

**Status: Done (user-accepted 2026-11-09: "this current job at hand is fully accepted"; closeout authorized; [note 413](documentation/413-d41-lighting-case-study.md) section 12; user report 2026-11-09, D10H9 acceptance: "the upgraded flamethrower glows blue, but our lighting doesnt change from the regular flamethrower"). Dependencies: D41I.**
The upgraded (hi-temp) flamethrower's flames are drawn blue, but the D41
dynamic lights from its stream, hand flash and flames in flight use the
regular flamethrower's warm, flickering fire colour. Lead:
`recomp/src/ttk/dynamic_lights.inc` already names the hi-temp flame records
(pool type 26, besides 14) and the weapon's hand flash; find where the
upgraded weapon (slot 27) and its type 26 records take the warm values, and
give them a blue that matches the drawn flame (sample it from captures), with
the same flicker, size and cost. Burning enemies set alight by the upgraded
flamethrower are included if their drawn flames are blue too. Regular
flamethrower, other lights and Vanilla unchanged. Test: matched stills of
both flamethrowers firing at a wall and burning an enemy, first and third
person, 180 Hz (spawn the upgrade with the console). Acceptance: user
playtest.

Scope added (user, 2026-11-09): "also check out the upgraded laser gatling
gun, make sure the colors on the laser lights are right too". The Laser
Gatling (slot 28, the Gatling's upgrade) gets the same check: its muzzle
flash, rounds in flight and impact lights match the colour of its drawn
laser shots; the regular Gatling is unchanged.

**Work log (2026-11-09, Needs playtest).** `dynamic_lights.inc`. Owners:
the held byte stays 7/9 and `0x8003df40` resolves the upgrade bit to records
28/27; the hand and flame record draws pick blue sprites for record 27 and
pool type 26; the Laser Gatling launches type 27, which took the default warm
flash. Now the HiTemp hand flash and flames in flight light blue-violet
(sampled from the drawn flame), and type 27 has a red-pink muzzle, spark and
glow in flight (sampled from the bolts), same sizes, flicker and cost. Burns
lit by the HiTemp stay warm: same flag word (`0x803`) and orange flame
sprites as the regular flamethrower. Lab stills (60 Hz third person, 180 Hz
first person, before/after): the HiTemp lights the street blue-violet and the
Laser Gatling lights pink; regular weapons and burns unchanged.
`ttk-input-test`, `ttk-aim-test` pass. [Note 413](documentation/413-d41-lighting-case-study.md)
section 12. Needs the user's playtest (`dnupgrade` upgrades both).

### D41J1 - Laser Gatling light reads red

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; user, 2026-11-09: "the laser gatling gun i think reads more white in hts projectile lighting. the same way we gave the energy weapon green and the freeze thrower blue light, can we give more of a red tone to the laser gatling gun's lighting"). Dependencies: D41J.**
D41J gave the Laser Gatling's projectile type 27 a red-pink muzzle flash,
wall spark and glow in flight (`recomp/src/ttk/dynamic_lights.inc`
`shot_lights`), sampled from the drawn bolts. In play the light reads
whitish: the pink keeps a strong blue part, the street's blue-grey base
adds to it, and the fast fire stacks lights into the shader's soft shoulder,
where red tops out first and green and blue catch up. Give type 27 a clear
red light that reads as red the way the freezer reads blue and the energy
weapon green, same sizes, lives and cost. Regular Gatling, other weapons and
Vanilla unchanged. Test: matched before/after stills of the Laser Gatling
firing at a wall, third person 60 Hz and first person 180 Hz (`dnupgrade`).
Acceptance: user playtest.

**Work log (2026-11-09, Needs playtest).** Cause, measured: a single
D41J light was red-pink as intended (console test light on the street: a
pure red light adds only red), but the Laser Gatling fires fast, so two or
three muzzle flashes and several bolt glows overlap near the gun. The
Polymorph light sum's soft shoulder tops red out near 1.1 while green and
blue keep climbing, so the D41J colour (blue 0.52, green 0.26) stacked toward
pale pink-white on the floor in front of Duke; the blue-grey street pushed
it toward lavender. Fix: `recomp/src/ttk/dynamic_lights.inc` `shot_lights`
type 27 is near-pure red: muzzle 1.25 0.05 0.02, spark 1.30 0.07 0.03, in
flight 1.05 0.04 0.02; sizes, lives and cost unchanged. A first try that kept
some green and blue (1.30 0.12-0.14 0.05-0.08) still stacked to salmon. Lab
`recomp/analysis/d41j/lab.py shots` (`UP=1 WANT=7`, `NSHOT` burst frames,
`LIGHTSOFF=1` baseline) and `probe` (test-light colours), first person
180 Hz and third person 60 Hz: the floor, pole and wall in front of the gun
now light deep red; the game's own drawn muzzle sprite is unchanged.
Before/after in `research/before-after/d41j1/` (local). Regular Gatling
(type 7), other weapons and Vanilla untouched. `ttk-input-test` passes;
`ttk-aim-test` fails its line 100 assertion on the committed source as well
(not from this job). Executable
`1c6addcdf5ba8f47b84704d7d6ce59f9bd6cd39e88c3968eb0d1e582c031b153`.
[Note 413](documentation/413-d41-lighting-case-study.md) section 13. Needs
the user's playtest (`dnupgrade`).

### D41K - Props respond to dynamic light like walls do

**Status: Done (user-accepted 2026-11-09: "lock it in. close it out"; [note 434](documentation/434-d41k-props-in-light.md)). Dependencies: D41A, D41H (Done).**
Seen in D53 (note 432, revisions 2 and 3): in Polymorph a dynamic light
barely brightens props. A light over the LEVEL00 control console at
strength 25, radius 600 only greyed the desk top, while the same light
setting lights walls clearly (the TV screen light, the power sign halo).
Hypothesis to confirm, not a finding: props are already bright from their
own vertex colour, and the shader's soft shoulder (D41A) leaves little room
to add light; or the prop path weighs the light differently.

Plan: measure one light at fixed distances on a wall and on several props
(console, crates, signs) and characters, logging the shader inputs; find
where the response is lost; fix it renderer-wide (Polymorph only), so blasts,
muzzle flashes, lamps, signs and the D53 lights all light props as they light
walls. Check characters and Duke (D41C), which share the model path, and
Classic/Vanilla unchanged. Acceptance: matched before/after shots of props
under a blast and a lamp, user playtest.

**Work log (2026-11-09, Needs playtest).** Props were lit (the subway
train's polygons all carry the lit flag, positions and normals; white light
brightens it steadily), but three things held them back. (1) The D41A soft
shoulder worked per channel on painted colour plus light, so a strong
coloured light washed to grey (green strength 25 on the train: (96, 122, 98);
the D53 "greyed desk top"). (2) It started at the neutral 0.5 where prop
corners sit (128), so prop light was compressed from its first unit while
dark walls (about 0.28) took it linearly, and painted colours above 0.5
dimmed whenever any light was in the image. Club, lamps only, over lights
off: table +9, floor +36. (3) The D41D1 neon rule listed whole meshes, so a
control panel with a small screen (type 478), the bar's shelves (2, 4),
light boxes (79) and red lamps (170) took no light at all. Fix: the shader
(`lit_col`, framework patch) keeps the painted colour and soft-limits only
the added light, linear to 0.35 then toward +1.0, on its brightest channel
(hue kept); the neon scan lists face records (`near_clip_set_glow_faces`,
`Poly::glow`) so only neon faces stay unlit (level 0: 234 -> 68 faces).
After: green strength 25 on the train (47, 103, 48); club lamps on props
table +24, lamp shade +26, floor +42; a real RPG blast looks the same on
walls (frame means equal from the frame after the peak). 180 Hz: 1833
redraws, 0 misses. `ttk-near-test`, `ttk-flash-test`, `ttk-live-env-test`
PASS; framework patch exported and verified against the pin. Developer:
`DNTTK_LIGHT_PROBE=1`, console `light at x y z r g b [radius]`. Stills
`research/before-after/D41K/`. Not measured in place: the D53 console (no
save near it). Needs the user's playtest (lamps, blasts, coloured lights on
props).

### D54 - Broken lights go out

**Status: Done (accepted 2026-11-10; [note 437](documentation/437-d54-broken-lights.md)).**

User, UI slot 6 (level 5 street): "In front of you is a barrel and two
traffic lights ... destroy all of them and you'll see that the lighting
effects remain ... when Breaking an object that Emits light then the light
should also get destroyed ... that's actually game wide". Scope:

- Find how TTK breaks a prop (object freed, mesh or kind swapped, a broken
  model) and every source of our light tied to a prop: D41D1 sign lamps
  (level files, `sign type N` notes), hand-placed lamps on a prop, D41L
  prop lights by model, D53 screen lights, and any other glow drawn for a
  prop.
- Bind each such light to its prop at run time; when the prop breaks, the
  light goes out (a short dying flicker, not a pop) and stays out, also
  after a savestate load in the broken state.
- Game-wide rule, not a list per level. Painted vertex light (the original's
  baked light) is reported, not changed, unless the investigation shows a
  safe way.
- Modernized Polymorph only; Vanilla and Classic unchanged.

Acceptance: in UI slot 6 the barrel and both traffic lights lose their light
when destroyed; other light-emitting props checked on other levels; user
playtest.

**Work log (2026-11-10, Needs playtest).** Investigation in a private copy of
UI slot 6 (file slot 5): the traffic lights (type 217) and the fire barrel
(type 220) broken by gunfire. At the breaking hit the prop's health (`+0x32`)
reaches 0 and bit 2 of `+0x00` is set; shortly after, the object is unlinked
from its room lists; type, place and mesh stay in memory. Our lights tied to
props: D41D1 sign lamps (fixed places in the level files, lit for good), D41L
woodburner and chandelier lights and D53 screen and power-sign lights (rescan
every 300 frames). Built: `recomp/src/ttk/prop_state.h`, a shared snapshot of
standing props (linked, kind 1, bit 2 clear, same type and place) with a room
table fingerprint so a level change does not read as every prop breaking.
Lamps take `prop TYPE x y z` entries in their notes; `lamp signs` writes them
and `lamp bind all` added them to every existing file (922 sign lamps, 21
levels, none unbound, lamp values unchanged). A light whose props all break
sputters out over 0.4 s and stays dark; loads take the state as found (no
sputter). D41L and D53 emitters check their prop each image and skip broken
props when scanning, and rescan on a savestate load; the D53 bounds writer
skips broken or unlinked monitors. Console: `lamp bind`, `unbind`, `bind
all`, `breaks off|on`. Evidence: the three lamps go out at their breaks
(log and matched shots `research/before-after/D54/`, local); a lab save made
after the breaks loads dark and the slot loads lit; flag set by hand on a
level 1 woodburner and a level 0 monitor puts out the fire light, the screen
light and the monitor's sign lamp; no false outs at level changes; build OK,
`ttk-input-test` and `ttk-near-test` pass. Remaining: user playtest in UI slot
6; the original baked (painted) light around a broken emitter stays (the
orange on the wall over the barrel), a possible follow-up.

### D55 - Traffic lights light the street in their own colour and flash

**Status: Done (accepted 2026-11-10; [note 438](documentation/438-d55-traffic-lights.md)).** Dependencies: D41D1, D54.

User, UI slot 6 (level 5 street), after D54: "I'd like traffic lights to all
be emitters, but they need to be synced to the color and the flash.
currently we have a flicker effect that is similar to the fire flicker.
essentially in the future we could put down a custom traffic light somewhere
and our colors would be correct". Scope:

- Find how TTK shows a traffic light's state (animated texture frames or
  another mechanism) and read it live.
- Every traffic light (each type, every level) gives a light whose colour and
  on/off follow the light the game is showing, frame for frame, instead of a
  fixed amber with a candle flicker.
- Driven by the prop, not by a list of places: a traffic light placed or
  spawned anywhere gets the right colours. Lights still go out when the prop
  breaks (D54).
- Modernized Polymorph with DYNAMIC LIGHTS only; Vanilla and Classic
  unchanged.

Acceptance: in UI slot 6 the street lights change colour and flash with the
traffic lights; matched shots of each state; user playtest.

**Work log (2026-11-10, Needs playtest).** TTK flashes its traffic lights by
texture animation: each model (217 on the kerb, 216 over the road) has one
animated face; its mesh's animation table (`+0x0c`) points at a running
animation record that the game copies each frame into. The script (record
`+28`) gives 2 frames of 600 ms: 217 lit amber / dark, 216 dark / lit red,
in step, so amber and red blink in turn. New lamp style `signal`
(`level_lights.inc`): each game tick the lamp reads its standing prop's
current frame and takes that frame's lit colour and its brightness against
the animation's brightest frame. The sign scan gives `signal` by itself to
any animated sign whose animation runs 100 ms frames or slower and has a
frame at most a tenth as bright as its brightest (flames run 40-66 ms frames
and keep `candle`), scores animated faces on their brightest frame (it used
the frame showing and, wrongly, the animation table of the last object
drawn, which made the red lights amber), takes flashing signs at half the
glow, and gives signal lamps a radius of at least 3000 so the over-road
lights reach the street. Level files: level 5's eight traffic light lamps
restyled (216 now red); levels 0 and 9 gain their eight (none before);
level 29's 16 pulsing neon lamps (1014, 1015, 142 ms frames) go from
`flicker` to `signal` at their old places. Evidence: signal trace (amber on
for 18 ticks, red on for the next 18), matched before/after shots
`research/before-after/D55/` (local), the traffic light's flag set by hand
puts its signal lamp out (D54), build OK, `ttk-input-test` and
`ttk-near-test` pass. Remaining: user playtest in UI slot 6; level 0, 9 and
29 lamps checked by scan and trace, not looked at in play.

### D56 - Necro blast and atomic health give light

**Status: Done (user-accepted 2026-11-10; [note 442](documentation/442-d56-necro-atomic-lights.md)).** Dependencies: D41B, D41L, D54.

User, 2026-11-10: "I am starting to feel that a couple of the in-game
objects could be light emitters too so if you load up my new slot 8, can we
make the necros blast emit. can we also make atomic health emit light too.
investigate these 2, stand up a ticket and work on them." Scope:

- The Necro's blast lights the world in its own colour while it flies, at
  the Necro when fired and where it hits, every Necro, every level.
- The atomic health pickup gives light, every copy in every level, and stops
  when it is picked up.
- Modernized Polymorph with LIGHTING on; Vanilla and Classic unchanged.

Acceptance: in UI slot 8 the blasts light the mine corridor as they come;
an atomic health glows; user playtest.

**Work log (2026-11-10, Needs playtest).** Lab `recomp/analysis/d56/`
(local), private copies of UI slots 8 and 6 (file slots 7 and 5). Necro
blast: Necros (type 50) launch projectile type 20 through the launcher
`0x800718ac` (traced: `launch enemy ... type 20`); in flight the pool record
reads flag `1` (bit 0), not the `3` of rounds, so the D41B flight lights
skipped it, and type 20 had no entry (default warm flash and spark). New
`ShotLight` field `bolt`: lit in flight on flag bit 0, with a small throb,
and a burst where the record vanishes unless a wall hit sparked there (hits
on characters skip `0x8006f1b8`; per tick, reset on level change and
savestate load). Type 20: violet launch flash (0.80 0.36 1.25, 2800, scaled
0.8 for enemies), violet flight light (0.75 0.34 1.20, 2600) and burst (0.95
0.45 1.35, 3000, 5 ticks); D39E's purple screen flash is the colour
reference. Atomic health: a pickup object (kind `+0x14` = 4) of type 581 in
the room lists. `prop_lights.cpp` scans them every 10 frames into the D41L
emitter list (room gate, budget, shadows) with a cool cyan-blue light (0.42
0.82 1.30, strength 0.85, radius 1700, 260 above its foot), a slow 1.4 s
throb, following the object's live position; a pickup no longer linked
(taken) gives no light at once. Console: `light bolts off|on`, `lamp pickups
off|on`; tuning `DNTTK_PROP_LIGHT_ATOMIC`. Evidence: slot 8 timed shots, the
left wall's mean colour from 24/21/18 (no blast near) to 53/33/53 as a blast
reaches Duke, bursts logged at the hits on Duke; matched off/on shots of
the corridor and of three spawned atoms on the level 5 street (cyan-blue pool
below them); the spawned atom picked up went dark; build OK, `ttk-input-test`
and `ttk-near-test` pass. Not checked: the player's GPU at 180 Hz, a single
atom's strength in play (the shot had three stacked), Vanilla launched (the
code is gated on Modernized and LIGHTING).

### D57 - Light emitter review: objects and items that should give light

**Status: Todo (backlog; user 2026-11-10: "do not work on this now").** Dependencies: D56.

User, 2026-11-10, after asking for D56: "there are many more, so we can
stand up a backlog job for a review of objects and game items that should be
light emitters. The first thing we should do in that backlog job is to make
an artifact of a list of items that should be emitters, ideally with
images." Scope:

- First step: survey every level's pickups (items, weapons, ammo, keys,
  power-ups), props, enemy projectiles and effects for things that look like
  they glow but give no light (glowing sprites, lit faces, energy effects),
  skipping what already gives light (D41B/D41I/D41J shots and fire, D41D/
  D41D1 lamps and signs, D41L woodburners and chandeliers, D53 screens, D55
  traffic lights, D56 Necro blast and atomic health).
- Publish the list as an artifact: per candidate an in-game image (offscreen
  lab shot, spawned where possible), object type or projectile type, levels
  seen, a proposed light colour and behaviour (steady, throb, flicker), and a
  priority. The user picks from it.
- Then: implementation in child jobs on the D56 framework (pickup lights by
  type in `prop_lights.cpp`, bolt lights in `dynamic_lights.inc`), Modernized
  Polymorph only; Vanilla and Classic unchanged.

Acceptance (first step): the published artifact, reviewed by the user.
