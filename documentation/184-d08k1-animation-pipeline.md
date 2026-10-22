# D08K1 - Animation pipeline: skeleton, pose preview and local override

Status: Done (user-accepted 2026-10-22; animation quality continues in D08K6).

Job: [D08K1](../MODERNIZATION_JOBS.md) (split from D08K, 2026-10-22). This is the
first of the crouch-walk sub-jobs: it proves that Duke's skeleton, meshes and
animations can be decoded offline, previewed, edited in Blender and loaded back
into the running game without touching the original media.

## What the formats are

Every format below was read from the original loader/reader code and checked
against live memory in the private lab, not inferred from file names.

### Level asset groups

`/D0/DBnn/` (DB00-DB09 on the US disc) holds one group per level set. The level
loader `0x80024fdc` reads them through a 0x2c-byte record per level at
`0x800be574`:

| Record field | File | Loader result |
| --- | --- | --- |
| +0x00 | `DBnnMOD.BIN` | model table at `[0x800ddb18]`, post-processed by `0x80023610` |
| +0x10 | `DBnnHIE.BIN` | hierarchy table at `[0x800ddb14]`, post-processed by `0x800236d0` |
| +0x04 | `DBnnANM.BIN` | copied to the fixed clip table `0x800d8328` (2048 records) |
| +0x08 | `DBnnFRM.BIN` | frame table at `[0x800d8324]` |
| +0x0c | `DBnnROT.BIN` | command table at `[0x800d8320]` |
| +0x24 / +0x28 | `DBnnA.RAW` / `DBnnB.RAW` | VRAM (512,0) 448x256 and (512,256) 512x256 (`0x800238b0`) |

### HIE (hierarchies)

256 u32 offsets. A hierarchy is a 0x44-byte header and 0x28-byte joint records:

- header byte 0 = joint count (Duke: 19), +0x24 = a 32-entry table mapping a
  ROT joint id to a record (0xff = none), plus two bounding boxes;
- record byte 2 = depth (the parent is the latest record one level up),
  +0x14 = int32 x/y/z offset from the parent, +0x24 = MOD index (the loader
  replaces it with the model pointer), +4..+0x13 = the joint's current
  quaternion at run time (zero on disc);
- record byte 1 = the ROT joint id, **written by the loader from the table**
  (`0x80023830`); it is 0 on disc.

Duke is hierarchy 1 in DB00. His records, joint ids and parts:

| Record | ROT id | Depth | Part |
| --- | --- | --- | --- |
| 0 | 0 | 1 | pelvis (root) |
| 1 | 9 | 2 | chest |
| 2-4 | 10-12 | 3-5 | left shoulder, forearm, hand |
| 5-7 | 13-15 | 3-5 | right shoulder, forearm, gun hand |
| 8 | 20 | 3 | chest attachment (jetpack) |
| 9 | 16 | 3 | head |
| 10-13, 14-17 | 1-4, 5-8 | 2-5 | legs |
| 18 | 21 | 3 | attachment on the second leg |

(The record table agrees with the D08O2A note; the ROT ids are new.)

### MOD (models)

2048 i32 offsets relative to file+0x2000 (-1 = none). A model has its vertex
count at +6, a bounding box at [+0x8], an optional block at [+0xc] that the
loader clears when byte 5 is 0, 8-byte vertices (x, y, z, pad) at **[+0x10]**,
and primitive groups at [+0x14]: u8 GPU code, u8, u16 count, then records.
`0x2c` textured quads and `0x24` textured triangles are 16 bytes (vertex
indices, uv0+CLUT, uv1+tpage, uv2, uv3); `0x28`/`0x20` flat quads/triangles
are 8 and `0x30` gouraud triangles 12. A 0xff code ends the list. Every
DB00 model parses to its end. Duke's 19 meshes use only `0x2c` and `0x24`.

### ANM / FRM / ROT (animation)

- ANM: 2048 clip records {u16 first frame, u16 frame count}; count 0 or first
  0xffff = unused.
- FRM: 8-byte frames {u16 events, u16 ticks, u16 first command, u16 count}.
- ROT: 8-byte commands. Word 1 bits 27-31 = ROT joint id. Word 0 bit 29 set =
  root translation (x = word 0 bits 0-13, y = bits 14-27, z = word 1 bits 0-13,
  signed 14-bit); otherwise a Q12 quaternion x, y, z (signed 14-bit, same
  positions) and w (word 1 bits 14-26, signed 13-bit). Readers `0x80058c70`,
  `0x80058ee0`, `0x80059004`, `0x800597e4`.
- The executable maps an **animation** id (336 of them) to **four clips** at
  `0x800c2d64`, one per channel, with flags at `0x800c2824`. The D08K audit
  read the four clips as alternatives; they are body channels:

| Channel | Player field | Joints (ROT ids) |
| --- | --- | --- |
| 0 | `player+0x60` | root and legs (0-8, 21) |
| 1 | `player+0x74` | right arm (13-15) |
| 2 | `player+0x88` | left arm (10-12) |
| 3 | `player+0x9c` | chest, head, jetpack joint (9, 16, 20) |

  A channel is 0x14 bytes: u16 animation, ticks left, previous frame, current
  frame (absolute FRM indices), state, reverse flag, then two joint masks.
  Weapon upper animations fill only some channels (anim 8 = `(-1, 472, -1, -1)`),
  so **a crouch walk needs only a new channel-0 clip**; the arm and aim
  channels keep working on top of it.
- Pose blending (`0x80059004`): each channel slerps (`0x8002a0c8`) from the
  previous frame to the current one with `t = (ticks - left) / ticks`.
- Root translation: the first frame's y is the clip's starting pelvis offset
  and later frames add to it (crouch enter 109 ends at 278, crouch 105 starts
  at 279, the rolls return to about 279); z is forward travel per frame (walk
  about 100 per 50 ticks, roll about 300 per frame).

### Pose maths

`0x8002a5ac` turns a quaternion into the transpose of the textbook matrix (a
zero quaternion gives the identity). `0x800987cc` builds the scratchpad chain
`S_child = S_parent * R(q)`, `t_child = S_parent * offset + t_parent`, the root
from the actor rotation, and stores the transposed matrices in the actor array
`[player+0x3c]`.

## Tools

- `recomp/tools/local/ttk_anim.py` (read-only on the disc): `info`,
  `dump-clip` (clip -> JSON), `render` (textured software render of any
  animation keyframe or clip JSON), `roundtrip` (decode and re-encode every
  clip of every group), `blender-export`, `blender-import`, `pack`.
- `recomp/tools/local/ttk_anim_blender.py`: the Blender side (run by
  `ttk_anim.py`; Blender 4.0 in background mode). The export is an armature
  (one bone per joint, identity rest orientation, so a bone's pose quaternion
  is the joint's local rotation), one mesh with rigid vertex groups and the
  original texture pages (nearest filtering), and one action per clip with a
  key per original frame (250 ticks/s). The import samples an action at a
  template clip's frame times. A game quaternion `(x, y, z, w)` is the Blender
  quaternion `(w, -x, -y, -z)` normalised; the encoder keeps w >= 0 and within
  the signed 13-bit field (the game normalises, so scale does not matter).
- `recomp/tests/local/test_ttk_anim.py`: encoding, pose layout, Blender
  conversion, pack layout, the 19-joint hierarchy, track channels, the crouch
  root continuity and the byte-exact re-encode of every clip.

Typical use (outputs under ignored `recomp/analysis/`):

```sh
T=recomp/tools/local/ttk_anim.py
python3 $T render --animation 178 --frame 0 --yaw 2.4 out.png
python3 $T dump-clip 105 c0105.json
python3 $T blender-export --animation 178 duke.blend      # edit in Blender
python3 $T blender-import duke.blend a178_legs_c0105 c0105.json c0105.edited.json
python3 $T pack my.pack c0105.edited.json
DNTTK_ANIM_PACK=$PWD/my.pack python3 recomp/tools/local/run.py
```

## Runtime override (`recomp/src/ttk/anim_override.cpp`)

At the player update (`0x8005a210`, an existing entry hook), Modernized only,
with a pack loaded: if the frame/command globals still point into main RAM,
the module

1. checks the loader and reader code (two guarded ranges);
2. finds each pack entry whose original clip (SHA-256 of its ANM record,
   frame records and commands) is loaded, in any group;
3. copies the referenced FRM and ROT tables into a 320 KiB buffer, appends the
   replacement frames and commands, points the clip's ANM record at them and
   repoints `[0x800d8324]` / `[0x800d8320]`;
4. moves any of the player's four channels that were inside the old clip range
   into the new range (frame for frame; otherwise a clip already playing
   steps through unrelated frames until it wraps).

The original tables stay where they were. A level load replaces the globals
and the clip table, so the next update re-applies. The buffer header keeps the
original pointers and clip records.

- Packs: `DNTTK_ANIM_PACK` (comma-separated, diagnostics) or `ttk-anim.pack`
  next to the executable. **No pack ships with this job**; without one nothing
  is allocated and no guest memory is touched.
- Pack format (`TTKANIM1`): u32 count; per entry u16 group, u16 clip, the
  32-byte original clip digest, u16 frames, u16 0, frame records {events,
  ticks, command count, 0}, then the command words. Built by `ttk_anim.py pack`.
- Savestates: the buffer is reserved in the GPU-DMA aperture at the first
  application, after the near clip's startup arena, so a saved state's
  allocation records equal or extend a loading process's (D20C restores or
  adopts them). A loaded state whose globals point at a valid buffer header
  is adopted. Without a pack, or in Vanilla, the originals and clip records
  are restored from the header.
- Vanilla: never applies a pack.

## Evidence

Private lab `recomp/analysis/d08k1-anim/` (ignored): offscreen, port 9401,
private copies of lab cards and savestates (slot 5: LEVEL00 street, slot 10:
made by the lab). No player card, save or preference was used.

| Check | Result |
| --- | --- |
| `ttk_anim.py roundtrip` | 9,481 clips in DB00-DB09 decode and re-encode **byte for byte** (0 mismatches) |
| Live VRAM (x 512-1023) vs `DBnnA/B.RAW` placement | identical (except a 64-pixel strip at x 960-1023, y 0-255 that A.RAW does not cover) |
| Offline FK from live joint quaternions vs the game's joint matrices (`verify.py`, 40 torn-free samples) | positions within 4.4 units, rotations within 24/4096 (fixed-point rounding; the exact integer path and `0x800b3e5c` renormalisation are not emulated) |
| Decoded frames slerped at the channel's (previous, current, t) vs the live joint quaternions | idle 63: 1.0/4096; walk 76/78: 6.0; crouch transition 176/178: 4.2; crouch 216: 7.8; roll 181: 6.8 |
| Render of anim 63 vs the game's camera view | same model, textures and pose (`preview/compare.png`); keyframe renders of 63, 176, 178 and 181 (`preview/sheet1.png`) |
| In-game captures of anims 178 and 181 beside renders of the decoded pose (`preview/e-side-by-side.png`) | torso, arms and lean agree; the chase camera crops the legs and the screenshot trails the state read by a frame or two, so these are illustrations |
| Blender export -> import of clip 105 | rotations within 1.08/4096; with `--keep-original` byte-identical |
| Override `rt.pack` (idle clips 0-3 re-encoded through Blender, not kept) in play | applied (`frames 5224+16, commands 21569+52`); 152 channel reads on the new frames, 456 joint checks: within **2.39/4096** of the original clip |
| Override `demo.pack` (idle legs clip 0 replaced by the crouch clip's frames) | Duke idles kneeling with his own idle arms (`shots/d-final-sheet.png`); the clip loops 5228 -> 5224 |
| Save with the demo override, reload with the pack | `modmem=restored`, buffer adopted, override still playing |
| Same save, reload without a pack / in Vanilla | originals restored (`0x80145774` / `0x8014fab4`, clip 0 = original frames) |
| Vanilla with the pack | not applied |
| Python suite | 175 tests OK (2 skipped), including 11 new `test_ttk_anim.py` |
| Native suites | `ttk-input-test`, `ttk-near-test`, `ttk-aim-test`, `ttk-controls-test` PASS |

## Limits

- The preview renderer is flat (no lighting) and approximates the game camera;
  pose agreement is established numerically, the images are illustrations.
- Root x/z travel is shown as data, not applied in the preview.
- The rebase covers the player's channels only. Another actor already playing
  a replaced clip at the moment of application would step through unrelated
  frames until its clip restarts. Pack entries are meant for Duke's clips.
- New clips need ids and animation ids. Clip ids are free in ANM (DB00 uses
  917 of 2048), but the animation -> clip table is in the executable (336
  entries); D08K3/D08K4 must add a host-side animation id or reuse one.
- Only DB00 was checked live; the format decode and round trip cover DB00-DB09.
