# D08AC - Normalise Shallow-Water Locomotion and Jump Physics

Status: Todo. Backlog and lightweight source research only, 2026-10-22.
No implementation, runtime reproduction or new physics measurements.

## Report and intended design

The user reports stunted jumping in the flooded subway, UI save slot 10,
compared with nearby dry ground. Grounded shallow water should share land
locomotion: speed, acceleration/deceleration, jump impulse/gravity/airtime,
input handling and air control. Water presentation and environmental effects
remain layered on top. Floating surface and underwater swimming retain their
existing physics. This is the preferred architecture, not a proven safe
replacement of every native water handler.

## Confirmed source findings

Paths below are under `recomp/src/ttk/`, inspected without edits.

- `swim.inc`, `water_depth_units`, `free_swimming`, `shallow_land_water`:
  water surface is player+0x834 (0x40000000 means absent), support height is
  +0x1c8; depth is support minus surface. Water presence and +0x20c's
  submerged-joint mask feed classification. Native modes at +0x22c are
  4 for surface swimming and 5 for underwater. Depth below 0x281 (641)
  can be grounded water; the 512..640 band has native mid-wade mode 1.
  Classification also uses animation/state: depth alone is not sufficient.
- `swim.inc`, `swim_update` calls `swim_try_wade_jump` for wading before
  handling native swimming. `modern_controls.cpp` calls the early swim
  update before its ordinary walking-jump preparation path.
- `swim_start_airborne_jump` directly calls original 0x8003e2d0 with
  animation 98 for directed jumps or 97 for neutral jumps. It clears the
  water mask for launch and arms a holdoff (22 update calls). It marks the
  flight owned and explicitly sets `air_cap=0`. `manual_jump.inc` requires
  a positive cap for `manual_air_control`; normal fresh flight seeds that
  cap in `modern_controls.cpp`. Thus wade launches deliberately bypass
  normal manual air steering.
- `swim_boost_horizontal` uses separate minimum horizontal velocities
  3600/4800 for walk/run, retaining a higher native launch magnitude.
  Directed wade jumps arm 12 boost calls and reassert the takeoff direction.
  These constants are not directly comparable to units per rendered frame.
- `swim_try_wade_jump` has wall/climb/ledge branches. The ledge branch sets
  delayed vertical velocity to -9000, may hold horizontal travel until the
  feet clear the lip, and extends water holdoff to 40 calls. A blocked
  forward jump can become neutral animation 97. The ordinary branch has
  no explicit replacement vertical impulse in this helper. Source comments
  contain historical arc estimates, not measurements of today's slot 10.
- D08AA already shares `modern_pace_step()` for water movement under
  SWIMMING MODERN. D08AB ORIGINAL deliberately restores forced shallow
  running and older water speeds. Neither establishes identical jump or
  acceleration curves. See [156](156-d08aa-unified-pace.md) and
  [157](157-d08ab-swimming-option.md).
- D08K11's `crouch_water_supported` and `crouch_ground_state` distinguish
  grounded modes 0/1 from swim states. Its scoped native mode-1 crouch veto
  bypass and clearance-checked handoff must survive any consolidation.
  `crouch_landing.inc` still excludes `in_water_early()` in its eligibility;
  review landing parity explicitly rather than assuming D08K11 covers it.
  See [193](193-d08k11-shallow-water-crouch.md).

## Existing evidence, not reproduced here

Note 193 records its private slot-10 copy at (9748, -6002, 39683), support
Y -5504, water Y -5632, depth 128, native mode 0/0, animation 63. Confirm
this against a fresh private copy before testing; UI slots can change.
That work covered grounded crouch at multiple depths and controlled deep
handoff, but did not establish this job's paired land/water jump parity.

Note 193 identifies original transition routine 0x80055e80 using neck/body
height, surface and volume depth, with crouch-animation exclusions. The
submerged-joint mask is native-owned, not a safe universal locomotion toggle.
Air drains in mode 5. Do not erase water identity to obtain land movement.

## Hypotheses and unresolved questions

- Separate takeoff selection, water-mask restoration, wall/ledge branches
  or native water collision response may explain the reported stunted arc.
  No exact root cause, reduced gravity, damping multiplier or measured
  vertical-velocity difference has been established in this investigation.
- Shared ground eligibility, pace, launch and airborne control appear a
  maintainable direction. Native water transitions/effects may still need
  dedicated handlers; prove their separation before removing any path.
- D08AB ORIGINAL's accepted forced shallow run conflicts with unconditional
  land parity. Define the option boundary explicitly during future design;
  preserve accepted swimming behavior and Vanilla. No option change now.
- Determine which shallow ledge assists remain necessary after parity;
  avoid carrying old boost overrides into a shared jump path blindly.

## Future paired investigation and acceptance route

1. Copy slot 10 to private test storage, use private settings/cards and
   record build hash, camera, jump style, SWIMMING and CROUCH choices.
   Confirm grounded water, then traverse to nearby comparable dry floor.
2. Compare neutral, forward/back/side/diagonal walk and Shift-run jumps,
   start/stop/reversal, both input orders and airborne steering. Keep
   collision geometry and settings comparable; isolate open floor from
   wall contact and ledge assistance. Repeat manual and assisted styles.
3. Trace simulation updates, +0x60 animation, +0x22c/+0x22d modes,
   +0x20c mask, support/water/body/foot Y, +0x1f4/+0x1f8/+0x1fc velocities,
   timestep, input consumption, flight ownership, air cap, boosts and
   holdoffs. Compare launch-to-apex height, vertical delta per simulation
   time, airtime, XZ speed/acceleration/deceleration and input-to-takeoff
   latency. Use matched traces, not render-frame counts or feel alone.
4. Verify natural dry/shallow/mid/deep boundaries, particularly jumping
   into deeper water, submerged entry, surface exit and return to ground.
   Use controlled fixtures only as supplementary evidence. Preserve water
   effects, splash/audio, air rules and native swim transitions.
5. Cover crouch walking, blocked stand-up, held crouch on touchdown,
   aiming/firing, both cameras, 60/high refresh, wall slide, edge grace,
   assisted exits and Vanilla. Surface/underwater swim, dive, ascent,
   swim-fire and both swimming options are regression checks only.
6. Implement only when selected later. Verify source identities and active
   overlays before hooks; use shared grounded policy where evidence permits.
   Require measured parity plus user acceptance, with tolerances explained
   from timestep/collision differences rather than guessed now.

Dependencies: D08AA, D08AB, D08K11, D08Z are accepted. Related work: D08C/I/L,
D08M/O/P/W/Y, D08Z1/Z3, D08K5/K9. Broad D08K5 coverage remains separate.
