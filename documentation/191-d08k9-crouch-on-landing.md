# D08K9 - Crouch held through a jump

Status: Done (user-accepted 2026-10-22). User accepted the functional result
and final Crouch-toggle/menu revision: "lock it in, close it out!"
Documentation, commit and push closeout authorized.

## Change

With CROUCH MODERN, crouch held in flight takes over on
an ordinary dry touchdown. With a direction held Duke starts the crouch
walk immediately; otherwise he enters the one-knee kneel. The last flight
pose and pelvis height blend into that pose over the existing six-update
crouch transition. There is no intermediate run/stand animation or 105 wait.
A press only after landing retains the existing ground crouch behavior.

`recomp/src/ttk/crouch_landing.inc` uses existing, authenticated hooks:

- At player update `0x8005a210` (caller `0x80041b34`), remember held crouch
  only in ordinary jump/fall mode 9, with modern options, dry water state,
  live camera/input, identity and the required crouch clips available.
- The original landing selector `0x80054c04` still runs. At lower-channel
  initialization `0x800493a4`, caller `0x8005a4d0`, accept only its soft
  landing results 105/78/84/85/96, mode 0 from 9, in that same update's
  stack and input epoch. Select 178 before the original initializes it;
  clear only the running-continuation bit in `+0x228`.
- At `0x80055e80`, caller `0x8005a4f8`, the original clip setup is finished.
  Install the host pose and take the first collision-checked crouch step.
  `crouch_walk.inc` permits previous mode 9 only for this explicit handoff
  and keeps the captured flight height as the transition's starting height.

Hard landing 106, damage, landing sound, death, water entry and traversal
choices remain with the original. Release, precision aim, interaction,
missing clips, changed identity, a different caller/stack or input epoch
cannot claim a landing. Original crouch and Vanilla do not use this path. The legacy landing
setting no longer gates crouch-on-landing. All hook/selector code is covered by existing guards.
No generated code, framework source, hook list or save format changed.

## Evidence

Executable: `5bff834e83785cb7d7023001198aebba1d66df881de7d05cbe13d051b081b091`.
Private lab: `recomp/analysis/d08k9-lab/`, port 9409, offscreen OpenGL,
dummy audio, private settings/cards and a read-only copy of player UI slot 4.
The player's files were not written.

- Baseline: standing jump 97 -> 105 at field 235, 63 at 273, 176 at 274,
  then 178 at 281. Running jump 104 -> 78 at 209, 176 at 211, 178 at 216.
- Candidate at 60 Hz: both go directly from flight to 178 with mode 0/9
  in the touchdown update. Running: 178 at 209; no 78 or 176 between.
- `matrix.py`, 120 Hz presentation: 24/24 cases pass. Standing, walking
  and running takeoffs, with/without a direction held at landing, manual
  and assisted jump styles, first and third person. Every sampled grounded
  animation is 178. Crouch release stands afterward. Health unchanged.
- Captured third-person landing sequences show the flight-to-kneel/walk
  blend; user judgment of smoothness remains pending. First-person 60 Hz
  follow-up reports requested/supported true and blend 1.0 after landing.
- `edges.py`: releasing Ctrl in flight retains 105 -> 63; pressing Ctrl
  only after touchdown retains 105 -> 63 -> 176 -> 178.
- Live option checks at 60 Hz: CROUCH ORIGINAL and LANDING ORIGINAL each
  retain 97 -> 105 -> 63 -> 176 -> 178 with held Ctrl.
- Hard-landing fixture: increase downward velocity in a private airborne
  state. The original selector chooses 106, health drops from 9250 to 1750,
  and 106 completes before normal held crouch starts. This is a controlled
  velocity fixture, not a naturally reached high-drop route.
- Player build succeeds. Native input, aiming and controls suites pass
  (controls with the D07 final-camera LEVEL00 fixture and D22A LEVEL01).
  The new controls group covers soft results and hard/death/water/traversal
  exclusions, release/late press, epoch, both Original options, missing
  clips, code identity, caller, stack and Vanilla. The initial water test
  fixture placed the surface at the feet without submerging Duke; corrected
  to a submerged state, it passes. No full Vanilla campaign route was run.

## Player review

Use UI slot 4 with CROUCH MODERN. Run with Shift+W,
jump with Space, then hold Ctrl through touchdown. Repeat without holding a
direction at landing, from standing and walking, and in both cameras (P).
Judge the immediate response and pose transition, including releasing Ctrl
and continuing to move. F10 captures the mouse if needed. D08K5's game-wide
coverage remains separate; D08K10 and D08K11 are separate refinements.

## Menu revision after functional acceptance

User: "functionally its perfect"; requested hiding Landing from Advanced
and making this part of the Crouch toggle. Landing is removed from Advanced,
and CROUCH's help names crouch walking and crouch on landing. D08K9 now
follows CROUCH alone; the legacy landing preference cannot disable it.
The older general movement-recovery setting/CLI stays compatible.

Player build and native controls suite pass. Private 60 Hz standing jump
with LANDING ORIGINAL and CROUCH MODERN goes directly 97 -> 178 on
touchdown. Original crouch and Vanilla exclusions remain tested. Earlier
LANDING ORIGINAL exclusion evidence above describes the first candidate
and is superseded by this revision.
Candidate executable: `a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`.
No commit or push; revised menu/toggle behavior ready for review.

## Acceptance

The user accepted the functional behavior as "functionally its perfect",
then accepted the final menu/toggle revision with "lock it in, close it out!".
The accepted baseline is
`a77ba035cdbbbfa983e6f02ca1d02aca552f97c1bf07531afa672f0e1d0bef91`.
The earlier pending-review statements above record the diagnostic stages;
this acceptance closes D08K9. No further build was needed for closeout.
