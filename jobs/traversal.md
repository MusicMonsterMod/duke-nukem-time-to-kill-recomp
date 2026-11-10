# Time to Kill jobs - Traversal

Job descriptions and work logs for D08 movement, jumping, climbing, swimming,
jetpack and pushing (D08A and its children are in items). Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D08 — Modern traversal controls — accepted iteration

**Done on explicit user acceptance.** The user confirms every tested ladder jump
behaved as expected, E reliably caught the next ladder, and the controls achieved
the requested fluidity. This closes the current movement/interaction/ladder
iteration, including Escape/Start, running continuity, automatic interaction
holstering/redraw, and the second-ladder transfer fix.

Evidence: [state matrix](documentation/25-traversal-state-matrix.md),
[transfer reproduction and delivery](documentation/31-ladder-transfer.md), and
[recorded checks](documentation/reports/d08-ladder-transfer.json).
The original broad D08 coverage is retained explicitly under **D08B**, rather
than claiming swimming, every script/map, or full campaign validation is complete.

The later **D08C** standing/walking jump follow-up is also **Done** on explicit
2026-09-27 user acceptance: W→Space and Space→W during takeoff preparation both
work. See [accepted jump contract](documentation/34-standing-directional-jumps.md).
This adds accepted input-order flexibility without reopening D08 or claiming
full airborne steering or broader D08B terrain/campaign coverage.

### D08B — Broader traversal and scripted-camera coverage

**Done on user acceptance (2026-09-29).** User: "d08b can be marked as
completely done man, its fine. accept it all" and "i have tested all of
them!". The user reports playtesting every limit recorded below; they are
kept as history only.

**Latest user feedback supersedes the earlier replay:** contact jumps remain
inconsistent; running off the bed still has residual slowdown; early pipe-bomb
pickup from the unopened bed is reproducible by the user again. Switch targeting
is awkward. Focused follow-up is D08H; none of these are declared fixed.

**Earlier furniture iteration — bounded automated evidence.** Close-contact bed jumps
recheck clearance while rising; normal walking can step off small furniture drops;
held movement survives short-drop landing. Final replay verifies bed contact entry,
bed/couch walk-off and continuous Shift-run landing. Central couch entry works;
outer-end/oblique approaches and the reported unopened-bed pipe-bomb pickup remain
open. See [furniture handoff](documentation/37-furniture-traversal.md). D08B remains
In progress for these limits and its broader campaign coverage.

**In progress — bounded fixes delivered 2026-09-27.** Attached input now checks
live gameplay context; E-owned redraw survives pause/inventory; verified attached
states can automatically recapture after pause. Final isolated replay verifies
recapture without F10, descent and bottom-exit redraw. A later ascent with a
pig cop on the landing stalls despite correct Up input; keep that obstruction
case open. See [implementation and evidence](documentation/35-traversal-and-apartment-followup.md).

The bounded standing/walking directional-takeoff follow-up is tracked as **D08C**;
its evidence does not close this broader job.

Carry forward the unverified portion of the original D08 scope: other ladder and
ledge types, oblique/diagonal approaches, standing/walking jumps, crouch/roll,
swimming/underwater, scripted cameras, other maps, and transitions through death,
respawn, inventory and level changes. Preserve the accepted D08 implementation.

**Acceptance:** expand the [state/ownership matrix](documentation/25-traversal-state-matrix.md)
with actual scenario evidence; required progression remains possible and modern
camera/input returns correctly after original-owned states. Repeat the first
security-card route with the delivered controls and retain unresolved scene-exit
and campaign/stability follow-ups. Prior successful user progression remains
valid evidence; it does not establish the earlier freeze's exact cause. Coordinate
full campaign/stability/audio coverage with D22/D23/D18.

### D08C — Directional jumps from standstill — accepted

**Done on explicit user acceptance (2026-09-27).** The user first confirmed
W→Space worked, then accepted the Space→W follow-up: “its working!! beautiful.”
This closes the bounded standstill/walking directional-jump and input-order iteration.

W/A/S/D (including diagonals) may precede Space or arrive during original takeoff
preparation after Space. No run-up is required. The first chosen direction persists
through early release; Space alone remains vertical. Original preparation is not
restarted, and original clearance, directional-jump arc, gravity and collision
remain. Running-jump selection, E interaction-only, C holster, precision aim,
wheel/Alt-wheel, Vanilla and customized bindings remain regression requirements.

Accepted delivery: `92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.
[Current engineering contract](documentation/34-standing-directional-jumps.md),
[final automated and human evidence](documentation/reports/d08c-input-order.json),
and [earlier implementation evidence](documentation/reports/d08c-standing-jumps.json).

Acceptance does not claim every diagonal, ceiling, moving support, map or ladder
was individually human-tested. Broader terrain/traversal/campaign coverage stays
under D08B. This feature accepts direction during preparation; it does not add
midair steering. Do not reopen the accepted feel/input-order work without a new
report or user-selected change. No other job is authorized by this sign-off.

### D08D — Apartment light-switch secret convenience

**Done — user confirms natural switch-first access, 2026-09-27.** Modernized removes only the
conversation prerequisite from the exact authenticated LEVEL00 lights-off
condition. Vanilla keeps the original condition. Native checks and the original
switch/bed replay verify opening with the prerequisite unset, one-time counting,
repeat toggles, subsequent dialogue and natural pipe-bomb pickup/6 selection.
The prerequisite-negative replay uses an explicit isolated conversation-bit reset;
the user has now verified untouched natural switch-first access. See
[the fixture boundary and delivery](documentation/35-traversal-and-apartment-followup.md).

User-requested Modernized convenience: switching off the apartment lights should
open the bed secret without first talking to the woman. User reports lights-off
alone did not move the bed; after turning lights on, talking, then switching off,
the bed moved and pipe bombs became accessible. Original intent/cause is unverified.

**Acceptance:** verify the exact LEVEL00 script/trigger and original prerequisites;
in Modernized, lights-off opens the secret with or without prior conversation,
including repeated toggles and already-open state. Preserve dialogue, reward,
collision and progression; Vanilla retains original behavior. Verify live script
identity before changing behavior; no guessed global flags or inventory grants.

### D08E — Responsive holstered fire and weapon transitions

**Done — user accepts held fire, release cancellation and faster transitions.**
Bound fire draws through the original action,
then fires while held. Equip/stow upper tracks advance at 1.5x with original events;
Vanilla and firing speed remain unchanged. Pistol live checks, four native suites,
57 Python tests and a 14-checkpoint Vanilla route pass. Broader weapon/era feel
remains open. See [delivery and evidence](documentation/36-weapon-response-and-furniture.md).

User requests fire to draw a manually holstered weapon, then
shoot while fire remains held, plus modestly faster draw/holster animations.
Use the bound fire action, preserve C and E ownership, original animation events,
weapon/ammo rules, attached traversal and Vanilla. Do not queue surprise shots
after release, focus loss or menus. Verify actual draw latency and no fire-rate change.

### D08G — Typed Duke-style debugging cheats

**Done for the delivered bounded scope:** user says the cheats work fine on
2026-09-27. Exact confirmation wording is now D08G1; font presentation is D19A.
Existing hidden-enemy save/load and broader-campaign limits remain documented.

User-selected 2026-09-27. Add familiar `dn...` cheats for private gameplay testing:
prioritize enemy-only hide/show (`dnmonsters`), invulnerability (`dnkroz`), keys
(`dnkeys`) and weapons/items (`dnstuff`), using verified original TTK paths where
available. Document supported equivalents and omissions; preserve Vanilla and
context/input isolation. Verify toggles, repeats, non-enemy preservation and reset.

### D08G1 — Original Duke3D cheat confirmation wording

**Done — 2026-09-27.** Private OpenGL captures verify every required label and both
toggle directions. Parsing, effects and guards are unchanged. User accepts cheat functionality and requests the original confirmation
labels, using their [InfoSuite reference](https://infosuite.duke4.net/index.php?page=references_cheats).
Change supported-code success messages, preserving capitalization and punctuation:

| Codes | Required success text |
| --- | --- |
| `dnkroz`, `dncornholio` | `God Mode: On` / `God Mode: Off` |
| `dnstuff`, `dnitems` | `Giving Everything!` |
| `dninventory` | `Got All Inventory` |
| `dnweapons` | `Got All Weapons/Ammo` |
| `dnkeys` | `Got All Keys` |
| `dnhyper` | `Steroids` |
| `dntlod` | `Play Zaxtor's Oblivion!` (D08G6; fun notice only) |
| `dnmonsters` | `Monsters: Off` when hidden / `Monsters: On` when shown |

**Acceptance:** verify each supported result and both toggle directions on screen;
no change to code parsing, gameplay effects or context guards. Keep useful rejected
request feedback and distinct labels for the three added debug codes. Wording must
not imply that TTK gained Duke3D armor/jetpack semantics. The supplied larger list
is a reference, not authorization to advertise unsupported cheats as implemented.

### D08G2 — Silent cheat entry and centered confirmations

**Done by explicit user acceptance, 2026-09-27:** entry is silent and results centered as requested. The implementation-stage evidence below remains valid.

**Implemented and privately verified, then user-accepted.** Silent parser entry and
centered/wrapped results verified in both renderers at three sizes. See
[evidence](documentation/44-traversal-inventory-implementation.md). User wants no prefix/partial-code echo,
no cancellation chatter, and a result only after a supported complete cheat.
Retain the accepted exact labels, Duke glyphs, cheat effects and context guards.
Center confirmations horizontally in the Duke3D-style upper message area instead
of top-left. The user said “middle of the screen” and “like Duke3D”; the reference
centers horizontally, not over the vertical-center aiming reticle. Record this
placement interpretation and verify the actual captures.

**Acceptance:** valid complete codes produce one correct result; partial, invalid,
cancelled and timed-out typing stays silent. A recognized but refused request may
retain a concise truthful refusal (never a false success); no unsolicited typing
status. Preserve input isolation/reset and added debug-code result labels. Verify
both toggles/aliases, resize/wrapping/transparency/expiry, repeated messages, both
renderers and Vanilla. Do not move unrelated OSD panels or slow normal controls.

### D08G3 - `dnupgrade` cheat: upgrade every weapon

**Accepted by the user, 2026-10-06** ("accepted!!"). Implemented and verified live in levels 6 and
12 (medieval and Rome) on private copies; see the work log and
[note 38](documentation/38-debug-cheats.md). Upgrades owned weapons (ammo
carried to the upgraded form) and makes the rest arrive upgraded. Grants no
weapons.

**User request, 2026-10-06.** "another cheat - dnupgrade, which
upgrades all weapons to their upgraded form. laser gatling gun etc."

**Scope:** a new typed cheat in the D08G family (silent entry, centered Duke
font confirmation as in D08G1/D08G2, same solid-ground rule as the other
cheats unless research shows it is safe elsewhere). It gives each weapon the
original upgrade the game itself grants through its upgrade pickups: known
pairs from note 33 are inventory flag 8 resolving Gatling 7 -> 28 Laser
Gatling, RPG 8 -> 29 Incendiary RPG and Flamethrower 9 -> 27 HiTemp
Flamethrower (`8003df40` resolves upgrades). Research first: the complete
list of upgradable weapons per era, how an original upgrade pickup sets the
flag and ammo (the upgraded form reads ammo from its resolved record), what
happens to a weapon Duke does not own yet (decide with the user: upgrade only
owned weapons, or grant and upgrade like `dnstuff`), whether the currently
drawn weapon switches to its upgraded model immediately, and save/load,
level change and death/Continue persistence. Use original data paths only.

**Acceptance:** typing `dnupgrade` shows its confirmation and every eligible
weapon becomes its upgraded form (Laser Gatling, Incendiary RPG, HiTemp
Flamethrower and any others found), firing and switching normally, in at
least two eras; the upgrades persist through a save/load; other cheats and
Vanilla unchanged; the user confirms.

### D08G4 - `dnstuff`, `dnitems` and `dninventory` also give 100% armor

**Done (user-accepted 2026-10-08: "fully accepted").** Full armor from all three. **Scope changed by
the user when selected** (acceptance 4 below is superseded): "if steroids are
already running, and the user types one of these cheat codes, steroids should
be filled back up, and the usage is also stopped ... user types dnstuff,
steroid usage stopps, full steroids are in inventory. the idea is that armor
has nothing to do with steroids in the new world. typing in any of these
codes would replenish armor to 100 either way". See the work log entry of the
same date and [note 38](documentation/38-debug-cheats.md).
User request, 2026-10-08.

**Change.** Each of `dnstuff`, `dnitems` and `dninventory` additionally sets
Duke's armor to 100%. That is the whole gameplay change. Today none of them
gives armor ([note 38](documentation/38-debug-cheats.md) says `dnitems` "does
not add a separate armor grant"; update that note).

**Do not change** anything else they do: weapons, ammo, inventory and
charges, keys, the D08A9 rule that the inventory grant leaves the gadget
selection alone, confirmations, the solid-ground rule. Do not make the three
equivalent or merge their implementations beyond what they already share
(`cheat_codes.h`, `cheats.inc`); other cheats (`dnweapons`, `dnkeys`,
`dnhyper`, ...) are out of scope.

**Steroid independence (see D08A13).** The armor grant must not touch steroid
state: player `+0x364` item-4 flags and the `+0x366` timer stay exactly as
they were. Example: armor 25% and steroids at 40 s before the cheat; armor
100% and steroids still at 40 s after.

**Check first:** the armor field (`+0x234`, which the status bar shows as
`armour / 100`, note 127) and the value an original full armor pickup sets,
so "100%" is the game's own full armor, not a guess; whether setting it
directly needs anything else the pickup does (HUD refresh, a flag).

**Acceptance.**
1. `dnstuff` with less than full armor: armor becomes 100%; everything else
   as before.
2. `dnitems`: same.
3. `dninventory`: same.
4. Each of the three while steroids run (an R dose and `dnhyper`): armor
   becomes 100% and the steroid timer and flags are unchanged (compare
   `+0x366` against the drain alone).
5. Armor shows in the status bar armor element; other cheats unchanged.

### D08G5 - Partial cheat entry must not cause mouse-look stutter

**Done (user-accepted, 2026-10-18).** User: "perfect!!". Executable
`18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79` is the regression baseline. User report, 2026-10-09: type part of a
cheat, for example `dnstu`, then move the mouse. Duke stutters until a key such
as W breaks the pending sequence. Prevent the incomplete cheat sequence from
disrupting normal play. Reported during the D08A20 playtest.

**Acceptance:**
1. In Modernized, enter `dnstu`, stop typing and move the mouse: camera and
   Duke remain smooth without needing a keyboard press to recover. Check
   first- and third-person views, plus other valid partial cheat prefixes.
2. Movement can interrupt a partial sequence immediately and normally;
   completing a valid cheat still triggers it once, with the existing silent
   entry and confirmation behavior preserved.
3. Invalid/cancelled sequences leave no lingering input suppression or camera
   disturbance; Vanilla behavior remains unchanged.

**Work log (2026-10-18):** cause, from `pc_input.cpp`: while a prefix of at
least `dn` was pending, `input_event()` returned early for every non-keyboard
event, so mouse motion, buttons and wheel were thrown away until the sequence
ended (a non-prefix letter, a non-letter key or the three-second gap). Starting
the prefix also ran the full `clear()`, which bumped the input epoch and reset
the look accumulators. The key that broke the sequence (W) was consumed, so it
needed a second press. Fix (PC input layer only, no framework or generated
code change): typing owns the keyboard only. Mouse motion and wheel stay live;
a mouse click cancels the prefix and acts normally; starting a prefix only
releases held keys and the pending jump (`release_keys()`), keeping the look
epoch and totals; a letter that breaks the prefix, or any non-letter key press,
resets the prefix and then acts as its normal key in the same event. Completed
codes, silent entry, confirmations, the focus/F10 cancel and the timeout are
unchanged. Vanilla never reaches this path (capture is Modernized only).
Evidence: `ttk-input-test` PASS with new checks: for the prefixes `dnstu`,
`dnk`, `dnmon`, `dnwea` and `dnu`, 20 motion events during the prefix reach
the gameplay snapshot exactly (300/-80 counts, same epoch), W then moves at
once (`move_y` 1) and releases cleanly, and no cheat fires; Space and a left
click break `dnst` so a following `uff` does not trigger `dnstuff`; every code
still completes once. The motion check FAILS against the previous source
(line 437), confirming it captures the reported stall. The full build gives
executable `18597e61f8fe332da32aff5154465d533dd852e6baefde24017766b109f23b79`
(reproducible: rebuilt twice to that hash). The old `pc_input_probe.py
--controls movement` Xvfb route stops at "independent diagonal without PS1
turn bits" on both this build and the accepted baseline `67eb7bc2...`, so that
is a stale probe expectation, not a regression. Accepted by the user after
their playtest.

### D08G6 - Fun cheat `dntlod`: "Play Zaxtor's Oblivion!"

**Done (user-accepted, 2026-10-09).** Typed cheat `dntlod` shows the centered
confirmation `Play Zaxtor's Oblivion!` only (same silent entry and solid-ground
notice path as the other D08G cheats). No game-state change. Executable
`74cec66e398217530d0b8abc3fa0ccfe06308f6b73a17e5aafa628b3f0f2a540` is the
regression baseline.

**Work log:** `Cheat::Tlod` / `dntlod` in `cheat_codes.h`; `cheats.inc` calls
`input_notice("Play Zaxtor's Oblivion!")` and returns. Manual, note 38 and the
D08G1 wording table updated. `ttk-input-test` PASS (types every code including
`dntlod`); `ttk-font-test` PASS with the new string. User: "accepted!"

**Acceptance:** in Modernized, on solid ground with capture, typing `dntlod`
shows that message once and does nothing else; other cheats and Vanilla
unchanged; the user confirms.

### D08G7 - dnkroz feel: GOD MODE ON / OFF text, red flash and pain sound on OFF

**Status: Done (accepted as preliminary 2026-11-03; [note 369](documentation/369-d08g7-kroz-feel.md)). The flash moves to the D39 framework in D39D. Dependencies: D08G2, D08Q4.** Make `dnkroz` (and
`dncornholio`) behave like Duke 3D / EDuke32. Reference:
`research/screencaps/dnkroz-dncornholio.mp4` (28 s). The code toggles: the
top-of-screen text reads "GOD MODE: ON" / "GOD MODE: OFF". Switching **off**
flashes the screen red (the damage tint), and Duke sometimes makes a single
short pain sound, the one-hit pain he makes when stepping in acid or fire or
getting burnt; the sound does not play every time. Switching **on** shows no
tint in the reference.

Initial research: `cheats.inc` toggles `0x800c3cc6` and shows "God Mode:
On/Off" (`input_notice`); no flash or sound today. Find the original TTK
damage tint and the matching short pain/burn voice, and trigger them through
original routines (not a host overlay) without dealing damage; decide the
"sometimes" rule from the reference (chance or cooldown). Wording follows the
D08G1 Duke3D confirmation style.

**Acceptance:** dnkroz and dncornholio identical; ON text with no tint; OFF
text with the red flash and, sometimes, the short pain sound; no health or
armour change; Vanilla unchanged; the user's playtest against the video.

### D08G8 - Cheats work anywhere, not only on solid ground

**Status: Done (user-accepted 2026-11-02). Dependencies: D08G2.** Today every typed cheat in Modernized
is refused off the ground with "Cheat unavailable here - stand on solid
ground". The user wants cheats accepted anywhere: in the air, swimming,
jetpack flight, climbing, ladders, pushing, and so on.

Initial research: one shared gate at the top of `cheats_update`
(`cheats.inc`) requires the ground movement lease (`movement_ready()`) as well
as gameplay context, player identity and the guarded guest stack, and
`cheats_update` runs only from the ground player-update path. Many cheats are
plain RAM writes (god, unlimited ammo, health, ammo, upgrade, steroids,
`dntlod`); `dnstuff`, weapons, inventory, keys and items call original
routines (`8003d7bc`, `8003d738`, `8003d840`) that need a safe guest call and
may assume a ground state. Work: run cheat processing for every player state
(swim, flight, traversal), keep the guest-call guards, and verify each
cheat's original routine outside the ground state (weapon/item grants during
swim, flight and attached traversal) before allowing it there. Refuse only
where genuinely unsafe (cutscenes, menus, death unless D08G9), with a precise
message.

**Acceptance:** every cheat works in the air, swimming, under water, in
jetpack flight and on ladders/climbs, with no stuck state, weapon or pose
glitch; Vanilla unchanged.

### D08G9 - dnkroz after death: Duke gets back up and play continues

**Status: Done (user-accepted 2026-11-03; [note 375](documentation/375-d08g9-dnkroz-revive.md)). Dependencies: D42, D08G7, D08G8, D08Q4.** In Duke 3D typing
`dnkroz` (or `dncornholio`) while dead brings Duke straight back with full
health and god mode on, and play continues. Reference:
`research/screencaps/dead.mp4` (32 s): killed ("PRESS OPEN TO RESTART
LEVEL"), the code is typed, "GOD MODE: ON", Duke is up at 100 health; after
being blown to pieces the gibs stay on the floor and Duke simply carries on
from there.

Whatever the death (shot, burnt, drowned, blown up with nothing left of the
body), a fresh living Duke continues from where the old one died; the remains
(corpse, blood, entrails) stay in the world. Initial research: god mode
currently only tops up a living Duke (`god_mode_update` requires health > 0)
and the cheat gate refuses outside the ground lease, so death needs its own
authenticated path. Research the native death states/animations (e.g. the
227 death pose), the game-over/restart flow and camera, the player actor
versus gib actors, weapons/inventory retained, and whether the original has a
respawn or level-restart routine to reuse rather than resurrecting by hand.

**Acceptance:** dnkroz during any death (including gibbed) returns a living,
controllable Duke at the death spot with god mode on and his inventory; the
remains stay; first and third person; savestates and Vanilla unchanged.

**Update 2026-11-03 (user): blocked on D42.** The revive builds on D42's
visible death state: Duke dies and stays on the death scene, the console is
reachable, `dnkroz` (or `dncornholio`) typed there and accepted by the
existing cheat system returns Duke to a valid playable state with no level
restart, the death menu never interferes, and the selected first- or
third-person view comes back. Do not assume the original cheat code can
revive. Investigate before implementing: the existing dnkroz handling
(`cheats.inc`, D08G7, D08Q4, D08G8's "refused while dead" gate), death and
player-state transitions, health and invulnerability, animation state
restoration (death pose, D37A save layouts), weapon and input state
(leases, held keys, the D08A switcher), camera restoration (D35L death
flags revoke the modern view), and conflicts with the recomp's own systems
(D36M / D39 death fades, D08A14 steroids cleared at death, D08Q2 jetpack,
D18D music). Decide whether dnkroz alone revives or a separate recovery
step (for example the original Continue respawn at the death spot) does it.
Added acceptance: the console stays usable while dead; Duke is never left
in a death animation or camera; movement, weapons and interactions resume;
first and third person tested; dnkroz while alive unchanged (D08G7 text,
D39D flash, D08Q4 top-up).

### D08G9 update

D08G9 (dnkroz after death) now depends on D42 as well: the revive happens
from D42's visible death state, with the console reachable and the death
menu closed or out of the way. Its text gains the user's 2026-11-03
requirements in its own section (2026-11-02 intake, "Update 2026-11-03").

### D08H — Apartment furniture, hidden pickup and switch targeting

**Done — user accepted remaining playtest, 2026-09-28.** Movement was already
accepted 2026-09-27; remaining pickup/switch checks were accepted in ongoing
play. Broader campaign furniture stays under D08B.

**Implemented; movement portion explicitly accepted (2026-09-27).** User reports perfect close-contact
bed entry, smooth walk/run departures without the pause, and easier couch entry.
They did not explicitly retest concealed/exposed pipe bombs or switch/NPC order
in this report; do not invent those results. New run-start/edge jump misses and
armed ladder grabs are D08I/D08J, not a rejection of accepted furniture clearance.

Original scope: See [reproduction and verification](documentation/40-feedback-implementation.md). Focused child of D08B. Original request: revisit the whole apartment route: touching the bed
and W+Space from rest; tiny-step-back and running approaches; bed/couch walking and
running off edges; the remaining drop/landing slowdown; E targeting the light
switch from reasonable nearby angles; pipe bombs collected through the end/pillow
of the unopened bed. The latest user report confirms early pickup still occurs;
the earlier non-reproduction was not a fix.

**Acceptance:** close-contact jump gains forward motion as soon as physical
clearance permits, without clipping; walk/run edge departure and landing maintain
appropriate held movement smoothly. Release still stops normally; intentional
large falls/obstructions remain physical. Prevent premature hidden pipe-bomb
collection while preserving normal collection after moving the bed. Verify light
switch targeting and lights-before-conversation secret behavior together. Use fresh
private level state plus repeated open/close and varied approach tests; preserve
NPC interaction, other pickups, ladders and Vanilla. Avoid a broad pickup-radius
change or apartment coordinate patch without establishing the actual cause.

### D08I — Responsive run-start and edge jumps

**Done — explicit user acceptance (2026-09-27).** Close bed on/off jumps accepted
after repeated physical attempts; earlier automated close-bed chord comparisons
are retained as supporting evidence. Preserve six-frame edge grace/furniture
behavior. Broader campaign edge coverage remains under D08B. See
[iteration 47](documentation/47-playtest-iteration.md).

**Follow-up: Needs playtest.** Nineteen valid close-bed comparisons retained directional motion after original feet clearance; the reported intermittent lost direction did not reproduce. No speculative traversal change. Original report and hardware/input/geometry limits remain open; see [current evidence](documentation/46-turning-audio-inventory-progress.md).

**Needs playtest; reproduced, fixed and privately verified.** Run-start stride
lockout and separate six-input-frame small-edge grace are documented in
[evidence](documentation/44-traversal-inventory-implementation.md). User reports Shift+W followed a split second later by
Space sometimes fails to jump. Near a bed/platform edge, a late jump also sometimes
fails. Reproduce these separately, including outside apartment platform transfers,
and distinguish input loss, gait transitions, grounded support and airborne state.
Existing jump buffering and guarded animation handling are leads, not proven causes.
Duke3D is the responsiveness baseline; preserve accepted W→Space and Space→W,
furniture contact clearance and smooth departures.

**Acceptance:** sweep measured press offsets around standing/walking/run start,
Shift order and edge departure; record input receipt, consume/expiry, gait, support,
takeoff and velocity. A valid grounded press must survive a brief gait transition
and execute once at the earliest legal update. If a short edge-grace/input-buffer
window is needed, bound and document it separately; no double jumps, indefinite
queued jumps, wall clipping or increased jump reach by accident. Test bed/couch,
exterior platforms, obstructions, large falls, release, FPS/timing variation,
focus/menu/scene invalidation and Vanilla. Give latency evidence plus human test.

### D08J — Armed airborne ladder grabs and automatic weapon transitions

**Done — explicit user acceptance (2026-09-28).** Armed run-jump+E to the second
exterior ladder succeeded repeatedly; player cleared the job. Preserve E-owned
4× stow (ground+air) and aim-held flight lease. See
[iteration 48](documentation/48-armed-ladder-reliability.md) and
[plan](documentation/49-controls-hud-plan.md).

**Needs playtest — armed reliability pass (2026-09-28).** Player can grab while
armed but misses more often than holstered. E-owned stow now boosts 4x on ground
and in flight; jump+E refreshes the lease; shorter airborne holster pulse.
Private early-E and nominal armed attaches passed. See
[iteration 48](documentation/48-armed-ladder-reliability.md).

**Done for bounded user-tested scope, 2026-09-27:** armed run/jump/E exterior ladder transfer succeeded twice and was explicitly accepted. Broader weapon/era coverage remains limited; preserve this behavior.

**Reproduced, fixed, privately verified and user-accepted.** E-owned airborne
stow including its final blend and pending intent through weapon switches now
pass pistol/shotgun ladder, exit and ammo checks. See
[evidence](documentation/44-traversal-inventory-implementation.md). User could not run-jump with pistol drawn and press E to
catch the second ladder outside the apartment. They want climbing and transfers
to manage required stow/redraw automatically without knowing C. This extends the
accepted holstered transfer route; reproduce armed and holstered cases separately.

**Acceptance:** walk-up to first ladder and run-jump/E onto second ladder succeed
in valid contact windows with pistol drawn, holstered and during weapon transition.
Trace original ladder eligibility, interaction intent lifetime and upper-body
stow; prove the failing path before choosing a fix. Use original attachment and
weapon events, remember intended weapon and restore safely on exit. No remote
snapping, through-wall grabs, forced fire or stale E after focus/menu/scene change.
Repeat from multiple approach timings; preserve ordinary E targets, accepted
ladder transitions, movement, aiming, ammo and Vanilla. Broader weapons are tested
by materially different stow tracks rather than assumed from the pistol.

### D08J1 - Hold-E run-up grab for overhead ladders (slot 6)

**Done (2026-10-01, user-accepted: "genuinely working solidly").** User: the ladder at save slot 6 is awkward to
get to; "I want to just be able to run up to it while holding E and i grab it,
but duke seems to bounce off of it."

Finding: that ladder's bottom rung hangs about 1040 units (about Duke's own height)
above the floor, so the original ground mount
(`0x80051cf0`, Up + Cross, 185) never sees it: walking or running into it with
E stops at the wall (94), and only an E jump catches it in the air. Jumps with
E caught it from good distances but met the wall low and bounced (107) from
others (walking at 400 and 1300 units). See
[measurements, change and evidence](documentation/79-d08j1-ladder-leap.md).

Change (Modernized only): with E held on the ground, holstered (E stows
first) and heading at a plain ladder whose bottom is 300..1400 above the
floor, the host presses the original jump once per E hold when Duke is inside
the catching window for his gait (running 750, walking 800, standing 450
units from the panel). The reach stays held through the leap (an E tap at the
wall works too), and in the air Duke eases along the panel into its width
when he started up to 180 units beside it. The original reach makes the
catch and climbs. Vanilla, jumps without E and ladders mountable from the
floor are unchanged.

**Acceptance:** at the slot-6 ladder, running or walking at it with E held,
or standing below it and pressing E, grabs it without Space, armed or
holstered, first and third person, both jump styles; no new grabs without E;
D08J/D08U/D08X ladder and ledge routes unchanged; user playtest confirms the
feel.

### D08J2 - Poles and chains: A/D turn the wrong way

**Accepted (2026-10-07, user: "fully and completely accept it all").**
Executable `129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b` is the new regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`129a64f2df3f5395f2d3485a310a03d74c85f9b1086c7dab37fee59277c7968b`. Cause: original. On a pole or
chain (hang-climb 192..195) Left/Right reach the sidestep `0x800439e4`,
where D-pad Left carries Duke round the object to his own right (Vanilla
private run confirms it); our A was D-pad Left. Fix (Modernized, by state, so
every pole and chain): A sends D-pad Right and D sends Left there
(`pole_sidestep_ready()`, `pc_input.cpp`). Ladders (186..189), climbing
walls and Vanilla unchanged. Private slot-3 runs: A goes left, D right, in
the third-person and first-person profiles; W/S climb; suites pass.
[Note 122](documentation/122-d08j2-pole-chain-sidestep.md).

**Todo. User request, 2026-10-07:** "when climbing poles or chains, left and
right are inverted, A turns right, and D turns left! i would like the
opposite to happen." In Modernized, while Duke is on a pole or chain, A
should turn (or circle) him left and D right, matching the screen.

**Research start:** find the pole/chain climb states (attached traversal,
compare the ladder anims 147-156 / 185-211 in `traversal_state_ready()`) and
how A/D reach the original there (D-pad Left/Right turn buttons, L2/R2
words, or the traversal camera-only lease that keeps the original
directional buttons, D08V). Establish whether the original itself turns this
way relative to the camera (third-person view from behind or in front of the
pole) or whether our mapping flips it; fix it game-wide for every pole and
chain, not one level. Check ladders are unaffected.

**Test location:** the user's UI slot 3 is a good place to playtest chain
climbing (user, 2026-10-07). Test only on dated private copies of that
savestate; verify its hash first and never use the player's cards.

**Acceptance:** in Modernized, on every pole and chain tried, A turns Duke
left and D right as seen on screen, in third person (and first person where
supported); ladders, ground movement and Vanilla unchanged; the user
confirms.

### D08J3 - Free camera while on ladders, poles and chains

**Accepted (2026-10-07, user: "we have an absolute winner once again!! it's
working well, very fun, looks great").** Executable
`5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520` (with climbs centring the view) is the
new regression baseline.
**Built 2026-10-07 (Needs playtest).** Executable
`2c94f72ffcb081320298ae37b3dd5bf66dee5f1f7dd654ee705d493a6cd6e593`. Finding: the climb keeps the
normal camera and Duke's pivot; in mode 3 the original only swaps in a high
look-down boom (0, 2000, -2000), and the host orbit lease stopped there (it
already covered mounts, the top exit, mantles, hangs and ceilings). Change
(Modernized): mode-3 climbs (147..156, 185..211, with entry frames) join the
camera-only traversal lease, so the mouse orbit, zoom, shoulder and V recentre
work while climbing; the original still moves Duke. First-person profile shows
the orbit during climbs (the D08V hang rule). Poles/chains: A/D follow the
screen from any side (D08J2 swap undone when the camera looks at Duke's
front, latched while a key is held). Private runs: ladder slot 5 full orbit
and climb, chain slot 3 A screen-left / D screen-right from behind, front and
side, ladder top mount / enemy exit / descent regression unchanged, Vanilla
original boom; suites pass. User playtest: "it works perfectly!"; follow-up built: climbs centre the
view whatever the H shoulder setting (eases back after), executable
`5a405e4fdb659cffaf8beb8ff9241a23c7157cf888b18ec57b9b1eb130c7b520`.
[Note 124](documentation/124-d08j3-climb-camera.md).

**Todo. User request, 2026-10-07:** "free camera while on ladders/climbing,
and of course this will require deep investigation and user friendliness
testing." In Modernized, the mouse camera should stay free (orbit and look)
while Duke climbs, instead of the original climb camera.

**Research start:** what owns the camera on ladders, poles and chains today
(the D08V camera-only traversal lease, attached states, original scripted
climb cameras), how climbing input is resolved relative to the camera (up /
down / sideways, mount and dismount, the D08U ladder-top mount and D08J
grabs), and what breaks if the view is free (direction of climb input when
looking away, collision with the wall the ladder is on, first-person).
Prototype behind the Modernized profile and test usability with the user
before settling defaults (for example free orbit with automatic recentre,
or a limited look range). Chain climbing: the user's UI slot 3 (see D08J2).

**Acceptance:** in Modernized, the camera can be moved freely while
climbing ladders, poles and chains; climbing controls stay predictable from
any view; mounting, dismounting and ladder-top/airborne grabs still work;
no camera clipping regressions; Vanilla unchanged; the user confirms after
usability testing.

### D08J3A - Climb camera revisit with aiming

**Status: Done (user-accepted and closed 2026-11-07: "excellent work!!! well done! lock it in close it out. testing was solid. i love it"; [note 400](documentation/400-d08j3a-climb-camera-revisit.md)). Dependencies: D08J3, D08J6B, D08J6D, D10F.** The free camera on
ladders, poles, chains, walls, hangs and monkey bars uses the ground solve
with the pivot against the climb surface. With firing on every climb (D08J6),
check orbiting into the wall behind a ladder or chain, framing while armed
(Duke and the crosshair target both visible, the gun side open), the centred
shoulder rule (D08J3 follow-up) against aiming, and the camera when the aim
leaves the gun arc. Apply D10F's result to climbs; user saves UI 12 (ladder),
11 (chain), 8 (wall), 6 (monkey bars), 9 (ledge hang), private copies only.

**2026-11-07 accepted and closed.** User: "excellent work!!! well done! lock it in close it out. testing was solid. i love it". Done at the candidate (climb floor 96, ground 450). Runtime patch re-export unchanged (no framework edits). No gameplay edits, builds or launches after acceptance.

**2026-11-07 candidate (Needs playtest).** [Note 400](documentation/400-d08j3a-climb-camera-revisit.md).
Full-circle sweeps, sidearm out, on the ladder, chain, climbing wall, ledge
and monkey-bar fixtures (private copies; user slots re-saved since the job
was written and not used). Finding: on the chain (6 of 12 views) and the
ledge hang (5 of 12) the D10F1 floor of 450 held the lens inside the
climbed wall when viewed from that side (dark geometry filling the frame on
the chain, a ghost of Duke on the ledge); the ladder and climbing wall were
clear. Change (Modernized third person, `clear_lens.inc`): attached to a
climb, ledge hang/shimmy or the monkey bars the floor is the 96 minimum, so
the lens stops in front of the surface; the ground keeps 450;
`DNTTK_CLIMB_CAMERA_FLOOR` tunes it. After: chain views stop at 219-348
with the room beyond clear (60 and 180 Hz identical), ledge at 128-422
without the ghost, ladder and wall unchanged, bars 298-331 (Duke's arm can
come into frame). Centred shoulder and the out-of-arc aim (D08J6E/F) left as
they are. `ttk-controls-test` 153 groups (new D08J3A case; D14 HUD block
bypassed for the run only), `ttk-aim-test` pass. Limit: from the surface
side Duke is not visible (no room for a camera behind him); raising the lens
or a first-person blend there are unbuilt options. Executable
`6ecfe38fb8b80476de41f6824381351da066b9dbf452093325c550008b3dd0c3`.

### D08J4 - Ceiling monkey-bar climbing drops Duke at the wrong points

**Accepted (2026-10-07, user: "it's a complete winner for me. I totally accept").**
Executable `0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa` is the new regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`0385fbf271054ac23474f18b719ecfefee113db2042ed15a32d476a914fd01fa`. User video
`research/screencaps/Video_2026-10-07_17-49-43.mp4`: jumping for a ledge Duke
grabs the ceiling, "all tank controls! ... you move the way duke is facing",
and he falls off. The ceiling hang is original mode 7 (149..151) under a
climbable object (flags `0x80`, here grate type 759). Falls: ours. D08X's
object-hang release let go when S was held, or after W had been held 40 updates
without a climb, so travel with W dropped Duke after about two seconds; raw
original Up never let go and stops at the ends. Tank feel: original (Up/Down
along the facing, Left/Right turn on the spot). Fix (Modernized, by state and
flags, every climbable ceiling; `ceiling_hang.inc`): WASD steer Duke
camera-relatively (host turns his yaw, the original Up carries him), Ctrl or
Space lets go; D08X now releases only crates (`0xc0`). Private slot-3 runs:
W/A/D/S/diagonals/mouse-turned W travel within 10 degrees of the camera
direction (A -98 and D +84 include the hand-swing wobble), no drops in 120-240
frames, ends hold, Ctrl/Space let go; Vanilla untouched (0 host updates);
suites pass. The user's red-carpet ledge was not located offscreen.
[Note 123](documentation/123-d08j4-ceiling-hang.md).

**Todo. User request, 2026-10-07:** "the ceiling climbing seems to drop off at
the wrong points in that same save slot (i just climbed the chain and re-saved
at that point). duke is supposed to be able to jump and climb across this
ceiling like monkey bars but he just randomly falls. Duke can actually make
all the jumps now with modern controls too. I wanna look into some kind of fix
for this area. it's not broken, it's still playable, fortunately."

**Test location:** the user's UI slot 3 (savestate file 02), re-saved
2026-10-07 at the top of the medieval chain (SHA-256
`a519f8bc0c2f8f23c2c61283793a548f8d2ede687cdd810e0c98f512eff262a6`; the
D08J2 copy `d85450de...` is the earlier save under the chain). Test only on
dated private copies; verify the hash first and never use the player's cards.

**Research start:** identify the ceiling-hang state (mode, animations, the
attached object or ceiling geometry in `+0x17c`/`+0x180`) and the original
test that lets go. Establish whether the drops are original (Vanilla, D-pad
only) or come from Modernized input (WASD fed as original buttons in the
attached-traversal block, camera-relative direction, held E or jump), and
whether they fall at gaps between the hang surfaces or in mid-span. Fix it
game-wide by state, not this room alone.

**Acceptance:** in Modernized, Duke can traverse the slot-3 ceiling end to end
without falling except where the player lets go or the ceiling really ends;
jumps between sections still work; Vanilla unchanged; the user confirms.

### D08J5 - Climb down chains

**Accepted (2026-10-07, user: "that definitely works, and i accept it ... it
works perfectly").** Executable
`3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be` is the new
regression baseline.
**Built 2026-10-07 (was Needs playtest).** Executable
`3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`. Finding: from
the slot-3 platform no original move reaches the chain (type 842, flags
`0x400`, top 509 below the floor, about 350 past where walking stops): the
jump overshoots into the pit and there is no top mount. Once attached, the
original descends with Down and steps off onto the walkway (191), but its
down probe (`0x8007d65c`) accepts a floor above Duke's feet, so on the
platform side near the top Down lifted him back onto the platform (Vanilla
too). Change (Modernized, `pole_climb.inc`, by flags and state): E at the top
of a pole or chain (the D08U request and hint) attaches it as the airborne
catch does (154) and swings Duke half a turn round it to the far side while
lowering him; a new codegen hook `0x8003964C` makes the down probe ignore
floors above Duke; Ctrl lets go anywhere, S at an end with nothing below lets
go; the 196/191 exits keep neutral directions. Private slot-3 runs: E mount,
S to the walkway and step-off (third and first person), W back out at the
top, Ctrl/Space let go, A/D screen-relative, platform-side S descends; Vanilla
unchanged; slot-5 ladder regression matches; suites pass.
[Note 125](documentation/125-d08j5-chain-descent.md).

**Todo. User request, 2026-10-07:** "Next up, we have to stand up a backlog job
which is the ability to climb down chains. Slot 3 is a great one for testing
this with as theres a chain right in front of us ready to attempt climbind
down."

**Test location:** the user's UI slot 3 (savestate file 02), re-saved again
2026-10-07 18:56 with a chain right in front of Duke (SHA-256
`b4a5d740570335bca3ee41951d3462b53d684e3fc028a0a1c7fcffc8e551acf8`; earlier copies `a519f8bc...`
(D08J4, top of the chain) and `d85450de...` (D08J2, under it) are other
saves). Test only on dated private copies; verify the hash first and never use
the player's cards.

**Research start:** how Duke gets onto this chain from where he stands (E
grab, a jump to it, or a top-of-chain mount like D08U's ladder-top mount),
what S does on the hang-climb (192..195, mode 3; D08J2 saw S descend a short
way at slot 3's lower chain) and where the original stops him: the chain's
lower end, a bottom hang, an automatic drop, or no descent at all. Compare
Vanilla (D-pad Down, Square) with Modernized (S, Ctrl, Space) to establish
whether the limit is original or ours (the D08U1 ladder S/let-go rules and
the D08X hang release are known Modernized interventions on similar states).
Cover poles with the same state where they share it. Fix by state, game-wide,
not this chain alone.

**Acceptance:** in Modernized, Duke can climb down the slot-3 chain from the
top to its lower end with S, and at the bottom steps off or lets go
predictably (and can let go anywhere with Ctrl or Space); W still climbs, A/D
and the D08J3 free camera unchanged; Vanilla unchanged; the user confirms.

### D08J6 - Firing while climbing (research first)

**Status: Done (D08J6A user-accepted and closed 2026-11-04: "great progress so far and i love the direction ... lock us in and close us out"; the remaining stages are jobs D08J6B, D08J6C and D08J6D; [note 377](documentation/377-d08j6-climb-fire-research.md)). Dependencies: D08J, D35E, D35F.** User:
"the ability to shoot while climbing ... Duke stopping and then shooting and
then you would be able to resume climbing". Example: slot 7 (file 6), the
castle tower with two floating enemies near the climb. Duke's hands are on the
ladder, chain or pole and are visible, so simultaneous climbing and firing is
not expected. Expected direction: pressing fire while climbing stops the
climb, draws the current weapon (fire draws it, as on land, D08E), aims and
fires; climbing input resumes the climb (weapon stowed). Report first: how
current games handle firing on ladders and other climbs (one-handed sidearm,
hang-and-aim, automatic holster on resume), which fits TTK's animations and
first/third person, what the game's climb states allow (mode 8, chains,
poles, climbing walls), and a staged plan. Vanilla unchanged.

**2026-11-03 research report.** The original game already stops, draws, fires
and resumes on ladders, chains and climbing walls, driven by a Circle tap:
draw about 0.45 s while attached, real rounds (ladder 9, chain 6, wall 6),
no climbing while armed (rung handler refuses upper animations with table
flag 8), a second tap stows and W/S climb again to the exits. Missing:
left click does nothing on climbs (fire-to-draw admits ground/flight/swim
only), climbing keys do not stow, shots are not camera-adapted (they hit
the ladder/wall in front), the armed pose is the ground upper body with
both hands off the rung, and first-person weapon presentation is wrong on
climbs. User: open to new animation and Blender, "absolute professionalism".
Recommended: hang-and-aim (off hand on the rung, torso clamp, gun arm aims)
in stages D08J6A mechanics, B third-person pose, C first person, D monkey
bars and ledge hangs. Waiting on the decisions listed in note 377 and a
user save state at the slot 7 tower. No code or build change.

**2026-11-04 D08J6A candidate (Needs playtest).** User chose every
recommendation ("Uncharted, and Tomb Raider are BRILLIANT comparisons ... we go
total cleanroom"). Held left click on a ladder, chain/pole or climbing wall
stops the climb, draws the sidearm (swapping out another selection, given back
on holstering or on landing after letting go armed) and fires at the
crosshair; W/S/A/D holster and climb on. Ladder, chain and wall routes pass at
60/120/180 settings (first shot 15-25 game frames after the press, climbing
again 21-25 after release); Vanilla unchanged; native controls, input, aim and
near suites pass. Pose and first-person weapon presentation are still the
original (D08J6B/C); monkey bars and ledge hangs D08J6D. Executable
`c5c6a30b010df99ad7981cf3452d6105ba02811d2cc2a050e0bff7b6abed9a56`.

**2026-11-04 accepted and closed.** User: "great progress so far and i love
the direction ... lock us in and close us out". D08J6A is Done; stages B-D are
the new jobs D08J6B, D08J6C and D08J6D, with the user's test saves (UI slot 12
ladder, 11 chain, 6 monkey bars, 8 climbing wall). New job D08J7, shooting
while mantling, is the next job by user priority. No gameplay edits, builds or
launches after acceptance.

### D08J6B - Hang-and-aim pose, third person

**Status: Done (user-accepted and closed 2026-11-04: "i love it all, this animation works perfectly fine"; [note 377](documentation/377-d08j6-climb-fire-research.md) D08J6B section). Dependencies: D08J6.** Note 377 stage B. Replace the original
two-handed upper body on climbs with a clean-room hang-and-aim pose: the off
hand stays on the rung, chain or wall hold (climb clip on records 0, 2-4 and
9-18), the chest turns toward the aim within a clamp, the gun arm (records
5-7) aims at the crosshair (look-at helpers as in D08O2A). Blender-authored
one-arm draw and holster, aim idle and recoil, and per-family grips through
the D08K1 pipeline; read note 265 for any new art. User test saves (private
copies only): UI slot 12 ladder, UI slot 11 chain, UI slot 8 climbing wall,
and the slot 7 castle tower with floating enemies. Validate in game from
several angles at 60/120/180, not only in Blender. Vanilla unchanged.

**2026-11-04 prototype (Needs playtest).** Modernized, third person, sidearm
out on a ladder, chain/pole or climbing wall: the chest turns toward the view
(50 degree clamp), the off-hand arm is solved back onto its climb-clip contact
(two-bone IK, grip kept), the head looks toward the view, and while firing the
original one-arm aim and head look-at take the view direction (gun arc 110
degrees right, 60 left). Before, the firing pistol pointed along Duke's
heading at the ladder or wall. New `climb_aim.inc` on the existing
`0x800411b8` and `0x800b42ec` hooks; the one-arm aim (no entry hook) reads
`+0x124`, lent for the gun shoulder's build only and restored at the next
joint; new guard `0x800978f8`; no hook, codegen or save change. Ladder, chain
and wall fixtures: off hand within about 12 units of its contact (86 before
the IK), firing arm within 13 degrees of the aim, the same at 60/120/180;
D08J6A routes unchanged; Vanilla unchanged; native controls 149 groups (new
D08J6B group), input, aim and near suites pass. Limits: no Blender clips yet
(waiting on the user's judgement of the prototype), idle keeps the original
high ready, beyond the arc the arm stops while rounds still go to the
crosshair, user saves and the slot 7 tower not used, no enemies framed. A/B
`DNTTK_CLIMB_AIM=0`. Executable
`b293c25036ef3124f1666c76c1a21a3fc531e80e11e71e2d7532a937354c9806`.

**2026-11-04 accepted and closed.** User, on the ladder: "duke can clearly
shoot in the right direction here and it looks superb. that one is a
winner"; after the chain, wall, behind-the-arc, idle and draw/holster checks:
"i love it all, this animation works perfectly fine". Done at the prototype build: the arc limit and the high-ready idle
stay as built; the Blender clips are not needed for now. First person is
D08J6C, monkey bars and hangs D08J6D. A low frame rate during the first test
was a host check: the game halved high-refresh presents after a savestate
load (audio running low); the host was idle, Cinnamon used about 20% of a core
continuously; the user's retest after restarting Cinnamon was fine (not
established as the cause). No gameplay edits, builds or launches after
acceptance.

### D08J6C - Firing while climbing in first person

**Status: Done (user-accepted and closed 2026-11-07: "honestly it looks so good!!!", "fully acepted - lock it in close it out"; [note 377](documentation/377-d08j6-climb-fire-research.md) D08J6C section). Dependencies: D08J6B, D35E, D35F, D36.** Note 377 stage C.
On ladders the normal weapon viewmodel replaces the climbing hands; on the
chain no weapon shows at all. Show the off hand on the rung and the gun arm
aiming from the other side in the ladder and pole/chain views (note 257
ownership, notes 305/306 arm coverage, the D35E/F clipping path). Same user
saves as D08J6B. Vanilla unchanged.

**2026-11-07 candidate (Needs playtest).** Before (this build's baseline,
fixture copies): armed on the ladder, chain and climbing wall the first-person
view showed only the pistol viewmodel; the off-hand arm was hidden for it.
The pistol already showed on the chain (the "no weapon on the chain" note
predates D08J6D/E). Owner: the D36H5 contact rig for the off hand, the D12
viewmodel for the gun arm, the D08J7D split. The original keeps the climb
clip on the off-hand arm while the sidearm is out (D08J6B probes), so its
native hand is already on the rung, chain, hold or bar. Now, with the
sidearm out on a settled climb (mode 3: ladder, chain/pole, climbing wall)
or under the ceiling monkey bars (mode 7), the off hand (parts 10-12) takes
the contact rig at its native world position, as on the holstered climb;
the gun arm stays the viewmodel aimed at the crosshair. The D08J7D lip pin
stays ledge-only (new `lip` field of the one-hand view state, carried to
replay redraws). Change: `first_person.inc` (`onehand_view_live`, the contact
rig pin and the viewmodel arm skip, now for every contact owner). Evidence
(`recomp/analysis/d08j6c/`, local): D35E ladder, D35F chain, D35I1 wall and
the slot 6 ceiling copy, idle and firing, turned and looking up/down: the
off hand on the rung, chain, hold and bar beside the pistol; the armed
ladder down-look and the wall/ceiling views match the holstered rig; ladder
the same at 60/120/180. Native controls 154 groups pass (new D08J6C group:
sidearm off hand on the rig at its native matrix, gun arm off the rig,
shotgun off the rig, render-state replay, native matrices untouched; the
known D14 widescreen HUD block, failing on the clean baseline, skipped for
that run only), aim and input pass; `ttk-near-test` does not link at HEAD
either (`cowboy_hat_mesh` undefined), not run. Limits: functional captures,
not feel; the chain at some angles shows the same large near forearm as the
accepted holstered chain view; third person, the ledge hang/pull-up
(D08J7D) and Vanilla unchanged by construction (first person, Modern
controls only), not re-captured; user saves not used; no enemies framed.
A/B `DNTTK_FP_CLIMB_OFFHAND=0`. Executable
`bfd83ade959a8b3f2bca73ef0bcb408f5ba119644a9c33b2ed20aae83d9c851a`.

**2026-11-07 accepted and closed.** User, after the ladder, chain, wall and
monkey-bar playtest: "honestly it looks so good!!!", then "fully acepted -
lock it in close it out". Done at the candidate build. The runtime patch
re-export was unchanged (no framework edits). No gameplay edits, builds or
launches after acceptance; no player data touched.

### D08J6D - Firing on monkey bars and ledge hangs

**Status: Done (user-accepted and closed 2026-11-04: "your work is absolutely solid here though. excellent work, accepted"; [note 377](documentation/377-d08j6-climb-fire-research.md) D08J6D section; follow-ups D08J6E, D08J6F, D08J3A). Dependencies: D08J6A, D08J4, D08J7.** Note 377 stage D.
Probe the original ceiling monkey bars (mode 7) and hangs (mode 6) live, then
extend the climb lease, draw, stow and view-aimed shots to them with the
D08J6B pose. User test save (private copy only): UI slot 6 monkey bars.
Vanilla unchanged.

**2026-11-04 candidate (Needs playtest).** Live probe of the original on a
private copy of the user's UI slot 6 (runtime slot 5; back up, then jump with
E to catch the ceiling grate: mode 7, 149): a Circle tap draws the pistol
(about 0.45 s), Cross fires real rounds (8) along Duke's heading (0
view-adapted), armed W/A/S/D do nothing, Circle stows, armed Space lets go
with the pistol out, armed Ctrl does nothing; the gun arm alone (channel 1)
carries the sidearm, as on climbs. Ledge hangs already had D08J7A's lease.
Modernized now: the settled ceiling hang (149 under a climbable object, 0x80
without 0x40; crates stay D08X) joins the climb lease (`climb_fire.inc`
`ceiling_weapon_ready`), so held left click draws the sidearm (the D08J6A
swap), fires at the crosshair, a short click only draws, W/A/S/D holster and
then travel, and the swapped weapon comes back once holstered. Fire held
during a hand-over-hand step sends no Up (`ceiling_hang.inc`): the step ends
at the next grip and the draw starts there. Armed Ctrl lets go through
original Square (`pc_input.cpp`), like armed Space; Duke lands armed. The
D08J6B hang-and-aim pose (`climb_aim.inc`) now also runs in the settled ledge
hang (mode 6) and the ceiling hang (mode 7): chest and head turn to the view,
the off hand is solved back onto the lip or bar, the firing arm takes the
clamped view. Evidence: slot 6 copy at 60/120/180 and first person, the
same outcomes (6 rounds, 6 view shots, shotgun selection back, travel after
the stow, fire mid-travel draws at the next grip, Ctrl/Space land armed,
click 0 rounds); joint geometry firing arm within 12-13 degrees of the aim
(ceiling and the D35G ledge hang, turns up to 86 degrees each way), off hand
within 33 units of its contact (ceiling) and 22 (ledge); A/B screenshots
with `DNTTK_CLIMB_AIM=0` (the original points the pistol along the heading).
D08J6A ladder/chain/wall and D08J7A hang routes identical to their accepted
results; Vanilla (raw pads, private hang state): original draw, 5 rounds
along the heading, no host shots or aim builds, armed Up refused. Native
controls 150 groups (new D08J6D group), input under Xvfb (armed Ctrl
Square), aim and near pass. Limits: first person shows the normal pistol
viewmodel with no off hand on the bar (D08J6C territory); the off-hand
drift on the ceiling (up to 33) is larger than on climbs (12) because the
hang clip moves while the solve uses the previous build's forearm; no
enemies framed; the user's slot 6 only (slot 3 D08J4 ceiling not re-run);
the chest turn snaps back when an armed ledge pull-up starts (from the code, not seen in play). Executable
`ae9d9f4f6845ce81a400a7269416bf8eae2b173cfe0eeab8916d5f43c93c010e`.

**2026-11-04 accepted and closed.** User: "it definitely works fine, but i have another reservation which i wanna flag up. your work is absolutely solid here though. excellent work, accepted". The
reservation (camera feel near surfaces, aim and mouse tightening, monkey-bar
facing) became the blast list D08J6E, D10F, D08J3A, D10G and D08J6F. No
gameplay edits, builds or launches after acceptance.

### D08J6E - Monkey bars and chains: Duke turns to the aim while armed

**Status: Done (user-accepted and closed 2026-11-04: "im happy with the results. lets lock it in and close it out"; chains and poles in scope per the user: "it's the same for chain climbing too, really, identical"; [note 377](documentation/377-d08j6-climb-fire-research.md) D08J6E section). Dependencies: D08J6D, D08J4.** User: "duke could easily change
his direction based on where the mouse is pointing". With the sidearm out
under a climbable ceiling, turn Duke on the bar toward the camera's yaw (the
D08J4 turn, rate-limited, original travel still refused while armed) so the
crosshair stays inside the gun arc; the off hand stays on the bar (D08J6B IK).
Decide with the user: always while armed, or only when the aim leaves the
arc, and the turn rate. Firing must not spin him behind the shot; first
person and Vanilla unchanged. Test on the private slot 6 copy (note 377
D08J6D route) at 60/120/180.

**2026-11-04 candidate (Needs playtest).** Probe (D35F chain copy): armed or
not, the original places Duke on a chain 225 units from its axis along his
yaw every update, so a yaw change takes him round it. Modernized, third
person, view aiming, sidearm out: on the monkey bars (settled 149, not
crates) and on poles and chains (192-195) Duke turns on the hold toward the
camera at every player update, 192 yaw units (about 17 degrees) at most and
half the remaining error near the end (`climb_aim.inc` `climb_turn_update`,
called after `ceiling_hang_update`). On the bars he turns on the spot; on a
chain he goes round it, with the view kept 35 degrees to his gun side (facing
it straight, the crosshair and rounds hit the chain, box 69 units each side
of the axis). A chain step is refused when the world is in the way from his
chest to his new place plus a body width (`view_segment_query`): without it
Duke went into the platform beside the D35F chain top and 4 of 7 rounds in a
fast turn were rejected. Default "always while armed" (follow);
`DNTTK_CLIMB_TURN=arc` turns only past the 50-degree chest clamp, `0` is the
A/B; `DNTTK_CLIMB_TURN_RATE` and `DNTTK_CLIMB_TURN_BIAS` tune it. Evidence:
slot 6 copy at 60/120/180, identical: Duke follows within 3 yaw units, no
travel, 10 of 10 view shots while turning; D35F chain at 60/120/180: full
circle lower down (0 blocks), still/slow/fast turns firing 7, 7, 7 rounds,
all view-aimed, 0 rejected. First person, holstered, the original aim key
and Vanilla (`--vanilla` raw pads: original draw, 5 rounds along the
heading, no host turn) unchanged. Regressions: D08J6D ceiling route at
120/180/first person as accepted apart from the new turn (one 180 Hz
fire-mid-travel run stopped one grip earlier); D08J6A ladder, chain and wall
routes identical; D08J7A hang identical in two reruns (a first run lost its
S drop and then the debug port reported "emu busy or frozen"; not
reproduced). Native: controls 151 groups (new D08J6E group), input under
Xvfb, aim and near pass. No new hook, guard, codegen or save change.
Limits: user decisions open (follow or arc, the rate, the 35-degree chain
bias); against a wall Duke stops rather than going the long way round; the
camera framing near the chain top (pushed into the wall in one run) is
D10F/D08J3A; ladders, walls and ledges stay D08J6F; no enemies framed; the slot 6 copy and the
D35F chain fixture only (user UI slot 11 chain not used).
Executable `c0402b1ac27410cf60976b39dd4c00bd3fc0eb6dff1dc6b0a93d917cca521b14`.

**2026-11-04 accepted and closed.** User: "incredible work as always, you can read my recent logs, but im happy with the results. lets lock it in and close it out!!!" Done at the candidate defaults (follow, 192 per update, 35-degree chain bias). The user's 17:10 session log (chain and climb, slot 5 state) shows a steady 180 presents/s and no errors. Runtime patch re-export unchanged (no framework edits). No gameplay edits, builds or launches after acceptance.

### D08J6F - Shots beyond the gun arc on climbs and hangs

**Status: Done (user-accepted and closed 2026-11-07: "this is phenomenal work. absolutely tight as it gets. very professional"; user decision: swing out on the grip, then after the option 1 playtest "he could literally reach absolutely everywhere": nothing refused; third person only, first person and Vanilla unchanged; [note 377](documentation/377-d08j6-climb-fire-research.md) D08J6F section). Dependencies: D08J6B, D08J6D, D08J6E.** User: "you can
sometimes end up shooting in impossible directions when duke is not facing
the right way". Today on ladders, climbing walls, chains/poles and ledge hangs
the arm stops at its arc while the rounds still go to the crosshair (kept at
D08J6B's acceptance). Options for the user: ladders, walls and ledges refuse
shots beyond the arc (crosshair feedback), or Duke leans out further. Monkey
bars, chains and poles are D08J6E (Duke turns or goes round to the aim).

### D08J7 - Shooting while mantling

**Status: Done (D08J7A user-accepted and closed 2026-11-04: "excellent work. lock it in and close it out!"; the mantle stage is job D08J7B; [note 378](documentation/378-d08j7-mantle-fire-research.md)). Dependencies: D08J6, D08X, D08Z2, D35G.** User, closing
D08J6A: "is it gonna be possible to add in the ability to shoot when
mantling, like that's just huge because currently it prevents you from
shooting whatsoever and you might take quite a lot of damage during that
essentially offenseless time - so if we could also add it in for mantling".
Duke stows before grabbing a ledge (D08J/D08X E stow) and cannot fire through
the hang, pull-up and mantle. Research first, as D08J6 did (note 377): what
the original mantle/hang states allow (draw request, Circle, Cross, the
one-arm aim of record 5 the skeleton build already has in modes 6/7, upper
track during the mantle clips), how current games handle it (Uncharted and
Tomb Raider: sidearm while hanging from a ledge, no shooting during the
pull-up itself, or a quick cancel), and which phases can shoot (hang,
shimmy, pull-up start/end). Then build on D08J6A's lease, sidearm swap,
view-aimed shots and automatic stow so mantling is never a defenceless
window, keeping the D08Z2 mantle upper-body handoff, D35G first person and
Vanilla unchanged. Test with private copies of the user's saves and spawned
pickups; ask the user for a save state where enemies attack during a mantle.

**2026-11-04 research report.** Private offscreen probes on the D35G alley
fixture (saved platform hang, dumpster and platform W+E climb). In the ledge
hang (149, mode 6) the original already draws on a Circle tap (about 0.4 s),
fires real rounds (8) with one hand kept on the lip, and stows on a second
tap; armed, it refuses shimmy, pull-up and drop. Left click does nothing
there and hang shots go into the wall (not camera-adapted). During the climb
(139 0.47 s, pull-up 140 1.74 s, about 2.2 s per climb, 4.5 s for the
two-climb route) a Circle tap is lost and the original weapon request waits
until the first ground frame. Recommended: A, hang and fight (fire draws the
sidearm in a hang, view-aimed shots, W/A/D/S/Space stow and act); B, fire
during the climb up the face stops Duke in the hang, fire during the pull-up
queues the draw for landing with the weapon he had before the E stow, and a
shorter Modernized pull-up (about 1.0-1.2 s). No shooting while both hands
push over the lip. Staged D08J7A/B; pose with D08J6B/C. Waiting on the six
decisions in note 378 and a user save state with enemies near a mantle. No
code or build change.

**2026-11-04 decisions.** User: 1 yes (sidearm swap in hangs), 3 recommended
(pull-up about 1.0-1.2 s), 5 recommended (D08J6D keeps monkey bars and object
hangs); 2 (fire stops the climb in the hang) and 4 (leaving the hang armed)
under discussion. User slot 9 (private copy): vertical E jump catches the
ledge (147, 149); enemies are to Duke's right along the wall (user) and a
pig cop stands on the lip above, so hang shots need a wide side arc and must
clear the lip upward. No code or build change.

**2026-11-04 D08J7A candidate (Needs playtest).** User chose 2b (fire never
interrupts the climb; weapon up on landing) and 4b (drop with the pistol
out). Held left click in a settled ledge hang draws the sidearm (D08J6A swap)
and fires at the crosshair; W and A/D holster and act; S and Space let go
armed through a one-update hang-pose handoff of the upper track, and the
swapped weapon is drawn back on the ground. Hang route at 60/120/180 settings,
first person, the user's slot 9 copy (9 view shots looking right, armed
drop) and the D08J6A climb regression pass; Vanilla unchanged; native
controls, input, aim and near suites pass. Pose still original (D08J6B/C).
D08J7B (queued landing draw, pull-up about 1.0-1.2 s) is next. Executable
`0ccd15e8ba4fd9ff24f43362cd87e0be416b6c19b25837517d177b8074bb025c`.

**2026-11-04 accepted and closed.** User: "excellent work. lock it in and
close it out!". D08J7A is Done; the mantle stage (user decisions 2b and 3) is
the new job D08J7B. No gameplay edits, builds or launches after acceptance.

### D08J7B - Weapon up on landing and a shorter pull-up

**Status: Done (closed 2026-11-04 with D08J7C, which is built on it and was accepted in play; [note 378](documentation/378-d08j7-mantle-fire-research.md)). Dependencies: D08J7.** Note 378 part B with the user's
decisions: fire never interrupts a climb (2b); fire held or clicked during
the climb or pull-up queues the original weapon request (the original
already defers it to the first ground frame) and brings back the weapon
Duke had before the E stow, drawn at the fast rate; the Modernized pull-up
(140, 1.74 s) plays in about 1.0-1.2 s (3) with the root motion unchanged so
Duke ends where he does today. Keep the D08Z2 upper-body handoff, D35G first
person, the D08X catches and Vanilla unchanged. Test on the D35G alley climb
(dumpster and platform), the slot-12 sewer wall catch and the user's slot 9
copy at 60/120/180.

**2026-11-04 D08J7B candidate (Needs playtest).** Modernized: fire held or
clicked during the climb up the face (139), the pull-up (140), a height
mantle (134-138) or the hang a W-held climb passes through queues the
weapon selected before the E stow; it is requested in the pull-up (the
original serves it on the first ground frame) or on the ground after a
height mantle, drawn 2x with its opening blend. The climb never stops: W
held in a ledge hang now climbs on through held fire (no sidearm draw), and
unarmed Cross stays off while the draw waits. The pull-up 140 plays 1.6x on
both tracks (104 -> 63-67 game frames, about 1.05 s at 60/120/180). End
position: dumpster identical to a no-speed-up A/B build; platform within 14
units. Alley route (shotgun up 0.17-0.20 s after landing at 60/120/180,
first person too), slot-12 sewer ledge (pistol out 2 frames after landing,
same end), user slot 9 copy (W + fire from the jump catch: pull-up, pistol
out on landing, no hang draw) at 60/120/180; D08J7A hang routes and D08J6A
ladder/chain/wall unchanged; native controls (new D08J7B group), input, aim
and near suites pass. Vanilla: every change behind Modernized (native
gates); the test input cannot drive Vanilla live. Executable
`ca7d1a22bf575a6ee4d7a3c010338e6f6d641e698e416a6b1bfa540fb90aa9ab`.

### D08J7C - One-handed armed pull-up and the sidearm return

**Status: Done (user-accepted and closed 2026-11-04: "the work here is absolutely quality. legendary work. i'm really happy with it. lock it in close it out"; revision 3: one-handed pull-up clip, shooting the whole way up, SIDEARM RETURN NEVER by default; [note 378](documentation/378-d08j7-mantle-fire-research.md)). Dependencies: D08J7B.** User, reviewing D08J7B: "i think
Duke could mantle up with 1 hand with the sidearm out you know". Today W in
an armed ledge hang holsters first, and with fire held W does nothing.
Decisions (2026-11-04):

- W in an armed hang (sidearm only) always does a one-handed pull-up with
  the sidearm out, firing or not. No holster step. Holstered pull-ups are
  unchanged.
- Same speed as the D08J7B pull-up (about 1.05 s); no armed slowdown.
- Shots allowed in the late phase of the pull-up (knee on the lip to
  standing), refused while the chest is below the lip (the aim cover check
  refuses them there too).
- Sidearm return, one rule for the armed pull-up, the D08J7A armed drop and
  the D08J6A armed let-go (recommended option C with D's cancel): Duke keeps
  the sidearm while you keep shooting; after a pause off the trigger
  (default about 0.75 s, timed in real seconds, the same at 60/120/180) the
  swapped weapon comes back by itself. Tapping the pistol never starts the
  swap between taps. Any manual weapon choice cancels the pending return.
  Fire pressed during the swap: check whether the original switch can be
  interrupted; otherwise the new weapon fires as soon as it is up.

Mechanics: the original refuses the pull-up while armed; reuse the D08J7A
armed-drop handoff (the hang pose on the upper track for one update, then the
kept weapon block) to start 140 armed and keep the sidearm through it.
Presentation: first a prototype with 140 on the body and off hand and the
weapon pose on the gun arm (records 5-7; confirm per-record mixing exists
before relying on it), judged in game; then a Blender one-handed pull-up clip
through the D08K1/D08K4 extra-clip pipeline, shared with D08J6B (read notes
265 and 257 first). First person: the sidearm viewmodel stays up in the
mantle view. Test on the D35G alley hang, the slot-12 sewer catch and the
user's slot 9 copy at 60/120/180; D08Z2, D35G and Vanilla unchanged.

**2026-11-04 D08J7C candidate (Needs playtest).** Modernized: W in a ledge
hang with the sidearm out pulls up one-handed, firing or not (no holster).
The hang pose takes the gun-arm track for one update so the original starts
the pull-up 140 (its check at `0x8004728c` refuses a weapon pose); then the
sidearm block comes back. The game mixes four channels (legs, right arm,
left arm, chest/head), so the body and off hand pull up while the gun arm
holds the pistol: the prototype needs no new clip. Same 1.6x speed (62-66
frames at 60/120/180). Cross is masked until the knee is on the lip (frame 6
of 10, measured: chest over the lip between 4 and 5, knee 5 and 6); later
rounds are view-aimed. Sidearm return, one rule for the armed pull-up, the
D08J7A drop and the D08J6A let-go: 0.75 s off the trigger (real time), taps
never start it, a weapon choice cancels it; fire during the swap waits for
the new weapon (the original switch is not interrupted). Alley hang at
60/120/180 in both views (no early rounds, late round view-aimed, return
0.78-0.82 s after release), first-person viewmodel stays up, user slot 9 copy
(66 frames, 6 view-aimed rounds), Vanilla raw-pad check (Up still refused);
the sewer climb is a direct mantle (no hang) and its D08J7B route is
unchanged. D08J6A/D08J7A/D08J7B regressions pass (D08J7A's W now pulls up
armed, as intended); native controls (146 groups, new D08J7C group), input,
aim and near suites pass. Pose is the original sidearm pose over the pull-up;
the Blender one-handed clip waits for the user's in-game judgement.
Executable `908d46a985d9ceb4ec56ae61ef3c655f7d7f1627d09cdf7e2a0072d39e62b3d6`.

**2026-11-04 revision 1 (Needs playtest).** User: "i'm in 2 minds about
whether we switch the gun back automatically", then chose a setting. OPTIONS >
ADVANCED > ITEMS > SIDEARM RETURN: AUTO (default, the 0.75 s rule) or NEVER
(keep the sidearm until you choose). Live and saved, profile schema 64,
`run.py --sidearm-return`, terminal choice A; Vanilla has no swap. Python
profile/input suites (138), native suites and a live never/auto check on the
alley hang pass. The first-person hang arm flicker and legs-only view the user
also reported predate this job (same on the D08J7B build): new job D36H9.
Executable `87d8b01d982d8a872c17b0770f749214cfe1a4e5b9e2bdae9f8fa521e042dbaa`.

**2026-11-04 build check (Needs playtest).** The current player build carries
D08J7C revision 1 with the accepted D36H9 on top (no-op rebuild, SIDEARM
RETURN present). No gameplay changes. Waiting on the user's in-game verdict:
the one-handed pose (original sidearm pose over the pull-up, or a Blender
clip next) and the AUTO/NEVER sidearm return. Executable
`89fe6df1b40ee15028a96d25041714ba39a988eba9c7c01de6d1d3d2e47b2ebe`.

**2026-11-04 revision 2 (Needs playtest).** User: "i wanna go for a proper
one handed look to this. And i think never should be the default :( this was
a really hard one to decided on and figure out. we should leave the option
there". SIDEARM RETURN defaults to NEVER (profile schema 65: a schema 64
`auto`, the old default, becomes `never` once; a schema 65 `auto` is kept;
menu, launcher, terminal and native default follow; AUTO unchanged). The
armed pull-up gets a one-handed upper body: host extra clip 41
(`recomp/tools/local/ttk_onehand_pullup.py`, in `ttk-anim.pack`), frame for
frame with 140 on Duke's skeleton - chest leaned onto the left arm (22 deg
yaw, 12 roll, 6 pitch, unwinding from frame 5 to standing), left hand
two-bone IK onto 140's own lip contact moved 35% under his centre, head on
the ledge, the pistol held muzzle up beside the head (high ready) until it
comes down into the sidearm pose by frame 6. `onehand_pullup.inc` writes it on
the joint records at `0x800411b8` after the channel blend, sampled at
channel 0's (previous, current, t), ramped over 140 ms real time; the gun arm
hands back to its channel over frames 5-6 so the late shots aim from the game
pose; the chest/head stand aside while the original aim owns the chest. Legs
and root stay on 140 (same end). Alley hang (both sides, front) at 60 and
fire at 60/120/180: no rounds before the late phase, 1 view-aimed round late,
lands with the sidearm; first person at 180: viewmodel up, the raised arm
never in view; user slot 9 copy: 66 frames, lands armed;
`DNTTK_ONEHAND_PULLUP=0` A/B gives the prototype (0 writes); Vanilla raw
pads: Up still refused. `ttk-controls-test` 147, input, aim, near, Python
profile/input 139 and `test_ttk_anim.py` 17 (new one-hand clip test) pass.
Procedural IK clip, not hand-keyed in Blender; judge in game. Executable
`c89d98930383a099dd60fc39fa2652d975e265d5dc5448b1ea68675e88d60b18`.

**2026-11-04 revision 3 (Needs playtest).** User: "with the one handed pull
up, duke actually never needs to stop shooting"; then "proceed" on the
proposal. Shots are allowed for the whole armed pull-up: only the one-update
handoff still masks Cross; `armed_pullup_shot_ready()` no longer waits for
frame 6. The aim's body-to-muzzle check keeps a muzzle behind the lip on
Duke's side, so a round aimed into the ledge face hits it. While firing
(`+0x224 & 0x100`) the gun arm leaves the high ready for its channel's aimed
sidearm pose over 70 ms and returns after; the original aim look-at
(`0x80097a44`) now takes the view direction during the armed pull-up
(`armed_pullup_view_aim()`, view aiming only), so the chest and pistol turn to
the crosshair (before, they pointed along Duke's heading while the round went
to the crosshair). The D08J7A hang keeps its accepted behaviour. Fire held at
60/120/180: 4 view-aimed rounds through the pull-up (revision 2: 1), pull-up
56-61 frames, the same end position at every rate, lands armed; side, chase
and first-person screenshots; user slot 9 copy with fire: 67 frames, 9
rounds, lands armed; D08J7A `cand.py` (hang fire, A shimmy, W, S/Space
drops, click) passed in 5 reruns with identical results, while 1 earlier
run fell behind real time (about 149 frames in about 5 s of script time; the
draw click did not register) and then lost the debug port at a screenshot,
on a host with load 2.3 and an unrelated eduke32 running: not reproduced, not
established as a game fault. Vanilla raw pads: Up still refused. `ttk-controls-test` 147, input, aim,
near, Python profile/input 139, `test_ttk_anim.py` 17 pass. Executable
`a040b2c15b2bf7426c7c9752c3e5c2e9f1565214727a401c44dbe87edfbbeaaf`.

**2026-11-04 accepted and closed.** User: "the work here is absolutely quality. legendary work. i'm really happy with it. lock it in close it out". D08J7C is
Done at revision 3; D08J7B (landing draw, 1.6x pull-up), which D08J7C is built
on and which shipped in the same accepted build, is closed with it. Follow-up
first-person work is job D08J7D. No gameplay edits, builds or launches after
acceptance.

### D08J7D - One-handed armed pull-up in first person

**Status: Done (user-accepted and closed 2026-11-04: "i'm really happy with this progress! lock it in and close it out"; off hand on the lip through the contact rig, pistol high ready; [note 378](documentation/378-d08j7-mantle-fire-research.md) D08J7D section). Dependencies: D08J7C, D35G, D36H9.** User, on D08J7C
revision 2: "do we have a backlog job in to get this animation into first
person too? it's very close already isnt it"; added with D08J7C revision 3
(2026-11-04). In third person the armed pull-up is one-handed (host clip 41,
note 378); in first person the D35G mantle view shows only the sidearm
viewmodel, unchanged through the climb: no left hand on the lip and no sense
of the pull. Show the one-handed pull-up from the eye: the off hand gripping
and pushing on the lip (the clip's IK contact), the sidearm viewmodel
following the climb (held up beside the head while the chest is below the
lip, down to aim while firing, the D08J7C revision 3 rule), and the raised
body gun arm kept out of the camera. Read notes 257, 305 and 306 first and
identify the actual presentation owner (D35G Mantle view, D36H4 body path,
D36H5 contact rig, viewmodel) and the current slot contents before changing
anything; keep the D36H9 hang fixes (no arm flicker, legs sliver). Test on
the D35G alley hang and the user's slot 9 copy at 60/120/180 with and
without fire, looking up, down and sideways; the holstered pull-up, third
person and Vanilla unchanged.

**2026-11-04 candidate (Needs playtest).** Owner: the D36H5 contact rig
(Mantle) for the off hand, the D12 viewmodel for the gun arm. Before, an
armed Mantle hid the off-hand chain (parts 10-12) at the eye for the
viewmodel. Now, in the armed sidearm hang and the one-handed pull-up, the off
hand takes the contact rig while it is on the lip: native hand within 250
units of the ledge top (`+0x1c8`) and at least 128 units ahead of the eye. Its
drawn height is pinned to within 40 units of the lip, so it stays planted
while the aimed chest lifts the clip's hand (firing). From pull-up frame 4,
140 lifts the hand 400-860 units off the lip and it is hidden (it is below
the view by then). The pistol viewmodel follows the clip's gun-arm weight
(`onehand_view_raise`): high ready (50 degrees muzzle up, rolled out, raised
to the right) while the chest is below the lip, aimed while firing (the
D08J7C 70 ms rule), and down to the normal pose at the late phase. Both reach
replay redraws through RenderState. Evidence: D35G alley at 60/120/180 with
and without fire (61-frame pull-up at every rate, lands armed). The 60 Hz
Xvfb recording shows the hand on the lip sliding down the view as Duke rises,
with no late glove. Looking up in the armed hang shows the off hand on the
lip (A/B: nothing). Looking left or down shows near forearm slabs for a few
frames, the same as the accepted holstered pull-up. User slot 9 copy: 65-69
frames at 60/120/180, lands armed, 9 rounds with fire. Without fire it shows
the hand and the raised pistol. With fire the hand is hidden: the aim holds
it level with the eye. The eye-depth rule removed the near slab there; the
A/B run shows the pig cop's weapon slab with and without the change. Third
person (61 frames) and Vanilla (Up refused) are unchanged. Native controls
148 groups (new D08J7D group: rig owner, lip pin, off-lip, at-eye and shotgun
cases through a render-state replay), input, aim and near suites pass.
Limits: lip reach and eye depth are measured constants (alley, slot 9). The
off hand is hidden while firing on slot 9. The high ready is a viewmodel
offset tuned on screenshots (developer `DNTTK_FP_ONEHAND_RAISE=pitch,roll,x,y,z`).
The hand pops out below the screen edge around frame 4. A 180 Hz recording
fell behind real time with an unrelated eduke32 at 100% CPU (load 4.7). The
unrecorded 180 runs were normal (61 frames). A/B: `DNTTK_FP_ONEHAND=0`.
Executable `396f207794e73cf19a59dfd2c4fe7da89bb7e276f4c3871f4d4d893bd0bec862`.

**2026-11-04 accepted and closed.** User: "i'm really happy with this progress! lock it in and close it out" D08J7D is Done at the
candidate build. The user also closed the eduke32 instance that loaded the
host during the 180 Hz recording. No gameplay edits, builds or launches after
acceptance.

### D08J8 - Shoot through traversal: no fire gaps after climbs, mantles and doors (batch)

**Status: Done (user-accepted and closed 2026-11-05 at revision 3: "this work is complete fully accepted. lock it in and close it out"; batch D08J8A-E; [note 394](documentation/394-d08j8-shoot-through-traversal.md)). Dependencies: D08J6, D08J7C, D10H6.** User
(2026-11-05, live commentary over UI save slots 6-12): "you should be able to
start shooting straight away just like we did with the mantle", the armed
one-handed pull-up from a ledge hang (D08J7C) being the model: "this works
perfectly with the mantling where you get up the ledge and you're
immediately able to start shooting". Sidearm rule kept (one hand on the
climb, D08J6A). Modernized only; Vanilla unchanged.

- **D08J8A - Water exit (UI slot 12).** Climbing out of the water over the
  ledge: fire must work at once, with no long delay.
- **D08J8B - Ladder top, chain and climbing-wall exits (UI slot 7, the
  ladder top).** Climbing over the top with W: shooting starts at once, with
  no gap before the gun comes back out.
- **D08J8C - Doors (UI slot 11).** Opening a door must not stop Duke
  shooting: "there's literally no reason ... why we stop shooting when
  opening the door".
- **D08J8D - Any mantle (UI slot 10, run and jump onto the crates).** Duke
  can shoot (the sidearm) from the moment he starts climbing up, whatever
  the weapon: "in any mantling situation you should be able to instantly
  shoot".
- **D08J8E - Fire rate on climbs and mantles.** The sidearm's held rate on
  a ladder (UI slot 7, pig cops behind), climbing wall (slot 8), monkey bars
  (slot 6), chain and mantles (slot 9) is lower than on the ground; bring it
  to the ground rate (D10H6: 6 per second).

Test on private copies of the slots; player saves and cards untouched.

**2026-11-05 candidate (Needs playtest).** Measured on private copies of UI
slots 6-12: the water exit (142), ladder top (190), crate mantle (137) and
the other climbs over a lip ran 1.5-1.7 s holstered (fire dropped, or
queued to the landing by D08J7B); the door kick 116 refused fire for 1.24 s;
held sidearm fire on climbs, hangs, monkey bars and the armed pull-up ran
200-237 ms (track 11, outside the D10H6 pacing). Cause: the fire request
`0x80058120` refuses lower clips with table bit 0x2000 (mantles 134-139, door
116) and the committed use bit 0x40; the shot itself is the fire track's key
event. Changes (Modernized only):
- A/B/D: held fire inside a climb over a lip (134-140, 142, 190, 196, 206)
  draws the sidearm at once (D08J6A swap, SIDEARM RETURN, 4x draw, D10H4
  first-shot boost; E held does not block it); the climb runs on and Duke
  lands armed. The original fires by itself in 140/142/190/196/206; in the
  0x2000 mantles the host runs the pistol fire track on the game clock
  (`trav_fire_drive`). An armed climb up the face that stops in the hang
  pulls up one-handed on W or E (D08J7C). The D08J7B queue remains when the
  sidearm has no ammo.
- C: an armed door kick plays 6x and ends through the use action's own
  completion once the strike key (which opens the door) has ended
  (`door_fire.inc`).
- E: track 11 and the climb states join the D10H6 pacing.
After (180 Hz): water exit first round 88 ms after the press, ladder top 106,
crate mantle 120 ms into the climb, chain/wall tops 94, door 307 ms (was
1240), all at 164-170 ms held; ladder, wall, chain, monkey bars, hang and
armed pull-up 163-171 ms (were 197-237). 60/120 Hz identical. First person
shows the pistol up and firing. Native `ttk-controls-test` 153 PASS (new
D08J8 group), `ttk-input-test`, `ttk-aim-test`, `test_player_profiles` pass.
Limits: no enemies framed; gun arm uses the sidearm's own poses over the
climb clips (one-handed clip only in the pull-up); one late shot at some
transitions. Executable
`1015e6de8daf67030b1895c9bcda5275fff939814fce884c9b59a74675aa70db`.

**2026-11-05 revision 2 (Needs playtest).** User: mantle fire and the climb
fire rate confirmed; third person did not point the gun where you aim (slot
10 crates, slot 8 wall top); doors should open on E, not left click, and
Duke should shoot straight away after E. Mantles 134-139: host-owned fire on
the climbs' one-arm track 11 with the D08J6B climb aim; water exit and
ladder/chain/wall tops: the original's aim takes the view (as the D08J7C
pull-up). Doors: E with a weapon out at a door (contact object +0x174 is a
pose-116 door) probes with armed Cross and kicks armed, no holster (first
round 73 ms after fire, was about 1.3 s); a probe's stray fire request is
put back; left click no longer opens doors (the use ends at its first
stage, first round 107 ms). Native controls 153 PASS, input and aim pass.
Executable `af4a4d6eeebd4f683f88c80761d44dc0e0688cae403d3fa9a9b9dd43dad2b26d`;
[note 394](documentation/394-d08j8-shoot-through-traversal.md) revision 2.

**2026-11-05 revision 3 (Needs playtest).** User: slot 10 third person
"works amazing" but the head jerks while shooting; slot 12 water exit
functionally accepted (refinement: D08J8F); slot 8 wall top always faces
forward; doors should open the normal way with Duke still able to shoot.
Head jerk: the driven track 11 dropped to the lowering 9 at every loop end
and restarted with a blend; it is now wrapped to its first key
(`trav_fire_wrap`), steady on 11. Wall top: the exits 197/206 were missing
from the mouse-camera lease (the camera could not turn); added, Duke now
faces the aim. Doors: the kick plays at full length again and the host owns
the fire through it (weapon's own fire track, 167 ms). Native controls 153
PASS, input and aim pass. Executable
`ab126a7f258b67539359d708d6d25cb9ce6c585fc6113ec7d1db23c419c715da`;
[note 394](documentation/394-d08j8-shoot-through-traversal.md) revision 3.

**2026-11-05 accepted and closed.** User: slot 10 "the head jerk is fixed
... the animations great ... accepted"; slot 8 "fully accepted ... duke
points in the right direction to where you're shooting"; slot 11 "fully
accepted ... you can shoot and open the door at the same time perfect";
slot 12 "points in the right direction when you get out of the water and he
naturally gets out of the water too ... fully accepted", then
"this work is complete fully accepted. lock it in and close it out". D08J8 is Done at revision 3; D08J8F is cancelled (no longer
needed). No framework edit; the runtime patch export is unchanged.

### D08J8F - Water exit: one-handed push while shooting

**Status: Cancelled (2026-11-05). User, on the revision 3 build: Duke "naturally gets out of the water too it looks much better fully accepted".** Dependencies: D08J8, D08J7C. User (2026-11-05, slot 12,
third person): functionally accepted, but "the third person animation is
like a little tiny bit rigid ... even when you're shooting he should be like
pushing himself out of the water onto the ledge". While Duke fires during
the water exit 142, the original's aim holds his upper body still. Build a
one-handed water-exit pose the D08J7C way (`ttk_onehand_pullup.py`, host
clip from 142's own frames: chest leaning onto the off arm, off hand on the
lip pushing up, gun arm free to aim), applied while armed in 142; judge in
game in third and first person; holstered exit and Vanilla unchanged.

### D08J9 - Doors and shooting, rewritten (UI slot 10)

**Status: Done (user-accepted and closed 2026-11-06: "WORKING!!!!! well done"; [note 405](documentation/405-d08j9-doors-and-shooting.md)). Dependencies: D08J8.** User (2026-11-06,
UI slot 10, a door): "open the door but try shooting at the same time ...
rapid fire happening from the shotgun. glitched out shots, no shots, or we
cant fire and open the door at the same time. kinda unpredictable
behaviour"; "we're supposed to be able to shoot completely normally when
opening doors"; test with different weapons and with the mouse held down; "i
want this to be really polished". Then: "instead of being hacky around this,
im totally opening to rewriting open-door code". Replace the D08J8C door
probe and host-driven fire tracks with a purpose-built door use: E opens the
door with any weapon out, the kick plays as in the original, and the
weapon's own fire, pump, reload and rate run through it. Left click never
opens doors. Holstered doors and Vanilla unchanged.

**2026-11-06 candidate (Needs playtest).** `door_use.inc` replaces
`door_fire.inc`. Causes found on a private copy of slot 10: E did nothing
there (the D08J8C probe waited for `+0x174`, which only the finder writes);
the host's door fire drive skipped the shotgun's gun-up hold 23 (no round for
the whole kick); and the kick's fire was a host loop, not the weapon's own
cycle. Now E (weapon out, on the ground) calls the finder `0x80077f28` and,
at a door, the original Action press `0x80051890` with a fresh Cross for that
call only. For the animation runner of each update of an armed door use,
`0x40` and 116's table bit `0x20000` are lent and put back at the fire
request `0x80058120`, under a host call (no savestate). The request's own
decision is made without the kick. Left click at a door sees Cross as held
since the last tick. The animation table's state guard is split at entry 116.
Results (10 weapons, 8 input patterns, 60/120/180 Hz, first person): every
armed kick opens the door, held gaps equal the ground's, and the first round
lands 55-70 ms after fire. Left click: door shut, weapon fires. Away from a
door, E still stows. Kept as in the original: the RPG point-blank kills Duke,
the Freezer point-blank freezes him, and the crossbow, Buffalo and
flamethrower ignore 70 ms taps. Each kick costs about 5.6 health, the same on
the D08J8 build (cause not established). Limit: one Gatling 70 ms tap on the
kick's last update was lost. Executable
`25253bf04fd558d130ba1b34867002a45c715926b48ac816676337ec5daa8e6f`.

### D08K - True crouch walking and animation feasibility

**Done (user-accepted 2026-10-22).** D08K5 final coverage fully accepted:
"total acceptance! lets lock it in and close this one out!!!" This completes
the parent: animation pipeline, low hull and safe stand-up, crouch-walk
animation, Modernized controls, refinements and final regression coverage.
[Note 198](documentation/198-d08k5-crouch-coverage.md) records the final
checks and coverage limits. Original crouch/roll remains available through
CROUCH ORIGINAL; Vanilla remains unchanged.

**Historical blocker, resolved by D08K1-D08K6:** no validated low-walk gait/override
pipeline and movement collision still uses standing clearance. See
[concrete findings and required next work](documentation/43-crouch-walk-audit.md).
No substitute slow-roll/sliding prototype was shipped. User wants held
crouch plus direction to walk low, not automatically roll. Treat this as a new
locomotion feature, not proof the present fast crouch transition is wrong. Inspect
original TTK lower-body animation clips, skeleton/event tables, collision height,
root motion and roll handlers; IDs labelled movement in source are not evidence
of an existing usable crouch-walk animation.

**Acceptance:** document available original animation assets and whether a viable
loop can be reused/blended or a new non-destructive local animation override is
needed. Deliver a bounded Modernized prototype if feasible: hold crouch to remain
low and move slowly in all directions with coherent body/feet, collision/headroom,
aiming/firing and safe stand-up. No automatic roll for ordinary crouch+direction;
retain original roll in Vanilla and document any separate Modernized roll binding
without replacing custom bindings. Test low ceilings, edges/stairs, release,
weapon/traversal interruption and camera. Do not promise a new skeletal animation
before verifying the asset pipeline; record concrete blockers if no safe prototype.

**Split into sub-jobs (2026-10-22, user request).** The audit's four
concrete dependencies are now separate jobs: D08K1 (animation pipeline),
D08K2 (crouch-height collision), D08K3 (crouch-walk animation), D08K4
(Modernized controls prototype) and D08K5 (coverage). All are now accepted;
D08K5 acceptance completes the parent.

### D08K1 - Animation pipeline: skeleton, pose preview and local override

**Done (user-accepted 2026-10-22).** User: "ok!!! now it's functionally working. the job is fulfilled on a technical level ... lock in and close out the done tasks which is the tech implementation." Animation quality continues in D08K6.

**Needs review (2026-10-22).** Implemented and verified; see
[note 184](documentation/184-d08k1-animation-pipeline.md). Formats recovered
from the readers: HIE (record byte 1 = ROT joint id written by the loader from
the header table), MOD (vertices at [+0x10], GPU-style primitive groups), ANM /
FRM / ROT (Q12 quaternions, cumulative root height, per-frame z travel), and
the animation -> four-clip table in the executable, whose clips are body
channels (legs/root, right arm, left arm, chest+head; player channels at
+0x60/+0x74/+0x88/+0x9c), not alternatives as the D08K audit read them.
`ttk_anim.py` decodes, renders, round-trips (9,481 clips byte for byte),
bridges to Blender 4.0 (`ttk_anim_blender.py`) and builds override packs;
`anim_override.cpp` loads them at run time (Modernized only, verified by clip
digest, buffer in the GPU-DMA aperture, savestate adopt/restore). Evidence:
live FK within 4.4 units; blended frames within 8/4096 for idle, walk, crouch
transition, crouch and roll; Blender re-encoded idle in play within 2.39/4096;
demo override visible; Vanilla route reviewed. Limits: preview is unlit, the
chase camera crops the in-game side-by-side, rebase covers the player's
channels only, and new animation ids (D08K3/D08K4) need a host-side mapping.

**Opened 2026-10-22 (split from D08K).** The D08K audit decoded the ANM
start/count pairs, eight-byte FRM records and eight-byte ROT commands
(readers `800593b4`, `80058c70`, `800597e4`; Q12 quaternion rotations,
bit29 translation, top five bits of word1 select the joint), but nothing yet
maps joints to Duke's mesh or proves a replacement animation can be loaded.
Start from `recomp/tools/local/audit_crouch_assets.py` and
[the audit](documentation/43-crouch-walk-audit.md).

**Acceptance:** document the HIE hierarchy and the joint-to-mesh mapping for
Duke. A local tool renders any original frame of any clip as a pose preview
that matches the live game (side-by-side capture of at least the crouch
178, crouch transition 176 and forward roll 181). A non-destructive local
override mechanism (outside `game/`, original media read-only, no
hand-edited generated C) replaces one original clip at runtime; re-encoding
an original clip through the pipeline and loading it as the override is
indistinguishable from the original in play, including frame events.
Vanilla loads no overrides. If any step is not feasible, record the concrete
blocker.

### D08K2 - Crouch-height movement collision and safe stand-up

**Done (user-accepted 2026-10-22).** User: "ok!!! now it's functionally working. the job is fulfilled on a technical level ... lock in and close out the done tasks which is the tech implementation." Animation quality continues in D08K6.

Was Needs review (2026-10-22). See [note 185](documentation/185-d08k2-crouch-hull.md).
The original move probe `0x8007926c` judges steps with standing clearance
0x370 and a head ray 0x370 up; `recomp/src/ttk/crouch_hull.inc` asks the same
original queries (object probe `0x800780b4`, floor/ceiling `0x8007765c`, head
ray `0x800774cc`, slope `0x80077438`) with a 0x2c0 hull (kneeling Duke is 591
tall, D08K1; the D08K3 crouch walk 654), refuses steps up or drops over 0x100, and follows the floor.
Debug-only test mover (`crouchwalk` / `crouchgo`) and `crouchscan` on the
backtick console. Finding: no crouch-only ceiling or overhang at any
selectable level start (scans of levels 0-3 and 5-12), and the walkthrough
mentions crouch/roll only for combat, so the hull is a safety rule; the
user's goal is precise combat movement. Evidence: walls, props, curbs,
stairs and two real edges in play; clearance gate shown with a test hull;
native fixture for the crouch-only case; suites and Vanilla route pass.
Limits: no natural low ceiling to show in play, steep slopes not met live,
feet slide in the test mover (animation and controls are D08K3/D08K4).

**Opened 2026-10-22 (split from D08K).** The roll movement path
(`80054308` -> `8007926c`, and `80054218`) uses a fixed standing clearance
`0x370` and head-ray offsets at `800794d0`, `80079530`, `800795dc`. A
crouched mover needs a lower hull. This job is independent of the animation
work and can proceed in parallel.

**Acceptance:** a Modernized-only movement mode moves Duke at a low hull
height so he passes under ceilings that block standing movement and is still
stopped by walls, edges and props. Stand-up is refused while the standing
headroom is blocked and allowed once it is clear. Prove it with a debug-only
test mover (the crouch idle may slide its feet here; this is not shipped
play), across low ceilings, slopes, stairs and ledge edges, with no clipping
into geometry and no falling through floors. Vanilla collision unchanged.

### D08K3 - Crouch-walk animation loops

**Done (user-accepted 2026-10-22).** User: "ok!!! now it's functionally working. the job is fulfilled on a technical level ... lock in and close out the done tasks which is the tech implementation." Animation quality continues in D08K6.

Was Needs review (2026-10-22). See [note 186](documentation/186-d08k3-crouch-walk-loops.md).
Every original crouch pose is a one-knee kneel, so the crouch walk is a new
gait: `recomp/tools/local/ttk_crouch_walk.py` solves two-bone IK on Duke's
skeleton (pelvis 330, 60% stance, feet flat and forward, travel equal to the
planted foot's slide) for forward, backward and both strafes, 8 keyframes at
about 200 units/s, footstep events like the walks, forward/backward split into
half-cycle clips like 72/74 and 82/83. Generated locally from the disc, never
committed. Planted feet slide at most 1.7 units (original walks 23-40 on
average); top 654, so the D08K2 hull is now 0x2c0. A preview pack replacing the
walk clips shows the crouch walk in game; a Blender file holds the clips for
review. Limits: D08K4 must drive the speed (Modernized pace is faster) and
judge the kneel entry/exit blends.

**Opened 2026-10-22 (split from D08K).** There is no original low walk; the
directional crouch clips (181-184) are full-body rolls. Using the D08K1
pipeline, author or blend (from the crouch 178, low pose 179 and walk
clips) low-gait loops.

**Acceptance:** loops for forward, backward and both strafe directions (or a
documented blend that covers them) with alternating planted feet, root
travel that matches the intended crouch speed (no foot sliding at that
speed), clean entry from and exit to the crouch 178, and original event
semantics kept (footsteps or none, no stray events). Weapon-holding upper
body stays usable for aiming. Previewed in the D08K1 tool and loaded in game
through the override. Any new art is ours, tracked as a project asset, not
derived retail data committed to the repo.

### D08K4 - Modernized crouch walking prototype

**Done (user-accepted 2026-10-22).** User: "ok!!! now it's functionally working. the job is fulfilled on a technical level ... lock in and close out the done tasks which is the tech implementation." Animation quality continues in D08K6.

Was Needs playtest (2026-10-22). See [note 187](documentation/187-d08k4-crouch-walking.md).
User: "the options should be vanilla (roll) OR modern crouch walking with
modern crouch being the default". CROUCH MODERN (default; schema 39, run.py
`--crouch`, OPTIONS > ADVANCED > MOVEMENT) walks low with held crouch plus a
direction, Duke facing the view, through the D08K2 crouch probe, playing the
D08K3 clips on channel 0 (host-driven frames, pack built from the disc by
CMake); CROUCH ORIGINAL keeps the roll; Vanilla unchanged. Lab: no roll,
crouch-walk frames in four directions, kneel on stop, stand on release;
original setting rolls. Executable `862d7b9b8a98c91c55427ada9a49e402ef2d3083be99450a5e7475e02fc81c32`. Limits: no footstep sounds,
diagonals use the dominant clip, fixed speed.

**Opened 2026-10-22 (split from D08K).** Combine the D08K2 low hull with the
D08K3 loops into the control the user asked for: hold crouch plus a
direction to walk low and slowly in every direction, not roll.

**Acceptance:** in Modernized, held crouch + direction walks low with the new
loops and camera-relative direction; no automatic roll for ordinary
crouch + direction. Aiming and firing work while crouch-walking; release
stands up only where headroom allows (otherwise stays low). The original
roll stays reachable through a documented separate Modernized binding (or a
menu choice) without replacing custom bindings; Vanilla keeps the original
roll and controls unchanged. Third and first person both work.

### D08K5 - Crouch-walk coverage and regression

**Done (user-accepted 2026-10-22).** Verification complete
without gameplay changes. [Note 198](documentation/198-d08k5-crouch-coverage.md)
records four-level/multi-era 120 Hz coverage, first-person 60/120 Hz checks,
combat, crouched saves, native mantle/ladder/swim transitions, death/Continue,
all three native suites and the reviewed Vanilla route. Earlier accepted
terrain coverage is retained; low-headroom safety is fixture-proven, not a
claim of a natural crouch-only vent. User reports extensive happy continued
use. Final acceptance now completes D08K too. User explicitly authorized
documentation, commit and push closeout.

**Opened 2026-10-22 (split from D08K).** Game-wide verification of D08K4,
following D22B (modern controls in every level).

**Acceptance:** tested in several levels and eras: low ceilings and vents,
stairs, slopes and edges, release under cover, weapon switching and firing,
taking damage, jumping/mantling/ladders/swimming interrupting a crouch
walk, both cameras, high refresh rates, savestate load while crouched, death
and Continue. Vanilla regression route passes unchanged. User playtest
accepts it; then D08K closes too.

The crouch refinements D08K7 (stuck crouch / roll on objects), D08K8 (kneel
idle), D08K9 (crouch on landing), D08K10 (first-person crouched weapons) and
D08K11 (shallow-water crouching), opened 2026-10-22, are best done first
so the coverage pass tests the final behaviour.

### D08K6 - Crouch-walk animation quality (Solid Snake, not "on the toilet")

**Done (user-accepted 2026-10-22).** User: "that's it!! i'm happy with this end to end. the anim is now fully complete. the technical implementation is now fully complete. this is totally playable in every way at this point. lock it in, close it out". Final: variant C, head level, hunched
firing on the crosshair, steady arms. Executable
`789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03`.

**Was Needs playtest (2026-10-22).** See [note 188](documentation/188-d08k6-crouch-walk-animation-quality.md),
which is also the reference for future custom animation and model work (user
request). Measured why D08K4 sat: pelvis 300 (hip joints 0.47 leg lengths over
the ankles against the capture's 0.69, thighs level), feet about 4x the hips'
spread, lean put in the pelvis (the capture's pelvis stays level and bends at
the spine), kneel arms hanging to the knees. New gait (`ttk_crouch_walk.py`
`TACTICAL`): hips at 0.62 leg lengths, feet narrow under the hips, knees over
the toes, pelvis 12 degrees with the capture's yaw/roll swing, chest 48
degrees over the knees with counter-swing, head level, fists up front when no
weapon owns the arms; upper body in separate host clips 11-15, written at
`0x800411b8` after every blend (`crouch_walk_upper`), weapon-owned arms
compensated to keep the pose's world direction, 6-update ramps. Left-arm
shake: animation 178's upper channels followed the host legs frames and were
reset every update, leaving the chest/left arm 2-26 degrees off every few
updates armed; now never reaches the model. Variant packs (`--set`,
`DNTTK_ANIM_PACK`), `ttk_pose_study.py` (sample / metrics / flipbook). Lab:
in-game pose numbers, chase shots of old and four variants, release stands;
`test_ttk_anim` 14 OK. Executable
`dda1d28f2626ffa5b0464d0adcc83bb99483dc3879ef9e1009069d342382a3be`.
Revision 2 (user: "arms are now pointing straight down after crouch"): armed,
the left-arm/chest channels' joint masks gained the right arm and chest
(masks only grow) and the weapon-arm compensation compounded on records the
game had not rewritten; masks are now trimmed to each channel's original
joints and the weapon arm is computed from its own channel frames. Arms match
their pre-crouch pose after release; the pistol stays up by the shoulder.
Executable `6ca0122ee58dce894263394fa0d1ba7623763e8bba5aa36f6967bf9c6d77dbcf`.
Revision 3 (user: "when duke shoots crouched, he shoots directly up"): the
original aim build replaces the chest with a view look-at while firing (bit
`0x100`, as in the swim work), so our lean/compensation tipped the gun 49-58
degrees up; while the bit is set the upper body now takes its channels' own
pose and the lean ramps back after. Crouched fire -9 to 0 degrees (standing
-2), also while crouch walking. Executable
`cd48d3e3d7cd5f0b35f3364b2179d0a4209b1ca3ac79b67ec6354312f92db900`.
Revision 4 (user: "i liked C ... he is constantly looking down ... his torso
doesnt jerk upwards like bolt upright"): default is variant C; the head was
never written with a weapon drawn (arm range bug) and now looks ahead (5
degrees); firing tilts the original aim's chest look-at down 50 degrees and
turns the shoulders back by the same tilt, so Duke stays hunched (62 -> 56
degrees) with the gun on the crosshair. Executable
`789d532a18086bf3c1f0612897106ac8ddde51f8007bbbdc81eb07e179cefa03`.
Limits: first person not judged, no footsteps, diagonals dominant clip.

**Opened 2026-10-22 from the user's D08K4 acceptance.** User: "now it's time
to actually refine the animation ... make it look much more realistic and
higher quality rather than the "duke sat on a toilet" look. we can use the
research/crouchwalk-anim as a deep study because it doesnt feel like this even
resembled this cool looking walk ... the only glitch im seeing now is that there
is a slight shake in duke's left arm when crouch walking. We need to go from
this "on the toilet" look to a real solid snake type look."

Starting point (D08K4 revision 3, note 187): the CC-BY "walk crouch" of
"Modern Army Soldier with +24 pro Animations" (abdlilah ben, Sketchfab) is
reduced to hips height, torso direction, ankle paths and foot rotations
(`recomp/assets/anim/crouch-walk-mocap.json`, `extract_crouch_mocap.py`) and
retargeted onto Duke's legs with two-bone IK (`ttk_crouch_walk.py`); the pelvis
is forced to 300 (deeper than the capture's 366) with only the lean taken from
the torso direction, the upper body stays on the original channels, strafes
turn the stride sideways.

Study and acceptance:
- Study the source rig's full motion (hips orientation including yaw/roll,
  spine/chest bend, knee direction, foot placement width and toe-out, arm
  carry) against what reaches Duke, side by side in Blender and in game, and
  find what makes it read as "sitting" (likely: pelvis too low and too far
  back for the stride, upright chest on a pitched pelvis, knees splayed,
  feet too wide, missing hip sway/rotation).
- A tactical crouch walk in the spirit of Solid Snake / the capture: weight
  forward, chest leaning over the knees, smooth heel-to-toe steps, natural
  hip and shoulder counter-rotation; strafes and backward that read as
  intentional (dedicated captured cycles or a retarget of the pack's other
  motions if better); the squat idle and entry/exit (`idle simple to crouch`,
  `simple to crouch`) considered.
- Upper body: carry the weapon low and ready while crouch walking where the
  channels allow (chest/arm presentation without breaking aim or fire).
- Fix the slight left-arm shake while crouch walking.
- Keep: planted feet at the walking pace, the D08K2 collision and hull,
  first person, Vanilla unchanged, the attribution.
- User playtest accepts the look.

### D08K7 - Stuck crouch and legacy roll on the slot 10 dumpster

**Done (user-accepted 2026-10-22).** User: "confirmed working! lock it in and close it out". Executable
`a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963` is the regression baseline.

Was Needs playtest (2026-10-22). See [note 189](documentation/189-d08k7-crouch-on-objects.md).
Confirmed in a lab copy of slot 10: after a step on the dumpster `+0x174`
holds it, every crouch probe was refused (`hull_object`), stand-up refused
forever, and W reached the original crouch handler (roll 181, off the
dumpster). The original walker (`0x8007926c`) never refuses a contact: it asks
`0x80077748` for the step onto the object's top, refuses one taller than a
step, walks on objects whose type has flag `0x1000` (the dumpsters) and lets
the room decide for others; it also judges a look-ahead point
(`(+0xb0 * 275) >> 12` along the move). The crouch probe and stand-up now
follow the same rules (`crouch_floor`, `crouch_stand_room`), and while a
released crouch waits for room the crouch walk keeps the directions (no
roll) and Duke stands by himself once there is room. Lab: slot 10 crouch walks
over the top, stops at the edge where standing does, stands on release, W no
roll; refused stand-up (debug `crouchstand block`) crouch walks and stands
when freed; CROUCH ORIGINAL rolls and now stands on the dumpster; the alley
dumpster stops the crouch at its side and is crouch walked on top. Suites
pass. Executable
`a98a334afdc8f5860894c73f82cfd5d515cebd2e14a998dd241054c6db591963`.

Opened 2026-10-22 from a user report.
First of three crouch refinements after D08K6 (D08K7, then D08K8, then D08K9).

Reproduction (player UI slot 10; level 5, the dumpster on top of which Duke
starts, type 165, flags `0x081090e2`, the fixed-step dumpster of D08T4 /
[note 163](documentation/163-d08t4-fixed-pushables.md)): hold LCtrl to crouch,
try to crouch walk forward, release everything. Duke stays on one knee instead
of standing; pressing W then plays the original roll.

Code findings (2026-10-22, reading only; not yet confirmed in a lab run):

- Stand-up is refused whenever Duke has an object contact.
  `stance_request()` (`recomp/src/ttk/crouch.inc`) returns early
  (`++crouch_blocked`) on release if `player+0x174` is non-zero, before the
  0x370 clearance query `0x8007765c`. Entering crouch has no such test, so on
  an object Duke can crouch but never stand up again. The refusal repeats every
  update with no time limit and no other way out. If standing on the dumpster
  keeps the dumpster in `+0x174`, it alone explains "stuck on one knee".
- The crouch walk cannot move there either. `crouch_probe()`
  (`recomp/src/ttk/crouch_hull.inc`) runs the object probe `0x800780b4` first
  and fails with `hull_object` whenever it leaves a contact. On top of an
  object that probe may report the supporting object itself, so every step is
  refused (count in `crouch_walk_blocked` / `hull_blocks[6]`).
- The roll is the original handler. After release `crouch_owned` stays true
  and the lower animation stays 178. `crouch_walk_eligible()` requires
  crouch held, so `crouch_walk_owning` drops, `crouch_walk_release()` returns
  channel 0 to animation 178's own clip (the original one-knee kneel, not the
  D08K4 squat idle), and `pc_input.cpp` (`crouch_walk_input_ready()`) stops
  withholding directions. W then reaches the original crouch handler, which
  starts the roll 181. The same holds wherever stand-up is refused for a
  real reason (a low ceiling): with Ctrl released, any direction rolls. That
  is a state-machine gap, not only a dumpster case.

So it is likely both: an object-relative stand-up rule that is too strict
(collision edge case) and a crouch state machine whose owned but refused
state hands directions to the original roll (broader). Confirm in a private
lab copy of slot 10 (offscreen, `DNTTK_TRAVERSAL_TRACE`, debug counters
`crouch_blocked`, `hull_blocks`, the value of `+0x174` and the floor record
under Duke, crouched and standing on the dumpster) before changing anything.

Fix direction to evaluate (not decided):
- Stand-up on an object: judge the standing headroom with the room query at
  Duke's position plus the supporting object's top. Treat the object
  underneath as floor, not a blocker, as the original standing move probe
  `0x8007926c` does when Duke walks on it. Still refuse under a real
  overhang.
- Crouch probe: ignore the contact that is the object Duke stands on; keep
  refusing objects in the way.
- While Modern crouch still owns a refused stance (Ctrl released, no room),
  keep the host owning the directions (crouch walk) so the original roll never
  starts, and stand up automatically when room appears. CROUCH ORIGINAL and
  Vanilla keep the original roll.

**Acceptance:** the exact slot 10 sequence stands Duke up on release and W
walks normally (no roll). Crouch walking works on top of objects Duke can
stand on, stopping at their edges. Where standing is genuinely blocked, Duke
stays crouched, directions crouch walk (never the original roll in CROUCH
MODERN), and he stands as soon as headroom allows. CROUCH ORIGINAL keeps its
roll; Vanilla unchanged. Checked on the slot 10 dumpster, the alley dumpster
(pushable) and at least one ordinary floor and one low ceiling.

### D08K8 - One-knee kneel as the stationary crouch pose

**Done (user-accepted 2026-10-22).** Implemented: original kneel, transitions from
the displayed pose, saved animation pack refresh. [Note 190](documentation/190-d08k8-stationary-kneel.md).
User: "this is perfect!! lock it in, close it out". The findings below record the design research.
After D08K7 (it showed the kneel the user wants).

User: the D08K4 still crouch (both feet planted, the squat idle) works, but
the original one-knee crouch looked better and fits Duke. Wanted: still
crouched = one knee, held steady; smooth into the crouch walk when moving,
back to the kneel on stop, and back to standing on release. Animation only,
no change to crouch physics or speed.

Code findings (2026-10-22):

- The one-knee pose is the original crouch idle, animation 178 (entered and
  left through 176, `recomp/src/ttk/crouch.inc`). Every original crouch pose
  is a one-knee kneel (note 186). In D08K7 it appears because
  `crouch_walk_release()` hands channel 0 back to 178's own clip.
- It was replaced on purpose. In D08K4 revision 2 (note 187, "Kneel / walk
  popping") stopping returned to the kneel and starting left it again, with a
  visible pop, so held Modern crouch now shows the squat idle
  (`crouch_idle_clip` 5 in `crouch_walk.inc`, upper body from host clip 15)
  and every change blends over `crouch_walk_ramp` (6) updates. Going back to
  the kneel means fixing that pop, not just changing the idle clip.
- What changes between kneel and walk: pelvis 282 (`crouch_kneel_pelvis`)
  against about 300 plus bob; one knee down against both feet planted; the
  kneel's upper channels (arms hanging toward the knees, note 188) against
  the D08K6 hunched chest and fists, or a weapon pose. The upper body is
  written by `crouch_walk_upper()`, which keeps its own weight ramp.
- Collision stays as it is. The kneel is 591 tall, under the 0x2c0 hull (D08K2).

Options to evaluate (pick by preview and in-game look, D08K1 tools
`ttk_anim.py`, `ttk_pose_study.py`):
1. Idle on 178's own frames for legs and the original upper channels.
   Authored transition clips kneel -> walk and walk -> kneel (host extra
   clips, a few frames, feet planted, knee rising first) rather than a 6-update
   slerp. Upper body ramps between the kneel's channels and the D08K6 carry.
2. Retarget the kneel into a host idle clip that shares the walk's pelvis
   frame and joint conventions, so the existing blend has less to bridge.
Also decide the upper body while kneeling armed (the weapon's crouched aim
pose stays original; the D08K6 firing hunch is for the walk) and keep the
D08K6 fixes (channel masks, no left-arm shake, crouched fire on the
crosshair).

**Acceptance:** in CROUCH MODERN, stationary crouch shows the one-knee kneel,
steady while held (armed and unarmed). Starting to move goes into the crouch
walk, and stopping returns to the kneel, both without a pop and with no input
delay. Releasing crouch stands as before (D08K7 rules). Firing from the kneel
aims on the crosshair. Crouch-walk look and speed unchanged; CROUCH ORIGINAL
and Vanilla unchanged. User playtest judges the look in third person.

### D08K9 - Crouch held through a jump: crouch at once on landing

**Done (user-accepted 2026-10-22).** Implemented after D08K8: held crouch
claims the soft landing before its animation initializes, blending from the
flight pose into the kneel or crouch walk. No 105 wait or intermediate gait.
24 private-copy live cases pass across both jump styles and cameras; native
suites pass. User accepted the functionality and final Crouch-toggle/menu revision. [Note 191](documentation/191-d08k9-crouch-on-landing.md).

The investigation below records the pre-fix behavior.

Reproduction (player UI slot 4, `dnmonsters` optional): Shift + W run, Space
to jump, hold LCtrl in the air and through the landing. A short pause or
judder between touching down and crouching. Wanted: Duke3D / Half-Life feel.
Crouch held in the air is honoured on landing with no standing frame, pause
or snap. Jumps, landings, fall damage and collision stay intact.

Code findings (2026-10-22, reading only; which path the run jump takes
must be traced):

- No crouch request survives the flight. `stance_request()` only starts
  crouch (176) on supported ground with `movement_ready()` and
  `!(+0x224 & 0x200002c5)`; it ignores the airborne updates, so the request
  happens only once the landing has handed back a ground state.
- Path A, landing recovery 105 (no direction held at touch-down, or a
  straight flight): D08Z3 `landing_recover()` (`recomp/src/ttk/landing.inc`)
  deliberately returns false when crouch is held, so the original landing
  pose handler `0x80052fb8` keeps 105 until it ends (about 37 fields at 60 Hz,
  about 0.6 s, no input taken) before crouch can begin.
- Path B, gait continued (a running flight with W still held): the landing
  selector `0x80054c04` continues the run 76. The crouch then starts on a
  later update from a running pose, so at least one run/stand frame is shown
  and the 176 entry (boosted 4x in `stance_advance()`) blends from a run
  stride.
- With W held the crouch walk can take over only after 176 has reached 178
  (`crouch_walk_eligible()`), adding the 176 entry and the 6-update ramp.
- Manual (D08Z) and assisted jumps share these paths. The D08Z3 hard landing
  106 and fall damage are original.

So the delay is most likely gameplay state handling (no airborne crouch
request; 105 lock or gait continuation) plus animation selection (176 from
a run pose). Confirm with a lab trace (private slot 4 copy, held keys at the
user's timing, `DNTTK_TRAVERSAL_TRACE` landing lines, per-update `+0x60`,
`+0x22c/+0x22d`, frame and position) in both paths before fixing.

Fix direction to evaluate (not decided): remember crouch held during an
owned flight; on the landing update (where `landing_recover()` runs) start
crouch directly instead of 105 or the gait, that is 176 at the frame matching
a landed crouch, or straight into 178 / the D08K8 idle with a short pelvis
ease, and with a direction held go straight into the crouch walk. Keep the
hard landing 106, fall damage, water entries and the landing sound.

**Acceptance:** with crouch held through a jump (standing, walking and
running jumps, manual and assisted jump styles, with and without a direction
held at landing), Duke is crouched from the landing update: no standing or
running frame, no 105 pause, no visible snap, in both cameras. Crouch pressed
only after landing behaves as today. Hard landings still play 106; jumps,
landings, collision and Vanilla unchanged.

### D08K9 follow-up - Crouch toggle owns crouch-on-landing

User accepts the functional result ("functionally its perfect"). LANDING
is hidden from Advanced; CROUCH alone controls crouch-on-landing, including
when the legacy landing preference is Original. Build/native controls and
a private live Original-landing check pass. Revised candidate
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`.
Ready for review; no documentation acceptance closeout, commit or push.

### D08K10 - First-person crouched weapon and arm alignment

**Done (user-accepted 2026-10-22).**
User accepted revision 2: "perfect!! lock it in, close it out. great work".
The six animated weapons and held items use their arm channel for the
first-person crouch display; each high-refresh replay frame carries the
correct eligibility. Standing, seeded guns and third-person animation retain
their paths. Build, native checks and presented 120 Hz captures pass; user
playtest accepts the result. Documentation, commit and push authorized.
[Implementation and evidence](documentation/192-d08k10-crouched-viewmodel.md).
The initial research and acceptance criteria follow.

User: in first person, five weapons sit noticeably differently when Duke is
crouched than when he stands: Throwing Knife, Throwing Axe, Dynamite, Holy
Hand Grenade and Crossbow. The others look right and are the reference.
Wanted: visual consistency across stances, with **the standing position as
the baseline** (crouched is brought to standing, not both moved to meet in
the middle). The user also expects the same fix to help the later
first-person jetpack (D11F) and swimming (D11E) jobs.

Slots (`documentation/33-controls-shortcuts.md`, `player+0x3b9`): knife 1,
axe 2, crossbow 3, Holy Hand Grenade 13, dynamite 14.

Code findings (2026-10-22, reading only; confirm in a lab before changing
anything):

- The five weapons are exactly the drawn-weapon slots without a seeded
  firing pose. `first_person_weapon` (`recomp/src/ttk/first_person.inc`,
  D12, [note 63](documentation/63-d12-first-person-weapons.md)) places the
  hand from `display_pose()`. Slots 4-11 have seeds in `fire_poses[]`
  (surveyed standing, level view) and are always held in that fixed pose:
  the live animation never reaches the screen, so stance cannot move them.
  Slots 1, 2, 3, 13 and 14 have no seed (they never draw the muzzle flash
  `0x800341e4`, or have not yet), so `display_pose()` uses the **live** hand
  relative to the torso joint (`live_q`, `live_rel`), placed by the pistol's
  standing torso seed and scaled about `weapon_reference` by
  `weapon_motion()` (0.5) around the anchor (110, 150, 520). Whatever the
  crouched animation does to the right arm relative to the chest therefore
  shows directly in the view. This is the likely cause, and it is the
  first-person path's design (D12), not a per-weapon bug.
- What changes the live arm when crouched: the original crouch (176/178)
  upper channels and the weapons' crouched carry/throw frames (legacy data,
  channels 1-3 at `player+0x74/+0x88/+0x9c`), and in CROUCH MODERN the
  D08K6 upper-body layer `crouch_walk_upper()` (`recomp/src/ttk/crouch_walk.inc`,
  hooked at `0x800411b8`), which writes the chest/arm joint records,
  re-bases weapon-owned arms through the `fix` compensation, and while firing
  (`+0x224 & 0x100`) tilts the chest look-at by `crouch_fire_lean`. Its
  ramps (`crouch_walk_ramp`, 6 updates) explain a shift that grows when
  entering crouch or the crouch walk. Which source dominates for each of the
  five must be measured (legacy data, the modern crouch layer, or both).
- The crossbow (slot 3) may draw a flash. If so, the unseeded path records
  its pose at the **first shot of the session**, crouched or not, and keeps
  it, so its first-person framing can depend on the stance of that first
  shot. Check with `DNTTK_FP_WEAPON_TRACE=1`.
- The pipe bomb (slot 12) is also unseeded and should show the same shift,
  although the user did not list it; check it with the five.
- Held items (`player+0x3b8` = 1, such as a remote detonator or an unlit
  stick) fall back to slot 32 in `held_weapon_slot()`: pistol torso, no tilt.
  Check whether Modern dynamite (D08A12, unlit until fire) passes through it.
- Not a cause: the eye. `first_person_update()` follows the head height and
  smooths it for all weapons alike, and the viewmodel is anchored in view
  space, so a pure eye-height change would move every weapon the same way.

Fix direction to evaluate (not decided; prefer one shared mechanism over
per-weapon numbers):

1. A stance-neutral viewmodel pose source. For unseeded slots, display the
   weapon's standing rest pose plus the throw/draw/reload motion measured
   relative to the current stance's rest, i.e. standing rest x (live / live
   rest of the current stance), so idle crouched equals idle standing and
   throws keep their motion. Read the arm from its own channel frames
   (as `crouch_channel_pose()` does), not the joint records, so the D08K6
   record writes and compensation never reach the view. The same layer then
   serves swimming and jetpack poses (D11E, D11F), which also bend the body.
2. Or seed the five with standing rest poses (a survey like D12's, standing,
   level view) and add the live motion as a delta. Simpler, but it needs the
   delta rule from option 1 anyway for the throws.
Avoid adding per-slot crouch offsets to `weapon_slot_offset[]`: they would
not track the D08K8 kneel or the crouch-walk ramps.

Dependencies and overlap:
- D08K8 (one-knee kneel idle) changes the stationary crouch's upper
  channels and the kneel <-> walk transitions. Do D08K10 after it, or define
  the pose source so it ignores which crouch idle is playing; re-check the
  five against the final kneel either way.
- D08K9 (crouch on landing) and D08K7 (refused stand-up keeps crouch) add
  more ways into and out of crouch: test those transitions too.
- D08K6 crouched-fire fixes (chest look-at tilt, channel masks) must stay
  intact in third person; this job changes only the first-person display.
- D08K5 coverage should run after this so it tests the final view.
- D12 seeds and framing of slots 4-11, D12A kick, D12D (rocket launcher
  framing) must not regress.

Investigation plan: private lab copy (offscreen, first person, `dnweapons`),
for each of slots 1, 2, 3, 12, 13, 14 and a reference (4, 5): standing idle,
crouched idle (CROUCH MODERN and ORIGINAL), crouch walk, entry/exit, and a
throw or shot in each; log `fp.weapon` debug JSON, `live_q`/`live_rel`,
channel animation ids `+0x74/+0x88/+0x9c`, `crouch_upper_writes` /
`crouch_upper_compensated`, and capture window grabs. A run with the D08K6
upper layer off tells legacy from modern.

**Acceptance:** in Modernized first person the five weapons (and the pipe
bomb if affected) look the same crouched, kneeling and crouch walking as
standing; no jump when entering or leaving crouch or the crouch walk;
throwing, firing, reloading, switching, view bob (D17V) and the D12 kick work
in every stance. Slots 4-11 and the third-person animations (including the
D08K6 crouched fire) unchanged; CROUCH ORIGINAL and Vanilla unchanged. User
playtest accepts the look.

### D08K11 - Enable Modern Crouching and Crouch-Walking in Shallow Water

**Done (accepted 2026-10-22).**
Accepted implementation and evidence: [note 193](documentation/193-d08k11-shallow-water-crouch.md).
Additional job in the Crouch Behaviour Refinement workstream (D08K7-D08K10).
Implementation and private-copy runtime testing authorized by the user selection.

**Original reproduction report (subsequently reproduced in the private lab):**
load save slot 10, locate Duke in the flooded subway's shallow water, confirm
normal walking, then hold Ctrl while stationary and with movement in each
direction. Duke does not enter the normal modern crouch or crouch walk even
though he can stand and walk. The implementation lab used private save/card and settings copies.

**Expected:** shallow water alone must not prevent modern crouching. Held
Ctrl permits stationary kneeling and crouch walking in every direction with
the existing crouch speed and responsiveness. Releasing Ctrl stands Duke
when clearance permits. All stance transitions remain smooth, with no
legacy roll. Preserve dry-land behaviour, normal water walk/run speeds,
surface/underwater swimming, and Vanilla / CROUCH ORIGINAL behaviour.
Crouching while surface swimming or underwater is outside this job.

**Initial investigation (2026-10-22; historical pre-implementation findings):**

- There is an explicit water veto, not merely a missing animation.
  `stance_request()` in `recomp/src/ttk/crouch.inc` clears `crouch_owned`
  and returns on `in_water_early()`. Its comment explains that animation
  176 is also the original water dip. `crouch_walk_eligible()` in
  `recomp/src/ttk/crouch_walk.inc` independently requires
  `!in_water_early()` and state bytes `player+0x22c/+0x22d == 0`.
  Thus removing only the entry veto would still leave walking unavailable.
- `in_water_early()` in `recomp/src/ttk/modern_controls.cpp` means body Y
  (`player+8`) below the water surface (`+0x834`), OR a present water plane
  and nonzero water flag (`+0x20c`). The no-water sentinel is `0x40000000`.
  It does not distinguish shallow walking from swimming. In contrast,
  `shallow_land_water_early()` excludes native states 4/5 and accepts depth
  (`+0x1c8` feet/floor Y minus `+0x834`) below `0x281`; `state()` already
  preserves a land movement lease for supported wading animations. This
  discrepancy is a likely explanation for walking working while crouch fails;
  slot 10's actual flags, depth and animation still need measurement.
- `recomp/src/ttk/swim.inc` documents the original `0x8005201c` bands:
  below `0x200` shallow land gaits, `[0x200,0x281)` mid-depth walking,
  and `>=0x281` deep-water handling. `free_swimming()` combines water flags,
  depth and animation, and always recognizes native state 4 (surface,
  animations 122-126) and 5 (underwater, 127-130/133). Depth alone is not
  sufficient: wade animations include 178/181, while 176 also serves the
  water dip. Audit the original handler before allowing crouch ownership
  there, including mid-depth 80/81 and entry/exit frames.
- `crouch_hull.inc` uses the original object, floor/ceiling, head-ray and
  slope queries with a `0x2c0` moving hull and `0x370` standing clearance;
  D08K7 judges object tops and retains crouch when stand-up is refused.
  The diagnostic test mover also rejects `in_water_early()`. The production
  probe has no matching blanket water veto, so collision is not established
  as the initiating cause. Validate its floor following and step limits at
  water edges without weakening clearance checks.
- The crouch walk uses `modern_pace_step(false)` and masks directional pad
  input through `crouch_walk_input_ready()` in `pc_input.cpp` to prevent
  original rolls. Water input separately assigns Ctrl to descent/dive and
  can own locomotion. Audit ownership and update order together, not just
  animation selection. Preserve the D08K8 stationary kneel and blends.
- The existing submersion helper compares body Y, not the animated head.
  The crouch hull and visual head height are different quantities. Trace
  original water-state/oxygen updates, body origin, feet, actual head and
  camera height when crouching puts the head below the surface. Establish
  the correct transition policy from that evidence; do not force grounded
  mode or clear water flags to keep crouch alive. Exact native head/oxygen
  thresholds and slot 10's depth are unresolved by this lightweight review.

**Dependencies and related investigation:** D08K2 collision/safe stand-up
([note 185](documentation/185-d08k2-crouch-hull.md)), D08K4 ownership/input
([note 187](documentation/187-d08k4-crouch-walking.md)), D08K7 state and
object collision investigation ([note 189](documentation/189-d08k7-crouch-on-objects.md)),
and D08K8 kneel ([note 190](documentation/190-d08k8-stationary-kneel.md)).
Preserve D08AA shared water pace and D08AB SWIMMING MODERN/ORIGINAL choices
([notes 156](documentation/156-d08aa-unified-pace.md) and
[157](documentation/157-d08ab-swimming-option.md)); the latter intentionally
retains its existing standing forced-run behaviour. Coordinate transition
coverage with D08K9 and camera/weapon pose coverage with D08K10; neither is
a prerequisite for investigation. Complete this refinement before D08K5's
final game-wide coverage pass.

**Future investigation / acceptance:**

1. Reproduce slot 10 with private copies. Log state/previous state, water
   flag, surface/body/feet/head heights, animation channels, crouch ownership
   and input leases before Ctrl, during stationary/moving crouch, and after
   release. Separate the confirmed source veto from any native restriction.
2. Duke crouches and crouch-walks forward, backward, sideways and diagonally
   at the existing modern crouch pace; stationary kneeling and all transitions
   are smooth. Ctrl release stands only with sufficient room; no roll or
   stuck crouch, including objects and low clearance.
3. Walk through dry/shallow/mid-depth/deep boundaries in both directions,
   with crouch held and released, including the head-below-surface case.
   No unexpected swim activation or stuck movement state; genuine submersion
   follows the investigated water-state rules. Do not add swimming crouch.
4. Dry-land crouch, walk/run water speeds (both SWIMMING options), surface
   swimming, dive, underwater movement, surfacing and swim-fire remain
   unchanged. Check both cameras for height/pose discontinuities, collision
   and animation regressions, plus CROUCH ORIGINAL and Vanilla controls.

### D08K12 - Crouch running

**Status: Done (user-accepted; closeout authorized 2026-10-27).** User selected D08K12 ahead
of the previously planned audio follow-ups. D18B/C/E/F are scheduling
preferences, not implementation dependencies. Implementation prerequisites:
D08K, D08K11, D08L1 and D21, all accepted.

Add Duke Nukem 3D / EDuke32-inspired crouch running: while crouched and
moving, holding the configured run action increases speed to the equivalent
of running speed while preserving crouch. Releasing run restores normal
crouch-walk speed. Support momentary Ctrl crouch and C's crouch toggle,
following configured actions and custom bindings instead of fixed keys.

Inspect current movement and crouch ownership before changing code. Preserve
crouch collision, low-space clearance, existing animations and behavior as
far as practical, including walking off ledges. Never stand Duke merely to
apply running speed. Report animation or speed limitations needing a design
decision.

Acceptance: test both crouch controls with and without run, custom bindings,
vent traversal, release of run, blocked standing under insufficient clearance,
and crouched ledge walk-offs. Preserve normal standing locomotion, C toggle
cancellation rules, save/load, first/third person and Vanilla. No implementation,
build or gameplay test is authorized by this backlog intake.

### D08K13 - Sustained backwards crouch movement enters a legacy roll

**Status: Done, user accepted and closed (2026-10-27). Priority 1.**

User reports continuous backwards crouch running enters the legacy roll after
about 3-5 seconds in both first and third person. Reproduce and trace the
ownership/input transition before fixing it. While the crouch action remains
active, supported crouch walking/running in every direction must remain Modern.
Preserve collision, clearance, blocked standing and ledge handoff, configured
controls, Ctrl/C takeover and Original/Vanilla behavior.

Acceptance: sustained backwards crouch running beyond 5 seconds in both views;
repeated forward/backward changes with and without run using both momentary and
toggled crouch; regression checks for low clearance, collision and ledges.
Distinguish actual gameplay evidence from native fixtures and user reports.
Findings and test matrix: [note 286](documentation/286-d08k13-d08k14-crouch-movement.md).

### D08K14 - MGS-based crouch walk/run quality, hand posture and HTML preview

**Status: Done, revision 2 accepted and closed (2026-10-27). Priority 2.**
User accepts the animation and requests lock-in/closeout. Natural-vent coverage
remains an explicit evidence limit. D08K13 sustained-input checks pass in both
views. The accepted HTML artifact and detailed record are in note 286.

Study both local references `research/screencaps/mgs-crouch-run.mp4` and
`research/screencaps/mgs-crouch-walk.mp4` in motion and across their gait cycles
before authoring. Use the existing animation pipeline and Duke's skeleton to
create a separate low run gait with purposeful longer strides and suitable foot
placement at actual crouch-run speed. Revision 2 explicitly permits improving
the walk and hand posture too; keep current travel speeds. Do not merely
accelerate walk frames for running. Blend smoothly on run press/release and
retain the vent-compatible silhouette. Additional user references may inform it.

Acceptance: provide a local HTML animation preview comparable to the existing
artifact; inspect in-game at actual speed, including vents, both travel
orientations and walk/run transitions. Check foot sliding, clipping and abrupt
pose changes; report skeleton, speed or clearance limits needing a design
decision. D08K13 takes precedence. Findings and preview/test evidence belong in
[note 286](documentation/286-d08k13-d08k14-crouch-movement.md).

### D08K15 - Duke jerks upright when the box he crouches on is kicked apart

**Status: Done (2026-11-08, accepted and closed; user: "thats a winner. lock it in and close it out"; [note 420](documentation/420-d08k15-fall-start-camera.md)). Dependencies: D08K7, D12A, D12E1.**
User: "for slot 9, when you crouch on this box, look down, and then quick kick
to break it, theres some weird glitch that happens when duke selfrights really
quickly, like jerks him somehow. this is all boxes of this type in fact."
Reproduce on a private copy of UI slot 9 (savestate file `slot08`; never the
player's saves): crouch on the breakable box, look down, quick kick it apart,
in first and third person at 180 Hz, then on other boxes of the same type
(game-wide, not this box only). Lead from D08K7 (note 189): on an object top
Duke's contact `player+0x174` holds the box and the crouch stand-up and hull
use it (`crouch_hull.inc` `crouch_floor`); when the box breaks the support
vanishes under the one-knee kneel (D08K8), so trace per frame the animation,
`+0x174`, feet height, crouch state and the quick-kick leg/body draw (D12A,
D12E1) to find which owner snaps him upright. Fix as a smooth drop (stay low
or recover with the normal stand-up), Modernized only; Vanilla unchanged.
Acceptance: user playtest of slot 9 and another box of the type.

**Work log (2026-11-08, Needs playtest).** Cause: the jerk was the view, not
Duke's body. When the crate breaks, the fall initializer sets 108 one update
before mode 9, and that update (108 at 0/0) was not in the camera lease. First
person dropped for one update ("unsupported-camera") and the orbit re-seeded
from the native camera, so the pitch snapped from looking down (1.046) to
level (0.039). `unowned_fall_early()` in `modern_controls.cpp` now also covers
108 at 0/0 when it is not an owned short fall: every unowned fall start, not
only this crate. Lab (`recomp/analysis/d08k15/`, private slot 9 copy, first
person, 180 and 60 Hz): first person stays active, pitch holds through the
break, fall and crouch landing (held Ctrl stays low; C toggle lands with the
normal soft landing and stands, as the D21 latch ends in the air).
`ttk-input-test` and `ttk-aim-test` pass. `ttk-controls-test` fails at the D14
`hud_intact()` assertion with or without this change (pre-existing; its
missing D49 `replay_beam_roots` stub was added so it links again). Test input
gains `p` (view key). Not tested in the lab: third person (a crouched Duke
cannot kick there) and a second type 303 crate (the teleport left the lab game
stuck). Executable
`38bdab2ea9e68371ab8e5b130ef52885c5c918c5ebd63058be857be08fe15fc2`. Needs
the user's playtest of slot 9 and another crate of the type.

**Playtest round 1 (2026-11-08, Needs playtest).** User: "this has
essentially fixed it", but crouched after the break, turning made the left
leg pop in and out. Cause: after the crouch landing the eye sat 353-354 above
the pelvis against the crouched-legs limit of 352 (ordinary crouch 225). The
native camera pivot leaves the root around the drop (about 280 above it on
the fall's first update, already on the floor while the D08K9 landing eases
the root down), and the eye's +-64 dead band, measured from that pivot, was
left 128 units high. Any jump landed with Ctrl held did the same. Now the
eye's height is measured from the root on the fall's first update and during
the landing ease (`first_person_anchor`, `crouch_landing_eye_root()`). The
eye descends with the body and settles 220-240 above the pelvis; the leg is
steady while turning (60 and 180 Hz, crate break and jump landing). Input and
aim suites pass. Executable
`d63d61048b3a4aaf298a54ce46aba88cc000ed069f20856d0f504ed68c291957`.

**Accepted (2026-11-08).** User: "thats a winner. lock it in and close it out".

### D08L — Inertial platform edge run-off

**Done - user accepted (2026-09-30): "im happy with that!"** Measured brake removed on large ledges: the
fire-escape platform run-off dropped from ~47 to 16.5 units per frame (original
fall velocity 3058) and lost mouse look; it now keeps stride speed (9172) and
the camera through the fall. Stride spikes (14000) on small drops are capped
below the running jump. See [D08L note](documentation/66-d08l-edge-run-off.md).

**Original report, 2026-09-27 playtest.** Running off platform edges still feels like it
brakes versus Duke3D’s continuous inertial departure. Investigate original gait /
support / edge handlers versus Modernized stride bridging; deliver a bounded
Modernized change only after a failing comparison is recorded. Preserve collision,
accepted furniture drops, jump grace and Vanilla.

**Acceptance:** documented before/after velocity or feel comparison on representative
exterior platforms and apartment furniture edges; no added reach, no double jumps,
no wall clipping; Vanilla unchanged.

### D08L1 - Modern locomotion - allow walking and crouch-walking off ledges

**Done - user-accepted 2026-10-23:** "it's definitely working. i'm happy to accept this lock it in and close it out".
The user confirms the candidate works, closing the remaining gameplay acceptance.
Standing/crouched bed and platform routes, native regressions, Vanilla comparison
and pushable-object edge checks are recorded in [note 209](documentation/209-d08l1-walk-offs.md).
Accepted executable: `8de27e4cae38b5053c5b456c3fc60ef96a2d2a46df4630dbb7309a8c01a63097`.

**Objective:** Duke can walk or crouch-walk naturally off platforms, objects
and ledges without jumping. On losing support, transition naturally to falling
with consistent modern gravity, collision and landing, responsive input and no
artificial stop simply because the ground ends. Include forced crouching under
low ceilings. Preserve jumping, mantling, crouching and Vanilla behavior.

**User reproduction / primary acceptance (current UI slot 12):**

1. Load a private copy of slot 12 and approach the dumpster.
2. Hold RMB and W to push it as far toward the ladder as permitted.
3. Release Grab and mantle onto the dumpster.
4. The overhead clearance forces Duke to remain crouched.
5. Attempt to crouch-walk off its edge: currently Duke cannot leave and is
   effectively trapped. After this job, he must walk off while crouched,
   transition to falling and land safely without jumping or changing Free Push.

**Investigation:** establish whether this restriction comes from the original
ledge/grounded movement handlers or the modern adapter. Trace ledge detection,
crouch movement gates, support loss, low-ceiling clearance, collision and
falling-state transitions; compare original and modern behavior using private
save copies. Existing D08L run-off momentum work does not establish that every
walking/crouched departure is permitted. Related: D08Z/Z3 jumping and landing,
D08K5/K9 crouch coverage/touchdown, D08X/Y mantling.

**Acceptance:** the slot-12 softlock is resolved by crouch-walking off; normal
standing walks and voluntary/forced crouch walks can leave representative
object and level-geometry ledges in first and third person. Falling, gravity,
collision and landing remain consistent, with no ceiling penetration, invisible
edge barrier or loss of responsiveness. Existing jumps, mantles and crouch
clearance remain intact; Vanilla unchanged; user confirms.

**Scope boundary:** this is Duke's own locomotion, not pushable-object travel.
D08T3 is accepted and must retain its collision and support checks. In
particular, slot 9's block must still stop before unsupported ground, and slot
12's dumpster must still stop before the ladder. Do not add object gravity or
relax object ledge safety. The reported softlock is a separate movement issue,
not a Free Push defect. See [note 208](documentation/208-d08t3-manual-push-pull.md).

### D08M — Modern underwater swimming controls (foundation)

**Done — shallow/ledge + Space ascend foundation accepted, 2026-09-28.**

Case study: [55-swim-controls-research.md](documentation/55-swim-controls-research.md).
Shallow Space jump + Space-facing ledge hop closed (revisit only if needed).
Deep: Space↑ and W/S work; E mantle works. Root-bridge baseline binary
`47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc`. Remaining
deep polish (A/D, Ctrl↓, mantle-only exit) → **D08O**. See
[54-swim-redesign.md](documentation/54-swim-redesign.md).

### D08N — Duke3D-style scuba gear item

**Cancelled — out of scope, 2026-09-28.** The user directed that a scuba device
must not be added to this game. Do not reopen, prototype, or treat this as a
follow-on to swimming. TTK has automatic underwater air and no scuba gadget;
that remains the product. Bio Mask stays TTK's gas mask. No auto-equip scuba,
breath extension, `SCUBA GEAR ON` toast, inventory slot, or HRP scuba art.

**Acceptance:** this ID stays cancelled. Underwater air is original and
automatic in both modes. Do not add a scuba item.

### D08O — Deep free-swim polish (strafe, Ctrl dive, mantle-only exit)

**Done — accepted by playtest 2026-09-28** (three rounds; final binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`). The user
confirmed A/D strafe, Ctrl dive + underwater W/A/S/D/Space/Ctrl, mantle-only
exit, the shallow → subway-platform ledge jump (round-3 plain 98 arc) and the
Bio Mask item-switcher icon (`biomask-small.png`) all working. Reopen only on a
reported regression (55 §6 hard rules 7–9).

Playtest gaps on the D08M binary: A/D strafe dead; Ctrl does not descend; Space
can still exit at a ledge. Root causes and fixes (`swim.inc`, `pc_input.cpp`):

1. **A/D strafe** — A/D were injected as D-pad Left/Right, which `8005201c`
   treats as *turn* buttons (anims 71/70); the stick-X word is also turn. The
   strafe branches read the layout's **L2/R2** words (`800d1a68` → 88/91/92,
   `800d1b80` → 89/90/93). Free swim now resolves those pad bits from the
   layout tables (`bit = (ptr−800d1440)/4 − 21·pad − 1`, verified live: 8/9)
   and injects them; stick X is no longer written. Verified in-engine by pad
   injection: L2 → anim 91 sideways-left, R2 → anim 90 sideways-right.
2. **Ctrl descend (round 2, D08O.2)** — the first fix (floor-margin gate on the
   soft-Y dip) still felt dead in playtest. Root cause: once the original hands
   over to its **native swim state machine** (`800455bc`, `player+0x22c` = 4
   surface / 5 underwater, anims 122–142), `80044c18` pins body Y to the
   surface every frame, so no host Y write can dive. Ctrl now calls the
   original dive `80045564(player, player+0x60)` (→ state 5, anim 133, sound
   0x3003). Underwater the host sets body yaw/pitch (`+0x1c/+0x24`,
   `+0x1e/+0x26`, +900 up … −900 down, camera-relative) from WASD / Space /
   Ctrl and injects Square so the original thrust integrator (`8007c788` /
   `800451a0`) moves Duke; it hands back to state 4 by itself when he breaks
   the surface. Free-swim soft-Y remains only for the land-side water anims.
3. **Mantle-only exit** — free-swim `swim_try_mantle` (Space → `8003e2d0(98)`
   at a lip) removed; Space is ascend only.
4. **Wade ledge assist (D08O.3/.4, subway platforms)** — user reported the
   shallow → platform jump still fails often. Lab measurement on the west
   platform (room 21 → 57, ledge 635 above the water floor): the directed jump
   98 rises only ~560, so it landed only from a 700–1250-unit run-up; touching
   the wall it was cancelled on frame one (splash), from 300–600 units it hit
   the wall face (107) and fell back (105) — 4/5 far, 0/6 near. The standing
   probe `800797f4` reaches ~350 units, so `swim_climb_ahead` never saw the
   ledge either. Fix: `swim_ledge_ahead` repeats the standing probe from the
   current position and from 320/640/960/1280 units along the view (world XZ
   written and restored inside the query); a refused step (result 1) with floor
   256–1792 above the feet is a ledge. Space then launches the ordinary
   directed **98** toward it (D08O.4 — the first cut launched vertical 97 and
   switched to 98 at the lip; the user found that hop far too high and asked
   for the plain jump back). The 98 keeps its own arc shape but takes off with
   VY −9000 instead of ≈−6900 (`delayed_vertical` applied at the `8003ebf4`
   ballistic hook, as the apartment-bed fix does; 98 integrates ≈VY/96 per
   tick, so the rise is ≈840 instead of ≈560 — about a head taller, enough for
   the 635 ledge with margin). `swim_ledge_ballistic` then owns XZ each tick:
   within one probe step of the wall (offset 0/320) XZ is held at zero until
   the animated foot (`+0x10`) is 32 above the ledge top, then 3400/4000,
   because 98's first stride into the wall face is exactly what bumped (107) or
   cancelled; further back XZ is 1900 + 3.5/unit (max 6400 ≈ 98's own 6007) from
   takeoff so Duke lands just past the lip. Result on the final code:
   **15/15** lab jumps landed on the platform (`ledge18`; plus 13/15 on the
   previous cut, both misses the near-oblique bump this hold fixes) — at the
   wall, 350–1350 units back, standing or running, Space alone or with W,
   headings up to ±40° off perpendicular. Beyond ~1650 the ledge is out of probe
   reach and the plain 98 falls short (too far to see). At a plain wall (no
   floor ahead) W+Space gives the vertical hop instead of the cancelled splash.
   A stale-assist bug (pending flag surviving a landing) was fixed at the same
   time: the flag now drops whenever anim ≠ 98 or state ≠ 9. Debug counter
   `ledge_assists`; `DNTTK_SWIM_TRACE=1` prints `ttk-ledge probe/launch/clear/
   abort/ballistic` and `ttk-swim` per-tick lines.

Hard rules unchanged (never invent `+0x20c`, never clear entry `+0x1f8`, never
primary world XYZ — the probe offset is a restored query, not a move). Full
contract + checklist in
[55-swim-controls-research.md](documentation/55-swim-controls-research.md) §1.2–§9.

Verified on the final binary: ttk-controls-test + ttk-input-test PASS; shallow
subway water — W/S/A/D camera-relative wading, Space → 98 in open water,
Ctrl → 105; deep zone 35 (teleport) — Ctrl dives (133 → state 5), W/A/S/D
thrust camera-relative (128), look-up + W rises, Ctrl descends, Space thrusts up
and auto-surfaces to 122/state 4, surface W = 125, A = 123; west platform ledge
15/15 as above. Playtest accepted the swimming (A/D strafe, Ctrl dive, underwater
WASD/Space/Ctrl, mantle exit) and the Bio Mask icon; playtest 3 accepted the round-3 ledge jump. Binary
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`.

**Acceptance:** A/D strafe; Ctrl dives and Duke swims underwater where he looks
(W/A/S/D), Space ascends/surfaces; free-swim Space = ascend only; exit via
mantle/E; shallow → subway platform jump lands reliably from the wall or a
run-up without a distance puzzle; no regression on D08M shallow/oxygen; Vanilla
unchanged. **All accepted 2026-09-28.**

### D08O1 - Fire weapons while swimming (Modernized, game-wide)

**Accepted (2026-10-06).** User: "i completely accept that this works!
mechanically, it does exactly what it's meant to, but the only issue is the
animation" - the weapon points downwards while Duke swims and fires; that is
D08O2. Underwater the original tests fire before
the swim thrust, so held fire stopped Duke (idle 127); Modernized now hides
fire from the swim handler only while a swim direction is held, so he keeps
swimming and the weapon keeps firing. Swim shots use view aiming and the
crosshair; weapon keys work in water, limited to the weapons the original
allows there. Vanilla unchanged. Level 6 verified live; the user's location
and a second level remain. [Implementation and evidence](documentation/107-d08o1-swim-fire.md).

User request, 2026-10-05: "Duke cannot shoot and swim at the same
time so that needs to be addressed just like we did with the jet pack work."
Reproduction: UI slot 2 (savestate file 01, SHA-256
`756b0e158f744157fb4a4a3a4c9eca9aa357a905166a112c7a420ba4af7cfe59`), first
medieval level. Test only on dated private copies; verify the hash first.

**Scope:** in Modernized, Duke can fire his weapon while swimming (surface and
deep free-swim, D08M/D08O), aimed at the view with the enabled crosshair, the
way D08Q3 made jetpack flight a shooting state. First establish what the
original allows in each water state (surface, shallow, deep), which weapons it
permits underwater, and how the weapon is stowed/drawn on entering and leaving
water; separate missing Modernized eligibility (aim/reticle/lease guards that
exclude swim modes 4/5) from an original restriction. Where the original
forbids firing, propose the modern behaviour (and any weapons that should stay
unusable) to the user before changing rules. Reuse the D08Q3 approach: swim
eligibility in the weapon-aim and reticle paths, weapon presentation, beam and
projectile completion, transitions into and out of water. Work with D11E
(first person while swimming).

**Acceptance:** in Modernized, Duke fires the agreed weapons while swimming in
at least two levels, aimed at the view with the crosshair, in third person (and
first person once D11E exists); entering/leaving water, holster and weapon
switching stay clean; the jetpack and ground aiming are unchanged; Vanilla
unchanged; the user confirms.

### D08O2 - Weapon points forward while swimming and firing in motion

**Accepted as v1 (2026-10-07)** (user: "it works and i accept this as a v1"; the more natural pose is D08O2A) ([note 118](documentation/118-d08o2-swim-weapon-forward.md)). **User request, 2026-10-06.** After accepting D08O1: "make the weapon
point forward when in use when swimming and in motion, as it currently points
downwards." While Duke swims (underwater thrust 128-130, and check the surface
strokes 123-126) and fires, the firing upper-body animation plays over the
stroke and the weapon points down; shots already go to the crosshair.

**Scope (Modernized):** the weapon and arms point forward along the view (the
crosshair) while swimming and firing in motion. Research first: how the swim
stroke and the upper-body weapon track combine (upper anims 9/10 pistol,
24-28 shotgun/rifle/crossbow, 29/33/34 Gatling, 39/41 pipe bomb), whether the
D07/D08Q3 presentation hook (`0x80097a44`, upper-body aim direction) runs in
the swim states and why it does not lift the arms there, the muzzle position
measured in D08O1 (about 420 units lower during the stroke), and the body
facing (the original thrust moves along body yaw/pitch, so Duke faces the
swim direction, not the view). Prefer original animation data and the
existing aim path over new art. Note 107 has the D08O1 evidence and lab.

**Acceptance:** in Modernized, while swimming in each direction and firing
each underwater weapon, the weapon visibly points toward the crosshair in
third person; idle underwater firing, surface swimming, ground, jetpack and
Vanilla unchanged; swim-and-fire mechanics from D08O1 unchanged; the user
confirms.

### D08O2A - Natural swim-fire pose v2 (arms raised, head up, torso in the stroke)

**Accepted (2026-10-07)** (user: "this is actually rock solid ... I'm happy with it and its 100% playable"; the last-shot-down issue from the capture was fixed in a follow-up the same day) ([note 119](documentation/119-d08o2a-natural-swim-fire-pose.md)).
Only the shoulders now take the floating-fire arm pose (aimed at the
crosshair) while the chest, head and legs keep the original stroke; v1's
chest turn is gone. The head stays original per the user ("his head is always
facing in the right direction anyway"). **User request, 2026-10-07**, on accepting D08O2 v1: "i want v2 in the
backlog though, which will be a more natural pose rather than his entire
torso standing up, so it will involve moving the arms in the position as if
firing up and his head looking up."

**Scope (Modernized):** while Duke swims underwater and fires, his torso and
legs keep the swim stroke (body pitched along the swim direction); only the
arms come up into a firing position toward the crosshair and the head turns
to look along the view. Replaces the v1 whole-upper-body turn.

**Research start (from D08O2, [note 118](documentation/118-d08o2-swim-weapon-forward.md)):**
v1 sets `+0x224 |= 0x100` so the original builder `0x80097c04` aims joint
`[model+0x2d]` (joint 1, which carries both arms, the head and the torso)
and `[model+0x34]` (joint 9, neck/head) through `0x80097a44`. `0x200000`
aims joint 9 only and leaves the arms in the stroke. Arm joints are 3-8
(joint 7 is the gun hand, 4 the other hand; 5/8 measured near the
shoulders). The builder also has a mode 3/6/7 path that aims
`[model+0x31]` with `0x800978f8` for upper anims 8/11. Candidates: aim the
shoulder/upper-arm joints (and joint 9 for the head) instead of joint 1,
for example by calling the aim helper for those joints from a hook on
`0x80097b90` (child composition) or by a dedicated joint-matrix pass after
the build, keeping the original firing upper animation. Prefer original
animation data and the existing aim path; check the first-person weapon
joints (D12) and the replayed frames at high refresh.

**Acceptance:** in Modernized, while swimming in each direction and firing
each underwater weapon, the torso and legs stay in the stroke, the arms and
weapon point toward the crosshair and the head looks along the view, in
third person; floating fire, surface swimming, ground, jetpack and Vanilla
unchanged; D08O1 mechanics unchanged; the user confirms.

### D08O2B - Swim-fire aims down again (regression)

**Done (user-accepted 2026-10-18)** (user: "it certainly is working, duke shoots in the right direction") ([note 146](documentation/146-d08o2b-swim-fire-costumes.md)).
Cause: the D08O2A shoulder hook picked joint records 2 and 5, which are the
shoulders only on the level 6 model; the level 2 sewer model (slot 3) orders
its joints differently, so a leg joint was turned and the arms stayed in the
stroke. The shoulders and chest are now found by part through the model's own
table (shared `duke_model.inc`, as first person does). Slot 3 copy, dive + W +
fire: gun hand 85 -> 221; level 6 matches the D08O2A table; Vanilla unchanged;
suites pass. Build `93093366872c16d100b05746cfc7d6c7745b0560eadecad459b474d2971bae25`.

**Originally queued 2026-10-09.** User report: on the level in player UI slot 3,
while swimming and firing the weapon points down again (the pre-D08O2 /
pre-D08O2A failure). UI slot 2 on the same baseline still aims forward while
swimming, so the accepted D08O2A pose is not gone everywhere. Reproduce only
from dated private copies of the player's slots; do not use the live player
card.

**Scope (Modernized):** restore view-aimed swim-fire presentation wherever
slot 3 fails, without breaking slot 2 or the accepted D08O2A pose (torso in
the stroke, arms to the crosshair). Start from the D08O2A path in
[note 119](documentation/119-d08o2a-natural-swim-fire-pose.md) (`swim_aim_begin`
/ `swim_aim_arm`, single-shot and surface release-tail rules). Compare slot 3
vs slot 2: water mode, upper animation, lease, weapon, fire held vs tap, and
whether the arm hook arms. Prefer a systemic fix over a slot-specific patch.
Vanilla unchanged.

**Acceptance:** in Modernized, swimming and firing in the slot 3 repro aims
at the crosshair as in slot 2; single shots and held fire stay forward through
raise / fire / lower; slot 2 and other accepted swim-fire cases unchanged;
Vanilla unchanged; the user confirms.

### D08O3 - E in water keeps the weapon

**Accepted (2026-10-07)** (user: "fully passed my playtest. feels great to play. i accept this job now as complete!!") ([note 120](documentation/120-d08o3-e-in-water.md)).
**User request, 2026-10-07:** "pressing E while swimming holstered my weapon
and i had to press one of the numbers to draw it. I dont want that to happen."
The original climbs out of water only with the weapon stowed, so E keeps
stowing first; the user chose "redraw if no climb" over never stowing (needs
the side-effecting ledge probe) or ignoring E in water.

**Acceptance:** in Modernized, E while swimming or floating with a weapon out
and no ledge to climb leaves Duke armed again within about half a second,
without stopping his swimming; E at a ledge still climbs out and draws on
land; E on land and Vanilla unchanged; the user confirms.

### D08O4 - Brief Ctrl dive keeps swimming forward after release

**Todo (queued 2026-10-18).** User report while accepting D08O2B: "when
loading up the save slot 3, when holding ctrl briefly to dive, upon letting
go, duke swims slightly forward including playing his forwards swim anim."

**Lab evidence (D08O2B lab, slot 3 copy = file 02, level 2 sewer pool,
build `93093366...`):** from the surface (mode 4, anim 122), Ctrl starts the
dive (mode 5, anim 133). Held about 15 frames: 133 keeps playing for about 15
more updates after release and Duke travels about 850 units horizontally (and
about 400 down) before idle 127. Held about 40 frames: 133 hands over to the
forward stroke 128 while Ctrl is still held, then 127 on release. Script
`recomp/analysis/d08o2b-20261018/dive.py`, log `dive.log`.

**Scope (Modernized):** Ctrl dive is a vertical control. Releasing it should
stop the descent promptly without carrying Duke forward or playing the forward
stroke when no movement key is held. Check whether the original dive (133) is a
fixed-length forward-pitched thrust and how the D08O surface Ctrl dive enters
it; prefer ending or replacing the thrust at the swim lease over masking
positions. Related: D08AA (speed unification).

**Acceptance:** in Modernized, tapping or holding Ctrl from the surface or
underwater dives straight down while held; on release Duke stops (no forward
drift, no forward stroke) unless W/A/S/D is held; diving with W still goes
forward; surfacing, mantle-only exit, swim-fire (D08O1/D08O2A/D08O2B) and
Vanilla unchanged; the user confirms.

**Needs playtest (2026-10-19).** Cause: the surface dive is the original
anim 133, a fixed thrust of about 30 updates whose root motion follows body
pitch and, unlike thrust 128, ignores Square. While Ctrl is held the host
pitches Duke -900 (straight down); on release `swim_steer_underwater` puts
body pitch back to the view, so the rest of 133 carried Duke along the view
(the "forward stroke"). Fix (`swim.inc`, `swim_end_dive`): at the early swim
update, before the original handler, a 133 with no W/A/S/D/Space/Ctrl held
ends the way its own handler `0x80046814` ends it - anim 127, track 0 (its
`+0x264` sound is unset during 133, measured). Counter `swim_dive_stops`.

Evidence (slot 3 copy = file 02, private cards, lab
`recomp/analysis/d08o4-20261019/`): horizontal drift after release, before
-> after: 3-update tap 1474 -> 9 units, 15-update hold 796 -> 10, 40-update
hold 55 -> 55 (already thrust 128). A 1-update tap returns to surface idle
122 through the original. Regression (`reg.py`): underwater Ctrl descends and
stops (0 drift), W thrust and W + fire swim forward (33 fire masks, 68
shoulder aims, 5 shots), Space surfaces to 122, Ctrl+W dives forward while
held; Vanilla Square tap still plays the full original 133 (1301 units, no
host stop). Suites: ttk-controls-test (LEVEL00 fixture, LEVEL01, all levels),
ttk-input-test, ttk-aim-test, ttk-near-test, ttk-inventory-test,
ttk-font-test, Python unittest (146, 2 skipped) pass. Executable
`3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d`. Not yet verified: the user's feel in play, mantle exit after a short
dive (code path untouched).

**Done (user-accepted 2026-10-19).** User: "this is awesome. fully accepted!".
Executable `3fe77432bdd35d84b0236bbe98dd19ee1d9a31300a1bb0362204cab2f7f4d42d` is the regression baseline.

### D08O5 - Left click draws a holstered weapon while swimming

**Status: Done (user-accepted 2026-11-01). Dependencies: D08O1, D08O3.** User report (2026-10-30, after
the D17AR2 playtest): "if you are swimming with the weapon holstered, left
click doesnt draw your weapon like it does on land. It needs to draw the
weapon." Expected: Modernized, game-wide, left click (fire) while swimming
with the weapon holstered draws the current weapon exactly as on land (D08E
responsive holstered fire); D08Q7 fixed the same gap for jetpack flight.
Vanilla unchanged. Reproduce with the player's real input in water (a
private copy of a swimming save), find where the swim states skip the
ground holstered-fire draw path, and verify draw, then fire, at surface and
underwater. Asked again in the 2026-10-31 playtest: "when you go into the
water I want to be able to draw the weapon when you start firing even if your
weapon is lowered".

Work log 2026-11-01: baseline reproduced in the slot-2 moat (holstered, held
fire underwater and on the surface: equipment 0, no shots). Fire now draws in
the swim states through the original weapon request (D08O3's path, not a
Circle tap that could end a stroke); a refused weapon takes the original swim
remap; unarmed Cross is held off while stowed so fire never climbs out. Live
60/third and 120/first: draw then fire, click draws only, stroke/strafe/dive
and surface swimming keep moving, four weapons; land, jetpack and Vanilla
unchanged; native controls/input/aim pass. Limits: no live ledge in the moat,
level 6 only. [Note 345](documentation/345-d08o5-swim-fire-draw.md).

Accepted 2026-11-01. User: "very good work!! i can confirm it works."
Closeout, commit and push authorized. Follow-up: holstered swimming arms
arms (disembodied in first person) recorded as D14G after intake research.

### D08P — Crystal-2 turret / scripted-camera control recovery

**Done — 2026-09-29 (iteration 6, user playtest: "it really works. perfectly").** The user showed it is the
whole flooded-corridor ledge area (**F7 UI slot 3**), not depth: run, mouse
and jump die and never return. Their session log had `ident=0` from the
ledge onward; a live per-guard diff found the cause — the LEVEL00 zone
script writes a hit position into the overlay's trailing 16-byte scratch
vector (`0x800ccf1c`), which the 9668-byte overlay identity guard covered.
The guard is now the 9652-byte code/table body. In the same water (depth
512) the original swaps all gaits for wade clips 80/81 at 8–14 units/frame,
forward only; their handler `0x800539f8` is now hooked (new generated
entry, three new guards) and its root retargeted to camera-relative WASD in
the land run band: 45–58 units/frame, mouse steering, strafe, Space jump,
0 identity refusals across and past the ledge; slot 2 regression clean.
Wading straight into a wall idles as in the original (turn to resume).
Iterations 4–5 (waist-deep land lease, held keys across recapture, fresh
capture offers) stand.
See [56-scripted-camera-controls.md](documentation/56-scripted-camera-controls.md).

**Acceptance:** F7 slot 3 ledge: run forward along and past it with run,
mouse and jump intact, mid-depth water at run pace with mouse steering;
F7 slot 2 wade uses dry-ground mouse look, camera-relative
WASD and run speed; leaving the water keeps mouse and Shift-run even when
Shift was held through the load/pause; the pause menu is not captured;
Escape / F7 recapture still work; true deep swim and Vanilla unchanged. If
it still degrades, report the on-screen reason and the session log.

### D08Q — Modern jetpack flight controls

**Done - 2026-09-29 (user-accepted):** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful."

User report: "when using the jetpack i
cannot control it at all aside from spacebar to ascend. wasd are blocked,
modern controls fail here completely." Confirmed on `79a3fd5b…` in an
isolated probe: original mode 10 (anims 163–170) was outside every lease,
so the mouse was dead, WASD only reached the original through the tank
fallback (Up/Down/L2/R2 relative to a body heading the player could not
turn) and the original's idle gravity sank Duke to a landing within a
second or two. Ctrl meant nothing.

Implemented as a host layer over the unchanged original handler
(`recomp/src/ttk/jetpack.inc`, camera-only lease in `state()`, pad bridge in
`input_pad()`, J gate in `select_weapon`): mouse orbit with the body facing
the view (`face_view`), so the original's body-relative thrust is
camera-relative — W/S → original Up/Down, A/D → the layout's strafe pads
(L2/R2); Space stays the original Square climb; **idle hovers** (host sets
the original hover lock `+0x224 & 0x08000000` at the current height; the
original drains fuel while hovering too); **Ctrl descends** at a steady
~48 units/update (hover lock released, ballistic `+0x1f8 = 2800`); WASD
flight releases the lock (under it the original only moves Duke by the raw
thrust vector, ~10 units/update) and pre-loads `+0x1e8 = 18` against the
handler's directional lift so flight stays level; J in flight is the
original inventory toggle → 108 fall, and the mouse camera survives that
fall until the landing. Three new code guards (`8004aaf8`, `8004ac08`,
`8004ade0`). No new generated hook. Vanilla untouched (all writes gated on
Modernized + capture + lease + identity).

Isolated Xvfb probe (level 1 turret room and ledge states, `dninventory`):
mouse turns body and camera together (`looks` 0→1); W/S/A/D headings
−33/154/−120/61° against camera −30° (expected −30/150/−120/60); level
flight ±25 units over 55 frames; W after a 100° mouse turn follows the new
view; speeds 25/29/24/14 units per frame (R2 strafe is the slower original
side); hover drift ≤ 32 over 60 frames; Ctrl 489 units in 30 frames, then
landing anim 105; Space climb 540–850 in 30 frames; J off → 108 fall with
working mouse → 105 → ground run at 45.7 with the land lease.
ttk-input-test (new jetpack pad case), ttk-aim-test, ttk-controls-test
PASS. Binary `481dd2ec…`; current player build (with the D23A frame-budget
fix) `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

Known/unverified: releasing WASD stops almost at once (hover engages) —
may feel abrupt; after Ctrl release Duke settles another ~130–300 units
before holding (the original's floor-approach in `8004ac08` pulls toward
~0x200 above a nearby floor); strafe is the original's half gain; fuel
drains while hovering (original rule); no probe of low ceilings, water
below, damage or death in flight. See
[57-jetpack-controls.md](documentation/57-jetpack-controls.md).

**Acceptance:** with the jetpack on (J), Space lifts off; in the air the
mouse turns the camera and Duke, WASD flies relative to the camera at a
useful speed and level height, releasing everything hovers, Ctrl descends
until a soft landing, Space climbs, J switches off in the air with a
controlled fall and the mouse still live, and the ground controls resume on
landing; fuel-out behaves the same as J; Vanilla flight unchanged.

### D08Q1 - Faster Modern jetpack Ctrl descent (underwater dive speed)

**Done on user acceptance (2026-09-29):** "verified working!!!"

`k_jet_descend_velocity` 2800 -> 5500: steady
Modern Ctrl descent 29.5 units/frame (was ~16), matching the underwater dive;
soft landings, no health loss. See
[D08Q1 section](documentation/57-jetpack-controls.md#d08q1---faster-modern-ctrl-descent).

User (2026-09-29): "for jetpack modern, i'd like ctrl to
descend be faster. basically the speed of ctrl in water to descend".

Scope: Modern scheme only; raise the host Ctrl descent velocity
(`k_jet_descend_velocity`, `jetpack.inc`) so steady descent matches the
underwater Ctrl dive (~29 units/frame measured in the D08O deep-water lab,
`/tmp/swimlab/deep4.log`: y -1438 -> -888 over 19 frames). Classic, Vanilla,
hover, WASD flight and Space unchanged.

**Acceptance:** Modern Ctrl descent measured near the underwater dive rate;
landing still ends in the original soft landing without extra damage; Classic
and the other Modern inputs unchanged; user confirms the feel.

### D08Q2 - Jetpack unavailable after death, Continue and dnstuff

**Done (2026-10-04, user-accepted):** "confirmed fixed! accepted."

The user died, chose
Continue, returned to gameplay, entered `dnstuff`, and then could not use the
jetpack. The inventory picker also skips the jetpack, preventing selection.
The user saved this failing state in **UI slot 10 (file 09)**. Investigation and
implementation were subsequently selected and authorized on 2026-10-04.

**Cause and repair:** death during deployment can strand the original jetpack
pending bit across Continue; the original inventory grant preserves it. The
reported save has full fuel and a closed/off pack with that bit still set.
The bounded Modernized repair clears only pending at that inactive endpoint.
Slot 10, a controlled interrupted-deployment/death/Continue route, both flight
schemes, fuel restrictions and save/load pass. The user confirms the repair
and accepts this job. A natural map-pickup replay was not performed in the
automated work; acceptance does not add that evidence. See [evidence and limits](documentation/90-d08q2-jetpack-continue.md).

**Reproduction/evidence:** private snapshot
`recomp/analysis/d08q2-jetpack-20261004/cards-reported`, with a SHA-256/mtime
manifest alongside it. Slot 10 SHA-256: `161253aa41f4f2c407f57ef7cff1371ba7889c4bdeba793183f9045b6184e18e`.
The snapshot preserves the resulting state, not necessarily the preceding
death/Continue transition. Reproduce that sequence separately on private
cards/settings, comparing a working jetpack state, before/after Continue,
and before/after `dnstuff`. Never write the player's cards or saved state.

**Investigation:** trace inventory ownership/quantity/fuel, selected item,
picker eligibility and activation gates; examine Continue/reset and cheat
grant paths. Check whether the cheat is acknowledged, whether its changes
persist, and whether direct activation and cycling disagree. Distinguish
retail restrictions from Modernized bridge state and save/load effects.
Keep modern/classic flight controls and the accepted renderer unchanged.

**Acceptance:** identify the responsible state/path and make a bounded repair
if justified. Verify slot 10 plus a fresh death/Continue/`dnstuff` sequence,
normal acquisition, picker selection, activation/deactivation, fuel handling
and save/load. Preserve legitimate empty/unavailable-item restrictions and
Vanilla behavior; confirm working Modern and Classic jetpack controls. User
confirms restored use.

### D08Q3 - Jetpack weapon aiming and missing crosshair

**Done (user-accepted 2026-10-04):** "i accept this! great work."
Guarded flight now admits view aiming, weapon presentation, beam completion
and the enabled crosshair. Both flight schemes pass private six-weapon firing
and transition checks; native suites and build pass. Accepted implementation:
`f284dc1`. Recorded live coverage limits remain documented. [Implementation and evidence](documentation/91-d08q3-jetpack-aim.md).

Original report after accepting D08Q2:
"i cant aim to shoot when using the jetpack / crosshair is also gone."
The private slot-10 intake now reproduces the baseline failure and verifies the candidate.

**Scope:** restore usable Modernized weapon aiming during jetpack flight and
show the enabled crosshair. Inspect flight-state eligibility in the weapon-aim
and reticle paths, original airborne weapon restrictions, and transitions into
and out of flight. Diagnose aiming and crosshair visibility separately; do not
assume that showing a reticle alone restores shots aimed at the view.

**Intake evidence (superseded by the verification above):** user report only;
cause and exact reproduction were initially unknown. D08Q2's slot-10 snapshot
was initially available for regression testing only.
Use private cards/settings; verify active overlays before changing hooks.

**Acceptance:** while using either Modern or Classic jetpack controls, the
player can aim and fire supported weapons with shots following the established
Modernized view-aim behavior. The enabled crosshair remains visible and meaningful
in flight; the disabled setting stays respected. Verify hovering, moving,
ascending/descending, mouse yaw/pitch, on/off, landing and fuel-out, with
representative hitscan and projectile weapons. Distinguish intentional original
weapon restrictions from defects and document any remaining unsupported cases.
Preserve flight controls, existing first-person/third-person flight transitions,
ground aiming, D08Q2's accepted recovery and Vanilla behavior. User confirms
restored aiming and crosshair. Do not reopen D08Q2 for this separate report.

### D08Q4 - Duke3D-style `dnkroz`: health to 100 and unlimited jetpack fuel

**Follow-up 2026-10-06 (needs playtest):** user: "it should also give the
jetpack too". While god mode is on, Duke has the jetpack at full fuel
([note 38](documentation/38-debug-cheats.md)).

**Accepted by the user, 2026-10-06** ("accepted!!"). Implemented; verified live in Modern and
Classic flight on private copies; see the work log and
[note 38](documentation/38-debug-cheats.md). Atomic Health above 100 is the
original pickup and was not driven live.

**User request, 2026-10-06.** "dnkroz should also allow for unlimited
jetpack, that's just the behavior in eduke also." While `dnkroz` /
`dncornholio` god mode is on (D08G), jetpack fuel does not drain in every
jetpack scheme; switching god mode off resumes normal drain from the current
amount. `dnkroz` stays god mode; unlimited jetpack is an addition, not a
replacement (user, 2026-10-06: "dnkroz is still god mode, but in duke3d it
also gave unlimited jetpack"). User, 2026-10-06, on Duke3D's `dnkroz`:
"health just goes to 100 and jetpack fuel stays on full too. duke can
actually receive more health but only by taking an atomic health." So turning
god mode on sets health to 100 (today's `dnkroz` does not restore health,
GAME_MANUAL) and keeps jetpack fuel full; health above 100 comes only from
TTK's equivalent of the Atomic Health pickup, up to the real maximum of 200
(user, 2026-10-06: "in which case it reaches the real health max of 200");
identify which original item plays that role and how TTK stores the cap. Check the Duke3D/EDuke32 source to confirm
these details and match them. Lead: the original mode-10 drain is
skipped when `800c3cc4` is set (note 57). The D08Q3 lab
(`analysis/d08q3-jetpack-aim/transitions.py`) cleared `800c3cc4` to "pause the
fuel cheat", so it is probably the original's own unlimited-fuel flag:
confirm what sets it before adding a host override. Also cover the D08Q2 dnstuff/Continue paths.

**Acceptance:** turning god mode on sets health to 100 and fills the jetpack;
with god mode on, flight in each scheme lasts indefinitely and the HUD % does
not fall; health goes above 100 only through the Atomic Health equivalent,
up to 200; with it off, fuel drains as before; no change without
the cheat; Vanilla unchanged unless the cheats already apply there.

### D08Q5 - Weapon switching while flying the jetpack

**Done (user-accepted 2026-10-06):** "excellent!! i accept."
In Modernized flight the weapon shortcuts
(1-0, wheel, semicolon/apostrophe, X) now reach the original weapon request;
the original mode-10 state completes it with the normal upper-body
holster/draw while flight continues. Verified live in Modern and Classic
flight with every weapon, firing after each switch; ground, swim and Vanilla
unchanged; suites pass.
[Implementation and evidence](documentation/108-d08q5-jetpack-weapon-switch.md).

**User request, 2026-10-06.** "allow for weapon switching while using
jetpack, both wheel and numbers." In Modernized flight (mode 10, every
jetpack scheme: Modern, Classic and the future EDuke32 scheme D08S), the
number keys 1-0 and the mouse wheel (plus semicolon/apostrophe and X, the
D08A shortcuts) change weapons as they do on the ground. Today D08A
selectors reject requests during flight.

Research first: whether the original allows weapon changes in mode 10 at all
(draw/holster animations on the upper body during anim 163/165 and the
lift-off/landing 106/108), which weapons the original permits while flying,
how the D08Q3 flight aim and crosshair follow a newly drawn weapon, and
whether a holstered or empty pack state matters. Use the original
equipment/draw path; do not swap models directly. Keep D08A groups, burst
resolution and the underwater rules (D08O1) unchanged.

**Acceptance:** while flying in each scheme, number keys and the wheel cycle
through owned weapons with the normal draw, and the new weapon fires with
flight aiming and the crosshair; flight, fuel and landing are unaffected;
ground and swimming switching unchanged; Vanilla unchanged; the user
confirms.

### D08Q6 - Modern jetpack altitude creeps upward in level flight

**Accepted by the user, 2026-10-06** ("i accept this as fixed! the test passes
and this feels great"), on revision 5b (binary `2a49b29acbd0de49...`).
Revision 5: Modern flight model rewritten.
The real cause of "cannot move with WASD unless ascending or descending" was
the original `8004ac08`, which drops horizontal motion whenever the
vertical root is 0. A precisely held height made that common. Modern now owns
the flight velocity at the original's speeds (16000 horizontal, -11800
climb). The original still gets its flight pads (flame, lean poses, fuel), and
their thrust is cancelled from the post-handler reading. Height is held with
the floor cap, and a corrected +-2 vertical step keeps horizontal moves
applied. History follows.

Revision 1: Modern flight keeps one held altitude,
captured when Space or Ctrl ends. Level flight steers back to it, hover pins to
it without the original bob, and it is capped at the original floor approach
height over higher ground. A lab circuit of about 2400 updates ends within
9 units of the start, compared with about 460 units of climb in six legs
before. Revision 2 (after the user found it jerky) parks the bob at its flat
sine peak instead of phase 0 and slews the base gently, so hover is flat and
stops settle over 3 updates. See
[note 109](documentation/109-d08q6-jetpack-altitude-hold.md).

**User report, 2026-10-06.** "duke keeps gaining height when using the
jetpack in modern mode. how i tested this was using dnkroz, and elevating to
a particular height, and in the first level i just kept circling round the
apartment building and through the alleyway and noticed duke was getting
higher and higher. i was also trying to look up quite a lot and i think that
had something to do with it. anyway, duke's elevation should just be fixed
when jetpacking unless the player uses ctrl or space"

**Scope:** in the Modern jetpack scheme (D08Q), Duke's altitude must stay
fixed during level flight and hover; only Space (climb) and Ctrl (descend)
change it. Classic (D08R) keeps its original burst/gravity physics and is out
of scope; the future D08S scheme should share whatever fix is made for Modern.

Research first: reproduce the climb on a private LEVEL01 copy with `dnkroz`
(long circuits around the apartment building and alley, with and without
pitching the view up, mouse turning, W/A/S/D combinations and diagonal
input). Measure Y per update against view pitch, yaw rate and held
directions. Candidates to test, not conclusions: the fixed
`k_jet_level_trim` (+0x1e8 = 18) only cancels the measured single-direction
lift, so diagonal input, view pitch or the D08Q3 view-aim presentation (body
lean +0x87c..+0x880 or pitch reaching the mode-10 thrust rotation) may leave
a residual climb; a release/re-engage of the hover pin at a drifted height
would also ratchet upward. Prefer holding the altitude captured when vertical
input ends over tuning a constant.

**Acceptance:** in Modern flight Duke's height stays fixed while hovering,
moving in any direction, turning and looking up or down for long circuits
(including the user's LEVEL01 apartment/alley route); Space climbs and Ctrl
descends as before, and releasing them holds the new height; landing,
fuel, J off, firing and D08Q5 weapon switching unchanged; Classic and Vanilla
unchanged; the user confirms.

### D08Q7 - Left click does not draw a holstered weapon in flight (bug 1)

**Status:** Done. **Dependencies:** D08Q3, D08Q5 (Done).

**Report:** Start jetpacking with the weapon holstered; left click does not
draw the weapon.

**Acceptance:** Left click draws an available weapon during flight after
holstered entry, with the expected fire behavior and no extra holster toggle.
Verify flight entry/exit, aiming and weapon switching across supported jetpack
schemes, without bypassing native weapon restrictions.

**Accepted (2026-10-25):** Settled flight now uses the existing fire-to-draw
Circle tap. Baseline reproduced; player build, controls/input/aim suites and
private Modern/Classic/EDuke32 routes pass. User accepted gameplay and authorized closeout.
[Note 249](documentation/249-d08q7-flight-fire-draw.md).

### D08Q8 - Jetpack clearance follows floor/ceiling smoothly (bug 4)

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D08Q, D08S (Done).

**Report:** In the tight space at current slot 12, jetpacking Duke treats a
descending ceiling or rising floor like a brick wall. The desired behavior
is smooth movement with the floor/ceiling, as in Duke3D. User reference:
`jpacking-stairs.mp4` in `screencaps` (not inspected during backlog intake).

**Acceptance:** Use a private slot-12 copy and the reference video to establish
the intended clearance response, then allow smooth vertical adjustment along
the rising floor/descending ceiling where Duke fits. Preserve solid-wall and
insufficient-clearance collision, flight controls and fuel behavior; verify
both travel directions and supported jetpack schemes. Include the user's
2026-10-25 follow-up: eliminate repeated head-contact judder at maximum
flight height and apply stable contact behavior across flight situations.

**Closeout:** User accepted ordinary-flight surface following and explicitly
requested lock-in and closeout. Modern/EDuke32 follow confirmed floor/ceiling
slopes without held Space/Ctrl, release at edges, and retain solid-wall and
narrow-gap collision. Classic retains native thrust/landing; Vanilla is
unchanged. [Note 250](documentation/250-d08q8-flight-clearance.md) records
verification, revisions and bounded coverage. No next job started.

### D08Q10 - Start the jetpack in water; the water surface is a floor while flying

**Status: Done (user-accepted 2026-11-02; revision 10, note 357). Dependencies: D08S, D08Q8, D08O.** Fixture: the player's UI
slot 12 (saved on the water surface); test on private copies only. The user
wants EDuke32 / Duke 3D behavior: press J while swimming on the surface, or
underwater, and the jetpack starts and Duke flies up and out of the water.
While the jetpack is on, Duke cannot descend below the water surface (Ctrl /
descent stops at the surface and he hovers there); turning the jetpack off
over water drops him back in as now. Reference recording:
`research/screencaps/water-jetpack.mp4` (EDuke32, 54 s: off underwater, J
on, rises out, hovers and flies over the water, never sinks back in).

Initial research: J currently refuses in water by design.
`recomp/src/ttk/shortcuts.inc` ("Jetpack cannot be switched in water; the
normal-state lease excludes it") returns when the player's height (`player+8`)
is below the water level (`player+0x834`, `0x40000000` = no water), and the
D08S instant-jetpack lease only runs in the normal state
([note 219](documentation/219-d08s-instant-jetpack.md)). D08S's checks note
that flying down into water today enters the native swim state (slot 9); that
becomes a surface floor for this job. D08Q8 owns flight clearance (floors,
ceilings, maximum height, [note 250](documentation/250-d08q8-flight-clearance.md)):
the surface floor belongs in the same clearance, game-wide, not per level.
Work: a purpose-built swim/underwater -> jetpack transition (state, animation,
first and third person, weapon), the surface floor while flying, fuel and
`dnkroz` unchanged, no snapping at the surface at 120 / 180 Hz. Vanilla
unchanged.

**Acceptance:** from slot 12 on the surface and from underwater, J starts the
jetpack and Duke rises out; with the jetpack on he cannot go below the
surface anywhere water exists; turning it off over water swims as now;
first person, third person and both jetpack schemes; the user's playtest
matches the EDuke32 feel.

### D08Q11 - Jetpack lifts off directly from toggle-crouch

**Status: Done (user-accepted 2026-11-02; [note 364](documentation/364-d08q11-crouch-jetpack-start.md)). Dependencies: D08Q (jetpack), D08S (instant jetpack), D08K
(crouch, C toggle).** Priority medium. Independent of the console jobs.

Observed (user): toggle-crouched with C, starting the jetpack does not lift
Duke; he lifts off only after a further Space. From standing, the jetpack
lifts off at once. Expected: the same direct lift-off from toggle-crouch.

Known from the code (to confirm): the Modernized start in
`recomp/src/ttk/jetpack.inc` accepts ground mode 0, airborne 9 and swim 4/5,
and sets the start lift (`jet_lift_active`, `k_jet_start_lift`) only for
mode 0 or a water-surface start. Crouch is a ground stance; whether the
crouched player is in mode 0 or another state, and whether the start is
refused or opened without the lift, is unverified. Separately,
`crouch_toggle_context()` (`recomp/src/ttk/pc_input.cpp`) drops `crouch_latched`
once `jetpack_input_ready()` is true, and the crouch hull (D08K2) may hold Duke
down or refuse the standing height on the first flight frame.

Investigation: trace J while toggle-crouched (mode `+0x22c`, animation,
`+0x358`, `jet_lift_active`, crouch latch) in first and third person; compare
with a standing start; check low ceilings (a crouched start under a low ceiling
must not push Duke into it: lift as far as clearance allows).

Acceptance (private save copies, first and third person):
1. C to crouch, then J: Duke stands up into the normal lift-off and hovers
   at the ground start height, with no Space press.
2. Ascent, descent, altitude hold and landing are unchanged (D08Q6/D08Q8);
   after landing Duke is standing (the latch stays off) and walks normally.
3. Ctrl-held (momentary) crouch then J behaves the same.
4. Under a low ceiling the start lifts only as far as the clearance allows.
5. Crouch, crouch-walk, crouch-jump and jetpack controls show no regressions;
   `ttk-controls-test` passes; Vanilla unchanged.

### D08R - Selectable jetpack scheme: Modern / Classic (WASD)

**Done on user acceptance (2026-09-29):** "it's absolutely rock solid."

**Revision 2 (2026-09-29).** `run.py --jetpack classic|modern`
persists a Modernized `jetpack` control (schema 11, default `modern`). Per user
direction Classic keeps every modern control (mouse facing, camera-relative
WASD, J) and uses the original burst physics: Space boost, gravity, no host
hover/Ctrl descent. Revision 1 (original camera + D-pad turning) was rejected
in playtest: mouse turning failed and controls "slipped" out of modern. See
[D08R section](documentation/57-jetpack-controls.md#d08r---selectable-jetpack-scheme-modern--classic).

**Todo.** User (2026-09-29): Modernized players should be able to choose
classic jetpack controls instead of the D08Q modern flight, and classic must
still work with WASD.

**Scope revised 2026-09-29:** ship this as a CLI switch only. The scheme
choice is one of many customizations that will later live in the D19
in-game customization menu (Sonic 3 A.I.R.-style), not a jetpack-specific
screen. Do not build menu UI for it here.

Scope: add a persisted Modernized profile setting (for example `jetpack` =
`modern` | `classic`) in `player_profiles.py`, a `run.py` flag and the
environment passed to the runtime. The terminal settings menu may list it
only if that is trivial; in-game menu exposure belongs to D19. **Modern** is the
current D08Q behaviour and stays the default. **Classic** runs the original
TTK mode-10 flight as the game intended: body-relative thrust, original
hover toggle, original lift and gravity, no host hover-on-release, no Ctrl
descent and no `face_view` camera coupling. The only change is WASD mapped
to the original pads (W/S to Up/Down, A/D to turn or strafe per the layout)
so it is playable without the tank fallback surprises D08Q found (body
heading not turnable, idle sink). Decide and document whether the mouse
camera stays live in Classic (camera-only lease) or the original camera is
used. Vanilla is untouched in either case.

**Acceptance:** the setting persists and can be switched from the CLI;
Classic flight is controllable with WASD + Space and matches the
original's thrust, hover toggle, lift, fuel and cut-out rules; Modern is
unchanged from D08Q; switching schemes needs no rebuild; Vanilla flight
unchanged. Tests cover the profile migration and the pad mapping.

### D08S - EDuke32-style jetpack (instant J on/off, 61-second fuel)

**Done (2026-10-24).** User accepted revision 4 and explicitly authorized
closeout, commit and push. The third `eduke32` scheme is implemented;
[Note 219](documentation/219-d08s-instant-jetpack.md) records implementation,
private live checks and acceptance.

**User correction (2026-10-24):** Modern must also start flight with J alone,
with a small automatic lift like the supplied EDuke32 video. Modern and
EDuke32 now share that startup lift; their fuel rules remain distinct.
The follow-up also enables Modern J during jumps/falls, including a quick
restart while its native close animation is still running.
**Latest clarification:** the ground boost stays, but midair activation must
arrest vertical movement and hold the activation height without that boost.
This supersedes the original requirement below to leave Modern activation unchanged.

**Original request, reopened 2026-10-06** (previously cancelled 2026-09-29). User: "eduke style jetpack, where you press J and immediately begin
flying, and the jetpack fuel lasts for 61 seconds, matching that of eduke,
pressing j again stops flying."

**Scope.** A third Modernized jetpack scheme, alongside Modern and Classic
(D08Q, D08R), selectable by `--jetpack` and later the menu (D19). **J** starts
flying immediately and pressing **J** again stops flying immediately, with no
graceful lift-off, landing or cut-out animation. It must also work **in
midair** (while jumping or falling), which TTK does not allow: the original
entry `8004aaf8` requires Square on the ground and rejects the airborne and
landing states (anim 105/106 and the `+0x224 & 0x241` flags). Controls while
flying: **Space** ascends, **Ctrl** descends, **WASD** moves relative to the
view, mouse turns camera and Duke together. No input holds position (Duke3D
hover). D08Q3 view aiming and crosshair stay available.

**Fuel:** a full jetpack lasts **61 seconds** of flight, matching EDuke32,
draining only while the pack is on (hovering included). Research how TTK's
fuel `+0x35a` and drain `-= dt` (`8004ade0`) map to seconds, what a pickup
grants, and how the HUD green % should read so 100% means 61 s in this scheme.

Research first: whether mode 10 can be entered directly from airborne modes
with the original handler kept intact, or whether this scheme needs a host
flight model that drives the root position and state flags itself; what the
instant J-off should do (Duke3D drops straight into a fall); animation choice
for instant take-off; collision with ceilings and water. Keep all writes gated
on Modernized + the selected scheme; Vanilla and the other two schemes
unchanged. Related: D08Q4 (`dnkroz` unlimited fuel), D11F (first person).

**Acceptance:** with the EDuke32 scheme selected, J toggles flight instantly
on the ground and in midair; Space/Ctrl climb and descend at steady rates;
WASD flies relative to the view; releasing input holds position; a full pack
gives 61 s of flight; J off drops Duke into a normal fall with controls live;
fuel-out behaves like J off; Modern, Classic and Vanilla unchanged; the user
confirms.

### D08T - Pushable objects: modern grab, push/pull and climb

**Opened 2026-09-30 from a user bug report.** In the first map's alley the
green dumpster is meant to be pushable, but in Modernized (third or first
person) Duke never grabs it: wedging into it and holding **E** does nothing
or climbs on top. On DuckStation the user could almost start the push
animation, then Duke climbed instead. The user wants both actions - push
and mount - with a modern, easy control that works for every pushable
object, not only the dumpster. Evidence: a private copy of the user's
slot-5 savestate (UI slot 5, in front of the dumpster).

**Acceptance:** in Modernized, Duke can grab any original pushable object
(object type flag `0x08000000`), push and pull it with camera-relative
movement and let go, without accidentally climbing; he can still climb
onto a climbable pushable object with a separate, obvious input; mouse
look stays live while grabbing; Vanilla keeps the original Action rules.

**2026-09-30 - Needs playtest.** Original rules traced and replayed from a
private copy of the slot-5 state: holstered, still, a fresh Action against an
object flagged `0x08000000` grabs it (anim 121); Action held + Up pushes (120),
+ Down pulls (119); any direction held with Action climbs a climbable one
(`0x80051cf0`, 139/140). The dumpster is both. Modernized failed because E's
Action dropped as soon as the grab began (no lease for 119..121) and W + E
took the climb branch. Implemented (`recomp/src/ttk/push.inc`, `pc_input.cpp`,
new generated hook `0x80051cf0`, three new SHA guards): **E** grabs any
pushable object even with W held; the grab latches; **W/S** push/pull relative
to the camera; **E** lets go; **Space** climbs a climbable one; mouse look
stays live; first person blends to the orbit while grabbing. Real-key Xvfb
checks in third and first person: grab without climbing, push 2582 -> 3713,
pull 2582 -> 1668, camera-facing W pulls, Space climbs to the top with
automatic holster and redraw, 59.5 fps. Vanilla push unchanged; Vanilla route
exit 0 with reviewed captures; native suites PASS. Limits: only the dumpster
checked; original line-up angle and push speed unchanged; Space next to a
climbable pushable object climbs instead of jumping. Details:
[documentation/64-d08t-pushable-objects.md](documentation/64-d08t-pushable-objects.md).
Binary `941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

**2026-09-30 - Done (user-accepted).** User playtest: "it works so much better than the original now. this is it rock solid. confidence level is very high." Remaining limits above stay recorded (only the dumpster checked).

### D08T1 - Separate push/pull manipulation from mantling

**Done (user-accepted, 2026-09-30).** E mantles pushable objects like any other;
holding Grab (RMB, or Alt; Alt only with legacy aiming, where RMB stays
precision aim) grabs, W/S push/pull, release lets go. Schema 17. Details and
evidence: [documentation/70-d08t1-grab-manipulate.md](documentation/70-d08t1-grab-manipulate.md)
and the 2026-09-30 D08T1 work log.

**User brief (2026-09-30), backlog only:** the accepted D08T controls make an
invisible object property change the traversal controls. On a pushable object
**E** grabs and **Space** climbs, while everywhere else **E** mantles. The
user wants pushability to add a verb, not remap one: **E** always mantles (a
pushable object mantles exactly like an equivalent non-pushable one), and a
new rebindable **Grab / Manipulate** action, **held**, grabs the object:
**W/S** push/pull while held, and releasing it lets go. There is no toggle and
no E LET GO. The default is **right mouse** in Modernized. The user decided
Modernized does not need right-mouse precision aim, since view aiming already
covers it. Vanilla keeps precision aim. Proposed: Alt is a second Grab
binding everywhere in Modernized, and the only one when legacy `original`
weapon aiming keeps RMB as precision aim (confirm at job start). Full
brief, including the other-input and interruption cases, the HUD text
(`W/S PUSH/PULL - RELEASE RMB TO LET GO`, using the live binding label if
possible) and the test list:
[documentation/69-d08t1-grab-manipulate-brief.md](documentation/69-d08t1-grab-manipulate-brief.md).

**Before implementing:** document how the current E/Space exception is built
(`recomp/src/ttk/push.inc`, `pc_input.cpp`, hook `0x80051cf0`). Audit every
Mouse2 / `original_aim` path in Modernized, including the D08Q aim-held
flight lease. Mantleable and manipulable stay independent capabilities, with
no dumpster-specific special case. Also check whether E while grabbing can
release and mantle cleanly. Release RMB then E must mantle at minimum.

**Acceptance:** normal and pushable objects both mantle with E; hold RMB
grabs, W/S push/pull with the existing D08T mechanics, and release disengages;
grab -> push -> release -> mantle and mantle -> leave -> grab -> pull ->
release repeat without stale state; RMB against ordinary geometry does
nothing odd; obstruction, damage, falling/separation, object loss and pause
end the grab cleanly. Third and first person. Vanilla unchanged.

### D08T2 - Duke-symbol pushable blocks with Modern controls (game-wide)

**Accepted (2026-10-05, user: "i accept everything aside from the ladder
glitch").** The hint changes are accepted too; the slot-12 ladder continues
under D08U1. Earlier: Needs playtest. Cause: the block is an ordinary original
pushable object (type 924, flags `0x081890e2`), but `object_type_flags()` in
`push.inc` refused types `>= 512`; the original type table holds 1062 types
(`0x7428` bytes allocated at `0x8001b634`). Fixed in the shared lookup, so
grab/push, the ladder-top mount and the ledge/crate mantle now see every type
(4 more pushable, 6 more ladder, 114 more climbable types). RMB pushes the
slot-1 block in third and first person; dumpster and Vanilla unchanged.
Candidate `06c1f8b10fde4f6249f1e692d39de6b6abb83a2086c19cc0a9cfab5e391b8b55`.
[Note 103](documentation/103-d08t2-object-type-range.md).

Selected 2026-10-05. Reproduction: UI slot 1 (savestate file
00, SHA-256 `ab8d2e156b6bdaf802f7a454449a07eaa8a47f530f99108a96fb0b8abefcbc72`,
written 2026-10-05 01:15), first medieval level: the block directly ahead
should push. Guide: on the right side of the castle, before the moat enters
under the wall, a ledge runs along the castle wall (jetpack onto it); the wall
with the atomic symbol pushes to open a secret (power-ups and ammo). Fix the
system for every pushable object, not this block. Private copies:
`recomp/analysis/d08t2-20261005/states/`.

User report, 2026-10-05 (D22B game-wide playtest).** Blocks marked
with the Duke Nukem symbol are meant to be pushed. With Modern controls they
cannot be pushed; after switching to Legacy controls the same block pushes
correctly, so the original interaction works and our modern mapping does not
reach it. Seen in the Roman area (one), the medieval area (one) and two near
the start of the medieval area. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Direction from the user: E = interact, RMB = grab/hold/push. For these
objects investigate RMB as the modern interaction: approach, hold RMB to
engage, move to push/manipulate, release RMB to let go. First inspect how the
original interaction actually works for these blocks (object type/flags,
which original button and state it uses, how it differs from the D08T alley
dumpster that RMB already handles), then fit it into the D08T/D08T1 grab
design rather than mapping another Legacy button.

**Game-wide:** every pushable object, not one block.

**Acceptance:** the reported blocks in at least two eras push with RMB in first
and third person; the alley dumpster still works; release and focus loss let
go; Vanilla/Legacy unchanged; the user confirms.

### D08T3 - Free manual push and pull while holding Grab

**Done - user-accepted 2026-10-23; explicit closeout/commit/push requested.**
Final user tests: slot 12 dumpster pushes freely and stops toward the ladder;
slot 11 push/pull works throughout permitted travel; slot 10 cube stops at an
enemy and remains stable; slot 9 block stops at its supported edge, preventing
the old floating result. Accepted behavior and prior regression evidence
satisfy this job's acceptance. Full-campaign coverage is not claimed.
The forced-crouch trap after mantling the moved dumpster is a separate
locomotion limitation, recorded as **D08L1 (Todo)** above, not implemented.
Accepted executable: `1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
The candidate history below is retained as evidence, not current status.

**Review revision, 2026-10-23 - Needs playtest.** Fixed the reproduced held-W
handoff into an original one-shot shove. Flat ladder panels and complete room
object lists now stop the dumpster before the ladder. Private keyboard/mouse
routes on slots 9-12 pass continuous movement, neutral hold, reverse, sustained
boundary refusal and release. Native regressions cover both missed cases.
See note 208 for revision evidence. No acceptance closeout or commit/push.

**2026-10-23 - Needs playtest.** Manual held push/pull, immediate directional
stop, retained grab, reverse and supported-edge/wall checks are implemented.
Private copies of slots 9-12 exercised; slot 9 stops at the supported edge
instead of reaching slot 8's already-floating position. Build/native suites
pass; release, pause, focus, saved moving pose, repeated loads and E mantle
checked. User feel, sound and puzzle-completion acceptance remain. Candidate
`1e6c1004a8d32da3758035c3eca3a90de66de3971ade2a5e4c27731738c3c118`.
[Note 208](documentation/208-d08t3-manual-push-pull.md). No commit/push.

**Selected 2026-10-23.** Test private copies of current UI slots 10, 11 and 12; slot 9 reproduces a block pushed off support after three shoves, and slot 8 records the floating result. Investigate original stopping rules and stop at unsupported edges rather than adding physics.

**Original request. User request, 2026-10-05 (D08T2 playtest).** "The rmb pull and push
mechanic seems to work in set amounts at a time ... hold rmb and press W and
duke will push the stone a set amount (im guessing 1130 units) and then stop
and release. I would love to give the user the actual freedom to properly move
the block manually by holding rmb, properly grabbing, and pushing and pulling
the stone block that way." Suggested as the next job.

Today the original push 120 / pull 119 (`0x80048e84`, object motion
`0x800910a0`) is one fixed shove of about 1130-1200 units along one world
axis chosen from Duke's heading; D08T1 lets it run to its end, and Duke then
lets go. Investigate how the original moves the object per update (step size,
axis, collision and blocking checks, the end-of-shove release, sound), whether
a shove can be continued, stopped early or chained while Grab and W/S stay
held, and what drives the push sound and animation loop. Then design modern
manipulation: while Grab is held, W/S move the object continuously (and stop
when released), Duke stays attached until Grab is released, and blocked
objects stop cleanly. Keep the original collision and puzzle rules (no pushing
through walls, keep axis-aligned puzzle positions such as the Duke-symbol
secret walls) unless the user agrees otherwise. Game-wide: every pushable type.

**Acceptance:** with Grab held, W/S push and pull a pushable object by any
amount the player chooses, stop when W/S is released, keep holding until Grab
is released, and stop at obstructions; the dumpster and the Duke-symbol
blocks work in third and first person; puzzles that need a pushed block still
complete; Vanilla unchanged; the user confirms.

### D08T4 - Save slot 10 dumpster: grab prompt but no movement

**Todo (opened 2026-10-20, user report; investigation first).** In the
player's save slot 10, Duke at a dumpster gets the "hold RMB to grab" prompt,
but holding Grab does not move it. Find out which of these it is:

1. A normally pushable dumpster that the level designers placed as a fixed
   step to the platform above (not pushable here by design).
2. Pushable in the original, but its current position (against a wall, an
   edge or another object) blocks every push and pull direction as intended.
3. A Modernized regression: it moves in Vanilla (original controls, the same
   save copy) but not with the RMB grab (D08T1/D08T2).

Method: a private copy of the slot 10 savestate, offscreen. Read the
object's type and flags, the push/pull checks the original runs on it, and
whether Vanilla can grab, push or pull it. If it is fixed by design or
blocked, the grab prompt should not appear (prompt only for an object that can
actually move); if it is a regression, fix it game-wide, not for this level.

**Acceptance:** the cause is established with evidence (Vanilla comparison);
either the dumpster moves as in the original, or the prompt no longer offers a
grab that cannot happen; other pushables unchanged; Vanilla unchanged.

**2026-10-20 - Needs playtest.** Cause: case 1, fixed by design. The dumpster
(level 5, object type 165, flags `0x081090e2`, identical to the alley one) sits
on floor whose record (`*(player+0x48)`, byte 4) has bit 0 clear; the original
idle grab `0x80051890` requires that bit before it even lines up, so in Vanilla
on the same save copy holstered Cross does nothing and Cross + Up climbs it.
Every reachable side reads 0; the alley reads 1. Our `push_contact_ready()`
checked only the type flag, so the hint showed and Grab holstered for nothing.
Fix (game-wide, `push.inc`): the contact predicate also requires the original's
floor bit, so the hint and the Grab request only happen where the original can
grab. Real keys: slot 10 no hint, no holster, nothing moves (A/B without the
gate reproduces the report); alley dumpster and medieval block still grab and
push (1131 / 1129); Vanilla identical before and after. Native suites pass.
Executable `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4`,
[note 163](documentation/163-d08t4-fixed-pushables.md).

**2026-10-20 - Done (user-accepted).** User: "lock it in, close it out,
another winner". Executable
`810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4` is the
regression baseline.

### D08T5 - Alley dumpster ladder approach limit

**Done - user-accepted 2026-10-24.** User: "lock it in!! close us out".
The candidate is accepted with its recorded existing floor-based regrab rules
and bounded route coverage. Implementation and documentation closeout authorized.
No further gameplay edits or rebuild. Candidate history below is retained.

**Needs playtest - candidate 2026-10-23.** Exact slot-7 limit X 7292 implemented.
Build/native suites pass; live held stop, reverse, return, saved position and
slot-9 edge regression checked. Real-input slot-7 route attaches to the ladder
and reaches climbing animation 187. Regrab from the west-side floor is refused
in both baseline and candidate by the existing floor permission rule; position
persistence is not a successful regrab test. Final spacing/ladder feel awaits
user acceptance. [Note 210](documentation/210-d08t5-dumpster-limit.md).

**Selected 2026-10-23.** User explicitly requested investigation,
backlog creation and implementation in this session.

Load a private copy of current UI slot 7 and record the alley dumpster's saved
position. In Modernized, use that point as the furthest permitted push toward
the ladder so a ladder approach does not put Duke onto the dumpster under the
platform. Scope to the authenticated level and this dumpster; preserve free
pulling away, other pushables, collision/support checks and Vanilla.

**Acceptance:** measure and visually inspect slot 7; sustained pushing stops at
its reference boundary without drift, release/regrab and save/load preserve the
limit, and reverse works. Verify the ladder approach in gameplay; retain existing
block edge/collision and crouched walk-off behavior. Already beyond-limit saves
must allow retreat without an unsolicited relocation. Build/native regressions
and private live tests; user playtest for final ladder feel. No commit or push
without explicit closeout instruction.

### D08T6 - Push/pull ownership and lock-zone investigation

**Done (user-accepted 2026-10-24; documentation/commit/push closeout authorized).**
Research separated floor-gated acquisition from held travel: releasing a movable
object on a floor without grab permission could prevent regrabbing, while held
reversal still worked. [Note 211](documentation/211-push-pull-lock-zones.md)
records the ownership map, bounded design and evidence limits.

The resulting implementation jobs are complete:

- D08T7 implemented safe reacquisition for the verified alley dumpster and three
  symbol-block lanes, including loaded boundary saves. Original alignment,
  support/collision, fixed scenery and D08T5's X 7292 cap remain enforced.
  [Note 212](documentation/212-d08t7-lane-regrab.md) records tests and acceptance.
- D08T8 resolved the separately reported hang, corrected to the apartment
  platform beyond the dumpster. Stale lower support blocked the original
  pull-up query; the guarded retry and Space-to-Down drop are accepted.
  [Note 214](documentation/214-d08t8-alley-platform-hang.md) records the fix.

D08T6 is complete as the research and continuity record linking those accepted
results. This closeout adds no gameplay changes or fresh tests and does not
claim every campaign placement was tested. The intermittent reload GPU halt
remains unresolved under D20G's recurrence guidance, separate from this work.

### D08T7 - Regrab movable objects throughout their permitted travel lane

**Done (user-accepted 2026-10-24).** Selected by the user after D08T6 investigation. User wants flexible push/pull within the
existing imaginary rail, including pulling slot 12's dumpster back after it
reaches the limit and is released. Preserve D08T3/D08T5 travel guardrails.

Investigate and implement object/placement mobility separate from Duke's local
floor grab permission. Cover both host prompt/Grab eligibility and original
acquisition. Do not globally bypass floor gates or unlock intentionally fixed
scenery. Determine a stable identity and loaded-save policy; a session-only
previous-grab latch is insufficient. See note 211 for source/evidence and matrix.

Acceptance:
- Movable dumpster and recent symbol blocks can be released, safely reacquired
  and reversed at reachable valid positions throughout their permitted lane.
- Continuous-held reverse remains responsive; the slot-12 X 7292 stop remains.
- Reload at the boundary retains eligibility; approach from both reachable lane
  ends works without snapping, changing axes or corrupting object placement.
- Support, wall/actor/ladder collision and Duke clearance remain enforced;
  fixed level-5 scenery stays fixed and prompts match actual eligibility.
- Private-copy slots 7/9/10/11/12 matrix, pause/load and both cameras/rates tested;
  slot 8 unsupported state is not relocated; Vanilla and player files unchanged.

Future editor metadata is an architectural consideration, not an editor task.

**Candidate:** verified placement lanes now permit original-aligned acquisition
without Duke's local floor grab bit. Guarded original line-up plus existing
support/collision checks; no floor mutation or save-state cache. Live boundary
release/regrab/reverse, save/reload, both dumpster ends, slots 9-11, fixed level-5
control and first-person preference at 120 FPS with pause/resume pass. Native
controls/input/aim pass, including lane refusal across all 21 level guards.
User reports the mechanic is "absolutely solid" after several tests and explicitly
authorized locking in D08T7 with commit/push. Broader campaign placements are not inferred.
[Note 212](documentation/212-d08t7-lane-regrab.md). D08T8 remains Todo.
Acceptance closeout and commit/push explicitly authorized. The separate load-time
fatal halt is D20G (Todo); its cause and relationship to D08T7 are not established.

### D08T8 - Apartment alley platform mantle stalls: W cannot climb, Space cannot drop

**Done - user-accepted 2026-10-24; closeout explicitly authorized.** Dependencies: D08V,
D08X, D08T3. User now confirms all dragging checks pass and the dumpster mantle works.
Load UI slot 7, climb the dumpster, then mantle the apartment alley platform
directly ahead: Duke hangs without pulling up; only S drops him. This corrects
the earlier dumpster-side attribution. Reproduce on a private copy before
assigning a cause or changing gameplay. Separate from accepted D08T7.

Acceptance:
- Reproduce on an identified private slot/placement and compare original input.
  Record animation/mode, attachment, input ownership and clearance evidence.
- Where the top is safely reachable, forward input completes the intended climb.
  Where blocked, provide a consistent safe cancellation/drop; Space must not
  silently fail in a cancellable hang. Preserve S's working escape.
- Verify side/end approaches before/after pushing, E tap/hold, both cameras,
  weapon states, ordinary mantles, D08L1 crouched escape, ladders/ceiling hangs
  and Vanilla. No forced climb through ceilings or loss of accepted guardrails.

Fresh reproduction: dumpster climb succeeds; platform hangs in mode 6,
animation 149. W and Space reach the host pad but do not change the hang;
S releases. [Note 214](documentation/214-d08t8-alley-platform-hang.md).
Candidate retries the original clearance query without stale lower support and
maps Space to original Down in settled hangs. Live route, drop and dragging
checks pass; user confirms the fix works exactly as intended. The broader
placement/obstruction matrix was not exhaustively tested.
A first-person 120 FPS private reload GPU halt remains unexplained; see note 214.

### D08T9 - Level-5 dumpster cannot be pushed again (bug 2)

**Status:** Done (user-accepted 2026-10-25). **Dependencies:** D08T3, D08T7 (Done).

**User-confirmed report (2026-10-25):** In current slot 8, after pushing the
level-5 dumpster once, it cannot be pushed again. The user confirmed this in
their own test. Investigation reproduced loss of grab permission after Duke
crosses onto a floor tile with the original grab bit clear. Short regrabs work.

**Acceptance:** Reproduce first movement followed by release/regrab and a
second push/pull on a private slot-8 copy. Establish intended
travel limits and object behavior; restore repeat movement where permitted
without moving fixed objects or bypassing genuine limits. Verify both
movement directions and repeated release/regrab cycles.

**Candidate (2026-10-25):** Extend verified placement acquisition to this
LEVEL05 Z lane. Repeated push/pull, release/regrab, private boundary save/load,
60/120 FPS, both travel limits, pause/resume and fixed-scenery regression pass.
Build and controls/input/aim native suites pass. User accepted the result and
explicitly authorized lock-in and closeout.
[Note 243](documentation/243-d08t9-level5-dumpster.md) records evidence and limits.

### D08T10 - Slot-5 box repeat grab after pulling

**Status: Done (user-accepted; closeout authorized 2026-10-29). Dependencies: D08T3, D08T7, D08T9 (Done).** Selected
2026-10-29. User reports the current slot-5 box can be pulled back once, then
cannot be grabbed again. Investigate private copies and distinguish floor
acquisition permission from contact, alignment and supported travel. Extend
verified placement eligibility if justified, retaining collision/support,
fixed scenery, existing lanes, first/third-person policy and Vanilla.
Acceptance: reproduce the failure, repeat release/reapproach/grab and reverse
at the affected position, verify a fresh private boundary reload, run native
regressions and obtain user gameplay acceptance. Player saves remain untouched.

### D08T11 - Slot-8 pushable block stuck; game-wide eligibility

**Status: Done (user-accepted 2026-10-30: "I can push and pull the block now no problem"; closeout authorized 2026-10-30; [note 332](documentation/332-d08t11-displaced-pushables.md)). Dependencies: D08T3, D08T7, D08T10 (Done).** Opened
2026-10-30 from the user's playtest: in UI slot 8 (saved 2026-10-30 12:22)
the block can no longer be pushed or pulled; there is no save from before it
stuck. The user asks whether all push/pull blocks can be fixed at once rather
than one at a time as play finds them. Reproduce on a private copy, find why
the stuck position is refused (lane, alignment, support or acquisition gate)
and prefer a game-wide rule over a per-object exception. If a single rule cannot
be proven safe for every object, say so and keep per-object fixes as the
fallback the user accepted.
Acceptance: the slot-8 block can be grabbed, pushed and pulled again from its
stuck position; earlier accepted blocks and dumpsters (D08T2-T10) still work;
native regressions pass; Vanilla unchanged; user playtest.

**Work log (2026-10-30, Needs playtest).** The block (type 924) had been
pushed 2490 units into a wall recess onto a no-grab floor, the same failure
class as D08T7/T9/T10. Game-wide rule in `push.inc`: the level's placement rows
(`*0x800de720`, 48 bytes, count `0x800de724`) keep each object's authored
x/y/z, so a pushable object away from its authored x or z has been pushed and
may be grabbed along that axis anywhere. Fixed scenery never moves and stays
fixed without a list; the verified lanes remain for authored spots. Private
slot 8: grab from the recess, pull out, push back, release, regrab, pull again.
`ttk-controls-test` new D08T11 group plus all D08T groups pass (115 groups).
Limit: card-save reloads were not tested (savestates only). Manual updated.

### D08T12 - Crouch soft lock at a pushed block

**Status: Done (user-accepted 2026-10-30: "the softlock is now fixed"; closeout authorized; [note 333](documentation/333-d08t12-squeezed-crawl.md)). Dependencies: D08K, D08T11.** Opened 2026-10-30. The user
reports that in slot 8 Duke can be soft-locked by pushing the block in and
then jumping onto it or crouching to get under it: he cannot stand up again.
Reproduce on a private copy, identify the uncrouch clearance test and the
block's collision at that spot, and make sure Duke can always leave (stand,
crawl out or be pushed clear) without passing through solid geometry.
Acceptance: no soft lock at the reported spot from either route; ordinary
crouch and crawl spaces still keep Duke crouched where the ceiling is low;
Vanilla unchanged; user playtest.

**Investigation (2026-10-30).** Three scripted routes on the private slot 8
copy did not reproduce it: with the block in its recess, W+E+Space reaches only
a hang on its face (animation 149; the recess leaves no room on top), holding W
does not climb higher, and S drops back; crouching in front and releasing
stands normally. Needs a save made while stuck (any free slot) or the exact
key sequence.

**Work log (2026-10-30, Needs playtest).** The user saved the soft lock in UI
slot 9 (13:24). Private copy: Duke kneels (178) on the block top inside the
recess; W/S/A/D, Ctrl, Space and C do nothing. The crouch was adopted and the
stand-up correctly refused (no headroom); every crawl probe failed `hull_low`
because the headroom where he kneels is already below the 704 crouch hull, so
every destination, including the way back, looked too low. Game-wide fix in
`crouch_hull.inc`: a crouch walk that starts squeezed may move where headroom
is at least what he has now (and at least 0x100), with the head ray just under
that ceiling; never into tighter space; other probes keep the full hull.
Private slot 9: S crawls along the block top, W crawls off the edge into the
original fall, he lands, stands (63) and walks. `ttk-controls-test` adds the
squeezed case to D08K12 (moves at 0x280, refuses at 0xf0); all 115 groups,
input, aim and near suites pass. Manual updated. Player save only read.

### D08T13 - Pushed block and Duke pass through metal bars

**Status: Done (user-accepted 2026-10-30: "the fix is in! and it's absolutely
perfect"; closeout authorized 2026-10-30; [note 334](documentation/334-d08t13-multibox-push-collision.md)).
Dependencies: D08T3, D08T11.** User report after D08T11/D08T12 acceptance: the
slot 8 Duke-symbol block, pushed in the other direction, goes straight through
a set of metal bars, and Duke clips through the bars while pushing. Wanted: a
game-wide safeguard (as D08T5 limited the alley dumpster), the block stopping
just short of the bars, and Duke never passing through bars while pushing.
Investigation so far (private slot 8 copy): pulling along the block's Z lane
toward -Z stops correctly at block Z 20389 against plain walls (Duke leads
and his checks stop him); a barred window is visible on the side wall, so the
reported push is probably along X from the spawn area, where the original
floor allows grabs on any face. Scripted placement of Duke behind the block
failed (the debug server writes single bytes while the game runs). Next: a
user save just before pushing into the bars; then compare the manual mover's
ray and clearance queries (`0x800774cc`, `0x8007765c`) with the original walk
collision at the bars and use the original collision for both block and Duke.
Acceptance: the block stops short of the bars and Duke cannot pass them at the
reported spot; other blocks/dumpsters (D08T2-T11) keep their travel; native
regressions; Vanilla unchanged; user playtest.

**Work log (2026-10-30, Needs playtest).** User saves: UI slot 8 (block just
before the bars) and UI slot 9 (in them). The bars are box 2 of a three-box
LEVEL06 frame object (type 354); the manual mover read only box 0 (the lintel
above the block), for the block and for Duke. `push.inc` now checks every box
of every room object; a box already overlapped at the start may only shrink
or let the block back out toward Duke. Slot 8: stops 2 units short of the
bars and pulls back; slot 9: cannot go further, backs out; Duke leading now
stops at his radius instead of walking through (Z 19465 before). Native gate
case added to the D08T3 group; 115 groups, input and aim pass. Earlier
"probably along X" guess was wrong: the bars are on the block's Z track.

### D08U - Top-of-ladder mount: grab a ladder from a platform and climb down

**Done (2026-10-01, user-accepted).** E at a ladder top lowers Duke onto it through
the original hang-to-ladder transfer; S climbs all the way down and steps off.
The original has no top mount for plain ladders. See
[research, change and evidence](documentation/72-d08u-ladder-top.md).

**User report (2026-09-30, during the D13 playtest):** standing on a platform
beside the top of a ladder that must be descended, there is no control or
mechanism that makes Duke latch onto the ladder and climb down. Reproduction:
the user's savestate **slot 12**, saved right next to that ladder. Test only on
a private copy of that state; never load or write the player's own cards or
slots in place.

The user wants it to feel "very fluid, natural, modern", to the standard of the
accepted traversal work (D08, D08B, D08J, D08T). Investigate first: whether the
original game has a top-of-ladder mount (a routine, animation or trigger volume)
that Modernized input fails to reach, or whether one has to be built from the
original climb state and animations. Record the ladder's location and level
from slot 12.

**Acceptance:** from the slot-12 platform, walking or backing toward the ladder
top with a simple, discoverable input (for example move toward the edge at
the ladder and/or the interact key) turns Duke to face the ladder and puts him
on it smoothly: no pop, fall, damage or camera snap. He then climbs down with
the existing ladder controls and dismounts at the bottom, redrawing a weapon as
D08J does. The input must not grab accidentally when walking past, and it must
not interfere with ordinary edge run-off (D08L) or jumps. Cover third and
first person. Vanilla keeps its original behavior. Check the other ladders
already known from D08B/D08J to see whether the same mechanism applies there,
and record any ladder that it does not cover.

### D08U1 - Ladder that cannot be descended with E (player slot 12)

**Accepted (2026-10-05, user: "accepted!! done, commit. great work").**
Executable `4f76da406a12958fe50e4751c7a99832b90a9f562710326060e2d1a66a732c83` is the regression baseline. Causes, in order found:
object type 639 refused by the old 512 type bound (D08T2); the ladder ends
above its floor, so the original swung Duke into a bottom-rung hang (now
stopped on the last rung via the original probe `0x8007ded0` and let go with
Square); and, the user's actual failure, the mount blend cut short when the
game drops frames, leaving Duke above the climbing line (now finished while he
rests). See the work log and [note 103](documentation/103-d08t2-object-type-range.md).

**Needs playtest (2026-10-05, second fix).** User, after the first fix: "Duke
cannot climb down the ladder ... when you start pressing S, he just glitches
out. we need to truly fix that." The first fix only let Duke drop out of the
original bottom-rung hang, so the sideways 156 -> 211 swing still played.
Second fix: the bottom is now decided before the swing. At each player update
on a plain ladder (186..189) the host asks the original's own probe
`0x8007ded0` (no ladder and no floor 500 below Duke). The 188/189 case at
`0x80044128` decides from the descent flag `+0x6a`, not from Down, and in the
same update; so when the probe is true during a step down, the flag is
cleared and the original takes its own stop-at-this-rung branch. On that
last rung S sends the original Square let-go from the climbing pose (fall 108,
landing 105), never the hang. Real keys on the slot-12 copy: holding S (third
and first person) climbs down and drops straight to the floor from the last
rung; tapping S stops on the last rung and the next tap drops; W from the
last rung climbs back up and exits at the top (190). Ladders that reach their
floor never trip the probe: the D08U sewer ladder keeps the 185 step-off and
the alley ladder climb is unchanged. Candidate `cf90d94246907355d3b81c8d2734715069e48edb23df8f8009ce895d029461da`.

First fix (superseded). Two causes. (1) The slot-12 ladder is object
type 639; the shared type lookup refused types `>= 512` (fixed by D08T2): E
now mounts it. (2) User retest: "when you press S to go down, duke starts
glitching". This ladder ends about 1000 units above the floor. At its lowest
rung the original (`0x8007ded0` finds no ladder 500 below) goes 156 -> 211 ->
207, a hang from the bottom rung; there original Square lets go and Up climbs
back, but Down (S) only flips 211/207 with Duke's body in the floor. The
original pad alone does the same, so it is an original state our top mount
now reaches. Fix (game-wide, `ladder_top.inc` `ladder_bottom_hang()`,
`pc_input.cpp`): S in that hang sends the original Square (let go) instead of
Down; on the way in (211) neither. Real keys, both views: S held from the top
reaches the floor (108 fall, landing, Modernized control, weapon redrawn).
The D08U sewer ladder still steps off with 185. Candidate
`fa28448d26694572d1c58d4ab2498c8da8329261200e3fc19c24f186418d774e`.
[Note 103](documentation/103-d08t2-object-type-range.md).

**Todo. User report, 2026-10-05 (D22B game-wide playtest).** Player UI slot 12
(savestate file 11 in `recomp/saves/local-play/openbios`, written 2026-10-05
00:37, SHA-256
`1608ee9c890ebb30b0311f055e6f13d21a18e61697a61557e384e82b48a59afd`): a ladder
just around the other side of the wall should be climbable downward. With
Modern controls, E does not start the descent. It is the first ladder found
where the modern controls fail. Test only on a dated private copy; verify the
hash first.

Determine why this ladder differs from those that work (D08U top-of-ladder
mount: object type/flags, collision box, reach/side/top gates, approach
direction, level overlay, original attach path) and fix the shared rule, not
this ladder.

**Acceptance:** the slot-12 ladder descends with E; the D08U/D08J1 ladders and
LEVEL00 slots 8 still work; Vanilla unchanged; the user confirms.

### D08U2 - Enemy at a ladder top blocks the climb-off

**Accepted (2026-10-07, user: "fully accepted. this is exactly what i wanted. well done!").**
Executable
`e8eebb809c3d5a4b29779f11f0c9db55efcd846dc5c71b39d5a59d43158eb08e`. Cause:
original. Resting on the top rung with Up, the handler's exit test
`0x8007d240` first casts a short segment forward (`0x8007cde8`) against
Duke's cell objects; the slot-5 LARD (type 57), about 530 units past the
ladder, is hit, so the exit 190 is refused for as long as it stands there.
Fix (Modernized, game-wide, `ladder_top.inc` `ladder_actor_exit`): when only
living characters (class 6 or 8) block that segment, the host runs the
original exit test with them out of the way for the length of the call and,
if it passes, starts the original exit exactly as the handler would. Walls
and other objects still refuse; the enemy is not moved (pushing was marked
optional by the user). Private runs: slot 5 in third and first person climbs
off (190 -> standing, pistol redrawn); second alley ladder with an enemy
respawned on its landing climbs off and lands 300 units short of it; D08U
top mount and S descent, D08U1 slot-12 ladder unchanged; suites pass.
[Note 121](documentation/121-d08u2-ladder-top-enemy.md).

User clarification: "whether duke actually pushes
the enemy back or not is not a hard factor in this job, but Duke should
certainly be able to make the climb".

**User request, 2026-10-07:** "when an enemy is stood at the top of a
ladder, duke cannot climb onto the surface. Duke should push into the enemy
forcing it back." In Modernized, climbing off the top of a ladder must not
be blocked by an enemy standing on the landing: Duke pushes the enemy back
and completes the climb-off.

**Test location:** the user's UI slot 5 is a good playtest for this (user,
2026-10-07). Test only on dated private copies of that savestate; verify
its hash first and never use the player's cards.

**Research start:** how the original ladder-top exit (the climb-off from the
ladder anims, compare D08U / D08U1 and `traversal_state_ready()`) tests for
room on the landing, and whether an actor in that space makes it refuse,
wait or loop; whether this also happens in Vanilla (original behaviour) or
only through our ladder handling. Prefer the original actor-push / collision
response (the way Duke and enemies already shove each other on the ground)
over moving the enemy by hand; keep it game-wide, not per level, and make
sure the enemy cannot be pushed through walls or off ledges unfairly.

**Acceptance:** in Modernized, with an enemy standing at a ladder top, Duke
climbs off onto the surface and the enemy is pushed back, at slot 5 and at
least one other ladder; normal ladder exits, ladder-top mounts and grabs
unchanged; Vanilla unchanged unless the user agrees otherwise; the user
confirms.

### D08V - Sewer mantle/hang modern-control coverage (slot 12 area)

**Done (user-accepted, 2026-09-30).** Mantles, hangs, pull-ups, the frames after
them and unowned falls keep the mouse camera and modern buttons; the original
routines are unchanged. Reproduced sweep: tank fallbacks 12 -> 0, mantles
identical. Details: [documentation/71-d08v-sewer-mantle-lease.md](documentation/71-d08v-sewer-mantle-lease.md).

**User report (2026-09-30, D13 playtest, 4x exclusive):** mantling around the
sewers near the slot-12 save is glitchy: controls keep switching in and out of
modern mode, and the user wants them to stay modern. Mantling also felt
slightly harder, but the user was unsure. Session log
`recomp/build-local/logs/session-20260930-112050.log` has 54 "WASD tank
fallback / Modern movement lease resumed" pairs. The lease was dropped for
`state` in anim 139/140/142 (mode 8/8, 8/0, 8/6), 107/108 (mode 9), and
63/76/78 (0/8, 0/9). Resolution cannot cause this: the lease and the
simulation do not depend on internal scale, and the user measured a steady
60 fps.

**Acceptance:** identify these states on a private copy of slot 12 (never the
player's own slot). Extend the guarded lease so that modern movement, camera
and Shift stay in control through the sewer mantles, hangs and pull-ups
without flicker, while the original traversal routines stay intact. No
`ORIGINAL MOVEMENT` banner during ordinary sewer traversal. Compare mantle
success before and after. Vanilla is unchanged.

### D08V1 - Modernized controls lost after a missed chain jump

**Accepted (2026-10-07, user: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area").** Executable
`cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43` is the new regression baseline. Playtest log
`session-20261007-202257.log`: two chain top exits (196), no identity failure.
**Built 2026-10-07 (was Needs playtest).** Executable
`cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`. Cause: the
original pole/hang side probe `0x800439e4` sets or clears bit 0x40 of the
upper-body flag entry of Duke's current animation (store `0x80043bec`, indexed;
its `lui` sits in a branch delay slot, so the earlier review missed it). On
every chain top exit that is 196. The code guard read it as changed code and the
lease stayed off for the session. It reproduced on the first private run (E
mount, W, top exit). The tank fallback then had no turn. Change: the flag table
moved to masked `state_guards` (`code_identity.h` `masked_identity`): bit 0x40
of all 280 entries, and the low half of entry 265 that the bonus levels'
`BONUS.OVR` writes, are state, and every other bit stays authenticated. An
identity refusal now turns Duke with the mouse through the original D-pad turn
and shows `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`. It still recovers
when the bytes return. New `tools/local/guard_writer_audit.py` scans the disc
for stores into guarded ranges (delay-slot aware) and diffs a live game. The
only other hit, `0x8001bd14`, is unreachable, and the other guard sets are
clean. Private slot-3 runs: top exit, two catch jumps, a missed jump and fall
keep the lease; a live audit shows 0 guarded words differ; a forced loss turns
with the mouse (+400 counts gives +491), moves, and recovers on restore; Vanilla
host idle. Suites pass; Python 126 OK. Bonus levels were not played.
[Note 126](documentation/126-d08v1-control-loss.md).


**Todo. User report, 2026-10-07:** "where i missed a jump, and somehow duke
never went back into modern controls mode, and ive lost the ability to control
him. this used to happen in some of the earlier builds, but we must basically
make sure that modern controls take precidence over everything, aggressively
hooking so this doesnt happen, because theres essentially no way back other
than to quit the game and reopen it i believe."

**Testbed:** the user's session log, copied before rotation to
`recomp/analysis/control-loss-20261007/session-20261007-195012.log` (binary
`3c0ca76e...`, the D08J5 baseline). Use dated private copies of the player's
saves only; the user's slot 3 (chain) is the nearest savestate.

**First evidence (read-only look at the log and the live game, 2026-10-07):**
- After several D08J5 chain mounts and climbs, a W + E jump caught the chain in
  the air (148 -> 154). At that moment the log shows `[TTK identity] guard 52
  (0x800c2a8c, 504 bytes) changed at 0x800c2b34: 0x00000021, expected
  0x00000061`, then `WASD tank fallback (... state)`, Duke climbed out with the
  top exit 196, and from then on every line is `[TTK lease] inactive
  (identity)` / `ORIGINAL MOVEMENT (identity)` until the user gave up.
- Live RAM (game still running, read through the debug port): the only
  difference from the executable in that guard is that byte. `0x800c2b34` is
  entry 196 of the original upper-body animation flag table (`0x800c2824 + 4 *
  anim`); 196 is the pole/chain top exit. The original toggles bit 0x40 of these
  entries at run time (already known for 149/152/153, excluded from the guards
  for that reason). So a game-state write was treated as tampered code, and
  `identity()` turned the whole Modernized lease off for the session with no
  recovery path.

**Scope:**
- Find every original writer of that table (`0x80055438` object-hang clear,
  `0x8004c8b8` ledge-hang set, and whatever touched 196 in a chain catch), and
  exclude runtime-written entries from the code guards (or guard the table
  with the toggled bit masked), game-wide.
- Make modern controls robust: a data-table or other non-code mismatch must
  never permanently drop the lease; identity loss should be re-checked and
  recover when the bytes return or prove to be state; distinguish real code
  tampering (fail closed) from known game-state tables; never leave the player
  with neither modern nor usable original controls; show what happened.
- Verify in a reproduction of the missed chain jump (slot 3) and audit the
  remaining guards for other runtime-written data.

**Acceptance:** the slot-3 missed jump / chain catch / top exit sequence keeps
Modernized controls; no guard trips on game state across the tested climbs,
hangs and exits; a forced identity loss recovers or degrades to working
controls rather than locking the player out; Vanilla unchanged; the user
confirms.

### D08W - Subway shallow-water sideways jumps (A/D + Space jumps forward)

**Done (2026-10-01, user-accepted).** Wade jumps now go in the held
camera-relative direction; the old boost kept the original launch's facing.
See [cause and evidence](documentation/74-d08w-wade-jump-direction.md).

**User report (2026-09-30):** in the shallow water of the first subway area,
running forward and then strafing (A or D, with Shift to run) and pressing
Space always makes Duke jump **forward**. A sideways jump is not possible
there at all. The user noted this to fix later.

Context to check first: D08C made directional jumps work from standstill and
while moving on dry ground. The crystal-2 flooded turret corridor wade (see the
Modernized movement notes in the manual) uses its own wade handling. The
subway's shallow water probably takes another wade/shallow-water path whose
jump uses Duke's facing instead of the held move direction, or where the
strafe input is not carried into the jump. Reproduce on a private savestate at
the start of the subway water. Never use the player's own slots.

**Acceptance:** in the subway's shallow water, W/A/S/D (with and without
Shift) plus Space jumps in the held camera-relative direction, with the same
feel as a dry-ground directional jump (D08C, D08I), including a pure
sideways jump while running forward and then strafing. Forward jumps and
original water behavior (slowdown, splashes, sounds) stay intact. Check the
crystal-2 wade for regressions. Vanilla is unchanged.

### D08X - Hold-E airborne ledge grab and mantle

**Done (2026-10-01, user-accepted).** Crate mantle mid-jump added for the boxes room. Ledge grabs accepted by the user ("the confidence level ... is 100%" from below a ledge). Follow-up requested: a Shift-run jump with E held into a box in the boxes room (user UI slot 11) bounces off instead of mantling onto it mid-jump. Original note: E held through a jump opens the original
reach at takeoff instead of at its late animation event, so a close ledge in
reach is caught instead of bounced off. See
[rules, change and evidence](documentation/73-d08x-ledge-grab.md).

**User request (2026-09-30):** in Modernized, holding **E** while jumping
toward a mantle-able ledge at grabbing height should make Duke catch the
ledge and pull himself up, the same way run-jump + E already catches a ladder
(D08J). The user wants it to feel fluid and modern, and to work with a high
degree of confidence: if the ledge is in reach and E is held, Duke grabs it.

Context to check first: the original game already has ledge hang and mantle
animations (134-142) and the 6/7 hanging dispatches, guarded separately from
camera-relative movement (see `documentation/26-d08-movement-followup.md`).
D08O reached a mantle from deep water with E, and D08V covers the sewer
mantle lease flicker. Trace the original ledge eligibility first: which
heights, distances, facing and velocity windows the original grab accepts,
whether it is automatic or button-driven, and why a Modernized jump misses
it (camera-relative facing, held move direction, speed, or the grab window
timing). Prove the failing path before choosing a fix; use the original
hang/mantle routines and animations rather than inventing a new climb.

**Acceptance:** in Modernized, a standing, walking or running jump toward a
ledge at grabbing height with E held (pressed before or during the jump)
reliably catches the ledge and mantles up with no pop, clip, damage or camera
snap, in third and first person. Grabbing works while armed, with the
automatic stow/redraw of D08J. E held into a ledge that is out of reach, or
into a wall with no ledge, does nothing odd; jumping without E keeps the
current behavior. It must not grab through walls or from remote distances,
and it must not break ladder grabs (D08J, D08U), pushable-object grabs
(D08T), ordinary E targets, edge run-off (D08L) or jumps (D08C, D08I).
Record grab success over repeated attempts on several representative ledges
(apartment exterior, sewer slot-12 area, crystal-2) and list any ledge type
that is not covered. Test only on private savestate copies. Vanilla is
unchanged.

### D08Y - Gap jump dead band: jump-mantle level-geometry ledges (slot 5 gap)

**Done (2026-10-01, user-accepted):** the slot-5 jump is made consistently, and
"the stuttering and audio issues, and freezeframes are now fixed" (round 5
build). Round 4: The remaining random freezes
(also without jumping) were the emulated PlayStation CPU running out of budget in
views the Modernized camera and widescreen reveal; Modernized now runs the
emulated CPU at 150% (setting, Vanilla stock): 144/59 slow frames to 0 on the
player's GPU. Round 3: Freeze frames during E
reaches (every reach frame ~50 ms) fixed with a per-update retry budget and
cheaper isolated calls (now 33 ms like any frame); the jump-mantle pop (a
~400-unit snap) is now a short glide. Round 2: root cause found: Modernized (D08I)
had switched off the original's run-jump postponement, which launches a jump
pressed near a gap from the lip; the levels are built around it. Before a large
drop (over 768) the original now decides again, and a tapped Space is held for
the queued jump; furniture and steps keep the immediate jump. Slot 5 without E
8/10 clean landings (was 0/10), E 14/14 across. Playtest 1 hitch (up to 27 ms
per reach update) fixed by cheaper isolated calls and fewer D08Y retries (~6.5
ms). D08Y jump mantles stay as the safety net. See
[research, change and evidence](documentation/77-d08y-gap-jump-research.md).

**User report (2026-10-01):** in UI save slot 5 Duke faces a gap to a second
platform at the same height. A running jump from the end of the walkway
rarely makes it: Duke "comically hits his head on the other platform and just
falls down". Starting the jump 3-4 feet earlier with E held catches the edge
and mantles. The user sees this as a level-design oversight and wants high
confidence on this jump.

Research (private copy of slot 5, real keys): without E the jump never crosses
(0/10; from the edge his feet are only 34..57 below the far top when he hits
its face, and the original bounces). With E, takeoff 0..200 units before the
edge lands across and 700..900 catches the ledge, but 300..600 is a dead band:
feet 270..540 below the top, too low to land and too high for the hang
acquisition, so he bounces whatever he holds. D08X's mid-air height mantle
covers only climbable objects (crates), not level-geometry ledges like this
one.

Planned approach: extend the D08X jump mantle to geometry ledges (the original
ground mantle heights 134..137/139, after the original line-up), and consider a
Modernized low-lip step-up landing (top at most about 0x100 above the feet) so
the from-the-edge jump without E also lands. Lowered hang retries are the
fallback.

**Acceptance:** on a private copy of slot 5, running jumps with E held from
any takeoff point 0..900 units before the edge end on the far platform (land,
mantle, or catch and pull-up), holstered and armed. Without E, at least the
from-the-edge jump no longer bounces off a lip of under about 0x100 if the
step-up is adopted. No false mantles onto tall walls. Slot-12 wall grab,
crate mantles, ladder transfers, ladder-top mount and D08L edge run-off are
unchanged. Vanilla is unchanged. Generic rule, no slot-5-specific data.

### D08Z - Optional manual modern jump (player-timed takeoff, air control)

**Done (2026-10-01, user-accepted: "i love it. lock it in").** `run.py --jump manual` (Modernized; assisted
stays the default): Duke leaves on the press everywhere, a jump pressed up to
~0.15 s after running off an edge still launches, WASD steer in the air
(camera-relative, never above the takeoff speed, also during the E reach), and
standing/walking jumps leave the ground ~3x sooner (5..8 fields instead of
14..18). Slot-5 gap without E: lands across when pressed in the last ~200
units or up to 10 fields after the edge, low-lip scramble when 200..600 early;
with E a mantle from 1000 back. Regression routes match assisted. See
[measurements, change and evidence](documentation/78-d08z-manual-jump.md).

User (2026-10-01, at selection): the long jump feels "on the rails", "not me",
"off in the hand"; wants to explore options for a much more modern feel.

**Original backlog note.** User: the D08Y jump makes the slot-5 gap
reliably, but it feels "on the rails", which is how the original intended it.
The user wants, later, a more manual, modern jump as an option.

Context: the original postpones a run jump pressed near a gap and launches it
from the lip (`0x80078c0c` look-ahead, `+0x228` bit 4; restored for large drops
in D08Y), its flight follows a fixed ballistic arc, and D08X/D08Y add mid-air
ledge help. A manual jump would take off on the press everywhere, and probably
allow some air steering, while the levels' gaps are tuned for the lip launch,
so a player-timed takeoff must still be able to clear them (for example a short
coyote window at the lip, or a slightly longer arc only in this mode).

**Acceptance:** a selectable jump style in Modernized (the D08Y lip launch stays
the default unless the user decides otherwise). In the manual style the jump
leaves on the press, has bounded air control, and the slot-5 gap and other
measured gaps remain clearable with reasonable timing. No double jumps,
no wall clipping. Vanilla is unchanged.

### D08Z1 - Keep jump momentum when bumping a wall (EDuke32-style air control)

**Done (2026-10-08, user-accepted: "mechanically this feels significantly better, where we can call the actual job as accepted.").** Follow-ups split out as D08Z2
(mantle pose regression) and D08Z3 (stuck after some landings).

Was Needs playtest (2026-10-08). Modernized `jump_walls` = `slide` (default;
`run.py --jump-walls original` restores the bounce; Vanilla unchanged). Before
each airborne update the host runs the original integration and sweep in
isolation; a wall only loses the velocity into it, a ceiling stops the rise,
so Duke keeps his arc and slides along with no 107, rumble or sound. Ledge
grabs, jump mantles and the low-lip scramble start from the contact instead of
the bounce. Evidence: no 107 in 48 wall jumps over three levels (8 of 16 bounced with
`original` where compared), D08X/D08Y/D08U/D08W/D08Z routes unchanged. Revision 1
(slot-6 monument corner still bounced): corner turn search and an original
bounce safety net. Executable `3f909c9f2cc4e5b09ba6faab5129ec9ae0ff938c4735e264ac67a66405c1ed34`. [Note 134](documentation/134-d08z1-jump-wall-slide.md).

**User request, 2026-10-06.** "i dont want duke to lose his motion when
you bump into a wall when jumping, i want that behavior to be more like eduke
also, duke should be able to jump into a wall without consequence." Today a
jump that touches a wall cancels Duke's travel (wall-touch cancel and the 107
bump seen during D08Y/D08Z), so he drops and loses the jump.

**Scope (Modernized, optional):** in the air, wall contact only removes the
velocity component into the wall; Duke slides along it and keeps his height,
arc and the remaining horizontal motion, with no bump/stagger animation and no
lost control, as in EDuke32. Research first: where the original airborne
handlers detect wall contact and zero or reverse velocity, which animations it
triggers (107 and others), how this interacts with the D08X/D08Y ledge grab
and jump-mantle (a wall with a reachable ledge must still be grabbable), with
D08Z manual jump air control, and with ceilings and steep slopes. Make it a
Modernized setting, on by default unless the user decides otherwise, so it can
be exposed in the future menu (D19); off restores current behavior.

**Game-wide:** the shared airborne collision path, every level.

**Acceptance:** with the option on, standing and running jumps into straight
and angled walls keep their arc and slide along the wall in at least two
levels; ledge grabs and jump-mantles still work; ground wall bumps, swimming,
jetpack and Vanilla unchanged; option off matches today; the user confirms.

### D08Z2 - Mantle pose regression since D08Z1

**Done (2026-10-08, user-accepted: "accepted!!").** Cause: the D08Y low-lip step-up (no E) could
start armed, and since D08Z1 it starts from every wall slide. The original
mantles only with a free upper body (`0x80051cf0` refuses on upper-table bit
8) and sets only the lower animation. So the legs played 134 while the upper
track held the static weapon-ready pose 20. Every jump mantle now starts in one
place (`jump_mantle_start`). A weapon pose on the upper track is handed to the
mantle and its block put back when the mantle ends, so the weapon fires again.
Holstered E mantles, the slot-12 grab and pull-up, and the crate mantles were
already full body and match `original` frame for frame.
Revision 1: the D08Z1 safety net's late 107 undo left 107's leg joints in
track 3's joint mask, freezing the legs (also in savestates); the undo now
happens at the bounce's contact sound (new hook `0x8006BBD8`).
[Note 135](documentation/135-d08z2-mantle-upper-body.md).

Was Todo. User report, 2026-10-08, at D08Z1 acceptance:** "this has also
introduced a regression where his mantling animation is now some static pose
with his legs and looks weird." Next job by the user's choice.

Leads (not yet investigated): D08Z1 starts the D08X jump mantle, D08X/D08Y
lowered retries and the D08Y low-lip step-up from a recent wall slide
(`jump_wall_recent()`) as well as from the first updates of a bounce 107. In
those cases Duke enters the mantle (134..140) from a flight animation
(98/103/104/109) instead of 107; the mantle starts with `+0x68/+0x6a` reset,
but the upper body or the D08Z1 safety net's 107 undo (which restores
`+0x60/+0x68/+0x6a` at the next runner call) could leave a stale pose. Also
check whether walk-in (ground) mantles are affected, and `jump_walls original`.

**Acceptance:** every mantle (ground walk-in, jump mantle onto crates and
ledges, low-lip scramble) plays its full original animation in Modernized with
`jump_walls slide`; Vanilla and `original` unchanged; the D08Z1 slide and
corner behavior stay as accepted; the user confirms.

### D08Z3 - Stuck for a moment after some landings (modern landing)

**Done (2026-10-08, user-accepted: "Completely accept this work as complete, working, and done.").** Modernized `landing` = `modern` (default;
`run.py --landing original` restores it; Vanilla unchanged). Cause: the
original's own recovery 105, not D08Z1. The landing selector `0x80054c04`
continues the gait only after a directional/running flight with a direction
held at the touch-down; a straight-up jump, a released direction or a bounce
lands in 105, which takes no input for about 37 fields (0.6 s), also in
Vanilla; a jump press in it is lost. Now, at the ground dispatcher
`0x80048410`, a held direction starts walk 72 / run 76 from 105 in the update
after the touch-down, and a jump press with no direction hands 105 to the
stance 63 so the buffered jump fires. Idle landings keep 105; 106 (damaging
fall), water entries and the continuation landings are unchanged. Evidence:
UI slots 7 and 10, `original` 36..44 fields before moving, `modern` 3..8;
landing hop airborne 12 fields after the touch-down (4/4); W-held, strafe and
running jumps identical; D08X/D08Y/D08U/D08W/D08Z routes same outcomes.
Profile schema 31. Executable `11187493c04ae830482c47c93b97ac332fbfce1de95f8eedaa2f8637d7fe164e`.
[Note 136](documentation/136-d08z3-modern-landing.md).

User, at selection (2026-10-08): "it may not actually be the bump causing it
because it happens also when you jump without run on for example. if you jump
forward, when you land, there's that horrible pause when you land before duke
starts moving. it's all clunk. I actually want that entire mechanic to be
totally smoothed out and modernized so that the flow feels much more natural".

Was Todo. User report, 2026-10-08, at D08Z1 acceptance:** "there is the
occasional jump where duke lands and he's stuck for a second before being able
to move again."

Leads (not yet investigated): a landing that follows wall slides or a corner
turn; the safety net's restored flight animation meeting the landing; the
landing animation the original picks (`0x80054c04`) after a slide (a hard
landing such as 127/128 holds input for a while). Reproduce first (which
level, jump style, wall or none), compare `jump_walls original`.

**Acceptance (widened at selection):** in Modernized no ordinary landing
holds Duke still when a direction or jump is pressed (straight-up,
directional, running and released jumps); landings feel natural; option off
and Vanilla keep the original; reproduction route documented; the user
confirms.

### D08Z4 - First person briefly cuts to third person when jumping into the cop car (UI slot 6)

**Done (user-accepted 2026-10-20).** User report, 2026-10-08: "slot 6. where im standing right in front
of the cop car, and by jumping into it, it briefly forces duke to go into third
person mode when im playing in first person mode. ... this bug is by no means
game breaking, but it would be nice to investigate it and see if we can make it
more robust so this doesnt happen."

Leads (not yet investigated): `first_person_update()` in
`recomp/src/ttk/first_person.inc` blends to the orbit whenever
`traversal_camera_early()` is set without a committed move (D08V: mantles,
hangs and unowned falls 107/108 show from the orbit). A jump into the car's
side may start one of those for a few frames: the D08Y low-lip step-up or
D08X jump mantle onto the car body, a D08Z1 safety-net catch whose 107 is
undone after the camera already saw it, or a short unowned fall off the car's
edge. Reproduce on a private copy of UI slot 6 with the user's settings (first
person, manual jump, `jump_walls slide`), log `fp_reason` and the animation
per frame, and compare `jump_walls original`.

Fix the system, not the car: first person should only leave the eye when the
eye would really be inside geometry (a full mantle or hang), and brief or
cancelled traversal states (a bump, an undone 107, a few-frame fall, a
step-up onto a low object) should keep the eye or hold off before blending.

**Acceptance:** in Modernized first person, jumping into the slot-6 cop car
from several headings and gaits (standing, walking, running; manual and
assisted jump) never flashes the third-person orbit; real mantles and hangs
still read correctly (keep the eye if it stays clear, otherwise the existing
orbit); D08V sewer mantle, D08X/D08Y/D08Z1 routes and third person unchanged;
Vanilla unchanged; the user confirms.

**2026-10-20 - Needs playtest.** Reproduced on a private copy of UI slot 6
(file `slot05`, SHA-256
`724c3cb83d252f586ed2c9583e5890b7fdf3cbb171a80eea8905da5a138de272`) with the
player's settings (first person, manual jump, `jump_walls slide`), Xvfb, real
keys. A short run-up jump lands on the sloped car body, where the original
steep-slope slide takes over: 145 (mode 2), 108 (9/2), 105 (0/9), then 108
with mode 9 / previous 0 and 9/9. The D22B slide latch keeps the eye only
through 145 and the first landing; the later 108 frames counted as D08V
unowned falls, which first person sent to the orbit, and the drop off the car
was shown from behind (blend down to 0 for about 0.7 s). Cause: D08V grouped
falls with mantles and hangs ("the eye would be inside the ledge"), but a fall
is the owned flight's own 107/108 animation and the eye already stays there for
owned jumps.

Fix (game-wide, `first_person.inc`): unowned falls (107/108) stay in the eye
view, like committed moves. Mantles, hangs, ladder mounts and climbs keep the
orbit; leases, input and third person are unchanged.

Evidence, executable `117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778`
([note 168](documentation/168-d08z4-falls-keep-eye.md)): before the fix the
run-up jump fell to `unsupported` (blend 0.0); after, 40 jumps (standing,
walking, running at five run-up lengths; straight on, turned left and right;
manual and assisted jump; `jump_walls` slide and original) never leave
`active` or drop below blend 1.0, including slide, fall and 107 bounce
frames. E-held run-ups still mantle onto the car (137, mode 8) from the orbit.
Mid-slide and mid-fall captures show a clear eye over the roof. Third person
never blends in; Vanilla's first-person code stays idle. `ttk-controls-test`
(with LEVEL01 and all levels), `ttk-aim-test`, `ttk-input-test` PASS.

Limits: tested at the slot-6 car only. Other unowned falls (off edges, failed
grabs) take the same path but were not driven here. A thin black wedge at the
top-left edge in some fall frames looks like the D17R sky-edge issue and is not
part of this change. The user has to confirm the feel.

**2026-10-20 - Done (user-accepted).** User: "thats the one! lock it in and
close it out!!". Executable
`117653dd0e38fa1c6cac2efe6e885449990ee1a5ffeaf23523024587a22e3778` is the regression baseline.

### D08Z5 - Wall Backflip option: Modern (slope jump that climbs) / Original

**Done (user-accepted 2026-10-21).** Opened 2026-10-21. User request: Duke's
automatic wall backflip is a legacy movement mechanic that gets in the way of
precise platforming, worst with the Modernized first-person controls. Make it
optional with a new ADVANCED setting, Modern by default.

User report: "When Duke jumps against certain walls or obstacles, the original
game can automatically trigger a backflip. In third-person mode, this is
visually apparent. However, in first-person mode, the animation is barely
noticeable, while the associated backwards displacement still occurs." During
consecutive jumps against a wall, or when climbing onto a raised surface, an
accidental backflip can throw Duke several feet backwards.

**Reproduction (player UI slot 10; check which file it maps to, e.g. file
`slot09`, and work only on a private copy):**

1. Load UI slot 10: Duke stands directly in front of a police car.
2. Enter first person.
3. Hold Shift to run.
4. Press W to move toward the police car.
5. Jump into the car at the right angle.
6. Duke does an automatic backwards backflip away from it.

Use the player's settings (Modernized, first person, their JUMP and JUMP WALLS
choices) and also repeat in third person, with `jump_walls original`, and in
Vanilla for reference.

**Relation to earlier jobs (read first).** This overlaps work already done:

- D08Z1 ([note 134](documentation/134-d08z1-jump-wall-slide.md)) added JUMP
  WALLS (`controls.jump_walls`, `slide` default) to stop the original
  airborne wall response (bounce animation 107, reflected velocity at 0.25..1.0
  of the speed, rumble, sound `0x2008`) in the running/directional airborne
  handler `0x80055904`. Its safety net undoes a 107 that still reaches
  `0x8003ef08` / `0x8003ef78` at convex corners, but that update's push has
  already been applied, and hits with a near-vertical normal are left to the
  original. So either the backflip comes from a path D08Z1 does not cover, or
  the slide or safety net misses it here.
- D08Z4 ([note 168](documentation/168-d08z4-falls-keep-eye.md)) found that a
  run-up jump at the UI slot 6 cop car lands on the sloped car body and the
  original steep-slope slide takes over (145 in mode 2, then 108, 105, 108).
  That fix keeps first person in the eye through those frames, which may be
  why the backflip is "barely noticeable" in first person while its
  displacement remains. Find out whether the backflip is 107, the 145
  steep-slope slide, a ceiling hit (sweep result 3), a landing/recovery
  animation or something else.
- D08Y/D08X/D08Z3 ledge helpers and the modern landing also start from wall
  contacts (`jump_wall_recent()`), so changes here touch them.

**Investigation (before implementing):**

- Where the backflip is triggered: handler, sweep result, animation number,
  and whether it is in Duke's mode 9 airborne path, the slope/landing path or
  another.
- What collision conditions start it (surface normal and slope, angle of
  approach, height of Duke's feet against the obstacle, speed, gait, manual vs
  assisted jump).
- What moves Duke back: animation root motion, a velocity write (reflection,
  slope slide), a position correction, or a mix.
- Whether it differs between first and third person (presentation only, or
  also behaviour).
- Dependencies: D08Z1 slide and safety net, D08X/D08Y mantles and step-ups,
  D08Z3 landing, D08L edge run-off, D08Z4 first-person falls, swimming and
  jetpack paths, scripted moves.
- Whether recent movement and collision changes (D08Z1 to D08Z4, D22B) changed
  when it fires; compare with `jump_walls original` and Vanilla.

**Proposed setting:** ADVANCED > MOVEMENT, **WALL BACKFLIP**: **MODERN**
(default) / **LEGACY**, following the D19N menu conventions and persisted in
the profile like JUMP WALLS (schema bump with migration, `run.py` flag,
`--settings` entry, environment override, Vanilla always Legacy). If the
investigation shows the backflip is the same mechanism JUMP WALLS already owns,
propose to the user whether to fold it into JUMP WALLS instead of adding an
overlapping row; do not ship two settings that fight each other.

- **Modern:** jumping into a wall, vehicle or other obstacle never starts the
  automatic backflip or its forced backwards movement. Normal collision,
  gravity and jumping stay; no wall climbing, no clipping into geometry, no
  other movement change.
- **Legacy:** the original behaviour exactly, animation and movement.

Fix the system, not the car: the rule must hold for every wall and obstacle in
every level.

**Acceptance:**

- The backflip reproduces reliably from UI slot 10 (private copy) before the
  change, and the cause is documented.
- Modern: no automatic wall backflip and no backwards displacement at the slot
  10 car from several headings and gaits (standing, walking, running; manual
  and assisted jump), and at other walls and obstacles in other levels.
- Legacy keeps the original behaviour; Vanilla is unchanged.
- Works the same in first and third person.
- The setting persists across sessions (profile, menu and CLI agree).
- Normal jumping, collision and platforming unaffected: D08Z1 slide routes,
  D08X/D08Y mantles and step-ups, D08Z3 landings, D08Z4 cop car first person,
  D08L run-off, ladder, swim and jetpack routes; native tests pass.
- No regression to the modern movement controls; the user confirms in play.

**2026-10-21 - Needs playtest.** Reproduced on a private copy of UI slot 10
(file `slot09`, SHA-256
`467a8934092b3b717b27736a2c866a8321904f9c26b7495b9a7050f5391a90b0`) with the
player's settings, first and third person: every run-up jump of 0.1 to 0.6 s
ends 525 to 812 units behind its furthest point. Cause: not a wall response
(no 107, no D08Z1 slide). Duke lands on the car body, a floor 40..70 degrees
steep; the ground step `0x80055e80` enters the steep-slope slide (mode 2, slip
pose 145, sound `0x2006`, handler `0x80054568`). When the face ends, the fall
`0x8003f060` takes a launch from the table `0x800c27c8` by the pose (145: 84
units straight back from Duke's facing, about (-3040, 0, -380) velocity), and
that launch, not the slide (about 70), carries him back. JUMP WALLS cannot
reach it (airborne walls only), so this is a separate row.

Change (`wall_backflip.inc`, game-wide, no geometry case): Modern keeps the
slide and zeroes that launch, only for falls started by the slide handler
(ra `0x800546c4/dc/ec`, mode 2), before the flight start rotates it
(`0x80066e34`, ra `0x8003e488`). Duke drops where the steep face ends. Setting
ADVANCED > MOVEMENT > WALL BACKFLIP MODERN (default) / ORIGINAL (the row
convention; ORIGINAL is the "Legacy" of this request), profile schema 37
`controls.wall_backflip`, `run.py --wall-backflip`, `--settings` choice P,
`DNTTK_WALL_BACKFLIP`, Vanilla always original. New entry hooks `0x8005446C`
and `0x8003F060` (regenerated, codegen hash unchanged, savestates load);
guards for `0x8005446c` and `0x80054568`.

Evidence, executable `191646ef67d6890ec435ddc771a9f31c9da349eb6e83d8a8c5b341860a4d3647`
([note 174](documentation/174-d08z5-wall-backflip.md)): slot-10 car, back for
0.1/0.2/0.4/0.6 s run-ups, Original 733/525/812/525 against Modern
280/70/410/160 (first person), 132/70/410/141 (third person), 218/70/394/141
(assisted); turned left, Original 625/594/0/597 against Modern 199/322/0/333.
Slot-6 car: Original 665/580/669 against Modern 140/70/137. Every Modern
slide was followed by a stopped launch. The rest is the slide down the panels
and the straight drop. D08Z1 regression routes in both modes: no slope slide
on any route, outcomes as before. OPTIONS row toggles live and persists;
Vanilla reports original. Python 162 OK; `ttk-controls-test` 49 groups (new
D08Z5 group), `ttk-aim-test`, `ttk-input-test` PASS.

Limits: only the two level-0 police cars were driven. The slide itself stays
(70 to 410 units at these cars). The slip pose 145 still shows briefly in
third person; a pose swap is the next step if it still reads as a flip.
Separate finding, not investigated: the player's slot 10 file halts the
emulator on some reloads in one session (GPU GP0 unknown command; always on a
second debug-port load, once on a third LOAD GAME). The user has to confirm
the feel.

**2026-10-21, first playtest: the backflip was still there; fixed.** User:
"this absolutely doesnt seem to have provided any kind of fix. as you can
see, the backflip still happens, third person or first person. there is even
a chance duke will slip out of first person" (video
`Video_2026-10-21_17-06-55.mp4`). The session logs showed animations 99/100,
which the first delivery never touched. The slope slide handler's jump branch
(`0x800547ac..0x80054834`): with Jump (Square word `0x800d1b88`) pressed
while Duke slides, `0x8005446c` picks 99 (100 after a 99) when Duke faces up
the slope, 98 when he faces down it. 99/100 is the backflip; the branch also
runs after the handler's fall start in the same update, so it begins in mode
2 or 9. Reproduced by mashing Space into the car: Original 2386 to 2449 units
back. The first harness held Space only 0.15 s before the landing and never
hit it. Change: the facing test from that branch (ra `0x800547fc`) arms
`wall_backflip_jump`; at the next initialization from the lower-body update
(`0x800493a4`, ra `0x8005a3a0`/`0x8005a4d0`) a 99/100 becomes 98, the
original's own slope jump, in Duke's facing (new plugin, no new hook entry).
A standing jump's 99/100 (Down at `0x8005492c`), ORIGINAL and Vanilla keep
theirs. `slide_state_early()` also covers 99/100 at mode 9 from 0 (enter,
slide and leave in one update), which dropped the lease and first person in
ORIGINAL.

Evidence, executable `cd458c38c3ab3b22658ef1b0896fc3bed36df2cf02a8976ce391d2514bd261d2`: slot-10 car, Space mashed for 1 s after a
0.1/0.2/0.4 s run-up jump: Modern third person 12 runs and first person 6
runs, no 99/100, 17 rewrites (`ttk-wall-backflip jump`, mode 2 and 9), no
lease loss; Duke often lands on the car. Back distance at most 70 in third
person, up to 487 in first person where repeated 98s land high on the roof
and slide down the panels again (slide only, no launch). ORIGINAL first
person: 99 kept (2386/2414 back), no lease loss. Native D08Z5 group extended
(armed 99/100 at mode 2 and 9, standing 99, 98, other caller, original,
Vanilla); `ttk-controls-test` 49 groups, `ttk-aim-test`, `ttk-input-test`,
Python 162 OK (2 skipped), `check_repo.py` OK. The D08Z1 regression routes
were not rerun: the rule acts only after the slide's jump branch, and none of
those routes slides.

**2026-10-21, second playtest: the slope jump now climbs.** User: "i dont like how this feels ... i prefer the idea of when jumping again, duke is actually trying to jump up a gradient slope a little higher ... What we've got right now is that dukes jump feels almost cancelled out".
Cause: the 98 that replaced the backflip never left the face. Duke takes off
pressed into it, so the airborne sweep `0x8007a98c` returns 2 (floor) on the
first update even while he moves away; the handler then moves him only
sideways and picks a landing pose (`0x80054c04`), and the next ground step
slides him again (no rise at all in the traces). From the bottom of the face
the fall has already started and 98 keeps the fall's velocity (no rise).
Change (`wall_backflip_climb`, own plugin at `0x8003ebf4`, ra `0x80055934`,
before D08Z1): on the jump's first airborne update Duke keeps the rise
(-6917; given to the bottom-of-face case too) and drifts straight uphill
(`+0x2bc` + 180 degrees) at k * rise / tan(face), face = -`+0x2be` (51.5
degrees at this car), k 0.25 (`DNTTK_SLOPE_JUMP_K`, diagnostics). While he
rises (98, mode 9, upward velocity, at most 40 updates) each update is tested
with isolated original integrations and sweeps, restoring Duke: the drift;
if that reports floor, straight up; if that does too, lifted 24/48/96/160
units, whichever the sweep finds free. Coming down, the face lands and slides
him as in the original.

Evidence, executable `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da`, slot-10 car, third person, Space tapped every
0.15 or 0.3 s for 3 s after a 0.1/0.2/0.4 s run-up jump: jumps rise about 430
units and land 90 to 390 higher on the face; 3 of 6 runs ended standing on
the car, the others slid back off. First person (tap 0.2 s): no lease loss.
The sweep log shows drift blocked, straight up free on the first update.
Native D08Z5 group extended (drift and rise from the face's tilt, the
bottom-of-face rise, another caller, once per jump); `ttk-controls-test` 49
groups, `ttk-aim-test`, `ttk-input-test`, Python 162 OK (2 skipped),
`check_repo.py` OK. Not rerun: the D08Z1 regression routes (the rule acts
only after a slope slide's jump branch).

**2026-10-21, accepted.** User: "perfect!!! lock it in close it out." D08Z5
Done; executable `4114333b209e8f1797ef6b4cf12b4898f97f22ee4d379c0c6b0659a3223b82da` is the regression baseline.

### D08Z6 - Fluid forward jumps onto tall steps/ledges from a blocked run

**Done (user-accepted 2026-10-24). Revision 2 accepts the first Space press during the grounded run stop and builds forward speed after clearance. [Note 218](documentation/218-d08z6-blocked-run-jumps.md).**
Use the player's current **UI slot 10**, with a private test copy when selected.
The tall step/ledge directly ahead produces a stiff, awkward jump with very
little forward inertia or freedom of movement.

**Reproduction:**
1. Load UI slot 10 and approach the step in front of Duke, preferably close
   to the ledge's **right-hand corner**. The user reports that this enhances
   the effect, although it may occur elsewhere along the ledge.
2. Hold **Shift**, then hold **W**, and run directly into the step.
3. Keep Shift + W held until Duke has stopped moving against the step.
4. While still holding both keys, press **Space**.
5. Observe Duke rising mostly vertically with very little forward carry onto
   or over the ledge. The jump feels rigid rather than fluid and modern.

**Goal:** make a jump initiated while pressing forward into a tall step feel
responsive and fluid, with useful forward movement as clearance becomes
available above the lip. Investigate the blocked-run takeoff, forward input/
velocity handling and corner collision before choosing a fix. The cause is
unconfirmed; do not assume it is the existing wall-slide or mantle behavior.
Compare the right corner with the middle/other approaches and ordinary open
running jumps. Record the active jump/wall options from the reproduction.

**Acceptance:** the slot-10 reproduction, especially near the right corner,
feels naturally forward-moving and controllable to the user. No clipping
through solid geometry, unintended automatic vault, excessive launch, or
regression in ordinary jumps, wall slides, mantles and landings. Verify both
camera views and representative other ledges. Vanilla remains unchanged.

### D08Z7 - Backward jumps use the forward jump's technique (UI slot 7)

**Status: Done (accepted by the user 2026-11-03, revision 2; see the work log above and [note 368](documentation/368-d08z7-backward-jumps.md)). Dependencies: D08Z, D08C, D08Z3.** User report: load UI
slot 7 and jump backwards, holding Shift and pressing S then Space (or just
holding all three): Duke ends up doing a forward jump. The user wants the
backward jump to work the same way as the modern forward jump: a natural
first-person-shooter hop in the direction pressed that may or may not gain
a little height, not a different technique.

Initial research (2026-11-02, private copy of UI slot 7, file slot 6, the
player's profile: manual jump, first person, lab
`recomp/analysis/d08g8-cheats-anywhere/probe.py` case `dj2`):

| Input | Animations | Peak | Takeoff velocity |
| --- | --- | --- | --- |
| W then Space | 98 (repeats while held) | about 320 | (4351, -4142), forward |
| S then Space | 98, then 106 / 103 after falling | about 310 | (-4351, 4142), backward |
| Shift+S then Space | 94, 96 (standing preparation), 98, 106, 103 | about 310 | (-2013, 4142) |
| Shift+S+Space together | 98, 78/94/96, 103 | about 340 | (-4351, 4142) |
| Space alone | 96, 97 (standing jump) | about 1060 | (1977, -1883) |

Duke's heading (`+0x1c`) never changes; the backward jump moves backwards
but plays the forward-jump body animation 98, and with Shift held from a
standstill it first plays the standing jump preparation 96. Behind Duke in
this save is a long drop, so held keys chain into a fall, hard landing 106
and running jumps 103 (anim 103 is the running forward jump). Not yet
established: which of these the user sees as "a forward jump" (the 98 pose
while going backwards, the 96 wind-up, or the 103 running jump after the
drop); reproduce with the player's real taps and holds and a session log
first, in first and third person.

Work: make the backward (and, for consistency, sideways and diagonal)
directional jump take off exactly like the modern forward jump (D08Z manual
and the assisted style): same quick takeoff, the same small, natural arc in
the pressed direction, no standing-jump wind-up and no switch to a running
forward jump; decide the body animation for a backward hop (for example a
neutral or backward-leaning airborne pose rather than the forward-leap 98).
Game-wide rule, not slot 7 only.

**Acceptance:** at UI slot 7 and elsewhere, Shift+S then Space and holding
Shift+S+Space give a backward hop like the forward jump (same takeoff
timing and height behaviour, moving backwards, no forward-jump or running
jump technique); forward, sideways and standing jumps unchanged unless the
user asks; manual and assisted jump styles; first and third person; Vanilla
unchanged; the user's playtest.

### D08Z8 - Mantling while firing the Desert Eagle: held fire stops the mantle

**Status: Done (user-accepted 2026-11-07, revision 7: "i accept it"; [note 408](documentation/408-d08z8-mantle-while-firing.md)). Dependencies: D08Y, D08J8.** User,
2026-11-07: "when you mantle something while actively shooting with the
desert eagle, duke cannot mantle it and you have to stop shooting. but
otherwise you can mantle and shoot at the same time if you stop, mantle, and
then shoot." Repro: run up to the post in UI slot 8 and mantle it while
holding fire. Fire held before the mantle starts blocks it; fire pressed after
the mantle has started works (D08J8 mantle fire).

Quick research (2026-11-07, about 5 minutes, code reading only, not
reproduced yet):
- The bounce mantle assist (`recomp/src/ttk/ledge_reach.inc`, `++step_ups`)
  returns early when `player+0x224 & 0x241`, and the jump-bounce mantle
  (`++air_mantles`) has its own gates; check whether the shot sets one of
  these bits while fire is held.
- The original's own mantle start (clips 134-139, D08Y step-up mantles in
  mode 0) may refuse while the upper body is in its fire track (`+0x74` 16
  for the Desert Eagle, the D10H6 held-fire pacing in `weapon_feel.inc` keeps
  it there): the native start likely needs an idle upper body. D08J8
  (`climb_fire.inc`, `weapon_feel.inc` D08J8D) already lets the sidearm fire
  inside the mantles once started.
- First step: on the slot 8 post (private copy), log `+0x224`, `+0x74`, the
  lower clip `+0x60` and the D08Y / ledge-reach counters for a held-fire
  approach versus release-then-mantle, to find which gate refuses.

**Acceptance:** with fire held, Duke mantles the slot 8 post (and other
step-up mantles) as he does without firing, and keeps firing through it per
D08J8; every weapon that can fire on a mantle; walk and run approaches;
first and third person; Vanilla unchanged; the user's playtest.

### D08Z9 - Stuck weapon: Duke cannot shoot or change weapons

**Status: Done (user-accepted 2026-11-07: "it's all working!!"; [note 411](documentation/411-d08z9-stuck-weapon.md)). Dependencies: D08Z8.** User, 2026-11-07, UI slot 7:
"i've just encountered that strange bug where i cant shoot or change
weapons again. can we please prevent this from happening. deep research if
you need to." The save: equipment 2 (Desert Eagle) with the upper track on
the legs' clip 63 (no weapon bit), following the legs; fire and the weapon
keys do nothing.

Cause (reproduced): the D08Z8 sidearm loan's close restored the upper id
only when it still equalled the lower clip of the loan; when the walk handler
changed the clip inside the loan (wall + E: 74 -> 63) the lent free upper had
copied it. Change: the loan keeps and returns the weapon's whole upper block
(no-mantle closes); `weapon_pose_guard.inc` redraws the weapon through the
original's request when the state lasts 6 player updates on settled ground
with nothing in progress, logging each repair.

**Acceptance:** UI slot 7 loads with a working weapon; E with the Desert
Eagle into walls, doors and objects never leaves Duke unable to fire or
switch; armed mantles, doors, crouch, run and jumps unchanged; no repair in
normal play; Vanilla unchanged; the user's playtest.

### D08AA - Unify walking/running speeds across movement states

**Done (user-accepted 2026-10-19).** User: "this is perfect. swimming now feels great! it's responsive, and the gameplay allows the player to dodge enemy fire and not face these jarring speed changes when swimming."
Executable `5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366` is the regression baseline. Follow-up D08AB: a menu option
to choose these speeds or the original swim speeds.

**Built 2026-10-19.** Executable
`5e48b2c7cd4d44c92e59f2c7c7d78cd219c0d2f64af53b97570276cc99ed6366`,
[note 156](documentation/156-d08aa-unified-pace.md). One shared pace,
`modern_walk_pace` / `modern_run_pace` (`modern_controls.cpp`, the measured
land baselines in the original's velocity units), drives every water state:
shallow water no longer forces the run (land gait, Shift decides); the
mid-depth wade root follows `modern_pace_step()`; surface strokes (forward,
back, sideways) and underwater thrust are rescaled at the swim tail's
rotation call (new hook `0x80066e34`, ra `0x80046ec8`), keeping the pad
pressure share; with Space / Ctrl held underwater the original vertical
speed stays. Idle, Space / Ctrl alone, the dive 133 and Vanilla untouched.
Private lab (dated savestate copies), units per vblank, walk / run:
land 10.95 / 50.0; ankle-deep 10.5 / 49.7 (was 49.8 / 50.2); mid-depth
10.6-11.1 / 49.4-50.1 (was ~22 / 53.5); surface W/D/S 9.9-10.7 / 50.2-50.7
(was 25.5 forward, 6 sideways, both); underwater 10.1 / 49.0-51.1 (was 27.6
both). D08O / D08O1 / D08O4 regression route, wade jumps, deep-water entry
and Vanilla (pad) unchanged; ttk-controls-test (new pace cases),
ttk-input-test, ttk-aim-test, ttk-near-test, Python 146 pass. Remaining:
the user's playtest. Walking now stops at a drop's edge in water, as on land.

Queued 2026-10-09. User request: fix and unify Duke's horizontal
walking and running speeds across all movement environments - land, shallow
water, surface swimming and underwater swimming - rather than per-state
workarounds.

**Current issues (Modernized):**
- Surface swimming is unusually slow.
- Underwater swimming always moves at running speed.
- Shallow water (including ankle-deep) forces running speed even without Shift.

**Expected behaviour (Modernized):**
- Shift released = normal walking speed; Shift held = normal running speed.
- Both speeds match the existing land walking and running baselines in every
  movement state, regardless of water depth.
- Prefer one shared speed path so a later global walk/run retune applies
  everywhere. Do not change those land baselines in this job; a separate job
  will adjust them.

**Scope:** investigate the underlying horizontal movement calculations and
implement a consistent Modernized solution. Preserve swimming physics,
vertical movement, water transitions, and the accepted swim-and-fire work
(D08O1 / D08O2 / D08O2A). Vanilla unchanged.

**Test saves (dated private copies only; never the live player card):**
- UI slots 2-3: surface and underwater swimming.
- UI slot 8: ankle-deep water in the subway.

**Acceptance:** in Modernized, walk and run horizontal speeds on land, in
shallow water, on the surface and underwater match the current land walk/run
baselines under the same Shift state; surface swim is no longer unusually
slow; underwater and shallow water no longer force run without Shift; swim
physics, vertical move, water transitions and swim-fire stay as accepted;
Vanilla unchanged; the user confirms.

### D08AB - SWIMMING menu option: modern or original swim speeds

**Done (user-accepted 2026-10-20).** User: "solid lock it in and close it out". Executable `c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508` is the regression baseline.

**Built 2026-10-19.** Executable
`c7557a215c099cfb6ae10484af3555b869a1c7acb627bd0ab72506d246922508`,
[note 157](documentation/157-d08ab-swimming-option.md). SWIMMING row in
OPTIONS > MODERNIZED after LANDING (MODERN default / ORIGINAL), live and
saved (profile schema 33, `--swimming`, settings key U, `DNTTK_SWIMMING`;
Vanilla always original). User decision: ORIGINAL also restores the forced
run in shallow water. `swimming_modern()` gates the D08AA paths: with
ORIGINAL the forced run (`wade_full_speed_ready()`), the old mid-wade band,
the fixed free-swim root 48 and the original stroke / thrust speeds come
back; no new hooks. Private lab, ORIGINAL: subway 49.5 / 49.8 (forced run),
surface W / D / S 26.1 / 6.05 / 15.1 with Shift ignored, underwater 27.6,
plain W underwater 987 per 30 updates (all the D08AA "before" numbers); land
unchanged. MODERN still measures as D08AA (10.4-10.6 walk, 47.9-50.1 run).
Toggling in the real pause menu mid-swim switched surface speed at once
(9.8 -> 25.1 and back 24.7 -> 9.8) and saved. Dive, D08O4 stop, swim-fire,
surfacing and Vanilla unchanged; ttk-controls-test, ttk-input-test,
ttk-aim-test, ttk-near-test, Python 149 pass. Remaining: the user's
playtest.

Queued 2026-10-19. User request, on accepting D08AA: "I want one
job in the backlog to add a toggle between modern / original swimming in our
menu".

**Expected behaviour:** a SWIMMING row in OPTIONS > PC CONTROLS (Modernized
controls section, beside JUMP WALLS, LANDING and DYNAMITE), values MODERN and
ORIGINAL, default MODERN, saved with the other preferences and with a CLI /
`DNTTK_SWIMMING` override like the neighbouring rows.
- MODERN: the D08AA behaviour - one walk/run pace in shallow water, the
  mid-depth wade, surface strokes and underwater thrust; Shift decides.
- ORIGINAL: the pre-D08AA water speeds with modern controls still on -
  the original fixed stroke and thrust speeds (`0x1333` / `0xaaa` / `0x444` /
  `0x149f`, ~26 forward, ~6 sideways, ~28 underwater) and the earlier wade
  speeds. Decide in the job whether ORIGINAL also restores the D08P forced
  run in shallow water or keeps Shift there (ask the user).

**Scope:** gate the D08AA paths (`swim_pace()` in `swim.inc`, the mid-wade
and free-swim root steps, the shallow-water run) on the option; switching
takes effect at once, in or out of water. Steering, swim-fire, dive, vertical
movement and transitions stay the same in both. Vanilla unchanged (it keeps
the original speeds whatever the row says).

**Acceptance:** the row appears and saves; MODERN measures as D08AA (walk
~10.5, run ~50 units per vblank in every water state); ORIGINAL measures as
the D08AA "before" table in [note 156](documentation/156-d08aa-unified-pace.md)
for the swim states; toggling mid-swim changes speed without a hitch; Vanilla
unchanged; the user confirms.

### D08AC - Normalise Shallow-Water Locomotion and Jump Physics

**Accepted (2026-10-22).** User: "perfect!!! lock it in and close us out".
Implemented shared land gaits, takeoffs,
air control and landing in grounded shallow/mid water with SWIMMING MODERN.
SWIMMING ORIGINAL retains its accepted wade behavior; Vanilla is unchanged.
Private slot-10 reproduction and paired traces confirm the old water path
selected a reduced forward jump even for neutral Space. See the work log
below for measured evidence and acceptance.
Research: [note 194](documentation/194-d08ac-shallow-water-locomotion.md).

**Objective:** grounded shallow water should use the same locomotion model
as dry land, with water visuals, sounds and environmental effects retained.
Prefer shared grounded movement, jump and air-control logic over duplicated
water physics. Surface and underwater swimming are outside the change.

**Dependencies:** D08AA shared pace, D08AB swimming option, D08K11 grounded
water crouch, D08Z manual/assisted jumping (all accepted). Related: D08C,
D08I, D08L, D08M/O/P/W/Y, D08Z1/Z3, D08K5/K9. Coordinate broader crouch
coverage with D08K5; it does not block investigation. Resolve how the new
shallow-ground policy coexists with D08AB's explicitly accepted ORIGINAL
shallow forced-run choice before implementation; do not silently redefine it.

**Reproduction:** use a dated private copy of UI
slot 10, confirm the flooded-subway grounded state, then compare walk, run,
standing and directional jumps with nearby dry ground under identical
settings and inputs. Record height, launch velocity, gravity/airtime,
horizontal travel and acceleration, input latency and airborne steering.
Test open floor separately from walls and assisted platform exits.

**Acceptance:**
- Shallow grounded jumps match dry-land height, vertical velocity,
  acceleration/gravity and airtime under the same selected jump style.
- Walk/run speed, acceleration/deceleration, sprint/jump input timing and
  directional responsiveness match dry ground; no stutter or artificial
  slowdown. Manual air control matches the dry-ground policy.
- Modern crouch/crouch-walk, safe stand-up and crouch-on-landing remain
  compatible, including grounded mid-depth water where Duke still walks.
- Natural shallow/deep transitions, jumps into deep water and return to
  ground remain stable; surface/underwater physics and swim-fire unchanged.
- Water audiovisual/environmental effects stay intact. Existing single-player
  traversal, collision, aiming and both cameras do not regress; Vanilla
  remains unchanged. Verify both jump styles and document option semantics.
- Measured paired traces plus user playtest establish parity; build success
  alone is insufficient.

**Initial findings:** the water jump path launches 97/98 directly, sets
`air_cap=0` (disabling manual steering), maintains its own horizontal boost,
and has separate ledge lift/hold logic. D08AA shares pace but did not unify
jump physics. D08AC bypasses that path for grounded MODERN water, and
scoped authenticated native branch changes prevent mid-water gait/landing
rewrites from cancelling ordinary jumps. Water queries and effects remain.

### D08AD - Fix third-person locomotion animations in deeper shallow water

**Accepted (2026-10-22).** User: "the animation is fixed. lock it in and close it out."
D08AD is Done. Separate reported deep-water entry regression queued as D08AE;
its investigation does not reopen the accepted animation work.
Walking's nested terrain probe was clearing the dispatcher patch scope before
native dispatch; the separate run-segment completion selector also still
chose water clips. Scope arming now follows the probe, and authenticated
completion branches retain both native land stride halves. Walk/run cycles
pass at 60/120 Hz; jump/crouch/swim regressions pass. User visual acceptance received. Executable
`9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
 Follow-up to accepted D08AC shallow-water locomotion and
jump physics, with D08K11 crouch behavior preserved. Dependencies are Done.
Research: [note 195](documentation/195-d08ad-deeper-water-animation.md).

**Reproduction:** the user updated UI save slot 10 to slightly deeper
shallow water. Walking appears frozen until returning to shallower water;
running has abbreviated leg steps and a repeatedly one-sided backpack swing.
The backpack observation is a visual hypothesis, not a confirmed cause.
Normal knee-depth water works perfectly; movement physics, jumping and
crouching work correctly at the new location, per the user's report.

**Scope:** investigate depth-dependent third-person clip selection, frame
progression and transitions, including legacy wade clips and movement-state
classification. Preserve
D08AC movement/jumps, D08K11 crouch, surface/underwater swimming, SWIMMING
ORIGINAL and Vanilla. Do not retune physics to mask an animation defect.

**Acceptance:**
- Walk and run use natural, complete locomotion cycles consistent with dry
  land throughout grounded shallow/mid-depth water.
- Crossing between shallow depths, starting/stopping and changing walk/run
  remain smooth without frozen poses or repeatedly restarted partial cycles.
- Backpack motion follows the complete animation naturally, with no assumed
  backpack-specific fix unless evidence establishes one is needed.
- Updated slot 10, nearby knee-depth water and dry ground pass matched
  third-person traces and visual playtesting at 60/high refresh; accepted
  movement, jumping, crouching, deep swimming and option boundaries survive.
- Root cause is supported by clip/state/track traces and user playtest;
  a build or speed measurement alone does not establish animation completion.

**Initial findings:** depth 512..640 is the native mid-wade band, but the
new slot's actual depth has not been measured. D08AC converts 80/81 to
63/72/76 and zeros +0x68/+0x6a; repeated native reselection could therefore
restart playback. Scoped dispatcher patches already suppress four water
rewrites. Check their coverage/lifetime and other transition writers before
assuming legacy clips alone explain the symptom. See note 195 for evidence,
alternative hypotheses and the future trace plan. No game launched here.

### D08AE - Restore deep-water entry resistance and timely swim handoff

**Done (user-accepted 2026-10-22).** Native deep-water entry restored by requiring
local volume depth <=640 for shallow-flight protections. Pool/moat entry
samples now match native comparison timing; both camera selections, jump
styles, 60/120 Hz, depth boundaries, shelf, gait/crouch and swim regressions
checked. Accepted executable `c1781e76badc0d8fb0459ef3fee4afd0abcc3dd960709eb92af06c40b26869af`.
User tested both reported locations and shallow water: "all solid".
Implementation/documentation commit and push authorized after D17R2 backlog creation.

The following findings describe the original report and investigation.
Reported during D08AD acceptance: jumping into deep water feels like jumping
into air, without resistance. Reproduce from current UI slot 9 (pool ahead)
and UI slot 2 (moat). Research: [note 196](documentation/196-d08ae-deep-water-entry.md).

**Findings:** private running/walking jumps and run-offs retain native airborne
mode 9 far below the surface with MODERN swimming. In running-jump samples,
mode 5 begins about 2452 units below the pool surface and 4378 below the moat
surface. Cached support equals water surface (apparent depth zero) despite
native volume depth 3072/about 5000. D08AC's flight protections use that cached
support predicate to bypass native swim entry. ORIGINAL comparison restores
earlier entry. This strongly implicates classification/handoff, not a missing
arbitrary drag coefficient; no corrective experiment or gameplay edit made.
Do not claim D08AD introduced the regression without historical comparison.

**Scope:** restore deep-water entry response and timely transition from air
to swimming while retaining accepted grounded shallow/mid-water behavior.
Investigate native volume/geometry/body checks and airborne ownership; avoid
blanket physics retunes or removal of shallow-water jump protection.

**Acceptance:** both saves' walk/run jumps and run-offs feel like entering
water and no longer continue an air-like descent nearly to the bottom;
matched state/velocity traces and user playtest confirm it. Cover manual and
assisted jumping, held/released input, both cameras and 60/120 Hz. Preserve
D08AC jumps/landing, D08AD animation cycles, D08K11 crouch, surface/underwater
controls and swim-fire, effects, ORIGINAL and Vanilla. Test natural shorelines,
shallow shelves in deep volumes and return to ground. See note 196 for detail.

**Dependencies:** D08AC, D08AD (Done). Related: D08AA/AB, D08O, D08Z, D08K11.

### D08AF - Smooth plunge-to-swimming control handoff at maximum depth

**Done. User-accepted 2026-10-24:** "yep! its a winner, lock it in and close it out".
Accepted implementation and evidence:
[note 217](documentation/217-d08af-plunge-swim-handoff.md). Blends held
horizontal input into the native plunge tail before collision; vertical
resistance and released-input damping stay native. Use the player's current **UI slot 9**
(private test copy when selected).

**Reproduction:** load UI slot 9, hold **Shift + W** to run along the ledge,
then press **Space** while continuing forward to jump straight into the water
below. Follow Duke through the plunge. At the deepest point of the entry,
just before full swimming control returns, the user feels a small slowdown
or hesitation between the downward carry from the fall and freely swimming.
The right observation point is the end of the plunge, not the initial splash.

**Goal:** a more modern, fluid and streamlined transition from the plunge
into controllable swimming, without the stiff pause at maximum entry depth.
The user describes fall-dependent inertia/depth; this is a perceived mechanism,
not an established diagnosis. Cause and exact state transition are unconfirmed.

**Investigation when selected:** reproduce with the stated held-input sequence;
trace entry depth, vertical/horizontal motion, animation, mode and input/control
ownership through the deepest point and first controlled swim movement. Compare
held versus released/reapplied movement, different approach/fall heights and
camera views. Distinguish physical deceleration, animation recovery, control
handoff and actual frame pacing. If timing evidence indicates a performance
slowdown, correlate gameplay logs with host activity as required by AGENTS.md.

Read the accepted D08AE water-entry work and
[note 196](documentation/196-d08ae-deep-water-entry.md) before changing the
handoff. D08AE restored water-entry resistance; this report concerns the feel
at the end of the plunge and does not establish a regression in that fix or
in D11E. Preserve believable entry resistance and existing shallow-water rules;
do not choose a blanket speed boost or immediate dive cancellation in advance.

**Acceptance:** the slot-9 running-jump entry flows into responsive swimming
without the reported hesitation at maximum depth, confirmed by the user and
input/state/motion evidence. Preserve collision, oxygen, surface/deep swimming,
water-entry resistance, shallow-water jumps and weapon use; no unintended
forward drift or excessive travel after release. Check both cameras, multiple
entry heights and another water location. Vanilla remains unchanged.
