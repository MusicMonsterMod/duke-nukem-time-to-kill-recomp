from pathlib import Path
import subprocess
import sys
import unittest
ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/local'))
from guard_writer_audit import guard_ranges, stores


class GuardWriterAuditTest(unittest.TestCase):
    def test_lui_in_branch_delay_slot_reaches_the_target(self):
        # The 0x800439e4 shape: beq ...; lui v0,0x800c (delay slot), then at the
        # target addiu v0,v0,0x2824; addu v1,v1,v0; sw v0,0(v1).
        words = [0x10400002, 0x3c02800c, 0, 0x24422824, 0x00621821, 0xac620000]
        self.assertEqual(list(stores(words, 0x80040000)), [(0x80040014, 'sw', 0x800c2824, True)])

    def test_exact_store(self):
        # lui v1,0x800c; sw v0,0x2b34(v1)
        self.assertEqual(list(stores([0x3c03800c, 0xac622b34], 0x80040000)),
                         [(0x80040004, 'sw', 0x800c2b34, False)])

    def test_load_clears_the_base(self):
        # lui v1,0x800c; lw v1,0(a0); sw v0,0x2b34(v1)
        self.assertEqual(list(stores([0x3c03800c, 0x8c830000, 0xac622b34], 0x80040000)), [])

    def test_state_guards_parse_with_masks(self):
        code, state = guard_ranges((ROOT / 'src/ttk/control_guards.inc').read_text())
        self.assertIn((0x800c2824, 1060, 0xffffffbf), state)
        self.assertFalse(any(a <= 0x800c2b34 < a + n for a, n in code))

    @unittest.skipUnless((ROOT / 'disc/time-to-kill.bin').exists(), 'owned disc not imported')
    def test_no_unreviewed_writer_on_the_owned_disc(self):
        subprocess.run([sys.executable, str(ROOT / 'tools/local/guard_writer_audit.py')],
                       check=True, capture_output=True)


if __name__ == '__main__':
    unittest.main()
