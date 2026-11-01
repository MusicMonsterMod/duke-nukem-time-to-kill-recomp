# D14G - Holstered swimming arms use the contact-arm rig

2026-11-01. Done, user accepted: "perfect. lock it in, close it out".
Commit/push closeout authorized. Coverage limits below remain in force. Evidence labels follow the accepted D36 audit
([note 257](257-d36-first-person-presentation-audit.md)): S source/native
tests, L isolated runtime observations, U user report.

## Problem

U (accepting D08O5): "when swimming with the weapon holstered, duke's arms
show a weird projection"; "the arms looks disembodied".

L (private copy of the slot-2 moat fixture, first person, offscreen, world
FOV 90, baseline executable): camera-space joint origins sampled during
strokes (`recomp/analysis/d14g/probe.py`, local only):

- Surface strokes (lower 122/125, mode 4): upper arms sit about 110-130
  units below the eye at depth -40..70; forearms 0..170; hands reach 130..280
  in front during the forward phase and go behind the eye on the recovery.
  With the default H=160 the native whole-joint cull drops the upper arms and
  often the forearms, so only the hands and cuffs reach the screen, low and
  unconnected. This matches the intake stills.
- Looking down at the surface puts an upper-arm joint near screen centre
  (for example depth 138, 20 units below centre). The parent torso is hidden
  at the eye, so plainly admitting the arm chain (the D36H4/D36K1 Body
  approach) would expose open shoulder ends: the D36H5 problem.
- Underwater strokes (127/128, mode 5): hands and arms stay mostly below
  the viewport; no arms in the sampled frames before or after.

S: the existing presentation owners did not cover this state. The D36H4/K1
Body rule runs only for Ground, Committed, Fall and Jetpack; the D36H5
contact rig only for the nine contact actions; the D14B display arms only
for a held weapon. Holstered swimming kept the native per-joint cull.

## Change

Holstered (equipment 0) swimming joins the accepted D36H5 contact rig
([note 305](305-d36h5-contact-arm-rig.md), [note 306](306-d36h5-interaction-arm-coverage.md)).
This is the same technique, with no new parameters: native hands stay
the exact world targets, so the stroke animation is unchanged; virtual
shoulders follow the eye; upper arm and forearm are solved with the
authored elbow pole and axial length extension. Arm parts 10-15 are claimed
as `Interaction` with the polygon-depth bridge, keeping world depth, lighting
and clipping (the D14A proximity-fade scope applies as for other contact
arms).

`swim_contact_arms` / `contact_arm_action` in `recomp/src/ttk/first_person.inc`
gate the rig. A drawn weapon (equipment 2) or held item keeps its existing
display arms, so the D08O5 draw into the armed swim pose is unchanged. Third
person, reflections, other actors and Vanilla keep native arms.
`DNTTK_FP_CONTACT_ARMS=native` also gives the one-launch native comparison
here. No framework, generated C, asset, save schema or setting change.

## Verification

S: player build; `ttk-controls-test` (LEVEL00 fixture, LEVEL01, all
levels), `ttk-aim-test` and `ttk-near-test` pass. The D11E swim test group
now runs the actual draw hook in both swim modes and both costume maps:
holstered arms take the contact rig (joined elbow/wrist, native hands and
matrices unchanged, native comparison, reflection isolation, depth bridge),
and equipment 2 never takes it. Mutation check: with the swim gate disabled
the new assertion fails.

L (private fixture copies, offscreen OpenGL, dummy audio; player saves,
cards and profile hash-checked unchanged after every run):

- `c1-60`, world FOV 90: surface strokes show both arms entering from the
  lower screen edge, connected to the hands; looking down shows a continuous
  arm from off-screen; looking up and underwater show no arms.
- `c1-120` at 120 Hz: the same, plus left click draws into the armed swim
  pose (weapon displayed), restow returns to the rig, private save/load
  and dive. Three replay workers, zero failed jobs or worker deaths.
- `c1-fov120`, maximum world FOV 120: connected arms in strokes and looking
  down; armed draw unchanged.

Where a stroke dips a forearm below the water surface while looking down,
the water's world depth hides that part and the hand breaks the surface.
This is world occlusion, kept authoritative as for all contact arms.

Limits: one moat route (level 6), default costume in live play, 60/120 Hz
presentation (not a 180 Hz performance check), sequential samples rather than
atomic frames. Human motion/appearance review is required.

Build SHA-256:
`a29515010be32f1d4695d5fa096b0ed9ac7cbfd96197cd9aa7f7695681a5faf7`.

## Playtest

First person, weapon holstered, swim on the surface and underwater; look
down and around during strokes; try the normal FOV and the maximum; left
click to draw, then holster again.

```sh
python3 recomp/tools/local/run.py
```
