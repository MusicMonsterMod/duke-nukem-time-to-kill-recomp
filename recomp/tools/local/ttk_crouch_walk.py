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
# D08K4 revision 3: the motion of a captured crouch walk (CC-BY 4.0, see
# recomp/assets/anim/crouch-walk-mocap.json, written by extract_crouch_mocap.py)
# retargeted onto Duke: his pelvis at PELVIS with the capture's bob and torso
# lean, each ankle on the capture's path (relative to the hips, scaled by leg
# length, the stride scaled so a cycle travels exactly CYCLE), feet rolling as
# in the capture, solved with the same two-bone IK.
MOCAP = Path(__file__).resolve().parents[2] / 'assets/anim/crouch-walk-mocap.json'
CYCLE = 640.0
# Strafes: quicker, smaller side steps (a 640 sideways stride reads as a split).
STRAFE_CYCLE = 380.0


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


def mocap_frames(group, direction, pelvis_height=None, width=None, idle=False):
    pelvis_height = PELVIS if pelvis_height is None else pelvis_height
    width = WIDTH if width is None else width
    mc = json.loads(MOCAP.read_text())
    src = resample_by_travel(mc)
    h = group.hierarchies[t.DUKE_HIERARCHY]
    idle_q, idle_roots = group.animation_quats(IDLE, 0)
    idle_pose = t.pose(h, idle_q, root_position=t.root_position(h, idle_roots))
    legs = [Leg(group, r) for r in LEGS]
    leg_len = float(np.linalg.norm(legs[0].thigh) + np.linalg.norm(legs[0].shin))
    ankle_world = [idle_pose[l.records[2]][0] for l in legs]
    toe = {l.rot[3]: idle_q.get(l.rot[3], (0, 0, 0, 0)) for l in legs}
    # Capture sides: legs[0] is Duke's negative-x leg.
    sides = ('left', 'right') if mc['left_is_game_negative_x'] else ('right', 'left')
    neutral_x = [idle_pose[l.records[2]][1][0] * width for l in legs]
    mean_hips = float(np.mean([f['hips_height'] for f in src]))
    rel = {sd: np.array([f[sd + '_ankle'] for f in src]) * leg_len for sd in sides}
    mean_z = {sd: float(rel[sd][:, 2].mean()) for sd in sides}
    # Stride: the planted (lower) foot's travel per cycle, scaled to CYCLE.
    n = len(src)
    travel = 0.0
    for k in range(n):
        a, b = src[k], src[(k + 1) % n]
        sd = min(sides, key=lambda x: a[x + '_ankle_height'] + b[x + '_ankle_height'])
        travel += abs(b[sd + '_ankle'][2] - a[sd + '_ankle'][2]) * leg_len
    stride = (STRAFE_CYCLE if direction.startswith('strafe') else CYCLE) / travel
    order = list(range(n))
    if direction == 'backward':
        order = order[::-1]
    out = []
    for k in order:
        f = src[k]
        torso = np.array(f['torso'])
        lean = math.atan2(torso[2], -torso[1])          # forward pitch, lateral twist dropped
        pelvis_m = axis_angle([1.0, 0.0, 0.0], -lean)
        bob = 0.0 if idle else (f['hips_height'] - mean_hips) * leg_len
        pelvis_t = np.array([0.0, -(pelvis_height + bob), 0.0])
        # Host clips carry the pelvis height over the feet (absolute, x = z = 0)
        # in a root translation command; crouch_walk.inc reads it.
        cmds = [dict(joint=0, root=[0, int(round(pelvis_height + bob)), 0], flags=2), dict(joint=0, q=game_q(pelvis_m), flags=0)]
        for i, leg in enumerate(legs):
            sd = sides[i]
            dz = 0.0 if idle else (rel[sd][k][2] - mean_z[sd]) * stride
            along = np.array([0.0, 0.0, mean_z[sd] + dz])
            if direction == 'strafe_pos_x':
                along = np.array([dz, 0.0, mean_z[sd]])
            elif direction == 'strafe_neg_x':
                along = np.array([-dz, 0.0, mean_z[sd]])
            height = 0.0 if idle else f[sd + '_ankle_height'] * leg_len
            target = np.array([neutral_x[i] + along[0], -(ANKLE_HEIGHT + height), along[2]])
            foot = np.eye(3) if idle else np.array(f[sd + '_foot_rotation'])
            foot_world = foot @ ankle_world[i]
            # Duke's foot is longer than the capture's (leg-relative): lift the
            # ankle so a pitched-down toe stays on the floor.
            toe_y = target[1] + (foot_world @ leg.toe)[1]
            if toe_y > 0:
                target[1] -= toe_y
            hip_l, knee_l, ankle_l, _ = leg.solve(pelvis_m, pelvis_t, target, foot_world, np.array([0.0, 0.0, 1.0]))
            cmds += [dict(joint=leg.rot[0], q=game_q(hip_l), flags=0), dict(joint=leg.rot[1], q=game_q(knee_l), flags=0),
                     dict(joint=leg.rot[2], q=game_q(ankle_l), flags=0)]
            if toe[leg.rot[3]] != (0, 0, 0, 0):
                cmds.append(dict(joint=leg.rot[3], q=list(toe[leg.rot[3]]), flags=0))
        out.append(dict(events=0, ticks=HOST_TICKS, commands=cmds))
    return out[:2] if idle else out


# D08K4 host-driven clips: the crouch walk's channel 0 is set by the host every
# update (crouch_walk.inc), so the frames are long (the original stepper never
# advances them) and carry no root commands (the host moves Duke and sets the
# pelvis height: PELVIS + BOB cos(4 pi phase)). Pack entries with group 0xffff
# are extra clips: appended, never bound to an original clip id.
HOST_CLIPS = {1: 'forward', 2: 'backward', 3: 'strafe_pos_x', 4: 'strafe_neg_x', 5: 'idle'}
HOST_TICKS = 30000
# The Duke3D-like crouch (user reference 2026-10-22): low, hunched forward,
# long strides at the walking pace. The host mirrors PELVIS/BOB and the cycle
# travel STEP / STANCE (crouch_walk.inc).
PELVIS, BOB, LEAN, STEP, STANCE, LIFT, WIDTH, FRAMES = 300.0, 10.0, 25.0, 240.0, 0.55, 45.0, 0.95, 12


def host_clip(group, direction):
    if MOCAP.exists():        # revision 3: the captured crouch walk
        return host_frames(mocap_frames(group, direction if direction != 'idle' else 'forward', idle=direction == 'idle'), keep_root=True)
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
    a = p.parse_args()
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
