import re, struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
UI = ROOT / 'assets/ui'
try:
    from PIL import Image
except ImportError:  # pragma: no cover
    Image = None


@unittest.skipUnless(Image, 'Pillow required')
class ProjectUiArt(unittest.TestCase):
    """The original UI art in assets/ui is what the game uses (D24A)."""

    def test_crosshair_source_matches_png(self):
        source = (ROOT / 'src/ttk/weapon_aim.cpp').read_text()
        table = re.search(r'k_crosshair\[81\]=\{(.*?)\};', source, re.S).group(1)
        compiled = [int(v, 16) for v in re.findall(r'0x([0-9a-fA-F]{8})u', table)]
        im = Image.open(UI / 'crosshair.png').convert('RGBA')
        self.assertEqual(im.size, (9, 9))
        expected = [((a << 24) | (r << 16) | (g << 8) | b) if a else 0 for r, g, b, a in im.getdata()]
        self.assertEqual(compiled, expected)

    def test_medkit_hud_cell_matches_png(self):
        # D08A7: k_medkit_cell / k_medkit_clut are gadget-medkit-15col.png as a 4bpp
        # cell: index 0 transparent, then colours (PSX 15-bit) in raster order.
        self.check_hud_cell('medkit', 'gadget-medkit-15col.png')

    def test_steroids_hud_cell_matches_png(self):
        # D08A4: the steroids box icon, reduced by tools/local/reduce_icon_15col.py.
        self.check_hud_cell('steroids', 'hud-steroids-15col.png')

    def test_powerup_hud_cells_match_png(self):
        # D08A17: the power-up coin boxes, the user's D08A16 art.
        for name, png in [('invincibility', 'hud-invincibility-15col.png'), ('invisibility', 'hud-invisibility-15col.png'),
                          ('double_duke', 'hud-double-duke-15col.png')]:
            with self.subTest(name=name):
                self.check_hud_cell(name, png)

    def test_hud_reductions_are_reproducible(self):
        sys.path.insert(0, str(ROOT / 'tools/local'))
        from reduce_icon_15col import reduce
        for name in ('steroids', 'invincibility', 'invisibility', 'double-duke'):
            with self.subTest(name=name):
                made = reduce(Image.open(UI / f'items/hud-{name}.png'))
                self.assertEqual(list(made.getdata()),
                                 list(Image.open(UI / f'items/hud-{name}-15col.png').convert('RGBA').getdata()))

    def check_hud_cell(self, name, png):
        source = (ROOT / 'src/ttk/gadget_hud.inc').read_text()
        def table(name):
            return [int(v, 16) for v in re.findall(r'0x([0-9a-fA-F]+)u', re.search(name + r'\[\d+\]=\{(.*?)\};', source, re.S).group(1))]
        im = Image.open(UI / 'items' / png)
        self.assertEqual((im.mode, im.size), ('RGBA', (16, 16)))
        palette, texels = [0], []
        for r, g, b, a in im.getdata():
            if not a:
                texels.append(0)
                continue
            self.assertEqual(a, 255)
            colour = (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10 or 0x8000
            self.assertEqual(((r >> 3) << 3 | r >> 5, (g >> 3) << 3 | g >> 5, (b >> 3) << 3 | b >> 5), (r, g, b))
            if colour not in palette:
                palette.append(colour)
            texels.append(palette.index(colour))
        self.assertLessEqual(len(palette), 16)
        self.assertEqual(table(f'k_{name}_clut'), palette + [0] * (16 - len(palette)))
        cell = [sum(texels[y * 16 + half * 8 + i] << 4 * i for i in range(8)) for y in range(16) for half in range(2)]
        self.assertEqual(table(f'k_{name}_cell'), cell)

    def test_switcher_digits_come_from_microfont(self):
        sheet = Image.open(UI / 'fonts/microfont/3x5-Microfont_1D.png').convert('RGBA')
        self.assertTrue((UI / 'fonts/microfont/LICENSE').read_text().count('CC0 1.0 Universal'))
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'digits.pack'
            for _ in range(2):
                subprocess.run([sys.executable, str(ROOT / 'tools/local/build_ttk_inv_digits.py'), '--output', str(out)],
                               check=True, capture_output=True)
                first = out.read_bytes() if _ == 0 else first
            data = out.read_bytes()
        self.assertEqual(data, first)
        self.assertEqual(data[:8], b'TTKDIG3\0')
        self.assertEqual(struct.unpack_from('<I', data, 8)[0], 11)
        offset = 12
        for tile, ch in [(3010 + d, str(d)) for d in range(10)] + [(3076, '%')]:
            t, w, h = struct.unpack_from('<HHH', data, offset)
            self.assertEqual((t, w, h), (tile, 3, 5))
            pixels = data[offset + 6:offset + 6 + 60]
            for y in range(5):
                for x in range(3):
                    lit = sheet.getpixel((3 * ord(ch) + x, y))[3] >= 128
                    self.assertEqual(pixels[(y * 3 + x) * 4 + 3] == 255, lit, (ch, x, y))
            offset += 66
        self.assertEqual(offset, len(data))

    def test_item_icons_cover_every_design(self):
        import json
        manifest = json.loads((UI / 'items/items.json').read_text())
        names = {item['name'] for item in manifest['items']}
        expected = {'SUBWAY SECURITY KEY', 'TRANSPORT ROOM ID', 'WAREHOUSE KEY', 'GANTRY KEY', 'VALVE KEY', 'LAB KEY',
                    'VALVE ROOM KEY', 'RED ENERGY CRYSTAL', 'BLUE ENERGY CRYSTAL', 'GREEN ENERGY CRYSTAL', 'SKELETON KEY',
                    'SCRAP OF PAPER', 'OLD NOTE', 'TORN PAPER', 'FAMILY JEWEL', 'STEROIDS', 'MEDKIT',
                    'INVINCIBILITY', 'INVISIBILITY', 'DOUBLE DUKE'}
        self.assertEqual(names, expected)
        for item in manifest['items']:
            for key in ('file', 'hud_file'):
                if key in item:
                    im = Image.open(UI / 'items' / item[key])
                    self.assertEqual((im.mode, im.size), ('RGBA', (16, 16)), item[key])
        used = {item[key] for item in manifest['items'] for key in ('file', 'hud_file') if key in item}
        on_disk = {p.name for p in (UI / 'items').glob('*.png')} - {'mark-used.png'}  # D08A19 tick: read by the mission pack builder
        self.assertEqual(used, on_disk)

    def test_item_frame_is_rgba_25x23(self):
        im = Image.open(UI / 'item-frame.png')
        self.assertEqual((im.mode, im.size), ('RGBA', (25, 23)))


if __name__ == '__main__':
    unittest.main()
