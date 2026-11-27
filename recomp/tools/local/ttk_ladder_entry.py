#!/usr/bin/env python3
"""D08J10: Duke's legs stepping onto a ladder from the floor (note 562).

MODERN LADDER CLIMBING starts the ground mount in the climb step 188 at
Duke's standing height (ladder_grab.inc); the upper body is 188's own. The
original grab triggers about 275 units from the panel and the climb line is
200 from it, so for the step's opening hold the host eases Duke 75 forward.
On 188's own legs his feet would climb the air in front of the rungs (user,
2026-11-26: "hes miming as if he's on the rungs when hes not"). This clip
holds the legs for that hold instead:

- the right leg (ROT 5-8) stays planted where he stood: two-bone IK onto its
  standing ankle while the pelvis moves over it;
- the left leg (ROT 1-4) lifts from the standing pose onto the bottom rung,
  reaching 188's own first-frame leg (which puts the toe on the bottom rung,
  measured: rungs at toe height floor - 234 - 259 n, 200 from the line).

Keys per player update after the conversion (KEYS + 1, the last is the full
step-in). The host (ladder_grab.inc) writes them over the legs at weight 1
and then hands back to 188's live legs, which keep the left foot on that
rung and bring the right foot up to the next one. Pelvis, root and upper body
are never written. Duke-local frame: x right, y down, z ahead; floor y = 0.

  python3 recomp/tools/local/ttk_ladder_entry.py --render OUT_DIR    # side/quarter sheets with the rungs drawn
"""
import argparse
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ttk_anim as t
import ttk_crouch_walk as cw

ENTRY_CLIP = 61               # host extra clip id (group 0xffff)
IDLE, CLIMB = 63, 188
KEYS = 3                      # updates of the step-in (ladder_entry_keys in ladder_grab.inc)
APPROACH = 75.0               # grab distance 275 - climb line 200
PELVIS = 515.0                # pelvis over the floor at the grab (alley 523, sewer 500)
RUNG0, RUNG_STEP, PANEL = 234.0, 259.0, 200.0
LEG_IDS = (1, 2, 3, 4, 5, 6, 7, 8)
RUNG_LEG = (10, 11, 12, 13)   # left: hip, knee, ankle, toe records (ROT 1-4)
SUPPORT_LEG = (14, 15, 16, 17)  # right (ROT 5-8)
PARAMS = dict(lift=90.0)      # extra knee lift mid-way (units of ankle height)


def ease(u):
    return u * u * (3 - 2 * u)


def carried(group, anim, k):
    q = {j: [0, 0, 0, 4096] for j in range(22)}
    for i in range(k + 1):
        q.update(group.animation_quats(anim, i)[0])
    return q


def slerp_q(a, b, w):
    q = t.slerp(a, b, w)
    n = float(np.linalg.norm(q))
    return [int(round(v * 4096.0 / n)) for v in q] if n > 0 else [0, 0, 0, 4096]


def fit13(q):
    q = list(q)
    if q[3] < 0:
        q = [-v for v in q]
    if q[3] > 4095:
        s = 4095.0 / q[3]
        q = [int(round(v * s)) for v in q]
    return q


def pelvis_at(u):
    return np.array([0.0, -PELVIS, APPROACH * ease(u)])


def key_pose(group, key, params=None):
    """Joint id -> Q12 quaternion of the legs at step-in key 0..KEYS."""
    p = {**PARAMS, **(params or {})}
    h = group.hierarchies[t.DUKE_HIERARCHY]
    stand, climb = carried(group, IDLE, 0), carried(group, CLIMB, 0)
    u = key / KEYS
    s = ease(u)
    # The live pelvis and chest are 188's (the host never writes them).
    base = {**climb}
    out = {}
    # Rung leg: standing -> 188's rung pose, with the knee raised mid-way.
    for j in (1, 2, 3, 4):
        out[j] = slerp_q(stand[j], climb[j], s)
    if p['lift'] and 0 < key < KEYS:
        mats = t.pose(h, {**base, **out}, root_position=pelvis_at(u))
        leg = cw.Leg(group, RUNG_LEG)
        pel_m, pel_t = mats[0]
        ankle_w = mats[RUNG_LEG[2]][1] + np.array([0.0, -p['lift'] * math.sin(math.pi * u), 0.0])
        fwd = pel_m @ np.array([0.0, 0.0, 1.0])
        hip, knee, ankle, _ = leg.solve(pel_m, pel_t, ankle_w, mats[RUNG_LEG[2]][0], fwd)
        out[1], out[2], out[3] = cw.game_q(hip), cw.game_q(knee), cw.game_q(ankle)
    # Support leg: the standing ankle stays planted on the floor.
    stand_mats = t.pose(h, {**base, **{j: stand[j] for j in (5, 6, 7, 8)}}, root_position=pelvis_at(0))
    planted_t, planted_m = stand_mats[SUPPORT_LEG[2]][1], stand_mats[SUPPORT_LEG[2]][0]
    mats = t.pose(h, {**base, **out, **{j: stand[j] for j in (5, 6, 7, 8)}}, root_position=pelvis_at(u))
    pel_m, pel_t = mats[0]
    leg = cw.Leg(group, SUPPORT_LEG)
    fwd = pel_m @ np.array([0.0, 0.0, 1.0])
    hip, knee, ankle, _ = leg.solve(pel_m, pel_t, planted_t, planted_m, fwd)
    out[5], out[6], out[7], out[8] = cw.game_q(hip), cw.game_q(knee), cw.game_q(ankle), stand[8]
    return {j: fit13(q) for j, q in out.items()}


def frames(group, params=None):
    fr = []
    for key in range(KEYS + 1):
        q = key_pose(group, key, params)
        fr.append(dict(events=0, ticks=cw.HOST_TICKS, commands=[dict(joint=j, q=q[j], flags=0) for j in LEG_IDS]))
    return cw.host_frames(fr)


def pack_entries(group):
    return [dict(group=0xffff, clip=ENTRY_CLIP, source_sha256='00' * 32, frames=frames(group))]


def contact_report(group, params=None):
    """Per key: the toes' height over the floor and distance short of the panel."""
    h = group.hierarchies[t.DUKE_HIERARCHY]
    climb = carried(group, CLIMB, 0)
    rows = []
    for key in range(KEYS + 1):
        u = key / KEYS
        mats = t.pose(h, {**climb, **key_pose(group, key, params)}, root_position=pelvis_at(u))
        panel = APPROACH + PANEL
        rows.append(dict(key=key, left_toe=[round(-mats[13][1][1]), round(panel - mats[13][1][2])],
                         right_toe=[round(-mats[17][1][1]), round(panel - mats[17][1][2])],
                         right_ankle=[round(v) for v in mats[16][1]]))
    return rows


def render_sheets(group, out, params=None):
    from PIL import Image, ImageDraw
    out.mkdir(parents=True, exist_ok=True)
    h = group.hierarchies[t.DUKE_HIERARCHY]
    climb = carried(group, CLIMB, 0)
    views = dict(side=-math.pi / 2, quarter=-2.2)
    size, distance, pitch = (300, 260), 1700.0, 0.12
    centre = np.array([0.0, -420.0, 120.0])
    for name, yaw in views.items():
        ims = []
        poses = [(f'key {k}', {**climb, **key_pose(group, k, params)}, pelvis_at(k / KEYS)) for k in range(KEYS + 1)]
        poses.append(('188 legs', climb, pelvis_at(1.0)))
        for label, q, root in poses:
            path = out / f'{name}-{len(ims)}.png'
            t.render(group, h, t.pose(h, q, root_position=root), path, size=size, yaw=yaw, pitch=pitch,
                     distance=distance, center=centre)
            im = Image.open(path).convert('RGB')
            d = ImageDraw.Draw(im)
            cy, sy, cp, sp = math.cos(yaw), math.sin(yaw), math.cos(pitch), math.sin(pitch)
            view = np.array([[1, 0, 0], [0, cp, sp], [0, -sp, cp]]) @ np.array([[cy, 0, -sy], [0, 1, 0], [sy, 0, cy]])
            hh, ww = size
            focal = 1.6 * hh

            def proj(pt):
                c = (np.asarray(pt, float) - centre) @ view.T
                z = c[2] + distance
                return ww / 2 + focal * c[0] / z, hh / 2 + focal * c[1] / z
            panel = APPROACH + PANEL
            d.line([proj((-400, 0, -300)), proj((400, 0, panel))], fill=(120, 120, 120))
            for n in range(3):
                y = -(RUNG0 + RUNG_STEP * n) + 6
                d.line([proj((-190, y, panel)), proj((190, y, panel))], fill=(255, 160, 40), width=2)
            d.line([proj((0, 0, panel)), proj((0, -900, panel))], fill=(160, 100, 40))
            d.text((4, 4), label, fill=(255, 255, 0))
            ims.append(im)
        w, hh = ims[0].size
        sheet = Image.new('RGB', (w * len(ims), hh))
        for i, im in enumerate(ims):
            sheet.paste(im, (i * w, 0))
        sheet.save(out / f'sheet-{name}.png')
        print(out / f'sheet-{name}.png')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--group', type=int, default=0)
    ap.add_argument('--render', type=Path)
    ap.add_argument('--set', action='append', default=[], metavar='KEY=VALUE')
    a = ap.parse_args()
    params = {}
    for kv in a.set:
        key, value = kv.split('=', 1)
        params[key] = float(value)
    group = t.Group(a.group)
    for row in contact_report(group, params):
        print(row)
    if a.render:
        render_sheets(group, a.render, params)
    for f in frames(group, params):
        for c in f['commands']:
            t.encode_command(c)
    print('ok')


if __name__ == '__main__':
    main()
