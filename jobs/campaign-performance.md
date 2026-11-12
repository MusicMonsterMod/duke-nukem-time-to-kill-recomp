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

**Status: Accepted, closeout not yet authorized (user, 2026-11-11: "I accept the D23K report and approve its jobs"; closeout selected with D50A and D23O part 1; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23J.** User
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

**Status: Needs playtest (first step 2026-11-11, [note 448](../documentation/448-d23m-direct-redraws.md); [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23K.**
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

### D23N - Captured redraws: replay a recorded frame instead of re-running game code

**Status: In progress (D23N1 done, D23N2 inventory done 2026-11-12, notes 454-455; N0 measured, note 449 section 7; design corrected before building, [note 449](../documentation/449-d23n-captured-redraws-research.md) section 6; research and design 2026-11-11; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23K.**
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

**Status: Todo (inventory done 2026-11-12, [note 455](../documentation/455-d23n2-what-a-later-job-changes.md)). Dependencies: D23N1.** Rebuild a later job of the same game
frame from the first job's record, with that job's camera and blended
poses: rooms first, then fixed props, then actors (model-space corners and
the joint or transform each hangs from, the GTE matrix recomputed exactly
as the game does). Acceptance: host packets identical word for word to the
later job's real stream; differences in the game's own packets listed (they
are D23N3's).


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

### D23N3 - The game's camera-dependent packets

**Status: Todo. Dependencies: D23N2.** About 1 in 10 of the game's own
packets change between the jobs of a frame (one semi-transparent quad and
about two textured quads a job, note 449 section 7). Identify their guest
routines and give each a host equivalent (or run that routine alone).
Acceptance: rebuilt later jobs identical word for word, game packets
included.

### D23N4 - Later jobs skip the game's code

**Status: Todo. Dependencies: D23N3.** Switch the rebuild on: the first job
of a frame captures (with a culling margin wide enough for the late
camera's turn within the frame, or falls back), the capture is shared
between worker processes, later jobs rebuild instead of running the game.
Acceptance: pictures identical (`PSX_RT_VALIDATE`, the comparison mode kept
as a developer check), worker CPU at 120/180/240 Hz measured against D23N's
baseline (about 1.5 cores at 180 Hz in UI slot 2), fallback to the full job
whenever the capture does not cover a job, Vanilla unchanged.

### D23O - Leaner render-thread submission

**Status: Needs playtest (part 1 accepted 2026-11-11; part 2 2026-11-11, [note 447](../documentation/447-d23o-render-threads.md); [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23K, D23N.**
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

**Status: Needs playtest (CPU part 2026-11-11; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23K, D23M.**
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

### D23R - Parity benchmark and acceptance

**Status: Todo (D23K plan, last; [note 444](../documentation/444-d23k-eduke32-parity.md)). Dependencies: D23L-D23P, D41G, D41F2.**
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
