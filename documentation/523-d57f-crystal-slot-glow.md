# D57F - A crystal set into the time machine shines as in its first holder

Status: **Done, user-accepted and closed out 2026-11-20** ("lock it in and close it out"). Executable
`92a3d1cd62b75e8f84de21f0b289660ff997d1cdbdb1a66395f4705bd63c22b6`. Job board: D57F.

## Report

User, 2026-11-14 (playthrough commentary): when the crystals are put into the
final time-portal machine in OBEY OR DIE, the slots should glow, in the
visual language of the original crystal holders. Acceptance: an elegant glow
per inserted crystal, nothing on empty slots, correct after loads.

## What was there

Level 5 (lab dump, fresh `level 5`; slots in the level object table
`0x800d2550`, see [note 519](519-d08a43-crystal-slots.md)):

| Slot | Type | What |
| --- | --- | --- |
| 2, 3, 4 | 176 red, 178 green, 177 blue | the crystals (object kind +0x14 = 1, a static prop) |
| 12, 13, 14 | 180, 182, 181 (kind 17, generic holder) | the holders they start in (+0x35 = 1, 2, 3) |
| 7, 10, 11 | 180, 181, 182 (kind 17) | the time machine's three holders, empty, around the portal pad (slot 9, type 179) |

- The "look of the original holders" is ours, not the game's: the D41D1 sign
  scan put a lamp at each crystal where it starts (`level05.lights`, also
  `level00` and `level09`: "sign type 176/177/178 ... prop 17x x y z"), red
  (1.00, 0.09, 0.36) reach 1791, green (0.31, 1.00, 0.25) 1710, blue
  (0.26, 0.19, 1.00) 1758, strength 1, 20 units below the crystal. It floods
  the holder and its walls in the crystal's colour.
- That lamp is bound to the crystal by type and place (D54, `prop_state.h`):
  when the crystal is taken it sputters out, as a broken lamp. The generic
  holder's mode 3 links the crystal into its room 194 units above the
  holder, a new place, so a crystal set into the machine (or put back into
  any holder) had no light: dark grey holders with a small green crystal.

## Fix (Modernized, Polymorph, game-wide)

`recomp/src/ttk/prop_lights.cpp`, with the woodburners, chandeliers and
glowing pickups:

- Every 10 game frames (with the pickup scan) each crystal object (type
  176-178) linked in a room, not broken, and seated in a crystal holder
  (a generic holder, level object kind +0x19 = 17, 120-400 units below it,
  within 64 units across; seated crystals sit 194 above) is checked against the level's
  lamps (`level_lamp_bound`, new in `level_lights.inc`, game thread): a lamp
  bound to it at exactly that place lights it already (the start holders,
  unchanged); otherwise it gets a live light with the colour, reach and
  strength of the level's lamp for that crystal type, at the same offset.
  A level with no such lamp (a custom level) uses the sign scan's colours.
- The light is steady (as the sign lamps), on at once, through the room
  gate like every prop light; the Relit gain applies as for the lamps. A
  crystal in Duke's hand is in no room, so it gives no light; taken from a
  holder its light goes on the next published image (the D54 snapshot shows
  it unlinked), without waiting for the scan.
- All state is the game's own (crystal and holder objects in guest RAM), so
  savestates and level loads need nothing; a load rescans at once.
- Crystal holders only. A first build lit any seated crystal, the D08A43
  conveyor power slot's too: its green light on the red brick wall right
  behind it turned the wall a muddy orange-brown
  (`research/before-after/D57F/rejected-conveyor-slot-lit.png`), and the
  slot already shows its power its own way (its ring turns green). Devices
  are left as the game draws them.

Developer: console `lamp crystals off|on` (session, for A/B);
`DNTTK_PROP_LIGHT_CRYSTAL_RED|GREEN|BLUE="r g b strength radius"` for the
fallback colours; `DNTTK_PROP_LIGHT_TRACE=1` logs
`[TTK prop light] level N: K crystals away from their first holder lit`.

Vanilla, Classic and LIGHTING ORIGINAL: unchanged (prop lights are
Modernized Polymorph only).

## Evidence

Lab `recomp/analysis/d57f/` (local; `probe.py` cases `survey`, `tm2`,
`back`; private card copies, offscreen on the real GPU, the player's saves
and profile hash-checked unchanged by every run).

- Fresh `level 5`, `dnkroz`; blue and green taken from their holders with E
  and set into machine holders 10 and 11 (+0x35 = 3, 2; crystals linked at
  y -6751): trace "2 crystals away from their first holder lit". Matched
  shots, same frame, `lamp crystals off` / `on`: off, dark grey holders with
  the crystal; on, the green holder and the floor around it green, the blue
  holder behind it blue, the empty red holder dark.
- Savestate saved, another loaded, this one loaded: both still lit, the
  empty holder still dark.
- Another run: red set into holder 7: "1 crystals away ... lit".
- Shots: `research/before-after/D57F/` (local).

## Revision 2: the machine's top (user, 2026-11-20)

User: "the top part of the machine does not need to be an emitter, only the
panel on the ground, and the crystal slots when you put crystals in. the top
part of the machine looks really weird emitting, like it's a big piece of
metal. if any part of the top part became an emitter, it should be the area
in the dead center of it".

- Owner: the hanging top (prop type 192; level 0 object 16 and level 5
  object 8, at 0x80128954 in the user's UI slot 9 save, which is level 0's
  time machine) has no lamp; no level lamp is bound to it. Its faces are
  ordinary Gouraud-shaded textured polygons (GPU 0x34) drawn by the
  static-prop loop (caller 0x80032288); the game itself lights it nearly
  white (bright grey with LIGHTING ORIGINAL too), and in Modernized the bake
  and the pad lamp add a blue cast. Over a dark room it read as a lamp.
- Fix (`prop_lights.cpp` kind `prop_light_portal_top`, found by its model
  data; `near_clip.cpp` object colours): in Modernized Polymorph its vertex
  colours are scaled to 0.45, so it is dark metal that takes the room's and
  the pad's light; a soft cool halo (the pad lamp's colour, a 3 s breath
  0.85-1.0) sits at its centre, the swirl disc between the prongs over the
  pad. It gives no light of its own. Developer:
  `DNTTK_PROP_PORTAL_TOP="tone size level y"` (default "0.45 520 95 260";
  "1 0 0 260" is the old look).
- Evidence: the user's UI slot 9 (private copy), 600 frames after the load
  (the bake shown), two runs, old look against new: the top's screen region
  mean (72, 90, 126) -> (51, 70, 106); the halo at the centre disc.
  `research/before-after/D57F/top-zoom-before-after.png`, `before-top-slot9.png`,
  `after-top-slot9.png`.
- The console `lamp props off|on` does not reach the redraw workers, which
  draw the static props (they keep the setting they were forked with), so it
  is no A/B for these props; the env setting is.

## Revision 3: the light is the centre between the claws (user, 2026-11-20)

User: "ok this is too crazy. it's even making the blood look like it's
emitting light on the floor. right so that big hanging thing in the middle,
dont have that as an emitter. instead, just emit light from the centre part
between it's claws."

- Why revision 2 still looked like a lamp (MEASURED, UI slot 9): with every
  level lamp hidden the top stayed as bright (bright-pixel mean 63, 89, 147;
  the crystal and centre lights add a little). Since D41P a model takes the
  bake of the floor under its origin (Quake's R_LightPoint); under the top
  is the pad, the brightest floor of the room (its blue sign lamp sits on
  it), so the metal 2450 (level 0) to 4200 (level 5) units above was lit as
  if beside the lamp.
- Fix: the top takes no floor bake (`near_clip.cpp` `Frame::hanging`, set
  for the `prop_light_portal_top` kind while the extras are on): it is lit
  by the live lamps at their real distance. Its colours are scaled to 0.6.
  A live light at its centre between the claws (`portal_light`, the pad
  lamp's colour 0.35 0.63 1.00, strength 0.9, reach 1900, the halo's 3 s
  breath; developer `DNTTK_PROP_LIGHT_PORTAL`) lights the claws from inside,
  with the soft halo at that point. The metal itself gives nothing.
- Evidence: UI slot 9, two views, old look against new: top bright pixels
  (101, 126, 184) -> (70, 84, 113) and (93, 116, 172) -> (68, 83, 113);
  fresh level 5 the same (`top-obey-or-die-before-after.png`,
  `top-zoom-before-after.png`).
- The blood: see revision 4 (revision 3's reading, "the decals' own look",
  was wrong).
- Not done: a general rule. A first build let every static prop more than
  1500 units over its floor skip the floor bake; a tour of levels 0-10 and
  12 found 33 such props drawn near the level starts (12 on level 5, some
  1500-1600 over the street), which would all change look and give back
  part of D41P's GPU saving. Left as a possible follow-up job.

## Revision 4: the glowing blood (user, 2026-11-20)

User, after accepting the top ("without a doubt, thats the one!"): "check
the same save slot (copy mine back over), and look at how glowy that blood
is now. its still glowy". A fresh copy of UI slot 9 (saved 19:02).

- Measured at the save's own view, blood pixels (one fixed mask) against the
  floor around them: Vanilla (72, 6, 4) on 31; Modernized (125, 28, 27) on
  24, the same with LIGHTING ORIGINAL or the crystal lights off, so not a
  light; with the small-model takeover off (`DNTTK_MODEL_DEPTH=0`, the game
  drawing the blood) (74, 7, 3), as Vanilla. (A first measurement at
  another patch with a mask taken per image looked like a Relit ratio; with
  one mask both dimmed alike. The shader's emitter-texel rule was tried and
  made no difference; that change was reverted.)
- Cause (D17AT small-model renderer, `model_depth.inc`): a blood pool's
  faces are flat textured quads, semi-transparent in mode 1 (additive; trace:
  command 0x2e, pages 0x3c/0x3d/0x3f, colours 3f3f3f/7e7e7e/ffffff), their
  palettes' texels STP (13-15 of 16 entries). The game draws them before the
  floor, onto black, and the PS1 mask bit keeps the floor off: solid dark
  crimson. Our host draw comes after the host floor, so the same blend added
  the floor under it and the blood glowed.
- Fix: a flat small-model face in semi mode 1 is drawn opaque (texel times
  its colour, which is what adding it onto black gives; palette entry 0
  stays transparent). As an opaque flat face it takes D17AT's decal path
  (depth, rank). Developer `DNTTK_BLOOD_OPAQUE=0` restores the old draw.
  Game-wide (every blood pool); Vanilla and Classic unchanged.
- Evidence (UI slot 9): blood (125, 28, 27) -> (62, 5, 2) with the crystal
  lights off, (69, 6, 2) with them on (the game: (74, 7, 3)); matched crops
  `research/before-after/D57F/blood-game-old-new.png` (game, before, after),
  `before-blood-slot9.png`, `after-blood-slot9.png`. Framework unchanged
  (patch byte-identical to HEAD).

## Revision 5: the machine did not open (user, 2026-11-20)

User: "check the same slot, slot 6. i used the dnstuff cheat crystals, and
the thing didnt open. thats never happened before. also the red crystal
doesnt show in the slot". Not this job's lights: D08A43's slot rule moved
the red crystal out of the time machine into an LA switch whose +0x35 (1,
on) matched red's number, emptying its holder. Fixed there (device type
flags, a seated crystal never moves, a repair for crystals already moved);
details in [note 519](519-d08a43-crystal-slots.md). UI slot 6: red comes
back on load, goes into holder 7 and the portal opens.

## Revision 6: the open portal went black (user, 2026-11-20)

User, new UI slot 6 (the machine open): "the portal itself just shows a
really weird color, like its gone black". Revision 4 drew every flat
additive small-model face opaque, and the open time portal's sparkle is one
too: its 13 cycling palettes (546c-576c) are mostly near black (0x8421)
with cyan, and black adds nothing over its pad, while drawn opaque the
black texels showed as black speckles (black share of the pad's region 41%
against 13% with the old draw and in Vanilla).

- Fix: only a stain is drawn opaque: a 4-bit palette whose blended entries
  are all coloured (none with every channel 3 of 31 or less). Blood (57ac,
  darkest 0x8007) is one; the portal's are glows and stay additive.
- Palettes are read from the level's VRAM snapshot the neon scan already
  takes (D23Z async, no GPU wait), shared with the renderer
  (`near_clip_set_vram`); with LIGHTING ORIGINAL, where the neon scan does
  not run, `level_vram_share_update` takes the same async snapshot itself.
  A first try read the palette with `gpu_vram_peek`, which syncs the GL
  framebuffer on every read: 11 s on the game thread, dropped.
- The verdicts live in a 64 KB table mapped shared before the redraw
  workers fork (as the D41P floor map): the workers draw the presented
  images and never get the snapshot, so they read the game thread's
  verdicts; until one exists, the game's blend.
- Evidence, UI slot 6 (portal) and 9 (blood), RELIT and LIGHTING ORIGINAL:
  portal black share 13% (as Vanilla), blood (62, 5, 2) and (69, 6, 2)
  against the game's (74, 7, 3); no game-thread stall.
  `research/before-after/D57F/portal-black-fixed-vanilla.png` (revision 4,
  now, Vanilla). Developer `DNTTK_STAIN_TRACE=1` logs each verdict.

## Revision 7: the pad flickered (user, 2026-11-20)

User: the portal sprite works, "the only trouble im seeing is that the pad
itself (as in the model beneath the circular sprite) flickers, when it is
illuminated" (clip `research/screencaps/Video_2026-11-20_21-15-33.mp4`);
"use the real display".

- Reproduced on the real display (windowed, 180 fps, private copy of UI
  slot 6, strafing past the pad): the pad's dome, rim and top under the
  sprite change shade from frame to frame. Not this job's lights (the
  claw-centre light off: still flickers), not the redraw workers (one
  worker: still); gone with models on the live lamps (`DNTTK_MODEL_BAKE=0`),
  so the floor bake.
- Cause (D41P, `near_clip.cpp`): a model takes the bake of the floor under
  its origin, which is computed back from the view (the GTE translation)
  with rounding under a unit. The pad (type 179) stands at x 23552, z 48128,
  exactly on a corner of the 128-unit floor grid: over one walk its lookups
  found floors 3375, 3507, 3445, 3573 or none (2453 lookups), each with
  different baked light. Any model on a grid line could do the same.
- Fix: the lookup point (and the D41P2 per-polygon point) is snapped to 8
  units, so the same place always finds the same cell. After: 2404 lookups,
  all floor 3573 (37 empty ones before the bake was loaded).
  Developer `DNTTK_FLOOR_TRACE_MESH=<hex mesh>` logs a mesh's lookups.
- Evidence: real-display sheets `research/before-after/D57F/pad-flicker-
  before-real-display.png` and `pad-flicker-after-real-display.png`.

## Limits

- Taking a crystal back out of the machine: see the work log (warp-and-press
  contact decides whether the lab's E reaches the holder).
- No halo was added: the light on the holder is the original holders' look.
  If the user wants more "glow", a soft additive halo at the crystal (D41S
  style) is the next step.
- Offscreen runs; the user's play through OBEY OR DIE is the playtest.
  Levels 0 and 9 use the same rule (not exercised).
