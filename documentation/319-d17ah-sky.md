# D17AH - Sky layer investigation and expanded FOV coverage

2026-10-29. In progress. This note records investigation, not acceptance.

## Fixture and baseline

Current UI slot 8 is `recomp/saves/local-play/openbios/state_800AB6FC_slot07.pst`,
a Western rooftop view. SHA-256:
`50884ee019d7ddbdda95f4da6270d41e563c160839a0f7806ab19582755a711d`.
Private copied cards, captured images, trace records and intake/preservation
manifest are under ignored `recomp/analysis/d17ah/`. Player files are read-only
inputs to the harness; all writable runtime state is private.

At 90, baseline presented screenshots show abrupt angular changes within the
cloud band as the camera turns. At 120 the extended candidate range exposes
additional rectangular cloud patches. These are not a whole sky-pass omission.
Near-clip counters in the initial 90 capture show zero packet skips, mesh
fallbacks or budget hits, with host packet peak 22,344 bytes.

## Prior fixes and investigation

Read notes 172, 173, 197 and 300 before changing sky behavior. The accepted
systems cover aspect expansion, separate packet arenas and aperture-aware replay,
loaded sky background selection, and inverse-H backdrop coverage respectively.
The current defect is not established as recurrence of those original causes.

Primitive traces authenticate the four original sky object-renderer callers:
`0x80038c94` backdrop, `0x80038cf0` horizon, and `0x80038da8` / `0x80038e44`
cloud layers. The near-clip path submits positive host depth for these layers,
including transparent clouds. Their camera-relative geometry occupies overlapping
view depths, while the original sky uses painter composition. The controlled same-build A/B confirms host-depth interference: enabling only
the authenticated sky painter-depth diagnostic removes the angular cloud patches
at the same saved camera at 90 and 120. Actual positive texture-perspective Z,
XY, UVs and native ordering-table insertion are preserved; only depth-buffer
participation is removed for those sky layers. This restores their original
painter composition, without disabling world culling or world depth.

## Confirmed range mismatch

D14D's backdrop adaptation used actual H only when H >= 128. The newly supported
120-degree horizontal-at-4:3 setting maps to H=92, so it fell back to aspect-only
coverage. The lower bound now derives from `fp_fov_projection(fp_fov_max)`.
Only the authenticated backdrop local X row changes; other rows, clipping,
fill, layer geometry and ownership guards remain intact. This range correction
is separate from the reported popping at 90.

`ttk-sky-test` passes with added H92/112/127/128 cases at margins 0/85/192;
H91 remains outside the supported correction range. Live and replay visual
validation of the combined candidate remains in progress.

## Candidate evidence so far

- `depth8` versus `painter8`: private slot 8, 60-Hz setting, 16:9, FOV
  64/90/120. Presented captures wait for the unique PNG before the next operation.
  Depth-on captures have angular/rectangular cloud patches; painter captures
  retain continuous cloud layers. Cloud time advances between runs, so these
  are matched saved-camera comparisons rather than pixel-identical timer states.
- `painter8replay`: same slot at 120-Hz setting, 16:9, 64/90/120, followed by
  twelve yaw/pitch samples at 120. Inspected captures retain sky, world, weapon
  and HUD. Replay reports 897 worker completions, zero failures and zero deaths.
  Capture/readback and concurrent diagnostic runtimes heavily reduce throughput;
  this does not certify sustained 120 FPS.
- Owned-data corrected `ttk-near-test` passes. New actual-hook cases cover all
  four sky draw callers, H256/160/92, opaque and translucent packets, preserved
  XY/UV/perspective Z, no deferred weapon-layer insertion, ordinary-depth adjacent
  callers, altered sky identity rejection and the Vanilla/no-FP exclusion.
- Initial baseline/trace harness used an asynchronous present request without
  completion waiting. Its native traces remain useful, but unmatched presented
  captures are not the A/B evidence. The A/B harness waits for unique output;
  subsequent sweeps additionally poll `present_shot_seq` and require `wrote`.
- A first 4:3 launch attempt used unsupported CLI spelling `4:3` and did not
  launch. The corrected route uses `--widescreen off`; the failed attempt is not
  gameplay evidence.

## Additional candidate checks

- `painter43a`: 4:3 (`off`), 120-Hz setting; current slots 8 (Western),
  2 (medieval canal) and 6 (Western caravan), each 64/90/120 followed by twelve
  yaw/pitch samples at 120. Western and medieval cloud variants retain continuous
  visible coverage in inspected captures. Slot 6 is primarily surrounding-world
  regression coverage; buildings obscure much of its sky. All captures wait for
  successful present completion. 3,064 surface-restoration checks have zero
  mismatches; 3,627 worker jobs have zero failures or deaths.
- `move8`: current slot 8, 16:9, 90, 120-Hz setting, backward/forward and lateral
  input samples. Captured clouds remain continuous with world, weapon and HUD
  intact. 321 restoration checks have zero mismatches; 462 worker completions
  have zero failures or deaths. This is a bounded rooftop movement route.
- All completed harness runs assert original player save/card/settings hashes
  and modification times unchanged. No save format or original fixture changed.

The shared renderer coordinator owns final promotion of the painter-depth rule.
The observed sky defect is corrected by the candidate; user playtest remains.
Whole-campaign skies, every angle, city black-overhead and Roman sky variants,
and sustained uncontended 120-FPS performance are not certified by these runs.
