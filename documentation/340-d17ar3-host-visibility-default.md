# D17AR3 part 3 - Host visibility on by default, level 28, regression sweep

2026-10-30. Third D17AR3 item, from the "Remaining" list of
[note 339](339-d17ar3-redraw-only-live-frames.md). Status: accepted 2026-10-31
after the playtest of revision 2 (last section).

Evidence is local under `recomp/analysis/d17ar3p3/` (`l28probe.py`,
`sweep.py`, `sum.py`, `routes.py`, `s3walk.py`, `s3r120.py`, `vanilla.py`;
logs, JSON and shots). Every run used private copies of the D17AQ fixture card
and savestates, offscreen video, dummy audio and private debug ports, and
hash-checked the player's saves and profile before and after.

## Changes

- **Default switch.** Host world visibility (D17AR2) is on by default in
  Modernized with Extended draw distance, unified geometry and the near path
  live; the player's normal launch engages it with no environment value.
  `DNTTK_HOST_VISIBILITY=0` (developer) turns it off; console
  `hostvis on|off|default` still overrides it for the session and reaches the
  redraw workers. The session log line no longer calls it a developer switch.
  Vanilla and Original draw distance never reach it (unchanged).
- **Level 28.** The draw-distance code refused the level's limits on every
  frame, so neither the extended limits, D17AQ open sky nor the host pass ran
  there. Cause: the level's own alternate room/prop limit (`0x800da34c`) is
  69511, already past the 0xffff GTE depth range, and the sanity check
  rejected any alternate limit of 0xffff or more. Its fade start is 32768 and
  mesh limit 37840 (both normal). The check now accepts an alternate limit up
  to 0xfffff, and `extend_limit` keeps 69511 as the game sets it (it never
  lowers a limit); the mesh limit is raised to 0xffff as in every other level
  (`draw_distance.inc`). Level 28 now has 0 refusals after its loading frame
  and draws about 61 host pieces a frame, none handed back.

## Regression sweep

**Levels (60 Hz, matched views).** All 21 selectable levels, five views each
(spawn, three turns, a walk), host off, on and off within about 0.6 s in one
process, in five configurations: FOV 90 16:9 first person (with the
piece-data proof), FOV 60 4:3, FOV 120 16:9, FOV 90 16:9 third person, FOV 120
4:3. That is 525 views:

- Host pass engaged in every view, level 28 included; no host refusals, no
  pieces handed back, no draw-distance refusals after load.
- No world piece data changes in any level (proof 2 repeated).
- 8 views differ beyond off/off noise. Each was checked by eye: an enemy
  appearing and firing (level 0), a projectile and blood (levels 23, 26),
  the known level-9 explosion, flame and creature animation and Duke's idle
  pose (levels 1, 2, 12). No geometry is missing or added wrongly in any of
  them.
- Level 28: host on matches host off in every view. In some views its sky is
  drawn as a box with black above and beside it; host off shows the same, so
  this is the existing sky drawing there, not visibility (noted for later).

**Fixture routes (player-like: scale 4, the default build, no environment).**
All 10 fixture slots (slot 10 is at jetpack height), routes still, turn and
walk (turning and walking alternated), 8 s each:

| Run | Seconds at full rate | Game fields/s (min) | Host engaged | Failed redraws | Host / draw-distance refusals | Pieces handed back |
| --- | --- | --- | --- | --- | --- | --- |
| 120 Hz, FOV 90, 16:9 | 180 / 180 | 59.7 | 30 / 30 | 0 | 0 / 0 | 0 |
| 120 Hz, host off (`DNTTK_HOST_VISIBILITY=0`) | 180 / 180 | 59.8 | 0 / 30 | 0 | - | - |
| 60 Hz, FOV 90, 16:9 | (no redraws at 60 Hz) | 59.7 | 30 / 30 | 0 | 0 / 0 | 0 |
| 120 Hz, FOV 120, 4:3 | 180 / 180 | 59.7 | 29 / 30 | 0 | 0 / 0 | 0 |

Redraw-only live frames engaged in every 120 Hz route and never at 60 Hz.
The one route without host frames (slot 3 walk, FOV 120 4:3) was Duke being
killed by an enemy during the preceding turn route: the death camera is the
original camera, first person is not live, and at 4:3 there is no widescreen
near path, so the game's own walk draws as before (repeated with per-second
counters and shots, `s3r120.py`). At 16:9 the widescreen near path keeps the
host pass on.

**Vanilla** (`--vanilla`, fixture slot 1): draw distance `original`, the
draw-distance and host passes run 0 frames.

**Native tests:** `ttk-near-test` and `ttk-input-test` pass. No framework
files changed (no runtime patch export needed), no hashed codegen header
touched; fixture savestates load.

## Not automated

- The player's own launch and saves at their settings (playtest).
- Flying the jetpack up and down (slot 10 views are hovering at jetpack
  height), pause/resume, inventory and level transitions with host visibility
  on by default.

## Remaining for D17AR3

- Playtest of the default switch.
- Then "how far we can push this renderer" and its name (user's request).
- Level 28 boxed sky (pre-existing, not visibility): to scope separately.

## Revision 2 (2026-10-31): club corruption and crash, stuck muzzle-flash light

User playtest of the default switch: performance and visuals "looking great",
with regressions: firing lights the ground around Duke and the light never
goes off (slot 12, the medieval level; any slot); walking into the club in
slot 4 garbles the screen (lines, the whole texture page), the frame rate
drops, and the game halted at 06:40 (session log: `FATAL: GPU GP0
unimplemented command 0x1F`). Host check for that window: no backup,
update or other load (last Timeshift snapshot the previous morning); the
slowdown was the game's. Evidence is local under `recomp/analysis/d17ar3p4/`
(private copies of the player's slot 4 and 12 states, which match the D17AQ
fixture; `hold.py`, `ctl.py`, `variant.sh`, `clubperf.py`, `burst.py`; shots).

### Club: one packet region per displayed frame

Reproduced offscreen: through the club door the screen went white, then
black, then a mass of lines, then halted (GP0 0x1F or 0xFD) or the watchdog
aborted a hang. Not caused by this part: the same happened with host
visibility off, redraw-only off, both off and with Original draw distance,
and with the D17AR2 build; the build before D17AR2 drew the club correctly.
The D17AR2 change that applies with host visibility off is the host packet
arena: two 256 KB regions instead of four of 128 KB.

Cause (trace of every region change): the region rotates at the composition
and also whenever a mesh pass starts on an empty OT. In the club two more
passes do (the world renderer called from `0x80037c54` and the object filter
from `0x80038c94`, about 29 and 24 KB each), so a frame took three regions.
With two, the frame's third pass overwrote its first pass's packets before
the GPU drew them; four regions had left a spare, which is why the club was
fine before D17AR2. The arena's own design rule (frame N's lists are done
before frame N+2 is built) assumes one region per displayed frame.

Fix (`near_clip.cpp`, `frame_replay.cpp`): the first reset after a flip
(`0x8001fcbc`) picks the frame's region; later resets before the next flip
keep appending to it. The flag travels to the redraw workers in the frame
state, and a savestate load counts as the frame's choice. A frame that
needed more than one region falls back to the original renderer for the rest
(the existing budget rule) instead of corrupting.

Results (offscreen, default switches): the club draws correctly through the
door and inside (`fix1-*.png`); no halt or hang in repeated walks. At the
player's settings (120 Hz, scale 4, god mode, walking and turning inside for
16 s): 16 of 16 seconds at full rate (118-122 presents/s, 59-60 game
fields/s), no failed redraws. The broken builds fell to 23-56 frames/s there,
the GPU walking garbage lists.

### Stuck muzzle-flash light

Reproduced on slot 12: after one shot the floor around Duke stayed lit
seconds later and after turning; only with host visibility and redraw-only
live frames both on. Cause: the mesh renderer clears each vertex's
colour-index nibble as it reads it (`0x80011170`), and lights such as the
muzzle flash set that nibble on the vertices they reach. The host emission
applies the clear, but a redraw-only live frame emits no pieces, so the marks
were never cleared in live RAM, and every redraw worker copied that RAM and
drew the light. Fix (`host_visibility.inc`, `near_clip.cpp`): a redraw-only
live frame applies just that clear to each selected piece in a non-flat room
(the same condition as the emission). After the fix the light is gone after
the shot; a firing burst looks the same as with host visibility off (shots
and impacts; the one-frame flash itself falls between captures either way).

### Checks after revision 2

- Canary (redraw-only, `PSX_LIVE_SKIP_CANARY=1`) over all 10 fixture slots,
  pause, thumbnail and reloads: no stale image shown.
- Level sweep, FOV 90 16:9, 21 levels x 5 views: all engaged, no refusals or
  pieces handed back; above noise only level 12, whose own brazier light
  flickers by 4-15% of pixels between host-off shots.
- 120 Hz fixture routes (10 slots x still/turn/walk, scale 4): 180 of 180
  seconds at full rate; one failed redraw in one run (slot 10 walk), not
  repeated in two further 15 s runs of slot 10.
- `ttk-near-test` passes; no framework files changed.

### Playtest of revision 2 (2026-10-31): accepted

User: no garbled screen walking into the slot-4 club, no stuck muzzle-flash
light; the club back room "this rendition is perfect"; slot 10 "not one
single indication of slow down"; "I'm extremely happy with the work and I
accept this". Follow-up jobs from the same playtest: D19U2, D17AT, D08J6,
D36M (board).

### Remaining
- As before: pushing the renderer further and naming it; level 28 boxed sky.
