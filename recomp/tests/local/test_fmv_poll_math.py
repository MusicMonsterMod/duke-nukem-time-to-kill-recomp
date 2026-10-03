"""Boundaries for the title-specific guest-time polling optimization."""
import ctypes
import os
from pathlib import Path
import random
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class PollMathTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        source = Path(cls.tmp.name) / 'wrapper.c'
        source.write_text('#include "fmv_poll_math.h"\n'
                          '#ifdef _WIN32\n#define API __declspec(dllexport)\n'
                          '#else\n#define API\n#endif\n'
                          'API uint32_t skip(uint32_t q,uint64_t d,uint32_t t) '
                          '{return ttk_poll_iterations(q,d,t);}\n'
                          'API int dma(unsigned c,uint32_t active,uint32_t a,uint32_t ch,'
                          'uint32_t n,uint32_t e){return ttk_poll_dma_safe(c,active,a,ch,n,e);}\n')
        library = source.with_suffix('.dll' if os.name == 'nt' else '.so')
        if os.name == 'nt' and shutil.which('cl'):
            command = ['cl', '/nologo', '/LD', '/W4', '/WX',
                       '/I' + str(ROOT / 'src/ttk'), str(source),
                       '/link', '/OUT:' + str(library)]
        else:
            compiler = shutil.which('cc') or shutil.which('gcc')
            if not compiler:
                raise RuntimeError('a C compiler is required for the polling safety tests')
            command = [compiler, '-shared', '-Wall', '-Wextra', '-Werror',
                       '-I', str(ROOT / 'src/ttk'), str(source), '-o', str(library)]
            if os.name != 'nt':
                command.append('-fPIC')
        subprocess.run(command, cwd=cls.tmp.name, check=True)
        cls.library = ctypes.CDLL(str(library))
        cls.skip = cls.library.skip
        cls.skip.argtypes = [ctypes.c_uint32, ctypes.c_uint64, ctypes.c_uint32]
        cls.skip.restype = ctypes.c_uint32
        cls.dma = cls.library.dma
        cls.dma.argtypes = [ctypes.c_uint32] * 6
        cls.dma.restype = ctypes.c_int

    @classmethod
    def tearDownClass(cls):
        if os.name == 'nt':
            import _ctypes
            _ctypes.FreeLibrary(cls.library._handle)
        cls.tmp.cleanup()

    def test_event_and_timeout_edges(self):
        for q, d, t, expected in [(0, 100, 9, 0), (47, 47, 9, 0),
                                  (47, 48, 9, 1), (47, 94, 9, 1),
                                  (47, 95, 9, 2), (47, 1000, 1, 0),
                                  (47, 1000, 2, 1), (47, 1000, 0, 0)]:
            self.assertEqual(self.skip(q, d, t), expected)

    def test_skips_never_cross_event_or_exhaust_timeout(self):
        rng = random.Random(583)
        for _ in range(20000):
            q = rng.randrange(1, 2**32)
            distance = rng.randrange(1, 2**64)
            timeout = rng.randrange(1, 2**32)
            count = self.skip(q, distance, timeout)
            self.assertLess(count * q, distance)
            self.assertLess(count, timeout)
            self.assertLessEqual(count * q, 1200000)

    def test_dma_protects_poll_code_and_descriptor(self):
        for address in (0xab370, 0xab384, 0xb81bc, 0xb8270,
                        0xe811c, 0xe8130, 0x120000):
            for channel in (1, 3):
                self.assertFalse(self.dma(channel, 1, address, 0, 1, 0x120000))
        self.assertTrue(self.dma(1, 1, 0x11fffc, 0, 1, 0x120000))
        self.assertFalse(self.dma(1, 1, 0x11fffc, 0, 2, 0x120000))
        self.assertTrue(self.dma(1, 1, 0x120004, 0, 10, 0x120000))

    def test_dma_rejects_wrap_reverse_and_unknown_channels(self):
        self.assertFalse(self.dma(1, 1, 0x1ffffc, 0, 2, 0x120000))
        self.assertFalse(self.dma(3, 1, 0x140000, 2, 1, 0x120000))
        for channel in (2, 4, 5, 6):
            self.assertFalse(self.dma(channel, 1, 0x140000, 0, 1, 0x120000))
            self.assertTrue(self.dma(channel, 0, 0x140000, 0, 1, 0x120000))
        self.assertTrue(self.dma(0, 1, 0x120000, 1, 100, 0x120000))
        self.assertFalse(self.dma(0, 1, 0x120000, 0, 100, 0x120000))


if __name__ == '__main__':
    unittest.main()
