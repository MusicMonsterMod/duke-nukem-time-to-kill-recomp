"""Offline oracle for the opt-in near-renderer provenance trace (no game launch)."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / 'build-local/ttk-near-test'
EXE = ROOT / 'disc/SLUS_005.83'


@unittest.skipUnless(BINARY.exists() and EXE.exists(), 'requires native harness and owned EXE')
class PrimitiveTrace(unittest.TestCase):
    def run_fixture(self, prefix=None, **extra):
        env = {k: v for k, v in os.environ.items() if not k.startswith('DNTTK_PRIMITIVE_TRACE')}
        if prefix:
            env['DNTTK_PRIMITIVE_TRACE'] = str(prefix)
        env.update(extra)
        return subprocess.check_output([str(BINARY), str(EXE)], env=env, text=True)

    def test_observational_trace_and_gate(self):
        baseline = self.run_fixture()
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory) / 'census'
            self.assertEqual(baseline, self.run_fixture(prefix))
            self.assertFalse(list(Path(directory).glob('*.jsonl')))
            Path(str(prefix) + '.enable').touch()
            self.assertEqual(baseline, self.run_fixture(prefix))
            paths = list(Path(directory).glob('*.jsonl'))
            self.assertEqual(len(paths), 1)
            rows = [json.loads(line) for line in paths[0].read_text().splitlines()]
            self.assertTrue(any(r['event'] == 'native_mesh' for r in rows))
            self.assertTrue(any(r['event'] == 'emit' for r in rows))
            meshes = [r for r in rows if r['event'] == 'mesh' and r['instance']]
            self.assertGreater(len({r['instance'] for r in meshes}), 1)
            self.assertEqual(len({r['mesh'] for r in meshes}), 1)
            calls = {r['call']: r for r in rows if r['event'] == 'mesh'}
            for r in rows:
                self.assertEqual(r['mesh'], calls[r['call']]['mesh'])
                self.assertEqual(r['instance'], calls[r['call']]['instance'])
                if r['event'] == 'emit':
                    self.assertGreater(r['face'], 0)
                    self.assertGreater(r['packet'], 0)
                    self.assertEqual(len(r['xy_depth']), 3)
                    self.assertTrue(0 <= r['slot'] < 2048)

    def test_bounded_output(self):
        with tempfile.TemporaryDirectory() as directory:
            prefix = Path(directory) / 'limited'
            Path(str(prefix) + '.enable').touch()
            self.run_fixture(prefix, DNTTK_PRIMITIVE_TRACE_LIMIT='3')
            rows = [json.loads(line) for p in Path(directory).glob('*.jsonl')
                    for line in p.read_text().splitlines()]
            self.assertEqual(len(rows), 4)
            self.assertEqual(rows[-1], {'event': 'limit'})


if __name__ == '__main__':
    unittest.main()
