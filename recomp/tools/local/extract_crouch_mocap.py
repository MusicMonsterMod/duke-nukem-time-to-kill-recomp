#!/usr/bin/env python3
"""D08K4: extract the crouch-walk motion from a Mixamo-rigged glTF (Blender).

Source used (CC-BY 4.0): "Modern Army Soldier with +24 pro Animations" by
abdlilah ben, https://sketchfab.com/3d-models/modern-army-soldier-with-24-pro-animations-37566d930fa7404f87847b7fca8dca97
(the user's download; kept locally under research/, never committed).

Writes a small, skeleton-independent motion description (tracked asset,
attribution inside) that ttk_crouch_walk.py retargets onto Duke with inverse
kinematics: per frame of the "walk crouch" cycle the hips height, the torso
direction (hips -> upper spine), the hips yaw, each ankle's position relative
to the hips and each foot's rotation relative to the standing pose
("idle simple" frame 0). Axes are mapped to the game's (x right of the body,
y down, z forward; Blender: forward -Y, up +Z), lengths are in leg lengths
(hip-to-ankle of the source rig), so no source mesh or bone data is kept.

  python3 recomp/tools/local/extract_crouch_mocap.py research/crouchwalk-anim/<file>.glb recomp/assets/anim/crouch-walk-mocap.json
"""
import json
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

BLENDER_SCRIPT = r'''
import bpy, json, sys
args = sys.argv[sys.argv.index('--') + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=args[0])
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
names = ['Hips', 'Spine2', 'LeftUpLeg', 'LeftLeg', 'LeftFoot', 'RightUpLeg', 'RightLeg', 'RightFoot']
def bone(n): return next(b for b in arm.pose.bones if b.name.split(':')[-1].rsplit('_', 1)[0] == n)
out = {}
for act in bpy.data.actions:
    key = act.name.split('_GLTF')[0]
    if key not in ('walk crouch', 'idle simple'): continue
    arm.animation_data.action = act
    f0, f1 = act.frame_range
    frames = []
    f = f0
    while f <= f1 + 1e-6:
        bpy.context.scene.frame_set(int(f), subframe=f - int(f))
        rec = {}
        for n in names:
            b = bone(n); m = arm.matrix_world @ b.matrix
            rec[n] = {'head': list(arm.matrix_world @ b.head), 'm': [list(r) for r in m.to_3x3()]}
        frames.append(rec); f += 1
    out[key] = {'fps': bpy.context.scene.render.fps, 'frames': frames}
json.dump(out, open(args[1], 'w'))
print('TTK_BLENDER_OK')
'''

# Blender world (forward -Y, up +Z) -> game (x, y down, z forward); a proper
# rotation (det +1): x = -X, y = -Z, z = -Y.
MAP = np.array([[-1.0, 0, 0], [0, 0, -1.0], [0, -1.0, 0]])


def main():
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    with tempfile.TemporaryDirectory() as tmp:
        script = Path(tmp) / 's.py'
        script.write_text(BLENDER_SCRIPT)
        raw = Path(tmp) / 'raw.json'
        r = subprocess.run(['blender', '-b', '--factory-startup', '--python', str(script), '--', str(src), str(raw)],
                           capture_output=True, text=True)
        if 'TTK_BLENDER_OK' not in r.stdout:
            sys.stderr.write(r.stdout[-3000:] + r.stderr[-3000:])
            raise SystemExit('blender sampling failed')
        data = json.loads(raw.read_text())
    walk = data['walk crouch']['frames'][:-1]           # last sample repeats the first
    stand = data['idle simple']['frames'][0]
    v = lambda f, n: np.array(f[n]['head'])

    def m(f, n):
        # The imported armature carries a scale (0.01): keep the rotation only.
        u, _, vt = np.linalg.svd(np.array(f[n]['m']))
        r = u @ vt
        return r if np.linalg.det(r) > 0 else -r
    leg = np.linalg.norm(v(stand, 'LeftUpLeg') - v(stand, 'LeftLeg')) + np.linalg.norm(v(stand, 'LeftLeg') - v(stand, 'LeftFoot'))
    ground = min(min(v(f, 'LeftFoot')[2], v(f, 'RightFoot')[2]) for f in walk)
    frames = []
    for f in walk:
        hips = v(f, 'Hips')
        rec = dict(hips_height=float((hips[2] - ground) / leg),
                   torso=list(MAP @ ((v(f, 'Spine2') - hips) / np.linalg.norm(v(f, 'Spine2') - hips))),
                   hips_rotation=[list(r) for r in MAP @ (m(f, 'Hips') @ m(stand, 'Hips').T) @ MAP.T])
        for side, foot in (('left', 'LeftFoot'), ('right', 'RightFoot')):
            rel = v(f, foot) - hips
            rec[side + '_ankle'] = list(MAP @ rel / leg)
            rec[side + '_ankle_height'] = float((v(f, foot)[2] - ground) / leg)
            rec[side + '_foot_rotation'] = [list(r) for r in MAP @ (m(f, foot) @ m(stand, foot).T) @ MAP.T]
        frames.append(rec)
    doc = dict(format='ttk-crouch-mocap', version=1,
               source='Modern Army Soldier with +24 pro Animations, animation "walk crouch"',
               author='abdlilah ben (https://sketchfab.com/m152027350)',
               url='https://sketchfab.com/3d-models/modern-army-soldier-with-24-pro-animations-37566d930fa7404f87847b7fca8dca97',
               license='CC-BY-4.0 (http://creativecommons.org/licenses/by/4.0/)',
               changes='sampled per frame, reduced to hips height, torso direction, ankle paths and foot rotations, '
                       'axes mapped to Time to Kill, retargeted onto Duke with inverse kinematics',
               fps=data['walk crouch']['fps'], left_is_game_negative_x=bool(np.mean([r['left_ankle'][0] for r in frames]) < 0),
               frames=frames)
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(json.dumps(doc, indent=1))
    print(dst, len(frames), 'frames')


if __name__ == '__main__':
    main()
