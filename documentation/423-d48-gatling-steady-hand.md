# D48 - Steady left hand on the Gatling gun

2026-11-08. Done, user-accepted. User: "can we stabalize duke's left hand with the
gatling gun, i dont think it needs to violently shake backwards and forwards
... is that really how the game wants to do it?" The D36 audit
([note 257](257-d36-first-person-presentation-audit.md)) and the D36H5 arm
notes ([305](305-d36h5-contact-arm-rig.md), [306](306-d36h5-interaction-arm-coverage.md))
apply. Evidence labels: S source, L private runtime, U user report.

## What the original does

L: private offscreen runs (copy of the D08A32 fixture, UI slot 12, LEVEL0,
60 and 180 Hz). The Gatling (key 4, slot 7) stands in upper clip 29 and fires
with clip 33 (one transition key), then clip 34, which alternates two keys
(frames 1794/1795) every game frame while fire is held. Clip 34 is the same
firing clip the Freezer uses (D36H11).

L: native skeleton sampled from RAM, left wrist (part 12) and right wrist
(part 15) in the torso (part 9) frame:

| Measure (game units) | Idle, clip 29 | Held fire, clip 34 (range per key flip) |
| --- | --- | --- |
| Left hand against torso | fixed (range 0-2) | 7 / 3 / 11 |
| Right hand (gun) against torso | fixed (range 0-2) | 13 / 5 / 30 |
| Left hand against right hand | (-45, 180, -55) | moves to (-72, 80, 92), then 24 / 41 / 9 |

So in the original pose the support hand barely moves against the body while
the gun kicks about 30 units through it. In third person that is what is seen
(a short rattling recoil, mostly hidden by Duke's back). That is the native
animation and D48 leaves it unchanged.

## Why first person shook

S: first person draws the gun in a steady camera-relative pose (D12/D36) and
builds the left arm from the native left-hand-to-right-hand offset
(`display_arm_matrix` in `recomp/src/ttk/first_person.inc`). Firing clip 34
moves that offset about 150 units, which lifts the glove from under the
barrel over the top. Each key flip then moves it 24/41/9 units against a gun
that stays still. The original gun recoil therefore showed as the hand
shaking. This came from the recomp's presentation of the native animation,
not from the original first-person design (the original has no first person).

## Change

S: for display slot 7 (base Gatling record 7 and Laser Gatling record 28)
the left chain is held on the ready grip, clip 29 key 0, the same mechanism
the Freezer (D36H11, slot 11) and Flamethrower/Energy (clip 24 key 2) use.
The held chain is expressed in the weapon matrix, so the gun's own recoil
kick (rotation and pullback) carries the hand with it. The D36H1 underside
palm shift is unchanged and still applies. The right arm, weapon anchor,
kick, fire events, animation clocks, gameplay and third person are not
touched. Modernized first person only; Vanilla never uses this path.

As with the Freezer, the left arm now holds its grip while running instead
of taking part in the native running swing (the right arm still swings).

Developer override: `DNTTK_FP_GATLING_HOLD=anim,key`; `-1,-1` restores the
native left chain for comparison. Not saved.

A present-trace point `support_hand` (with `DNTTK_PRESENT_TRACE=1`) records
the drawn support wrist, for `recomp/tools/local/present_trace.py --relative weapon_hand`.

## Verification

L: lab `recomp/analysis/d48/` (`cap.py`, `probe.py`, `rel.py`, `sheet.py`;
local, ignored). Private settings and card copies, offscreen, dummy audio.

- 60 Hz, FOV 90, 16:9: idle, 16 frames of held fire, settle. Before: the
  glove sits under the barrel at idle, then jumps above it and back while
  firing. After: the glove stays under the barrel every frame, moving only
  with the gun's kick. Looking down/up while firing, running fire and crouch
  fire also keep the grip.
- 180 Hz (player rate), support wrist against the gun grip on screen, view
  units per half-second block of redraws while firing: native up to
  357 / 131 / 136 with 227-unit steps; held at most 5 / 22 / 7 with steps of
  about 11-22 (the gun's own recoil tilt carrying the hand).
- Laser Gatling (`dnupgrade`): same held grip through firing.
- Suites: `ttk-input-test` and `ttk-near-test` pass. `ttk-controls-test`
  stops at its D14 HUD-layout block (`hud_intact`, line 1759) with or without
  this change (unchanged source rebuilt to check). With only that block skipped in a
  local build, the rest of the suite passes, including the first-person arm,
  Gatling and Vanilla cases. The test file is unchanged.
- Third person: baseline captures only (native clip 34 recoil); no code path
  changed there.

Matched before/after frames are kept locally in
`research/before-after/d48-gatling-steady-hand/`.

Limits: sparse frames plus a 180 Hz trace, not every combination. Swimming,
jetpack, other costumes and FOVs other than 90 were not captured. The
third-person firing animation remains the original.

Candidate executable SHA256:
`9b22caea694ed3cc8459fdf43df29ab6e6007227aea9625ebae76f7a1b049f43`.

## Acceptance - 2026-11-08

User playtest: "excellent, once back to the hold you just did, let's lock it
in and close it out, im very happy with it". An Advanced menu toggle for the
grip was started and then dropped at the user's request ("stop / revert");
no toggle ships. The accepted executable is the candidate above.
