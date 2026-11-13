"""D23W: AUTO internal resolution from the GPU probe (gpu_auto.py); no GPU needed."""
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools/local'))
import gpu_auto


class GpuAutoTest(unittest.TestCase):
    def test_scale_rule(self):
        # Field laptop (HD 5500): about 6.4 ms -> 2x; 3x needs <= 4.44 ms, 4x <= 2.5 ms.
        self.assertEqual([gpu_auto.scale_for(ms) for ms in (0.4, 2.5, 2.6, 4.44, 4.5, 6.4, 9.9, 10.1, 40)],
                         [4, 4, 3, 3, 2, 2, 2, 1, 1])

    def test_probe_is_cached_per_fingerprint_and_failures_are_not(self):
        with tempfile.TemporaryDirectory() as d:
            cache = Path(d) / 'gpu-probe.json'
            exe = Path(d) / 'game'
            exe.write_text('x')
            lines = []
            with patch.object(gpu_auto, 'run_probe', return_value={'error': 'context'}) as probe:
                self.assertEqual(gpu_auto.resolve(exe, cache, lines.append), gpu_auto.FALLBACK_SCALE)
                self.assertFalse(cache.exists())
                self.assertEqual(gpu_auto.cached_scale(cache), None)
            with patch.object(gpu_auto, 'run_probe', return_value={'ms': 6.4, 'renderer': 'r', 'version': 'v'}) as probe:
                self.assertEqual(gpu_auto.resolve(exe, cache, lines.append), 2)
                self.assertEqual(gpu_auto.resolve(exe, cache, lines.append), 2)
                self.assertEqual(probe.call_count, 1)
                self.assertEqual(json.loads(cache.read_text())['ms'], 6.4)
                self.assertEqual(gpu_auto.cached_scale(cache), 2)
                exe.write_text('rebuilt binary')   # another size: measured again
                self.assertEqual(gpu_auto.resolve(exe, cache, lines.append), 2)
                self.assertEqual(probe.call_count, 2)
            self.assertTrue(any('GPU probe unavailable' in line for line in lines))
            self.assertTrue(any(line.startswith('AUTO resolution: 2x') for line in lines))

    def test_probe_output_parsing(self):
        class Done:
            stdout = 'noise\npsxrecomp: gpu probe: ms=6.940 renderer="Mesa Intel(R) HD Graphics 5500 (BDW GT2)" version="4.6"\n'
        with patch.object(gpu_auto.subprocess, 'run', return_value=Done()):
            self.assertEqual(gpu_auto.run_probe('game'), {'ms': 6.94, 'renderer': 'Mesa Intel(R) HD Graphics 5500 (BDW GT2)', 'version': '4.6'})
        Done.stdout = 'psxrecomp: gpu probe: failed (shader link)\n'
        with patch.object(gpu_auto.subprocess, 'run', return_value=Done()):
            self.assertEqual(gpu_auto.run_probe('game'), {'error': 'shader link'})


if __name__ == '__main__':
    unittest.main()
