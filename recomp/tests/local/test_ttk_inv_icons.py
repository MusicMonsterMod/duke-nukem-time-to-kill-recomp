import struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE, EXE, PACK = ROOT / 'disc/time-to-kill.bin', ROOT / 'disc/SLUS_005.83', ROOT / 'assets/ttk-inv-icons.pack'


def entries(data):
    assert data[:8] == b'TTKICO2\0'
    count, offset, out = struct.unpack_from('<I', data, 8)[0], 12, []
    for _ in range(count):
        kind, item, tile, w, h = struct.unpack_from('<HHHHH', data, offset)
        out.append((kind, item, w, h, data[offset:offset + 10 + w * h * 4]))
        offset += 10 + w * h * 4
    assert offset == len(data)
    return out


@unittest.skipUnless(IMAGE.exists() and EXE.exists() and PACK.exists(), 'prepared owned disc and pack required')
class InventoryIconBuilder(unittest.TestCase):
    def test_rebuild_is_reproducible_and_keeps_frame(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'icons.pack'
            for _ in range(2):
                subprocess.run([sys.executable, str(ROOT / 'tools/local/build_ttk_inv_icons.py'), '--output', str(out),
                                '--frame-from', str(PACK), '--png-dir', tmp], check=True, capture_output=True)
                first = out.read_bytes() if _ == 0 else first
            self.assertEqual(out.read_bytes(), first)
            built = entries(first)
            self.assertEqual([(k, i) for k, i, *_ in built], [(0, 1), (0, 2), (0, 3), (0, 5), (1, 0)])
            for kind, item, w, h, _ in built[:4]:
                self.assertTrue(8 <= w <= 16 and 8 <= h <= 16, (item, w, h))
            frame = [e for e in entries(PACK.read_bytes()) if e[0] == 1][0]
            self.assertEqual(built[4][4], frame[4])
            for name in ('jetpack', 'biomask', 'goggles', 'medkit', 'steroids'):
                self.assertTrue((Path(tmp) / f'{name}.png').exists())


if __name__ == '__main__':
    unittest.main()
