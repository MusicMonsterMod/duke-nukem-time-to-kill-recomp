# D08Z5 - Wall backflip: Modern (slope jump that climbs, no backwards launch) / Original

Status: **Done** (user-accepted 2026-10-21, after the first and second playtest changes below). Modernized option `wall_backflip`,
`modern` by default; `original` and Vanilla keep the original launch.

## Report

User (2026-10-21): "When Duke jumps against certain walls or obstacles, the
original game can automatically trigger a backflip. In third-person mode, this
is visually apparent. However, in first-person mode, the animation is barely
noticeable, while the associated backwards displacement still occurs."
Reproduction: UI slot 10, Duke in front of a police car, first person, Shift+W,
jump into the car at the right angle.

## Reproduction

Private copy of the player's UI slot 10 (file `slot09`, SHA-256
`467a8934092b3b717b27736a2c866a8321904f9c26b7495b9a7050f5391a90b0`) with the
player's settings (manual jump, `jump_walls slide`), first and third person,
in `recomp/analysis/d08z5-20261021/` (ignored): `common.py` (Xvfb :97, real
xdotool keys, port 9272), `jump.py` (gait, run-up, heading; samples position,
velocity, animation and mode about every 9 ms; "back" = how far Duke ends up
behind the furthest point he reached).

The slot was saved from the in-game SAVE GAME page, so the harness boots a
clean save and loads slot 10 through the pause menu's LOAD GAME, as the player
does, then relaunches for every jump (see Limits). A run-up of 0.1 to 0.6 s
reproduces it every time; standing and walking jumps hit the car side and
land in front of it.

## Cause (static analysis of the owned executable, confirmed in traces)

It is not a wall response: no 107 bounce and no D08Z1 slide happen in these
jumps (`jump_walls` slides 0). The car body is a steep floor.

1. The jump lands on a face 40..70 degrees steep (the steep test `0x80077438`
   accepts a floor normal 20..50 degrees above level).
2. The next ground step `0x80055e80` asks `0x800544e8` whether the floor is
   too steep to stand on. If so it stores mode 2 (`0x80055fb4`, the only store
   of mode 2 in the executable) and the slip pose: 145 when Duke faces up the
   slope, 143 when he faces down it (`0x8005446c`).
3. The ground dispatcher `0x80048410` runs the slide handler `0x80054568` for
   143..146 (it switches on the animation, not the mode). The first update
   plays the slide sound `0x2006`. Duke slides down the face at about 70 world
   units per update.
4. When the floor ends under him (`0x800789a8`), the handler starts the fall
   108 through `0x8003f060`. Its flight start `0x8003e2d0` picks a launch from
   the table `0x800c27c8` by the pose: 145 takes entry 10, 84 units straight
   back from Duke's facing; 143 takes entry 9, the same forwards; neither
   rises. `0x80066e34` (ra `0x8003e488`) rotates it into the velocity: about
   (-3040, 0, -380) at the slot-10 car.

That launch is the push: over a fall off the car it carried Duke about 450 to
500 world units back, after about 70 of slide. In first person the slip pose
is out of view (D08Z4 keeps the eye there), so only the push is felt. Third
person shows the lean-back slip and the flight backwards.

D08Z1 does not see it: its pre-pass and safety net act on airborne wall hits
in the directional airborne handler, and this is a landing on a floor.

## Change (`recomp/src/ttk/wall_backflip.inc`)

Modern keeps the slide (gravity on a face too steep to stand on) without the
launch. At the entry of `0x8003f060` from the slide handler's three exits (ra
`0x800546c4`, `0x800546dc`, `0x800546ec`, mode 2), the next rotation of the
launch vector by the flight start (`0x80066e34`, ra `0x8003e488`, a1 =
`+0x1f4`) is armed; there the vector is zeroed, as the table's entry 0 does
for other falls. Duke drops straight down where the slope ends. The slip pose,
slide sound, slide speed, fall and landing are the original's; other falls and
other rotations are untouched.

- Option: profile schema 37 `controls.wall_backflip` `modern` (default) or
  `original`; `run.py --wall-backflip`, `--settings` choice P,
  `DNTTK_WALL_BACKFLIP` (always `original` for Vanilla), and OPTIONS >
  ADVANCED > MOVEMENT > WALL BACKFLIP (live, like the other rows). A separate
  row from JUMP WALLS because it is a different mechanism (a steep floor, not
  an airborne wall hit); the two never act on the same update.
- Hooks: new entries `0x8005446C` (slide start, diagnostics) and `0x8003F060`
  in `game.local.toml`, regenerated; codegen hash `0x8bab543c` unchanged, so
  savestates load. `0x80066e34` was already hooked. The plugins use their own
  id `ttk.wall.backflip`: the runtime refuses a second plugin with the same id
  at one address (the swim pace already uses `ttk.modern.controls` there).
- Guards: `0x8005446c` (124 bytes) and `0x80054568` (740 bytes) added;
  `0x80055e80`, `0x8003f060`, `0x8003e2d0`, `0x800c27c8` and `0x80066e34`
  were already guarded.
- Diagnostics: `ttk_controls.wall_backflip` (`modern`, `slides`, `stops`);
  `DNTTK_TRAVERSAL_TRACE` prints `ttk-wall-backflip` lines.

Not changed: the slip pose. The slide handler is chosen by the pose (143..146),
so swapping it would need the store redirected; side-by-side captures of
143..146 at this distance were indistinguishable and each lasts only one or
two updates. It can be revisited if the user still sees a flip.

## Evidence

Executable `191646ef67d6890ec435ddc771a9f31c9da349eb6e83d8a8c5b341860a4d3647`
(before: `9b296ae1...dece3f`). Every run below is on that one binary, Modern
against `wall_backflip original`. "Back" is world units (Duke is about 1040
tall); "slides/stops" are the diagnostics counters.

**UI slot 10 car (level 0), run-up jumps, back for 0.1 / 0.2 / 0.4 / 0.6 s:**

| Case | Original | Modern |
| --- | --- | --- |
| First person, manual jump | 733 / 525 / 812 / 525 | 280 / 70 / 410 / 160 |
| Third person | - | 132 / 70 / 410 / 141 |
| First person, assisted jump | - | 218 / 70 / 394 / 141 |
| Turned left (mouse -250) | 625 / 594 / 0 / 597 | 199 / 322 / 0 / 333 |
| `jump_walls original` | - | 280 / 70 / 0 / 160 |

Standing and walking jumps hit the car side and never slide (back 0..57, the
landing pose's own drift). Turned right (+250) Duke clears the car in both
modes. In every Modern run slides = stops (1 to 4 per jump): no launch got
through. What remains is the slide itself, about 70 units per update down
the panels (each step also about 87 lower), then a straight drop (velocity
(0, y, 0) from the slope edge to the street), plus the landing 105's usual
small drift. In Original the fall after the slide flies at (-3040, 0, -380).

**UI slot 6 car (level 0, the D08Z4 private copy `724c3cb8...`), first
person, back for 0.1 / 0.2 / 0.4 / 0.6 s:** Original 665 / 580 / 669 / 0 (the
last missed the car), Modern 140 / 70 / 137 / 235. Third-person captures
(0.2 s): the same slip pose 145 on the car body, then Original flies back
over the street, Modern drops beside the car.

**Regression routes** (`regress.sh`, the D08Z1 set with JUMP WALLS `slide`:
slot-12 wall grab and pull-up, slot-12 E cases, crate mantles,
crate-to-crate, D08X angle grabs, D08U ladder top, D08W subway wade jumps and
ledge sweep, D08Y furniture run-offs, D08Z slot-5 gap in both jump styles),
Modern and Original on the same binary: no slope slide happened on any route
(no `ttk-wall-backflip` line in 19 game logs with 5286 traversal trace lines),
so Modern acts nowhere there. Outcomes agree, up to the run-to-run variation
D08Z1 recorded (crate B, the assisted no-E gap case at 86300). The first
Original furniture run hit "emu busy or frozen" while turning Duke after its
first savestate load, before any jump; the rerun completed and matches Modern.

**Settings:** OPTIONS > ADVANCED > MOVEMENT shows WALL BACKFLIP between JUMP
WALLS and LANDING; Right switches MODERN/ORIGINAL, CONFIRM applies it live
(`modern` false at once) and the launcher writes it to the profile at exit;
CANCEL keeps it. Vanilla: `modern` false, first-person code idle. Python 162
OK (2 skipped), including schema 37 migration, CLI, environment, `--settings`
choice P and validation. Native: `ttk-controls-test` (LEVEL01 and all levels)
49 groups, including the new D08Z5 group (the three slide exits zero the
launch; another fall start, another rotation, another vector, a fall not
from mode 2, `original` and Vanilla keep it; one fall arms one rotation);
`ttk-aim-test`, `ttk-input-test` PASS.

## First playtest (2026-10-21): the backflip itself

User: "this absolutely doesnt seem to have provided any kind of fix ... the
backflip still happens, third person or first person. there is even a chance
duke will slip out of first person" (video `Video_2026-10-21_17-06-55.mp4`).
The session logs showed `anim=99 state=9/0` and `anim=100 state=9/9` with the
lease lost: the backflip is animations 99/100, which the delivery above never
touched.

Cause: the slide handler's last block (`0x800547ac..0x80054834`) checks Jump
(the Square word table `0x800d1b88`, no `+0x224 & 0x241`, no jetpack thrust).
If pressed, `0x8005446c` compares Duke's yaw `+0x24` with the slope direction
`+0x2bc`: more than 90 degrees apart (facing up the slope) gives 99, or 100
when the last jump `+0x23c` was 99; otherwise 98. It writes `+0x23e` and
`+0x60` and clears the track, and the lower-body update initializes it
(`0x800493a4`). The block also runs after the handler's fall start
(`j 0x80054780`), so the jump can begin in mode 2 or already 9, and the slide
can begin and end in one update (99 at 9/0). 99/100 fly backwards: in the
harness about 2400 units. The first harness held Space 0.15 s before the
landing, so it never pressed Jump during a slide; the player does. A standing
jump also picks 99/100 when Down is held at `0x8005492c`; Modernized never
feeds Down there.

Change: the facing test's entry from that block (ra `0x800547fc`) arms
`wall_backflip_jump`; at the next `0x800493a4` from the lower-body update (ra
`0x8005a3a0` or `0x8005a4d0`), Modern turns 99/100 into 98 (`+0x60`,
`+0x23e`), the jump the original makes off a slope when Duke faces down it,
here in Duke's facing (often onto the car). Anything else disarms it. No new
hook entry (`0x800493a4` was already hooked; own plugin id). Separately,
`slide_state_early()` now keeps the camera for 99/100 at mode 9 from 0 or 9,
so ORIGINAL keeps first person through its backflip. Diagnostics:
`wall_backflip.jumps`; trace `ttk-wall-backflip jump`.

Evidence, executable `cd458c38c3ab3b22658ef1b0896fc3bed36df2cf02a8976ce391d2514bd261d2`, `jump.py` with `MASH=1.0` (Space tapped
throughout the second after the jump), the player's settings (assisted jump,
JUMP WALLS slide):

| Case | Runs | 99/100 | Back (units) | Lease lost |
| --- | --- | --- | --- | --- |
| Modern, third person, 0.1/0.2/0.4 s | 12 | 0 (6 rewritten to 98) | 0 to 70 | 0 |
| Modern, first person, 0.1/0.4 s | 6 | 0 (11 rewritten) | 14 to 487 | 0 |
| ORIGINAL, first person, 0.1/0.4 s | 6 | 4 | up to 2414 | 0 after the lease fix (2 of 4 before) |

The larger Modern distances are repeated 98s that land high on the roof and
slide down the panels again; each fall off the face still has no launch.
Native: the D08Z5 group checks armed 99 and 100 at mode 2 and 9, a standing
99, a 98, another initializer caller, ORIGINAL and Vanilla.

## Second playtest (2026-10-21): a jump that climbs

User: "i dont like how this feels ... i prefer the idea of when jumping again, duke is actually trying to jump up a gradient slope a little higher ... What we've got right now is that dukes jump feels almost cancelled out".

Cause: the Modern 98 never rose. Duke takes off pressed into the face, so the
airborne handler's sweep (`0x8003ebf4` then `0x8007a98c`) returns 2, floor,
on the first update even while he moves away from it. Result 2 moves him only
horizontally and picks a landing pose (`0x80054c04`); the next ground step
finds the face too steep and slides him again. Traces showed no rise at all.
The original 98 also flies forward at about 49 degrees, below the face's 51.5,
so it would run into the face anyway. Pressed at the bottom of the face, the
fall has already started and 98 keeps the fall's velocity: no jump.

Change (`wall_backflip_climb`, plugin at `0x8003ebf4` from the airborne
handler, ra `0x80055934`, registered before D08Z1's presolve):

- First airborne update of the slope jump: Duke keeps the rise (-6917, also
  given to the bottom-of-face case) and drifts straight uphill (`+0x2bc` is
  downhill) at k * rise / tan(face), face = -`+0x2be` (the face's tilt, from
  the steep test `0x800544e8`). The arc then leaves the face and comes back
  down on it at best 4k(1-k) of the jump's height above the take-off. k is
  0.25 (`DNTTK_SLOPE_JUMP_K` for diagnostics).
- Every rising update (98, mode 9, upward velocity, at most 40): isolated
  original integrations and sweeps, restoring Duke (as D08Z1): the drift; if
  it reports floor, straight up; if that does too, lifted 24, 48, 96 or 160
  units, the first the sweep finds free. The drift resumes once clear.
- Coming down, the original lands him on the face and slides him; the next
  jump starts from there. Diagnostics `climbs`, `lifts`; trace
  `DNTTK_SLOPE_JUMP_TRACE`.

Evidence, executable `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da`, slot-10 car (face 51.5 degrees), third person,
Space tapped every 0.15 or 0.3 s for 3 s after a 0.1/0.2/0.4 s run-up jump:
each slope jump rises about 430 units and lands 90 to 390 higher on the face;
3 of 6 runs ended standing on the car, the others slid back down and off.
With much faster mashing a single jump from high on the face could clear the
roof edge. First person, tap 0.2 s: no lease loss. Native group extended.

Limits: tuned at one car face. A long steep slope elsewhere can be climbed by
well-timed jumps (as in Duke Nukem 3D); k sets how easily. The original's
slope jump facing down the slope (98 from 0x8005446c returning 0) is
unchanged.

## Limits

- Tested at the two level-0 police cars (slot 10 and the slot-6 copy). The
  rule is on the generic slide path, so every steep face in every level
  takes it, but no other level's slope was driven: none of the regression
  routes in level tags 3, 4 and 12 slides.
- The slide down the face stays (about 70 units per update, so 70 to 410
  units at these cars, more down a long steep slope). Removing it would mean
  standing on, or clipping into, a face too steep to stand on.
- The slip pose 145 and the slide sound stay; in third person the slip still
  shows for one or two updates per panel. If it still reads as a flip, a pose
  swap is the next step.
- Separate finding: the player's slot 10 file (saved from SAVE GAME's
  OVERWRITE THIS SAVE?) halts the emulator on some reloads in one session
  (GPU GP0 `0x1F`, `0xFE`, `0xF8` "unknown command"): a second debug-port
  load always, and the third LOAD GAME of it in one session once; a
  debug-port load also leaves textures corrupted. Two LOAD GAMEs in a row were
  fine, and other SAVE GAME saves (UI slots 7 and 8) reload cleanly. Not
  caused or changed by this job; not yet investigated.
