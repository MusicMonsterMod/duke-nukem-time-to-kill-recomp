# D08B/D08D — traversal suspension and apartment secret

## Delivery implementation — 2026-09-27

The user closed their game and authorized resumption. Testing uses isolated
settings/cards, private Xvfb, dummy audio and logged fixtures. The current source
contains three bounded D08B repairs and the requested D08D convenience. Broader
D08B swimming, other maps, security-card and campaign routes remain unfinished.

### Traversal ownership, restoration and recapture

- Attached WASD/E eligibility now requires the existing gameplay-context gate.
  A native regression failed before the fix for frontend/menu/suspension contexts.
- E-owned temporary holstering survives pause/inventory/suspension. Previously
  `interaction_alive()` reused the active-gameplay predicate, canceling restoration
  during a pause despite the input module intentionally retaining it. The native
  context assertion fails before the fix and passes after it. The lifetime check
  now retains identity/player/death/frontend guards without granting any input or
  draw permission. Actual redraw still requires active, settled normal gameplay.
- Original-owned ladder/ledge states can offer automatic capture from the existing
  authenticated player-animation update (`8005a210`, player, table `800c2754`,
  caller `80041b34`). This uses verified traversal state/identity independent of
  an already captured input frame. It does not modify attached motion/camera or
  expand the animation whitelist. Explicit F10 opt-out is respected. Native cases
  cover inactive-frame bootstrap, wrong caller, pause rejection and opt-out.

The apartment delivery replay verified armed approach, automatic E holster,
pausing during the ladder route, ascent/exit and original pistol redraw (equipment
2, ammunition 200, no manual C). Log review then found that the old exploration
helper had used F10 once on an attached resume. That route is NOT no-F10 evidence;
it motivated the separate recapture repair and a replay with fallback disabled.
The attempted descent on the intermediate candidate entered exit animation before
S was applied; it is NOT descent coverage.

### Apartment condition change

Modernized patches two authenticated live LEVEL00 instructions through
`psx_mod_write_code_word`, keeping executable-RAM invalidation:
`(level_flags & 0x408) == 8` becomes `(level_flags & 0x400) == 0`.
Only the conversation prerequisite is removed. The original already-open test,
flag write, bed scheduler, sound, secret counter and pickup behavior remain.
Conversation flag 8 is never written by the production patch. Vanilla does not
install it; profile selection occurs at launch.

Installation runs only at an authenticated normal player-camera callback with the
exact resident/overlay identity and LEVEL00 index. The identity reader canonicalizes
only the exact two-word replacement pair for comparison to the original overlay
SHA-256; every other word is checked unchanged on every invocation. Half-patched,
foreign/corrupt code is rejected. The pristine pair can be installed again after a
loader restores the original overlay. Native checks cover Vanilla, wrong level,
wrong caller, pair rejection, reload and the retained one-shot condition.
No generated C, original disc or player cards were modified.

### Apartment evidence and fixture boundary

The original-condition baseline reached the room using SDL movement, killed both
guards by normal firing, talked, then used the switch. Live callback tracing binds
kind 4 to the woman and kind 24 to the switch. Flags became 0x408, the bed moved,
and secret count became 1.

The modified apartment replay used the same kind of natural traversal and combat.
The untouched switch-first approach was not completed: attempts were blocked by
the NPC/positioning or missed the switch. After natural conversation moved the NPC,
one explicit private fixture reset only conversation bit 8 (0x8 -> 0x0), leaving
the unopened flag and secret counter untouched. This is a prerequisite-negative
fixture, NOT proof of natural no-conversation access. Its exact frame/address and
purpose are in `recomp/analysis/d08b-progression/conversation-fixture.json`.

Subsequent real E input invoked the original switch. With bit 8 unset, flag 0x400
was set, the bed moved from Z 7741 to its settled 8765, and the original “Found a
Secret Area” message appeared. Repeated light toggles retained count 1 and bed
position; dialogue afterward set bit 8, leaving flags 0x408 and count 1. These are
state/capture checks with dummy audio, not listening evidence.

Walking behind the moved bed naturally acquired five pipe bombs (slot 12
unowned/0 -> owned/5); pressing 6 selected slot 12 with all five remaining. There
were no inventory grants or bomb throws. This supports that specific D08A mapping,
not every weapon/era or empty detonator case.

The apartment run exited 0 on SHA-256
`3c5285043806dc79b48f327721179e8c15e66ba8878ecba64444d971b7ddf8e4`.
The later recapture-only change does not alter the secret implementation. Final
capture/Vanilla results and binary identity are recorded in the delivery report.

### Excluded evidence and remaining work

The intermediate `d08b-apartment-candidate` died before the secret test. Its old
tracking helper stopped at enemy health 2 even though the enemy was still alive;
this was corrected to require the original death flag. The exploration driver
now rejects player death, verifies actual pause globals between operations, logs
operations and fallback capture repairs, and can require automatic capture.
No successful process exit overrides these semantic failures.

D08D remains Needs playtest for the user's untouched switch-before-conversation
route. D08B remains In progress: swimming, broader ledges, other maps, security-card
progression and campaign transition coverage are not closed. The final strict
recapture replay verifies a real first-ladder descent (Y -10493 -> -9871 in the
sampled interval), bottom exit and pistol redraw, with no F10 capture log or
manual C. It also resumes attached climbing after pause. A later re-ascent
stops at Y -10964, animation 186/state 3/3 despite held W and SIO Up. Reviewed
capture shows a pig cop standing on the upper landing: obstruction is plausible,
but the exact cause is not established. That upper exit is not a pass.
The earlier apartment replay does verify the upper exit on its recorded binary.
See [delivery evidence](reports/d08b-d08d-delivery.json) for exact run identities.
No exact cause of the historical security-card freeze is newly established.

## Historical first-pass checkpoint (superseded by the delivery above)

## Current scope

The user selected autonomous work on the next job in order (D08B) and requested
that Modernized lights-off open the apartment bed secret regardless of conversation
order (D08D). Both remain In progress. The player's game was active throughout
this investigation; it was not stopped, modified, attached to, or used as a fixture.
No player executable rebuild or gameplay replay has occurred in this pass.

## D08B source fix and native reproduction

`traversal_input_ready()` checked Modernized capture, equipment, actor/camera,
animation, matching traversal states and resident/LEVEL00 identity, but omitted
the existing `gameplay_context()` gate. Native fixtures keeping attached state
while switching frontend/inventory/suspension globals reproduced acceptance of
traversal and E outside live gameplay. The added assertion failed before the fix.

The adapter now requires `gameplay_context()` just as normal movement does.
Three fixture cases cover frontend mode (`800bcbb0`), inventory/pause (`800be568`)
and suspension (`800d2540`), then verify restoration of eligibility when each gate
returns to gameplay. Existing native movement, jump, camera, interaction, shortcut,
crouch and guard checks pass. This establishes the adapter contract defect and fix;
it does not claim the player observed input leakage or that actual paused ladders
have been replayed. No motion, collision, animation or camera algorithm changed.

Evidence: `recomp/analysis/d08b-progression/native-controls.log` and
`python-tests.log`. Full Python suite with owned-disc integration: 57 tests passed.
Native target compilation passed. The player binary remains the accepted
`92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.
The source fix is not yet in that executable.

Next: after the player closes their game, build the player through the normal
workflow and run isolated real-input checks for ladder descent, interruption,
exit/recovery and security-card progression. Expand the matrix only with observed
results. Swimming, other overlays and campaign coverage remain unverified.

## D08D original-code finding

The exact owned LEVEL00 overlay SHA-256 is
`f38747adab56fa69347a1b8488a1a25e093a1f428232250d299fd50266568508`.
Its `800cc14c` interaction dispatcher selects by object byte `+34`.
Candidate kind 4 sets bit `8` in `800dd878` and requests an actor animation.
Candidate kind 24 toggles object byte `+35` and darkens room/object colours.
The lights-off branch at `800cc580` requires `(flags & 0x408) == 8` before it
sets `0x400`, places the object from `800d257c` on handler 43, schedules it,
plays sound and increments the player's secret count at `+3b0`.

This is strong static evidence for the user's reported conversation-before-lights
gate, not yet live confirmation that these candidate objects are the apartment
woman/switch/bed. [Structured research](reports/d08d-secret-research.json) records
the boundaries. Original media, generated C, guest code and trigger flags are
unchanged. Do not set the conversation bit globally as a shortcut: that would also
change dialogue eligibility. Preserve the already-open guard and secret count.

Next isolated reproduction: approach without dialogue, snapshot object identity
and flags, switch off, switch on, talk, switch off, and observe the moving object
and secret counter. Then implement a verified Modernized-only condition change
and compare both orders, repeat toggles, already-open state and Vanilla. No fix
for D08D is delivered yet.

## Final delivery verification

Final binary `a2822c303d1ce95ef82d274a564a91d02392cffae8f570367fe2ec91875a4caf`
passed the 14-checkpoint Vanilla route (`d08b-d08d-final`); firing, jumping, inventory
and turning captures were reviewed. All game/probe processes exited. Final source
and executable hashes match the delivery report; recorded player profile/card/state
hashes remain unchanged. No additional campaign, natural switch-first or upper-exit
acceptance is implied by this regression.
