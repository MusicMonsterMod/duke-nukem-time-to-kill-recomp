# D10H4 - Shot on press: WEAPON FEEL RETRO

2026-11-05. Status: Done (accepted and closed 2026-11-05). Follows the D10H1 lab
([note 389](389-d10h1-weapon-feel-lab.md)) and the D10H report
([note 388](388-d10h-duke3d-weapon-feel.md)).

User (2026-11-05): changing native timing in Modernized is fine, "and make it
togglable"; the TTK-only weapons keep their timing. During this job: "ultimately
we could end up with two weapons profiles here. modern (where we are now) or
eduke where we will be going" ("just a thought"), named "eduke32". So the
toggle is a profile, **WEAPON FEEL: RETRO / MODERN**, that later EDuke32
parity work (D10H3 auto aim, D10H6 repeat rate) can join. It was first named
EDUKE32; user, after playtesting D10H6: "i dont want to call this "eduke" in
the options because it's only a close simulation of it for now. let's call it
"retro"".

## What the original does (measured and read)

- The game logic updates about every 33 ms. The upper-body track
  (`player+0x74`, advanced by `0x80059db0` from `0x8005a5a8`) plays the
  weapon. Its callback `0x8004e840` (table `0x800c2754[5]`, a switch on the
  upper animation, jump table `0x80014da8`) starts the raise from rest
  (`0x8005804c`, which also sets the original's own fire request,
  `+0x224 |= 0x110`) and, in the fire tracks, tests fire and calls the gun
  (`0x8003c500` from `0x8004f7ac` / `0x8004fa80`; `+0x3b2` counts shots).
- Upper animations per weapon (debug-port polling, 180 Hz, from rest):

| Weapon (id) | Rest | Raise | Fire track, shot after its start | After the shot |
| --- | --- | --- | --- | --- |
| Desert Eagle (4) | 5 | 15, about 98 ms | 16, about 66 ms | 14 gun up, 15 backwards to rest |
| Combat Shotgun (5) | 20 | 24, about 272 ms | 25, about 60 ms | 26 pump (about 450 ms), 23 |
| Gatling (7, 28), Freezer (11) | 29 | 33, about 198 ms | 34, about 66 ms | 32 gun up |
| RPG (8, 29) | 35 | none | 37, about 163 ms | 35 |

- **Correction to note 389.** D10H1's "presses under about 150 ms fire
  nothing" was mostly a lab artifact: at the lab's level 1 spot Duke is hurt
  even with `dnkroz` (health 9626 to 0 in about 13 s, then death animation
  212), and the click test ran after the timing runs. Hit and death animations
  (212, 215, 216) block fire. On a healthy Duke the original's own request
  keeps a short click: every 60 ms click on a healthy Duke fired (20 of 20
  from rest, about 190 ms later), and so did the one 30 ms click made before
  he died. What does drop is a click shorter than one game update:
  16 ms taps fired 0 of 4 (first probe), 12 of 20 (MODERN A/B below).

## Change (Modernized, WEAPON FEEL RETRO only)

`recomp/src/ttk/weapon_feel.inc` (included by `modern_controls.cpp`):

- **Latch.** `pc_input.cpp` reports each captured fire press edge
  (`weapon_feel_press`). Until the next authenticated player update
  (`0x8005a210`) has seen it, and then for two more updates when Duke holds
  one of the analog weapons armed (`+0x3b8 == 2`), `input_pad()` keeps the
  fire pad down. The later masks (fire draw, climbs, pull-ups, pushables)
  still win.
- **Earlier shot.** From that press until its round leaves (the player's gun
  call, reported by `weapon_aim.cpp` in every aim mode), the raise gets 4x
  and the fire track 2x the upper-track delta, once per player update, at the
  same guarded call as the existing draw boosts (`a0 == player+0x74`,
  `a3 == 1`, delta at entry `sp-0x2e`). The original runs every event and the
  shot test itself. Ground, jumps and falls, and the jetpack only (swimming and
  climbs keep their timing). A press the original refuses (pump, reload, hit
  reaction, switch) stops waiting after 30 updates. Developer factors:
  `DNTTK_WEAPON_FEEL_RAISE` / `_FIRE` (1-8).
- Held repeats are native (D10H6). Ammo, damage, spread, the Crossbow,
  Buffalo Rifle, Flamethrower, Energy Weapon and throws are untouched.
- Setting: profile schema 67 `weapon_feel` (`retro` default, `modern`; the first build saved `eduke32`, read as `retro`),
  `run.py --weapon-feel`, terminal `--settings` key H, OPTIONS > ADVANCED >
  COMBAT > WEAPON FEEL (live). `run.py` passes `modern` for Vanilla.
- No hook, codegen or framework change; existing savestates load.
  Debug: `ttk_input` `controls.weapon_feel` (presses, latches, boosted shots,
  expired). The `ttk_input` JSON buffer grew to 16384 (the new field had
  pushed it past 10240 and truncated it).

## Evidence

Local lab `recomp/analysis/d10h4/` (ignored): `anim_probe.py` (upper track
timeline), `click2-4.py`, `idle.py` (the dying-Duke finding), `feel_lab.py`
(A/B; the weapon feel is set in the lab's private profile copy, because
`run.py` sets `DNTTK_WEAPON_FEEL` from the profile). D10H1 harness: private
D10B apartment card copies, level 1, `dnstuff` and `dnkroz`, offscreen
180 Hz, a level restart before each half of a weapon's rows. Player files
hash-checked unchanged after every run.

Press to shot, ms, third person (4 clicks of 60 ms each):

| Weapon | RETRO rest | MODERN rest | EDuke32 (note 389) | RETRO gun up | MODERN gun up |
| --- | --- | --- | --- | --- | --- |
| Desert Eagle | 72-102 | 171-212 | 58 | 49-74 | 92-109 |
| Combat Shotgun | 112-145 | 354-380 | 123 | 81-111 | 4 dropped (pump) |
| Gatling | 81-97 | 256-301 | 88 | 28-47 (2 rows: Duke died) | 91-109 |
| RPG | 87-151 | 196-226 | 108 | 92-100 (2 rows: Duke died) | 4 dropped |
| Freezer | 79-108 | 283-297 | 81 | 19-50 | 83-105 |

- 16 ms taps: RETRO 20 of 20 fired (third person), MODERN 12 of 20.
- Fire held 2.5 s: first shot RETRO 91-129 ms, MODERN 204-377; repeat
  unchanged (pistol 4.99 / 4.94 per s, shotgun 1.22 / 1.24, Gatling
  8.49 / 8.55, RPG 2.24 / 2.23, Freezer 5.96 / 5.67).
- First person, RETRO: Desert Eagle 83-105 rest, 52-79 gun up; Gatling
  85-112 / 83-107; 16 ms taps 6 of 8 (Duke was being hit during that
  segment; not separated).
- Suites: `ttk-controls-test` (camera fixture, LEVEL01, all levels),
  `ttk-input-test`, `ttk-aim-test`, Python `test_player_profiles` (125, new
  `WeaponFeelOptionTest`) pass.

## Not verified / limits

- The real mouse at 180 Hz on the player's display; whether 4x raise looks
  natural (Duke snaps the gun up). The user judges feel.
- Swimming and climbs were not run (the boost is off there by design; the
  latch applies). Vanilla was not launched: every path is behind
  `input_modernized()` and `run.py` passes `modern` for Vanilla.
- The Desert Eagle stays about 15-40 ms behind EDuke32's 58 ms: the game
  update (33 ms) is the floor of raise plus fire. A "ready stance" (gun kept
  up) was not needed for the other weapons and would change the idle look.
- Clicks during the shotgun pump or a reload are still dropped, as in Duke 3D.
- Pressing in the lab spot's hit reactions does nothing (original).

## Revision 2 (2026-11-05): the round leaves on the next update

User: "when shooting normally there is a small delay between actually
shooting first and pressing the mouse button. in eduke it's absolutely RAPID".
Traced from a lowered gun: the game sees the press in update 1 and switches
the upper track to the raise without advancing it; update 2 ran the 4x raise
out and carried 16 units into the fire track; the fire key ended in update 3.
The upper advancer carries into the next track in the same call, so the
first update on the raise now advances the rest of the raise plus the fire
track's units to the end of its fire key (+1): the Desert Eagle the later of
track 16's 20 and the running track 10's 40. On a fire track while a press
waits, it advances to the end of the fire key. The round leaves on the update
after the game saw the press, as EDuke32's on the tic after.

Press to shot, 180 Hz, the player's settings (CPU SPEED 200), lab timing
(`ttk_lab.py timing`, `taps.py`):

| Weapon | Before | After, from rest | After, gun up | EDuke32 |
| --- | --- | --- | --- | --- |
| Desert Eagle | 72-102 | 58-70 | 71-82 | 58 |
| Combat Shotgun | 112-145 | 84-99 | (pump) | 123 |
| Gatling | 81-97 | 57-71 | 74-79 | 88 |
| RPG | 87-151 | 50-80 | 77 | 108 |
| Freezer | 79-108 | 50-66 | 73-81 | 81 |

Desert Eagle taps: 50 ms 55-67 ms, 16 ms 20 of 20 fired in 55-66 ms; running
forward or sideways 52-72 ms (one 98). Held rates unchanged (6.0, 9.99,
7.49 per s). Not measured: the first shot in water and on climbs. The lab
times from the test input file, so the host's mouse path adds its own small
delay on the real machine.

