# D39 - Screen flash framework (Duke3D `palfrom`-style colour flashes)

Job: [D39](../MODERNIZATION_JOBS.md) and D39A-D39G. Status: D39A accepted
2026-11-03 (spec table below, locked); D39B accepted 2026-11-03
(section below); D39C accepted 2026-11-03 (section below); D39D accepted
2026-11-03 (section below); D39E accepted 2026-11-03 (section below); D39G accepted 2026-11-03 (section below). Follows D08G7 ([note 369](369-d08g7-kroz-feel.md)),
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

## D39B - Polymorph present tint (accepted 2026-11-03)

### Framework (`recomp/psxrecomp`, in the exported patch)

- `psx_mod_set_present_tint(fn, user)` (`mod_plugins.h`): a title callback,
  asked once per present on the window's thread, fills `rgbw` and returns
  non-zero to tint. `slot` is the replay image being shown (the replay
  provider's own return value: 0 the real image, n > 0 cache entry n - 1) or
  -1 for a present outside replay presentation. Blank presents are never
  asked.
- `gpu_gl_renderer.c` `gl_draw_present_tint`: one quad with constant-alpha
  blending (`glBlendColor(w)`, `CONSTANT_ALPHA / ONE_MINUS_CONSTANT_ALPHA`,
  colour from a 1x1 texel), so `out = in * (1 - w) + rgb * w` in display
  values with the exact weight and no shader of its own. Idle (callback
  returns 0) it draws nothing.
- **Layer, settled:** over the fitted picture and the title's HUD overlays
  (crosshair, quote text, D08A5 inventory box, key card, LOAD GAME
  thumbnail), as Duke3D tints its status bar and quotes; under the developer
  console (moved after the tint; it is a developer tool and spans the whole
  window) and the runtime's overlays (volume, rewind, savestate panel).
  Letterbox bars and bezels outside the picture stay as they are.
- Every present path goes through `gl_swap_with_osd`, so live, replay,
  hold-last and `present_shot` captures all carry the tint. The render-thread
  copies of the renderer (`render_copy/rc*_names.h`, regenerated with
  `gen_names.py`) never present.

### Title (`recomp/src/ttk/screen_flash.{h,cpp}`, `frame_replay.cpp`)

- **Record:** two words after the D08G7 word in the near-clip arena tail (mod
  memory, `near_clip_flash_record()` = mirror + 8): `0x39` tag, 6-bit seq and
  6-bit r, g, b; then `0x5d39` tag and the level, the palfrom intensity x 10
  in frame-delta units (300 a second: one 30 Hz tic is 10, so a fade keeps
  its duration at 20 fps). Savestates and redraw workers carry it; an old
  state reads none. `screen_flash_palfrom(I, r, g, b)` replaces the flash
  (seq + 1); `screen_flash_update()` runs first in each Modernized player
  update (next to the D08G7 tick) and subtracts the frame delta.
- **Display-rate fade:** every captured frame stores the record. A cached
  redraw's tint is mixed when the redraw enters the cache, at the redraw's
  own alpha between the two frames it blends (`flash_mix`): the fade follows
  the same clock and world delay as the motion, and a repeated image keeps
  its tint. A flash started in the next frame (seq changed or level rose)
  shows from the next pair at alpha 0 at full intensity: Duke3D's instant
  onset on the tic's first frame, never blended in.
- **Live presents** (FRAME RATE 60, the real image, pause): the live record,
  carried on from the last player update by at most one update's step. Only
  in gameplay (`0x800bcbb0` = 1, not suspended); paused it is frozen and still
  drawn (as EDuke32); with no player update for 500 ms (a load or a scene)
  nothing is drawn.
- **Steps:** `DNTTK_FLASH_STEPS=1` (developer, for now; D39C may expose it)
  draws each game frame's own level: Duke3D's 30 Hz staircase.
- Modernized and Polymorph only: Vanilla never registers the callback;
  Classic returns no tint (D39G decides). The D08G7 god-mode quads are
  untouched until D39D.
- Diagnostics: `render_replay` debug JSON `plugin.flash` (record, level,
  colour, seq, tint counters, last tint, the last 2048 presents as
  `ms/slot/weight`); `DNTTK_TEST_INPUT` `flash=I,r,g,b[,hold]` starts a flash
  (hold 1 restarts it every update: Duke3D's palfrom each tic, a steady
  level). No player-facing trigger yet: D39C adds the API, console command
  and option.

### Evidence (local lab `recomp/analysis/d39b-present-tint/`, ignored)

`probe.py` on private card copies, offscreen on the NVIDIA GPU
(`SDL_VIDEODRIVER=offscreen`; under Xvfb software GL no redraw ever presented,
so Xvfb is unusable here), third person, slot 4. Executable
`c7bc0fca498011a02f09578e3b4c6d5c337a11346465c1043b83cdc6f566a8cf`.
Player saves and profile hash-checked unchanged after every run.

| Check | Result |
| --- | --- |
| Colour (180 Hz, eight held flashes, per-channel least squares of the whole frame against an unflashed one) | Drawn weight exactly I/64 (8 -> 0.125 ... 63 -> 0.984). Fitted `1 - w` within 0.015 of the spec (0.861-0.872 for 0.875; 0.488-0.501 for 0.5; 0.017-0.020 for 0.016); offsets within 0.2 levels of `4 C w` (31.94 for 31.5 at the darkest, 63.83 for 64, 155.89 for 156, 247.9 for 248.06, 144.0 for 144); mean residual 0.14-0.54 levels |
| Fade, god-off 32 32 0 0, 180 Hz | 193 presents over 1066.7 ms (32 tics), 193 distinct weights, onset 0.4994, slope -0.468/s (spec 0.469), max residual from a straight line 0.0065, largest present-to-present drop 0.0052; three repetitions identical |
| Fade, 120 Hz | 129 presents, 129 distinct weights over 1066 ms, same slope and residual |
| Fade, FRAME RATE 60 | No in-between redraws at 60: the game presents only its own images (30 a second), so the fade has 33 even steps over 1068 ms, like the picture it is drawn on (onset 0.494) |
| `palfrom 100 0 0 63` | full strength (0.984) held 1.23 s (37 tics, I - 63), then the same straight fade |
| Steps setting, 180 Hz | 33 distinct levels, onset 0.5: the Duke3D staircase |
| Replace | red 48 then green 16 two fields later: level 160, green, seq + 1 |
| Savestate | saved at level 950-960, loaded state restores the record and fades on from it |
| Pause | frozen (950 and 950 one second apart) and drawn under the pause menu, as EDuke32 |
| Idle | 360-370 presents asked, none drawn |
| Coverage (captures) | world, Duke, crosshair and status bar tinted; developer console's translucent panel and text over the tint |
| Vanilla (60) / Classic (180) | captures with a requested flash identical to without (mean RGB equal to 0.1); Classic draws 0 tints |

Suites: new `ttk-flash-test` (mix rules: weight, colour, linear fade, steps,
instant onset, replacement, 20 fps line), `ttk-controls-test`,
`ttk-input-test`, `ttk-aim-test`, `ttk-near-test`, animation save/restore
pass; Python 223 with the one failure that predates this job
(`test_primitive_trace`), 2 skipped. Framework patch exported; applied to a
clean checkout of the pin it reproduces every modified and new framework
file byte for byte.

### Limits

- At FRAME RATE 60 the fade steps at the game's 30 images a second (as the
  picture does); smoothness above that needs the in-between redraws
  (FRAME RATE above 60, the default AUTO on the player's display).
- Paused at full strength the pause menu is hard to read (EDuke32 behaves
  the same); D39C's Reduced/Off option and the game's own flashes, which are
  short, make this rare.
- Not checked: first person (same present path; no first-person-specific
  code), widescreen bezels with letterbox bars (bars are left untinted by
  design), a flash across a level change.

## D39C - Game flash API, console command and Screen Flashes option (accepted 2026-11-03)

### API (`recomp/src/ttk/screen_flash.{h,cpp}`)

- `screen_flash(intensity, r = 0, g = 0, b = 0)` is the game API (D39B's
  `screen_flash_palfrom`, renamed): palfrom units and rules on the emulation
  thread, in the Modernized player update. Every call replaces the flash,
  weaker or stronger; the last call in a game frame wins; intensity is
  clamped to 0..255 (0 clears), components to 0..63. D39D-F call it.
- Other threads (the console, `DNTTK_TEST_INPUT flash=`) use
  `screen_flash_request`, an atomic word the next player update turns into a
  `screen_flash` call. The record (mod memory) is still written only on the
  emulation thread.
- Rules are pure inline functions, tested natively: `flash_parse_palfrom`
  (console arguments), `flash_option_rgbw` / `flash_option_parse` (option).

### Console (`palfrom`, alias `flash`)

`palfrom <intensity> [r] [g] [b]` at the backtick console (dispatched from
`level_console_command`; listed by `help`, the one framework line in the
exported patch). Whole numbers only; intensity 0..255, components 0..63,
omitted components 0; anything else is refused with the reason and a usage
line. Accepted: the reply echoes the four values (noting SCREEN FLASHES OFF or
REDUCED) and the console closes so the flash is seen. Refused with a reason
outside play (`0x800bcbb0` != 1), in Vanilla and on Classic. The debug port's
`console` command runs the same line.

The flash appears with the replayed game frame of the update that started it
(D39B: the tint travels with the frame, so it stays in step with the
picture). Measured: the first tinted present comes about 3 game frames after
the console call at 180 Hz, the same delay as the motion.

### SCREEN FLASHES option

OPTIONS > GAMEPLAY > SCREEN FLASHES ON / REDUCED / OFF (profile
`controls.screen_flashes`, schema 62, default `on`; `run.py --screen-flashes`;
terminal choice X; env `DNTTK_SCREEN_FLASHES`, live; Vanilla always `off`).

- Applied where the tint is drawn (`present_tint`, after the live/replay
  choice), never to the record: the fade, savestates and replay are the same
  under every option, and a change applies to the next present.
- **On:** the measured weight. **Reduced:** `min(w / 2, 1/4)`: god mode off
  (32) draws as 16, a pickup (16) as 8, white 63 as 16; durations unchanged.
  **Off:** nothing drawn.
- The D08G7 god-mode quads (until D39D) follow it too: Off skips the flash,
  Reduced starts it at half (170 of 340; a quarter mix at the peak).

### Debug JSON

`render_replay` `plugin.flash` gains `option` (`on` / `reduced` / `off`) and
`requests` (console and test-input requests), beside D39B's record, level,
colour, seq, tint counters and per-present notes.

### Evidence (local lab `recomp/analysis/d39c-flash-api/`, ignored)

`probe.py` (D39B's with console cases) on private card copies, offscreen on
the NVIDIA GPU at 180 Hz, third person, slot 4; flashes issued through the
debug port's `console` command. Player saves and profile hash-checked
unchanged after every run. Executable
`e8734712ae3f0a5e5e394406e56b14449facff3359d8a839fbe9e46d215a1ceb`.

| Check | Result (spec from the D39A table) |
| --- | --- |
| `palfrom 32 32` (god off) | 193 presents, 1066.7 ms (32 tics), onset 0.498 (0.5), slope -0.468/s (0.469), 193 distinct weights, max residual 0.007 |
| `palfrom 48 52` | 1599.8 ms (48 tics), onset 0.748 (0.75) |
| `flash 16 0 32` | 533.3 ms (16 tics), onset 0.249 (0.25) |
| `palfrom 63 63 63 63` | 2099.9 ms (63 tics), onset 0.982 (0.984) |
| `palfrom 100 0 0 63` | full strength 0.984 held 1227.8 ms (37 tics, I - 63), then the same line |
| Colour (whole-frame per-channel fit of six console flashes, mid-fade) | one weight for all three channels (within 0.004); implied 6-bit colour within 0.2 of the request for five, about 1 unit for 48 48 0 48 |
| Replace | `palfrom 48 63 0 0`, then `flash 16 0 63 0`: level 160, green, seq + 1; `palfrom 0` clears (level 0) |
| Refusals | empty, 256, component 64, words, negative, five numbers: the reason and usage; `flashy` stays an unknown command |
| Reduced | held 32 32: weight 0.25; 63 white: 0.25; 16 green: 0.125 |
| Off | held flashes: record runs (level 320 / 630 / 160), 0 tints drawn; reply notes OFF |
| OPTIONS | GAMEPLAY row drawn and cycled (ON, REDUCED, OFF); CONFIRM saved `screen_flashes: off` to the private profile and applied live (a following console flash replied OFF and drew nothing) |
| God mode off (D08G7 quads) | peak level 340 On, 170 Reduced, none Off |
| Vanilla / Classic | console reply "Modernized only" / "need the Polymorph renderer"; nothing drawn |

Suites: `ttk-flash-test` (new option and argument cases), `ttk-controls-test`
(LEVEL00 fixture, LEVEL01, all levels; 125 PASS lines), `ttk-input-test`,
`ttk-aim-test`, `ttk-near-test` pass; Python 225 with the one failure that
predates this job (`test_primitive_trace`), 2 skipped; the new schema 62
tests pass and the older schema references moved to 62. Framework patch exported; applied to a clean
checkout of the pin `08ec704` it reproduces all 78 framework files byte for
byte.

### Limits

- No game event calls `screen_flash` yet besides the console: D39D (god
  mode), D39E (damage) and D39F (pickups, explosions).
- `DNTTK_FLASH_STEPS` (the Duke3D 30 Hz staircase) stays a developer
  setting; it is not on OPTIONS.
- Reduced (half, capped at a quarter) was accepted with the job; not yet
  judged against real hit flashes (D39E).

## D39D - God-mode off flash on the present tint (accepted 2026-11-03)

### Change

- `cheats.inc` `god_mode_off_feel`: switching god mode off calls
  `screen_flash(32, 32, 0, 0)`, the flash D39A measured from `dnkroz` in
  EDuke32. The SCREEN FLASHES option applies where it is drawn
  (`present_tint`), as for every flash. The GOD MODE: OFF text and the 50%
  pain grunt (D08G7) are unchanged; ON shows no flash.
- Classic (no present tint): the D08G7 PS1 quads stay its god-off flash, with
  their option gate (Off skips, Reduced starts at half), until D39G decides.
  The quad hook in `modern_controls.cpp` now draws only on Classic, so an old
  savestate carrying a D08G7 level shows no quads on Polymorph (the level just
  runs out).
- `screen_flash_classic()` (`screen_flash.h`) is the one Classic check, used by
  the console refusal, `present_tint` and the cheat.

### Evidence (local lab `recomp/analysis/d39d-god-flash/`, ignored)

`probe.py` (D39C's with `kroz`, `krozopt`, `ktrace` cases) on private card
copies, offscreen on the NVIDIA GPU, slot 4. Player saves and profile
hash-checked unchanged after every run. Executable
`22e2c33b9dcfd0407fdbc2d67c81b207d7947df673b94f3448fd03db85022015`.

| Check | Result (spec: D39A god mode off, `palfrom 32 32 0 0`) |
| --- | --- |
| Trigger | one OFF pulse: one flash (seq + 1, colour 32 0 0, level 320); ON: none |
| Fade, 180 Hz, third person, x3 | 193-194 presents, 1066.7-1072 ms (32 tics), 193-194 distinct weights, onset 0.4997 (0.5), slope -0.468/s (0.469), max residual 0.007 |
| Fade, 120 Hz | 130 presents, 1075 ms, slope -0.468/s |
| First person | same fade (193 presents, 1066 ms); held flashes of eight colours fit within 0.003 of the spec weight and 0.3 levels of the colour |
| Colour, god-off frame against an unflashed one | red: w 0.499, colour 31.8 (0.5, 32); green and blue: w 0.49-0.55 (idle animation between the two frames) |
| D08G7 quads (Polymorph) | level 0 throughout |
| Reduced / Off | peak weight 0.25 / 0 tints drawn |
| Classic | quad peak 340 (as D08G7), 0 tints |

Suites: `ttk-flash-test`, `ttk-near-test`, `ttk-input-test`, `ttk-aim-test`,
`ttk-controls-test` (125 PASS) pass; Python 225 with the old
`test_primitive_trace` failure, 2 skipped. No framework change.

### Limits

- The GOD MODE: OFF text is host-drawn at once; the flash rides the replayed
  game frame, so the text can lead the red by about 3 game frames (0.1 s, the
  replay's world delay measured in D39C). One of three first-person captures
  caught that gap. In EDuke32 they start together.
- Accepted on the user's playtest (2026-11-03): "amazing work! lock it in,
  close it out".

## D39E - Damage flashes from real hits (accepted 2026-11-03)

### Survey: Duke's damage handler `0x800a40a8`

Read from the owned executable (`recomp/analysis/d39e-damage-flash/exedis.py`,
`callers.py`, `handler.txt`; ignored lab). Arguments: a0 victim, a1 hit
position, a2 amount, a3 type, stack +0x10 the attacker or source object
(+0x14..+0x1c more). Every call reaches it through the class table
`0x800c5ecc` (Duke's class 2 is `0x800c5ed4`); projectile hits take their type
from the 64-byte projectile table `0x800c4bc0` (+0x12; 37 rows, types 0-7, 10,
14, 17, 18, 20 used).

| Step | Address | Effect |
| --- | --- | --- |
| Entry cancels | `0x800a40ec` | victim word bit `0x20000000`; self-inflicted types 5, 15, 10 return 0 |
| Attacker | `0x800a4114` | an attacking player (id 32) counts the hit (+0x3b4) and doubles it with +0x89e |
| Steroid shield | `0x800a4154` | the original shield (Modernized hides it for Duke, D08A13) |
| Difficulty | `0x800a41c8` | skill `0x800c3d7c` scales the amount (1000 -> 750 in the test state) |
| Type switch | `0x800a42b0`, table `0x80018aec` | 1, 2, 3, 10, 17, 19 hit-part armour, armour, blood; 4, 13, 16 knockback (`0x800a3e80`), armour, big rumble; 5, 14 armour; 6, 9 rumble only; 7 the Freezer: freezes Duke (word bit `0x20000`, timer +0x214 3000, sound `0x101e`; note 275) and deals nothing; 12 gas, nothing when the Bio Mask is worn; 20 lowers +0x8a2; 8, 11, 15, 18 plain |
| Gate | `0x800a44ec` | god mode `0x800c3cc6`, victim word bit `0x10000`, +0x8a0 invincibility: no health change. Armour was already taken above (god mode still wears armour, as the original does) |
| Health / death | `0x800a4528` / `0x800a4638` | health falls, pain sound (fall `0x200a`, gas `0x1c004` 10%, under water `0x1c000` 15%, else `0x2009 + rand % 2`); or death |

Runtime (`DNTTK_DAMAGE_TRACE=1`, `probe.py survey`, slots 0-11 standing for
15 s): slot 3's pig cop (id 57) shoots projectile row 2, type 2, 1000 -> 750
health a hit, 12 hits; slot 2 one type-2 hit cancelled by the gate; slots 9
and 10 thousands of type-15 calls with Duke as his own source, all cancelled at
entry. Frozen (type 7, slot 4): the timer +0x214 runs from 3000 to 0 over
about 3-4.6 s and Duke loses no health while frozen. (A first draft called
this "set alight"; note 275 had already identified it as the Freezer.)

Necro: `spawn enemy 50` in level 6 (the debug enemy spawn added for this job)
and walking into it: its bolt is projectile row 20 (type 2, 1000), fired by
object type 50. The single slot-2 cancelled hit seen in the survey was the
same row and attacker.

TTK has no shrinker. The Freezer (type 7) takes Duke3D's frozen blue and the
Necro (TTK's Octabrain) its purple COOLEXPLOSION1 flash (user, 2026-11-03:
"when the necros hit you (the analogue to the octobrains), i think it should
be the same purple flash as eduke32"). The scaffold maps any type, attacker
or projectile (user, 2026-11-03: "even if we have the ability to
script it, and you dont spot any analogous scenarious in ttk, that's a win in
itself").

### Behaviour

- `damage_flash.inc`, entry hook on `0x800a40a8` (after the D08A13 steroid
  pass and the D08A24 Bio Mask), Modernized, Polymorph, Duke only. The entry
  notes health, armour, the gate and the frozen bit; the outcome is read
  after the call returns (at the next handler entry or the next player
  update), so a flash follows what the original actually did, never the
  arguments. A hit counts when it took health, armour with the gate open, his
  life, or (type 7) froze him. Hits in one game frame add up (a shotgun
  blast's pellets are separate calls).
- `damage_flash_for` (`screen_flash.h`, native-tested), Duke3D's catalog
  values, by the handler's own families:

| TTK hit | palfrom | Duke3D source (catalog) |
| --- | --- | --- |
| Shots, blows, other types (1-3, 5, 6, 8-10, 14, 15, 17-20), 3% or more | 32 32 | a hit (the measured god-off zero-damage hit; fire laser) |
| The same, under 3% | 16 32 | knee |
| Blast (4, 13, 16), 15% or more or fatal | 48 52 | RPG / radius |
| Blast under 15% | 32 32 | a hit |
| Necro bolt (attacker type 50 or projectile row 20), any size | 48 48 0 48 | Octabrain COOLEXPLOSION1 |
| Frozen (7), then held every frame while frozen, cleared at once on the thaw | 16 0 0 24 | frozen (each tic while frozen), EDuke32's colour |
| Fall (11), 25% or more or fatal | 32 32 | a hit |
| Fall under 25% | 16 32 | knee |
| Toxic gas (12), each frame it hurts | 16 24 32 0 | ours: Duke3D has no gas flash; yellow-green to tell poison from pickups (green) |

  Percentages are of health plus armour taken (10000 = 100%).
- **Stronger flash kept (ours):** a hit draws only when its intensity is at
  least the level still fading (`I x 10 >= level`), so a light hit never cuts
  a heavy one short. The raw `screen_flash` / console keep palfrom's replace
  rule.
- **Frozen blue (replaces D35J5's quads on Polymorph):** while Duke is
  frozen (`frozen_eye_state`: alive, bit `0x20000`, timer running) each
  game frame gives `palfrom 16 0 0 24` again (unless a stronger flash is
  fading), in either view; the thaw (or death) clears it at once, as in
  EDuke32 (user: "it absolutely does not need to fade out. in eduke it's
  instant"). History: the first version used 16 0 0 24 but the hold never ran
  in a real freeze (see Per-frame hook), so the user saw nothing; a D35J5
  half blend (32 2 2 16) was tried in between. EDuke32's colour, fitted from
  `freezethrow.mp4` by matching each channel's mean and spread on the world
  area of a frozen frame and an after-thaw frame of the same view (the frozen
  eye is lower, so pixel fits misalign): every channel x0.75 (0.744-0.763),
  red and green offset about 0, blue +22.7; black becomes blue 23. That is
  16 0 0 24 exactly (x0.75, blue +24). Measured
  from the user's EDuke32 capture `research/screencaps/freezethrow.mp4` (10
  fps means): unfrozen (27.7, 21.4, 15.8), frozen hold for about 4.5 s at
  (17.4, 15.3, 36.6) against 16 0 0 24's predicted (20.8, 16.0, 35.9), then a
  0.5 s fade (16 tics); the bright frames at onset are the ice blast, not a
  tint. D35J5's half-blend PS1 quads now draw only on Classic (D39G).
- **Per-frame hook:** the flash's game frame (fade `screen_flash_update`, hit
  resolution, frozen hold, `hurt=`) runs at the entry of Duke's update
  `0x800412a4`, every game frame. It first ran from the step-table entry
  `0x8005a210` (D39B), which the game skips while Duke is frozen: no blue
  hold, no fade, and a Freezer hit was never judged. The session log notes
  each freeze and thaw with the hold count and any skip reason.
- **Thaw frame:** the drain that thaws Duke (`0x8004157c`, `(13653 x delta)
  >> 12` from +0x214, doubled with the jetpack, skipped for a scripted player)
  runs after the per-frame hook, so the hook predicts it with the same rule and
  clears the blue in the thaw frame itself; otherwise the first thawed frame
  (no ice, Duke moving) kept blue on all its presents. Developer trace
  `DNTTK_FLASH_TRACE=1` prints per present the game frame, blend, Duke frozen
  in that frame and the weight: the blue ends exactly at the first present
  composed from the thawed frame.
- **Debug enemy spawn** (`spawn.inc`, for testing): `spawn enemy` alone (or
  `enemies`) lists the creature types the level places; `spawn enemy <type>` moves an unspawned
  record of that type (`*0x800de720`, +8 x/y/z) 1500 units ahead of the view
  and creates it with the original `0x80098f3c` (a live one is removed first
  with `0x80098d70`; at the live cap `0x800c56a4`/`0x800c56a8` the farthest
  live enemy is removed first; a failed creation's permanent-disable flag
  0x10 is cleared). Note 111.
- SCREEN FLASHES applies where the tint is drawn, as for every flash. Vanilla
  never registers it; Classic draws no damage flash (D39G).
- Developer: `DNTTK_TEST_INPUT` `hurt=type,amount` sends one hit through the
  original handler on a private stack (no attacker), so every type,
  god mode and the option can be tested; `DNTTK_DAMAGE_TRACE=1` prints each
  hit; debug JSON `plugin.flash` gains `damage_hits` and `damage` (the last
  hit and what it drew).

### Evidence (local lab `recomp/analysis/d39e-damage-flash/`, ignored)

`probe.py` on private card copies, offscreen on the NVIDIA GPU, 180 Hz, third
person. Player saves and profile hash-checked unchanged after every run.
Executable `fbf48386913386d70a7277a5f4a83debd041bf85b23831f2536f8df618f0f45d` (Necro, frozen and enemy spawn rows; earlier rows on `858654965a05...`).

| Check | Result |
| --- | --- |
| Real hits, slot 3 pig cop, 12 s | 9 hits, 9 flashes (seq + 1 each), 466 tinted presents, peak weight 0.4998; hit frame against an unflashed one: red 1 - w 0.504, offset 62.7 (spec 0.5, 64); green and blue 0.49 / 0.48 with offsets near 1 |
| Types 1-20 through the handler (`hurt=t,1000`, 750 taken) | each flashes as the table: 4, 13, 16 blast 32 32; 7 freeze 16 0 0 24 (nothing taken, frozen); 11 fall 16 32; 12 gas 16 24 32 0; the rest 32 32 |
| Size | type 2: 74 and 224 taken 16 32, 749 32 32; type 4: 374, 1124 32 32, 3000 48 52; fall 2250 16 32; a killing shot 32 32 |
| God mode (`dnkroz`) | types 2, 4, 11, 12, 7: no flash; health unchanged (2 and 4 still took armour, as the original) |
| Stronger kept | blast 48 (level 460), then hits of 74 and 749 taken: not drawn (level 410, 360 fading on, same seq); later at level 290 a 32 replaces it (seq + 1) |
| Necro, level 6, `spawn enemy 50`, no god mode | 6 bolts (row 20, attacker 50, 750 each): 6 flashes `48 48 0 48`; drawn tint weight 0.734 toward (192, 0, 192) mid-fade (spec 0.75) |
| Frozen, third person (`hurt=7`) | record held at 160, blue (0, 0, 24), drawn weight 0.25 for the 3.0 s freeze; thaw: fade to 0 in 0.55 s (16 tics) |
| Frozen, first person | same hold and fade; frame mean (61.8, 35.7, 51.2) against 0.75 x unfrozen + (0, 0, 24) = (61.9, 35.8, 51.0) |
| Frozen, final (16 0 0 24, instant thaw), UI slot 11, first person (`hurt=7`) | 91 holds in the freeze; weight 0.25; frame mean (25.5, 22.3, 40.3) against 0.75 x unfrozen + (0, 0, 24) = (25.8, 22.8, 40.7); level 0 on the first frame after the thaw |
| Frozen at D35J5 strength (32 2 2 16, superseded), UI slot 11, first person (`hurt=7`) | held weight 0.5 toward (8, 8, 64) for the 2.9 s freeze, then a straight fade over 1.0 s; frame mean (21.3, 19.2, 42.7) against 0.5 x unfrozen + (4, 4, 32) = (21.2, 19.2, 43.1). The rows above with 0.25 were the first, Duke3D-strength version |
| Frozen, Classic, first person | no flash record; D35J5 quads still drawn (whole-frame fit 0.51 with blue offset 31) |
| SCREEN FLASHES | heavy blast peak weight On 0.749 (48/64), Reduced 0.25, Off 0 tints drawn |
| Vanilla | no flash plugin, no damage trace, `hurt=` not available |
| Classic | the hit lands (health 7165 -> 6415), no damage flash, 0 tints |

Suites: `ttk-flash-test` (new damage mapping cases), `ttk-near-test`,
`ttk-input-test`, `ttk-aim-test`, `ttk-controls-test` (125 PASS) pass; Python
225 with the old `test_primitive_trace` failure, 2 skipped. No framework
change.

### Limits

- EDuke32 comparison for ordinary shots was not completed: under Xvfb the
  private EDuke32 copy stopped advancing after 4 tics (probe CON that spawns
  an enemy beside Duke and logs health and `pals`: lab
  `eduke32-hit.con`), so `32 32` for a plain hit rests
  on D39A's measured engine hit (god mode off is a zero-damage hit, `palfrom
  32 32 0 0`) and the catalog, not a captured trooper shot.
- Not seen in play: blasts, falls, gas and Freezer hits from real sources (only
  through `hurt=`); multiple hits in one frame (the frame sum) has no test.
- Types 5, 6, 9, 14, 18, 20 are unidentified sources; they flash as hits.
- Under steady fire (slot 3: a hit every 1.3 s) the 1.07 s red is nearly
  continuous, as Duke3D's would be; REDUCED is the softer choice.

## D39G - Classic renderer: no god-mode flash (accepted 2026-11-03)

Decision (user, 2026-11-03): "remove the godmode flash unfortunately, and
leave it there. the freeze one is harmless enough. the godmode one was just
not smooth enough to have in the game, so we leave classic renderer simple".

### Change

- `cheats.inc` `god_mode_off_feel`: Classic starts no flash; the log reads
  `flash none (Classic)`. Polymorph still calls `screen_flash(32, 32, 0, 0)`.
- Removed the D08G7 quads entirely: `near_clip_screen_flash()`,
  `near_clip_screen_flash_set()`, `near_clip_screen_flash_draw()`, the
  frame-delta fade tick, the draw hook at `0x8001fc44` (return `0x800265c4`),
  the 192-byte fixed packet slot at the end of each host region's base part
  (`packet_alloc` now only keeps packets from straddling the base and extra
  parts), the native contract test and the `screen_flash` /
  `screen_flash_draws` fields of the near-clip debug JSON.
- The tagged word at mirror + 4 is retired, not reused: an old savestate may
  carry a nonzero D08G7 level there, which nothing reads. The D39B flash
  record stays at mirror + 8.
- Unchanged: D35J5's half-blend frozen quads (Classic's frozen tint), the
  console refusal on Classic, Classic damage (no flash), Polymorph, Vanilla.

### Evidence (local lab `recomp/analysis/d39g-classic/`, ignored)

`probe.py` (D39E's, plus a `cgod` case) on private card copies, offscreen,
slot 4, 180 Hz. Player saves and profile hash-checked unchanged after every
run.

| Check | Result |
| --- | --- |
| Classic, third person, god off x3 | frame mean RGB before OFF and 4 captures after within 0.05 levels; 0 tints drawn; log `flash none (Classic)` with the pain grunt |
| Classic, first person, frozen (`hurt=7`) | D35J5 quads drawn: whole-frame fit (0.508, 3.85), (0.508, 3.83), (0.513, 30.89), equal to D39E's |
| Polymorph, third person, god off x3 | 193 presents, 1066.7 ms, onset 0.498-0.499, slope -0.468/s (as D39D); onset frame colour fit w 0.49-0.55 toward (31.8, 0.3, 0.4) on two reps, the third capture missed the onset |
| Polymorph, third person, frozen | fit (0.384, 25.99), (0.390, 14.25), (0.412, 31.02), equal to D39E's |

Suites: `ttk-flash-test`, `ttk-near-test` (D08G7 contract removed),
`ttk-input-test`, `ttk-aim-test`, `ttk-controls-test` (125 PASS) pass.
User playtest on Classic, 2026-11-03: "classic renderer looks fine".

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
4. **Classic renderer** (frozen): no flash (D39G decided; the D08G7 quads
   are removed, the D35J5 frozen quads stay).
5. **Accessibility:** a Screen Flashes option (On / Reduced / Off), D21.

## Risks

- Present-path work must cover the main present, render replay images,
  `present_shot`, widescreen bezels and the savestate panel; the tint must not
  apply to menus, loading screens or FMV.
- The redraw fraction must be the same one the replay picks, or the fade can
  step against motion.
- Framework edits need `export_runtime_patch.py` and a clean-pin check.
