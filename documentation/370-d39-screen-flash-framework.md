# D39 - Screen flash framework (Duke3D `palfrom`-style colour flashes)

Job: [D39](../MODERNIZATION_JOBS.md) and D39A-D39G. Status: D39A accepted
2026-11-03 (spec table below, locked); no code yet. Follows D08G7 ([note 369](369-d08g7-kroz-feel.md)),
accepted as preliminary.

## Why

The user wants to own coloured screen flashes the way EDuke32 does: hit,
damage, pickup, explosion (white on a big blast, as in their own CON mods)
and the god-mode flash. D08G7 showed the limit of drawing them as PS1 GPU
commands: the GPU has four fixed blend modes (B/2+F/2, B+F, B-F, B+F/4), so a
fade toward a colour by a variable amount can only be approximated, and its
level stepped at the game's 30 Hz (20 Hz on overrun) tick, which staggers on
a 180 Hz display (user, 2026-11-03: "the fade isnt smooth enough, it kind of
staggers"). The renderer is ours (Polymorph, `recomp/psxrecomp`, tracked in
`recomp/patches/time-to-kill-accepted-source.patch`); the limit was the
chosen path, not the stack.

## Duke3D / EDuke32 behaviour (the spec)

`palfrom <intensity> [red] [green] [blue]` (EDuke32 wiki "Palfrom",
"Pals_time"): intensity 0-63 (63 fully opaque; above 63 prolongs full
opacity; the wiki allows up to 255), colour components 0-63 (6-bit palette units), omitted components
0. It sets the player's `pals` (colour) and `pals_time` (intensity); a new
palfrom replaces the current flash.

Measured from the user's EDuke32 recording (D08G7, three god-off flashes;
mean red dominance per 60 fps frame):

- weight = intensity / 64 toward the colour (x4 to 8-bit): the god-off
  flash is intensity 32 toward red 32 (G, B x0.47; R x0.47 + 63 in video
  levels, i.e. half way to red ~128);
- onset instant; about 2 tics at the peak (D39A: capture cadence, there is
  no hold); then intensity falls by 1 a
  30 Hz tic, a straight line, to 0 (32 tics, about 1.1 s);
- the whole frame is tinted: world, weapon, status bar and quote text.

## D39A - measured spec (EDuke32 r10669, 2026-11-03)

Clean room: behaviour only, no engine source read. Setup: a private copy of
the user's EDuke32 (`-usecwd`, copied config and `kroztest` save, the user's
GRP read in place, sound off) on an offscreen X display, 640x480, 32-bit
Polymost (the user's renderer: `Polymer = 0`, `ScreenBPP = 32`), capped at
60 fps and captured losslessly at 60 fps. A test CON module (`-mx`, appendix)
issued scripted `palfrom` calls and logged `pals_time` once per game tic
(`EVENT_GAME`) and once per drawn frame (`EVENT_DISPLAYREST`). The god-off
flash was driven with the real `dnkroz` cheat. Frames were fitted per channel
against an unflashed frame of the same static view (least squares over every
pixel).

| Property | Measured | Evidence |
| --- | --- | --- |
| Blend | `out = in * (1 - w) + 4 * C * w` per channel, in display (8-bit sRGB) values, no gamma or palette step | Six steady holds; per-channel linear fits, max error 0.4-1.5 levels over the whole frame |
| Weight | `w = min(I, 63) / 64`; at 63 the frame is 1/64 original (never fully opaque) | Fitted `1 - w`: I 8 0.876, 16 0.749, 32 0.499, 48 0.248, 63 0.016 |
| Colour scale | 8-bit colour = 4 x 6-bit component (63 -> 252, 32 -> 128, 16 -> 64); omitted components 0 | Fitted offsets / w: 251.9, 252.6, 126.7, 62.3 |
| Onset | Instant: the first frame drawn after the tic that called `palfrom` shows I | Frame log: `pals_time` 32 on the first frame of tic 90; video jumps 0 -> 63.2 in one frame |
| Fade | `pals_time` falls by exactly 1 a game tic (30 Hz) to 0; a flash of I lasts I tics (32 -> 1.07 s) | Every scripted flash, e.g. 48 -> 1 over tics 270-317 |
| Sub-tic | None: frames within a tic are identical (60 fps pairs); the drawn level is the integer `pals_time` | Video 60 fps: measured level steps 62.2, 62.2, 61.2, 61.2 ... |
| Peak hold | No hold: the peak shows for one tic like every other level | Tic and frame logs, CON and `dnkroz`; the "about 2 tics" read from the user's 60 fps recording is capture cadence |
| Above 63 | `pals_time` stores the value (100 accepted) and counts down from it; drawn weight clamps at 63, so I > 63 holds full strength for I - 63 tics | `palfrom 100 0 0 63`: 38 tic-steps at 63.3, then the normal fade |
| New flash | Always replaces intensity and colour, weaker or stronger | 48 red then 16 green 5 tics later: 44 -> 16, green; 16 then 48: 15 -> 48 |
| Same tic | The last call wins | `palfrom 48 63 0 0` then `palfrom 16 0 63 0` in one tic: 16, green |
| Repeated each tic | Holds the level (re-set each tic before it is drawn) | Six 60-tic holds read constant |
| God mode off | Exactly `palfrom 32 32 0 0` (the engine path, not a CON call) | `dnkroz` x6: `pals_time` 32, 31, ... 1; onset frame I 32.6, C (31.6, 0.1, 0.1) |
| Coverage | The whole presented frame: world, weapon, status bar, quotes, the in-game menu and the OSD console | Whole-frame fits; screenshots with the menu and console open during a hold |
| Pause | The flash is frozen while the game is paused (menu open), and still drawn | Menu screenshot at `pals_time` 32, unchanged until resume |
| Console | EDuke32 has no console flash: `palfrom 32 0 63` at the OSD answers "palfrom: unknown command or cvar"; the wiki console list has none | OSD log; wiki console command list |

Not measured: the 8-bit Classic renderer (palette-based, so likely 6-bit
steps), Polymer, flashes across save/load, death or level change.

### What D39B/C implement from this

- **Faithful core:** exactly the table: `w = min(I, 63) / 64`, colour x4,
  replace on every call, I > 63 extends the full-strength hold, minus 1 per
  game tic.
- **Our improvement (D39B):** evaluate `w` per present from the flash's
  remaining tics minus the interpolated sub-tic fraction, so the same curve
  is a smooth line at 120/180/240 Hz instead of 30 Hz steps; a "Duke3D 30 Hz
  steps" setting keeps the integer staircase. TTK ticks at 30 Hz (20 Hz on
  overrun), so the fade must count game ticks, not wall time, to keep the
  duration.
- **Layer and pause:** EDuke32 tints menus and its console and keeps the
  flash frozen on screen while paused. For TTK, D39B settles whether that
  holds for the pause screen; loading screens and FMV are never tinted.
- **Console (user, 2026-11-03: "i'm pretty sure you can issue a color flash
  just from the console. we should aim for that functionality"):** EDuke32
  cannot, so ours goes further: D39C adds a developer console command taking
  palfrom arguments directly (`palfrom 48 52`, alias `flash`), with the same
  rules as the game API.

### Appendix: probe CON (test module, ours)

```
// D39A behaviour probe: scripted palfrom calls, logged per tic and per frame.
gamevar fltest_t 0 0
gamevar fltest_pt 0 0
gamevar fltest_tc 0 0
definequote 9000 TIC %d PT %d
definequote 9001 x
definequote 9003 x
definequote 9002 FRM %d PT %d CLK %d

onevent EVENT_GAME
  ifactor APLAYER
  {
    addvar fltest_t 1
    ifvare fltest_t 90 palfrom 32 32
    ifvare fltest_t 180 palfrom 16 0 32
    ifvare fltest_t 270 palfrom 48 52
    ifvare fltest_t 360 palfrom 63 63 63 63
    ifvare fltest_t 480 palfrom 100 0 0 63
    ifvare fltest_t 660 palfrom 48 52
    ifvare fltest_t 665 palfrom 16 0 32
    ifvare fltest_t 760 palfrom 16 0 32
    ifvare fltest_t 762 palfrom 48 0 0 63
    ifvare fltest_t 860 { palfrom 48 63 0 0 palfrom 16 0 63 0 }
    ifvarg fltest_t 959 ifvarl fltest_t 1021 palfrom 32 63 0 0
    ifvarg fltest_t 1049 ifvarl fltest_t 1111 palfrom 16 0 63 0
    ifvarg fltest_t 1139 ifvarl fltest_t 1201 palfrom 48 0 0 63
    ifvarg fltest_t 1229 ifvarl fltest_t 1291 palfrom 32 63 63 63
    ifvarg fltest_t 1319 ifvarl fltest_t 1381 palfrom 8 32 16 0
    ifvarg fltest_t 1409 ifvarl fltest_t 1471 palfrom 63 0 0 0
    getplayer[THISACTOR].pals_time fltest_pt
    qsprintf 9001 9000 fltest_t fltest_pt
    echo 9001
  }
endevent

onevent EVENT_DISPLAYREST
  getplayer[THISACTOR].pals_time fltest_pt
  setvarvar fltest_tc totalclock
  qsprintf 9003 9002 fltest_t fltest_pt fltest_tc
  echo 9003
endevent
```

Run from a private copy: `./eduke32 -usecwd -nosetup -ns -mx fltest.con`,
load the save, then read `TIC`/`FRM` lines from `eduke32.log`.

### Where Duke3D flashes (GAME.CON in the user's DUKE3D.GRP, 35 uses)

| Event | palfrom | Look |
| --- | --- | --- |
| Item pickup (health, armour, ammo, inventory; quick get) | 16 0 32 | light green |
| Weapon pickup | 32 0 32 | green |
| Shield (pigcop armour) | 24 0 32 | green |
| Hit by fire laser, shark bite | 32 32 | red |
| Hit by RPG / radius explosion | 48 52 | strong red |
| Knee (kick) hit | 16 32 | light red |
| Shrinker / expander (COOLEXPLOSION1) hit | 48 48 0 48 | magenta |
| Grow spark (dying) | 48 48 | red |
| Frozen (each tic while frozen) | 16 0 0 24 | blue |
| Squished | 32 63 63 63 | white |
| Gibbed by growing / squished small | 48 64 | full red |
| Fire, burning barrel, steam, burning state (near, damaging) | 16 16 / 24 16 / 32 32 | red/orange |
| Enemy melee (octabrain 8 32, newbeast 16 16, turret 32 16, commander 32, boss4 32 32 0 0) | various | red |
| Boss stomp | 63 63 | full red |
| god mode off (engine: a zero-damage hit) | (32 32) | red |

Night vision and steroids are palette swaps in Duke3D, not palfrom.

## TTK side (known)

- TTK draws no hit tint. Its damage handler `0x800a40a8` (hooked since D08A13)
  rumbles and plays pain sounds per damage type (note 369 table: type 11 fall,
  12 toxic gas, mode 5 under water, others `0x2009`/`0x200a`). It is the
  natural source for damage flashes, with the type, amount, attacker and
  source object in its arguments.
- Pickups: `0x80081a48` (hooked, D08A9). Explosions: effect spawns such as
  `0x8006d594(type, pos, ...)` and the explosion/death calls (`0x80070f7c`)
  need a survey (D39F).
- The flash state already lives in guest mod memory (the word after the D20C
  mirror, tag `0x5d087000`), so savestates and redraw workers carry it.

## Design direction (to settle in D39B/C)

1. **Polymorph present tint (framework).** A final full-frame pass: out =
   mix(frame, colour, weight), applied to the composed image after the game
   chains (choose: before or after host overlays such as the inventory box
   and OSD; Duke3D tints its text). Exact, any colour and weight.
2. **Display-rate fade.** The renderer evaluates the weight per present from
   the flash's start and the interpolated game time (redraw fraction), so a
   fade is smooth at 120/180/240 Hz while the game logic stays 30 Hz;
   optionally Duke3D-faithful 30 Hz steps as a setting.
3. **Game API.** `screen_flash(intensity, r, g, b)` with palfrom units and
   rules, state in mod memory; a console command (`flash 48 63 63 63`) and
   debug JSON for tests; Vanilla never flashes.
4. **Classic renderer** (frozen): keep the D08G7 add/subtract quads as its
   fallback, or no flash.
5. **Accessibility:** a Screen Flashes option (On / Reduced / Off), D21.

## Risks

- Present-path work must cover the main present, render replay images,
  `present_shot`, widescreen bezels and the savestate panel; the tint must not
  apply to menus, loading screens or FMV.
- The redraw fraction must be the same one the replay picks, or the fade can
  step against motion.
- Framework edits need `export_runtime_patch.py` and a clean-pin check.
