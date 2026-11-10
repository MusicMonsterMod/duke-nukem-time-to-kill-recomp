# Job board history - 2026-10-01

Dated session log entries from 2026-10-01, moved verbatim from
`MODERNIZATION_JOBS.md` on 2026-11-10 to keep the board under the repository
file size limit. Job descriptions and statuses remain on the board.

## 2026-10-01 - Playtest: D08U, D08W, D08A3 Done; D08X follow-up

User playtest of the autonomous chain (binary `db5288c2...3689`):
- D08U: "I completely accept that": E at the slot-12 ladder top grabs on,
  third-person descent flawless; running off the edge still drops.
- D08W: sideways and forward water jumps "all good ... completely accepted".
- D08A3: original HUD icons for health/medkit, jetpack, Bio Mask and night
  vision: "phenomenal ... I love that they are the authentic icons".
- D08X: ledges in the drained-water area catch reliably when standing below
  and jumping with E ("100%"); the ones that failed were too high. Follow-up:
  in the boxes room (wooden crates; user saved UI slot 11) Shift + W + Space
  with E held bounces off a box instead of mounting it mid-jump. D08X back to
  In progress for that case.
- D14: user asked for a 16:9 preview launch; run.py now honours a dev-only
  `DNTTK_GAME_CONFIG` override for the private 16:9 config copy.

## 2026-10-01 - D08X boxes-room crate mantle (Needs playtest)

Reproduced on a private copy of the user's UI slot 11: Shift + W + Space with E
into a crate bounced (107) 4/4. The crate (type 20, climbable 0x80/0x40) only
becomes the contact after the reach's acquisition, so it never caught. New
`ledge_reach_mantle()`: at the start of an E bounce off a climbable object whose
top is 0x100..0x3ff above Duke's feet (contact or cell-list fallback, in front),
start the original height-matched mantle 134..138 from mid-air. Evidence
(binary `9bade2a5...ba9c`): straight crate 3/4 on top (miss = jump from contact),
diagonal crate on top when reached, no-E bounces unchanged 8/8, wall ledge grab
and alley ladder transfer unchanged, native fixture and suites PASS. Also: the
user's standing permission to close an open game is now in AGENTS.md and the skill.

## 2026-10-01 - D08X crate hang and identity loss fixed (Needs playtest)

User: an E jump into crates under the opening hung Duke on a crate and modern
controls were lost. Cause 1 (pre-existing): the original object hang toggles
bit 0x40 of flag-table entries 149/152/153, which the controls identity guard
treated as code; guard split around them, plus a failing-guard log line.
Cause 2: that hang (mode 7) cannot climb when the space above is blocked; in
Modernized S lets go, and W with no climb for 40 updates lets go on its own.
Binary `bf368b68...bd45`; stack 3/3 release with modern controls kept;
regressions unchanged; suites PASS.

## 2026-10-01 - D08X first-tier crate hang (Needs playtest)

User: grabbing the first tier of a stacked crate glitched and A/D spun Duke
round. Cause: the reach catches the upper crate at the first tier's top into
the original pole-style object hang (mode 7), where A/D circle the object. In
Modernized a mode-7 catch on a crate-type object (0xc0) now lets go at once
(2..3 frames, then the normal fall/bounce). Binary `c1c49119...6fe7`;
crate mantle and offset-stack climb unchanged; suites PASS.

## 2026-10-01 - D08X higher grab confidence (Needs playtest)

User asked for higher confidence when jump-grabbing ledges. The original reach
probes 425 above Duke's root, so ledges just above the apex were missed. New
`ledge_reach_lift()` retries the original acquisition during an E reach with
Duke raised 160/320/480; the original decides the catch; Duke is then eased up
from his real height (no pop). Binary `43eda09c...0f61`: 2048 wall still
refused at 480 (caught at an experimental 960, smooth rise); 1024 wall, crates,
stack release, ladder transfer and ladder-top mount unchanged; native retry
test; suites PASS. Playtest: the drained-water ledges.

## 2026-10-01 - D08X angle forgiveness (Needs playtest)

User: crate-top jump to the ledge needs Duke perfectly square. The original
facing test allows 45 degrees of the body heading, which follows the camera in
flight. The reach retry now also tries the heading turned +-25 and +-51
degrees; the original catch squares Duke. Binary `8bb01385...1701`: mid-jump
camera turns -35/-50 now catch, approach sweep -15..+15 all catch; regressions
unchanged; suites PASS.

## 2026-10-01 - D08X crate-to-crate mantles (Needs playtest)

User: crate-to-crate mantles low confidence at an angle; keep it game-wide. The
jump mantle now works by proximity to any climbable object's own rotated box
through the whole E jump, uses the original ground line-up (0x8007ec4c) before
every mantle, takes the full climb 139 for steps just above the mantle range,
and skips objects with something stacked on them. Binary `ccb4eb2a...0107`:
crate-to-crate 18/18 from -45..45 degrees; no-E unchanged; floor crate, stack
drop, alley object, wall grab, ladder transfer unchanged; suites PASS.

## 2026-10-01 - D08X flush hang (Needs playtest)

User: hands sometimes grab the air above the ledge until a shimmy. The catch
kept the probe height; the shimmy settles the root 454..463 below the ledge
top. E-jump catches now ease to ledge top + 456 (old rise-by-lift easing
removed). Binary `60161f81...369c`; hang matches the shimmy within ~5 on
two ledges; regressions unchanged; suites PASS.

## 2026-10-01 - D08X accepted (Done)

User: "thats it, fully accepted this!!!" after the flush-hang build
(`60161f81...369c`). D08X covers: early E reach, crate and object mantles from
any angle (original line-up, full climb, stacked-object skip), reach retries
(lift up to 480, heading +-51 degrees), flush hang settle, object-hang release
and the identity-guard fix for the original flag-table toggles.

## 2026-10-01 - D14 preview triage; session accepted

User accepted everything else from the session ("everything else wins for me
and i accept"). D14 16:9 preview reported off centre with large geometry
holes; 2-minute triage points at TTK's 4:3 projection centre, draw area and
culling under the native-wide path. Backlogged with ordered next steps; D14
stays In progress for a fresh session.

## 2026-10-01 - D14 widescreen (Needs playtest)

User asked for widescreen at least as good as DuckStation fullscreen. Causes of
the broken preview: `nw_hud_corners` shifts every untagged polygon for a title
without a sprite anchor (off centre, torn), and TTK culls against its 4:3 portal
root rectangle `0x800d2210` (holes). Native-wide (original projection, wide
surface, 1:1 present) kept over the squash path after an A/B.
- `widescreen_view_rect()` widens `0x800d2210` by the margin at the render's
  projection load (`0x800b4d9c`, ra `0x8002e4d0`) before the portal walk.
- D11B conservative near clip also runs in third person while widescreen is live.
- HUD: new hooks `0x8008BA30` (status bar: layout `0x800dd778` x moved out by the
  margin) and `0x8001FC44` (restore); regenerated.
- Plugin accepts 16:9/16:10/21:9 and `auto` (adaptive to 21:9); profile schema 18
  `controls.widescreen` (default 16:9), `run.py --widescreen`, settings choice W;
  `game.local.toml [widescreen] gte_game_mode = true`.
Evidence (binary `3f726b91...af2c`): margin black 0.39 -> 0.11 at fixed headings,
near-wall margins 0.945 -> 0.000; HUD corners at 16:9/21:9; pause/Select/night
vision/first person/inventory strip; FMV/title/menus pillarboxed; `auto` resize
16:9 -> 21:9 -> 4:3; run.py path; DuckStation (portable, private config) shows
the same view stretched by 4/3. ttk-controls-test, ttk-near-test, ttk-input-test,
Python 92 OK; Vanilla route `d14-vanilla-1` exit 0. Software-GL fps at 4x drops
about 20-30% (fill rate); real-GPU fps not measured. Needs a playtest across
levels.

## 2026-10-01 - D14 widescreen accepted (Done)

User: "im very happy with it! i accept!" (binary `3f726b91...af2c`). Case study
recorded in the D14 notes: DuckStation's widescreen hack gives the same wider
view but stretches every 2D element (text, HUD, menus) and Duke by 4/3, while
the recomp renders the extra columns natively and keeps text, HUD and movies
at their original proportions.

## 2026-10-01 - D08Y backlogged with research

User report: slot 5 gap jump bounces off the far platform unless started 3-4
feet early with E. Private-copy sweep (takeoff 0..900 back, E and no E,
current binary): no E 0/10 cross; E lands 0..200, catches 700..900 (all via the
D08X turned retry), bounces in a 300..600 dead band. Cause: the D08X jump
mantle accepts only climbable objects; this edge is level geometry. Job D08Y
added as Todo with options and acceptance;
[research notes](documentation/77-d08y-gap-jump-research.md). No code changed.

## 2026-10-01 - D08Y implemented (Needs playtest)

`ledge_reach.inc`: `ledge_reach_drop()` retries the original acquisition
`0x80055208` lowered 120..480 during an E reach; a ledge catch 0x60..0x4c0 above
the feet becomes the original height mantle at the real height (misses and
ladder/object catches restore the whole player record). `ledge_step_up()` turns
the first updates of a bounce after a 98/103/104 jump into mantle 134 when a
ledge's top is at most 0x100 above the feet, presenting the acquisition's
held-Cross bit for the isolated call only. Slot 5 (private copy, binary
`dce01716db09cbd3b72038d99e3051ef0fbe61fd91a16ac1802d2e994cad209c`): E 0..900
back 10/10 armed + 4/4 holstered across (900 back: catch, then pull-up 140,
2/2); no E 0..300 back across 4/4, 400..900 bounce as before. Regressions vs a
D08Y-disabled baseline: slot-12 wall grab/pull-up, crate mantles, crate-to-crate
8/8, ladder-top regress and the alley ladderjump fixture (no attach on either
build) unchanged; angle +15 now mantles onto the ledge instead of catch and
pull-up (same end). Vanilla: D08Y counters 0. Native controls (new D08Y
fixture), input, aim and Python 92 OK. Finding: a Vanilla running jump from the
edge crosses this gap; Modernized's flies ~3% slower horizontally (53.1 vs 54.9
units/frame) and falls short. Not changed here.

## 2026-10-01 - D08Y round 2: lip launch restored, reach hitch fixed (Needs playtest)

User playtest 1: the jump works (cleared or a quick mantle), but a brief hard
pause follows it; changing the jump is acceptable if the levels need it.
Hitch: per-update timing showed the D08X + D08Y isolated acquisitions at up to
27 ms per reach update. `original_call()` copies by words, D08Y snapshots by
words, the lowered retry runs only descending and unturned: ~6.5 ms average.
Jump: identical flight physics; Vanilla holds a jump pressed near a gap until
the lip (`0x80078c0c` look-ahead queues `+0x228` bit 4), which the Modernized
`0x800780b4` hook had overridden. It now leaves the look-ahead alone before a
drop over 768 and `pc_input` holds Square for a queued edge jump (40 updates
after a press). Slot 5: no E 8/10 land (misses pressed >1024 out), E 14/14.
Bed jumps still immediate; fire-escape presses launch from the lip. Regression
routes, Vanilla and suites as in documentation/77. Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`.

## 2026-10-01 - D08Y round 3: freeze frames and pop smoothed (Needs playtest)

User: the jump is made now; an odd freeze frame / pop remains. Rail-like jump
kept; a manual modern jump is backlogged as D08Z. `DNTTK_FRAME_TRACE=1`
showed every E-reach frame late (~50 ms, 5.7 ms of hooks). One budgeted
scheduler (`ledge_reach_retry()`, 4 core + 1 extra isolated acquisitions per
update), two step-up tries per bounce update, and changed-words-only restores:
reach frames 33.3 ms median, none over 40 ms. The E jump mantle no longer snaps
Duke ~400 units to the catch point; it glides there over its first updates.
Slot 5 and all regression routes as in documentation/77. Binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`.

## 2026-10-01 - D08Y round 4: freezes were emulated-CPU overruns (Needs playtest)

User session log (`DNTTK_FRAME_TRACE`): 386 of 4,203 frames at 20 fps in
clusters while running; Modernized hooks ~0.03 ms in them. Reproduced on the
player's GPU offscreen with a new diagnostics-only driver (`DNTTK_TEST_DRIVE`):
144 (third person) / 59 (first person) slow frames per ~869. Cause: TTK's own
code exceeds the PlayStation CPU budget per frame in views the Modernized
camera and widescreen reveal. Fix: runtime CPU overclock (new reviewed patch;
device timing, game speed, save states unchanged; skips exempt; movie poll
adjusted) and profile schema 19 `cpu_overclock` (Modernized default 150,
`run.py --cpu-overclock`). Result 0 / 0 slow frames; intro movie real time;
jumps unchanged; suites pass. Xvfb timing proved misleading (documented).
Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`.

## 2026-10-01 - D08Y round 5: overclock only in gameplay (Needs playtest)

User: audio slowdowns with the round-4 build. Reproduced: boot/loading at
150% fell behind real time (14,000 underruns). The overclock is now leased
from the Modernized player update (lapses 3 fields after gameplay stops) and
pauses for 5 s if emulation falls behind real time. Real GPU and audio: boot
0 underruns, gameplay 59.95 fields/s, 0-1 slow frames. Binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`.

## 2026-10-01 - D08Y accepted (Done)

User: "the stuttering and audio issues, and freezeframes are now fixed.
confirmed", after earlier confirming the slot-5 gap is made consistently.
Read-only system check on request: no Timeshift snapshot, scheduled job or
disk stall during the play sessions; Cinnamon idles at ~36% CPU and the
storage drive (sdb) reports 113 C (noted to the user, unrelated to the game).
Next job: user's choice (D08Z manual modern jump is backlogged).

## 2026-10-01 - D08Z manual jump style implemented (Needs playtest)

Measured what makes the jump feel on rails (slot 5, first person): the lip
launch near gaps, a fixed ~1 s arc with no air control, and a 14..18 field
crouch before standing/walking jumps. New Modernized option `jump`
(profile schema 20, `run.py --jump assisted|manual`, `--settings` choice J,
`DNTTK_JUMP`; Vanilla always assisted). Manual (`recomp/src/ttk/manual_jump.inc`):
jump on the press before gaps too, edge grace for every run-off (14 input
frames), bounded air steering from the owned ballistic update (98/103/104,
owned short falls, E reach 109; wade jumps excluded), preparation 96 x3.
Variable jump height and extra air speed considered and not taken. Evidence:
slot-5 timing table and same-binary regression comparison in
documentation/78-d08z-manual-jump.md; native controls/input/aim pass, Python
95 OK. Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`. Playtest: feel of steering rate and grace
(`DNTTK_AIR_CONTROL` scales the rate without a rebuild).

## 2026-10-01 - D08Z accepted (Done)

User played the manual style: "i love it. lock it in." The player's profile
keeps `jump: manual`; `assisted` remains the default for new profiles and
Vanilla is unchanged. Tuning values stay as shipped (steering 0.14 of the
takeoff speed per update, 14-frame edge grace, preparation x3). Binary
`4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`.
