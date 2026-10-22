#!/usr/bin/env python3
"""Duke Nukem: Time to Kill skeletal model and animation pipeline (D08K1).

Read-only decoder/encoder for an owned USA SLUS-00583 disc's per-group assets
(``/D0/DBnn/DBnnHIE|MOD|ANM|FRM|ROT.BIN`` and the ``DBnnA/B.RAW`` texture
pages), validated against the original readers (documentation note 184):

- HIE (loader callback 0x800236d0): 256 u32 offsets; a hierarchy is a 0x44-byte
  header (byte 0 joint count, +0x24 a 32-entry ROT-joint -> record map, 0xff =
  none) and 0x28-byte joint records (byte 1 ROT joint id, written by the loader
  from the map, byte 2 depth, +4..+0x13
  runtime quaternion, +0x14 int32 x/y/z offset from the parent, +0x24 MOD index).
- MOD (callback 0x80023610): 2048 i32 offsets relative to file+0x2000; a model
  has its vertex count at +6, a bounding box at [+0x8], 8-byte vertices at
  [+0x10] and primitive groups at [+0x14] (u8 GPU code, u8, u16 count; 0xff ends).
- ANM: 2048 (u16 first FRM, u16 count) clip records. FRM: 8-byte frames (u16
  events, u16 ticks, u16 first ROT, u16 count). ROT: 8-byte commands, word1 top
  five bits = ROT joint id; word0 bit 29 = root translation (three signed 14-bit
  fields), otherwise a Q12 quaternion x, y, z (signed 14 bits) and w (signed 13).
- Animation id -> four clip ids (one per channel: legs/root, right arm, left
  arm, chest+head) from the executable table 0x800c2d64; per-animation flags at
  0x800c2824.
- Pose: quaternion -> matrix as 0x8002a5ac (the transpose of the textbook form,
  zero quaternion = identity) and composition as 0x800987cc / 0x800b42ec:
  S_child = S_parent * R(q), t_child = S_parent * offset + t_parent. The actor
  array (player+0x3c) stores the transposed matrices.

Run from the workspace root. Original media stay read-only; outputs go to the
paths you name (keep them under recomp/analysis/ or another ignored folder,
except tracked override packs built from our own animation data).
"""
import argparse
import hashlib
import json
import math
import struct
import subprocess
import sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
WORKSPACE = HERE.parents[2]
EXE_PATH = WORKSPACE / 'recomp/disc/SLUS_005.83'
DISC_PATH = WORKSPACE / 'recomp/disc/time-to-kill.bin'
EXE_BASE = 0x80010000 - 0x800
ANIM_TRACKS = 0x800c2d64
ANIM_FLAGS = 0x800c2824
ANIMATIONS = 336
CHANNELS = ('legs', 'right_arm', 'left_arm', 'chest_head')
DUKE_HIERARCHY = 1


def sext(value, bits):
    value &= (1 << bits) - 1
    return value - (1 << bits) if value & (1 << (bits - 1)) else value


# ---------------------------------------------------------------- disc access
def read_exe():
    from disc_lab import EXE_SHA256
    data = EXE_PATH.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        raise SystemExit('recomp/disc/SLUS_005.83 is not the verified USA executable')
    return data


def read_group(group):
    """All files of /D0/DBnn as {short name: bytes} (e.g. 'HIE', 'A.RAW')."""
    from disc_lab import Disc
    prefix = f'/D0/DB{group:02d}/'
    disc = Disc(DISC_PATH)
    try:
        out = {}
        for f in disc.files():
            if f['path'].startswith(prefix):
                name = f['path'][len(prefix):].split(';')[0]
                key = name[len(f'DB{group:02d}'):] if name.startswith(f'DB{group:02d}') else name
                out[key.replace('.BIN', '')] = disc.read(f['lba'], f['size'])
        return out
    finally:
        disc.close()


def exe_tables(exe=None):
    exe = exe or read_exe()
    def at(address, size):
        return exe[address - EXE_BASE:address - EXE_BASE + size]
    tracks = [struct.unpack_from('<4h', at(ANIM_TRACKS, ANIMATIONS * 8), 8 * i) for i in range(ANIMATIONS)]
    flags = list(struct.unpack_from(f'<{ANIMATIONS}I', at(ANIM_FLAGS, ANIMATIONS * 4)))
    return tracks, flags


# ------------------------------------------------------------- HIE and MOD
def parse_hierarchies(hie):
    out = {}
    for index, offset in enumerate(struct.unpack_from('<256I', hie)):
        if not offset:
            continue
        count = hie[offset]
        table = list(hie[offset + 0x24:offset + 0x44])
        # The loader (0x80023830) writes record byte 1 from this table; on disc it is 0.
        rot_of = {record: rot for rot, record in enumerate(table) if record != 0xff}
        joints = []
        for r in range(count):
            rec = hie[offset + 0x44 + 0x28 * r:offset + 0x44 + 0x28 * (r + 1)]
            joints.append(dict(rot_id=rot_of.get(r, -1), depth=rec[2], offset=list(struct.unpack_from('<3i', rec, 0x14)),
                               model=struct.unpack_from('<I', rec, 0x24)[0]))
        out[index] = dict(count=count, header=hie[offset:offset + 0x24].hex(),
                          map=list(hie[offset + 0x24:offset + 0x44]), joints=joints)
    return out


PRIM_SIZES = {0x2c: 16, 0x24: 16, 0x28: 8, 0x20: 8, 0x30: 12}


def parse_model(mod, index):
    offset = struct.unpack_from('<i', mod, 4 * index)[0]
    if offset == -1:
        return None
    o = offset + 0x2000
    count = struct.unpack_from('<H', mod, o + 6)[0]
    # +0x8 bounding box, +0xc optional (cleared by the loader when byte 5 is 0),
    # +0x10 vertices, +0x14 primitives.
    _, _, vertex_at, prim_at = struct.unpack_from('<4I', mod, o + 8)
    verts = [struct.unpack_from('<4h', mod, o + vertex_at + 8 * v) for v in range(count)]
    prims = []
    p = o + prim_at
    while True:
        code, _, n = struct.unpack_from('<BBH', mod, p)
        p += 4
        if code == 0xff:
            break
        size = PRIM_SIZES[code]
        for _ in range(n):
            b = mod[p:p + size]
            p += size
            if code in (0x2c, 0x24):
                k = 4 if code == 0x2c else 3
                uv = [(b[4], b[5]), (b[8], b[9]), (b[12], b[13]), (b[14], b[15])][:k]
                prims.append(dict(code=code, idx=list(b[:k]), uv=uv, clut=struct.unpack_from('<H', b, 6)[0],
                                  tpage=struct.unpack_from('<H', b, 10)[0]))
            else:
                k = 4 if code == 0x28 else 3
                prims.append(dict(code=code, idx=list(b[:k]), color=list(b[4:7])))
    return dict(index=index, vertices=[list(v[:3]) for v in verts], bbox=list(struct.unpack_from('<6h', mod, o + 0x18)),
                prims=prims)


# -------------------------------------------------------- ANM / FRM / ROT
def decode_command(word0, word1):
    joint = word1 >> 27
    if (word0 >> 29) & 1:
        return dict(joint=joint, root=[sext(word0, 14), sext(word0 >> 14, 14), sext(word1, 14)],
                    flags=word0 >> 28)
    return dict(joint=joint, q=[sext(word0, 14), sext(word0 >> 14, 14), sext(word1, 14), sext(word1 >> 14, 13)],
                flags=word0 >> 28)


def encode_command(cmd):
    """Exact inverse of decode_command (including unused flag bits)."""
    flags = cmd.get('flags', 2 if 'root' in cmd else 0)
    if 'root' in cmd:
        x, y, z = cmd['root']
        for v in (x, y, z):
            if not -8192 <= v < 8192:
                raise ValueError(f'root component {v} out of 14-bit range')
        word0 = (x & 0x3fff) | ((y & 0x3fff) << 14) | ((flags & 0xf) << 28)
        word1 = (z & 0x3fff) | (cmd['joint'] << 27)
        return word0, word1
    x, y, z, w = cmd['q']
    for v, bits in ((x, 14), (y, 14), (z, 14), (w, 13)):
        if not -(1 << (bits - 1)) <= v < (1 << (bits - 1)):
            raise ValueError(f'quaternion component {v} out of {bits}-bit range')
    word0 = (x & 0x3fff) | ((y & 0x3fff) << 14) | ((flags & 0xf) << 28)
    word1 = (z & 0x3fff) | ((w & 0x1fff) << 14) | (cmd['joint'] << 27)
    return word0, word1


def frame_count(anm):
    """Frames the ANM table references (entries with first 0xffff are unused)."""
    return max((s + n for s, n in (struct.unpack_from('<HH', anm, 4 * i) for i in range(2048)) if n and s != 0xffff),
               default=0)


def decode_clip(anm, frm, rot, clip):
    first, count = struct.unpack_from('<HH', anm, 4 * clip)
    if not count or first == 0xffff:
        return None
    frames = []
    for i in range(first, first + count):
        events, ticks, begin, n = struct.unpack_from('<4H', frm, 8 * i)
        frames.append(dict(events=events, ticks=ticks,
                           commands=[decode_command(*struct.unpack_from('<II', rot, 8 * j)) for j in range(begin, begin + n)]))
    return dict(clip=clip, frames=frames)


def encode_clip_frames(clip, rot_base):
    """FRM and ROT bytes for a clip whose commands start at ROT index rot_base."""
    frm, rot = bytearray(), bytearray()
    at = rot_base
    for f in clip['frames']:
        frm += struct.pack('<4H', f['events'], f['ticks'], at, len(f['commands']))
        for c in f['commands']:
            rot += struct.pack('<II', *encode_command(c))
        at += len(f['commands'])
    return bytes(frm), bytes(rot)


# ------------------------------------------------------------------- pose
def quat_matrix(q):
    """0x8002a5ac in float: the transpose of the textbook matrix of (x,y,z,w)."""
    x, y, z, w = (v / 4096.0 for v in q)
    n = x * x + y * y + z * z + w * w
    if n <= 0:
        return np.eye(3)
    s = 2.0 / n
    xx, yy, zz, xy, xz, yz = x * x * s, y * y * s, z * z * s, x * y * s, x * z * s, y * z * s
    wx, wy, wz = w * x * s, w * y * s, w * z * s
    return np.array([[1 - (yy + zz), xy + wz, xz - wy],
                     [xy - wz, 1 - (xx + zz), yz + wx],
                     [xz + wy, yz - wx, 1 - (xx + yy)]])


def slerp(a, b, t):
    """0x8002a0c8 in float (shortest path; linear below a 1/16 angle gap)."""
    a, b = np.asarray(a, float), np.asarray(b, float)
    c = float(a @ b) / (4096.0 * 4096.0)
    if c < 0:
        b, c = -b, -c
    if 1 - c < 256 / 4096:
        return a * (1 - t) + b * t
    th = math.acos(min(c, 1.0))
    return (math.sin((1 - t) * th) * a + math.sin(t * th) * b) / math.sin(th)


def joint_quats_for_frame(frame):
    return {c['joint']: c['q'] for c in frame['commands'] if 'q' in c}


def pose(hierarchy, quats, root_matrix=None, root_position=(0, 0, 0)):
    """World (scratchpad-convention) matrices and positions for each record.

    quats maps ROT joint id -> Q12 quaternion; missing joints keep identity.
    """
    root_matrix = np.eye(3) if root_matrix is None else root_matrix
    stack, out = {}, []
    for j, joint in enumerate(hierarchy['joints']):
        local = quat_matrix(quats.get(joint['rot_id'], (0, 0, 0, 0)))
        if j == 0:
            m, t = root_matrix @ local, np.asarray(root_position, float)
        else:
            pm, pt = stack[joint['depth'] - 1]
            m, t = pm @ local, pm @ np.asarray(joint['offset'], float) + pt
        stack[joint['depth']] = (m, t)
        out.append((m, t))
    return out


def cumulative_root(frames):
    """Root track up to the last given frame: the first frame's y is the clip's
    starting pelvis offset and later frames add to it (crouch enter 109 ends at
    278, crouch 105 starts at 279, the rolls return to it); x and z are per-frame
    travel and are returned as the last frame's values."""
    y, last = 0, [0, 0, 0]
    for f in frames:
        for c in f['commands']:
            if 'root' in c:
                y += c['root'][1]
                last = c['root']
    return [last[0], y, last[2]]


def root_position(hierarchy, roots):
    """Pelvis position over the floor (y = 0): the root record's height plus the
    legs track's root translation height, as 0x80058c70 sets the actor's y.
    Horizontal root components are movement data and are not applied."""
    y = roots.get(0, [0, 0, 0])[1]
    return (0.0, float(hierarchy['joints'][0]['offset'][1] + y), 0.0)


# --------------------------------------------------------------- textures
def vram_from_group(files):
    """1024x512 16-bit VRAM with the group's texture pages where 0x800238b0 puts
    them: A.RAW at (512,0) 448x256 and B.RAW at (512,256) 512x256."""
    vram = np.zeros((512, 1024), np.uint16)
    a = np.frombuffer(files['A.RAW'], '<u2').reshape(256, 448)
    b = np.frombuffer(files['B.RAW'], '<u2').reshape(256, 512)
    vram[0:256, 512:960] = a
    vram[256:512, 512:1024] = b
    return vram


def bgr555_rgba(v):
    v = np.asarray(v, np.uint32)
    r, g, b = (v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3
    a = np.where(v == 0, 0, 255)
    return np.stack([r | (r >> 5), g | (g >> 5), b | (b >> 5), a], -1).astype(np.uint8)


def texture_page(vram, tpage, clut):
    """256x256 RGBA image of a texture page as seen through a CLUT."""
    px, py = (tpage & 0xf) * 64, ((tpage >> 4) & 1) * 256
    depth = (tpage >> 7) & 3
    cx, cy = (clut & 0x3f) * 16, clut >> 6
    if depth == 0:
        words = vram[py:py + 256, px:px + 64]
        idx = np.stack([(words >> (4 * k)) & 0xf for k in range(4)], -1).reshape(256, 256)
        return bgr555_rgba(vram[cy, cx:cx + 16][idx])
    if depth == 1:
        words = vram[py:py + 256, px:px + 128]
        idx = np.stack([words & 0xff, words >> 8], -1).reshape(256, 256)
        return bgr555_rgba(vram[cy, cx:cx + 256][idx])
    return bgr555_rgba(vram[py:py + 256, px:px + 256])


# --------------------------------------------------------------- the model
class Group:
    def __init__(self, group):
        self.group = group
        self.files = read_group(group)
        self.tracks, self.flags = exe_tables()
        self.hierarchies = parse_hierarchies(self.files['HIE'])
        self._models = {}
        self._vram = None
        self._pages = {}

    @property
    def anm(self): return self.files['ANM']
    @property
    def frm(self): return self.files['FRM']
    @property
    def rot(self): return self.files['ROT']

    def model(self, index):
        if index not in self._models:
            self._models[index] = parse_model(self.files['MOD'], index)
        return self._models[index]

    def clip(self, clip):
        return decode_clip(self.anm, self.frm, self.rot, clip)

    def page(self, tpage, clut):
        if self._vram is None:
            self._vram = vram_from_group(self.files)
        key = (tpage & 0x1ff, clut)
        if key not in self._pages:
            self._pages[key] = texture_page(self._vram, tpage, clut)
        return self._pages[key]

    def animation_quats(self, animation, frame_index, hierarchy=None):
        """Q12 quaternions of one keyframe of every track of an animation.

        frame_index is clamped per track (tracks may have different lengths)."""
        quats, roots = {}, {}
        for channel, clip_id in enumerate(self.tracks[animation]):
            if clip_id < 0:
                continue
            clip = self.clip(clip_id)
            if not clip:
                continue
            k = min(frame_index, len(clip['frames']) - 1)
            quats.update(joint_quats_for_frame(clip['frames'][k]))
            height = [c['root'] for f in clip['frames'][:k + 1] for c in f['commands'] if 'root' in c]
            if height:
                roots[channel] = cumulative_root(clip['frames'][:k + 1])
        return quats, roots


# ---------------------------------------------------------- software render
def render(group, hierarchy, mats, path, size=(480, 640), yaw=0.0, pitch=0.25, distance=1900.0, center=None,
           background=(40, 44, 52)):
    """Textured, z-buffered, nearest-sampled render of a posed model to PNG."""
    from PIL import Image
    h, w = size
    tris = []
    for j, joint in enumerate(hierarchy['joints']):
        model = group.model(joint['model'])
        if not model:
            continue
        m, t = mats[j]
        verts = np.array(model['vertices'], float) @ m.T + t
        for p in model['prims']:
            if 'uv' not in p:
                continue
            idx = p['idx']
            corners = [(0, 1, 2), (1, 3, 2)] if p['code'] == 0x2c else [(0, 1, 2)]
            for a, b, c in corners:
                tris.append((verts[[idx[a], idx[b], idx[c]]], np.array([p['uv'][a], p['uv'][b], p['uv'][c]], float),
                             p['tpage'], p['clut']))
    allv = np.concatenate([t[0] for t in tris])
    if center is None:
        center = (allv.min(0) + allv.max(0)) / 2
    cy, sy, cp, sp = math.cos(yaw), math.sin(yaw), math.cos(pitch), math.sin(pitch)
    # camera looks at center from distance; PSX y is down
    rot_y = np.array([[cy, 0, -sy], [0, 1, 0], [sy, 0, cy]])
    rot_x = np.array([[1, 0, 0], [0, cp, sp], [0, -sp, cp]])
    view = rot_x @ rot_y
    focal = 1.6 * h
    color = np.zeros((h, w, 3), np.uint8)
    color[:] = background
    depth = np.full((h, w), np.inf)
    for v, uv, tpage, clut in tris:
        c = (v - center) @ view.T
        z = c[:, 2] + distance
        if (z < 10).any():
            continue
        sx = w / 2 + focal * c[:, 0] / z
        syy = h / 2 + focal * c[:, 1] / z
        x0, x1 = int(max(0, math.floor(sx.min()))), int(min(w - 1, math.ceil(sx.max())))
        y0, y1 = int(max(0, math.floor(syy.min()))), int(min(h - 1, math.ceil(syy.max())))
        if x1 < x0 or y1 < y0:
            continue
        area = (sx[1] - sx[0]) * (syy[2] - syy[0]) - (sx[2] - sx[0]) * (syy[1] - syy[0])
        if abs(area) < 1e-9:
            continue
        gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
        l1 = ((sx[2] - sx[0]) * (gy - syy[0]) - (syy[2] - syy[0]) * (gx - sx[0])) / -area
        l2 = ((sx[1] - sx[0]) * (gy - syy[0]) - (syy[1] - syy[0]) * (gx - sx[0])) / area
        l0 = 1 - l1 - l2
        inside = (l0 >= 0) & (l1 >= 0) & (l2 >= 0)
        if not inside.any():
            continue
        # perspective-correct interpolation
        iz = l0 / z[0] + l1 / z[1] + l2 / z[2]
        zz = 1 / np.where(iz > 0, iz, 1)
        u = (l0 * uv[0, 0] / z[0] + l1 * uv[1, 0] / z[1] + l2 * uv[2, 0] / z[2]) * zz
        vv = (l0 * uv[0, 1] / z[0] + l1 * uv[1, 1] / z[1] + l2 * uv[2, 1] / z[2]) * zz
        page = group.page(tpage, clut)
        texel = page[np.clip(vv.astype(int), 0, 255), np.clip(u.astype(int), 0, 255)]
        sub = depth[y0:y1 + 1, x0:x1 + 1]
        write = inside & (zz < sub) & (texel[..., 3] > 0)
        sub[write] = zz[write]
        color[y0:y1 + 1, x0:x1 + 1][write] = texel[..., :3][write]
    Image.fromarray(color).save(path)
    return center


# -------------------------------------------------------- clip JSON format
def clip_to_json(group, clip_id):
    clip = group.clip(clip_id)
    if not clip:
        raise SystemExit(f'clip {clip_id} is not present in DB{group.group:02d}')
    return dict(format='ttk-anim-clip', version=1, group=group.group, clip=clip_id,
                source_sha256=clip_digest(group, clip_id), frames=clip['frames'])


def clip_digest(group, clip_id):
    """SHA-256 of a clip's ANM record, FRM records and ROT commands (original)."""
    first, count = struct.unpack_from('<HH', group.anm, 4 * clip_id)
    h = hashlib.sha256(struct.pack('<HH', first, count))
    for i in range(first, first + count):
        rec = group.frm[8 * i:8 * i + 8]
        _, _, begin, n = struct.unpack_from('<4H', rec)
        h.update(rec[:4] + struct.pack('<H', n))
        h.update(group.rot[8 * begin:8 * (begin + n)])
    return h.hexdigest()


# ------------------------------------------------------- override packs
PACK_MAGIC = b'TTKANIM1'


def build_pack(entries, path):
    """Binary override pack read by recomp/src/ttk/anim_override.cpp.

    Layout (little endian): magic 'TTKANIM1', u32 entry count, then per entry:
    u16 group, u16 clip, 32-byte SHA-256 of the original clip (clip_digest),
    u16 frame count, u16 reserved, frame records (u16 events, u16 ticks,
    u16 command count, u16 reserved), then u32 command pairs (word0, word1).
    """
    out = bytearray(PACK_MAGIC + struct.pack('<I', len(entries)))
    for e in entries:
        out += struct.pack('<HH', e['group'], e['clip']) + bytes.fromhex(e['source_sha256'])
        out += struct.pack('<HH', len(e['frames']), 0)
        for f in e['frames']:
            out += struct.pack('<4H', f['events'], f['ticks'], len(f['commands']), 0)
        for f in e['frames']:
            for c in f['commands']:
                out += struct.pack('<II', *encode_command(c))
    Path(path).write_bytes(bytes(out))
    return hashlib.sha256(out).hexdigest()


# --------------------------------------------------------- Blender bridge
BLENDER_SCRIPT = HERE / 'ttk_anim_blender.py'


def blender_model_json(group, hierarchy_index):
    """Rest skeleton, meshes and texture pages for the Blender side."""
    hierarchy = group.hierarchies[hierarchy_index]
    rest = pose(hierarchy, {})
    joints, meshes, pages = [], [], {}
    parent_at = {}
    for j, joint in enumerate(hierarchy['joints']):
        parent = parent_at.get(joint['depth'] - 1, -1) if j else -1
        parent_at[joint['depth']] = j
        joints.append(dict(name=f"j{j:02d}_r{joint['rot_id']:02d}", rot_id=joint['rot_id'], parent=parent,
                           head=list(rest[j][1]), offset=joint['offset']))
        model = group.model(joint['model'])
        faces = []
        for p in model['prims'] if model else []:
            if 'uv' not in p:
                continue
            key = f"{p['tpage'] & 0x1ff:04x}_{p['clut']:04x}"
            pages[key] = (p['tpage'], p['clut'])
            order = [0, 1, 3, 2] if p['code'] == 0x2c else [0, 1, 2]
            faces.append(dict(idx=[p['idx'][k] for k in order], uv=[list(p['uv'][k]) for k in order], page=key))
        meshes.append(dict(joint=j, vertices=[list(np.asarray(v, float) + rest[j][1]) for v in model['vertices']]
                           if model else [], faces=faces))
    return dict(group=group.group, hierarchy=hierarchy_index, joints=joints, meshes=meshes), pages


def run_blender(args):
    cmd = ['blender', '-b', '--factory-startup', '--python', str(BLENDER_SCRIPT), '--'] + [str(a) for a in args]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0 or 'TTK_BLENDER_OK' not in r.stdout:
        sys.stderr.write(r.stdout[-4000:] + r.stderr[-4000:])
        raise SystemExit('blender step failed')
    return r.stdout


def quat_to_blender(q):
    """Game Q12 quaternion -> unit Blender pose quaternion (w, x, y, z).

    The game's matrix is the textbook matrix of the conjugate, so the bone
    rotation is (w, -x, -y, -z) normalised; a zero quaternion is identity."""
    x, y, z, w = q
    n = math.sqrt(x * x + y * y + z * z + w * w)
    if n == 0:
        return [1.0, 0.0, 0.0, 0.0]
    return [w / n, -x / n, -y / n, -z / n]


def blender_to_quat(bq):
    """Unit Blender quaternion (w, x, y, z) -> game Q12 (x, y, z, w), w >= 0 and
    scaled so w fits the signed 13-bit field (the game normalises anyway)."""
    w, x, y, z = bq
    if w < 0:
        w, x, y, z = -w, -x, -y, -z
    n = math.sqrt(w * w + x * x + y * y + z * z) or 1.0
    scale = 4096.0 / n
    if w * scale > 4095:
        scale = 4095.0 / w
    return [int(round(-x * scale)), int(round(-y * scale)), int(round(-z * scale)), int(round(w * scale))]


def same_rotation(qa, qb, tolerance=1.5 / 4096):
    return np.abs(quat_matrix(qa) - quat_matrix(qb)).max() <= tolerance


# -------------------------------------------------------------------- CLI
def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    s = sub.add_parser('info', help='hierarchy, track and clip summary')
    s.add_argument('--group', type=int, default=0)
    s = sub.add_parser('dump-clip', help='write a clip as ttk-anim-clip JSON')
    s.add_argument('--group', type=int, default=0)
    s.add_argument('clip', type=int)
    s.add_argument('output', type=Path)
    s = sub.add_parser('render', help='render an animation keyframe (or a clip JSON) to PNG')
    s.add_argument('--group', type=int, default=0)
    s.add_argument('--hierarchy', type=int, default=DUKE_HIERARCHY)
    s.add_argument('--animation', type=int)
    s.add_argument('--frame', type=int, default=0)
    s.add_argument('--clip-json', type=Path, action='append', default=[])
    s.add_argument('--yaw', type=float, default=0.0)
    s.add_argument('--pitch', type=float, default=0.25)
    s.add_argument('output', type=Path)
    s = sub.add_parser('roundtrip', help='decode and re-encode every clip of every group; report mismatches')
    s.add_argument('--groups', default='0-31')
    s = sub.add_parser('blender-export', help='write a .blend with the rigged model and an animation as actions')
    s.add_argument('--group', type=int, default=0)
    s.add_argument('--hierarchy', type=int, default=DUKE_HIERARCHY)
    s.add_argument('--animation', type=int, action='append', default=[])
    s.add_argument('--clip-json', type=Path, action='append', default=[])
    s.add_argument('output', type=Path)
    s = sub.add_parser('blender-import', help='read an action from a .blend back into a ttk-anim-clip JSON')
    s.add_argument('blend', type=Path)
    s.add_argument('action')
    s.add_argument('template', type=Path, help='clip JSON giving group, clip id, frame timing, events and joint set')
    s.add_argument('output', type=Path)
    s.add_argument('--keep-original', action='store_true',
                   help='re-use the template quaternion wherever the imported rotation is the same within 1.5/4096')
    s = sub.add_parser('pack', help='build a runtime override pack from clip JSON files')
    s.add_argument('output', type=Path)
    s.add_argument('clips', type=Path, nargs='+')
    a = p.parse_args()

    if a.cmd == 'info':
        g = Group(a.group)
        for i, h in g.hierarchies.items():
            print(f'hierarchy {i}: {h["count"]} joints; map {h["map"]}')
        print('FRM frames', len(g.frm) // 8, 'referenced', frame_count(g.anm), 'ROT commands', len(g.rot) // 8)
        return
    if a.cmd == 'dump-clip':
        g = Group(a.group)
        a.output.write_text(json.dumps(clip_to_json(g, a.clip), indent=1))
        print(a.output)
        return
    if a.cmd == 'render':
        g = Group(a.group)
        h = g.hierarchies[a.hierarchy]
        quats, roots = {}, {}
        if a.animation is not None:
            quats, roots = g.animation_quats(a.animation, a.frame)
        for path in a.clip_json:
            c = json.loads(path.read_text())
            k = min(a.frame, len(c['frames']) - 1)
            quats.update(joint_quats_for_frame(c['frames'][k]))
            roots[0] = cumulative_root(c['frames'][:k + 1])
        render(g, h, pose(h, quats, root_position=root_position(h, roots)), a.output, yaw=a.yaw, pitch=a.pitch,
               center=np.array([0.0, -480.0, 0.0]), distance=2400.0)
        print(a.output)
        return
    if a.cmd == 'roundtrip':
        lo, hi = (int(x) for x in a.groups.split('-'))
        bad = total = 0
        for gi in range(lo, hi + 1):
            try:
                files = read_group(gi)
            except Exception:
                continue
            if 'ANM' not in files:
                continue
            anm, frm, rot = files['ANM'], files['FRM'], files['ROT']
            clips = 0
            for ci in range(2048):
                clip = decode_clip(anm, frm, rot, ci)
                if not clip:
                    continue
                clips += 1
                first = struct.unpack_from('<H', anm, 4 * ci)[0]
                begin = struct.unpack_from('<H', frm, 8 * first + 4)[0]
                f2, r2 = encode_clip_frames(clip, begin)
                n = sum(len(f['commands']) for f in clip['frames'])
                if f2 != frm[8 * first:8 * (first + len(clip['frames']))] or r2 != rot[8 * begin:8 * (begin + n)]:
                    bad += 1
            total += clips
            print(f'DB{gi:02d}: {clips} clips, FRM {len(frm) // 8} (referenced {frame_count(anm)})')
        print(f'{total} clips, {bad} mismatches')
        raise SystemExit(1 if bad else 0)
    if a.cmd == 'blender-export':
        g = Group(a.group)
        model, pages = blender_model_json(g, a.hierarchy)
        out = a.output.resolve()
        work = out.with_suffix('.d')
        work.mkdir(parents=True, exist_ok=True)
        from PIL import Image
        for key, (tpage, clut) in pages.items():
            Image.fromarray(g.page(tpage, clut)).save(work / f'page_{key}.png')
        actions = []
        for anim in a.animation:
            for channel, clip_id in enumerate(g.tracks[anim]):
                if clip_id < 0 or not g.clip(clip_id):
                    continue
                actions.append(dict(name=f'a{anim:03d}_{CHANNELS[channel]}_c{clip_id:04d}', clip=clip_to_json(g, clip_id)))
        for path in a.clip_json:
            c = json.loads(path.read_text())
            actions.append(dict(name=path.stem, clip=c))
        for act in actions:
            for f in act['clip']['frames']:
                for c in f['commands']:
                    if 'q' in c:
                        c['bq'] = quat_to_blender(c['q'])
        model['actions'] = actions
        model['pages'] = {k: str(work / f'page_{k}.png') for k in pages}
        spec = work / 'model.json'
        spec.write_text(json.dumps(model))
        run_blender(['export', spec, out])
        print(out)
        return
    if a.cmd == 'blender-import':
        template = json.loads(a.template.read_text())
        dump = a.output.with_suffix('.blender.json')
        run_blender(['import', a.blend.resolve(), a.action, a.template.resolve(), dump.resolve()])
        data = json.loads(dump.read_text())
        frames = []
        changed = 0
        for k, f in enumerate(template['frames']):
            cmds = []
            for c in f['commands']:
                c = dict(c)
                if 'q' in c:
                    bq = data['frames'][k]['bones'].get(str(c['joint']))
                    if bq is not None:
                        q = blender_to_quat(bq)
                        if a.keep_original and same_rotation(q, c['q']):
                            q = c['q']
                        if q != c['q']:
                            changed += 1
                        c['q'] = q
                elif 'root' in c and data['frames'][k].get('root') is not None:
                    c['root'] = [int(round(v)) for v in data['frames'][k]['root']]
                cmds.append(c)
            frames.append(dict(f, commands=cmds))
        out = dict(template, frames=frames, imported_from=str(a.blend), action=a.action)
        a.output.write_text(json.dumps(out, indent=1))
        print(a.output, f'{changed} quaternions differ from the template')
        return
    if a.cmd == 'pack':
        entries = [json.loads(p.read_text()) for p in a.clips]
        digest = build_pack(entries, a.output)
        print(a.output, digest)
        return


if __name__ == '__main__':
    main()
