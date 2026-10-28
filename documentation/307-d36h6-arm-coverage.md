# D36H6 - Remaining arm coverage and cold unarmed restoration

2026-10-28. Done for the user-accepted repair and coverage pass. The user
explicitly authorized lock-in and closeout. Broader coverage limits remain below.
Evidence labels: S source/native tests, L private runtime observations,
A previously accepted behavior, U user visual acceptance.
Baseline: [contact-arm design](305-d36h5-contact-arm-rig.md),
[interaction coverage and repair guide](306-d36h5-interaction-arm-coverage.md),
and the [D36 ownership audit](257-d36-first-person-presentation-audit.md).

## Confirmed defect and bounded repair

L: a fresh 60-Hz launch loading a private unarmed ground save exposes the
upper-arm ends when looking down. Drawing a weapon, holstering it and loading
the same private save removes the exposure. Both captures have equipment 0,
Ground perspective, FOV 90 and pitch 1.047198. The baseline images are
`recomp/analysis/d36h6/cold-down-{cold,warm}.png` and their state sidecars.
This is a session-history inconsistency, not a contact-rig defect.

S: empty-handed Ground/Committed/Fall presentation lowers all six native arm
segments by 120 camera-down units. That existing D36H4 path required a nonzero
`hidden_matrix` without allocating it. Weapon draws and replay initialization
normally reserve that scratch, masking the omission. A direct 60-Hz unarmed
load can precede both. The path now acquires the same 32-byte scratch on its
first eligible draw, as the accepted contact path already does.

The accepted pose, amount of lowering and native animation remain unchanged.
This does not introduce a new unarmed rig or convert equipment/swim/flight to
contact arms. Native gameplay matrices, camera freedom, FOV, weapon grips,
collision, attacks and save formats are unchanged. Allocation failure retains
native arm admission without dereferencing a missing buffer. Third person,
reflections, Vanilla and the displaced-eye rejection retain their existing gates.

S: the new actual-hook regression forks fresh children before any weapon test
can allocate scratch. Both costume joint orders exercise all six arm segments,
the exact shared translation, scratch reuse and unchanged native matrices.
The corrected fixture fails against the unfixed source at the first arm's
unchanged matrix pointer and passes with the repair. Existing controls, input,
aim, near-clipping and animation-save restoration suites pass.

L: the rebuilt player's fresh-load comparison restores the accepted lower
framing immediately. `cold-before-after.png` compares the same saved scene;
`cold-fixed60-*` and `cold120-matched-*` retain fresh/warm captures, state,
executable identity and replay reports. The original private input is preserved
in `cold-cards`; each subsequent run uses a separate writable copy.

Additional fresh/warm checks cover FOV 60 at 4:3/60 (`cold43`) and FOV 90 at
21:9/120 (`cold219-matched`). These are sampled combinations, not the entire
FOV/aspect matrix. The corrected 120 harness waits for the actual pitch to
reach 1.04 before releasing look input and capturing, rather than assuming a
fixed wall-clock hold reached the intended angle during startup.

## Coverage discipline

Local evidence and reproduction scripts are under `recomp/analysis/d36h6/`.
Runs use private settings/cards, Xvfb, dummy audio and continuous video where
motion is surveyed. Player files are compared by hashes and modification times.
Captured state reads are sequential, not atomic. Requested 120-Hz presentation
is not a measured performance guarantee; recording and debug readback add cost.
Dummy audio cannot establish sound quality.

The survey checks actual equipment/record bytes, lower and upper animations,
health, action owner, camera/FOV and upgrade mask. Group 1 first selects the
unarmed boot; its stale weapon byte must not be counted as an equipped pistol.
Pipe-bomb recovery can be equipment 1/item 0, and dynamite recovery includes a
real empty-hand interval. A requested input or a successfully exited script
does not establish that its named animation occurred.

L: the initial base-weapon 60-Hz pass observes all fourteen records 1-14,
including separate knife/axe, all three throwables and the crossbow. It retains
idle, look extremes, crouch, firing/throw input, recovery and short movement
sequences at FOV 90. Inspected views retain connected visible chains and the
accepted weapon framing. Pipe bomb transitions to the remote, and dynamite
returns through an empty-hand phase. This is a bounded first-map survey, not
all costumes, every animation frame or every supported projection combination.

L: holstered ground sampling confirms 63, 78, 178, 97 and 105; flight confirms
163/164 under the Jetpack owner. The sample named `empty-land` is still pose 97
and is not evidence of completed landing. No new flight pose policy is inferred.

L: `upgrades120-rest` completes the six-upgrade survey with mask 0x3f checked
after every load. Ordinary selection keeps base held IDs 4/5/7/8/9/10 while
the native resolver uses the upgrade flags; requiring held IDs 27-29 here
would misclassify valid upgrades. The 120-Hz empty-hand/flight repeat also
confirms equipment 0. Replay reports have zero failed captures/jobs or worker
deaths; this is lifecycle evidence, not a cadence benchmark.

L: `challenge120` travels through the native level-select route to challenge
stages 23/24/25 and verifies directly equipped records 28/29/27. Idle, look
sweeps, attack/recovery and crouch inputs are recorded with actual states.
Inspected frames preserve the accepted support grips and framing, including
the large twin-barrel RPG. No native weapon-record replacement is used.

L: `items120` reuses identified archived card/crystal routes. Both sample
upper 47/equipment 1, committed Use 266 or 267 while still carrying the item,
then equipment 0 contact arms. The keycard reader changes red to green and
the weapon returns. Crystal insertion completes; removal 268 is not sampled
by this pass. These samples retain the equipment/contact ownership boundary
and do not establish every mission-item model or every callback variant.

L: `swim60` and `swim120` use two archived natural swimming states, with actual
equipment 0 and poses 122/133/127/128. Forward/strafe/look, first-to-third-to-first
switches and private save/load complete at both settings. Each saved input is
copied to a separate `preserved-swimN` directory before restoration. The step
named `surface` still samples swimming and does not establish a completed
water exit. Inspected dark underwater views and retained videos are bounded
evidence, not all lighting/effect variants. The 120 replay report records three
workers, zero failed captures/jobs and zero worker deaths.

Baseline executable SHA256:
`b84fac116fcae3a2b03c615cc7d06ba4db9fc1b3c81d3231fa100bd356d09b68`.
Accepted executable SHA256:
`134c98ee73f2224768cda39d36e3c43b94113c4154fde5524fe8e0df8fbb68c1`.

## Excluded attempts and remaining obligations

The first smoke harness repeated a cheat by placing its reset and command in
the same input frame; it was stopped and excluded. Corrected input writes one
cheat/action token per frame. An initial semantic holster/recenter attempt did
not invoke the physical bindings; its supposed empty-hand samples actually
held a pistol and are excluded. The final base pass uses actual Z/V bindings
and checks equipment. An upgraded 120-Hz pass stopped when god-mode setup was
not observed after a load; completed samples remain identifiable, and later
groups require a confirmed setup before proceeding.

Earlier fixed-duration 120-Hz cold comparisons reached different look angles
and are excluded from matched-image claims. One ultrawide warm preparation did
not finish holstering within its fixed wait; the final comparison draws the
weapon to warm scratch, then loads the preserved unarmed state directly and
checks actual equipment/pitch. This is a harness correction, not a claim of a
new gameplay regression.

An initial native-test invocation used an already-patched old LEVEL00 fixture
and failed before the tested behavior. The canonical original-word fixture is
`analysis/pc-input/d07-final-camera/level00-guard-fixture.bin` beneath `recomp/`.
The first cold-test fixture also triggered the displaced-eye guard because its
synthetic pelvis was not beneath the camera; the corrected fixture isolates
arm lowering and produces the intended before/after failure.

U: the user accepts the significant quality improvement, describing the weapons
as superb and brief costume testing with handheld items and interactions as
superb. This is acceptance of the delivered result, not exhaustive campaign
testing. Broad all-costume, lighting/gas/power-up,
all held-item and camera-script combinations remain unverified. Rare selectors,
callbacks and alternate-camera events listed in note 306 still need established
natural routes and actual owner identification before implementation changes.
The isolated overhead 120-Hz save/load halt remains unexplained; this ground-arm
allocation repair is not claimed to fix it. Preserve any future failing private
save before repeats. D36H6 must retain these limits after this bounded repair.

## Accepted closeout

U: "lock it in, close it out" explicitly authorizes the implementation, tests
and documentation commit/push as MusicMonsterMod. Existing build/native/runtime
results stand; no post-acceptance gameplay edit, rebuild or launch. No framework
source or pin change, generated-C edit or original-media write. Future reports
start with note 306 and preserve the actual failing private state. D36J3 stays
Todo; no next implementation job is selected.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```
