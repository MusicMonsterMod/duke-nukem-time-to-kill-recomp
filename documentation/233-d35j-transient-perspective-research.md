# D35J - Transient first-person perspective research

## Accepted closeout - 2026-10-25

The user accepted the delivered combined result and explicitly authorized
closeout ("lock it all in and close us out"). D35J bounded research and
D35J1-J3/D35M/D35N are Done. The evidence below records the earlier candidate
and its coverage limits; pending-acceptance wording describes that delivery,
not the current status. No broader damage/hazard or campaign acceptance is
implied. The final combined executable is
`78687162fae6d941cf1beb9245d8687c7467c994214783a97cc5f127b2702d09`.


2026-10-25. Needs review. Research candidate, not gameplay implementation or acceptance.

## Baseline and method

User authorized autonomous tests and supplied current player UI slots 8 (key),
9 (crystal), 10 (NPC) and 11 (Miner 69er lever), plus idle-animation reports.
Private copies use file slots 07-10. The source hash/mtime manifest and all raw
traces, scripts and screenshots remain local under `recomp/analysis/d35j/`.
No player cards are used. Separate writable cards, profile, Xvfb window and
loopback debug port 9276 isolate the diagnostic sessions.

The executable is the accepted D35I build:
`6b2b0ec48fa56c3fbd3a0a5aefac23dcb111ea4c95d9e66010f989e5408a2d42`.
No build, generated-C edit, title/framework implementation change or dependency
pin change is part of this investigation. Existing framework modifications are
preserved. Tests use Modernized, independent camera, first person, OpenGL and
60 presentation setting. Results do not establish high-refresh or full-campaign
coverage.

Sampling reads actor lower/upper animation, mode/history, flags, health,
camera owner/pivot/back-pointer and host first-person diagnostics. Requests
are sequential live reads, not atomic frame snapshots: occasional transition
samples pair a new animation with the prior camera diagnostic. Conclusions
below use sustained intervals and recovery, not a single boundary sample.
The initial `intake.json` field named `health` mistakenly reads +0x1c4, a terrain
field, and must not be interpreted as health. Subsequent traces correctly read
+0x32; native health units are hundredths of a HUD point.

## Reproduced cases

| Case | Natural input route | Observed transition and recovery |
| --- | --- | --- |
| Idle fidgets | Slot 10, tap holster for 250 ms, enable Caps Lock run mode, stand for 40 seconds | Lower/upper 63 -> 66, 65 and 64 in successive fidgets; first-person blend falls to zero with `unsupported-camera`, then returns to one on idle 63. |
| Native held inventory | Slot 10, hold Scroll Lock for 4-5 seconds, release | Lower 63/upper 5 stay unchanged; +0x224 gains 0x200, view falls back until release clears that bit. Recovery restores first person. The native weapon-selection UI is visible in the captured third-person image. |
| NPC interaction | Slot 10, E | Weapon stows, lower/upper 277 runs with action bit 0x40, view falls back, idle 63 and the drawn weapon return. Reproduced again when an attempted shot near the NPC activates the interaction; that attempt is not damage evidence. |
| Key and door use | Slot 8, approach the lock, E (held 1.2 seconds in the repeat) | Upper item poses 47/48 retain first person while lower is 63. The subsequent full-body lower/upper 266 loses it. Idle and weapon redraw recover normally. Item presentation and door-opening presentation are distinct stages. |
| Miner 69er lever | Slot 11, walk forward about 0.35 seconds, E | Lower/upper 118, +0x224 action bit 0x40 and transient 0x2000; fallback during the action, recovery to idle and drawn weapon. Later presses on the already-used lever do not reproduce a second operation. |
| Crystal use | Slot 9, approach, short forward jump, settle near the circular structure, hold E | Native finder confirms type 857/kind 7 at `0x801de724`. Upper item pose 47 keeps first person; lower/upper 266 then loses it, followed by idle and weapon redraw recovery. |
| Survivable explosive damage | Slot 11, back away about two seconds, `dnweapons`, disable the save's god mode with `dnkroz`, select RPG, aim down, fire | Health falls from 15000 to 3037 (150 to about 30 HUD health). Lower 63 persists; first-person blend remains one in sampled damage/recovery. No knockdown or perspective failure reproduced in this event. |

Across the sustained reproduced fallbacks the selected view remains first
person. The ordinary camera still owns Duke: camera+0xa4 is `0x800d7198`,
actor+0x7d4 is `0x800d6eb0`, and both pivots are `0x800d7954`
(actor+0x7bc). Mode/history remain 0/0, actor death bits are clear, and the
native alternate-camera bit 0x20000000 is absent. Thus these cases do not
support a special-camera ownership failure or a lost saved preference.

## Source-confirmed mechanisms

`recomp/src/ttk/modern_controls.cpp:state` permits idle 63 and enumerated
movement/traversal poses; it does not permit lower 64-69, 118, 266 or 277.
Those reproduced lower poses therefore fail `camera_eligible`, causing the
normal-camera hook to release first person with `unsupported-camera`.
This does not justify adding an entire numeric range to the movement lease.

The same function's land inhibit mask is 0x20000200. The held native inventory
sets 0x200 without changing lower animation or camera owner, and fails that
separate test. The existing swim exception is presentation-specific and is
not proof that a land inventory exception is already safe.
`hold_button_sampled` documents the native hold threshold and release history.

Read-only resident disassembly confirms:

- `0x80053038` owns the idle timer at actor+0x240. Input resets it to 3000;
  it counts down by the native tick amount. At expiration the selector chooses
  66 or one of 64/65/67/68/69, depending on its counter/random selection.
  Only 64/65/66 were naturally observed in this run; 67-69 remain source-backed
  candidates, not individually playtested poses.
- `pc_input.cpp:input_pad` supplies the original Walk bit in default walking
  mode even while stationary. Private idle samples had pad 64511 and the
  timer stayed at 3000. Caps Lock run mode produced neutral pad 65535 and a
  decreasing timer, enabling the reproduced fidgets. The early 45-second
  walking-mode idle waits are not evidence that fidgets are safe. The first
  short Scroll Lock tap also missed stowing; the later 250 ms tap verified it.
- The native interaction selector around `0x80051544..0x8005173c` calls the
  target object's callback and chooses action poses by type metadata. Its
  branches explicitly select 277 at `0x800515d4`, 266 at `0x800515dc`, and
  118 at `0x800515fc`. Native completion around `0x80051740..0x80051780`
  handles restoration and clears action bit 0x40. These are committed actions
  in the ordinary camera, not evidence for a global script-bit override.

## Ownership and implementation boundaries

D35J1 records idle-fidget presentation, D35J2 held-native-inventory
presentation, and D35J3 the reproduced NPC action. These are Todo, not started.
The NPC case is not a confirmed D35K special view.
Key/door and lever evidence belongs in D35M's use-interaction work. The shared
native selector is useful evidence, but does not establish that every callback,
item consumption event, alignment movement or camera policy is identical.

Future fixes must separate presentation eligibility from movement, firing,
interaction and capture permission. Preserve native input suspension during
inventory, committed interaction timing, item consumption, actor alignment,
callback count, weapon stow/redraw and recovery. Verify interruption/load,
first/third-person settings and Vanilla before accepting an implementation.
For idle, test canceling the fidget with movement without granting unrelated
hurt/action poses. For inventory, preserve selection and release semantics
without adding a redundant view toggle. No DAMAGE REACTION VIEW setting is
justified by the single damage event observed here.

## Explicit coverage limits

All five user-supplied cases are reproduced. Crystal use required a short
forward jump to reach the activation position (about 5083,7811,46241). A
read-only scan located the type-857/kind-7 candidate; the native finder then
confirmed it at actor+0x174 during the natural route. No teleport, object
callback injection, animation forcing or guest RAM patch was used. Earlier
failed E attempts are retained as failed attempts, not evidence of safe use.

Natural enemy-hit variants, actual knockdown/stagger, heavy-fall damage,
water hazards and armed fidget variants remain unverified. The RPG case is
one survivable explosion, not comprehensive damage acceptance. Death and
respawn remain D35L. No unsupported claim of all-level fidelity or absence
of other transient gaps follows from these tests.

## Validation and disposition

Private diagnostic sessions are stopped. Source save SHA-256 and modification
times match the intake manifest. No player save/card/settings writes, commit
or push occurred. The board is Needs review for this bounded research result;
it is not marked Done. D35M retains the key/door, lever and crystal use work.
The damage/hazard coverage limits above remain explicit follow-up research
within D35J unless the user accepts this bounded result. No fixes have been
implemented, and no other job has been started.
