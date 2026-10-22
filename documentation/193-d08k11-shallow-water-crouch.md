# D08K11 - Crouching in shallow water

Status: Done. User accepted the behaviour and authorized closeout on 2026-10-22.

Executable: `1e16b7eaabeef854bddc5f777cbc2f487b89f839491a9dfca4a78b576039c1f8`.

## Findings and implementation

The current private copy of player slot 10 starts in the flooded subway at
(9748, -6002, 39683), support Y -5504, water Y -5632: depth 128, native
mode 0/0, animation 63. Walking worked; Ctrl never acquired crouch.

- Both stance entry and crouch-walk eligibility rejected all water. They
  now share a grounded-water policy: Modern crouch, depth below 641, native
  modes 0/1 and their boundary transitions. Native surface/underwater modes
  4/5 remain excluded, even when their support field gives a small depth.
- Native mode 1 is the mid-depth 512..640 walking band. Its crouch-idle
  handler at 0x800548a4 forcibly reverses animation 176 to stand up. At the
  authenticated player ground-dispatch call, an owned Modern crouch in that
  band temporarily changes 0x800548e8 from 0x14400008 to 0x10000008, taking
  the handler's existing ordinary-input path. The next hook restores the
  instruction before identity checks, including Vanilla and savestate
  recovery. The existing resident guard covers 0x8005484c..0x8005492c.
  No generated C, framework source, original media or water flags changed.
- Leaving the supported depth band releases the host kneel through the
  original reverse-176 stand-up, subject to the existing standing-clearance
  query. Simply dropping ownership stranded 178: the native swim-entry
  routine explicitly excludes crouch animations 176..184. State bytes,
  submersion bits and the eventual swimming transition remain original.
- The crouch mover, pace, animation pack, input masking and collision
  queries are unchanged. CROUCH ORIGINAL retains its water behaviour.

## Water and air rules

The original 0x80042298 update derives +0x20c from 0x80040af4: it is a
submerged-joint mask, not a host-owned swimming toggle. It changes naturally
as Duke kneels. Body Y alone is not a head-submersion test.

Native 0x80055e80 uses the part-0x10 neck position minus 64 and body Y,
water surface and volume depth for swim transitions, but exempts crouch
animations 176..184. Native 0x80041fd0 drains the air counter at +0x236 only
in mode 5 and replenishes it outside that mode (maximum 12000). This job
preserves those rules. A crouched head below the surface in grounded mode
0/1 does not itself drain air; this is not a new drowning mechanic.

## Verification

Private lab: `recomp/analysis/d08k11-lab/`, port 9411, offscreen OpenGL,
private settings and copies of saves. Player files were read only for copying.

- Reproduced slot 10 before the change; candidate kneels, walks and stands.
- Eight crouch directions, stationary firing, held crouch, released crouch,
  blocked stand-up and movement while stand-up is refused pass at depths
  128 and 600. The blocked-clearance live case uses the existing diagnostic
  refusal; native collision fixtures separately cover geometry/objects.
- 60/120 Hz checks and presented captures cover first and third person.
  Both SWIMMING choices retain crouch walking; Original swimming still has
  its standing forced-run flag. The actual third-person run is `final-third`
  (requested false, blend 0); earlier attempts with invalid view spellings
  were first-person runs and are not third-person evidence.
- Controlled depth fixtures modify the private scene's water plane, leaving
  original queries and state updates live. Depths 128, 256, 400, 511, 512,
  600 and 640 permit crouch; dry/shallow/mid changes retain it. Raising the
  water to depth 800 while crouched now releases into native swimming.
  These fixtures are not a traversed campaign shoreline. Lowering an entire
  water plane while already swimming can leave mode 5 without water; that
  artificial drained-pool case is not evidence of a normal shoreline exit.
- A separate, unmodified private deep-water save passes dive, underwater
  movement, swim-fire (3 shots, arm-aim updates), Space ascent and surfacing.
  Air drains in mode 5 and recovers to 12000 at the surface. CROUCH ORIGINAL
  produces zero host crouch entries in shallow water. Vanilla shallow pad
  inputs and native dive also produce zero Modern crouch/swim interventions.
- Player build and native input, aiming and controls suites pass. New
  controls fixtures cover shallow/mid entry, submerged body, deep/4/5,
  Original/Vanilla exclusions, native mask/state preservation, scoped
  instruction restoration (including Vanilla), and clearance-gated deep
  handoff. Existing dry crouch, hull, landing and viewmodel fixtures pass.

## Acceptance and coverage limits

User: "awesome! i accept the behaviour here, very good. lock it in and close it out."
The tested executable above is now the accepted regression baseline.
Implementation, manual, job board and status/handoff are included in closeout.
No next gameplay job started.

The controlled water-plane fixtures do not establish a naturally traversed
campaign shoreline or full game-wide collision coverage. D08K5 retains that
broader sweep; user acceptance does not expand the lab evidence above.
