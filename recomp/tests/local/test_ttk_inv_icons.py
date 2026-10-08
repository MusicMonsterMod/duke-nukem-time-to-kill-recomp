import struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE, EXE, PACK = ROOT / 'disc/time-to-kill.bin', ROOT / 'disc/SLUS_005.83', ROOT / 'assets/ttk-inv-icons.pack'
FRAME = ROOT / 'assets/ui/item-frame.png'


def entries(data):
    assert data[:8] == b'TTKICO2\0'
    count, offset, out = struct.unpack_from('<I', data, 8)[0], 12, []
    for _ in range(count):
        kind, item, tile, w, h = struct.unpack_from('<HHHHH', data, offset)
        out.append((kind, item, w, h, data[offset:offset + 10 + w * h * 4]))
        offset += 10 + w * h * 4
    assert offset == len(data)
    return out


@unittest.skipUnless(IMAGE.exists() and EXE.exists(), 'prepared owned disc required')
class InventoryIconBuilder(unittest.TestCase):
    def test_rebuild_is_reproducible_with_project_frame(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'icons.pack'
            for _ in range(2):
                subprocess.run([sys.executable, str(ROOT / 'tools/local/build_ttk_inv_icons.py'), '--output', str(out),
                                '--png-dir', tmp], check=True, capture_output=True)
                first = out.read_bytes() if _ == 0 else first
            self.assertEqual(out.read_bytes(), first)
            built = entries(first)
            self.assertEqual([(k, i) for k, i, *_ in built], [(0, 1), (0, 2), (0, 3), (0, 4), (0, 5), (1, 0)])
            for kind, item, w, h, _ in built[:5]:
                # D08A21: the 14x14 steroids pill trims to 6x14.
                self.assertTrue(6 <= w <= 16 and 6 <= h <= 16, (item, w, h))
            # The frame is the project's own item-frame.png, pixel for pixel.
            sys.path.insert(0, str(ROOT / 'tools/local'))
            import build_ttk_inv_icons as builder
            w, h, rgba = builder.read_rgba_png(FRAME)
            self.assertEqual((built[5][2], built[5][3]), (w, h))
            self.assertEqual(built[5][4][10:], rgba)
            # D08A4: steroids (item 4) are the project's own hud-steroids.png, trimmed.
            w, h, rgba = builder.read_rgba_png(ROOT / 'assets/ui/items/hud-steroids.png')
            icon = builder.trim([[tuple(rgba[(y * w + x) * 4:(y * w + x) * 4 + 4]) for x in range(w)] for y in range(h)])
            self.assertEqual(built[3][4][10:], b''.join(bytes(p) for r in icon for p in r))
            for name in ('jetpack', 'biomask', 'goggles', 'medkit', 'armor'):
                self.assertTrue((Path(tmp) / f'{name}.png').exists())


if __name__ == '__main__':
    unittest.main()
