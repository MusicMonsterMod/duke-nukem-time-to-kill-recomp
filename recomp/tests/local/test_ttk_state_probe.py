import hashlib
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools/local'))
import ttk_state_probe as probe


class StateProbeTest(unittest.TestCase):
    def test_unsupported_executable_refused_before_debug_access(self):
        with patch.object(Path, 'read_bytes', return_value=b'wrong executable'):
            with self.assertRaisesRegex(ValueError, 'unsupported'):
                probe.sample(lambda *a, **kw: self.fail('debug access before identity validation'))

    def exercise(self, corrupt_code=False, corrupt_pointer=False, corrupt_overlay=False):
        exe = bytes(0xA0000)
        commands = []
        def call(command, **kwargs):
            commands.append(command)
            if command == 'frame':
                return {'frame': 100}
            self.assertEqual(command, 'read_ram')
            start = int(kwargs['addr'], 16)
            length = kwargs['len']
            data = bytearray(length)
            if start == probe.CAMERA:
                if not corrupt_pointer:
                    struct.pack_into('<I', data, 0xa4, probe.PLAYER)
                struct.pack_into('<I', data, probe.PLAYER-probe.CAMERA+0x7d4, probe.CAMERA)
            elif start == probe.LEVEL00_LOAD and corrupt_overlay:
                data[0] = 1
            elif corrupt_code:
                data[0] = 1
            return {'hex': data.hex()}
        with patch.object(Path, 'read_bytes', return_value=exe), patch.object(probe, 'EXE_SHA256', hashlib.sha256(exe).hexdigest()), patch.object(probe, 'LEVEL00_SHA256', hashlib.sha256(bytes(probe.LEVEL00_SIZE)).hexdigest()):
            result = probe.sample(call)
        self.assertTrue(set(commands) <= {'frame', 'read_ram'})
        return result

    def test_live_code_mismatch_refused(self):
        with self.assertRaisesRegex(ValueError, 'code identity mismatch'):
            self.exercise(corrupt_code=True)

    def test_backreference_mismatch_refused(self):
        with self.assertRaisesRegex(ValueError, 'back-reference mismatch'):
            self.exercise(corrupt_pointer=True)

    def test_overlay_mismatch_refused(self):
        with self.assertRaisesRegex(ValueError, 'overlay identity mismatch'):
            self.exercise(corrupt_overlay=True)

    def test_valid_probe_only_reads(self):
        result = self.exercise()
        self.assertEqual(result['player_base'], hex(probe.PLAYER))
        self.assertEqual(len(result['code_guards']), len(probe.GUARDS))
        self.assertEqual(result['player_position_candidate'], (0, 0, 0))


if __name__ == '__main__':
    unittest.main()
