"""D62: run.py's build line tells the normal and the installed optimized build apart."""
import json
import os
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools/local'))
import optimized_build  # noqa: E402


class BuildLabelTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        local = pathlib.Path(self.tmp.name) / 'build-local'
        local.mkdir()
        self.exe = local / optimized_build.EXE_NAME
        self.exe.write_bytes(b'x' * 10)
        self.saved = optimized_build.INSTALLED
        optimized_build.INSTALLED = local / 'optimized-installed.json'

    def tearDown(self):
        optimized_build.INSTALLED = self.saved
        self.tmp.cleanup()

    def stamp(self, **kw):
        st = self.exe.stat()
        info = dict(kind='pgo', head='abc1234', built='2026-11-26 01:22:26',
                    size=st.st_size, mtime_ns=st.st_mtime_ns, **kw)
        optimized_build.INSTALLED.write_text(json.dumps(info))

    def test_no_stamp_is_normal(self):
        self.assertIn('normal', optimized_build.describe_build(self.exe))

    def test_installed_pgo(self):
        self.stamp()
        self.assertEqual(optimized_build.describe_build(self.exe),
                         'build: optimized LTO+PGO @abc1234, built 2026-11-26 01:22:26')

    def test_relinked_after_install_is_normal(self):
        self.stamp()
        self.exe.write_bytes(b'y' * 12)  # a normal relink replaced it
        self.assertIn('normal', optimized_build.describe_build(self.exe))

    def test_same_size_new_time_is_normal(self):
        self.stamp()
        st = self.exe.stat()
        os.utime(self.exe, ns=(st.st_atime_ns, st.st_mtime_ns + 10**9))
        self.assertIn('normal', optimized_build.describe_build(self.exe))

    def test_bad_stamp_is_normal(self):
        optimized_build.INSTALLED.write_text('{')
        self.assertIn('normal', optimized_build.describe_build(self.exe))


if __name__ == '__main__':
    unittest.main()
