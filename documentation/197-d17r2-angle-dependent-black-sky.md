# D17R2 - Sky turns black at certain viewing angles

2026-10-22. Status: Done (user-accepted).
Accepted executable:
`c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`.
User: "fully accepted. lock it in, and close us out". Implementation, tests
and documentation closeout, commit and push authorized. Framework export is
unchanged; its patch applies to the clean pinned dependency and reproduces all
50 modified/new framework files. No rebuild for this documentation closeout.

## Reproduced cause and implementation

Current UI slot 8 (file 07) is LEVEL06, SHA-256
`13c069726aa3ef9d550a2ce7444946124648493f3002747b80986d0f412c2bb5`.
Private copies and evidence: `recomp/analysis/d17r2-sky/`.
The copied player profile is first person, independent camera, 16:9, OpenGL
4x, corrected geometry/textures, extended distance, 120 FPS, 150% CPU.

This is a whole sky-pass omission, not clipping, missing textures or exhausted
packets. At yaw 0/60 degrees, pitch -20, the portal walk lists only room 28
(flags `0x20`). At yaw 90 it also lists room 22 (`0x18`); at yaw 180 rooms
0 (`0x1`) and 1 (`0x9`) also enter. Only visible rooms with bit 8 set request
sky (`0x80062898..0x800628b4`). The renderer returns that request from its
stack +0x28 (`0x8002e830`); the composition saves it in s5 and skips the
entire sky routine when zero (`0x800265b4`). Room 28 has an exposed roof view
but no sky flag. Turning away from its outdoor neighbour therefore turns the
background black. Setting bit 8 in private loaded room-28 RAM immediately
restored the sky, then reverting it reproduced the omission.

`sky_widescreen.cpp` now uses the existing `0x8002ffec` entry, authenticated
caller `0x8002e6dc`, to set the eventual sky return when the level has sky
components. This is AFTER the flag selected the original room far limit, so
visibility, room metadata, far limits and geometry remain unchanged. The
original sky composition still draws the loaded layers behind the world.
The rule is independent of level/room IDs and camera angle, covering the
Modernized first- and third-person views and both real and replay renders.

Gates: Modernized, single player, primary camera, exact caller, valid stack,
nonempty loaded sky, original sky routine identity and authenticated render
return/caller code. A nonzero original request is untouched. Empty-sky levels
and Vanilla are unchanged. No framework changes, new hooks, regeneration,
texture changes or persistent setting. `DNTTK_SKY_BACKGROUND=0` is an
unsaved developer comparison only.

## Candidate evidence

- Reproduced at 60 FPS; disabling only the new rule at 120 reproduces the
  same black sky. Candidate restores yaw 0/60 at pitch -20 at both rates.
- Current slot: 48-view sweep at 120 (30-degree yaw steps, pitches
  0/-20/-40/-60) stays continuous. At -60 the original dark sky fill is
  visible above the cloud band; it is not an omitted black background.
- Moving turns: compositor captures at 60 and two 24-present redraw
  sequences at 120 retain sky, world, weapon and HUD. No failed redraws,
  packet skips or mesh fallbacks in the recorded current-slot checks;
  host arena peak 34320 bytes.
- Same scene at 4:3, third person, and original draw distance with both
  precisions Original: five sampled views per configuration pass at 60.
- Prior D17R1 medieval file 01 and western file 07, D17R western file 08,
  and D17O city file 09: three yaw/pitch views plus moving 24-present
  sequences each at 120. Zero packet skips, mesh fallbacks and failed
  redraws; cumulative host peak 35560 bytes. City black overhead remains
  the original no-fill sky. Sky trace: 360 native records, camera-relative
  offsets zero, no substituted sky transforms.
- Matched three-second idle windows before expensive capture dumps:
  candidate 60.0 guest frames/s, 119.9 presents/s; developer-disabled rule
  60.3 / 119.9. Both have pace divisor 1 and zero failed redraws.
  Capture dumping itself can trigger temporary pacing backoff, so its wall
  time is excluded from the performance comparison.
- Vanilla loads and renders the same private slot with Modernized disabled
  and no redraws. The save inherited a Modernized-menu pause; after clearing
  that flag in private RAM, original pad movement advances Duke normally.
  This is a rendering/movement smoke check, not a full Vanilla regression route.
- All 39 original player save/card files and the saved profile remain
  byte-identical to the intake manifest/copy. Test processes are stopped.
- Incremental player build, `ttk-sky-test EXE`, `ttk-near-test EXE` pass.
  Sky tests exercise background selection at zero/wide margins, each sky
  component, empty sky, Vanilla, split screen, camera/caller/stack gates,
  altered code rejection and preservation of the original nonzero flag and
  far limit. Existing widening and near-packet contracts still pass.

## Acceptance and limits

The user fully accepted the slot-8 fix on 2026-10-22. The background rule can render sky behind
opaque indoor geometry in sky-bearing levels; the world still covers it.
The short performance window and sampled scenes are not a whole-campaign
or long-session performance certification. 240 FPS/Unlimited and every
indoor location were not tested. No separate camera job is started.

Launch: `python3 recomp/tools/local/run.py`; F10 captures the mouse if needed.

## Initial investigation (history)

The following hypotheses were recorded before reproduction; the whole-pass
visibility finding above supersedes them as the cause of this report.

## Report and reproduction

Load current UI save slot 8 (file slot 07). Immediately look left; if needed,
look slightly upwards while facing left. The sky unexpectedly becomes black,
apparently depending on the view direction. Expected: sky remains correctly
rendered through camera turns, with no black gaps or exposed empty background.

Slot numbers are reused. On later implementation, copy the current save and
settings to private storage and record its hash, level, camera, yaw/pitch,
aspect ratio, precision, draw distance and frame rate. Do not assume it is
the LEVEL01 slot 8 used by D17R1 without checking the current save.

## Previous work and initial technical findings

- D17O ([note 92](92-d17o-sky-intake.md)): `frame_replay.cpp::transform_load`
  leaves the three authenticated sky matrix calls camera-relative, using each
  redraw worker's current eye and rotation. `sky_render.h` identifies return
  addresses `0x80038c68`, `0x80038d7c`, `0x80038e18` and guards resident sky
  routine `0x800388e4` (0x578 bytes). A missed caller/identity gate could allow
  the world-object interpolation path; that is a hypothesis to trace, not a
  diagnosis of this report. Compare real frames and redraws during movement.
- D17R ([note 172](172-d17r-sky-edges.md)): `sky_widescreen.cpp` separately
  widens the sky colour fill, scales row 0 of the camera-facing backdrop and
  expands the object-renderer clip rectangle. The rectangle helper acts only
  on an exact -256/+256 pair; the margin also requires a valid sky identity
  and a single-player view. Capture the actual rectangle/matrix/caller at the
  failing angle. Partial backdrop coverage, a missed gate or view-dependent
  near/outcode rejection are plausible candidates, not established causes.
- D17R1 ([note 173](173-d17r1-sky-blocks.md)): near cloud quads went missing
  when the near-clip takeover exhausted its former shared render-ring budget.
  Current `near_clip.cpp` uses four separate 128 KB GPU DMA regions, but
  `mesh_budget_fallback` and packet-budget fallback paths still exist. Inspect
  `host_bytes_peak`, `mesh_fallbacks`, `packet_skips`, culled polygons and the
  failing frame's GP0 packets before ruling out dropped geometry. Preserve
  the aperture-aware redraw walker and vertex-address keys in the exported
  framework patch. Their existence is not proof the current angle is covered.
- The prior sky survey found city levels 0/5/9 have zero fill colour and
  original black overhead. That is context to compare with Vanilla, not an
  explanation for the current slot-8 symptom. Establish the level and missing
  layer first. No texture corruption or missing art has been demonstrated.
- D08AE changes only shallow-water flight eligibility. It does not alter sky,
  clipping or replay code. A regression from a particular sky improvement must
  be demonstrated with matched views/builds rather than inferred from timing.

## Investigation when selected

1. Reproduce the exact left/up angle from a private current slot-8 copy;
   capture both sides of the failure boundary, then sweep yaw and pitch.
2. Compare 4:3/16:9, first/third person, 60/saved high refresh, original/extended
   draw distance and Vanilla. Use `DNTTK_SKY_WIDE=0` and `DNTTK_SKY_CAMERA=0`
   only as controlled diagnostic comparisons, not proposed player workarounds.
3. Trace `DNTTK_SKY_TRACE=1`, sky selectors/fill colour, backdrop matrices,
   clip bounds, per-polygon projection/culling, near-clip budget and OT/GP0
   coverage. Determine whether pixels were never submitted, clipped, culled,
   overdrawn or lost only in a redraw. Inspect motion sequences as well as
   still images; D17R1's first playtest exposed a redraw-only failure.
4. Compare the previous accepted sky scenarios and matched older builds where
   available. Prove the failing path before changing geometry, culling or art.

## Acceptance

- Current slot 8 no longer turns black when looking left or slightly up.
- Sky remains stable throughout camera-angle sweeps and moving turns.
- D17O camera-relative sky, D17R widescreen coverage and D17R1 whole cloud
  polygons remain intact, including their previous private regression views.
- No new rendering, clipping/culling, packet-budget or performance regression;
  Vanilla and existing display/camera modes remain correct.
- Focused sky/near tests, actual presented motion at 60/high refresh and user
  playtest substantiate the fix. Broad claims require corresponding coverage.

Dependencies: D17O, D17R, D17R1 (Done). Priority: minor visual follow-up.
No separate camera or texture job created.
