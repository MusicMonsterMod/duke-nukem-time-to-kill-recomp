# D17AC - Slot-11 phone rear panel and wall depth

2026-10-28. Done, user-accepted. User: "wonderful. lock it in and close it out".
Implementation, tests and documentation commit/push closeout are authorized as
MusicMonsterMod. Acceptance preserves the bounded evidence and limits below.

## Reproduction and cause

The current private UI slot 11 is LEVEL00 (overlay tag 3), with state SHA-256
`5f2144fa0cc6e02cf8445d3facdb0cf2ffe9888f978b0c4f69161e712d5d7ae2`.
This is not D17L's historical tabletop checkpoint. Approaching the blue wall
phone makes its textured rear panel disappear while its surround stays visible.
Retreat restores it. This was reproduced before editing the renderer.

Primitive tracing identifies static instance `0x801daca4`, mesh `0x8011a8d8`,
and rear FT4 face `0x8011a9a4` with indices 0,4,2,6. Those addresses are
local evidence, not repair conditions. The phone panel continues emitting
triangles in both captures; it is not rejected by culling or packet budget.

The mounting wall at face `0x80192a80` moves from native ownership at distance
to host depth-tested ownership on approach. Its plane is parallel to the phone
back, but the phone is embedded in it: sampled normal separations are 11.273
and 9.857 world units. Existing two-unit prop contact tolerance cannot resolve
that authored overlap. Native painter ordering previously kept the phone visible.
This differs from D17L's table painting over a depth-tested cup.

## Bounded repair

`near_clip.cpp` recognizes the retail ten-vertex phone shape at the already
verified static-prop renderer caller. Only its opaque textured rear quad gets
a 16-world-unit depth allowance, and only from the open side of the phone.
Modified vertex shapes, other faces, actors, viewmodels, translucent/faded
surfaces, foreign callers and Vanilla do not acquire this allowance.

The allowance changes depth metadata only. Raster positions, texture coordinates,
perspective weights, native ordering, object placement, collision, interaction
callbacks and the wall remain unchanged. Depth testing remains active; nearer
occluders still win. The allowance is deliberately specific to this embedded
panel, not an increase in all-prop contact tolerance. It can favour this panel
within its 16-unit contact band; it is not an exact geometric wall repair.

Developer comparison `DNTTK_PHONE_BACK_DEPTH=0` restores the previous depth
behavior and reproduces the disappearance on approach. No new player option or
activation step is required. No framework, generated C or asset edits were made.

## Verification and limits

- Player build and owned-EXE near-renderer packet suite pass. New contracts
  verify the bounded depth change, unchanged XY/UV/perspective/order, nearer
  occlusion, and modified-shape/caller/face/transparency/back-side/actor/Vanilla
  exclusions. All four geometry/texture precision combinations and the disabled
  comparison pass. Both primitive-trace observation/gating tests pass.
- Private offscreen approach/retreat captures at 60 and 120 settings retain the
  phone through the reproduced failure range. Separate third-person 120 and
  original-geometry/original-texture 60 routes pass. Sideways and pitch samples
  retain expected surround occlusion and nearby pickup visibility.
- Full-width real/replay captures retain the phone at 120. An attempted injected
  view toggle did not change view; those captures are first person. Third-person
  evidence comes from a separate launch with the live requested view verified.
- A separate expensive replay restoration diagnostic reports 542 comparisons,
  zero mismatches and zero worker failures. It slowed execution substantially;
  it is not performance evidence. Normal 120 runs were repeated without it.
- Vanilla smoke reports Modernized disabled, zero host-depth triangles and zero
  replay workers. This is not an exhaustive Vanilla campaign playtest.
- All 41 original player save/card/settings files retain their intake hashes and
  mtimes. All diagnostic games are closed. Tests used private copies only.

The E attempt did not establish a complete phone interaction lifecycle; no
interaction behavior was changed or newly certified. The user accepted the delivered fix. Other phone placements and exhaustive
high-refresh/campaign coverage remain unverified. No locked frame-rate or audio claim is made from offscreen captures.
Private evidence stays in `recomp/analysis/d17ac/`.

Candidate executable SHA-256:
`695bbf84c857972f852e9aa5813125e13a4220d3858fa6f6fa87a347a1c64e84`.

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Load F7 UI slot 11 and approach/retreat from the phone; also check diagonal
angles and the usual view/refresh setting. Mouse capture is automatic.
