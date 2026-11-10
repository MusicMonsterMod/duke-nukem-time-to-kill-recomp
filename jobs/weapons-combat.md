# Time to Kill jobs - Weapons and combat

Job descriptions and work logs for D38, D39, D44, D49 and D51. Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D38 - Play the piano with E

**Status: Done (user-accepted; closeout authorized 2026-10-29).** Selected 2026-10-29; revised to the user's
`research/music/pianobag.wav` and a piano-centred audible radius. Finish
loudness, dynamics and decay while preserving the supplied performance. On successful Modernized piano use,
play the performance while preserving the native animation. Authenticate the
current slot-5 object and interaction; prevent repeated/overlapping starts,
respect effects volume and world/load boundaries, and preserve Vanilla.
Acceptance: private slot-5 interaction/repeat/load checks, unrelated-use and
Vanilla isolation, audio mix regressions, finished render and user playtest.

#### D38A - Re-recorded saloon piano performance

**Status: Done (user-accepted; closeout authorized 2026-10-30). Dependencies: D38 (Done).** Selected 2026-10-30. The user
re-recorded the piano line on a MIDI keyboard; the accepted source is now the
audio of `research/music/pianobag-final.mp4` (local, untracked). Extract it,
trim the leading silence and cut-off tail, give it a quality/mastering pass
(mono fold-down, cleanup EQ, gentle dynamics, short room character, natural
tail fade, normalization to the D38 level) so it sits in the game's mix, and
replace `recomp/assets/audio/piano.wav`. Keep the performance's notes, timing
and pitch. Runtime behavior (native use hook, distance falloff, repeat/load
guards, Vanilla isolation) is unchanged. Acceptance: reproducible processing
with recorded hashes/measurements, build and audio regressions, a private
check of the saloon piano in a copy of UI slot 9 and the user's listening
playtest.

### D39 - Screen flash framework (Duke3D palfrom-style coloured flashes)

**Status: Done (D39A-D39G accepted 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md)).**
User, 2026-11-03: "i want to be able to do any damage or explosion flashes
that we want ... We need to really own this". Own coloured full-screen
flashes the way EDuke32's `palfrom <intensity> <r> <g> <b>` does: an
instant tint toward a colour at intensity/64 that fades by one step a 30 Hz
tic, over the world, weapon, HUD and text. D08G7 drew its flash as PS1 GPU
commands, which only approximate a variable mix (bright blues went purple)
and stepped at the game tick (staggered at 180 Hz). The renderer is ours, so
the flash moves there. Note 370 holds the measured spec, the catalog of
Duke3D's 35 palfrom uses and the design direction. Modernized only; Vanilla
never flashes. Polymorph is the target; Classic is frozen (D39G).

#### D39A - Flash spec: measure palfrom in EDuke32

**Status: Done (user-accepted; closeout authorized 2026-11-03; spec table in [note 370](documentation/370-d39-screen-flash-framework.md)). Dependencies: none.** Confirm the D08G7 measurements in
EDuke32 itself (user install under `~/Games`, the `kroztest` save; private
copies only): drive a few flashes (a test CON or the console: pickup
16 0 32, RPG hit 48 52, white 63 63 63 63, an intensity above 63, two
flashes in a row), capture frames and measure the curve. Record: decrement
per tic, the >63 hold, whether a new flash replaces or keeps the stronger,
the 2-tic peak, the blend (linear per channel in 6-bit palette units), what
is tinted (HUD, text, menus). Clean room: behaviour only, no engine code.
**Acceptance:** a measured table in note 370 that D39B/C implement.

#### D39B - Polymorph present tint

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md); no in-game trigger until D39C). Dependencies: D39A.** A framework capability in
`recomp/psxrecomp` (export with `export_runtime_patch.py`, verify against
the clean pin): a final full-frame mix(frame, colour, weight) pass on the
presented image, for the live present and render-replay redraws, with the
weight evaluated per present from the flash state and the interpolated game
time so the fade is smooth at 120/180/240 Hz (optional Duke3D 30 Hz steps).
Settle the layer (before or after host overlays such as the inventory box and
OSD). Never on menus, loading screens, FMV or the pause screen beyond what
the spec says. **Acceptance:** exact colour against the spec on captures,
smooth measured fade at 60/120/180 Hz, savestates and redraws consistent, no
cost when idle, Vanilla and Classic unchanged.

#### D39C - Game flash API and Screen Flashes option

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md), D39C section). Dependencies: D39B.** `screen_flash(intensity, r, g, b)`
with palfrom units and rules, state in guest mod memory (D08G7's word or a
small versioned record), a developer console command (`flash 48 63 63 63`),
debug JSON, and an OPTIONS / profile setting Screen Flashes On / Reduced /
Off (accessibility, D21). **Acceptance:** native tests of the rules; console
flashes match D39A captures; option persists. User, 2026-11-03: "i'm
pretty sure you can issue a color flash just from the console. we should aim
for that functionality". D39A found EDuke32 itself has none (`palfrom` at its
OSD: "unknown command or cvar"), so the console command takes palfrom
arguments directly: `palfrom 48 52` (alias `flash`).

#### D39D - God-mode off flash on D39

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md), D39D section). Dependencies: D39C, D08G7.** Replace the D08G7 add/subtract
quads with `screen_flash(32, 32, 0, 0)` (the measured god-off flash). Keep
the text and the 50% pain grunt. **Acceptance:** user's playtest against
`research/screencaps/dnkroz-dncornholio.mp4`: smooth, exact red.

#### D39E - Damage flashes from real hits

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md), D39E section). Dependencies: D39C.** From the original damage handler
`0x800a40a8` (type, amount, attacker, source object), map TTK damage to
Duke3D-style flashes (shots and fire red, explosions strong red, freeze blue,
shrink/expand magenta, toxic gas, falls), not while god mode or the steroid
shield cancels the hit. Survey the damage types first. **Acceptance:** each
mapped type flashes as specified; no flash for cancelled hits; Vanilla
unchanged; user's playtest.

#### D39F - Pickup and explosion flashes

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md), D39F section; pickups only; the user removed the explosion flashes, left for D41). Dependencies: D39C.** Pickups through `0x80081a48`
(item 16 0 32, weapon 32 0 32, armour 24 0 32), and explosions: survey the
explosion and effect spawns (e.g. `0x8006d594`, `0x80070f7c`) and add a white
flash scaled by size and distance for big blasts (the user's own EDuke32
mods flash white on big explosions). **Acceptance:** pickups and blasts
flash as specified; user's playtest.

#### D39G - Classic renderer fallback

**Status: Done (user-accepted; closeout authorized 2026-11-03; [note 370](documentation/370-d39-screen-flash-framework.md), D39G section). Dependencies: D39C.** Decision (user, 2026-11-03): no god-mode flash on Classic; the D08G7 quads are removed; the D35J5 frozen quads stay. Classic is frozen (2026-11-01): decide
between the D08G7 PS1-quad approximation and no flash, behind the same API.
**Acceptance:** Classic shows the chosen behaviour without regressions.

### D49 - Shots in step with the gun (energy beam lags the aim)

**Status: Done (user-accepted 2026-11-08: "it's really working!!", then "close this out then"; energy beam, first person; impact glow split to D49A, rounds in flight to D49B; [note 416](documentation/416-d49-beam-follows-gun.md)). Dependencies: none.**
User: "the latency between weapon projectile and the weapon movement itself,
really needs to be synchronized. it's most visible with the energy weapon
itself. when you move around, the projectile beam lags behind where duke is
pointing." While Duke turns or moves and fires, the gun and view follow the
input at once, but the beam (and likely other rounds and their flashes) is
drawn where the gun pointed a moment earlier. No open job covers this
(board checked 2026-11-08). Related finished work to read first: D17B
(input latency at high refresh), the D17 redraw path (redraws between game
ticks), D10G3/D10G4 (the shown view's world time, about 90 ms behind at
120/180 Hz) and D22D (the beam's lock and updater `0x800733C8`). Measure
the gap per weapon (beam start and end against the muzzle each presented
frame, 60 and 180 Hz, first and third person), find whether it comes from
the game tick, the redraw or the beam's own update, then draw shots from
the same aim the gun shows. Modernized only; Vanilla unchanged; hit results
must stay as they are. Acceptance: user playtest turning and strafing while
firing the energy weapon, then the other guns.

**Work log (2026-11-08, Needs playtest).** Measured on a private copy of UI
slot 8 (lab `recomp/analysis/d49/lab.py`, offscreen). Cause: the redraw, not
the game tick or the beam's update. At 120/180 Hz the late view turns the
picture to the newest mouse look and the first-person gun is drawn with it
(D14H), but the world is drawn about 90 ms back, and the beam is a world
segment from the muzzle at its shot (D22D origin record) along its shot
angles, alive about three ticks. A dx=20 turn left up to 29 degrees between
a drawn beam and the view. Fix: each beam's record also holds the view it was
aimed from; late first-person redraws move Duke's own beams (origin, start,
end unless locked, angles) by the view change since the shot, in the
worker's copy only. Gameplay, hits and D22D locks unchanged; 60 Hz, third
person and Vanilla unchanged. Console `beamfollow`, `DNTTK_BEAM_FOLLOW=0`.
After: fast and slow turns at 180 and 120 Hz draw the streak from the muzzle
toward the crosshair; still identical. `ttk-aim-test` (new D49 checks) and
`ttk-input-test` pass. Remaining: the impact glow and sparks are game
particles at the real hit point and still trail by the world delay; the
flamethrower's fireballs and Gatling wall puffs trail the same way; third
person unchanged (Duke and beam both delayed). Needs the user's turning and
strafing playtest with the energy weapon.

**Work log (2026-11-08, part 2, Needs playtest).** User: the beam's root
still left the barrel. Second cause: the first-person gun is drawn in its
presentation pose (D12/D36: anchor pushed past the near cull, per-weapon
offsets, kick) under the body camera, about 330 units from the game muzzle
the beam started at. Each first-person Duke draw now records the barrel tip
exactly as the muzzle flash places it (display hand matrix and the slot's
muzzle vector), and at the list step after Duke's draw each of Duke's beams
gets its origin there under the world camera, angles re-aimed at the end.
Lab: every beam draw at 180 and 120 Hz rooted on the barrel; still, fast
turns and running strafe start at the barrel tip. Suites pass. 60 Hz, third
person, impact glow and other guns unchanged.

**Closeout (2026-11-08, Done).** User playtest at 180 Hz: "it's really
working!! as you say the only thing that doesnt line up as such as the
impact point lags behind", then "close this out then". The impact glow is
D49A; launched rounds are D49B. Matched before/after captures (fast turn,
slow turn, running strafe; `DNTTK_BEAM_FOLLOW=0` against default) are kept
locally in `research/before-after/d49-beam-follows-gun/`.

### D49A - Impact glow at the drawn beam's end (then every gun's wall hits)

**Status: Done (user-accepted 2026-11-08: "its absolutely rock solid!", then "lock us in and close it out"; D41B impact lights stay at the real hit, not raised by the user; beam animation rate split to D49C; user request 2026-11-08, after the D49 playtest: "it's really working!! ... the only thing that doesnt line up as such as the impact point lags behind"; [note 417](documentation/417-d49a-impacts-follow-beam.md)). Dependencies: D49 (beam drawn from the barrel).**
At 120/180 Hz in first person the energy beam now leaves the barrel toward
the crosshair (D49), but the bright glow and sparks where it meets a wall
stay where the beam really hit, about 90 ms of turning behind, so on fast
turns the beam's end and its glow separate. They are game particles, not the
beam: heap records (D49 lab, [note 416](documentation/416-d49-beam-follows-gun.md))
with a list link at +0/+4, a drifting position at +0x10, the spawn point at
+0x50 (exactly the beam's real end +0x34), a word 0x800606f4 at +0x60 and
type 0x0e; static pointers to them sit around 0x800de610..0x800e10f0. Find
their list and spawner, then in late first-person redraws draw the impact
particles of Duke's own beams at the drawn beam's end (the same view change
D49 applies, or anchored to the moved end), drawing only. Then the same for
the other guns' wall-hit effects (Gatling and pistol puffs and sparks, which
trail the crosshair the same way), and decide with the user whether
D41B's impact lights follow. Gameplay, hit points, damage and Vanilla stay
unchanged; 60 Hz and third person unchanged unless agreed. Acceptance: user
playtest turning fast while firing the energy weapon at near and far walls,
then the Gatling.

**Work log (2026-11-08, Needs playtest).** The glow is not a heap particle:
D49's `0x800606f4` lead was stale stack. The beam updater spawns it through
the effects system (`0x8009cd74(0x307, ...)` at the beam's end every fourth
tick; pool `*0x800c5894`, active list `0x800deb50`, position `+0x1c`, drawn as
a billboard by `0x8009ca2c`); it lives about four game frames and starts ten
bounce-engine sparks (kind `0x04002706`, list `0x800dd7f0`, position `+0x10`)
that fall for a second or more. Bullet wall puffs are effects `0x12` (and
`0x20`/`0x21`/`0x1c` by round type) spawned at the hit by `0x8006f1b8`. Fix
(`impact_follow.inc` in `frame_replay.cpp`, `weapon_aim.cpp`): just before each
gameplay frame is published to the redraw workers, new glows at the end of
Duke's unlocked view-aimed beams take that beam's shot view, new sparks beside
them take the glow's, and new puffs at Duke's view-aimed rounds' wall impacts
(each round bound to its shot by its muzzle at the first sweep) take the
shot's view; tags live in guest memory. Late first-person redraws move tagged
records in the worker's copy by the same view change as D49's beams: glows
always, sparks and puffs for 3 frames, then fading back to their real place
over 5. Drawing only; hits, decals, bullet holes, D41B lights, locks, 60 Hz,
third person and Vanilla unchanged. `DNTTK_IMPACT_FOLLOW=0` compares;
`beamfollow off` turns both off. Lab (private slot 8 copy, offscreen): every
glow tagged at 180 and 120 Hz (0 misses); fast turns put the glow on the
drawn beam's end at the crosshair instead of behind the turn; Gatling 21/21,
pistol 12/12, shotgun 30/30 wall impacts tagged, newest puff on the
crosshair. `ttk-aim-test` (new D49A check) and `ttk-input-test` pass. Open:
whether D41B's impact lights should follow (they stay at the real hit). Needs
the user's playtest: fast turns firing the energy weapon at near and far
walls, then the Gatling.

**Accepted (2026-11-08).** User playtest: "its absolutely rock solid!" D41B's
impact lights stay at the real hit point (the user did not ask for them to
follow). The user's next observation, the beam's own animation looking slower
than the game, is D49C.

### D49B - Rounds leave the gun (flamethrower fireballs, rockets, rounds in flight)

**Status: Done (user-accepted 2026-11-08, "lock it in and close us out"; no code change: covered by D10H8; [note 424](documentation/424-d49b-rounds-covered-by-d10h8.md); user playtest: "it honestly feels pretty damn good!", "the rpg felt just phenomenally good!"; press-to-fire lag on the energy weapon and flamethrower split to D10H9; scoped 2026-11-08 from the D49 findings). Dependencies: D49.**
Rounds that fly on their own (flamethrower fireballs, RPG rockets, freezer
shots and other projectiles Duke launches) are world objects drawn about
90 ms back at 120/180 Hz, so while turning or strafing the newest ones appear
beside the gun instead of leaving the barrel (D49 lab screenshots: the
flamethrower stream off to the side of the crosshair). They cannot simply be
glued to the view like the short-lived beam: draw each of Duke's newest
rounds from the drawn barrel (D49's recorded barrel tip) and along the view
it was aimed from, blending into its real world path within a short age
(about 100-200 ms), drawing only. Measure per weapon (first and third
person, 120 and 180 Hz) before choosing the blend. Hits, flight, damage, D41B
flight lights and Vanilla unchanged. Acceptance: user playtest turning and
strafing while firing the flamethrower, then the RPG and the other launched
weapons.

**Work log (2026-11-08, Needs playtest, no code change).** This job was
scoped before D49D (moving world hits to hide delay read as sliding; the user
chose the EDuke32 feel) and D10H8 (the first-person world draw now replays
the newer game frame: new things appear about 18 ms after their game frame
instead of about 70, about 7 instead of 24 degrees of trail at 360 deg/s).
Lab `recomp/analysis/d49b/lab.py` (local; private apartment card copies,
level 1, `dnstuff`, offscreen, 180 Hz, first person): the flamethrower
launches one fireball (type 14) per game frame; still, the newest fireball is
on the crosshair; slow turns put it a little behind; the stream bends with the
turn as each fireball keeps its own shot's path, as in EDuke32. A/B against
the old draw (`DNTTK_WORLD_DRAW=pair`): the newest fireball sits nearer the
crosshair now, and only very fast turns (dx=20) leave it clearly behind. The
RPG run was spoiled (Duke killed by his own point-blank rocket) and the
freezer key did not select. Asked how to proceed, the user chose to playtest
first. If the current build feels like EDuke32, close as covered by D10H8.
Otherwise choose between the original barrel blend (risks the D49D sliding
look) and a smaller birth-point ease, where new rounds start at the drawn
barrel tip without being turned.

**Accepted (2026-11-08).** User playtest of the current build: "it honestly
feels pretty damn good!", the RPG "felt just phenomenally good". No code
change: launched rounds are covered by D10H8. The user's other observation,
a lag between mouse button down and the first shot on the energy weapon and
flamethrower ("nothing to do with the work you're doing right now"), is
D10H9.

### D49C - Energy beam animated at the display rate

**Status: Done (user-accepted 2026-11-08: "it's amazing work! lock it in and close it out"; [note 421](documentation/421-d49c-beam-animated-per-redraw.md); user report 2026-11-08, D49A acceptance: "the beam itself looks like it's a lower framerate than the game ... it looks like it's animated at a lower framerate than how we've got the rest of the game to run at our high auto value"; the user is unsure whether it is real). Dependencies: D49.**
At 120/180 Hz the energy beam looks good and stays on the gun (D49) with its
glow on its end (D49A), but its own look (texture scroll, flicker, width or
colour pulse, or the beam being respawned every game tick) may only change
at the game's tick rate while the rest of the picture is interpolated or
redrawn at the display rate. First establish whether it is real: capture the
beam at 180 and 120 Hz in first person (still and turning) and compare
successive redraws of the same game frame; find which beam fields and draw
inputs change per tick and whether the late redraws repeat them unchanged.
If they step at the tick rate, advance the beam's animated look smoothly
between ticks in the redraws (interpolate or extrapolate by redraw time),
drawing only; if it is already per frame, record that this is how it looks
and close. Hits, beam timing, damage, D49/D49A placement, 60 Hz, third
person and Vanilla unchanged. Acceptance: user playtest at the AUTO rate
firing the energy weapon, still and turning.

**Work log (2026-11-08, Needs playtest).** Real: the beam draw `0x80072868`
advances the beam's look itself each time it runs (wave phase `+0x40` by
2454 of a 4096 turn for 3 strands over 10 segments, stripe parity by the
draw count `+0x58`), once per game frame (about 30 Hz), and every redraw
reloads the frame's memory, so the wave stood still for six presents at
180 Hz. Fix (`frame_replay.cpp` `beams_animate`, `weapon_aim.cpp`): each
gameplay redraw job carries its time past the drawn list's frame and the
worker advances every live beam's phase by that fraction (short way round:
-1642 per frame, the wave flows from the gun) and the draw count by whole
frames on the same clock, worker copy only. Stripes still swap at the game
rate, as on the console. Hits, locks, D49/D49A placement unchanged; 60 Hz
and Vanilla have no redraws; third person smoothed too. Console `beamanim`,
`DNTTK_BEAM_ANIM=0`. Lab (private slot 8 copy): per-redraw phases at 180
and 120 Hz, still, turning and third person, continuous across frames.
`ttk-aim-test` and `ttk-input-test` pass. Executable
`2bff2c5ca40aeb384cb738fa3605c9b3a866853454be6e3bbd087a7f6f868160`. Needs
the user's playtest at AUTO firing the energy weapon, still and turning.

**Accepted (2026-11-08, Done).** User: "it's amazing work! lock it in and
close it out". Stripes stay at the game rate. Still captures (`beamanim` off
against default) are kept locally in
`research/before-after/d49c-beam-animated-per-redraw/`.

### D49D - Pistol and shotgun wall smoke in step with the bullet holes

**Status: Done (user-accepted 2026-11-08: "it trails behind again but its so much better", then "lock this one in and close it out"; bullet hits left at the real hit like EDuke32, the remaining trail is D10H8; first pass judged worse by the user; user report 2026-11-08, after D49A: "if you load up the pistol, or shotgun, the trailing smoke seems to be out of sync slightly with the bullets when you're firing against a wall face ... giving the illusion that it \"moves towards\" the bullet hole in the wall"; [note 418](documentation/418-d49d-holes-with-smoke.md)). Dependencies: D49A.**
At 120/180 Hz in first person, rapid pistol or shotgun fire into a wall face
while moving and turning quickly makes the smoke at each hit look as if it
travels toward its bullet hole instead of starting on it. Likely lead, to be
confirmed: D49A draws each new wall puff (effects `0x12`/`0x20`/`0x21`/`0x1c`)
under the crosshair for 3 game frames and then eases it back over 5 to its
real hit point, while the bullet hole decal is drawn at the real hit point
from the start, so the puff visibly slides onto the hole. Measure the puff
and decal positions per redraw (pistol and shotgun, still, strafing and fast
turns, 180 and 120 Hz), then make smoke and hole agree without bringing back
the D49A lag: for example move the new decal with its puff by the same view
change, shorten or reshape the settle, or anchor the puff to the hole once
the view change is small. Drawing only; hits, decals' real placement, damage,
D49/D49A beam behaviour, 60 Hz, third person and Vanilla unchanged. Test:
private copy of UI slot 8, pistol then shotgun, rapid fire at a wall face
while moving and turning quickly. Acceptance: user playtest of the same.

**Work log (2026-11-08, Needs playtest).** Lead confirmed: D49A drew each new
puff under the crosshair and eased it back while the hole stayed at the real
hit. The hole is a persistent sprite record made by the same impact
(`0x8006f1b8` -> `0x8006d848` -> `0x800a291c`, just before the puff): active
list `0x800e1ef8`, pool `*0x800c5984` (96 of 0x3c bytes), position `+0x20`
exactly the puff's point. Fix (`impact_follow.inc` only): each new hole at one
of Duke's tagged round impacts takes the same tag as its puff, and late
first-person redraws move it by the same view change and weights, so smoke
and hole appear together under the crosshair and settle onto the real hit
together. Drawing only; the game's hole records, hits, damage, D41B lights,
the beam, 60 Hz, third person and Vanilla unchanged. `DNTTK_IMPACT_HOLES=0`
compares. Lab (private slot 8 copy, offscreen): every hole tagged with its
puff (pistol 16/16 and 17/17 at 180/120 Hz, shotgun 60/60, Gatling 27/27);
180 Hz pistol fast-turn presents show the hole outside its puff before and
inside it in every frame after. `ttk-aim-test` and `ttk-input-test` pass.
Open: the pair still glides back to the real hit (D49A timing); needs the
user's playtest, pistol then shotgun, rapid fire at a wall while moving and
turning quickly.

**User playtest (2026-11-08): "unfortunately i think it looks worse".** The
hole sliding across the wall read worse than the smoke sliding onto it.
Discussion: the cause is the high-refresh world delay (50-90 ms), not hitscan;
EDuke32 draws its world 0-33 ms behind and never moves a hit (smoke and hole
are world sprites at the hit). Options discussed: (1) leave bullet hits at the
real hit, (2) shrink the world delay (new job D10H8), (3) move the decorative
hit to the crosshair for real (advised against: guessed placement, marks away
from the damage and lights, everything else still late). User chose (1):
"eduke32 feel (cleanroom) IS the goal".

**Work log (2026-11-08, second pass, Needs playtest).** Bullet wall hits are
no longer moved: the D49D hole tagging and D49A's puff following are removed,
with D49A's round impact queue (`weapon_aim.cpp` shot views, round binding,
`aim_take_impacts`) and its `ttk-aim-test` group. The energy beam's glow and
sparks still follow (D49/D49A). Smoke and hole now appear together at the real
hit and never move, like EDuke32; on fast turns the hit trails the crosshair by
the world delay (D10H8). Lab: 180 Hz pistol fast-turn presents show smoke and
hole as one world-fixed hit in every frame; energy weapon 64 glows and 220
sparks tagged, 0 misses. `ttk-aim-test` and `ttk-input-test` pass. Needs the
user's playtest: pistol, shotgun and Gatling at a wall while turning.

**Accepted (2026-11-08).** User playtest: "it trails behind again but its so
much better", then "lock this one in and close it out". Shrinking the trail
itself is D10H8.

### D49E - Laser Gatling bolts leave from the true muzzle

**Status: Done (user-accepted 2026-11-09: "lock it all in and close us out"; user, 2026-11-09, D41J acceptance: "the laser gatling gun appears to not shoot from the true muzzle like we've been working on, i'd like that to be tightened up and look better, synced properly with the laser gatling gun"). Dependencies: D10H8, D49B, D41J.**
The Laser Gatling (record 28, the Gatling's upgrade; launches projectile
type 27) does not appear to fire from its drawn muzzle the way the other
guns now do after the D10H8/D49B muzzle work (rounds drawn from the barrel,
in step with the gun). Find where its bolts are drawn and placed in their
first frames, in first and third person at 180 Hz, compared with the regular
Gatling (type 7), and make the bolts start at the Laser Gatling's drawn
barrel and stay in step with its spin and fire animation. Game-wide, not
level-specific; regular Gatling, other weapons and Vanilla unchanged; keep
the D41J bolt lights with the bolts. Test with `dnupgrade`. Acceptance: user
playtest.

**Work log (2026-11-09, Needs playtest).** Cause: the bolt (type 27) is the
game's stretched laser-streak model (0x76a), drawn by `0x8003177c` from the
pool draw loop `0x8006d08c`; the streak reaches back from the bolt by its
drawn length (+0x46), and on its first drawn tick (one tick, 2000 units,
after launch) it runs back to the launch point, the third-person skeleton's
muzzle (`player+0xbc`). In first person the gun is drawn in its own
presentation pose, so the streak came out of the gun's side; a shot from
rest also launches from the previous frame's rest-pose muzzle (as in D47).
The regular Gatling's type 7 has no model, so only the Laser Gatling showed
it; fire timing was already right (streak and flash on the same tick).
Fix: `recomp/src/ttk/bolt_barrel.inc` `bolt_at_barrel`, called from
`frame_replay.cpp` at the bolt's transform load `0x800292a0`, turns and
stretches only that draw's transform copy so Duke's type 27 streak runs on
its first drawn tick from the drawn barrel (first person: the D49
`first_person_drawn_muzzle` point, same as the flash; third person: that
draw's muzzle) to the bolt's real position. Flight, hits, lights and game
records untouched; live frames and 180 Hz redraw workers; Modernized only.
Console `boltbarrel on|off|default`, `DNTTK_BOLT_BARREL=0`. Measured gap
between the streak tail and the drawn barrel before the fix (180 Hz): first
person taps 779 units median (39 deg from the eye), held 663; third person
presses from rest 416, held 42; after, on the barrel. Matched stills
`research/before-after/D49E/`. `ttk-input-test`, `ttk-aim-test` pass.
Executable `4b911c4c301cad13648182c13701b6f7235b94a8af46715a681052c1355168b8`.
Limits: bolts are not blended between game ticks at 180 Hz (as the
original; blending would touch every round); enemy laser bolts (type 35, same
model) unchanged; the muzzle light stays at the launch point; Vanilla not
run (code is Modernized only).

### D49F - Laser Gatling bolts bend from the gun to the crosshair

**Status: Done (user-accepted 2026-11-09: "lock it in and close it out"; user chose Option 3, with Option 4 if the gun still looks off, 2026-11-09; both implemented, regular Gatling in the same pose at the user's request, [note 426](documentation/426-d49f-laser-bolts-from-barrel.md); user, 2026-11-09, after the D49E closeout: "in first person, the lasers appear to fire from the direction of the gun and then bend to the direction of the crosshair. I need some real help with that one as i cant even think of the solution here"). Dependencies: D49E.**
In first person the Laser Gatling's bolts (type 27) leave the drawn barrel
along one line, then turn onto the line to the crosshair. Likely cause, from
the D49E work (to confirm by measuring): D49E only redraws the streak's first
tick, from the drawn barrel to the bolt's real position 2000 units out. From
the second tick on, the streak is drawn on the bolt's real path, which starts
at the third-person muzzle (`player+0xbc`, about 660 to 780 units from the
drawn barrel in first person) and runs to the crosshair. The visible path
is therefore barrel to first head, then the real line: a kink about one tick
out. The drawn gun's own pointing direction and the 180 Hz tick stepping
(bolts not blended between ticks) may add to it.
Options (on the "Laser Gatling Bend" artifact, https://claude.ai/artifact/GUphhKf5yuzEYmyHtyTUZs;
the user chooses before work starts):
1. Visual convergence to the hit: draw every tick of the bolt on one
   straight line from the drawn barrel to where it will land, with the
   offset shrinking to zero at the predicted hit. Hits unchanged.
2. Visual convergence over a fixed distance: the offset fades over the
   first few ticks, so the kink moves further out and gets much shallower.
   Simplest; hits unchanged; a small bend remains.
3. Real launch from the barrel (Modernized gameplay): the bolt leaves the
   drawn barrel's world point, aimed at the point under the crosshair (a
   trace from the eye), so it really flies straight. Hits change slightly;
   needs a guard for walls right in front of the barrel.
4. Gun toe-in: turn the first-person Laser Gatling pose so its barrel points
   at the crosshair's convergence point; pairs with 1, 2 or 3.
5. Skip the first tick's streak: no bend, but the bolt no longer touches
   the barrel (undoes the D49E look).
Game-wide; regular Gatling, other weapons, third person (unless the same
kink is found there) and Vanilla unchanged; keep the D41J bolt lights with
the bolts. Test with `dnupgrade` at 180 Hz, with `boltbarrel on|off` for A/B.
Acceptance: measured path straightness before and after, matched stills in
`research/before-after/D49F/`, user playtest.

**Work log (2026-11-09, Needs playtest).** Cause confirmed by measuring: the
bolt launched from the skeleton muzzle `player+0xbc`, 400-850 units under
and beside the drawn barrel (about 1050 units ahead of the eye), and D49E
drew only its first tick from the barrel, so the path bent about one tick
out (180 Hz first person, `boltlaunch off`: 14.5 deg median on taps, the drawn
barrel 340 units off the bolt's line; 4.3 deg held; 3.1 deg held while
turning). Option 3 (`weapon_aim.cpp`, gun path, record 28, first person,
Modernized): the origin is the drawn barrel tip (the muzzle flash's point,
D49 `first_person_drawn_muzzle`) placed under the view the shot aims from
(D10G3 shown view, whose whole rotation `frame_replay.cpp` now keeps, or the
game camera at 60 Hz); the direction is still the crosshair point's. Guards:
body-to-origin cover (existing), eye-to-barrel floor and ceiling, and a
step-back toward the eye when the line would leave more than 20 deg off the
view or meet the world short of the point (looking down 36 deg the bolt
grazed the floor and struck 30-90 units past the crosshair point; now 1
unit). After: taps 0.03 deg (barrel 1 unit off the line), held 0.27, turning
0.91 (the view moving after the shot, D49D's EDuke32 behaviour); 60 Hz 0.04
and 0.30. Option 4 was needed: slot 7's yaw of -26 put the barrel 30.4 deg
off the bolts' line on screen. The Laser Gatling now has its own tilt
(-12, -8.8, 16): slot 7's pose turned in by yaw alone, keeping the accepted
raise and side view; on screen the barrel now lines up with the bolts (0.0
deg; the axis meets the view ray about 2700 units out). The barrel axis was
taken from the mesh (hexagonal prism along the hand's +y). Regular Gatling
keeps slot 7's pose. `boltlaunch on|off|default` / `DNTTK_BOLT_LAUNCH=0`
switch launch and toe-in together (exact pre-D49F state; redraw workers via
the render state). Hits: 1-6 units from the crosshair point at walls; a
pinned pig cop took 7/7 bolts at 1580 units and 6/6 at 2227. Third person and
the regular Gatling: no barrel launches. `ttk-aim-test` (new D49F case) and
`ttk-input-test` pass. Matched stills `research/before-after/D49F/`. Limits:
cover between the barrel and the target now decides what the bolt meets (as
drawn); targets inside the barrel's depth (about 1100 units) keep the
original retraction to the body; enemy laser bolts, rockets and fireballs
unchanged; Vanilla not run (code is Modernized only, asserted in the suite).
Needs the user's playtest at 180 Hz with `dnupgrade`.

**Work log (2026-11-09, part 2, Needs playtest).** User: "duke should hold the
non-upgraded gatling gun in the same new position too". Slot 7's tilt itself
is now (-12, -8.8, 16), so both Gatlings share it (the Laser Gatling's own
entry removed; override `DNTTK_FP_WEAPON_TILT7`); `boltlaunch off` shows the
old pose (-12, -26, 16) for both. Lab: the regular Gatling logs the same pose
matrix as the Laser Gatling, its rounds are unchanged; stills
`research/before-after/D49F/fp180-gatling-*`. Suites pass.

### D51 - Continuous fire stops when running into a wall

**Status: Done (user-accepted 2026-11-08: "lock it in, close it out, that is fixed!"; closeout authorized; [note 391](documentation/391-d10h6-held-fire-clock.md)). Dependencies: none.**
User: "load up slot 8. fire continuously with the energy weapon (selected
already) or flamethrower, and then holding shift, run backwards, right, or
forwards into any wall (backwards is easiest). it cuts the flow of fire.
very annoying". Reproduce on a private copy of UI slot 8 (internal slot 7):
hold fire with the energy weapon or flamethrower, hold Shift (run) and run
into a wall backwards, sideways or forwards; the stream stops. Find what
ends the fire at the wall contact (a wall-bump or blocked-run state, a
D08/D35 traversal or mantle check, the D10 aim or muzzle-cover logic,
D22D's beam lock, or the original game), whether walking into the wall does
the same, and whether other held weapons (Gatling) are affected. Fix so
held fire keeps flowing while running into walls, Modernized only, Vanilla
unchanged. Acceptance: user playtest in slot 8, both weapons, all three
directions.

**Work log (2026-11-08, Needs playtest).** Private copy of UI slot 8 (lab
`recomp/analysis/d51/lab.py`, offscreen). Reproduced: running back into the
wall with the beam held, Duke plays the wall-bump stumble 95 for about 0.3 s
and the beam stops for 633 ms, then resumes while he keeps pushing. The
stumble is the original's run collision (`0x800537f4`..`0x800538d8`);
D10H6 revision 4 (`weapon_feel_bump`) already swapped it for the original's
plain stop, but only when `weapon_feel_armed()`, i.e. the four guns with a
Duke 3D analog. Not a traversal, mantle, aim or beam-lock check. Fix: the
swap now needs only a drawn weapon (+0x3b8 2) under Modernized RETRO weapon
feel. After, first and third person: Energy Weapon and Flamethrower running
back, right and forward into walls: 33-37 ms between beam rounds, no 94/95,
`bumps_stopped` counts each contact; running left found no wall. Walking (S)
into the same wall never bumps (before or after). Gatling was already covered
(100 ms throughout). The Energy Weapon shows a single 82 ms gap running or
walking forward in open space, unrelated to walls. Vanilla: the swap is gated
by `input_modernized()`. `ttk-input-test`, `ttk-aim-test` pass;
`ttk-controls-test` stops at the known D14 HUD check. User playtest accepted
2026-11-08: "lock it in, close it out, that is fixed!"
