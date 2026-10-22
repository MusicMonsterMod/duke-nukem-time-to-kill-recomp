#!/usr/bin/env python3
"""D08K3: build crouch-walk legs clips for Duke (documentation note 186).

The original has no low walk: every crouch pose (176-180) is a one-knee kneel
and directional crouch input rolls. These clips are a new gait solved with
two-bone inverse kinematics on Duke's own skeleton (hierarchy 1): the pelvis
stays low, each foot follows a stance/swing path (a planted foot slides back
under the body by exactly the body's travel, a swinging foot lifts and steps
ahead), the hip and knee hinge are solved for the ankle target with the knee
pointing forward, and the ankle keeps the standing idle's flat, forward foot.
The root travel per keyframe equals the planted feet's slide, so feet do not
skate. Footstep events follow the original walk (4 = one foot, 2 = the other,
1 = clip end).

The skeleton geometry and the idle's foot orientation are read from the
owned disc; the generated clips are written where you say (keep them under
ignored recomp/analysis/ or the build tree).

  python3 recomp/tools/local/ttk_crouch_walk.py OUT_DIR [--pelvis 330] [--speed 200]
  python3 recomp/tools/local/ttk_crouch_walk.py --host-pack build-local/ttk-anim.pack
  python3 recomp/tools/local/ttk_crouch_walk.py --host-pack low.pack --set hip_ratio=0.6   # a variant
"""
import argparse
import json
import math
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ttk_anim as t

# Direction -> unit travel (game x, z) and the original walk whose legs clip a
# preview pack replaces (forward 72, backward 82, strafes 90 / 91).
DIRECTIONS = {'forward': ((0, 1), 72), 'backward': ((0, -1), 82), 'strafe_pos_x': ((1, 0), 90),
              'strafe_neg_x': ((-1, 0), 91)}
# The original walks are half cycles (one step each, footstep 4 or 2): forward
# 72 then 74, backward 82 then 83; the strafes 90 / 91 are whole cycles.
HALVES = {'forward': (72, 74), 'backward': (82, 83)}
IDLE = 63
LEGS = ((10, 11, 12, 13), (14, 15, 16, 17))   # hip, knee, ankle, toe records per leg
ANKLE_HEIGHT = 36.0                           # idle ankle over the floor


def textbook(q):
    w, x, y, z = q / np.linalg.norm(q)
    return np.array([[1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)],
                     [2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
                     [2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)]])


def quat_of(m):
    tr = np.trace(m)
    if tr > 0:
        s = math.sqrt(tr + 1.0) * 2
        q = np.array([0.25 * s, (m[2, 1] - m[1, 2]) / s, (m[0, 2] - m[2, 0]) / s, (m[1, 0] - m[0, 1]) / s])
    else:
        i = int(np.argmax(np.diag(m)))
        j, k = (i + 1) % 3, (i + 2) % 3
        s = math.sqrt(1.0 + m[i, i] - m[j, j] - m[k, k]) * 2
        q = np.zeros(4)
        q[0] = (m[k, j] - m[j, k]) / s
        q[1 + i] = 0.25 * s
        q[1 + j] = (m[j, i] + m[i, j]) / s
        q[1 + k] = (m[k, i] + m[i, k]) / s
    return q


def game_q(local):
    """Game Q12 quaternion whose 0x8002a5ac matrix is `local` (= textbook(q)^T)."""
    w, x, y, z = quat_of(local.T)
    if w < 0:
        w, x, y, z = -w, -x, -y, -z
    s = 4095.0 if w * 4096 > 4095 else 4096.0
    return [int(round(x * s)), int(round(y * s)), int(round(z * s)), int(round(w * s))]


def axis_angle(axis, angle):
    axis = np.asarray(axis, float) / np.linalg.norm(axis)
    return textbook(np.array([math.cos(angle / 2), *(axis * math.sin(angle / 2))]))


def align(a, b):
    """Smallest rotation taking direction a to direction b."""
    a, b = a / np.linalg.norm(a), b / np.linalg.norm(b)
    v, c = np.cross(a, b), float(a @ b)
    if np.linalg.norm(v) < 1e-9:
        return np.eye(3) if c > 0 else axis_angle(np.cross(a, [1, 0, 0]) if abs(a[0]) < 0.9 else np.cross(a, [0, 1, 0]), math.pi)
    vx = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    return np.eye(3) + vx + vx @ vx * (1 / (1 + c))


class Leg:
    def __init__(self, group, records):
        h = group.hierarchies[t.DUKE_HIERARCHY]
        self.records = records
        self.rot = [h['joints'][r]['rot_id'] for r in records]
        self.hip_offset = np.array(h['joints'][records[0]]['offset'], float)
        self.thigh = np.array(h['joints'][records[1]]['offset'], float)
        self.shin = np.array(h['joints'][records[2]]['offset'], float)
        self.toe = np.array(h['joints'][records[3]]['offset'], float)

    def knee_local(self, d):
        """Knee hinge (local x) whose thigh+shin span distance d (bent forward)."""
        def span(theta):
            return np.linalg.norm(self.thigh + t.quat_matrix([math.sin(theta / 2) * 4096, 0, 0, math.cos(theta / 2) * 4096]) @ self.shin)
        lo, hi = 0.0, math.pi * 0.95
        d = min(max(d, span(hi) + 1e-3), span(lo) - 1e-3)
        for _ in range(60):
            mid = (lo + hi) / 2
            if span(mid) > d:
                lo = mid
            else:
                hi = mid
        theta = (lo + hi) / 2
        return t.quat_matrix([math.sin(theta / 2) * 4096, 0, 0, math.cos(theta / 2) * 4096])

    def solve(self, pelvis_m, pelvis_t, ankle_target, ankle_world, forward):
        """(hip, knee, ankle) local matrices putting the ankle at ankle_target."""
        hip_pos = pelvis_m @ self.hip_offset + pelvis_t
        knee = self.knee_local(np.linalg.norm(ankle_target - hip_pos))
        v_world_dir = ankle_target - hip_pos
        # Hip world rotation: thigh+shin direction onto the target, twisted about
        # it so the knee points forward.
        v_local = self.thigh + knee @ self.shin
        base = align(v_local, v_world_dir)
        best, best_score = base, -1e9
        for k in range(720):
            cand = axis_angle(v_world_dir, k * math.pi / 360) @ base
            score = (cand @ self.thigh) @ forward
            if score > best_score:
                best, best_score = cand, score
        hip_world = best
        hip_local = pelvis_m.T @ hip_world
        knee_world = hip_world @ knee
        ankle_local = knee_world.T @ ankle_world
        return hip_local, knee, ankle_local, hip_pos + hip_world @ self.thigh


def gait(phase, stance, step, lift):
    """Foot offset along the travel (relative to its neutral point) and lift."""
    phase %= 1.0
    if phase < stance:                       # planted: slides back from +step/2 to -step/2
        u = phase / stance
        return step / 2 - step * u, 0.0
    u = (phase - stance) / (1 - stance)      # swing: back to front with a lift
    s = (1 - math.cos(math.pi * u)) / 2
    return -step / 2 + step * s, lift * math.sin(math.pi * u)


def build(group, direction, pelvis_height=330.0, speed=200.0, frames=8, stance=0.6, step=150.0, lift=55.0,
          stance_width=1.15, bob=8.0, lean=0.0):
    """lean: forward pitch of the pelvis in degrees (the upper body follows)."""
    (dx, dz), _ = DIRECTIONS.get(direction, ((0, 1), 72))
    travel_dir = np.array([dx, 0.0, dz])
    h = group.hierarchies[t.DUKE_HIERARCHY]
    idle_q, idle_roots = group.animation_quats(IDLE, 0)
    idle = t.pose(h, idle_q, root_position=t.root_position(h, idle_roots))
    legs = [Leg(group, r) for r in LEGS]
    ankle_world = [idle[l.records[2]][0] for l in legs]          # flat, forward feet
    neutral = [np.array([idle[l.records[2]][1][0] * stance_width, 0.0, 10.0]) for l in legs]
    toe = {l.rot[3]: idle_q.get(l.rot[3], (0, 0, 0, 0)) for l in legs}
    cycle_travel = step / stance                                 # body travel per cycle
    cycle_ticks = cycle_travel / speed * 250.0
    ticks = max(1, int(round(cycle_ticks / frames)))
    per_frame = cycle_travel / frames
    pelvis_m = axis_angle([1.0, 0.0, 0.0], -math.radians(lean))   # top of the body towards +z
    out = []
    prev_y = None
    root_offset = h['joints'][0]['offset'][1]                    # -561
    for k in range(frames):
        phase = k / frames
        p_height = pelvis_height + bob * math.cos(4 * math.pi * phase)
        pelvis_t = np.array([0.0, -p_height, 0.0])
        cmds = []
        root_y = int(round(-p_height - root_offset))             # pelvis y = -561 + root y
        ry = root_y if prev_y is None else root_y - prev_y
        prev_y = root_y
        move = travel_dir * per_frame
        cmds.append(dict(joint=0, root=[int(round(move[0])), ry, int(round(move[2]))], flags=2))
        if lean:
            cmds.append(dict(joint=0, q=game_q(pelvis_m), flags=0))
        events = 0
        for i, leg in enumerate(legs):
            lp = phase + 0.5 * i
            along, up = gait(lp, stance, step, lift)
            target = neutral[i] + travel_dir * along + np.array([0.0, -(ANKLE_HEIGHT + up), 0.0])
            hip_l, knee_l, ankle_l, _ = leg.solve(pelvis_m, pelvis_t, target, ankle_world[i], np.array([0.0, 0.0, 1.0]))
            cmds += [dict(joint=leg.rot[0], q=game_q(hip_l), flags=0), dict(joint=leg.rot[1], q=game_q(knee_l), flags=0),
                     dict(joint=leg.rot[2], q=game_q(ankle_l), flags=0)]
            if toe[leg.rot[3]] != (0, 0, 0, 0):
                cmds.append(dict(joint=leg.rot[3], q=list(toe[leg.rot[3]]), flags=0))
            # touchdown on this keyframe: footstep (4 for the first leg, 2 for the second)
            if abs((lp % 1.0)) < 1e-9 or abs((lp % 1.0) - 1.0) < 1e-9:
                events |= 4 if i == 0 else 2
        if k == frames - 1:
            events |= 1
        out.append(dict(events=events, ticks=ticks, commands=cmds))
    return out


def metrics(group, frames, upper=178, sub=4):
    """Foot planting and height of a looping legs clip (keyframes and slerp
    steps between them, travel applied over each interval): the planted (lower)
    foot's horizontal slide per step, the planted foot's height over the floor,
    and the highest vertex with `upper`'s upper body."""
    h = group.hierarchies[t.DUKE_HIERARCHY]
    up, _ = group.animation_quats(upper, 0)
    q = [t.joint_quats_for_frame(f) for f in frames]
    roots = [next((c['root'] for c in f['commands'] if 'root' in c), [0, 0, 0]) for f in frames]
    heights, y = [], 0
    for r in roots:
        y += r[1]
        heights.append(y)
    base = h['joints'][0]['offset'][1]
    feet_records = [legs[3] for legs in LEGS]
    samples, travel, top = [], np.zeros(3), 0.0
    n = len(frames)
    for k in range(n):
        b = (k + 1) % n
        d = np.array([roots[b][0], 0.0, roots[b][2]])
        for s in range(sub):
            u = s / sub
            pose = dict(up)
            pose.update({j: t.slerp(q[k].get(j, (0, 0, 0, 4096)), q[b].get(j, (0, 0, 0, 4096)), u) for j in set(q[k]) | set(q[b])})
            mats = t.pose(h, pose, root_position=(0.0, base + heights[k] * (1 - u) + heights[b] * u, 0.0))
            feet = []
            for j in feet_records:
                m, tr = mats[j]
                v = np.array(group.model(h['joints'][j]['model'])['vertices'], float) @ m.T + tr
                feet.append(v[int(np.argmax(v[:, 1]))] + travel + d * u)
            for j, joint in enumerate(h['joints']):
                m, tr = mats[j]
                v = np.array(group.model(joint['model'])['vertices'], float) @ m.T + tr
                top = max(top, -float(v[:, 1].min()))
            samples.append(feet)
        travel = travel + d
    slide, ground = [], []
    for a, b in zip(samples, samples[1:]):
        st = int(np.argmax([a[0][1], a[1][1]]))
        ground.append(-a[st][1])
        if abs(a[st][1] - b[st][1]) < 15:
            slide.append(float(np.hypot(*(b[st][[0, 2]] - a[st][[0, 2]]))))
    return dict(slide_max=max(slide), slide_mean=sum(slide) / len(slide), ground_min=min(ground),
                ground_max=max(ground), top=top)


def split_half(frames, first, count):
    """frames[first:first+count] as a clip of its own: the root height of its
    first frame made absolute again, the clip-end event on its last frame."""
    heights, y = [], 0
    for f in frames:
        y += next(c['root'][1] for c in f['commands'] if 'root' in c)
        heights.append(y)
    out = []
    for k in range(first, first + count):
        f = json.loads(json.dumps(frames[k]))
        for c in f['commands']:
            if 'root' in c:
                c['root'][1] = heights[k] if k == first else heights[k] - heights[k - 1]
        f['events'] = (f['events'] & ~1) | (1 if k == first + count - 1 else 0)
        out.append(f)
    return out


# ------------------------------------------------------------------ mocap
# D08K6: a tactical crouch walk. The motion of a captured crouch walk (CC-BY
# 4.0, see recomp/assets/anim/crouch-walk-mocap.json, written by
# extract_crouch_mocap.py) retargeted onto Duke by proportion, plus a designed
# upper body (documentation note 188):
# - legs: the hip joints at the capture's height in leg lengths (HIP_RATIO),
#   each ankle on the capture's path (scaled to Duke's leg, the stride scaled
#   so a cycle travels exactly CYCLE), the feet narrow under the hips (WIDTH x
#   the hip joints' spread, slight toe-out) and rolling heel to toe as
#   captured, knees over the toes, the same two-bone IK. D08K4 put the pelvis
#   at 300 (thighs level) with the feet 4x the hips apart: Duke "sitting".
# - pelvis: nearly level (PELVIS_PITCH) with the capture's hip yaw and roll
#   swing; the capture bends at the spine, not the pelvis.
# - chest (Duke's torso is one joint): leaning CHEST_PITCH over the knees with
#   a shoulder counter-swing; head (HEAD_PITCH) looking ahead.
# - arms, when no weapon owns them: fists up in front of the chest with a small
#   counter-swing (the kneel's arms hung to the knees).
# Upper-body joints go in separate host clips (UPPER_CLIPS) that the runtime
# reads directly; they are never put on a channel (a channel's joint mask only
# grows, see crouch_channels_sanitize in crouch_walk.inc).
MOCAP = Path(__file__).resolve().parents[2] / 'assets/anim/crouch-walk-mocap.json'
CYCLE = 640.0
# Strafes: quicker, smaller side steps (a 640 sideways stride reads as a split).
STRAFE_CYCLE = 380.0
STRAFE_FORWARD = 0.3
TACTICAL = dict(hip_ratio=0.56, width=1.5, toe_out=6.0, pelvis_pitch=12.0, yaw_swing=1.0, roll_swing=1.0,
                chest_pitch=62.0, chest_counter=0.6, head_pitch=5.0,
                hand_forward=150.0, hand_down=70.0, hand_in=40.0, arm_swing=12.0)
ARMS = ((2, 3, 4), (5, 6, 7))   # shoulder, forearm, hand records (left, right)


def _orthonormal(m):
    u, _, vt = np.linalg.svd(m)
    r = u @ vt
    return r if np.linalg.det(r) > 0 else -r


def resample_by_travel(mc, count=24):
    """The capture resampled at even steps of planted-foot travel: the host
    advances the gait phase with distance, so equal phase steps must be equal
    travel or the feet skate where the captured pace is uneven."""
    src = mc['frames']
    n = len(src)
    sides = ('left', 'right')
    steps = []
    for k in range(n):
        a, b = src[k], src[(k + 1) % n]
        sd = min(sides, key=lambda x: a[x + '_ankle_height'] + b[x + '_ankle_height'])
        steps.append(max(abs(b[sd + '_ankle'][2] - a[sd + '_ankle'][2]), 1e-6))
    cum = np.concatenate([[0.0], np.cumsum(steps)])
    total = cum[-1]
    out = []
    for j in range(count):
        u = total * j / count
        k = int(np.searchsorted(cum, u, side='right') - 1)
        w = (u - cum[k]) / (cum[k + 1] - cum[k])
        a, b = src[k % n], src[(k + 1) % n]
        rec = {}
        for key, va in a.items():
            vb = b[key]
            if isinstance(va, float):
                rec[key] = va * (1 - w) + vb * w
            elif isinstance(va[0], list):
                rec[key] = _orthonormal(np.array(va) * (1 - w) + np.array(vb) * w).tolist()
            else:
                rec[key] = list(np.array(va) * (1 - w) + np.array(vb) * w)
        out.append(rec)
    return out


def rot_x(deg):
    """Pitch: +deg tips the top of the body towards +z (forward)."""
    return axis_angle([1.0, 0.0, 0.0], -math.radians(deg))


def rot_y(deg):
    return axis_angle([0.0, 1.0, 0.0], math.radians(deg))


def rot_z(deg):
    return axis_angle([0.0, 0.0, 1.0], math.radians(deg))


def yaw_roll(m):
    """Yaw (about y) and roll (about z) of a game-axes rotation, degrees."""
    fw, rt = m @ np.array([0.0, 0.0, 1.0]), m @ np.array([1.0, 0.0, 0.0])
    return math.degrees(math.atan2(fw[0], fw[2])), math.degrees(math.atan2(-rt[1], rt[0]))


class Arm(Leg):
    """Shoulder, forearm (a hinge about local x that bends negative, as in the
    original arms) and hand records."""
    def __init__(self, group, records):
        h = group.hierarchies[t.DUKE_HIERARCHY]
        self.records = records
        self.rot = [h['joints'][r]['rot_id'] for r in records]
        self.hip_offset = np.array(h['joints'][records[0]]['offset'], float)
        self.thigh = np.array(h['joints'][records[1]]['offset'], float)
        self.shin = np.array(h['joints'][records[2]]['offset'], float)

    def knee_local(self, d):
        def hinge(theta):
            return t.quat_matrix([math.sin(theta / 2) * 4096, 0, 0, math.cos(theta / 2) * 4096])

        def span(theta):
            return np.linalg.norm(self.thigh + hinge(theta) @ self.shin)
        lo, hi = 0.0, -math.pi * 0.9
        d = min(max(d, span(hi) + 1e-3), span(lo) - 1e-3)
        for _ in range(60):
            mid = (lo + hi) / 2
            if span(mid) > d:
                lo = mid
            else:
                hi = mid
        return hinge((lo + hi) / 2)


def mocap_frames(group, direction, params=None):
    """Legs frames (channel 0: root height command, pelvis, legs) for a
    direction ('forward', 'backward', 'strafe_pos_x', 'strafe_neg_x', 'idle');
    each frame also carries 'upper': ROT id -> Q12 quaternion for the chest,
    head and both arms."""
    p = dict(TACTICAL, **(params or {}))
    idle = direction == 'idle'
    mc = json.loads(MOCAP.read_text())
    src = resample_by_travel(mc)
    n = len(src)
    h = group.hierarchies[t.DUKE_HIERARCHY]
    joints = h['joints']
    idle_q, idle_roots = group.animation_quats(IDLE, 0)
    idle_pose = t.pose(h, idle_q, root_position=t.root_position(h, idle_roots))
    legs = [Leg(group, r) for r in LEGS]
    arms = [Arm(group, r) for r in ARMS]
    leg_len = float(np.linalg.norm(legs[0].thigh) + np.linalg.norm(legs[0].shin))
    ankle_world = [idle_pose[l.records[2]][0] for l in legs]
    toe = {l.rot[3]: idle_q.get(l.rot[3], (0, 0, 0, 0)) for l in legs}
    # Capture sides: legs[0] is Duke's negative-x leg.
    sides = ('left', 'right') if mc['left_is_game_negative_x'] else ('right', 'left')
    mean_hips = float(np.mean([f['hips_height'] for f in src]))
    rel = {sd: np.array([f[sd + '_ankle'] for f in src]) * leg_len for sd in sides}
    mean_z = {sd: float(rel[sd][:, 2].mean()) for sd in sides}
    # Stride: the planted (lower) foot's travel per cycle, scaled to CYCLE.
    travel = 0.0
    for k in range(n):
        a, b = src[k], src[(k + 1) % n]
        sd = min(sides, key=lambda x: a[x + '_ankle_height'] + b[x + '_ankle_height'])
        travel += abs(b[sd + '_ankle'][2] - a[sd + '_ankle'][2]) * leg_len
    stride = (STRAFE_CYCLE if direction.startswith('strafe') else CYCLE) / travel
    swings = [yaw_roll(np.array(f['hips_rotation'])) for f in src]
    mean_yaw, mean_roll = (float(np.mean([s[i] for s in swings])) for i in (0, 1))
    # Pelvis over the floor: the hip joints (offset +y below it) at the capture's
    # height over the ankles, in leg lengths.
    pelvis_height = ANKLE_HEIGHT + p['hip_ratio'] * leg_len + joints[LEGS[0][0]]['offset'][1]
    order = list(range(n))
    if direction == 'backward':
        order = order[::-1]
    out = []
    for k in order:
        f = src[k]
        yaw = 0.0 if idle else (swings[k][0] - mean_yaw) * p['yaw_swing']
        roll = 0.0 if idle else (swings[k][1] - mean_roll) * p['roll_swing']
        pelvis_m = rot_y(yaw) @ rot_z(roll) @ rot_x(p['pelvis_pitch'])
        height = pelvis_height + (0.0 if idle else (f['hips_height'] - mean_hips) * leg_len)
        pelvis_t = np.array([0.0, -height, 0.0])
        # Host clips carry the pelvis height over the feet (absolute, x = z = 0)
        # in a root translation command; crouch_walk.inc reads it.
        cmds = [dict(joint=0, root=[0, int(round(height)), 0], flags=2), dict(joint=0, q=game_q(pelvis_m), flags=0)]
        for i, leg in enumerate(legs):
            sd = sides[i]
            side = -1.0 if leg.hip_offset[0] < 0 else 1.0
            dz = 0.0 if idle else (rel[sd][k][2] - mean_z[sd]) * stride
            along = np.array([0.0, 0.0, mean_z[sd] + dz])
            # Strafes: the side step under the hips (the capture's forward
            # stance offset would bring the knees forward: "sitting").
            if direction == 'strafe_pos_x':
                along = np.array([dz, 0.0, mean_z[sd] * STRAFE_FORWARD])
            elif direction == 'strafe_neg_x':
                along = np.array([-dz, 0.0, mean_z[sd] * STRAFE_FORWARD])
            lift = 0.0 if idle else f[sd + '_ankle_height'] * leg_len
            target = np.array([leg.hip_offset[0] * p['width'] + along[0], -(ANKLE_HEIGHT + lift), along[2]])
            toe_out = rot_y(side * p['toe_out'])
            foot = np.eye(3) if idle else np.array(f[sd + '_foot_rotation'])
            foot_world = toe_out @ foot @ ankle_world[i]
            # Duke's foot is longer than the capture's (leg-relative): lift the
            # ankle so a pitched-down toe stays on the floor.
            toe_y = target[1] + (foot_world @ leg.toe)[1]
            if toe_y > 0:
                target[1] -= toe_y
            hip_l, knee_l, ankle_l, _ = leg.solve(pelvis_m, pelvis_t, target, foot_world, toe_out @ np.array([0.0, 0.0, 1.0]))
            cmds += [dict(joint=leg.rot[0], q=game_q(hip_l), flags=0), dict(joint=leg.rot[1], q=game_q(knee_l), flags=0),
                     dict(joint=leg.rot[2], q=game_q(ankle_l), flags=0)]
            if toe[leg.rot[3]] != (0, 0, 0, 0):
                cmds.append(dict(joint=leg.rot[3], q=list(toe[leg.rot[3]]), flags=0))
        # Upper body: world targets turned into locals under the pelvis.
        chest_m = rot_y(-yaw * p['chest_counter']) @ rot_x(p['chest_pitch'])
        head_m = rot_y(-yaw * 0.3) @ rot_x(p['head_pitch'])
        upper = {9: game_q(pelvis_m.T @ chest_m), 16: game_q(chest_m.T @ head_m)}
        chest_t = pelvis_m @ np.array(joints[1]['offset'], float) + pelvis_t
        swing = 0.0 if idle else math.sin(2 * math.pi * k / n) * p['arm_swing']
        for a, arm in enumerate(arms):
            side = -1.0 if a == 0 else 1.0
            shoulder = chest_m @ arm.hip_offset + chest_t
            target = shoulder + np.array([-side * p['hand_in'], p['hand_down'], p['hand_forward'] + (swing if a == 0 else -swing)])
            # Elbows back, down and out; the fist turned palm-in.
            s, e, hand, _ = arm.solve(chest_m, chest_t, target, rot_x(-70), np.array([side * 0.5, 0.6, -1.0]))
            upper.update({arm.rot[0]: game_q(s), arm.rot[1]: game_q(e), arm.rot[2]: game_q(hand)})
        out.append(dict(events=0, ticks=HOST_TICKS, commands=cmds, upper=upper))
    return out[:2] if idle else out


def pose_top(group, frames):
    """Highest vertex over the floor of the full pose (legs and upper body)."""
    h = group.hierarchies[t.DUKE_HIERARCHY]
    top = 0.0
    for f in frames:
        q = t.joint_quats_for_frame(f)
        q.update(f.get('upper', {}))
        mats = t.pose(h, q, root_position=(0.0, -f['commands'][0]['root'][1], 0.0))
        for j, joint in enumerate(h['joints']):
            v = np.array(group.model(joint['model'])['vertices'], float) @ mats[j][0].T + mats[j][1]
            top = max(top, -float(v[:, 1].min()))
    return top


# D08K4 host-driven clips: the crouch walk's channel 0 is set by the host every
# update (crouch_walk.inc), so the frames are long (the original stepper never
# advances them). Pack entries with group 0xffff are extra clips: appended,
# never bound to an original clip id. UPPER_CLIPS hold the same frames' chest,
# head and arm joints (D08K6), read by the host only.
HOST_CLIPS = {1: 'forward', 2: 'backward', 3: 'strafe_pos_x', 4: 'strafe_neg_x', 5: 'idle'}
UPPER_CLIPS = {cid + 10: name for cid, name in HOST_CLIPS.items()}
HOST_TICKS = 30000
# Procedural fallback (no motion asset): the D08K4 revision 2 gait. The host
# mirrors PELVIS/BOB and the cycle travel STEP / STANCE (crouch_walk.inc).
PELVIS, BOB, LEAN, STEP, STANCE, LIFT, WIDTH, FRAMES = 300.0, 10.0, 25.0, 240.0, 0.55, 45.0, 0.95, 12


def host_clip(group, direction, upper=False):
    if MOCAP.exists():
        frames = mocap_frames(group, direction)
        if upper:
            frames = [dict(events=0, ticks=HOST_TICKS, commands=[dict(joint=j, q=q, flags=0) for j, q in f['upper'].items()])
                      for f in frames]
        return host_frames(frames, keep_root=True)
    if upper:
        return None
    if direction == 'idle':   # the crouch held still: both feet planted under the hips
        frames = build(group, 'forward', PELVIS, frames=2, stance=STANCE, step=0.0, lift=0.0, stance_width=WIDTH, bob=0.0, lean=LEAN)
    else:
        frames = build(group, direction, PELVIS, frames=FRAMES, stance=STANCE, step=STEP, lift=LIFT, stance_width=WIDTH, bob=BOB, lean=LEAN)
    return host_frames(frames)


def host_frames(frames, keep_root=False):
    out = []
    for f in frames:
        # No frame events: the host rewrites the frames every update.
        out.append(dict(events=0, ticks=HOST_TICKS,
                        commands=[c for c in f['commands'] if keep_root or 'root' not in c]))
    # A leading copy of the first frame that the host never shows (a margin
    # before the clip, as the padding clip is after the last one).
    return [json.loads(json.dumps(out[0]))] + out


def write_host_pack(group, path):
    entries = [dict(group=0xffff, clip=cid, source_sha256='00' * 32, frames=host_clip(group, name))
               for cid, name in HOST_CLIPS.items()]
    for cid, name in UPPER_CLIPS.items():
        frames = host_clip(group, name, upper=True)
        if frames:
            entries.append(dict(group=0xffff, clip=cid, source_sha256='00' * 32, frames=frames))
    # Padding after the last clip: the original stepper (0x80059db0) loops a
    # channel within the animation's own clip, so if it ever advanced a
    # host-driven channel it would walk on past the extra clips; with a weapon
    # drawn it did, into the command table, and the blend 0x80059004 then
    # never ended (D08K4 hang). Host frames are long (HOST_TICKS) so it should
    # not; if it does it still reads valid frames here.
    pad = host_clip(group, 'idle')[1]
    entries.append(dict(group=0xffff, clip=99, source_sha256='00' * 32, frames=[pad] * 16))
    return t.build_pack(entries, path)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('output', type=Path)
    p.add_argument('--group', type=int, default=0)
    p.add_argument('--pelvis', type=float, default=330.0, help='pelvis height over the floor')
    p.add_argument('--speed', type=float, default=200.0, help='units per second')
    p.add_argument('--step', type=float, default=150.0)
    p.add_argument('--frames', type=int, default=8)
    p.add_argument('--host-pack', action='store_true',
                   help='write OUTPUT as the runtime crouch-walk pack (ttk-anim.pack) instead of clip JSON files')
    p.add_argument('--set', action='append', default=[], metavar='KEY=VALUE',
                   help='tactical crouch-walk parameter (TACTICAL), e.g. --set hip_ratio=0.62; for variant packs '
                        'tried with DNTTK_ANIM_PACK')
    a = p.parse_args()
    for kv in a.set:
        key, value = kv.split('=', 1)
        if key not in TACTICAL:
            p.error(f'unknown parameter {key}; one of {", ".join(TACTICAL)}')
        TACTICAL[key] = float(value)
    group = t.Group(a.group)
    if a.host_pack:
        a.output.parent.mkdir(parents=True, exist_ok=True)
        print(a.output, write_host_pack(group, a.output))
        return
    a.output.mkdir(parents=True, exist_ok=True)
    for name, (_, walk) in DIRECTIONS.items():
        frames = build(group, name, a.pelvis, a.speed, a.frames, step=a.step)
        parts = [(name, walk, frames)]
        if name in HALVES:
            half = len(frames) // 2
            parts = [(f'{name}-{i + 1}', anim, split_half(frames, i * half, half)) for i, anim in enumerate(HALVES[name])]
        for label, anim, part in parts:
            clip_id = group.tracks[anim][0]
            doc = dict(format='ttk-anim-clip', version=1, group=a.group, clip=clip_id, direction=label,
                       source_sha256=t.clip_digest(group, clip_id), replaces_walk=anim,
                       pelvis=a.pelvis, speed=a.speed, step=a.step, frames=part)
            path = a.output / f'crouch-{label}.json'
            path.write_text(json.dumps(doc, indent=1))
            print(f'{path}: {len(part)} frames x {part[0]["ticks"]} ticks, events {[f["events"] for f in part]}')


if __name__ == '__main__':
    main()
