from pathlib import Path
import hashlib
import struct
import subprocess
import sys
import unittest
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/local'))
from level_overlay_guards import level_guard


def overlay(store_offset):
    # tag; lui v0,0x800d; addiu a1,v0,<data>; sw zero,<store>(v0); jr ra; nop;
    # then an 8-byte data block at 0x18 whose last word the code writes.
    words = [5, 0x3c02800d, (9 << 26) | (2 << 21) | (5 << 16) | 0xa980,
             (0x2b << 26) | (2 << 21) | ((0x800ca968 + store_offset) & 0xffff), 0x03e00008, 0, 0x1234, 0]
    return struct.pack('<8I', *words)


class LevelOverlayGuardsTest(unittest.TestCase):
    def test_trailing_store_is_scratch(self):
        data = overlay(0x1c)
        row = level_guard(5, data)
        self.assertEqual((row['code_end'], row['body'], row['scratch']), (0x18, 0x1c, 4))
        self.assertEqual(row['digest'], hashlib.sha256(data[:0x1c]).hexdigest())

    def test_store_inside_code_refused(self):
        with self.assertRaises(SystemExit):
            level_guard(5, overlay(0x4))

    def test_unwritten_word_after_scratch_refused(self):
        # A write to the first data word leaves the last one unwritten.
        with self.assertRaises(SystemExit):
            level_guard(5, overlay(0x18))

    @unittest.skipUnless((ROOT / 'disc/time-to-kill.bin').exists(), 'owned disc not imported')
    def test_control_guards_match_owned_disc(self):
        subprocess.run([sys.executable, str(ROOT / 'tools/local/level_overlay_guards.py'), '--check'],
                       check=True, capture_output=True)


if __name__ == '__main__':
    unittest.main()
