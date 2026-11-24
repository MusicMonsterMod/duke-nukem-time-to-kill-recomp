# Time to Kill jobs - Campaign, audio and performance

Job descriptions and work logs for D18, D22, D23, D37, D45 and D50. Status and
dependencies are on [the job board](../MODERNIZATION_JOBS.md); the dated
session log is in the `log-YYYY-MM.md` files beside this one. Moved verbatim
from the board on 2026-11-10 (D58), one section per job in ID order.

## Jobs

### D18 - FMV and audio presentation safeguards

**Status: Done (research accepted by user 2026-10-27).** The user confirmed this was a completed research job. The following verification limits remain evidence limits, not outstanding D18 acceptance gates. All eight native-table clips covered, including guarded private fixtures for optional/ending clips. Skip, cadence, seek/read and clean debug-quit evidence recorded. Listening/sync, real desktop fullscreen/window-close and natural campaign ending routes remain open. No decoder/gameplay changes justified by this pass. [Note 281](documentation/281-d18-fmv-reliability.md) contains the matrix, diagnostic tool and limitations.

Expand coverage beyond the opening movie: playback, skipping, transitions and return to gameplay. Preserve native movie cadence and audio synchronization through display/profile changes. Optional scaling must not invent a higher source frame rate or regress the fixed streaming path.

**Acceptance:** a movie/transition matrix records tested clips, sync and underrun observations; repeated voice-bank playback does not return on the regression route. Both modes close cleanly during movie playback.

### D18A — Voice/music/gunfire crackle investigation

**Done — player closed (2026-09-28).** No crackle/artefacts in club play; reopen if
it returns. Lab club starvation evidence remains historical in notes 46–47 and is
not treated as an open player defect.

**In progress.** Club no-poll starvation remains the positive lab reproduction; no
audio repair has passed that capture. The 2026-09-27 player session reported no
crackle in the club, which does **not** close the job. Keep producer/frame-time
attribution and matched comparisons. See notes
[46](documentation/46-turning-audio-inventory-progress.md) and
[47](documentation/47-playtest-iteration.md).

**In progress; output starvation reproduced in private club captures, no audio repair yet.** See [current comparisons and limitations](documentation/46-turning-audio-inventory-progress.md). User reports crackle during strip-club voice/music
and gunfire at the station-facing ledge Pig Cop; regression versus pre-existing is
unknown. **Acceptance:** reproduce/capture audible evidence, investigate mixing,
streaming and callback timing, verify any supported fix on the same scenes without
sync/latency regression. Dummy audio is insufficient. See [current brief](documentation/45-playtest-turning-audio-follow-up.md).

### D18B — Concurrent voice with music (no music mute)

**Done (user-accepted 2026-10-27; closeout, commit and push authorized).**
Independent Modernized music now
continues through native speech, using the existing SOUND controls.
[Note 287](documentation/287-d18b-concurrent-music.md) records implementation,
PCM/counter evidence and restoration/coverage limits. User accepted playback
and explicitly requested lock-in and closeout.

Original request: voice clips cut out music. Allow voice
and music to play together at sensible relative levels, as a precursor to D21
channel volume controls. Investigate original XA/SPU ducking versus host mixer
policy before changing behavior. Do not hide D18A starvation by muting sources.

**Acceptance:** documented overlapping voice+music route with music continuing;
no new underruns/clipping on the tested scenes; Vanilla/default behavior policy
recorded; volume-channel hooks prepared for D21 without requiring the full menu.

### D18C - Load-sensitive crackle at construction signs and train ledge

**Status: Done (user-confirmed 2026-10-30: no audio issues on repeated play of
the scene "in a long time").** Closed on the user's playtest with no code
change; no cause was isolated. Reopen if the crackle is heard again. The
original report and acceptance plan below are retained as history.

**Original report.** Current UI 1 (`cards-user8`, file 00): turn around, approach the
construction signs, move/fire, then jump onto the ledge; user hears brief,
self-resolving crackle. Current UI 10 (file 09): turn/move/jump around train
platform ledge. Include UI 12 club movement as an audio stress comparison.
D18A's older acceptance remains historical; this is a new, bounded follow-up.

Slot 1's tested route had zero output underruns, which does not establish
absence of an audible source/mixer/device defect. Slot 10 and some slot 12
stress runs show production starvation. Missing-sample counters are not a
count of audible crackles. Dummy-sink runs are diagnostics, not listening tests.

**Acceptance:** reproduce/capture on the actual audio device at 120, correlate
source/host PCM, producer/callback timing, queue fill, missing/dropped samples,
loads and firing/jumps. Distinguish starvation from clipping/streaming/device
issues. Verify any fix without increased audio latency, lost voice/music,
FMV desynchronization or gameplay/frame-pacing regressions. Check 180 too;
coordinate D17G/H without duplicating those movement fixes.

### D18D - Music silent after death and Continue until Duke's next voice line

**Done (user-accepted 2026-10-21: "i can confirm the music started with no further issue. lock it in, close it out").** Cause found in the emulated CD drive, not
the menus: at Continue the game seeks back to the music's saved position a
few sectors away from where death paused the drive. The runtime finished
that near seek in 0x800 cycles, inside libcd's `CdControl`, before the game
installed its seek callback (`0x8003b838`), so the music player stayed in
state 1 with the CD muted until a voice line restarted it. Framework fix:
near SeekL/SeekP now take Beetle's seek time (at least 20000 cycles, plus
the restart from a paused drive); far seeks unchanged. Modernized and
Vanilla both resume the music two frames after CONTINUE; level-start
speech-to-music and the intro FMV unchanged; one load seek per level gets
about 37 ms longer. Audible output, DuckStation/real hardware and other
levels not checked by tooling; the user heard the music resume. Executable
`23bea317011d6444d73fe58140fb80f5e8ae149bc0ea98c06b7b262bd911842a` is the regression baseline.
[Note 182](documentation/182-d18d-music-after-continue.md).

**Original report - user (2026-10-04).** After Duke is killed and the player
chooses Continue, level music does not resume. It only starts once Duke says
a voice line. Check whether Vanilla behaves the same and whether original
hardware/DuckStation (portable copy only) does too, before assigning cause.

**Investigation:** trace the Continue/respawn path for CD-XA music restart,
SPU/CD mute or volume state and the voice-line path that apparently restores
it. Distinguish an original-game behavior from a recomp/host audio defect.

**Acceptance:** after death and Continue, level music resumes promptly in the
tested levels without waiting for a voice line, if reference behavior shows
that is correct; no regression to voice playback, FMV audio, D18A/D18C
crackle routes or pause/menu audio. Use private save/card copies only.

### D18E - Challenge stages have no music

**Todo. User request, 2026-10-08:** "challenge level has no music. What are our
options here? Would be ideal to reuse appropriate music from the game on these
levels." User's suggested mapping:

| Level | Stage | Setting | Music |
| --- | --- | --- | --- |
| 21 | Challenge Stage 1 | warehouse | Time to Kill music |
| 22 | Challenge Stage 2 | industrial | Time to Kill music |
| 23 | Challenge Stage 3 | castle | medieval music |
| 24 | Challenge Stage 4 | castle 2 | medieval music |
| 25 | Challenge Stage 5 | chicken warehouse | already has Time to Kill music |
| 26 | Challenge Stage 6 | colosseum | Roman music |

First: confirm against the original (DuckStation portable copy, not the
flatpak) whether 21-24 and 26 are silent on real hardware too, or whether the
recomp loses their music (compare D18D, music silent after Continue). Then
find how a level selects its music (XA streams in `/SOUND/MUSIC*.IDF`, see
[note 14](documentation/14-gameplay-voice-seek.md); the per-level table or
call) and which track each era uses. Options to present before building: point
the silent stages at existing tracks through the original music call
(Modernized option, default on, Vanilla untouched), or leave the original
silence. No new audio, no replaced disc files.

**Acceptance:** findings documented (original silence or recomp loss; how
levels pick music); with the chosen option each challenge stage plays the
mapped track from level start, through death/Continue and restart, without
breaking voice lines or the stage timer; stage 5 keeps its own music; Vanilla
keeps the original; the user confirms.

### D18F - Recomp-owned FMV playback and replacement/mod foundation

**Status: Todo. Backlog only; not selected for implementation. Dependency: D18
reliability evidence and outstanding playback acceptance. Requested 2026-10-27.**

Give the recomp ownership of FMV playback, following the project's existing
custom texture, model and sprite asset paths. Preserve the original movies as
the default while gaining explicit control of timing, presentation and audio.
Build a foundation for future enhancements, replacements and mod support;
this request does not select remastering or replacement content now.

Start from [D18 findings, note 281](documentation/281-d18-fmv-reliability.md):
eight native table entries, original movie counts and sector selections,
startup/optional/ending routes, XA audio, seek/teardown behavior and measured
cadence. The fixture coverage does not prove natural campaign lifecycle or
listening/sync acceptance. Carry those gaps and the unresolved offscreen audio
starvation forward; they are not evidence that a new player already works.

When selected, inspect the current asset override conventions and movie entry,
completion, skip and audio ownership points. Design a host playback boundary
that leaves game progression and eligibility gates intact, while the recomp
owns media resolution, decoding/scheduling, presentation and audio delivery.
Decide whether to reuse existing decoding components or add a backend based on
measured fidelity and integration cost. Do not preselect a codec or renderer
without that inspection.

Planned scope:

- Resolve movies by stable game-facing identity, with owned original assets as
  the default and an optional external override path. Document format/version,
  asset lookup/precedence, metadata and missing/invalid replacement behavior.
  Keep original disc/media untouched; do not distribute extracted retail assets.
- Use explicit timestamps and a documented clock/audio synchronization policy.
  Preserve original playback speed, source cadence, aspect ratio and audio;
  60/120 presentation must not accelerate the movie or invent source frames.
  Keep enhanced scaling, interpolation and replacement audio opt-in future work.
- Define start, skip, completion and cancellation contracts, buffer/decoder
  cleanup and audio ownership transfer. Prevent overlapping original/host movie
  audio, stale playback after skip, repeated speech-bank reads, or lost gameplay
  music when control returns. Coordinate channel controls with D21 without
  silently including that separate job.
- Support display resize/fullscreen and clean exit during playback. Specify
  focus-loss/pause and device-change behavior, with useful playback diagnostics.
- Preserve Vanilla's existing playback path and provide a documented fallback
  for Modernized if a replacement is absent, invalid or fails. Decide failure
  timing and recovery explicitly so fallback cannot double-play audio or leave
  game state waiting forever.

**Acceptance:** the original eight-clip matrix passes on the owned assets with
verified visual/audio fidelity, duration/cadence and listening/sync; startup,
optional and natural ending/credits routes preserve their game gates and return
states. Early/late skips, repeated playback, resize/fullscreen and quitting are
safe at 60/120 presentation. Gameplay speech/music handoff remains correct;
Vanilla is unchanged. A small non-retail test replacement demonstrates the
lookup, metadata/timing and fallback contract without committing retail media.
Document the authoring interface, supported formats, limitations and extension
points for future mod/replacement work. Record hardware-backed results and
remaining platform gaps; private fixtures alone do not close acceptance.

No implementation, build or game launch authorized by this backlog intake.

### D22 — Campaign fidelity and overlay coverage

Maintain a level-by-level matrix for loading, progression items, enemies, weapons, bosses, saves, movies and end-game flow. Investigate missing native coverage according to observed correctness/performance needs. Treat extra game modes as explicit test scope, not implicitly supported features.

**Acceptance:** the intended single-player campaign has recorded completion evidence and no known progression blocker; failures get reproduction steps and dedicated jobs. Record native/interpreted boundaries honestly. This long-running job must not prevent earlier prototypes.

**Split 2026-10-30 (user request: smaller, manageable jobs; no work started).** D22 is now a parent tracker. Its work is divided into D22E-D22O below; D22 closes when they are Done and no known progression blocker remains. D22A-D22D are earlier bounded bugs and stay as recorded. The matrix lives in [note 330](documentation/330-d22e-campaign-matrix.md). Every sub-job uses private save/card copies and offscreen runs where possible, never the player's saves or memory cards; Vanilla stays unchanged. Failures found in any sub-job get their own dated job with reproduction steps rather than being fixed inside the survey job.

### D22A - Portal transition loses Modernized controls and first person (slot 7)

**Accepted (2026-10-04, user playtest).** "Level 2 is working and is completely
playable. The teleport/level transition is functioning correctly." Cause found
and fixed: the control lease
authenticated only the LEVEL00 overlay body; the portal loads LEVEL01.OVR over
the same base, so it failed closed. LEVEL01 is now an authenticated level
overlay alongside LEVEL00. See the work log and
[note 100](documentation/100-d22a-portal-level-identity.md).

User report on 2026-10-03:
load UI save slot 7 (file 06), walk into the portal and start the next level.
Duke then loses all Modernized controls and first-person view. The user reports
this as an easy reproduction; no independent reproduction or root-cause claim
has been made in this documentation-only session.

**Expected:** when playable control resumes in the next level, the selected
Modernized movement, independent camera/aiming and first-person preference
remain available. A temporary scripted camera during the transition must not
permanently disable them or reset the player's saved preferences.

Verify the current save identity and create a dated private copy before testing;
do not assume an older slot-7 snapshot still represents this portal. Use
background/offscreen tests with private cards/profiles. Trace the level/overlay
transition, supported-code identity checks, modern-control lease, input capture
and first-person activation/state restoration. Determine whether this is missing
next-level coverage, transition state invalidation, or another cause before
changing hooks. Do not bypass code/overlay guards to force controls on an
unverified level. This bounded bug can be investigated without completing the
whole D22 campaign audit; record any necessary verified coverage explicitly.

**Acceptance:** reproduce the portal route from the private slot-7 copy,
identify the cause and implement a guarded fix so Modernized controls and the
selected first-person view work when gameplay resumes. Verify movement, mouse
look, aiming/fire, capture/pause/resume and first-/third-person selection in the
new level; repeat the transition and check the accepted first-level baseline.
Vanilla stays original, preferences and player saves remain intact. User confirms
control/view continuity. Do not infer full-campaign support from this one route.

### D22B - Modern controls and first person in every level and transition

**Accepted (2026-10-05, user: "Accepted.").** The user then played large parts
of the game through `level N`: armed rolls (Ctrl) in Level 9 work, and general
game-wide playability is "very encouraging". Executable
`5b486ce0ba6ad569d03f0246a8a82edc81631c0c350a50f5eb3153e42c394017` is the
regression baseline. Candidate summary: all 21 selectable levels now authenticate
(one rule derived from the owned disc by `tools/local/level_overlay_guards.py`);
dodge rolls and steep-slope slides keep the mouse camera and view; the
original's own back-steps/strafes are taken over by the lease; unowned jumps
keep the camera in third person too. Transitions (statistics screen, savestate,
pause, level select) return by themselves. Swimming first person is D11E.
See the work log and [note 102](documentation/102-d22b-every-level.md).

Selected as the follow-up to D26A, 2026-10-04. User, on
accepting D26A: "the modern controls dont carry over, and first person etc. so
stand up the next job ... as making all that work." D26A now reaches every
level: `levels` / `level N` in the backtick console. Offscreen survey
(documentation/101): Modernized controls and first person are live only in
LEVEL00 (TIME TO KILL) and LEVEL01 (DUKE HILL). In the other 19 selectable
levels (2, 3, 5-12, 21-29) the lease refuses (`[TTK identity] level overlay tag
... is not an authenticated level`, `fp=lease`), so the original pad controls
and third-person view return. Scope includes the selected first-/third-person
view, mouse look, view aiming and the Modernized conveniences, not only
movement.

Earlier direction, 2026-10-04: "There should basically be no situation
where the classic controls override modern controls ideally. We want full,
total modernization, and full ownership of the controls system end to end."

D22A made the lease accept one more authenticated level (LEVEL01). Every other
level, and every state the lease does not yet cover, still drops back to the
original pad layout. This job removes that as a normal player experience:

- Level coverage: verify each LEVELxx.OVR (load base, tag word, run-time data
  such as the LEVEL00/LEVEL01 scratch tail) and authenticate it in
  `level_overlays[]`, or replace the per-level allowlist with an equally strict
  but general rule derived from the owned disc. Use D26A to reach each level.
  Keep the fail-closed principle for genuinely unknown code; the goal is to
  verify it, not to bypass it.
- State coverage: inventory every gameplay state where the lease refuses
  today (for example ladders, scripted cameras, vehicles/turrets, cutscene
  hand-offs, death/Continue, level start/end) and give each a modern
  equivalent or a documented reason it must stay original.
- Transitions: level loads, portals, Continue, savestate loads and menus
  return to the same Modernized controls and view without player action.
- Diagnostics: a lease refusal in normal play is a bug to report with its
  reason (`[TTK lease]` / `[TTK identity]`), not a silent fallback.

**Acceptance:** in every level `levels` lists, reached with D26A and through
the natural transitions, Modernized controls (movement, mouse look, aiming,
fire, jump/traversal) and the selected first- or third-person view stay in
charge from level start to level exit, including transitions and savestates; any remaining original-control state is listed with its reason
and a follow-up job. Vanilla stays original. Fix shared systems, not
individual levels, wherever the evidence allows.

### D22C - Level 11 starts with Legacy controls (control mode must persist)

**Accepted (2026-10-05, user: "I accept this work").** Earlier: Needs playtest. Reproduced and fixed; not specific to Level
11. An F10 release followed by an F10 recapture left automatic capture opted
out, so the next host release (opening the backtick console for `level N`,
menus) arrived in the new level with the mouse free and the original
controls until F10. F10 that captures now opts back in. Candidate
`f9d4a09a8a6c946f5717d4ca6eb20164f99bf7757bd88504e672ca60fd646c8b`. See the
work log.

User report, 2026-10-05 (D22B game-wide playtest): loading Level 11
(LET THE GAMES BEGIN with `level 11`) unexpectedly started in Legacy controls;
F10 restored Modern controls. Level transitions and the debug level select must
not change the player's selected control scheme. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Investigate the reset path: level or player-state initialization, mouse
capture (F10 is the capture toggle, so check whether capture was released
rather than the mode changed), savestate/level-select behaviour, configuration
reload, or a lease refusal specific to that level. The D22B offscreen survey
reached Level 11 with the lease live, so reproduce with the user's real
window/focus path too.

**Acceptance:** Level 11, reached by `level 11` and by natural progression,
starts in the selected control mode with capture as before; other levels
unchanged; the user confirms.

### D22D - Energy weapon fires but has no effect on enemies

**Done (user-accepted 2026-10-19, user: "it's perfect!!").** Modernized
view-aimed beams lock onto the enemy under the crosshair and damage it;
executable `3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98`
is the regression baseline. See the work log below.

**User report, 2026-10-08:** "The energy weapon in slot 7 does not work.
It fires but it has no effect on enemies." (Key 7, weapon slot 10; its upgraded
form, SUPER ZAPPER, upgrades in place.)

First establish whether this is the original's behaviour or a recomp/
modernization regression: compare Vanilla in the recomp and the original in
the DuckStation portable copy on the same enemy, base and upgraded. Then
check the modern aiming path (D07C projectile coverage, D08Q3 view aiming),
whether the shot's hit test or damage call runs, ammo use, and whether some
enemy types are immune by design. Fix the system, not one enemy.

**Acceptance:** cause documented with evidence; the energy weapon damages
enemies as in the original, base and upgraded, in first and third person, in
at least two levels; other weapons and Vanilla unchanged; the user confirms.

**Work log, 2026-10-19 - Needs playtest.** Cause: a D07C Modernized
regression, not the original. The beam updater `0x800733C8` damages only the
actor in projectile `+0x14`; D07C's beam completion cleared `+0x14` on every
view-aimed beam, so no beam could damage. Same savestate, same pig cop:
Vanilla locked 5 of 6 beams and killed it (flags `0x1` -> `0x803`); the
Modernized baseline left 16 of 18 beams unlocked, enemy untouched. Fix in
`weapon_aim.cpp`: the first beam locks onto the targetable actor under the
crosshair (room targetable list, not dead, within the original reach of
6000), or none (a miss, no auto-aim); the upgrade's extra beams keep the
original's other locks inside 45 degrees of the view, never doubling the
crosshair actor. The completion guard no longer races the host input
sequence (up to 8 of 13 beams had kept the original auto-lock). Executable
`3442279bb8cdde64dbb8d7239586b7f769d5aa311dfae372d61390d1e374ac98`. Lab:
levels 1 and 0, enemy types 3 and 4, first and third person, kills after
1-5 locks; 8 degrees off is a clean miss; upgraded extra beam verified;
Vanilla unchanged. `ttk-aim-test` links again (D19J stubs) with a new D22D
case; controls, input and Python (146 OK) suites pass. Not verified: the
user's playtest, bosses and other enemy types, levels beyond 0 and 1. Found:
D07E (stale-lease shots use the original auto-aim; `original` weapon aim
keeps a stale aim vector). [Note 152](documentation/152-d22d-energy-beam.md).

### D22E - Campaign matrix and evidence baseline (documentation first)

**Status: Done (user-accepted with the D22E-D22O job list; closeout authorized 2026-10-30, [note 330](documentation/330-d22e-campaign-matrix.md)). Dependencies: D01.** Create one engineering note holding the campaign matrix: one row per campaign level (all 21 selectable levels, in the authoritative order from D26A/D26D where known) plus the ending. Columns: load, handoff to the next level, progression items, enemies, weapons, bosses, saves, movies/scripted sequences, native coverage and completed-by-play. Each cell carries an evidence label: user playtest (with date), automated/offscreen run, inferred from an earlier job, or unverified. Fill only what existing jobs, notes and the handoff already prove (D22A-D22D, D22B's every-level controls pass, D26A level select, D20B statistics screen, earlier level-specific jobs). No game runs, builds or code changes.

**Acceptance:** the note exists, is linked from the board and status, and every cell is either filled from cited evidence or marked unverified. The remaining gaps define the scope of D22F-D22O.

### D22F - Level load and level-to-level handoff sweep

**Status: Todo. Dependencies: D22E.** For every level, using background/offscreen runs and the debug console (`level N`, `endlevel`): the level loads from a new game route and from level select, play starts in Modernized with controls and the selected view, and the level-complete/statistics screen hands off to the correct next level. Record any hang, wrong destination, missing overlay or fallback. Load and handoff only; it does not prove the level can be completed by play.

**Acceptance:** matrix load and handoff columns filled for every level, with failures split into dedicated jobs.

### D22G - Per-level native/interpreted coverage measurement

**Status: Todo. Dependencies: D22E. Coordinate with D23G.** Per level, capture dispatch-miss, interpreter/fallback and dirty-code counters for a representative route (load, early play, one transition). Record which overlays and code paths run natively and which are interpreted, honestly and without claiming exhaustive dynamic coverage. Measurement only; making more code native and the fast-path work belong to D23G.

**Acceptance:** the native coverage column is filled per level with the measured route named, and any correctness or performance problem traced to interpretation becomes a D23G item or a dedicated job.

### D22H - Progression items and objectives per level

**Status: Todo. Dependencies: D22E.** Per level, list the required keys, cards, switches and mission items (from data, notes and play) and confirm each can be obtained and used, and that the gate it opens works, in Modernized first and third person. Use private saves placed before each gate. Legibility is covered by the D08A5 mission inventory (D29 cancelled 2026-10-30); this job only checks that progression works.

**Acceptance:** progression column filled per level; each item/gate is verified or has a dedicated blocker job.

### D22I - Enemies and weapons per level

**Status: Done (user-accepted, 2026-11-08: "they are your flying enemies. they are famous in this game for not being easy to kill, only really the energy weapon works on them. however with our hitscan logic, you can shoot them now with the desert eagle. this is amazing and a much welcomed change to the game. well done accepted"); closed 2026-11-08. [Note 412](documentation/412-d22i-enemies-weapons.md). Dependencies: D22E.** Result: all 14 weapons damaged an enemy in every non-boss level; every placed hostile type took damage except 540 (explosives only), 605 (none) and the flyers 44 (killed in play by the user with the Desert Eagle via hitscan) and the untestable 847/848; bosses take damage; no placed weapon or ammo pickup is missing. Split: D22I1; D22I2 resolved by the user (both turrets are the original design). Original scope: per level, confirm the enemy types present spawn, react, take damage and die, and that every weapon available there fires and damages correctly with Modernized aiming (D22D covered the energy weapon on levels 0 and 1 only). Note pickups and ammo that fail to appear. Bosses are D22J.

**Acceptance:** enemies and weapons columns filled per level; failures split into dedicated jobs.

### D22I1 - Spawn tool: path, wall and ceiling creatures and mounted turrets

**Status: Todo. Dependencies: D22I.** `spawn enemy` cannot place classes 21, 27 and 52 (types 44, 847, 848): they leave the spawn point at once along fixed lines above or away from Duke and no shot can meet them. Type 44 is the flying enemy; the user killed them in play with the Desert Eagle via AUTO AIM HITSCAN (2026-11-08, private UI slot 8), so only 847 and 848 lack any damage evidence. Wall-mounted turrets (540) spawn in mid-air. After such spawns, later spawns in the same level often fail ("Spawn could not complete"). Find how the original places these (their spawn points, rails or mounts) and spawn them in a reachable, visible place, then run the D22I sweep (`recomp/analysis/d22i/sweep.py`) for those types. [Note 412](documentation/412-d22i-enemies-weapons.md), finding 3.

**Acceptance:** each of 44, 847, 848 and 540 can be spawned visibly in front of Duke in its levels, later spawns keep working, and the sweep records whether each takes damage and dies.

### D22I2 - Damage rules of turrets 540 and 605

**Status: Done (resolved by user report 2026-11-08; no work). Dependencies: D22I.** The D22I sweep found the level 1 gatling turret (540) ignores bullets and dies to explosives, and the level 3 turret (605, health 60000) takes no damage at all. User: 540 "that's actually how the game plays. you have to destroy it with an explosive" (misleading after level 0's Desert Eagle turrets, but kept as is); 605 "is quite simply a turret. the turret cannot be harmed, you have to deactivate it". Both are the original design. [Note 412](documentation/412-d22i-enemies-weapons.md), findings 1-2.

### D22J - Boss encounters

**Status: Todo. Dependencies: D22E.** Identify every boss encounter and its level. For each: the encounter starts, the boss can be damaged and defeated with Modernized controls and aiming in first and third person, and defeat triggers the correct progression. Use private saves placed just before each boss; the user's playthrough confirms difficulty and feel where automation cannot.

**Acceptance:** each boss has a recorded win (automated or user playtest) and correct progression, or a dedicated blocker job.

### D22K - Save and load round-trips per level

**Status: Todo. Dependencies: D22E.** Per level, verify SAVE GAME / LOAD GAME (savestates) and the statistics-screen save: saving mid-level and after completion, then loading, resumes in the right place with inventory, health, objectives and Modernized controls intact. Private cards and saves only.

**Acceptance:** saves column filled per level; any lost state or failed load has a dedicated job.

### D22L - Movies and scripted sequences per level

**Status: Todo. Dependencies: D22E. Related: D18F.** Inventory the FMVs and in-engine scripted sequences tied to each level (intros, cutscenes, between-level movies) and confirm each plays once at the right time, with audio, and returns control correctly. Recomp-owned FMV playback and replacement is D18F, not this job.

**Acceptance:** movies column filled per level; missing, repeated or stuck sequences get dedicated jobs.

### D22M - End-game flow: final level, ending and return to title

**Status: Todo. Dependencies: D22E, D22J.** Verify the final level's completion, the ending movie and credits, and the return to the title or menu, plus any post-game unlocks the original grants. Use a private save near the end.

**Acceptance:** the ending plays through to its normal conclusion in Modernized with no hang, recorded in the matrix.

### D22N - Extra modes: explicit test scope (challenge stages, two-player)

**Status: Todo. Dependencies: D22E.** List the modes outside the single-player campaign (challenge stages 21-26, two-player split-screen per D34B/D34C, character selection) and record each as tested, partially tested or untested, with what was observed. These modes are not part of D22's campaign acceptance; this job prevents them from being implicitly claimed as supported. D18E (challenge music) stays separate.

**Acceptance:** a short scope table in the matrix note with an honest status for every extra mode.

### D22O - Completion playthrough evidence and closeout

**Status: Todo. Dependencies: D22F, D22H-D22M.** Build a short checklist of the levels, bosses and the ending that still lack completed-by-play evidence and present it to the user for playtesting, which can be done in level groups across several sessions. Record each completion with its date and executable, update the matrix, and close D22 when every campaign level has completion evidence and no known progression blocker remains.

**Acceptance:** completed-by-play column filled for the whole campaign; D22 parent marked Done with the user's confirmation.

### D23 — Performance budgets and long-session stability

Define representative hardware and scenes, then measure frame pacing, audio underruns, memory growth, loading and shutdown. Exercise repeated level transitions and a sustained session in each available mode.

**Acceptance:** documented measurements satisfy the chosen budgets or identify explicit supported-setting limits; no unresolved leak, hang or audio regression in the tested route. Use profiling evidence before optimizing.

### D23A — Modernized frame-budget regression (guard identity cost)

**Done - 2026-09-29 (user-accepted):** "the in game stutter fix works fine".

User: "that stuttering audio/slowness
issue" before the D08Q playtest. Reproduced in isolation on this machine:
Modernized **47.5–49.3 fps** with continuous audio underruns (output fill
17–34 ms vs 180 ms target) while Vanilla ran 60.0 fps clean; the Sep-27
build sat at 57.5. Root cause measured: `ttk::identity()` compared all
106 guards (81 KB, 20,305 words) word-by-word through the guest bus on
**every** call — 46 calls per frame × 108 µs = **5 ms/frame, 24 % of wall
time**; the aim guards added a second copy. Fix: `memcmp` against the
runtime RAM image with the exact per-word path as fallback (~28 µs), and a
per-frame verdict memo keyed on the host frame and `g_dirty_ram_code_gen`
(overlay loads/restores still invalidate at once). Result: **59.94 fps, 0
underruns, fill 267 ms**, 1 check/frame, 0.17 % wall; jetpack flight 60.1.
Debug JSON: `identity_calls/checks/us`. ttk-input/aim/controls tests PASS
(new memo case). Likely the mechanism behind D18A's historical club
starvation, not re-measured there. See
[58-modernized-frame-budget.md](documentation/58-modernized-frame-budget.md).

**Acceptance:** the user's session no longer stutters or slows in
Modernized play (turret room, ledge, jetpack); `audio_stats.out.fill_ms`
stays near/above target with flat underruns; Vanilla unchanged.

### D23B - Intro FMV stutter: stranded native movie shard

**Done - 2026-09-29.** User: "im still getting the stuttering, but only on
the fmv at the beginning?" after D23A, then "make the playback more robust".
Cause measured: the native MOVIE.OVR shard's cache folder is keyed by the
`game.local.toml` overlay config hash, which includes the host hook list.
The 28 Sep hook additions moved it from `gc01bd77ae` (last shard, 27 Sep) to
`gc15256f69` (empty), so the intro's VLC decoder ran interpreted: ~52 fps,
continuous underruns, 30 % interpreter share. The loader reported this only
over the debug port. Shard rebuilt: 59.92 fps, 0 underruns, fill 267 ms,
4 % interpreter; the user confirmed smooth playback. Hardening: always-run
`ttk-movie-shard` CMake target in the `local-dev` preset, `run.py` pre-launch
check (`build_movie_overlay.py --if-ready --quiet`, ~0.2 s when current),
and a `ttk-fmv:` session-log line from `fmv_poll.c` that reports the native
decoder or a WARNING with the loader's mismatch message. See
[59-fmv-shard-namespace.md](documentation/59-fmv-shard-namespace.md).

**Acceptance:** the intro FMV plays at full speed without audio underruns
after hook changes to `game.local.toml`, and a missing shard is repaired on
build/launch or reported in the session log.

### D23C - Medieval castle moat slowdown with Necros (performance polish)

**Done (user-accepted), 2026-10-26.** User approved closeout. Current user playtest and bounded measurements do not reproduce the original sustained slowdown. The brief water-entry game-image cadence dip remains recorded with its cause unresolved; no optimization made. See [note 253](documentation/253-d23c-moat-recheck.md).

**Original user report, 2026-10-05 (D22B game-wide playtest).** In the
medieval castle/moat level (reported as Level 6), the initial section slows
down, particularly while the Necros around the moat are active. Gameplay is
not broken and stays completely playable, but the quality drop is perceptible
compared with better-performing areas. Level numbers in this report follow the D26A console (`level N`); D26D will
confirm how they map to the game's own campaign numbering.

Do not optimise blind. Profile first and determine whether the slowdown
correlates mainly with the number/type of active enemies, Necro AI or
animation, visibility/render workload, original simulation workload,
effects/projectiles, high-refresh interpolation, or another system. Reuse
the D23A/D17 frame-budget tooling (`phase_profile`, `phase_hot`, fps stats).

**Acceptance:** a measured profile names the dominant cost; a fix, if any, is
shared and verified against the accepted baselines; the user confirms.

### D23D - Minor slowdown in Level 9's strip-club-type area (low priority)

**Done, user-accepted and closeout authorized 2026-10-26.** The user tried to
reproduce the reported slowdown and encountered no problems during the playthrough.
The original report is retired on that user evidence, as requested. Unlike D23C's
retained brief water-entry observation, no residual issue was reported for D23D.
No new automated profiling, gameplay changes, rebuild or launch was needed;
the historical cause and attribution to any prior optimization remain unconfirmed.
Reopen if a repeatable slowdown returns, checking contemporaneous host activity
and gameplay telemetry before changing code.

Historical report (2026-10-05): minor slowdown around Level 9's strip-club-type
area, low priority and nonblocking. Level numbers follow the D26A console
(`level N`); D26D will confirm their campaign mapping.

### D23E - Stutter with enemies on screen while walking (UI slots 9 and 10)

**Accepted 2026-10-07** (user: these are "now working significantly better"). **User report, 2026-10-05.** Load UI save slot 9 (file 08), look down
toward the enemies on the ground, then walk left and right: the game stutters
in this area. The user suspects the characters on screen cause it. UI slot 10
(file 09) may show the same effect: turn on `dnkroz` first because enemies
attack immediately, then walk up and down the strip. Reported savestates
(`recomp/saves/local-play/openbios`, 2026-10-05; copy them for tests, never
write to the originals):

- UI 9, `state_800AB6FC_slot08.pst`, SHA-256
  `3fe93bb39eb114f5c22d349ed030a68a14b14b1cfac86b336b762cd507525c8f`
- UI 10, `state_800AB6FC_slot09.pst`, SHA-256
  `c1dc55452203ab3864da17a440278bded255608679698604d92aa5959d09ceaf`

Record which level each slot is in. Possibly related to D23C (moat Necros),
D23D (Level 9 strip-club area) and D17H (platform movement stutter): check
whether this is the same cause before fixing anything separately, and merge
or cross-reference the jobs if it is.

Do not optimise blind. Profile with the D23C method (`phase_profile`,
`phase_hot`, fps stats) and determine whether the stutter follows the number
or type of visible characters (skinning, animation, AI), render workload,
original simulation time, or high-refresh interpolation/presentation. Compare
the same route with the enemies looked away from, with `dnmonsters` off, in
Vanilla and Modernized, and at 60 and 120 Hz.

**Acceptance:** a measured profile names the dominant cost on both routes; any
fix is game-wide (not per level), keeps original gameplay timing and passes the
accepted regression baselines; the user confirms the stutter is gone or
acceptably reduced.

**Work log 2026-10-05 - Needs playtest.** Both slots are in the western town.
Measured offscreen on the real GPU with the player's settings and private
copies ([note 104](documentation/104-d23e-busy-scene-stutter.md)). Two causes:
(1) at the player's `cpu_overclock 100` the 16:9 view gives the emulated CPU
more than a 3-field frame's work, so game frames land on 4 fields (15 fps)
with 5-6 field steps (Vanilla 4:3: steady 3 in slot 9; slot 10 is 4-5 in
Vanilla too); draw distance has no effect. (2) At 120 Hz the default 150%
could not hold: late-camera redraws (120/s, about 1.4 ms each, 19% of the
emulation thread) pushed emulation behind real time, and the D08Y safety net
paused the overclock for 5 s at a time (20 <-> 15 fps swings). At 150% and
60 Hz both slots hold a steady 3. Fix (game code only): the redraws shed load
first (`replay_shed_load`: presents every 2nd/3rd refresh, existing step-up
rule), and the overclock pauses only when nothing is left to shed. 150%,
120 Hz: slot 9 from about 30% to 93% of frames at 2-3 fields, slot 10 98% at
3 over 90 s; light slots unchanged. Native tests pass. Candidate
`18fc1c6e63f182217c9b2c6b6aaf8d21fa224394d46fdbcd906afad5bf8fd19e`.
Remaining: the user plays slots 9 and 10 at 150% (and judges the lower present
rate there) and real audio; at 100% the stutter is the emulated CPU itself.
Not shown to be the same cause as D23C/D23D/D17H; measure those the same way.

**Follow-up 2026-10-05.** The user confirmed 150% relieves the stutter but
prefers 100% for now: 150% felt different, with more visible seams and a
slight audio degradation at the start. Measured: identical still images and
wall subdivision at both speeds; at 150% and 120 Hz the picture rate drops in
light scenes too (slot 1 75/s, slot 9 54/s against 120 at 100%), because one
emulation thread cannot do both. That work moved to D23F. D23E stays Needs
playtest at 100% (its shedding applies whenever emulation falls behind).

### D23F - Faster timing model: emulation-thread budget for 150% at high refresh

**Accepted (2026-10-05).** User: "the behaviour is amazing. the behaviour is
absolutely beautiful, and a real pleasure to play ... the responsiveness is
literally like PC accurate now", and asked to make it "the entire new default
method of launch". `cpu_timing` defaults to fast from profile schema 26 (older
saved `accurate` switches once; Vanilla stays accurate). Follow-up work is
D23G.

Implemented as a Modernized profile setting `cpu_timing` (`run.py --cpu-timing
fast|accurate`):
a purpose-built timing model for the recompiled game code, force-included
into the generated shards (generated C and hashed headers untouched, so
savestates still load), leased from gameplay like the overclock. At 150% and
120 Hz slots 1, 9 and 10 keep 120 presents per second with no shedding or
overclock pauses; slot 9/10 60 s routes steady 3 fields. See
[note 105](documentation/105-d23f-fast-timing.md) and the work log. Remaining
for acceptance: the user's playtest of feel and audio, and the decision on
the default.

**Opened 2026-10-05 from D23E.** The user wants the best gameplay
as the default and likes the faster timing model concept from the Disruptor
reference ([note 93](documentation/93-disruptor-reference-research.md),
section 9). They play at 100% CPU for now: at 150% the game felt "different,
not smoother" and seams looked more visible.

D23E evidence ([note 104](documentation/104-d23e-busy-scene-stutter.md)):
150% draws pixel-identical still images and the same whole/subdivided wall
counts as 100%. What changes is the picture rate: on the player's PC
(i7-5960X, GTX 1080 Ti) one emulation thread cannot run the faster game
(more game frames per second) and 120 in-between pictures at once, even
walking in a light scene (slot 1: 120 presents/s at 100%, 75 at 150%; slot 9:
120 against 54). Either presents drop (D23E shedding) or, before D23E, the
overclock paused 5 s at a time.

Emulation thread at 150%, moving in slot 1 (`PSX_PROF`): cycle-timing model
about 34% (`psx_cyc_base`, `psx_cyc_lds`, `psx_cyc_charge`, `psx_cyc_step`,
instruction-cache simulation, dependency tracking), interrupt checks about 4%,
the recompiled game code 4%, GL driver and redraw/batch paths about 12%,
always-on observers (call/trace logging, `debug_server_cyc_observe`,
`fntrace_record`, `xprobe_event`, parity/overlay watches) about 4-5%.

Work, in order:

1. **Observers off in player sessions** (small, low risk): find which
   per-store/per-call observers still run with `PSX_FORENSICS=0` and gate them;
   helps 100% too.
2. **Faster timing model** (the core): a design study before code. Disruptor's
   `disruptor_fast_timing.h` levels: no per-block diagnostics, no game
   instruction-cache simulation, static per-basic-block cycle charges, flat
   charges for eligible RAM loads, an inlined branch-edge check that services
   interrupts and accumulated charges at a threshold, explicit fallbacks
   (device/MMIO/BIOS, polling edges, save/debug maintenance). Decide what is
   computed at recompile time versus runtime, how it coexists with the TTK
   overclock lease (`psx_overclock_compress`), render workers
   (`g_psx_render_untimed`) and idle skipping, and whether it changes hashed
   framework headers or generated code identity (savestate compatibility: see
   the codegen-hash rule; the user accepts losing savestates for this job). Framework edits go through
   `export_runtime_patch.py`.
3. **Cheaper in-between pictures** (optional): fewer GL state changes per
   redraw feed, or drawing redraws off the emulation thread.

User direction (2026-10-05): "im completely happy to build anything new rather
than patch over old crap". Prefer a clean, purpose-built timing system over
layering more patches onto the existing cycle model, even when that is larger;
keep the framework patch export workflow. Savestates: "i genuinely dont mind
if we lose save states" (2026-10-05), so D23F may change hashed framework
headers or generated code identity; say so when it happens, and tests still
use private copies (never the player's files).

Vanilla keeps the accurate model unless the user decides otherwise; the fast
model is a Modernized option until proven.

**Acceptance:** at 150% and 120 Hz the player's PC keeps 120 presents per
second in light scenes and the western town (slots 1, 9, 10) without shedding
or overclock pauses; game frames at least as steady as D23E's 60 Hz result
(slot 9 steady 3 fields); audio, FMV, loading, CD streaming, savestates and
the accepted regression baselines unchanged; Vanilla untouched; the user
judges the feel at 150% and decides whether it becomes the default.

### D23G - Finish the fast path: dispatch, overlays, interpreter, observers and redraw cost

**Done (user-accepted 2026-11-02; step 5 note 356, step 3 closed as measured note 355). Earlier: part 2 (steps 1-2, dispatch fast path and per-call observers, [note 354](documentation/354-d23g-dispatch-fast-path.md)) user-accepted 2026-11-01. Part 1 (renderer feed, [note 331](documentation/331-d23g-renderer-feed.md)) user-accepted 2026-10-30; steps 3 and 5 remain. Selected 2026-10-30. All-in-one. Opened 2026-10-05 from D23F's limits** ([note 105](documentation/105-d23f-fast-timing.md)).
D23F moved the statically recompiled game code to the fast model. What is
left on the emulation thread (fast, slot 9, 150%): the call/return dispatch
path about 13% (`psx_dispatch_impl` -> BIOS table miss -> `dirty_ram_dispatch`
-> `psx_game_find_entry`, `dirty_ram_text_native_ok_ranges_from`,
`dirty_ram_is_dirty`, plus `fntrace_record`, `xprobe_event`,
`text_xlate_on_dispatch` on every hop), GL driver and redraw feed about 5-12%,
level overlay libraries about 2%, the interpreter about 5%.

Work, measured one by one with the D23F harness:

1. **Dispatch fast path**: a purpose-built lookup for clean game text
   (direct table or cache keyed by address, invalidated by the existing
   dirty-RAM/overlay generation) so a call or return does not walk the BIOS
   table and the dirty-RAM checks. Framework change through
   `export_runtime_patch.py`; keep identical targets (counter checks).
2. **Dispatch-path observers off in player sessions** (`fntrace_record`
   rings, `xprobe_event`, parity/overlay watches that are diagnostics),
   still available with `PSX_FORENSICS` and armed traces.
3. **Fast timing for level overlay code** (compiled overlay libraries) and,
   if measurable, the dirty-RAM interpreter, with the same lease and the same
   calibration.
4. **Cheaper in-between pictures**: fewer GL state changes per redraw feed or
   feeding redraws off the emulation thread.
5. **Vanilla host cost**: the D23F flag tests cost Vanilla about 3 points of
   host CPU (guest behavior identical); remove if a clean design allows.

Also investigate why the Vanilla regression route now diverges from its
long-standing captures with or without D23F (last matching run before D23E).

**Acceptance:** each step shows a measured emulation-thread saving with game
frame rate, savestates, audio, FMV and loading unchanged; Vanilla guest
behavior unchanged; the user's playtest finds no regression and the same
feel.

**Work log (2026-10-30, re-profile; no code change).** Current build (D17AM
unified geometry, D14F 90 FOV default), D23F harness copied to
`recomp/analysis/d23g-20261030/` (local only): offscreen real GPU, private
copy of the current player profile, the D23F private savestate copies, dummy
audio, idle host (load under 1, no backups running).

| Run (150%, moving) | Thread | Game frames/s | Presents/s | `[TTK cpu]` reduced |
| --- | --- | --- | --- | --- |
| Slot 9, 60 Hz (D23F: 0.49) | 0.58 | 26.1 | - | 0 |
| Slot 9, 120 Hz, FOV 90 (D23F: 0.73, 120.1) | 0.85 | 26.2 | 110.0 | 1 |
| Slot 10, 120 Hz, FOV 90 (D23F: 0.82, 120.2) | 0.94 / 0.93 | 21.6 / 21.9 | 107.1 / 106.7 | 2 / 2 |
| Slot 10, 120 Hz, FOV 60 | 0.91 | 24.0 | 120.4 | 0 |
| Slot 1, 120 Hz, FOV 90 | 0.78 | 30.0 | 120.1 | 0 |

Findings: the emulation thread has lost the headroom D23F bought. At the
player's 120 Hz the busy scenes (slots 9, 10) now drop presents; the same
slot 10 route at FOV 60 holds 120, so the wider default view is what tips it
over. At 60 Hz the dispatch path is still about 15% of the thread
(`psx_dispatch_impl` 4.0, `psx_game_find_entry` 2.3,
`dirty_ram_text_native_ok_ranges_from` 2.0, observers `fntrace_record` /
`xprobe_event` / `text_xlate_on_dispatch` about 1.8). At 120 Hz slot 10 the
largest cost is the GL feed: NVIDIA driver 17.8% self plus libc 7.4% and GL
callers (texture batch passes 3.8, flush 3.1, framebuffer binds 2.9,
enable/disable 2.9, replay begin/end and rect blits about 3); dispatch is
about 9%. FOV 90 against 60 grows GL driver, host vertex store, `d16_match`
and texture batch time. Caller attribution is approximate (`gl_perf_mirror_end`
appears although `PSX_GL_PERF=0` returns at once).

Proposed order by measured impact at 120 Hz: (a) step 4, GL feed and redraw
cost (fewer binds/state changes and draw passes per redraw); (b) steps 1-2,
dispatch fast path and per-call observers (the mod and text-translation hooks
in `fntrace_record` must stay); (c) step 3, overlay/interpreter. Target:
slots 9 and 10 hold 120 presents/s at FOV 90 with no reduced-present events,
with the D23G acceptance unchanged.

**Work log (2026-10-30, part 1: renderer feed; Needs playtest).** User
report: the UI slot 2 lake (saved 2026-10-05) is the one place that still
slows down, worst in the water; the user suspected draw distance. Measured on
private copies: in the water the emulation thread was 100% busy while
advancing 1-9 fields per quarter second (frames up to 53 fields), the GPU
peaked at 80%, and 46% of the thread was in the NVIDIA driver. The batch
counters showed over 1000 textured batches per frame, almost all split on
depth-tested versus painter-order primitives (ground versus translucent
water), each paying a full texture/uniform/upload setup and two widescreen
framebuffer binds. It is batching, not draw distance.
Changes (framework, exported patch): depth runs inside one textured batch
(`PSX_GL_DEPTH_RUNS=0` compares), `GL_DEPTH_TEST` in the GL state cache,
`d16_match` without its per-call 128 KiB stack-probe frame, memoised display
information, compact host-vertex probe arrays.
Results at 150%/120 Hz: D23F slot 9 110 -> 120.4 presents/s; D23F slot 10
107 -> 120.0-120.3; user slot 2 72-82 -> 110.6 with the longest game frame
3-4 fields (was 53) and no water hitches in two 40 s routes (17-19 slow
windows each before); user slot 10 unchanged at 120.3. Images after loading
slots 2, 8 and 10 are pixel-identical to the previous build. Native suites
pass; Vanilla 59.93 fields/s; pristine checkout plus patch reproduces all 55
framework files. Still open: the view right after loading slot 2 needs about
0.9 of the thread at 120 Hz and the pacer presents every second refresh for
about 5 s after the load. The slot 10 pop-in is room visibility, not frame
rate (slot 10 holds 120 at 0.66): new job D17AQ.

**Playtest (2026-10-30): part 1 accepted.** User on slot 2: "Buttery smooth
this is the best it's ever been by far ... it's playing perfectly the
stability is just absolutely next level ... studio grade quality in terms of
performance and feel." Part 1 closed out (commit/push authorized 2026-10-30); D23G stays open.

**Work log (2026-11-01, part 2: dispatch fast path and per-call observers;
Needs playtest).** Re-profiled on the D17AW build (slot 10 vista, 180 Hz):
with redraws on the render thread, the dispatch path was 12-13% of the
emulation thread at about 190 ns per dispatch (600,000 per second). Changes
(framework, exported patch): a cache of "this function's code is still the
original" answers, cleared by every code-page change (`PSX_TEXT_OK_CACHE=0`
compares, `=verify` re-checks every hit); one validity check per call instead
of two; the per-call `xprobe_event` record only with forensics or an armed
trip/watch; mod function-entry hooks matched by a compact address list; the
generated BIOS dispatcher sends game text straight to the game path instead of
a BIOS-table search that cannot match. The last one is emitter output, so the
codegen hash is now `0xebce51ea`; the user agreed to lose savestates, but the
regenerated game code and every BIOS function body are byte-identical, so
`boot_state.c` accepts the old hash (reviewed exception) and existing saves
load. `build.py` passes `--force-bios` so the BIOS C is always regenerated.
Results: 188-196 -> 147-148 ns per dispatch (about 23%, 2.5-3 points of the
thread). Verify mode over 21 levels: 80.4 million hits re-checked, 0
disagreements. Final build over the same 21 levels: 59.9-60.1 fields/s,
no errors. Images after loading six fixture slots identical to the old
build; old-hash saves load and a new save reloads; Vanilla 59.9-60.0 fields/s,
0 underruns; native suites pass; pristine checkout plus patch reproduces all 78
framework files. Playtest: usual play, loads from LOAD GAME (existing saves
must load), a level change, saving a new game and loading it.

**Playtest (2026-11-01): part 2 accepted.** User: "the stability now is
literally next level. lock it in, close us out!" Executable
`f53b391249e45ce816b56ef1cf0cb76d7e93cccfcad2001b43faa61da53afd6e` is the regression baseline
(codegen hash `0xebce51ea`). Part 2 closed out; D23G stays open for steps 3
and 5.

**Work log (2026-11-02, step 3: overlay and interpreter timing; measured,
no code change).** [Note 355](documentation/355-d23g-step3-overlay-interpreter.md).
Level overlays are not compiled libraries: the cache holds only MOVIE.OVR and
the geometry math routine (`0x800B3D9C`), and every `LEVELxx.OVR` runs in the
dirty-RAM interpreter. Over all 21 selectable levels, moving, the interpreter
runs 31,500-139,900 instructions per second (most of it the BIOS interrupt
handler; under 0.3% of guest instructions), phase share 0.9-3.1%; the math
library is called 6,100-25,300 times per second, phase share 0.6-3.7%. Slot 9
perf at 180 Hz: math library 0.56% of the thread, its accurate timing helpers
about 0.11%; `dirty_ram_dispatch` 1.38%; the text check's cost is part 2's
cache hits, not the math call's failing check. Fast timing here would save
about 0.1 of a point and shift D23F's calibration; compiling level overlays
would save well under one point. Recommendation: close step 3 as measured;
step 5 remains.

**Decision (2026-11-02): step 3 closed as measured.** User: "let's go with
your recommendation, then close it out". D23G stays open for step 5 only.

**Work log (2026-11-02, step 5: Vanilla flag cost; Needs playtest).**
[Note 356](documentation/356-d23g-step5-vanilla-flag-cost.md). Measured with
debug turbo (fields per CPU-second of the emulation thread, no pacing), D17AQ
fixture slots 9 / 1 / 10, private copies: the per-instruction flag tests cost
the accurate model 7-9% (current 40.9 / 51.7 / 99.2 against 43.6 / 55.6 /
108.1 for a build without the include), more than D23F's 3-point estimate.
Cause: byte stores in the accurate step may alias the flag, so it was
reloaded before every instruction. Change (`fast_timing.h`/`.c` only; no
generated C, emitter or framework edit; codegen hash unchanged): each
generated function's `psx_cyc_bb_defer_begin()` opening declares a local copy
of the flag, the hooks test the copy, and the branch edge refreshes it after
any host call; calls re-enter through the opening. Results: accurate 43.7 /
54.3 / 103.1 (all of the cost back in slot 9, about two thirds in 1 and 10);
fast (Modernized) 64.6 -> 65.8, 84.3 -> 87.6, 217.6 -> 226.9. Vanilla 60
fields/s, 0 underruns, thread 71.6 -> 64.2% (slot 1) and 67.2 -> 63.4%
(slot 9). Images after loading six slots identical; fixture saves load and a
new save reloads; levels 0-3, 5-12, 21-29 at 59.8-60.0 fields/s; native suites
pass; framework patch unchanged. Executable
`20c7e77484093a6e7ddbdc8cdd268d50bcf34cb0b3aeefc49a335910a0c51f6e`.
Playtest: a Vanilla session (`run.py --vanilla`) and usual Modernized play,
same feel, loads and a level change.

**Playtest (2026-11-02): step 5 accepted; D23G Done.** User: "I was really
thrilled with the performance, the game is feeling better than ever ... The
save states loaded with no issue in vanilla and modern play too. So i fully
accept." Session logs: Vanilla slot 1 load clean; Modernized about 14 minutes
at 59.9-60.0 game fields/s and 180 presents/s (lower minutes only with the
pause menu open), one brief audio-low pace step right after a load, saves to
slots 10 and 11 written. Executable
`20c7e77484093a6e7ddbdc8cdd268d50bcf34cb0b3aeefc49a335910a0c51f6e` is the
regression baseline. All D23G steps are done (step 3 closed as measured).

### D23H - Presents fall from 120 to about 60 over extended play

**Accepted (2026-10-06).** User, after playing with several saves: "i fully
accept ... this is absolutely wonderful work." See the work log below and
[note 106](documentation/106-d23h-present-rate-recovery.md).

**Opened 2026-10-06 from the user's evening play on the D23F default
build** (fast timing, 150%, 120 Hz, executable `e0737a9f...`). User report:
"after like 10 mins of gameplay, the frame rate goes from the buttery smooth
120 to what feels/looks like a lower one, like 60 or something." It does not
come back by itself during play.
After reviewing the log evidence, the user agrees the drop follows savestate
use (2026-10-06).

**Log evidence** (`recomp/build-local/logs/`, local only; the launcher keeps
the last five sessions):

| Session | Length | Saves | Loads | Presents reduced | Overclock paused 5 s |
| --- | --- | --- | --- | --- | --- |
| `session-20261005-233139` | about 26 min | 4 | 0 | 2 | 2 |
| `session-20261005-235829` | about 25 min | 6 | 2 | 2 | 5 |
| `session-20261006-002316` | about 2 min | 1 | 1 | 1 | 0 |
| `session-20261005-212814` | about 11 min | 0 | 3 | 1 | 0 |

- All 13 slowdown events in these sessions come right after a savestate save
  or load. There is no other `emulation behind real time` line. The measured
  host rate in that second is 6.6-32.3 frames/s, so a save stalls the
  emulation thread for a large part of a second. Loads report about 17 ms of
  work, so the save side (compression or file write on the emulation
  thread?) is the suspect. Not measured yet.
- The game logic rate itself holds: the `[FPS]` lines in `233139` show a
  steady 59.9 game fps (1.00x) between events. The drop the user sees is in
  presents (in-between pictures), not in game speed.
- Presents per second are not in the session log, so the "about 60" has not
  been measured directly. It is consistent with a present divisor of 2.

**Likely mechanism (from code reading; not yet reproduced):**

1. `overclock_lease()` (`src/ttk/modern_controls.cpp`) sees the save hitch as
   "emulation behind real time" and calls `replay_shed_load()` (D23E), which
   raises the present divisor from 1 to 2 (120 to 60 presents/s at 120 Hz).
   If it is already at its limit, it pauses the overclock for 5 s instead.
2. Stepping back up (`late_pace()`, `src/ttk/frame_replay.cpp`) needs
   `4 * pace_backoff` consecutive clean windows of 90 presents (under 2%
   repeats). Any window between 2% and 10% resets the count.
3. `pace_backoff` doubles (up to 32) whenever a slowdown follows within 3
   windows of a step-up, and **it never decays**. One-off hitches are
   treated the same as a busy scene. After a few saves in a session,
   recovery needs up to 128 clean windows: at 60 presents/s that is over
   3 minutes of perfectly clean play, and busy scenes rarely manage that.
   So the game appears to stay at 60.

The D23E comment says "a one-off hitch ... costs a few seconds", which holds
only for the first hitch of a session.

**Other causes to rule out** before settling on the above: a slow time-based
growth (memory, GL objects, worker processes, the trace or frame rings,
savestate buffers), thermal or GPU clock drops, and the separate
`late_pace()` step-down when repeats exceed 10% (not logged at all).

**Work:**

1. **Measure first.** Log present-rate changes (`pace_div` up or down with
   the reason: emulation shed, repeats, recovery) and a one-line summary
   every minute or so (presents/s, game fps, `pace_div`, `pace_backoff`, RSS)
   in the player session log, cheaply. Reproduce with a private profile copy
   and test card/state copies: play 15-20 minutes offscreen with periodic
   saves, and a control run with no saves. Confirm whether presents stay
   at 60 and whether the decline occurs without saves.
2. **Savestate saves must not stall gameplay**: take the snapshot on the
   emulation thread (the copy only) and compress and write it on another
   thread, or otherwise show where the time goes. Saves, loads and their
   rejection rules must not change (hashed headers untouched).
3. **Shedding must recover from one-off hitches**: ignore or discount the
   seconds around a known hitch (save, load, level load, worker start), and
   let `pace_backoff` decay with clean time so the session's history does
   not keep 60 forever. A genuinely busy scene must still settle on one
   steady rate (the D23E aim).
4. If step 1 shows a slow time-based decline as well, find it and fix it in
   the same job.

**Acceptance:** in a 20+ minute Modernized session at 150%/120 Hz with
several saves and loads, presents return to the display rate within a few
seconds of each save or load and stay there in scenes that held 120 at the
start. The busy western town (UI slots 9/10) still settles calmly as in
D23E. Savestates stay compatible, there is no audio regression, Vanilla is
unchanged, and the user's long play session confirms it.

**Work log 2026-10-06 - Needs playtest.** Measured offscreen (private profile
copy at the player's settings, private states, port 9273; harness
`recomp/analysis/d23h-20261006/`). The suspected `pace_backoff` was not the
cause. **A shed was permanent:** at 120 Hz and Match Display the default
present timeline never calls `late_pace()` (only at Unlimited), so nothing ever
undid a D23E shed. One hitch gave 60 presents/s and two gave 40 until restart.
**One-off hitches shed:** a save stalls emulation 83-98 ms (loads 36-57 ms), and a
second containing an F7 menu visit is far lower, so any single bad second shed.
Fix (game code only, `src/ttk`): `replay_load_window()` owns shedding in both
directions, judged once per gameplay second by `overclock_lease()` in every
frame-rate mode. It sheds only after two behind seconds in a row, does not judge
gaps (menus, loads) or seconds while the overclock is paused, steps up after
5 s x backoff clean seconds, doubles backoff on a failed try (max 16), and halves
it per clean minute at full rate. Added `[TTK pace]` session-log lines (each
change with its reason; a per-minute summary with presents/s and RSS) and
`render_replay` fields. Evidence: A/B with a 0.8 s stop every 20 s and a save
every 30 s: before 120 -> 60 -> 40 for good, with 5 overclock pauses; after
120 throughout. 21-minute run with 20 saves, 8 loads and 13 stops: 120 in 124
of 125 ten-second blocks; the one shed (a stop and a save in the same second)
recovered in 5 s. A 15 s sustained load returns 40 -> 60 -> 120 about 15 s after
it ends. Western town UI slots 9 and 10 with hitches: 119-120, no change.
Unlimited unchanged (repeat pacing only). Native and unit tests pass. No framework or
hashed header change (savestates compatible); Vanilla does not reach this code.
Executable `c6c221d1...`. Remaining: the user's long play session; Unlimited's
own repeat backoff still does not decay; saves still cost about 90 ms on the
emulation thread (no longer affects the rate); RSS rose 400 -> 466 MB over 21
min (rate unaffected, not investigated).

### D23I - Memory growth in long sessions (investigation)

**Status: Done (accepted 2026-11-09: history rings opt-in and a per-minute memory line). Dependencies: D23, D23H (Done).**
The user's session logs of 2026-11-09 (`recomp/build-local/logs`, `[TTK
pace] minute` lines) show resident memory climbing steadily in every
Modernized session at 180 Hz, first person, Polymorph: 401 to 511 MB in 8
minutes, 395 to 506 MB in 7, 402 to 893 MB in 21, 374 to 498 MB in 6, about
14 to 23 MB a minute. D23H measured 3 MB a minute over 21 minutes (note 106,
"worth a look in a longer session"); D23 traced earlier growth to the SIO and
audio history rings, bounded below about 84 MiB together (note 252). Today's
growth is well past that and unexplained. Every kept log postdates D53, so it
is not known whether D53 is involved.

Plan:
1. Rule D53 in or out: two matched offscreen runs (private save copies,
   same slot and inputs, a few minutes each) with `--security-monitors
   duke3d` and `ttk`, comparing growth.
2. Measure as D23 did: combined PSS of the game and its redraw workers,
   `/proc/PID/smaps` and pagemap attribution of new pages, separating bounded
   history rings from heap, mod memory, GL/driver mappings and worker
   re-forks (9 to 15 spawns per session in the logs).
3. Bisect against recent builds or switches (dynamic lights, sleeping
   enemies, sky, near clip) if the cause is not obvious.
Acceptance: the cause named with evidence; then a bounded fix or a reasoned
ceiling, agreed with the user, and a long-session run showing growth
flattening.

Work log 2026-11-09 (investigation): [note 433](documentation/433-d23i-memory-growth.md).
Seven offscreen runs (4-20 min; idle, savestate loads, level travel, mixed
play). A scratch heap tracker showed live malloc +0.47 MiB over 9 min while
RSS rose 41 MB: not a heap leak. Pagemap attribution put the growth in `.bss`:
`gpu.c` host-vertex aperture tables (two used in the main process, 54 MiB
each, high-water mark of the busiest frame, 23 MiB each from 10 to 20 min)
and D23's SIO/audio rings (about 2.2 MiB/min, 84 MiB cap). D53 ruled out
(duke3d 540 vs ttk 547 MB at 410 s). 20-minute run 419 -> 589 MB; projected
ceiling about 690 MB. Not reproduced: the 893 MB session (log rotated; real
display, audio and combat not covered). Options in the note: drop or shrink
the forensic rings in player builds, add a per-minute memory breakdown and
keep more logs, or accept a reasoned ceiling. Waiting for the user's choice.

Work log 2026-11-09 (fix, user: "we absoltely dont need to be recording
audio. yes lets go with your recommendation"): the SIO, SPU, audio event and
PCM history rings (about 180 MiB together) are recorded only with
`PSX_TRACE_HISTORY=1` (`--diagnostics`, `fmv_reliability.py`); counters and
audio output unchanged. A `[TTK memory] minute N` line breaks resident memory
down by owner each minute; `run.py` keeps 20 session logs. Same 20-minute
route: main RSS 589 -> 482 MB at 20 min, combined PSS 814 -> 656, minutes
10-20 +8 MB (was +27). History on/off checked through the debug commands;
Vanilla smoke, `ttk-input-test` pass; framework patch exported and verified.
Remaining: a long real session (window, audio, combat), and the unreproduced
893 MB case, which the new line will attribute.

Accepted and closed 2026-11-09. User: "in that case, you've done all the testing this time around. i'm playing
the game now, all is fine. lock this in and close it out".

### D23J - Lighter Polymorph: base cost of the 180 Hz redraws

**Status: Done (user-accepted 2026-11-11; [note 443](../documentation/443-d23j-lighter-polymorph.md)). Dependencies: D17AW, D41F1.** After
D41F1, the largest GPU cost is the base scene: about 28% of the player's GTX
1080 Ti with lighting off in the level 6 view at 180 Hz, most of it
independent of resolution. For comparison (2026-11-11, the user playing,
nvidia-smi weighted by graphics clock): EDuke32 at 1280x720, vsync off, used
about 13-14% at full clock on average in both Polymost and Polymer (Polymer
peaks to about 33% while firing); World Tour used about 5% itself, with the
desktop compositor (cinnamon 9%, Xorg 4.5%) taking more than the game. User:
the game plays smoothly (no stutters, 180 Hz held, about 45% peak), but
weaker GPUs would reach full load and shed frames where EDuke32 would not.

Where the cost lies (to confirm by measurement first):

- **The redraws.** The game runs at 30 Hz; Polymorph replays each game frame's
  draw list about six times with a late camera, resending the whole PS1 scene
  as many small batches each time.
- **The main texture shader.** Every PS1 pixel runs TEX_FS, which carries the
  lighting and shadow code even when it is off.

Plan:

1. Measure per program (`nvidia-smi pmon`) to split the game's own share from
   the desktop's (compositing a windowed game); check whether fullscreen
   bypasses the compositor. Then split the game's cost into batches,
   per-batch overhead and pixel shading (GL timers, `PSX_GPU_PHASES`).
2. Merge each game frame's batches once into far fewer, larger draws and
   replay those.
3. A lean shader variant for surfaces with no lighting or shadows.
4. Skip or cheapen redraws when the camera has not moved.

Acceptance: a clearly lower base cost in the level 6 view and slot 8 at 180
Hz (lighting off and SMOOTH), measured per program and at full clock, with
EDuke32 as the yardstick; identical pictures (image comparison); 180
presents/s held in a 60 s walk; feel unchanged or better; Vanilla unchanged.

Work log, 2026-11-11. The player's monitor was off, so the lab ran headless
on the player's GPU (SDL `offscreen`/EGL, 180 Hz, scale 3, UI slot 8,
`recomp/analysis/d23j/lab.py`, local). New developer `PSX_GPU_PHASES=2` times
each kind of GPU work inside a redraw. Findings: batching was already good
(about 5 textured draws a redraw), so step 2 had nothing to gain; per redraw
the copy-on-write save and restore cost about as much as the textured draws,
and a depth clear after the depth-stencil blits took 0.13 ms (0.014 ms after
raw copies); the lighting layer's registers cost lighting ORIGINAL about a
fifth. Changes (framework): raw `glCopyImageSubData` copies for copy-on-write,
band capture and wide centre (first copy checked, blits on a GL error);
colour-only copy-on-write on the render thread while stencil is stale (depth
is cleared before use in every session); a redraw identical to the thread's
last one (same snapshot, stream and SHADOW EDGES) reuses its image; a lean
textured program without the lighting layer for batches with no light in
effect. Each has a developer switch (`PSX_REPLAY_COPY_IMAGE`,
`PSX_REPLAY_DS_SKIP`, `PSX_REDRAW_REUSE`, `PSX_TEX_LEAN` = 0).
Results, clock-weighted GPU use, old -> new: still view ORIGINAL 16.0% ->
10-13%, ENHANCED + SMOOTH 20.8% -> 15-16%, RELIT + SMOOTH 22.1% -> 15.3%;
turning all the time ORIGINAL 19.0% -> 14%, ENHANCED + SMOOTH 25.0% -> 21%,
RELIT + SMOOTH 26.7% -> 21.9%. Images: 5016 render-thread redraws exact
against full redraws (`PSX_RT_VALIDATE`), 2112 lean redraws exact against the
full program (new `PSX_REPLAY_VALIDATE_LEAN`), copy-on-write checker clean.
60 s walk at 180 Hz: pace minute 179.8 presents/s, as before. Near, flash,
live-env and sky tests PASS; Vanilla boots and draws; patch verified on a
clean `08ec704a`. Not yet done: the real-display measurement (the player's
28% included the desktop; windowed 180 Hz is composited by Cinnamon, which
honours SDL's bypass only for fullscreen), the EDuke32 comparison on the same
display, and the feel playtest. Left for later: copy-on-write without the
wide centre (about 0.05 ms a redraw), skipping repeated small uploads (about
0.05 ms); the lit path's cost belongs to D41G and D41F2.

Closeout, 2026-11-11. User playtest on the medieval level: "the playtest
felt absolutely perfect"; GPU at most about 50% looking over the whole castle
from jetpack height (the heaviest view so far), about 40% in UI slot 2 (the
most drawn in natural play); whole-GPU figures on the windowed desktop.
Accepted and closed. The real-display, compositor and low-end measurements
move to D23K, with the goal of performance comparable to EDuke32.

### D23K - Performance parity with EDuke32: research and job plan

**Status: Done (user, 2026-11-12: "if ive closed any out, then they are accepted"; report accepted 2026-11-11, its jobs approved; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23J.** User
goal: "performance comparable or better than eduke32" - that is the real
target. After D23J the game holds 180 Hz on the player's GTX 1080 Ti with
about 40-50% whole-GPU use in the heaviest views (castle from jetpack height,
UI slot 2), against about 13-14% for EDuke32 (Polymost and Polymer, 1280x720,
vsync off, 2026-11-11). Concerns raised at the D23J verdict: the player's
figures include the desktop compositor (windowed at 180 Hz is composited by
Cinnamon); the per-frame GPU work is about three times EDuke32's; nothing has
been measured on weak hardware (Steam Deck class, laptop iGPU); each redraw
re-runs the game's drawing in a worker (about one extra CPU core at 180 Hz,
roughly three busy cores in all).

Part 1 - research (deep, with sources and measurements):

- An EDuke32 yardstick on the same machine and display: same resolution,
  refresh and mode (windowed and fullscreen), per program (`nvidia-smi pmon`,
  clock-weighted) and CPU per thread; comparable scenes (an open outdoor view,
  a dense indoor view, combat with effects). Define the metrics and the
  parity bar (GPU ms a frame, CPU cores, frame-time stability, behaviour on a
  weak GPU and CPU).
- Our full cost: GPU per kind (`PSX_GPU_PHASES=2`), the lit path (lights,
  shadows), present and compositor, CPU per thread (emulation, workers,
  render thread), memory bandwidth, the castle and UI slot 2 views.
- Low-end: how the game behaves on a much weaker GPU and CPU (a real device
  if available, otherwise simulated: clock limits, core limits, lower scale),
  and how AUTO and the D23H governor respond.
- How others do it: EDuke32 Polymost/Polymer, Nightdive's KEX ports, other
  recompilation and emulator projects (Beetle/SwanStation/DuckStation
  hardware renderers, N64Recomp/RT64), frame generation and reprojection
  designs; which of their techniques fit the redraw architecture.
- Ranked opportunities with estimated gains: for example fewer or cheaper
  worker redraws, the lit path (D41G, D41F2), copy-on-write without the wide
  centre and repeated uploads (note 443), present and fullscreen bypass,
  quality/performance presets for weak hardware.

Part 2 - stand up the jobs: a numbered research report, then one board entry
per opportunity (description, dependencies, measurable acceptance) forming a
plan that reaches parity, with an order of work. Reuse existing jobs where
they fit (D41G, D41F2) and link them.

Acceptance (for the programme the jobs form): performance comparable to or
better than EDuke32 on the same machine and display, by the metrics the
research defines, with identical pictures, feel unchanged or better, and
Vanilla unchanged. D23K itself is done when the report is accepted and the
jobs are on the board.

Work log, 2026-11-11. Measured on the player's machine and display (monitor
on), our game with the player's own profile copied read-only (Relit, SMOOTH,
scale 3, AUTO 180) and EDuke32 r10669 (the player's build, a scratch copy
with copied settings, attract demos, 180 fps cap). Lab
`recomp/analysis/d23k/lab.py` (local; CPU now summed over the worker
processes). Findings, [note 444](../documentation/444-d23k-eduke32-parity.md):
whole-GPU at 180 Hz EDuke32 3.6-3.9% fullscreen and 7-8% windowed, ours
17-26% borderless and 22-29% windowed (five to seven times the GPU per
frame); Cinnamon composites a windowed game every frame (5-6 points for us,
3.6 for EDuke32); a redraw costs 0.6-0.95 ms of which about 0.35-0.4 ms is
redrawing into emulated VRAM; RELIT adds 2-3 points and SMOOTH 0-2.5 in
natural views; CPU 2.6 cores at 180 Hz (workers 1.5), 180 Hz not held on 4
logical CPUs (governor sheds cleanly: 88 on 2, 58 on 1 core). Research with
sources (EDuke32 Polymer, RT64, Ironwail, shadow caching, clustered shading,
reprojection, compositor bypass, weak-GPU figures). Parity bar defined
(section 5); plan of ten jobs ranked (section 6): D41G, D41F2, then new
D23M (direct redraws), D23N (captured redraws), D23O, D23L, D23P, D23Q
(a lab-only startup stall seen on the real display), D23R (final benchmark).
Waiting for the user to accept the report.

### D23L - Display cost: compositor and the output pass

**Status: Done (user-accepted 2026-11-12; [note 453](../documentation/453-d23l-compositor.md), [note 444](../documentation/444-d23k-eduke32-parity.md), [note 448](../documentation/448-d23m-direct-redraws.md)). Dependencies: D23K.**
Windowed at 180 Hz, Cinnamon composites every frame: our window costs about
5-6 points of whole-GPU use more than borderless fullscreen, EDuke32's about
3.6 (note 444, section 2). Each redraw also ends with an output pass of about
0.11 ms. Work: find why our composited window costs more than EDuke32's
(window size, damage, buffer copies, swap behaviour), cut the output pass
(skip work when the picture is unchanged, cheaper scaling), and say in the
manual that borderless fullscreen (F11) avoids the compositor. Acceptance:
windowed minus borderless no more than EDuke32's difference; output pass
halved; identical pictures; Vanilla unchanged.

Work log, 2026-11-11 (Needs playtest). The "end of redraw" that note 444
took for the output pass was mostly the copy-on-write restore: with D23M it
is 0.037-0.05 ms a redraw (was 0.11-0.12), so the output pass is halved
with identical pictures (`PSX_RT_VALIDATE` exact). GAME_MANUAL (Display):
a composited desktop copies a window at every refresh; borderless or
exclusive fullscreen avoids it. Not done: the compositor half. Windowed
against borderless can only be measured on the real display, which the
standing rule keeps to sessions with the user present (D23K froze the
display driver unattended); the lab is ready (`recomp/analysis/d23k/lab.py`,
whole-GPU and per-program `nvidia-smi` sampling, EDuke32 the same way).

Work log, 2026-11-12 (Needs review). Measured on the real display with the
user present ([note 453](../documentation/453-d23l-compositor.md)): windowed
(1428 wide) against fullscreen, turning at 180 Hz, the player's profile.
Ours: UI slot 2 31.4% / 26.6% borderless / 26.8% exclusive, UI slot 8 28.7%
/ 23.7% / 24.4%, so the compositor costs 4.8-5.0 points. EDuke32 r10669 at
the same window size, in game on E1L1 and E1L2 (Polymost and Polymer, 180
fps, turning): 3.7-6.3 points (mean 4.8). In fullscreen `cinnamon` drops out
of pmon for both games (Muffin bypasses any window that asks and covers the
monitor; SDL asks for every window). The cost is the desktop's and matches
EDuke32's; D23K's 5-6 against 3.6 compared different window sizes and
demos. Nothing in our window to cut (no mapped helper windows, depth 24).
GAME_MANUAL (Display): a maximised window is still composited. Acceptance
"windowed minus borderless no more than EDuke32's difference": equal within
run-to-run noise (4.8-5.0 against a mean of 4.8; Polymost alone 3.7 and
4.7), the output pass halved by D23M. For the user to accept or ask for
more. Lab pitfall recorded in the note: an invalid `display` value
restores the defaults (a maximised window).


Playtest, 2026-11-12 (user, real display, after D23L and D23N1): "it was
smooth across the board"; "gameplay-wise it was a perfect experience". GPU
(btop): UI slot 9 the heaviest spot of the session, 50% for about a second
in one area, dropping as soon as Duke leaves it; average 25-30%; the larger
views of Family Jewels up to 40%. The fullscreen comparison was not clean:
btop was pinned above the game, so Cinnamon kept compositing; the lab
measurement of note 453 stands. Accepted; closeout authorized.
### D23M - Direct redraws: draw into their own target, not emulated VRAM

**Status: Done (first step accepted, user, 2026-11-12: "if ive closed any out, then they are accepted"; the rest is D23M2; [note 448](../documentation/448-d23m-direct-redraws.md)). Dependencies: D23K.**
A redraw is drawn into the emulated PS1 VRAM surface, so it saves and
restores what it overwrites (copy-on-write, 0.14 ms), re-uploads (0.06 ms),
clears, captures the band and runs an output pass: about 0.35-0.4 ms of the
0.6-0.95 ms a redraw (note 444, section 3). Research first: which parts of a
frame's stream (uploads, VRAM copies, fills, reads back of drawn pixels) a
redraw truly needs, then draw redraws into a dedicated target sized for the
output, with the frame's uploaded VRAM shared read-only, so nothing has to
be restored. Acceptance: GPU per redraw down by at least a third in UI slots
2 and 8 at 180 Hz (lab as note 444), identical pictures against full
redraws (`PSX_RT_VALIDATE`), copy-on-write checker clean, Vanilla unchanged.

Work log, 2026-11-11 (research; implementation not started). Baseline
per redraw (`PSX_GPU_PHASES=2`, still views, RELIT, offscreen; the desktop
browser was using about 13% of the GPU at full clock, so absolute times
are inflated, compare kinds only): UI slot 2 textured 2.0-2.4 ms, "other"
0.46-0.70 ms (14 a redraw, more than note 444's 0.08: to identify first),
copy-on-write save 0.07-0.08 + restore 0.08, uploads 0.07 (11), depth
clear 0.08-0.24, fill/copy 0.06-0.08, band 0.03, wide clears 0.01-0.04; UI
slot 8 textured 1.0-1.2 ms with the same small kinds. Code map (render
thread session, `gpu_gl_renderer.c` `gl_replay_begin`/`end`, copy-on-write
up to 48 rectangles plus one full raw-mirror blit each way, band copy into
the image cache, `rp_present_slot`): what a separate target must still
provide inside a redraw is framebuffer feedback (`flush_pack_if_sampling`
when a texture page or CLUT overlaps pixels drawn this frame), VRAM copies
and fills that read drawn pixels, blending, mask/stencil and depth. The
redraws' 11 uploads are probably repeats of the snapshot's bytes (a worker
re-runs the frame tail) but this is not measured; note 443's "skip an
upload whose bytes match" was never built. The ceiling of D23M is about
0.25-0.3 ms a redraw here. Next: measure the uploads against the snapshot,
identify "other", then the separate target with `PSX_RT_VALIDATE` exact
comparisons, on a desktop with the GPU otherwise idle.

Work log, 2026-11-11 (first step, Needs playtest). On the idle desktop,
"other" turned out to be timeline gaps (the GPU waiting for the render
thread), not work. A render thread's snapshot stays unchanged while
referenced, so a session now restores what it overwrote from the snapshot's
own textures (no save copies, no raw-mirror save), holds the snapshot
reference until the session ends, and defers that restore to its next load
(dropped when a new snapshot overwrites the surfaces anyway); the 1 MB CPU
VRAM copy runs only with a surface load. Per redraw on the GPU timeline,
turning at 180 Hz: UI slot 2 1.008 -> 0.827 ms, UI slot 8 0.990 -> 0.806 ms
(18-19%); the "end of redraw" (D23K's output pass) 0.11 -> 0.04 ms; whole
GPU 1-1.5 points lower. `PSX_RT_VALIDATE` exact (457 of 457),
`PSX_REPLAY_COW_CHECK` clean, no publishes refused for held snapshots. Not
met: a third (a redraw's own target and upload skipping remain). Note 448.


### D23M2 - Direct redraws: their own target and no repeated uploads

**Status: Done (user-accepted 2026-11-12 after playing UI slots 9 and 1, closeout authorized; the third-less acceptance was not met and the remainder is D23M3 and D23U; [note 457](../documentation/457-d23m2-shared-snapshot-redraws.md)). Dependencies: D23M.** The rest of D23M's work (its
acceptance was a third less GPU a redraw; the first step gave 18-19%). Draw
each redraw into a target of its own sized for the output, with the frame's
uploaded VRAM shared read-only, so nothing has to be restored; skip a
redraw's uploads whose bytes repeat the snapshot's (note 443's idea, never
built; measure first how many of a redraw's uploads repeat). What a separate
target must still provide inside a redraw: framebuffer feedback when a
texture page or CLUT overlaps pixels drawn this frame, VRAM copies and fills
that read drawn pixels, blending, mask/stencil and depth (D23M work log).
Acceptance: GPU a redraw down by at least a third against D23K's baseline in
UI slots 2 and 8 at 180 Hz (lab as note 444), identical pictures
(`PSX_RT_VALIDATE`), Vanilla unchanged.


Work log, 2026-11-12 (Needs playtest). Measured first, as asked: every
render-thread redraw makes 7 uploads (452 pixels) and all 7 repeat the
snapshot's bytes over pixels it has not drawn. New timers also showed the
cost the redraw timeline never counted: the emulation thread copied every
surface into a snapshot 30 times a second (0.20-0.34 ms) and render thread 0
copied them all again into its own (0.26-0.44 ms, plus thread 1's loads):
about 16-27 ms of GPU a second, 0.1-0.18 ms per redraw, while a redraw
reads well under 2 MB of the 60 MB. Built (framework, `gpu_gl_renderer.c`,
`render_thread.h`): copy-on-read, the render threads' surfaces start every
session invalid and load 32 x 32 tiles from the held snapshot on first read
or partial write (a write that covers a tile whole needs no copy; the raw
mirror is copied whole only when a session wrote it; nothing is restored any
more); an exact upload skip (rectangles known to be exactly an upload of the
current CPU bytes travel with the snapshot; an upload inside one with the
snapshot's bytes, over pixels the redraw has not written, is skipped); a
colour-only capture while the stencil is stale; a fast path so the per-
primitive check costs one comparison. Pictures: `PSX_RT_VALIDATE` exact in
UI slots 2, 8, 9 and third person except the first redraws at the level's
bake upload and one 60-pixel edge at the first redraw in UI slot 9, both of
which the matched unmodified binary shows identically. Matched builds,
turning at 180 Hz: whole GPU 23.8% -> 22.0% (UI slot 2), 20.8% -> 19.3% (UI
slot 8); redraw on the GPU timeline 0.835 -> 0.757 and 0.952 -> 0.937 ms
(another 3-run pair: 0.870 -> 0.789, 1.005 -> 0.858); copies outside redraws
about 27 / 16 -> 3.2-5 ms a second; uploads 0.04 -> 0.005 ms; CPU equal to
the baseline (render threads 41% / 15% in UI slot 2). Vanilla screenshots
byte-identical to the baseline's; `ttk-input-test` and `ttk-near-test` pass;
patch exported and verified on the pinned `08ec704a`. Not met: a third less
GPU a redraw against D23K (1.008 / 0.990 ms): now 22-25% less in UI slot 2
and 5-13% in UI slot 8. What is left is the scene itself (textured batches
0.58-0.78 ms) at 2.8-2.9 passing samples per output pixel, now D23U, and the
VRAM-shaped band work (about 0.07 ms), now D23M3. Playtest: 180 Hz in UI slots
9 and 1, first and third person: no new flicker, stale patches, missing HUD
rows or lighting pops. [Note 457](../documentation/457-d23m2-shared-snapshot-redraws.md).

Accepted 2026-11-12. User after playing UI slots 9 and 1 at 180 Hz: "performance/gameplay-wise for slots 9 and 1 its perfect ... lock it in and close it out"; the GPU still reaches 49% for a very short time in the heaviest part, "and even then causes absolutely no impediment". Closeout authorized; D23N2 closed with it.

### D23M3 - Redraws straight into an output-sized target

**Status: Done (closed 2026-11-12; build split into D23M3B, [note 460](../documentation/460-d23m3-output-sized-redraws.md)). Dependencies: D23M2.**
A render thread's redraw still draws into VRAM-shaped surfaces and then
copies its band into the cache image, copies the canonical centre into the
wide surface, mirrors every batch into the two widescreen margins and clears
depth over the VRAM draw area: about 0.07 ms a redraw at 180 Hz (band 0.03,
depth clear 0.015-0.02, mirror 0.01, wide clear 0.01, fill 0.008). Draw it
straight into an output-sized target with the snapshot's VRAM read through
copy-on-read (D23M2), falling back to the VRAM path for any redraw that
reads back what it draws (texture pages or CLUTs over drawn pixels, VRAM
copies out of the band). Acceptance: `PSX_RT_VALIDATE` exact, GPU a redraw
measured on matched builds in UI slots 2 and 8 at 180 Hz, Vanilla unchanged.

Work log, 2026-11-12 (Needs review). Measured before building. New
developer counters (`[m3 reads]`) over about 3,400 redraw sessions in UI
slots 2 and 8 at 180 Hz: no pack, VRAM copy, CPU readback or second draw
area, so a VRAM fallback would almost never be taken. The bound: dropping the
centre and band copies and the redraw depth clear outright
(`PSX_GL_DEV_SKIP=96`, wrong pictures), three alternating pairs a view with
two render threads: whole GPU 22.7 / 25.2 / 24.6% -> 23.0 / 25.2 / 25.3% (UI
slot 2) and 21.7 / 20.6 / 20.4% -> 21.2 / 20.6 / 21.5% (UI slot 8), no
difference above the noise; on the timeline about 0.05 ms a redraw at full
clock (band 0.030, depth clear 0.016-0.030, mirror 0.010). Exact design
written (note 460 section 4): the centre keeps the canonical projection with
the viewport origin moved by whole pixels (the wide projection's division
would change edge pixels), margins keep the mirror mapping, the cache image
is the target with one output-sized depth-stencil per render thread,
copy-on-read for its tiles (tile row 7 is not reached by the frame's clear),
and a VRAM-path redo on any read-back. Not built: below the noise, it reaches
every draw path of the wide pipeline, and the GPU floor found in D23U is far
larger. The build is D23M3B, after D23V.

Closed 2026-11-12 with D23T and D23U ("close it out"); the build is D23M3B.

### D23M3B - Build the output-sized redraw target

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; not exact, about 0.5 ms a redraw at best, note 491). Dependencies: D23M3, D23V.**
Build the design of note 460 section 4: a render thread draws a redraw's
centre into the cache image with the canonical projection and a viewport
shifted by whole pixels, its margins with the mirror mapping, one
output-sized depth-stencil per render thread, copy-on-read tiles for the
output from the snapshot, and a redo on the VRAM path for any redraw with a
pack, VRAM copy, CPU readback, band upload or second draw area. Expected
saving about 0.05 ms a redraw at full clock (below one point of whole GPU at
180 Hz, measured as not resolvable in D23M3); worth doing once D23V has
lowered the floor, or for a Deck-class GPU where copies cost more.
Acceptance: `PSX_RT_VALIDATE` exact in UI slots 2, 8 and 9, GPU a redraw and
whole GPU measured on matched builds at 180 Hz, Vanilla unchanged.

Work log, 2026-11-16 (field laptop, [note 486](../documentation/486-d23v3-d23m3b-d23r-measurements.md)):
measured on the weak GPU: the copies the output-sized target removes (band
copy 0.345 ms, margin mirror 0.092 ms, depth clear 0.066 ms) are about 0.5 ms
of a 5.05 ms redraw in UI slot 2 at 120, ten times the desktop estimate.


Work log, 2026-11-16 overnight (laptop lane, exactness on the desktop, [note 491](../documentation/491-d23m3b-shifted-viewport-not-exact.md)):
built as designed behind developer `PSX_RT_OUTPUT_TARGET=1` (band from the
first primitive, queued fills, viewport-shifted centre and margins into an
FBO of the cache image, `u_fcoff` for the screen-door hash, copy-on-read
tiles, a redo on the VRAM path for packs, copies, readbacks, uploads into
the band, stencil rebuilds and another band). Every session in UI 2, 8 and
9 qualified and none was redone, but the validator found about four in five
images different: 3-140 pixels each, an edge or texel one pixel over, in
the margins as well as the centre. The shifted viewport origin changes how
the GPU rounds window coordinates (INFERRED), so a target drawn at another
origin than the canonical surface cannot be exact this way. Reverted (the
diff kept in the local lab); the laptop measurement was not run. Blocked.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): not exact, about 0.5 ms a redraw at best, note 491.

### D23N - Captured redraws: replay a recorded frame instead of re-running game code

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; the captured-redraws line ends with D23N4; D23N1-N3 and N2B stay done, D23N5 stays open as its own switch). Dependencies: D23K.**
At 180 Hz three worker processes re-run the game's drawing code for every
redraw with the late camera: about 10 ms of CPU each, 1.5 cores in all (note
444). RT64 (Zelda64Recomp) records a frame's draw calls once and replays them
with interpolated transforms on the GPU, never running game code again.
Plan: research what the game's view composition decides per camera
(visibility, clipping, level of detail, sorting, billboards, the host
precise vertices) and what can be captured once per game frame in world
space and re-projected per redraw; prototype on one level; keep the workers
as the fallback. Acceptance: CPU at 180 Hz at most 1.5 cores, 180 Hz held
with 4 logical CPUs, identical or provably equivalent pictures against the
worker path, feel unchanged, Vanilla unchanged.

Evidence, 2026-11-11 (from D41M, note 446): walking down the level 3
corridor of UI slot 9 offscreen, the three redraw workers ran at 94-100%
of a core at 180 and 240 Hz in every lighting mode, ORIGINAL included, and
redraw-only switched off for "workers behind" in every configuration (9-16
times a launch with the unmodified build). Worker jobs average about 17 ms
there (D50A). This, not the lamps, is the corridor's stutter: D23N is the
job that addresses it.

Work log, 2026-11-11 (research and design, Needs review). Profiled the
three redraw workers in the UI slot 9 corridor at 180 Hz: 40-41% emulation
around the game's code (guest memory with write tracking, timing, dispatch,
GTE, recording), 28-30% our own unified near-clip renderer (it already draws
every world, prop and actor polygon host-side), 10-11% libc/libm, about 1%
game overlays. Design (note 449): N1 capture the frame's scene (world-space
polygons with their fixed inputs, plus the rest of the stream) in its first
job and re-project it for the later jobs through the same clipping code; N2
the camera-dependent remainder (sky, billboards, particles); N3 projection
on the render thread, removing the workers from redraws. Not built.


Work log, 2026-11-12 (review before building; not built). User asked for
D23N to be built autonomously. A new worker profile (UI slot 2 turning at
180 Hz, offscreen; flat samples and call stacks) and a code read of
`worker_draw`, `build_params` and `near_clip.cpp` found two errors in the
design before any code was written: each job blends actor joint matrices,
object transforms and poses by its own fraction (only rooms and fixed props
are frame-constant), and our host pieces are linked into the game's own
ordering table with the game's sprites, particles, sky and first-person
layer, much of which depends on the camera or the fraction. A job is about
25% host rooms, 12% host objects and 58% game code with its emulation; the
emulation items that look like overhead are already cached (a few percent
at most). Building N1 as written would either change pictures or save
little, so it was not started; note 449 section 6 has the corrected plan
with a new first step, N0: an inventory of every primitive the game draws
outside `world()`/`object()` and what it depends on.

Work log, 2026-11-12 (N0 measured). A temporary worker dump (removed; the
framework patch is unchanged) listed every recorded packet's source: per job
about 26 of the game's own packets against 800-6,000 host pieces, in UI
slots 2, 8 and 9 at 180 Hz, and 89-90% of the game's packets are identical
in content across the jobs of a frame; the rest are one semi-transparent
quad and about two textured quads a job. Drawing modes, flat quads and VRAM
uploads never change. So N1 can skip the game's code if the few
camera-dependent packets get host equivalents; note 449 section 7. Next:
identify those packets' routines, then build N1.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): the captured-redraws line ends with D23N4; D23N1-N3 and N2B stay done, D23N5 stays open as its own switch.

### D23N1 - Same-job capture and rebuild

**Status: Done (user-accepted 2026-11-12, [note 454](../documentation/454-d23n1-capture-and-rebuild.md)). Dependencies: D23N.** First step of N1
([note 449](../documentation/449-d23n-captured-redraws-research.md) sections
6-7). In a redraw job, record every input our host renderer receives while
the game's code runs (each `world()`/`object()` and host-visibility call
with its GTE state, render-context fields and guest tables, the other host
emitters, the lights/shadows/lamp records) and every packet the game emits
itself, with its place among ours in the ordering table. Then, in the same
job, rebuild the stream from the record alone (no game code) and compare.
Acceptance: developer switch; rebuilt streams identical word for word to
the real ones in UI slots 2, 8 and 9 at 180 Hz over thousands of jobs;
nothing changes when the switch is off; Vanilla unchanged.

Work log, 2026-11-12 (Needs review). Built `recomp/src/ttk/redraw_check.inc`
(developer switch `DNTTK_N1_CHECK=N`): a checked job forks a copy of its
worker at its start, logs every top-level store of the game's code and
every top-level hook entry (CPU state, stream positions; a new runtime
observer in `psx_mod_function_entry`), and the copy rebuilds the job from
that record alone (stores applied, hooks called, the game's own stream items
spliced, chain close and submit run for real), then the streams are compared
word for word. About 12,000 jobs in UI slots 2, 5, 8 and 9, first and third
person, walking, turning and firing, at 180 Hz: all identical, except one
job a worker after the save loads in third person, where a game packet's
PGXP record (excluded: the rebuild runs no game GTE work) starts the
stream's source annotations; identical once those are set aside. Defects
found and fixed in the system: worker hook depth (workers forked inside a
callback), wall-clock reads while drawing (a held draw clock), the lamp bake
table changing mid-job (held per job), splice annotations, I/O stores in the
rebuild. A rebuild costs 55-62% of the composition (upper bound on a rebuilt
later job before re-projection). Check off: `PSX_RT_VALIDATE` 693 of 693
exact in two runs, 180 Hz held, Vanilla boots; `ttk-input-test` and
`ttk-near-test` pass. Framework changes exported; the patch applies to the
pin. [Note 454](../documentation/454-d23n1-capture-and-rebuild.md).


Accepted 2026-11-12 with D23L after the playtest above ("all these tests
happily passed"); closeout authorized.
### D23N2 - Rebuild for another camera and fraction

**Status: Done (pieces 1-3, accepted with D23M2 2026-11-12, closeout authorized; rest in D23N2B; [note 456](../documentation/456-d23n2-transform-camera-visibility.md), inventory [note 455](../documentation/455-d23n2-what-a-later-job-changes.md)). Dependencies: D23N1.** Rebuild a later job of the same game
frame from the first job's record, with that job's camera and blended
poses: rooms first, then fixed props, then actors (model-space corners and
the joint or transform each hangs from, the GTE matrix recomputed exactly
as the game does). Original acceptance: host packets identical word for
word to the later job's real stream; differences in the game's own packets
listed (they are D23N3's). Rescoped 2026-11-12 (see the work log): this job
is the transform, camera and visibility pieces with their exact checks; the
rest of the rebuild is D23N2B.


Work log, 2026-11-12 (inventory). Added a record diff to the D23N1 tool
(`DNTTK_N2_DIFF=1`): every gameplay job is recorded and compared with the
previous job of the same frame. Turning at 180 Hz (UI slots 2, 8, 9): 22-30%
of pairs entered a different hook sequence (other rooms and objects passed
the game's visibility tests), 62-83% of hook entries saw other CPU state
(GTE rotation and translation, derived registers), 9-18% of the game's
stores differ in value (half in the scratchpad, then the stack, arena and
matrix pages). Still (UI slot 2): the same hook sequence in every pair, 8%
of hook entries differ (actor poses), 0.9% of stores. The camera drives the
game's whole view pipeline, so a later job needs host equivalents of the
transform (`0x800292a0`), room walk, culling, water, small models, sky and
HUD hooks, each verified exact against the game's result; build order in
[note 455](../documentation/455-d23n2-what-a-later-job-changes.md). Next:
the object transform on the runtime's GTE emulator with its verifier.


Work log, 2026-11-12 (pieces 1-3, Needs review). Built in
`recomp/src/ttk/redraw_n2.inc` (developer `DNTTK_N2_CHECK=mask`, nothing runs
without it). First the record was extended (store PCs, hook entry and exit
times, `DNTTK_N2_TRACE` pair dumps) to measure where a later job's time goes:
UI slot 2 turning, 55.5% our hook work (room walk 25%, objects 17%), 44.5% the
game's own code; only 5.9% of a job is game code that does the same in two
jobs of a frame, 40.9% is camera- or fraction-dependent, so a later job must
recompute nearly all of it (in host code to save anything). Piece 1: a host
port of the transform `0x800292a0` (halving, ApplyMatrixLV, transpose,
MulMatrix0), checked at every call against the routine's own stores and the
GTE state of the next hook: about 1.13 million calls in UI slots 2, 8, 9 and
third person, all exact (71,267 GTE states, about 25,000 halved
translations). Piece 1c: every later-job transform's camera predicted at the
job start (the game's `0x80039c7c` on a scratch CPU, plus our body camera):
none missed. Piece 1b/1d: substituted object transforms predicted by
(object, call index), all but 60 of 85,742 in UI slot 9 (cause not
established); actor joint table entries unchanged since the job start in
about 46,000 calls (direct in third person, scaled and presented by our own
code in first person); matrices built during the job (first-person Duke and
weapon, a few game builders) remain. Pieces 2-3: the game's portal walk
`0x8006276c` run at a later job's start on a scratch CPU with its writes
undone gives exactly the room list the job later computes, and every input
of our host room walk is predicted: 12,190 of 12,190 later jobs. The D23N1
same-job check is still exact with the extended record (992 jobs). Found on
the way: the object renderer projects every vertex of a mesh the host
emptied (about 6-8% of a job), now D23T. Rescoped: the remaining matrices,
object culling, camera-dependent hooks, the game's sprite and particle code
and the whole-job rebuild are D23N2B. No framework change. [Note 456](../documentation/456-d23n2-transform-camera-visibility.md).

### D23N2B - The rest of a later job's rebuild

**Status: Done (2026-11-15, accepted by the user; desktop, [note 481](../documentation/481-d23n2b-d23n3-d23n4-desktop.md): the weapon attachments exact; every compared later job whole-exact in UI 2, 8, 9 and 5; 2026-11-15 night, field laptop: whole later jobs exact with their game packets in 93-99% of later jobs, the muzzle flash run alone, [note 480](../documentation/480-d23n2b-d23n3-field-laptop.md); 2026-11-15, pieces 9-13: small models, water, room transforms, sky and the portal walk exact in a rebuilt job, host packets exact in 93-100% of later jobs, [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 2; pieces 6-7 exact and piece 8 (a whole later job) built, [note 477](../documentation/477-d23n2b-whole-later-job.md); piece 6 measured, [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 3; piece 5 exact, [note 474](../documentation/474-d23y9-d23n2b-field-laptop.md) section 6; pieces 4c-4d [note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md) section 3; piece 4b [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 6; piece 4a [note 469](../documentation/469-d23y7-d23y5-d23n2b-field-laptop.md) section 5; split from D23N2 2026-11-12, [note 456](../documentation/456-d23n2-transform-camera-visibility.md) section 6). Dependencies: D23N2.**
With the transform, camera and room visibility of a later job known at its
start (D23N2), the rest of its inputs: matrices the game and our
presentation code build during the job (first-person Duke and weapon
`0x800388e4`, `0x80098204`, `0x8002b068`/`0x8002b188`/`0x800b3e5c`), object
culling (the props and actors pass `0x8002ffec`, the object pass
`0x800374cc`, per-object tests), the camera-dependent hooks (water and fills
`0x8002bc18`/`0x800b4d2c`, small models `0x8002d848`/`0x8002bd48`, the room
lift `0x800a2690`, the sky, the HUD hooks `0x800b4b28`-`0x800b4b50`), and the
game's sprite and particle code after them (`0x8002fa48`, `0x8002f450`,
`0x8002f900`, `0x800b51cc`, `0x80062258`), each as host code with an exact
check like D23N2's. Then a whole later job rebuilt from the first job's
record and these pieces, compared word for word with its real stream (the
D23N1 comparison). Also explain the 60 unmatched substitutions of UI slot 9.
Acceptance: host packets of rebuilt later jobs identical word for word in UI
slots 2, 8 and 9 at 180 Hz over thousands of jobs, the game's own packet
differences listed for D23N3, the rebuild's CPU time measured against the
job's; Vanilla unchanged.

Work log, 2026-11-14 (In progress, piece 4a), [note 469](../documentation/469-d23y7-d23y5-d23n2b-field-laptop.md) section 5.
Autonomous session on the field laptop (third of D23Y7, D23Y5, D23N2B). The
matrices built in a later job, by caller (`DNTTK_N2_CHECK=2`, offscreen 180
Hz): Duke's joint loop (transform call returning to `0x80034c6c`) is most of
them (UI slot 2 20,247 of 25,080; UI slot 8 12,636 of 18,829; UI slot 9
26,238 of 28,063). New developer check, bit 8 (`redraw_n2.inc`, namespace
`jt`): at that transform entry before and after our callbacks. In UI slots
2, 8 and 9 every joint passes its table entry directly, every entry is as at
the job start (36,992, 29,952, 48,384 calls), and our own callbacks replace
every matrix the routine reads (in mod memory 32,640 / 24,960 / 37,383; the
camera block for joints kept out of the view 4,352 / 4,992 / 11,001). So a
rebuilt later job gets them by running our D36 joint code from the job-start
inputs. A host port of `ScaleMatrix` `0x800b4b5c` (from its disassembly) is in
the check; the game's scaled path is not taken in these views. Next: the
inputs our joint callbacks read, checked as at the job start or predicted;
then the callbacks run from the later job's start and compared word for
word; then the other callers (`0x80033584`, `0x8003133c`, the weapon/HUD
`0x80038c68`/`0x80038d7c`/`0x80038e18`), and the rest of note 456 section 6.

Work log, 2026-11-14 (In progress, piece 4b), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 6. New developer
check, bit 16 of `DNTTK_N2_CHECK` (`redraw_n2.inc`, namespace `jr`): a copy
of the worker forked at every 4th job's start runs the job's top-level hooks
with their recorded CPU state on the job-start memory (none of the game's
stores), with the eye piece 2 predicts, and compares every matrix our D36
joint callbacks pass for Duke's joints word for word. Offscreen at 180: UI
slots 2, 8, 9 still 4,320 / 5,472 / 3,744 exact, walking, turning and firing
7,072 / 5,760 / 9,216 exact, none differ (without the predicted eye about a
quarter differed by 1-2 units of translation). Next: the same from the
earlier job's record of the frame (as D23N4 will run), the other transform
callers (`0x80033584`, `0x8003133c`, `0x80038c68` / `0x80038d7c` /
`0x80038e18`), then note 456 section 6 (object culling, camera-dependent
hooks, sprites and particles, a whole later job).

Work log 2026-11-14, afternoon ([note 471](../documentation/471-d41p2-d41s-d23n2b-field-laptop.md)
section 3; user: "resume after piece 4b ... Go as far as exact matches
allow, then leave a clear handoff"). `DNTTK_N2_CHECK` bits 32 and 64
(`redraw_n2.inc`; `redraw_check.inc` keeps the previous job's record for
them). Piece 4c (bit 32): the hooks run with the earlier job's registers
(same frame, another fraction) when its hook list matches call for call:
every joint matrix exact, UI slots 2, 8, 9 walking 3,315 / 972 / 4,284 of
as many; 27-62 jobs a run skipped because their hook list differs from the
earlier job's. Piece 4d (bit 64): every transform call by caller, from the
job start: `0x80031dec` 98% and `0x80032f48` 93% exact, `0x80033584` 82%,
`0x8003133c` and the sky's `0x80038c68` / `0x80038d7c` / `0x80038e18` never
(one each a job, built from the camera), a few rare callers. Next
(resumable): host builds of the camera-built matrices (sky, `0x8003133c`),
then what differs in `0x80033584` and the others (objects, matrix words),
the hook-list differences of piece 4c, then note 456 section 6.

Work log, 2026-11-14 (piece 5, exact), [note 474](../documentation/474-d23y9-d23n2b-field-laptop.md) section 6. Disassembled the
four calls built in the job: the sky routine `0x800388e4` builds three
matrices from the camera block (the backdrop from the look direction through
VectorNormal, ratan2 and an acos table; two cloud layers from the timer
`0x800c0c64`, one scaled) through `0x8002afe4` (rsin/rcos products, then
MatrixNormal with GTE OP and the VectorNormal core); `0x8003133c` is not from
the camera (note 471 inferred that): `0x80030fe8` builds an object's yaw
matrix from its record and a height argument. Host ports in
`redraw_n2.inc` (namespace `sk`, developer `DNTTK_N2_CHECK=128`), checked at
each transform entry before our callbacks: offscreen at 180, all four exact,
0 differ (UI 2 2,240 each, UI 8 1,664, UI 9 2,624 of `0x8003133c`, DUKE
HILL's town 64). One decoding slip (the core's GPF is sf 0) was found by the
check itself. Next (resumable): what differs in `0x80033584` (82% exact from
the start) and the 2-7% of `0x80031dec` / `0x80032f48`, the hook-list
differences of piece 4c, then note 456 section 6. D23N3 and D23N4 not
reached this session.

Work log, 2026-11-15 (piece 6, measured), [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 3. `0x80033584` is
a second actor joint loop (in `0x80033160`): it passes the joint's entry of
the actor's matrix table (`*(s3 +0x3c) + s2`) or a ScaleMatrix stack copy.
New counts in `DNTTK_N2_CHECK` bit 64: in UI 8 every one of 12,598 calls
passes an entry the job itself rewrote before the call (UI 2 1,479 of
1,479; no calls in UI 9's walk), 79% are still equal to the job start; the
differing ones were last written at `0x8009876c` / `0x8009837c` inside
`0x80098204`, which rebuilds the actor's joint table every job (rotation
from the animation through `0x80010d60`, translation = joint offset + the
actor's position `+4/+8/+0xc`); they differ by 1 unit in a translation
word (INFERRED: the per-fraction actor position). Next (resumable): run
`0x80098204` after a later job's pose patches in the forked copy and check
the entries word for word; then `0x80031dec` / `0x80032f48`'s 2-7%, piece
4c's hook-list differences, note 456 section 6. D23N3 and D23N4 not reached.

Work log, 2026-11-15 (pieces 6-8, desktop), [note 477](../documentation/477-d23n2b-whole-later-job.md). Autonomous session
(user: "D23N2B piece 6 ... then the rest of D23N2B. 2. D23N3, then D23N4 as
far as exact matches allow"). New desktop lab `recomp/analysis/d23n2b/`
(local; card copies, `RATE=180` in the lab's profile copy, settings line
checked, offscreen). Piece 6 (`DNTTK_N2_CHECK` bit 256): the actor draw
`0x80032e78` builds the actor's joint table at `0x80032ff8` (`0x80098204`,
or `0x800987cc` by the type and flags) just before its `0x80032bd8` hook;
the copy runs the same build there after the job's pose patches: every
`0x80033584` call exact (UI 8 13,058, UI 2 7,956; before 64% and 99.5%).
Piece 7 (bit 512): the matrices built on the caller's stack, in host code:
actors `0x80032f48` and props `0x80031dec` (`0x8002afe4` of `+0x1c`, the
position, the props' ScaleMatrix by `+0x22`), `0x8003133c` (its height is
Duke's `+0x204`), the sky (its timer advanced by `*0x800d21fc` once a job):
every transform call from the job start exact in UI 8, 2 and 9 except four
rare callers (about 0.1%, left to a fallback). Piece 8 (bits 1024 and
2048, `DNTTK_N2_WHOLE`): a later job rebuilt from its start, hook by hook
and then the game's packets after the flip, word for word; sanity with the
job's own record 66 of 66 exact. With the earlier job's stores and
registers (D23N4's setting): UI 9 121-122 later jobs host-exact (85-97%),
113 with the game's packets (79-90%); UI 8, 2 and 5: none. The roots (each
job's first differing hook): small models (`0x8002d848` and the face
emitters project in game code; 14-71 jobs a view), water (`0x8002ef90`'s
grid; 4-25), the room walk (one stretch late in its stream; 3-12), the sky
backdrop object (840 words against 600; 1-15), rarely Duke's draw and the
object pass. Rebuild 2.1-5.1 ms against 7.2-16.1 ms a job (with recording).
Check off: interleaved A/B against the pre-session build, same game rate,
presents and validator results; Vanilla boots. Next (resumable): host ports
of the small-model projection and the water grid, then the room walk and
backdrop differences, then the rare callers; D23N3's packets (note 477
section 4).

Work log, 2026-11-15 (In progress), [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 2. Autonomous
session on the desktop (user: "host ports of the small-model projection
(0x8002d848 and its face emitters) and the water grid (0x8002ef90), each
checked exact with the piece 8 whole-job check, then the room walk and sky
backdrop differences, as far as exact matches allow"). Developer code only
(`DNTTK_N2_CHECK` bits 4096-32768, `recomp/src/ttk/redraw_n2.inc`).
Piece 9 (4096): the small-model draw in host code (GTE transform
`0x8002df20`, vertex records `0x8002dce0`, the face walk `0x8002d848` with
its culls and keys, each emitter's corner words, every `0x8002bd48` link).
Piece 10 (8192): the water grid (room transform `0x80029660`, records and
flags hook by hook, each tile's corners, depth-cued colour and key, every
`0x8002bc18` link). Piece 11 (16384): every room transform `0x80029660`
redone for the job's eye (the sky backdrop's about the eye: TR 0). The
clouds' timer no longer advances twice when stores are replayed. Piece 13
(32768): the room walk's difference was the portal walk's output; the copy
runs the game's portal walk for the job's camera (game code, not yet a
host port). 70 s a view, bits off against on: later jobs host-exact UI 2 0
of 75 -> 84 of 90, UI 8 0 of 131 -> 147 of 148, UI 9 175 of 177 -> 169 of
169, UI 5 0 of 157 -> 145 of 147; every model face hook (8,182) and water
tile (9,290) before a job's first difference exact. Left: Duke's draw from
the rare transform caller `0x8003437c` (a random-yaw attachment, 1-6 jobs a
view), a host port of the portal walk, and D23N3's game packets (whole jobs
exact UI 9 166 of 169, the other views 2-16). Check off: validator and game
rate unchanged; Vanilla shots identical.

Work log, 2026-11-15 night (In progress), field laptop, [note 480](../documentation/480-d23n2b-d23n3-field-laptop.md). Autonomous
(user: "finish the rebuild of a later job ... port the rare transform caller
at ra 0x8003437c ... then a host port of the portal walk"). Developer code
only (`redraw_n2.inc`; `DNTTK_N2_CHECK=2096904`, `DNTTK_N2_WHOLE="1 1"`).
Baseline here (note 478's setting): host-exact 94-100% as on the desktop,
but whole jobs only 2-6 a view and 3-9 runaway rebuilds. Correction to note
478: the earlier job's record is replaced by the job's own at its flip, so
note 478's "earlier" GTE at ra `0x8003437c` was the job itself; the check
now keeps the earlier record at the job's start. The flash: the copy runs
`0x800341e4` from its hook (bit 524288); still not exact (the mesh's depth
differs: our D36 weapon matrix follows the camera and the game's code
copies it onto its stack; the weapon mesh's scaled copy at ra `0x80033f44`
likewise), 1-5% of later jobs. The portal walk: not ported; running the
game's costs 0.43-0.76 ms of a 4.7-9.0 ms rebuild. With D23N3's pieces:
whole later jobs exact, game packets included, UI 2 299 of 320, UI 8 223 of
227, UI 9 431 of 437, UI 5 249-284 of 258-288, 60 s each at 180; rebuild
4.7-9.0 ms against 15.3-21.6 ms a job (both with the check's recording).
Check off: validator 857 of 857 exact, Vanilla shots identical.

Work log, 2026-11-15 late (In progress), desktop, [note 481](../documentation/481-d23n2b-d23n3-d23n4-desktop.md) section 3. The
flash's run in the copy never reached our D36 weapon-matrix hook: it was
dispatched with the check's sentinel as its return address, and
`first_person_weapon_attached` acts only for Duke's draw's call (ra
`0x80035348`), so the copy's flash used Duke's real hand joint. The copy
now dispatches with the call's own return address (as the portal walk
did): every flash exact. The scaled weapon mesh (ra `0x80033f44`):
`0x80033e40` copies our matrix to its stack and scales it when the slot has
a scale; the copy replayed the earlier fraction's copy. It now runs alone
(bit 2097152). The attach point `0x80032364` is redone before the hooked
`0x80032bd8` (bit 16777216; see D23N3). Result, `DNTTK_N2_CHECK=33554184`,
60 s a view at 180: every compared later job whole-exact, UI 2 78/78, UI 8
98/98, UI 9 150/150, UI 5 128/128 (hook lists differ in 6, 5, 1, 2,
skipped). A host port of the portal walk was not built: 0.20-0.35 ms of a
6-51 ms rebuild here, worth it only once D23N4 runs rebuilds. Check off:
validator 906 of 907 (the pre-session binary 914 of 915, both a level-load
frame), Vanilla shots identical. User playtests (weapon, flash and lines in
UI 8; Duke's sprites and particles in UI 9 and 5; Vanilla): "all 3 play
beautifully fully accepted". Done; the portal-walk port, if ever wanted,
belongs to D23N4.

### D23N3 - The game's camera-dependent packets

**Status: Done (2026-11-16, closeout authorized; [note 482](../documentation/482-d23n3-d23n4a-d23n4b-desktop.md): the UI 5 residue fixed, the remaining arena class detected by D23N4a and left to fall back. Earlier: 2026-11-15 late, desktop, [note 481](../documentation/481-d23n2b-d23n3-d23n4-desktop.md) section 4: the line effect exact, Duke's joint sprites and the particles run in the copy; 4 of 265 UI 5 jobs left in a 120 s run; 2026-11-15 night, [note 480](../documentation/480-d23n2b-d23n3-field-laptop.md) section 3: every host-exact rebuilt later job identical with its game packets in UI 2, 8, 9 and 5 but one unexplained worker; the line effect's port not yet seen running; 2026-11-15: listed, [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 2). Dependencies: D23N2B.** About 1 in 10 of the game's own
packets change between the jobs of a frame (one semi-transparent quad and
about two textured quads a job, note 449 section 7). Identify their guest
routines and give each a host equivalent (or run that routine alone).
Acceptance: rebuilt later jobs identical word for word, game packets
included.

Work log, 2026-11-15: not reached (the user's order was D23N2B, then
D23N3, then D23N4 "as far as exact matches allow"; D23N2B's whole-job
rebuild is still open), [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 3.

Work log, 2026-11-15 (listed, not built), [note 477](../documentation/477-d23n2b-whole-later-job.md) section 4. D23N2B's whole-job
check now compares the game's packets after the flip too. In host-exact
rebuilt later jobs of UI 9, 8 of 121 differ there: a semi-transparent raw
textured quad (GP0 `0x2e`) whose corners move 2-6 units between fractions,
and gouraud textured triangles and quads (`0x34`, `0x3c`) the later job
draws and the earlier did not. Their guest routines are not identified yet
(the record's store PCs for those packets would name them). Waits on
D23N2B's roots in the other views.

Work log, 2026-11-15 night (In progress), field laptop, [note 480](../documentation/480-d23n2b-d23n3-field-laptop.md) section 3.
Found with a new dump (`DNTTK_N2_DUMP`) and an offline alignment of the
packets by their store PCs. (1) The frame's bucket link-up `0x8002bb80`
(via `0x8001fba0` and `0x8001fc44`): the copy replayed the earlier job's,
which links the earlier job's buckets; now host code on the copy's buckets
(bit 65536): game-packet differences 75,000 -> 530 items in UI 9, 177,000
-> 1,600 in UI 2, no runaways. (2) The semi-transparent quad `0x2e`: Duke's
shadow `0x80030fe8` (bounds of Duke's joints, which follow the camera in
first person); a projection-only port was not enough; the routine now runs
alone in the copy (bit 131072). (3) The FT4 sprites: the sprite queue
`0x8009b1c0` via `0x8009b750` (no entry hook; its wrapper's store of ra
marks the call), run alone (bit 262144). (4) Mono lines `0x40` (UI 8, 2
jobs): `0x8008ffc4`/`0x8008f71c`, run alone from its transform hook (bit
1048576; not yet seen in a run). Result: in every host-exact job of UI 2,
8, 9 and 5 the game's packets identical word for word (60 s each, 180 Hz),
but six jobs of one worker in one UI 5 run (host arena packets where the
rebuild had the game's; not in the repeat). The earlier `0x30`/`0x34`/`0x3c`
"effect triangles" (note 477) were cascades of (1).

Work log, 2026-11-15 late (In progress), desktop, [note 481](../documentation/481-d23n2b-d23n3-d23n4-desktop.md) section 4. The
line effect (orange lines from `0x8006f7a4` through the line list
`0x800c4580`) drew in UI 8 and was not exact: the copy's skip range ran
into the camera render's next calls (the sprite queue, small models) and
dropped the sprite queue's marker store. Both fixed: UI 8, 120 s, 5 line
jobs, 199 of 199 later jobs whole-exact. UI 9's semi-transparent sprite
quads: Duke's draw queues one sprite per joint (`0x800307c0`, objects with
flag 0x800) at joint positions that follow the camera; the loop now runs in
the copy (bit 4194304). UI 5's rare ones: the particle walk `0x8009ce44`
(bit 8388608) queues particles at Duke's attach point, which `0x80032364`
writes during the job; redone in the copy (bit 16777216). In 60 s runs
every compared later job identical with its game packets in UI 2, 8, 9 and
5; in a 120 s UI 5 run 4 of 265 jobs still differ: gouraud quads `0x38`
from the game's own object-renderer path (store PCs `0x80010680-
0x8001068c`) and a flat quad written inside a hook (cause not
established). Note 480's worker incident did not recur in five UI 5 runs.

Work log, 2026-11-16 (desktop, [note 482](../documentation/482-d23n3-d23n4a-d23n4b-desktop.md)): the UI 5 residue was the
object renderer's own body (`0x80010000`, quads at PCs `0x80010680-0x8001068c`),
replayed from the earlier fraction; the copy now runs it (bit 33554432),
UI 5 game-packet differences 6/272 -> 0/256 on the same run. Wider check, 120
s: UI 2 176/176, UI 8 212/212, UI 9 322/322, UI 5 (DUKE HILL town) 291/293,
FAMILY JEWELS start (made in lab slot 11 with the console) 166/224. The rest
is one class: the renderer's own face culls follow the camera, so the copy's
packet arena leaves the source's and later replayed packet stores no longer
fit; not rebuildable by replay, counted per job and flagged by the D23N4a
detectors (falls back). One FAMILY JEWELS host difference remains unexplained.

### D23N4 - Later jobs skip the game's code

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; cannot pay at any number of workers, 4-18% dearer than plain, note 487). Dependencies: D23N3.** Switch the rebuild on: the first job
of a frame captures (with a culling margin wide enough for the late
camera's turn within the frame, or falls back), the capture is shared
between worker processes, later jobs rebuild instead of running the game.
Acceptance: pictures identical (`PSX_RT_VALIDATE`, the comparison mode kept
as a developer check), worker CPU at 120/180/240 Hz measured against D23N's
baseline (about 1.5 cores at 180 Hz in UI slot 2), fallback to the full job
whenever the capture does not cover a job, Vanilla unchanged.

Work log, 2026-11-15: not reached, so not switched on and the CPU clock,
emulation thread and picture cadence at 120 were not compared against it.
D23Y7 and D23AA (the DUKE HILL town and the FAMILY JEWELS start) now wait on
it: in both the redraw worker rebuilding the game's geometry is the cost, [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md)
sections 4-5.

Work log, 2026-11-15 (not switched on), [note 477](../documentation/477-d23n2b-whole-later-job.md) section 5. The rebuild of a later
job from the earlier job's record is exact only in views without actors,
small models and water (UI 9: 79-90% of later jobs); elsewhere it would
change pictures, so D23N4 was not built and, as the user's condition said,
the 120 Hz CPU clock, emulation thread and cadence and the DUKE HILL town
and FAMILY JEWELS start were not re-measured. Needed first: D23N2B's small
model and water ports, its room walk and backdrop differences, D23N3.

Work log, 2026-11-15 night (not switched on), [note 480](../documentation/480-d23n2b-d23n3-field-laptop.md) section 5. With D23N3,
every rebuilt later job whose hook list is the earlier job's is exact, but
1-8% of later jobs a view (UI 8 the most) have another list: water tiles,
the portal walk's recursion, actor and prop draws, small models against
sprite faces, all camera-dependent culling. The check skips them by
comparing with the job's own list; a switched-on rebuild would replay the
earlier list and show the earlier fraction's culling. With the weapon
attachments, 2-10% of later jobs would change, so it stays off (the user's
rule), and its A/B at 120 and 180 and the D23Y7 and D23AA re-measurements
were not made. Needed: a later job's culling known before its rebuild (the
portal walk already runs for the job's camera; objects, faces and tiles
would need the same), the weapon attachments, then a fallback to the full
job for anything still uncovered.

Work log, 2026-11-15 late (groundwork; not built), desktop, [note 481](../documentation/481-d23n2b-d23n3-d23n4-desktop.md) section
5. Every place a later job's hook list leaves the earlier job's (water
tiles, prop draws, the portal walk's recursion, small models, the shadow's
link; 6, 5, 1 and 2 of 84-151 later jobs in UI 2, 8, 9, 5) is a culling test
the game takes between two hooks, on values the copy already computes for
its own fraction: the prop test in `0x80031d10` (far against context
+0x74, near against `0x800b4d2c`, the frustum test `0x80031584` on the
transform's GTE results), the small-model and water tests (already on the
host in the copy), the portal walk and the shadow (already run in the
copy). So a rebuild can follow the game's decisions and fall back to the
full job when one differs from the earlier job's. Not built: the prop and
actor tests need exact ports of `0x80031584` and of the GTE data registers
the transform leaves, and the water and small-model ports their unlinked
side, before "no job the rebuild gets wrong is accepted" can be proven; so,
by the user's rule, no D23N4 switch and no measurement.

Work log, 2026-11-16 (desktop, [note 482](../documentation/482-d23n3-d23n4a-d23n4b-desktop.md)): **D23N4a**: the frustum test
`0x80031584` reads GTE control registers only (TRX/TRY/TRZ, H; not data
registers as note 481 assumed); ported and checked against the game's own
routine at its six call sites: about 930,000 calls identical. The
transform's GTE data registers (MulMatrix0's last MVMVA: V0, IR1-3, MAC1-3,
FLAG) ported, exact at about 1.6 million next hooks that follow it directly.
Detectors (bit 134217728): every job records its culling decisions; a
rebuilt later job compares its own (far/near/frustum per site, the prop's box
tests, the occluder stage and the actor hit test `0x80076330` as unprovable,
water boxes and tiles, small models, the hooks of every routine run alone,
the packet arena, game GTE work before the object renderer). Proof, 120 s in
UI 2, 8, 9, 5 and the FAMILY JEWELS start: 371-378 wrong jobs, **0
unflagged**; false alarms still high (UI 5: the actor hit test). **D23N4b**:
built behind `DNTTK_N4_REBUILD=1`, off by default (lean first-job capture per
worker, in-place rebuild, image reload and the full job on any flag; verify
switches `DNTTK_N4_VERIFY`/`DNTTK_N4_VERIFY_FIRST`). It only applies with one
worker (at 180 with three workers a frame's jobs go to different workers).
Not exact: 2-6% of accepted rebuilds differ from the full job (mostly the
game packets after the flip); not cheaper: UI 5 worker CPU 94.5% of a core
against 91.2% plain. Left off; pictures with every switch off: validator 908/908,
Vanilla shots 6/6 identical. Field laptop (same session): no saving beyond run order (the check's recording
alone measures the same as N4b in the town; FAMILY JEWELS start: one later job
rebuilt in 60 s), town 191/192 accepted rebuilds exact; default-on: no. D23Y8
(part 3) not started (note 482 sections 8-9).

Work log, 2026-11-16 (desktop, [note 483](../documentation/483-d23n4b-exactness-desktop.md); part 2 not run, [note 484](../documentation/484-d23n4b-field-laptop-not-run.md)):
the arena cursor logged at every hook (`DNTTK_N4_CURSOR=1`) found note 482's
one-packet difference: the job built one more water tile than its source.
The detector had judged tiles on the flag words as stored; our water near
path zeroes a near vertex's word before the game's tile loop reads it. The
copy now walks the game's tile loop on its own records, one tile at a time,
and the source's tile links must be exactly those. The actor hit test
`0x80076330` (one site, the segment query `0x80077560`) is bounded: the
game's routine run on a scratch CPU in the source and in the copy, its
result and writes hashed and compared (UI 5: rebuilt later jobs 117 -> 777
of 1,113). Duke's joint loop (`0x80034c6c`/`0x80034c8c`) got its own
detector, with our D35E depth raise modelled. The worker was being killed at
its 250 ms deadline in UI 5 and the FAMILY JEWELS start: the joint table
build ran on a stack past the mod-memory watermark after an image load and
halted; it now runs on the job's own stack (0 kills). Lean mask 201326344
(only the ports' self-check dropped). Proof at 120 s: verify-first UI 2
1,070/1,072, UI 8 1,088/1,088, UI 9 1,792/1,792, UI 5 3,122/3,248 (a full
job there is not repeatable: the object pass's two host-dispatched
renderers keep host state; with both off 1,411/1,424 and a full job 880/880
repeatable), FAMILY JEWELS start 688/688; fork proof: wrong jobs unflagged
UI 2 2, UI 5 1, the rest 0. CPU (one worker, interleaved, two each): N4b
6-16% more than plain (UI 2 113.5 -> 122.0%, UI 5 111.3 -> 119.3%, FAMILY
JEWELS start 120.4 -> 138.7%). Not exact, not cheaper: off; part 2 not run.

Work log, 2026-11-16 overnight (desktop, [note 487](../documentation/487-d23n4-cost-decision.md)):
new developer timers `DNTTK_N4_COST=1` split a worker's time by job class
and part. The capture is small (0.4-0.9 ms a recorded job: observer 0.17-0.32
ms, of it decisions 0.13-0.23 and scratch runs 0.05-0.16, flip copies
0.11-0.19, the store log the rest); what makes N4b dear is that a rebuild
still runs every host hook (UI 2: 8.1 ms against a 10.2 ms plain job) and
that 25-59% of later jobs fall back, each paying the attempt, a 0.5 ms
reload and a full job. Exact cut: a flagged in-place rebuild stops at its
first flag (flags are only added; developer `DNTTK_N4_EARLY=0` runs it to
the end): failed attempts 7.2 -> 3.9 ms (UI 2), 5.2 -> 2.8 (UI 5), 6.1 ->
3.3 (FAMILY JEWELS start). After it, one worker, interleaved, two runs each:
mean worker job +15-16% (UI 2), +4-5% (UI 5), +7-18% (FAMILY JEWELS start)
against plain; whole-game CPU 112.4 -> 119.2%, 110.9 -> 114.6%, 124.9 ->
131.2%. A perfect N4b (no fallback cost, no image save) would be -6% to +9%.
Decision: N4b cannot pay at any number of workers (more workers record more
first jobs); Blocked, off by default. The open detector classes and a shared
capture (this session's D2) were not started: the gap is not under 5%.

Work log, 2026-11-16 overnight, later (desktop, [note 489](../documentation/489-d23n4-repeatable-jobs.md)):
note 483's non-repeatable UI 5 job was the checks' own clock. A new
developer tool (`DNTTK_N4_STATEDIFF`, every static object compared between
a job's start and its second run) pointed at the D41L prop glows, which
follow the draw clock: `worker_draw` run inside a job (the verify-first
reference, the self run, N4b's fallback) made its own clock hold and its
release left the outer job on the live clock. The clock and the bake table
are now held at the outermost call only. UI 5, both host draws on: self
check 1,648/1,648 identical (before 941/1,184); verify-first 1,341/1,344
accepted rebuilds exact (before 3,122/3,248). The ordinary redraw path does
not nest and is unchanged (Vanilla shots 6/6, validator 914/915 with the
level-load frame).

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): cannot pay at any number of workers, 4-18% dearer than plain, note 487.

### D23N5 - One host pass per game frame for a frame's redraws

**Status: Needs playtest (2026-11-16, [note 493](../documentation/493-d23n5-shared-frame-work-desktop.md): built off by default, exact, measured on the desktop, the field laptop and citadel; default on is the user's decision). Dependencies: D23N4 (note 487).**

Note 487 found that a redraw worker job is mostly our own host work (the
Polymorph draws, the near clip, the D36 joint callbacks, the world pass,
model lighting), which D23N4b's rebuild could not skip. Work: profile a
worker job by host hook in UI 2, 5, 9 and the FAMILY JEWELS start (ms a job,
calls, what depends on the in-between camera or pose and what is the same
for every job of a game frame); then build the largest exact cut: compute
once per game frame what every in-between job of that frame shares, keyed by
its inputs and reused by the later jobs, verified word for word by a
developer check, behind a developer switch. Acceptance: verify-first stream
comparison 0 differences, `PSX_RT_VALIDATE` exact, Vanilla shots
byte-identical; worker CPU measured at 1 and 2 workers on the desktop, then
on the field laptop and citadel.

Work log, 2026-11-16 (desktop lane, [note 493](../documentation/493-d23n5-shared-frame-work-desktop.md)):
D1: worker profiles by host hook (`PSX_PROF_WORKERS` with stacks, one
worker, 180, offscreen, UI 2, 5, 9 and the FAMILY JEWELS start, two runs
each): the world pass 12-27% of a job (most of it our `draw`/`emit`), the
object path 3-20%, water 0-15%, the D36/aim hooks about 2%, the game's own
code 29-37%, dispatch bookkeeping 13-19%. Mapped (with a code-reading
pass) what depends on the in-between camera and poses: every projection,
cull, clip and emit, the D36 callbacks and the normals; the same for every
job of an image: the decode (2-3% of a job, with state that blocks reuse)
and the image itself. A developer statistic (`PSX_RW_IMAGE_STATS=1`):
a load copies 3.5-5.5 MB, a job changes 21-57 pages, half the jobs at one
worker (17-26% at two) draw the image their worker already holds. D2: the
same-image restore (`PSX_RW_RESTORE=1`, off by default): the worker tracks
its writes with the kernel's asynchronous userfaultfd write protection and
`PAGEMAP_SCAN` and copies back only the written pages, the text guard's
bitmap redone only for them; a first `mprotect`/SIGSEGV version was exact
but its signals cost what it saved, replaced. Developer check
`PSX_RW_RESTORE_VERIFY=N` (restored image byte for byte, text bitmap,
then the job again from a whole load on the same draw clock, streams word
for word): 3,240 jobs at 1 and 2 workers, 0 images or bitmaps differing, 3
streams differing, all at a level's lightmap upload, which the same check
shows with the restore off (its baseline). Validator 910 of 912 (the two
in UI 9's level load, as with the restore off: 914 of 916), Vanilla shots
6 of 6 byte-identical. Worker CPU, interleaved, two runs: within the
spread at 1 and 2 workers on the desktop (a restore 74 us against a 647 us
load, the whole loads 150 us dearer). Found on the way: this desktop's
framework tree lacked last night's laptop-lane framework commits (D23V3's
capture, the D23Y5 OSD cache; only in the tracked patch); merged three
ways onto HEAD's framework, the exported patch verified against a clean
pinned checkout. Next: the field laptop (one worker, slower memory) and
citadel. Limitations: offscreen only; the restore needs Linux 6.7+.

Data, 2026-11-17/18 ([note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md) section 5, pure AUTO, real screens, two runs): the emulation thread 0-6 points lower on the field laptop, 0-3 on citadel; game and pictures unchanged (60, lowest tenth 58.1-59.6). The default stays the user's decision.

Work log, 2026-11-23 (found during D23AH): this switch has been on by default in Modernized since the D23AK closeout (2026-11-18, `run.py` MODERNIZED_REDRAW_DEFAULTS); the "built off by default" wording above predates that. The player's field-laptop sessions of 2026-11-23 ran with it on.

### D23O - Leaner render-thread submission

**Status: Done (user, 2026-11-12: "if ive closed any out, then they are accepted"; [note 447](../documentation/447-d23o-render-threads.md)). Dependencies: D23K, D23N.**
The render thread uses about half a core at 180 Hz turning each redraw's GP0
stream into GL calls. Profile it (perf, `eu-stack`), then apply what pays:
persistent mapped vertex buffers, fewer state changes, skipping repeated
uploads whose bytes match (note 443). Acceptance: render thread CPU at 180
Hz halved, no GPU regression, identical pictures.

Work log, 2026-11-11 (part 1, Needs playtest). The user's playtest after
D50A: UI slot 9 still unplayable. The session log's misses were almost all
"render thread drawing": one render thread drew every redraw and ran at
about 90% in the corridor (light lists and shadow classes 18% of it).
Corridor walk at 180 Hz: one thread 9-10 redraw-only switch-offs and 680-
830 misses a launch in RELIT (2-4 and 125-454 even in ORIGINAL); both
instances 0 and 0 at 120, 180 and 240 Hz. Both are now started on 8+
CPUs (one below; `PSX_RENDER_THREADS` overrides), the second drawing only
while the first is busy. Cost: corridor GPU 40% -> 53% (redraws drawn
instead of dropped), CPU +0.4 core; light views +2-5 GPU points. Note 447.
Part 2 (not started): the per-triangle light work, driver and copies.

User playtest, 2026-11-11 (session 19:39, real display, 180 Hz, UI slots 9
and 1 through both slow stretches): "amazing work ... i didnt even get the
slowdowns". Log: 1 redraw-only switch-off (the savestate load) against 60
the session before, 50 render-thread misses (around the load) against
5,014, every logged second at 60 game frames and 180-181 presents, no
underruns. Part 1 accepted; part 2 (lighter render thread) stays open.

Work log, 2026-11-11 (part 2, Needs playtest). Profiled render thread 0 in
the UI slot 9 corridor at 180 Hz (call stacks with library leaves): light
lists 8%, shadow classes 9%, driver 11%, driver mutex contention 11%,
copies 9%. Root cause found in the D41F2 classes: rebuilt world corners
jitter by 0.1-1 unit with the camera, so the per-instance cache key changed
almost every redraw. Built: one shared lock-free class table and a
classifier thread, corners snapped to the 16-unit lattice of level geometry
(96% of pairs known, was about a third; verify modes 0 pixels); light lists
four lights at a time (same lists); a mapped staging ring plus GPU copy into
a video-memory vertex ring (the driver's copy under its shared lock gone; a
first version drawing from mapped memory cost 18 GPU points and was
replaced); snapshot state imported once without copies, a stream hash for
the reuse check, CPU VRAM saved by row, uploads gathered by row. Corridor
walk, game process CPU 162/223/236% -> 144/194/215% at 120/180/240 Hz, GPU
within 2 points, 0 switch-offs (2-4 before); light views render threads
15-22% lighter, GPU slightly lower. Not met: "halved" (the rest is triangle
setup, D23N/D23M). Pre-existing finding: redraw reuse (D23J) differs from a
fresh draw in 4-5 of about 150 validated redraws during a walk, also on the
unmodified build. Note 447, part 2.

### D23P - Low-end scaling: presets and AUTO from measured headroom

**Status: Done (CPU part accepted, user, 2026-11-12: "if ive closed any out, then they are accepted"; GPU presets are D23P2). Dependencies: D23K, D23M.**
With 4 logical CPUs the game asks 180 and sheds (177, audio low); with 2 it
holds 88, with one core 58; a Deck-class GPU (about 1/7 of the 1080 Ti)
would need 7-10 ms a redraw today (note 444). Work: test on a real weak
device if one is available, otherwise simulate (core limits, internal scale,
frame rate); performance presets (internal scale, redraw rate, light and
shadow quality) picked by AUTO from measured headroom at startup, with the
choice shown in the options; check why a fixed 90 Hz presented 60 a second
in one headless run. Acceptance: on 4 logical CPUs and at scale 1, a steady
rate (120 or 90) with no audio low and no governor oscillation; the presets
documented in the manual; Vanilla unchanged.

Work log, 2026-11-11 (CPU part, Needs playtest). Measured after D23O part 2
and D23M, offscreen, `taskset`: 4 logical CPUs hold 180 Hz in UI slot 2
(audio ring 101-264 ms; D23K saw 177 and "audio running low") but about 110
in the UI slot 9 corridor (one switch-off); 120 there holds (0 switch-offs,
ring 234-277 ms); 2 CPUs slow the game itself at 180 (31-59 game frames a
second, underruns) on this and on the unmodified build alike. Built
(framework, `replay_rate.h`, `render_thread.c`): AUTO takes at most 120
images a second with 4-7 CPUs and 60 below 4, as a whole number of
refreshes (180 Hz -> 90), logged once (`AUTO frame rate: at most ...`);
developer `PSX_AUTO_CPUS=N`; fixed choices unchanged; the CPU count is the
process affinity. AUTO itself cannot run offscreen (no display refresh), so
the 4-CPU cap needs a real-display check. The "fixed 90 Hz presented 60"
of D23K: 90 is not a FRAME RATE choice (`player_profiles.py`); inferred,
the profile check reset it to AUTO, which offscreen (no refresh) is 60. Not built: GPU presets (internal scale, light and
shadow quality) chosen from measured GPU headroom, which need a weak GPU or
a reliable GPU-time estimate at startup.


### D23P2 - GPU presets picked from measured headroom

**Status: Done (user-accepted 2026-11-13, closeout authorized; 2026-11-13, [note 464](../documentation/464-d23p2-graphics-profiles.md); 2026-11-12: D23W built its first piece, internal scale AUTO from a measured GPU probe at 60 pictures a second, [note 461](../documentation/461-d23w-field-laptop.md). 2026-11-13, user: "i think it is more realistic to think we have some graphics profiles like simple, pretty etc, maybe 4 gradings. i want the most out of this small machine": four named profiles, AUTO choosing one). Dependencies: D23P.** The GPU half of D23P: performance
presets (internal scale, redraw rate, light and shadow quality) that AUTO
picks from headroom measured at startup, the choice shown in the options;
test on a real weak device if one is available, otherwise simulate (a
Deck-class GPU is about 1/7 of the 1080 Ti: 7-10 ms a redraw before D23M);
also check why a fixed 90 Hz presented 60 a second in one headless run.
Acceptance: on 4 logical CPUs and at scale 1, a steady rate (120 or 90)
with no audio low and no governor oscillation; the presets in the manual;
Vanilla unchanged.

Work log, 2026-11-13 (Needs playtest), [note 464](../documentation/464-d23p2-graphics-profiles.md).
Built: OPTIONS > DISPLAY > GRAPHICS (first row) with SIMPLE (1x, original
light, simple shadows, original draw distance, 60), BALANCED (2x, Enhanced,
simple, extended, AUTO), PRETTY (3x, Relit, sharp, extended, AUTO), ULTRA (4x,
Relit, sharp, extended, AUTO), AUTO (one of the four from the D23W probe:
`gpu_auto.grade_for`, at or under 2.5 / 5.0 / 14.0 ms) and CUSTOM; profile
schema 74 `graphics` (fresh installs AUTO, older profiles CUSTOM with every
choice kept); `run.py --graphics`, the grade applied at launch and shown and
saved back by OPTIONS; a row a grade sets, changed by hand or by flag, makes
it CUSTOM (`game_menu.cpp` `graphics_grade` / `graphics_custom`, table
checked against the Python presets by a test). Only AUTO runs the probe.
Measured on the field laptop (the user's other settings, real display, 30 s
walks, UI slots 2/8/9): SIMPLE 60 game frames, lowest tenth 59-60, GPU busy
38-46%; BALANCED 60, 60, 51-54%, no underrun; PRETTY 59-60, 57-59, 64-69%,
one underrun a walk; ULTRA 56-59, 51-54, 74-79%. AUTO picks BALANCED there
(probe 6.3-6.9 ms). Checked in game with real keys: a grade confirmed in
OPTIONS > DISPLAY set and saved its rows. Tests: schema 74 migration, the
launch rows per grade, AUTO, CUSTOM by flag, Vanilla never graded, the grade
rule, the menu table; 141 profile tests pass. The redraw-rate side of the
original job (refresh-aware presets above 60) is D23Y's on 2 cores and stays
with D23R elsewhere. Remaining: the user's play of the grades on the laptop.

- 2026-11-13 closeout: the user, playing on the field laptop: "it runs so unbelievably well in this latest iteration. it's now totally playable and acceptable on this laptop ... i am pleased with the settings and quality, so lock this in and close it out". GRAPHICS grades and AUTO kept as built; GEOMETRY and TEXTURES now default to CORRECTED for new and reset profiles.

### D23Q - Renderer restart stall after an early savestate load

**Status: Done (user-accepted 2026-11-12, closeout authorized; [note 452](../documentation/452-d23q-restart-stall.md); D23K finding, [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23K.**
In 4 of 7 D23K lab launches on the real display (windowed and borderless),
the starvation watchdog (4 s) aborted the game during the renderer's restart
right after a savestate load at boot; headless runs did not stall. Not seen
by the player so far. Work: time the restart (window, GL context, shader
builds, render thread), find what blocks the emulation thread and move it
off the heartbeat path or make it fast. Acceptance: 20 lab launches on the
real display with a savestate load at boot and the default watchdog, none
aborted.

Work log, 2026-11-12 (Needs playtest; note 452). Cause, confirmed in code
and by a lab freeze: the "restart" is the render threads' start (hidden
windows, shared contexts, their renderers), made on the emulation thread at
the first gameplay redraw, which follows the load. The starvation watchdog
is checked on that same thread between guest cycles and measured wall time,
so a host block over 4 s (the context creation on the troubled display
driver of that hour) aborted the game after it had recovered; the D23K dump
has no guest activity after the last heartbeat. Changes: the watchdog now
counts only time the guest runs (gaps over 250 ms between checks move the
heartbeat; gaps of 1 s or more are logged); the render threads start half a
second into the opening when Polymorph has asked for them (a normal launch
used to start them only after the player's load); the log times the start.
Offscreen 180 Hz: windows and contexts 2.5-5.9 ms, each thread's renderer
56-69 ms, workers' fork 7 ms, no host gap of 30 ms or more; the whole
process stopped twice for 6 s in play: both freezes logged and survived
(the earlier variant aborted on the second, the old failure); Vanilla
unchanged (no render threads). Remaining: the 20 real-display launches need
the user present (real-display runs are not done unattended).

Accepted 2026-11-12. The user played on the real display, loading saves early
and often: "all the save/load stuff is fine"; no abort. The user's play stands
in for the 20 unattended real-display launches. Done; closeout authorized.

### D23S - Console overlay re-uploaded every present

**Status: Done (user-accepted 2026-11-12, closeout authorized; fixed 2026-11-11; follow-up 2026-11-12, note 450). Dependencies: D23K.**
User (playtest 2026-11-11): "the GPU goes up to like 70% when the console is
open". Reproduced offscreen at 180 Hz (UI slot 2): GPU work 34% closed, 82%
open. Cause: the console's 1280 x 440 image went through the shared OSD
texture with a full upload (2.25 MB) and a CPU alpha scan at every present,
about 400 MB/s at 180 Hz, although it changes only when typed into. Fix: the
console keeps its own texture, uploaded only when `host_osd` redraws it
(`host_osd_console_generation`). After: 35% open against 36% closed; shots
with the console open, typed into, backspace and closed draw as before.

Follow-up, 2026-11-12 (user: "opening the console still adds about 60",
GPU memory or GPU %, measure both; also on the real display). Measured GPU
work and memory closed / open / typed / closed, offscreen and on the real
display (one windowed launch each, player presentation, captured mouse):
UI slot 2 real display 35.6% / 36.5% / 35.1% / 35.7%, game process memory
+4 MiB while open; pause menu 11.6% throughout; UI slot 6 about 60% open and
closed alike. So the console adds no GPU work; the "60" was the Necro bolts'
shadow maps (D41N, fixed: UI slot 6 now 26-28% on the real display). The
console's texture is now deleted once the console has fully retracted, so a
closed console holds no GPU memory. Note 450 section 2. Not built: glyph
quads from a font atlas (Quake's console), which would only save the 4 MiB
while open.

Accepted, 2026-11-12. User after playing: "colossally huge improvement in resource usage ... a significant drop in GPU usage in regular gameplay". Closeout authorized.


### D23T - The object renderer projects emptied meshes

**Status: Done (user-accepted 2026-11-12, closeout authorized; [note 458](../documentation/458-d23t-object-renderer-short-pass.md)). Dependencies: D23N1.**
Since unified geometry (D17AM3) our host draws every polygon of a prop or
actor joint, and the object hook (`near_clip.cpp`, `object()`) hands the
game's object renderer `0x80010000` a mesh copy with the polygons it left;
with none left the routine still projects every vertex (RTPT, the vertex
pass of `0x80010010`, about 2,900 stores a job). The world renderer got a
null mesh for the same case in D17AQ. Measured in UI slot 2 turning at 180
Hz: the game code after the object hook is 7.7% of a redraw job, mostly that
pass (note 456). Work: pass a null mesh when no polygon is left (the routine
returns at `0x80010008`), keeping whatever else the vertex pass leaves that
later code reads (the GTE data registers, the scratch vertex buffer, any
colour-index side effect: find out first); prove it with a fork-and-compare
of the same job (the D23N1 tools) and `PSX_RT_VALIDATE`. Acceptance: redraw
streams identical word for word over thousands of jobs in UI slots 2, 8 and
9; worker and emulation-thread CPU measured on matched builds; Vanilla
unchanged.

Work log, 2026-11-12 (Needs review). Asked first: what does the skipped
vertex pass leave that later code reads? Built a developer check on the
D23N1 tools (`recomp/src/ttk/redraw_t_check.inc`, `DNTTK_T_CHECK=N`): a checked
job forks; the copy runs the whole pass, the job the new path; both log every
top-level store (address, value, size, PC) and the registers at every hook
entry, record their whole GP0 stream, and are compared after the submit
(stream, every store, every hook entry, CPU state, RAM, scratchpad and mod
memory at the flip). With a bare null mesh and every leftover poisoned
(`DNTTK_T_CHECK_MODE=4`), streams stayed identical, but stores and memory
differed in 1,542 of 1,632 jobs: callers store v0/v1/t-registers into a
matrix pad halfword that `0x80010d60` copies on, so a null mesh is not exact.
A shortened pass then showed the game also reads stale vertex-buffer slots
(a matrix built at scratchpad `0x1f800040` by `0x80097b78` leaves its pad,
slot 3 of the buffer, unwritten). Built (`near_clip.cpp`, `object()`, no
framework change): the routine now gets a mesh copy with only its last two
vertex triples and an empty list, so its own instructions leave the GTE,
PGXP, registers and save area exactly as the whole pass does, and our host
code writes the buffer's later slots from the exact host projections. Check:
9,824 jobs in UI slots 2, 8, 9 and third person, all exact (about 132
million stores, 2.1 million hook entries); poisoning every buffer range at
job start changes nothing read. Cost, same binary with `DNTTK_OBJECT_NULL=0`,
turning at 180 Hz: workers 1-2 points of a core lower in each view, the
emulation thread unchanged; per job 0.10-0.15 ms (1.6%) in UI slot 2 and
0.15-0.19 ms (2.7%) in UI slot 9. The 6-8% estimated in D23N2 was measured
with every store logged by the recorder. Vanilla never reaches this code
(every path into `object()` needs the Modernized profile).

Accepted 2026-11-12 after the user's playtest (UI slots 8 and 2 and 10 minutes in third person: "plays absolutely beautifully"); closeout authorized.


Work log, 2026-11-15 (re-checked, still Done), [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 1. The user
asked for D23T again in a desktop session; it was already built and
accepted, so the current build was re-checked instead of rebuilding it:
2,160 checked jobs exact in UI 2, 8 and 9 (`DNTTK_T_CHECK=3`); per-job
worker time, interleaved, whole pass against the short pass: median UI 2
12.01/12.04 -> 11.72/11.81 ms, UI 8 6.38/6.69 -> 6.04/6.30 ms, UI 9 within
the spread (slowest tenth 15.6-15.8 / 8.5-8.9 / 3.0-3.1 ms, about the
same).
### D23U - Opaque scene drawn front to back

**Status: Done (user-accepted 2026-11-12, closeout authorized; off by default until D23W, [note 459](../documentation/459-d23u-opaque-depth-prepass.md)). Dependencies: D23M2.**
After D23M2 a redraw is mostly the scene itself: textured batches are
0.58-0.78 ms of a 0.76-0.94 ms redraw at 180 Hz (UI slots 2 and 8), and
`[gpu overdraw]` counts 2.8-2.9 samples passing per output pixel: the PS1
ordering table draws back to front, so every layer of every pixel runs the
full lighting shader. Our host geometry has exact host depth (D17, D41), so
opaque host triangles can be drawn nearest first with a strict depth test
(the ordering table kept for translucent, mask-tested and depthless
primitives, and the equal-depth tie rule kept: the later primitive of the
original order must still win), letting the GPU reject hidden fragments
before shading. Find out first how the texel-0 discard and the STP passes
limit early depth rejection on the target GPUs. Acceptance: pictures
identical (`PSX_RT_VALIDATE`, matched shots), GPU a redraw and whole GPU on
matched builds in UI slots 2 and 8 at 180 Hz and on a Deck-class estimate,
Vanilla unchanged.

Work log, 2026-11-12 (Needs review). Measured first, as asked: a new
`GL_FRAGMENT_SHADER_INVOCATIONS` count beside the samples passed shows the
GPU rejects almost nothing early today (2.94 invocations against 2.78 samples
a pixel in UI slot 2), and that in a depth-EQUAL pass without depth writes it
does reject hidden fragments before the textured shader despite its texel-0
and coverage discards; the STP stencil fix-up runs only under a mask check,
which no triangle in UI slots 2, 8 or 9 uses. Built (framework,
`gpu_gl_renderer.c`; ordering table kept): a run of opaque host-depth
triangles is drawn depth-only with a new prepass program (only the decisions
that remove a fragment), then shaded at depth EQUAL in submission order, so
equal depths still resolve to the later primitive; translucent, depthless,
test-only and depth-only runs and mask checks keep the ordering table. A
developer segment variant spans one prepass over many batches
(`PSX_GL_ZPRE_SEGMENT=1`). Pictures: `PSX_REPLAY_VALIDATE_ZPRE` (each redraw
drawn with and without, RGBA-exact) exact over about 21,000 redraws in UI
slots 2, 8 and 9 in every variant, except the first redraw at the level's
lamp bake upload, which differs identically with the prepass off on both
sides (a validator artifact, as in D23M2); `PSX_RT_VALIDATE` 691 of 691;
Vanilla screenshots byte-identical to this session's starting binary.
Modernized screenshots do not repeat between launches (two launches in the
same mode differ by 296,152 pixels), so matched shots cannot show it there;
the in-process validator does. Cost: without a length rule the prepass broke
180 Hz in UI slot 2 with two render threads (84-100 presents; about 143 short
runs a redraw cost the render thread a third more); with a 64-triangle rule,
UI slot 8 about 1.5 points of whole GPU lower (9 pairs) and 0.06 ms a redraw
(6%) lower at full clock, UI slot 2 equal with two render threads and up to
0.17 ms a redraw higher with one. Cost attribution (`PSX_GL_DEV_SKIP`): the
opaque host scene is only 0.78 of 2.76 samples a pixel in UI slot 2; with no
textured scene drawn at all the whole GPU is still 14.4% (UI slot 8) and
19.2% (UI slot 2): that floor is the larger cost, now D23V. Left off by
default (`PSX_GL_ZPREPASS=1` enables it). Deck-class estimate (note 444,
factor 7): UI slot 8 about 0.4 ms a redraw less, UI slot 2 none; the 4 ms bar
is not reached. Framework patch exported and verified on the pinned
`08ec704a`.

Accepted 2026-11-12. User, playing with `PSX_GL_ZPREPASS=1`: UI slot 8 "perfectly smooth ... maxing at 42%gpu", the water "completely fine"; closeout authorized. The default is decided on real weak hardware in D23W.

### D23V - The GPU floor

**Status: Done (2026-11-15, user playtest; [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) sections 1-2; found in D23U 2026-11-12, [note 459](../documentation/459-d23u-opaque-depth-prepass.md) section 3). Dependencies: D23K.**
With no textured scene drawn at all (developer `PSX_GL_DEV_SKIP=31`, wrong
pictures) the whole GPU at 180 Hz is still 14.4% in UI slot 8 and 19.2% in
UI slot 2 (normal 19.9% and 24.0%; one render thread, turning, offscreen),
about 0.8-1.05 ms of GPU a presented frame against the D23K bar of 0.4 ms;
the redraw timeline still shows 0.54-0.89 ms of "tex" with nothing drawn,
part of it waiting for the render thread. The textured scene itself is only
about 5 points. Find what the floor is with the same skip-and-measure method
(per-batch setup and vertex-ring copies, presents and the output pass,
captures and loads, lighting, shadow and bake setup, the emulation thread's
own frames, flat batches, timestamps) and remove the large parts.
Acceptance: a measured breakdown of the floor in UI slots 2 and 8 at 180 Hz,
jobs for its parts, pictures exact, Vanilla unchanged.

Work log, 2026-11-15 (Needs playtest), [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) sections 1-2. Autonomous
session on the field laptop (user: "1. D23V GPU floor (run
recomp/analysis/d23y6/d23v.sh, then find and remove the waste; before/after
at 120 and 180)"). Host quiet; the idle desktop alone keeps the HD 5500 out
of RC6 7.5-8.2% at 300 MHz. On this GPU busy hardly moves (75-82%), the
clock does, so the measure is busy x clock. Developer bounds 128-1024 added
to `PSX_GL_DEV_SKIP` (capture copies, fills and wide clears in redraws, flat
batches, the presents' quad). At 180 in UI 2 / UI 8: no textured runs -30%
/ -42%, no capture copies -14% / -25% (CPU clock +270 / +530 MHz), band
copies, clears and fills, flats and presents each within the spread. So the
floor past the scene is the per-frame snapshot capture (2.2-2.5 ms of GPU a
game frame). Built, exact (framework): the direct band (a redraw's margins
from the wide surface, its centre straight from the canonical surface; one
large copy fewer) and the margins-only wide capture (no redraw reads a wide
surface's centre). Developer `PSX_REPLAY_BAND_DIRECT=0`,
`PSX_REPLAY_CAPTURE_MARGINS=0`. Validator exact (UI 2 174/174, UI 8
107/107, UI 9 295/295); matched shots new against old 0.00-0.11% (two runs
of one build 0.00-0.95%). Before/after offscreen at 180 and 120: capture
2.26-2.33 -> 1.65-1.70 ms, band 0.64-0.66 -> 0.36-0.37 ms, emulation thread
-1 to -5 points, whole-GPU work and CPU clock within the spread. Vanilla:
no redraws, neither path runs (smoke launch). The rest of the capture (the
canonical surface) is D23V2.

Work log, 2026-11-15 (Done). User playtest on the field laptop: "i think it
plays smooth, it feels very good to play"; no picture faults reported.
Closed at the user's request ("lock it in and close it out").

### D23V2 - The rest of the snapshot capture

**Status: Done (2026-11-15, user: "it runs fine" ... "yep close it out", accepting a third instead of half; [note 479](../documentation/479-d23v2-field-laptop.md); built, exact, [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 3; split from D23V 2026-11-15, [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 7). Dependencies: D23V.**
After D23V the per-frame snapshot capture still costs 1.65-1.70 ms of GPU a
game frame on the field laptop, nearly all of it the 2x canonical surface
(2048 x 1024 colour at 2x) and the raw mirror. Copy only what a redraw can
read (its band, packs, VRAM copy sources and sampled pages), or only the
32 x 32 tiles written since that snapshot slot was last filled, together
with D23V's margins-only wide copy. Note 469 measured an exact incremental
capture alone with no game-frame gain, so measure GPU work (busy x clock),
the CPU clock and present cadence at 120 and 180, not only game frames.
Acceptance: capture GPU time at least halved again, pictures exact
(validator and matched shots), Vanilla unchanged.

Work log, 2026-11-15 (Needs measurement on the field laptop), [note 478](../documentation/478-d23t-d23n2b-d23v2-desktop.md) section 3.
Built on the desktop (user: "build it and prove pictures exact here; leave
the GPU measurement for the field laptop"). Framework (exported): every
write to the canonical surface stamps its 32 x 32 tiles through
`coh_invalidate`; a capture copies only the tiles stamped since its
snapshot slot was last filled (`glCopyImageSubData`, depth-stencil too when
copied), the whole surface for a new slot, another scale or missing
depth-stencil. Developer `PSX_REPLAY_CAPTURE_INC=0`,
`PSX_REPLAY_CAPTURE_CHECK=N`. Exact: 285 sampled captures read back, 0
differing tiles (245 of 512 tiles copied a capture); validator 912/914,
309/309, 309/309 against the old binary's 919/920, 304/305, 306/306 (the
same two differing images in both, also with the whole copy); Vanilla
matched shots identical. On the 1080 Ti the capture is too cheap to judge
(0.12-0.17 -> 0.08-0.09 ms in one round, noise in the other). Not done:
the acceptance measurement on the field laptop (capture GPU time, busy x
clock, CPU clock, cadence at 120 and 180).
User check on the desktop (water, sky, fire, explosions, HUD, loads and
menus at 180): "all 3 tests look fantastic"; locked in, the measurement
next on the field laptop.

Work log, 2026-11-15 (In progress), field laptop, [note 479](../documentation/479-d23v2-field-laptop.md).
User: "work on D23V2 autonomously: measure the incremental snapshot capture
against the whole copy ... Acceptance (note 478 section 3): capture GPU time
at least halved again." Lab `recomp/analysis/d23y6/` (`d23v2.sh`,
`d23v2x.sh`, `d23v2v.sh`, `v2sum.py`). Five interleaved rounds, 120 and 180,
UI 2, 8 and 9, offscreen: capture 1.59-1.65 -> 1.04-1.08 ms of GPU a game
frame (-33 to -35% in every cell; 245 of 512 tiles copied, the game's two
frame bands every frame); whole-GPU work -6 to +3% (inside the spread), CPU
clock unchanged, emulation thread 1-2 points lower, presents, game frames
and swap cadence unchanged. **Acceptance not met.** INFERRED from the A/B: the
raw mirror and the wide margins are about 0.55 ms that no tile selection
removes; halving needs the canonical copy at about 115 tiles, so the bands
must be left out where no redraw reads their old contents (or the mirror and
margins cut). Fixed (framework, exported): 1 run in 18 never went
incremental, because only `rr_rect_blit` set the checked flag for
`glCopyImageSubData`; the capture now checks it itself with one live tile.
12 of 12 incremental runs afterwards. Exact: readback check 1,250 captures,
0 differing tiles; validator 963/963, 952/952 (whole copy), 962/962,
938/938; Vanilla matched shots against the pre-D23V2 binary byte-identical.

Closed 2026-11-15 on the user's acceptance after the field-laptop
measurement (user: "it runs fine" ... "yep close it out"): a third off the
capture instead of half; the rest is D23V3.

### D23V3 - Leave the frame bands out of the capture

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; no gain on the real screens, more GPU work on the field laptop, note 497). Dependencies: D23V2.**
After D23V2 the capture is 1.04-1.08 ms of GPU a game frame on the field
laptop: 245 of 512 canonical tiles a capture, nearly all of them the game's
two frame bands (rewritten every frame), and about 0.55 ms (INFERRED) of raw
VRAM mirror and D23V's wide margins that no tile selection reaches. Leave a
band's tiles out where no redraw reads their old contents (first check
`rp_band_direct`, the band copy's sources and the validator's paths), or cut
the mirror and margins. Low priority: D23V2's third showed no whole-GPU,
CPU clock or cadence gain, so do it when the GPU is short elsewhere.
Acceptance: capture GPU time at most half of D23V2's (about 0.53 ms),
pictures exact (readback check, validator, matched shots), Vanilla
unchanged; measure with `recomp/analysis/d23y6/d23v2.sh` and `v2sum.py`.

Work log, 2026-11-16 (field laptop, [note 486](../documentation/486-d23v3-d23m3b-d23r-measurements.md)):
measured: the capture 1.03-1.07 ms (whole copy 1.59-1.63) at 120, 245 of 512
tiles a capture; the render threads read only about 38 tiles a session from
the snapshot, all in tile row 7 (VRAM rows 224-255). Capturing only what a
redraw can read before writing it would reach the target; first list every
snapshot reader.

Work log, 2026-11-16 overnight (laptop lane, [note 488](../documentation/488-d23v3-needed-tiles-capture.md)):
every redraw read of a canonical snapshot tile is a render thread's lazy
load or a whole load (the validator redraws from the live surfaces; the
band copy reads the thread's own surfaces after a lazy load). Built behind
developer `PSX_REPLAY_CAPTURE_NEED=1`: render threads report the tiles they
load per slot; after 120 learning captures a capture copies only stamped
tiles in that sticky set (16 of 512 in UI 2, 8 and 9, all of tile row 7)
and marks the rest stale in the slot; a load of a stale tile fails that
session and the slot's later redraws are drawn on the emulation thread
until it is captured again, so no picture comes from a stale tile. The
validator now drops a validation whose redraw was replaced. Field laptop,
two interleaved runs each: capture 1.05-1.09 -> 0.46-0.48 ms a game frame
at 120 and 180 (target 0.53), 0 misses; whole-GPU work within the spread.
Exact: readback check 0 differing tiles, validator 898/899 against 907/908
without it (the one the level-load frame), Vanilla shots 6/6. Needs the
user's real-display look and default-on decision.

Work log, 2026-11-16 (three-machine session, [note 497](../documentation/497-d23v3-three-machines.md)): `inc` against
`need`, interleaved, offscreen, host checked before every run. Exact on
every machine: 0 stale tiles; 0-8 redraws a run drawn on the emulation
thread. Field laptop: at SIMPLE the GPU's work -1 to -9% in 9 of 10 cells
with pictures unchanged; at BALANCED no loss outside the unsteady FAMILY
JEWELS start; at the player's 4x no conclusion (redraws mostly off).
Desktop: pictures unchanged, work -10 to -12% in UI 2 and the town at 120,
equal elsewhere. Citadel: work lower in 9 of 10 cells but UI 2 at 120 fell
back to the original presentation ("workers behind") in 4 of 10 runs with
`need` against 1 of about 20 without. Not "not worse anywhere": default on
not proposed; Blocked until citadel's worker count (D23AC) is settled.

Work log, 2026-11-16 evening ([note 497](../documentation/497-d23v3-three-machines.md) section 6b): the real
screens, the D23AD/D23AE build, two interleaved runs a scene: at pure AUTO
60 pictures a second either way on the field laptop and citadel, 0 stale
tiles; whole-GPU work with `need` 6-9% higher on the laptop in four of five
scenes, equal on citadel; at ULTRA 120 on citadel equal in four scenes, one
`need` run at the FAMILY JEWELS start dipped (48 pictures). No gain where it
matters; stays off and Blocked, not proposed for default on.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): no gain on the real screens, more GPU work on the field laptop, note 497.

### D23W - Field laptop: measure on real weak hardware

**Status: Done (user-accepted 2026-11-13: "it plays well"; closeout authorized; [note 461](../documentation/461-d23w-field-laptop.md)). Dependencies: D23T, D23U.**
Every weak-hardware figure so far is an estimate (note 444's factor of 7 for a
Deck-class GPU) or a CPU-limit simulation on the 1080 Ti machine (D23P). On
the user's field laptop:

1. Setup with the user: clone the repo (with submodules), install the tools of
   `documentation/04-build-and-run.md`, place their owned SLUS-00583 dump in
   `game/`, build with `recomp/tools/local/build.py`. The repo has no saves:
   the user copies the savestates for UI slots 2, 8 and 9 into the laptop's
   `recomp/saves/local-play` (the lab works only on private copies of them).
   Record the hardware (CPU model, logical CPUs, GPU and driver, RAM, display
   refresh, power profile, on battery or mains) without personal names or
   paths.
2. Make the lab vendor-neutral: `recomp/analysis/d23k/lab.py` reads the GPU
   through `nvidia-smi`; add the laptop GPU's busy measure (for example
   `gpu_busy_percent` in sysfs on AMD, `intel_gpu_top` on Intel) and keep the
   CPU-per-thread measure.
3. Measure, offscreen first, in UI slots 2, 8 and 9, turning and walking: what
   AUTO picks (D23P), presents a second held, CPU per thread, GPU busy and GPU
   a redraw (`PSX_GPU_PHASES=1`), audio lows and governor shedding; then the
   same with `PSX_GL_ZPREPASS=1` (D23U), whose default is decided here (it
   helped UI slot 8 and did not help UI slot 2 on the 1080 Ti, and was worse
   there with one render thread, which is what a 4-7 CPU laptop gets); then
   lower internal scales.
4. With the user present, a short real-display check of the same views.

Acceptance: a measured weak-hardware report replacing the estimates in notes
444 and 459, the D23U default decided from it, inputs for D23P2 (presets) and
D23R (parity bar on a weak GPU), Vanilla unchanged.

Work log, 2026-11-12 (Needs playtest), [note 461](../documentation/461-d23w-field-laptop.md).
Setup: cloned with submodules, the user's dump and private card copies in
place, `build.py --jobs 4` in 55 minutes. Hardware: i7-5600U (2 cores, 4
logical CPUs), Intel HD Graphics 5500 on Mesa 26.2.6, 8 GB, 1920x1080 at
60 Hz, XFCE on X11, performance profile, mains. The user plays UZDoom (native
1080p, dynamic lights, SSAO and MSAA off, 60 cap), EDuke32 (Polymost
1600x900) and Banjo Recompiled (MSAA 2x) here. Lab `recomp/analysis/d23w/`
(local): the D23K lab with a vendor-neutral GPU busy (`gpu.py`: nvidia-smi,
amdgpu `gpu_busy_percent`, Intel RC6 residency without root) and a `rates`
case (game frames, pictures shown, busiest thread, CPU clock, per second).

Found: the user's first play ("it runs like ass") was AUTO slowing the game
itself: 4 logical CPUs allowed the replay path, and AUTO keeps replay at any
cap, so in UI slot 2 the game ran 33 frames a second with 14 underruns in
25 s, against 60 with no underrun at a fixed 60. Fewer workers did not help;
the replay share of the emulation thread is the cost. Built: physical cores
in the cap (`psx_host_physical_cores`, `replay_auto_cap_for`: 2 or fewer cores
-> 60) and AUTO at 60 as the original presentation (`launch_rate` in
`frame_replay.cpp`, at launch and in OPTIONS). Then the GPU: 4x failed on the
real display (dips to 41), 3x dipped to 42, 2x held 60 every second. Built:
`--gpu-probe` (`gpu_probe.c`, a timed synthetic 4x frame, about 1 s) and
`gpu_auto.py` (largest scale with probe x scale^2/16 <= 2.5 ms, cached in
`config/gpu-probe.json` per GPU PCI ids and binary, failures not cached, 2x
fallback); internal scale 0 = AUTO is the Modernized default (`--internal-scale
auto`, OPTIONS > DISPLAY > RESOLUTION > AUTO), saved fixed scales unchanged.
Here: 6.33-6.94 ms -> 2x.

Evidence, fresh default profile on the real display, 30 s walking, turning
and firing: UI slots 2, 8 and 9 each 60 game frames and 60 pictures a second
(lowest tenth 59), busiest thread 63-66%, GPU busy 65-71%, no underrun; worst
seconds 51 and 54 with one "behind" each. D23U prepass (`PSX_GL_ZPREPASS=1`):
70% against 71% (UI slot 8), 68% against 65% (UI slot 2), within noise;
whether this path engaged it is not shown by the logs; it stays off. Vanilla
checked: 1x, fixed 60, no probe, no workers, 60. Tests: `test_gpu_auto.py`
(rule, cache, parsing), the AUTO cap cases in `test_auto_frame_rate.py`,
profile tests for AUTO (the probe isolated from the player's cache). Also
fixed: `run.py` returned from a `finally` (Python 3.14 warning; it also hid
errors).

Remaining: the user's own play of the new defaults on the laptop (gameplay
acceptance); one laptop calibrates the probe budget, which is for 60 pictures
a second (refresh-aware presets and light and shadow quality stay D23P2); the
Windows core count is compile-checked only.

Work log, 2026-11-13 (Needs playtest), second pass, [note 461](../documentation/461-d23w-field-laptop.md) section 6.
The user's settings (DISPLAY, 3x, Relit, first person): DISPLAY had brought
replay back (session log 17-56 game frames a second); it now takes AUTO's CPU
cap. UZDoom reference (Ancient Aliens MAP01, the user's settings): GPU busy
62%, main thread 28%. Mesa `INTEL_MEASURE` per render pass showed about 3.6 ms
of a 9.5-10 ms GPU frame around the scene; removed without changing the
picture: the per-present drawable copy for hold-last (only rewind and netplay
show it), the widescreen centre copy over the full VRAM height (now the band
the caller reads; the opening matches row for row), the full window clear for
pillarbox bars (bars only). Blits 2.21 -> 0.55 ms, clears 0.84 -> 0.42 ms;
the user's settings: lowest tenth 56/55/55 -> 58/59/58, GPU busy 80/78/75% ->
72/73/68%. Remaining dips are one-off CPU events (lamp snapshot, sleeping-enemy
handovers, relit room changes). Jump: a walking jump at a step got the original
running jump (the step probe's run gait 76 was outside the takeoff's walk
gaits); fixed in `modern_controls.cpp`. Tests: 251 local, OK.

Accepted 2026-11-13. User after playing with their settings (DISPLAY, 3x,
Relit, first person): "it plays well, but there feels to be a slight latency";
at 120 "the visuals seem way smoother ... but there's so much stuttering the
gameplay becomes unplayable". Closeout authorized; follow-ups D23X (latency
and vsync), D23Y (120 fps on 2-core hardware), D23Z (one-off CPU spikes) and
the four graphics profiles of D23P2.

### D23X - Input latency and vsync on the field laptop

**Status: Done (user-accepted 2026-11-13, closeout authorized; 2026-11-13, [note 462](../documentation/462-d23x-input-latency.md); backlog, user 2026-11-13: "it plays well, but there feels to
be a slight latency with it. i cant quite explain. am i imagining this?").
Dependencies: D23W.** On the field laptop (note 461) the original 60
presentation shows each new game image for two refreshes (the game updates
about 30 times a second), under the driver's vsync at 60 Hz with the
wall-clock pacer; 120 on the 60 Hz panel felt like "vsync off" to the user.
Measure input-to-picture latency (input sample to the present that first shows
it, the debug latency ring and a camera-free method such as a key-triggered
colour flash read back from the window) at 60, at DISPLAY and at 120 against
EDuke32 and UZDoom on the same machine; find where the time goes (input
sampling point, emulation frame, present queue, compositor, driver buffering);
offer a player-facing vsync choice (on, off, adaptive where the driver has it)
in the options and the manual. Acceptance: measured latency numbers before and
after, the user's judgement on the laptop, Vanilla unchanged.

Work log, 2026-11-13 (Needs playtest), [note 462](../documentation/462-d23x-input-latency.md).
Measured with a camera-free probe (XTest mouse turn, XShm screen polling;
local lab `recomp/analysis/d23x/`), median input to picture in UI slot 2:
ours 129-135 ms at the original 60 (vsync on or off, windowed, borderless or
exclusive within a few ms), 66 ms with late-camera redraws at 60, 64-74 at
120; EDuke32 22, Quake Brutalist Jam III about 35, UZDoom with Ancient Aliens
40, Quake Alkaline about 52. The user is not imagining it. Traced
(`PSX_LAT_TRACE`, `DNTTK_LAT_TRACE`, new developer traces): the event reaches
the next field's pump, the next 30 Hz logic frame takes the look and the view
composition reads it within 2 ms, and the image is flipped two game frames
later; the same with the Classic renderer, so it is the original game's
pipeline. Ours added one field: the present ran at the start of vblank and
the game flips about 19,900 cycles later, inside the console's vertical
blank. Built: present at the flip (framework `main.cpp` display half split
out as `sdl_present_display`, run from the service edge after GP1(05h) or the
end of vertical blank; `gpu.c` flip flag; `psx_mod_set_flip_present`,
Modernized only, `DNTTK_FLIP_PRESENT=0` off; netplay, movies and replay keep
the old path): 113-119 ms with vsync off, 126-129 with vsync on. OPTIONS >
DISPLAY > VSYNC ON / OFF / ADAPTIVE (schema 74 `vsync`, `run.py --vsync`,
`PSX_VSYNC`, live through the new `psx_mod_request_vsync`), checked in game
with real keys (`psxrecomp: vsync off`, saved). Opening, gameplay and pause
menu pictures compared with the old path: the same. The rest of the latency
needs the late camera at every present, which on 2 cores is D23Y's work
(note 463). Remaining: the user's judgement on the laptop.

- 2026-11-13 closeout: the user, playing on the field laptop: "it runs so unbelievably well in this latest iteration. it's now totally playable and acceptable on this laptop ... i am pleased with the settings and quality, so lock this in and close it out". present at the flip and VSYNC kept as built; the remaining latency (the game's 30 Hz pipeline) moves to the late camera and D23Y2-D23Y3.

### D23Y - 120 fps on 2-core hardware

**Status: Done (research and plan accepted 2026-11-13, closeout authorized; further efficiency continues in D23Y2-D23Y5; [note 463](../documentation/463-d23y-120-on-two-cores.md); child jobs D23Y1-D23Y5; backlog, user 2026-11-13: "we need to put a backlog job in,
or series of backlog jobs in now to get hardware of this level to work with
our 120 fps. ancient aliens can do it. eduke can do it. half life can do it.
there's no reason why we cant do it with enough deep research ... i want it
feeling essentially perfect on this computer"). Dependencies: D23W;
coordinate D23N (captured redraws) and D23X.** On the field laptop (i7-5600U, 2
cores, 4 threads; HD Graphics 5500) any replay rate slowed the game itself
(AUTO at 120: 33 game frames a second; note 461): the replay path's share of
the emulation thread, not the worker processes, was the cost (one worker
changed nothing), and the CPU and GPU share one 15 W budget. First a deep
research report: where every millisecond of a 120 Hz frame goes on this
machine (emulation thread, capture, workers, render thread, GPU, power), how
EDuke32, UZDoom and Half-Life reach 120 here, what D23N's captured redraws
(no game code re-run per redraw) would save, and alternatives (interpolating
from the captured scene on the GPU, one render thread sharing the emulation
core, fewer redraws with frame generation), then a plan of child jobs, each
with a measured target. Acceptance for the series: a steady 120 on the
laptop's 60 Hz panel and on a 120 Hz display if one is available, in UI slots
2, 8 and 9 with the user's settings, game speed never lowered, and the user's
judgement that it feels right.

Work log, 2026-11-13 (Needs review), [note 463](../documentation/463-d23y-120-on-two-cores.md).
Research on the field laptop: the package is limited to 15 W sustained (RAPL
PL1, 25 W for about 28 s) for CPU and GPU together; redraws at 60 raised all
threads from 61-67% to 121-138% and the clock fell from about 3.1 GHz to
1.2-2.3, so the game itself slowed (54-58 game frames, lowest tenth 36-48;
at 120: 43-51, 26-33). With the governor off a redraw costs the workers
23-30 ms of CPU here (about 10 ms on the desktop); the emulation thread's
profile is unchanged with redraws but needs 38% more CPU time at the same
clock (shared core and cache). References on the laptop: EDuke32 holds 120
(one thread, GPU 73%); UZDoom with the user's settings reaches about 70 at a
120 cap (GPU-bound, 92%); both use one thread for a whole frame. Plan:
D23Y1 (built), D23Y2-D23Y3 a GPU late warp for in-between presents (the
likeliest way to 120 here, also the latency fix), D23Y4 thread placement,
D23Y5 a lighter emulation thread, D23N2B-N4 with a laptop target. Waits for
the user's review of the plan.

- 2026-11-13 closeout: the user, playing on the field laptop: "it runs so unbelievably well in this latest iteration. it's now totally playable and acceptable on this laptop ... i am pleased with the settings and quality, so lock this in and close it out". the plan stands; D23Y2 (late warp) is the next step when efficiency work resumes.

### D23Y1 - Game speed first: the governor's last step

**Status: Done (user-accepted 2026-11-13, closeout authorized). Dependencies: D23Y.** The D23H
governor stepped presents down to every 3rd refresh and then paused the CPU
overclock, but the redraws kept running and the game stayed slow (the field
laptop at a fixed 120: 43-56 game frames a second). Acceptance: no second
under 57 game frames after the fallback in UI slots 2, 8 and 9 at a fixed
120 on the laptop; retries bounded; FRAME RATE unchanged; faster machines
unaffected while they keep up.

Work log, 2026-11-13 (Needs playtest), [note 463](../documentation/463-d23y-120-on-two-cores.md) section 4.
Built (`frame_replay.cpp`, `replay_load_window`): still behind at every 3rd
refresh for 3 judged seconds, the redraws stop and the original presentation
runs; retry after 120 s of gameplay, the wait doubling (to 8 minutes) when a
retry falls back within a minute, a retry behind in its first 20 s back at
its first slow second; a FRAME RATE choice cancels a pending retry; log
`[TTK pace] the original presentation (...)` and `redraws again`; developer
`DNTTK_REPLAY_FLOOR=0`, `DNTTK_REPLAY_FLOOR_WAIT`. Measured at 120, 45 s
walks: fallback after 10-11 s, then 58-60 game frames a second (UI slots 2
and 9); a retry cost one second at 24. Remaining: UI slot 8, and the user's
play.

- 2026-11-13 closeout: the user, playing on the field laptop: "it runs so unbelievably well in this latest iteration. it's now totally playable and acceptable on this laptop ... i am pleased with the settings and quality, so lock this in and close it out". the fallback to the original 60 kept as built.

### D23Y2 - Late warp research spike

**Status: Cancelled (user, 2026-11-13: "lets just absolutely can that warp thing. its awful"; code removed; [note 467](../documentation/467-d23y2-late-warp.md) keeps the measurements). Dependencies: D23Y.** For each in-between
present, re-draw the newest finished scene image with the newest camera
look using its host depth, and draw the HUD, first-person weapon and screen
overlays unwarped on top; measure on the field laptop. Acceptance: 120
pictures a second at 2x with CPU as the original 60 (all threads under 75%),
GPU busy under 80%, input to picture under 50 ms (note 462's probe), the
artefacts listed (uncovered edges, transparent layers, moving objects).

Work log, 2026-11-13 (Needs playtest), [note 467](../documentation/467-d23y2-late-warp.md).
Built as an experimental option, off by default (`run.py --late-warp HZ`,
this launch only; `DNTTK_LATE_WARP=HZ`): replay presentation at HZ with no
redraw workers, and at each present the real image's world layer re-projected
on the GPU to the newest look (a rotation about the eye, no depth needed).
The world layer is copied when the GPU display list reaches the first packet
after the world (slot 1's head once near_clip links the weapon; framework
`dma.c` boundary, `gl_late_warp_world_done`), one layer per buffer; pixels
where the final image differs from it (weapon, HUD) are shown as drawn
(`lw_render`); the frame camera comes from the host visibility walk, with the
game's aspect in its rows (focal 256 across, 160 down); the tint and
overlays stay unwarped. Checked: a 3-degree warp of a still view against a
real 3-degree turn: no horizontal shift (1 pixel vertically); 10 degrees: 2
pixels. Measured on the field laptop, three interleaved 30 s walks per slot
(UI slots 2, 8, 9): at 2x, 120 pictures a second (lowest tenth 119-120),
game frames 60 (lowest tenth 59.4-59.6), GPU busy 73-77%, all threads 67-78%
(UI slot 8 over 75%: the presents run on the emulation thread); at the
user's 3x the GPU reaches 87-89% and the game slows. Input to picture
(note 462's probe, UI slot 2): 110-119 ms at the original 60 -> 20 ms with
the warp at 120 (2x), 30 ms at 3x, 33 ms with the warp at 60 (where the
timeline presented only 22-30 pictures a second: a phase bug for D23Y3).
Artefacts: clamped edges on turns, translucent HUD layers keep the old world
behind them, world and enemies still at 30 images a second, a 0.07-degree
pitch offset (1 pixel), no warp without the weapon on screen. Default path
unchanged (still views identical). Remaining: D23Y3 decides the player build
(edge fill, calibration, HUD boundary, presents off the emulation thread,
OPTIONS); the user's judgement.

- 2026-11-13 playtest: the user: "late warp is completely unplayable. it stutters and glitches like absolutely crazy"; FRAME RATE 120 (redraws) "feels like how the real 60fps should actually feel. it's playable, but i think we can get it even smoother". Their warp session at 3x: game 44-46 frames a second (GPU), 1,623 of about 5,600 presents refused (snap back); host quiet. Lab turns with real mouse events: the warp runs 5-20 degrees ahead of the image on fast turns (the game's ~100 ms pipeline), so a band of 15-26% of the width is uncovered; the note and manual corrected (they had called it a few pixels). The spike's verdict: not playable as built; a warp limited to the change since the image appeared, or a wider image, is the D23Y3 question. Status stays experimental, off by default.
- 2026-11-13 after the playtest: the stutter's main cause found and fixed ([note 467](../documentation/467-d23y2-late-warp.md) section 9): the game draws a buffer's next frame while its image is still presented, so the world layer was overwritten on screen and the final image re-copied mid-display; those presents (measured: up to 93% of the picture) fell back to the unwarped picture, jumping 30-35 pixels back on a steady turn. Now three world layers and the final image frozen at its flip: no backward steps in 40 shots of a steady turn. Edge band, 3x GPU and 30-image world remain; needs the user's retest.
- 2026-11-13 retest with the fixed build (user's screen recording, 20:38, and session log): the edge band dominates. In real mouse play the warp ran 10-20 degrees ahead of the image in 1,267 presents, 20-35 in 644 and over 35 in 244 of about 4,700 (lab xdotool turns had shown mostly 5-20), so a third to half of the screen is smeared on every turn; game frames 47-60 at 3x. Conclusion for D23Y3: a rotation warp over this game's ~100 ms pipeline is not usable as the player's picture; only a warp limited to the change since the image appeared (at most about one game image) or a much wider image could be.
- 2026-11-13 user's recording 20:42 (8.6 s, frame-to-frame motion measured): steady turns move 5-25 pixels a frame (658 wide) with smeared bands of a third to half of the screen, and on fast turns the picture swings back and forth by 135-190 pixels (a quarter of the width) between consecutive frames: presents whose lead passed 35 degrees are drawn unwarped, a snap back to the 100 ms old image. The rotation warp is not playable here.
- 2026-11-13 cancelled by the user: "lets just absolutely can that warp thing. its awful. our goal needs to be to make 120fps playable on this laptop". Removed: the framework boundary and warp pass (`dma.c`, `gpu_gl_renderer.c`, `main.cpp`, `mod_plugins.h`, the render-copy names), the title's camera, boundary and provider (`frame_replay.cpp`, `modern_controls.cpp`, `near_clip.cpp`, `host_visibility.inc`), `run.py --late-warp` and the manual entry; the patch re-exported; local tests 258 OK. The work continues as D23Y6.

### D23Y3 - Late warp in the player build

**Status: Cancelled (with D23Y2, user 2026-11-13). Dependencies: D23Y2.** The warp of D23Y2 as the
2-core path above 60: edge fill, transparent layers, moving objects, the
OPTIONS behaviour (FRAME RATE above 60 on 2 cores uses it), Vanilla
unchanged. Acceptance: D23Y's series acceptance on the laptop.

Work log, 2026-11-13 (inputs from D23Y2, not started; [note 467](../documentation/467-d23y2-late-warp.md) sections 5-6).
The spike's warp gives 120 at 2x on the field laptop and about 20 ms input
to picture; for the player build: an edge fill (turns clamp the last
column), the 0.07-degree pitch offset, slot 0's head as the boundary when no
weapon is drawn, presents off the emulation thread (UI slot 8 reached 78% of
all threads at 2x), the 60 timeline that presented only 22-30 pictures a
second at the guest's own rate, an OPTIONS row, and whether 3x should warp at
60 (33 ms, GPU 62-67%) rather than 120.

### D23Y4 - Thread placement on 2 cores

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; placement made no measurable difference, the 15 W package is the wall, note 466). Dependencies: D23Y.** With redraws on a
2-core, 4-thread CPU, keep the emulation thread's physical core free of
redraw workers (affinity), one worker on the other core; A/B against the
default over five runs each (the laptop's clock varies run to run).
Acceptance: redraws at 60 with 60 game frames and the lowest tenth 57 or
more, or a measured reason why not.

Work log, 2026-11-13 (Blocked, measured reason), [note 466](../documentation/466-d23y4-thread-placement.md).
Built: developer `PSX_PLACEMENT=1` (`render_thread.c`, `render_worker.c`):
on Linux with 2 physical cores and SMT, the emulation thread is pinned to
the first core's logical CPUs when the workers start, and each worker process
and render thread pins itself to the other core (checked: CPUs 0-1 against
2-3). Measured redraws at 60 (DISPLAY on the 60 Hz panel with the developer
`PSX_AUTO_CPUS=8` and `DNTTK_REPLAY_FLOOR=0`), the user's 3x Relit settings,
the real display, 30 s walks of UI slots 2, 8 and 9, five interleaved runs
per configuration and slot: lowest tenth of game frames (pooled) default
55.2, placement 55.3, placement with one worker 56.2; seconds under 57
12.4% / 13.6% / 11.7%; per-run lowest tenths 37-59 in every configuration;
all threads 120-126%, clock 2.3-2.5 GHz (3.1 at the original 60). No clear
win, so not the default; none reaches 57. Reason: with redraws the 15 W
package limit lowers the clock and the emulation thread (about 80% of a core
in UI slot 8) falls behind whichever core the workers use; placement does not
reduce the work or the power. Unblocked by less redraw work: D23Y2/D23Y3
(late warp), D23N2B-N4, D23Y5.

- 2026-11-14 (D23Y6, note 468): at SIMPLE with presents at the display's rate
  the clock stays high and placement wins (UI slot 8 emulation thread 81%
  instead of 87%, presents lowest tenth 56 instead of 30; with one worker
  59): it is now the default on 2-core SMT CPUs once workers start, with one
  worker. This job's status is unchanged (its 3x Relit bar is still not met).

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): placement made no measurable difference, the 15 W package is the wall, note 466.

### D23Y5 - A lighter emulation thread

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; no exact cut of the needed size left, note 508; PGO/LTO (D23AH) is what moves the emulation thread now). Dependencies: D23Y; coordinate D23Z.** The
original 60 presentation's emulation thread takes 55-70% of a core at
3.1 GHz on the field laptop; profile it (`PSX_PROF`, `recomp/analysis/d23x/sym.py`)
and cut its largest shares (memory access, dispatch lookups, SPU reverb,
dirty tracking). Acceptance: under 50% at 3.1 GHz in UI slots 2, 8 and 9,
behaviour and pictures unchanged.

Work log, 2026-11-13 (notes from D23Z, [note 465](../documentation/465-d23z-emulation-thread-spikes.md) section 2; not started).
Hotspots on the emulation thread found while profiling D23Z on the field
laptop (the user's 3x Relit settings, original 60): (1) heavy views just after
a load in UI slots 2 and 9 cost 14.5-16.6 ms of CPU a 16.7 ms field for about
1.5 s and make the one slow second left in those walks (52-56 game frames);
against the same walk later they add host world emission (`host_vis_walk` ->
`near_clip_host_world_piece`, draw/emit) +1.7 ms a field, the emulated GPU's
command processing (`dma_gpu_ll_advance` -> `gpu_write_gp0` ->
`gp0_execute_command` -> `glb_draw_shaded_textured_triangle`,
`prepare_precise_triangle`, `tex_ring_put`) +2.7 ms, and in UI slot 9 the
sleeping copies (`sleeping_draw`, the game's actor renderer) +1.4 ms; a GPU
command thread or a cheaper host emission would take most of it. (2) Flat
shares over a whole walk: shared libraries 24% (clock reads 7.7%, now gone
with the pacer change; libc 7%; the Mesa driver 5% on this thread even with
its own driver thread), `psx_write_word_raw` 3.7%, `psx_game_find_entry`
2.4%, `psx_read_word` 2.2%, `psx_write_word` 2.0%,
`dirty_ram_text_native_ok_ranges_from` 1.6%, `spu_render` 1.4%. The
overclock costs CPU in heavy views: without its pause UI slot 9's heavy
second fell to 39 game frames. Tools: `PSX_PROF_FRAMES=1` and the lab's
`stackdiff.py` profile a window of fields.

Work log, 2026-11-14 (Blocked, measured), [note 469](../documentation/469-d23y7-d23y5-d23n2b-field-laptop.md) section 4.
The original 60, UI slot 2, offscreen at a steady 3.1 GHz: the emulation
thread 57.4-58.6% (median, 4 runs; spread about 0.5-1.5 points). Inclusive
shares from call stacks: the emulated GPU's command processing 13.7% (GP0 into
GL batches), host world and model emission about 11%, presentation 6.4%, SPU
3.3%, dispatch lookups 2.1%; exclusive guest memory access about 11%. In
redraw mode it also presents redraws (10.6%), publishes the worker image
(4.7%) and captures the render threads' snapshot (2.0%). Tried, all exact:
the per-store debug-tool write hooks bypassed (57.4% against 56.4%, no gain),
the redraw fence before the swap and an overlay image cache (62.3% against
62.3%; 49.4% against 49.9%): taken out again. The spread shares leave no
single exact change large enough to reach 50%; the structural step, the
emulated GPU's commands and presentation on their own thread, is D23Y8. The
model pass's emission in redraw-only frames (8.5% inclusive in DUKE HILL)
is another candidate, but skipping it changes what the DMA walker reads and
needs its own proof.

Work log, 2026-11-14 (Blocked, re-measured), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 4. The original 60,
offscreen, 3 runs each at 3.09-3.12 GHz: emulation thread median UI slot 2
57.0-58.8%, UI slot 8 63.3-64.5%, UI slot 9 57.0-57.5% (target under 50%).
Unchanged. D23Y8 measured: at most about 9 points could move to a GPU
thread. What remains: guest memory access and the title's host emission
(note 469), and D23N2B-N4 for redraws.

Work log, 2026-11-16 (field laptop, [note 485](../documentation/485-d23y5-d23y8-field-laptop.md)):
fresh profile, UI slot 2: the emulation thread 62% at the player's settings
(FRAME RATE 120, 2770 MHz) and 61% at the original 60 (3130 MHz). At 120 the
redraw tick 12.0%, presentation 11.1%, image publishing 7.2%, the
render-thread snapshot 2.7%; at 60 the GPU command path 14.7%. Still
Blocked.

Work log, 2026-11-16 overnight (laptop lane, [note 490](../documentation/490-d23y5-publish-changed-pages.md)):
the published image is 3.5-5.0 MB (main RAM, scratchpad, the mod memory and
GPU apertures in use). Main RAM is also written outside the CPU's store
path (DMA, the overlay loader, savestate loads), so dirty-page tracking is
not exact by construction and was not built. Built instead behind
developer `PSX_REPLAY_PUBLISH_DIFF=1`: each page compared with the slot's
copy and written only when it differs (byte for byte the save's): 7-10% of
the pages copied, but the publish took 4% less time on the desktop and
15-25% more on the field laptop (UI 2 at 120: 951-953 -> 1,163-1,188 us;
profile 3.3-3.4% -> 4.5-4.6% of the emulation thread). Reverted. Still
Blocked; the next lever is not copying the image at all.

Work log, 2026-11-16 overnight, later (laptop lane, [note 492](../documentation/492-d23y5-osd-upload-cache.md)):
every present uploaded and alpha-scanned each host OSD image (crosshair,
level stats, inventory strip, messages) into one shared texture. Each OSD
image source now keeps its own texture and a copy of its pixels; an image
equal to the copy (byte for byte) is drawn without an upload. On by default
(developer `PSX_OSD_UPLOAD_CACHE=0` off, `=2` reads every hit back):
readback 0 differing in 6,144 hits on the field laptop and on the desktop;
Vanilla shots 6/6 identical. Field laptop, UI 2 at 120, two interleaved
profiles each: `gl_swap_with_osd` 3.4-3.5% -> 1.8-1.9% of the emulation
thread (`gl_draw_osd_image` 1.8-1.9 -> 0.6%). Target still not met.
User, after playing the build at ULTRA 120 on the field laptop: "the game
plays undeniably, phenomenally well ... There is far more at 120 than there
has ever been on the field laptop. lock it in and close it out": the cut
is accepted; D23Y5 stays Blocked on its own target.

Work log, 2026-11-17/18 ([note 506](../documentation/506-d23y-chain-baseline.md), [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)): re-measured on both real screens.
At the original 60 and 3.1 GHz the field laptop's emulation thread is
53-61% (UI 2 53, UI 9 57, FAMILY JEWELS start 56, DUKE HILL town 61),
citadel 46-60%. Built `PSX_STARVATION_COARSE=1` (off): the starvation
ring's stamps and watchdog read the kernel tick clock; citadel -1 to -4
points (its TSC is distrusted, so every clock read is an HPET system call:
6-8% of its emulation thread, a host setting for the user), laptop
unchanged. D23Y8's GL server was the only block of the needed size and
does not shed its queue's cost on these CPUs. What remains is the title's
host world (about 25 points), device emulation (31) and game code with
dispatch (30): Blocked with the numbers in note 507 section 3.3.

Work log, 2026-11-18 (field laptop, [note 508](../documentation/508-d23y5-host-world-caches.md)):
user: "profile the title's host world (object pass, visibility walk, near
clip, emit, object coplanar ranking) per frame on the field laptop at the
original 60 and find exact per-frame caches". Fresh profiles of UI 2, UI 9,
DUKE HILL town and FAMILY JEWELS start (real screen): the host world 11-24%
of the emulation thread, almost all per-face emission that follows the
view (projection, clipping, subdivision, packets); room and piece
selection about 0.1%. No exact cache of size exists: smooth model normals
weld corners by view position, world-mesh parsing would cost its own
check, and a model-space cache of the object coplanar pairs would not
reproduce the view-space thresholds. That search (all planar pairs of a
prop, every frame) was 8.6% in UI 9: it now runs the same rule only on
pairs three exact bounds cannot reject (normal components, plane offset
from a point on the model, in-plane overlap of the corner box), returning
the same pairs in the same order. On by default; developer
`DNTTK_COPLANAR_PRUNE=0` full search, `=2` both with a count. Exact:
173,329 meshes compared on the desktop and laptop, 0 differing;
`PSX_VRAM_DUMP_DIR` dumps identical off against on (desktop UI 2, UI 9,
town, Vanilla; laptop Vanilla; FAMILY JEWELS start and the laptop's
Modernized dumps vary between identical runs either way). Laptop: UI 9
search 8.6 -> 3.6%, emulation thread 57 -> 55%; UI 2, town, FAMILY
JEWELS unchanged (54, 62-63, 56-58%). Target not met; still Blocked. Next
in UI 9: a cheaper pair visit for the 196-face model (about 2 points).
Also seen: modern controls' step probe (`terrain_step_probe`, game
collision code, 1-2%) is gameplay, left alone.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): no exact cut of the needed size left, note 508; PGO/LTO (D23AH) is what moves the emulation thread now.

### D23Y6 - 120 fps playable on the field laptop, SIMPLE at 2x

**Status: Done (user-accepted 2026-11-14, closeout authorized; [note 468](../documentation/468-d23y6-120-on-the-field-laptop.md)). Dependencies: D23Y; coordinate D23Y5, D23N2B-N4, D23P2.**
User, 2026-11-13, after the late warp was cancelled: "our goal needs to be
to make 120fps playable on this laptop. even at simple settings. the simple
profile needs to ideally be 2x, 1x is really bad btw. 2x is the first
'acceptible' look, and it does work on here. 120fps does sort of work, but i
feel its jerkier now than when i playtested it on the gold and guns level a
few hours ago." On the field laptop (note 461) FRAME RATE 120 runs the
late-camera redraws; in the user's sessions the governor settled at every
3rd refresh (41-46 redrawn pictures a second) with seconds of 13-35 game
frames (session logs 13:16 and 20:10-20:48 in `recomp/build-local/logs`).

1. **The regression first.** The user feels 120 is jerkier than at the
   13:16 playtest (before D23Z). Compare those session logs (present
   cadence, governor steps, hitch seconds, game frames) and A/B on the
   laptop at 120 with redraws, same scenes: the D23Z frame-pacer sleep
   (`PSX_PACER_SPIN=1` restores the spin; under full load a sleeping
   emulation thread may wake late, and the pacer also drives the present
   deadlines), the asynchronous neon snapshot (`PSX_GL_ASYNC_SNAPSHOT=0`),
   anything else changed since 13:16; measure present intervals (the replay
   interval histogram, repeats), not only counts. Restore the old behaviour
   where it was the cause (for example the spin only while redraws run).
2. **SIMPLE at 2x.** The SIMPLE grade's resolution becomes 2x (was 1x;
   `player_profiles.GRAPHICS_PRESETS`, the OPTIONS table in `game_menu.cpp`,
   `test_menu_table_matches_the_presets`, the manual); the other rows stay
   the lightest. Check what AUTO picks on the laptop.
3. **120 playable at SIMPLE (2x), then the heavier grades.** Measure the
   redraw path's cost on the laptop and cut it: captured redraws (D23N2B-N4,
   no game code re-run per later job), the lighter emulation thread (D23Y5,
   hotspots in its log), fewer or cheaper redraw jobs per picture, even
   present cadence from the governor (an even 60 or 120 rather than every
   3rd refresh), the 15 W package budget (note 463). Interleaved A/B runs,
   medians and the lowest tenth, the CPU clock noted per run.

Acceptance: at FRAME RATE 120 with SIMPLE (2x) on the field laptop, UI slots
2, 8 and 9 and the Gold and Guns level: 60 game frames with the lowest tenth
57 or more, evenly paced pictures (no every-3rd-refresh spells in normal
play), pictures unchanged where exact; the regression explained and fixed;
the user's judgement that 120 feels smooth; Vanilla unchanged.

Work log, 2026-11-13/14 (Needs playtest), [note 468](../documentation/468-d23y6-120-on-the-field-laptop.md).
Autonomous session on the field laptop (user: "work on D23Y6 autonomously and
completely ... Don't stop to ask me anything"). Lab `recomp/analysis/d23y6/`
(local): 30 s walks on the real display (offscreen ran far lighter than the
window and does not show the compositor's cadence), private settings and
card copies, runs held while the user was active; per second game frames,
presents, CPU clock; every swap time, modelled against the 60.01 Hz panel.
Scenes: Gold and Guns (level 3, savestate slot 0, the 13:16 playtest) and UI
slots 2, 8 and 9.

1. Regression: the 13:16 session played Gold and Guns, tonight's UI slot 2
   (level 6); the logs show more rate changes tonight (every 2nd and 3rd
   refresh, 4-6 a minute), not slower game frames. A/B at 120 with the
   user's settings, 3 interleaved runs per scene: `PSX_PACER_SPIN=1` and
   `PSX_GL_ASYNC_SNAPSHOT=0` within the run-to-run spread both ways (game
   lowest tenth 53.6-56.9 in every configuration); D23Z stays. The cause is
   the scene plus the governor: 120 on a 60 Hz panel draws pictures the
   panel never shows, the game falls behind and presents swing between 120,
   60 and an uneven 40 (UI slot 2: 25 ms gaps 39% of a walk, 5 changes).
2. Built: a fixed FRAME RATE above the display's refresh presents at the
   display's rate (`display_cap_check`, framework
   `psx_mod_display_refresh_hz`; developer `DNTTK_DISPLAY_CAP=0`); the
   governor's last step keeps at least 30 even presents (`last_div`: every
   2nd on 60 Hz); slow seconds there clear after 10 clean ones
   (`floor_good`), so the D23Y1 fallback is reached; the fallback keeps the
   display rate for its retry (`floor_active`); D23Y4 placement on by default
   on 2-core SMT CPUs once workers start, with one redraw worker
   (`PSX_PLACEMENT=0`, `DNTTK_REPLAY_WORKERS` override); the latency query
   sorts once per finished redraw. SIMPLE renders at 2x (presets, OPTIONS
   table, test, manual); GRAPHICS AUTO still picks BALANCED here (probe 6.3
   ms).
3. Measured, final build against the old behaviour on the same build, 2
   interleaved runs per scene (SIMPLE also 3 earlier): SIMPLE at 120 game
   lowest tenth 59.0-59.2 (before 53.9-56.8), presents lowest tenth 59 (before
   40-60), 0 rate changes (before 2.5-4.5), screen fresh 97.7-98.2%; BALANCED
   58.9-59.3, presents 59-60 in three scenes and one even-30 spell a walk in
   UI slot 9 (before 52.9-57.3 with 40-69% uneven 25 ms gaps); PRETTY
   GPU-bound: even 30 or the original presentation, game lowest tenth
   54.4-59.0 (before 31.0-52.9). UI slot 8's emulation thread: 63% at the
   original 60, 88-90% with redraws at the same 3.1 GHz, the extra spread
   over the whole profile (sharing the core with workers); placement 81%.
4. Pictures: Vanilla paused shots 0 differing pixels (new default against
   placement off); Modernized changes only the present schedule, CPUs and
   worker count; paused shots differ only in animated sky, title glow and a
   flame, as much as two runs of one build. Local tests 258 OK.

Remaining: the user's judgement; BALANCED UI slot 9's 30 spell; PRETTY needs
less GPU per picture; the emulation thread's share (D23Y5).

- 2026-11-14 playtest and closeout. User: "it's completely playable. yes of
  course, there are a few slowdowns, but i'm fine ... I'm very happy with the
  performance. there are a few bits where it dips but they dont usually last
  long, such as on duke hill. the best overall performance was on time to
  kill. this job is accepted but i still want to revisit this to squeeze even
  more out of it ... lock it in and close it out". Their session logs
  (00:30-00:56): TIME TO KILL (level 0) about 12 minutes at 120 with 60
  pictures a second (median) and game frames median 59; DUKE HILL (level 1)
  stepped to an even 30 and twice to the original presentation (game frames
  50-58 in its slow seconds). Done; the dips are D23Y7.

### D23Y7 - The remaining dips at 120 (Duke Hill)

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; the 15 W package limit, no single code path, note 494; struggle points go to D23AQ and D23AH). Dependencies: D23Y6; coordinate D23Y5, D23N2B-N4, D23Y8.**
After D23Y6 FRAME RATE 120 on the field laptop holds 60 even pictures a
second in TIME TO KILL and the measured scenes, but DUKE HILL (level 1) steps
to 30 a second and falls back to the original presentation for minutes
(user's session 2026-11-14 00:30-00:56). Measure DUKE HILL's views at 120 with
the D23Y6 lab (`recomp/analysis/d23y6/`, case `cad`; add a DUKE HILL savestate
to the private card copy), profile the emulation thread, worker and GPU, and
cut the cost there (D23Y5's emulation-thread hotspots, fewer or cheaper
redraws, D23N2B-N4 captured redraws). Acceptance: DUKE HILL at 120 with the
user's settings, 60 game frames with the lowest tenth 57 or more and 60 even
pictures in normal play; pictures and behaviour unchanged; Vanilla unchanged;
the user's judgement.

Work log, 2026-11-14 (Blocked, measured), [note 469](../documentation/469-d23y7-d23y5-d23n2b-field-laptop.md).
Autonomous session (user: "work on D23Y7, D23Y5 and D23N2B autonomously and
completely on this field laptop ... without changing pictures or
behaviour"). Host quiet (no backups or updates, no thermal throttling). Lab
`recomp/analysis/d23y6/` extended: DUKE HILL savestates in the private card
copy (private slot 10 a fresh `level 1` start, KILLS 0/50; private slot 4
the copy of UI slot 5), per-thread CPU with the worker, worker statistics.
At the player's settings (2x, LIGHTING on, SIMPLE edges, extended, 120): the
start holds 60 even pictures (game 60.0 / lowest tenth 58.8, presents 60/59,
0 rate changes, 5 runs); the open town from UI slot 5 steps to 30 (game
59.9-60.1 / 56.3-58.4, presents median 31-39, 4.3-5 changes, 7 runs). At a
steady 60 there the CPU clock falls to 2.0 GHz, all threads 223%, the
emulation thread 91-93%, the worker about one core, render thread 30%, GPU
busy 79%; game lowest tenth 51-54. FRAME RATE 60 there: 67% and 3.1 GHz.
Diagnostics: lights off 57.1 (2.3 GHz), 1x 59.1 (3.1 GHz): the GPU's share of
the 15 W package pulls the CPU down. Tried: the D23U depth prepass (worse,
55.7 against 57.7); the snapshot capture's copies (bound +2 to +4.5 points;
an exact incremental capture copied about half and gained nothing
measurable: 53.5 against 53.5); a fence before the swap and an overlay cache
(no change). All taken out; framework unchanged. D23Y6's cap, placement and
one worker against `PSX_PLACEMENT=0` and two workers: the defaults keep the
most 60-picture time (screen fresh 72.5% against 56.0% and 63.4%), kept.
Blocked until a redraw or the emulation thread costs less (D23N2B-N4,
D23Y8), or a lighter picture chosen by the player.

Work log, 2026-11-14 (Blocked, re-measured), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 5. The real display,
the player's settings at 120 and governor, 3 interleaved runs each, with
D41P on: DUKE HILL's start meets the target with SIMPLE (game 59.9 / lowest
tenth 58.6, presents 60 / 59, screen fresh 98.1%, no rate changes) and with
LIVE SHADOWS OFF (59.9 / 58.7, 60 / 59, 97.8%). The town (UI slot 5 copy)
meets the game-frame part (lowest tenth 57.7 SIMPLE, 57.8 OFF) but still
steps to 30 (presents median 42 and 40, 4.0 and 4.7 rate changes a run).
Held at a steady 60 there, OFF gains about 5 points of game lowest tenth
(D41R). D23Y6's cap, placement and one worker kept; the late warp stays
cancelled. User target 2026-11-14: 60 game frames and 120 pictures at 2x on
this laptop, original lighting if need be.

Work log, 2026-11-15 (Blocked, re-measured), [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 4. The real
display while the user was idle, the player's settings at 120 and 2x (each
settings line checked), 3 interleaved runs each, D23V's build against the
same build with its two cuts off. DUKE HILL town: presents median 31 /
lowest tenth 30 both (screen fresh 66.4% new, 69.0% old; game lowest tenth
57.4 / 56.4; 4.7 rate changes). UI 8: 60 / 33 against 60 / 30, fresh 91.5%
against 85.4%, 1.3 against 2.0 changes. UI 9: 60 / 59 against 59 / 30,
fresh 93.0% against 75.2%. Offscreen town profile at 120: worker 98% of a
CPU, emulation thread 78% (old 88%), render thread 39%. The town still
needs captured redraws (D23N4) or a lighter emulation thread (D23Y8).

Work log, 2026-11-16 (three-machine session, laptop lane, [note 494](../documentation/494-d23y7-d23aa-field-laptop-today.md)):
today's build, the field laptop, offscreen, three interleaved runs at 120
with GRAPHICS SIMPLE and ULTRA applied by the lab as `run.py` applies them.
The town at SIMPLE: game 59.9 / 58.8, 120 pictures a second in every run.
At ULTRA: game 50.1 / 41.9, 45 pictures (the redraws mostly off), the
emulation thread 93%, the GPU 90% busy, the CPU 1,224 MHz. Profiled the
worst seconds (frame-tagged emulation-thread stacks): they run at
1.1-1.2 GHz with the same CPU time a frame as the best seconds (16.8 against
15.8 ms); three others are waits at full clock. The dominant cost is the
clock the 15 W package leaves the CPU while the 4x GPU runs; Blocked: less
emulation-thread work (D23Y5, D23Y8) or less GPU power a frame would help.
Real-display look at ULTRA and PRETTY in the town queued for the user.

Re-measured, 2026-11-17/18 ([note 506](../documentation/506-d23y-chain-baseline.md), [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)): pure AUTO holds 60 / 60 in the town on both machines; ULTRA 120 on the field laptop 41.8-44.3 (lowest tenth 22.7-24.6) in every configuration, the 4x GPU 84-89% busy: still the package. Still Blocked.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): the 15 W package limit, no single code path, note 494; struggle points go to D23AQ and D23AH.

### D23Y8 - Emulated GPU commands on their own thread

**Status: Cancelled (2026-11-18, retired after measurement; [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)). Dependencies: D23Y5; coordinate D23Y7, D23N4.**
At the original 60 the emulation thread spends 13.7% of its samples in the
emulated GPU's command processing (DMA linked lists, GP0 into the GL
renderer's batches) and 6.4% presenting, in UI slot 2 on the field laptop.
Move that work to a GPU thread fed by a command queue (as other emulators'
threaded renderers do), with the emulation thread waiting only where the
game reads back (VRAM reads, GPUSTAT that depends on drawing), savestates,
redraw snapshots and the render threads synchronised. Acceptance: the
emulation thread under 50% of a core at 3.1 GHz in UI slots 2, 8 and 9 at
the original 60; pictures identical (frame checks), behaviour unchanged
(guest timing as before), Vanilla unchanged.

Work log, 2026-11-14 (Blocked, measured), [note 470](../documentation/470-d41r-d41p-d23y8-field-laptop.md) section 3. A read-only map of
the GPU path (CODE): the emulation thread owns the GL context; GP0
processing feeds guest-visible state (VRAM reads come from GL through
`ensure_cpu`, the widescreen classifier reads guest RAM) and the title sets
renderer state in GP0 order, so only the GL backend calls and presentation
can leave the thread, behind drain barriers at every readback. Measured from
note 469's 21,495 call stacks of UI slot 2 at the original 60
(`d23y8_split.py`): GL backend work 7.2%, presentation's display half 1.9%,
GP0 decode (stays) 4.8%. At most about 9 points could move before the
queue's own cost: UI slot 2 near 48-50% at best, UI slot 8 (63-64%) near
55%, so D23Y5's target is not reachable by this job alone; not built (the
exact queue touches the renderer's three copies, render threads, redraw
sessions and savestates). Resumable plan in the note: presentation on a
shared-context thread first, then a `gr_*` / `gl_set_*` proxy queue.

Work log, 2026-11-16 (field laptop, [note 485](../documentation/485-d23y5-d23y8-field-laptop.md)):
step 1 sized, not built: at the player's 120 the present and its OSD are
3.8% of the emulation thread (about 4 points movable), so UI slot 2 would
go from 62% to about 58%. What building it takes (CODE): its own GL path or a
third renderer copy (the state cache and VAOs are per context), the OSD
turned into data on the emulation thread, `present_shot` moved with it, the
other present paths drained first.


Work log, 2026-11-16 overnight (laptop lane, [note 492](../documentation/492-d23y5-osd-upload-cache.md)):
step 1 not built. Most of what it would move (the present's OSD, 2.1% of
the emulation thread at 120) was redundant uploads, now cut in place under
D23Y5 (about 1.6 points, exact); about 2 points of present work are left
for a presentation thread, whose pacing can only be judged on the real
display. Still Blocked (measured).

Work log, 2026-11-17/18 (field laptop and citadel real screens, desktop
offscreen for exactness, [note 506](../documentation/506-d23y-chain-baseline.md), [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)): built. `PSX_GL_SERVER=1` (developer,
off) creates the renderer's context on its own thread; 147 renderer entry
points, the backend's vtable among them, and 10 render-thread entry points
are wrapped by `runtime/src/gl_server_gen.py` (synchronous by default:
exact by construction; primitives, draw state and the per-triangle host
state queued, their outside reads audited and snapshotted; `gpu.c` waits
for an idle GL thread before writing VRAM directly). `gpu.c` sets a host
triangle's state in one call (`gl_set_host_triangle`, exact). Exactness
(`PSX_VRAM_DUMP_DIR`, frame-exact VRAM and pictures): identical on the
desktop (Vanilla and Modernized) and in Vanilla on both real screens;
Modernized VRAM identical on the field laptop; **on citadel Modernized
with LIGHTING on differs from +200 frames** (two-CPU and synchronous-neon
controls identical, LIGHTING off identical; cause in the dynamic-light
path not found). Cost: the original 60, field laptop +3 to +6 points
(also with the GL thread pinned to the other core), citadel -1 to +1;
pure AUTO -1 to -4; frame rates unchanged. The driver's own thread
(`PSX_GL_THREAD=1`, Mesa glthread) exact, no gain. Two measuring defects
fixed (statistics' clock reads on HPET; the lab's per-entry present-ring
queries). Blocked: not exact on citadel and no net gain on the 2-core
laptop; a coarser GPU thread (whole GP0 packets) would be the next design.

Cancelled, 2026-11-18. User: "i continue with your recommendation to
retire it". The GL server and the driver-thread switch are removed from
the code (the generated wrappers, `gl_server*.inc`, `gl_server_gen.py`,
`gpu.c`'s quiesce calls and the merged host-triangle call); the build is
byte-identical in play to before the work (frame-exact VRAM and pictures,
Vanilla and Modernized, desktop). Kept: the developer exactness dump
`PSX_VRAM_DUMP_DIR`. Why (note 507 section 2): on a 2-core laptop the
queue's cost exceeds the drawing it moves, radeonsi already threads its
driver work, frame rates are GPU- or package-bound where they are not
already 60, and Modernized was not exact on citadel. A future attempt
would need a coarser unit (whole GP0 packets or frames), not backend calls.

### D23Y9 - Resolution headroom at 120

**Status: Cancelled (2026-11-14, user: "i dont want to degrade the game that way ... revert it first, and then scratch it off the board"). Dependencies: D23Y7; coordinate D23P2.**
Measure DUKE HILL's town (DH5, private slot 4) and start (DH, private slot
10) and UI slots 2, 8 and 9 at the player's settings at FRAME RATE 120 on
the real display at 1x against 2x (and 1.5x if the renderer allows it),
interleaved, with game frames, present cadence, GPU busy and clock and CPU
clock per run, to find whether the town is GPU-bound or CPU-bound at 120 on
the field laptop. If a lower resolution holds the pictures, design and build
a dynamic internal resolution that drops below 2x only while a redraw misses
its budget and returns when it fits, as a setting with 2x fixed the default
until the user's playtest; matched before/after pictures and a manual entry.
If not, record the measurement. Acceptance: the measurement; if built, the
setting off by default, pictures at 2x unchanged, Vanilla unchanged.

Work log, 2026-11-14 (measured, built, then Cancelled and removed), [note 474](../documentation/474-d23y9-d23n2b-field-laptop.md) sections 1-5.
Real display, the player's settings at 120, 3 interleaved runs per scale:
at 1x every scene holds 60 even pictures (screen fresh 95.7-98.3%, no rate
changes); at 2x the town (presents median 31, lowest tenth 30, 2.7 changes a
run), UI 8 (lowest tenth 31) and UI 9 (one run at 30) step down, every step
"emulation behind", GPU busy only 69-76% but its clock 130-250 MHz higher and
the CPU's lower (UI 2 3.12 -> 2.30 GHz): power-bound, the GPU's share of the
15 W package. 1.5x does not exist (the GL surfaces are VRAM at a whole
multiple). Built DYNAMIC RESOLUTION (framework live rescale between presents,
render threads following the snapshot's scale, a governor step before the
present-rate step, OPTIONS row, schema 76): pictures exact (the 1x stretch
equals a 1x launch, the return equals 2x, 0.00% at the original 60); on
against off at 120: the town held 60 (screen fresh 97.6-97.7% against
76.1-76.5%, no rate changes) but it also dropped to 1x for 57-96% of walks
that hold 60 at 2x. User 2026-11-14: "i just cant accept this dynamic
resolution thing as a solution when quake alkaline plays at the full 120fps
at native resolution ... i dont want to degrade the game that way, and for
that reason, i request we revert it first, and then scratch it off the
board". Removed completely (framework patch identical to before, the menu
row, flag, console command and manual entry gone); the profile schema stays
76 and drops the retired key. The measurement stands: the town's cure is
less CPU and package power per redraw (D23N2B-N4, D23V).

Work log, 2026-11-16 later (three-machine session, [note 493](../documentation/493-d23n5-shared-frame-work-desktop.md) section 6b):
desktop, both D23N5 and D23Y10 on with both checks, 120 s in five views:
8,815 restored jobs, 0 differing. Field laptop (one worker), SIMPLE: the
check 2,814 jobs, 0 differing; the worker at 120 up to 5 points lower (UI 9
71 to 66, the FAMILY JEWELS start 72 to 68), unchanged at 180 where it is
saturated. Citadel: the check 672 jobs (3 workers), 0 differing; with one
worker 1-8 points lower (UI 9 70 to 62, UI 2 80 to 72), with AUTO's three
within the spread. Needs playtest: default on is the user's decision.

### D23Y10 - Publish the worker image without a copy

**Status: Needs playtest (2026-11-16, [note 495](../documentation/495-d23y10-tracked-publish.md): built off by default, exact, measured on all three machines; default on is the user's decision). Dependencies: D23Y5.**

D23Y5 (notes 490, 492): the emulation thread copies its image for the redraw
workers every published frame. Replace the copy with a copy-on-write fork or
a double-buffered image the emulation thread writes in place, exact by
construction. Acceptance: exact (validator, Vanilla shots), the emulation
thread measured on the field laptop at 120 against today's publish.

Work log, 2026-11-16 (three-machine session, desktop lane, [note 495](../documentation/495-d23y10-tracked-publish.md)):
of the three ways (a copy-on-write fork a publish, a double-buffered image
written in place, or copying only what changed with the kernel saying what
changed), the third: the smallest change and exact by construction. Built
off by default, `PSX_PUBLISH_TRACK=1`: the emulation process registers its
RAM, mod memory and DMA window for asynchronous userfaultfd write
protection at its first publish; each publish scans the pages written since
the previous one into every slot's pending set; the published slot gets
only its pending pages and any range an extent grew over; a slot's first
publish and any failure save whole. Developer check
`PSX_PUBLISH_TRACK_VERIFY=N` compares a tracked publish with a whole save
of the same moment. Desktop: a tracked publish 61-104 us (62-134 pages,
49-85 written between publishes) against 656 us whole; the emulation
process 1-3 points of a core lower in all four views, every run below its
pair; 8,128 tracked publishes checked, 0 differing; validator 908 of 908;
Vanilla shots 6 of 6. Next: the field laptop at 120 (the acceptance).

Work log, 2026-11-16 later (three-machine session, [note 495](../documentation/495-d23y10-tracked-publish.md) sections 5-5b):
field laptop at SIMPLE 120: 2,032 tracked publishes checked, 0 differing,
138-148 us a publish; the emulation thread 0-3 points lower (UI 2 59 to 58,
the FAMILY JEWELS start 63 to 60), pictures 120 either way; at the
player's 4x the redraws mostly stay off, so little is published. Citadel at
AUTO 120: 3,040 checked, 0 differing; the emulation thread 1-3 points
lower in all five scenes. Desktop, both D23N5 and D23Y10 on: the emulation
process 1.1-3.6 points lower in every pair; 17,880 more tracked publishes
checked in 120 s runs, 0 differing. Needs playtest: default on is the
user's decision.

Data, 2026-11-17/18 ([note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md) section 5): 0-6 points lower on the field laptop, 1-5 on citadel; its write tracking costs 1,800-2,250 page faults a second on the emulation thread (90-190 without), and one laptop run at FAMILY JEWELS fell back to the original presentation (lowest tenth 55.7). The default stays the user's decision.

Work log, 2026-11-23 (found during D23AH): this switch has been on by default in Modernized since the D23AK closeout (2026-11-18, `run.py` MODERNIZED_REDRAW_DEFAULTS); the "built off by default" wording above predates that. The player's field-laptop sessions of 2026-11-23 ran with it on.

### D23Z - One-off CPU spikes on the emulation thread

**Status: Done (user-accepted 2026-11-19: "this work is accepted ... lock it in and close it out"; [note 465](../documentation/465-d23z-emulation-thread-spikes.md); found in D23W 2026-11-13). Dependencies: D23W.** On
the field laptop the remaining short slowdowns at 60 (one or two seconds of
45-57 game frames in a 30 s walk, each followed by a 5 s overclock pause)
lined up with one-off events on the emulation thread: the level-start lamp
snapshot (24 ms on the game thread), sleeping-enemy handovers as Duke moves
between rooms, and Relit room brightness changes; continuous firing and a walk
with lights off showed none (note 461 section 6). Measure each with the host
profiler over the event, move the work off the emulation thread or spread it
over frames, and decide whether the overclock pause should wait for a second
behind second on weak CPUs. Acceptance: no "behind" second in 30 s walks of UI
slots 2, 8 and 9 on the laptop with the user's settings, pictures and
behaviour unchanged.

Work log, 2026-11-13 (Needs playtest), [note 465](../documentation/465-d23z-emulation-thread-spikes.md).
Autonomous session on the field laptop (user: "work on D23Z, D23Y4 and D23Y2
autonomously and completely"). Lab `recomp/analysis/d23z/` (local): 30 s
walks of UI slots 2, 8 and 9 at the user's 3x Relit settings on the real
display, interleaved A/B runs, slow seconds counted from the game's own
seconds. New developer tools: `DNTTK_SPIKE_TRACE=MS` (slow updates with their
hooks) and `PSX_PROF_FRAMES=1` (profiler stacks tagged by field). Measured:
the neon-sign snapshot took 13.3-15.2 ms on the game thread (a synchronous
GPU readback at 3x); the frame pacer's last-2 ms spin was 7.7% of the
emulation thread's samples; sleeping-enemy handovers and Relit room changes
were not spikes; the remaining slow second in UI slots 2 and 9 is the heaviest
view of the walk just after the load (14.5-16.6 ms of CPU a field for about
1.5 s: host world emission, GPU command processing, in UI slot 9 the sleeping
copies), not an event. Built: the snapshot reads VRAM back asynchronously
(pixel-pack buffer and fence, `gpu_vram_copy_async`; 3.7-5.1 ms; developer
`PSX_GL_ASYNC_SNAPSHOT=0`); the pacer sleeps to 0.3 ms before its deadline on
Linux (developer `PSX_PACER_SPIN=1`). Decided: the overclock pause stays at
the first slow second (option `DNTTK_OC_PAUSE_AFTER`, measured: waiting for a
second made UI slot 8 slow in every run and UI slot 9's worst second 39
frames instead of 52-53). After, 6 runs each: slow seconds per run UI slot 2
2/1/1/1/0/0 (before 1/1/1), UI slot 8 0/1/0/0/0/0 (before 0/1/1), UI slot 9
one every run (worst 52.4; before 52.8); emulation thread 71% -> 66% (UI
slot 8), all threads -1 to -5 points; game frames median 60, lowest tenth
59.2-59.7. Pictures: the old paths against the new, still views: 0
differing pixels (the pacer alone: 66, the LEVEL STATS clock). Remaining:
the heavy views are D23Y5's (work log there); the user's play.


Work log, 2026-11-19 (Needs playtest), [note 465](../documentation/465-d23z-emulation-thread-spikes.md)
section 7. Re-measured on the field laptop after its checkout was brought to
main (e95c5fa, after D23AO; this job's code unchanged) at the player's current
settings (BALANCED 2x, FRAME RATE 60, first person): 30 s walks, real display,
3 runs a slot, host idle. Slow seconds in the walk window: UI slot 2 0/1/0,
UI slot 8 2/2/1 (worst 45.3), UI slot 9 0/2/0 (the first seconds after the
load, worst 40.8); pictures 60 throughout, no "emulation behind" line. Per
thread, UI slot 8's emulation thread holds 87-90% for about 15 s near Duke
-282 5630 21198 (redraw worker 25-50%). `DNTTK_SPIKE_TRACE`: after the load no
update dominated by a hook (UI slot 8: median 30.8 ms of a 33 ms budget, 4
updates of 46-56 ms whose largest hook is at most 6.4 ms; UI slots 2 and 9:
largest 38.0 and 47.5 ms). No one-off event is left; the remaining slow
seconds are sustained heavy views, written to D23AQ. No code changed.
Remaining: D23AQ (the heavy views).

2026-11-19, user: "this work is accepted ... lock it in and close it out".
Done: no one-off spike left; the walks' remaining slow seconds are D23AQ's.
### D23R - Parity benchmark and acceptance

**Status: Todo (D23K plan, last; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23M2, D23M3, D23P2, D23U.**
The programme's acceptance: re-run the note 444 comparison (EDuke32 Polymost
and Polymer against our game, same display, windowed and fullscreen, the
same views, CPU and the 4-CPU test) and check the parity bar in section 5:
0.4 ms GPU or less a presented frame at 180 Hz, 12% or less windowed, 1.5
cores or less, 180 Hz on 4 logical CPUs, under 4 ms a redraw on the weak GPU
estimate; pictures identical, feel unchanged or better, Vanilla unchanged.

Interim reading, 2026-11-11 (headless, offscreen, after D23O part 2, D41M
and D23M; the EDuke32 side and windowed figures need the real display with
the user present). Against the bar of note 444 section 5: CPU at 180 Hz in
UI slot 2 about 2.3 cores (2.6 in D23K; bar 1.5, the workers are 1.2 of it:
D23N); 180 Hz with 4 logical CPUs held in UI slot 2 (bar met there), not
in the level 3 corridor (about 110; AUTO now caps at 120 with 4-7 CPUs,
D23P); GPU per redraw on the timeline 0.81-0.83 ms in UI slots 2 and 8
(1.0 before D23M; bar 0.4). Still Todo: D23L-D23P are not done.


Interim reading, 2026-11-12 (real display, the user present for D23L;
[note 453](../documentation/453-d23l-compositor.md)). Same display, turning
at 180 Hz, the player's profile; EDuke32 in game (E1L1, E1L2), not demos.
Whole-GPU: ours windowed 28.7-31.4% (bar 12%), fullscreen 23.7-26.8%, that
is about 1.3-1.5 ms of GPU a presented frame (bar 0.4); EDuke32 windowed
8.2-12.5%, fullscreen 3.7-6.2% (0.2-0.35 ms). CPU 2.4-2.6 cores (bar 1.5;
the workers 1.4-1.5 of it, D23N). The compositor is equal for both games
(D23L). Still Todo: D23N (CPU) and the GPU per redraw are the open gaps.

Work log, 2026-11-16 ([note 486](../documentation/486-d23v3-d23m3b-d23r-measurements.md) section 4):
of its dependencies only D23N4 is open (N4b off: not exact, not cheaper);
the GPU per redraw needs D23M3B; it still needs the real display with the
user for the EDuke32 and windowed figures, and one repeatable lab case.

### D37 - Temporary movement failure at 07:26 UK, 2026-10-26

**Status: Done. Research accepted and closed by user instruction; repair selected as D37A. [Note 254](documentation/254-d37-movement-failure.md).**
No prerequisite on D36. Prior control recovery and performance notes are references,
not evidence of a shared cause. This is not a confirmed first-person rendering bug.

User report: at approximately 07:26 local UK time on 26 October 2026, camera turning
remained possible but movement became stuttered/broken, Duke's animation appeared
stationary and normal gameplay was impossible. Closing/reopening restored normal
behavior; reloading save slot 9 then worked. Preserve this time and sequence when
correlating logs; verify timestamp/timezone conventions rather than assuming them.

When selected, inspect available session logs and relevant system events around the
reported time. Check backups/Timeshift/rsync, updates, CPU load, disk I/O pressure,
memory/swap and historical host activity as well as game telemetry. Later idle
readings cannot establish host conditions during the incident, and background
activity alone does not establish causation. If historical logs are unavailable or
insufficient, say so explicitly. Do not substitute fresh successful play for an
explanation of the earlier failure.

Trace relevant animation/movement/input ownership, first-person overrides and
other candidate subsystems only as supported by evidence. Record log timestamps,
code paths, reproducibility and competing hypotheses. Do not invent a cause or
assume the visual defects are connected. Any reproduction must use isolated test
settings and private save/card copies, preserving the accepted build and player
files. No fixes are authorized by this research job.

**Deliverable and acceptance:** a separate numbered research report using the next
available documentation number at execution time, with chronology, available and
missing evidence, source references, confirmed findings versus hypotheses,
reproduction outcome/limits and recommended follow-up tests or a scoped fix job.
An unresolved or unreproduced result is valid if stated honestly; report for user
review before changing status to Done. Correlate with D36 only if supported.

### D37A - Safe animation restoration across save allocation layouts

**Status: Done (user playtested and fully accepted 2026-10-26); [note 255](documentation/255-d37a-animation-save-restore.md). User selected repair and closed D37 research on 2026-10-26.**

Repair the new-game -> saved-game allocation mismatch that clears animation data
while the guest still references it. Recover verified animation metadata from the
loaded save without restoring incompatible GPU packet arenas. Support existing saves,
repeat loads and both allocation orders. Preserve Vanilla and current GPU safety.

Acceptance: focused native coverage for mismatched/absent/malformed saved metadata;
private live reproduction with movement and animation progressing after warm loads;
repeat and cross-level loads, first/third person, crouch and quick-kick checks;
Vanilla regression. Export and validate framework patch against the pinned dependency.
No player saves/cards modified. Present implementation for playtest/review; no automatic
commit/push or repair closeout.

### D50 - Frame rate halves after loading (audio running low at load)

**Status: Accepted, closeout not yet authorized (user, 2026-11-08: "it is now fixed!!!!"; load-time shed fixed; the energy-weapon stalls traced to native-widescreen mirror framebuffer switches on the render thread and fixed with a deferred mirror, images verified exact; [note 415](documentation/415-d50-load-audio-shed.md)). Dependencies: none.**
User: slowdown "with the energy weapon in that area" (UI slot 8), and
"maybe a touch of audio crackle". Session logs: in four of five sessions on
2026-11-08 the governor logged "audio running low" (38-59 of 180 ms) a few
seconds after the savestate load and shed to every 2nd refresh (90 presents
at 180 Hz); it steps back up only after 30 clean gameplay seconds, and a
pause menu holds it there (11:26 session: about 2.5 minutes at 90). Not
reproduced offscreen with the dummy audio driver. Find what stalls the game
after a load long enough to drain the audio ring, fix it, and make a
load-time shed recover quickly. Acceptance: loads keep 180 presents/s
without a shed (logs), no crackle, user confirms in slot 8.

### D50A - Load and level start empty the audio ring again

**Status: Accepted, closeout not yet authorized (user playtest 2026-11-11; [note 445](../documentation/445-d50a-loads-keep-the-audio-ring.md)). Dependencies: D50.**
D50 (accepted 2026-11-08) fixed the load-time shed. The 2026-11-11 session
log (real display, 180 Hz) shows it again: after savestate loads (UI slots 1
and 2, an old save and a newly made one alike), after a new game's level 0
start and after a save, the governor logged "audio running low" with the
audio ring at 0-40 of 180 ms, and up to about 1,700 underruns in one
interval; presents fell to about 90/s for 20-30 s while the ring refilled at
60-65 game frames/s. One load (UI slot 0, level 3) kept the ring full, so
not every load does it; compare the two. The redraw workers also fall behind
for the first seconds after a load (job age up to 40 ms, up to 55 failed
redraws). Separately, the neon sign pass (`[TTK lamp] level N: ... neon sign
faces take no dynamic light`) stalls the game thread once per level start:
20 ms on level 6, 36 ms on level 0 and 70 ms on level 3 (165 lamps).
User: did not notice the first one; the game did shed. Find what empties the
ring (the load itself, the restart of audio, or a game-thread stall), keep
it full across loads, saves and level starts, and move the neon pass off the
game thread or spread it over frames. Acceptance: loads, saves and level
starts keep 180 presents/s without a shed (logs), no underruns, no level
start stall over one frame, Vanilla unchanged.

Work log, 2026-11-11 (Needs playtest). Root causes (note 445): the audio
bridge waited for its target fill only once at startup, so after anything
that took audio out of the ring (a savestate load's stall, a turbo-load
mute that drains it by design, an 85 ms save on the emulation thread) play
resumed short, refilled by the 0.5% trim at about 5 ms a second; and the
neon pass scanned meshes and VRAM through the emulator on the game thread.
Changes: the bridge re-buffers to its target after running dry, on a
savestate load (dropping the old audio, as the legacy path clears its
queue) and after a turbo mute; saves are copied on the emulation thread
(4-7 ms) and compressed and written by a writer thread, byte-identical to
the direct save (`PSX_SAVE_VERIFY=1`); the slot sidecar is taken at the
copy; a purpose-built neon scanner reads a RAM/VRAM copy on its own thread
(game thread 1.3-3.0 ms, was 20-69 ms; face sets identical on levels 0, 2,
3, 6). Lab (offscreen, private card copy, real audio into a null sink),
loads at 120/180/240 Hz: the ring's lowest point after 2 s rose from
79-105 ms to 119-146 ms (means 119-136 to 160-170 ms) and repeated loads no
longer stack. The lab's level travel did not reproduce the player's level
0 start drain. Found on the way: D50B (a save can end the game; a load
after a save can stall it; both predate this change) and D50C (the level 3
pit). Needs the user's playtest: loads, saves and level starts at 180 Hz
with real audio, session log without `audio running low` sheds.

User playtest, 2026-11-11 (session 19:00): no crackle after loads (slots 1
and 2), F6/F9 saves and loads fine with no hitch or exit, a new game
started without crackle. Accepted.

Closed out, 2026-11-17: the user authorized the closeout ("D50A can just be
closed off"). The implementation was already committed with the later
performance work; this closeout records the status only. Done.

### D50D - Quick save reports could not be saved while it saves

**Status: Done (user-accepted 2026-11-12, closeout authorized; fixed 2026-11-11). Dependencies: D50A.**
User (playtest 2026-11-11, UI slots 6 and 2): "the game could not be saved
... but it does look like it actually did save". Session log: `[TTK cheat]
THE GAME COULD NOT BE SAVED` right after the save's admission, then
`save_write ... outcome=success` about 90 ms later. Cause: since D50A the
file is written on a writer thread; `savestate_pending()` turns false at
capture, and the F6 notice (and SAVE GAME's result) read the success flag
before the write was reported, so it said failure whenever the menu polled
inside those 90 ms.

Fix: framework `savestate_busy()` (staged, or a write still in flight);
the quick-save notice, SAVE GAME's result, the one-at-a-time guard and
delete wait for it. `savestate_pending()` keeps its meaning for the
framework's own polling. Lab (private card copy, UI slot 6 save): 12 F6
saves in a row, 12 GAME SAVED, each after its successful write.

Accepted, 2026-11-12. User after playing: "it's working! 'saved to slot 07' every time. solid". Closeout authorized.

### D50C - Walking into the level 3 pit halts the emulation

**Status: Done (user-accepted 2026-11-12, closeout authorized; [note 451](../documentation/451-d50b-d50c-saves-and-the-pit.md)). Dependencies: none.**
Found in the D50A/D41M lab (offscreen, private copy of UI slot 9, the level
3 corridor, Duke walking forward with nothing else pressed for about 25 s):
Duke reaches the end of the corridor, drops about 7000 units (to 24065,
6665, -17157) and the emulation halts with `FATAL: GPU GP0 unknown command
0xFE` (or 0xF9), or the emulation thread spins at 100% with 56 presents a
second. Seen with the unmodified build and with the D50A and D41M builds,
in 3 of 3 long walks. Not reported by the player. Work: reproduce, find
where the GP0 stream goes wrong (the GPU command ring at the halt), and
whether the original game falls there (death pit, cutscene, out of
bounds). Acceptance: the same walk ends in the game's own outcome (death
or landing) without a halt, Vanilla unchanged.

Observation, 2026-11-11 (D23M lab, not investigated further): a corridor
walk of about 13 s from UI slot 9 (offscreen, 180 Hz) dropped Duke from y
683 to 6264 within two seconds; game frames fell to 5 a second, then the
4 s starvation watchdog stopped the run (no heartbeat 4.7 s; the last PC
sample 0x800B6E44, no exception pending, I_MASK 0x6D). Log kept locally as
`recomp/analysis/perf/d50c-repro-1.log`.

Work log, 2026-11-12 (Needs playtest). Duke dies in the pit (flags 0x802,
health 0); the game's own outcome is its death view and PRESS E TO OPEN
MENU. The halts came from the GPU stream: with Duke dead outside the level
the game builds broken packets (a node with a corrupt word count sent link
headers to the GPU as data; a garbage link 0xA805E5), and the framework
halted on GP0 words the console ignores. The same family halted a
third-person load of UI slot 9 (GP0 0xFE). Framework fixes: unknown GP0
commands are one-word NOPs and GP0(1Fh) requests IRQ1, as on the console
(logged); the DMA list walker ends a list at a bit-23 link that is not
allocated mod packet memory instead of folding it into RAM (new walker test
case). Lab (offscreen, private card copy): 4 pit walks (180 Hz and 60 Hz,
first and third person) end in the death prompt with no halt; 10
third-person loads of UI slot 9, none halted (was the first load). Vanilla:
same framework path, only for words the console ignores. Remaining: after
the death the game runs at 53-60 of 60 fields a second; the user's look.

User playtest, 2026-11-12: "the game no longer crashes at that point ... testing is solid". Accepted; closeout authorized.

### D50B - A save can end the game (PC=0)

**Status: Done (user-accepted 2026-11-12, closeout authorized; [note 451](../documentation/451-d50b-d50c-saves-and-the-pit.md)). Dependencies: none.**
Found in the D50A lab (offscreen, private card copies, saves through the
debug port `savestate op=save` in the level 3 corridor of UI slot 9 a few
seconds after a load): in 4 of about 16 launches the game ended with
`psxrecomp runtime: execution completed, PC=0x00000000`, either before the
state was written or right after `SAVED`, and once the guest stopped
(`[FPS] ... (0.00x)`) after the next load. The unmodified build did it too
(2 of 7 launches), so it predates D50A. Every failing save was admitted
with `save_admission ... outcome=irq_return_to_flat_scheduler
resolved=0x800293FC` after `deferring ... [waiting_flat_scheduler]`; saves
admitted directly never failed. Not reported by the player so far (quick
save is F6; SAVE GAME saves from the pause screen). Work: reproduce with a
loop of debug saves, trace the unwind from the IRQ return to the flat
scheduler (savestate.c, savestate_admission) and what leaves the
continuation at PC 0. Acceptance: 50 consecutive gameplay saves (debug port
and F6) without an exit or stall, the saved files load, Vanilla unchanged.

Work log, 2026-11-11. Two symptoms, two causes:

1. Exit at PC 0 after a save admitted at the IRQ return
   (`irq_return_to_flat_scheduler`, the failing saves resolved 0x800293FC).
   `savestate_poll_irq_return` longjmps to the scheduler without the limits
   the active admission keeps (outermost dispatch only, no overlay call unit
   open), so a save taken inside nested compiled frames abandoned their guest
   continuations. Fixed (framework, `savestate.c`): such a save now waits
   for the next boundary (logged once as `irq_return_nested_wait`). Not seen
   again in 150 save-and-load rounds (UI slot 9, offscreen); the guard did
   not fire in them either (the failing context did not recur), so the fix
   is by construction, not yet by a reproduction.
2. The guest stops after a load that follows a save (host presents, game
   `(0.00x)`, "fresh-frame wait timed out"). Reproduced on the fixed and
   the unmodified build (load, 4 s, save slot 11, 2.5 s, load: stalls after
   2-11 rounds); 40 loads with no saves never stall. Every stall followed a
   save admitted by unwinding to the scheduler (`unwind_to_flat_scheduler`,
   e.g. 0x8001CA6C, 0x80076EDC). State at the stall (debug port
   `get_registers`): pc 0, EPC 0x800B4544 (the loaded state's resume PC),
   I_STAT 0x21 pending with IEc set, VBlanks raised but no longer delivered.
   So the first interrupt taken at the load's resume point returned to
   PC 0, the "continue the live chain" value, although after a load there is
   no live chain; something the unwinding save leaves behind (a latch of
   `psx_scheduler_snapshot_at` or the scheduler-top serialization) changes
   that first delivery. Next: trace that delivery (`interrupts.c`,
   same-thread resume and the top-level resume flag) after an unwinding
   save. Lab: `recomp/analysis/perf/lab.py` cases `saveloop`, `d50b`,
   `loadloop` (local).

Work log, 2026-11-12 (Needs playtest). The old loop (load the player's state,
save, load the player's state again) no longer failed (185 rounds, 60 of them
with every save forced through the scheduler unwind). The acceptance loop
(save through the port or F6, load that save back through the port or F9)
failed within 2 to 25 rounds at 60 and 180 Hz: PC 0 after the load, or the
guest stopped, often after a 1 s fresh-frame hold. Vanilla: 50 rounds clean.
The failing states held a zeroed live frame (the camera render's return
address 0 at 0x801fff48). Cause: 13 of our host-to-guest calls (host and pole
visibility, crouch hull, kicks, cheats, gadget HUD, shortcuts, sleeping
enemies, steroids, damage flash) ran the game on a copied CPU state with only
our own `guest_busy` flag; a load admitted inside one applied the loaded
registers to that copy and resumed the game with the real CPU's stale ones
against the loaded RAM. Fix: those calls now declare themselves to the
framework (`psx_mod_host_call_begin/end`), as the aim trace and menu text
already did, so a load or save waits for the next poll. Lab after the fix,
offscreen, private card copies: 175 save-and-load-back rounds (60 port at
60 Hz; 50 at 180 Hz, 25 F6/F9 and 25 port; 30 third-person; 35 more at
180 Hz), no exit, stall or hold; one round of the first 180 Hz run ran at
8-13 fields a second after the load during the tail of a Timeshift backup
(08:00-08:26), not reproduced by the same state fresh or by the 50-round
rerun after the backup (suspected host contention, not established).
Vanilla: 25 rounds clean. Remaining: the user's playtest of F6/F9 and SAVE
GAME / LOAD GAME at 180 Hz.

User playtest, 2026-11-12: "the game no longer crashes at that point ... testing is solid". Accepted; closeout authorized.

### D50E - Exploding barrels crash the game (Resistance Is Feudal)

**Status: Done (2026-11-14). Dependencies: -.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: the game crashed when Duke blew up barrels (biohazard and
radiation symbols) in a room of RESISTANCE IS FEUDAL.

Evidence (read 2026-11-14 from the user's session, nothing changed):
`recomp/build-local/logs/session-20261114-134841.log` stops at 14:28 with no
shutdown lines (level 7 loaded at its line 792, a slot 0 save just before).
systemd-coredump kept a core (`coredumpctl`, PID 31710, SIGSEGV, the build of
2026-11-14 12:52). gdb on that core:

- the faulting thread is a Polymorph redraw render thread:
  `render_thread.c:464 thread_main` -> `render_replay.c:528
  rc2_render_replay_rt_session` -> `gpu.c:4553
  gp0_exec_shaded_textured_quad` -> `gpu_gl_renderer.c:3929
  gpu_textured_triangle` -> `sh_classify_tri`, `gpu_gl_renderer.c:4699`
  (the D41F2 / D23O part 2 shadow classifier);
- the triangle's light mask was `0xc0000001` (lights 0, 30 and 31 of the
  32-entry table, `GL_MAX_LIGHTS`);
- the loop is `for (int i = 0; m >> i; i++)`. With bit 31 set, `i` reaches
  32, where the shift count is out of range (undefined in C; x86 uses the
  count mod 32, so `m >> 32` is `m` again and never zero). The loop never
  ends and reads `s_light_data[i * 8]` past the table: `i` was 29,109,918
  (`r13 = 0x1bc2e9e`) when it left mapped memory.

Why the barrels: their explosions add dynamic lights, so the frame's light
table filled to its last slot and a lit wall's list held light 31. Any scene
that fills slot 31 on a classified (shadowed) wall can crash the same way;
the barrels are only the likely way to get there.

Same class, found in the D41T sweep: `lz_ensure` (`gpu_gl_renderer.c`
about 7197-7209) shifts by the tile column, which reaches 32 or more on a
surface wider than 1024 px (unlikely; fix it with this one). Lighting
follow-up: D41T (chains share one light, no blast shadows).

Work: bound the loop to the table (iterate set bits, `i < 32`), check the
other light-mask loops for the same pattern (only this one matched
`m >> i` in `runtime/src`), export the framework patch. Acceptance: a lab run
with 32 lights in view (spawned barrels or the console) is clean, the user's
barrel room can be blown up repeatedly, including after a load; pictures
unchanged (`PSX_RT_VALIDATE`).

Work log:

- 2026-11-14 ([note 472](../documentation/472-d50e-d41t-barrel-chains.md) section 1). `sh_classify_tri` walks the set bits of the mask (same order as before for every mask without bit 31, so no picture can change); `lz_ensure` rows are 64-bit masks and wide surfaces over 2048 px load whole, so no shift reaches the mask width. No other `m >> i` loop in `runtime/src`. Framework patch exported; a pristine pinned checkout plus the patch matched all 90 framework files. Lab (own empty card dir, new game, `level 7`, `light bake off`, 24 test lights round Duke): with only the old loop restored the game crashed when the table filled, same stack as the user's core (`i` 29,109,864 against 29,109,918); the fix ran 2 minutes at 31-32 lights, through a load and a refill, and quit cleanly. `PSX_RT_VALIDATE` (classes on) 12 of 377 differing against 11 of 379 on the old loop, the class cache's usual noise; 384 of 384 with classes and reuse off. Remaining: the user's barrel room, repeatedly and after a load. Needs playtest.
- 2026-11-14: accepted and closed. User: "lock it in and close it out! it 100% works".

### D50F - Saving or loading sometimes slows the game and the save notice is unclear

**Status: Todo (backlog; user playthrough commentary 2026-11-14). Dependencies: D50D.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: a save or load sometimes gives a noticeable slowdown, and
they do not always get a clear confirmation that the save worked.

Evidence so far (logs of 2026-11-14 12:53, 13:06, 13:48): every save
succeeded (42 writes to UI slots 1, 9 and 10, each `outcome=success`
followed by `GAME SAVED`); capture 2.8-9.1 ms on the emulation thread,
write 93-108 ms on the writer, done about 135-140 ms after the request;
loads about 23 ms. So these sessions show no failed save; the slowdown is
not reproduced yet. Lead: the 13:06 session's redraw workers report one job
of 10.4 s (`job_us_max` 10431530) and a level 5 start with the audio ring at
17 ms (`audio 17->264`), worth placing against the user's saves and loads.

Work: reproduce with the user's real pattern (F6 / F9, menu saves, loads in
a row) at 180 Hz; measure the game thread and presentation around save
admission, capture, write and load; make the notice unambiguous (shown after
the write succeeds, a clear failure notice otherwise); check whether
repeated save/load contributes to D17BH. Acceptance: no visible slowdown on
save or load in the user's playtest, success and failure each clearly shown.

### D50G - A sleeping enemy on screen corrupts the frame; the GPU runs away

**Status: Done (user-accepted 2026-11-15, closeout authorized). Dependencies: D17BF.**

User, 2026-11-14 (RESISTANCE IS FEUDAL, level 7, first person): "a
catastrophic game crash" just after Duke was killed: a frozen image, then on
the repeat "it's gone absolutely crazy ... my computer itself actually made a
physical sound". The user saved the spot as UI slot 6 (Duke dies almost at
once after the load).

Observed (session logs 22:07 and 22:26, live captures): the death prompt over
garbage blocks; the GPU at 100 % and 240 W (the fans were the sound); a game
field every 2-5 s; resident memory 646 MB to 2 GB; the debug port slow; the
starvation watchdog silent (the thread is in host GPU work). Log lines
`dma: GPU list link ... is outside the allocated mod packet memory` (the D50C
check) and `[TTK redraw-only] off: workers behind`.

Mechanism (core dump of the frozen game, gdb on lab runs; evidence labels as
in note 472):

- MEASURED: the main thread is in the DMA2 linked-list walker, sending
  `glb_copy_rect` and draws from garbage, about 15,000 VRAM copies and 23,000
  draws a game field. The list's chain links into the middle of 10-word GP0
  0x2C packets in the game's packet ring (`0x800f382c`-`0x80124a0c`, cursor at
  render context `0x800d67a8` +0); vertex words read as headers lead into game
  code, then into a five-node loop (`0 -> 0xFF0000 -> 0xDEC -> 0x58000 -> 0x8`)
  that the walker follows to its 0x40000-node cap every list. The D50C check
  does not end it: the loop never leaves RAM or the allocated aperture.
- MEASURED: `sleeping_draw` (`recomp/src/ttk/sleeping_enemies.inc`, D17BF)
  saves the 0x88-byte render context, has the game's actor renderer
  `0x80032e78` draw each stand-in, then writes the whole context back. When a
  stand-in is on screen the renderer allocates its packets from the ring and
  links them into the ordering table: the cursor moves (for example
  `0x8011ec9c -> 0x8011f00c`, 600-3,000 bytes), and the restore rewinds it.
  The rest of the frame reallocates that memory, so the stand-in's linked
  packets are overwritten and links land mid-packet. The rewinds were seen
  immediately before the first bad lists.
- MEASURED (A/B, private copy of UI slot 6, offscreen, 180 Hz, gdb list
  checker): sleeping enemies on, every run broke by list 1,755-1,836 (6 runs,
  including `DNTTK_NEAR_CLIP=0`); `DNTTK_SLEEPING_ENEMIES=0`, no bad list in
  8,400. Third person did not break at this spot (the stand-in is not in that
  view); the cause is not first person or death themselves.
- MEASURED: the stand-in draw changes context words +0x00 (the ring cursor)
  and +0x50, +0x54, +0x5c, +0x60 (actor light and draw state).
- INFERRED: any frame with a stand-in on screen can corrupt the list; most
  overwrites happen to keep packet boundaries (at worst wrong polygons), and a
  misaligned one makes the loop. D50C (level 3 pit death) may be the same
  cause; not checked.

Fix (specified, not implemented):

1. `sleeping_draw`: restore the render context except word +0x00, so the ring
   cursor keeps what the stand-ins allocated, as a live enemy's draw does.
   Words +0x50-+0x60 and the rest are restored as now. A search of
   `recomp/src` on 2026-11-14 found no other host call that saves and
   restores `0x800d67a8`; new ones must keep the cursor too.
2. Optional safety net, decided with the user: end a DMA2 list at a node
   already visited (or bound the payload a list may send), so a broken list
   costs one bad frame, never a runaway GPU. Vanilla must stay unchanged.

Test: the lab in `recomp/analysis/d50g/` (local): `gdb/run_trace.sh` loads
the private copy of UI slot 6, watches with no input and checks every list
(`gdb/chain.py`); `gdb/rewind.py` shows the cursor at the restore. Acceptance:
no bad list over several thousand lists with sleeping enemies on, the
cursor no longer rewinds, the stand-ins still drawn in the right place
(monitors and distant views, D17BF), Vanilla unchanged; then the user's
playtest at the same spot.

Work log, 2026-11-15 (Needs playtest). Fix 1 built: `sleeping_draw`
(`recomp/src/ttk/sleeping_enemies.inc`) restores render-context words
+0x04..+0x84 and leaves +0x00, the packet ring cursor, where the stand-in
draws left it. No other code saves the context. No framework change.
Lab (private copy of UI slot 6, offscreen, 180 Hz, sleeping enemies on, gdb
list checker): 0 bad lists in 11,400 (before: every run broke by list
1,755-1,836); at each on-screen stand-in the cursor keeps its +448 to +1,952
bytes and every other context word matches the saved value; the stand-in's
packets reach the GPU intact (12-14 GP0 0x24/0x2C polygons in the frame's
list, no bad list in 4,800). Before, those packets were overwritten in the
same frame, so a stand-in on screen could draw wrong or not at all. Vanilla:
the stand-ins exist only in Modernized with the extended draw distance, so
its path is unchanged. Not done: the optional safety net (fix 2, left for the
user to decide); D50C's pit not re-checked. Local tests: 259 run, 1 failure,
`test_primitive_trace` (pre-existing since 2026-11-14, not touched);
check_jobs and check_repo OK. Remaining: the user's playtest at the spot.

User playtest, 2026-11-15, at UI slot 6: "looks to be working!! lock it in and close it out". Accepted; closeout authorized. The optional DMA loop guard (fix 2) was not taken up.

### D23AA - FAMILY JEWELS start stutters until the drawbridge is lowered

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; the 15 W package limit, waited on the cancelled D23N4, note 494; struggle points go to D23AQ and D23AH). Dependencies: D23N4.**
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14: noticeable stuttering right at the start of FAMILY
JEWELS; after lowering the moat drawbridge it largely disappears.

Work: per the AGENTS.md performance rule, first check the host's state for
the session (backups, updates, CPU, disk, memory). Then profile the start
before and after the drawbridge (headless lab, 180 Hz, spawn or start
position): room visibility, redraw workers, lamp bake, sleeping enemies,
shadow maps. Decide whether the change is the level's state (bridge up
changes what is visible or animated) or warm-up (caches filled by then).
Acceptance: the start plays smoothly before and after the bridge in the
user's playtest, pictures unchanged.

Work log, 2026-11-15 (Blocked, measured), [note 476](../documentation/476-d23v-d23n2b-d23y7-d23aa-field-laptop.md) section 5. Host state: the
report came from the 2026-11-14 playthrough on the other machine (this
laptop's cards are unchanged since 11-13 and its session logs of that day
rotated out), so that session's host conditions cannot be checked here; the
laptop was idle during these runs. A fresh FAMILY JEWELS start (private
slot 11 of the lab's card copy). Real display, the player's settings at
120, 3 interleaved runs against DUKE HILL's start: 225 / 117 / 60 late
repeats a run (DUKE HILL 5 / 3 / 9), run medians 60 30 59 pictures, a
redraw job 13.6-15.1 ms (9.0-9.5); the repeats fall in the first ~14 s,
while Duke is at the start. Standing there facing the castle: 30.5-36.8 ms
a job (21.9 with DRAW DISTANCE ORIGINAL), presents 40-46 a second. The view
over the moat (66 water meshes), the castle walls and through the gate into
rooms 18,000-28,000 units away, with enemies in view, is about three times
DUKE HILL's geometry (1,279 object-path polygons and 129 KB of packets a
frame against 377-638 and 36-56 KB), rebuilt by every redraw. The raised
drawbridge object is not the cost (moved out of view in the private copy:
1,269 polygons, the interior shows instead); INFERRED: the stutter fades
once the bridge is down because Duke has left that view. Tried a
cache-friendlier host-vertex table in the recorder: no gain, reverted. No
exact cut found; the cure is captured redraws (D23N4).

Work log, 2026-11-16 (three-machine session, laptop lane, [note 494](../documentation/494-d23y7-d23aa-field-laptop-today.md)):
today's build, the field laptop, offscreen, three runs at 120. The FAMILY
JEWELS start at SIMPLE: 59.9 / 58.8, 120 pictures a second. At ULTRA:
58.6 / 51.0, 51 pictures. The worst seconds (frame-tagged profile) all run
at 1.2-1.8 GHz with the emulation thread at 93-97%; a frame costs 16.9 ms
there against 11.8 ms in the best seconds, spread over device service,
timing edges, GP0 and our world pass in about the clock's proportion: the
package limit again, not the drawbridge view's own code. Blocked as before.

Re-measured, 2026-11-17/18 ([note 506](../documentation/506-d23y-chain-baseline.md), [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)): pure AUTO holds 60 / 60 at the start on both machines; ULTRA 120 on the field laptop 50-56 (GPU-bound), citadel 52.5-60 with occasional fallbacks. Still Blocked.

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): the 15 W package limit, waited on the cancelled D23N4, note 494; struggle points go to D23AQ and D23AH.

### D23AB - Three-machine baseline (desktop, field laptop, citadel)

**Status: Done (2026-11-16, [note 496](../documentation/496-d23ab-three-machines.md); the real-display questions answered in note 498). Dependencies: D23Y6.**

A third test machine, citadel (a TV media PC: 4-core Ryzen 3 PRO 3200GE,
Radeon Vega 8 integrated GPU), joins the desktop and the field laptop.
Work: set it up like the field laptop (checkout, owned disc kept private,
build, a lab with private card copies), record its AUTO choices (cores,
GPU probe, GRAPHICS grade), then measure FRAME RATE 60/120/180 at GRAPHICS
AUTO and ULTRA in UI 2, 8, 9, DUKE HILL's town and the FAMILY JEWELS start
(game and present rates, emulation thread and worker CPU, GPU busy and
clock, CPU clock), the same on the field laptop at the player's settings,
and write what bounds each machine (CPU or GPU) at 120.

Work log, 2026-11-16 (three-machine session, [note 496](../documentation/496-d23ab-three-machines.md)): citadel cloned
into `~/CODE` with submodules, a venv (cmake and ninja from pip, no system
packages changed), the owned disc copied privately from the desktop
(SHA-256 checked), built in about 50 minutes; its lab is the field
laptop's with a `GRADE` switch and AMD clock sampling. AUTO there: 4 CPUs,
3 redraw workers, GRAPHICS BALANCED as the fallback because the GPU probe
fails offscreen. Offscreen, two interleaved runs a cell, host checked
before each: citadel at AUTO holds 120 in UI 2, 8 and the town and 110-114
in UI 9 and the FAMILY JEWELS start with neither side saturated (the
Vega's clock stays at 264-432 MHz); 123-166 pictures at 180 (CPU-bound);
63-104 at ULTRA 120 (GPU-bound); and 45 pictures at 60 offscreen. The
field laptop at the player's 4x: 45-52 pictures at 120 (package-bound;
note 494); SIMPLE holds 120 and 180. The desktop at 120: headroom
everywhere. Needs the real display: citadel's AUTO probe on its TV and its
60-rate pacing.

### D23AC - AUTO redraw workers on four-core CPUs

**Status: Cancelled (2026-11-16 evening: not needed after D23AD; citadel's pure AUTO with 3 workers holds 60 on its real screen, [note 498](../documentation/498-d23ad-60hz-pacing.md) section 4b). Dependencies: D23AB.**

On citadel (Ryzen 3 3200GE, 4 cores without SMT) AUTO starts three redraw
workers; at FRAME RATE 120 and 2x, UI 9 and the FAMILY JEWELS start then
show 110-114 pictures a second, while with one worker every scene holds
120 (offscreen, two runs each; the GPU's clock the same either way). Work:
measure one, two and three workers on citadel (and one against two on the
field laptop) at 120 and 180 in the five D23AB scenes, then choose AUTO's
count from the cores and what the emulation and render threads take.
Acceptance: AUTO's count at least as good as any fixed count on each
machine, pictures unchanged, measured offscreen and confirmed on the real
display.

### D23AD - 60 Hz screens keep 60 pictures

**Status: Done (2026-11-16, accepted by the user on the field laptop, [note 498](../documentation/498-d23ad-60hz-pacing.md)). Dependencies: D23Y6.**

User, 2026-11-16: the field laptop played worse than in the morning; "we
have GOT to get this working on lower end hardware comfortable". At FRAME
RATE 120 on a 60 Hz screen the governor stepped the redraws to every 2nd
refresh (30 pictures a second) and stayed there under backoff, and a stall
at startup or before a load counted as the game falling behind. Acceptance:
60 pictures a second on 60 Hz screens wherever the game holds 60, the
original presentation where it cannot, game speed unchanged, faster
displays unchanged; the user's playtest on the field laptop and citadel.

Work log, 2026-11-16 ([note 498](../documentation/498-d23ad-60hz-pacing.md)): host and build ruled out first (a
real-display A/B of this morning's build against today's: no consistent
difference); the morning's settings were LIGHTING ON and LIVE SHADOWS
SIMPLE at 2x, the profile had since moved to RELIT and SMOOTH (60 against
41-55 pictures on the real screen). Then: a second under 20 game frames is
a stall, not judged (`overclock_lease`); on a timeline under 75 Hz the
redraws' last step is every refresh, a scene that cannot hold it goes to
the original presentation (`last_div`; developer `DNTTK_REPLAY_SKIP30=0`
restores the step). Real screens, interleaved, two runs a cell: citadel at
ULTRA 120 58.5-59.6 pictures a second in all five scenes (before 36.5-59.5);
the field laptop at 2x with RELIT or ON 59.8-60 in all three (before
44-60). GAME_MANUAL updated. Needs the user's playtest.

Closeout, 2026-11-16 evening: the user, after playing on the field laptop
at pure AUTO: "ok now gameplay is looking significantly better! ... lock it in and close it out" Their session (19:40-19:47, BALANCED, FRAME RATE
AUTO): no 30-picture step, hitch seconds 9% with the game at 59.8 a second
in their median, one underrun, one hand-over to the original presentation
and back; minutes of 58.8, 60.0, 60.0 and 50.3 presents a second (one
minute of 15.6, a pause or menu). Done.

### D23AE - AUTO on 2-core CPUs with SMT uses the redraws

**Status: Done (2026-11-16, accepted with D23AD, [note 498](../documentation/498-d23ad-60hz-pacing.md) section 7). Dependencies: D23AD.**

D23W capped FRAME RATE AUTO at the original 60 presentation on CPUs with 2
or fewer physical cores (the field laptop then slowed to about half speed
with redraws). Since one worker on 2 SMT cores (D23Y4/D23Y6) and D23AD's
fallback, the field laptop holds the redraws at its 60 Hz screen's rate.
Change: 2 physical cores with 4 or more logical CPUs take AUTO's 120 cap
(`replay_auto_cap_for`, replay_rate.h); 2 logical CPUs or fewer keep 60.
Acceptance: pure AUTO on the field laptop shows 60 fresh pictures a second
with the game at 60, the original presentation where a scene cannot; the
user's playtest.

Work log, 2026-11-16 ([note 498](../documentation/498-d23ad-60hz-pacing.md) section 7): real screen, GRAPHICS BALANCED: FRAME
RATE AUTO (before: the original presentation) 60 pictures, game 60 / 59.3-
59.6, emulation thread 52-63%; FRAME RATE 120 (the redraws at 60) 60
pictures, game 59.9-60 / 55.7-59.1. After the change, pure AUTO (GRAPHICS
and FRAME RATE AUTO): "at most 120 images a second (4 CPUs, 2 cores)", one
worker, 60 pictures a second in all six scenes, game median 59.9-60, lowest
tenth 58.1-59.1, the town handing over to the original presentation. Unit
test updated; framework patch verified.

### D23AF - Campaign-wide performance census and hot-spot surgery

**Status: Done (2026-11-17, user: "lock in this work and close us out"; [note 503](../documentation/503-d23af-campaign-census.md)). Dependencies: D23AB, D23AD.**

User, 2026-11-17: the D23 chain has run out of single-spot fixes (D23N4b
cannot pay, D23M3B not exact, D23Y5/Y7/Y8/AA Blocked by the field laptop's
15 W package); the whole campaign has never been measured. Find where the
game is actually heavy, level by level and machine by machine, then cut the
biggest costs.

Work:

- Phase 0: host state on each machine logged with every run (load,
  backups, updates, I/O pressure, swap, thermals; contention waited out);
  the field laptop and citadel at the checkout's HEAD plus the tracked
  framework patch; private card and savestate copies only, the player's
  saves, cards and profile hash-checked before and after every run.
- Phase 1: census. Every level the debug level select offers, entered with
  `level N`, one scripted route from its start (stand 5 s, a slow 360 turn,
  a forward walk with turns, look up and down, one weapon burst), the same
  inputs on every machine, first and third person. Desktop at 180 and 120
  (offscreen on the real GPU, GRAPHICS CUSTOM with the ULTRA rows); the
  field laptop and citadel at pure AUTO on the real display, plus the
  laptop at SIMPLE 120 and ULTRA 120. Per second: game rate, presents and
  fresh pictures, fallbacks to the original presentation, emulation thread,
  render threads and redraw workers, other host CPU, CPU clocks,
  clock-weighted GPU busy, present-interval p50/p95/max, spikes over two
  target intervals; a savestate and a picture at each level's worst second.
- Phase 2: the top five hot spots reproduced from their savestates and
  profiled to the owning code path; host contention, package or thermal
  limits and real code cost told apart. Each becomes a child job
  (D23AF1...).
- Phase 3: the best saving per risk fixed behind a developer switch, proven
  exact (picture and packet comparisons, validator, Vanilla shots), A/B
  measured on all three machines with alternating repeats; default-on
  proposed only where all three agree.

Acceptance: the census JSON per run and the ranked hot-spot table in note
503; every child job evidenced; each fix exact and measured on all three
machines; Vanilla unchanged, Classic untouched.

Work log, 2026-11-17 (overnight, started): see note 503 and the entries
below as each phase ends.

Work log, 2026-11-17 09:00 (checkpoint, phases 0-1 running). Phase 0 done:
the field laptop and citadel at `fe18de2` with the tracked patch, full
rebuilds (about 50 min, two runtime headers changed). Harness
`recomp/analysis/d23af/census.py` (local) with `queue.sh`, `summary.py`,
`abbake.sh`. Desktop census done (180 and 120, both views, 21 levels each,
02:56-04:02, every host check passed, player files unchanged). The
desktop holds its target everywhere but FAMILY JEWELS at 180 in first person
(fresh pictures p10 140, three workers saturated: a first-person job there
costs 11.9 ms against 7.3 ms in third person at the same spot, a larger
view, not a different hook). The remote builds finished at 03:24 but the
completion watcher never reported, so the laptop and citadel census started
only at 08:37 (lost machine time, recorded). First weak-machine finding,
D23AF1: a level's first lamp bake after an update (the bake's key changed
with D55A) runs 15-27 s on the field laptop and 5-8 s on citadel beside the
game; on the laptop the game falls to 38-50 frames a second and the
governor's fallback then keeps the redraws off for minutes. Built behind
`DNTTK_BAKE_YIELD=1` (off): the bake yields to a game behind real time, one
thread on 2-core CPUs, bake seconds not judged. Exact by construction and
proven on the desktop: the .bake files with the switch on and off are
byte-identical on 5 levels. Remote A/B queued after their census runs.

Work log, 2026-11-17 12:10 (census done, note 503): runs on all three
machines (desktop ULTRA 180/120, field laptop pure AUTO cold and warm,
SIMPLE 120, ULTRA 120, citadel pure AUTO, both views), 21 levels each,
host checked before every launch and level, player files unchanged in every
run. Ranked hot spots and children: D23AF1 (first bakes), D23AF2 (worker
restarts), D23AF3 (fork cost), D23AF4 (the laptop's package at AUTO),
D23AF5 (FAMILY JEWELS at 180), D23AF6 (the validator at RELIT), D23AF7 (a
watchdog crash, fixed). Framework patch exported and verified against the
pinned framework (92 files); 260 local tests, the five CI runtime tests,
check_jobs and check_repo pass; all three machines on the same final build.
Tests for the user in note 503 section 5.

### D23AF1 - A level's first lamp bake yields to the game

**Status: Done (2026-11-17, user: "turn the bake yield on by default too, then lock it in and close it out"; on by default, `DNTTK_BAKE_YIELD=0` turns it off, [note 503](../documentation/503-d23af-campaign-census.md) section 6.1). Dependencies: D23AF.**

Census finding (MEASURED): a level's first bake after any change to the
bake's inputs (D55A changed every level's key) ran 15-27 s on the field
laptop (TIME TO KILL 27.2 s, LET THE GAMES BEGIN 22.8 s) and 5-8 s on
citadel, beside the game on the same cores. On the laptop the game ran at
38-50 frames a second throughout and the redraw governor took the original
presentation; its backoff then kept the redraws off for several levels
after the bake had ended. With warm caches the level starts are clean on
every machine (fresh pictures 99.8-100% of the target in the first 2 s).

Change (`DNTTK_BAKE_YIELD=1`, developer, off): while the game is behind real
time (a judged second under 58.5 frames, sticky 3 s) each bake thread
sleeps three times its last chart's work after it (a quarter duty); one bake
thread on CPUs with 2 physical cores; seconds while a bake runs are not
judged by the governor. Exact by construction (charts are independent; only
the pace changes); proven on the desktop: the .bake files with the switch on
and off are byte-identical on 5 levels.

Acceptance: on the field laptop and citadel, cold-bake A/B (fresh cache
folders, levels 0 2 3 5, alternating, repeats): game speed during the bake
no worse and the redraws back after it; bakes byte-identical; Vanilla
unchanged (it bakes nothing).

Work log, 2026-11-17 (note 503 section 4.1): A/B with fresh cache folders.
Desktop and citadel: no effect (game 59.5-60.2 during the bakes either way,
redraws on). Field laptop, route only: 0-7 seconds under 57 a level against
8-23, redraws kept, 0 fallbacks against 1, but most bakes unfinished within
the 35 s route; staying 100 s: GOLD AND GUNS 0 slow seconds (7-8 off), bake
28.6-31.9 s (16.7-16.8 off); TIME TO KILL bake 110 s or more (28 s off) and
10 / 29 slow seconds (24 off) with the redraws kept running. Bakes
byte-identical on and off on all three machines (and across machines). Next:
judge bake seconds again but retry a fallback taken during a bake as soon as
it ends without doubling the backoff; one bake thread at full duty on 2-core
CPUs instead of a quarter duty; re-measure levels 0 and 3.

Work log, 2026-11-17 afternoon (note 503 section 6.1): refined as queued.
Bake seconds are judged again; a floor fallback taken during a bake keeps
the wait undoubled and the redraws are retried the moment the bake ends
(`replay_load_bake`, `frame_replay.cpp`; the retry counts as a fresh start,
and a shed backoff raised during the bake is restored); 2-core CPUs bake on
one thread at full duty, more cores keep the quarter duty. Field laptop,
cold caches, levels 0 and 3 staying 100 s, off/on alternating, 3 repeats:
TIME TO KILL bake 36.7-38.8 s (off 27.3-28.0), game 58.7-59.4 a second
during it (off 46.3-47.7), 7/2/6 seconds under 57 (off 23/23/23), the
fallback retried at the bake's end in every run and the redraws then held
(off: held off by the backoff, 2% of the run); GOLD AND GUNS bake
26.7-26.8 s (off 16.7), 1/0/0 slow seconds (off 7-9), redraws 100% (off
74-78%). Bakes byte-identical on and off (levels 0, 3, 6, six runs).
Proposed default on; left off pending the user's decision and playtest.
Remaining: the bake is 1.4x longer on the laptop (the level is lit by the
live lamps meanwhile); citadel's quarter duty unchanged (neutral there).

Closeout, 2026-11-17 (user: "turn the bake yield on by default too, then lock it in and close it out"): the yield is on by default
(`DNTTK_BAKE_YIELD=0` switches it off). Desktop smoke on the default, cold
cache, offscreen ULTRA 120: GOLD AND GUNS and the next level baked (3.5 s
and 2.3 s), game 59.9, 120 pictures a second (p10 119), 0 fallbacks, player
files unchanged. Local tests OK.

### D23AF2 - Redraw worker restarts without the hitch

**Status: Done (2026-11-17, user: "turn the bake yield on by default too, then lock it in and close it out"; asynchronous reap on by default (`PSX_RW_ASYNC_REAP=0` turns it off); coalescing off, [note 503](../documentation/503-d23af-campaign-census.md) section 6.2). Dependencies: D23AF.**

Census finding (MEASURED): the session's first shot hitches on every
machine (desktop 50-61 ms, citadel 44-47 ms, field laptop 30.5 ms a present
interval). Field-tagged profile: the emulation thread in
`submit_job` > `maintain` > `stop_workers`/`start_workers` (framework
`render_worker.c`): a plugin's lazy mod-memory allocation (+1,300 bytes)
makes `maintain()` re-fork every redraw worker, and the stop waits for each
killed worker's full teardown. The session's first level start does the same
about 30 times in a second (+2,192-byte steps); THE REAPER once (+188).

Change: `PSX_RW_ASYNC_REAP=1` (developer, off): killed workers are reaped
later; exact by construction (an epoch bumped before the kill, re-checked by
every worker after each claim; a retired worker hands a newer job back and
exits; the job slots it held stay out of use until it is reaped; a detector
drops and counts any finished job whose worker generation differs).
`PSX_RW_COALESCE_MS=N` (developer, off): a growth retires the workers once
and the new ones are forked N ms after the last growth (the emulation thread
draws the live frames meanwhile, pictures exact).

Acceptance: restart cost and the first-shot and level-start hitches measured
on all three machines, alternating; detector counts 0; Vanilla unchanged.

Work log, 2026-11-17 (note 503 sections 4.2 and 4.4): rotated A/B, levels
0, 27, 1 from a fresh launch. Asynchronous reap: a restart's worst case
desktop 39-41 to 22-26 ms, citadel 28-33 to 18-22, field laptop 14-25 to 7;
the session's restart stall halved (desktop 745-768 to 347-370 ms); first
shot 3-12 ms shorter; hand-backs and foreign-generation jobs 0 in every
run; `runtime/tests/test_render_worker_epoch.py` pins the ordering (in CI).
Proposed default on. Coalescing 120 ms: the desktop's stall 76-81 ms, but
the second after a coalesced restart shows a new gap on every machine
(field laptop 59-64 ms, citadel 33-39, desktop 16-21): off. Vanilla 6 of 6
shots byte-identical against HEAD's build.

Work log, 2026-11-17 afternoon (note 503 section 6.2): user selection -
the asynchronous reap is on by default (framework `render_worker.c`:
`PSX_RW_ASYNC_REAP=0` switches it off); `PSX_RW_COALESCE_MS` stays off.
Patch exported and verified against the pinned framework;
`test_render_worker_epoch.py` and the other CI runtime tests pass. Ten
remote runs on the default (field laptop, citadel): worst restart 5.0-7.3
ms on the laptop and 16.4 ms on citadel, the async figures of section 4.4
(INFERRED from cost); detector 0, hand-backs 0, no aborts. Remaining: the
user's playtest (first shot, level starts). Closed out 2026-11-17 with
D23AF1 and D23AF8 (user: "turn the bake yield on by default too, then lock it in and close it out").

### D23AF3 - Cheaper worker forks

**Status: Needs playtest (2026-11-17/18, built off by default; [note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)). Dependencies: D23AF2.**

After D23AF2's asynchronous reap a restart still costs about 15 ms on the
desktop: three forks, each copying the page tables of the game process's
private memory (MEASURED during TIME TO KILL at ULTRA 180: 383 MB anonymous,
243 MB private dirty; the largest private mappings 128, 83, 65 and 48 MB).
Find which host-only buffers the workers never read (GL staging, bake
outputs, audio) and keep them out of the fork (`MADV_DONTFORK`, or a shared
mapping), proven exact (workers fault on any use), measured.

Second route, measured the same day (temporary allocation trace, desktop):
the restarts come from lazy plugin allocations: `sleeping_update_body`
takes one 2,192-byte stand-in block per sleeping enemy on first use (34 in
TIME TO KILL; up to 48, reused afterwards), a `register_ttk_controls` hook
on `0x8003C500` 1,300 bytes at the first shot, one on `0x80028A2C` 176 bytes
in THE REAPER. Allocating them before the first fork removes the restarts
outright, but changes the mod-memory layout that savestates record (a load
keeps the saved bytes only up to the first differing allocation record), so
it needs a savestate-compatibility design first; not done unattended.

Where the copied memory is (MEASURED, desktop, TIME TO KILL, ULTRA 180):
the largest private mapping is the executable's `.bss` (1.3 GB, about
125 MB touched), whose biggest objects are always-on observability rings
and buffers: `g_dirty_ram_block_log` 224 MB, `g_fntrace_ring` 144 MB,
`s_taps` 64 MB, six host-vertex apertures (`s_hpv_ap_live`/`_replay`) of
57 MB, `sio_irq_buf` 40 MB, `s_sreg_trace` 36 MB, `sio_txn_buf` 34 MB,
`s_events` and `s_ring` 32 MB; then the heap (64 MB) and three anonymous
mappings of 41-48 MB. Host-only rings are `MADV_DONTFORK` candidates (page
aligned, never written by a worker), but some (the dirty-RAM block log)
may be written by code a worker runs: each needs that proof first.

Work log, 2026-11-17/18 ([note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md) section 4): built. `PSX_FORK_WIPE=1` marks
29 observability rings (`psx_obs.h`, `PSX_OBS_REGISTER`) wipe-on-fork;
`PSX_FORK_WIPE_CHECK=1` maps them `PROT_NONE` in workers and logs any
access: two first candidates were touched and removed, the 29 never in any
scene on any machine. Steady copy-on-write faults of the emulation thread
halve (laptop 120-190 to 38-78 a second, citadel 136-222 to 64-106); fork
time unchanged on the Mesa machines (3.0-3.4 ms). `PSX_FORK_DEV=1` keeps
the GL driver's `/dev/dri` / `/dev/nvidia` maps out of the fork (0 worker
deaths): with the wipe -35-40% on the desktop (NVIDIA's PFN maps), -5-10%
on citadel, nothing on the laptop. Hitch (longest present interval) within
the spread. Recommendation: the wipe on by default; the device maps off
except NVIDIA. The remaining fork cost is private memory workers need and
the mapping count (the no-restart route stays).

### D23AF4 - AUTO grade for 15 W shared-package laptops

**Status: Todo (the user's look decision; [note 503](../documentation/503-d23af-campaign-census.md)). Dependencies: D23AF.**

Census finding (MEASURED, field laptop, pure AUTO = BALANCED 2x, warm,
third person): with the redraws on, the GPU runs 75-80% busy at 500-900 MHz
and the 15 W package leaves the emulation thread's core at 2.1-2.5 GHz; the
game slips to 56-57 and the governor takes the original presentation (CPU
back at 3.1 GHz, GPU 300-600 MHz), then keeps it for minutes. AUTO's grade
comes from a GPU-only probe that cannot see the shared package. Compare the
census's SIMPLE 120 and AUTO runs and propose AUTO's choice for 2-core 15 W
machines (the leanest grade that keeps the redraws).

Data, 2026-11-17/18 ([note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md) section 5, FRAME RATE AUTO, real screens): field laptop SIMPLE lowest tenth 58.8-59.1 against BALANCED 57.9-59.1, emulation thread 3-6 points lower in UI 2 and 9, both at the 15 W package; citadel both hold 60, SIMPLE 0.4-0.7 W and 10 GPU points less. The look is the user's call.

### D23AF5 - FAMILY JEWELS start in first person at 180

**Status: Cancelled (user 2026-11-23 optimization triage: "close the whole manure pile"; desktop first person at 180, hung off the cancelled D23AA, note 503). Dependencies: D23AA.**

The only desktop census cell under its target: FAMILY JEWELS at 180 in
first person, fresh pictures p10 140 with three workers saturated (300%).
Repro from the census's worst-second savestate, one worker: 82 pictures a
second in first person against 138 in third person; a first-person job
costs 11.9 ms against 7.3 (the same hooks in the same shares, a bigger view:
the eye-height camera sees more of the castle and moat). D23AA's geometry
finding; no new exact cut found.

Re-measured on the field laptop and citadel, 2026-11-17/18 ([note 507](../documentation/507-d23y8-gl-server-d23y5-d23af3.md)): FAMILY JEWELS start (each machine's own view setting) holds 60 / 60 at the original 60 and pure AUTO on both. The first-person 180 Hz desktop case was not re-run (the desktop was background-only this session).

Work log, 2026-11-23: cancelled in the optimization triage (user 2026-11-23 optimization triage: "close the whole manure pile"): desktop first person at 180, hung off the cancelled D23AA, note 503.

### D23AF6 - Render-thread validator differs at ULTRA with three workers

**Status: Done (2026-11-18, closed with D23AG; [note 509](../documentation/509-d23ag-overnight-chain.md) section 2, [note 503](../documentation/503-d23af-campaign-census.md)). Dependencies: D23AF.**

Observed during D23AF2's A/B (MEASURED, desktop, ULTRA 180, third person,
levels 0, 27 and 1, three workers): `PSX_RT_VALIDATE=1` reported differing
pixels in every comparison, 158 of 158 and 166 of 166 with every new
switch off (80,000-280,000 pixels, 1-174 levels, boxes inside the centre of
the picture). D23Y10 measured 908 of 908 exact with one worker in UI slots
2, 8, 9. Find whether the validator's own redraw is not comparable here
(live lights or shadow state moved between the two draws) or the redraws
really differ from the single-thread path; if they differ, fix and prove.

Narrowed the same day (MEASURED, desktop, DUKE HILL, ULTRA 180, one worker):
LIGHTING RELIT 156 differing images, LIGHTING OFF 0. Three workers on the
same level: 2. The differences come from live light state (lamp waves and
flicker, shadows), which is not part of a redraw job: the validator draws
the held job again later, under newer light values. So the validator cannot
judge exactness at RELIT as it stands (D23Y10's 908/908 ran with lighting
that held still). Work: freeze the light state a job was drawn with and
give it to the validator's second draw, then re-run at RELIT; only a
difference left after that would be a redraw defect.

Work log, 2026-11-18 overnight ([note 509](../documentation/509-d23ag-overnight-chain.md)
section 2): the lab census now reads the validator's counters per travel
and per route. MEASURED (desktop, ULTRA rows at 180, three workers): every
gameplay comparison was already exact; the bad ones were all on one
loading screen a session (the spinning logo, up to 11 levels). A new
developer state signature, stored with each render-thread image and
printed by the validator for a differing one, named `lamps bake_eye` in
153 of 153: a loading-screen image is never walked, so its redraw stream
carried no bake lamp levels and each renderer instance used its previous
image's. Fix: every replayed image starts from no lights and the bake's
inactive lamp levels (`render_replay.c`), walked images always send theirs
(`dynamic_lights.inc`), and a validation spanning a savestate load (the
render thread's epoch moved on, the image is discarded unshown) counts as
skipped. Test `ttk_replay_light_state`.

### D23AF7 - The starvation watchdog aborts on a fresh heartbeat

**Status: Done (2026-11-17, closed with D23AF; fixed in the framework patch, [note 503](../documentation/503-d23af-campaign-census.md)). Dependencies: D23AF.**

MEASURED: a census run on citadel (pure AUTO, real display, MINER 69ER's
level start, 2026-11-17 09:37) ended with
`starvation_watchdog: 18446744073709551467 us without heartbeat ... aborting`
(exit 2): -149 us read as unsigned. CODE: `starvation_ring.c` compared
`now - s_last_heartbeat_us` unsigned on the emulation thread, while the
debug server's I/O thread also refreshes the heartbeat for responses in
flight (`debug_server.c`); a refresh just after the check's clock sample
wrapped the difference. Any session with the debug port in use could hit
it (the census polls it several times a second).

Fix (always on; a correctness fix, not a performance trade): the heartbeat
is read and written atomically and a heartbeat newer than the check's sample
counts as fresh; `runtime/tests/test_starvation_watchdog_wrap.py`
(registered) pins it.

### D23AF8 - Graphics grades leave FRAME RATE alone

**Status: Done (2026-11-17, user: "turn the bake yield on by default too, then lock it in and close it out"; launcher schema 78, [note 503](../documentation/503-d23af-campaign-census.md) section 6.3). Dependencies: D23AF.**

User, 2026-11-17: "the only issue i have really with graphics simple is that
it always downplays the fps. i dont really see any reason why we have to run
graphics simple on low fps as default. i dont think eduke does that";
option 2 chosen: grades stop touching FRAME RATE.

CODE: `GRAPHICS_PRESETS` (`recomp/tools/local/player_profiles.py`) sets
FRAME RATE in every grade: SIMPLE `60` (the original presentation: no
redraws, the game's own new images, about 30 a second in the census), the
others `auto`. Every grade overwrites the player's FRAME RATE, and a grade
that sets AUTO is why the census had to run the desktop's ULTRA rows as
CUSTOM to hold a fixed rate offscreen.

Evidence (MEASURED, note 503): SIMPLE at 120 on the field laptop kept the
redraws in 100% of its seconds, the game at 60, fresh pictures 48-56 a
second in the lowest tenth; AUTO's own safety nets cover weaker machines
(2 or fewer logical CPUs: the original presentation, D23AE; 60 Hz screens:
the original presentation instead of a 30-picture step, D23AD; the floor
fallback, D23Y1).

Work: remove FRAME RATE from the grade rows (a grade sets internal
resolution, LIGHTING, LIVE SHADOWS and DRAW DISTANCE); FRAME RATE is its own
row, AUTO by default, kept across grade changes and no longer part of what
makes a profile CUSTOM; a profile migration keeps existing choices (a SIMPLE
profile's stored `60` becomes AUTO only if it came from the grade, not from
the player); the OPTIONS menu, launcher text, `run.py --graphics` help,
GAME_MANUAL and tests follow. Vanilla unchanged (it presents at 60).

Acceptance: SIMPLE at FRAME RATE AUTO on the field laptop and citadel (real
60 Hz screens) holds the redraws like the census's SIMPLE 120 (game 60,
fresh pictures near 60); a player's FRAME RATE survives a grade change;
`test_player_profiles` covers the migration; the user's playtest.

Work log, 2026-11-17 afternoon (note 503 section 6.3): built.
`GRAPHICS_PRESETS` (`player_profiles.py`) and `grade_rows`
(`game_menu.cpp`) no longer carry `frame_rate`; FRAME RATE is not a grade
row, so choosing it (OPTIONS or `--frame-rate`) keeps the grade. Schema 78:
a non-CUSTOM Modernized profile's FRAME RATE (necessarily the grade's
before 78) becomes AUTO with a notice; CUSTOM keeps its rate; Vanilla
unchanged (60). Launcher help, `--settings` description, OPTIONS comments
and GAME_MANUAL's GRAPHICS section updated. Tests: grade launches leave
FRAME RATE AUTO, a chosen 144 survives four grade changes,
`test_frame_rate_migration_78` (six cases), schema bumps; 262 local tests
OK. Measured, real 60 Hz screens, GRAPHICS SIMPLE at AUTO, all 21 levels,
first and third person: field laptop game 60 (p10 59.3), fresh pictures
median 59.9 (p10 48.4 first, 55.0 third), redraws 100%, 2 slow seconds,
0 fallbacks: the census's SIMPLE 120 (p10 43.4 / 54.5, 3 / 2 slow
seconds); citadel game 60, pictures 60 (p10 59.1-59.3), redraws 100%, 0
slow seconds. Closed out 2026-11-17 with D23AF1 and D23AF2 (user: "turn the bake yield on by default too, then lock it in and close it out").

### D23AG - FRAME RATE menu counts new pictures, not refreshes

**Status: Done (2026-11-18, user: "ok yes confirmed working. lock it in and close it out"; [note 509](../documentation/509-d23ag-overnight-chain.md) section 1). Dependencies: D23AF8.**

User, 2026-11-18: "explain to me why 60 looks like the frame rate of the
ps1, and 120 looks like the real 60 ... our scale isnt hz, its fps", then
"add that backlog job now". The FRAME RATE scale mixes units (manual,
"Frame rate (Modernized)"): `60` is the original presentation, a 60 Hz
output where the game draws about 30 new pictures a second and each is
shown twice (the PS1's own rhythm); every other number counts new
pictures (`120` on a 60 Hz screen shows 60 new pictures a second, D23Y6;
`180` on a 180 Hz screen shows 180). So the menu's "60" is the PS1's
30 fps and its "120" on a 60 Hz panel is what players call 60 fps.

Work: relabel so every FRAME RATE value counts new pictures a second: the
original presentation becomes `ORIGINAL 30` (wording to agree with the
user), and `60` means 60 new pictures a second (what `120` gives on a
60 Hz screen today). Covers the in-game OPTIONS > DISPLAY row, the
launcher (`--frame-rate`, `--settings` F, help text; old saved values and
flags migrate or stay accepted as aliases, with a profile schema step if
needed), any on-screen FPS/statistics wording, the console, and
GAME_MANUAL.md. AUTO's behaviour does not change, only how it is named
and reported. Vanilla is untouched (always the original presentation).

Acceptance: on a 60 Hz screen the option labelled 60 shows 60 new
pictures a second and the original look is named as such; on the 180 Hz
desktop the labels match the measured new pictures a second; existing
saved settings keep the same behaviour after the upgrade; Vanilla
unchanged.

Work log, 2026-11-18 overnight ([note 509](../documentation/509-d23ag-overnight-chain.md)
section 1): built. New saved and runtime value `original` (OPTIONS shows
**ORIGINAL 30**: the menu font draws `(` `)` as `<` `>`, so the user's
`ORIGINAL (30)` became bracketless; wording left for the playtest); `60`
is now redraws at 60 new pictures a second, timed on the display when the
display is within 5% of 60 Hz (the path `120` takes there). Launcher schema
79: a saved `60` becomes `original` once (both profiles) with a notice;
Vanilla launches `original`; `--frame-rate`, `--settings` F, the summary,
help and GAME_MANUAL say new pictures. MEASURED: desktop offscreen 60 gives
59.8-59.9 new pictures a second (redraws every second), `original` the
original presentation; real 60 Hz screens, BALANCED: field laptop 60 =
59.9 / 41.6 (levels 0 / 27) against 120 = 60.0 / 44.4, citadel 59.7 / 59.9
against 60.0 / 59.5, both logging the display-timed path; the OPTIONS row
switches an `original` launch to 60 at once and the launcher keeps it.
Tests: schema 79 migration, menu choices equal the launcher's.
User playtest on the field laptop, 2026-11-18: at first "60 looks the same
as 30, and 120 looks like the smooth motion": the session log showed 60
running display-timed at 60 presents for about 30 s, then the governor's
fallback (emulation 46.6 frames a second, first person, BALANCED) holding
the original presentation 120 s; choosing 120 cleared the fallback. With
that explained and SIMPLE for the comparison: "ok yes confirmed working.
lock it in and close it out". Closed out the same day with D23AF6 and
D23AJ.

### D23AG1 - Fixed frame rates between display multiples

**Status: Todo (found by D23AG, 2026-11-18; [note 509](../documentation/509-d23ag-overnight-chain.md) section 1). Dependencies: D23AG.**

MEASURED (desktop, real 180 Hz screen, ULTRA, level 0): with VSYNC on, a
fixed FRAME RATE below the display's refresh used to present at every
refresh (D17BH's placement ignored the rate's period): 60 gave 180 presents
and 90 new pictures a second, 120 gave 177. D23AG fixed whole fractions (60
on 180 or 120 Hz now presents every 3rd / 2nd refresh: 60.0 presents, 59.9
new pictures). Other ratios keep every refresh: 120 or 144 on 180 Hz still
show about 175 new pictures, more than their label. Placing them at their
own rate was tried: presents then come one or two refreshes apart, and the
in-betweens, planned evenly at the rate (`update_steps`), are shown twice
or skipped (120: 119.9 presents but 67-82 new pictures, visible judder).

Work: plan each game frame's in-betweens at the times the presents will
actually take on the refresh grid (or choose the nearest even divisor and
say so in the menu), so the label equals the measured new pictures.

### D23AH - Profile-guided and link-time optimized build

**Status: Done (2026-11-24, user: "the performance increase on d23ah is significant, lock it in and close it out"; [note 554](../documentation/554-d23ah-lto-pgo-build.md): opt-in LTO and LTO+PGO builds, `optimized_build.py`; exact (Vanilla identical; an LTO-only difference traced to the title's constructor order and pinned for every build); field laptop LTO+PGO emulation thread -13 to -32%; both the laptop and the desktop run the LTO+PGO build). Dependencies: D23AF.**

User, 2026-11-18: "new jobs D23AH (PGO/LTO build) ... Goal: BALANCED at 60
new pictures a second on the laptop". The field laptop (i7-5600U, 2 cores,
15 W shared package) is bound by the emulation thread and the redraw worker
sharing one package with the GPU (note 503). A profile-guided (PGO) and
link-time optimized (LTO) build may cut CPU time per game frame and per
redraw without any change to what is drawn.

Work: measure the release build against LTO and LTO+PGO (profile from a
scripted campaign route on the desktop, applied to every machine's build);
the generated recompilation shards are the bulk of the code, so check build
time and memory on the laptop and citadel. Keep it only where it is exact
(same pictures, same game state) and measurably faster; opt-in or default
by measurement.

Acceptance: CPU per game frame and per redraw, A/B on the field laptop's
real screen at BALANCED 60; images and state identical; build still works
on all three machines.

Work log, 2026-11-18 overnight ([note 509](../documentation/509-d23ag-overnight-chain.md)
sections 4 and 6): laptop stack samples show the emulation thread mostly in
small runtime helpers called from the generated shards. Separate
`build-lto` and `build-pgo` directories (player build untouched). MEASURED,
desktop offscreen BALANCED 60 (levels 27, 22, 29): LTO -5 to -11% on the
emulation thread, LTO+PGO about -25%, the redraw workers unchanged; field
laptop with citadel's LTO executable: -8 to -16%, no governor fallback in
either LTO run, worker still saturated; citadel-trained PGO on the laptop:
-12 to -21%, no fallback, 22 and 27 at 47-59 new pictures (from 39-52),
29 still worker-bound. The framework's retiring workers now
write profile counts first (`worker_exit`, weak `__gcov_dump`, a no-op
outside an instrumented build). Builds: desktop 7-10 min, 1.8-3.6 GB;
citadel LTO 34 min, 5.7 GB (its RAM). Not done: image-by-image exactness of
the LTO/PGO executables. Proposed: opt-in `build.py --lto` / `--pgo`.

Work log, 2026-11-23/24 ([note 554](../documentation/554-d23ah-lto-pgo-build.md)).
User: "let's go in HARD with your suggestion now ... the gold and guns ones by
the lava pit are grizzly feeling too, and maybe even some of time to kill".
Built: `recomp/tools/local/optimized_build.py` (lto, instrument, train, use,
pgo, install in build-lto / build-pgo; the player build untouched),
`pgo_train.py` (the training route, every level in both views and Vanilla),
`build_compare.py` (RAM, VRAM and pictures at +60/+200/+400 after a
savestate load, two interleaved runs an executable), `run.py` developer
`DNTTK_EXE`, the runtime's exactness hook now also dumps guest RAM
(framework patch exported), developer `PSX_NO_LTO_GLOB` in CMakeLists.
Found: an LTO executable played Vanilla differently from the same savestate;
bisected to the title's hook registrations, whose constructors allocate the
GPU DMA arenas (savestates restore that aperture only up to the first
allocation that differs) and share hook addresses, in link order that LTO does
not keep. Fixed for every build: `src/ttk/ttk_register.h`, the 17 registering
sources with explicit constructor priorities in the normal build's order
(normal build identical before and after). Exact: Vanilla identical for LTO
and LTO+PGO (a second-load difference for PGO traced to host timing, not
code: the same executable matches under load); Modernized VRAM identical,
its RAM not repeatable within any build. Field laptop, three rounds: LTO+PGO
-16% level 0, -13% level 3, -32% level 22, -16% / -25% at the lava pit (UI 10
/ 12), game speed 60 throughout; LTO alone -0 to -13%. The laptop runs
LTO+PGO for the playtest (`.pre-d23ah` kept). Remaining: the user's playtest
on the laptop; the desktop's default (opt-in install); citadel untested.

Closeout, 2026-11-24. User, after playing GOLD AND GUNS on the field laptop
with the LTO+PGO build: "am i tripping? because it feels like it's playing
significantly better", then "the performance increase on d23ah is significant, lock it in and close it out". The laptop's session log agrees: 5 min
29 s of GOLD AND GUNS with 2 seconds under 57 game frames (one a pause) and no
audio underrun, against 33 seconds (31 under 50) and 44 underruns in 3 min 11 s
on 2026-11-23. Done. Both machines run the LTO+PGO executable
(`optimized_build.py install --from pgo`; the normal one kept as
`build-local/...Recompiled.normal` on the desktop and `.pre-d23ah` on the
laptop); a normal rebuild of build-local replaces it until
`optimized_build.py pgo` and `install` again (retrain after large changes).

### D23AI - BALANCED GPU passes on the field laptop's HD 5500

**Status: Done (2026-11-18, closed out: "lock in this session and close it out"; [note 511](../documentation/511-d23ai-balanced-gpu-passes-hd5500.md)). Dependencies: D23AF4.**

User, 2026-11-18: "D23AI (BALANCED GPU passes on the laptop's HD 5500)".
D23AF4's evidence: at BALANCED the redraws' GPU work (75-80% busy at
500-900 MHz) takes the 15 W package from the CPU. Work: time each GPU pass
of a BALANCED redraw on the laptop (world draw, lights, live shadows and
their maps, lightmap, resolve and present), find the costliest, and make it
cheaper without changing the picture (or with a change the user accepts),
following the leanest-choice rule (D41M).

Acceptance: per-pass GPU timings before and after; BALANCED at FRAME RATE
60 (D23AG) on the laptop's real 60 Hz screen nearer 60 new pictures a
second in the census scenes, Polymorph only.

Work log, 2026-11-18 overnight ([note 509](../documentation/509-d23ag-overnight-chain.md)
section 3): BALANCED 60 census on all levels, then one change at a time on
levels 6, 8, 1, 5 (real screen; package 15 W in every case): LIGHTING
ORIGINAL GPU 54 -> 37-41%, no fallback; draw distance ORIGINAL 41%, worker
92 -> 69%; LIVE SHADOWS OFF little change; 1x GPU 22-25%, CPU at full turbo
3.1 GHz, worker -30%, no fallback. Two workers fall back at once (D23Y6
holds). `PSX_SHADOW_PROF` cannot split passes. Not done: per-pass GPU
timing (instrumentation proposed) and any GPU change: the saturated levels
are bound by the redraw worker (D23AK).

Work log, 2026-11-18 ([note 511](../documentation/511-d23ai-balanced-gpu-passes-hd5500.md)),
after D23AK: per-pass GPU timing under the existing `PSX_GPU_PHASES=2`
(`gpu_gl_renderer.c`: `tex-lit` beside the lean `tex`, `shadow-map` and
`present` kinds, `gp_span` timeline spans outside redraws with mean and
maximum; summary `recomp/analysis/d23ai/kinds.py`, local). Field laptop,
real 60 Hz screen, BALANCED 60, census route, D23AK defaults, levels 6, 21,
22, 24, 26, 27, 29: a redraw 6.0-7.6 ms GPU, lit textured batches 4.6-6.0
(75-80%; no lean batch at BALANCED), band 0.34, wide clears 0.33, fills and
copies 0.24; present 0.7 ms; shadow maps outside redraws rare but 20-44 ms.
Medians 59-60 new pictures, lowest tenth 36-59. Compile-out A/B
(`PSX_SHADER_STRIP`): lighting layer 1.2-2.0 ms, shadow lookups and exact
edges within the spread. Tried and removed: the full program without the
exact-edge layer while SHADOW EDGES is SIMPLE (exact, desktop VRAM dumps and
present shots matched; no gain on the HD 5500). D23U's prepass again no net
gain (lit shading -1.0-1.3 ms, prepass +0.5-1.0 ms). No exact GPU cut kept;
the framework keeps only the developer instrumentation (patch exported and
verified against a clean pinned checkout). Open for the user: LIGHTING
ORIGINAL on the laptop as a look trade-off, or a larger exact job (lean
program for triangles no light reaches, run order kept).

Work log, 2026-11-18 (user laptop test 1, "it literally went down to
22fps"; note 511 section 8): the 22 was a one-second stall of the game
itself with the console open (D23AM, not reproduced); level 6 in first
person fell back to the original presentation for 15 s and 30 s, worker
bound (D23AL). Neither is a GPU pass.


Note, 2026-11-19: the developer timing code added with this job's closeout
(592a75f: `PSX_GPU_PHASES`, `DNTTK_SLEEP_VIEW_TEST`, `DNTTK_OBJ_RECT_STATS`)
is not on main since D23AO returned main to a139fa3's code; it is on the local
branch `shelved/d23al-d23an`. The measurements in note 511 stand.

### D23AJ - Field laptop campaign census at 30 and 60 new pictures

**Status: Done (2026-11-18, closed with D23AG; [note 509](../documentation/509-d23ag-overnight-chain.md) section 5). Dependencies: D23AG, D23AI.**

User, 2026-11-18: "D23AJ (laptop campaign census, SIMPLE and BALANCED at 30
and 60 new pictures a second)". The D23AF census (note 503) on the field
laptop, real 60 Hz screen, every level-select level, first and third
person, at GRAPHICS SIMPLE and BALANCED with FRAME RATE ORIGINAL 30 and
60 (D23AG's names). Goal: BALANCED at 60 new pictures a second.

Acceptance: per-level game rate, new pictures a second (median and lowest
tenth), fallbacks and slow seconds for the four configurations, with the
host state checked; the gap to the goal named level by level.

Work log, 2026-11-18 overnight ([note 509](../documentation/509-d23ag-overnight-chain.md)
section 5): four configurations, third person, all 21 levels, real 60 Hz
screen, host gate OK at every level. All seconds: SIMPLE 60 59.9 / 55.1 new
pictures (median / lowest tenth), redraws every second; BALANCED 60 47.5 /
29.5, 249 of 744 s in the governor's fallback; ORIGINAL 30 30.0 / 29.1 in
both grades; game 60 (lowest tenth 59.0-59.2) in all. Gap: 22, 27, 29
(worker-bound, 37-40), 6, 21, 24, 26 (lowest tenth 31-45) and the fallback
that follows them. First person at 60: BALANCED 59.6 / 32.1, SIMPLE 59.9 / 45.4,
no fallback; 22, 27, 29 short in both (29 at SIMPLE 41.8).

### D23AK - A lighter redraw worker

**Status: Done (user-accepted 2026-11-18, closeout authorized: "lock in the way the game plays right now, and then close us out"; the bundle is the Modernized default, [note 510](../documentation/510-d23ak-what-a-redraw-shares.md) section 7). Dependencies: D23AJ.**

MEASURED (field laptop, real 60 Hz screen, BALANCED, FRAME RATE 60): the
single redraw worker (D23Y6 placement; a second one starves the emulation
thread) runs at 92-96% on THE REAPER, levels 22 and 29, which show 37-46
new pictures a second; neither LTO nor PGO moves it. 149 stack samples:
about 20% inclusive in the title's host world walk (`host_vis_walk`,
`draw_distance_apply`, `near_clip_host_world_piece`), redone for every
redraw image, 17% self in other title helpers, 4-8% recording host
vertices.

Work: find what of the worker's walk depends only on the game frame (not
the in-between camera) and compute it once per game frame for all of its
redraws; exact (PSX_RT_VALIDATE, VRAM dumps), behind a developer switch
first. Goal: BALANCED at 60 new pictures a second on the laptop in the
worker-bound levels.

Work log, 2026-11-18 ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)):
worker profiles, census route, BALANCED 60, one level a launch, field
laptop real screen (27 / 29 / 22: 39.5 / 38.4 / 43.8 new pictures, worker
95.6 / 95.1 / 91.4%, 2,273-3,082 job samples) and the desktop (27). The
world walk is 30.0 / 33.6 / 17.0% of a job (not 20%), the object path
14.1 / 12.9 / 18.6%, the sleeping enemies 17.6% on level 22; `draw` and
`emit` are 84% of the walk. CODE: every step of the walk and of the
sleeping enemies' draw follows the job's camera (interpolated, and the
late camera per present); what only the game frame decides (records,
texture entries, the lamp surface lookup) is about 2% of a job, as D23N5
found at the AUTO grade. So no exact once-per-frame cut can reach the
30% the goal needs; nothing built. D23N5's restore (`PSX_RW_RESTORE=1`)
at BALANCED on 27 and 29, interleaved, two runs: within the spread.
Directions for the user (note 510 section 5): exact per-job cuts (perhaps
10-15% together), D23AI's picture trade-offs, or the game frame's own
image as one picture of two (not exact, D17B's judder).

Work log, 2026-11-18 later ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)
section 6): user, on the directions: "resoliution 1x is unacceptible so that
one is out", then "option 1 it is". Built, behind developer `PSX_RW_LEAN=1`
(off by default): batched host writes and reads for the title's packets and
mesh records and the replay's DMA walk (`psx_mod_write_words`,
`psx_mod_read_words`, `rec_words`), the host-vertex record without its hash
misses (same keeps and drops), a lean clip shape, `subdivide`'s early emit,
an inline exact `lround`, a per-thread settings cache in `ttk_live_env`.
Developer check `PSX_RW_LEAN_VERIFY=N` (each Nth job again with the cuts
suspended, streams word for word) and `DNTTK_LEAN_BISECT`. The check's
re-run now reads the bake half its first run pinned (it had differed at a
bake publish, as D23N5's had). Exactness: 3,840 checked jobs, 0 differing;
`PSX_RT_VALIDATE` 952 exact, 0 bad; VRAM dumps identical in Modernized UI 2
and Vanilla UI 9 (UI 9 Modernized's +400 follows host timing: it moves under
CPU load with the cuts off). Worker ms a finished job, two runs each, with
D23N5's restore: desktop 27 / 29 / 22 9.9-10.7 -> 7.3-8.9; field laptop real
screen 18.4-22.3 -> 15.4-19.4, new pictures 36-44 -> 38-46 (within the
spread). A fifth of the laptop's finished jobs miss their present; 60 needs
about 15 ms a job there. Left: the game's code 17.6%, the dispatcher's
lookups about 11%, the image load about 6%, the world walk's arithmetic.
The framework edits are exported and verified against a clean pinned
checkout; local CI passes.

Work log, 2026-11-18 evening ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)
section 6b): user: "keep pushing forward with it as far as you can ... I
think the dispatcher yes". The dispatcher's per-call chain is generated code;
its cold per-job caches were worth at most 0.2-0.6 ms a job (measured by
skipping the invalidation, not kept). The larger find: worker jobs ran the
accurate timing model's per-block hooks because D23F's fast model is leased
by the player update, which a worker never runs; `PSX_RW_LEAN=1` now runs
each worker job on the fast model (`ttk_fast_timing_job`), with the device
deadline lifted for the job (no device runs in a worker; the slow-edge path
2.6% -> 0.07%), and smooth normals take each face length once. Claim orders
(oldest, latest, earliest feasible present) were tried and dropped.
Exactness: 3,776 (desktop) and 1,536 (laptop real screen) checked jobs, 0
differing; `PSX_RT_VALIDATE` 964 exact, 0 bad; VRAM dumps identical
(Modernized UI 2, Vanilla UI 9). Worker ms a job: desktop 10.3-11.0 ->
7.5-7.9; field laptop 17.4-22.6 -> 12.9-15.7. Field laptop real screen new
pictures median / lowest tenth: 27 35.0-53.2 / 27.8-35.4 -> 57.8-59.5 /
38.1-38.7; 29 38 / 27-28 -> 42.2-52.5 / 29.0-32.0; 22 41.9-46.2 / 33.6-34.8
-> 51.4-54.0 / 31.8-33.5. The worker is 75-86% busy, no longer saturated; the
dips that remain come with the package-limited CPU clock and a busy
emulation thread. Needs playtest: the user's feel on the laptop with the
switches, and whether to make them the default.

Work log, 2026-11-18 night ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)
section 6c): user: "keep pushing, tackle the remaining dips too". The dip
seconds are the heavy views with all three threads busy on two cores at the
package-limited clock; D23Y4's placement had put the worker and the render
thread on one core. Developer `PSX_PLACEMENT_RENDER=side|emu|all`
(render_thread.c, default unchanged): `all` lifts the laptop's lowest tenth
on 22 / 27 / 29 from 30-38 / 44-53 / 31-33 to 46-59 / 55-59 / 37-51 (third
person; first person 27 from 23-26 to 40, 29 from 25-27 to 32-35); `emu` is
higher still but makes the governor fall back (18-19 s). The governor's 120
s first wait turned one heavy patch into two minutes of the original
presentation; with the existing `DNTTK_REPLAY_FLOOR_WAIT=15` the fallbacks
are 2-14 s. Campaign census, laptop real screen, BALANCED 60, all 21 levels,
the whole bundle: every level's median 59.5-60.0, lowest tenth 52.8-59.8
but THE REAPER 41.6, 70 of 749 seconds without redraws (note 509: 249 of
744). Not changed: the clear lens, the object path on hidden live frames,
the render thread's drawing. Proposed for the user's decision: the bundle as
the default (Modernized), the render placement and a 15 s first wait with it.

Work log, 2026-11-18 late ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)
section 6d): user: "keep pushing, fix the reaper dips and first person 29".
Admission of redraws that would miss their present: tried, within the
spread, removed. Developer `DNTTK_REDRAW_ONLY_OBJECTS=1`: on a hidden
redraw-only live frame a world object keeps every decision of the hook and
emits nothing (Duke's weapon, hand and body still draw: the shot's barrel
and pose counters); the emulation thread 4-13 points lower on the laptop
(desktop 48 -> 44%); THE REAPER third person lowest tenth 55-58 -> 58-59.
Clip planes' guard ratios once a polygon (exact, within the spread). In-worker
check, first person 27 and 29: 2,432 jobs, 0 differing. First person on 29
stays short (median 47-58, lowest tenth 26-35): both cores full at 2.3 GHz,
real work left; smaller exact cuts are below the runs' resolution. Needs
playtest, including the hidden-object skip (gameplay with objects, the
weapon's shots, the redraw-only switch-off after a stall).

Work log, 2026-11-18 closeout ([note 510](../documentation/510-d23ak-what-a-redraw-shares.md)
section 7): the user played the bundle on the field laptop (a desktop
launcher) and reported one drop when THE REAPER hit Duke; its log showed the
render thread behind (779 and 238 finished redraws not yet drawn at their
presents) and one 15 s fallback. Not reproduced (census route with 88-99
REAPER hits, test hits, a live per-second recorder while the user played;
the user could not repeat it): a possible recurrence for the user to report.
User: "lock in the way the game plays right now, and then close us out".
`run.py` sets the six switches as Modernized defaults
(`MODERNIZED_REDRAW_DEFAULTS`, overridable), none in Vanilla (checked in the
launched process); `test_redraw_defaults.py`; GAME_MANUAL's fallback
schedule (15 s first). Done.

### D23AL - Level 6 worker-bound dips on the field laptop

**Status: Cancelled (2026-11-19, reverted; [note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md)). Dependencies: D23AK.**

User, 2026-11-18: "absolutely! i want you to add it" (the hidden-object rule
as the named first step). Work, in order:

1. Hidden draws: in the object pass and the sleeping stand-ins, skip an
   object or stand-in whose bound lies wholly outside the screen rectangle of
   every shown room it can reach (its own room's, plus the rooms its box
   meets, as D17AZ's reach test), on the player's camera. The bound must
   cover the drawn mesh (from the mesh's own vertex extents, not the actor's
   radius); cover the enemy renderer 0x80032e78's calling convention too.
   Game-wide (no per-level data), Polymorph and extended draw distance only.
2. The stand-ins' own cost (about 0.5 ms each on the desktop game thread).

Acceptance: identical pictures (VRAM dumps and present shots off against on,
the in-worker lean check, `PSX_RT_VALIDATE`), measured skipped draws and
worker ms a job at UI slot 9 and level 6, new pictures a second on the
laptop's real screen nearer 60 there; Vanilla unchanged.

User laptop test (D23AI test 1, first person): level 6 (FAMILY JEWELS) fell to
the governor's fallback twice (15 s, then 30 s). MEASURED, field laptop, real
60 Hz screen, BALANCED 60, census route, D23AK defaults: first person median
57-59, lowest tenth 27-31; third person lowest tenth 35-36 (59 in another
run). In the slow seconds the redraw worker is 93-99% busy (31-60 jobs a
second), the game thread keeps 60 fields, the package swings between 23 W at
3.1 GHz and 15 W at 1.9-2.5 GHz. Worker profile: about 80% the game's own
generated code, host drawing 22-26%, objects 20-26%, world walk 6-10%; no
single hot spot; first person adds a little (its hook 16% against 9%).

User, 2026-11-18: "gold and guns as per slot 9 is also diabolical, it has
like 20 something unique frames". MEASURED, laptop, real screen, BALANCED 60,
first person, the lab's identical copy of UI slot 9 (census REPRO, 20-25 s):
standing, new pictures median 24.4-26.1 / lowest tenth 20.8-21.2 with the
game at 60 fields; the worker 96% busy finishing 50-55 jobs a second (about
19 ms a job; 7.5 ms facing away) of which 21-35 are shown in time; render
thread 57-66%; GPU about 85% busy, 10.7 ms a redraw (lit textured 9.3);
after about 13 s the package falls to 15 W (2.2-2.4 GHz) and the worker to
35 jobs. Turning: 60 a second facing away, 20-22 when the view returns.
Worker profile, 25,095 samples: world walk 29%, sleeping enemies
(`sleeping_draw`) 18%, objects 23%; `draw_shape` 37%, `emit` 26% inclusive.

Analysis of the slot 9 view (user: "analyzing that area in my statesave ...
if the end user doesnt see any physical difference im 100% here for it";
desktop, the lab's identical copy, note 511 section 9): a narrow mine
tunnel, not a vast area: 10 rooms, 48 world pieces, about 160 world
polygons a frame; about 93 object meshes a frame (about 640 polygons), 5
sleeping stand-ins drawn a frame at about 0.5 ms each (desktop game thread).
Developer switches (`sleeping_enemies.inc`, `host_visibility.inc`, off by
default): `DNTTK_SLEEP_VIEW_TEST=1` (view test for stand-ins in the game's
listed rooms: none culled here, they lie ahead down the tunnel), `=1r`
(also their room's screen rectangle: 5 -> 3 drawn a frame, game thread 2.7
-> 2.2 ms), `DNTTK_OBJ_RECT_STATS=1` (counts only): about 30% of the prop
renderer 0x80031d10's draws lie wholly outside their room's rectangle
(behind walls), in the game thread and the worker; the enemy renderer
0x80032e78 passes its camera differently (not counted yet). Not proven exact
(a radius-based box can be smaller than a long prop's mesh): the cut needs
mesh bounds or a validation pass.

Work: profile level 6's worker job with more samples (D23AK `lprof.sh`
route, both views), find exact cuts as D23AK did; exact (lean check,
`PSX_RT_VALIDATE`, VRAM dumps). Goal: lowest tenth near 60 on level 6.

### D23AM - One-second game stall with the console open

**Status: Monitoring (2026-11-18, waits for your report). Dependencies: none.**

User laptop session 2026-11-18 (D23AI test 1): on level 3, after standing
still about a minute, one second at 21.4 game frames and 21 presents, the
audio buffer drained (one underrun), while the console was open just before
`level 6`. Host idle (on AC, no backups, updates or timers, nothing in the
journal). Not reproduced on the desktop or the laptop's real screen: 70 s
idle, console open and close, `levels`, `fps`, `help`, `level 6` by the
debug port and typed with real key events (xdotool), 11 attempts, slowest
second 55.8, longest frame gap 86 ms. Reopen with a recurrence's log
(`recomp/analysis/play/logs/` on the laptop) and what was typed or pressed.

### D23AN - Reprojected in-between pictures

**Status: Cancelled (2026-11-19, reverted; [note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md)). Dependencies: D23AK.**

User, 2026-11-18: "absolutely! i want you to add it". In heavy views on the
field laptop the redraw worker finishes 50-55 pictures a second but only
21-35 reach their present (note 511 section 8); the others are shown as a
repeat. Work: when a redraw misses its present, warp the newest finished
picture to that present's camera with its host depth (a GPU pass), instead
of repeating it. Rotation needs no new information; small translations
leave thin disocclusion edges to fill. Duke's own layers (weapon, hands,
body) and the HUD are drawn unwarped (needs their separation from the
world picture). Polymorph only, developer switch first; Vanilla unchanged.

Acceptance: on the laptop's real screen, GOLD AND GUNS at UI slot 9 and
level 6 show close to 60 distinct pictures a second; turning judged smooth
by the user on the real display; warp cost per present measured (GPU
timeline); artefacts (disocclusion, moving enemies one picture behind)
documented and accepted by the user before any default.


Work log, 2026-11-19: D23AL and D23AN were built and tested on the field
laptop the same day (the hidden-draw rule exact but no laptop gain; the warp
raised the pictures a second, then each user test found a new visible
problem: the level 6 sky, smeared edges when turning, judder and edge
artefacts on slot 12's run and jump). After five fix attempts the user
reverted both: "maybe we revert and just see how it's looking for now. And
then carefully add features back in". The code and its notes (note 512) are
on the local branch `shelved/d23al-d23an`; the slot 12 ammo fix found on the
way is D23AP. See [note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md).

### D23AO - The frame rate never changes

**Status: Done (2026-11-19, user: "let's acually lock this work in and close out this sesh"). Dependencies: D23AK.**

User, 2026-11-19: "it should just never change framerate", then "we
maintain frame rate. let the game struggle if it needs to. we should be
focussed on optimizing those struggle points". Modernized launches set
`DNTTK_REPLAY_FLOOR=0` (no fallback to the original presentation) and
`DNTTK_REPLAY_SHED=0` (no 2nd/3rd-refresh steps), overridable from the
environment; Vanilla unchanged. Main returned to the 18 November build: the
code of a139fa3 (D23AK) plus D23AP; 592a75f's developer measurement code
(D23AI) is not on main.

Work log, 2026-11-19 ([note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md)):
built beside the main checkout on the field laptop as ef6c866, then + D23AP,
then + D23AK and the fixed frame rate, each played by the user. Playtest
(level 3, about four minutes, BALANCED 60, real screen): every whole minute
60.0 presents a second, no governor change, no first-person drop; the game
fell behind 11 times to 52.7-57.2 frames a second (D23AQ). User: "honestly
look at that. i didnt wanna stop playing. how good was that session!!!".
Desktop: Vanilla VRAM unchanged (69cd1d7f3e), Modernized VRAM 3da3a9be7c;
local tests 266 OK.

### D23AP - Objects in the room they stand in

**Status: Done (2026-11-19, user-tested on the field laptop). Dependencies: none.**

User, 2026-11-19: "if you load slot 12 ... the ammo disappears when you take
a step towards it". Found during D23AL (in every build, not caused by it):
the ammo box by the mine door has room byte 1 (beyond the doorway) while it
lies on this side; the renderer culls it against room 1's rectangle, which
shrinks to the doorway above the box as Duke steps closer. Fix
(`recomp/src/ttk/object_room.inc`, Modernized with EXTENDED draw distance,
developer `DNTTK_OBJECT_ROOM=0` off): when the object lies inside the box of
other shown rooms, the renderer gets a copy of its room entry whose rectangle
also covers theirs; host depth still hides what is behind their walls. Ported
on its own onto the 18 November build ([note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md) section 4).
User: "the obvious glitch is the disappearing ammo fix we already added in,
so i simply want you to add that back first".

### D23AQ - The field laptop's struggle points

**Status: Todo (2026-11-19). Dependencies: D23AO.**

With the frame rate held (D23AO), the field laptop's game itself falls
behind real time for a few seconds in heavy spots: 88-95% speed at nine level
3 spots in the user's playtest ([note 513](../documentation/513-d23ao-baseline-revert-fixed-frame-rate.md)
section 5, with Duke's positions; the worst 52.7 frames a second at 6222 4613
2305, with an audio underrun). Work: recreate each spot on the desktop (spawn
or teleport, the laptop's settings), profile the emulation thread and the
redraw worker there, and cut the cost exactly (identical pictures); confirm
on the laptop's real screen with the user's actions. Vanilla unchanged.

Note, 2026-11-19 (from D23Z, [note 465](../documentation/465-d23z-emulation-thread-spikes.md)
section 7): two more struggle points measured on the field laptop at the
player's settings (BALANCED 2x, FRAME RATE 60). UI slot 8's walk: the
emulation thread at 87-90% for about 15 s near Duke -282 5630 21198, game
seconds down to 45.3, in every run; the cost is spread over the whole update
(no plugin hook above 6.4 ms). UI slot 9: the first three seconds after the
load at 40-43 game frames in one run of three. Reproduce with the D23Z lab's
walk (`CASES=rates`, the player's profile).

### D23AR - GOLD AND GUNS top level in shot

**Status: Todo (backlog, user 2026-11-24; research in progress). Dependencies: D23AH; coordinate D23AQ.**

User, 2026-11-24, playing GOLD AND GUNS (level 3) on the field laptop with
the D23AH build: "for me that's been the heaviest going part. it's
interesting to note that it's not the lava pit that causes the slowdowns. if
duke is just looking over the ledge down at the lava pit, the frame rate is
consistently high. it's the top level, when it's in shot, that causes the
slowdowns in that area." The player's slot 8 (UI 9) is that view (copied
read only into the labs as `top08`). "here's an observation for a research
piece, and we can see what kind of solutions are available really. maybe
with an artifact. it can go into the backlog happily for now but you may do
a little research."

Work: measure the view against the lava pit looked down on (which threads,
how much: emulation thread, redraw worker, GPU), split the cost by DRAW
DISTANCE and redraws, and list the solutions available with their expected
gain, exactness and effort; publish the findings as a page.

Acceptance: a measured cost breakdown of the top-level view on the field
laptop and a ranked list of solutions for the user to choose from.

### D22P - GOLD AND GUNS skeleton key and the closed room in the lava hall (research)

**Status: Done (user-accepted 2026-11-20: "lock it in and close it out"; fixed by D08A48). Dependencies: -.** [Note 517](../documentation/517-d22p-gold-and-guns-skeleton-key.md).
Source: the user's long playthrough on 2026-11-14 (spoken commentary, tidied into a list; the user's own words are kept where they matter).

User, 2026-11-14, UI slot 10 (the 13:42 save, GOLD AND GUNS): the mission
items list shows a skeleton key for this level, but where it is and what it
does is unclear. There is also a room at the far end of the long lava hall
with a switch visible inside and no way in found. Walkthroughs did not
explain either.

Research: the level's mission items and how the inventory maps them; whether
the key really exists here, where it is obtained; the room, its switch and
what triggers it; whether the DeLorean secret is linked; compare with the
original on DuckStation (portable copy only) for a recomp regression.

Test save (2026-11-15): a private copy of UI slot 10 (runtime slot 9, the
2026-11-14 13:42 save, GOLD AND GUNS) with the player's cards, in
`recomp/analysis/d22p/cards` (local, ignored; hash-checked identical when
copied). Test on this copy; the player is free to save over their slot.
Do not invent a route or change progression. Acceptance: a written answer
(how they work, or that they are unused or shown wrongly); a fix job only if
a real bug is confirmed.

User update, 2026-11-20: the room behind the gate is solved, and the key is
not for it. "you drop down through the entrance in the lava pit and then
through the room at the end of the corridor, you can access the other side of
the gate, pull the lever and open it - thus proving this mystery key isnt used
for that." The open question is now the key alone: "why there is a mystery
key in the inventory that seemingly has no need to collect? i dont know where
the lock is for this key."

Refocused research (the gate route above is the user's, not to re-derive):
1. The level's mission list: which item the key is (number, name, icon), why
   the mission inventory lists it for GOLD AND GUNS (the game's own per-level
   item table, or our D08A5 mapping), and whether it shows as "NOT FOUND YET"
   or found in the user's save.
2. Where the key is: any pickup object of its type in the level (position,
   room, reachable or hidden), or none at all.
3. What uses it: every object whose handler wants that item (mode 2, as the
   jewel wheel and the card readers, D08A18), in the level overlay and the
   shared lock helper; or none.
4. The original: the same checks on the original game (DuckStation portable
   copy only) to tell a recomp regression or our listing from the retail
   design (a leftover from cut content is a valid answer).
Acceptance unchanged: a written answer with evidence labels; a fix job only
if a real bug (ours) is confirmed, for example our mission list showing an
item the level never uses.

Work log, 2026-11-20 (research only, no code changed; note 517):
- Answer: GOLD AND GUNS has no skeleton key and no lock for one. The game's
  item-name table names item 6 "Skeleton Key" for level 3, but nothing gives
  it and nothing takes it: a leftover, most likely cut or unbuilt content.
  No hidden room or pickup behind it.
- Disc: the pickup dispatcher gives item 6 only for types 153/542/744/862;
  LEVEL03.OVR has no lock-helper call, no CreateObject call, no key type and
  never touches item 6, the hand or the use item; every other level with a
  working key calls the lock helper.
- Run (private offscreen, `level 3` fresh start and the user's slot 10 copy;
  player files unchanged): 378 placed objects, their contents and 52 enemy
  records hold no key; no object has the lock flag `0x01000000`. The item-6
  lock call seen in RAM is stale LEVEL02.OVR code past LEVEL03's end.
- Ours: the retail Select screen shows a mission item only once found, so
  it never shows this key; D08A5's mission inventory lists every named item
  and shows 0/1 forever. Fix job D08A48 (game-wide rule, no level data).
- Survey of all mission-item levels: level 3 is the only one with no lock;
  level 2 item 7 and level 6 item 8 have no giver at the start (suspected
  leftovers, for D08A48 to settle).
- Not done: the original in DuckStation (no debug port; the static evidence
  is from the disc's own bytes, and object placement was observed through the
  original loader in the recomp only).
