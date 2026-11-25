"""D61 level atlas reader (tools/local/level_atlas.py) on a synthetic level:
no game, no retail data. Rooms, portals, a floor, objects, a spawner, a
loaded model nothing uses, names, the plan and the code-reference scan."""
import pathlib
import struct
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/local'))
import level_atlas as la  # noqa: E402


class Mem:
    def __init__(self):
        self.m = bytearray(la.SIZE)

    def w(self, a, v):
        struct.pack_into('<i' if v < 0 else '<I', self.m, a - la.BASE, v)

    def h(self, a, v):
        struct.pack_into('<h' if v < 0 else '<H', self.m, a - la.BASE, v)

    def b(self, a, v):
        self.m[a - la.BASE] = v

    def s(self, a, text):
        self.m[a - la.BASE:a - la.BASE + len(text) + 1] = text.encode() + b'\0'


TYPES, MODELS, ROOMS, PIECE, STR, OBJ, SPAWN = (0x80110000, 0x80118000, 0x80120000, 0x80130000,
                                               0x80140000, 0x80150000, 0x80160000)


def level():
    m = Mem()
    m.w(la.LEVEL_INDEX, 3)
    for k in range(100):                       # string ids 0-99, id 92 the level's name
        m.w(la.STRINGS + 4 * k, STR + 32 * k)
        m.s(STR + 32 * k, 'GOLD AND GUNS' if k == 92 else f'S{k}')
    m.h(la.NAME_IDS + 2 * 3, 92)
    m.w(la.PLAYER + 4, 1000), m.w(la.PLAYER + 8, -2546), m.w(la.PLAYER + 12, 1000)
    m.w(la.PLAYER + 0x1c8, -2048), m.b(la.PLAYER + 0x2e, 0)
    # Types: 900 a lock, 901 a pickup with model 5, 902 an unused type with model 6.
    m.w(la.TYPES, TYPES), m.w(la.MODELS, MODELS)
    m.w(TYPES + 28 * 900, la.LOCK_FLAG)
    m.w(TYPES + 28 * 901, 2), m.h(TYPES + 28 * 901 + 16, 5)
    m.w(TYPES + 28 * 902, 2), m.h(TYPES + 28 * 902 + 16, 6)
    m.w(MODELS + 4 * 5, 0x80170000), m.w(MODELS + 4 * 6, 0x80170100)
    # Three rooms 4096 wide: 0 and 1 joined by a portal, 2 an island.
    m.w(la.ROOMS, ROOMS), m.w(la.ROOM_COUNT, 3)
    for k in range(3):
        a = ROOMS + 88 * k
        m.w(a, 4096 * k), m.w(a + 4, -4096), m.w(a + 8, 0)
        m.w(a + 0x20, 4096 * k + 4096), m.w(a + 0x24, 0), m.w(a + 0x28, 4096)
        # One piece: a floor quad 2048 below the room's top (y byte 16 << 7).
        piece = PIECE + 0x100 * k
        m.b(a + 0x32, 1), m.w(a + 0x40, PIECE + 0x1000 + 4 * k), m.w(PIECE + 0x1000 + 4 * k, piece)
        m.h(piece + 0x14, 4), m.w(piece + 0x18, 0x1c + 32)
        for j, (x, z) in enumerate([(0, 0), (4, 0), (0, 4), (4, 4)]):
            m.w(piece + 0x1c + 8 * j, x | (16 << 8) | (z << 16))
        m.w(piece + 0x3c, 0x61 | (1 << 16)), m.w(piece + 0x40, 0x03020100), m.w(piece + 0x48, 0xff)
    # Portal 0 -> 1 on the shared wall (room-relative corners in memory).
    m.b(ROOMS + 0x30, 1), m.w(ROOMS + 0x38, PIECE + 0x2000)
    for j in range(4):
        m.h(PIECE + 0x2000 + 8 * j, 4096)
    m.h(PIECE + 0x2000 + 14, 1)
    # Objects in room 0: a lock, a pickup and one outside every room.
    nodes = PIECE + 0x3000
    objs = [(900, (500, -2048, 500)), (901, (1500, -2048, 1500)), (901, (90000, 0, 90000))]
    for k, (t, pos) in enumerate(objs):
        o = OBJ + 0x60 * k
        m.h(o + 0x2c, t)
        for i, v in enumerate(pos):
            m.w(o + 4 + 4 * i, v)
        m.w(nodes + 16 * k + 8, o)
        m.w(nodes + 16 * k + 4, nodes + 16 * (k + 1) if k + 1 < len(objs) else 0)
    m.w(ROOMS + 0x48, nodes)
    m.w(la.LEVEL_OBJECTS + 4 * 2, OBJ)
    # One spawner of type 57 in room 1, and an empty record.
    m.w(la.SPAWNERS, SPAWN), m.w(la.SPAWNER_COUNT, 2)
    m.w(SPAWN, 57), m.w(SPAWN + 8, 6000), m.w(SPAWN + 12, -2048), m.w(SPAWN + 16, 2000)
    # Code: li a0, 902 in the overlay; li a0, 901 in the EXE.
    m.w(la.OVERLAY + 0x100, (0x09 << 26) | (4 << 16) | 902)
    m.w(0x80020000, (0x0d << 26) | (4 << 16) | 901)
    return bytes(m.m)


class LevelAtlasTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = level()
        cls.a = la.read_level(cls.data)

    def test_level_and_rooms(self):
        a = self.a
        self.assertEqual((a['level'], a['name'], a['duke_room']), (3, 'GOLD AND GUNS', 0))
        self.assertEqual(len(a['rooms']), 3)
        self.assertEqual(a['rooms'][1]['min'], [4096, -4096, 0])
        self.assertEqual(a['rooms'][0]['faces'], 2)

    def test_portals_are_absolute_and_reachability(self):
        p = self.a['rooms'][0]['portals'][0]
        self.assertEqual(p['to'], 1)
        self.assertEqual(p['corners'][0][0], 4096)          # room 0 origin 0 + 4096
        self.assertEqual(self.a['reach']['joined'], [0, 1])
        self.assertEqual(self.a['reach']['islands'], [[2]])

    def test_standing_point_on_the_floor(self):
        st = self.a['rooms'][1]['stand']
        self.assertTrue(st)
        x, y, z = st[0]['pos']
        self.assertEqual(y, -2048)
        self.assertEqual(st[0]['headroom'], 2048)
        self.assertTrue(4096 <= x <= 8192 and 0 <= z <= 4096)

    def test_objects_spawners_and_flags(self):
        obs = self.a['objects']
        self.assertEqual([o['type'] for o in obs], [900, 901, 901])
        self.assertTrue(obs[0]['lock'])
        self.assertEqual(obs[0]['table'], 2)
        self.assertEqual([o['outside'] for o in obs], [False, False, True])
        self.assertEqual(len(self.a['spawners']), 1)        # the empty record is skipped
        self.assertEqual(self.a['spawners'][0]['room'], 1)
        kinds = {f['kind'] for f in self.a['flags']}
        self.assertIn('island', kinds)
        self.assertIn('outside', kinds)
        self.assertIn('lock-without-key', kinds)

    def test_unused_model_and_code_refs(self):
        self.assertEqual(self.a['models'], {5: [901], 6: [902]})
        self.assertEqual(self.a['used_models'], [5])
        refs = la.code_refs(self.data, [901, 902, 903])
        self.assertEqual(refs[902], {'exe': 0, 'overlay': 1})
        self.assertEqual(refs[901], {'exe': 1, 'overlay': 0})
        self.assertEqual(refs[903], {'exe': 0, 'overlay': 0})
        atl = {3: dict(self.a, flags=list(self.a['flags']))}
        la.cross_flags(atl, dumps={3: self.data})
        unused = [f for f in atl[3]['flags'] if f['kind'].startswith('unused-model')]
        self.assertEqual([(f['model'], f['kind']) for f in unused], [(6, 'unused-model')])

    def test_plan_and_lamps(self):
        svg = la.plan_svg(self.a)
        self.assertTrue(svg.startswith('<svg') and 'GOLD AND GUNS' in svg)
        with tempfile.TemporaryDirectory() as d:
            p = pathlib.Path(d) / 'level03.lights'
            p.write_text('# x y z\n100 -200 300  1.0 0.5 0.5  4000  0.60  steady  # a lamp\nbad line\n')
            self.assertEqual(la.read_lamps(p)[0]['pos'], [100, -200, 300])
            self.assertEqual(len(la.read_lamps(p)), 1)


if __name__ == '__main__':
    unittest.main()
