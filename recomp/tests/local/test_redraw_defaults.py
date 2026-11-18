import re
import unittest
from pathlib import Path

RUN = Path(__file__).resolve().parents[2] / 'tools/local/run.py'


class RedrawDefaultsTest(unittest.TestCase):
    """D23AK: the accepted redraw bundle is a Modernized launch default."""

    def test_bundle(self):
        text = RUN.read_text()
        m = re.search(r"MODERNIZED_REDRAW_DEFAULTS = \{(.*?)\}", text, re.S)
        self.assertIsNotNone(m)
        pairs = dict(re.findall(r"'([A-Z_]+)': '([^']*)'", m.group(1)))
        self.assertEqual(pairs, {'PSX_RW_LEAN': '1', 'PSX_RW_RESTORE': '1', 'PSX_PUBLISH_TRACK': '1',
                                 'PSX_PLACEMENT_RENDER': 'all', 'DNTTK_REDRAW_ONLY_OBJECTS': '1',
                                 'DNTTK_REPLAY_FLOOR_WAIT': '15'})

    def test_modernized_only_and_overridable(self):
        text = RUN.read_text()
        self.assertRegex(text, r"if modernized:\n\s+for key, value in MODERNIZED_REDRAW_DEFAULTS.items\(\): env.setdefault\(key, value\)")


if __name__ == '__main__':
    unittest.main()
