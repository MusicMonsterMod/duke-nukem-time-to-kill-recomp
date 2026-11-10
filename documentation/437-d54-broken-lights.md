# D54 - Broken lights go out

Status: **Done** (accepted 2026-11-10; playtested by the user in UI slot 6). Modernized Polymorph with DYNAMIC
LIGHTS only; Vanilla and Classic unchanged. Builds on the D41D1 sign lamps
([note 429](429-d41d1-signs-waves-levels.md)), the D41L prop lights
([note 436](436-d41l-saloon-light.md)) and the D53 screen lights
([note 432](432-d53-security-monitors.md)).

## Report

User, UI slot 6 (level 5 street): "In front of you is a barrel and two
traffic lights ... destroy all of them and you'll see that the lighting
effects remain ... when Breaking an object that Emits light then the light
should also get destroyed ... that's actually game wide".

## How TTK breaks a prop (CONFIRMED in play, level 5)

Shot in a private copy of UI slot 6 (file slot 5): both traffic lights
(type 217, lying on the road, amber `sign` lamps under them) and the fire
barrel (type 220). The ground fire beside the slot (type 449) is not a
barrel and does not break.

- A static prop object (kind `+0x14` = 1) keeps its health at `+0x32`
  (traffic light 250, barrel 1250), from the fourth word of its type entry
  (type table `0x800d2660`, 28 bytes per type).
- At the hit that takes the health to 0, the object's word `+0x00` gains
  bit 2 (value 4). Across the street's 410 linked props `+0x00` is otherwise
  0, 1 or 1024.
- A little later the object is unlinked from its room's object lists (`+0x48`,
  `+0x4c`). Its memory (type, place, mesh `+0x44`) is left as it was.
- In the type entry, third word bit `0x10000` is set on the types that break
  in play (217, 220 and, on level 0, EXIT 171, the club sign 475, neon 16, 79,
  193, red lights 170, 213); woodburners (261), the chandelier (263) and the
  monitors (858) do not have it. Inferred, not used by the code: every prop
  is checked the same way.

## Our lights tied to a prop

| Source | Found | Before D54 |
| --- | --- | --- |
| D41D1 sign lamps (`sign type N` in the level files) | file, fixed place | stayed lit for good |
| Placed lamps (editor) | file | stayed lit |
| D41L woodburner fire, chandelier candles and fill | prop scan by model, every 300 frames | stayed until the rescan (up to 10 s at 30 Hz) |
| D53 TV screen light, power sign light | prop scan by model, every 300 frames | as D41L; the bounds writer kept writing into the unlinked object's memory |
| Painted-light scan lamps (`scan room N`) | world vertex colours | not tied to a prop |
| Drawn extras (flames, embers, keypad glow, TV mesh) | drawn with the prop | already gone with the prop |

## What is in

- `recomp/src/ttk/prop_state.h` (header-only): a snapshot of the linked
  static props, rebuilt at most every 5 ms and shared by the three light
  sources. A prop stands while it is linked, kind 1, bit 2 of `+0x00` clear,
  and keeps its type and place. Lights are bound by type and place, never by
  address alone (unlinked memory can be reused), so a savestate taken after a
  break keeps the light out and one taken before brings it back.
- The snapshot carries a fingerprint of the room table (room count and
  origins). A light set compares only against snapshots of the level it was
  found on: at a level change the next level's props are in place a moment
  before the lamp set reloads. Without this every lamp of the old level
  "broke" at each level change (924 log lines over the sweep; none visible,
  the screen is loading); with it, none.
- A light whose props have all broken sputters out over 12 ticks (0.4 s:
  lit for two ticks, then three ticks in four, fading) and stays dark. After a
  level or savestate load the state is taken as found, without the sputter.
- **Level lamps** (`level_lights.inc`): a lamp's note may carry `prop TYPE x
  y z` entries (a stack of props can share one lamp: any standing keeps it
  lit). Every bound lamp is checked each game tick, near or not, so a light
  broken out of sight does not sputter when Duke comes back. `lamp signs`
  writes the entries itself.
- **Every level file bound** (`lamp bind all`, on a fresh `level N` load of
  each level with a file): per prop type, the median model-space offset from
  each lamp to its nearest prop, then each lamp takes every prop of that type
  that would put a lamp within 700 of it. 922 sign lamps on 21 levels, none
  without a prop; only notes changed (lamp values byte-identical, checked).
- **D41L and D53**: each emitter keeps its prop; a break sputters it out at
  once; broken props are skipped by the scans; a savestate load rescans. The
  D53 bounds writer no longer writes into a monitor that has broken or been
  unlinked.
- Console: `lamp bind` (the selected lamp goes out with the standing prop
  nearest the crosshair, for placed lamps), `lamp unbind`, `lamp bind all`,
  `lamp breaks off|on` (session compare). Log: `[TTK lamp] lamp N goes out:
  its prop broke (...)`, `[TTK prop light] ... goes out`, `[TTK security] ...
  goes out`.

## Evidence

Private copies of the player's cards and savestates
(`recomp/analysis/d54/cards`, local), offscreen, debug port 9561; player
saves, cards and profile hash-checked unchanged after every run.

- UI slot 6: each traffic light and the barrel broken by gunfire; the log
  names each lamp at its break (`lamp 27`, `lamp 26`: type 217; `lamp 31`:
  type 220). Matched shots, same camera, `lamp breaks off` / on / all lamps
  off (`research/before-after/D54/barrel-trip.png`, `lights-trip.png`,
  local): the amber pools on the road and sidewalk and the barrel's yellow
  pool are gone; the building corner over the barrel keeps a faint orange,
  the same as with all lamps off (the original painted light, see limits).
- Savestates (`reload-pair.png`): the slot (props standing) shows the lamps;
  a lab save made after the breaks loads with them dark; no `goes out` line
  on either load.
- Level 1 (D41L): bit 2 set by hand on the saloon woodburner: `[TTK prop
  light] fire light goes out`. Level 0 (D53): the same on a monitor (type
  858): its sign lamp and its screen light both go out. (Code path only:
  these types do not break in play.)
- Level sweep: no `goes out` lines at level changes after the fingerprint.
- Build OK; `ttk-input-test`, `ttk-near-test` pass. No framework change.

## Limits

- **The original painted light stays.** TTK bakes its lamps' light into
  vertex colours; the barrel's orange on the wall above it (and any baked
  pool under a traffic light) is the original game's and is not changed. A
  follow-up could dim baked light around a broken emitter in the host world
  path; not done here.
- Painted-light scan lamps (`scan room N`) belong to no prop. A scan lamp
  that sits on a breakable fitting stays lit; `lamp bind` can tie one to it.
- Placed lamps are not bound automatically (level 0's club sign lamp, level
  1's stage pool); `lamp bind` does it per lamp.
- Real breaks confirmed for two kinds (traffic light, fire barrel). Signs,
  neon and red lights rely on the same routine; not each shot in play.
- Not measured on the player's machine (one prop-list walk per game tick,
  about 400 props).
