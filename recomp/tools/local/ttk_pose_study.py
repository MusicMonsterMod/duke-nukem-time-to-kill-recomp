#!/usr/bin/env python3
"""D08K6: study a source animation against Duke (documentation note 188).

Read-only on the disc and the source file; outputs go where you say (keep them
under ignored recomp/analysis/). Three commands:

  sample SRC.glb OUT.json
      Blender: every action of a Mixamo-style glTF rig, every frame, the world
      head/tail/rotation of the body bones (no mesh data). Input to `metrics`.
  metrics OUT.json [--action "walk crouch"] [--set hip_ratio=0.62 ...]
      Posture numbers in Duke-comparable units (leg lengths, degrees) for the
      source action next to Duke's crouch walk (ttk_crouch_walk.py with the
      given TACTICAL overrides): pelvis and hip-joint height over the ankles,
      knee flexion, thigh pitch, knee and ankle spread, torso and head pitch,
      hip yaw/roll, shoulder yaw. The numbers that found D08K4's "sitting":
      thighs level, feet four times the hips' spread, upright chest.
  flipbook OUT_PREFIX [--direction forward] [--frames 12] [--set k=v ...]
      Textured renders (ttk_anim.render) of Duke's crouch-walk frames with the
      designed upper body, side and rear-3/4 views, one sprite sheet per view
      (OUT_PREFIX-side.jpg, OUT_PREFIX-rear.jpg).
  source-flipbook SRC.glb OUT_PREFIX [--action "walk crouch"] [--frames 12]
      Blender workbench renders of the source rig's action from the same views.

Axes: Blender world (forward -Y, up +Z) maps to the game's (x right, y down,
z forward) as x = -X, y = -Z, z = -Y (a proper rotation), as in
extract_crouch_mocap.py.
"""
import argparse
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ttk_anim as t            # noqa: E402
import ttk_crouch_walk as cw    # noqa: E402

MAP = np.array([[-1.0, 0, 0], [0, 0, -1.0], [0, -1.0, 0]])
BONES = ['Hips', 'Spine', 'Spine1', 'Spine2', 'Neck', 'Head', 'HeadTop_End', 'LeftShoulder', 'LeftArm', 'LeftForeArm',
         'LeftHand', 'RightShoulder', 'RightArm', 'RightForeArm', 'RightHand', 'LeftUpLeg', 'LeftLeg', 'LeftFoot',
         'LeftToeBase', 'RightUpLeg', 'RightLeg', 'RightFoot', 'RightToeBase']

SAMPLE = r'''
import bpy, json, sys
args = sys.argv[sys.argv.index('--') + 1:]
keep = json.loads(args[2])
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=args[0])
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
bones = {b.name.split(':')[-1].rsplit('_', 1)[0]: b for b in arm.pose.bones if ':' in b.name}
out = {}
for act in bpy.data.actions:
    arm.animation_data.action = act
    f0, f1 = act.frame_range
    frames, f = [], f0
    while f <= f1 + 1e-6:
        bpy.context.scene.frame_set(int(f), subframe=f - int(f))
        rec = {}
        for n in keep:
            b = bones[n]; m = arm.matrix_world @ b.matrix
            rec[n] = {'head': list(arm.matrix_world @ b.head), 'm': [list(r) for r in m.to_3x3()]}
        frames.append(rec); f += 1
    out[act.name.split('_GLTF')[0]] = frames
json.dump(out, open(args[1], 'w'))
print('TTK_BLENDER_OK')
'''

RENDER = r'''
import bpy, sys, math, json
args = sys.argv[sys.argv.index('--') + 1:]
src, out, action, frames = args[0], args[1], args[2], json.loads(args[3])
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
for o in bpy.data.objects:
    if o.type == 'MESH' and o.name.startswith('Icosphere'): o.hide_render = True
arm.animation_data.action = next(a for a in bpy.data.actions if a.name.split('_GLTF')[0] == action)
sc = bpy.context.scene
sc.render.engine = 'BLENDER_WORKBENCH'
sc.display.shading.light = 'STUDIO'; sc.display.shading.color_type = 'TEXTURE'
sc.render.resolution_x, sc.render.resolution_y = 260, 340
sc.world = bpy.data.worlds.new('w'); sc.world.color = (0.0094, 0.0103, 0.0137)
cam = bpy.data.objects.new('cam', bpy.data.cameras.new('cam')); sc.collection.objects.link(cam); sc.camera = cam
cam.data.type = 'ORTHO'; cam.data.ortho_scale = 4.0
tgt = bpy.data.objects.new('tgt', None); sc.collection.objects.link(tgt); tgt.location = (0.0, -0.6, 1.35)
con = cam.constraints.new('TRACK_TO'); con.target = tgt; con.track_axis = 'TRACK_NEGATIVE_Z'; con.up_axis = 'UP_Y'
for view, loc in (('side', (8.0, -0.6, 1.6)), ('rear', (4.4, 4.0, 3.3))):
    cam.location = loc
    for i, f in enumerate(frames):
        sc.frame_set(int(f), subframe=f - int(f))
        sc.render.filepath = f'{out}-{view}-{i:02d}.png'
        bpy.ops.render.render(write_still=True)
print('TTK_BLENDER_OK')
'''


def blender(script, *args):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / 's.py'
        path.write_text(script)
        r = subprocess.run(['blender', '-b', '--factory-startup', '--python', str(path), '--', *map(str, args)],
                           capture_output=True, text=True)
    if 'TTK_BLENDER_OK' not in r.stdout:
        sys.stderr.write(r.stdout[-3000:] + r.stderr[-3000:])
        raise SystemExit('blender failed')


def sheet(paths, out, size=(260, 340)):
    from PIL import Image
    ims = [Image.open(p).convert('RGB').resize(size) for p in paths]
    s = Image.new('RGB', (size[0] * len(ims), size[1]))
    for i, im in enumerate(ims):
        s.paste(im, (size[0] * i, 0))
    s.save(out, quality=82)
    print(out)


def _flex(a, b, c):
    u, w = a - b, c - b
    return 180 - math.degrees(math.acos(np.clip(u @ w / np.linalg.norm(u) / np.linalg.norm(w), -1, 1)))


def _pitch(v):
    """Forward lean of a segment from vertical (game axes: up -y, forward +z)."""
    return math.degrees(math.atan2(v[2], -v[1]))


def posture(j, leg, ankle_y):
    """j: name -> game position; ankle_y: the ankles' y when planted."""
    r = {'pelvis_h': (ankle_y - j['pelvis'][1]) / leg,
         'hip_joint_h': (ankle_y - (j['lhip'][1] + j['rhip'][1]) / 2) / leg}
    for s in 'lr':
        thigh = j[s + 'knee'] - j[s + 'hip']
        r[s + '_knee_flex'] = _flex(j[s + 'hip'], j[s + 'knee'], j[s + 'ankle'])
        r[s + '_thigh_pitch'] = math.degrees(math.atan2(thigh[2], thigh[1]))
        r[s + '_knee_out'] = abs(j[s + 'knee'][0] - j['pelvis'][0]) / leg
        r[s + '_ankle_out'] = abs(j[s + 'ankle'][0] - j['pelvis'][0]) / leg
    r['torso_pitch'] = _pitch(j['neck'] - j['pelvis'])
    r['head_pitch'] = _pitch(j['headtop'] - j['neck'])
    hl, sl = j['lhip'] - j['rhip'], j['lsh'] - j['rsh']
    r['hip_yaw'] = math.degrees(math.atan2(hl[2], -hl[0]))
    r['hip_roll'] = math.degrees(math.atan2(-hl[1], -hl[0]))
    r['shoulder_yaw'] = math.degrees(math.atan2(sl[2], -sl[0]))
    r['hip_spread'] = abs(hl[0]) / leg
    return r


def source_posture(data, action):
    def joints(f):
        g = lambda n: MAP @ np.array(f[n]['head'])
        return dict(pelvis=g('Hips'), lhip=g('LeftUpLeg'), rhip=g('RightUpLeg'), lknee=g('LeftLeg'), rknee=g('RightLeg'),
                    lankle=g('LeftFoot'), rankle=g('RightFoot'), neck=g('Neck'), headtop=g('HeadTop_End'),
                    lsh=g('LeftArm'), rsh=g('RightArm'))
    st = joints(data['idle simple'][0]) if 'idle simple' in data else joints(data[action][0])
    leg = np.linalg.norm(st['lhip'] - st['lknee']) + np.linalg.norm(st['lknee'] - st['lankle'])
    frames = [joints(f) for f in data[action]]
    ankle_y = max(max(j['lankle'][1], j['rankle'][1]) for j in frames)
    return [posture(j, leg, ankle_y) for j in frames]


def duke_posture(group, frames):
    h = group.hierarchies[t.DUKE_HIERARCHY]
    up, _ = group.animation_quats(178, 0)
    leg = float(np.linalg.norm(h['joints'][11]['offset']) + np.linalg.norm(h['joints'][12]['offset']))
    out = []
    for f in frames:
        q = dict(up)
        q.update(t.joint_quats_for_frame(f))
        q.update(f.get('upper', {}))
        mats = t.pose(h, q, root_position=(0.0, -f['commands'][0]['root'][1], 0.0))
        p = lambda i: mats[i][1]
        head = np.array(group.model(h['joints'][9]['model'])['vertices'], float) @ mats[9][0].T + mats[9][1]
        j = dict(pelvis=p(0), lhip=p(10), rhip=p(14), lknee=p(11), rknee=p(15), lankle=p(12), rankle=p(16),
                 neck=p(9), headtop=head[int(np.argmin(head[:, 1]))], lsh=p(2), rsh=p(5))
        out.append(posture(j, leg, -cw.ANKLE_HEIGHT))
    return out


def summary(rows):
    return {k: (min(r[k] for r in rows), float(np.mean([r[k] for r in rows])), max(r[k] for r in rows)) for k in rows[0]}


def apply_sets(sets):
    for kv in sets:
        key, value = kv.split('=', 1)
        if key not in cw.TACTICAL:
            raise SystemExit(f'unknown parameter {key}; one of {", ".join(cw.TACTICAL)}')
        cw.TACTICAL[key] = float(value)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    s = sub.add_parser('sample'); s.add_argument('source', type=Path); s.add_argument('out', type=Path)
    m = sub.add_parser('metrics'); m.add_argument('samples', type=Path); m.add_argument('--action', default='walk crouch')
    m.add_argument('--direction', default='forward'); m.add_argument('--set', action='append', default=[])
    f = sub.add_parser('flipbook'); f.add_argument('prefix'); f.add_argument('--direction', default='forward')
    f.add_argument('--frames', type=int, default=12); f.add_argument('--set', action='append', default=[])
    sf = sub.add_parser('source-flipbook'); sf.add_argument('source', type=Path); sf.add_argument('prefix')
    sf.add_argument('--action', default='walk crouch'); sf.add_argument('--frames', type=int, default=12)
    a = p.parse_args()
    if a.cmd == 'sample':
        blender(SAMPLE, a.source.resolve(), a.out.resolve(), json.dumps(BONES))
        print(a.out)
    elif a.cmd == 'metrics':
        apply_sets(a.set)
        data = json.loads(a.samples.read_text())
        rows = data[a.action][:-1] if 'walk' in a.action else data[a.action]   # a looping action repeats its first frame
        src = summary(source_posture(dict(data, **{a.action: rows}), a.action))
        group = t.Group(0)
        duke = summary(duke_posture(group, cw.mocap_frames(group, a.direction)))
        fmt = lambda v: f'{v[0]:7.2f}/{v[1]:7.2f}/{v[2]:7.2f}'
        print(f"{'metric':15} {'source min/mean/max':>24} {'Duke min/mean/max':>24}")
        for k in src:
            print(f'{k:15} {fmt(src[k]):>24} {fmt(duke[k]):>24}')
    elif a.cmd == 'flipbook':
        apply_sets(a.set)
        group = t.Group(0)
        h = group.hierarchies[t.DUKE_HIERARCHY]
        up, _ = group.animation_quats(178, 0)
        frames = cw.mocap_frames(group, a.direction)
        picks = [round(k * len(frames) / a.frames) % len(frames) for k in range(a.frames)]
        with tempfile.TemporaryDirectory() as tmp:
            for view, (yaw, pitch) in (('side', (-math.pi / 2, 0.05)), ('rear', (-math.pi * 0.22, 0.3))):
                paths = []
                for i, k in enumerate(picks):
                    q = dict(up)
                    q.update(t.joint_quats_for_frame(frames[k]))
                    q.update(frames[k].get('upper', {}))
                    mats = t.pose(h, q, root_position=(0.0, -frames[k]['commands'][0]['root'][1], 0.0))
                    path = Path(tmp) / f'{view}-{i:02d}.png'
                    t.render(group, h, mats, path, size=(340, 260), yaw=yaw, pitch=pitch, distance=1500.0,
                             center=np.array([0.0, -300.0, 0.0]), background=(24, 26, 31))
                    paths.append(path)
                sheet(paths, f'{a.prefix}-{view}.jpg')
    else:
        with tempfile.TemporaryDirectory() as tmp:
            # Frame range of the action: sample once to read it.
            raw = Path(tmp) / 'raw.json'
            blender(SAMPLE, a.source.resolve(), raw, json.dumps(['Hips']))
            n = len(json.loads(raw.read_text())[a.action]) - 1
            frames = [0.8 + n * k / a.frames for k in range(a.frames)]   # glTF actions here start at 0.8
            blender(RENDER, a.source.resolve(), Path(tmp) / 'f', a.action, json.dumps(frames))
            for view in ('side', 'rear'):
                sheet(sorted(Path(tmp).glob(f'f-{view}-*.png')), f'{a.prefix}-{view}.jpg')


if __name__ == '__main__':
    main()
