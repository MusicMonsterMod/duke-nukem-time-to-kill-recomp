import struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE, EXE = ROOT / 'disc/time-to-kill.bin', ROOT / 'disc/SLUS_005.83'
sys.path.insert(0, str(ROOT / 'tools/local'))


@unittest.skipUnless(IMAGE.exists() and EXE.exists(), 'prepared owned disc required')
class DiscFontPack(unittest.TestCase):
    """D24A: the Modernized text glyphs come from the player's disc only."""

    def build(self, tmp):
        out = Path(tmp) / 'fonts.pack'
        subprocess.run([sys.executable, str(ROOT / 'tools/local/build_ttk_fonts.py'), '--output', str(out)],
                       check=True, capture_output=True)
        return out.read_bytes()

    def test_reproducible_three_sets(self):
        with tempfile.TemporaryDirectory() as tmp:
            first, second = self.build(tmp), self.build(tmp)
        self.assertEqual(first, second)
        self.assertEqual(first[:8], b'TTKFONT2')
        size, digest = struct.unpack_from('<II', first, 8)
        payload = first[16:]
        self.assertEqual(size, len(payload))
        h = 2166136261
        for b in payload:
            h = ((h ^ b) * 16777619) & 0xffffffff
        self.assertEqual(h, digest)
        self.assertEqual(struct.unpack_from('<I', payload, 0)[0], 12)
        # line height, scale, shadow: messages, headings, console, then the D24B panel
        # title, slot, selected slot, text and dim text, then the D08A5 FOUND text
        # and the three 2x Microfont mission sets.
        expected = [(19, 1, 1), (19, 2, 0), (13, 1, 1), (19, 1, 1), (13, 1, 1), (13, 1, 1), (10, 1, 0), (10, 1, 0),
                    (10, 1, 0), (7, 2, 0), (7, 2, 0), (7, 2, 0)]
        for s, want in enumerate(expected):
            line, scale, shadow, _, colour = struct.unpack_from('<BBBBI', payload, 4 + s * 768)
            self.assertEqual((line, scale, shadow), want)
            if shadow:
                self.assertEqual(colour, 0xff0c0c18)
            # All caps: 'a' draws exactly as 'A'.
            rec = lambda c: struct.unpack_from('<BBBBI', payload, 4 + s * 768 + 8 + 8 * (ord(c) - 32))
            pix = lambda r: payload[r[4]:r[4] + r[0] * r[1] * 4]
            self.assertEqual(rec('a')[:4], rec('A')[:4])
            self.assertEqual(pix(rec('a')), pix(rec('A')))

    def test_ui_sprite_pack(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.build(tmp)
            ui = (Path(tmp) / 'ttk-ui.pack').read_bytes()
        self.assertEqual(ui[:8], b'TTKUI1\0\0')
        count, offset, ids = struct.unpack_from('<I', ui, 8)[0], 12, []
        for _ in range(count):
            ident, w, h = struct.unpack_from('<HHH', ui, offset)
            ids.append((ident, w, h)); offset += 6 + w * h * 4
        self.assertEqual(offset, len(ui))
        self.assertEqual(ids, [(1, 16, 16), (2, 16, 16), (3, 16, 16), (4, 16, 16), (5, 16, 16), (6, 96, 96)])

    def test_builder_never_reads_research(self):
        source = (ROOT / 'tools/local/build_ttk_fonts.py').read_text()
        self.assertNotIn('research', source.replace('no research', ''))


if __name__ == '__main__':
    unittest.main()
