#!/usr/bin/env python3
"""D12E1: Duke's third-person quick kick, legs and pelvis (documentation note 382).

The original third-person kicks (112-115) are full-body clips of about one
second. The quick kick keeps the arms, chest, head and weapon on the original
aim and plays a fast MMA-style roundhouse on the pelvis and legs only, always
the left leg (user: "LEFT LEG!"; measured on the reference's right, then mirrored), at the first-person quick kick's cadence (kick.inc,
450 ms).

Revision 2 (user, 2026-11-04: "always the same leg ... more of the MMA kick ...
like a faster version of this"): the key table below was measured from the
user's reference motion (a mixed-martial-arts roundhouse, 30 fps, 65 frames,
kept locally under research/, never in the repository) with --measure, about
3.5x faster from the first hip move to the strike: the hips turn over (side-on
and tipped) while the knee chambers across the body, the shin snaps out to
about waist height, then the knee re-chambers and the foot comes back down.
Per key it gives, in Duke's level axes (x right, y down, z ahead) and scale:

- the pelvis rotation from a square stance (an axis-angle vector, degrees),
  applied at HIP_TURN of its angle (Duke's torso is a single joint, so a full
  side-on turn would twist it apart; the host keeps the chest near the aim);
- the kicking ankle and knee from the right hip joint (world vectors): the
  ankle is solved with two-bone IK and the knee hinge turned toward the knee;
- the foot's rotation from its start (axis-angle);
- the support foot's pivot (degrees of yaw), applied at SUPPORT_TURN; the
  support ankle stays planted where the standing idle 63 puts it.

The first and last keys ease from and back to Duke's own standing pose
(NEUTRAL_W), since the reference starts and ends in a bladed fighting stance.
The pelvis height is never moved, so the clip carries no root command; toes
keep the live pose. The host (tp_kick.inc) keeps the same key times
(tp_kick_key_ms) and checks the frame count.

  python3 recomp/tools/local/ttk_quick_kick.py --render OUT_DIR       # side/front/quarter/behind sheets
  python3 recomp/tools/local/ttk_quick_kick.py --measure REF.fbx      # print a KEYS table (needs Blender)
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
import ttk_anim as t
import ttk_crouch_walk as cw

IDLE = 63
KICK_CLIP = 51
KICK_LEG = 'left'
WRITTEN = (0, 1, 2, 3, 5, 6, 7)      # pelvis, left hip/knee/ankle, right hip/knee/ankle (toes keep the live pose)
RIGHT, LEFT = (14, 15, 16, 17), (10, 11, 12, 13)   # hip, knee, ankle, toe records
DUKE_LEG = 485.0                     # thigh + shin, for the reference scale
PARAMS = dict(hip_turn=0.7, support_turn=0.45)
# ms, pelvis axis-angle (deg), ankle from the right hip joint, knee from it,
# foot axis-angle (deg), support foot yaw (deg); reference frame in the comment.
KEYS = (
    (0, (2, 18, -1), (-6, 409, -211), (39, 250, -58), (-7, 3, -4), -41),        # 11: weight shifts forward
    (40, (10, 14, -10), (37, 363, -172), (118, 231, -8), (-41, 16, 48), -41),    # 15: rear foot leaves the floor
    (67, (15, -2, -15), (42, 320, -109), (151, 205, 52), (-34, 0, 74), -44),     # 17
    (92, (14, -27, -22), (45, 275, -38), (126, 174, 146), (-32, -29, 73), -56),  # 19: knee rising, hips turning
    (110, (9, -52, -32), (63, 203, 50), (-9, 102, 239), (-60, -49, 40), -80),    # 21: chamber
    (125, (4, -66, -43), (66, 116, 180), (-137, 25, 219), (-70, -58, -23), -100),  # 23: knee across, hips over
    (138, (3, -68, -52), (16, 25, 363), (-150, 6, 212), (-50, -79, -74), -107),  # 25: the shin whips out
    (150, (4, -67, -57), (-53, -37, 478), (-47, 0, 255), (-37, -100, -105), -110),  # 27: strike
    (200, (5, -67, -58), (-58, -39, 478), (-40, -1, 257), (-30, -100, -115), -111),  # 28
    (283, (6, -68, -59), (-44, -20, 476), (-56, 9, 253), (-28, -90, -118), -111),  # 29: end of the hold
    (317, (7, -72, -60), (12, 53, 383), (-139, 36, 216), (-38, -63, -105), -111),  # 31: re-chamber
    (350, (10, -74, -56), (38, 183, 191), (-137, 43, 216), (-57, -57, -42), -111),  # 34
    (375, (13, -72, -47), (22, 301, 47), (-38, 153, 206), (-62, -50, 8), -111),  # 37
    (400, (16, -61, -31), (35, 350, -43), (-4, 220, 137), (-59, -50, 20), -112),  # 40: foot coming down
    (425, (15, -34, -12), (62, 361, -128), (40, 248, 66), (-60, -45, 2), -93),   # 44
    (450, (8, 4, -1), (21, 398, -219), (58, 247, -56), (-10, 1, 0), -21),        # 51: down
)
KEY_MS = tuple(k[0] for k in KEYS)   # tp_kick_key_ms in tp_kick.inc
# Weight of Duke's own standing pose at each key (1 = neutral, 0 = the reference).
NEUTRAL_W = (0.85, 0.45, 0.15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.1, 0.3, 0.6, 1.0)


def axis_angle(v):
    v = np.asarray(v, float)
    a = float(np.linalg.norm(v))
    return np.eye(3) if a < 1e-9 else cw.axis_angle(v / a, math.radians(a))


def fit13(q):
    q = list(q)
    if q[3] < 0:
        q = [-v for v in q]
    if q[3] > 4095:
        s = 4095.0 / q[3]
        q = [int(round(v * s)) for v in q]
    return q


def key_pose(group, k, idle_mats, params=None):
    p = {**PARAMS, **(params or {})}
    _, pelvis_v, foot_v, knee_v, foot_r, support_yaw = KEYS[k]
    n = NEUTRAL_W[k]
    pelvis_m = axis_angle(np.asarray(pelvis_v, float) * (1 - n) * p['hip_turn']) @ idle_mats[0][0]
    pelvis_t = idle_mats[0][1]
    right, left = cw.Leg(group, RIGHT), cw.Leg(group, LEFT)
    hip = pelvis_m @ right.hip_offset + pelvis_t
    target = (1 - n) * (hip + np.asarray(foot_v, float)) + n * idle_mats[RIGHT[2]][1]
    pole = np.asarray(knee_v, float) - 0.5 * np.asarray(foot_v, float)
    pole = (1 - n) * pole / max(1e-6, float(np.linalg.norm(pole))) + n * np.array([0.0, 0.0, 1.0])
    ankle_world = axis_angle(np.asarray(foot_r, float) * (1 - n)) @ idle_mats[RIGHT[2]][0]
    hr, kr, ar, _ = right.solve(pelvis_m, pelvis_t, target, ankle_world, pole)
    turn = cw.rot_y(support_yaw * p['support_turn'] * (1 - n))
    hl, kl, al, _ = left.solve(pelvis_m, pelvis_t, idle_mats[LEFT[2]][1], turn @ idle_mats[LEFT[2]][0],
                               turn @ np.array([0.0, 0.0, 1.0]))
    # The pelvis is the root record: its local rotation is its world rotation.
    q = {0: cw.game_q(pelvis_m), 5: cw.game_q(hr), 6: cw.game_q(kr), 7: cw.game_q(ar),
         1: cw.game_q(hl), 2: cw.game_q(kl), 3: cw.game_q(al)}
    return {j: fit13(q[j]) for j in WRITTEN}


def mirrored(q):
    """The pose reflected through Duke's x = 0 plane: quaternion (x, -y, -z, w),
    left and right legs swapped (his leg rig is symmetric within a few units)."""
    swap = {0: 0, 1: 5, 2: 6, 3: 7, 5: 1, 6: 2, 7: 3}
    return {swap[j]: [v[0], -v[1], -v[2], v[3]] for j, v in q.items()}


def key_poses(group, params=None):
    h = group.hierarchies[t.DUKE_HIERARCHY]
    idle_q, _ = group.animation_quats(IDLE, 0)
    idle_mats = t.pose(h, idle_q)
    # Solved for the right leg as measured, then mirrored onto the left (KICK_LEG).
    poses = [key_pose(group, k, idle_mats, params) for k in range(len(KEYS))]
    return [mirrored(q) for q in poses] if KICK_LEG == 'left' else poses


def frames(group, params=None):
    """Host extra-clip frames (ttk_crouch_walk.host_frames format, leading copy)."""
    fr = [dict(events=0, ticks=cw.HOST_TICKS, commands=[dict(joint=j, q=q[j], flags=0) for j in sorted(q)])
          for q in key_poses(group, params)]
    return cw.host_frames(fr)


def pack_entries(group):
    return [dict(group=0xffff, clip=KICK_CLIP, source_sha256='00' * 32, frames=frames(group))]


def render_sheets(group, out, params=None):
    from PIL import Image, ImageDraw
    out.mkdir(parents=True, exist_ok=True)
    h = group.hierarchies[t.DUKE_HIERARCHY]
    idle_q, _ = group.animation_quats(IDLE, 0)
    views = dict(side=-math.pi / 2, front=math.pi, quarter=-2.4, behind=0.0)
    poses = key_poses(group, params)
    for name, yaw in views.items():
        ims = []
        for k, q in enumerate(poses):
            mats = t.pose(h, {**idle_q, **q})
            path = out / f'{name}-{k:02d}.png'
            t.render(group, h, mats, path, size=(400, 300), yaw=yaw, pitch=0.12, distance=1800.0,
                     center=mats[0][1] + np.array([0.0, 150.0, 0.0]))
            ims.append(Image.open(path))
        w, hh = ims[0].size
        cols = 8
        sheet = Image.new('RGB', (w * cols, hh * ((len(ims) + cols - 1) // cols)))
        d = ImageDraw.Draw(sheet)
        for i, im in enumerate(ims):
            sheet.paste(im, ((i % cols) * w, (i // cols) * hh))
            d.text(((i % cols) * w + 4, (i // cols) * hh + 4), f'{KEY_MS[i]} ms', fill=(255, 255, 0))
        sheet.save(out / f'sheet-{name}.png')
        print(out / f'sheet-{name}.png')


# --measure: Blender reads the reference FBX (a humanoid rig with Hips,
# RightUpLeg/RightLeg/RightFoot and the left leg) and dumps its world joints.
BLENDER_DUMP = r"""
import bpy,sys,json
src,out=sys.argv[sys.argv.index('--')+1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=src)
arm=[o for o in bpy.data.objects if o.type=='ARMATURE'][0]
act=arm.animation_data.action
sc=bpy.context.scene;frames=[]
for f in range(int(act.frame_range[0]),int(act.frame_range[1])+1):
    sc.frame_set(f);d={}
    for pb in arm.pose.bones:
        n=pb.name.split(':')[-1];m=arm.matrix_world@pb.matrix
        d[n]=dict(head=list(arm.matrix_world@pb.head),rot=[list(r) for r in m.to_3x3()])
    frames.append(d)
json.dump(dict(fps=sc.render.fps,frames=frames),open(out,'w'))
"""
# The reference's own frame numbers sampled at the KEYS times.
MEASURE_FRAMES = (11, 15, 17, 19, 21, 23, 25, 27, 28, 29, 31, 34, 37, 40, 44, 51)


def measure(fbx):
    with tempfile.TemporaryDirectory() as tmp:
        script, dump = Path(tmp) / 'dump.py', Path(tmp) / 'dump.json'
        script.write_text(BLENDER_DUMP)
        subprocess.run(['blender', '-b', '--python', str(script), '--', str(fbx), str(dump)], check=True,
                       stdout=subprocess.DEVNULL)
        d = json.loads(dump.read_text())['frames']
    M = np.array([[-1, 0, 0], [0, 0, -1], [0, -1, 0]], float)   # reference (x, y back, z up) -> Duke

    def P(f, n):
        return np.array(d[f - 1][n]['head']) * 10000

    def R(f, n):
        u, _, vt = np.linalg.svd(np.array(d[f - 1][n]['rot']))
        return M @ (u @ vt) @ M.T

    def rotvec(m):
        a = math.acos(max(-1.0, min(1.0, (np.trace(m) - 1) / 2)))
        if a < 1e-6:
            return np.zeros(3)
        return np.array([m[2, 1] - m[1, 2], m[0, 2] - m[2, 0], m[1, 0] - m[0, 1]]) / (2 * math.sin(a)) * math.degrees(a)
    s = DUKE_LEG / (np.linalg.norm(P(1, 'RightUpLeg') - P(1, 'RightLeg')) + np.linalg.norm(P(1, 'RightLeg') - P(1, 'RightFoot')))
    line = M @ (P(1, 'LeftUpLeg') - P(1, 'RightUpLeg'))
    square = cw.rot_y(-math.degrees(math.atan2(-line[2], -line[0])))   # undo the start's bladed stance
    fmt = lambda v: '(%s)' % ', '.join('%d' % round(x) for x in v)
    print('KEYS = (')
    for ms, f in zip(KEY_MS, MEASURE_FRAMES):
        D = R(f, 'Hips') @ R(1, 'Hips').T @ square
        foot = s * (M @ (P(f, 'RightFoot') - P(f, 'RightUpLeg')))
        knee = s * (M @ (P(f, 'RightLeg') - P(f, 'RightUpLeg')))
        sv = R(f, 'LeftFoot') @ R(1, 'LeftFoot').T @ np.array([0, 0, 1.0])
        print(f'    ({ms}, {fmt(rotvec(D))}, {fmt(foot)}, {fmt(knee)}, '
              f'{fmt(rotvec(R(f, "RightFoot") @ R(1, "RightFoot").T))}, {round(math.degrees(math.atan2(sv[0], sv[2])))}),  # {f}')
    print(')')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--group', type=int, default=0)
    ap.add_argument('--render', type=Path, help='write key-pose sheets here')
    ap.add_argument('--measure', type=Path, help='print a KEYS table from a reference FBX (Blender)')
    ap.add_argument('--set', action='append', default=[], metavar='KEY=VALUE', help='hip_turn or support_turn')
    a = ap.parse_args()
    if a.measure:
        measure(a.measure)
        return
    params = {}
    for kv in a.set:
        key, value = kv.split('=', 1)
        if key not in PARAMS:
            ap.error(f'unknown parameter {key}; one of {", ".join(PARAMS)}')
        params[key] = float(value)
    group = t.Group(a.group)
    if a.render:
        render_sheets(group, a.render, params)
    else:
        for e in pack_entries(group):
            for f in e['frames']:
                for c in f['commands']:
                    t.encode_command(c)
            print(e['clip'], len(e['frames']), 'frames')


if __name__ == '__main__':
    main()
