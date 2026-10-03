# D17A - High-refresh instability: first investigation (2026-10-02)

Historical engineering record. D17A and D17B were explicitly accepted on
2026-10-03; [note 86](86-d17-acceptance-and-regression-baseline.md) supersedes
the parent-job statuses and source-publication policy below.

User report (D17A job entry): at 180 Hz, parts of the scene vanish for a moment
and show black, popping and flicker near the eye (apartment after the light
switch, wardrobe, alley platform, street pavement, subway edges, club stairs
and bar stools), slight overall shimmer.

Harness: `recomp/analysis/d17-high-refresh/` on the real GPU offscreen, private
profile and card copies, the player's settings where it matters (first person,
16:9, 4x or 2x, 180 Hz, CPU 100%). New card copy `cards-club` (strip club main
room, from the D12 private cards).

## What was ruled out (with evidence)

| Hypothesis | Test | Result |
| --- | --- | --- |
| In-betweens draw different game state than the real frames | `DNTTK_REPLAY_TEST=alpha1` (every in-between must equal the next real image), `a1slots.sh` over 12 savestate slots, subway, apartment, club, at 150% and 100% CPU | Equal everywhere except known 30 fps content: muzzle-flash lighting, fire and neon animation, blinking barrier lights |
| Redraws are not faithful | `DNTTK_REPLAY_TEST=interp_off` (alpha 0), apartment, street, club, about 1800 redraws | Pixel-identical to the real image |
| Texture or palette uploads between a frame and its in-betweens | GP0 opcode counts while walking (street, alley, subway, apartment, switch press) | 0 CPU-to-VRAM uploads during play |
| Out-of-order presents | Present trace (`backsteps.py`), club at 100% CPU | Index and game frame always monotonic |
| A redraw drawing into or capturing the wrong buffer | New runtime counter `multi_area` (sessions whose primitives hit more than one draw area) | 0 |
| Worker state left from a previous job | Single worker; alpha 0 runs after several worker re-forks | Same behaviour; alpha 0 exact |

The 12 s "starvation watchdog" aborts seen in harness runs come from
`replay_dump` writing 24 large PNGs on the emulation thread (4x); the harness
now raises the watchdog limit. The player's session logs show no such abort.

## Reproduced: near-wall flicker, first person, 16:9

In the club, walking and turning against the orange-framed wall panels at
(5357, -1900) (`cwtest.py`), consecutive presents go back and forth: an
in-between matches the real image, the next one shows a near wall feature
(the orange frame) moved or missing, the next matches again. `altcheck.py`
counts presents nearer to the one two back than to the previous one.

| Configuration (club spot, 180 Hz, CPU 100%) | Back-and-forth presents per 18 sequences |
| --- | --- |
| Default (conservative near clip, 16:9) | 8 to 22 (several runs) |
| `DNTTK_NEAR_MODE=full` | 0 |
| Widescreen 4:3 | 0 |
| `DNTTK_NEAR_CLIP=object` (world near clip off) | 0 |
| `DNTTK_NEAR_CLIP=world` | 12 |
| `DNTTK_REPLAY_TEST=alpha1` | 0 |

Per-job logs (`DNTTK_REPLAY_LOG=1`) show the interpolated camera is monotonic
and the game uses it unchanged; the near clip takes the same polygons in every
job (here two very wide wall polygons, emitted as pieces in ordering-table
slots 9 and 10 with stable corners). The intermediate images themselves differ
in which near-wall polygon is on top: the conservative world near clip links
its pieces into a slot before the original renderer adds the rest of the mesh,
so they draw last within that slot, and a neighbouring original polygon whose
key sits on a slot boundary (nearest corner SZ >> 5) changes slot with a
one-unit rounding wobble of the eye. Real frames do the same, but only 30 times
a second; at 180 Hz the flip shows up to five times per game frame.

Not yet a fix:
- Moving the pieces one slot further back (`DNTTK_NEAR_WORLD_BIAS`, removed):
  no change.
- Linking the pieces at the chain close so they draw first in their slot
  (`DNTTK_NEAR_DEFER`, removed): fewer back-and-forth presents in the club,
  but near walls in the apartment and street vanished (see-through to the
  alley). Rejected.
- `DNTTK_NEAR_MODE=full` removes this flicker but keeps the dotted seams and
  prop-bias problems that made D11B choose conservative.

Other observations not explained yet: one real (not in-between) frame in the
subway with a black block at the right widescreen edge (`sb2120`, once).

## Kept in the build

- The near clip's frame accounting (packet budget, held weapon packets) ships
  with each redraw job (`near_clip_frame_state`), so a worker starts where the
  live frame did. No measurable effect on this flicker; it removes a source of
  worker-dependent decisions. `DNTTK_REPLAY_NEAR_STATE=0` (developer) restores
  the old behaviour.
- Diagnostics: `DNTTK_REPLAY_LOG=1` (per job: alpha, requested and used camera,
  near clip counts), runtime `render_replay` fields `multi_area` and
  `multi_last`.
- Alpha 0 rechecked after the change: apartment, street and club, 18 sequences,
  all exact. Runtime patch regenerated (17 files). Binary
  `04ec89349eb4064cd0623755f18d551ebb0229dc494d3cf2f0f35b0fb9f9cf97`.

## Not reproduced yet

The apartment with the lights off (the scripted E press did not trigger the
switch), the alley platform, the club stairs and stools, the street pavement.
Savestates (F7) taken by the player at those places would make them testable;
copies go into private harness folders, never the player's cards.

## Next step

Make the conservative world pieces sort stably against same-slot original
polygons without moving them under farther geometry: for example insert each
taken polygon's pieces at the position the original would have inserted that
polygon (its index in the mesh list), or give near wall pieces and the
polygons coplanar with them one consistent key. Then rerun `cwtest.py`
(target 0), the apartment and street A/B stills (`deferab.py`, no vanished
walls) and alpha 0/alpha 1.

## Harness added

`holes.py` (one-present black holes), `a1check.py` / `a1slots.sh` (alpha 1
consistency), `a0check.py` (alpha 0), `altcheck.py` (back-and-forth presents),
`backsteps.py` (present trace order), `cwtest.py` (club wall spot), `screen.py`,
`walkscan.py`, `roam.py`, `goto.py` (walk to x,z), `crops.py`, `sheet.py`
(contact sheets, flipped upright: dumps are stored bottom-up), `deferab.py`
(still A/B). `steer.face()` pitch sign fixed; `d17.py down` now ends a harness
game that refuses `quit`.

## Second pass (2026-10-02): user savestates, precise near geometry, late camera

User test (fresh game, strip club, 180 Hz, CPU 100%): mouse turning slower and
heavier while the two dancers are on screen; reproduction savestates UI 4 to 9
(F7 UI slot N is file slot N-1): chair through a table, black voids in a wall,
a wall warping while walking beside it, a wall turning black, the closet
see-through. Private copies in `recomp/analysis/d17-high-refresh/cards-user2`
(savestates only, no player cards).

### Mouse slowdown with the dancers (D17B)

Measured, not assumed: the emulation keeps 60 fields per second, and turning
speed is unchanged (14.4 degrees per second for the same mouse input facing the
stage or away). What changes is TTK's own frame rate: at original CPU speed the
dancers push its frame past two fields and it runs at 20 instead of 30 game
frames per second (150% CPU keeps 30). In-between images blended two game
cameras, so mouse movement reached the screen only with the next game frame:
step response (one mouse step, first present that moved, `latency.py`) about
66 ms at 30 game fps and about 95 ms at 20.

Late camera (default; `DNTTK_LATE_CAMERA=0` restores the blend): every
presented image is a redraw whose view rotation is the newest mouse look,
read from the input pumped so far plus SDL motion still queued (peeked, not
consumed); the eye position, Duke and objects stay interpolated, and gameplay
keeps its own 30 Hz camera and aim. Jobs are submitted about 12 ms before their
present (`DNTTK_LATE_LEAD_MS`); a redraw at alpha 0 replaces the real image,
which would swing the view back each game frame; the nth present of a game
frame shows in-between n; when an image is not ready the last one repeats
instead of the older real image. First person rotates about the eye, third
person about the orbit pivot. Result: step response about 17-19 ms at both 30
and 20 game fps; presented yaw monotonic during turns (0 backward steps, 0
real-image fallbacks).

### Near geometry: host precise vertices (D17A)

The user states showed the cause of "warping": large polygons beside the eye
are mapped affinely by the PS1 (the club table's stripe bent into a chevron).
The near clip's full mode fixed that by subdividing, but D11B kept it off for
dotted seams. New runtime channel `psx_mod_gpu_host_vertex(addr, word, x16,
y16, z, u, v)`: a plugin gives each vertex word of a packet it wrote its exact
sub-pixel position, view depth and texture coordinates; the GPU uses them for
that triangle (sub-pixel corners and perspective-correct texturing), validated
by the word, whatever the PGXP settings. Live frames and redraw sessions keep
separate tables (8-way probed); a redraw worker records the entries and the
packet addresses in its GP0 stream (items 4 and 5) and the feed installs them.
The near clip registers every piece: new corners carry the GTE snap error of
their edge (interpolated in screen space) so piece edges follow the original
renderer's integer edges exactly, which removed the black crack slivers; with
exact texture coordinates the GL path no longer clamps to the packet's rounded
UV box (that clamp caused offset texture blocks). Pieces are split only to keep
their sort depth local (`DNTTK_NEAR_SPLIT_PX` 192, `DNTTK_NEAR_SPLIT_RATIO`
1.5), never along texel lines.

Full mode with precise vertices is now the default
(`DNTTK_NEAR_MODE=conservative` and `DNTTK_NEAR_PRECISE=0` for comparison).

| Check | Result |
| --- | --- |
| Table (UI 4) | straight perspective stripe, chair no longer through the table |
| Bookshelves, wall, closet (UI 6, 8, 9) | correct perspective, no slivers, no see-through |
| Club wall flicker (`cwtest.py`) | 3 back-and-forth in 27 sequences (default before: 8-22 in 18), remaining ones normal turning |
| Apartment and street, 8 headings | no regressions; a diagonal artifact on the street facade gone |
| Alpha 0 (redraw = real image) | exact in apartment, street, club, UI 4 state |
| Gameplay equivalence 60 vs 180 | club driven input 631/631 identical in three runs; no-input identical; the street run differs between 60 Hz runs too (known synthetic-driver weakness) |
| Python suite, native tests | 97 OK (with the .bin image); `ttk-near-test` (fixed: link stubs), `ttk-controls-test`, `ttk-aim-test` PASS; `ttk-input-test` needs a desktop window |
| Vanilla route `d17a-vanilla-1` | exit 0, captures normal |

Cost (offscreen, GPU at idle clocks, 4x, 180 Hz, CPU 100%): guest 60 fields per
second everywhere; distinct images per second 93-180 (street side headings
lowest, club 138-177, subway 120-180); about 56% of a redraw's main-thread time
is the GL driver. The late camera's redraw budget cap is 8 ms per field (4
before).

Not reproduced: Duke stuck after loading UI slot 3 (file 02). The session log
shows animation 105 (jump landing) with the modern lease inactive right after
that load; in the harness the same state jumps, lands and walks in every
direction. Needs the exact actions after loading.

## Playtest regression and late camera default off (2026-10-02, evening)

User: a new one-player game at Match Display was "extremely jerky, not
playable". Reproduced from boot (fresh private cards, new game, first street):
at CPU 100% this area runs the game at 20-30 fps and the emulation thread has
little spare time; redraws reach only about 100 distinct images per second on
the real display even with the D17 settings, and fewer with the late camera,
which needs a fresh redraw for every present and otherwise repeats the last
image (heavy judder). The late camera is therefore off by default
(`DNTTK_LATE_CAMERA=1` enables it). Fixes kept from that work: late view only
while the modern camera owns the view (`last_lease_cam`, camera target Duke),
100-degree guard, in-between lag 3 in late mode (pair k-1 to k, prepared a
frame ahead), nth present shows in-between n, redraw budget cap 8 ms in late
mode, 5 frame slots (runtime 6 worker slots).

Next for D17B: present-time reprojection (rotate the last finished image to the
newest mouse view on the GPU at each present), so turning follows the mouse at
the display rate without needing a redraw per present.

## Third pass (2026-10-02, night): user states UI 1-5, late camera back with pacing

User test (third playtest, strip club and apartment, Match Display 180 Hz,
CPU 100%) with savestates UI 1-5, private copies in
`recomp/analysis/d17-high-refresh/cards-user3` (files 00-04).

### Closet triangles around the medkit (UI 3) - fixed: host depth buffer

Stray triangles of the closet's back drew over the medkit and shelf: the full
near clip's pieces, split for sort depth, sorted into ordering-table slots
that disagree with the original polygons around them. Host triangles (those
with precise vertices) now also write and test a depth buffer (`GL_LEQUAL`,
depth `1-16/z` from the exact view depth); everything else draws as before
(no depth test, PS1 order). The depth is cleared with the GP0 fill and the wide
clears, and copy-on-write blits carry it. The first-person weapon registers its
pieces with a negative depth (perspective, no depth test), so walls never cut
it. `PSX_HOST_DEPTH=0` (developer) turns it off. Result: closet correct in the
UI 3 state (standing and turning); earlier UI 4-9 states unchanged.

### Woman's neck artifact (UI 4) - fixed: sampling limits from exact UVs

Texel noise at her collar: the exact texture coordinates sampled past the
polygon's texels because the GL sampling window was the whole texture page.
The window is now computed from the triangle's exact UV range. `PSX_HOST_UV=0` (developer) returns to the packet UVs.

### Missing apartment wall, lights off (UI 5)

With the current build the wall is present in every dumped present while
standing, turning and walking along it. It was the conservative near
clip's slot ordering fixed in the second pass.

### Idle breathing world (UI 1) - D17C, fixed: stable first-person eye

Measured: standing still, Duke's idle animation lifts his root about 18 units
and back; the first-person eye followed the root, the game's look-at
(`0x8002A524`) turned the view toward a fixed point, and with the floor close
under the eye the whole world appeared to breathe. The PS1 wobble itself was
not the cause. The eye now takes its height from the camera pivot
(`player+0x7bc`, height at `+0x7c0`, level during idle and the gait) and x/z
from the root through a dead band (24 horizontal, 64 vertical), so idle and
gait motion no longer move it while crouching, jumping, steps and drops still
do at once. Measured idle: eye position exactly constant (2928, -10011, 5981);
the view rotation varies by only a few units of 4096, from the game's own
look-at. Geometry wobble while moving is unchanged (not removed globally).

### View bob - new deliberate system (D17C)

A separate, intentional bob driven by distance walked (dip up to 8 units twice
per 640 units walked, side sway 3 once; strength follows speed, full at 24
units per frame and up to 1.6x running, easing out when Duke stops).
Setting: Modernized `view_bob` = off / subtle / on (default) / strong, launcher
`--view-bob`, settings menu `B`; `DNTTK_VIEW_BOB` also takes a numeric scale
for tuning. Vanilla keeps the original camera.

### Mouse in the strip club (UI 2) - D17B: late camera on, adaptive pacing

Measured at the dancers: TTK runs 15-20 game frames per second at CPU 100%
(emulation 60 fields/s); the mouse is applied directly as angles (0.12 degrees
per count, no analogue emulation, sensitivity unchanged); turning rate is
unchanged, but each step reached the screen only with the next game frame
(66-130 ms). The cost is the game's own frame, so the fix is to stop tying the
view to it.

Late camera is the default again (`DNTTK_LATE_CAMERA=0` restores the blend):
every present is a redraw whose view rotation is the newest mouse look (SDL
motion pumped plus peeked). What made the first version jerky was presents
that had no fresh redraw and repeated the last one. Now adaptive pacing
measures the repeat share over 90 presents and, above 25%, presents a new
image every 2nd or 3rd refresh (evenly, never a stutter of repeats); it steps
back after four windows under 3% (`DNTTK_LATE_PACING=0` off). On the real
display at 180 Hz: club UI 2 settles at every 2nd refresh (90 presents/s, 70-92
distinct), the fresh opening street at every 3rd (60/s, every one moving
evenly). Step response about 17-19 ms in both (was 66-130 ms).

`frame_pacing.c` waits with a spin: the vDSO time in profiles is idle, not
work. The opening street is expensive for the emulator (overlay dispatch) and
limits redraws in every mode.

### Checks (final binary)

| Check | Result |
| --- | --- |
| Alpha 0 (club, driven input, 12 sequences, about 200 redraws) | exact |
| Vanilla route `d17a-vanilla-2` | exit 0, captures normal |
| Python suite (with the .bin image) | 98 OK (profile schema 22, view bob tests) |
| `ttk-near-test`, `ttk-controls-test`, `ttk-aim-test` | PASS |

Runtime patch regenerated (17 files). Binary
`b1193a90c1f25d87f79ba8257d7926745127a3a6e7dc02faa56e9472a24f4d68`.

Open: Duke stuck after loading the earlier UI 3 (second pass) still not
reproduced; the exact actions after loading would make it testable.
Present-time reprojection (rotate the last image to the newest view on the
GPU at each present) remains the next step if paced presents at every 2nd or
3rd refresh still feel heavy.

## Fourth pass (2026-10-03): see-through regression, dancer jerk

User test (fourth playtest): idle stability accepted, collar fixed, club
slowdown better; new: sky through the club doorway (UI 1), building, tree and
statue popping (UI 2), apartment see-through (UI 4), and a pop/jerk sweeping
the mouse across the dancers (UI 3). Private copies `cards-user4` (files
00-03).

### See-through (UI 1, 2, 4) - fixed: depth cleared per frame

A/B stills: the holes appear only with the third pass's host depth buffer
(`PSX_HOST_DEPTH=0` and the conservative near clip render correctly). Cause:
the depth was cleared only with GP0(02h) fills and wide clears. Outdoors TTK
does not clear its frame (the sky covers it), so depth from an earlier frame,
drawn with another camera, stayed in the buffer and new walls and props failed
the test: sky through buildings, the tree and statue cut away, popping as the
camera moved. Now each new drawing area (GP0 E3/E4, once per frame) and each
redraw session marks the depth for clearing, and the clear (depth only, the
drawing area in the canonical surface and the rows of the wide surface) runs
before the next host triangle; a session saves and restores the mark. New wide
surfaces also start with cleared depth. `render_replay.host_depth` reports
[depth-tested triangles, clears].

| Check | Result |
| --- | --- |
| UI 1, 2, 4 stills | identical to depth off (sky, tree, statue, apartment walls solid) |
| UI 2 / UI 1 / UI 4 while turning and walking (4 sequences each) | no see-through in any present |
| Closet (third pass UI 3) | still correct (stray triangles only with depth off) |

### Dancer jerk (UI 3) - D17B

Measured with the present trace (now with present time and the planned alpha
of the shown image):

1. Position sawtooth under pacing. A game frame's in-betweens were planned per
   display refresh; with pacing at every 2nd or 3rd refresh only the first
   third or half were shown, so Duke, the dancers and every moving object went
   0 to 0.33 of the way and then jumped the rest at the frame change. Pacing
   drops at the dancers, so this started exactly there. Now the plan uses the
   paced present rate (steps per frame from hz / pace_div).
2. Frames longer than planned. The plan covers the previous frame's interval;
   at the dancers frames alternate between 2 and 3 fields. When the plan ran
   out, the last image repeated with an old mouse look and the view caught up
   at the next frame. Now, while the game has not issued its next display flip
   (its DISPENV still equals the shown one), a fresh redraw is kept two
   presents ahead with positions held at the frame's end (alpha 1) and the
   newest look (`late_overruns`; `DNTTK_LATE_OVERRUN=0` off).
3. Finished images refused at their own present: the redraw budget (token
   bucket) also governed the present that wanted the image. A late-camera
   image finished in a worker is now drawn at its present regardless
   (`DNTTK_LATE_DUE_FREE=0` off); the budget still governs preparation.
4. Submission lead 16 ms (was 12): real display, 4x, 180 Hz, real mouse sweeps
   across the dancers (`realsweep.sh`, `staleclass.py`): about 3.7% of moving
   presents repeated an image at 16 ms, about 7% at 12 ms.

Ruled out: turning ahead of the game camera does not hide actors at the
screen edge (redraws turned 25 degrees by `DNTTK_LATE_TEST_YAW` match turning
for real), so there is no visibility pop. The harness's synthetic mouse moves
once per field, so its sub-frame smoothness numbers are not meaningful; real
X input (`realsweep.sh`) is used for those.

Real display, 4x, 180 Hz, CPU 100%, real mouse sweeps: emulation 60 fields/s;
UI 3 dancers 2.6-5.2% stale moving presents at full rate (some runs drop to
every 2nd refresh after the load, 13%); UI 2 0.3%; UI 4 1.1%; UI 1 9.7% with two
pacing changes. Game 23 fps at the dancers, 30 elsewhere.

Remaining limit: a present repeats when the redraw is not finished in time
(worker latency at 4x). Present-time reprojection would remove it but would
also shake the HUD and weapon, which are drawn into the same image; it needs
the world and the overlay captured separately.

### Checks

| Check | Result |
| --- | --- |
| Alpha 0 (late camera off, driven input): club 8, apartment 8, doorway 8+20+20 sequences | exact (one doorway sequence once compared a redraw with the previous game frame's image; 40 reruns exact) |
| Vanilla route `d17a-vanilla-3` | exit 0, captures normal |
| Python suite (with the .bin image) | 98 OK |
| `ttk-near-test`, `ttk-controls-test`, `ttk-aim-test` | PASS |

Runtime patch regenerated (17 files). Binary
`f9f64faa4e939326b5a10f4eb0b6cbdaa1d31d47af979a25100986f6378865ea`.

## Fifth pass (2026-10-03): loss of control, jerkiness, late camera rebuilt

User test (fifth playtest, fresh game, 180 Hz, 4x, CPU 100%): turning jerky
from the start; Duke lost all movement after loading UI 4 and stayed frozen
after loading other saves; tree/statue (UI 2) and apartment see-through (UI 4)
confirmed fixed; a small artifact on the table by the club exit (UI 1) and
closet popping (new UI 6) reported. Private copies `cards-user5` (files 00-05).

### Loss of control (D17B/D08 regression class) - fixed

The session log showed the lease refused with `state` and player flags
`0x202`. `+0x224 0x200` is set by `0x8004063c` in `0x80040584`, the game's
tap-or-hold handling of its use/holster button (Circle by default): a per-button
press history (16 words from `0x800d1444`, one per pad bit, shifted each game
frame by `0x8001d320`) is summed over the frame
times of consecutive presses (`0x8001e5b4` with the per-frame time ring
`0x800d2218`); at 100 ticks the hold-to-select inventory mode starts (`0x200`,
Duke stops) and only the button's release edge (history `& 3 == 2`) clears it
and uses the item. The Modernized layer sent Circle as a hold: for as long as
fire was held to draw a holstered weapon, and as 4-6 frame pulses for E stow
and weapon restore. At 15-20 game fps those crossed the threshold (reproduced
at the dancers: holster, hold fire, flags `0x306`, lease `inactive (state)
flags=00000200`). Host Circle requests are now one tap: Circle is pressed until
the game's history word (`*(0x800d1a90 + 4 * player+0x233)`, the one that
routine reads) shows it sampled, then released until the request ends (8 input
frames at most). A bound holster key stays raw, so holding it opens the
original inventory hold as before. Checked: holster then fire click or 1.5 s
fire hold draws the weapon, no `0x200`, E unchanged.

Found on the way: after a savestate load the Modernized camera kept the
previous yaw and turned Duke away from his saved facing within 0.2 s (loading
file 4 after starting on file 3: W walked into an obstacle). New runtime call
`psx_mod_savestate_loads()` (count of applied loads); on a change the camera
re-seeds from the loaded game's view and pending host requests (stow, restore,
push, queued jumps) are dropped. Checked: start on file 3, load 4, 5, 0, 4:
Duke walks every time (before: 0 units on 4 and 5).

### Jerkiness - late camera redraws rebuilt

The fourth pass made turning worse: its frame-end ("overrun") redraws were
appended almost every present (about 100/s), costing worker and main-thread
time, pushing pacing down and oscillating it, and holding positions at each
frame end. Measured instead of tuned:

- One redraw per present. `submit_due` now makes, about one lead before each
  coming present, a redraw whose positions are at that present's own time in
  the game frame (alpha = elapsed / interval, held at 1 while the next image is
  late); the provider shows the redraw made for the present. The change time is
  exact once the game has flipped (the next VBlank; the flip is issued 15-20 ms
  before it shows) and predicted otherwise, so near a change both frames get a
  redraw only when needed. Positions now follow time exactly (median shown blend
  minus time share 0.00).
- Prefetch feeds only the redraw for the coming present and frees worker jobs
  of passed presents (it used to spend main-thread time on images that would
  never be shown).
- The mouse look is taken at a fixed lead before each present from
  timestamped motion events (SDL3 event time), not "now" at an irregular
  submission time (`DNTTK_LATE_SAMPLE=0` restores now).
- Adaptive lead from measured worker latency (`lead_ms`, `ready_ms` in the
  status; `DNTTK_LATE_LEAD_MS` fixes it).
- Pacing with hysteresis: step to a slower rate above 10% repeated images per
  90 presents (was 25%), step back only after 4 clean windows under 2%, doubled
  each time the faster rate fails again within 3 windows (at most 32).

Real display, 4x, 180 Hz, CPU 100%, real mouse sweeps (`realsweep.sh`,
`holds.py`, 1 px per 1 ms): holds of two or more presents while turning 0.1/s
in the opening street and 0.6/s at the dancers with pacing; without pacing
(every refresh) 2-6.5/s, so pacing stays on. The synthetic mouse's coarse
moves dominate per-present smoothness numbers at 180 Hz (`yawsmooth.py`), so
those are not used for decisions.

### Not reproduced

- UI 1 table by the exit: still and turning/walking sequences at 285 degrees
  (48 presents, depth on/off, near clip off) show the table and its wooden
  plank stable; no back-and-forth presents.
- UI 6 closet: driven turning and strafing (6 sequences x 2 runs, default,
  depth off, conservative) show the same images; `altcheck.py` finds no
  back-and-forth presents in 54 sequences across the closet, apartment,
  doorway and statue. The fourth-pass frame-end redraws (positions held, then
  a jump) and the stale camera after loads are candidates for what was seen.

### Checks

| Check | Result |
| --- | --- |
| Alpha 0 (late camera off, club, 3 x 8 sequences) | exact in 2 runs; one 6x5 px patch in 4 redraws of the first sequence after the load in 1 run |
| Load sequence 3 -> 4, 5, 0, 4 | Duke walks after every load |
| Circle tap (dancers, holster then fire click / 1.5 s hold, E) | weapon drawn, no `0x200` |
| Vanilla route `d17a-vanilla-4` | exit 0, captures normal |
| Python suite (with the .bin image) | 98 OK |
| `ttk-near-test`, `ttk-controls-test`, `ttk-aim-test` | PASS |

Runtime patch regenerated (18 files: `savestate.c` added with its committed
baseline). Binary
`5f7406a6465570624fe129aaadcecee5960d80cca8707bab5fa1adbe642d2b39`.
