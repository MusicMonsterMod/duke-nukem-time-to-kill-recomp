# D17AE - Slot-11 ceiling above the door

2026-10-29. User-confirmed in play. Known regression: the embedded slot-12
payphone (D17AL). The user-reported symptom (exterior sign post
through the door header) and its scene-depth fix are at the end of this note.
The backface-cull removal described in the next sections was later reverted.

Current UI slot 11 maps to `state_800AB6FC_slot10.pst` under
`recomp/saves/local-play/openbios/`. Testing uses private copies. The saved
view is inside a wood-panelled entrance, looking through an open doorway below
a red EXIT sign.

## Symptom

At the `w01` camera position (approximately 1.5 steps forward from the save),
a black void appears in the upper portion of the screen where a ceiling/wall
panel mesh should be visible. The PSX native render shows the correct green
ceiling tiles. The baseline `w00` position renders correctly on both host and
PSX.

Captures under `recomp/analysis/d17ae/`:
- `gp011-slot11-w01.png` - host, black void in upper area above doorway arch
- `gp011-slot11-w01-native.png` - PSX native, correct green ceiling tiles
- `gp011-slot11-90.png` - host baseline, ceiling correct, EXIT sign visible

Trace: `gp011.1061060.jsonl` (frame 413 = w01 position, frame 207 = baseline).

## Root cause

Mesh `0x8018e1d4` (33 verts, 25 faces) contains the near-doorway ceiling
surface. At frame 207 (baseline), vertices v14 and v15 of faces
`[17,18,14,15]` and `[18,19,15,16]` are safe (sz >= perspective_z=1536), so
these faces are `native_kept` - the PSX renders them and the host does not
re-evaluate them.

At frame 413 (w01), the player has moved closer. v14 (sz=763) and v15
(sz=671) both drop below perspective_z=1536. The near-clip system marks the
whole mesh `host_candidate` via the mesh-level `any` flag. Now the host must
re-evaluate and emit or discard faces that the PSX can no longer safely produce
at the new resolution.

The host's `world()` function at `near_clip.cpp:806` calls `front_facing()`
on each `host_candidate` face. `front_facing()` computes det(v0,v1,v2) in
view space to determine winding order. For face `[17,18,14,15]`:
- v17: pixel_y ~= 81 (upper screen)
- v18: pixel_y ~= 128 (screen center)
- v14: pixel_y ~= -5 (just above top)
- v15: pixel_y ~= 98 (above center)

The determinant is negative - the host treats this as a back-facing polygon
and culls it. Face `[18,19,15,16]` is similarly culled. Together these two
quads cover approximately pixel_y 0-128, which is exactly where the void
appears.

At frame 207 the determinant is also negative, but the face is `native_kept`
so `front_facing()` is never called. If the PSX itself called NCLIP on these
faces, the ceiling would be missing from the native image too - but it is
present. This confirms: TTK's world renderer at `0x80011020` pushes all faces
to the ordering table WITHOUT calling NCLIP. The host backface check does not
have a PSX counterpart for world geometry.

## Fix

Removed the backface cull check from `world()` in `near_clip.cpp` (formerly
line 920). `host_candidate` world faces now always proceed to emit, matching
the PSX world renderer's behaviour.

Removed code:
```cpp
if(!(psx_mod_read_half(record+6)&0x200) && !front_facing(f,p,flags,true)) {
    ++world_stats.culled;trace_record(f,"backface",record);continue;
}
```

Replaced with:
```cpp
// D17AE: TTK's world renderer pushes faces without calling NCLIP, so
// host_candidate faces must also render regardless of winding.
```

The `object()` function at line 1286 retains its own `front_facing()` check.
Object geometry (actors, items) may use NCLIP; only world geometry was
confirmed to skip it.

The `world_stats.culled` counter no longer increments. It is only used in the
`near_clip_debug_json()` diagnostic output and no runtime logic depends on it.

## Test results

Unit test: `.venv/bin/ninja -C recomp/build-local ttk-near-test` - PASS.
No near-clip test exercises the `world_stats.culled` counter directly; the
fix is validated by the PSX/host comparison analysis above.

Full build and gameplay verification at the gp011 w01 position: Needs
playtest.

## Ceiling meshes are unrelated

Calls 90-94 at frame 413 process the large overhead ceiling panels (mesh
`0x80187c38` and others). All their vertices have sz=2197-9750, all above the
2048 threshold for the mesh `any` flag. These meshes are `native_kept` and
handled correctly on both frames. The void is caused solely by the
`host_candidate` world faces on mesh `0x8018e1d4`.

## Movement flicker - root cause and fix

After the static-void fix above, a second symptom remained: ceiling panels
above the gp011 doorway flickered during movement. In bad frames a large black
void covered the entire upper screen. The static position was correct, so this
was a separate failure mode triggered only while moving.

### Root cause

Movement trace `recomp/analysis/d17ae/move_trace.1085935.jsonl` compared good
frame f111 and bad frame f169 for mesh `0x8018e1d4`. The GTE H register
(perspective focal length) changed between frames:
- f111: h=160 (native PSX FOV), ceiling fully visible
- f169: h=277 (widescreen FOV raised from movement), large void

The guard band check in `plane()` (`near_clip.cpp`, formerly line 576) was:

```cpp
case 1: return guard_x*v.p[2]-h*v.p[0];  // right: vx/vz <= guard_x/h
```

With fixed `guard_x=480`:
- At h=160: threshold = 480/160 = 3.0
- At h=277: threshold = 480/277 = 1.73

The narrower threshold at h=277 incorrectly rejected vertices that are still
visible on screen. Example - v25 (vx=1758.5, vz=922.0, screen_x=528):
- At h=160: 480*922 - 160*1758.5 = +161200 -> INSIDE (correct)
- At h=277: 480*922 - 277*1758.5 = -44545  -> OUTSIDE (incorrect)

v11 similarly failed the bottom guard at h=277. Five faces that should be
clipped-and-emitted were entirely clip_rejected, producing the void.

### Fix

Replaced fixed pixel constants with angular ratios in `near_clip.cpp`:
- `constexpr double guard_x=480, guard_y=240` -> `guard_xr=3.0, guard_yr=1.5`
  (3.0 = 480/160, 1.5 = 240/160, the native-FOV ratios)
- Guard plane expressions:
  - Old: `guard_x*v.p[2] - h*v.p[0]` (sign depends on h)
  - New: `guard_xr*v.p[2] - v.p[0]` (sign is h-independent)

The threshold is now a fixed angular ratio (vx/vz <= 3.0, vy/vz <= 1.5)
regardless of the GTE H register. At h=160 the sign is identical (proportional
by 1/h, which is positive). At h=277 the guard is wider, keeping vertices
visible from any FOV inside the clip region.

The `h` parameter in `plane()` is now unused for guard planes (marked
`/*h*/`); it was never used for the near-plane case k=0.

### Verification

After the fix, v25 (vx/vz = 1.907 < 3.0) is INSIDE at any h, and v11
(vy/vz at h=277 bottom guard: 1.5*1024.25 + (-1071.5) = +465 > 0) is also
INSIDE. All five culprit faces now proceed to clip-and-emit.

Movement probe `recomp/analysis/d17ae/probe.py` (SLOTS=10, MOVE=1, TAG=fix)
ran 16 rock frames. All 16 host captures show the ceiling fully rendered with
no void. Captures under `recomp/analysis/d17ae/fix-slot11-rock*.png`.

Unit test `ttk-near-test`: PASS. Full build: clean.

### Guard band validity across D14F FOV range (90-120 deg)

The fixed guard_xr=3.0 is safe across the full user-accessible FOV range:

- At h=160 (FOV=90, native): guard_xr=3.0 = 480/160, identical to original.
- At h=277 (narrower sub-90 view, triggered by game camera during movement):
  guard_xr=3.0 is wider than original 480/277=1.73. Five previously incorrect
  clip_rejects now emit correctly. This is the movement-flicker fix.
- At h=92 (FOV=120, widest user setting): vertices with vx/vz in [3.0, 10.87]
  are safe at h=92 (tx = 92*vx/vz <= 1000), so their faces are native_kept.
  The PSX renders them natively. near_clip only handles faces with very close
  vertices (vx/vz > 10.87), which project to screen_x >> 661 (off screen at
  h=92). The guard at 3.0 clips off-screen host_candidate content; no visible
  gap results.

Static probe (SLOTS=10, VALUES=75, TAG=trace_fov75) and movement probe at
FOV=60 (SLOTS=10, MOVE=1, VALUES=60, TAG=fov60_move) confirm that any
remaining dark areas at sub-90 FOV are consistent between the PSX native
capture and the host render, indicating correct level geometry rather than a
near_clip regression. These FOV settings are outside the D14F user range.

## 2026-10-29 - User symptom identified: exterior sign post over the door header

User clarification: in slot 11 the outdoor "Bootylicious" sign shows through
the top of the doorframe (research/screencaps/flickering.mp4). The visible
artifact is the sign's tan support post drawn over the dark door header
below the red EXIT sign. User re-created slot 11 from a brand-new game after
the 15:36 build containing both fixes above, and it still reproduces.

### Correction to the backface finding above

The retail executable's world renderer does call NCLIP: GTE NCLIP
(`0x4B400006`) occurs at `0x800115e4`, `0x80011734`, `0x80011764` and
`0x80011788` inside `0x80011020`. The premise for removing `front_facing()`
from `world()` is therefore false, and the removal should be reverted
before acceptance. The static w01 void needs a different explanation.

### Evidence that the post is native painter order

Private copy of the new slot 11, offscreen, 16:9:

- The post covers the header in host and native captures at FOV default, 60,
  90 and 120, while walking forward and back, and with
  `DNTTK_NEAR_CLIP=0`. Neither D17AE edit causes or fixes it.
- A GP0 frame dump (`PSX_FORENSICS=1`, `gpu_frame_dump`) at the save view,
  pixel (-14,-35) from the projection centre (native capture pixel 242,85),
  shows the header quad (seq 40891, op 0x3C, src `0x000F4FB8`) drawn and then
  the post quad (seq 40894, op 0x2C, src `0x000F573C`) drawn three packets
  later over it. Both packets are game-built in ordinary RAM, not host
  near-clip pieces.

The post lies outside, physically behind the header, but the original
ordering-table sort places it in a nearer slot. A primitive trace at the same
view matches the header to world mesh `0x8018e1d4` (corners 30,17,27,14;
depths 2117-3363; farthest-corner slot 105) and the post to prop mesh
`0x8012fc6c` from the static-prop loop (depths 3272-3368; average slot
about 103). Both lie beyond the existing near/platform/compact-prop depth
coverage. This is the same class of fault as D17AG (native painter order),
not a missing face or near-clip failure.

Probe: `recomp/analysis/d17ae/gp0probe.py`; coverage helper
`recomp/analysis/d17ae/cover.py`. Player saves/cards unchanged (hash check).

## Scene depth correction

`near_clip.cpp` (Modernized, `DNTTK_SCENE_DEPTH=1` default):

- At each world (`0x80011020`) and scenery object (`0x80010000`) mesh, the
  projected corners are kept with the game's ring cursor. At the next mesh,
  and at the drawing-chain close `0x8001fba0`, the packets that mesh wrote are
  walked. Each opaque polygon whose corners all match unique projected
  corners receives their view depth through `psx_mod_gpu_host_vertex_coverage`
  (native UVs kept; perspective weight only when corrected textures are on,
  as for existing depth-only faces).
- Packets, OT slots, colours and UVs are unchanged. Translucent packets,
  sprites and other unmatched packets, sky, water, the first-person layer,
  Duke's body and Vanilla keep painter order.
- The world backface cull removed earlier is restored.

Framework `render_replay.c`: in a replay worker with PGXP mesh metadata on,
the DMA walk recorded PGXP metadata (painter depth) for every native polygon
after the plugin's record for the same vertex word, replacing it. The worker
now skips PGXP metadata for words a plugin described with depth in the same
job, matching the live path where plugin host vertices take precedence.
`export_runtime_patch.py` run; applying the patch to the clean pinned
dependency reproduces all 55 changed files.

### Evidence (private copies; player files hash-checked unchanged)

- New slot 11, FOV default/60/90/120: post over header with
  `DNTTK_SCENE_DEPTH=0`; hidden with it on at the load view and in 16-frame
  forward/back rocks at 60 FPS (`sd0`/`sd1`) and 120 FPS (`r120`, before the
  replay fix: post still visible in in-between images; `r120b`, after: hidden
  in all 16). Its whole extent lay within the header, so nothing legitimately
  visible through the opening is lost.
- 8-step walks through the doorway on the new save and from the original
  route's `positions/gp011-slot11-w03` state: ceiling intact with the cull
  restored (`walk`, `oldw`).
- Slot 6 (D17AG): canopy holes at the load view with depth off; continuous at
  FOV 64/90/120, 60 and 120 FPS with it on (`sw0`/`sw1`, `g0`/`g1`).
- 12-step turn sweeps on slots 5, 6, 8, 10 and 12 at 60 FPS show no z-fighting,
  missing faces or translucency changes. Static load views differ from
  depth-off only at slot 6's canopy (0.28% of pixels) and under 0.04%
  elsewhere.
- Coverage counter on slot 11: 84% of the game's opaque polygon packets in
  traced meshes matched (the rest are sprites/unsafe-corner faces).
- 120-FPS replay on slots 6 and 12 sweeps: 0 misses/failures; worker time per
  image about 2.12 ms versus 2.02 ms with depth off.
- `ttk-near-test` (new D17AE scene-depth contract with the owned EXE: native
  world packet corners carry view depth; translucent and Vanilla do not; OT
  untouched), controls (d05 fixture), input, aim, gas, loading and live-env
  suites pass.

Limits: offscreen 16:9 only; no 4:3 or other aspect run, no full campaign
sweep, and no live user playtest yet. Sprites and effects drawn by other
routines keep painter order.

## Known regression (D17AL)

User slot 12 (new save): the payphone's front unit is hidden by its wall at
medium distance and pops in close. It is embedded about 10-11 units in the
wall (D17AC). Scene depth gives its distant native packets exact depth; the
host near path (props under 3072) applies the 16-unit allowance. Confirmed by
comparison with `DNTTK_SCENE_DEPTH=0`. Tracked as D17AL.
