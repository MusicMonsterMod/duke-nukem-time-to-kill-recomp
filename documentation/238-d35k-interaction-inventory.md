# D35K - Interaction inventory, toilet and alternate-camera callers

2026-10-25. Done, bounded research accepted with D35 closeout.

The user accepts the delivered first-person work and closes D35. D35K2 toilet
use is also Done; note 239 records its implementation and acceptance. The
remaining unmapped routes below are retained as leads for future reported
problems, not active jobs or a claim of complete campaign coverage. The original
research result below predates that implementation and acceptance.

## Scope and result

User authorized autonomous interaction research starting with the current
slot-5 toilet, building on the accepted NPC, door, mirror and crystal work.
This pass confirms a new toilet presentation gap, decodes the shared action
selector, and resolves direct alternate-camera entry calls in three overlays.
It is not a claim that all campaign interactions have been played.
D35K1 remains accepted; D35K2 records the proposed toilet fix without starting it.

Baseline executable SHA-256:
`4f8a04989350f79c087aef6e106a4f3b81b03828245692c294c40df316cbec69`.
Existing framework edits and dependency pins are untouched. No build, gameplay
source edit, generated-code edit, commit or push was performed.

## Method and evidence limits

Private copies of all 40 local-play files, private settings, Xvfb, dummy audio
and port 9277 isolate the runs. Local scripts, RAM snapshots, logs and captures
are under `recomp/analysis/d35k-inventory/`. All source save/card hashes and
modification times match the intake manifest after diagnostics stop.
Read-only native disassembly uses the owned executable and local extracted
LEVEL overlays. No teleport, forced pose or guest-memory write was used.

Live samples are sequential reads, not atomic frame snapshots. Sustained action
intervals and completed recovery support the conclusions. RAM scans across all
12 private save slots provide candidates only: matching bytes can belong to
unrelated data. A scan hit is not proof of a live object or reachable interaction.
The complete 1062-entry type-table classification is retained separately.
Current slot identities supersede older notes' descriptions of those slots.

## Confirmed toilet gap

Current UI slot 5 is LEVEL00's toilet, not the earlier door checkpoint.
Press E from the saved position; the initial baseline also held W for two
seconds against the fixture. Native finder confirms object `0x801dc4c4`,
type 80, kind 16, position `(13119,-11264,10812)`. Duke starts near
`(12688,-11774,10828)`.

Weapon stow precedes lower/upper pose 275 and committed-action bit `0x40`.
The first-person blend becomes zero with `unsupported-camera`. Throughout
sampled pose-275 intervals the ordinary owner is `0x800d7198`, both pivots
are `0x800d7954`, back-pointer is `0x800d6eb0`, mode/history are 0/0 and
alternate-camera bit `0x20000000` is absent. Native completion restores idle
63, redraws the weapon and recovers first person. Captures visibly show Duke
using the toilet in third person.

| Run | Pose-275 samples | Result |
| --- | ---: | --- |
| Modernized first person, 60 | 72 | Sustained fallback, then recovery |
| Modernized first person, 120 | 71 | Same fallback and recovery |
| Modernized global third person, 60 | 69 | Native action completes in third person |
| Vanilla, attempted native pad activation | 0 | Activation not reproduced; no Vanilla action-completion claim |

At 100 HUD health, these runs do not establish healing behavior. Native kind-16
callback `0x800928b8` dispatches type 80 at `0x80092968`; its event-3 branch
`0x80092a4c..0x80092a6c` checks object+0x35, sets that byte once and calls
`0x80096cac` with argument 5. A future presentation fix must preserve that
callback and its one-time effect, not synthesize a new use action.

The type table gives type 80 interaction class 8. Selector `0x800513e4`
reads the class at type-record+4, branches through `0x80015850`, selects
275 at `0x80051630`, and installs it at `0x80051730`. Completion at
`0x80051740..0x80051780` clears the committed state and handles weapon return.
Current `use_interaction_eye()` does not admit 275. This is an ordinary
presentation gap, not evidence for relaxing special-camera ownership guards.

## Shared selector inventory

These are source-confirmed mappings, not new gameplay acceptance for every type.
Classes index the type metadata; they are distinct from an object's kind byte.

| Class | Selected pose(s) | Current disposition |
| --- | --- | --- |
| 2, 3 | 277 | Accepted NPC/use presentation; individual callbacks still vary |
| 4 | 266 | Accepted use presentation |
| 5 | 116 | Accepted hinged door, D35K1 |
| 6 | 267/268; 131 in underwater mode 5 | Land crystal accepted; underwater trigger/appearance unverified |
| 7 | 270 | Unmapped gameplay variant; candidate types 206, 546, 776, 892, 1001 |
| 8 | 275 | Type-80 toilet reproduced; type 179 also has class 8 but is not validated as a usable toilet |
| 9 | 269 | Selector branch exists; no class-9 entry in the sampled full type table |
| 10 | 118 | Accepted lever presentation |
| 11, 15 | 278 | Accepted mirror presentation |
| 12 | 176 | Candidate type 368; already camera-eligible generally, exact interaction unverified |
| 13 | 279 | Candidate types 838, 926; exact interaction and perspective unverified |
| 14 | 277 plus metadata mutation | Existing pose support; special callback semantics not newly tested |
| 16, 17 | No selected full-body pose | Selector advances its state; not proof of no callback camera effects |
| 18 | 266/268 by action subtype | Poses supported; exact item/callback variants unverified |

Do not whitelist the whole numeric pose range. In particular, 269/270/279
need object identification and natural activation before any behavior change.
A private slot-7 mirror route naturally produced 278 at 120 and retained
first person throughout sampled action frames. E at current slots 6 and 11
produced no full-body action; those attempts are not coverage evidence.

## Alternate-camera entry callers resolved

An aligned direct jump/call and literal-address scan of the executable and
25 local LEVEL overlay files found three direct entry call sites. Surrounding
instructions confirm these are code calls, not data coincidences. This scan
does not prove absence of computed/indirect entry or other camera mechanisms.

All three dispatch on object+0x34. Their event-3 paths call `0x8003b36c`
with alternate camera `0x800d7010 + player_index*0xb0`, the player, and a
level-specific camera object. Event-0 availability reads object+0x35.

| Overlay | Callback | Object subtypes | Entry call | Camera-object globals |
| --- | --- | --- | --- | --- |
| LEVEL00 | `0x800cc14c` | 39, 40, 41 | `0x800cc258` | `0x800d2608`, `0x800d2604`, `0x800d2600` respectively |
| LEVEL05 | `0x800cd878` | 17, 18, 19 | `0x800cdda0` | `0x800d25bc`, `0x800d25c4`, `0x800d25c0` respectively |
| LEVEL09 | `0x800cd20c` | 11, 12, 13 | `0x800cd8e4` | `0x800d25bc`, `0x800d25c0`, `0x800d25c4` respectively |

LEVEL00 slot-5 RAM contains plausible type-858/kind-1 objects for subtypes
39/40/41 at `(3086,-6777,36032)`, `(3085,-6777,36861)` and
`(3086,-6777,37696)`. The referenced camera objects are type 722. These
coordinates are route targets, not confirmed event names or live activation.
Do not label them turrets, monitors or cinematics without observing them.

Native entry `0x8003b36c` sets script bit `0x20000000`, assigns the alternate
camera target/owner from the supplied object and sets object+0x35. Exit
`0x8003b474` clears the bit, sets player bit `0x20`, restores the player's
camera through `0x800286b8` and clears the object's byte. Resident direct exit
calls are at `0x800427d0` and `0x80043580`. Exact user input/timing and visual
meaning of these nine event variants remain unverified.

## Review and next work

- D35K2: bounded toilet first-person presentation proposal. Reuse USE
  INTERACTION VIEW, preserve committed input restrictions, callback timing,
  one-time effect and weapon recovery. Verify first/third choices, pause/load,
  60/120 and a successfully activated Vanilla comparison before acceptance.
- Continue D35K mapping for classes 7/9/13 and the underwater class-6 branch;
  identify objects and natural routes before naming fixes.
- Trace the nine alternate-camera event subtypes above in gameplay. Classify
  intentional observation/cinematic views versus controllable gameplay, and
  verify their exits. No SCRIPTED GAMEPLAY VIEW option is justified yet.
- Level openings, boss/event views, endings, indirect calls and two-player
  ownership remain explicit coverage gaps, not certified by this pass.

D35K is Done by user acceptance of this bounded research batch. The proposed
toilet work above has since shipped and been accepted as D35K2. Other research
leads remain dormant until selected; no next job is started.
Player-facing behavior is unchanged, so no manual change is needed.

Launch: `python3 recomp/tools/local/run.py`.
