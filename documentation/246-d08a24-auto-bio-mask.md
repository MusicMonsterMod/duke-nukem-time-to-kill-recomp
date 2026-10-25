# D08A24 - Automatic and instant Bio Mask

Status: Done, user-accepted 2026-10-25. User selected
D08A24 and UI slot 11, then explicitly requested instant use while shooting.
User: "lock it in, close it out". Implementation, tests, licensed edited audio,
manual and documentation closeout, commit and push are explicitly authorized.
Accepted executable (revision 2): `bc3828d8fc5b2a2cfaf3d82edd92ebda1d382124f4f8a9fafcdbdaddaa1e48b4`.

## Behavior

AUTO BIO MASK ON (default) automatically activates an owned, charged mask before
native gas damage. B and inventory Enter toggle it instantly, including during
fire, movement and weapon transitions. No hand equipment, upper-body animation,
weapon request or camera mode is changed. OFF preserves original manual use.
Vanilla always receives OFF.

The accepted implementation automatically removes the mask after three consecutive
native player updates with no head exposure, stopping charge drain and playing
one removal cue. A brief boundary dip resets on renewed exposure instead of
chattering; entry activation remains immediate. Manual B use in clean air still
works until a gas entry/exit cycle. Native charge drain while worn is unchanged. Manual removal in gas suppresses
automatic reactivation until the native head exposure clears. Selecting Modern
night vision instead also suppresses it. Loading a state starts a fresh exposure
check: an active gas save stays protected without replaying a cue; an active
clean-air save removes after the exit grace with one cue. Saved charge is retained. Empty or
unowned masks grant nothing. Native depletion still removes the mask. Neither
water nor other damage types activate it.

The persistent in-game row is OPTIONS > ADVANCED > ITEMS > AUTO BIO MASK.
CONFIRM applies it live. Schema 52 adds auto_bio_mask on/off with migration and
backup; run.py --auto-bio-mask on|off and terminal settings M share the setting
through DNTTK_AUTO_BIO_MASK.

## Native evidence and implementation

The owned executable's 800409e0..80040af4 reads the room gas flag 0x40,
queries native gas volumes, updates the body-joint exposure mask at player+0x88c,
and tests the head joint from [player+0x40]+0x34. It dispatches damage type 12
only when that head bit is set. This is not a room-number or water-depth guess.

Duke's existing hooked damage handler 800a40a8 receives the victim in a0 and
damage type in a3. Its jump table 80018aec selects 800a4494 for type 12. That
branch blocks gas damage with a charged active Bio Mask, retaining native
protection and other damage rules. The new callback requires the native area caller 80040ad0 and head exposure,
so other sources of damage type 12 do not auto-equip a mask. It runs before that code;
it changes native item/model endpoints, not damage amounts or health.

The mask's equip/removal endpoints are 8004e894..8004e94c: face equipment +0x854
is 2; the head joint mesh comes from [800ddb18]+0x1030, with the original head
restored from [[800ddb14]+4]. The active bit is +0x35c bit 1 and charge is +0x35e.
Goggles are exclusive: replacing them clears +0x360 bit 1 and +0x224 bit 0x80000,
without spending their charge or playing a goggles toggle cue. Dynamic pointers
are validated before any writes. Native pending gadget actions are left alone.
The already-guarded 800420bc charge handler owns drain and depletion.

Added owned-executable guards cover mask endpoints, gas-area code and the damage
jump table. Existing single-player identity, gameplay and live-player gates apply.
No new hook registration in game.local.toml, generated-code edit, framework edit
or dependency pin change was needed. Existing framework edits remain untouched.
DNTTK_BIO_MASK_TRACE logs native damage types, amounts, callers, health, mask
flags and head exposure for diagnosis.

## Requested sound

User supplied CreatorsHome's Gas Decompression MP3 and authorized editing as
needed. The edited 0.55-second cue retains the gas burst and decay, removes the
quiet tail, filters 100-7000 Hz, uses 3 ms / 35 ms fades and limits peaks to
-6 dBFS. No pitch or speed change. Actual RMS is -26.89 dBFS. Source/license,
hashes, exact processing and the reproduction command are in
[the audio asset notes](../recomp/assets/audio/README.md).

Successful automatic activation/removal and manual B/Enter on/off play one cue.
No cue on empty/refused use, held repeats, native depletion, pause or restoring
an active mask in gas. A loaded active mask in clean air produces a real removal
transition and one cue after the exit grace.
The existing host mixer follows effects volume, runtime fade and master volume;
no SPU/game audio state changes. A shared gadget voice replaces the current cue
on rapid switches, without a queue. Night vision's accepted PCM is unchanged.
No additional framework changes were necessary.

## Verification

Private scripts, copied slot 11, cards and logs: recomp/analysis/d08a24/ (ignored).
The live harness uses Xvfb, real keyboard/mouse events, OpenGL and dummy audio.
UI slot 11 is file slot10. No writes go to the player's saves, cards or settings.

The route contains enemy fire as well as gas. Trace type 2 from caller 8006febc
reduces health before exposure and with the mask worn; type 12 from 80040ad0 is
blocked immediately after automatic activation. Health loss from enemies must
not be mistaken for a gas-protection failure.

Early harness failures were wrong ammo offset, a two-byte charge write racing
native drain, and treating captured input as proof that a debug-loaded paused
save was resumed. Corrected probes inspect native pause fields and write the
high charge byte first. Goggles coexistence tests explicitly supply charge only
in the private test state; slot 11 does not start with goggles available.

Build, native controls/input/aim/audio suites and all 104 launcher/profile tests
pass. The controls test checks the actual shortcut and damage callbacks,
including other actors/types/callers, no head exposure, missing/empty item,
invalid model, pending native action, dead player, OFF/Vanilla, manual suppression,
re-entry and load-generation reset. Audio tests verify exact PCM, effects gain,
mute, pause/load/reset, feature gates, rapid restart and face-cue replacement.

The final 120 FPS smoke verifies the strict native gas-area caller/head guard,
activation during held fire, and one cue on automatic activation/manual removal.

Live 60 FPS third-person and 120 FPS first-person routes pass: B and Enter during
held fire, ammunition decreasing, one toggle per held key, automatic protection,
manual-off suppression, re-entry, active save/load, pause/resume, native depletion,
empty refusal and goggles replacement. The menu route switches OFF and ON live;
OFF retains native equip/removal and takes gas damage without auto-activation.
Menu and both-view screenshots were inspected.

The sound route captures host PCM on activation/removal and with effects muted.
The first 0.2 seconds of the cue, high-passed at 1 kHz for analysis to reject
ambient low-frequency transients, has matched gains 0.355/0.329 before filtering
and 0.165/0.164 after filtering. Muted filtered gains are below 0.005.
Unfiltered correlation alone has substantial ambient false positives and is not
used as mute evidence. Native mixer tests independently prove zero cue at mute.
This verifies the audio pipeline, not the user's listening/mix preference.

All 40 player save/card files retain original hashes and mtimes. Diagnostics use
private writable cards/settings. The existing framework patch is unchanged.

The user accepted the behavior and sound. For regression checks, use slot 11,
walking into the gas while
firing, then B off/on and inventory Enter. Compare AUTO BIO MASK OFF if desired.
The diagnostic route is one campaign location and costume, not a full campaign
playtest or audio listening assessment.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

## Revision 2 - automatic gas exit (2026-10-25)

User reported that the mask stayed on after leaving gas and requested automatic
removal. The existing normal player-update hook now observes native head
exposure. Three consecutive clear updates remove an active mask via the same
validated instant endpoint as B, without holster, pose or weapon changes.
The damage hook observes entry only; it cannot count a second dry update.
Exposure and exit grace reset on loads, and OFF clears the automatic tracking.
Pending native gadget actions are allowed to finish before retrying removal.

Native controls tests verify boundary jitter resets the grace, three dry updates
remove once with one sound, remaining charge and weapon/pose are unchanged,
and renewed gas exposure activates again. Private slot-11 routes at 120 FPS
first person and 60 FPS third person cover three crossings while firing,
steady exposure without repeated cues, no charge drain after removal, manual use
in clean air, active clean-air save/load removal, and active gas save/load followed
by removal on exit. Logs: exit-controls.log, exit120.log, exit60.log.

No audio asset/mixer, framework, settings schema or dependency changes in this
revision. The source save/card manifest was refreshed before copying slot 11;
the player's files were checked against this revision's manifest. The user
accepted the exit timing and authorized closeout.

## Acceptance and closeout

User: "lock it in, close it out". D08A24 is Done. No post-acceptance gameplay
changes, builds or launches. Existing verification and campaign-coverage limits
above remain. Runtime patch export is unchanged and matches all 51 modified/new
files when applied to clean pinned framework 08ec704a974b1f3a16335b4afeb340b9eff19926.
Dependency pins and player data remain untouched. No next job started.
