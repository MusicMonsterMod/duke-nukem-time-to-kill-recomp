#!/usr/bin/env python3
"""D08J7C: Duke's one-handed armed pull-up (documentation note 378).

The original pull-up 140 is two-handed (clips 245 legs/root, 247 right arm,
248 left arm, 246 chest/head; 10 frames). With the sidearm out the gun hand
cannot hold the lip, so this builds the upper body of a one-handed version
on Duke's own skeleton, frame for frame with 140:

- the chest twists and rolls onto the left arm (left shoulder forward and up)
  while the chest is below the lip, then unwinds as Duke stands;
- the left hand stays on the lip, solved with two-bone IK onto 140's own hand
  position moved part of the way under his centre, keeping 140's hand
  orientation and elbow direction;
- the head keeps 140's world orientation (he keeps looking at the ledge);
- the gun arm holds the pistol muzzle up beside the head (a high ready, clear
  of the lip) and comes down into the original sidearm pose (animation 5) by
  frame armed_pullup_late (6, knee on the lip), where shots open.

Legs and root stay on 140 (the host never writes them), so Duke ends where he
does today. The frames are a host extra clip (ONEHAND_CLIP), written onto the
joint records by climb_fire.inc while the armed pull-up runs.

  python3 recomp/tools/local/ttk_onehand_pullup.py --render OUT_DIR   # side/front/back sheets
"""
import argparse
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ttk_anim as t
import ttk_crouch_walk as cw

PULLUP = 140
PISTOL = 5                    # sidearm idle: the gun arm pose shots resume from
ONEHAND_CLIP = 41             # host extra clip id (group 0xffff)
UPPER_IDS = (9, 16, 10, 11, 12, 13, 14, 15)
LATE = 6                      # armed_pullup_late in climb_fire.inc
# Chest offsets at full weight (degrees, in the chest's own axes).
PARAMS = dict(yaw=22.0, roll=12.0, pitch=6.0, medial=0.35,
              # Gun wrist in chest space at full high ready (x right, y down, z forward).
              gun_x=135.0, gun_y=-235.0, gun_z=25.0, muzzle_fwd=12.0)
# Per-frame weight of the lean (1 while the chest is below the lip).
LEAN_W = (1.0, 1.0, 1.0, 1.0, 1.0, 0.8, 0.55, 0.3, 0.12, 0.0)
# Per-frame weight of the high ready (0 = the sidearm pose), 0 from LATE on.
READY_W = (1.0, 1.0, 1.0, 1.0, 1.0, 0.55, 0.0, 0.0, 0.0, 0.0)

LEFT, RIGHT = (2, 3, 4), (5, 6, 7)    # shoulder, forearm, hand records


def q_local(m):
    return cw.game_q(m)


def slerp_q(a, b, w):
    q = t.slerp(a, b, w)
    n = float(np.linalg.norm(q))
    return [int(round(v * 4096.0 / n)) for v in q] if n > 0 else [0, 0, 0, 4096]


def fit13(q):
    """Keep w >= 0 and inside the signed 13-bit field (the game normalises)."""
    q = list(q)
    if q[3] < 0:
        q = [-v for v in q]
    if q[3] > 4095:
        s = 4095.0 / q[3]
        q = [int(round(v * s)) for v in q]
    return q


def base_quats(group, k):
    """140's pose at frame k. A frame may leave a joint out (frame 9 has no
    pelvis, chest, toe or jetpack command); the channel blend then keeps the
    previous frame's value, so carry earlier frames forward."""
    q = {}
    for i in range(k + 1):
        q.update(group.animation_quats(PULLUP, i)[0])
    return q


def frame_pose(group, k, params=None):
    """Joint id -> Q12 quaternion of the one-handed upper body at 140 frame k."""
    p = {**PARAMS, **(params or {})}
    h = group.hierarchies[t.DUKE_HIERARCHY]
    base = base_quats(group, k)
    mats = t.pose(h, base)
    chest_m, chest_t = mats[1]
    lean = cw.rot_y(-p['yaw']) @ cw.rot_z(p['roll']) @ cw.rot_x(p['pitch'])
    chest_full = chest_m @ lean
    out = {}
    # Chest: lean onto the left arm.
    out[9] = q_local(mats[0][0].T @ chest_full)
    # Head: keep 140's world orientation under the leaned chest.
    out[16] = q_local(chest_full.T @ mats[9][0])
    # Left arm: hand on the lip, part of the way under the body.
    left = cw.Arm(group, LEFT)
    lw, rw = mats[4][1], mats[7][1]
    target = lw + p['medial'] * ((lw + rw) / 2 - lw) * np.array([1.0, 0.0, 1.0])
    elbow_dir = mats[3][1] - mats[2][1]
    sh, fo, ha, _ = left.solve(chest_full, chest_t, target, mats[4][0], elbow_dir)
    out[10], out[11], out[12] = q_local(sh), q_local(fo), q_local(ha)
    # Gun arm: high ready in chest space (pistol up beside the head).
    right = cw.Arm(group, RIGHT)
    wrist = chest_full @ np.array([p['gun_x'], p['gun_y'], p['gun_z']]) + chest_t
    up = chest_full @ np.array([0.0, -1.0, 0.0])
    fwd = chest_full @ np.array([0.0, 0.0, 1.0])
    a = math.radians(p['muzzle_fwd'])
    muzzle = up * math.cos(a) + fwd * math.sin(a)
    top = -(fwd * math.cos(a) - up * math.sin(a))      # the sights face Duke
    # Hand local axes (from the pistol idle pose): muzzle = local -z, top = local +y.
    hand_world = np.column_stack([np.cross(top, -muzzle), top, -muzzle])
    elbow_pref = chest_full @ np.array([0.6, 1.0, -0.2])
    sh, fo, ha, _ = right.solve(chest_full, chest_t, wrist, hand_world, elbow_pref)
    ready = {13: q_local(sh), 14: q_local(fo), 15: q_local(ha)}
    pistol, _ = group.animation_quats(PISTOL, 0)
    # Weights: the lean fades out as Duke stands; the gun arm reaches the
    # sidearm pose by LATE.
    w = LEAN_W[k]
    for j in (9, 16, 10, 11, 12):
        out[j] = fit13(slerp_q(base[j], out[j], w))
    r = READY_W[k]
    for j in (13, 14, 15):
        out[j] = fit13(slerp_q(pistol[j], ready[j], r))
    return out


def frames(group, params=None):
    """Host extra-clip frames (ttk_crouch_walk.host_frames format, leading copy)."""
    n = len(group.clip(group.tracks[PULLUP][0])['frames'])
    fr = []
    for k in range(n):
        q = frame_pose(group, k, params)
        fr.append(dict(events=0, ticks=cw.HOST_TICKS,
                       commands=[dict(joint=j, q=q[j], flags=0) for j in UPPER_IDS]))
    return cw.host_frames(fr)


def pose_mats(group, k, onehand=True, params=None):
    h = group.hierarchies[t.DUKE_HIERARCHY]
    q = base_quats(group, k)
    if onehand:
        q = {**q, **frame_pose(group, k, params)}
    return h, t.pose(h, q)


def render_sheets(group, out, params=None):
    from PIL import Image, ImageDraw
    out.mkdir(parents=True, exist_ok=True)
    views = dict(side=-math.pi / 2, front=math.pi, quarter=-2.4)
    for name, yaw in views.items():
        tiles = []
        for onehand in (False, True):
            for k in range(10):
                h, mats = pose_mats(group, k, onehand, params)
                path = out / f'{name}-{"one" if onehand else "orig"}-{k}.png'
                centre = mats[1][1] + np.array([0.0, -60.0, 0.0])
                t.render(group, h, mats, path, size=(400, 300), yaw=yaw, pitch=0.15, distance=1500.0, center=centre)
                tiles.append(path)
        ims = [Image.open(p) for p in tiles]
        w, hh = ims[0].size
        sheet = Image.new('RGB', (w * 10, hh * 2))
        d = ImageDraw.Draw(sheet)
        for i, im in enumerate(ims):
            sheet.paste(im, ((i % 10) * w, (i // 10) * hh))
            d.text(((i % 10) * w + 4, (i // 10) * hh + 4), f'{"one" if i >= 10 else "140"} {i % 10}', fill=(255, 255, 0))
        sheet.save(out / f'sheet-{name}.png')
        print(out / f'sheet-{name}.png')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--group', type=int, default=0)
    ap.add_argument('--render', type=Path, help='write comparison sheets (original 140 / one-handed) here')
    ap.add_argument('--set', action='append', default=[], metavar='KEY=VALUE')
    a = ap.parse_args()
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
        for i, f in enumerate(frames(group, params)):
            for c in f['commands']:
                t.encode_command(c)
        print('ok')


if __name__ == '__main__':
    main()
