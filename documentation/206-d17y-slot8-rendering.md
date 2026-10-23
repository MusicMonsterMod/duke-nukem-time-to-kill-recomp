# D17Y - Slot 8 fire platforms and upper castle walls

2026-10-23. Done - user accepted and authorized closeout.

User confirmed: "this fix is excellent. it's completely resolved these graphical
oddities. i fully accept. lock it in. close it out". This accepts both reported
slot-8 fixes. Implementation, tests and this evidence are included in the
requested commit/push closeout. The runtime patch was re-exported unchanged
and checked against the clean pinned framework.

## Reproduction and isolation

Private copy of the current UI slot 8 (file slot07), LEVEL06. State SHA-256:
`13c069726aa3ef9d550a2ce7444946124648493f3002747b80986d0f412c2bb5`.
Sidecar SHA-256:
`f16c0d679c70a6fd1b0db2bc32ff18ef537c0be1963ca4a8188c63fcc14e67f3`.
Player settings: Modernized, first person, independent camera, corrected
geometry/textures, extended draw distance, OpenGL 4x, 16:9, Auto frame rate,
150% CPU fast timing, strong view bob and manual jump.

Evidence is local under `recomp/analysis/d17y-20261023/`. All launches used
private profiles/cards, offscreen video, dummy audio and debug port 9351.
The save loads paused; the harness resumes before steering/jumping. Its
`matrix-*.json` records player/eye positions, camera angles and visible rooms.
Each route captures takeoff, ascent, apex, descent and landing. Fire view:
absolute yaw 110, pitch 20 degrees; wall view: yaw 180, pitch -8 degrees.

## Fire platform

The grating itself was not losing its depth. Four faces in the animated
fire object (mesh `0x8012774c`) reference the game's missing-texture descriptor
at `0x800c5784`. Its three texture words are zero. The original bank fallback
at `0x8003cd6c` also returns this descriptor. Resolving those faces produces
CLUT 0 and TPAGE 0x60, with UVs in 0..31: these sample the live framebuffer.
Black patches then cover the separate grating surface as the viewpoint and
render order change. The valid flame animation uses a different descriptor.

The artifact persisted with near clipping, precise geometry/textures, host
depth, plane correction and GL batching individually disabled. Software
rendering did not show the same black covering in the matched jump. This
comparison isolates a rendering-path difference; it does not establish exact
retail hardware texture-cache behavior.

`near_clip.cpp` now removes only animated object faces referencing that exact
zero sentinel, from a private copied polygon list, before either native or
host rendering. The original mesh stays intact. Valid descriptors with zero
words are retained. The filter is Modernized-only, identity guarded, bounded,
and works with near clipping disabled and in third-person 4:3. It does not
use a level, save slot, object type or instance address. Developer A/B:
`DNTTK_MISSING_ANIMATION=0` restores the previous path.

## Upper castle walls

Visible rooms stayed `[28,22,0,1]` throughout the failing jump. The top of room
22's portal rectangle moved down from the root view's -120 edge to roughly
-104/-91/-80 on ascent. Native wall faces above that edge were rejected.
Room 28's direct portal into the taller sky-bearing room 22 ends at room 28's
upper bound. The independent camera can see the neighboring walls above it.

`draw_distance.inc` extends that direct neighbor's upper clip edge to the root
view top when the portal starts at the source room's upper bound and leads
into a taller sky-bearing room. Horizontal and lower bounds retain the existing
margin behavior. Indirect rooms and interior/shorter openings retain their
apertures. Extending every visible room was rejected because it exposed distant
geometry through other openings. Developer A/B: `DNTTK_OPEN_PORTAL_TOP=0`.
The change belongs to Modernized Extended draw distance; Original and Vanilla
keep their previous behavior. Sky geometry and room flags are unchanged.

## Verification

- Player build passes. Executable SHA-256:
  `793c6e6c5ffa28835d24ca141447a50cac48ef23372c26a7f1e8999c994e4eb8`.
- `ttk-near-test` passes with the owned EXE, both original and corrected
  precision. New tests cover copied-list filtering, preservation of a valid
  zero descriptor, source preservation, malformed lists, context and Vanilla.
  Existing near/depth/coplanar/whole-polygon packet contracts also pass.
- `ttk-controls-test` passes with the owned EXE, clean LEVEL00 fixture,
  LEVEL01 and the complete available overlay directory. New portal tests cover
  the open edge, indirect/indoor/equal-height/short openings, invalid pointers,
  caller guard and Vanilla. Existing controls/sky/camera tests pass.
- Live matrix: first/third person, 4:3/16:9, 60/120 FPS, both jump routes.
  The grating and upper wall strips remain visible in reviewed captures;
  flame animation, nearby platforms, Duke and the established sky remain.
- Saved Auto is also checked offscreen. No physical monitor refresh is
  available there, so Auto resolves to 60; explicit 120 runs exercise the
  high-refresh replay path. The subsequent user retest fully accepted both fixes; the user did not
  separately report the resolved monitor refresh rate.
- Player profile and all 39 files in the player's save/card directory match
  the pre-work hashes. No new framework changes. Commit/push closeout was explicitly authorized
  after the user retest.

This is focused slot-8 and native regression evidence plus user acceptance
of both reported fixes, not full campaign visual acceptance. No further
D17Y playtest is pending.

Launch: `python3 recomp/tools/local/run.py`. Gameplay captures automatically;
Escape pauses/releases the mouse and resume recaptures. No F10 step.
