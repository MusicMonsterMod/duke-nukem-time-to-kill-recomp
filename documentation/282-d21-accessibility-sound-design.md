# D21 - Reworked sound menu and HELD CROUCH toggle

Date: 2026-10-27. Status: Done, user-accepted; closeout authorized.

## 2026-10-27 - D21 accepted and closed

User accepted the completed SOUND menu and HELD CROUCH behavior and explicitly
authorized closeout. D21 is Done. SOUND provides Master, Sound Effects, Music,
Speech and stereo/mono mode with confirm/cancel/defaults and persistence.
C toggles crouch; Ctrl or Space cancels it and stands Duke up where there is
room. The cancelling hold is consumed; fresh Ctrl crouches and fresh Space
jumps. Both actions remain configurable. No accessibility panel was added;
VIEW BOB remains in GAMEPLAY and Vanilla keeps its existing play behavior.

Build, native SDL regression, 118 Python input/profile tests and private live
menu/input checks passed. User acceptance supplies the final playtest decision.
Note 282 retains the test evidence and historical iterations. D18 remains Done
as accepted research; its separate implementation jobs are unchanged.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.

Earlier candidate and Needs playtest statements below are historical.

## Selected scope

User narrowed D21 to the redesigned SOUND menu and configurable C crouch.
The user subsequently clarified HELD CROUCH: C toggles crouch on/off, while
Left Ctrl remains momentary. The initial duplicate held-key interpretation
was incorrect and is superseded by the toggle correction below.

The earlier accessibility panel proposal is withdrawn. No new accessibility
screen, menu text sizing, subtitles or moved view-bob setting.
VIEW BOB stays in GAMEPLAY. D18 is complete as accepted research.

## Sound menu

Retain the accepted OPTIONS layout and TTK menu rendering. Replace the native
SOUND page with a matching recomp-owned page containing, in order:

- MASTER: expose the existing host master volume.
- SOUND EFFECTS: existing native effects level, including modern gadget cues.
- MUSIC: existing native music level.
- SPEECH: existing native speech level.
- MODE: existing stereo/mono choice.

Use existing menu navigation, sliders and confirm/cancel semantics. Persist
confirmed values through the established settings owner. No new mixer or
concurrent voice/music policy is selected; D18B remains separate.

Preview values are examples, not the player's settings. The HTML changes only
its local state and produces no audio. Its CONFIRM/CANCEL buttons demonstrate
draft versus confirmed values. The final game should use its own font/art.

## Source findings and implementation considerations

- `recomp/src/ttk/game_menu.cpp`: SOUND delegates to the native screen;
  the other settings pages use draft/apply-live/atomic persistence. Reuse this.
- `recomp/psxrecomp/runtime/src/main.cpp`: host master gain already exists after
  fade, with runtime get/set and persistence. Check both output paths and avoid
  double gain. The runtime settings owner differs from title control settings.
- `recomp/src/ttk/night_vision_audio.cpp`: gadget PCM uses native effects gain
  at guarded address 0x800c39e0 before host master volume. Preserve this routing.
- Note 166: existing custom sliders use three original SOUND slider objects in
  guest RAM for high-refresh replay safety. Four volume sliders require checking
  storage/packet lifetime; do not blindly reuse one mutable slider object.
- `input_bindings.def`: crouch currently defaults to Left Ctrl.
- `pc_input.cpp` and `tools/local/pc_input.py`: C is currently reserved for
  holster binding, although captured Modernized play suppresses its native
  Circle input. Both validators and migration need deliberate updates for C.
- Add an independently rebindable secondary crouch action without shifting
  stable action IDs. Combine held inputs through the existing crouch owner;
  releasing one while the other stays held must not stand Duke up. Check all
  direct primary-binding readers, not just the main action query.
- Preserve a player's custom C binding; do not steal it during migration.

The findings above guided the implementation below; audible acceptance remains open.

## Implementation acceptance

Verify title/pause SOUND at 60/120, all four sliders, MODE, confirm/cancel,
mouse/keyboard navigation and existing OPTIONS layout. Verify correct channels
at 0/50/100, native effects and gadget cues, both audio output paths, persistence
through restart and private save/load, and no clipping/underrun regression.
Separate measured routing from audible listening evidence. Preserve Vanilla.

For secondary crouch, test C press/hold/release, Left Ctrl, overlap, rebinding,
unbinding, migration with custom C, pause/capture and native menu Circle behavior.
Reuse existing crouch mechanics, including ceiling and landing behavior.

## Artifacts and limits

Current fragment: `recomp/analysis/d21-design/sound-menu-preview.html`.
Standalone linked preview: `recomp/analysis/d21-design/preview-standalone.html`.
The earlier preview path also contains the revised sound-only design.
Preview syntax/readback checks passed during design. The game candidate below
supersedes the earlier design-only status. No commit or push authorized.


## Initial implemented candidate (C behavior superseded below)

The SOUND page uses the existing menu row, navigation and confirm/cancel
mechanisms. Its first three sliders reuse the native objects; its fourth has
an independent DMA-addressable allocation made at initialization, before replay.
All four render in the inspected pause 60/120 and title 120 captures.

Native SOUND gain globals are 0x800c39e0 (effects), 0x800c39e4 (music),
0x800c39e8 (speech), 0x800c39ec (mode; 0 stereo, 1 mono). Volume conversion is
integer percent * 16383 / 100, matching the original handler. Music confirmation
uses the original music-preview setter 0x8006acec; speech follows its original
stored-level behavior. Added exact executable guards for the native handler and
setter. This does not implement simultaneous music and speech or a new mixer.
Master uses host_volume_get/set; it is not an extra multiplicative gain.

Schema 53 adds optional sound preferences (-1 means keep the existing setting
until configured) and the appended stable crouch_alt action. Confirmed sound
levels are restored after save loads. Master writes preserve the newly confirmed
value before its pending runtime application. Existing runtime/numpad master
changes synchronize back to the same menu-settings persistence path.

HELD CROUCH defaults to C, uses the existing held-crouch action and is independently
rebindable/clearable. Both validators accept C for crouch; migration keeps a
custom C binding and leaves HELD CROUCH unbound if occupied. Existing Ctrl behavior,
including its contextual dive/descent use, is shared by the secondary binding.
No crouch mechanics, animations or camera policy changed.

## Verification

- Player build and movie/geometry shard checks pass. Candidate SHA-256:
  `7b82d7e8fbc03a7411200df44c956d3264ced6bce8273504aceace2cb2d89a60`.
- 104 profile tests and 14 input/profile migration tests pass. Tests cover
  schema-52 migration with custom C/crouch bindings, sound roundtrip and invalid
  payload atomicity. Existing expected schema numbers/wire lengths updated.
- Native SDL input suite passes, including secondary hold, overlapping Ctrl/C,
  rebinding and unbinding; existing context and Vanilla input checks still pass.
- Private pause-menu 60/120 routes: visual inspection of four sliders,
  confirm/cancel, defaults, native gain values spanning 0/50/100, stereo/mono,
  Master persistence payload and restoration after loading an older save pass.
- Private title 120 route: SOUND opened through the native Cross shortcut;
  all four sliders visible. A short Enter pulse in the offscreen steering
  harness did not enter OPTIONS; this is not evidence of a player Enter defect.
- Restart with configured master/effects/music/speech/mode: menu shows
  0/50/100/0/mono and native gains match. Vanilla retains the private save's
  native gains instead of applying Modernized preferences.
- Real SDL input under private Xvfb: C enters crouch animation 178 from idle 63;
  overlapping Ctrl keeps the crouch after C release; final release stands up.
  The initial offscreen menu harness deliberately disabled capture and could
  not validate held gameplay input, so the real SDL route replaced that check.
- Real SDL numpad +/- changes persist Master as 5 then 0 in the menu sidefile.
- Repository hygiene and diff whitespace checks pass. No new framework edits;
  pre-existing submodule changes preserved. No player save/card writes.

Artifacts: `recomp/analysis/d21-sound/`, including private harnesses and captures.
Audio drivers in these automated runs were dummy: actual listening, balance,
clipping/underrun acceptance and full movie/voice transition coverage remain
for playtest. No claim of full campaign regression coverage.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.

## User review - binding label

User accepted the audio settings as perfect and requested the secondary crouch
binding label HELD CROUCH. Renamed the menu label and help text; behavior and
saved binding identifier remain unchanged. No acceptance closeout, commit or
push requested.


## HELD CROUCH correction - user clarification

The user intended a static toggle, not another momentary Ctrl binding. C now
flips a host-side crouch latch on each non-repeated press; releasing C does not
clear it. The next press releases the request, subject to existing standing
clearance. Ctrl ORs its momentary request with the latch, so turning the toggle
off while Ctrl remains down does not stand up until Ctrl is released.

The existing crouch_alt saved binding identifier is preserved. Keyboard and
mouse rebindings work; the label remains HELD CROUCH and its help explicitly
says to press again to stand. No sound implementation changed in this correction;
the sound menu is user-accepted.

Pause/focus changes preserve the latch. Load, initialization, changing either
crouch binding, loss of the live player/Modern control context, swimming
(surface or underwater), jetpack and unsupported traversal clear it. A C press
in swimming/jetpack cannot start a persistent descent request. Ctrl retains
its existing contextual behavior. Existing crouch animation/collision owners
consume the combined intent; no animation or collision code changed.

Verification: player rebuild and native SDL input suite pass. Added cases for
release retention, second press, repeat suppression, Ctrl overlap, pause,
load/death, swim/underwater/jetpack/traversal cancellation, key rebinding,
unbinding and mouse rebinding. Fourteen Python input/profile tests still pass.
Private real-SDL gameplay with a copied save and DNKROZ isolation verifies
C tap/release retention, second-tap stand, momentary Ctrl, pause/resume, overlap
and load reset. The initial unprotected route reached animation 214 during the
pause check; it was not used to infer an input regression. No player saves/cards
touched. Corrected build SHA-256:
`58103f1b4ca8a32b7ceee522bd58a7c84159898183df372dd58e4eb06350a633`.

C behavior remains Needs playtest; no closeout, commit or push authorized.

## Ctrl cancellation follow-up

The user accepted the toggle and requested Ctrl cancel it. A fresh press of
the configurable CROUCH action now clears an active HELD CROUCH latch and
suppresses momentary crouch for the rest of that hold. Releasing and pressing
again restores normal momentary crouch. Existing standing-clearance checks
still apply. This supersedes the earlier overlap description for a fresh Ctrl
press while the latch is active; no sound or collision code changed.

Native SDL regression covers cancellation, continued hold, key repeat, release,
next-press crouch and a rebound mouse button. Player build and fourteen Python
tests pass. Private real-SDL gameplay with copied cards/save and DNKROZ isolation
verified C toggle, Ctrl cancellation, staying upright throughout the cancelling
hold, next-press momentary crouch, pause preservation and load reset. No player
saves/cards touched. Candidate SHA-256:
`b2677119331a777ddbdb630dcd3d2f57702268bdeb89827ddee233a83122c5af`.
Cancellation is ready for user playtest; no closeout, commit or push.

## 2026-10-27 - D21 Space exits HELD CROUCH

Space now cancels C's crouch toggle and stands Duke up where there is room,
without jumping. The cancelling jump hold is consumed until release; a fresh
press jumps normally. This follows the configurable JUMP binding. Ctrl's
previous cancellation remains intact. SOUND is unchanged.

Player build, native SDL regression and fourteen Python input/profile tests
pass. Private real-SDL gameplay verifies standing idle throughout the consumed
Space hold and jumping on the next press, alongside C/Ctrl/pause/load regression.
Ready for user playtest; no closeout, commit or push.
Launch: `python3 recomp/tools/local/run.py` from the workspace root.

The jump press clears the crouch latch and buffered jump request, suppresses
held jump in both host intent and guest pad routing, and resets consumption on
release/load/rebinding. Existing clearance handling still owns standing up.
The live check used private copied saves/cards, dummy audio and DNKROZ isolation.
Standing animation 63 persisted during the consumed hold; a fresh Space press
left idle for the jump. Native tests cover buffered/held jump suppression,
repeat, release and the next press. Candidate SHA-256:
`9bb17ff978819456f5e59961365598cc99ae4b19c50803bc29eed0d2b802d058`.
