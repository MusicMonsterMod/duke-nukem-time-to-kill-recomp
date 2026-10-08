import json, re, struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXE = ROOT / 'disc/SLUS_005.83'
sys.path.insert(0, str(ROOT / 'tools/local'))
import build_ttk_mission_items as mission  # noqa: E402


def entries(data):
    assert data[:8] == b'TTKMIS1\0'
    count, offset, out = struct.unpack_from('<I', data, 8)[0], 12, []
    for _ in range(count):
        kind, length = struct.unpack_from('<HH', data, offset); offset += 4
        name = data[offset:offset + length].decode('ascii'); offset += length
        w, h = struct.unpack_from('<HH', data, offset); offset += 4
        out.append((kind, name, w, h, data[offset:offset + w * h * 4])); offset += w * h * 4
    assert offset == len(data)
    return out


def table_from_source():
    source = (ROOT / 'src/ttk/inventory_hud.cpp').read_text()
    body = re.search(r'mission_table\[\]=\{(.*?)\};', source, re.S).group(1)
    return [(int(l), int(i), n) for l, i, n in re.findall(r'\{(\d+),(\d+),"([A-Z ]+)"\}', body)]


def original_select_names(exe):
    """Run the original Select inventory name function 0x80087d4c (item -> name
    pointer for the level at 0x800be570, or the 'none' string) for every level."""
    def word(a):
        return struct.unpack_from('<I', exe, a - 0x80010000 + 2048)[0]

    def string(a):
        o = a - 0x80010000 + 2048
        return exe[o:exe.index(b'\0', o)].decode('latin1')

    def call(item, level):
        r = [0] * 32; r[4] = item; pc, npc = 0x80087d4c, 0x80087d50
        for _ in range(500):
            ins = word(pc); op, rs, rt, rd = ins >> 26, (ins >> 21) & 31, (ins >> 16) & 31, (ins >> 11) & 31
            imm = ins & 0xffff; si = imm - 0x10000 if imm & 0x8000 else imm
            target = None
            if ins == 0: pass
            elif op == 0 and ins & 63 == 0: r[rd] = (r[rt] << ((ins >> 6) & 31)) & 0xffffffff
            elif op == 0 and ins & 63 == 0x21: r[rd] = (r[rs] + r[rt]) & 0xffffffff
            elif op == 0 and ins & 63 == 8:
                if rs == 31: return r[2]  # every return's delay slot is a nop
                target = r[rs]
            elif op == 2: target = (pc & 0xf0000000) | ((ins & 0x3ffffff) << 2)
            elif op == 4: target = pc + 4 + si * 4 if r[rs] == r[rt] else None
            elif op == 5: target = pc + 4 + si * 4 if r[rs] != r[rt] else None
            elif op == 9: r[rt] = (r[rs] + si) & 0xffffffff
            elif op == 0xa: r[rt] = int((r[rs] - (1 << 32) if r[rs] >> 31 else r[rs]) < si)
            elif op == 0xb: r[rt] = int(r[rs] < (si & 0xffffffff))
            elif op == 0xf: r[rt] = imm << 16
            elif op == 0x23:
                a = (r[rs] + si) & 0xffffffff
                r[rt] = level if a == 0x800be570 else word(a)
            else: raise AssertionError(f'unexpected instruction at {pc:#x}')
            r[0] = 0
            pc, npc = npc, target if target is not None else npc + 4
        raise AssertionError('no return')

    none = word(0x800c60f4)
    return [(level, item, string(p)) for level in range(32) for item in range(6, 17)
            if (p := call(item, level)) != none]


class MissionItems(unittest.TestCase):
    """D08A5: the mission row's data and art."""

    def test_pack_reproducible_and_complete(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'm.pack'
            for _ in range(2):
                subprocess.run([sys.executable, str(ROOT / 'tools/local/build_ttk_mission_items.py'), '--output', str(out)],
                               check=True, capture_output=True)
                first = out.read_bytes() if _ == 0 else first
            data = out.read_bytes()
        self.assertEqual(data, first)
        found = entries(data)
        manifest = json.loads((ROOT / 'assets/ui/items/items.json').read_text())
        names = {i['name'] for i in manifest['items'] if not i.get('hud') and not i.get('gadget')}
        self.assertEqual({n for k, n, *_ in found if k == 0}, names)
        self.assertEqual({n for k, n, *_ in found if k == 1}, names)
        frames = {k: px for k, n, w, h, px in found if 2 <= k <= 4}
        self.assertEqual(sorted(frames), [2, 3, 4])
        # D08A19: the user's used tick, as delivered (at most 16x16).
        ticks = [(w, h, px) for k, n, w, h, px in found if k == 5]
        self.assertEqual(len(ticks), 1)
        self.assertEqual(ticks[0], mission.read_rgba_png(ROOT / 'assets/ui/items/mark-used.png'))
        for kind, ramp in [(3, mission.RAMP_ORANGE), (4, mission.RAMP_STEEL)]:
            colours = {frames[kind][i:i + 3].hex() for i in range(0, len(frames[kind]), 4) if frames[kind][i + 3]}
            self.assertTrue(colours <= set(ramp), kind)
            self.assertGreater(len(colours), 3, kind)  # the bevel keeps several steps
        # Silhouettes are grey and dimmer than the icons.
        for kind, name, w, h, px in found:
            if kind == 1:
                self.assertTrue(all(px[i] == px[i + 1] == px[i + 2] and px[i] <= 160 for i in range(0, len(px), 4)), name)

    def test_every_table_name_has_an_icon(self):
        manifest = json.loads((ROOT / 'assets/ui/items/items.json').read_text())
        names = {i['name'] for i in manifest['items']}
        for level, item, name in table_from_source():
            self.assertIn(name, names)
            self.assertTrue(6 <= item <= 16)

    @unittest.skipUnless(EXE.exists(), 'prepared disc executable required')
    def test_table_is_the_original_select_inventory(self):
        self.assertEqual(table_from_source(), original_select_names(EXE.read_bytes()))


if __name__ == '__main__':
    unittest.main()
