# D35K1 - Slot-5 door perspective and right-leg kick

2026-10-25. Done (user-accepted); broader D35K event inventory remains Todo.

## Accepted closeout

User explicitly instructed "lock it in, close it out!" after playtesting the
door perspective fix and right-leg polish. This bounded result is D35K1 Done;
it does not mark the broader D35K scripted-event inventory complete.
Implementation, tests, manual and documentation are included in the authorized
MusicMonsterMod commit/push. No post-acceptance gameplay edits, rebuilds or
launches; existing framework modifications and pins are preserved.
Historical candidate wording below records the pre-acceptance evidence and
limits, not outstanding acceptance of this delivered result.

## Right-leg presentation polish

The user confirmed the first-person fix works, then requested a visible
right-leg kick while opening the door. The revised candidate reuses the
existing quick-kick leg keys, current-costume right-leg meshes, thigh treatment
and viewmodel rendering. It places the leg on the right with inward yaw.
The normal quick kick keeps its existing placement and behavior.

Presentation reads native channel 0's previous/current keyframes and ticks
left in pose 116, clip 600. The six native keys map to ready, ready, chamber,
strike, retract and rest in the existing kick pose. The original clip has
60/50/50/50/50/50 ticks and the strike key carries event 0x800. Read-only model
renders confirmed the raised-leg and extended-boot keys before implementation.
The initial blend from the preceding idle frame is excluded until both keys
belong to the door clip. No host timer, new replay state or save format is
needed. Pause/load and redraw use the native animation state already restored
by the runtime. Malformed/out-of-range clip frames are refused.

This is presentation only: it never starts the quick-kick attack, calls its
damage sphere, plays its sounds, or replaces the native door callback/events.
Native tests cover all six keys, interpolated chamber/strike, right-side
placement, unchanged original joint matrices, invalid frame rejection,
action Third Person and Vanilla exclusion, and no attack calls. Existing
quick-kick tests still pass.

Revised executable:
`4f8a04989350f79c087aef6e106a4f3b81b03828245692c294c40df316cbec69`.

Polish validation: player build and controls/input/aim/near suites pass.
Private 60/120 routes show the right-leg boot extending and retracting (captures
inspected), 92 leg-joint draws each, and zero quick-kick attacks/hits/sounds.
Action Third Person, global Third Person and Vanilla saved-action completion
show zero added leg draws. Pause at the strike and load of that private action
resume and complete correctly. Diagnostics are stopped; user framing/comfort
playtest remains. No commit/push or acceptance closeout.

The polish reused the original private slot-5 copy. The earlier source manifest
now differs in slot 5's save/sidecar/thumbnail and writable-state.log: all four
source mtimes are 09:33:56 UTC, before the polish build (09:37:53) and diagnostic
launches (from 09:39:09). The stale-manifest comparison therefore failed and is
not claimed as polish non-modification proof. Diagnostic commands used only
private card/settings paths; no player save or setting writes were issued.
The unchanged-source statement in the earlier validation section below applies
to the initial perspective-fix delivery, before the user's intervening playtest.

## Selected scope and cause

The user selected D35K with current UI slot 5 as a concrete example: opening
its door returns Duke to third person. This pass investigates and fixes that
reported interaction. It does not claim the full per-level special-camera
inventory is complete or reopen accepted D35M/N behavior.

The private copy is file slot 04, LEVEL00.OVR (authenticated tag 3). Starting
at approximately (7868, -6393, 79763), walk toward the door for about two
seconds, stop and hold E. The native finder selects object 0x801d8f04,
type 233/kind 20, at (6628, -6848, 79117). Simply pressing E at the saved
position only stows/redraws the weapon. Early sideways approaches missed the
activation point and are not evidence of successful interaction coverage.

The reproduced action uses lower/upper pose 116 and committed-action bit
0x40, in mode/history 0/0. Camera owner 0x800d7198, actor back-pointer
0x800d6eb0 and pivots 0x800d7954 remain reciprocal and normal. Death bits and
script inhibit bit 0x20000000 are absent. The baseline sampled 21 pose-116
frames at blend 0 with unsupported-camera, then recovered first person.

Read-only resident disassembly and the live overlay callback table map kind
20 to 0x80092734. Type metadata selects branch 0x800515e0, which chooses pose
116 at 0x800515e4; 0x80051730 installs it. The native interaction callback
starts the door action through 0x80091130. The existing completion path at
0x80051740..0x80051780 clears the committed-action state and returns native
weapon handling. This is an ordinary use-action presentation gap, not a
confirmed alternate-camera dispatch or cinematic ownership bug.

## Implementation

Added only pose 116 to modern_controls.cpp's existing use_interaction_eye
predicate. USE INTERACTION VIEW controls it, with the existing stable eye and
native interaction-hand rendering. Global Third Person and Vanilla retain
their policies. The action flag, mode/history, identity, death, script and
camera-ownership gates remain; locomotion and firing permissions are not widened.
No new hook, schema, generated C, callback mutation or framework change.

The native transient-action tests now include pose 116 for eye presentation,
action lockout, view choice, pause/input suspension, load reseeding, replay
restoration and death/script/foreign-owner rejection. Adjacent pose 117 stays
unsupported. The player manual includes hinged-door actions in the existing
setting.

## Validation

Player build and controls/input/aim/near-clipping suites pass. Controls include
the owned executable and all 21 selectable overlay fixtures. Live private
slot-5 sequences at 60 and 120 each captured 20 pose-116 samples, all at blend
1. Native action completion returns to idle and redraws the weapon. First-person
action and completion captures were inspected.

Action Third Person and global Third Person comparisons each captured 20
pose-116 samples at blend 0; the action choice restores the selected first
person afterward. Pause/resume and loading a private mid-action save retain
first person through completion. A Vanilla load of that same native action
completes at blend 0. This verifies saved-action completion in Vanilla, not
fresh Vanilla activation of the door. Immediate-load samples in this run
already show the active pose; no wider load-timing guarantee is inferred.

All diagnostic sessions are stopped. All 40 source player-save/card files
retain their original hashes and mtimes. Tests used private profile files;
no player settings were written. Existing framework modifications and both
dependency pins remain untouched. No framework export was needed.

Candidate executable SHA-256:
`e8c065ed61fb9cb44d0205a933ce4a52d34aac7050f4ed8cfe39aa7070deccd7`.

Private scripts, copied saves, traces, screenshots and source hash/mtime
manifest are under `recomp/analysis/d35k/`. Input is ordinary X11 keyboard
input; no teleport, forced animation or guest RAM edits. Live read samples
are sequential, not atomic snapshots or performance measurements.

## Remaining scope

User framing/comfort and familiar-route playtest remain. Broader D35K work
still needs per-level scripted/cinematic event mapping, including boss/event
views and endings; this one ordinary-door route does not establish that
coverage. D35 remains Needs review. No next job, acceptance closeout, commit
or push is authorized by delivery of this candidate.

Launch: `python3 recomp/tools/local/run.py`. Use current slot 5, global First
Person and USE INTERACTION VIEW First Person; approach the door and press E.
