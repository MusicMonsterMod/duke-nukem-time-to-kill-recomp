# D08Y - Gap jump dead band (UI slot 5)

Status: **Done** (user-accepted 2026-10-01: jump made consistently; "the
stuttering and audio issues, and freezeframes are now fixed"). Research first, then the
change and evidence below. Modernized only; Vanilla unchanged.

## User report

UI save slot 5: Duke stands on a grated walkway facing a gap to a second
platform. A running jump from the very end of the walkway rarely makes it:
Duke "comically hits his head on the other platform and just falls down".
Starting the jump about 3-4 feet before the edge with E held works: Duke
catches the far edge and mantles up. The user reads this as an oversight in
the level design and wants more confidence.

## Setup

Private copy of the player's UI slot 5 (runtime slot 4,
`state_800AB6FC_slot04.pst`) in `recomp/analysis/d08y-gap-jump/cards`.
Modernized, private Xvfb display, real keys (xdotool), private profile,
`DNTTK_TRAVERSAL_TRACE=1`. Scripts: `look.py`, `edge.py`, `sweep.py`.
Shotgun drawn (equip 2), so every E case also exercises the D08J automatic
stow. Binary as built for D14 (no rebuild).

## Geometry

- Duke spawns at X 38414, Z 82881, heading 4086 (almost exactly +Z), standing
  on floor Y -6144 (root about -6650).
- Walking forward, the original stops him at Z 86725 with a 2048 drop ahead
  (`+0x1c4` = 2048). Below the gap is a lower floor (Y about -4100) and, further
  on, water (mode 5).
- The far platform's top is at the **same height** as the walkway (landing root
  -6645). Its front face is met at Duke's Z about 89740..89970 (his centre, about
  370 off the face).
- A running jump (Shift + W, anims 103/104) from the edge flies about 3100
  units and drops back to takeoff height right at the far face. The gap is a
  few dozen units too long for a jump without help.

## Measured outcomes (running jump, takeoff N units before the stop point)

Bounce column: Duke's feet below the far platform's top when he hits its face
(107).

| Back | E held | Result | Bounce: feet below top |
| --- | --- | --- | --- |
| 0, 100, 200 | yes | lands across (78) | - |
| 300 | yes | bounce 107, falls to the lower floor | 273 |
| 400 | yes | bounce, falls into the water | 374 |
| 500 | yes | bounce, lower floor | 408 |
| 600 | yes | bounce, lower floor | 535 |
| 700, 800, 900 | yes | ledge catch 148, hang 149 (pull-up available) | - |
| 0 | no | bounce, water | 34 |
| 100, 200 | no | bounce, lower floor | 57 |
| 300..900 | no | bounce every time | 144..779 |

- Without E Duke **never** crosses (0/10). From the edge his feet are only
  34..57 below the far top when he meets the face, and the original bounces him
  off a lip that small. That is the "hits his head" fall.
- With E, takeoff 0..200 back lands across; 700..900 back catches the ledge
  (the user's working technique). Between them, **300..600 back is a dead band**:
  feet 270..540 below the top, too high for the hang acquisition and too low to
  land. Duke bounces whatever he holds.
- All three catches came from the D08X angle-forgiveness retry
  (`ttk-lift-catch lift=0 turn=290`): the original acquisition at Duke's real
  heading (4086) refused the far face. Why it refuses straight on is not yet
  known (possibly the face is not square to +Z, or the probe point).
- The landed-across E cases likely owe their success to the reach pose (109)
  replacing the running flight (104); not yet confirmed which part of the reach
  makes the difference.

## Why it happens

- The original airborne rules have exactly two outcomes at a face: land if the
  feet clear the top, or reach acquisition (`0x80055208`) for a hang when the
  ledge is above the probe point (root + 425 up). Anything in between is the
  impact 107 bounce.
- D08X added, for jumps with E: early arming of the reach, lifted/turned retries
  of the hang acquisition, and a mid-air **height mantle** (134..139). That
  mantle only accepts **climbable objects** (flags 0xc0, crates). This far edge
  is level geometry, so the dead band falls through to the bounce.
- The band in which the bounce happens (feet 34..540 below the top) is the same
  range the original ground mantles cover (134..137 for `+0x1c4` of
  -0x100..-0x33f). Walking into the wall with W + E would mantle it; jumping into
  it does not.

## Options considered (research stage)

1. **Geometry air mantle with E (recommended core).** Extend
   `ledge_reach_mantle()` (`recomp/src/ttk/ledge_reach.inc`) from objects to
   wall ledges: during an E jump/fall or the first updates of a bounce, when the
   contact is level geometry, ask the original ground obstacle/ledge probe for
   the ledge height above the feet and start the matching original mantle
   134..137 (139 above 0x33f), after the original line-up. First find the
   geometry path the walk-in W + E wall mantle uses (`0x80051cf0` is the object
   path; the 1024 wall beside slot 12 mantles with 139/140 from the ground).
2. **Low-lip step-up landing, with or without E.** When a descending jump meets
   a face whose top is at most about 0x100 above the feet (the 34..144 cases),
   finish as a landing on top instead of 107. This is the classic
   "ledge forgiveness" of modern games and fixes the from-the-edge jump without
   E. Modernized only; a Vanilla bounce stays.
3. **Lowered hang retries (fallback).** Add negative lifts (-160, -320) to the
   D08X retry so the original hang acquisition catches the dead band; the
   existing settle and pull-up then run. Simpler, but Duke drops into a hang
   before climbing, which reads worse than a mantle.
4. **Not recommended:** stretching the jump distance or editing the level.
   Changes the original physics everywhere, or touches original media.

Also worth a look: why the straight-on hang acquisition refuses this face
(only the turned retry catches).

## Test plan (research stage)

- `sweep.py` on the private slot copy: takeoff 0..900 back in 100 steps, with
  and without E. Target: with E every takeoff ends on the far platform (land,
  mantle or catch + pull-up); without E at least the 0..200 edge takeoffs land
  if option 2 is taken. Repeat with the weapon holstered.
- Regressions: slot-12 wall grab and pull-up, boxes-room crate mantles and
  crate-to-crate, stacked-crate release, alley ladder transfer, ladder-top mount,
  D08L edge run-off, jumps into tall walls (no false mantle), Vanilla bounce
  unchanged.

## Further findings (implementation)

- The ground walk-in wall mantle is the same routine as the crate one:
  `0x80051cf0` writes 139 at `0x80051fe0` (write trace on the slot-12 sewer
  wall). Its ledge height `+0x1c4` comes from the ground collision
  (`0x800776c4`, `0x800790ac`). In flight `+0x1c4` stays 0, so there is no
  original height to read mid-air.
- The acquisition `0x80055208` starts with `*(*(0x800d1b50 + 4 * +0x233)) & 1`
  (the held Cross the reach supplies) or a hang flag (0x241); without either it
  returns 0 at once. Its facing tolerance is 0x1e5 for every animation but the
  vertical jump 97 (0xeb). On a ledge catch it sets mode 6, grab 147/148 and the
  ledge top in `+0x1c8`.
- The no-E bounce is not a feet-versus-top test that a small lift could beat:
  lifting Duke up to 1000 units (and his previous position `+0xcc`/`+0x7d8`)
  in the update the ledge first comes in reach still bounces at the same Z. The
  bounce is already decided in that update.

## Change (`ledge_reach.inc`, `modern_controls.cpp`)

Both rules are generic (no slot-5 data) and run from the player update after
the D08X reach retry. They only start original mantles.

1. **E jump mantle for level-geometry ledges** (`ledge_reach_drop()`). During an
   E reach (109, holstered, the same guards as the D08X lift) or the first
   updates of an E bounce, retry the original acquisition with Duke lowered 120,
   240, 360 and 480 (headings 0 and +-290). A lowered **ledge** catch (mode 6)
   whose top is 0x60..0x4c0 above the feet becomes the original height mantle
   (134..137, 139 as the D08X object mantle chooses) from the catch's position
   and squared heading, at Duke's real height: floor `+0x1c8` = feet,
   `+0x1c4` = top - feet, velocity cleared, mode 0. Anything else (a miss, a
   ladder or object catch, a lower lip, a taller ledge) restores the whole
   player record. A lip under 0x60 is left to the flight's own landing.
2. **Ledge forgiveness without E, armed or not** (`ledge_step_up()`). In the
   first three updates of a bounce 107 that follows a running or directional
   jump (98/103/104), retry the acquisition lowered 400..720 with the held bit
   presented for the isolated call only (the pad word is restored at once). A
   ledge catch whose top is at most 0x100 above the feet (or up to 0x40 below)
   starts the smallest original mantle 134 instead of the bounce. Taller ledges
   keep the original bounce. The weapon stays drawn; the mantle plays with it
   and Duke can fire right after.

Diagnostics: `drop_mantles`, `last_drop_rel`, `step_ups`, `last_step_rel` in
`ttk_controls`; `ttk-drop-catch` and `ttk-step-up` lines with
`DNTTK_TRAVERSAL_TRACE=1`.

## Evidence (binary `dce01716db09cbd3b72038d99e3051ef0fbe61fd91a16ac1802d2e994cad209c`)

Private copy of slot 5, Xvfb, real keys, `sweep.py` (Shift + W, Space N units
before the walk stop point), `final3.out`:

| Takeoff back | E, armed (auto stow) | E, holstered | No E, armed |
| --- | --- | --- | --- |
| 0 | lands across (78) | lands across | mantle 134 on the bounce |
| 100..300 | mantle 134 | 300: mantle 134 | mantle 134 on the bounce |
| 400..600 | mantle 134 | 600: mantle 134 | bounce, fall (lip 325..480: above 0x100) |
| 700, 800 | mantle 134 | - | bounce, fall |
| 900 | catch 148, W pulls up 140 (2/2 in `hang900.py`) | mantle 134 | bounce, fall |

Before: E 3 land + 3 catch + 4 bounce, no E 0/10. The armed no-E mantle plays
with the shotgun drawn; Duke fires normally right after (`armed.py`).

Regressions (`regress/`), D08Y build against a build with the two calls
disabled (same source otherwise):

- Slot-12 1024 wall: W + E + Space grabs 148 and pulls up 140; W + Space
  bounces and lands (no step-up off a tall wall). `cases2`: no D08Y mantle
  fired; only lowered ladder catches (mode 3), rejected and restored.
- Boxes room crate A 4/4 with E on both builds; crate B the same pattern on
  both; crate-to-crate 8/8 on top; D08U ladder-top regress unchanged.
- Alley `ladderjump.py`: no attach on either build (3/3 each). The fixture
  fails without D08Y as well; not a D08Y change.
- Angle route: -15..+10 the same; +15 now mantles onto the ledge (lowered catch,
  rel -425) instead of catch and pull-up, ending on the same ledge (3/3).
- Vanilla at the gap (`vanilla.py`, pad route): D08Y counters stay 0.
- Native `ttk-controls-test` (new D08Y fixture: lowered E catch -> height
  mantle; lip, ladder/object, armed, aim, no-E and Vanilla refusals; no-E
  bounce off a low lip -> 134 with the held bit restored; taller lip and a
  bounce without a preceding jump refused), `ttk-input-test`, `ttk-aim-test`,
  Python 92 OK.

## Finding (first pass): Modernized running jump looked shorter

A Vanilla running jump from the edge crosses this gap; the Modernized one fell
short. The first comparison (flight speed per frame) was misleading; the real
cause is the takeoff point, below.

## Limits

- Only this gap and the regression routes were exercised; other geometry
  ledges follow the same rules.
- The E mantle needs the weapon stowed (as every D08X reach); with a weapon out
  E stows during the run-up. Armed without E only the low-lip rule applies.
- Per reach update up to 12 more isolated acquisition calls (each restores the
  player record); no frame-rate measurement was made on the player's GPU.

## Playtest 1 (2026-10-01): hitch after the jump, and the real cause of the gap

User: the jump is now made consistently (cleared, or a quick mantle), but right
after the jump there is a brief hard pause, "like a couple of frames where the
game freezes", that was not there before. The user is open to changing the jump
itself if needed, since the levels may all be tuned to the original jump.

### Hitch

Timed per player update (temporary instrumentation, since removed): during an
E reach the D08X lifted retry already cost 7..14 ms per update (19 isolated
acquisitions) and the D08Y lowered retry added 5..16 ms (12 more), up to 27 ms
in one update, every update of the reach. About half of each call was copying
the 4 KiB stack and 1 KiB scratchpad byte by byte through the runtime's checked
guest accessors (every guest store is also traced by the runtime), the rest
the original acquisition itself (~250 us).

- `original_call()` (`shortcuts.inc`) now copies by words: ~70 us instead of
  ~250 us of copying per call. This also makes the D08X retries cheaper.
- The D08Y player-record snapshots copy by words.
- The lowered retry runs only while Duke descends and straight ahead (every
  slot-5 catch was unturned and on the way down): 4 calls instead of 12.

Result: a reach update averages ~6.5 ms (peak ~12 ms), below what D08X alone
cost before D08Y. The headless frame-timing probe is too noisy to show the
difference either way, so this is a CPU-time measurement, not a felt result.

### Jump distance: the original postpones a run jump to the edge

Real-key measurements on the slot-5 approach (`latency.py`, `mjump.py`,
`vjump.py`, `takeoff.py`, `runoff.py`):

- Flight physics are identical: takeoff velocity 10085 (Modernized) vs 10099
  (Vanilla), the same vertical impulse and apex; both run off the edge at
  Z ~87000..87107.
- Vanilla, jump held while running toward the gap from 420..750 units out:
  launches **at the lip** (Z 87106, 8..12 frames later). Far from an edge it
  launches in 2..4 frames. Modernized launched 4 frames after the press
  wherever Duke was, 100..550 units before the lip.
- Original mechanism (run handler `0x80053500`): with jump held,
  `0x80078c0c` looks 1024 units ahead along the heading (point via
  `0x800780b4`, floor `0x8007765c`); a drop over 0x100 there queues the jump
  (`+0x228 |= 4`, at `0x800539a8`) instead of launching; when the ground probe
  reports the edge, a queued jump launches as 103/104 from the lip
  (`0x800535ec`). `0x80055ea4` drops the queue as soon as the jump button is
  released or Duke stops running, so in Vanilla a tap near an edge falls.
- Modernized (D08I) had deliberately overridden that look-ahead in the
  `0x800780b4` hook: "Modern jump means jump now". That is what made this gap
  (and potentially every gap built around the lip launch) fall short.

Change (`modern_controls.cpp`, `pc_input.cpp`):

- The look-ahead override now first runs the original floor probe at the
  helper's own 1024-ahead endpoint; if the drop there is over 768 (a real gap
  or pit, D08L's large-drop threshold), the helper is left alone and the
  original queues the jump for the lip. Smaller drops (furniture, steps) keep
  the D08I jump-now. If Duke touches an object, nothing changes.
- A queued edge jump (`+0x228` bit 4, run gait 76/78, `edge_jump_queued()`)
  keeps the jump button held on the pad for up to 40 updates after the Space
  press, so a tap behaves like the original's held button. The press is
  forgotten once a jump launches.

Evidence (binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`):

- Slot-5 launch point, Space tapped or held 420..750 units before the lip:
  launches from the lip at Z 87107 (was 86548..86990). Far from the edge:
  unchanged (4 frames).
- Apartment bed (drop ~383): jumps still launch in 2..3 frames, never
  postponed (6/6). Fire-escape platform (large drop): presses 7..16 frames
  before the edge launch from the lip at the same spot (Z ~14356, 3/3).
- Native: controls (large drop leaves the look-ahead alone and restores the
  probe fields; small drop still overrides), input (queued hold for 40 updates
  after a press, none without a recent press, none after launch).

Slot-5 sweep and regressions on that binary (`regress2/`):

| Takeoff (Space pressed N before the walk stop point) | No E | E (armed / holstered) |
| --- | --- | --- |
| 0..600, 800 | lands across from the lip (78) 8/8 | lands across 9/9, 600 and 800 with a hard landing 105 |
| 700, 900 | bounce: pressed >1024 before the lip, jumps at once as in Vanilla | 700, 900: mantle 134 (lowered catch) |
| holstered 0, 300, 600, 900 | - | 4/4 across |

Before this round without E: 0/10 (original), then 4/10 with the step-up only.
The 900-back E catch still pulls up with W (2/2). Vanilla gap runs identical to
the earlier Vanilla run, D08Y counters 0. Slot-12 wall grab/pull-up and no-E
bounce, `cases2`, crate A 4/4, crate-to-crate 8/8, D08U regress unchanged;
crate B and the alley ladder fixture the same as on the D08Y-disabled baseline;
angle route ends on the ledge in all five cases (+10/+15 via the jump mantle).
Native controls/input/aim and Python 92 OK.

Limits of this round: only this gap, the bed and the fire-escape platform were
measured for the lip launch; the 768 threshold and the 40-update hold are
untested elsewhere. A press more than 1024 units before a gap still jumps at
once (original). The hitch fix is measured as CPU time, not felt.

## Playtest 2 (2026-10-01): smoothing the odd freeze frame and pop

User: the jump is made now; the odd freeze frame and pop remain, "not very
prominent but definitely present". The rail-like original jump is kept; a more
manual modern jump is backlogged as D08Z.

### Freeze frames

`DNTTK_FRAME_TRACE=1` (new, off by default) logs the wall time between player
updates (one 30 Hz game logic frame) and the CPU time spent in the Modernized
hooks during it. On the round-2 build, every E-reach frame (109) was late:
median 49.9 ms against 33.4 ms for every other state, with 5.7 ms of hooks (19
D08X + 4 D08Y isolated acquisitions per update, ~250 us each).

- One budgeted scheduler (`ledge_reach_retry()`) replaces the D08X lift loop
  and the D08Y lowered loop: at most four core tries and one extra try per
  update. Core (turned at the original height, lifted 160/320/480, and, while
  falling, lowered 120..480) comes round every three updates; the lifted
  turned tries every twelve. Same catches and conversions as before.
- The no-E bounce step-up tries two heights per bounce update.
- `original_call()` and the player snapshots write back only the words a call
  changed (guest stores are the expensive, traced part).

Result (slot 5, six jump variants): E-reach frames median 33.3 ms, none over
40 ms (was every one ~50 ms); hooks 1.2 ms median, 2.3 ms peak.

### Pops

Per-frame position steps over 260 units during the jumps: the E jump mantle
moved Duke up to ~400 units to the catch point (at the face) in one update.
The mantle now starts where Duke is and glides to that point over its first
updates (a third of the remaining distance, at least 60, per update; at most
eight updates), while the original mantle animation plays. After: no such
step in the E 700/900 mantles. (Paired 480-unit up-and-back steps at E 300 are
the debug sampler reading RAM mid-retry; every retry restores Duke before the
frame is drawn.)

The runtime's always-on write tracing makes every guest store expensive
(debug-tools build); that affects the whole game and is a build-configuration
question outside D08Y.

Evidence (binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`, `regress3/`, frame trace on):

- Slot 5: no E 8/10 land (700/900 pressed beyond the look-ahead, as before);
  E 14/14 across. Every jump state at 33.3 ms median, **no frame over 40 ms**
  (109: 443 frames, max 38.7 ms; hooks 1.2 ms median, 2.7 ms peak).
- 900-back E now mantles straight up (134) instead of catch and pull-up, ends
  on the platform with modern controls ready (2/2).
- Bed jumps immediate, fire-escape presses launch from the lip (unchanged);
  Vanilla counters 0 (6/6); slot-12 wall grab/pull-up and bounce, `cases2`,
  crate A 4/4, crate B same class as the baseline, crate-to-crate 8/8, angle
  route all on the ledge, D08U regress unchanged.
- Native controls (budgeted schedule order and rotation, glide), input, aim,
  Python 92 OK.

## Playtest 3 (2026-10-01): random freezes, also without jumping

User (session log with `DNTTK_FRAME_TRACE=1`, about 10 freezes/pops in ~2.5
min): freezes still happen every so often, the last one without jumping; the
first jump of the session froze.

### What the log showed

- 4,203 logic frames: 386 took 41..50 ms (one or two extra video fields), in
  55 clusters of up to ~1.5 s (49 frames at 20 fps in a row), nearly all while
  running (76/78). Modernized hooks during those frames: ~0.03 ms; max 1.6 ms.
  Not the D08X/D08Y code.
- The new tracer was extended to the near-clip and weapon-aim plugins: all
  Modernized plugin code together is 0.11 ms per frame median, 0.37 ms peak.

### Dead ends (kept for the next investigator)

- Xvfb measurements mislead for performance: software OpenGL (and the
  software renderer) make the host the bottleneck in 16:9, and a random
  running route changes the scenery. Both produced a false "widescreen GPU
  cost" and a false "overclock makes it worse".
- `frame_perf` scene GPU time (~14.5 ms) is the span of the whole emulated
  frame, not GPU load (it does not change between 1x and 3x scale).
- The monitor is 180 Hz, so the presenter already runs the wall-clock pacer
  with vsync off; no vsync beat.

### Reproduction on the player's real GPU

SDL offscreen + NVIDIA EGL (no window on the desktop) with a new
diagnostics-only input driver: `DNTTK_TEST_DRIVE=<mouse counts per frame>`
makes the input layer act captured and holds Shift + W with a constant turn
(`pc_input.cpp`; never set by run.py). Boxes-room state, 16:9, 4x, 30 s:
third person 144 slow frames, first person 59, first person 4:3 38; repeat
runs identical. Host GPU work stays small (2%); the game spends the rest of
the time in its own wait loop.

### Cause

The emulated PlayStation CPU runs out of budget. VBlank is raised every
564,480 emulated cycles; TTK runs its logic every two fields (30 fps) and
drops to three (20 fps) when its frame's work does not fit. The Modernized
independent camera and widescreen show more of a level than the original
camera (more rooms, objects and polygons for the game's own code to
transform), so busy views overrun on any host. That is the freeze/pop.

### Fix: emulated CPU overclock (Modernized default 150%)

- Runtime patch `patches/time-to-kill-zzzzzzzzz-cpu-overclock.patch`
  (`psx_cycles.c`, `load_accel.c`): at each device service the CPU-charged
  part of the unserviced interval is scaled by 100/percent; devices (VBlank,
  timers, CD, SPU, DMA) keep their rate, so game speed, audio and streaming are
  unchanged. Idle-loop skips and the vsync/poll horizon jumps are exempt;
  GTE/mul-div completion stamps and the idle detector move back with the
  clock. Nothing in the inlined charge path changes, so the codegen hash and
  every existing save state stay valid (an inline-header version broke save
  state loading and was discarded).
- `fmv_poll.c`: the movie poll measures its loop on the uncompressed clock and
  registers its skips as exempt (without this the intro movie ran at ~41 fps).
- Profile schema 19: `controls.cpu_overclock` 100/125/150/175/200, default 150
  for Modernized; `run.py --cpu-overclock N`; Vanilla always runs at 100
  (`PSX_CPU_OVERCLOCK`).

Measured on the real GPU (binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`), same route:

| 16:9, captured, running and turning | 100% | 125% | 150% | 200% |
| --- | --- | --- | --- | --- |
| Third person, slow frames of ~869 | 144 | 44 | **0** | 5, plus 453 one-field frames |
| First person | 59 | - | **0** | - |

At 200% the game starts finishing frames within one field (its own pacing
changes), so 150 is the default. Game distance per 60 fields is unchanged.
Intro movie at 150: real time (2,695 fields in 45 s), skips normal. Slot-5
jumps unchanged. Native controls/input/aim and Python 93 OK.

Limits: measured offscreen on the player's GPU and CPU, not in the visible
window; scenes other than the boxes room, slot 5 and the intro movie were not
timed. Very heavy views could still exceed 150%; `--cpu-overclock 175` is
available.

## Playtest 4 (2026-10-01): audio slowdowns with the overclock

User: "all sorts of audio slowdowns" before testing (fresh boot into the
apartment). Reproduced on the real GPU with real audio: a fresh boot at 150%
fell to ~45 fields/s about 5 s in (startup/loading), audio fill 6 ms, ~14,000
underruns; 100% clean. Loading busy-waits on devices in loops the runtime does
not skip, and an overclocked CPU spins more host iterations per emulated
second.

Change: the overclock is a lease. `psx_overclock_renew()` (runtime patch) is
called from the Modernized player update; the overclock lapses three fields
after the last renewal, so boot, menus, movies and loading run at the
original speed. Safety net in the plugin: if host frames fall under 57 per
second while leased, renewals pause for 5 s (`[TTK cpu] emulation behind real
time ...` in the session log).

Evidence (binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`, real GPU, real audio): boot at 150 has 0 underruns
(min fill 162 ms); boxes-room third/first person 59.95/59.97 fields/s with 1/0
slow frames and no pauses; apartment 0 underruns. Under Xvfb (host-bound) the
safety net pauses as intended. Suites pass.

## User observation after acceptance (2026-10-01)

The user played with `--cpu-overclock 100` and `150`: both perfectly playable,
no noticeable difference. Reading: the 20 fps dips the overclock removes are
real in measurement but subtle in play; the freezes felt earlier most likely
came mainly from the E-reach hitch (fixed independently of the setting) and the
desktop compositor (a Cinnamon restart, idle at ~36% CPU after 3 days, was done
before the final tests). 150 stays the default (steady 30 fps in busy views, no
downside since the round-5 lease); 100 remains a supported choice.

