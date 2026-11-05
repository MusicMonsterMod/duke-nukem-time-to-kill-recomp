# D44 - Explosion rumble: EDuke32 earthquake and Time to Kill research

2026-11-05. Status: Done (research only, closed out by the user's standing
instruction for this job). Implementation: D44A (Todo). No game code, build or
framework change.

## Request

User (2026-11-05): in EDuke32, saves `rumbletest` (shoot the fire extinguisher
in front of Duke), `rumblewall` (left click detonates the placed explosive) and
`bigrumble` (shoot the gas canisters) show the rumble that big explosions give
Duke Nukem 3D. "I want to add in the ability for rumbles to happen in Time to
Kill." Starting points: UI slot 12, an explosive-vest pig ("this should cause a
rumble right when you shoot those guys"), and UI slot 10, rooms of TNT barrels
("if we could make rumbles happen for every single explosive barrel that would
be perfect ... it's pretty anticlimactic"). Then find other things that would
work with the rumble.

Follow-up mid-research (user): "can we try and make that camera shake really
look 1:1 like the one in eduke. or get bloody close to it". This is the D44A
acceptance target.

Evidence labels: **S** source read (EDuke32 r10669 source, `research/eduke32`),
**M** measured in a private run, **D** static disassembly of SLUS_005.83.

## EDuke32: how the rumble works

### The effect (S, M)

One global countdown, `g_earthquakeTime` (`global.h`). It is not per sector or
per player and has no distance: while it is above 0 every player's view shakes,
wherever they are on the map.

- Countdown: `G_MoveWorld` path, `game.cpp:7312`, minus 1 per game tic (30 Hz).
- Camera (`G_DrawRooms`, `game.cpp:1058`), only while the player is on the
  ground (`on_ground == 1`; jetpack, jumping and swimming are immune):

  ```
  camera z   += 256 - ((t & 1) << 9)          // +256 / -256, flips every tic
  camera ang += (2 - (t & 2)) << 2            // +8 for two tics, 0 for two
  ```

  256 Build z units are 16 xy units (1/38 of Duke's 38<<8 height). 8 angle
  units of 2048 are 1.41 degrees of yaw. So: a 15 Hz up/down bob of +-16 units
  and a 7.5 Hz sideways twitch between straight and 1.41 degrees right. Square
  waves, no easing, no random, no fade: full strength until the timer ends.
- Frame level: no sub-tic interpolation; frames within one tic are identical.
- Pad: `I_AddForceFeedback(t << 8, t << 8, t << 3)` each drawn frame (both
  motors, strength falling with the remaining time), when controller rumble is
  on.
- A new event sets the timer (16 or 48); it never adds. A chain of detonations
  keeps restarting it, which is what makes `bigrumble` one long quake.

### Why it feels like the ground shaking

User: "the illusion is so good that it doesnt look like a mere camera shake, it
really feels like the ground shakes". From the code:

- **It moves the eye, it barely turns it.** The main term is a vertical
  translation of the camera. The whole world goes up and down against the
  screen-space weapon and status bar, which do not move (inference: they are
  drawn after `G_DrawRooms` and the offset is applied only to the 3D camera).
  A rotation (TTK's native jitter, mostly roll) reads as "the camera wobbles"
  instead.
- **Square wave at 15 Hz.** A hard flip every tic, no easing, reads as
  vibration through the floor, not a sway.
- **No fade.** Full strength to the last tic, then it stops dead, like a
  physical jolt.
- **Only on the ground.** Jump and it stops, so it feels carried by the floor.
- **Everywhere at once.** It does not depend on distance, so a detonation feels
  like the building moving.
- The small 1.41 degree yaw twitch at half the rate breaks the symmetry so it
  is not a clean bob.

D44A should keep all of these. Translate the eye and keep the weapon and HUD
still. In third person Duke stands on the shaking ground, so he moves with
the world against the HUD.

### What starts it (S)

| Source | Timer | Code |
| --- | --- | --- |
| `DETONATE`: a SEENINE (explosive canister) or OOZFILTER goes off, including each link of a same-hitag chain | 16 (0.53 s) | `actors.cpp:2509` |
| A FIREEXT (fire extinguisher) with hitag > 0, shot | 16, via DETONATE | `actors.cpp:2402-2435` |
| A CRACK sprite with a hitag hit by FIREEXT, RPG, RADIUSEXPLOSION (pipe bomb and other radius damage), SEENINE or OOZFILTER | 16, via DETONATE | `actors.cpp:2362-2392` |
| HELECOPT / DUKECAR crash after 8 s | 16 | `actors.cpp:4471` |
| Sector effector SE 2 (Earthquake), started by a master switch: a switch, touch plate or activator with the same lotag (`G_OperateMasterSwitches`) | 48 at three pulses (every 32 tics over 96), plus the EARTHQUAKE sound and the sector floor tilting to its slope | `actors.cpp:6945-6975`, `2568` |
| CON `quake <tics>` | any, plus the EARTHQUAKE sound | `gameexec.cpp:5353` |

During any quake, SE 33 sprites spawn falling scrap at random (`actors.cpp:8587`).
DETONATE also fires the same-hitag SE 13 explosive sectors, SE 8, SE 18 and SE
21, so the rumble comes with the wall or floor blowing open.

Not a rumble in Duke3D: the plain EXPLODINGBARREL (gas can; GAME.CON
`actor EXPLODINGBARREL` has no `quake`), a FIREEXT without hitag, RPG and pipe
bomb blasts on their own. GAME.CON uses `quake` nowhere. The rumble belongs to
level-design detonations, which mappers place with hitags and switches.

### The three saves (M)

Private copy of the user's EDuke32 (`-usecwd`, copied config and saves, sound
off) on an offscreen X display. A test module logged per drawn frame
`cameraz - posz`, `cameraang - ang` and `on_ground` (EVENT_DISPLAYREST, game
tic counter in EVENT_GAME). Fire was the bound key.

| Save | What goes off | Measured |
| --- | --- | --- |
| `rumbletest` | The extinguisher at the end of the room (a linked FIREEXT) | Quake tics 108-123: exactly 16 tics. z +-256 every tic, angle 8, 8, 0, 0. |
| `rumblewall` | A remote pipe bomb at the cinema entrance crack (CRACK hit by RADIUSEXPLOSION) | 16 tics, 102-117, starting the tic after the detonator press |
| `bigrumble` | The canisters down the arcade corridor (SEENINE chain) | One quake from tic 111 to 173 (63 tics, 2.1 s); the phase resets at each link (tics 119, 129, 134, 143, 148, 157) |

The first frames after each shot also carry unrelated z offsets (fire and
explosion knock); the quake is the clean +-256 / 8 pattern.

Lab note: under Xvfb EDuke32 ran no game tics after a load until
`r_maxfps "60"` and `vid_vsync "0"` were set (Xvfb reports no refresh rate).
Polymost also needs a window manager (`metacity`) to draw at the window origin.

## Time to Kill today

### The native shake (D, M)

`0x80028a2c(camera, position, A, B)`: if the blast is within 0x4000 units of
the camera eye on every axis, falloff `0x800289ac` = 1 - 0.225 x distance /
4096 (0.1 at 16384), then camera `+0x48` = A x falloff (strength) and `+0x4a` =
B x falloff (time). It overwrites: a weak far blast replaces a strong near one.

`0x80028bdc` (camera update, from `0x80029270`) plays it: `+0x4a` falls by the
frame step `0x800d21fc` (300 a second), `+0x48` falls in proportion, so the
strength fades linearly to 0. Each game frame it builds a random rotation
(about strength/10 on two axes and up to `+0x48` on the third, 4096 a turn),
multiplies it into the camera matrix and rumbles the pad (`0x8001e97c`,
strength x 20 capped at 255). A random jitter, not Duke3D's square bob.

| Blast routine | A (strength) | B (time) | Who |
| --- | --- | --- | --- |
| `0x800702bc` | 160 | 450 (1.5 s) | rockets, grenades |
| `0x800707cc` | 160 | 450 | pipe bombs, Holy Hand Grenade |
| `0x80070f7c` | 160 | 450 | dynamite |
| `0x80063f0c` | 400 | 400 (1.33 s) | a character blown apart, including the vest pig |
| `0x80097090` | 130 | 64 (0.21 s) | TNT barrels |

Values from `0x800c3d04-0x800c3d12`.

### UI slot 12: the explosive-vest pig (M)

Runtime slot 11, private card copies, Modernized, third person, held fire.
The pig dies in a large fireball and the `0x80063f0c` shake starts at strength
and time 234 (Duke about 7500 units away, falloff 0.585): 0.78 s, fading
linearly. Sampled camera rotation steps 3-10 degrees on some game frames, 0 on
others. So there is a shake, but it is short, random and fades from the first
frame. It does not read as Duke3D's sustained rumble.

### UI slot 10: TNT barrels (M)

Runtime slot 9. Each barrel that goes off gives a `0x80097090` shake: strength
52-67 and time 25-32 at Duke's distance, 0.1 s each. The chain gives a few
blips over 1 s. This is why the barrels feel anticlimactic. Note 370 already
found that this routine plays no blast sound of its own.

## What D44A should build

Target (user): EDuke32's earthquake 1:1, or as close as the game allows.

1. **One global rumble timer**, Duke3D semantics: a source sets it (never adds),
   it falls at the game's 30 Hz tic, the whole level feels it, only while Duke
   is on the ground (also not swimming, on a ladder or jetpacking).
2. **The exact pattern**: per tic, camera height +-16 Duke3D units flipping each
   tic; yaw +1.41 degrees for two tics, 0 for two; full strength to the end; no
   interpolation between tics (redraws at 120/180 Hz hold the tic's offset).
   Scale the bob to TTK units by eye height: Duke3D's is 1/38 of Duke's height;
   calibrate against a side-by-side capture of the EDuke32 saves. Apply it as a
   presentation offset in the redraw camera (D10H frame replay), so it reaches
   every present and never feeds back into aim, movement or collision; in
   third person move the lens, in first person the eye.
3. **Pad**: keep the native `0x8001e97c` route, strength following the timer
   as `I_AddForceFeedback` does.
4. **Sources** (all Modernized; Vanilla unchanged):
   - TNT barrel (`0x80097090`): 16 tics each, chains restart it, like a SEENINE
     chain in `bigrumble`.
   - Explosive-vest pig and any character blown apart (`0x80063f0c`): 16 tics,
     maybe longer for the vest; decide with the user.
   - Candidates to try with the user: dynamite (`0x80070f7c`), pipe bombs and
     the Holy Hand Grenade (`0x800707cc`), rockets and grenades (`0x800702bc`).
     Duke3D does not quake for its own RPG or pipe bomb, so these could be
     distance gated or off.
   - Scripted level events (collapses, crashes, large set pieces), Duke3D's SE 2
     analogue, after a survey of the level overlays' event code.
   - A console command, like CON `quake <tics>`, for testing.
5. **The native jitter**: in Modernized replace it for these sources, or keep
   it underneath at reduced strength. Choose by comparing captures with the
   user.
6. **Option**: a Rumble on/off in the presentation settings (and the
   controller rumble option for the pad).

Tests: the slot 10 barrel chain and slot 12 pig at 60/120/180 Hz;
frame-by-frame comparison with the EDuke32 measurements above (tic count,
phase, amplitude); airborne and water immunity; pause; savestate across a
quake; third and first person; Vanilla byte-identical behaviour.

## Files

- Probe: `recomp/analysis/d44-rumble/probe.py` (local; from the D39F probe;
  case `shoot`: private card copy, load a slot, hold fire, sample camera
  `+0x48/+0x4a` and the rotation each frame). Player saves were hash-checked
  unchanged after every run.
- EDuke32 lab: private copy in the job's temporary folder, module `quake.con`
  (appendix).

### Appendix: EDuke32 probe module

```
gamevar qt_cz 0 0
gamevar qt_pz 0 0
gamevar qt_ca 0 0
gamevar qt_pa 0 0
gamevar qt_og 0 0
gamevar qt_t 0 0
definequote 9002 FRM %d DZ %d DA %d OG %d
definequote 9003 x
onevent EVENT_GAME
  ifactor APLAYER addvar qt_t 1
endevent
onevent EVENT_DISPLAYREST
  getplayer[THISACTOR].posz qt_pz
  getplayer[THISACTOR].ang qt_pa
  getplayer[THISACTOR].on_ground qt_og
  setvarvar qt_cz cameraz
  setvarvar qt_ca cameraang
  subvarvar qt_cz qt_pz
  subvarvar qt_ca qt_pa
  qsprintf 9003 9002 qt_t qt_cz qt_ca qt_og
  echo 9003
endevent
```

Run: `./eduke32 -usecwd -nosetup -ns -mx quake.con` with `r_maxfps "60"` and
`vid_vsync "0"` in `settings.cfg`.
