# D08A35 - The cowboy hat stays on under the Bio Mask (accepted)

2026-11-04. Revision 2 accepted by the user: "legendary. this is the one. lock
it in and close us out". User (UI slot 3, third person, Old West costume): the Bio Mask
comes on in the toxic gas and Duke loses his cowboy hat; "there is no reason
for this to happen".

## Cause

The mask is not an item drawn on top of the head. The native equip
(`8004e894..8004e94c`, D08A24) writes the head joint's mesh pointer (model
record `+0x68` of the neck-part joint) with one shared masked-head mesh,
`[800ddb18]+0x1030`; the goggles do the same with `+0x10c4` (D08A23), and
removal or depletion restores the pristine model's head from `[[800ddb14]+4]`.
Both shared heads are the default bare-headed Duke (47 vertices, flat top at
y -122), so the Old West model (levels 1, 2, 3, 27; neck at joint 15) loses
its hat, which lives in its own head mesh (53 vertices, 102 GT3 faces).

Measured on a private copy of UI slot 3: cowboy head `0x8011dc40`, mask head
`0x801309c0`, goggles head `0x80133cb4`. In the cowboy head the hat is 50
faces (brim CLUTs 887/2039, crown 951 and 4141/567) on 30 vertices, all at or
above y -60.

## Implementation

`recomp/src/ttk/modern_controls.cpp` `cowboy_hat_mesh_begin`, called from the
Modernized mesh-entry filter at `0x80010000` (`near_clip.cpp`, after the D17Y
missing-texture filter, the same mechanism). When the joint loop
(ra `0x800351f4`, s4 the player) submits the neck joint's mesh, face equipment
is 2 or 3 and the mesh is that item's shared head, and the model is the Old
West one (neck joint 15), a0 is replaced by a private guest-memory copy:

- the bare head's header, its whole primitive list, and its vertices with
  the head's top tucked under the hat (away from the side respirator
  canisters at |x| >= 45). At the back, where the brim meets the crown low
  (about y -84 at z -41), everything above y -66 is pressed under the brim
  line (to -76 at most) and 15% inward, as a hat flattens hair: otherwise the
  hair and strap showed between brim and crown, where the painter sort drew
  them over the hat. At the front, where the brim meets the crown high (about
  -98 at z 49), only the forehead above -95 is lowered to -95. Blended over
  z 0..20. (Revision 1 pressed the front like the back: it folded the mask's
  face plate and opened a hole at the eyes, user report 2026-11-04.)
- one more GT3 group: every face of the pristine costume head lying wholly
  at or above y -60 and reaching the crown (y < -98), the brim (|x| > 35) or
  its back (z < -60), with its vertices appended unchanged (light data
  included). This rule selects exactly the 50 hat faces. At least 16 such
  faces, 8 of them reaching the brim (|x| > 60), are required, otherwise the
  head draws native;
- a bounding box that is the union of both heads' boxes.

The game's model, the head pointer and saves never hold the copy: only a0
changes, and the copy is rebuilt at every draw (redraw workers draw from
their own RAM). The near-clip object path accepts the copy's own vertex and
list addresses (`cowboy_hat_mesh`). First person hides the head as before.
Other costumes, Vanilla (the filter is Modernized-only) and the native equip,
drain, depletion and goggles exclusivity are unchanged.
`DNTTK_COWBOY_HAT=0` (developer) keeps the native bare head.

## Evidence

Revision 2 executable `f57c694de9e655fa7d50f1a109b9c31e7bcf2fcae76590809499591cb419e070` (revision 1 `0fce22c66b0f8b5c2d0e20cf4a9a6fca6f954c333b8e5121856cc9effc8117fc`). Private runs offscreen from a copy of UI slot 3
(file slot 2), blank cards and private settings (`recomp/analysis/d08a35/`,
ignored): player saves, cards and profile hash-checked unchanged.

- Before: the native mask removes the hat (reproduced).
- Mask and goggles on, third person, back and Free Look front views: the
  hat sits as on the costume head, the mask shows beneath the brim, no hair or
  strap through the crown. Goggles off: the native cowboy head returns.
- The head pointer in game RAM stays the native mask/goggles mesh throughout;
  a savestate saved while masked and loaded again holds native pointers (the
  D08A24 rule then settles the mask off outside gas, as before).
- Level 6 (non-hat costume) with the mask: the native bare masked head. An
  earlier rule without the Old West gate took that costume's hair for a hat;
  the gate and brim test fixed it.
- Revision 2, eye-level Free Look front view: the face plate and both lenses
  whole under the brim, matching the native masked head; back views with
  mask and goggles still clean.
- First person with the mask in the cowboy level: native view, D08A26 overlay.

Limits: the real gas crossing was not driven (the mask came from the item
action, the same native endpoint the auto mask uses); one cowboy level; the
Vanilla route was not run (structural exclusion only); mirrors not checked;
the user judged the look in game and accepted revision 2.
