import difflib
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools/local'))
import apply_runtime_patches as runtime


class RuntimePatchStackTest(unittest.TestCase):
    def test_overlapping_stack_clean_apply_repeat_and_conflict(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / 'patches').mkdir()
            (root / 'psxrecomp/runtime').mkdir(parents=True)
            source = root / 'psxrecomp/runtime/test.c'
            original = [f'line{i}\n' for i in range(20)]
            first = original.copy(); first[3] = 'first fix\n'
            second = first.copy(); second[4] = 'second fix\n'
            paths = []
            for name, a, b in [('a', original, first), ('z', first, second)]:
                path = root / f'patches/time-to-kill-{name}.patch'
                path.write_text(''.join(difflib.unified_diff(a, b, fromfile='a/runtime/test.c', tofile='b/runtime/test.c')))
                paths.append(path)
            source.write_text(''.join(original))
            with patch.object(runtime, 'ROOT', root):
                self.assertFalse(runtime.installed_stack(paths))
                runtime.main()
                self.assertEqual(source.read_text(), ''.join(second))
                self.assertTrue(runtime.installed_stack(paths))
                runtime.main()
                self.assertEqual(source.read_text(), ''.join(second))
                # Unrelated local edits survive the scratch reverse check.
                source.write_text(''.join(second) + 'unrelated local work\n')
                before = source.read_bytes()
                self.assertTrue(runtime.installed_stack(paths))
                self.assertEqual(source.read_bytes(), before)
                source.write_text(source.read_text().replace('second fix', 'conflicting work'))
                self.assertFalse(runtime.installed_stack(paths))
