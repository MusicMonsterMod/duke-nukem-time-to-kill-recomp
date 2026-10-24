# D10B - Tight-space third-person camera

2026-10-24. Accepted and closed out.

User: "fully accept!! lock it in and close it out". D10B is Done.
The user accepted the candidate below and explicitly authorized documentation,
commit and push closeout. No post-acceptance gameplay changes or rebuild.

## Agreed behavior

Keep the player's third-person view, distance and shoulder preference. Resolve
obstructions before smoothing; compress the camera, bring the shoulder toward
center when obstructed, and ease back when clear. Only Duke fades at close
range. Nearby doors and walls stay solid. No automatic perspective switch,
shoulder swap, FOV change or additional mouse-aim smoothing. Vanilla and the
Original camera retain their behavior.

The user selected this policy after reviewing the original D10B brief. It
supersedes the old absolute requirement to show the entire head and body in
spaces that cannot accommodate them. D11C already fixed the saved head-hide
flag; that separate defect was not reopened.

## Implementation

- Keep the authenticated native four-candidate collision solve. Its minimum
  boom previously pushed a solved position back out to 768 units. For this
  Modernized third-person solve, temporarily lower it to 256, then restore the
  native constant at the following camera hook, as the existing first-person
  path already does. This is not a persistent game or save modification.
- At the existing follow-easing calls, apply inward collision corrections
  immediately. Outward movement holds briefly (8 host fields after a meaningful
  inward correction), then recovers with an approximately 180 ms exponential
  time constant. Recovery uses host elapsed fields, not presentation FPS.
  Original camera constraints and room queries remain in place.
- Shoulder compression follows solved distance / preferred boom, so choosing
  a short camera distance in open space does not disable the chosen shoulder.
  The saved distance and side never inherit the temporary correction.
- Close Duke coverage follows a smooth distance curve (650-1700 world units),
  with less fading when his chest is away from the view center. The original
  close-camera darkening is cleared consistently for the root and limbs.
  Intentional power-up translucency/effects keep their original behavior.
- OpenGL uses a stable spatial coverage mask, shared across body polygons,
  rather than stacking translucent limbs. This retains depth occlusion and
  reveals the scene through the body without revealing rooms through walls.
  Fully opaque Duke stays on the native mesh path. Software keeps the native
  body fade; camera compression and solid scenery still apply.
- Extend the existing host-vertex metadata with coverage, default 1. The
  render-replay codec accepts older 7/8-word records and the new 9-word record.
  Flat and textured OpenGL primitives consume coverage and reset it after use.
  The ordinary world, HUD, other actors and first-person weapons retain full
  coverage. The captured render state carries camera ownership and renderer
  capability explicitly, so forked redraw workers do not query a stale GL
  initialization state.
- Extend the guarded first-person occluder suppression to the owned modern
  third-person camera. Near clipping also operates in 4:3 third person, not
  only the already-covered widescreen path.

Relevant title files: `modern_controls.cpp`, `tight_camera.h`, `near_clip.cpp`.
No generated C or hook-list edits. Existing camera/actor/mesh guards cover the
call sites. Developer A/B comparison: `DNTTK_TIGHT_CAMERA=0`.

## Verification

Evidence is local under `recomp/analysis/d10b/`. Only private copies of older
analysis saves and private profiles/cards were used. The historical sewer
slot 12 starts at the ladder top; the private route descends it before entering
the sewer. The apartment fixture is the earlier wardrobe clipping route.

Accepted executable SHA-256: `4396ea72a48d95b6de13999501ff8b4f53f46515533b8aa7d7a6ca42d54429eb`.

- Final build and native controls, input, aim and near-clipping suites PASS.
  Camera tests cover the temporary minimum/restoration, immediate inward
  correction, outward hold/recovery, preference retention and caller guards.
  Rendering tests cover fade restoration, intentional effects, nested draw-list
  scope, opaque/native routing, Vanilla exclusion and forked worker state.
- Framework host-vertex/replay codec test PASS, including legacy record sizes,
  independent texture/depth values and malformed stream rejection.
- Final private apartment runs at 60 and 120 FPS each recorded 69 samples.
  Both passed full yaw sweeps, three shoulder selections, first/third switching,
  firing, private slot-5 save/load, preference/head-flag checks and forward/back
  movement. The 3-second idle windows measured 59.88 and 59.52 host fields/s;
  the 120 FPS run measured 120.04 distinct presentations/s. These short samples
  establish bounded playback, not a campaign performance benchmark.
- Final sewer ladder-bottom heading sweep completed with the 4:3 setting.
  Duke remains opaque where clearance permits; minimum sampled coverage was
  0.8939. Apartment sweeps reached below 0.011 coverage with sub-481-unit booms.
- Final wardrobe approach completed eight movement steps with solid doors.
  The 18-sample sewer movement route compressed to 647.98 units and reduced
  Duke coverage to 0.1512 during the drop, recovering to full coverage in the
  water. Both routes used the final executable.
- Visually reviewed the corrected body fade and preserved first-person weapon
  at the wardrobe. The fade uses spatial dithering without temporal filtering;
  the pattern is visible, especially at low internal scale. It is deliberately
  confined to Duke, with no room/door transparency.
- Exported runtime patch applies cleanly to pinned framework
  `08ec704a974b1f3a16335b4afeb340b9eff19926` and reproduces all 51 existing
  modified/new framework files byte-for-byte, preserving prior accepted work.

Local captures: `shots/body-scope-final-angle2.png` (corrected fade),
`shots/apartment-baseline-angle2.png` (feature-disabled comparison),
`shots/final-120-first.png` (weapon regression),
`shots/final-sewer-4x3-angle2.png` (opaque Duke at ladder bottom),
`shots/final-door-door7.png` (solid wardrobe), and
`shots/final-sewer-walk-3.png` (sewer movement).
The earlier captures before the dedicated Duke draw-scope correction are not
proof of the new fade: native translucency was still active in those images.

## Acceptance and retained limits

User accepted the camera behavior and fade on 2026-10-24. The original collision
solver still defines reachable camera positions; bounded routes do not prove
every campaign location or dynamic door state. The spatial coverage pattern
remains visible, especially at low internal scale. The software-renderer body
fade remains original. No automatic first-person transition was introduced.

Closeout includes authored title implementation/tests, the exported framework
patch, manual, board and status/handoff. Patch verification against the clean
pinned framework preserves earlier accepted work. No retail assets, generated
code, builds, captures or player saves/cards are included. No next job started.
