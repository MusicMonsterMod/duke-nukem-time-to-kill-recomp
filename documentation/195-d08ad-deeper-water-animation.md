# D08AD - Third-person locomotion animations in deeper shallow water

2026-10-22. Status: Done, user-accepted: "the animation is fixed. lock it in
and close it out." Implementation and documentation commit/push authorized.
Separate reported deep-water entry regression: D08AE, [note 196](196-d08ae-deep-water-entry.md),
investigated and queued only. No correction to deep-water entry in this closeout.

## Accepted implementation and evidence

Executable SHA256:
`9bf9a7ff60c818149978f0a4317b7ff4f6464c2943a1bd26b08c5999d6030c68`.
Private evidence: `recomp/analysis/d08ad-lab/` (ignored).

The fresh slot-10 copy starts at depth 256 in LEVEL00, around
(16615, -6389, 109137). Traversing the flooded corridor reaches natural depth
512, where both reported animation failures reproduced. This differs from
the earlier subway copy. A private intermediate save and repositioning within
that corridor isolated sustained cycles; the original slot was never written.

Confirmed causes:
- Walking's `terrain_step_probe()` runs guest code inside the host dispatcher
  hook, before the native dispatcher has pushed its stack frame. D08AC armed
  its patch scope before this probe. The nested 0x800780b4 hook found the
  would-be saved return address contained the player pointer instead, rejected
  the scope, and left the native walk-to-wade rewrite active. Write traces
  show 72 -> 81 at 0x80048614 followed by the host's conversion back to 72.
  Each update reinitialized frame 16 with duration 50: a frozen walk pose.
- Running reached its segment-completion selector 0x8004b644, which has
  separate depth checks. It chose 81 at 0x8004d484 instead of the opposite
  land stride half. The host conversion then restarted 76. Frames 105..107
  repeated without the 78 half (117..119), explaining the abbreviated steps.
  There is no evidence of an independent backpack defect.

Changes:
- Arm the existing dispatcher scope after the host hook's terrain probe,
  immediately before native dispatch, rather than before its nested calls.
- Add an authenticated entry hook for the native segment-completion selector.
  In eligible Modernized grounded water, bypass only its four depth-based
  wade choices and let the native selector alternate the land gait halves.
  The selector's saved return address is at entry SP-8; the dispatcher uses
  SP-4. Both scopes restore on exit and remain identity/epoch/caller guarded.
- Keep native animation initialization, frame timing and interpolation. No
  host reconstruction of frame progress, backpack edits, water-mask changes,
  speed retuning or jump-physics changes. ORIGINAL and Vanilla are excluded.
- Regenerated via the existing CLI after adding the hook to game.local.toml;
  generated C was not hand-edited. Movie/math shards rebuilt for the config.

Verification:
- Player build, native controls across all 21 overlays, input and aiming
  suites pass; five Python hook-contract tests pass. The new controls case
  executes a nested terrain hook before native dispatch, verifies that the
  scope survives subsequent native calls, and covers completion phase/frame
  preservation, restoration, altered code, foreign caller, deep/swim,
  ORIGINAL and Vanilla exclusions.
- Sustained depth-512 walk now alternates 72/74 through frames 16..19 and
  32..35; run alternates 76/78 through 105..107 and 117..119 at 60 and 120 Hz.
  Start/stop, walk/run changes and direction changes were traced. The natural
  slot-10 route crosses 256 -> 512 -> shallower/dry ground with both run
  halves intact. These locomotion runs recorded zero lease refusals.
- Offscreen 120 Hz screenshots show changing leg poses and backpack tilt
  through both run halves. This supports cycle completion, not a substitute
  for the user's assessment of smoothness, now accepted.
- Mid-water speed samples remain about 11 units per vblank walking and 50
  running (short wall-clock samples, not exact timing benchmarks).
- Paired D08AC geometry fixtures at depths 128/512/640 retain manual neutral
  rise 1070 (one crouch trial 1069), walking rise 351, and running rise
  597/594/594, matching the earlier accepted trace measurements. Held crouch
  lands in 178. No fixture entered surface/underwater mode during these jumps.
- Live Modernized crouch/crouch-walk, dive/descend, underwater travel and
  swim-fire (three shots), release and ascent back to surface mode 4 pass.
- SWIMMING ORIGINAL repeats the crouch/deep-swim regression route with its
  legacy water speeds; Vanilla's native movement/dive run records zero host
  crouch, dive and aim interventions. First-person assisted neutral/run
  jumps at depths 512/640 pass (neutral rise 1100, native run jump 104).
- All 39 player save/card files match the initial SHA256 manifest. Private
  processes were closed after testing; player preferences were not changed.

The user accepted third-person animation behavior. The prior deep-swim checks
started in swimming states and did not establish deep-water entry fidelity;
D08AE now records the separately reported entry regression. Broader campaign
shoreline coverage remains unproven. No gameplay rebuild for closeout.
The existing framework patch re-exported unchanged, applies to the pinned
clean dependency, and matches all 50 exported source files.

## Initial backlog investigation (historical)

The following source findings and hypotheses were recorded before the
implementation and runtime traces above.

## Report and regression boundary

The user has replaced UI save slot 10 with a slightly deeper shallow-water
location. Third-person walking looks completely frozen and resumes in
shallower water. Running shows abbreviated steps and a backpack apparently
swinging repeatedly to one side. The backpack observation is a visual
hypothesis, not evidence of a separate attachment or animation bug.
Movement physics, jumping and crouching work correctly there; normal
knee-depth water works perfectly, according to the user.

This follows accepted [D08AC](194-d08ac-shallow-water-locomotion.md), with
[D08K11](193-d08k11-shallow-water-crouch.md) and
[D08AA](156-d08aa-unified-pace.md) as regression context. Preserve their
accepted physics, crouching and swimming. The old slot-10 coordinates and
depth in note 193 do not describe this newly updated save. No save/card was
accessed and no runtime reproduction was performed in this investigation.

## Confirmed source findings

Paths below are under `recomp/src/ttk/`.

- `modern_controls.cpp`, `shallow_land_water_early()` excludes native modes
  4/5 and requires support Y (+0x1c8) minus surface Y (+0x834) below 0x281
  (641), together with water presence/submersion evidence. Depth alone does
  not establish grounded locomotion. `grounded_water_modern()` additionally
  requires Modernized and SWIMMING other than ORIGINAL.
- `swim.inc`, `mid_wade_state()` recognizes mode 1, clips 80/81 and depth
  at least 0x200 (512), subject to `wading()`. Its documented native path
  uses dispatcher 0x800486c8 and siblings and handler 0x800539f8. Track
  advancement/root extraction precedes that handler (0x80059d18..0x80059d88
  per the existing source research). D08AA's root speed normalization does
  not itself prove a complete visual animation cycle.
- `grounded_water.inc`, `grounded_water_gait()` changes eligible mode 1
  and previous mode 1 to 0. For clips 80/81 it writes idle 63, walk 72 or
  run 76 according to movement and `input_running()`, then zeros the track
  fields +0x68/+0x6a. It has input, camera, identity, flags, mode and
  animation guards; it does not reset these counters for every land gait.
- `modern_controls.cpp` invokes that conversion at player update
  0x8005a210, selected lower-body initializer 0x800493a4 callers, and the
  authenticated 0x80048410 dispatcher entry from 0x8004b634. Ordering of
  native selection, initialization and track advancement therefore matters.
- `grounded_water.inc` already bypasses four native post-gait water rewrites
  at 0x800485e4, 0x80048628, 0x8004866c and 0x800486b0. Substitutions are
  scoped to authenticated stack/epoch/identity conditions and restored before
  controls hooks. Additional patches preserve jump/landing behavior; changing
  their scope blindly risks the accepted physics.
- D08AC's accepted evidence includes controlled mid-depth physics tests and
  user acceptance. It does not establish full animation-cycle completion at
  this newly supplied natural location. Its documented baseline executable
  is `30ca9b3fa5bd0a6350dd8b7ef1a509914ecfb3e60dcf74ec9ebba0982b7eb838`.

## Suspected causes, not established diagnoses

1. A native path may reselect 80/81 after conversion, causing repeated
   72/76 entry and counter resets. This could explain frozen walking or
   short repeated running segments while translation remains correct.
   Trace both sides of the conversion and native writers to prove it.
2. The scoped rewrite bypass may miss another clip-selection path or lose
   coverage across a nested hook. Check actual patch lifetime and caller
   identities; the existence of patches is not evidence they failed here.
3. Depth/body/submerged-mask updates may alternate movement classification
   or select an incompatible track/initializer. Compare modes and animation
   tracks before blaming depth alone. The new save's depth band is unknown.
4. If selected clips and their counters progress normally, investigate pose
   evaluation and high-refresh replay/interpolation. A render-only freeze
   must be distinguished from stalled simulation playback.

No evidence yet establishes a backpack-specific defect, a truncated source
clip, or which of selection, progression and classification is responsible.

## Future investigation and acceptance route

1. When selected for implementation, take a dated private copy of updated
   slot 10, private settings/cards, and record build hash and options. Do not
   overwrite or test against player saves/cards. Confirm actual depth, active
   overlay and grounded state rather than reusing the old slot's metadata.
2. Compare stationary, start/stop, sustained walk, sustained run, direction
   changes and Shift transitions on dry land, normal knee-depth water and
   the reported deeper area. Traverse natural depth boundaries both ways.
3. Sample per simulation update: clip +0x60, track fields +0x68/+0x6a,
   modes +0x22c/+0x22d, flags +0x224, mask +0x20c, support/surface/body/foot
   heights, input/run intent, root deltas and relevant selector/initializer
   callers. Record before/after gait conversion and dispatcher processing;
   correlate rendered frames with simulation samples for several full cycles.
4. Establish whether tracks reset, stall, use the wrong clip, or progress
   correctly while the rendered pose repeats. Verify native identities and
   overlay before any future hooks. Prefer a narrow animation/transition fix
   that preserves the shared movement policy; do not alter water masks or
   speeds to hide the symptom.
5. Require natural complete walk/run loops and smooth depth transitions in
   third person, including complete backpack motion, at 60 and high refresh.
   Recheck D08AC movement/jump arcs and both jump styles, crouch/crouch-walk
   and stand-up, deep entry/exit, surface/underwater swim, dive/ascent and
   swim-fire. Preserve SWIMMING ORIGINAL and Vanilla; first-person behavior
   is a regression check. User visual playtest is required for completion.

The initial backlog-only pass made no gameplay edits, build or launch. The
user subsequently selected implementation; see the candidate evidence above.
