# D08J8 - Shoot through traversal (batch)

2026-11-05. Status: Done (user-accepted and closed 2026-11-05 at revision 3; see Closeout). Batch D08J8A-E from the user's live
commentary over UI save slots 6-12: "you should be able to start shooting
straight away just like we did with the mantle", the model being the D08J7C
armed one-handed pull-up ([note 378](378-d08j7-mantle-fire-research.md)).
Sidearm rule kept (one hand on the climb, D08J6A,
[note 377](377-d08j6-climb-fire-research.md)). Modernized only.

## What was measured first

Private offscreen lab `recomp/analysis/d08j8/` (local only): `probe.py`
loads a private copy of the player's savestates (`fx/`, copied byte for
byte; the player's saves, cards and settings are hash-checked unchanged after
every run), plays a key script through `DNTTK_TEST_INPUT` and samples Duke's
lower/upper tracks, mode, equipment and the shot times (`ttk-launch`). 180 Hz
unless stated; UI slot N is runtime slot N-1.

| Case (UI slot) | Clip, mode | Before |
| --- | --- | --- |
| Water exit (12): surface, W+E | 142, mode 8, 1.5 s | fire during it dropped; on land the press drew and the first round left about 390 ms later |
| Ladder top (7): W | 190, mode 8, 1.7 s | holstered the whole exit, fire ignored |
| Crate mantle (10): W+E jump | 137, mode 8, 1.6 s | D08J7B queue: drawn on landing, first round about 290 ms after it |
| Door (11): left click | pose 116, mode 0, 0.95 s | first round 1.24 s after the press |
| Held sidearm on a ladder (7), wall, chain, monkey bars (6), ledge hang and armed pull-up (9) | climb fire track 11 | 200-237 ms, about 4.6 per second (ground 167 ms, 6/s) |

### Why

- **Fire request** `0x80058120` (once per update, mode other than 5) refuses
  fire and clears `+0x224` bit 0x100 when the lower or upper clip's entry in
  the table `0x800c2824` carries bit 0x2000, or when `+0x224 & 0x241`
  (0x40 is the committed use action). Bit 0x2000 is set on the mantles
  134-139 and the door kick 116; 140, 142, 190, 196 and 206 do not have it.
- **The shot** itself comes from the upper fire track's key event (flag 8,
  `+0xb8`), not from the request. Restarting the pistol fire track 16 inside
  the crate mantle fired real rounds while Duke kept climbing (debug-port
  prototype).
- **Draw inside the climb**: equipment 2, weapon 4 and the draw 6 written on
  the upper track during 137, 142 or 190 keep the climb running to its end,
  and in 142/190 the original then fires by itself (raise 9, fire track 10).
- **Door**: the use stage machine `0x800513e4` (`+0x27d`) installs 116 at
  stage 4 -> 5 and its completion is stage 6/8 (`0x80051740..0x80051780`:
  `+0x27c..+0x27e`, `+0x290` cleared, bit 0x40 off). The door opens when
  116's strike key (key 3, event 0x800) ends: cutting the pose after key 3
  opened it, cutting at key 3 left it shut.
- **Climb rate**: the climbs' one-arm fire track 11 has the same keys as
  track 10 (20/20/30, event on the second key), but the D10H6 pacing
  ([note 391](391-d10h6-held-fire-clock.md)) admitted neither the track nor
  the climb states.

## Change (Modernized; Vanilla unchanged)

- **D08J8A/B/D, traversal draw** (`climb_fire.inc`, `trav_fire_update`):
  held fire while Duke climbs over a lip draws the sidearm at once, inside the
  climb: mantles 134-139 (mode 8, and the D08Y step-ups in mode 0), pull-up
  140, water exit 142, ladder, chain and wall tops 190, 196, 206. Another
  selected weapon is swapped (the D08J6A rule; SIDEARM RETURN applies on the
  ground). The draw runs 4x and counts as a fresh press for the D10H4
  first-shot boost. E held from the grab does not hold it back. It replaces
  the D08J7B landing queue whenever the sidearm has ammo; without it the
  queue still serves. Unarmed Cross stays masked until the draw. In the
  pull-up after such a draw, the D08J7C one-handed pose and the D08J7D first
  person apply.
- **Climb up the face into a hang**: armed, the original's 139 -> 140 chain
  stops in the ledge hang (the alley dumpster route). In a hang a climb with
  this draw ended in, W or E (still held from the grab) starts the D08J7C
  armed pull-up (`trav_hang_climbing`, `hang_fire_update`, `pc_input.cpp`).
- **D08J8D, mantle fire track** (`weapon_feel.inc`, `trav_fire_drive`): in a
  clip with bit 0x2000, sidearm out, fire held, the pistol fire track 16 is
  started on the game clock and run through its fire key the update a shot
  is due; between shots it is held before its end. RETRO 5 tics (50 clock
  units, 6/s), MODERN 6. Not during a draw, stow or clip reload (12, 17).
- **D08J8C, doors** (`door_fire.inc`): with a weapon out, the kick 116 (use
  stage 5, bit 0x40) plays 6x on every channel playing it, and once the
  strike key has ended the use action takes its own completion (stage 8,
  idle 63). Holstered kicks and other use actions are unchanged.
- **D08J8E, rate** (`weapon_feel.inc`): track 11 is a Desert Eagle fire
  track, its raise 9 gets the first-shot boost, and climbs, hangs, monkey
  bars, the armed pull-up and the traversal clips join the RETRO pacing.
- Debug: `controls.mantle_fire.trav` (`ready`, `shot`, `drawn`, `draws`,
  `swaps`, `pullups`, `drive`) and `controls.mantle_fire.door` (`kicks`,
  `cuts`).
- No new hook address, guard, generated code, framework or codegen change;
  existing savestates load. Runtime patch export unchanged.

## Evidence

Executable SHA-256
`1015e6de8daf67030b1895c9bcda5275fff939814fce884c9b59a74675aa70db`.

| Case | After (180 Hz) |
| --- | --- |
| Water exit, fire pressed mid-climb (W+E held) | sidearm out 37 ms later, first round 88 ms after the press, then 165-170 ms through the climb and on land |
| Ladder top, fire pressed in 190 | first round 106 ms after the press, 165-169 ms through the exit (one 200 at the landing) |
| Crate mantle, fire held through the jump | drawn on 137's first frame, first round 120 ms into the mantle, 165-169 ms to the top and on |
| Chain and wall tops (D35F, D35I1 fixtures) | first round 94 ms after the press |
| Alley dumpster (D35G fixture), W+E+fire | 139 drawn and firing, the hang held about 100 ms, armed pull-up, 166-170 ms |
| Door | opens; first round 307 ms after the press (kick about 140 ms), then 164-170 ms |
| Held sidearm: ladder, wall, chain, monkey bars, ledge hang and armed pull-up | 163-171 ms (before 197-237) |

- 60 and 120 Hz: ladder, crate and door identical (166-170 ms; door 307 ms).
- First person: the pistol viewmodel is up and firing during the crate mantle
  and the water exit (`fp-sheet.png`), and the ladder exit (`s7-sheet.png`).
- Shotgun selected before a crate mantle: the sidearm is swapped in and
  fires; NEVER keeps it, AUTO requests the shotgun about 0.75 s after release.
- MODERN weapon feel: climbs keep the original 200-235 ms; the mantle track
  fires about every 200 ms.
- Holstered ladder top without fire: unchanged (equipment 0 throughout).
- Native: `ttk-controls-test` with LEVEL01 and all levels, 153 PASS (new
  D08J8 group: draw for every clip and in mode 0, E held, the holster, no
  ammo, other clips and modes, Vanilla, the swap, 4x draw, the mantle fire
  track's start, hold, next shot and refusals, the door kick 6x once per
  update, the cut after key 3 only, holstered and Vanilla doors; the D08J7B
  group now tests its queue with the sidearm empty); `ttk-input-test` (E is
  Up in a hang a climb ended in armed, never elsewhere); `ttk-aim-test`;
  Python `test_player_profiles` 127.

## Limits and not verified

- The user's real mouse and display; enemies were not framed in the lab runs
  (shots measured by launches, ammo and screenshots), so the pig cops behind
  the slot 7 ladder were not fought.
- Pose: outside the pull-up, the gun arm plays the sidearm's own poses over
  the climb clip; no one-handed clips were built for 134-139, 142, 190, 196
  or 206 (judge in game first, as D08J7C revision 1 was).
- A shot can be one update late at a transition (one 200-233 ms gap at a
  landing or the hang handoff).
- Door: the kick is still visible but brief; the cut applies only to the
  kick-open pose 116 (kind 20 doors), not other use actions.
- The mantle fire track does not run the original's fire-request aim build
  (`+0x280..`): rounds follow the view through the existing shot adaptation.
- Vanilla was not driven live (the test input works only in Modernized);
  every path is behind `input_modernized()` and covered by the native Vanilla
  checks.
- `ttk-near-test` does not link at HEAD (pre-existing, `cowboy_hat_mesh`).

## Revision 2 (2026-11-05): third-person aim, doors on E

User playtest: the slot 10 mantle fires fine and "in first person especially
this looks perfect", but in third person "Duke doesn't always point the gun
in the direction that you're pointing in"; at the slot 8 wall top "in third
person I could not seem to aim" (fine in first person); the fire rate is
confirmed fixed on ledges, climbing walls, monkey bars and chains. Doors:
"we don't need left click to open doors and it should be the E key", and
"you open the door with the E key and you should just be able to shoot
straight away" (E holstered Duke first, then about 1.3 s to the first round).

Measured: the rounds already went to the crosshair (aim log: view-adapted,
no cover rejections); only the pose was wrong. With the camera turned about
70 degrees off Duke's facing during the crate mantle, the gun pointed along
his heading. In the mantles 134-139 the host played track 16 with no aim
build; at the ladder, chain and wall tops and the water exit the original
fires from track 10, whose aim build was given Duke's heading.

- **Mantles 134-139** (`trav_fire_owned`): the host owns the fire. The
  game's Cross is masked, and with fire held the climbs' one-arm fire track
  11 is put on the gun arm (it loops by itself without the fire request,
  measured; the D10H6 pacing times it). The D08J6B climb aim
  (`climb_aim.inc`: chest and head to the view, off hand kept on its hold,
  the one-arm aim lent the clamped view) now covers them; the one-arm aim
  build runs only with `+0x224` 0x100, so the host sets it for the model
  build (`climb_aim_begin`, 0x800411b8) and takes it back at the next player
  update before the game reads it. A first attempt that restarted the track
  each shot fired at 101-135 ms (the runner ignores a rewrite of the playing
  track and restarts a new one on its next call); it was replaced.
- **Water exit, ladder/chain/wall tops** (`trav_view_aim`): the original's
  aim look-at (0x80097a44) takes the view direction while it fires, as for
  the D08J7C armed pull-up. Putting track 11 there instead failed: the
  original reselects 10 every update with the fire request, and lowers to 9
  without it.
- **Doors** (`door_fire.inc`): a door is an object whose type metadata
  selects pose 116 (type table `0x800d2660` byte 4 = kind 5, jump-table
  entry `0x80015850` = `0x800515e0`), and `player+0x174` is the object Duke is
  touching (the door while he stands at it, 0 at crates, walls and ledges;
  measured). E with a weapon out at a door no longer holsters: a probe sends
  armed Cross for up to 4 updates; the game's finder starts the door use and
  the quick armed kick runs. A fire request the probe raised instead is put
  back (with the upper block) for up to three updates, so no round leaves,
  and the original E follows. E while firing releases Cross for one update,
  then presses it. Left click no longer opens doors: a door use started by
  fire alone is ended at its first stage (before the use callback).

| Case | After |
| --- | --- |
| Crate mantle, camera about 45 and 70 degrees right | chest turned and muzzle out toward the crosshair; 154-168 ms |
| Ladder top, camera 55 degrees left | Duke turns and fires toward the crosshair (track 10, view-aimed rounds); 166-169 ms |
| Door, E then fire | no holster; door open; first round 73 ms after pressing fire (was about 1.3 s) |
| Door, E while firing | door opens; one 367 ms gap around the kick, then 167 ms |
| Door, left click | the door stays shut; first round 107 ms after the press |
| E at the crates (W+E jump) and in slot 8 | no probe, the original stow on time, no round |

All earlier cases re-run on this build (ladder, monkey bars, wall,
hang and armed pull-up, alley route, water exit, ladder top): 164-170 ms.
Native: `ttk-controls-test` 153 PASS (the D08J8 group now covers the host
fire, the Cross mask, the left-click door cancel, refusals for holstered,
non-door, E held and Vanilla, and the E probe's keep and revert);
`ttk-input-test`, `ttk-aim-test` pass. Executable
`af4a4d6eeebd4f683f88c80761d44dc0e0688cae403d3fa9a9b9dd43dad2b26d`.

Limits: the slot 8 wall itself could not be mounted with scripted input, so
the wall top was checked on the D35I1 wall fixture (same clip 206) and the
slot 7 ladder top; the arm's arc is the D08J6B clamp (chest 50 degrees, gun
arm 110 right / 60 left), beyond it the rounds still go to the crosshair.
Firing in a door kick itself is not possible (the kick refuses fire); with
E pressed mid-fire that is the one gap.

## Revision 3 (2026-11-05): steady mantle pose, wall-top camera, doors at full length

User playtest of revision 2: the slot 10 third-person mantle "works amazing",
but "when Duke shoots his head jerks backwards and forwards"; the slot 12
water exit is functionally accepted, with the wish that Duke still looks
like he is pushing himself out while shooting (fine tuning); at the slot 8
wall top "he's always facing forward in third person"; and the door: "i want
the door to open in the normal way, but duke is just able to still shoot"
(the revision 1 quick kick looked wrong and seemed inconsistent).

- **Head jerk** (measured with `DNTTK_AIM_TRACE` per update): the driven
  track 11 is no loop of its own. At its end the runner moved on to the
  lowering 9 inside the same call, and the next call started 11 again as a
  new track, with its blend, after every shot. The host now wraps the
  driven track to its first key once this update's delta (after the D10H6
  pacing) would reach its end, and gives the runner no delta
  (`trav_fire_wrap`; it stops at the fire key if a carry would pass it).
  After: the gun arm stays on 11 through the whole mantle, 152-168 ms.
- **Wall top** (slot 8, reached on the player's own save with W+E+Space and
  W): during the exits 197/206 the mouse did not move the camera at all (yaw
  fixed at 0 while the ladder top 190 followed the mouse), so the view, and
  with it Duke's aim, stayed straight ahead. The wall exits now join the
  mouse-camera lease of the ladder and pole exits (`mantle_state_early`).
  After: the camera turns over the top and Duke turns to face and fire at
  the crosshair.
- **Doors**: the 6x kick and its early end are removed; the kick plays at
  the original speed (about 0.9 s). With a weapon out the host owns the fire
  through it as in the mantles (`door_fire_owned`): Cross masked, the
  weapon's own ground fire track (pistol 16, shotgun 25, Gatling and Freezer
  34, RPG 37) on the gun arm while fire is held, kept looping by the same
  wrap, the D10H6 pacing, view-aimed rounds and the original's aim given the
  view. E still kicks armed and left click still never opens a door.

| Case | After |
| --- | --- |
| Door, E then fire | kick at full length; first round 207 ms after E, then 167 ms through the kick |
| Door, firing then E | 167 ms throughout, one 200 ms gap |
| Door, left click | door shut, 167 ms |
| Crate mantle | gun arm steady on 11, 152-167 ms |
| Slot 8 wall top, camera turned 40-50 degrees during the exit | camera follows, Duke faces and fires at the crosshair, 167 ms |

Re-run on this build: ladder, ladder top, water exit, monkey bars, ledge
hang and armed pull-up, alley route: 163-170 ms (one 200-233 ms gap at
some transitions, as before). First person during the door kick: the kick
leg and the pistol firing together. Native `ttk-controls-test` 153 PASS
(door group rewritten for revision 3), `ttk-input-test`, `ttk-aim-test`
pass. Executable
`ab126a7f258b67539359d708d6d25cb9ce6c585fc6113ec7d1db23c419c715da`.

Not done here: the water exit's one-handed push (new job D08J8F). The
original's own aim holds the upper body still while it fires from the
water; the proven route is a one-handed water-exit clip like the D08J7C
pull-up's host clip 41, built from 142's frames. In third person the slot
11 kick still puts the camera behind the door frame (job D10F2, unchanged).

## Closeout

2026-11-05. User, on the revision 3 build: slots 10, 8, 11 and 12 fully
accepted ("the head jerk is fixed", "duke points in the right direction",
"you can shoot and open the door at the same time perfect", "he naturally
gets out of the water too"), then "this work is complete fully accepted. lock it in and close it out". D08J8 is Done; D08J8F (a
one-handed water-exit push) is cancelled as no longer needed. No framework
edit; the runtime patch export is unchanged.
