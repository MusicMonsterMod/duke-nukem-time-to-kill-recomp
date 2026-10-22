"""D08K1 animation pipeline: format round trips, pose math and pack layout."""
import json, math, struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE, EXE = ROOT / 'disc/time-to-kill.bin', ROOT / 'disc/SLUS_005.83'
sys.path.insert(0, str(ROOT / 'tools/local'))
import ttk_anim as t


class Encoding(unittest.TestCase):
    def test_command_round_trip(self):
        for cmd in (dict(joint=9, q=[71, 0, 0, 4095], flags=0), dict(joint=31, q=[-8192, 8191, -1, -4096], flags=0),
                    dict(joint=0, root=[-5, 279, -8192], flags=2), dict(joint=3, q=[1, 2, 3, 4], flags=0xd)):
            self.assertEqual(t.decode_command(*t.encode_command(cmd)), cmd)

    def test_range_checked(self):
        with self.assertRaises(ValueError):
            t.encode_command(dict(joint=1, q=[0, 0, 0, 4096]))

    def test_quaternion_matrix_matches_game_layout(self):
        # 0x8002a5ac stores m01 = 2(xy + wz): the transpose of the textbook matrix.
        m = t.quat_matrix([0, 0, 2896, 2896])  # 90 degrees about z
        self.assertAlmostEqual(m[0, 1], 1.0, places=3)
        self.assertAlmostEqual(m[1, 0], -1.0, places=3)
        self.assertTrue((t.quat_matrix([0, 0, 0, 0]) == t.quat_matrix([0, 0, 0, 4096])).all())

    def test_blender_conversion_round_trip(self):
        for q in ([71, 0, 0, 4095], [-2843, 128, -744, 2849], [3227, 0, 0, 2521], [0, 0, 0, 4095]):
            back = t.blender_to_quat(t.quat_to_blender(q))
            self.assertTrue(t.same_rotation(back, q), (q, back))
            self.assertTrue(back[3] <= 4095)

    def test_pack_layout(self):
        clip = dict(group=0, clip=7, source_sha256='ab' * 32,
                    frames=[dict(events=1, ticks=20, commands=[dict(joint=1, q=[1, 2, 3, 4000], flags=0)])])
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'p.pack'
            t.build_pack([clip], path)
            data = path.read_bytes()
        self.assertEqual(data[:8], b'TTKANIM1')
        self.assertEqual(struct.unpack_from('<IHH', data, 8), (1, 0, 7))
        self.assertEqual(data[16:48], bytes.fromhex('ab' * 32))
        self.assertEqual(struct.unpack_from('<HH4H', data, 48), (1, 0, 1, 20, 1, 0))
        self.assertEqual(len(data), 60 + 8)


@unittest.skipUnless(IMAGE.exists() and EXE.exists(), 'prepared owned disc required')
class OwnedDisc(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.g = t.Group(0)

    def test_every_clip_re_encodes_byte_exact(self):
        r = subprocess.run([sys.executable, str(ROOT / 'tools/local/ttk_anim.py'), 'roundtrip'],
                           capture_output=True, text=True)
        self.assertEqual(r.returncode, 0, r.stdout[-500:])
        self.assertIn(' 0 mismatches', r.stdout)

    def test_duke_hierarchy(self):
        h = self.g.hierarchies[t.DUKE_HIERARCHY]
        self.assertEqual(h['count'], 19)
        # ROT joint ids come from the map (the loader writes record byte 1).
        self.assertEqual([j['rot_id'] for j in h['joints']],
                         [0, 9, 10, 11, 12, 13, 14, 15, 20, 16, 1, 2, 3, 4, 5, 6, 7, 8, 21])
        self.assertEqual(h['joints'][0]['offset'], [0, -561, 0])

    def test_duke_models_parse_textured(self):
        for joint in self.g.hierarchies[t.DUKE_HIERARCHY]['joints']:
            m = self.g.model(joint['model'])
            self.assertTrue(m['prims'])
            for p in m['prims']:
                self.assertIn(p['code'], (0x2c, 0x24))
                self.assertTrue(all(i < len(m['vertices']) for i in p['idx']))

    def test_tracks_are_body_channels(self):
        # Anim 178 (stationary crouch) has one clip per channel; anim 8 drives only the right arm.
        self.assertEqual(self.g.tracks[178], (105, 106, 107, 108))
        self.assertEqual(self.g.tracks[8], (-1, 472, -1, -1))
        legs = {c['joint'] for f in self.g.clip(105)['frames'] for c in f['commands']}
        self.assertTrue({0, 1, 2, 3, 5, 6, 7} <= legs)

    def test_crouch_root_continues_from_transition(self):
        enter = t.cumulative_root(self.g.clip(109)['frames'])[1]
        start = t.cumulative_root(self.g.clip(105)['frames'][:1])[1]
        self.assertLess(abs(enter - start), 4)

    def test_crouch_walk_gait(self):
        # D08K3: four directions, planted feet (the original walks slide 23-40
        # units a step under the same metric), feet on the floor, under the hull.
        import ttk_crouch_walk as cw
        for name in cw.DIRECTIONS:
            frames = cw.build(self.g, name)
            self.assertEqual(len(frames), 8)
            self.assertEqual([f['events'] & 6 for f in frames][0::4], [4, 2])
            self.assertEqual(frames[-1]['events'] & 1, 1)
            m = cw.metrics(self.g, frames)
            self.assertLess(m['slide_max'], 4.0, (name, m))
            self.assertLess(abs(m['ground_min']), 10.0, (name, m))
            self.assertLess(m['top'], 0x2c0, (name, m))
            for f in frames:
                for c in f['commands']:
                    t.encode_command(c)    # every value fits the ROT fields
        halves = [cw.split_half(cw.build(self.g, 'forward'), i * 4, 4) for i in (0, 1)]
        self.assertEqual([f['events'] for f in halves[0]], [4, 0, 0, 1])
        self.assertEqual([f['events'] for f in halves[1]], [2, 0, 0, 1])
        self.assertEqual(halves[1][0]['commands'][0]['root'][1], halves[0][0]['commands'][0]['root'][1])

    def test_mocap_crouch_walk(self):
        # D08K4 revision 3: the captured crouch walk retargeted onto Duke: the
        # planted ankle stays put once the cycle's travel is applied, the feet
        # reach the floor, every value encodes, and the host pack has the
        # sentinel, four walks, the idle and the padding.
        import ttk_crouch_walk as cw
        if not cw.MOCAP.exists():
            self.skipTest('no motion asset')
        mc = json.loads(cw.MOCAP.read_text())
        self.assertIn('CC-BY-4.0', mc['license'])
        h = self.g.hierarchies[t.DUKE_HIERARCHY]
        frames = cw.mocap_frames(self.g, 'forward')
        n = len(frames)
        heights = [next(c['root'][1] for c in f['commands'] if 'root' in c) for f in frames]
        for f in frames:
            for c in f['commands']:
                t.encode_command(c)
        ankles = {12: [], 16: []}
        for k, f in enumerate(frames):
            mats = t.pose(h, t.joint_quats_for_frame(f), root_position=(0, -heights[k], k * cw.CYCLE / n))
            for r in ankles:
                ankles[r].append(mats[r][1])
        for pts in ankles.values():
            low = [k for k in range(n - 1) if -pts[k][1] < 45 and -pts[k + 1][1] < 45]
            self.assertTrue(low)
            for k in low:
                self.assertLess(math.hypot(pts[k + 1][0] - pts[k][0], pts[k + 1][2] - pts[k][2]), 3.0)
            self.assertLess(abs(min(-p[1] for p in pts) - cw.ANKLE_HEIGHT), 6.0)
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'anim.pack'
            cw.write_host_pack(self.g, path)
            data = path.read_bytes()
        self.assertEqual(struct.unpack_from('<I', data, 8)[0], 11)  # 4 walks, idle, their 5 upper clips, padding

    def test_tactical_crouch_posture(self):
        # D08K6: not "sitting": hip joints well above knee height, feet near
        # hip width, chest leaning over the knees, everything under the hull,
        # and the upper body encodes and stays separate from the legs.
        import ttk_crouch_walk as cw
        if not cw.MOCAP.exists():
            self.skipTest('no motion asset')
        h = self.g.hierarchies[t.DUKE_HIERARCHY]
        for name in ('forward', 'backward', 'strafe_pos_x', 'strafe_neg_x', 'idle'):
            frames = cw.mocap_frames(self.g, name)
            self.assertLess(cw.pose_top(self.g, frames), 0x2c0, name)
            drops = []
            for f in frames:
                self.assertEqual(set(f['upper']), {9, 16, 10, 11, 12, 13, 14, 15})
                self.assertFalse({c['joint'] for c in f['commands'] if 'q' in c} & set(f['upper']))
                for j, q in f['upper'].items():
                    t.encode_command(dict(joint=j, q=q, flags=0))
                q = t.joint_quats_for_frame(f)
                q.update(f['upper'])
                m = t.pose(h, q, root_position=(0, -f['commands'][0]['root'][1], 0))
                drops += [m[knee][1][1] - m[hip][1][1] for hip, knee in ((10, 11), (14, 15))]
                if name != 'strafe_pos_x' and name != 'strafe_neg_x':
                    self.assertLess(abs(m[12][1][0] - m[16][1][0]), 180, name)  # feet near hip width
                lean = m[9][1] - m[1][1]
                self.assertGreater(math.degrees(math.atan2(lean[2], -lean[1])), 45, name)
            # Thighs pitched down on average: D08K4's were level (about 20);
            # the user's chosen low crouch (variant C, hip ratio 0.56) is about 48.
            self.assertGreater(sum(drops) / len(drops), 40, name)

    def test_stationary_kneel(self):
        import ttk_crouch_walk as cw
        original, _ = self.g.animation_quats(178, 0)
        legs = cw.host_clip(self.g, 'idle')[1]
        upper = cw.host_clip(self.g, 'idle', upper=True)[1]
        lq, uq = t.joint_quats_for_frame(legs), t.joint_quats_for_frame(upper)
        self.assertFalse(set(lq) & set(uq))
        for j, q in {**lq, **uq}.items():
            self.assertEqual(tuple(q), tuple(original.get(j, (0, 0, 0, 4095))))
        self.assertEqual(legs['commands'][0]['root'], [0, 282, 0])
        for f in (legs, upper):
            for command in f['commands']:
                a, b = t.encode_command(command)
                self.assertEqual('root' in command, 'root' in t.decode_command(a, b))

    def test_clip_digest_stable(self):
        self.assertEqual(t.clip_digest(self.g, 0), t.clip_digest(self.g, 0))
        self.assertNotEqual(t.clip_digest(self.g, 0), t.clip_digest(self.g, 1))


if __name__ == '__main__':
    unittest.main()
